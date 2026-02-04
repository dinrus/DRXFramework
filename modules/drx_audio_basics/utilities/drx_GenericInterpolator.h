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

/**
    An interpolator base class for resampling streams of floats.

    Note that the resamplers are stateful, so when there's a break in the continuity
    of the input stream you're feeding it, you should call reset() before feeding
    it any new data. And like with any other stateful filter, if you're resampling
    multiple channels, make sure each one uses its own interpolator object.

    @see LagrangeInterpolator, CatmullRomInterpolator, WindowedSincInterpolator,
         LinearInterpolator, ZeroOrderHoldInterpolator

    @tags{Audio}
*/
template <class InterpolatorTraits, i32 memorySize>
class DRX_API  GenericInterpolator
{
    static auto processReplacingCallback()
    {
        return [] (auto, auto newValue) { return newValue; };
    }

    static auto processAddingCallback (f32 gain)
    {
        return [gain] (auto oldValue, auto newValue) { return oldValue + gain * newValue; };
    }

public:
    GenericInterpolator() noexcept                        { reset(); }

    GenericInterpolator (GenericInterpolator&&) noexcept = default;
    GenericInterpolator& operator= (GenericInterpolator&&) noexcept = default;

    /** Returns the latency of the interpolation algorithm in isolation.

        In the context of resampling the total latency of a process using
        the interpolator is the base latency divided by the speed ratio.
    */
    static constexpr f32 getBaseLatency() noexcept
    {
        return InterpolatorTraits::algorithmicLatency;
    }

    /** Resets the state of the interpolator.

        Call this when there's a break in the continuity of the input data stream.
    */
    z0 reset() noexcept
    {
        indexBuffer = 0;
        subSamplePos = 1.0;
        std::fill (std::begin (lastInputSamples), std::end (lastInputSamples), 0.0f);
    }

    /** Resamples a stream of samples.

        @param speedRatio                   the number of input samples to use for each output sample
        @param inputSamples                 the source data to read from. This must contain at
                                            least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples                the buffer to write the results into
        @param numOutputSamplesToProduce    the number of output samples that should be created

        @returns the actual number of input samples that were used
    */
    i32 process (f64 speedRatio,
                 const f32* inputSamples,
                 f32* outputSamples,
                 i32 numOutputSamplesToProduce) noexcept
    {
        return interpolateImpl (speedRatio,
                                inputSamples,
                                outputSamples,
                                numOutputSamplesToProduce,
                                processReplacingCallback());
    }

    /** Resamples a stream of samples.

        @param speedRatio                   the number of input samples to use for each output sample
        @param inputSamples                 the source data to read from. This must contain at
                                            least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples                the buffer to write the results into
        @param numOutputSamplesToProduce    the number of output samples that should be created
        @param numInputSamplesAvailable     the number of available input samples. If it needs more samples
                                            than available, it either wraps back for wrapAround samples, or
                                            it feeds zeroes
        @param wrapAround                   if the stream exceeds available samples, it wraps back for
                                            wrapAround samples. If wrapAround is set to 0, it will feed zeroes.

        @returns the actual number of input samples that were used
    */
    i32 process (f64 speedRatio,
                 const f32* inputSamples,
                 f32* outputSamples,
                 i32 numOutputSamplesToProduce,
                 i32 numInputSamplesAvailable,
                 i32 wrapAround) noexcept
    {
        return interpolateImpl (speedRatio,
                                inputSamples,
                                outputSamples,
                                numOutputSamplesToProduce,
                                numInputSamplesAvailable,
                                wrapAround,
                                processReplacingCallback());
    }

    /** Resamples a stream of samples, adding the results to the output data
        with a gain.

        @param speedRatio                   the number of input samples to use for each output sample
        @param inputSamples                 the source data to read from. This must contain at
                                            least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples                the buffer to write the results to - the result values will be added
                                            to any pre-existing data in this buffer after being multiplied by
                                            the gain factor
        @param numOutputSamplesToProduce    the number of output samples that should be created
        @param gain                         a gain factor to multiply the resulting samples by before
                                            adding them to the destination buffer

        @returns the actual number of input samples that were used
    */
    i32 processAdding (f64 speedRatio,
                       const f32* inputSamples,
                       f32* outputSamples,
                       i32 numOutputSamplesToProduce,
                       f32 gain) noexcept
    {
        return interpolateImpl (speedRatio,
                                inputSamples,
                                outputSamples,
                                numOutputSamplesToProduce,
                                processAddingCallback (gain));
    }

    /** Resamples a stream of samples, adding the results to the output data
        with a gain.

        @param speedRatio                   the number of input samples to use for each output sample
        @param inputSamples                 the source data to read from. This must contain at
                                            least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples                the buffer to write the results to - the result values will be added
                                            to any pre-existing data in this buffer after being multiplied by
                                            the gain factor
        @param numOutputSamplesToProduce    the number of output samples that should be created
        @param numInputSamplesAvailable     the number of available input samples. If it needs more samples
                                            than available, it either wraps back for wrapAround samples, or
                                            it feeds zeroes
        @param wrapAround                   if the stream exceeds available samples, it wraps back for
                                            wrapAround samples. If wrapAround is set to 0, it will feed zeroes.
        @param gain                         a gain factor to multiply the resulting samples by before
                                            adding them to the destination buffer

        @returns the actual number of input samples that were used
    */
    i32 processAdding (f64 speedRatio,
                       const f32* inputSamples,
                       f32* outputSamples,
                       i32 numOutputSamplesToProduce,
                       i32 numInputSamplesAvailable,
                       i32 wrapAround,
                       f32 gain) noexcept
    {
        return interpolateImpl (speedRatio,
                                inputSamples,
                                outputSamples,
                                numOutputSamplesToProduce,
                                numInputSamplesAvailable,
                                wrapAround,
                                processAddingCallback (gain));
    }

private:
    //==============================================================================
    forcedinline z0 pushInterpolationSample (f32 newValue) noexcept
    {
        lastInputSamples[indexBuffer] = newValue;

        if (++indexBuffer == memorySize)
            indexBuffer = 0;
    }

    forcedinline z0 pushInterpolationSamples (const f32* input,
                                                i32 numOutputSamplesToProduce) noexcept
    {
        if (numOutputSamplesToProduce >= memorySize)
        {
            const auto* const offsetInput = input + (numOutputSamplesToProduce - memorySize);

            for (i32 i = 0; i < memorySize; ++i)
                pushInterpolationSample (offsetInput[i]);
        }
        else
        {
            for (i32 i = 0; i < numOutputSamplesToProduce; ++i)
                pushInterpolationSample (input[i]);
        }
    }

    forcedinline z0 pushInterpolationSamples (const f32* input,
                                                i32 numOutputSamplesToProduce,
                                                i32 numInputSamplesAvailable,
                                                i32 wrapAround) noexcept
    {
        if (numOutputSamplesToProduce >= memorySize)
        {
            if (numInputSamplesAvailable >= memorySize)
            {
                pushInterpolationSamples (input,
                                          numOutputSamplesToProduce);
            }
            else
            {
                pushInterpolationSamples (input + ((numOutputSamplesToProduce - numInputSamplesAvailable) - 1),
                                          numInputSamplesAvailable);

                if (wrapAround > 0)
                {
                    numOutputSamplesToProduce -= wrapAround;

                    pushInterpolationSamples (input + ((numOutputSamplesToProduce - (memorySize - numInputSamplesAvailable)) - 1),
                                              memorySize - numInputSamplesAvailable);
                }
                else
                {
                    for (i32 i = numInputSamplesAvailable; i < memorySize; ++i)
                        pushInterpolationSample (0.0f);
                }
            }
        }
        else
        {
            if (numOutputSamplesToProduce > numInputSamplesAvailable)
            {
                for (i32 i = 0; i < numInputSamplesAvailable; ++i)
                    pushInterpolationSample (input[i]);

                const auto extraSamples = numOutputSamplesToProduce - numInputSamplesAvailable;

                if (wrapAround > 0)
                {
                    const auto* const offsetInput = input + (numInputSamplesAvailable - wrapAround);

                    for (i32 i = 0; i < extraSamples; ++i)
                        pushInterpolationSample (offsetInput[i]);
                }
                else
                {
                    for (i32 i = 0; i < extraSamples; ++i)
                        pushInterpolationSample (0.0f);
                }
            }
            else
            {
                for (i32 i = 0; i < numOutputSamplesToProduce; ++i)
                    pushInterpolationSample (input[i]);
            }
        }
    }

    //==============================================================================
    template <typename Process>
    i32 interpolateImpl (f64 speedRatio,
                         const f32* input,
                         f32* output,
                         i32 numOutputSamplesToProduce,
                         i32 numInputSamplesAvailable,
                         i32 wrap,
                         Process process)
    {
        auto originalIn = input;
        b8 exceeded = false;

        const auto pushSample = [&]
        {
            if (exceeded)
            {
                pushInterpolationSample (0.0);
            }
            else
            {
                pushInterpolationSample (*input++);

                if (--numInputSamplesAvailable <= 0)
                {
                    if (wrap > 0)
                    {
                        input -= wrap;
                        numInputSamplesAvailable += wrap;
                    }
                    else
                    {
                        exceeded = true;
                    }
                }
            }
        };

        interpolateImpl (speedRatio,
                         output,
                         numOutputSamplesToProduce,
                         process,
                         pushSample);

        if (wrap == 0)
            return (i32) (input - originalIn);

        return ((i32) (input - originalIn) + wrap) % wrap;
    }

    template <typename Process>
    i32 interpolateImpl (f64 speedRatio,
                         const f32* input,
                         f32* output,
                         i32 numOutputSamplesToProduce,
                         Process process)
    {
        i32 numUsed = 0;

        interpolateImpl (speedRatio,
                         output,
                         numOutputSamplesToProduce,
                         process,
                         [this, input, &numUsed] { pushInterpolationSample (input[numUsed++]); });

        return numUsed;
    }

    template <typename Process, typename PushSample>
    z0 interpolateImpl (f64 speedRatio,
                          f32* output,
                          i32 numOutputSamplesToProduce,
                          Process process,
                          PushSample pushSample)
    {
        auto pos = subSamplePos;

        for (auto i = 0; i < numOutputSamplesToProduce; ++i)
        {
            while (pos >= 1.0)
            {
                pushSample();
                pos -= 1.0;
            }

            *output = process (*output, InterpolatorTraits::valueAtOffset (lastInputSamples, (f32) pos, indexBuffer));
            ++output;
            pos += speedRatio;
        }

        subSamplePos = pos;
    }

    //==============================================================================
    f32 lastInputSamples[(size_t) memorySize];
    f64 subSamplePos = 1.0;
    i32 indexBuffer = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericInterpolator)
};

} // namespace drx
