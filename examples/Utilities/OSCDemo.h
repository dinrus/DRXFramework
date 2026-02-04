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

 name:             OSCDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Application using the OSC protocol.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_osc
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OSCDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class OSCLogListBox final : public ListBox,
                            private ListBoxModel,
                            private AsyncUpdater
{
public:
    OSCLogListBox()
    {
        setModel (this);
    }

    ~OSCLogListBox() override = default;

    //==============================================================================
    i32 getNumRows() override
    {
        return oscLogList.size();
    }

    //==============================================================================
    z0 paintListBoxItem (i32 row, Graphics& g, i32 width, i32 height, b8 rowIsSelected) override
    {
        ignoreUnused (rowIsSelected);

        if (isPositiveAndBelow (row, oscLogList.size()))
        {
            g.setColor (Colors::white);

            g.drawText (oscLogList[row],
                        Rectangle<i32> (width, height).reduced (4, 0),
                        Justification::centredLeft, true);
        }
    }

    //==============================================================================
    z0 addOSCMessage (const OSCMessage& message, i32 level = 0)
    {
        oscLogList.add (getIndentationString (level)
                        + "- osc message, address = '"
                        + message.getAddressPattern().toString()
                        + "', "
                        + Txt (message.size())
                        + " argument(s)");

        if (! message.isEmpty())
        {
            for (auto& arg : message)
                addOSCMessageArgument (arg, level + 1);
        }

        triggerAsyncUpdate();
    }

    //==============================================================================
    z0 addOSCBundle (const OSCBundle& bundle, i32 level = 0)
    {
        OSCTimeTag timeTag = bundle.getTimeTag();

        oscLogList.add (getIndentationString (level)
                        + "- osc bundle, time tag = "
                        + timeTag.toTime().toString (true, true, true, true));

        for (auto& element : bundle)
        {
            if (element.isMessage())
                addOSCMessage (element.getMessage(), level + 1);
            else if (element.isBundle())
                addOSCBundle (element.getBundle(), level + 1);
        }

        triggerAsyncUpdate();
    }

    //==============================================================================
    z0 addOSCMessageArgument (const OSCArgument& arg, i32 level)
    {
        Txt typeAsString;
        Txt valueAsString;

        if (arg.isFloat32())
        {
            typeAsString = "float32";
            valueAsString = Txt (arg.getFloat32());
        }
        else if (arg.isInt32())
        {
            typeAsString = "i32";
            valueAsString = Txt (arg.getInt32());
        }
        else if (arg.isString())
        {
            typeAsString = "string";
            valueAsString = arg.getString();
        }
        else if (arg.isBlob())
        {
            typeAsString = "blob";
            auto& blob = arg.getBlob();
            valueAsString = Txt::fromUTF8 ((tukk) blob.getData(), (i32) blob.getSize());
        }
        else
        {
            typeAsString = "(unknown)";
        }

        oscLogList.add (getIndentationString (level + 1) + "- " + typeAsString.paddedRight (' ', 12) + valueAsString);
    }

    //==============================================================================
    z0 addInvalidOSCPacket (tukk /* data */, i32 dataSize)
    {
        oscLogList.add ("- (" + Txt (dataSize) + "bytes with invalid format)");
    }

    //==============================================================================
    z0 clear()
    {
        oscLogList.clear();
        triggerAsyncUpdate();
    }

    //==============================================================================
    z0 handleAsyncUpdate() override
    {
        updateContent();
        scrollToEnsureRowIsOnscreen (oscLogList.size() - 1);
        repaint();
    }

private:
    static Txt getIndentationString (i32 level)
    {
        return Txt().paddedRight (' ', 2 * level);
    }

    //==============================================================================
    StringArray oscLogList;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCLogListBox)
};

//==============================================================================
class OSCSenderDemo final : public Component
{
public:
    OSCSenderDemo()
    {
        addAndMakeVisible (senderLabel);
        senderLabel.attachToComponent (&rotaryKnob, false);

        rotaryKnob.setRange (0.0, 1.0);
        rotaryKnob.setSliderStyle (Slider::RotaryVerticalDrag);
        rotaryKnob.setTextBoxStyle (Slider::TextBoxBelow, true, 150, 25);
        rotaryKnob.setBounds (50, 50, 180, 180);
        addAndMakeVisible (rotaryKnob);
        rotaryKnob.onValueChange = [this]
        {
            // create and send an OSC message with an address and a f32 value:
            if (! sender1.send ("/drx/rotaryknob", (f32) rotaryKnob.getValue()))
                showConnectionErrorMessage ("Error: could not send OSC message.");
            if (! sender2.send ("/drx/rotaryknob", (f32) rotaryKnob.getValue()))
                showConnectionErrorMessage ("Error: could not send OSC message.");
        };

        // specify here where to send OSC messages to: host URL and UDP port number
        if (! sender1.connect ("127.0.0.1", 9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");
        if (! sender2.connect ("127.0.0.1", 9002))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9002.");
    }

private:
    //==============================================================================
    z0 showConnectionErrorMessage (const Txt& messageText)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Connection error",
                                                         messageText);
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    Slider rotaryKnob;
    OSCSender sender1, sender2;
    Label senderLabel { {}, "Sender" };
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCSenderDemo)
};

//==============================================================================
class OSCReceiverDemo final : public Component,
                              private OSCReceiver,
                              private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    OSCReceiverDemo()
    {
        addAndMakeVisible (receiverLabel);
        receiverLabel.attachToComponent (&rotaryKnob, false);

        rotaryKnob.setRange (0.0, 1.0);
        rotaryKnob.setSliderStyle (Slider::RotaryVerticalDrag);
        rotaryKnob.setTextBoxStyle (Slider::TextBoxBelow, true, 150, 25);
        rotaryKnob.setBounds (50, 50, 180, 180);
        rotaryKnob.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (rotaryKnob);

        // specify here on which UDP port number to receive incoming OSC messages
        if (! connect (9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");

        // tell the component to listen for OSC messages matching this address:
        addListener (this, "/drx/rotaryknob");
    }

private:
    //==============================================================================
    z0 oscMessageReceived (const OSCMessage& message) override
    {
        if (message.size() == 1 && message[0].isFloat32())
            rotaryKnob.setValue (jlimit (0.0f, 10.0f, message[0].getFloat32()));
    }

    z0 showConnectionErrorMessage (const Txt& messageText)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Connection error",
                                                         messageText);
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    Slider rotaryKnob;
    Label receiverLabel { {}, "Receiver" };
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCReceiverDemo)
};

//==============================================================================
class OSCMonitorDemo final : public Component,
                             private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    OSCMonitorDemo()
    {
        portNumberLabel.setBounds (10, 18, 130, 25);
        addAndMakeVisible (portNumberLabel);

        portNumberField.setEditable (true, true, true);
        portNumberField.setBounds (140, 18, 50, 25);
        addAndMakeVisible (portNumberField);

        connectButton.setBounds (210, 18, 100, 25);
        addAndMakeVisible (connectButton);
        connectButton.onClick = [this] { connectButtonClicked(); };

        clearButton.setBounds (320, 18, 60, 25);
        addAndMakeVisible (clearButton);
        clearButton.onClick = [this] { clearButtonClicked(); };

        connectionStatusLabel.setBounds (450, 18, 240, 25);
        updateConnectionStatusLabel();
        addAndMakeVisible (connectionStatusLabel);

        oscLogListBox.setBounds (0, 60, 700, 340);
        addAndMakeVisible (oscLogListBox);

        oscReceiver.addListener (this);
        oscReceiver.registerFormatErrorHandler ([this] (tukk data, i32 dataSize)
                                                {
                                                    oscLogListBox.addInvalidOSCPacket (data, dataSize);
                                                });
    }

private:
    //==============================================================================
    Label portNumberLabel    { {}, "UDP Port Number: " };
    Label portNumberField    { {}, "9002" };
    TextButton connectButton { "Connect" };
    TextButton clearButton   { "Clear" };
    Label connectionStatusLabel;

    OSCLogListBox oscLogListBox;
    OSCReceiver oscReceiver;

    i32 currentPortNumber = -1;

    //==============================================================================
    z0 connectButtonClicked()
    {
        if (! isConnected())
            connect();
        else
            disconnect();

        updateConnectionStatusLabel();
    }

    //==============================================================================
    z0 clearButtonClicked()
    {
        oscLogListBox.clear();
    }

    //==============================================================================
    z0 oscMessageReceived (const OSCMessage& message) override
    {
        oscLogListBox.addOSCMessage (message);
    }

    z0 oscBundleReceived (const OSCBundle& bundle) override
    {
        oscLogListBox.addOSCBundle (bundle);
    }

    //==============================================================================
    z0 connect()
    {
        auto portToConnect = portNumberField.getText().getIntValue();

        if (! isValidOscPort (portToConnect))
        {
            handleInvalidPortNumberEntered();
            return;
        }

        if (oscReceiver.connect (portToConnect))
        {
            currentPortNumber = portToConnect;
            connectButton.setButtonText ("Disconnect");
        }
        else
        {
            handleConnectError (portToConnect);
        }
    }

    //==============================================================================
    z0 disconnect()
    {
        if (oscReceiver.disconnect())
        {
            currentPortNumber = -1;
            connectButton.setButtonText ("Connect");
        }
        else
        {
            handleDisconnectError();
        }
    }

    //==============================================================================
    z0 handleConnectError (i32 failedPort)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "OSC Connection error",
                                                         "Error: could not connect to port " + Txt (failedPort));
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    z0 handleDisconnectError()
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Unknown error",
                                                         "An unknown error occurred while trying to disconnect from UDP port.");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    z0 handleInvalidPortNumberEntered()
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Invalid port number",
                                                         "Error: you have entered an invalid UDP port number.");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    b8 isConnected() const
    {
        return currentPortNumber != -1;
    }

    //==============================================================================
    b8 isValidOscPort (i32 port) const
    {
        return port > 0 && port < 65536;
    }

    //==============================================================================
    z0 updateConnectionStatusLabel()
    {
        Txt text = "Status: ";

        if (isConnected())
            text += "Connected to UDP port " + Txt (currentPortNumber);
        else
            text += "Disconnected";

        auto textColor = isConnected() ? Colors::green : Colors::red;

        connectionStatusLabel.setText (text, dontSendNotification);
        connectionStatusLabel.setFont (FontOptions (15.00f, Font::bold));
        connectionStatusLabel.setColor (Label::textColorId, textColor);
        connectionStatusLabel.setJustificationType (Justification::centredRight);
    }

    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCMonitorDemo)
};

//==============================================================================
class OSCDemo final : public Component
{
public:
    OSCDemo()
    {
        addAndMakeVisible (monitor);
        addAndMakeVisible (receiver);
        addAndMakeVisible (sender);

        setSize (700, 400);
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        auto lowerBounds = bounds.removeFromBottom (getHeight() / 2);
        auto halfBounds  = bounds.removeFromRight  (getWidth()  / 2);

        sender  .setBounds (bounds);
        receiver.setBounds (halfBounds);
        monitor .setBounds (lowerBounds.removeFromTop (getHeight() / 2));
    }

private:
    OSCMonitorDemo  monitor;
    OSCReceiverDemo receiver;
    OSCSenderDemo   sender;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCDemo)
};
