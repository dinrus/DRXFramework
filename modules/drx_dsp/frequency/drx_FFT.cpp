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

namespace drx::dsp
{

struct FFT::Instance
{
    virtual ~Instance() = default;
    virtual z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept = 0;
    virtual z0 performRealOnlyForwardTransform (f32*, b8) const noexcept = 0;
    virtual z0 performRealOnlyInverseTransform (f32*) const noexcept = 0;
};

struct FFT::Engine
{
    Engine (i32 priorityToUse) : enginePriority (priorityToUse)
    {
        auto& list = getEngines();
        list.add (this);
        std::sort (list.begin(), list.end(), [] (Engine* a, Engine* b) { return b->enginePriority < a->enginePriority; });
    }

    virtual ~Engine() = default;

    virtual FFT::Instance* create (i32 order) const = 0;

    //==============================================================================
    static FFT::Instance* createBestEngineForPlatform (i32 order)
    {
        for (auto* engine : getEngines())
            if (auto* instance = engine->create (order))
                return instance;

        jassertfalse;  // This should never happen as the fallback engine should always work!
        return nullptr;
    }

private:
    static Array<Engine*>& getEngines()
    {
        static Array<Engine*> engines;
        return engines;
    }

    i32 enginePriority; // used so that faster engines have priority over slower ones
};

template <typename InstanceToUse>
struct FFT::EngineImpl  : public FFT::Engine
{
    EngineImpl() : FFT::Engine (InstanceToUse::priority)        {}
    FFT::Instance* create (i32 order) const override            { return InstanceToUse::create (order); }
};

//==============================================================================
//==============================================================================
struct FFTFallback final : public FFT::Instance
{
    // this should have the least priority of all engines
    static constexpr i32 priority = -1;

    static FFTFallback* create (i32 order)
    {
        return new FFTFallback (order);
    }

    FFTFallback (i32 order)
    {
        configForward.reset (new FFTConfig (1 << order, false));
        configInverse.reset (new FFTConfig (1 << order, true));

        size = 1 << order;
    }

    z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept override
    {
        if (size == 1)
        {
            *output = *input;
            return;
        }

        const SpinLock::ScopedLockType sl (processLock);

        jassert (configForward != nullptr);

        if (inverse)
        {
            configInverse->perform (input, output);

            const f32 scaleFactor = 1.0f / (f32) size;

            for (i32 i = 0; i < size; ++i)
                output[i] *= scaleFactor;
        }
        else
        {
            configForward->perform (input, output);
        }
    }

    const size_t maxFFTScratchSpaceToAlloca = 256 * 1024;

    z0 performRealOnlyForwardTransform (f32* d, b8) const noexcept override
    {
        if (size == 1)
            return;

        const size_t scratchSize = 16 + (size_t) size * sizeof (Complex<f32>);

        if (scratchSize < maxFFTScratchSpaceToAlloca)
        {
            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255)
            performRealOnlyForwardTransform (static_cast<Complex<f32>*> (alloca (scratchSize)), d);
            DRX_END_IGNORE_WARNINGS_MSVC
        }
        else
        {
            HeapBlock<t8> heapSpace (scratchSize);
            performRealOnlyForwardTransform (unalignedPointerCast<Complex<f32>*> (heapSpace.getData()), d);
        }
    }

    z0 performRealOnlyInverseTransform (f32* d) const noexcept override
    {
        if (size == 1)
            return;

        const size_t scratchSize = 16 + (size_t) size * sizeof (Complex<f32>);

        if (scratchSize < maxFFTScratchSpaceToAlloca)
        {
            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255)
            performRealOnlyInverseTransform (static_cast<Complex<f32>*> (alloca (scratchSize)), d);
            DRX_END_IGNORE_WARNINGS_MSVC
        }
        else
        {
            HeapBlock<t8> heapSpace (scratchSize);
            performRealOnlyInverseTransform (unalignedPointerCast<Complex<f32>*> (heapSpace.getData()), d);
        }
    }

    z0 performRealOnlyForwardTransform (Complex<f32>* scratch, f32* d) const noexcept
    {
        for (i32 i = 0; i < size; ++i)
            scratch[i] = { d[i], 0 };

        perform (scratch, reinterpret_cast<Complex<f32>*> (d), false);
    }

    z0 performRealOnlyInverseTransform (Complex<f32>* scratch, f32* d) const noexcept
    {
        auto* input = reinterpret_cast<Complex<f32>*> (d);

        for (i32 i = size >> 1; i < size; ++i)
            input[i] = std::conj (input[size - i]);

        perform (input, scratch, true);

        for (i32 i = 0; i < size; ++i)
        {
            d[i] = scratch[i].real();
            d[i + size] = scratch[i].imag();
        }
    }

    //==============================================================================
    struct FFTConfig
    {
        FFTConfig (i32 sizeOfFFT, b8 isInverse)
            : fftSize (sizeOfFFT), inverse (isInverse), twiddleTable ((size_t) sizeOfFFT)
        {
            auto inverseFactor = (inverse ? 2.0 : -2.0) * MathConstants<f64>::pi / (f64) fftSize;

            if (fftSize <= 4)
            {
                for (i32 i = 0; i < fftSize; ++i)
                {
                    auto phase = i * inverseFactor;

                    twiddleTable[i] = { (f32) std::cos (phase),
                                        (f32) std::sin (phase) };
                }
            }
            else
            {
                for (i32 i = 0; i < fftSize / 4; ++i)
                {
                    auto phase = i * inverseFactor;

                    twiddleTable[i] = { (f32) std::cos (phase),
                                        (f32) std::sin (phase) };
                }

                for (i32 i = fftSize / 4; i < fftSize / 2; ++i)
                {
                    auto other = twiddleTable[i - fftSize / 4];

                    twiddleTable[i] = { inverse ? -other.imag() :  other.imag(),
                                        inverse ?  other.real() : -other.real() };
                }

                twiddleTable[fftSize / 2].real (-1.0f);
                twiddleTable[fftSize / 2].imag (0.0f);

                for (i32 i = fftSize / 2; i < fftSize; ++i)
                {
                    auto index = fftSize / 2 - (i - fftSize / 2);
                    twiddleTable[i] = conj (twiddleTable[index]);
                }
            }

            auto root = (i32) std::sqrt ((f64) fftSize);
            i32 divisor = 4, n = fftSize;

            for (i32 i = 0; i < numElementsInArray (factors); ++i)
            {
                while ((n % divisor) != 0)
                {
                    if (divisor == 2)       divisor = 3;
                    else if (divisor == 4)  divisor = 2;
                    else                    divisor += 2;

                    if (divisor > root)
                        divisor = n;
                }

                n /= divisor;

                jassert (divisor == 1 || divisor == 2 || divisor == 4);
                factors[i].radix = divisor;
                factors[i].length = n;
            }
        }

        z0 perform (const Complex<f32>* input, Complex<f32>* output) const noexcept
        {
            perform (input, output, 1, 1, factors);
        }

        i32k fftSize;
        const b8 inverse;

        struct Factor { i32 radix, length; };
        Factor factors[32];
        HeapBlock<Complex<f32>> twiddleTable;

        z0 perform (const Complex<f32>* input, Complex<f32>* output, i32 stride, i32 strideIn, const Factor* facs) const noexcept
        {
            auto factor = *facs++;
            auto* originalOutput = output;
            auto* outputEnd = output + factor.radix * factor.length;

            if (stride == 1 && factor.radix <= 5)
            {
                for (i32 i = 0; i < factor.radix; ++i)
                    perform (input + stride * strideIn * i, output + i * factor.length, stride * factor.radix, strideIn, facs);

                butterfly (factor, output, stride);
                return;
            }

            if (factor.length == 1)
            {
                do
                {
                    *output++ = *input;
                    input += stride * strideIn;
                }
                while (output < outputEnd);
            }
            else
            {
                do
                {
                    perform (input, output, stride * factor.radix, strideIn, facs);
                    input += stride * strideIn;
                    output += factor.length;
                }
                while (output < outputEnd);
            }

            butterfly (factor, originalOutput, stride);
        }

        z0 butterfly (const Factor factor, Complex<f32>* data, i32 stride) const noexcept
        {
            switch (factor.radix)
            {
                case 1:   break;
                case 2:   butterfly2 (data, stride, factor.length); return;
                case 4:   butterfly4 (data, stride, factor.length); return;
                default:  jassertfalse; break;
            }

            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255)
            auto* scratch = static_cast<Complex<f32>*> (alloca ((size_t) factor.radix * sizeof (Complex<f32>)));
            DRX_END_IGNORE_WARNINGS_MSVC

            for (i32 i = 0; i < factor.length; ++i)
            {
                for (i32 k = i, q1 = 0; q1 < factor.radix; ++q1)
                {
                    DRX_BEGIN_IGNORE_WARNINGS_MSVC (6386)
                    scratch[q1] = data[k];
                    DRX_END_IGNORE_WARNINGS_MSVC
                    k += factor.length;
                }

                for (i32 k = i, q1 = 0; q1 < factor.radix; ++q1)
                {
                    i32 twiddleIndex = 0;
                    data[k] = scratch[0];

                    for (i32 q = 1; q < factor.radix; ++q)
                    {
                        twiddleIndex += stride * k;

                        if (twiddleIndex >= fftSize)
                            twiddleIndex -= fftSize;

                        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6385)
                        data[k] += scratch[q] * twiddleTable[twiddleIndex];
                        DRX_END_IGNORE_WARNINGS_MSVC
                    }

                    k += factor.length;
                }
            }
        }

        z0 butterfly2 (Complex<f32>* data, i32k stride, i32k length) const noexcept
        {
            auto* dataEnd = data + length;
            auto* tw = twiddleTable.getData();

            for (i32 i = length; --i >= 0;)
            {
                auto s = *dataEnd;
                s *= (*tw);
                tw += stride;
                *dataEnd++ = *data - s;
                *data++ += s;
            }
        }

        z0 butterfly4 (Complex<f32>* data, i32k stride, i32k length) const noexcept
        {
            auto lengthX2 = length * 2;
            auto lengthX3 = length * 3;

            auto strideX2 = stride * 2;
            auto strideX3 = stride * 3;

            auto* twiddle1 = twiddleTable.getData();
            auto* twiddle2 = twiddle1;
            auto* twiddle3 = twiddle1;

            for (i32 i = length; --i >= 0;)
            {
                auto s0 = data[length]   * *twiddle1;
                auto s1 = data[lengthX2] * *twiddle2;
                auto s2 = data[lengthX3] * *twiddle3;
                auto s3 = s0;             s3 += s2;
                auto s4 = s0;             s4 -= s2;
                auto s5 = *data;          s5 -= s1;

                *data += s1;
                data[lengthX2] = *data;
                data[lengthX2] -= s3;
                twiddle1 += stride;
                twiddle2 += strideX2;
                twiddle3 += strideX3;
                *data += s3;

                if (inverse)
                {
                    data[length] = { s5.real() - s4.imag(),
                                     s5.imag() + s4.real() };

                    data[lengthX3] = { s5.real() + s4.imag(),
                                       s5.imag() - s4.real() };
                }
                else
                {
                    data[length] = { s5.real() + s4.imag(),
                                     s5.imag() - s4.real() };

                    data[lengthX3] = { s5.real() - s4.imag(),
                                       s5.imag() + s4.real() };
                }

                ++data;
            }
        }

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTConfig)
    };

    //==============================================================================
    SpinLock processLock;
    std::unique_ptr<FFTConfig> configForward, configInverse;
    i32 size;
};

FFT::EngineImpl<FFTFallback> fftFallback;

//==============================================================================
//==============================================================================
#if (DRX_MAC || DRX_IOS) && DRX_USE_VDSP_FRAMEWORK
struct AppleFFT final : public FFT::Instance
{
    static constexpr i32 priority = 5;

    static AppleFFT* create (i32 order)
    {
        return new AppleFFT (order);
    }

    AppleFFT (i32 orderToUse)
        : order (static_cast<vDSP_Length> (orderToUse)),
          fftSetup (vDSP_create_fftsetup (order, 2)),
          forwardNormalisation (0.5f),
          inverseNormalisation (1.0f / static_cast<f32> (1 << order))
    {}

    ~AppleFFT() override
    {
        if (fftSetup != nullptr)
        {
            vDSP_destroy_fftsetup (fftSetup);
            fftSetup = nullptr;
        }
    }

    z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept override
    {
        auto size = (1 << order);

        DSPSplitComplex splitInput  (toSplitComplex (const_cast<Complex<f32>*> (input)));
        DSPSplitComplex splitOutput (toSplitComplex (output));

        vDSP_fft_zop (fftSetup, &splitInput,  2, &splitOutput, 2,
                      order, inverse ?  kFFTDirection_Inverse : kFFTDirection_Forward);

        f32 factor = (inverse ? inverseNormalisation : forwardNormalisation * 2.0f);
        vDSP_vsmul ((f32*) output, 1, &factor, (f32*) output, 1, static_cast<size_t> (size << 1));
    }

    z0 performRealOnlyForwardTransform (f32* inoutData, b8 ignoreNegativeFreqs) const noexcept override
    {
        auto size = (1 << order);
        auto* inout = reinterpret_cast<Complex<f32>*> (inoutData);
        auto splitInOut (toSplitComplex (inout));

        inoutData[size] = 0.0f;
        vDSP_fft_zrip (fftSetup, &splitInOut, 2, order, kFFTDirection_Forward);
        vDSP_vsmul (inoutData, 1, &forwardNormalisation, inoutData, 1, static_cast<size_t> (size << 1));

        mirrorResult (inout, ignoreNegativeFreqs);
    }

    z0 performRealOnlyInverseTransform (f32* inoutData) const noexcept override
    {
        auto* inout = reinterpret_cast<Complex<f32>*> (inoutData);
        auto size = (1 << order);
        auto splitInOut (toSplitComplex (inout));

        // Imaginary part of nyquist and DC frequencies are always zero
        // so Apple uses the imaginary part of the DC frequency to store
        // the real part of the nyquist frequency
        if (size != 1)
            inout[0] = Complex<f32> (inout[0].real(), inout[size >> 1].real());

        vDSP_fft_zrip (fftSetup, &splitInOut, 2, order, kFFTDirection_Inverse);
        vDSP_vsmul (inoutData, 1, &inverseNormalisation, inoutData, 1, static_cast<size_t> (size << 1));
        vDSP_vclr (inoutData + size, 1, static_cast<size_t> (size));
    }

private:
    //==============================================================================
    z0 mirrorResult (Complex<f32>* out, b8 ignoreNegativeFreqs) const noexcept
    {
        auto size = (1 << order);
        auto i = size >> 1;

        // Imaginary part of nyquist and DC frequencies are always zero
        // so Apple uses the imaginary part of the DC frequency to store
        // the real part of the nyquist frequency
        out[i++] = { out[0].imag(), 0.0 };
        out[0]   = { out[0].real(), 0.0 };

        if (! ignoreNegativeFreqs)
            for (; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    static DSPSplitComplex toSplitComplex (Complex<f32>* data) noexcept
    {
        // this assumes that Complex interleaves real and imaginary parts
        // and is tightly packed.
        return { reinterpret_cast<f32*> (data),
                 reinterpret_cast<f32*> (data) + 1};
    }

    //==============================================================================
    vDSP_Length order;
    FFTSetup fftSetup;
    f32 forwardNormalisation, inverseNormalisation;
};

FFT::EngineImpl<AppleFFT> appleFFT;
#endif

//==============================================================================
//==============================================================================
#if DRX_DSP_USE_SHARED_FFTW || DRX_DSP_USE_STATIC_FFTW

#if DRX_DSP_USE_STATIC_FFTW
extern "C"
{
    uk fftwf_plan_dft_1d     (i32, uk, uk, i32, i32);
    uk fftwf_plan_dft_r2c_1d (i32, uk, uk, i32);
    uk fftwf_plan_dft_c2r_1d (i32, uk, uk, i32);
    z0 fftwf_destroy_plan     (uk);
    z0 fftwf_execute_dft      (uk, uk, uk);
    z0 fftwf_execute_dft_r2c  (uk, uk, uk);
    z0 fftwf_execute_dft_c2r  (uk, uk, uk);
}
#endif

struct FFTWImpl  : public FFT::Instance
{
   #if DRX_DSP_USE_STATIC_FFTW
    // if the DRX developer has gone through the hassle of statically
    // linking in fftw, they probably want to use it
    static constexpr i32 priority = 10;
   #else
    static constexpr i32 priority = 3;
   #endif

    struct FFTWPlan;
    using FFTWPlanRef = FFTWPlan*;

    enum
    {
        measure   = 0,
        unaligned = (1 << 1),
        estimate  = (1 << 6)
    };

    struct Symbols
    {
        FFTWPlanRef (*plan_dft_fftw) (u32, Complex<f32>*, Complex<f32>*, i32, u32);
        FFTWPlanRef (*plan_r2c_fftw) (u32, f32*, Complex<f32>*, u32);
        FFTWPlanRef (*plan_c2r_fftw) (u32, Complex<f32>*, f32*, u32);
        z0 (*destroy_fftw) (FFTWPlanRef);

        z0 (*execute_dft_fftw) (FFTWPlanRef, const Complex<f32>*, Complex<f32>*);
        z0 (*execute_r2c_fftw) (FFTWPlanRef, f32*, Complex<f32>*);
        z0 (*execute_c2r_fftw) (FFTWPlanRef, Complex<f32>*, f32*);

       #if DRX_DSP_USE_STATIC_FFTW
        template <typename FuncPtr, typename ActualSymbolType>
        static b8 symbol (FuncPtr& dst, ActualSymbolType sym)
        {
            dst = reinterpret_cast<FuncPtr> (sym);
            return true;
        }
       #else
        template <typename FuncPtr>
        static b8 symbol (DynamicLibrary& lib, FuncPtr& dst, tukk name)
        {
            dst = reinterpret_cast<FuncPtr> (lib.getFunction (name));
            return (dst != nullptr);
        }
       #endif
    };

    static FFTWImpl* create (i32 order)
    {
        DynamicLibrary lib;

      #if ! DRX_DSP_USE_STATIC_FFTW
       #if DRX_MAC
        auto libName = "libfftw3f.dylib";
       #elif DRX_WINDOWS
        auto libName = "libfftw3f.dll";
       #else
        auto libName = "libfftw3f.so";
       #endif

        if (lib.open (libName))
      #endif
        {
            Symbols symbols;

           #if DRX_DSP_USE_STATIC_FFTW
            if (! Symbols::symbol (symbols.plan_dft_fftw, fftwf_plan_dft_1d))     return nullptr;
            if (! Symbols::symbol (symbols.plan_r2c_fftw, fftwf_plan_dft_r2c_1d)) return nullptr;
            if (! Symbols::symbol (symbols.plan_c2r_fftw, fftwf_plan_dft_c2r_1d)) return nullptr;
            if (! Symbols::symbol (symbols.destroy_fftw,  fftwf_destroy_plan))    return nullptr;

            if (! Symbols::symbol (symbols.execute_dft_fftw, fftwf_execute_dft))     return nullptr;
            if (! Symbols::symbol (symbols.execute_r2c_fftw, fftwf_execute_dft_r2c)) return nullptr;
            if (! Symbols::symbol (symbols.execute_c2r_fftw, fftwf_execute_dft_c2r)) return nullptr;
           #else
            if (! Symbols::symbol (lib, symbols.plan_dft_fftw, "fftwf_plan_dft_1d"))     return nullptr;
            if (! Symbols::symbol (lib, symbols.plan_r2c_fftw, "fftwf_plan_dft_r2c_1d")) return nullptr;
            if (! Symbols::symbol (lib, symbols.plan_c2r_fftw, "fftwf_plan_dft_c2r_1d")) return nullptr;
            if (! Symbols::symbol (lib, symbols.destroy_fftw,  "fftwf_destroy_plan"))    return nullptr;

            if (! Symbols::symbol (lib, symbols.execute_dft_fftw, "fftwf_execute_dft"))     return nullptr;
            if (! Symbols::symbol (lib, symbols.execute_r2c_fftw, "fftwf_execute_dft_r2c")) return nullptr;
            if (! Symbols::symbol (lib, symbols.execute_c2r_fftw, "fftwf_execute_dft_c2r")) return nullptr;
           #endif

            return new FFTWImpl (static_cast<size_t> (order), std::move (lib), symbols);
        }

        return nullptr;
    }

    FFTWImpl (size_t orderToUse, DynamicLibrary&& libraryToUse, const Symbols& symbols)
        : fftwLibrary (std::move (libraryToUse)), fftw (symbols), order (static_cast<size_t> (orderToUse))
    {
        ScopedLock lock (getFFTWPlanLock());

        auto n = (1u << order);
        HeapBlock<Complex<f32>> in (n), out (n);

        c2cForward = fftw.plan_dft_fftw (n, in.getData(), out.getData(), -1, unaligned | estimate);
        c2cInverse = fftw.plan_dft_fftw (n, in.getData(), out.getData(), +1, unaligned | estimate);

        r2c = fftw.plan_r2c_fftw (n, (f32*) in.getData(), in.getData(), unaligned | estimate);
        c2r = fftw.plan_c2r_fftw (n, in.getData(), (f32*) in.getData(), unaligned | estimate);
    }

    ~FFTWImpl() override
    {
        ScopedLock lock (getFFTWPlanLock());

        fftw.destroy_fftw (c2cForward);
        fftw.destroy_fftw (c2cInverse);
        fftw.destroy_fftw (r2c);
        fftw.destroy_fftw (c2r);
    }

    z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept override
    {
        if (inverse)
        {
            auto n = (1u << order);
            fftw.execute_dft_fftw (c2cInverse, input, output);
            FloatVectorOperations::multiply ((f32*) output, 1.0f / static_cast<f32> (n), (i32) n << 1);
        }
        else
        {
            fftw.execute_dft_fftw (c2cForward, input, output);
        }
    }

    z0 performRealOnlyForwardTransform (f32* inputOutputData, b8 ignoreNegativeFreqs) const noexcept override
    {
        if (order == 0)
            return;

        auto* out = reinterpret_cast<Complex<f32>*> (inputOutputData);

        fftw.execute_r2c_fftw (r2c, inputOutputData, out);

        auto size = (1 << order);

        if (! ignoreNegativeFreqs)
            for (i32 i = size >> 1; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    z0 performRealOnlyInverseTransform (f32* inputOutputData) const noexcept override
    {
        auto n = (1u << order);

        fftw.execute_c2r_fftw (c2r, (Complex<f32>*) inputOutputData, inputOutputData);
        FloatVectorOperations::multiply ((f32*) inputOutputData, 1.0f / static_cast<f32> (n), (i32) n);
    }

    //==============================================================================
    // fftw's plan_* and destroy_* methods are NOT thread safe. So we need to share
    // a lock between all instances of FFTWImpl
    static CriticalSection& getFFTWPlanLock() noexcept
    {
        static CriticalSection cs;
        return cs;
    }

    //==============================================================================
    DynamicLibrary fftwLibrary;
    Symbols fftw;
    size_t order;

    FFTWPlanRef c2cForward, c2cInverse, r2c, c2r;
};

FFT::EngineImpl<FFTWImpl> fftwEngine;
#endif

//==============================================================================
//==============================================================================
#if DRX_DSP_USE_INTEL_MKL
struct IntelFFT final : public FFT::Instance
{
    static constexpr i32 priority = 8;

    static b8 succeeded (MKL_LONG status) noexcept        { return status == 0; }

    static IntelFFT* create (i32 orderToUse)
    {
        DFTI_DESCRIPTOR_HANDLE mklc2c, mklc2r;

        if (DftiCreateDescriptor (&mklc2c, DFTI_SINGLE, DFTI_COMPLEX, 1, 1 << orderToUse) == 0)
        {
            if (succeeded (DftiSetValue (mklc2c, DFTI_PLACEMENT, DFTI_NOT_INPLACE))
                 && succeeded (DftiSetValue (mklc2c, DFTI_BACKWARD_SCALE, 1.0f / static_cast<f32> (1 << orderToUse)))
                 && succeeded (DftiCommitDescriptor (mklc2c)))
            {
                if (succeeded (DftiCreateDescriptor (&mklc2r, DFTI_SINGLE, DFTI_REAL, 1, 1 << orderToUse)))
                {
                    if (succeeded (DftiSetValue (mklc2r, DFTI_PLACEMENT, DFTI_INPLACE))
                         && succeeded (DftiSetValue (mklc2r, DFTI_BACKWARD_SCALE, 1.0f / static_cast<f32> (1 << orderToUse)))
                         && succeeded (DftiCommitDescriptor (mklc2r)))
                    {
                        return new IntelFFT (static_cast<size_t> (orderToUse), mklc2c, mklc2r);
                    }

                    DftiFreeDescriptor (&mklc2r);
                }
            }

            DftiFreeDescriptor (&mklc2c);
        }

        return {};
    }

    IntelFFT (size_t orderToUse, DFTI_DESCRIPTOR_HANDLE c2cToUse, DFTI_DESCRIPTOR_HANDLE cr2ToUse)
        : order (orderToUse), c2c (c2cToUse), c2r (cr2ToUse)
    {}

    ~IntelFFT() override
    {
        DftiFreeDescriptor (&c2c);
        DftiFreeDescriptor (&c2r);
    }

    z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept override
    {
        if (inverse)
            DftiComputeBackward (c2c, (uk) input, output);
        else
            DftiComputeForward (c2c, (uk) input, output);
    }

    z0 performRealOnlyForwardTransform (f32* inputOutputData, b8 ignoreNegativeFreqs) const noexcept override
    {
        if (order == 0)
            return;

        DftiComputeForward (c2r, inputOutputData);

        auto* out = reinterpret_cast<Complex<f32>*> (inputOutputData);
        auto size = (1 << order);

        if (! ignoreNegativeFreqs)
            for (i32 i = size >> 1; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    z0 performRealOnlyInverseTransform (f32* inputOutputData) const noexcept override
    {
        DftiComputeBackward (c2r, inputOutputData);
    }

    size_t order;
    DFTI_DESCRIPTOR_HANDLE c2c, c2r;
};

FFT::EngineImpl<IntelFFT> fftwEngine;
#endif

//==============================================================================
//==============================================================================
// Visual Studio should define no more than one of these, depending on the
// setting at 'Project' > 'Properties' > 'Configuration Properties' > 'Intel
// Performance Libraries' > 'Use Intel(R) IPP'
#if _IPP_SEQUENTIAL_STATIC || _IPP_SEQUENTIAL_DYNAMIC || _IPP_PARALLEL_STATIC || _IPP_PARALLEL_DYNAMIC
class IntelPerformancePrimitivesFFT final : public FFT::Instance
{
public:
    static constexpr auto priority = 9;

    static IntelPerformancePrimitivesFFT* create (i32k order)
    {
        auto complexContext = Context<ComplexTraits>::create (order);
        auto realContext    = Context<RealTraits>   ::create (order);

        if (complexContext.isValid() && realContext.isValid())
            return new IntelPerformancePrimitivesFFT (std::move (complexContext), std::move (realContext), order);

        return {};
    }

    z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept override
    {
        if (inverse)
        {
            ippsFFTInv_CToC_32fc (reinterpret_cast<const Ipp32fc*> (input),
                                  reinterpret_cast<Ipp32fc*> (output),
                                  cplx.specPtr,
                                  cplx.workBuf.get());
        }
        else
        {
            ippsFFTFwd_CToC_32fc (reinterpret_cast<const Ipp32fc*> (input),
                                  reinterpret_cast<Ipp32fc*> (output),
                                  cplx.specPtr,
                                  cplx.workBuf.get());
        }
    }

    z0 performRealOnlyForwardTransform (f32* inoutData, b8 ignoreNegativeFreqs) const noexcept override
    {
        ippsFFTFwd_RToCCS_32f_I (inoutData, real.specPtr, real.workBuf.get());

        if (order == 0)
            return;

        auto* out = reinterpret_cast<Complex<f32>*> (inoutData);
        const auto size = (1 << order);

        if (! ignoreNegativeFreqs)
            for (auto i = size >> 1; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    z0 performRealOnlyInverseTransform (f32* inoutData) const noexcept override
    {
        ippsFFTInv_CCSToR_32f_I (inoutData, real.specPtr, real.workBuf.get());
    }

private:
    static constexpr auto flag = IPP_FFT_DIV_INV_BY_N;
    static constexpr auto hint = ippAlgHintFast;

    struct IppFree
    {
        template <typename Ptr>
        z0 operator() (Ptr* ptr) const noexcept { ippsFree (ptr); }
    };

    using IppPtr = std::unique_ptr<Ipp8u[], IppFree>;

    template <typename Traits>
    struct Context
    {
        using SpecPtr = typename Traits::Spec*;

        static Context create (i32k order)
        {
            i32 specSize = 0, initSize = 0, workSize = 0;

            if (Traits::getSize (order, flag, hint, &specSize, &initSize, &workSize) != ippStsNoErr)
                return {};

            const auto initBuf = IppPtr (ippsMalloc_8u (initSize));
            auto specBuf       = IppPtr (ippsMalloc_8u (specSize));
            SpecPtr specPtr = nullptr;

            if (Traits::init (&specPtr, order, flag, hint, specBuf.get(), initBuf.get()) != ippStsNoErr)
                return {};

            return { std::move (specBuf), IppPtr (ippsMalloc_8u (workSize)), specPtr };
        }

        Context() noexcept = default;

        Context (IppPtr&& spec, IppPtr&& work, typename Traits::Spec* ptr) noexcept
            : specBuf (std::move (spec)), workBuf (std::move (work)), specPtr (ptr)
        {}

        b8 isValid() const noexcept { return specPtr != nullptr; }

        IppPtr specBuf, workBuf;
        SpecPtr specPtr = nullptr;
    };

    struct ComplexTraits
    {
        static constexpr auto getSize = ippsFFTGetSize_C_32fc;
        static constexpr auto init = ippsFFTInit_C_32fc;
        using Spec = IppsFFTSpec_C_32fc;
    };

    struct RealTraits
    {
        static constexpr auto getSize = ippsFFTGetSize_R_32f;
        static constexpr auto init = ippsFFTInit_R_32f;
        using Spec = IppsFFTSpec_R_32f;
    };

    IntelPerformancePrimitivesFFT (Context<ComplexTraits>&& complexToUse,
                                   Context<RealTraits>&& realToUse,
                                   i32k orderToUse)
        : cplx (std::move (complexToUse)),
          real (std::move (realToUse)),
          order (orderToUse)
    {}

    Context<ComplexTraits> cplx;
    Context<RealTraits> real;
    i32 order = 0;
};

FFT::EngineImpl<IntelPerformancePrimitivesFFT> intelPerformancePrimitivesFFT;
#endif

//==============================================================================
//==============================================================================
FFT::FFT (i32 order)
    : engine (FFT::Engine::createBestEngineForPlatform (order)),
      size (1 << order)
{
}

FFT::FFT (FFT&&) noexcept = default;

FFT& FFT::operator= (FFT&&) noexcept = default;

FFT::~FFT() = default;

z0 FFT::perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept
{
    if (engine != nullptr)
        engine->perform (input, output, inverse);
}

z0 FFT::performRealOnlyForwardTransform (f32* inputOutputData, b8 ignoreNegativeFreqs) const noexcept
{
    if (engine != nullptr)
        engine->performRealOnlyForwardTransform (inputOutputData, ignoreNegativeFreqs);
}

z0 FFT::performRealOnlyInverseTransform (f32* inputOutputData) const noexcept
{
    if (engine != nullptr)
        engine->performRealOnlyInverseTransform (inputOutputData);
}

z0 FFT::performFrequencyOnlyForwardTransform (f32* inputOutputData, b8 ignoreNegativeFreqs) const noexcept
{
    if (size == 1)
        return;

    performRealOnlyForwardTransform (inputOutputData, ignoreNegativeFreqs);
    auto* out = reinterpret_cast<Complex<f32>*> (inputOutputData);

    const auto limit = ignoreNegativeFreqs ? (size / 2) + 1 : size;

    for (i32 i = 0; i < limit; ++i)
        inputOutputData[i] = std::abs (out[i]);

    zeromem (inputOutputData + limit, static_cast<size_t> (size * 2 - limit) * sizeof (f32));
}

} // namespace drx::dsp
