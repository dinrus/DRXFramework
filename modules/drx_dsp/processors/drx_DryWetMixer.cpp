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

//==============================================================================
template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer()
    : DryWetMixer (0)
{
}

template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer (i32 maximumWetLatencyInSamplesIn)
    : dryDelayLine (maximumWetLatencyInSamplesIn),
      maximumWetLatencyInSamples (maximumWetLatencyInSamplesIn)
{
    dryDelayLine.setDelay (0);

    update();
    reset();
}

//==============================================================================
template <typename SampleType>
z0 DryWetMixer<SampleType>::setMixingRule (MixingRule newRule)
{
    currentMixingRule = newRule;
    update();
}

template <typename SampleType>
z0 DryWetMixer<SampleType>::setWetMixProportion (SampleType newWetMixProportion)
{
    jassert (isPositiveAndNotGreaterThan (newWetMixProportion, 1.0));

    mix = jlimit (static_cast<SampleType> (0.0), static_cast<SampleType> (1.0), newWetMixProportion);
    update();
}

template <typename SampleType>
z0 DryWetMixer<SampleType>::setWetLatency (SampleType wetLatencySamples)
{
    dryDelayLine.setDelay (wetLatencySamples);
}

//==============================================================================
template <typename SampleType>
z0 DryWetMixer<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    dryDelayLine.prepare (spec);
    bufferDry.setSize ((i32) spec.numChannels, (i32) spec.maximumBlockSize, false, false, true);

    update();
    reset();
}

template <typename SampleType>
z0 DryWetMixer<SampleType>::reset()
{
    dryVolume.reset (sampleRate, 0.05);
    wetVolume.reset (sampleRate, 0.05);

    dryDelayLine.reset();

    fifo = SingleThreadedAbstractFifo (nextPowerOfTwo (bufferDry.getNumSamples()));
    bufferDry.setSize (bufferDry.getNumChannels(), fifo.getSize(), false, false, true);
}

//==============================================================================
template <typename SampleType>
z0 DryWetMixer<SampleType>::pushDrySamples (const AudioBlock<const SampleType> drySamples)
{
    jassert (drySamples.getNumChannels() <= (size_t) bufferDry.getNumChannels());
    jassert (drySamples.getNumSamples() <= (size_t) fifo.getRemainingSpace());

    auto offset = 0;

    for (const auto& range : fifo.write ((i32) drySamples.getNumSamples()))
    {
        if (range.getLength() == 0)
            continue;

        auto block = AudioBlock<SampleType> (bufferDry).getSubsetChannelBlock (0, drySamples.getNumChannels())
                                                       .getSubBlock ((size_t) range.getStart(), (size_t) range.getLength());

        auto inputBlock = drySamples.getSubBlock ((size_t) offset, (size_t) range.getLength());

        if (maximumWetLatencyInSamples == 0)
            block.copyFrom (inputBlock);
        else
            dryDelayLine.process (ProcessContextNonReplacing<SampleType> (inputBlock, block));

        offset += range.getLength();
    }
}

template <typename SampleType>
z0 DryWetMixer<SampleType>::mixWetSamples (AudioBlock<SampleType> inOutBlock)
{
    inOutBlock.multiplyBy (wetVolume);

    jassert (inOutBlock.getNumSamples() <= (size_t) fifo.getNumReadable());

    auto offset = 0;

    for (const auto& range : fifo.read ((i32) inOutBlock.getNumSamples()))
    {
        if (range.getLength() == 0)
            continue;

        auto block = AudioBlock<SampleType> (bufferDry).getSubsetChannelBlock (0, inOutBlock.getNumChannels())
                                                       .getSubBlock ((size_t) range.getStart(), (size_t) range.getLength());
        block.multiplyBy (dryVolume);
        inOutBlock.getSubBlock ((size_t) offset).add (block);

        offset += range.getLength();
    }
}

//==============================================================================
template <typename SampleType>
z0 DryWetMixer<SampleType>::update()
{
    SampleType dryValue, wetValue;

    switch (currentMixingRule)
    {
        case MixingRule::balanced:
            dryValue = static_cast<SampleType> (2.0) * jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - mix);
            wetValue = static_cast<SampleType> (2.0) * jmin (static_cast<SampleType> (0.5), mix);
            break;

        case MixingRule::linear:
            dryValue = static_cast<SampleType> (1.0) - mix;
            wetValue = mix;
            break;

        case MixingRule::sin3dB:
            dryValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<f64>::pi * (1.0 - mix)));
            wetValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<f64>::pi * mix));
            break;

        case MixingRule::sin4p5dB:
            dryValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * (1.0 - mix)), 1.5));
            wetValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * mix), 1.5));
            break;

        case MixingRule::sin6dB:
            dryValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * (1.0 - mix)), 2.0));
            wetValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * mix), 2.0));
            break;

        case MixingRule::squareRoot3dB:
            dryValue = std::sqrt (static_cast<SampleType> (1.0) - mix);
            wetValue = std::sqrt (mix);
            break;

        case MixingRule::squareRoot4p5dB:
            dryValue = static_cast<SampleType> (std::pow (std::sqrt (1.0 - mix), 1.5));
            wetValue = static_cast<SampleType> (std::pow (std::sqrt (mix), 1.5));
            break;

        default:
            dryValue = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - mix);
            wetValue = jmin (static_cast<SampleType> (0.5), mix);
            break;
    }

    dryVolume.setTargetValue (dryValue);
    wetVolume.setTargetValue (wetValue);
}

//==============================================================================
template class DryWetMixer<f32>;
template class DryWetMixer<f64>;


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct DryWetMixerTests final : public UnitTest
{
    DryWetMixerTests() : UnitTest ("DryWetMixer", UnitTestCategories::dsp) {}

    enum class Kind { down, up };

    static auto getRampBuffer (ProcessSpec spec, Kind kind)
    {
        AudioBuffer<f32> buffer ((i32) spec.numChannels, (i32) spec.maximumBlockSize);

        for (u32 sample = 0; sample < spec.maximumBlockSize; ++sample)
        {
            for (u32 channel = 0; channel < spec.numChannels; ++channel)
            {
                const auto ramp = kind == Kind::up ? sample : spec.maximumBlockSize - sample;

                buffer.setSample ((i32) channel,
                                  (i32) sample,
                                  jmap ((f32) ramp, 0.0f, (f32) spec.maximumBlockSize, 0.0f, 1.0f));
            }
        }

        return buffer;
    }

    z0 runTest() override
    {
        constexpr ProcessSpec spec { 44100.0, 512, 2 };
        constexpr auto numBlocks = 5;

        const auto wetBuffer = getRampBuffer (spec, Kind::up);
        const auto dryBuffer = getRampBuffer (spec, Kind::down);

        for (auto maxLatency : { 0, 100, 200, 512 })
        {
            beginTest ("Mixer can push multiple small buffers");
            {
                DryWetMixer<f32> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push samples one-by-one
                    for (u32 sample = 0; sample < spec.maximumBlockSize; ++sample)
                        mixer.pushDrySamples (AudioBlock<const f32> (dryBuffer).getSubBlock (sample, 1));

                    // Mix wet samples in one go
                    auto outputBlock = wetBuffer;
                    mixer.mixWetSamples ({ outputBlock });

                    // The output block should contain the wet and dry samples averaged
                    for (u32 sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        for (u32 channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((i32) channel, (i32) sample);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }

            beginTest ("Mixer can pop multiple small buffers");
            {
                DryWetMixer<f32> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push samples in one go
                    mixer.pushDrySamples ({ dryBuffer });

                    // Process wet samples one-by-one
                    for (u32 sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        AudioBuffer<f32> outputBlock ((i32) spec.numChannels, 1);
                        AudioBlock<const f32> (wetBuffer).getSubBlock (sample, 1).copyTo (outputBlock);
                        mixer.mixWetSamples ({ outputBlock });

                        // The output block should contain the wet and dry samples averaged
                        for (u32 channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((i32) channel, 0);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }

            beginTest ("Mixer can push and pop multiple small buffers");
            {
                DryWetMixer<f32> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push dry samples and process wet samples one-by-one
                    for (u32 sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        mixer.pushDrySamples (AudioBlock<const f32> (dryBuffer).getSubBlock (sample, 1));

                        AudioBuffer<f32> outputBlock ((i32) spec.numChannels, 1);
                        AudioBlock<const f32> (wetBuffer).getSubBlock (sample, 1).copyTo (outputBlock);
                        mixer.mixWetSamples ({ outputBlock });

                        // The output block should contain the wet and dry samples averaged
                        for (u32 channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((i32) channel, 0);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }

            beginTest ("Mixer can push and pop full-sized blocks after encountering a shorter block");
            {
                DryWetMixer<f32> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                constexpr auto shortBlockLength = spec.maximumBlockSize / 2;
                AudioBuffer<f32> shortBlock (spec.numChannels, shortBlockLength);
                mixer.pushDrySamples (AudioBlock<const f32> (dryBuffer).getSubBlock (shortBlockLength));
                mixer.mixWetSamples ({ shortBlock });

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push a full block of dry samples
                    mixer.pushDrySamples ({ dryBuffer });

                    // Mix a full block of wet samples
                    auto outputBlock = wetBuffer;
                    mixer.mixWetSamples ({ outputBlock });

                    // The output block should contain the wet and dry samples averaged
                    for (u32 sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        for (u32 channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((i32) channel, (i32) sample);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }
        }
    }
};

static const DryWetMixerTests dryWetMixerTests;

#endif

} // namespace drx::dsp
