#ifndef THR_GENERIC_HPP
#define THR_GENERIC_HPP

#include "defaults.hpp"

#include <boost/function.hpp>
#include <boost/thread/thread.hpp>

namespace thr
{
  /// Convenience typedef.
  typedef boost::function<void()> Task;

  /// Convenience typedef.
  typedef boost::shared_ptr<boost::condition_variable> CondSptr;

  /// Convenience typedef.
  typedef boost::shared_ptr<boost::thread> ThreadSptr;

  /// Get hardware concurrency.
  ///
  /// Throws an error if the information is not available.
  extern unsigned hardware_concurrency();

  /// Get a timestamp in microseconds.
  ///
  /// The zero point of the timestamp is unspecified.
  ///
  /// \return Timestamp in microseconds.
  extern uint64_t nsec_get_timestamp();

  /// Sleep an amount of time in microseconds.
  ///
  /// \param op Sleep time in microseconds.
  extern void nsec_sleep(uint64_t op);
}

#endif
