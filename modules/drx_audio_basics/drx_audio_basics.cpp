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

#ifdef DRX_AUDIO_BASICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#include "drx_audio_basics.h"

#if DRX_USE_SSE_INTRINSICS
 #include <emmintrin.h>
#endif

#if DRX_MAC || DRX_IOS
 #ifndef DRX_USE_VDSP_FRAMEWORK
  #define DRX_USE_VDSP_FRAMEWORK 1
 #endif

 #if DRX_USE_VDSP_FRAMEWORK
  #include <Accelerate/Accelerate.h>
 #endif

 #include "native/drx_AudioWorkgroup_mac.h"

#elif DRX_USE_VDSP_FRAMEWORK
 #undef DRX_USE_VDSP_FRAMEWORK
#endif

#if DRX_USE_ARM_NEON
 #include <arm_neon.h>
#endif

#include "buffers/drx_AudioDataConverters.cpp"
#include "buffers/drx_FloatVectorOperations.cpp"
#include "buffers/drx_AudioChannelSet.cpp"
#include "buffers/drx_AudioProcessLoadMeasurer.cpp"
#include "utilities/drx_IIRFilter.cpp"
#include "utilities/drx_LagrangeInterpolator.cpp"
#include "utilities/drx_WindowedSincInterpolator.cpp"
#include "utilities/drx_Interpolators.cpp"
#include "utilities/drx_SmoothedValue.cpp"
#include "midi/drx_MidiBuffer.cpp"
#include "midi/drx_MidiFile.cpp"
#include "midi/drx_MidiKeyboardState.cpp"
#include "midi/drx_MidiMessage.cpp"
#include "midi/drx_MidiMessageSequence.cpp"
#include "midi/drx_MidiRPN.cpp"
#include "mpe/drx_MPEValue.cpp"
#include "mpe/drx_MPENote.cpp"
#include "mpe/drx_MPEZoneLayout.cpp"
#include "mpe/drx_MPEInstrument.cpp"
#include "mpe/drx_MPEMessages.cpp"
#include "mpe/drx_MPESynthesiserBase.cpp"
#include "mpe/drx_MPESynthesiserVoice.cpp"
#include "mpe/drx_MPESynthesiser.cpp"
#include "mpe/drx_MPEUtils.cpp"
#include "sources/drx_BufferingAudioSource.cpp"
#include "sources/drx_ChannelRemappingAudioSource.cpp"
#include "sources/drx_IIRFilterAudioSource.cpp"
#include "sources/drx_MemoryAudioSource.cpp"
#include "sources/drx_MixerAudioSource.cpp"
#include "sources/drx_ResamplingAudioSource.cpp"
#include "sources/drx_ReverbAudioSource.cpp"
#include "sources/drx_ToneGeneratorAudioSource.cpp"
#include "sources/drx_PositionableAudioSource.cpp"
#include "synthesisers/drx_Synthesiser.cpp"
#include "audio_play_head/drx_AudioPlayHead.cpp"
#include "midi/drx_MidiDataConcatenator.h"
#include "midi/ump/drx_UMP.h"
#include "midi/ump/drx_UMPUtils.cpp"
#include "midi/ump/drx_UMPView.cpp"
#include "midi/ump/drx_UMPSysEx7.cpp"
#include "midi/ump/drx_UMPMidi1ToMidi2DefaultTranslator.cpp"
#include "midi/ump/drx_UMPIterator.cpp"
#include "utilities/drx_AudioWorkgroup.cpp"

#if DRX_UNIT_TESTS
 #include "utilities/drx_ADSR_test.cpp"
 #include "midi/ump/drx_UMP_test.cpp"
#endif
