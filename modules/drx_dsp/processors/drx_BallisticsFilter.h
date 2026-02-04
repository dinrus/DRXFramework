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

enum class BallisticsFilterLevelCalculationType
{
    peak,
    RMS
};

/**
    A processor to apply standard attack / release ballistics to an input signal.
    This is useful in dynamics processors, envelope followers, modulated audio
    effects and for smoothing animation in data visualisation.

    @tags{DSP}
*/
template <typename SampleType>
class BallisticsFilter
{
public:
    //==============================================================================
    using LevelCalculationType = BallisticsFilterLevelCalculationType;

    //==============================================================================
    /** Constructor. */
    BallisticsFilter();

    //==============================================================================
    /** Sets the attack time in ms.

        Attack times less than 0.001 ms will be snapped to zero and very i64 attack
        times will eventually saturate depending on the numerical precision used.
    */
    z0 setAttackTime (SampleType attackTimeMs);

    /** Sets the release time in ms.

        Release times less than 0.001 ms will be snapped to zero and very i64
        release times will eventually saturate depending on the numerical precision
        used.
    */
    z0 setReleaseTime (SampleType releaseTimeMs);

    /** Sets how the filter levels are calculated.

        Level calculation in digital envelope followers is usually performed using
        peak detection with a rectifier function (like std::abs) and filtering,
        which returns an envelope dependant on the peak or maximum values of the
        signal amplitude.

        To perform an estimation of the average value of the signal you can use
        an RMS (root mean squared) implementation of the ballistics filter instead.
        This is useful in some compressor and noise-gate designs, or in specific
        types of volume meters.
    */
    z0 setLevelCalculationType (LevelCalculationType newCalculationType);

    //==============================================================================
    /** Initialises the filter. */
    z0 prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the filter. */
    z0 reset();

    /** Resets the internal state variables of the filter to the given initial value. */
    z0 reset (SampleType initialValue);

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= yold.size());
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

       #if DRX_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero();
       #endif
    }

    /** Processes one sample at a time on a given channel. */
    SampleType processSample (i32 channel, SampleType inputValue);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    z0 snapToZero() noexcept;

private:
    //==============================================================================
    SampleType calculateLimitedCte (SampleType) const noexcept;

    //==============================================================================
    std::vector<SampleType> yold;
    f64 sampleRate = 44100.0, expFactor = -0.142;
    SampleType attackTime = 1.0, releaseTime = 100.0, cteAT = 0.0, cteRL = 0.0;
    LevelCalculationType levelType = LevelCalculationType::peak;
};

} // namespace drx::dsp
