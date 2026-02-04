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

  ID:                 drx_audio_processors
  vendor:             drx
  version:            8.0.7
  name:               DRX audio processor classes
  description:        Classes for loading and playing VST, AU, LADSPA, or internally-generated audio processors.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_gui_extra, drx_audio_basics
  OSXFrameworks:      CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:      AudioToolbox

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_AUDIO_PROCESSORS_H_INCLUDED

#include <drx_gui_basics/drx_gui_basics.h>
#include <drx_gui_extra/drx_gui_extra.h>
#include <drx_audio_basics/drx_audio_basics.h>

//==============================================================================
/** Config: DRX_PLUGINHOST_VST
    Enables the VST audio plugin hosting classes. You will need to have the VST2 SDK files in your header search paths. You can obtain the VST2 SDK files from on older version of the VST3 SDK.

    @see VSTPluginFormat, VST3PluginFormat, AudioPluginFormat, AudioPluginFormatManager, DRX_PLUGINHOST_AU, DRX_PLUGINHOST_VST3, DRX_PLUGINHOST_LADSPA
*/
#ifndef DRX_PLUGINHOST_VST
 #define DRX_PLUGINHOST_VST 0
#endif

/** Config: DRX_PLUGINHOST_VST3
    Enables the VST3 audio plugin hosting classes.

    @see VSTPluginFormat, VST3PluginFormat, AudioPluginFormat, AudioPluginFormatManager, DRX_PLUGINHOST_VST, DRX_PLUGINHOST_AU, DRX_PLUGINHOST_LADSPA
*/
#ifndef DRX_PLUGINHOST_VST3
 #define DRX_PLUGINHOST_VST3 0
#endif

/** Config: DRX_PLUGINHOST_AU
    Enables the AudioUnit plugin hosting classes. This is Mac-only, of course.

    @see AudioUnitPluginFormat, AudioPluginFormat, AudioPluginFormatManager, DRX_PLUGINHOST_VST, DRX_PLUGINHOST_VST3, DRX_PLUGINHOST_LADSPA
*/
#ifndef DRX_PLUGINHOST_AU
 #define DRX_PLUGINHOST_AU 0
#endif

/** Config: DRX_PLUGINHOST_LADSPA
    Enables the LADSPA plugin hosting classes. This is Linux-only, of course.

    @see LADSPAPluginFormat, AudioPluginFormat, AudioPluginFormatManager, DRX_PLUGINHOST_VST, DRX_PLUGINHOST_VST3, DRX_PLUGINHOST_AU
 */
#ifndef DRX_PLUGINHOST_LADSPA
 #define DRX_PLUGINHOST_LADSPA 0
#endif

/** Config: DRX_PLUGINHOST_LV2
    Enables the LV2 plugin hosting classes.
 */
#ifndef DRX_PLUGINHOST_LV2
 #define DRX_PLUGINHOST_LV2 0
#endif

/** Config: DRX_PLUGINHOST_ARA
    Enables the ARA plugin extension hosting classes. You will need to download the ARA SDK and specify the
    path to it either in the Projucer, using drx_set_ara_sdk_path() in your CMake project file.

    The directory can be obtained by recursively cloning https://github.com/Celemony/ARA_SDK and checking out
    the tag releases/2.1.0.
*/
#ifndef DRX_PLUGINHOST_ARA
 #define DRX_PLUGINHOST_ARA 0
#endif

/** Config: DRX_CUSTOM_VST3_SDK
    If enabled, the embedded VST3 SDK in DRX will not be added to the project and instead you should
    add the path to your custom VST3 SDK to the project's header search paths. Most users shouldn't
    need to enable this and should just use the version of the SDK included with DRX.
*/
#ifndef DRX_CUSTOM_VST3_SDK
 #define DRX_CUSTOM_VST3_SDK 0
#endif

#if ! (DRX_PLUGINHOST_AU || DRX_PLUGINHOST_VST || DRX_PLUGINHOST_VST3 || DRX_PLUGINHOST_LADSPA)
// #error "You need to set either the DRX_PLUGINHOST_AU and/or DRX_PLUGINHOST_VST and/or DRX_PLUGINHOST_VST3 and/or DRX_PLUGINHOST_LADSPA flags if you're using this module!"
#endif

#ifndef DRX_SUPPORT_LEGACY_AUDIOPROCESSOR
 #define DRX_SUPPORT_LEGACY_AUDIOPROCESSOR 1
#endif

//==============================================================================
#include "utilities/drx_AAXClientExtensions.h"
#include "utilities/drx_VST2ClientExtensions.h"
#include "utilities/drx_VST3ClientExtensions.h"
#include "format_types/drx_ARACommon.h"
#include "utilities/drx_ExtensionsVisitor.h"
#include "processors/drx_AudioProcessorParameter.h"
#include "processors/drx_HostedAudioProcessorParameter.h"
#include "processors/drx_AudioProcessorEditorHostContext.h"
#include "processors/drx_AudioProcessorEditor.h"
#include "processors/drx_AudioProcessorListener.h"
#include "processors/drx_AudioProcessorParameterGroup.h"
#include "processors/drx_AudioProcessor.h"
#include "processors/drx_PluginDescription.h"
#include "processors/drx_AudioPluginInstance.h"
#include "processors/drx_AudioProcessorGraph.h"
#include "processors/drx_GenericAudioProcessorEditor.h"
#include "format/drx_AudioPluginFormat.h"
#include "format/drx_AudioPluginFormatManager.h"
#include "scanning/drx_KnownPluginList.h"
#include "format_types/drx_AudioUnitPluginFormat.h"
#include "format_types/drx_LADSPAPluginFormat.h"
#include "format_types/drx_LV2PluginFormat.h"
#include "format_types/drx_VST3PluginFormat.h"
#include "format_types/drx_VSTMidiEventList.h"
#include "format_types/drx_VSTPluginFormat.h"
#include "format_types/drx_ARAHosting.h"
#include "scanning/drx_PluginDirectoryScanner.h"
#include "scanning/drx_PluginListComponent.h"
#include "utilities/drx_AudioProcessorParameterWithID.h"
#include "utilities/drx_RangedAudioParameter.h"
#include "utilities/drx_AudioParameterFloat.h"
#include "utilities/drx_AudioParameterInt.h"
#include "utilities/drx_AudioParameterBool.h"
#include "utilities/drx_AudioParameterChoice.h"
#include "utilities/drx_ParameterAttachments.h"
#include "utilities/drx_AudioProcessorValueTreeState.h"
#include "utilities/drx_PluginHostType.h"
#include "utilities/ARA/drx_ARADebug.h"
#include "utilities/ARA/drx_ARA_utils.h"

//==============================================================================
// These declarations are here to avoid missing-prototype warnings in user code.

// If you're implementing a plugin, you should supply a body for
// this function in your own code.
drx::AudioProcessor* DRX_CALLTYPE createPluginFilter();

// If you are implementing an ARA enabled plugin, you need to
// implement this function somewhere in the codebase by returning
// SubclassOfARADocumentControllerSpecialisation::createARAFactory<SubclassOfARADocumentControllerSpecialisation>();
#if DrxPlugin_Enable_ARA
 const ARA::ARAFactory* DRX_CALLTYPE createARAFactory();
#endif
