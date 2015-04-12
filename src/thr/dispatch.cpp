#include "thr/dispatch.hpp"

#include "data/circular_buffer.hpp"
#include "thr/promise.hpp"
#include "thr/worker_thread.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/thread/condition_variable.hpp>

using namespace thr;

/** Convenience typedef. */
typedef boost::shared_ptr<ThreadStorage> ThreadStorageSptr;

/** Convenience typedef. */
typedef boost::shared_ptr<WorkerThread> WorkerThreadSptr;

/** Convenience typedef. */
typedef std::map<boost::thread::id, WorkerThreadSptr> ThreadMap;

/** Active workers. */
static ThreadStorage workers_active;

/** Suspended workers that are available for work. */
static ThreadStorage workers_sleeping;

/** Workers waiting on a collection point. */
static ThreadStorage workers_waiting;

/** Temporary condition variable storage. */
static std::vector<CondSptr> temporary_cond_storage;

/** Temporary condition variable queue. */
static data::CircularBuffer<boost::condition_variable*> temporary_conds;

/** The main privileged thread. */
static WorkerThread privileged_thread;

/** Thread list. */
static ThreadMap threads;

/** Task list. */
static data::CircularBuffer<Task> tasks_normal;

/** High-priority task list. */
static data::CircularBuffer<Promise*> tasks_important;

/** Privileged task list. */
static data::CircularBuffer<Task> tasks_privileged;

/** The dispatcher needs to be guarded. */
static boost::mutex mut;

/** Sleeping area for anyone waiting for normal task queue to be emptied. */
static boost::condition_variable cond_wait_normal;

/** Sleeping area for anyone waiting for privileged task queue to be emptied. */
static boost::condition_variable cond_wait_privileged;

/** True if quitting the dispatching system. */
static bool quitting = false;

/** \brief Tell if given thread is the primary thread.
 *
 * If the thread system has not been initialized, will always return false.
 *
 * \param op Thread id.
 */
static inline bool is_primary_thread(const boost::thread::id &op)
{
  return (privileged_thread.id() == op);
}

/** \brief Tell if this is the primary thread.
  *
  * If the thread system has not been initialized, will always return false.
  */
static inline bool is_primary_thread()
{
  return is_primary_thread(boost::this_thread::get_id());
}

/** \brief Get string representation of a thread.
 *
 * \param tid Thread ID to represent.
 * \return String representation.
 */
static inline std::string THRID(boost::thread::id tid)
{
  if(is_primary_thread(tid))
  {
    std::ostringstream sstr;
    sstr << "privileged thread " << tid;
    return sstr.str();
  }

  if(threads.find(tid) != threads.end())
  {
    std::ostringstream sstr;
    sstr << "worker thread " << tid;
    return sstr.str();
  }

  std::ostringstream sstr;
  sstr << "thread " << tid;
  return sstr.str();
}

/** \brief Wrapper to string representation of a thread.
 *
 * Uses ID of current thread.
 *
 * \return String representation.
 */
static inline std::string THRID()
{
  return THRID(boost::this_thread::get_id());
}

/** \brief Inner common implementation of dispatch.
 */
static void inner_dispatch()
{
  WorkerThread* thr = workers_waiting.last();
  if(NULL != thr)
  {
    workers_waiting.remove(thr);
    workers_active.addLast(thr);
    thr->notify(true); // Needed for work.
    return;
  }

  thr = workers_sleeping.last();
  if(NULL != thr)
  {
    workers_sleeping.remove(thr);
    workers_active.addLast(thr);
    thr->notify();
    return;
  }
}

/** \brief Inner common implementation of dispatch_privileged.
 *
 * \param pfunctor Task to execute.
 * \return Task id created.
 */
static void inner_dispatch_privileged(const Task &pfunctor)
{
  tasks_privileged.put(pfunctor);

  if(privileged_thread.isSleeping())
  {
    ThreadStorage *storage = privileged_thread.getStorage();

    storage->remove(&privileged_thread);
    workers_active.addLast(&privileged_thread);
    privileged_thread.notify(true);
  }
}

/** \brief Inner task running, important (promised) tasks.
 *
 * \param pscope Previously created scoped lock.
 * \return True if executed something, false if not.
 */
static bool inner_run_important(boost::mutex::scoped_lock &pscope)
{
  if(tasks_important.empty())
  {
    return false;
  }
  Promise *promise = tasks_important.get();
  pscope.unlock();
  promise->task();
  pscope.lock();
  return true;
}

/** \brief Inner task running, normal tasks.
 *
 * \param pscope Previously created scoped lock.
 * \return True if executed something, false if not.
 */
static bool inner_run_normal(boost::mutex::scoped_lock &pscope)
{
  if(tasks_normal.empty())
  {
    return false;
  }
  Task functor = tasks_normal.get();
  pscope.unlock();
  functor();
  pscope.lock();
  return true;
}

/** \brief Inner task running, privileged tasks.
 *
 * \param tlist Task list.
 * \param scope Previously created scoped lock.
 * \return True if executed something, false if not.
 */
static bool inner_run_privileged(boost::mutex::scoped_lock &pscope)
{
  if(tasks_privileged.empty())
  {
    return false;
  }
  Task functor = tasks_privileged.get();
  pscope.unlock();
  functor();
  pscope.lock();
  return true;
}

/** \brief Acquire a temporary condition variable.
 *
 * Must be called from a locked context.
 *
 * \return Condition variable that must be released later.
 */
static boost::condition_variable* temporary_cond_acquire()
{
  if(temporary_conds.empty())
  {
    CondSptr ret(new boost::condition_variable());
    temporary_cond_storage.push_back(ret);
    return ret.get();
  }
  return temporary_conds.get();
}

/** \brief Release a previously acquired temporary condition variable.
 *
 * Must be called from a locked context.
 *
 * \param op Condition variable to release.
 */
static void temporary_cond_release(boost::condition_variable *op)
{
  temporary_conds.put(op);
}

/** \brief Wake up anyone waiting for normal tasks to complete.
 *
 * Must be called from a locked context.
 *
 * Do not call this function if normal task queue is not empty.
 *
 * If the caller is a worker thread, it must be in the active worker storage.
 *
 * \param tid Thread id.
 */
static bool wake_normal()
{
  if(1 != workers_active.size())
  {
    return false;
  }
  // We were the only worker active.
  workers_waiting.notifyAll(&workers_active);
  cond_wait_normal.notify_all();
  return true;
}

/** \brief Clean up important jobs if we're a normal thread.
 *
 * Ensured that an important job can't queue up other important jobs that could potentially create deadlocks.
 *
 * \param pfunctor Task to execute.
 * \param scope Previously created scoped lock.
 * \param tid Id of this thread.
 * \return True if cleaned up the queue, false if not.
 */
static bool cleanup_important(const Task &pfunctor, boost::mutex::scoped_lock &pscope, boost::thread::id tid)
{
  if(threads.end() == threads.find(tid))
  {
    return false;
  }
  while(inner_run_important(pscope));
  pscope.unlock();
  pfunctor();
  pscope.lock();
  return true;
}

/** \brief Clean up privileged jobs if we're a privileged thread.
 *
 * Ensures that a privileged job can't queue up other privileged jobs that would potentially create deadlocks.
 *
 * \param pfunctor Task to execute.
 * \param scope Previously created scoped lock.
 * \param tid Id of this thread.
 * \return True if cleaned up the queue, false if not.
 */
static bool cleanup_privileged(const Task &pfunctor, boost::mutex::scoped_lock &pscope, boost::thread::id tid)
{
  if(!is_primary_thread(tid))
  {
    return false;
  }
  while(inner_run_privileged(pscope));
  pscope.unlock();
  pfunctor();
  pscope.lock();
  return true;
}

/** \brief The normal thread run function.
 *
 * The function to be started by normal workers.
 */
static void run_normal()
{
  boost::mutex::scoped_lock scope(mut);
  boost::thread::id tid = boost::this_thread::get_id();
  WorkerThread* thr = (*threads.find(tid)).second.get();

  while(!quitting)
  {
    if(inner_run_important(scope))
    {
      continue;
    }
    if(inner_run_normal(scope))
    {
      continue;
    }
    wake_normal();
    workers_active.remove(thr);
    workers_sleeping.addLast(thr);
    thr->suspend(scope);
  }

  // Remove from active workers before exiting.
  workers_active.remove(thr);
}

void thr::dispatch_ext(const Task &pfunctor)
{
  boost::mutex::scoped_lock scope(mut);

  tasks_normal.put(pfunctor);

  inner_dispatch();
}

void thr::dispatch_privileged_ext(const Task &pfunctor)
{
  boost::thread::id tid = boost::this_thread::get_id();
  boost::mutex::scoped_lock scope(mut);

  if(cleanup_privileged(pfunctor, scope, tid))
  {
    return;
  }

  inner_dispatch_privileged(pfunctor);
}

void thr::thr_init()
{
  if(!privileged_thread.isUninitialized())
  {
    std::stringstream sstr;
    sstr << "thread system already initialized";
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }

  privileged_thread.acquire();
  workers_active.addLast(&privileged_thread);
}

void thr::thr_main(unsigned nthreads)
{
  if(privileged_thread.isUninitialized())
  {
    thr_init();
  }
  quitting = false;

  if(!is_primary_thread())
  {
    std::ostringstream sstr;
    sstr << "trying to enter main loop from unprivileged thread";
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }

  if(0 >= nthreads)
  {
    nthreads = thr::hardware_concurrency() - 1;
  }

  // Lock this scope only.
  {
    boost::mutex::scoped_lock scope(mut);

    // Add other threads.
    while(threads.size() < nthreads)
    {
      WorkerThreadSptr thr(new WorkerThread(run_normal));

      threads[thr->id()] = thr;
      workers_active.addLast(thr.get());
    }

    while(!quitting)
    {
      if(inner_run_privileged(scope))
      {
        continue;
      }

      // In main loop and the list of privileged tasks was emptied, can safely
      // wake up all other tasks waiting for it.
      cond_wait_privileged.notify_all();

      if(inner_run_important(scope))
      {
        continue;
      }

      if(inner_run_normal(scope))
      {
        continue;
      }

      // Nothing to do, perform potential wakeup for collection point.
      wake_normal();

      workers_active.remove(&privileged_thread);
      workers_sleeping.addFirst(&privileged_thread);
      privileged_thread.suspend(scope);
    }

    scope.unlock();
    BOOST_FOREACH(const ThreadMap::value_type &vv, threads)
    {
      vv.second->join();
    }
    scope.lock();
    threads.clear();
  }
}

void thr::thr_quit()
{
  boost::mutex::scoped_lock scope(mut);

  quitting = true;

  // Wake all threads.
  workers_waiting.notifyAll(&workers_active);
  workers_sleeping.notifyAll(&workers_active);

  // Wake all waiters, their wishes will not be fullfilled.
  cond_wait_normal.notify_all();
  cond_wait_privileged.notify_all();

  // Clear all tasks, they will not be done.
  tasks_normal.clear();
  tasks_important.clear();
  tasks_privileged.clear();
}

void thr::wait()
{
  boost::thread::id tid = boost::this_thread::get_id();
  boost::mutex::scoped_lock scope(mut);

  // Privileged thread has a special loop.
  if(is_primary_thread(tid))
  {
    while(!quitting)
    {
      if(inner_run_privileged(scope))
      {
        continue;
      }

      if(inner_run_important(scope))
      {
        continue;
      }

      if(inner_run_normal(scope))
      {
        continue;
      }

      if(wake_normal())
      {
        return;
      }

      workers_active.remove(&privileged_thread);
      workers_waiting.addFirst(&privileged_thread);
      if(!privileged_thread.wait(scope))
      {
        return;
      }
    }
    return;
  }

  // All normal workers have a simpler loop.
  ThreadMap::iterator iter = threads.find(tid);
  if(threads.end() != iter)
  {
    WorkerThread* thr = (*iter).second.get();

    while(!quitting)
    {
      if(inner_run_important(scope))
      {
        continue;
      }

      if(inner_run_normal(scope))
      {
        continue;
      }

      if(wake_normal())
      {
        return;
      }

      workers_active.remove(thr);
      workers_waiting.addLast(thr);
      if(!thr->wait(scope))
      {
        return;
      }
    }
    return;
  }

  if(tasks_normal.empty() && workers_active.empty())
  {
    return;
  }
  cond_wait_normal.wait(scope);
}

void thr::wait_ext(const Task &pfunctor)
{
  boost::thread::id tid = boost::this_thread::get_id();
  boost::mutex::scoped_lock scope(mut);

  if(cleanup_privileged(pfunctor, scope, tid))
  {
    return;
  }

  if(cleanup_important(pfunctor, scope, tid))
  {
    return;
  }

  boost::condition_variable *temporary_cond = temporary_cond_acquire();
  Promise pr(pfunctor, temporary_cond);
  tasks_important.put(&pr);
  inner_dispatch();
  pr.wait(scope);
  temporary_cond_release(temporary_cond);
}

void thr::wait_privileged_ext(const Task &pfunctor)
{
  boost::thread::id tid = boost::this_thread::get_id();
  boost::mutex::scoped_lock scope(mut);

  if(cleanup_privileged(pfunctor, scope, tid))
  {
    return;
  }

  inner_dispatch_privileged(pfunctor);
  cond_wait_privileged.wait(scope);
}

