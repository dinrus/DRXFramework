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

#include <drx_core/system/drx_PlatformDefs.h>

#ifndef DRX_API
 #define DRX_API
#endif

#if (DrxPlugin_Enable_ARA || (DRX_PLUGINHOST_ARA && (DRX_PLUGINHOST_VST3 || DRX_PLUGINHOST_AU))) && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)

namespace drx
{

//==============================================================================
 #if (DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS) || DRX_LOG_ASSERTIONS
  #define ARA_ENABLE_INTERNAL_ASSERTS 1
 #else
  #define ARA_ENABLE_INTERNAL_ASSERTS 0
 #endif // (DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS) || DRX_LOG_ASSERTIONS

//==============================================================================
 #if ARA_ENABLE_INTERNAL_ASSERTS

DRX_API z0 DRX_CALLTYPE handleARAAssertion (tukk file, i32k line, tukk diagnosis) noexcept;

  #if !defined(ARA_HANDLE_ASSERT)
   #define ARA_HANDLE_ASSERT(file, line, diagnosis)    drx::handleARAAssertion (file, line, diagnosis)
  #endif

  #if DRX_LOG_ASSERTIONS
   #define ARA_ENABLE_DEBUG_OUTPUT 1
  #endif

 #endif

} // namespace drx

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments", "-Wmissing-prototypes")
 #include <ARA_Library/Debug/ARADebug.h>
DRX_END_IGNORE_WARNINGS_GCC_LIKE

#endif
