#ifndef THR_PROMISE_HPP
#define THR_PROMISE_HPP

#include "thr/generic.hpp"

#include <boost/thread/future.hpp>

#include <iostream>

namespace thr
{
 /** \brief Promise.
  *
  * Promise is an abstract concept of a job that has been promised to be fullfilled.
  *
  * Used for waiting for a specific task to be performed.
  */
  class Promise
  {
    private:
      /** Task to execute. */
      Task m_task;

      /** Condition variable to use for fulfilling the promise. */
      boost::condition_variable *m_cond;

    public:
      /** \brief Constructor.
       *
       * \param pfunctor Function to run.
       * \param pcond Condition variable to signal.
       */
      Promise(const Task& pfunctor, boost::condition_variable *pcond) :
        m_task(pfunctor),
        m_cond(pcond) { }

    public:
      /** \brief Run the task in the promise.
       *
       * Implicitly fulfills the promise.
       */
      void task()
      {
        m_task();
        m_cond->notify_all();
      }

      /** \brief Wait for the promise to be fulfilled.
       *
       * \param pscope Scoped lock to release for the duration of wait.
       */
      void wait(boost::mutex::scoped_lock &pscope)
      {
        m_cond->wait(pscope);
      }
  };
}


#endif

