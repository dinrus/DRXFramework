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

 name:             MidiDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Handles incoming and outcoming midi messages.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MidiDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
struct MidiDeviceListEntry final : ReferenceCountedObject
{
    explicit MidiDeviceListEntry (MidiDeviceInfo info) : deviceInfo (info) {}

    MidiDeviceInfo deviceInfo;
    std::unique_ptr<MidiInput> inDevice;
    std::unique_ptr<MidiOutput> outDevice;

    using Ptr = ReferenceCountedObjectPtr<MidiDeviceListEntry>;

    z0 stopAndReset()
    {
        if (inDevice != nullptr)
            inDevice->stop();

        inDevice .reset();
        outDevice.reset();
    }
};


//==============================================================================
class MidiDemo final : public Component,
                       private MidiKeyboardState::Listener,
                       private MidiInputCallback,
                       private AsyncUpdater
{
public:
    //==============================================================================
    MidiDemo()
        : midiKeyboard       (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
          midiInputSelector  (new MidiDeviceListBox ("Midi Input Selector",  *this, true)),
          midiOutputSelector (new MidiDeviceListBox ("Midi Output Selector", *this, false))
    {
        addLabelAndSetStyle (midiInputLabel);
        addLabelAndSetStyle (midiOutputLabel);
        addLabelAndSetStyle (incomingMidiLabel);
        addLabelAndSetStyle (outgoingMidiLabel);

        midiKeyboard.setName ("MIDI Keyboard");
        addAndMakeVisible (midiKeyboard);

        midiMonitor.setMultiLine (true);
        midiMonitor.setReturnKeyStartsNewLine (false);
        midiMonitor.setReadOnly (true);
        midiMonitor.setScrollbarsShown (true);
        midiMonitor.setCaretVisible (false);
        midiMonitor.setPopupMenuEnabled (false);
        midiMonitor.setText ({});
        addAndMakeVisible (midiMonitor);

        if (! BluetoothMidiDevicePairingDialogue::isAvailable())
            pairButton.setEnabled (false);

        addAndMakeVisible (pairButton);
        pairButton.onClick = []
        {
            RuntimePermissions::request (RuntimePermissions::bluetoothMidi,
                                         [] (b8 wasGranted)
                                         {
                                             if (wasGranted)
                                                 BluetoothMidiDevicePairingDialogue::open();
                                         });
        };
        keyboardState.addListener (this);

        addAndMakeVisible (midiInputSelector .get());
        addAndMakeVisible (midiOutputSelector.get());

        setSize (732, 520);

        updateDeviceLists();
    }

    ~MidiDemo() override
    {
        midiInputs .clear();
        midiOutputs.clear();
        keyboardState.removeListener (this);

        midiInputSelector .reset();
        midiOutputSelector.reset();
    }

    //==============================================================================
    z0 handleNoteOn (MidiKeyboardState*, i32 midiChannel, i32 midiNoteNumber, f32 velocity) override
    {
        MidiMessage m (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
        m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
        sendToOutputs (m);
    }

    z0 handleNoteOff (MidiKeyboardState*, i32 midiChannel, i32 midiNoteNumber, f32 velocity) override
    {
        MidiMessage m (MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
        m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
        sendToOutputs (m);
    }

    z0 paint (Graphics&) override {}

    z0 resized() override
    {
        auto margin = 10;

        midiInputLabel.setBounds (margin, margin,
                                  (getWidth() / 2) - (2 * margin), 24);

        midiOutputLabel.setBounds ((getWidth() / 2) + margin, margin,
                                   (getWidth() / 2) - (2 * margin), 24);

        midiInputSelector->setBounds (margin, (2 * margin) + 24,
                                      (getWidth() / 2) - (2 * margin),
                                      (getHeight() / 2) - ((4 * margin) + 24 + 24));

        midiOutputSelector->setBounds ((getWidth() / 2) + margin, (2 * margin) + 24,
                                       (getWidth() / 2) - (2 * margin),
                                       (getHeight() / 2) - ((4 * margin) + 24 + 24));

        pairButton.setBounds (margin, (getHeight() / 2) - (margin + 24),
                              getWidth() - (2 * margin), 24);

        outgoingMidiLabel.setBounds (margin, getHeight() / 2, getWidth() - (2 * margin), 24);
        midiKeyboard.setBounds (margin, (getHeight() / 2) + (24 + margin), getWidth() - (2 * margin), 64);

        incomingMidiLabel.setBounds (margin, (getHeight() / 2) + (24 + (2 * margin) + 64),
                                     getWidth() - (2 * margin), 24);

        auto y = (getHeight() / 2) + ((2 * 24) + (3 * margin) + 64);
        midiMonitor.setBounds (margin, y,
                               getWidth() - (2 * margin), getHeight() - y - margin);
    }

    z0 openDevice (b8 isInput, i32 index)
    {
        if (isInput)
        {
            jassert (midiInputs[index]->inDevice.get() == nullptr);
            midiInputs[index]->inDevice = MidiInput::openDevice (midiInputs[index]->deviceInfo.identifier, this);

            if (midiInputs[index]->inDevice.get() == nullptr)
            {
                DBG ("MidiDemo::openDevice: open input device for index = " << index << " failed!");
                return;
            }

            midiInputs[index]->inDevice->start();
        }
        else
        {
            jassert (midiOutputs[index]->outDevice.get() == nullptr);
            midiOutputs[index]->outDevice = MidiOutput::openDevice (midiOutputs[index]->deviceInfo.identifier);

            if (midiOutputs[index]->outDevice.get() == nullptr)
            {
                DBG ("MidiDemo::openDevice: open output device for index = " << index << " failed!");
            }
        }
    }

    z0 closeDevice (b8 isInput, i32 index)
    {
        auto& list = isInput ? midiInputs : midiOutputs;
        list[index]->stopAndReset();
    }

    i32 getNumMidiInputs() const noexcept
    {
        return midiInputs.size();
    }

    i32 getNumMidiOutputs() const noexcept
    {
        return midiOutputs.size();
    }

    ReferenceCountedObjectPtr<MidiDeviceListEntry> getMidiDevice (i32 index, b8 isInput) const noexcept
    {
        return isInput ? midiInputs[index] : midiOutputs[index];
    }

private:
    //==============================================================================
    struct MidiDeviceListBox final : private ListBoxModel,
                                     public ListBox
    {
        MidiDeviceListBox (const Txt& name,
                           MidiDemo& contentComponent,
                           b8 isInputDeviceList)
            : ListBox (name),
              parent (contentComponent),
              isInput (isInputDeviceList)
        {
            setModel (this);
            setOutlineThickness (1);
            setMultipleSelectionEnabled (true);
            setClickingTogglesRowSelection (true);
        }

        //==============================================================================
        i32 getNumRows() override
        {
            return isInput ? parent.getNumMidiInputs()
                           : parent.getNumMidiOutputs();
        }

        z0 paintListBoxItem (i32 rowNumber, Graphics& g,
                               i32 width, i32 height, b8 rowIsSelected) override
        {
            auto textColor = getLookAndFeel().findColor (ListBox::textColorId);

            if (rowIsSelected)
                g.fillAll (textColor.interpolatedWith (getLookAndFeel().findColor (ListBox::backgroundColorId), 0.5));


            g.setColor (textColor);
            g.setFont ((f32) height * 0.7f);

            if (isInput)
            {
                if (rowNumber < parent.getNumMidiInputs())
                    g.drawText (parent.getMidiDevice (rowNumber, true)->deviceInfo.name,
                                5, 0, width, height,
                                Justification::centredLeft, true);
            }
            else
            {
                if (rowNumber < parent.getNumMidiOutputs())
                    g.drawText (parent.getMidiDevice (rowNumber, false)->deviceInfo.name,
                                5, 0, width, height,
                                Justification::centredLeft, true);
            }
        }

        //==============================================================================
        z0 selectedRowsChanged (i32) override
        {
            auto newSelectedItems = getSelectedRows();
            if (newSelectedItems != lastSelectedItems)
            {
                for (auto i = 0; i < lastSelectedItems.size(); ++i)
                {
                    if (! newSelectedItems.contains (lastSelectedItems[i]))
                        parent.closeDevice (isInput, lastSelectedItems[i]);
                }

                for (auto i = 0; i < newSelectedItems.size(); ++i)
                {
                    if (! lastSelectedItems.contains (newSelectedItems[i]))
                        parent.openDevice (isInput, newSelectedItems[i]);
                }

                lastSelectedItems = newSelectedItems;
            }
        }

        //==============================================================================
        z0 syncSelectedItemsWithDeviceList (const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices)
        {
            SparseSet<i32> selectedRows;
            for (auto i = 0; i < midiDevices.size(); ++i)
                if (midiDevices[i]->inDevice != nullptr || midiDevices[i]->outDevice != nullptr)
                    selectedRows.addRange (Range<i32> (i, i + 1));

            lastSelectedItems = selectedRows;
            updateContent();
            setSelectedRows (selectedRows, dontSendNotification);
        }

    private:
        //==============================================================================
        MidiDemo& parent;
        b8 isInput;
        SparseSet<i32> lastSelectedItems;
    };

    //==============================================================================
    z0 handleIncomingMidiMessage (MidiInput* /*source*/, const MidiMessage& message) override
    {
        // This is called on the MIDI thread
        const ScopedLock sl (midiMonitorLock);
        incomingMessages.add (message);
        triggerAsyncUpdate();
    }

    z0 handleAsyncUpdate() override
    {
        // This is called on the message loop
        Array<MidiMessage> messages;

        {
            const ScopedLock sl (midiMonitorLock);
            messages.swapWith (incomingMessages);
        }

        Txt messageText;

        for (auto& m : messages)
            messageText << m.getDescription() << "\n";

        midiMonitor.insertTextAtCaret (messageText);
    }

    z0 sendToOutputs (const MidiMessage& msg)
    {
        for (auto midiOutput : midiOutputs)
            if (midiOutput->outDevice != nullptr)
                midiOutput->outDevice->sendMessageNow (msg);
    }

    //==============================================================================
    b8 hasDeviceListChanged (const Array<MidiDeviceInfo>& availableDevices, b8 isInputDevice)
    {
        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                : midiOutputs;

        if (availableDevices.size() != midiDevices.size())
            return true;

        for (auto i = 0; i < availableDevices.size(); ++i)
            if (availableDevices[i] != midiDevices[i]->deviceInfo)
                return true;

        return false;
    }

    ReferenceCountedObjectPtr<MidiDeviceListEntry> findDevice (MidiDeviceInfo device, b8 isInputDevice) const
    {
        const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                      : midiOutputs;

        for (auto& d : midiDevices)
            if (d->deviceInfo == device)
                return d;

        return nullptr;
    }

    z0 closeUnpluggedDevices (const Array<MidiDeviceInfo>& currentlyPluggedInDevices, b8 isInputDevice)
    {
        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                : midiOutputs;

        for (auto i = midiDevices.size(); --i >= 0;)
        {
            auto& d = *midiDevices[i];

            if (! currentlyPluggedInDevices.contains (d.deviceInfo))
            {
                if (isInputDevice ? d.inDevice .get() != nullptr
                                  : d.outDevice.get() != nullptr)
                    closeDevice (isInputDevice, i);

                midiDevices.remove (i);
            }
        }
    }

    z0 updateDeviceList (b8 isInputDeviceList)
    {
        auto availableDevices = isInputDeviceList ? MidiInput::getAvailableDevices()
                                                  : MidiOutput::getAvailableDevices();

        if (hasDeviceListChanged (availableDevices, isInputDeviceList))
        {
            ReferenceCountedArray<MidiDeviceListEntry>& midiDevices
                = isInputDeviceList ? midiInputs : midiOutputs;

            closeUnpluggedDevices (availableDevices, isInputDeviceList);

            ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

            // add all currently plugged-in devices to the device list
            for (auto& newDevice : availableDevices)
            {
                MidiDeviceListEntry::Ptr entry = findDevice (newDevice, isInputDeviceList);

                if (entry == nullptr)
                    entry = new MidiDeviceListEntry (newDevice);

                newDeviceList.add (entry);
            }

            // actually update the device list
            midiDevices = newDeviceList;

            // update the selection status of the combo-box
            if (auto* midiSelector = isInputDeviceList ? midiInputSelector.get() : midiOutputSelector.get())
                midiSelector->syncSelectedItemsWithDeviceList (midiDevices);
        }
    }

    //==============================================================================
    z0 addLabelAndSetStyle (Label& label)
    {
        label.setFont (FontOptions (15.00f, Font::plain));
        label.setJustificationType (Justification::centredLeft);
        label.setEditable (false, false, false);
        label.setColor (TextEditor::textColorId, Colors::black);
        label.setColor (TextEditor::backgroundColorId, Color (0x00000000));

        addAndMakeVisible (label);
    }

    z0 updateDeviceLists()
    {
        for (const auto isInput : { true, false })
            updateDeviceList (isInput);
    }

    //==============================================================================
    Label midiInputLabel    { "Midi Input Label",  "MIDI Input:" };
    Label midiOutputLabel   { "Midi Output Label", "MIDI Output:" };
    Label incomingMidiLabel { "Incoming Midi Label", "Received MIDI messages:" };
    Label outgoingMidiLabel { "Outgoing Midi Label", "Play the keyboard to send MIDI messages..." };
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent midiKeyboard;
    TextEditor midiMonitor  { "MIDI Monitor" };
    TextButton pairButton   { "MIDI Bluetooth devices..." };

    ReferenceCountedArray<MidiDeviceListEntry> midiInputs, midiOutputs;
    std::unique_ptr<MidiDeviceListBox> midiInputSelector, midiOutputSelector;

    CriticalSection midiMonitorLock;
    Array<MidiMessage> incomingMessages;

    MidiDeviceListConnection connection = MidiDeviceListConnection::make ([this]
    {
        updateDeviceLists();
    });

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDemo)
};
