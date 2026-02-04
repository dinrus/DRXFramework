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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 DRX Module Format.md file.


 BEGIN_DRX_MODULE_DECLARATION

  ID:                 drx_audio_plugin_client
  vendor:             drx
  version:            8.0.7
  name:               DRX audio plugin wrapper classes
  description:        Classes for building VST, VST3, AU, AUv3, LV2 and AAX plugins.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_audio_processors

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once

#include <drx_gui_basics/drx_gui_basics.h>
#include <drx_audio_basics/drx_audio_basics.h>
#include <drx_audio_processors/drx_audio_processors.h>

/** Config: DRX_VST3_CAN_REPLACE_VST2

    Enable this if you want your VST3 plug-in to load and save VST2 compatible
    state. This allows hosts to replace VST2 plug-ins with VST3 plug-ins. If
    you change this option then your VST3 plug-in will be incompatible with
    previous versions.
*/
#ifndef DRX_VST3_CAN_REPLACE_VST2
 #define DRX_VST3_CAN_REPLACE_VST2 1
#endif

/** Config: DRX_FORCE_USE_LEGACY_PARAM_IDS

    Enable this if you want to force DRX to use a continuous parameter
    index to identify a parameter in a DAW (this was the default in old
    versions of DRX). This is index is usually used by the DAW to save
    automation data and enabling this may mess up user's DAW projects.
*/
#ifndef DRX_FORCE_USE_LEGACY_PARAM_IDS
 #define DRX_FORCE_USE_LEGACY_PARAM_IDS 0
#endif

/** Config: DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE

    Enable this if you want to force DRX to use a legacy scheme for
    identifying plug-in parameters as either continuous or discrete.
    DAW projects with automation data written by an AudioUnit, VST3 or
    AAX plug-in built with DRX version 5.1.1 or earlier may load
    incorrectly when opened by an AudioUnit, VST3 or AAX plug-in built
    with DRX version 5.2.0 and later.
*/
#ifndef DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
 #define DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE 0
#endif

/** Config: DRX_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS

    Enable this if you want DRX to use parameter ids which are compatible
    with Studio One, as Studio One ignores any parameter ids which are negative.
    Enabling this option will make DRX generate only positive parameter ids.
    Note that if you have already released a plug-in prior to DRX 4.3.0 then
    enabling this will change your parameter ids, making your plug-in
    incompatible with old automation data.
*/
#ifndef DRX_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS
 #define DRX_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS 1
#endif

/** Config: DRX_AU_WRAPPERS_SAVE_PROGRAM_STATES

    Enable this if you want to receive get/setProgramStateInformation calls,
    instead of get/setStateInformation calls, from the AU and AUv3 plug-in
    wrappers. In DRX version 5.4.5 and earlier this was the default behaviour,
    so if you have modified the default implementations of get/setProgramStateInformation
    (where the default implementations simply call through to get/setStateInformation)
    then you may need to enable this configuration option to maintain backwards
    compatibility with previously saved state.
*/
#ifndef DRX_AU_WRAPPERS_SAVE_PROGRAM_STATES
 #define DRX_AU_WRAPPERS_SAVE_PROGRAM_STATES 0
#endif

/** Config: DRX_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE

    Enable this if you want your standalone plugin window to use kiosk mode.
    By default, kiosk mode is enabled on iOS and Android.
*/

#ifndef DRX_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
 #define DRX_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE (DRX_IOS || DRX_ANDROID)
#endif

#include "detail/drx_CreatePluginFilter.h"
