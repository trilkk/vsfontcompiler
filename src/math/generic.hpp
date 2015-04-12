#ifndef MATH_GENERIC_HPP
#define MATH_GENERIC_HPP

#include "defaults.hpp"

#include <cfloat>
#include <cmath>

namespace math
{
  /** \brief Absolute value.
   *
   * \param v Value.
   * \return Absolute value of v.
   */
  template<typename T> inline T abs(const T &v);
  /** \cond */
  template<> inline int abs(const int &v)
  {
    return (v < 0) ? -v : v;
  }
  template<> inline float abs(const float &v)
  {
    return ::fabsf(v);
  }
  template<> inline double abs(const double &v)
  {
    return ::fabs(v);
  }
  /** \endcond */

  /** \brief Argus cos.
   *
   * \param v value.
   * \return Argus sin of value.
   */
  template <typename T> inline T acos(const T &v);
  /** \cond */
  template<> inline float acos(const float &v)
  {
    return ::acosf(v);
  }
  template<> inline double acos(const double &v)
  {
    return ::acos(v);
  }
  /** \end */

  /** \brief Argus sin.
   *
   * \param v value.
   * \return Argus sin of value.
   */
  template <typename T> inline T asin(const T &v);
  /** \cond */
  template<> inline float asin(const float &v)
  {
    return ::asinf(v);
  }
  template<> inline double asin(const double &v)
  {
    return ::asin(v);
  }
  /** \end */

  /** \brief Argus tangent.
   *
   * \param v value.
   * \return Argus tangent of value.
   */
  template <typename T> inline T atan(const T &v);
  /** \cond */
  template<> inline float atan(const float &v)
  {
    return ::atanf(v);
  }
  template<> inline double atan(const double &v)
  {
    return ::atan(v);
  }
  /** \end */

  /** \brief Ceil wrapper.
   *
   * Unlike normal ceil(), this returns an integer.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline int ceil(const T &v);
  /** \cond */
  template<> inline int ceil(const float &v)
  {
    return static_cast<int>(::ceilf(v));
  }
  template<> inline int ceil(const double &v)
  {
    return static_cast<int>(::ceil(v));
  }
  /** \endcond */

  /** \brief Congruence.
   *
   * Like modulo, but always returns a positive number.
   *
   * \param v Value.
   * \param m Modulo.
   * \return Congruence.
   */
  template<typename T> inline T congr(const T &v, const T &m);
  /** \cond */
  template<> inline int congr(const int &v, const int &m)
  {
    int ret = v % m;
    return (ret < 0) ? (m + ret) : ret;
  }
  template<> inline float congr(const float &v, const float &m)
  {
    float ret = ::fmodf(v, m);
    return (ret < 0.0f) ? (ret + m) : ret;
  }
  template<> inline double congr(const double &v, const double &m)
  {
    double ret = ::fmod(v, m);
    return (ret < 0.0) ? (ret + m) : ret;
  }
  /** \endcond */

  /** \brief Cos wrapper.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline T cos(const T &v);
  /** \cond */
  template<> inline float cos(const float &v)
  {
    return ::cosf(v);
  }
  template<> inline double cos(const double &v)
  {
    return ::cos(v);
  }
  /** \endcond */

  /** \brief Cot wrapper.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline T cot(const T &v);
  /** \cond */
  template<> inline float cot(const float &v)
  {
    return ::tanf(static_cast<float>(M_PI * 0.5) - v);
  }
  template<> inline double cot(const double &v)
  {
    return ::tan(static_cast<double>(M_PI * 0.5) - v);
  }
  /** \endcond */

  /** \brief Floor wrapper.
   *
   * Unlike normal floor(), this returns an integer.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline int floor(const T &v);
  /** \cond */
  template<> inline int floor(const float &v)
  {
    return static_cast<int>(::floorf(v));
  }
  template<> inline int floor(const double &v)
  {
    return static_cast<int>(::floor(v));
  }
  /** \endcond */

  /** \brief Convert from floating point to integer correctly.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline int lround(const T &v);
  /** \cond */
  template <> inline int lround(const float &v)
  {
#ifdef WIN32
    return static_cast<int>(v + 0.5f);
#else
    return static_cast<int>(::lroundf(v));
#endif
  }
  template <> inline int lround(const double &v)
  {
#ifdef WIN32
    return static_cast<int>(v + 0.5);
#else
    return static_cast<int>(::lround(v));
#endif
  }
  /** \endcond */

  /** \brief Max wrapper.
   *
   * \param v1 First value.
   * \param v2 Second value.
   */
  template<typename T> inline T max(const T & v1, const T & v2)
  {
    return std::max(v1, v2);
  }
#ifndef WIN32
  template<> inline float max(const float &v1, const float &v2)
  {
    return ::fmaxf(v1, v2);
  }
  template<> inline double max(const double &v1, const double &v2)
  {
    return ::fmax(v1, v2);
  }
#endif

  /** \brief Multiple max wrapper.
   *
   * \param v1 First value.
   * \param v2 Second value.
   * \param v3 Third value.
   */
  template<typename T> inline T max(const T &v1, const T &v2, const T &v3)
  {
    return max(v1, max(v2, v3));
  }

  /** \brief Multiple max wrapper.
   *
   * \param v1 First value.
   * \param v2 Second value.
   * \param v3 Third value.
   * \param v4 Fourth value.
   */
  template<typename T> inline T max(const T &v1, const T &v2, const T &v3, const T &v4)
  {
    return max(v1, max(v2, max(v3, v4)));
  }

  /** \brief Min wrapper.
   *
   * \param v1 First value.
   * \param v2 Second value.
   */
  template<typename T> inline T min(const T &v1, const T &v2)
  {
    return std::min(v1, v2);
  }
  #ifndef WIN32
    template<> inline float min(const float &v1, const float &v2)
    {
      return ::fminf(v1, v2);
    }
    template<> inline double min(const double &v1, const double &v2)
    {
      return ::fmin(v1, v2);
    }
  #endif

  /** \brief Multiple min wrapper.
   *
   * \param v1 First value.
   * \param v2 Second value.
   * \param v3 Third value.
   */
  template<typename T>
  inline T min(const T &v1, const T &v2, const T &v3)
  {
    return min(v1, min(v2, v3));
  }

  /** \brief Multiple min wrapper.
   *
   * \param v1 First value.
   * \param v2 Second value.
   * \param v3 Third value.
   * \param v4 Fourth value.
   */
  template<typename T> inline T min(const T &v1, const T &v2, const T &v3, const T &v4)
  {
    return min(v1, min(v2, min(v3, v4)));
  }

  /** \brief OpenGL -style mix().
   *
   * \param fl Flor value.
   * \param cl Ceil value.
   * \param ii Interpolation value.
   */
  template <typename T> inline T mix(const T &fl, const T &cl, const T &ii);
  /** \cond **/
  template<> inline float mix(const float &fl, const float &cl, const float &ii)
  {
    return fl + (cl - fl) * ii;
  }
  template<> inline double mix(const double &fl, const double &cl, const double &ii)
  {
    return fl + (cl - fl) * ii;
  }
  /** \endcond */

  /** \brief Pow wrapper.
   *
   * \param v Value.
   * \param p Power.
   * \return Result.
   */
  template<typename T> inline T pow(const T &v, const T &p);
  /** \cond */
  template<> inline float pow(const float &v, const float &p)
  {
    return ::powf(v, p);
  }
  template<> inline double pow(const double &v, const double &p)
  {
    return ::pow(v, p);
  }
  /** \endcond */

  /** \brief Modulo wrapper.
   *
   * \param v Value.
   * \param m Modulo.
   */
  template<typename T> inline T mod(const T &v, const T &m);
  /** \cond */
  template<> inline float mod(const float &v, const float &m)
  {
    return ::fmodf(v, m);
  }
  template<> inline double mod(const double &v, const double &m)
  {
    return ::fmod(v, m);
  }
  /** \endcond */

  /** \brief Sin wrapper.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline T sin(const T &v);
  /** \cond */
  template<> inline float sin(const float &v)
  {
    return ::sinf(v);
  }
  template<> inline double sin(const double &v)
  {
    return ::sin(v);
  }
  /** \endcond */

  /** \Sqrt wrapper.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline T sqrt(const T &v);
  /** \cond */
  template<> inline float sqrt(const float &v)
  {
    return ::sqrtf(v);
  }
  template<> inline double sqrt(const double &v)
  {
    return ::sqrt(v);
  }
  /** \endcond */

  /** \brief Tan wrapper.
   *
   * \param v Value.
   * \return Result.
   */
  template<typename T> inline T tan(const T &v);
  /** \cond */
  template<> inline float tan(const float &v)
  {
    return ::tanf(v);
  }
  template<> inline double tan(const double &v)
  {
    return ::tan(v);
  }
  /** \endcond */
}

#endif
