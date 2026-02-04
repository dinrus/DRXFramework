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

#if DrxPlugin_Build_AU

#include <drx_core/system/drx_CompilerWarnings.h>

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wambiguous-reversed-operator",
                                     "-Wc99-extensions",
                                     "-Wcast-align",
                                     "-Wcomment",
                                     "-Wconversion",
                                     "-Wdeprecated-anon-enum-enum-conversion",
                                     "-Wextra-semi",
                                     "-Wextra-tokens",
                                     "-Wfloat-equal",
                                     "-Wformat-pedantic",
                                     "-Wfour-t8-constants",
                                     "-Wgnu-zero-variadic-macro-arguments",
                                     "-Wignored-qualifiers",
                                     "-Wimplicit-fallthrough",
                                     "-Wmissing-prototypes",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wparentheses",
                                     "-Wshadow-all",
                                     "-Wswitch-enum",
                                     "-Wunknown-attributes",
                                     "-Wunused",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant")

// From MacOS 10.13 and iOS 11 Apple has (sensibly!) stopped defining a whole
// set of functions with rather generic names. However, we still need a couple
// of them to compile the files below.
#ifndef verify
 #define verify(assertion) __Verify(assertion)
#endif
#ifndef verify_noerr
 #define verify_noerr(errorCode)  __Verify_noErr(errorCode)
#endif

#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUBase.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUBuffer.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUBufferAllocator.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUEffectBase.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUInputElement.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUMIDIBase.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUMIDIEffectBase.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUOutputElement.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUPlugInDispatch.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/AUScopeElement.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/ComponentBase.cpp>
#include <drx_audio_plugin_client/AU/AudioUnitSDK/MusicDeviceBase.cpp>

#undef verify
#undef verify_noerr

DRX_END_IGNORE_WARNINGS_GCC_LIKE

#endif
