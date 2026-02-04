/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include <drx_core/system/drx_CompilerWarnings.h>

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4100 4127 4189 4244 4245 4265 4267 4309 4310 4312 4456 4457 4458 4459 4701 4702 4706 6001 6011 6239 6244 6246 6262 6297 6313 6319 6326 6336 6385 6386 28251)

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wformat-pedantic",
                                     "-Wextra-semi",
                                     "-Wc++98-compat-extra-semi",
                                     "-Wshadow-field",
                                     "-Wfloat-equal",
                                     "-Wformat",
                                     "-Wpedantic",
                                     "-Wmicrosoft-exception-spec",
                                     "-Wmicrosoft-cast",
                                     "-Wconditional-uninitialized",
                                     "-Wexpansion-to-defined",
                                     "-Wunsafe-loop-optimizations",
                                     "-Wformat-overflow",
                                     "-Woverflow",
                                     "-Wnontrivial-memcall",
                                     "-Wimplicit-fallthrough")

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

#define HAVE_ATEXIT 1

#if DRX_LINUX || DRX_BSD
 #ifndef DRX_USE_FREETYPE
  #define DRX_USE_FREETYPE 1
 #endif
#endif

#if DRX_USE_FREETYPE
 #define HAVE_FREETYPE 1
#endif

#if DRX_WINDOWS
 #define HAVE_DIRECTWRITE 1
 #define _CRT_SECURE_NO_WARNINGS 1
#elif DRX_MAC || DRX_IOS
 #define HAVE_CORETEXT 1
#endif

// HB enables some warnings in its header, but we would rather build cleanly
#define HB_NO_PRAGMA_GCC_DIAGNOSTIC_WARNING
#define HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR

// This is a hack, because harfbuzz headers define hb_has_builtin to
// expand to a macro that includes a 'defines' expression, which triggers
// -Wexpansion-to-defined on gcc 7. There's no way to turn that warning off
// locally, so we sidestep it.
#if ! defined(__has_builtin) && defined(__GNUC__) && __GNUC__ >= 5
 #define __has_builtin(x) 1
#endif

#include <utility>
#include <drx_graphics/fonts/harfbuzz/hb.hh>
#include <drx_graphics/fonts/harfbuzz/harfbuzz.cc>

#undef HAVE_DIRECTWRITE
#undef HAVE_FREETYPE
#undef HAVE_CORETEXT

DRX_END_IGNORE_DEPRECATION_WARNINGS
DRX_END_IGNORE_WARNINGS_GCC_LIKE
DRX_END_IGNORE_WARNINGS_MSVC
