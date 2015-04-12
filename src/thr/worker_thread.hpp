#ifndef THR_WORKER_THREAD_HPP
#define THR_WORKER_THREAD_HPP

#include "thr/thread_storage.hpp"

namespace thr
{
  /** \brief Thread construct with private synchronization.
   */
  class WorkerThread
  {
    private:
      enum State
      {
        /** Thread is woken up, but not requested to do anything. */
        ACTIVE,

        /** Thread is woken up, and requested to work. */
        REQUIRED,

        /** Thread is asleep. */
        SLEEPING
      };

    private:
      /** Thread. */
      boost::thread m_thread;

      /** Thread id. */
      boost::thread::id m_id;

      /** Condition variable to sleep in. */
      boost::condition_variable m_cond;

      /** Curretn storage. */
      ThreadStorage *m_storage;

      /** Current Storage index. */
      unsigned m_storage_index;

      /** State of the thread. */
      State m_state;

    public:
      /** \brief Get the storage of this thread.
       *
       * \return Storage.
       */
      ThreadStorage* getStorage() const
      {
        return m_storage;
      }

      /** \brief Get the storage index of this thread.
       *
       * \return Storage index.
       */
      unsigned getStorageIndex() const
      {
        return m_storage_index;
      }

      /** \brief Get the ID of this thread.
       *
       * \return Thread ID.
       */
      boost::thread::id id() const
      {
        return m_id;
      }

      /** \brief Tell if the thread is sleeping.
       *
       * \return True if yes, false if no.
       */
      bool isSleeping() const
      {
        return (SLEEPING == m_state);
      }

      /** \brief Tell if the thread is uninitialized.
       *
       * \return True if yes, false if no.
       */
      bool isUninitialized() const
      {
        return (boost::thread::id() == m_id);
      }

      /** \brief Set the storage of this thread.
       *
       * \param pstorage Storage.
       */
      void setStorage(ThreadStorage *pstorage)
      {
        m_storage = pstorage;
      }

      /** Set the storage index of this thread.
       *
       * \param pidx Storage index.
       */
      void setStorageIndex(unsigned pidx)
      {
        m_storage_index = pidx;
      }

    public:
      /** \brief Constructor template.
       *
       * \param pfunc Function to execute in the thread.
       */
      template<typename T> WorkerThread(T pfunc) :
        m_thread(pfunc),
        m_id(m_thread.get_id()),
        m_state(ACTIVE) { }

      /** \brief Empty constructor.
       */
      WorkerThread() :
        m_state(ACTIVE) { }

    public:
      /** \brief Take control of a thread.
       *
       * Will set the id of current thread to the id of this thread object.
       */
      void acquire()
      {
        BOOST_ASSERT(this->isUninitialized());

        m_id = boost::this_thread::get_id();
      }

      /** \brief Join this thread.
       *
       * If thread was constructed using thread id, do nothing.
       */
      void join()
      {
        boost::thread::id nat;

        if(m_thread.get_id() != nat)
        {
          m_thread.join();
        }
      }

      /** \brief Signal This thread to wake up.
       *
       * Signalling a thread always moves it into the storage of active workers.
       *
       * Do not write the 'needed' flag for the thread. It is not expected to
       * require it.
       */
      void notify()
      {
        m_state = ACTIVE;
        m_cond.notify_one();
      }

      /** \brief Signal This thread to wake up.
       *
       * Signalling a thread always moves it into the storage of active workers.
       *
       * \param pneeded Flag telling the thread if it's needed for work.
       */
      void notify(bool pneeded)
      {
        m_state = pneeded ? REQUIRED : ACTIVE;
        m_cond.notify_one();
      }

      /** \brief Suspend this thread.
       *
       * When returning from function, don't care if the thread was needed for
       * work or not.
       *
       * \param pscope Lock to release.
       */
      void suspend(boost::mutex::scoped_lock &pscope)
      {
        m_state = SLEEPING;
        m_cond.wait(pscope);
      }

      /** \brief Wait on a condition, then wake up.
       *
       * When returning from this function, return a value depending if the thread
       * is still needed for work or not.
       *
       * \param pscope Lock to release.
       * \return True if not free to go.
       */
      bool wait(boost::mutex::scoped_lock &pscope)
      {
        m_state = SLEEPING;
        m_cond.wait(pscope);
        return (m_state == REQUIRED);
      }
  };
}

#endif

