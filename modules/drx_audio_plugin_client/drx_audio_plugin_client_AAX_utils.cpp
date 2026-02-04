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

#if DrxPlugin_Build_AAX

#include <AAX_Version.h>

static_assert (AAX_SDK_CURRENT_REVISION >= AAX_SDK_2p4p0_REVISION, "DRX requires AAX SDK version 2.4.0 or higher");

#if DRX_INTEL || (DRX_MAC && DRX_ARM)

#include <drx_core/system/drx_CompilerWarnings.h>

// Utilities
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")
#include <Libs/AAXLibrary/source/AAX_CAutoreleasePool.Win.cpp>
DRX_END_IGNORE_WARNINGS_GCC_LIKE

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations",
                                     "-Wextra-semi",
                                     "-Wfloat-equal",
                                     "-Winconsistent-missing-destructor-override",
                                     "-Wshift-sign-overflow",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wfour-t8-constants",
                                     "-Wdeprecated-copy-with-user-provided-dtor",
                                     "-Wdeprecated",
                                     "-Wlanguage-extension-token",
                                     "-Wmicrosoft-enum-value",
                                     "-Wmisleading-indentation",
                                     "-Wregister")
DRX_BEGIN_IGNORE_WARNINGS_MSVC (6001 6053 4996 5033 4068 4996 5272)

#include <Libs/AAXLibrary/source/AAX_CChunkDataParser.cpp>
#include <Libs/AAXLibrary/source/AAX_CHostServices.cpp>

#if defined (_WIN32) && ! defined (WIN32)
 #define WIN32
#endif
#include <Libs/AAXLibrary/source/AAX_CMutex.cpp>

#include <Libs/AAXLibrary/source/AAX_CommonConversions.cpp>
#include <Libs/AAXLibrary/source/AAX_CPacketDispatcher.cpp>
#include <Libs/AAXLibrary/source/AAX_CString.cpp>

// Versioned Interfaces
#include <Interfaces/ACF/CACFClassFactory.cpp>
#include <Libs/AAXLibrary/source/AAX_CACFUnknown.cpp>

#include <Libs/AAXLibrary/source/AAX_CUIDs.cpp>
#include <Libs/AAXLibrary/source/AAX_IEffectDirectData.cpp>
#include <Libs/AAXLibrary/source/AAX_IEffectGUI.cpp>
#include <Libs/AAXLibrary/source/AAX_IEffectParameters.cpp>
#include <Libs/AAXLibrary/source/AAX_IHostProcessor.cpp>
#include <Libs/AAXLibrary/source/AAX_Properties.cpp>
#include <Libs/AAXLibrary/source/AAX_VAutomationDelegate.cpp>
#include <Libs/AAXLibrary/source/AAX_VCollection.cpp>
#include <Libs/AAXLibrary/source/AAX_VComponentDescriptor.cpp>
#include <Libs/AAXLibrary/source/AAX_VController.cpp>
#include <Libs/AAXLibrary/source/AAX_VDescriptionHost.cpp>
#include <Libs/AAXLibrary/source/AAX_VEffectDescriptor.cpp>
#include <Libs/AAXLibrary/source/AAX_VFeatureInfo.cpp>
#include <Libs/AAXLibrary/source/AAX_VHostProcessorDelegate.cpp>
#include <Libs/AAXLibrary/source/AAX_VHostServices.cpp>
#include <Libs/AAXLibrary/source/AAX_VPageTable.cpp>
#include <Libs/AAXLibrary/source/AAX_VPrivateDataAccess.cpp>
#include <Libs/AAXLibrary/source/AAX_VPropertyMap.cpp>
#include <Libs/AAXLibrary/source/AAX_VTransport.cpp>
#include <Libs/AAXLibrary/source/AAX_VViewContainer.cpp>
#include <Libs/AAXLibrary/source/AAX_CEffectDirectData.cpp>
#include <Libs/AAXLibrary/source/AAX_CEffectGUI.cpp>

#include <Libs/AAXLibrary/source/AAX_CEffectParameters.cpp>
#include <Libs/AAXLibrary/source/AAX_CHostProcessor.cpp>
#include <Libs/AAXLibrary/source/AAX_CParameter.cpp>
#include <Libs/AAXLibrary/source/AAX_CParameterManager.cpp>
#include <Libs/AAXLibrary/source/AAX_Init.cpp>
#include <Libs/AAXLibrary/source/AAX_SliderConversions.cpp>

DRX_END_IGNORE_WARNINGS_MSVC
DRX_END_IGNORE_WARNINGS_GCC_LIKE

#else
 #error "This version of the AAX SDK does not support the current platform."
#endif
#endif
