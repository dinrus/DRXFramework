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
    A simple compressor with standard threshold, ratio, attack time and release time
    controls.

    @tags{DSP}
*/
template <typename SampleType>
class Compressor
{
public:
    //==============================================================================
    /** Constructor. */
    Compressor();

    //==============================================================================
    /** Sets the threshold in dB of the compressor.*/
    z0 setThreshold (SampleType newThreshold);

    /** Sets the ratio of the compressor (must be higher or equal to 1).*/
    z0 setRatio (SampleType newRatio);

    /** Sets the attack time in milliseconds of the compressor.*/
    z0 setAttack (SampleType newAttack);

    /** Sets the release time in milliseconds of the compressor.*/
    z0 setRelease (SampleType newRelease);

    //==============================================================================
    /** Initialises the processor. */
    z0 prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    z0 reset();

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((i32) channel, inputSamples[i]);
        }
    }

    /** Performs the processing operation on a single sample at a time. */
    SampleType processSample (i32 channel, SampleType inputValue);

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    SampleType threshold, thresholdInverse, ratioInverse;
    BallisticsFilter<SampleType> envelopeFilter;

    f64 sampleRate = 44100.0;
    SampleType thresholddB = 0.0, ratio = 1.0, attackTime = 1.0, releaseTime = 100.0;
};

} // namespace drx::dsp
