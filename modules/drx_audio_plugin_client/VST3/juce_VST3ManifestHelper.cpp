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

// This suppresses a warning in drx_TargetPlatform.h
#ifndef DRX_GLOBAL_MODULE_SETTINGS_INCLUDED
 #define DRX_GLOBAL_MODULE_SETTINGS_INCLUDED 1
#endif

#include <drx_core/system/drx_CompilerWarnings.h>
#include <drx_core/system/drx_CompilerSupport.h>

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wc++98-compat-extra-semi",
                                     "-Wdeprecated-declarations",
                                     "-Wexpansion-to-defined",
                                     "-Wfloat-equal",
                                     "-Wformat",
                                     "-Wmissing-prototypes",
                                     "-Wpragma-pack",
                                     "-Wredundant-decls",
                                     "-Wshadow",
                                     "-Wshadow-field",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant")

DRX_BEGIN_IGNORE_WARNINGS_MSVC (6387 6031)

// As of at least 3.7.12 there is a bug in fplatform.h that leads to SMTG_CPP20
// having the wrong value when the /Zc:__cplusplus is not enabled. This work
// around prevents needing to provide that flag

#include <drx_audio_processors/format_types/VST3_SDK/pluginterfaces/base/fplatform.h>

#ifdef SMTG_CPP20
 #undef SMTG_CPP20
 #define SMTG_CPP20 DRX_CXX20_IS_AVAILABLE
#endif

#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
 #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#ifndef NOMINMAX
 #define NOMINMAX 1
#endif

#if DRX_MAC
 #include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module_mac.mm>
#elif DRX_WINDOWS
 #include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module_win32.cpp>
#elif DRX_LINUX
 #include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module_linux.cpp>
#endif

#include <drx_audio_processors/format_types/VST3_SDK/pluginterfaces/base/coreiids.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/pluginterfaces/base/funknown.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/samples/vst-utilities/moduleinfotool/source/main.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/common/commonstringconvert.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/common/memorystream.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/common/readfile.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/moduleinfo/moduleinfocreator.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/utility/stringconvert.cpp>
#include <drx_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/vstinitiids.cpp>

DRX_END_IGNORE_WARNINGS_MSVC
DRX_END_IGNORE_WARNINGS_GCC_LIKE
