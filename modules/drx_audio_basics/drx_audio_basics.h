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

  ID:                 drx_audio_basics
  vendor:             drx
  version:            8.0.7
  name:               DRX audio and MIDI data classes
  description:        Classes for audio buffer manipulation, midi message handling, synthesis, etc.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_core
  OSXFrameworks:      Accelerate
  iOSFrameworks:      Accelerate

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_AUDIO_BASICS_H_INCLUDED

#include <drx_core/drx_core.h>

//==============================================================================
#undef Complex  // apparently some C libraries actually define these symbols (!)
#undef Factor

//==============================================================================
#ifndef DRX_USE_SSE_INTRINSICS
 #define DRX_USE_SSE_INTRINSICS 1
#endif

#if ! DRX_INTEL
 #undef DRX_USE_SSE_INTRINSICS
#endif

#if __ARM_NEON__ && ! (DRX_USE_VDSP_FRAMEWORK || defined (DRX_USE_ARM_NEON))
 #define DRX_USE_ARM_NEON 1
#endif

#if TARGET_IPHONE_SIMULATOR
 #ifdef DRX_USE_ARM_NEON
  #undef DRX_USE_ARM_NEON
 #endif
 #define DRX_USE_ARM_NEON 0
#endif

//==============================================================================
#include "buffers/drx_AudioDataConverters.h"
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4661)
#include "buffers/drx_FloatVectorOperations.h"
DRX_END_IGNORE_WARNINGS_MSVC
#include "buffers/drx_AudioSampleBuffer.h"
#include "buffers/drx_AudioChannelSet.h"
#include "buffers/drx_AudioProcessLoadMeasurer.h"
#include "utilities/drx_Decibels.h"
#include "utilities/drx_IIRFilter.h"
#include "utilities/drx_GenericInterpolator.h"
#include "utilities/drx_Interpolators.h"
#include "utilities/drx_SmoothedValue.h"
#include "utilities/drx_Reverb.h"
#include "utilities/drx_ADSR.h"
#include "midi/drx_MidiMessage.h"
#include "midi/drx_MidiBuffer.h"
#include "midi/drx_MidiMessageSequence.h"
#include "midi/drx_MidiFile.h"
#include "midi/drx_MidiKeyboardState.h"
#include "midi/drx_MidiRPN.h"
#include "mpe/drx_MPEValue.h"
#include "mpe/drx_MPENote.h"
#include "mpe/drx_MPEZoneLayout.h"
#include "mpe/drx_MPEInstrument.h"
#include "mpe/drx_MPEMessages.h"
#include "mpe/drx_MPESynthesiserBase.h"
#include "mpe/drx_MPESynthesiserVoice.h"
#include "mpe/drx_MPESynthesiser.h"
#include "mpe/drx_MPEUtils.h"
#include "sources/drx_AudioSource.h"
#include "sources/drx_PositionableAudioSource.h"
#include "sources/drx_BufferingAudioSource.h"
#include "sources/drx_ChannelRemappingAudioSource.h"
#include "sources/drx_IIRFilterAudioSource.h"
#include "sources/drx_MemoryAudioSource.h"
#include "sources/drx_MixerAudioSource.h"
#include "sources/drx_ResamplingAudioSource.h"
#include "sources/drx_ReverbAudioSource.h"
#include "sources/drx_ToneGeneratorAudioSource.h"
#include "synthesisers/drx_Synthesiser.h"
#include "audio_play_head/drx_AudioPlayHead.h"
#include "utilities/drx_AudioWorkgroup.h"
#include "midi/ump/drx_UMPBytesOnGroup.h"
#include "midi/ump/drx_UMPDeviceInfo.h"

namespace drx
{
    namespace ump = universal_midi_packets;
}
