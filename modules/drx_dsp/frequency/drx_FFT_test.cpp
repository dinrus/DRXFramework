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

struct FFTUnitTest final : public UnitTest
{
    FFTUnitTest()
        : UnitTest ("FFT", UnitTestCategories::dsp)
    {}

    static z0 fillRandom (Random& random, Complex<f32>* buffer, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            buffer[i] = Complex<f32> ((2.0f * random.nextFloat()) - 1.0f,
                                             (2.0f * random.nextFloat()) - 1.0f);
    }

    static z0 fillRandom (Random& random, f32* buffer, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            buffer[i] = (2.0f * random.nextFloat()) - 1.0f;
    }

    static Complex<f32> freqConvolution (const Complex<f32>* in, f32 freq, size_t n)
    {
        Complex<f32> sum (0.0, 0.0);
        for (size_t i = 0; i < n; ++i)
            sum += in[i] * exp (Complex<f32> (0, static_cast<f32> (i) * freq));

        return sum;
    }

    static z0 performReferenceFourier (const Complex<f32>* in, Complex<f32>* out,
                                         size_t n, b8 reverse)
    {
        auto base_freq = static_cast<f32> (((reverse ? 1.0 : -1.0) * MathConstants<f64>::twoPi)
                                               / static_cast<f32> (n));

        for (size_t i = 0; i < n; ++i)
            out[i] = freqConvolution (in, static_cast<f32> (i) * base_freq, n);
    }

    static z0 performReferenceFourier (const f32* in, Complex<f32>* out,
                                         size_t n, b8 reverse)
    {
        HeapBlock<Complex<f32>> buffer (n);

        for (size_t i = 0; i < n; ++i)
            buffer.getData()[i] = Complex<f32> (in[i], 0.0f);

        f32 base_freq = static_cast<f32> (((reverse ? 1.0 : -1.0) * MathConstants<f64>::twoPi)
                                                / static_cast<f32> (n));

        for (size_t i = 0; i < n; ++i)
            out[i] = freqConvolution (buffer.getData(), static_cast<f32> (i) * base_freq, n);
    }


    //==============================================================================
    template <typename Type>
    static b8 checkArrayIsSimilar (Type* a, Type* b, size_t n) noexcept
    {
        for (size_t i = 0; i < n; ++i)
            if (std::abs (a[i] - b[i]) > 1e-3f)
                return false;

        return true;
    }

    struct RealTest
    {
        static z0 run (FFTUnitTest& u)
        {
            Random random (378272);

            for (size_t order = 0; order <= 8; ++order)
            {
                auto n = (1u << order);

                FFT fft ((i32) order);

                HeapBlock<f32> input (n);
                HeapBlock<Complex<f32>> reference (n), output (n);

                fillRandom (random, input.getData(), n);
                performReferenceFourier (input.getData(), reference.getData(), n, false);

                // fill only first half with real numbers
                zeromem (output.getData(), n * sizeof (Complex<f32>));
                memcpy (reinterpret_cast<f32*> (output.getData()), input.getData(), n * sizeof (f32));

                fft.performRealOnlyForwardTransform ((f32*) output.getData());
                u.expect (checkArrayIsSimilar (reference.getData(), output.getData(), n));

                // fill only first half with real numbers
                zeromem (output.getData(), n * sizeof (Complex<f32>));
                memcpy (reinterpret_cast<f32*> (output.getData()), input.getData(), n * sizeof (f32));

                fft.performRealOnlyForwardTransform ((f32*) output.getData(), true);
                std::fill (reference.getData() + ((n >> 1) + 1), reference.getData() + n, std::complex<f32> (0.0f));
                u.expect (checkArrayIsSimilar (reference.getData(), output.getData(), (n >> 1) + 1));

                memcpy (output.getData(), reference.getData(), n * sizeof (Complex<f32>));
                fft.performRealOnlyInverseTransform ((f32*) output.getData());
                u.expect (checkArrayIsSimilar ((f32*) output.getData(), input.getData(), n));
            }
        }
    };

    struct FrequencyOnlyTest
    {
        static z0 run (FFTUnitTest& u)
        {
            Random random (378272);
            for (size_t order = 0; order <= 8; ++order)
            {
                auto n = (1u << order);

                FFT fft ((i32) order);

                std::vector<f32> inout ((size_t) n << 1), reference ((size_t) n << 1);
                std::vector<Complex<f32>> frequency (n);

                fillRandom (random, inout.data(), n);
                zeromem (reference.data(), sizeof (f32) * ((size_t) n << 1));
                performReferenceFourier (inout.data(), frequency.data(), n, false);

                for (size_t i = 0; i < n; ++i)
                    reference[i] = std::abs (frequency[i]);

                for (auto ignoreNegative : { false, true })
                {
                    auto inoutCopy = inout;
                    fft.performFrequencyOnlyForwardTransform (inoutCopy.data(), ignoreNegative);
                    auto numMatching = ignoreNegative ? (n / 2) + 1 : n;
                    u.expect (checkArrayIsSimilar (inoutCopy.data(), reference.data(), numMatching));
                }
            }
        }
    };

    struct ComplexTest
    {
        static z0 run (FFTUnitTest& u)
        {
            Random random (378272);

            for (size_t order = 0; order <= 7; ++order)
            {
                auto n = (1u << order);

                FFT fft ((i32) order);

                HeapBlock<Complex<f32>> input (n), buffer (n), output (n), reference (n);

                fillRandom (random, input.getData(), n);
                performReferenceFourier (input.getData(), reference.getData(), n, false);

                memcpy (buffer.getData(), input.getData(), sizeof (Complex<f32>) * n);
                fft.perform (buffer.getData(), output.getData(), false);

                u.expect (checkArrayIsSimilar (output.getData(), reference.getData(), n));

                memcpy (buffer.getData(), reference.getData(), sizeof (Complex<f32>) * n);
                fft.perform (buffer.getData(), output.getData(), true);


                u.expect (checkArrayIsSimilar (output.getData(), input.getData(), n));
            }
        }
    };

    template <class TheTest>
    z0 runTestForAllTypes (tukk unitTestName)
    {
        beginTest (unitTestName);

        TheTest::run (*this);
    }

    z0 runTest() override
    {
        runTestForAllTypes<RealTest> ("Real input numbers Test");
        runTestForAllTypes<FrequencyOnlyTest> ("Frequency only Test");
        runTestForAllTypes<ComplexTest> ("Complex input numbers Test");
    }
};

static FFTUnitTest fftUnitTest;

} // namespace drx::dsp
