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

 name:                  MultiOutSynthPlugin
 version:               1.0.0
 vendor:                DRX
 website:               http://drx.com
 description:           Multi-out synthesiser audio plugin.

 dependencies:          drx_audio_basics, drx_audio_devices, drx_audio_formats,
                        drx_audio_plugin_client, drx_audio_processors,
                        drx_audio_utils, drx_core, drx_data_structures,
                        drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:             xcode_mac, vs2022

 moduleFlags:           DRX_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             MultiOutSynth

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class MultiOutSynth final : public AudioProcessor
{
public:
    enum
    {
        maxMidiChannel    = 16,
        maxNumberOfVoices = 5
    };

    //==============================================================================
    MultiOutSynth()
        : AudioProcessor (BusesProperties()
                          .withOutput ("Output #1",  AudioChannelSet::stereo(), true)
                          .withOutput ("Output #2",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #3",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #4",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #5",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #6",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #7",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #8",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #9",  AudioChannelSet::stereo(), false)
                          .withOutput ("Output #10", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #11", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #12", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #13", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #14", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #15", AudioChannelSet::stereo(), false)
                          .withOutput ("Output #16", AudioChannelSet::stereo(), false))
    {
        // initialize other stuff (not related to buses)
        formatManager.registerBasicFormats();

        for (i32 midiChannel = 0; midiChannel < maxMidiChannel; ++midiChannel)
        {
            synth.add (new Synthesiser());

            for (i32 i = 0; i < maxNumberOfVoices; ++i)
                synth[midiChannel]->addVoice (new SamplerVoice());
        }

        loadNewSample (createAssetInputStream ("singing.ogg"), "ogg");
    }

    //==============================================================================
    b8 canAddBus    (b8 isInput) const override   { return ! isInput; }
    b8 canRemoveBus (b8 isInput) const override   { return ! isInput; }

    //==============================================================================
    z0 prepareToPlay (f64 newSampleRate, i32 samplesPerBlock) override
    {
        ignoreUnused (samplesPerBlock);

        for (auto* s : synth)
            s->setCurrentPlaybackSampleRate (newSampleRate);
    }

    z0 releaseResources() override {}

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midiBuffer) override
    {
        auto busCount = getBusCount (false);

        for (auto busNr = 0; busNr < busCount; ++busNr)
        {
            if (synth.size() <= busNr)
                continue;

            auto midiChannelBuffer = filterMidiMessagesForChannel (midiBuffer, busNr + 1);
            auto audioBusBuffer = getBusBuffer (buffer, false, busNr);

            // Voices add to the contents of the buffer. Make sure the buffer is clear before
            // rendering, just in case the host left old data in the buffer.
            audioBusBuffer.clear();

            synth [busNr]->renderNextBlock (audioBusBuffer, midiChannelBuffer, 0, audioBusBuffer.getNumSamples());
        }
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GenericAudioProcessorEditor (*this); }
    b8 hasEditor() const override                        { return true; }

    //==============================================================================
    const Txt getName() const override                  { return "Multi Out Synth PlugIn"; }
    b8 acceptsMidi() const override                      { return false; }
    b8 producesMidi() const override                     { return false; }
    f64 getTailLengthSeconds() const override           { return 0; }
    i32 getNumPrograms() override                          { return 1; }
    i32 getCurrentProgram() override                       { return 0; }
    z0 setCurrentProgram (i32) override                  {}
    const Txt getProgramName (i32) override             { return "None"; }
    z0 changeProgramName (i32, const Txt&) override   {}

    b8 isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        const auto& outputs = layout.outputBuses;

        return layout.inputBuses.isEmpty()
            && 1 <= outputs.size()
            && std::all_of (outputs.begin(), outputs.end(), [] (const auto& bus)
               {
                   return bus.isDisabled() || bus == AudioChannelSet::stereo();
               });
    }

    //==============================================================================
    z0 getStateInformation (MemoryBlock&) override {}
    z0 setStateInformation (ukk, i32) override {}

private:
    //==============================================================================
    static MidiBuffer filterMidiMessagesForChannel (const MidiBuffer& input, i32 channel)
    {
        MidiBuffer output;

        for (const auto metadata : input)
        {
            const auto message = metadata.getMessage();

            if (message.getChannel() == channel)
                output.addEvent (message, metadata.samplePosition);
        }

        return output;
    }

    z0 loadNewSample (std::unique_ptr<InputStream> soundBuffer, tukk format)
    {
        std::unique_ptr<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension (format)->createReaderFor (soundBuffer.release(), true));

        BigInteger midiNotes;
        midiNotes.setRange (0, 126, true);
        SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);

        for (auto* s : synth)
            s->removeSound (0);

        sound = newSound;

        for (auto* s : synth)
            s->addSound (sound);
    }

    //==============================================================================
    AudioFormatManager formatManager;
    OwnedArray<Synthesiser> synth;
    SynthesiserSound::Ptr sound;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiOutSynth)
};
