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

 name:                  ArpeggiatorPlugin
 version:               1.0.0
 vendor:                DRX
 website:               http://drx.com
 description:           Arpeggiator audio plugin.

 dependencies:          drx_audio_basics, drx_audio_devices, drx_audio_formats,
                        drx_audio_plugin_client, drx_audio_processors,
                        drx_audio_utils, drx_core, drx_data_structures,
                        drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:             xcode_mac, vs2022

 moduleFlags:           DRX_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             Arpeggiator

 useLocalCopy:          1

 pluginCharacteristics: pluginWantsMidiIn, pluginProducesMidiOut, pluginIsMidiEffectPlugin

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class Arpeggiator final : public AudioProcessor
{
public:

    //==============================================================================
    Arpeggiator()
        : AudioProcessor (BusesProperties()) // add no audio buses at all
    {
        addParameter (speed = new AudioParameterFloat ({ "speed", 1 }, "Arpeggiator Speed", 0.0, 1.0, 0.5));
    }

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate, i32 samplesPerBlock) override
    {
        ignoreUnused (samplesPerBlock);

        notes.clear();
        currentNote = 0;
        lastNoteValue = -1;
        time = 0;
        rate = static_cast<f32> (sampleRate);
    }

    z0 releaseResources() override {}

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midi) override
    {
        // A pure MIDI plugin shouldn't be provided any audio data
        jassert (buffer.getNumChannels() == 0);

        // however we use the buffer to get timing information
        auto numSamples = buffer.getNumSamples();

        // get note duration
        auto noteDuration = static_cast<i32> (std::ceil (rate * 0.25f * (0.1f + (1.0f - (*speed)))));

        for (const auto metadata : midi)
        {
            const auto msg = metadata.getMessage();
            if      (msg.isNoteOn())  notes.add (msg.getNoteNumber());
            else if (msg.isNoteOff()) notes.removeValue (msg.getNoteNumber());
        }

        midi.clear();

        if ((time + numSamples) >= noteDuration)
        {
            auto offset = jmax (0, jmin ((i32) (noteDuration - time), numSamples - 1));

            if (lastNoteValue > 0)
            {
                midi.addEvent (MidiMessage::noteOff (1, lastNoteValue), offset);
                lastNoteValue = -1;
            }

            if (notes.size() > 0)
            {
                currentNote = (currentNote + 1) % notes.size();
                lastNoteValue = notes[currentNote];
                midi.addEvent (MidiMessage::noteOn  (1, lastNoteValue, (u8) 127), offset);
            }

        }

        time = (time + numSamples) % noteDuration;
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    b8 isMidiEffect() const override                     { return true; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GenericAudioProcessorEditor (*this); }
    b8 hasEditor() const override                        { return true; }

    //==============================================================================
    const Txt getName() const override                  { return "Arpeggiator"; }

    b8 acceptsMidi() const override                      { return true; }
    b8 producesMidi() const override                     { return true; }
    f64 getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    i32 getNumPrograms() override                          { return 1; }
    i32 getCurrentProgram() override                       { return 0; }
    z0 setCurrentProgram (i32) override                  {}
    const Txt getProgramName (i32) override             { return "None"; }
    z0 changeProgramName (i32, const Txt&) override   {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream (destData, true).writeFloat (*speed);
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        speed->setValueNotifyingHost (MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
    }

private:
    //==============================================================================
    AudioParameterFloat* speed;
    i32 currentNote, lastNoteValue;
    i32 time;
    f32 rate;
    SortedSet<i32> notes;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Arpeggiator)
};
