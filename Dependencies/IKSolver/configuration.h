#ifndef OPENTISSUE_CONFIGURATIOM_H
#define OPENTISSUE_CONFIGURATIOM_H
//
// OpenTissue Template Library
// - A generic toolbox for physics-based modeling and simulation.
// Copyright (C) 2008 Department of Computer Science, University of Copenhagen.
//
// OTTL is licensed under zlib: http://opensource.org/licenses/zlib-license.php
//
#if (_MSC_VER >= 1200)
# pragma once
# pragma warning(default: 56 61 62 191 263 264 265 287 289 296 347 529 686)
# pragma warning(disable: 503)
#endif

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  define _USE_MATH_DEFINES
#  define NOMINMAX
#  include <windows.h>
#  undef WIN32_LEAN_AND_MEAN
#  undef NOMINMAX
#endif


/**
 * OpenTissue Version
 */
#define OPENTISSUE_VERSION        0.994
#define OPENTISSUE_VERSION_MAJOR  0
#define OPENTISSUE_VERSION_MINOR  994

#include <string>

/**
 * OpenTissue Path.
 * This is the path where OpenTissue was copied onto ones
 * system. It can be used to locate shader programs or data resources.
 */
std::string const opentissue_path = "C:/Users/Administrator/Desktop/sandbox/";

/**
 * OpenTissue Version String.
 * This string value can be used by end users for compatibility testing.
 */
std::string const opentissue_version = "0.994";

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401

//OPENTISSUE_CONFIGURATIOM_H
#endif
