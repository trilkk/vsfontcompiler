#include "thr/generic.hpp"

#if !defined(WIN32)
#include <sys/time.h>
#endif

using namespace thr;

unsigned thr::hardware_concurrency()
{
  unsigned ret = boost::thread::hardware_concurrency();

  if(ret == 0)
  {
    std::ostringstream sstr;
    sstr << "hardware concurrency information not available";
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }

  return ret;
}

uint64_t thr::usec_get_timestamp()
{
#if defined(WIN32)
  return static_cast<uint64_t>(timeGetTime()) * 1000;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 +
    static_cast<uint64_t>(tv.tv_usec);
#endif
}

void thr::usec_sleep(uint64_t op)
{
#if defined(WIN32)
  Sleep(static_cast<unsigned>(op / 1000));
#else
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(op / 1000000000);
  ts.tv_nsec = static_cast<long int>(op % 1000000000);
  nanosleep(&ts, NULL);
#endif
}

