#ifndef THR_DISPATCH_HPP
#define THR_DISPATCH_HPP

#include "thr/generic.hpp"

namespace thr
{
  /** \brief Add a job.
   *
   * Anyone may execute this job.
   *
   * This function will never block, the job will always be added to the queue for execution. The actual time
   * of execution is undetermined.
   *
   * \param pfunctor Functor to store.
   */
  extern void dispatch_ext(const Task &pfunctor);

  /** \brief Add a privileged job.
   *
   * Add a primary, privileged job for execution. Only the privileged main executor may execute this job. A
   * typical job of this kind would be for example any OpenGL call.
   *
   * Privileged jobs are executed in a first-in-first-out -manner. The usual method would be to dispatch
   * privileged jobs until no jobs remain, then call wait_privileged(const Task&) with a screen flip task,
   * which would return when all privileged tasks have been executed.
   *
   * If called from the privileged thread, start executing privileged functions and continue until privileged
   * job queue is empty, then execute given function and return.
   *
   * \param pfunctor Functor to store.
   */
  extern void dispatch_privileged_ext(const Task &pfunctor);

  /** \brief Initialize threading system.
   *
   * Must be called from the main thread before any other threading calls are
   * invoked.
   */
  extern void thr_init();

  /** \brief Enter threading main loop.
   *
   * This function should be called from the main thread, it will block until thr_quit is called. The main
   * thread calling will stay here executing primary tasks.
   *
   * \param ntasks Additional tasks to create. 0 to autodetect (default).
   */
  extern void thr_main(unsigned ntasks = 0);

  /** \brief Stop threading and dispatching.
   *
   * Orders all threads to quit, including the main thread, which will join all other threads before returning
   * (thus being the last thread to stop running in a dispatcher).
   */
  extern void thr_quit();

  /** \brief Wait until all outstanding jobs are done.
   *
   * If no jobs are in execution, this function returns immediately.
   *
   * If called from within a worker thread, also execute the jobs until the job queue is empty.
   *
   * If called from a primary thread, execute all jobs with priority on the privileged jobs until no jobs of
   * any kind remain.
   */
  extern void wait();

  /** \brief Add an important job and wait for it to complete.
   *
   * If called from a privileged thread, execution of privileged functions still has higher priority than the
   * given higher-priority task.
   *
   * If called from within a worker thread, execute important jobs until the given job has been completed.
   *
   * \param pfunctor Functor to store.
   */
  extern void wait_ext(const Task &pfunctor);

  /** \brief Add a privileged job and wait for it to complete.
   *
   * As dispatch_privileged(const Task&), but always block until the privileged job queue is empty.
   *
   * If called from the privileged thread, start executing privileged functions and continue until privileged
   * job queue is empty, then execute given function and return.
   *
   * \param pfunctor Functor to store.
   */
  extern void wait_privileged_ext(const Task &pfunctor);

  /** \brief Wrapper for dispatch_ext.
   *
   * \param op Any binding.
   */
  template <typename Type> inline void dispatch(Type op)
  {
    dispatch_ext(op);
  }
  /** \cond */
  template <typename T0, typename T1>
  inline void dispatch(T0 op0, T1 op1)
  {
    dispatch_ext(boost::bind(op0, op1));
  }
  template <typename T0, typename T1, typename T2>
  inline void dispatch(T0 op0, T1 op1, T2 op2)
  {
    dispatch_ext(boost::bind(op0, op1, op2));
  }
  template <typename T0, typename T1, typename T2, typename T3>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3, op4));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3, op4, op5));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  inline void dispatch(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8, T9 op9)
  {
    dispatch_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8, op9));
  }
  /** \endcond */

  /** \brief Wrapper for dispatch_privileged_ext.
   *
   * \param op Any binding.
   */
  template <typename Type> inline void dispatch_privileged(Type op)
  {
    dispatch_privileged_ext(op);
  }
  /** \cond */
  template <typename T0, typename T1>
  inline void dispatch_privileged(T0 op0, T1 op1)
  {
    dispatch_privileged_ext(boost::bind(op0, op1));
  }
  template <typename T0, typename T1, typename T2>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2));
  }
  template <typename T0, typename T1, typename T2, typename T3>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3, op4));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  inline void dispatch_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8, T9 op9)
  {
    dispatch_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8, op9));
  }
  /** \endcond */

  /** \brief Wait until all outstanding jobs are done.
   *
   * As wait(), but add one job before continuing.
   *
   * \param op Any binding.
   */
  template <typename Type> inline void wait(Type op)
  {
    wait_ext(op);
  }
  /** \cond */
  template <typename T0, typename T1>
  inline void wait(T0 op0, T1 op1)
  {
    wait_ext(boost::bind(op0, op1));
  }
  template <typename T0, typename T1, typename T2>
  inline void wait(T0 op0, T1 op1, T2 op2)
  {
    wait_ext(boost::bind(op0, op1, op2));
  }
  template <typename T0, typename T1, typename T2, typename T3>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3)
  {
    wait_ext(boost::bind(op0, op1, op2, op3));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4)
  {
    wait_ext(boost::bind(op0, op1, op2, op3, op4));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5)
  {
    wait_ext(boost::bind(op0, op1, op2, op3, op4, op5));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6)
  {
    wait_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7)
  {
    wait_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8)
  {
    wait_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  inline void wait(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8, T9 op9)
  {
    wait_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8, op9));
  }
  /** \endcond */

  /** \brief Wrapper for wait_privileged_ext.
   *
   * As wait(), but add one job before continuing.
   *
   * \param op Any binding.
   */
  template <typename Type> inline void wait_privileged(Type op)
  {
    wait_privileged_ext(Task(op));
  }
  /** \cond */
  template <typename T0, typename T1>
  inline void wait_privileged(T0 op0, T1 op1)
  {
    wait_privileged_ext(boost::bind(op0, op1));
  }
  template <typename T0, typename T1, typename T2>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2));
  }
  template <typename T0, typename T1, typename T2, typename T3>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3, op4));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8));
  }
  template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
  inline void wait_privileged(T0 op0, T1 op1, T2 op2, T3 op3, T4 op4, T5 op5, T6 op6, T7 op7, T8 op8, T9 op9)
  {
    wait_privileged_ext(boost::bind(op0, op1, op2, op3, op4, op5, op6, op7, op8, op9));
  }
  /** \endcond */
}

#endif
