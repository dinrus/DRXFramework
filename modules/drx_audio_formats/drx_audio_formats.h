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

  ID:                 drx_audio_formats
  vendor:             drx
  version:            8.0.7
  name:               DRX audio file format codecs
  description:        Classes for reading and writing various audio file formats.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_audio_basics
  OSXFrameworks:      CoreAudio CoreMIDI QuartzCore AudioToolbox
  iOSFrameworks:      AudioToolbox QuartzCore

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_AUDIO_FORMATS_H_INCLUDED

#include <drx_audio_basics/drx_audio_basics.h>

//==============================================================================
/** Config: DRX_USE_FLAC
    Enables the FLAC audio codec classes (available on all platforms).
    If your app doesn't need to read FLAC files, you might want to disable this to
    reduce the size of your codebase and build time.
*/
#ifndef DRX_USE_FLAC
 #define DRX_USE_FLAC 1
#endif

/** Config: DRX_USE_OGGVORBIS
    Enables the Ogg-Vorbis audio codec classes (available on all platforms).
    If your app doesn't need to read Ogg-Vorbis files, you might want to disable this to
    reduce the size of your codebase and build time.
*/
#ifndef DRX_USE_OGGVORBIS
 #define DRX_USE_OGGVORBIS 1
#endif

/** Config: DRX_USE_MP3AUDIOFORMAT
    Enables the software-based MP3AudioFormat class.
    IMPORTANT DISCLAIMER: By choosing to enable the DRX_USE_MP3AUDIOFORMAT flag and to compile
    this MP3 code into your software, you do so AT YOUR OWN RISK! By doing so, you are agreeing
    that DinrusPro is in no way responsible for any patent, copyright, or other
    legal issues that you may suffer as a result.

    The code in drx_MP3AudioFormat.cpp is NOT guaranteed to be free from infringements of 3rd-party
    intellectual property. If you wish to use it, please seek your own independent advice about the
    legality of doing so. If you are not willing to accept full responsibility for the consequences
    of using this code, then do not enable this setting.
*/
#ifndef DRX_USE_MP3AUDIOFORMAT
 #define DRX_USE_MP3AUDIOFORMAT 0
#endif

/** Config: DRX_USE_LAME_AUDIO_FORMAT
    Enables the LameEncoderAudioFormat class.
*/
#ifndef DRX_USE_LAME_AUDIO_FORMAT
 #define DRX_USE_LAME_AUDIO_FORMAT 0
#endif

/** Config: DRX_USE_WINDOWS_MEDIA_FORMAT
    Enables the Windows Media SDK codecs.
*/
#ifndef DRX_USE_WINDOWS_MEDIA_FORMAT
 #define DRX_USE_WINDOWS_MEDIA_FORMAT 1
#endif

#if ! DRX_WINDOWS
 #undef DRX_USE_WINDOWS_MEDIA_FORMAT
 #define DRX_USE_WINDOWS_MEDIA_FORMAT 0
#endif

//==============================================================================
#include "format/drx_AudioFormatReader.h"
#include "format/drx_AudioFormatWriter.h"
#include "format/drx_MemoryMappedAudioFormatReader.h"
#include "format/drx_AudioFormat.h"
#include "format/drx_AudioFormatManager.h"
#include "format/drx_AudioFormatReaderSource.h"
#include "format/drx_AudioSubsectionReader.h"
#include "format/drx_BufferingAudioFormatReader.h"
#include "codecs/drx_AiffAudioFormat.h"
#include "codecs/drx_CoreAudioFormat.h"
#include "codecs/drx_FlacAudioFormat.h"
#include "codecs/drx_LAMEEncoderAudioFormat.h"
#include "codecs/drx_MP3AudioFormat.h"
#include "codecs/drx_OggVorbisAudioFormat.h"
#include "codecs/drx_WavAudioFormat.h"
#include "codecs/drx_WindowsMediaAudioFormat.h"
#include "sampler/drx_Sampler.h"

#if DrxPlugin_Enable_ARA
 #include <drx_audio_processors/drx_audio_processors.h>

 #include "format/drx_ARAAudioReaders.h"
#endif
