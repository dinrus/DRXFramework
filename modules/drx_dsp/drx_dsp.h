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

  ID:                 drx_dsp
  vendor:             drx
  version:            8.0.7
  name:               DRX DSP classes
  description:        Classes for audio buffer manipulation, digital audio processing, filtering, oversampling, fast math functions etc.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_audio_formats
  OSXFrameworks:      Accelerate
  iOSFrameworks:      Accelerate

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once

#define DRX_DSP_H_INCLUDED

#include <drx_audio_basics/drx_audio_basics.h>

#if DRX_INTEL

 #if defined (_M_X64) || defined (__amd64__)
  #ifndef __SSE2__
   #define __SSE2__
  #endif
 #endif

 #ifndef DRX_USE_SIMD
  #define DRX_USE_SIMD 1
 #endif

 #if DRX_USE_SIMD
  #include <immintrin.h>
 #endif

#elif DRX_ARM

 #ifndef DRX_USE_SIMD
  #define DRX_USE_SIMD 1
 #endif

 #if DRX_64BIT && DRX_WINDOWS
  #include <arm64_neon.h>
 #else
  #include <arm_neon.h>
 #endif

#else

 // No SIMD Support
 #ifndef DRX_USE_SIMD
  #define DRX_USE_SIMD 0
 #endif

#endif

#ifndef DRX_VECTOR_CALLTYPE
 // __vectorcall does not work on 64-bit due to internal compiler error in
 // release mode VS2017. Re-enable when Microsoft fixes this
 #if _MSC_VER && DRX_USE_SIMD && ! (defined (_M_X64) || defined (__amd64__))
  #define DRX_VECTOR_CALLTYPE __vectorcall
 #else
  #define DRX_VECTOR_CALLTYPE
 #endif
#endif

#include <complex>


//==============================================================================
/** Config: DRX_ASSERTION_FIRFILTER

    When this flag is enabled, an assertion will be generated during the
    execution of DEBUG configurations if you use a FIRFilter class to process
    FIRCoefficients with a size higher than 128, to tell you that's it would be
    more efficient to use the Convolution class instead. It is enabled by
    default, but you may want to disable it if you really want to process such
    a filter in the time domain.
*/
#ifndef DRX_ASSERTION_FIRFILTER
 #define DRX_ASSERTION_FIRFILTER 1
#endif

/** Config: DRX_DSP_USE_INTEL_MKL

    If this flag is set, then DRX will use Intel's MKL for DRX's FFT and
    convolution classes.

    If you're using the Projucer's Visual Studio exporter, you should also set
    the "Use MKL Library (oneAPI)" option in the exporter settings to
    "Sequential" or "Parallel". If you're not using the Visual Studio exporter,
    the folder containing the mkl_dfti.h header must be in your header search
    paths, and you must link against all the necessary MKL libraries.
*/
#ifndef DRX_DSP_USE_INTEL_MKL
 #define DRX_DSP_USE_INTEL_MKL 0
#endif

/** Config: DRX_DSP_USE_SHARED_FFTW

    If this flag is set, then DRX will search for the fftw shared libraries
    at runtime and use the library for DRX's FFT and convolution classes.

    If the library is not found, then DRX's fallback FFT routines will be used.

    This is especially useful on linux as fftw often comes pre-installed on
    popular linux distros.

    You must respect the FFTW license when enabling this option.
*/
 #ifndef DRX_DSP_USE_SHARED_FFTW
 #define DRX_DSP_USE_SHARED_FFTW 0
#endif

/** Config: DRX_DSP_USE_STATIC_FFTW

    If this flag is set, then DRX will use the statically linked fftw libraries
    for DRX's FFT and convolution classes.

    You must add the fftw header/library folder to the extra header/library search
    paths of your DRX project. You also need to add the fftw library itself
    to the extra libraries supplied to your DRX project during linking.

    You must respect the FFTW license when enabling this option.
*/
#ifndef DRX_DSP_USE_STATIC_FFTW
 #define DRX_DSP_USE_STATIC_FFTW 0
#endif

/** Config: DRX_DSP_ENABLE_SNAP_TO_ZERO

    Enables code in the dsp module to avoid floating point denormals during the
    processing of some of the dsp module's filters.

    Enabling this will add a slight performance overhead to the DSP module's
    filters and algorithms. If your audio app already disables denormals altogether
    (for example, by using the ScopedNoDenormals class or the
    FloatVectorOperations::disableDenormalisedNumberSupport method), then you
    can safely disable this flag to shave off a few cpu cycles from the DSP module's
    filters and algorithms.
*/
#ifndef DRX_DSP_ENABLE_SNAP_TO_ZERO
 #define DRX_DSP_ENABLE_SNAP_TO_ZERO 1
#endif


//==============================================================================
#undef Complex  // apparently some C libraries actually define these symbols (!)
#undef Factor
#undef check

namespace drx::dsp
{

template <typename Type>
using Complex = std::complex<Type>;

template <size_t len, typename T>
using FixedSizeFunction = drx::FixedSizeFunction<len, T>;

//==============================================================================
namespace util
{
    /** Use this function to prevent denormals on intel CPUs.
        This function will work with both primitives and simple containers.
    */
  #if DRX_DSP_ENABLE_SNAP_TO_ZERO
    inline z0 snapToZero (f32&       x) noexcept            { DRX_SNAP_TO_ZERO (x); }
   #ifndef DOXYGEN
    inline z0 snapToZero (f64&      x) noexcept            { DRX_SNAP_TO_ZERO (x); }
    inline z0 snapToZero (real_t& x) noexcept            { DRX_SNAP_TO_ZERO (x); }
   #endif
  #else
    inline z0 snapToZero ([[maybe_unused]] f32&       x) noexcept            {}
   #ifndef DOXYGEN
    inline z0 snapToZero ([[maybe_unused]] f64&      x) noexcept            {}
    inline z0 snapToZero ([[maybe_unused]] real_t& x) noexcept            {}
   #endif
  #endif
}

}

//==============================================================================
#if DRX_USE_SIMD
 #include "native/drx_SIMDNativeOps_fallback.h"

 // include the correct native file for this build target CPU
 #if DRX_INTEL
  #ifdef __AVX2__
   #include "native/drx_SIMDNativeOps_avx.h"
  #else
   #include "native/drx_SIMDNativeOps_sse.h"
  #endif
 #elif DRX_ARM
  #include "native/drx_SIMDNativeOps_neon.h"
 #else
  #error "SIMD register support not implemented for this platform"
 #endif

 #include "containers/drx_SIMDRegister.h"
 #include "containers/drx_SIMDRegister_Impl.h"
#endif

#include "maths/drx_SpecialFunctions.h"
#include "maths/drx_Matrix.h"
#include "maths/drx_Phase.h"
#include "maths/drx_Polynomial.h"
#include "maths/drx_FastMathApproximations.h"
#include "maths/drx_LookupTable.h"
#include "maths/drx_LogRampedValue.h"
#include "containers/drx_AudioBlock.h"
#include "processors/drx_ProcessContext.h"
#include "processors/drx_ProcessorWrapper.h"
#include "processors/drx_ProcessorChain.h"
#include "processors/drx_ProcessorDuplicator.h"
#include "processors/drx_IIRFilter.h"
#include "processors/drx_IIRFilter_Impl.h"
#include "processors/drx_FIRFilter.h"
#include "processors/drx_StateVariableFilter.h"
#include "processors/drx_FirstOrderTPTFilter.h"
#include "processors/drx_Panner.h"
#include "processors/drx_DelayLine.h"
#include "processors/drx_Oversampling.h"
#include "processors/drx_BallisticsFilter.h"
#include "processors/drx_LinkwitzRileyFilter.h"
#include "processors/drx_DryWetMixer.h"
#include "processors/drx_StateVariableTPTFilter.h"
#include "frequency/drx_FFT.h"
#include "frequency/drx_Convolution.h"
#include "frequency/drx_Windowing.h"
#include "filter_design/drx_FilterDesign.h"
#include "widgets/drx_Reverb.h"
#include "widgets/drx_Bias.h"
#include "widgets/drx_Gain.h"
#include "widgets/drx_WaveShaper.h"
#include "widgets/drx_Oscillator.h"
#include "widgets/drx_LadderFilter.h"
#include "widgets/drx_Compressor.h"
#include "widgets/drx_NoiseGate.h"
#include "widgets/drx_Limiter.h"
#include "widgets/drx_Phaser.h"
#include "widgets/drx_Chorus.h"
