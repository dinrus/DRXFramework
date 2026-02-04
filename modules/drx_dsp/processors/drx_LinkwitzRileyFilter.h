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

enum class LinkwitzRileyFilterType
{
    lowpass,
    highpass,
    allpass
};

/**
    A filter class designed to perform multi-band separation using the TPT
    (Topology-Preserving Transform) structure.

    Linkwitz-Riley filters are widely used in audio crossovers that have two outputs,
    a low-pass and a high-pass, such that their sum is equivalent to an all-pass filter
    with a flat magnitude frequency response. The Linkwitz-Riley filters available in
    this class are designed to have a -24 dB/octave slope (LR 4th order).

    @tags{DSP}
*/
template <typename SampleType>
class LinkwitzRileyFilter
{
public:
    //==============================================================================
    using Type = LinkwitzRileyFilterType;

    //==============================================================================
    /** Constructor. */
    LinkwitzRileyFilter();

    //==============================================================================
    /** Sets the filter type. */
    z0 setType (Type newType);

    /** Sets the cutoff frequency of the filter in Hz. */
    z0 setCutoffFrequency (SampleType newCutoffFrequencyHz);

    //==============================================================================
    /** Returns the type of the filter. */
    Type getType() const noexcept                      { return filterType; }

    /** Returns the cutoff frequency of the filter. */
    SampleType getCutoffFrequency() const noexcept     { return cutoffFrequency; }

    //==============================================================================
    /** Initialises the filter. */
    z0 prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the filter. */
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

        jassert (inputBlock.getNumChannels() <= s1.size());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((i32) channel, inputSamples[i]);
        }

       #if DRX_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero();
       #endif
    }

    /** Performs the filter operation on a single sample at a time. */
    SampleType processSample (i32 channel, SampleType inputValue);

    /** Performs the filter operation on a single sample at a time, and returns both
        the low-pass and the high-pass outputs of the TPT structure.
    */
    z0 processSample (i32 channel, SampleType inputValue, SampleType &outputLow, SampleType &outputHigh);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    z0 snapToZero() noexcept;

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    SampleType g, R2, h;
    std::vector<SampleType> s1, s2, s3, s4;

    f64 sampleRate = 44100.0;
    SampleType cutoffFrequency = 2000.0;
    Type filterType = Type::lowpass;
};

} // namespace drx::dsp
