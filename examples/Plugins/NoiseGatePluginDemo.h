/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             NoiseGatePlugin
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Noise gate audio plugin.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_plugin_client, drx_audio_processors,
                   drx_audio_utils, drx_core, drx_data_structures,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        NoiseGate

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class NoiseGate final : public AudioProcessor
{
public:
    //==============================================================================
    NoiseGate()
        : AudioProcessor (BusesProperties().withInput  ("Input",     AudioChannelSet::stereo())
                                           .withOutput ("Output",    AudioChannelSet::stereo())
                                           .withInput  ("Sidechain", AudioChannelSet::stereo()))
    {
        addParameter (threshold = new AudioParameterFloat ({ "threshold", 1 }, "Threshold", 0.0f, 1.0f, 0.5f));
        addParameter (alpha     = new AudioParameterFloat ({ "alpha",     1 }, "Alpha",     0.0f, 1.0f, 0.8f));
    }

    //==============================================================================
    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // the sidechain can take any layout, the main bus needs to be the same on the input and output
        return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
                 && ! layouts.getMainInputChannelSet().isDisabled();
    }

    //==============================================================================
    z0 prepareToPlay (f64, i32) override { lowPassCoeff = 0.0f; sampleCountDown = 0; }
    z0 releaseResources() override {}

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer&) override
    {
        auto mainInputOutput = getBusBuffer (buffer, true, 0);
        auto sideChainInput  = getBusBuffer (buffer, true, 1);

        auto alphaCopy     = alpha->get();
        auto thresholdCopy = threshold->get();

        for (i32 j = 0; j < buffer.getNumSamples(); ++j)
        {
            auto mixedSamples = 0.0f;

            for (i32 i = 0; i < sideChainInput.getNumChannels(); ++i)
                mixedSamples += sideChainInput.getReadPointer (i)[j];

            mixedSamples /= static_cast<f32> (sideChainInput.getNumChannels());
            lowPassCoeff = (alphaCopy * lowPassCoeff) + ((1.0f - alphaCopy) * mixedSamples);

            if (lowPassCoeff >= thresholdCopy)
                sampleCountDown = (i32) getSampleRate();

            // very in-effective way of doing this
            for (i32 i = 0; i < mainInputOutput.getNumChannels(); ++i)
                *mainInputOutput.getWritePointer (i, j) = sampleCountDown > 0 ? *mainInputOutput.getReadPointer (i, j) : 0.0f;

            if (sampleCountDown > 0)
                --sampleCountDown;
        }
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    AudioProcessorEditor* createEditor() override            { return new GenericAudioProcessorEditor (*this); }
    b8 hasEditor() const override                          { return true; }
    const Txt getName() const override                    { return "NoiseGate"; }
    b8 acceptsMidi() const override                        { return false; }
    b8 producesMidi() const override                       { return false; }
    f64 getTailLengthSeconds() const override             { return 0.0; }
    i32 getNumPrograms() override                            { return 1; }
    i32 getCurrentProgram() override                         { return 0; }
    z0 setCurrentProgram (i32) override                    {}
    const Txt getProgramName (i32) override               { return "None"; }
    z0 changeProgramName (i32, const Txt&) override     {}
    b8 isVST2() const noexcept                             { return (wrapperType == wrapperType_VST); }

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*threshold);
        stream.writeFloat (*alpha);
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

        threshold->setValueNotifyingHost (stream.readFloat());
        alpha->setValueNotifyingHost     (stream.readFloat());
    }

private:
    //==============================================================================
    AudioParameterFloat* threshold;
    AudioParameterFloat* alpha;
    i32 sampleCountDown;

    f32 lowPassCoeff;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseGate)
};
