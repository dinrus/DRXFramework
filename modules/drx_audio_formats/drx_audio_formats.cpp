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

#ifdef DRX_AUDIO_FORMATS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1

#include "drx_audio_formats.h"

//==============================================================================
#if DRX_MAC
 #include <AudioToolbox/AudioToolbox.h>

#elif DRX_IOS
 #import <AudioToolbox/AudioToolbox.h>
 #import <AVFoundation/AVFoundation.h>

//==============================================================================
#elif DRX_WINDOWS && DRX_USE_WINDOWS_MEDIA_FORMAT
 #include <wmsdk.h>
#endif

//==============================================================================
#include "format/drx_AudioFormat.cpp"
#include "format/drx_AudioFormatManager.cpp"
#include "format/drx_AudioFormatReader.cpp"
#include "format/drx_AudioFormatReaderSource.cpp"
#include "format/drx_AudioFormatWriter.cpp"
#include "format/drx_AudioSubsectionReader.cpp"
#include "format/drx_BufferingAudioFormatReader.cpp"
#include "sampler/drx_Sampler.cpp"
#include "codecs/drx_AiffAudioFormat.cpp"
#include "codecs/drx_CoreAudioFormat.cpp"
#include "codecs/drx_FlacAudioFormat.cpp"
#include "codecs/drx_MP3AudioFormat.cpp"
#include "codecs/drx_OggVorbisAudioFormat.cpp"
#include "codecs/drx_WavAudioFormat.cpp"
#include "codecs/drx_LAMEEncoderAudioFormat.cpp"

#if DrxPlugin_Enable_ARA
 #include "drx_audio_processors/utilities/ARA/drx_ARADocumentControllerCommon.cpp"
 #include "format/drx_ARAAudioReaders.cpp"
#endif

#if DRX_WINDOWS && DRX_USE_WINDOWS_MEDIA_FORMAT
 #include "codecs/drx_WindowsMediaAudioFormat.cpp"
#endif
