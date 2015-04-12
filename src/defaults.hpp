#ifndef DEFAULTS_HPP

#ifdef WIN32
#define _USE_MATH_DEFINES
#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <algorithm>
#include <iostream>
#include <limits.h>
#include <math.h>
#include <stdexcept>
#include <stdint.h>

#define BOOST_DISABLE_ASSERTS
#if defined(DEBUG)
#undef BOOST_DISABLE_ASSERTS
#endif
#if defined(NDEBUG)
#if !defined(BOOST_DISABLE_ASSERTS)
#error "both DEBUG and NDEBUG defined at same time"
#endif
#endif

#define BOOST_FILESYSTEM_VERSION 3

#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/throw_exception.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#endif
