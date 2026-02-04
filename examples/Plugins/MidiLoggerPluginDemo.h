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

 name:                  MIDILogger
 version:               1.0.0
 vendor:                DRX
 website:               http://drx.com
 description:           Logs incoming MIDI messages.

 dependencies:          drx_audio_basics, drx_audio_devices, drx_audio_formats,
                        drx_audio_plugin_client, drx_audio_processors,
                        drx_audio_utils, drx_core, drx_data_structures,
                        drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:             xcode_mac, vs2022, linux_make

 moduleFlags:           DRX_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             MidiLoggerPluginDemoProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginWantsMidiIn, pluginProducesMidiOut, pluginIsMidiEffectPlugin

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include <iterator>

class MidiQueue
{
public:
    z0 push (const MidiBuffer& buffer)
    {
        for (const auto metadata : buffer)
            fifo.write (1).forEach ([&] (i32 dest) { messages[(size_t) dest] = metadata.getMessage(); });
    }

    template <typename OutputIt>
    z0 pop (OutputIt out)
    {
        fifo.read (fifo.getNumReady()).forEach ([&] (i32 source) { *out++ = messages[(size_t) source]; });
    }

private:
    static constexpr auto queueSize = 1 << 14;
    AbstractFifo fifo { queueSize };
    std::vector<MidiMessage> messages = std::vector<MidiMessage> (queueSize);
};

// Stores the last N messages. Safe to access from the message thread only.
class MidiListModel
{
public:
    template <typename It>
    z0 addMessages (It begin, It end)
    {
        if (begin == end)
            return;

        const auto numNewMessages = (i32) std::distance (begin, end);
        const auto numToAdd = drx::jmin (numToStore, numNewMessages);
        const auto numToRemove = jmax (0, (i32) messages.size() + numToAdd - numToStore);
        messages.erase (messages.begin(), std::next (messages.begin(), numToRemove));
        messages.insert (messages.end(), std::prev (end, numToAdd), end);

        NullCheckedInvocation::invoke (onChange);
    }

    z0 clear()
    {
        messages.clear();

        NullCheckedInvocation::invoke (onChange);
    }

    const MidiMessage& operator[] (size_t ind) const     { return messages[ind]; }

    size_t size() const                                  { return messages.size(); }

    std::function<z0()> onChange;

private:
    static constexpr auto numToStore = 1000;
    std::vector<MidiMessage> messages;
};

//==============================================================================
class MidiTable final : public Component,
                        private TableListBoxModel
{
public:
    MidiTable (MidiListModel& m)
        : messages (m)
    {
        addAndMakeVisible (table);

        table.setModel (this);
        table.setClickingTogglesRowSelection (false);
        table.setHeader ([&]
        {
            auto header = std::make_unique<TableHeaderComponent>();
            header->addColumn ("Message", messageColumn, 200, 30, -1, TableHeaderComponent::notSortable);
            header->addColumn ("Time",    timeColumn,    100, 30, -1, TableHeaderComponent::notSortable);
            header->addColumn ("Channel", channelColumn, 100, 30, -1, TableHeaderComponent::notSortable);
            header->addColumn ("Data",    dataColumn,    200, 30, -1, TableHeaderComponent::notSortable);
            return header;
        }());

        messages.onChange = [&] { table.updateContent(); };
    }

    ~MidiTable() override { messages.onChange = nullptr; }

    z0 resized() override { table.setBounds (getLocalBounds()); }

private:
    enum
    {
        messageColumn = 1,
        timeColumn,
        channelColumn,
        dataColumn
    };

    i32 getNumRows() override          { return (i32) messages.size(); }

    z0 paintRowBackground (Graphics&, i32, i32, i32, b8) override {}
    z0 paintCell (Graphics&, i32, i32, i32, i32, b8)     override {}

    Component* refreshComponentForCell (i32 rowNumber,
                                        i32 columnId,
                                        b8,
                                        Component* existingComponentToUpdate) override
    {
        delete existingComponentToUpdate;

        const auto index = (i32) messages.size() - 1 - rowNumber;
        const auto message = messages[(size_t) index];

        return new Label ({}, [&]
        {
            switch (columnId)
            {
                case messageColumn: return getEventString (message);
                case timeColumn:    return Txt (message.getTimeStamp());
                case channelColumn: return Txt (message.getChannel());
                case dataColumn:    return getDataString (message);
                default:            break;
            }

            jassertfalse;
            return Txt();
        }());
    }

    static Txt getEventString (const MidiMessage& m)
    {
        if (m.isNoteOn())           return "Note on";
        if (m.isNoteOff())          return "Note off";
        if (m.isProgramChange())    return "Program change";
        if (m.isPitchWheel())       return "Pitch wheel";
        if (m.isAftertouch())       return "Aftertouch";
        if (m.isChannelPressure())  return "Channel pressure";
        if (m.isAllNotesOff())      return "All notes off";
        if (m.isAllSoundOff())      return "All sound off";
        if (m.isMetaEvent())        return "Meta event";

        if (m.isController())
        {
            const auto* name = MidiMessage::getControllerName (m.getControllerNumber());
            return "Controller " + (name == nullptr ? Txt (m.getControllerNumber()) : Txt (name));
        }

        return Txt::toHexString (m.getRawData(), m.getRawDataSize());
    }

    static Txt getDataString (const MidiMessage& m)
    {
        if (m.isNoteOn())           return MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) + " Velocity " + Txt (m.getVelocity());
        if (m.isNoteOff())          return MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) + " Velocity " + Txt (m.getVelocity());
        if (m.isProgramChange())    return Txt (m.getProgramChangeNumber());
        if (m.isPitchWheel())       return Txt (m.getPitchWheelValue());
        if (m.isAftertouch())       return MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + Txt (m.getAfterTouchValue());
        if (m.isChannelPressure())  return Txt (m.getChannelPressureValue());
        if (m.isController())       return Txt (m.getControllerValue());

        return {};
    }

    MidiListModel& messages;
    TableListBox table;
};

//==============================================================================
class MidiLoggerPluginDemoProcessor final : public AudioProcessor,
                                            private Timer
{
public:
    MidiLoggerPluginDemoProcessor()
        : AudioProcessor (getBusesLayout())
    {
        state.addChild ({ "uiState", { { "width",  600 }, { "height", 300 } }, {} }, -1, nullptr);
        startTimerHz (60);
    }

    ~MidiLoggerPluginDemoProcessor() override { stopTimer(); }

    z0 processBlock (AudioBuffer<f32>& audio,  MidiBuffer& midi) override { process (audio, midi); }
    z0 processBlock (AudioBuffer<f64>& audio, MidiBuffer& midi) override { process (audio, midi); }

    b8 isBusesLayoutSupported (const BusesLayout&) const override           { return true; }
    b8 isMidiEffect() const override                                        { return true; }
    b8 hasEditor() const override                                           { return true; }
    AudioProcessorEditor* createEditor() override                             { return new Editor (*this); }

    const Txt getName() const override                                     { return "MIDI Logger"; }
    b8 acceptsMidi() const override                                         { return true; }
    b8 producesMidi() const override                                        { return true; }
    f64 getTailLengthSeconds() const override                              { return 0.0; }

    i32 getNumPrograms() override                                             { return 0; }
    i32 getCurrentProgram() override                                          { return 0; }
    z0 setCurrentProgram (i32) override                                     {}
    const Txt getProgramName (i32) override                                { return "None"; }
    z0 changeProgramName (i32, const Txt&) override                      {}

    z0 prepareToPlay (f64, i32) override                                 {}
    z0 releaseResources() override                                          {}

    z0 getStateInformation (MemoryBlock& destData) override
    {
        if (auto xmlState = state.createXml())
            copyXmlToBinary (*xmlState, destData);
    }

    z0 setStateInformation (ukk data, i32 size) override
    {
        if (auto xmlState = getXmlFromBinary (data, size))
            state = ValueTree::fromXml (*xmlState);
    }

private:
    class Editor final : public AudioProcessorEditor,
                         private Value::Listener
    {
    public:
        explicit Editor (MidiLoggerPluginDemoProcessor& ownerIn)
            : AudioProcessorEditor (ownerIn),
              owner (ownerIn),
              table (owner.model)
        {
            addAndMakeVisible (table);
            addAndMakeVisible (clearButton);

            setResizable (true, true);
            lastUIWidth .referTo (owner.state.getChildWithName ("uiState").getPropertyAsValue ("width",  nullptr));
            lastUIHeight.referTo (owner.state.getChildWithName ("uiState").getPropertyAsValue ("height", nullptr));
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());

            lastUIWidth. addListener (this);
            lastUIHeight.addListener (this);

            clearButton.onClick = [&] { owner.model.clear(); };
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();

            clearButton.setBounds (bounds.removeFromBottom (30).withSizeKeepingCentre (50, 24));
            table.setBounds (bounds);

            lastUIWidth  = getWidth();
            lastUIHeight = getHeight();
        }

    private:
        z0 valueChanged (Value&) override
        {
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
        }

        MidiLoggerPluginDemoProcessor& owner;

        MidiTable table;
        TextButton clearButton { "Clear" };

        Value lastUIWidth, lastUIHeight;
    };

    z0 timerCallback() override
    {
        std::vector<MidiMessage> messages;
        queue.pop (std::back_inserter (messages));
        model.addMessages (messages.begin(), messages.end());
    }

    template <typename Element>
    z0 process (AudioBuffer<Element>& audio, MidiBuffer& midi)
    {
        audio.clear();
        queue.push (midi);
    }

    static BusesProperties getBusesLayout()
    {
        // Live and Cakewalk don't like to load midi-only plugins, so we add an audio output there.
        const PluginHostType host;
        return host.isAbletonLive() || host.isSonar()
             ? BusesProperties().withOutput ("out", AudioChannelSet::stereo())
             : BusesProperties();
    }

    ValueTree state { "state" };
    MidiQueue queue;
    MidiListModel model; // The data to show in the UI. We keep it around in the processor so that
                         // the view is persistent even when the plugin UI is closed and reopened.

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiLoggerPluginDemoProcessor)
};
