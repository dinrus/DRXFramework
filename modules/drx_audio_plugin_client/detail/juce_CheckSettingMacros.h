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

// The following checks should cause a compile error if you've forgotten to
// define all your plugin settings properly..

#if ! (DrxPlugin_Build_VST || DrxPlugin_Build_VST3 \
        || DrxPlugin_Build_AU  || DrxPlugin_Build_AUv3 \
        || DrxPlugin_Build_AAX || DrxPlugin_Build_Standalone \
        || DrxPlugin_Build_LV2 || DrxPlugin_Build_Unity)
 #error "You need to enable at least one plugin format!"
#endif

#ifdef DRX_CHECKSETTINGMACROS_H
 #error "This header should never be included twice! Otherwise something is wrong."
#endif
#define DRX_CHECKSETTINGMACROS_H

#ifndef DrxPlugin_IsSynth
 #error "You need to define the DrxPlugin_IsSynth value!"
#endif

#ifndef DrxPlugin_ManufacturerCode
 #error "You need to define the DrxPlugin_ManufacturerCode value!"
#endif

#ifndef DrxPlugin_PluginCode
 #error "You need to define the DrxPlugin_PluginCode value!"
#endif

#ifndef DrxPlugin_ProducesMidiOutput
 #error "You need to define the DrxPlugin_ProducesMidiOutput value!"
#endif

#ifndef DrxPlugin_WantsMidiInput
 #error "You need to define the DrxPlugin_WantsMidiInput value!"
#endif

#ifdef DrxPlugin_Latency
 #error "DrxPlugin_Latency is now deprecated - instead, call the AudioProcessor::setLatencySamples() method if your plugin has a non-zero delay"
#endif

#ifndef DrxPlugin_EditorRequiresKeyboardFocus
 #error "You need to define the DrxPlugin_EditorRequiresKeyboardFocus value!"
#endif

//==============================================================================
#if DrxPlugin_Build_AAX && ! defined (DrxPlugin_AAXIdentifier)
 #error "You need to define the DrxPlugin_AAXIdentifier value!"
#endif

#if defined (__ppc__)
 #undef DrxPlugin_Build_AAX
 #define DrxPlugin_Build_AAX 0
#endif
