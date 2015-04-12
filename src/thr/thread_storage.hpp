#ifndef THR_THREAD_STORAGE
#define THR_THREAD_STORAGE

#include "thr/generic.hpp"

#include <boost/scoped_array.hpp>

namespace thr
{
  class WorkerThread;

  /** \brief List of worker threads. */
  class ThreadStorage
  {
    private:
      /** Maximum number of threads. */
      const unsigned m_size;

      /** Array of threads. */
      boost::scoped_array<WorkerThread*> m_threads;

      /** Current number of threads. */
      unsigned m_count;

    public:
      /** \brief Tell if the storage is empty.
       *
       * \return True if yes, false if no.
       */
      bool empty() const
      {
        return (0 >= m_count);
      }

      /** \brief Get the last worker.
       *
       * \return Last worker in storage or NULL.
       */
      WorkerThread* last() const
      {
        if(m_count > 0)
        {
          return m_threads[m_count - 1];
        }
        return NULL;
      }

      /** \brief Get the current size of the storage.
       *
       * \return Number of threads in storage currently.
       */
      unsigned size() const
      {
        return m_count;
      }

    public:
      /** \brief Constructor.
       *
       * If 0 is specified or the size parameter omitted, use number of hardware
       * threads by default.
       *
       * \param psize Maximum number of threads. */
      ThreadStorage(unsigned psize = 0);

    public:
      /** Insert a worker thread into the first position in the storage.
       *
       * \param pthread Thread to insert.
       */
      void addFirst(WorkerThread* pthread);

      /** Insert a worker thread into the last position in the storage.
       *
       * \param pthread Thread to insert.
       */
      void addLast(WorkerThread* pthread);

      /** Notify all threads sleeping in this storage.
       *
       * \param pstorage Move woken threads into this storage.
       */
      void notifyAll(ThreadStorage *pstorage);

      /** Remove a worker thread from the storage.
       * 
       * \param pthread Thread to remove.
       */
      void remove(WorkerThread* pthread);
  };
}

#endif

