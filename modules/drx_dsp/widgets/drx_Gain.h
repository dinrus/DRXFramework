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
    Applies a gain to audio samples as single samples or AudioBlocks.

    @tags{DSP}
*/
template <typename FloatType>
class Gain
{
public:
    Gain() noexcept = default;

    //==============================================================================
    /** Applies a new gain as a linear value. */
    z0 setGainLinear (FloatType newGain) noexcept             { gain.setTargetValue (newGain); }

    /** Applies a new gain as a decibel value. */
    z0 setGainDecibels (FloatType newGainDecibels) noexcept   { setGainLinear (Decibels::decibelsToGain<FloatType> (newGainDecibels)); }

    /** Returns the current gain as a linear value. */
    FloatType getGainLinear() const noexcept                    { return gain.getTargetValue(); }

    /** Returns the current gain in decibels. */
    FloatType getGainDecibels() const noexcept                  { return Decibels::gainToDecibels<FloatType> (getGainLinear()); }

    /** Sets the length of the ramp used for smoothing gain changes. */
    z0 setRampDurationSeconds (f64 newDurationSeconds) noexcept
    {
        if (! approximatelyEqual (rampDurationSeconds, newDurationSeconds))
        {
            rampDurationSeconds = newDurationSeconds;
            reset();
        }
    }

    /** Returns the ramp duration in seconds. */
    f64 getRampDurationSeconds() const noexcept              { return rampDurationSeconds; }

    /** Возвращает true, если the current value is currently being interpolated. */
    b8 isSmoothing() const noexcept                           { return gain.isSmoothing(); }

    //==============================================================================
    /** Called before processing starts. */
    z0 prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    /** Resets the internal state of the gain */
    z0 reset() noexcept
    {
        if (sampleRate > 0)
            gain.reset (sampleRate, rampDurationSeconds);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    template <typename SampleType>
    SampleType DRX_VECTOR_CALLTYPE processSample (SampleType s) noexcept
    {
        return s * gain.getNextValue();
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        auto&& inBlock  = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();

        jassert (inBlock.getNumChannels() == outBlock.getNumChannels());
        jassert (inBlock.getNumSamples() == outBlock.getNumSamples());

        auto len         = inBlock.getNumSamples();
        auto numChannels = inBlock.getNumChannels();

        if (context.isBypassed)
        {
            gain.skip (static_cast<i32> (len));

            if (context.usesSeparateInputAndOutputBlocks())
                outBlock.copyFrom (inBlock);

            return;
        }

        if (numChannels == 1)
        {
            auto* src = inBlock.getChannelPointer (0);
            auto* dst = outBlock.getChannelPointer (0);

            for (size_t i = 0; i < len; ++i)
                dst[i] = src[i] * gain.getNextValue();
        }
        else
        {
            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255 6386)
            auto* gains = static_cast<FloatType*> (alloca (sizeof (FloatType) * len));

            for (size_t i = 0; i < len; ++i)
                gains[i] = gain.getNextValue();
            DRX_END_IGNORE_WARNINGS_MSVC

            for (size_t chan = 0; chan < numChannels; ++chan)
                FloatVectorOperations::multiply (outBlock.getChannelPointer (chan),
                                                 inBlock.getChannelPointer (chan),
                                                 gains, static_cast<i32> (len));
        }
    }

private:
    //==============================================================================
    SmoothedValue<FloatType> gain;
    f64 sampleRate = 0, rampDurationSeconds = 0;
};

} // namespace drx::dsp
