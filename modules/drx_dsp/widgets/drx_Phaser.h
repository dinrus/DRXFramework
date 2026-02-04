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
    A 6 stage phaser that modulates first order all-pass filters to create sweeping
    notches in the magnitude frequency response.

    This audio effect can be controlled with standard phaser parameters: the speed
    and depth of the LFO controlling the frequency response, a mix control, a
    feedback control, and the centre frequency of the modulation.

    @tags{DSP}
*/
template <typename SampleType>
class Phaser
{
public:
    //==============================================================================
    /** Constructor. */
    Phaser();

    //==============================================================================
    /** Sets the rate (in Hz) of the LFO modulating the phaser all-pass filters. This
        rate must be lower than 100 Hz.
    */
    z0 setRate (SampleType newRateHz);

    /** Sets the volume (between 0 and 1) of the LFO modulating the phaser all-pass
        filters.
    */
    z0 setDepth (SampleType newDepth);

    /** Sets the centre frequency (in Hz) of the phaser all-pass filters modulation.
    */
    z0 setCentreFrequency (SampleType newCentreHz);

    /** Sets the feedback volume (between -1 and 1) of the phaser. Negative can be
        used to get specific phaser sounds.
    */
    z0 setFeedback (SampleType newFeedback);

    /** Sets the amount of dry and wet signal in the output of the phaser (between 0
        for full dry and 1 for full wet).
    */
    z0 setMix (SampleType newMix);

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
        jassert (inputBlock.getNumChannels() == lastOutput.size());
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        i32 numSamplesDown = 0;
        auto counter = updateCounter;

        for (size_t i = 0; i < numSamples; ++i)
        {
            if (counter == 0)
                numSamplesDown++;

            counter++;

            if (counter == maxUpdateCounter)
                counter = 0;
        }

        if (numSamplesDown > 0)
        {
            auto freqBlock = AudioBlock<SampleType> (bufferFrequency).getSubBlock (0, (size_t) numSamplesDown);
            auto contextFreq = ProcessContextReplacing<SampleType> (freqBlock);
            freqBlock.clear();

            osc.process (contextFreq);
            freqBlock.multiplyBy (oscVolume);
        }

        auto* freqSamples = bufferFrequency.getWritePointer (0);

        for (i32 i = 0; i < numSamplesDown; ++i)
        {
            auto lfo = jlimit (static_cast<SampleType> (0.0),
                               static_cast<SampleType> (1.0),
                               freqSamples[i] + normCentreFrequency);

            freqSamples[i] = mapToLog10 (lfo, static_cast<SampleType> (20.0),
                                         static_cast<SampleType> (jmin (20000.0, 0.49 * sampleRate)));
        }

        auto currentFrequency = filters[0]->getCutoffFrequency();
        dryWet.pushDrySamples (inputBlock);

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            counter = updateCounter;
            i32 k = 0;

            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto input = inputSamples[i];
                auto output = input - lastOutput[channel];

                if (i == 0 && counter != 0)
                    for (i32 n = 0; n < numStages; ++n)
                        filters[n]->setCutoffFrequency (currentFrequency);

                if (counter == 0)
                {
                    for (i32 n = 0; n < numStages; ++n)
                        filters[n]->setCutoffFrequency (freqSamples[k]);

                    k++;
                }

                for (i32 n = 0; n < numStages; ++n)
                    output = filters[n]->processSample ((i32) channel, output);

                outputSamples[i] = output;
                lastOutput[channel] = output * feedbackVolume[channel].getNextValue();

                counter++;

                if (counter == maxUpdateCounter)
                    counter = 0;
            }
        }

        dryWet.mixWetSamples (outputBlock);
        updateCounter = (updateCounter + (i32) numSamples) % maxUpdateCounter;
    }

private:
    //==============================================================================
    z0 update();

    //==============================================================================
    Oscillator<SampleType> osc;
    OwnedArray<FirstOrderTPTFilter<SampleType>> filters;
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> oscVolume;
    std::vector<SmoothedValue<SampleType, ValueSmoothingTypes::Linear>> feedbackVolume { 2 };
    DryWetMixer<SampleType> dryWet;
    std::vector<SampleType> lastOutput { 2 };
    AudioBuffer<SampleType> bufferFrequency;
    SampleType normCentreFrequency = 0.5;
    f64 sampleRate = 44100.0;

    i32 updateCounter = 0;
    static constexpr i32 maxUpdateCounter = 4;

    SampleType rate = 1.0, depth = 0.5, feedback = 0.0, mix = 0.5;
    SampleType centreFrequency = 1300.0;
    static constexpr i32 numStages = 6;
};

} // namespace drx::dsp
