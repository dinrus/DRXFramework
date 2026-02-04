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

enum class StateVariableTPTFilterType
{
    lowpass,
    bandpass,
    highpass
};

//==============================================================================
/** An IIR filter that can perform low, band and high-pass filtering on an audio
    signal, with 12 dB of attenuation per octave, using a TPT structure, designed
    for fast modulation (see Vadim Zavalishin's documentation about TPT
    structures for more information). Its behaviour is based on the analog
    state variable filter circuit.

    Note: The bandpass here is not the one in the RBJ CookBook as its gain can be
    higher than 0 dB. For the classic 0 dB bandpass, we need to multiply the
    result by R2.

    Note 2: Using this class prevents some loud audio artefacts commonly encountered when
    changing the cutoff frequency using other filter simulation structures and IIR
    filter classes. However, this class may still require additional smoothing for
    cutoff frequency changes.

    see IIRFilter, SmoothedValue

    @tags{DSP}
*/
template <typename SampleType>
class StateVariableTPTFilter
{
public:
    //==============================================================================
    using Type = StateVariableTPTFilterType;

    //==============================================================================
    /** Constructor. */
    StateVariableTPTFilter();

    //==============================================================================
    /** Sets the filter type. */
    z0 setType (Type newType);

    /** Sets the cutoff frequency of the filter.

        @param newFrequencyHz the new cutoff frequency in Hz.
    */
    z0 setCutoffFrequency (SampleType newFrequencyHz);

    /** Sets the resonance of the filter.

        Note: The bandwidth of the resonance increases with the value of the
        parameter. To have a standard 12 dB / octave filter, the value must be set
        at 1 / sqrt (2).
    */
    z0 setResonance (SampleType newResonance);

    //==============================================================================
    /** Returns the type of the filter. */
    Type getType() const noexcept                      { return filterType; }

    /** Returns the cutoff frequency of the filter. */
    SampleType getCutoffFrequency() const noexcept     { return cutoffFrequency; }

    /** Returns the resonance of the filter. */
    SampleType getResonance() const noexcept           { return resonance; }

    //==============================================================================
    /** Initialises the filter. */
    z0 prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the filter. */
    z0 reset();

    /** Resets the internal state variables of the filter to a given value. */
    z0 reset (SampleType newValue);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    z0 snapToZero() noexcept;

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

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    SampleType g, h, R2;
    std::vector<SampleType> s1 { 2 }, s2 { 2 };

    f64 sampleRate = 44100.0;
    Type filterType = Type::lowpass;
    SampleType cutoffFrequency = static_cast<SampleType> (1000.0),
               resonance       = static_cast<SampleType> (1.0 / std::sqrt (2.0));
};

} // namespace drx::dsp
