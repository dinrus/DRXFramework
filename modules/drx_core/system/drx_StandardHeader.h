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

#pragma once

//==============================================================================
/** Current DRX version number.

    See also SystemStats::getDRXVersion() for a string version.
*/
#define DRX_MAJOR_VERSION      8
#define DRX_MINOR_VERSION      0
#define DRX_BUILDNUMBER        7

/** Current DRX version number.

    Bits 16 to 32 = major version.
    Bits 8 to 16 = minor version.
    Bits 0 to 8 = point release.

    See also SystemStats::getDRXVersion() for a string version.
*/
#define DRX_VERSION   ((DRX_MAJOR_VERSION << 16) + (DRX_MINOR_VERSION << 8) + DRX_BUILDNUMBER)

#if ! DOXYGEN
#define DRX_VERSION_ID \
    [[maybe_unused]] volatile auto juceVersionId = "drx_version_" DRX_STRINGIFY(DRX_MAJOR_VERSION) "_" DRX_STRINGIFY(DRX_MINOR_VERSION) "_" DRX_STRINGIFY(DRX_BUILDNUMBER);
#endif

//==============================================================================
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string_view>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

//==============================================================================
#include <drx_core/system/drx_CompilerSupport.h>
#include <drx_core/system/drx_CompilerWarnings.h>
#include <drx_core/system/drx_PlatformDefs.h>

//==============================================================================
// Now we'll include some common OS headers..
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4514 4245 4100)

#if DRX_MSVC
 #include <intrin.h>
#endif


#if DRX_MAC || DRX_IOS
 #include <libkern/OSAtomic.h>
 #include <libkern/OSByteOrder.h>
 #include <xlocale.h>
 #include <signal.h>
#endif

#if DRX_LINUX || DRX_BSD
 #include <cstring>
 #include <signal.h>

 #if __INTEL_COMPILER
  #if __ia64__
   #include <ia64intrin.h>
  #else
   #include <ia32intrin.h>
  #endif
 #endif
#endif

#if DRX_MSVC && DRX_DEBUG
 #include <crtdbg.h>
#endif

DRX_END_IGNORE_WARNINGS_MSVC

#if DRX_ANDROID
 #include <cstring>
 #include <byteswap.h>
#endif

// undef symbols that are sometimes set by misguided 3rd-party headers..
#undef TYPE_BOOL
#undef max
#undef min
#undef major
#undef minor
#undef KeyPress

//==============================================================================
// DLL building settings on Windows
#if DRX_MSVC
 #ifdef DRX_DLL_BUILD
  #define DRX_API __declspec (dllexport)
  #pragma warning (disable: 4251)
 #elif defined (DRX_DLL)
  #define DRX_API __declspec (dllimport)
  #pragma warning (disable: 4251)
 #endif
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 1125) // (virtual override warning)
 #endif
#elif defined (DRX_DLL) || defined (DRX_DLL_BUILD)
 #define DRX_API __attribute__ ((visibility ("default")))
#endif

//==============================================================================
#ifndef DRX_API
 #define DRX_API   /**< This macro is added to all DRX public class declarations. */
#endif

#if DRX_MSVC && DRX_DLL_BUILD
 #define DRX_PUBLIC_IN_DLL_BUILD(declaration)  public: declaration; private:
#else
 #define DRX_PUBLIC_IN_DLL_BUILD(declaration)  declaration;
#endif

/** This macro is added to all DRX public function declarations. */
#define DRX_PUBLIC_FUNCTION        DRX_API DRX_CALLTYPE

#ifndef DOXYGEN
 #define DRX_NAMESPACE drx  // This old macro is deprecated: you should just use the drx namespace directly.
#endif
