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

#ifdef DRX_AUDIO_DEVICES_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1

#ifndef DRX_USE_WINRT_MIDI
 #define DRX_USE_WINRT_MIDI 0
#endif

#if DRX_USE_WINRT_MIDI
 #define DRX_EVENTS_INCLUDE_WINRT_WRAPPER 1
#endif

#include "drx_audio_devices.h"

#include "audio_io/drx_SampleRateHelpers.cpp"
#include "midi_io/drx_MidiDeviceListConnectionBroadcaster.cpp"

//==============================================================================
#if DRX_MAC || DRX_IOS
 #include <drx_audio_basics/native/drx_CoreAudioTimeConversions_mac.h>
 #include <drx_audio_basics/native/drx_AudioWorkgroup_mac.h>
 #include <drx_audio_basics/midi/drx_MidiDataConcatenator.h>
 #include <drx_audio_basics/midi/ump/drx_UMP.h>
 #include "midi_io/ump/drx_UMPBytestreamInputHandler.h"
 #include "midi_io/ump/drx_UMPU32InputHandler.h"
#endif

#if DRX_MAC
 #define Point CarbonDummyPointName
 #define Component CarbonDummyCompName
 #import <CoreAudio/AudioHardware.h>
 #import <CoreMIDI/MIDIServices.h>
 #import <AudioToolbox/AudioServices.h>
 #undef Point
 #undef Component

 #include "native/drx_CoreAudio_mac.cpp"
 #include "native/drx_CoreMidi_mac.mm"

#elif DRX_IOS
 #import <AudioToolbox/AudioToolbox.h>
 #import <AVFoundation/AVFoundation.h>
 #import <CoreMIDI/MIDIServices.h>

 #if TARGET_OS_SIMULATOR
  #import <CoreMIDI/MIDINetworkSession.h>
 #endif

 #if DRX_MODULE_AVAILABLE_drx_graphics
  #include <drx_graphics/native/drx_CoreGraphicsHelpers_mac.h>
 #endif

 #include "native/drx_Audio_ios.cpp"
 #include "native/drx_CoreMidi_mac.mm"

//==============================================================================
#elif DRX_WINDOWS
 #if DRX_WASAPI
  #include <mmreg.h>
  #include "native/drx_WASAPI_windows.cpp"
 #endif

 #if DRX_DIRECTSOUND
  #include "native/drx_DirectSound_windows.cpp"
 #endif

 #if DRX_USE_WINRT_MIDI && (DRX_MSVC || DRX_CLANG)
  /* If you cannot find any of the header files below then you are probably
     attempting to use the Windows 10 Bluetooth Low Energy API. For this to work you
     need to install version 10.0.14393.0 of the Windows Standalone SDK and you may
     need to add the path to the WinRT headers to your build system. This path should
     have the form "C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\winrt".

     Also please note that Microsoft's Bluetooth MIDI stack has multiple issues, so
     this API is EXPERIMENTAL - use at your own risk!
  */
  #include <windows.devices.h>
  #include <windows.devices.midi.h>
  #include <windows.devices.enumeration.h>

  DRX_BEGIN_IGNORE_WARNINGS_MSVC (4265)
  #include <wrl/event.h>
  DRX_END_IGNORE_WARNINGS_MSVC

  DRX_BEGIN_IGNORE_WARNINGS_MSVC (4467)
  #include <robuffer.h>
  DRX_END_IGNORE_WARNINGS_MSVC
 #endif

 #include <drx_audio_basics/midi/drx_MidiDataConcatenator.h>
 #include "native/drx_Midi_windows.cpp"

 #if DRX_ASIO
  /* This is very frustrating - we only need to use a handful of definitions from
     a couple of the header files in Steinberg's ASIO SDK, and it'd be easy to copy
     about 30 lines of code into this cpp file to create a fully stand-alone ASIO
     implementation...

     ..unfortunately that would break Steinberg's license agreement for use of
     their SDK, so I'm not allowed to do this.

     This means that anyone who wants to use DRX's ASIO abilities will have to:

     1) Agree to Steinberg's licensing terms and download the ASIO SDK
         (see http://www.steinberg.net/en/company/developers.html).

     2) Enable this code with a global definition #define DRX_ASIO 1.

     3) Make sure that your header search path contains the iasiodrv.h file that
        comes with the SDK. (Only about a handful of the SDK header files are actually
        needed - so to simplify things, you could just copy these into your DRX directory).
  */
  #include <iasiodrv.h>
  #include "native/drx_ASIO_windows.cpp"
 #endif

//==============================================================================
#elif DRX_LINUX || DRX_BSD
 #if DRX_ALSA
  /* Got an include error here? If so, you've either not got ALSA installed, or you've
     not got your paths set up correctly to find its header files.

     The package you need to install to get ASLA support is "libasound2-dev".

     If you don't have the ALSA library and don't want to build DRX with audio support,
     just set the DRX_ALSA flag to 0.
  */
  DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-length-array")
  #include <alsa/asoundlib.h>
  DRX_END_IGNORE_WARNINGS_GCC_LIKE
  #include "native/drx_ALSA_linux.cpp"
 #endif

 #if (DRX_LINUX && DRX_BELA)
  /* Got an include error here? If so, you've either not got the bela headers
     installed, or you've not got your paths set up correctly to find its header
     files.
  */
  #include <Bela.h>
  #include <Midi.h>
  #include <drx_audio_basics/midi/drx_MidiDataConcatenator.h>
  #include "native/drx_Bela_linux.cpp"
 #endif

 #undef SIZEOF

 #if ! DRX_BELA
  #include <drx_audio_basics/midi/drx_MidiDataConcatenator.h>
  #include "native/drx_Midi_linux.cpp"
 #endif

//==============================================================================
#elif DRX_ANDROID

namespace drx
{
    using RealtimeThreadFactory = pthread_t (*) (uk (*) (uk), uk);
    RealtimeThreadFactory getAndroidRealtimeThreadFactory();
} // namespace drx

#include "native/drx_Audio_android.cpp"

 #include <drx_audio_basics/midi/drx_MidiDataConcatenator.h>
 #include "native/drx_Midi_android.cpp"

 #if DRX_USE_ANDROID_OPENSLES || DRX_USE_ANDROID_OBOE
  #include "native/drx_HighPerformanceAudioHelpers_android.h"

  #if DRX_USE_ANDROID_OPENSLES
   #include <SLES/OpenSLES.h>
   #include <SLES/OpenSLES_Android.h>
   #include <SLES/OpenSLES_AndroidConfiguration.h>
   #include "native/drx_OpenSL_android.cpp"
  #endif

  #if DRX_USE_ANDROID_OBOE
   #if DRX_USE_ANDROID_OPENSLES
    #error "Oboe cannot be enabled at the same time as openSL! Please disable DRX_USE_ANDROID_OPENSLES"
   #endif

   DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter",
                                        "-Wzero-as-null-pointer-constant",
                                        "-Winconsistent-missing-destructor-override",
                                        "-Wshadow-field-in-constructor",
                                        "-Wshadow-field",
                                        "-Wsign-conversion",
                                        "-Wswitch-enum")
   #include <oboe/Oboe.h>
   DRX_END_IGNORE_WARNINGS_GCC_LIKE

   #include "native/drx_Oboe_android.cpp"
  #endif
 #else
// No audio library, so no way to create realtime threads.
  namespace drx
  {
      RealtimeThreadFactory getAndroidRealtimeThreadFactory() { return nullptr; }
  }
 #endif

#endif

#if (DRX_LINUX || DRX_BSD || DRX_MAC || DRX_WINDOWS) && DRX_JACK
 /* Got an include error here? If so, you've either not got jack-audio-connection-kit
    installed, or you've not got your paths set up correctly to find its header files.

    Linux: The package you need to install to get JACK support is libjack-dev.

    macOS: The package you need to install to get JACK support is jack, which you can
    install using Homebrew.

    Windows: The package you need to install to get JACK support is available from the
    JACK Audio website. Download and run the installer for Windows.

    If you don't have the jack-audio-connection-kit library and don't want to build
    DRX with low latency audio support, just set the DRX_JACK flag to 0.
 */
 #include <jack/jack.h>
 #include "native/drx_JackAudio.cpp"
#endif

#include "midi_io/drx_MidiDevices.cpp"

#if ! DRX_SYSTEMAUDIOVOL_IMPLEMENTED
namespace drx
{
    // None of these methods are available. (On Windows you might need to enable WASAPI for this)
    f32 DRX_CALLTYPE SystemAudioVolume::getGain()         { jassertfalse; return 0.0f; }
    b8  DRX_CALLTYPE SystemAudioVolume::setGain (f32)   { jassertfalse; return false; }
    b8  DRX_CALLTYPE SystemAudioVolume::isMuted()         { jassertfalse; return false; }
    b8  DRX_CALLTYPE SystemAudioVolume::setMuted (b8)   { jassertfalse; return false; }
}
#endif

#include "audio_io/drx_AudioDeviceManager.cpp"
#include "audio_io/drx_AudioIODevice.cpp"
#include "audio_io/drx_AudioIODeviceType.cpp"
#include "midi_io/drx_MidiMessageCollector.cpp"
#include "sources/drx_AudioSourcePlayer.cpp"
#include "sources/drx_AudioTransportSource.cpp"
