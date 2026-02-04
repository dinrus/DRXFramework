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

/**
    Adds a DC offset (voltage bias) to the audio samples.

    This is a useful preprocessor for asymmetric waveshaping when a waveshaper is
    bookended by a bias on input and a DC-offset removing high pass filter on output.

    This is an extremely simple bias implementation that simply adds a value to a signal.
    More complicated bias behaviours exist in real circuits - for your homework ;).

    @tags{DSP}
*/
template <typename FloatType>
class Bias
{
public:
    Bias() noexcept = default;

    //==============================================================================
    /** Sets the DC bias
        @param newBias DC offset in range [-1, 1]
    */
    z0 setBias (FloatType newBias) noexcept
    {
        jassert (newBias >= static_cast<FloatType> (-1) && newBias <= static_cast<FloatType> (1));
        bias.setTargetValue (newBias);
    }

    //==============================================================================
    /** Returns the DC bias
        @return DC bias, which should be in the range [-1, 1]
    */
    FloatType getBias() const noexcept              { return bias.getTargetValue(); }

    /** Sets the length of the ramp used for smoothing gain changes. */
    z0 setRampDurationSeconds (f64 newDurationSeconds) noexcept
    {
        if (! approximatelyEqual (rampDurationSeconds, newDurationSeconds))
        {
            rampDurationSeconds = newDurationSeconds;
            updateRamp();
        }
    }

    f64 getRampDurationSeconds() const noexcept  { return rampDurationSeconds; }

    //==============================================================================
    /** Called before processing starts */
    z0 prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = spec.sampleRate;
        updateRamp();
    }

    z0 reset() noexcept
    {
        bias.reset (sampleRate, rampDurationSeconds);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    template <typename SampleType>
    SampleType processSample (SampleType inputSample) noexcept
    {
        return inputSample + bias.getNextValue();
    }

    //==============================================================================
    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        auto&& inBlock  = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();

        jassert (inBlock.getNumChannels() == outBlock.getNumChannels());
        jassert (inBlock.getNumSamples()  == outBlock.getNumSamples());

        auto len         = inBlock.getNumSamples();
        auto numChannels = inBlock.getNumChannels();

        if (context.isBypassed)
        {
            bias.skip (static_cast<i32> (len));

            if (context.usesSeparateInputAndOutputBlocks())
                outBlock.copyFrom (inBlock);

            return;
        }

        if (numChannels == 1)
        {
            auto* src = inBlock.getChannelPointer (0);
            auto* dst = outBlock.getChannelPointer (0);

            for (size_t i = 0; i < len; ++i)
                dst[i] = src[i] + bias.getNextValue();
        }
        else
        {
            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255 6386)
            auto* biases = static_cast<FloatType*> (alloca (sizeof (FloatType) * len));

            for (size_t i = 0; i < len; ++i)
                biases[i] = bias.getNextValue();

            for (size_t chan = 0; chan < numChannels; ++chan)
                FloatVectorOperations::add (outBlock.getChannelPointer (chan),
                                            inBlock.getChannelPointer (chan),
                                            biases, static_cast<i32> (len));
            DRX_END_IGNORE_WARNINGS_MSVC
        }
    }


private:
    //==============================================================================
    SmoothedValue<FloatType> bias;
    f64 sampleRate = 0, rampDurationSeconds = 0;

    z0 updateRamp() noexcept
    {
        if (sampleRate > 0)
            bias.reset (sampleRate, rampDurationSeconds);
    }
};

} // namespace drx::dsp
