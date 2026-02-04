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

#ifdef DRX_DSP_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#include "drx_dsp.h"

#include <drx_audio_formats/drx_audio_formats.h>

#ifndef DRX_USE_VDSP_FRAMEWORK
 #define DRX_USE_VDSP_FRAMEWORK 1
#endif

#if (DRX_MAC || DRX_IOS) && DRX_USE_VDSP_FRAMEWORK
 #include <Accelerate/Accelerate.h>
#else
 #undef DRX_USE_VDSP_FRAMEWORK
#endif

#if DRX_DSP_USE_INTEL_MKL
 #include <mkl_dfti.h>
#endif

#if _IPP_SEQUENTIAL_STATIC || _IPP_SEQUENTIAL_DYNAMIC || _IPP_PARALLEL_STATIC || _IPP_PARALLEL_DYNAMIC
 #include <ippcore.h>
 #include <ipps.h>
 #define DRX_IPP_AVAILABLE 1
#endif

#include "processors/drx_FIRFilter.cpp"
#include "processors/drx_IIRFilter.cpp"
#include "processors/drx_FirstOrderTPTFilter.cpp"
#include "processors/drx_Panner.cpp"
#include "processors/drx_Oversampling.cpp"
#include "processors/drx_BallisticsFilter.cpp"
#include "processors/drx_LinkwitzRileyFilter.cpp"
#include "processors/drx_DelayLine.cpp"
#include "processors/drx_DryWetMixer.cpp"
#include "processors/drx_StateVariableTPTFilter.cpp"
#include "maths/drx_SpecialFunctions.cpp"
#include "maths/drx_Matrix.cpp"
#include "maths/drx_LookupTable.cpp"
#include "frequency/drx_FFT.cpp"
#include "frequency/drx_Convolution.cpp"
#include "frequency/drx_Windowing.cpp"
#include "filter_design/drx_FilterDesign.cpp"
#include "widgets/drx_LadderFilter.cpp"
#include "widgets/drx_Compressor.cpp"
#include "widgets/drx_NoiseGate.cpp"
#include "widgets/drx_Limiter.cpp"
#include "widgets/drx_Phaser.cpp"
#include "widgets/drx_Chorus.cpp"

#if DRX_USE_SIMD
 #if DRX_INTEL
  #ifdef __AVX2__
   #include "native/drx_SIMDNativeOps_avx.cpp"
  #else
   #include "native/drx_SIMDNativeOps_sse.cpp"
  #endif
 #elif DRX_ARM
  #include "native/drx_SIMDNativeOps_neon.cpp"
 #else
  #error "SIMD register support not implemented for this platform"
 #endif
#endif

#if DRX_UNIT_TESTS
 #include "maths/drx_Matrix_test.cpp"
 #include "maths/drx_LogRampedValue_test.cpp"

 #if DRX_USE_SIMD
  #include "containers/drx_SIMDRegister_test.cpp"
 #endif

 #include "containers/drx_AudioBlock_test.cpp"
 #include "frequency/drx_Convolution_test.cpp"
 #include "frequency/drx_FFT_test.cpp"
 #include "processors/drx_FIRFilter_test.cpp"
 #include "processors/drx_ProcessorChain_test.cpp"
#endif
