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
    A simple limiter with standard threshold and release time controls, featuring
    two compressors and a hard clipper at 0 dB.

    @tags{DSP}
*/
template <typename SampleType>
class Limiter
{
public:
    //==============================================================================
    /** Constructor. */
    Limiter() = default;

    //==============================================================================
    /** Sets the threshold in dB of the limiter.*/
    z0 setThreshold (SampleType newThreshold);

    /** Sets the release time in milliseconds of the limiter.*/
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

        firstStageCompressor.process (context);

        auto secondContext = ProcessContextReplacing<SampleType> (outputBlock);
        secondStageCompressor.process (secondContext);

        outputBlock.multiplyBy (outputVolume);

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            FloatVectorOperations::clip (outputBlock.getChannelPointer (channel), outputBlock.getChannelPointer (channel),
                                         (SampleType) -1.0, (SampleType) 1.0, (i32) numSamples);
        }
    }

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    Compressor<SampleType> firstStageCompressor, secondStageCompressor;
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> outputVolume;

    f64 sampleRate = 44100.0;
    SampleType thresholddB = -10.0, releaseTime = 100.0;
};

} // namespace drx::dsp
