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

enum class FirstOrderTPTFilterType
{
    lowpass,
    highpass,
    allpass
};

//==============================================================================
/**
    A first order filter class using the TPT (Topology-Preserving Transform) structure.

    This filter can be modulated at high rates without producing audio artefacts. See
    Vadim Zavalishin's documentation about TPT structures for more information.

    Note: Using this class prevents some loud audio artefacts commonly encountered when
    changing the cutoff frequency using of other filter simulation structures and IIR
    filter classes. However, this class may still require additional smoothing for
    cutoff frequency changes.

    see StateVariableFilter, IIRFilter, SmoothedValue

    @tags{DSP}
*/
template <typename SampleType>
class FirstOrderTPTFilter
{
public:
    //==============================================================================
    using Type = FirstOrderTPTFilterType;

    //==============================================================================
    /** Constructor. */
    FirstOrderTPTFilter();

    //==============================================================================
    /** Sets the filter type. */
    z0 setType (Type newType);

    /** Sets the cutoff frequency of the filter.

        @param newFrequencyHz cutoff frequency in Hz.
    */
    z0 setCutoffFrequency (SampleType newFrequencyHz);

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

    /** Resets the internal state variables of the filter to a given value. */
    z0 reset (SampleType newValue);

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
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((i32) channel, inputSamples[i]);
        }

       #if DRX_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero();
       #endif
    }

    //==============================================================================
    /** Processes one sample at a time on a given channel. */
    SampleType processSample (i32 channel, SampleType inputValue);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    z0 snapToZero() noexcept;

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    SampleType G = 0;
    std::vector<SampleType> s1 { 2 };
    f64 sampleRate = 44100.0;

    //==============================================================================
    Type filterType = Type::lowpass;
    SampleType cutoffFrequency = 1000.0;
};

} // namespace drx::dsp
