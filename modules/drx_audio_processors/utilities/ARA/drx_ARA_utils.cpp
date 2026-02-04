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

#if (DrxPlugin_Enable_ARA || (DRX_PLUGINHOST_ARA && (DRX_PLUGINHOST_VST3 || DRX_PLUGINHOST_AU))) && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)
namespace drx
{
 #if ARA_ENABLE_INTERNAL_ASSERTS
DRX_API z0 DRX_CALLTYPE handleARAAssertion (tukk file, i32k line, tukk diagnosis) noexcept
{
  #if (DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS)
    DBG (diagnosis);
  #endif

    logAssertion (file, line);

  #if (DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS)
    if (drx_isRunningUnderDebugger())
        DRX_BREAK_IN_DEBUGGER;
    DRX_ANALYZER_NORETURN
  #endif
}
 #endif
}
#endif

#if DrxPlugin_Enable_ARA
#include "drx_ARADocumentControllerCommon.cpp"
#include "drx_ARADocumentController.cpp"
#include "drx_ARAModelObjects.cpp"
#include "drx_ARAPlugInInstanceRoles.cpp"
#include "drx_AudioProcessor_ARAExtensions.cpp"

ARA_SETUP_DEBUG_MESSAGE_PREFIX (DrxPlugin_Name);
#endif
