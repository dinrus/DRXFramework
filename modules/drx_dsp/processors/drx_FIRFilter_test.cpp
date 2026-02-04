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

class FIRFilterTest final : public UnitTest
{
    template <typename Type>
    struct Helpers
    {
        static z0 fillRandom (Random& random, Type* buffer, size_t n)
        {
            for (size_t i = 0; i < n; ++i)
                buffer[i] = (2.0f * random.nextFloat()) - 1.0f;
        }

        static b8 checkArrayIsSimilar (Type* a, Type* b, size_t n) noexcept
        {
            for (size_t i = 0; i < n; ++i)
                if (std::abs (a[i] - b[i]) > 1e-6f)
                    return false;

            return true;
        }
    };

   #if DRX_USE_SIMD
    template <typename Type>
    struct Helpers<SIMDRegister<Type>>
    {
        static z0 fillRandom (Random& random, SIMDRegister<Type>* buffer, size_t n)
        {
            Helpers<Type>::fillRandom (random, reinterpret_cast<Type*> (buffer), n * SIMDRegister<Type>::size());
        }

        static b8 checkArrayIsSimilar (SIMDRegister<Type>* a, SIMDRegister<Type>* b, size_t n) noexcept
        {
            return Helpers<Type>::checkArrayIsSimilar (reinterpret_cast<Type*> (a),
                                                       reinterpret_cast<Type*> (b),
                                                       n * SIMDRegister<Type>::size());
        }
    };
   #endif

    template <typename Type>
    static z0 fillRandom (Random& random, Type* buffer, size_t n) { Helpers<Type>::fillRandom (random, buffer, n); }

    template <typename Type>
    static b8 checkArrayIsSimilar (Type* a, Type* b, size_t n) noexcept { return Helpers<Type>::checkArrayIsSimilar (a, b, n); }

    //==============================================================================
    // reference implementation of an FIR
    template <typename SampleType, typename NumericType>
    static z0 reference (const NumericType* firCoefficients, size_t numCoefficients,
                           const SampleType* input, SampleType* output, size_t n) noexcept
    {
        if (numCoefficients == 0)
        {
            zeromem (output, sizeof (SampleType) * n);
            return;
        }

        HeapBlock<SampleType> scratchBuffer (numCoefficients
                                            #if DRX_USE_SIMD
                                             + (SIMDRegister<NumericType>::SIMDRegisterSize / sizeof (SampleType))
                                            #endif
                                             );
       #if DRX_USE_SIMD
        SampleType* buffer = reinterpret_cast<SampleType*> (SIMDRegister<NumericType>::getNextSIMDAlignedPtr (reinterpret_cast<NumericType*> (scratchBuffer.getData())));
       #else
        SampleType* buffer = scratchBuffer.getData();
       #endif

        zeromem (buffer, sizeof (SampleType) * numCoefficients);

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = (numCoefficients - 1); j >= 1; --j)
                buffer[j] = buffer[j-1];

            buffer[0] = input[i];

            SampleType sum (0);

            for (size_t j = 0; j < numCoefficients; ++j)
                sum += buffer[j] * firCoefficients[j];

            output[i] = sum;
        }
    }

    //==============================================================================
    struct LargeBlockTest
    {
        template <typename FloatType>
        static z0 run (FIR::Filter<FloatType>& filter, FloatType* src, FloatType* dst, size_t n)
        {
            AudioBlock<const FloatType> input (&src, 1, n);
            AudioBlock<FloatType> output (&dst, 1, n);
            ProcessContextNonReplacing<FloatType> context (input, output);

            filter.process (context);
        }
    };

    struct SampleBySampleTest
    {
        template <typename FloatType>
        static z0 run (FIR::Filter<FloatType>& filter, FloatType* src, FloatType* dst, size_t n)
        {
            for (size_t i = 0; i < n; ++i)
                dst[i] = filter.processSample (src[i]);
        }
    };

    struct SplitBlockTest
    {
        template <typename FloatType>
        static z0 run (FIR::Filter<FloatType>& filter, FloatType* input, FloatType* output, size_t n)
        {
            size_t len = 0;
            for (size_t i = 0; i < n; i += len)
            {
                len = jmin (n - i, n / 3);
                auto* src = input + i;
                auto* dst = output + i;

                AudioBlock<const FloatType> inBlock (&src, 1, len);
                AudioBlock<FloatType> outBlock (&dst, 1, len);
                ProcessContextNonReplacing<FloatType> context (inBlock, outBlock);

                filter.process (context);
            }
        }
    };

    //==============================================================================
    template <typename TheTest, typename SampleType, typename NumericType>
    z0 runTestForType()
    {
        Random random (8392829);

        for (auto size : {1, 2, 4, 8, 12, 13, 25})
        {
            constexpr size_t n = 813;

            HeapBlock<t8> inputBuffer, outputBuffer, refBuffer;
            AudioBlock<SampleType> input (inputBuffer, 1, n), output (outputBuffer, 1, n), ref (refBuffer, 1, n);
            fillRandom (random, input.getChannelPointer (0), n);

            HeapBlock<t8> firBlock;
            AudioBlock<NumericType> fir (firBlock, 1, static_cast<size_t> (size));
            fillRandom (random, fir.getChannelPointer (0), static_cast<size_t> (size));

            FIR::Filter<SampleType> filter (*new FIR::Coefficients<NumericType> (fir.getChannelPointer (0), static_cast<size_t> (size)));
            ProcessSpec spec {0.0, n, 1};
            filter.prepare (spec);

            reference<SampleType, NumericType> (fir.getChannelPointer (0), static_cast<size_t> (size),
                                                input.getChannelPointer (0), ref.getChannelPointer (0), n);

            TheTest::template run<SampleType> (filter, input.getChannelPointer (0), output.getChannelPointer (0), n);
            expect (checkArrayIsSimilar (output.getChannelPointer (0), ref.getChannelPointer (0), n));
        }
    }

    template <typename TheTest>
    z0 runTestForAllTypes (tukk unitTestName)
    {
        beginTest (unitTestName);

        runTestForType<TheTest, f32, f32>();
        runTestForType<TheTest, f64, f64>();
       #if DRX_USE_SIMD
        runTestForType<TheTest, SIMDRegister<f32>, f32>();
        runTestForType<TheTest, SIMDRegister<f64>, f64>();
       #endif
    }


public:
    FIRFilterTest()
        : UnitTest ("FIR Filter", UnitTestCategories::dsp)
    {}

    z0 runTest() override
    {
        runTestForAllTypes<LargeBlockTest> ("Large Blocks");
        runTestForAllTypes<SampleBySampleTest> ("Sample by Sample");
        runTestForAllTypes<SplitBlockTest> ("Split Block");
    }
};

static FIRFilterTest firFilterUnitTest;

} // namespace drx::dsp
