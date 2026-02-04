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

#ifdef DRX_AUDIO_UTILS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1

#include "drx_audio_utils.h"
#include <drx_gui_extra/drx_gui_extra.h>

#if DRX_MAC
 #import <DiscRecording/DiscRecording.h>
 #import <CoreAudioKit/CABTLEMIDIWindowController.h>
#elif DRX_IOS
 #import <CoreAudioKit/CoreAudioKit.h>
#elif DRX_WINDOWS
 #if DRX_USE_CDBURNER
  /* You'll need the Platform SDK for these headers - if you don't have it and don't
     need to use CD-burning, then you might just want to set the DRX_USE_CDBURNER flag
     to 0, to avoid these includes.
  */
  #include <imapi.h>
  #include <imapierror.h>
 #endif
#endif

#include "gui/drx_AudioDeviceSelectorComponent.cpp"
#include "gui/drx_AudioThumbnail.cpp"
#include "gui/drx_AudioThumbnailCache.cpp"
#include "gui/drx_AudioVisualiserComponent.cpp"
#include "gui/drx_KeyboardComponentBase.cpp"
#include "gui/drx_MidiKeyboardComponent.cpp"
#include "gui/drx_MPEKeyboardComponent.cpp"
#include "gui/drx_AudioAppComponent.cpp"
#include "players/drx_SoundPlayer.cpp"
#include "players/drx_AudioProcessorPlayer.cpp"
#include "audio_cd/drx_AudioCDReader.cpp"

#if DRX_MAC
 #include "native/drx_BluetoothMidiDevicePairingDialogue_mac.mm"

 #if DRX_USE_CDREADER
  #include "native/drx_AudioCDReader_mac.mm"
 #endif

 #if DRX_USE_CDBURNER
  #include "native/drx_AudioCDBurner_mac.mm"
 #endif

#elif DRX_IOS
 #include "native/drx_BluetoothMidiDevicePairingDialogue_ios.mm"

#elif DRX_ANDROID
 #include "native/drx_BluetoothMidiDevicePairingDialogue_android.cpp"

#elif DRX_LINUX || DRX_BSD
 #if DRX_USE_CDREADER
  #include "native/drx_AudioCDReader_linux.cpp"
 #endif

 #include "native/drx_BluetoothMidiDevicePairingDialogue_linux.cpp"

#elif DRX_WINDOWS
 #include "native/drx_BluetoothMidiDevicePairingDialogue_windows.cpp"

 #if DRX_USE_CDREADER
  #include "native/drx_AudioCDReader_windows.cpp"
 #endif

 #if DRX_USE_CDBURNER
  #include "native/drx_AudioCDBurner_windows.cpp"
 #endif

#endif
