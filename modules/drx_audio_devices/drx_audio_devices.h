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

  ID:                 drx_audio_devices
  vendor:             drx
  version:            8.0.7
  name:               DRX audio and MIDI I/O device classes
  description:        Classes to play and record from audio and MIDI I/O devices
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_audio_basics, drx_events
  OSXFrameworks:      CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:      CoreAudio CoreMIDI AudioToolbox AVFoundation
  linuxPackages:      alsa

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_AUDIO_DEVICES_H_INCLUDED

#include <drx_events/drx_events.h>
#include <drx_audio_basics/drx_audio_basics.h>

#if DRX_MODULE_AVAILABLE_drx_graphics
#include <drx_graphics/drx_graphics.h>
#endif

//==============================================================================
/** Config: DRX_USE_WINRT_MIDI
    Enables the use of the Windows Runtime API for MIDI, allowing connections
    to Bluetooth Low Energy devices on Windows 10 version 1809 (October 2018
    Update) and later. If you enable this flag then older versions of Windows
    will automatically fall back to using the regular Win32 MIDI API.

    You will need version 10.0.14393.0 of the Windows Standalone SDK to compile
    and you may need to add the path to the WinRT headers. The path to the
    headers will be something similar to
    "C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\winrt".
*/
#ifndef DRX_USE_WINRT_MIDI
 #define DRX_USE_WINRT_MIDI 0
#endif

/** Config: DRX_ASIO
    Enables ASIO audio devices (MS Windows only).
    Turning this on means that you'll need to have the Steinberg ASIO SDK installed
    on your Windows build machine.

    See the comments in the ASIOAudioIODevice class's header file for more
    info about this.
*/
#ifndef DRX_ASIO
 #define DRX_ASIO 0
#endif

/** Config: DRX_WASAPI
    Enables WASAPI audio devices (Windows Vista and above).
*/
#ifndef DRX_WASAPI
 #define DRX_WASAPI 1
#endif

/** Config: DRX_DIRECTSOUND
    Enables DirectSound audio (MS Windows only).
*/
#ifndef DRX_DIRECTSOUND
 #define DRX_DIRECTSOUND 1
#endif

/** Config: DRX_ALSA
    Enables ALSA audio devices (Linux only).
*/
#ifndef DRX_ALSA
 #define DRX_ALSA 1
#endif

/** Config: DRX_JACK
    Enables JACK audio devices.
*/
#ifndef DRX_JACK
 #define DRX_JACK 0
#endif

/** Config: DRX_BELA
    Enables Bela audio devices on Bela boards.
*/
#ifndef DRX_BELA
 #define DRX_BELA 0
#endif

/** Config: DRX_USE_ANDROID_OBOE
    Enables Oboe devices (Android only).
*/
#ifndef DRX_USE_ANDROID_OBOE
 #define DRX_USE_ANDROID_OBOE 1
#endif

/** Config: DRX_USE_OBOE_STABILIZED_CALLBACK
    If DRX_USE_ANDROID_OBOE is enabled, enabling this will wrap output audio
    streams in the oboe::StabilizedCallback class. This class attempts to keep
    the CPU spinning to avoid it being scaled down on certain devices.
    (Android only).
*/
#ifndef DRX_USE_ANDROID_OBOE_STABILIZED_CALLBACK
 #define DRX_USE_ANDROID_OBOE_STABILIZED_CALLBACK 0
#endif

/** Config: DRX_USE_ANDROID_OPENSLES
    Enables OpenSLES devices (Android only).
*/
#ifndef DRX_USE_ANDROID_OPENSLES
 #if ! DRX_USE_ANDROID_OBOE
  #define DRX_USE_ANDROID_OPENSLES 1
 #else
  #define DRX_USE_ANDROID_OPENSLES 0
 #endif
#endif

/** Config: DRX_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS
    Turning this on gives your app exclusive access to the system's audio
    on platforms which support it (currently iOS only).
*/
#ifndef DRX_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS
 #define DRX_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS 0
#endif

//==============================================================================
#include "midi_io/drx_MidiDevices.h"
#include "midi_io/drx_MidiMessageCollector.h"

namespace drx
{
    /** Available modes for the WASAPI audio device.

        Pass one of these to the AudioIODeviceType::createAudioIODeviceType_WASAPI()
        method to create a WASAPI AudioIODeviceType object in this mode.
    */
    enum class WASAPIDeviceMode
    {
        shared,
        exclusive,
        sharedLowLatency
    };
}

#include "audio_io/drx_AudioIODevice.h"
#include "audio_io/drx_AudioIODeviceType.h"
#include "audio_io/drx_SystemAudioVolume.h"
#include "sources/drx_AudioSourcePlayer.h"
#include "sources/drx_AudioTransportSource.h"
#include "audio_io/drx_AudioDeviceManager.h"

#if DRX_IOS
 #include "native/drx_Audio_ios.h"
#endif
