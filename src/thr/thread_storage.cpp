#include "thr/thread_storage.hpp"

#include "thr/worker_thread.hpp"

using namespace thr;

ThreadStorage::ThreadStorage(unsigned psize) :
  m_size((psize > 0) ? psize : thr::hardware_concurrency()),
  m_threads(new WorkerThread*[m_size]),
  m_count(0) { }

void ThreadStorage::addFirst(WorkerThread *pthread)
{
  BOOST_ASSERT(m_count < m_size);

  if(m_count > 0)
  {
    WorkerThread* thr = m_threads[0];

    thr->setStorageIndex(m_count);
    m_threads[m_count] = thr;
  }

  pthread->setStorage(this);
  pthread->setStorageIndex(0);
  m_threads[0] = pthread;
  ++m_count;
}

void ThreadStorage::addLast(WorkerThread *pthread)
{
  BOOST_ASSERT(m_count < m_size);

  pthread->setStorage(this);
  pthread->setStorageIndex(m_count);
  m_threads[m_count++] = pthread;
}

void ThreadStorage::notifyAll(ThreadStorage *pstorage)
{
  for(unsigned ii = 0; (ii < m_count); ++ii)
  {
    WorkerThread* thr = m_threads[ii];

    thr->notify();
    pstorage->addLast(thr);
  }

  m_count = 0;
}

void ThreadStorage::remove(WorkerThread* pthread)
{
  unsigned index = pthread->getStorageIndex();
  unsigned last_index = --m_count;

  BOOST_ASSERT(pthread->getStorage() == this);

  if(last_index != index)
  {
    WorkerThread* thr = m_threads[last_index];
    
    thr->setStorageIndex(index);
    m_threads[index] = thr;
  }
}

