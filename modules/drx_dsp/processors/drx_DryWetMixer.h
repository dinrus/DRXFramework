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

enum class DryWetMixingRule
{
    linear,          // dry volume is equal to 1 - wet volume
    balanced,        // both dry and wet are 1 when mix is 0.5, with dry decreasing to 0
                     // above this value and wet decreasing to 0 below it
    sin3dB,          // alternate dry/wet mixing rule using the 3 dB sine panning rule
    sin4p5dB,        // alternate dry/wet mixing rule using the 4.5 dB sine panning rule
    sin6dB,          // alternate dry/wet mixing rule using the 6 dB sine panning rule
    squareRoot3dB,   // alternate dry/wet mixing rule using the regular 3 dB panning rule
    squareRoot4p5dB  // alternate dry/wet mixing rule using the regular 4.5 dB panning rule
};

/**
    A processor to handle dry/wet mixing of two audio signals, where the wet signal
    may have additional latency.

    Once a DryWetMixer object is configured, push the dry samples using pushDrySamples
    and mix into the fully wet samples using mixWetSamples.

    @tags{DSP}
*/
template <typename SampleType>
class DryWetMixer
{
public:
    //==============================================================================
    using MixingRule = DryWetMixingRule;

    //==============================================================================
    /** Default constructor. */
    DryWetMixer();

    /** Constructor. */
    explicit DryWetMixer (i32 maximumWetLatencyInSamples);

    //==============================================================================
    /** Sets the mix rule. */
    z0 setMixingRule (MixingRule newRule);

    /** Sets the current dry/wet mix proportion, with 0.0 being full dry and 1.0
        being fully wet.
    */
    z0 setWetMixProportion (SampleType newWetMixProportion);

    /** Sets the relative latency of the wet signal path compared to the dry signal
        path, and thus the amount of latency compensation that will be added to the
        dry samples in this processor.
    */
    z0 setWetLatency (SampleType wetLatencyInSamples);

    //==============================================================================
    /** Initialises the processor. */
    z0 prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    z0 reset();

    //==============================================================================
    /** Copies the dry path samples into an internal delay line. */
    z0 pushDrySamples (const AudioBlock<const SampleType> drySamples);

    /** Mixes the supplied wet samples with the latency-compensated dry samples from
        pushDrySamples.

        @param wetSamples    Input:  The AudioBlock references fully wet samples.
                             Output: The AudioBlock references the wet samples mixed
                                     with the latency compensated dry samples.

        @see pushDrySamples
    */
    z0 mixWetSamples (AudioBlock<SampleType> wetSamples);

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> dryVolume, wetVolume;
    DelayLine<SampleType, DelayLineInterpolationTypes::Thiran> dryDelayLine;
    AudioBuffer<SampleType> bufferDry;

    SingleThreadedAbstractFifo fifo;
    SampleType mix = 1.0;
    MixingRule currentMixingRule = MixingRule::linear;
    f64 sampleRate = 44100.0;
    i32 maximumWetLatencyInSamples = 0;
};

} // namespace drx::dsp
