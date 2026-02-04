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

#include <drx_core/system/drx_TargetPlatform.h>
#include <drx_audio_plugin_client/detail/drx_CheckSettingMacros.h>

#if DrxPlugin_Enable_ARA

#include <drx_audio_plugin_client/detail/drx_IncludeSystemHeaders.h>
#include <drx_audio_plugin_client/detail/drx_IncludeModuleHeaders.h>

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter",
                                     "-Wgnu-zero-variadic-macro-arguments",
                                     "-Wmissing-prototypes",
                                     "-Wfloat-equal",
                                     "-Wc++20-extensions")
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4100)

#include <ARA_Library/PlugIn/ARAPlug.cpp>
#include <ARA_Library/Dispatch/ARAPlugInDispatch.cpp>
#include <ARA_Library/Utilities/ARAPitchInterpretation.cpp>
#include <ARA_Library/Utilities/ARAChannelArrangement.cpp>

DRX_END_IGNORE_WARNINGS_MSVC
DRX_END_IGNORE_WARNINGS_GCC_LIKE

#endif
