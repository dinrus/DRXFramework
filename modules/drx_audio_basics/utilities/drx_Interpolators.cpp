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

namespace drx
{

#if DRX_UNIT_TESTS

class InterpolatorTests final : public UnitTest
{
public:
    InterpolatorTests()
        : UnitTest ("InterpolatorTests", UnitTestCategories::audio)
    {
    }

private:
    template <typename InterpolatorType>
    z0 runInterplatorTests (const Txt& interpolatorName)
    {
        auto createGaussian = [] (std::vector<f32>& destination, f32 scale, f32 centreInSamples, f32 width)
        {
            for (size_t i = 0; i < destination.size(); ++i)
            {
                auto x = (((f32) i) - centreInSamples) * width;
                destination[i] = std::exp (-(x * x));
            }

            FloatVectorOperations::multiply (destination.data(), scale, (i32) destination.size());
        };

        auto findGaussianPeak = [] (const std::vector<f32>& input) -> f32
        {
            auto max = std::max_element (std::begin (input), std::end (input));
            auto maxPrev = max - 1;
            jassert (maxPrev >= std::begin (input));
            auto maxNext = max + 1;
            jassert (maxNext < std::end (input));
            auto quadraticMaxLoc = (*maxPrev - *maxNext) / (2.0f * ((*maxNext + *maxPrev) - (2.0f * *max)));
            return quadraticMaxLoc + (f32) std::distance (std::begin (input), max);
        };

        auto expectAllElementsWithin = [this] (const std::vector<f32>& v1, const std::vector<f32>& v2, f32 tolerance)
        {
            expectEquals ((i32) v1.size(), (i32) v2.size());

            for (size_t i = 0; i < v1.size(); ++i)
                expectWithinAbsoluteError (v1[i], v2[i], tolerance);
        };

        InterpolatorType interpolator;

        constexpr size_t inputSize = 1001;
        static_assert (inputSize > 800 + InterpolatorType::getBaseLatency(),
                       "The test InterpolatorTests input buffer is too small");

        std::vector<f32> input (inputSize);
        constexpr auto inputGaussianMidpoint = (f32) (inputSize - 1) / 2.0f;
        constexpr auto inputGaussianValueAtEnds = 0.000001f;
        const auto inputGaussianWidth = std::sqrt (-std::log (inputGaussianValueAtEnds)) / inputGaussianMidpoint;

        createGaussian (input, 1.0f, inputGaussianMidpoint, inputGaussianWidth);

        for (auto speedRatio : { 0.4, 0.8263, 1.0, 1.05, 1.2384, 1.6 })
        {
            const auto expectedGaussianMidpoint = (inputGaussianMidpoint + InterpolatorType::getBaseLatency()) / (f32) speedRatio;
            const auto expectedGaussianWidth = inputGaussianWidth * (f32) speedRatio;

            const auto outputBufferSize = (size_t) std::floor ((f32) input.size() / speedRatio);

            for (i32 numBlocks : { 1, 5 })
            {
                const auto inputBlockSize = (f32) input.size() / (f32) numBlocks;
                const auto outputBlockSize = (i32) std::floor (inputBlockSize / speedRatio);

                std::vector<f32> output (outputBufferSize, std::numeric_limits<f32>::min());

                beginTest (interpolatorName + " process " + Txt (numBlocks) + " blocks ratio " + Txt (speedRatio));

                interpolator.reset();

                {
                    auto* inputPtr = input.data();
                    auto* outputPtr = output.data();

                    for (i32 i = 0; i < numBlocks; ++i)
                    {
                        auto numInputSamplesRead = interpolator.process (speedRatio, inputPtr, outputPtr, outputBlockSize);
                        inputPtr += numInputSamplesRead;
                        outputPtr += outputBlockSize;
                    }
                }

                expectWithinAbsoluteError (findGaussianPeak (output), expectedGaussianMidpoint, 0.1f);

                std::vector<f32> expectedOutput (output.size());
                createGaussian (expectedOutput, 1.0f, expectedGaussianMidpoint, expectedGaussianWidth);

                expectAllElementsWithin (output, expectedOutput, 0.02f);

                beginTest (interpolatorName + " process adding " + Txt (numBlocks) + " blocks ratio " + Txt (speedRatio));

                interpolator.reset();

                constexpr f32 addingGain = 0.7384f;

                {
                    auto* inputPtr = input.data();
                    auto* outputPtr = output.data();

                    for (i32 i = 0; i < numBlocks; ++i)
                    {
                        auto numInputSamplesRead = interpolator.processAdding (speedRatio, inputPtr, outputPtr, outputBlockSize, addingGain);
                        inputPtr += numInputSamplesRead;
                        outputPtr += outputBlockSize;
                    }
                }

                expectWithinAbsoluteError (findGaussianPeak (output), expectedGaussianMidpoint, 0.1f);

                std::vector<f32> additionalOutput (output.size());
                createGaussian (additionalOutput, addingGain, expectedGaussianMidpoint, expectedGaussianWidth);
                FloatVectorOperations::add (expectedOutput.data(), additionalOutput.data(), (i32) additionalOutput.size());

                expectAllElementsWithin (output, expectedOutput, 0.02f);
            }

            beginTest (interpolatorName + " process wrap 0 ratio " + Txt (speedRatio));

            std::vector<f32> doubleLengthOutput (2 * outputBufferSize, std::numeric_limits<f32>::min());

            interpolator.reset();
            interpolator.process (speedRatio, input.data(), doubleLengthOutput.data(), (i32) doubleLengthOutput.size(),
                                  (i32) input.size(), 0);

            std::vector<f32> expectedDoubleLengthOutput (doubleLengthOutput.size());
            createGaussian (expectedDoubleLengthOutput, 1.0f, expectedGaussianMidpoint, expectedGaussianWidth);

            expectAllElementsWithin (doubleLengthOutput, expectedDoubleLengthOutput, 0.02f);

            beginTest (interpolatorName + " process wrap f64 ratio " + Txt (speedRatio));

            interpolator.reset();
            interpolator.process (speedRatio, input.data(), doubleLengthOutput.data(), (i32) doubleLengthOutput.size(),
                                  (i32) input.size(), (i32) input.size());

            std::vector<f32> secondGaussian (doubleLengthOutput.size());
            createGaussian (secondGaussian, 1.0f, expectedGaussianMidpoint + (f32) outputBufferSize, expectedGaussianWidth);
            FloatVectorOperations::add (expectedDoubleLengthOutput.data(), secondGaussian.data(), (i32) expectedDoubleLengthOutput.size());

            expectAllElementsWithin (doubleLengthOutput, expectedDoubleLengthOutput, 0.02f);
        }
    }

public:
    z0 runTest() override
    {
        runInterplatorTests<WindowedSincInterpolator> ("WindowedSincInterpolator");
        runInterplatorTests<LagrangeInterpolator>     ("LagrangeInterpolator");
        runInterplatorTests<CatmullRomInterpolator>   ("CatmullRomInterpolator");
        runInterplatorTests<LinearInterpolator>       ("LinearInterpolator");
    }
};

static InterpolatorTests interpolatorTests;

#endif

} // namespace drx
