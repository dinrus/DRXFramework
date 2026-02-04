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

  ID:                 drx_audio_utils
  vendor:             drx
  version:            8.0.7
  name:               DRX extra audio utility classes
  description:        Classes for audio-related GUI and miscellaneous tasks.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_audio_processors, drx_audio_formats, drx_audio_devices
  OSXFrameworks:      CoreAudioKit DiscRecording
  iOSFrameworks:      CoreAudioKit

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_AUDIO_UTILS_H_INCLUDED

#include <drx_gui_basics/drx_gui_basics.h>
#include <drx_audio_devices/drx_audio_devices.h>
#include <drx_audio_formats/drx_audio_formats.h>
#include <drx_audio_processors/drx_audio_processors.h>

//==============================================================================
/** Config: DRX_USE_CDREADER
    Enables the AudioCDReader class (on supported platforms).
*/
#ifndef DRX_USE_CDREADER
#define DRX_USE_CDREADER 0
#endif

/** Config: DRX_USE_CDBURNER
    Enables the AudioCDBurner class (on supported platforms).
*/
#ifndef DRX_USE_CDBURNER
#define DRX_USE_CDBURNER 0
#endif

//==============================================================================
#include "gui/drx_AudioDeviceSelectorComponent.h"
#include "gui/drx_AudioThumbnailBase.h"
#include "gui/drx_AudioThumbnail.h"
#include "gui/drx_AudioThumbnailCache.h"
#include "gui/drx_AudioVisualiserComponent.h"
#include "gui/drx_KeyboardComponentBase.h"
#include "gui/drx_MidiKeyboardComponent.h"
#include "gui/drx_MPEKeyboardComponent.h"
#include "gui/drx_AudioAppComponent.h"
#include "gui/drx_BluetoothMidiDevicePairingDialogue.h"
#include "players/drx_SoundPlayer.h"
#include "players/drx_AudioProcessorPlayer.h"
#include "audio_cd/drx_AudioCDBurner.h"
#include "audio_cd/drx_AudioCDReader.h"
