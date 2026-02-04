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

namespace drx
{

struct SimpleDeviceManagerInputLevelMeter final : public Component,
                                                  public Timer
{
    SimpleDeviceManagerInputLevelMeter (AudioDeviceManager& m)  : manager (m)
    {
        startTimerHz (20);
        inputLevelGetter = manager.getInputLevelGetter();
    }

    z0 timerCallback() override
    {
        if (isShowing())
        {
            auto newLevel = (f32) inputLevelGetter->getCurrentLevel();

            if (std::abs (level - newLevel) > 0.005f)
            {
                level = newLevel;
                repaint();
            }
        }
        else
        {
            level = 0;
        }
    }

    z0 paint (Graphics& g) override
    {
        // (add a bit of a skew to make the level more obvious)
        getLookAndFeel().drawLevelMeter (g, getWidth(), getHeight(),
                                         (f32) std::exp (std::log (level) / 3.0));
    }

    AudioDeviceManager& manager;
    AudioDeviceManager::LevelMeter::Ptr inputLevelGetter;
    f32 level = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDeviceManagerInputLevelMeter)
};

static z0 drawTextLayout (Graphics& g, Component& owner, StringRef text, const Rectangle<i32>& textBounds, b8 enabled)
{
    const auto textColor = owner.findColor (ListBox::textColorId, true).withMultipliedAlpha (enabled ? 1.0f : 0.6f);

    AttributedString attributedString { text };
    attributedString.setColor (textColor);
    attributedString.setFont (owner.withDefaultMetrics (FontOptions { (f32) textBounds.getHeight() * 0.6f }));
    attributedString.setJustification (Justification::centredLeft);
    attributedString.setWordWrap (AttributedString::WordWrap::none);

    TextLayout textLayout;
    textLayout.createLayout (attributedString,
                             (f32) textBounds.getWidth(),
                             (f32) textBounds.getHeight());
    textLayout.draw (g, textBounds.toFloat());
}


//==============================================================================
class AudioDeviceSelectorComponent::MidiInputSelectorComponentListBox final : public ListBox,
                                                                              private ListBoxModel
{
public:
    MidiInputSelectorComponentListBox (AudioDeviceManager& dm, const Txt& noItems)
        : ListBox ({}, nullptr),
          deviceManager (dm),
          noItemsMessage (noItems)
    {
        updateDevices();
        setModel (this);
        setOutlineThickness (1);
    }

    z0 updateDevices()
    {
        items = MidiInput::getAvailableDevices();
    }

    i32 getNumRows() override
    {
        return items.size();
    }

    z0 paintListBoxItem (i32 row, Graphics& g, i32 width, i32 height, b8 rowIsSelected) override
    {
        if (isPositiveAndBelow (row, items.size()))
        {
            if (rowIsSelected)
                g.fillAll (findColor (TextEditor::highlightColorId)
                               .withMultipliedAlpha (0.3f));

            auto item = items[row];
            b8 enabled = deviceManager.isMidiInputDeviceEnabled (item.identifier);

            auto x = getTickX();
            auto tickW = (f32) height * 0.75f;

            getLookAndFeel().drawTickBox (g, *this, (f32) x - tickW, ((f32) height - tickW) * 0.5f, tickW, tickW,
                                          enabled, true, true, false);

            drawTextLayout (g, *this, item.name, { x + 5, 0, width - x - 5, height }, enabled);
        }
    }

    z0 listBoxItemClicked (i32 row, const MouseEvent& e) override
    {
        selectRow (row);

        if (e.x < getTickX())
            flipEnablement (row);
    }

    z0 listBoxItemDoubleClicked (i32 row, const MouseEvent&) override
    {
        flipEnablement (row);
    }

    z0 returnKeyPressed (i32 row) override
    {
        flipEnablement (row);
    }

    z0 paint (Graphics& g) override
    {
        ListBox::paint (g);

        if (items.isEmpty())
        {
            g.setColor (Colors::grey);
            g.setFont (0.5f * (f32) getRowHeight());
            g.drawText (noItemsMessage,
                        0, 0, getWidth(), getHeight() / 2,
                        Justification::centred, true);
        }
    }

    i32 getBestHeight (i32 preferredHeight)
    {
        auto extra = getOutlineThickness() * 2;

        return jmax (getRowHeight() * 2 + extra,
                     jmin (getRowHeight() * getNumRows() + extra,
                           preferredHeight));
    }

private:
    //==============================================================================
    AudioDeviceManager& deviceManager;
    const Txt noItemsMessage;
    Array<MidiDeviceInfo> items;

    z0 flipEnablement (i32k row)
    {
        if (isPositiveAndBelow (row, items.size()))
        {
            auto identifier = items[row].identifier;
            deviceManager.setMidiInputDeviceEnabled (identifier, ! deviceManager.isMidiInputDeviceEnabled (identifier));
        }
    }

    i32 getTickX() const
    {
        return getRowHeight();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputSelectorComponentListBox)
};


//==============================================================================
struct AudioDeviceSetupDetails
{
    AudioDeviceManager* manager;
    i32 minNumInputChannels, maxNumInputChannels;
    i32 minNumOutputChannels, maxNumOutputChannels;
    b8 useStereoPairs;
};

static Txt getNoDeviceString()   { return "<< " + TRANS ("none") + " >>"; }

//==============================================================================
class AudioDeviceSelectorComponent::MidiOutputSelector final : public Component,
                                                               private ChangeListener
{
public:
    explicit MidiOutputSelector (AudioDeviceManager& dm)
        : deviceManager (dm)
    {
        deviceManager.addChangeListener (this);
        selector.onChange = [&]
        {
            const auto selectedId = selector.getSelectedId();
            jassert (selectedId != 0);

            const auto deviceId = selectedId == -1
                                ? Txt{}
                                : MidiOutput::getAvailableDevices()[selectedId - 1].identifier;
            deviceManager.setDefaultMidiOutputDevice (deviceId);
        };

        updateListOfDevices();
        addAndMakeVisible (selector);
    }

    ~MidiOutputSelector() final
    {
        deviceManager.removeChangeListener (this);
    }

    z0 resized() final { selector.setBounds (getLocalBounds()); }

private:
    z0 updateListOfDevices()
    {
        selector.clear();

        const auto midiOutputs = MidiOutput::getAvailableDevices();

        selector.addItem (getNoDeviceString(), -1);
        selector.setSelectedId (-1, dontSendNotification);
        selector.addSeparator();

        for (auto [id, midiOutput] : enumerate (midiOutputs, 1))
        {
            selector.addItem (midiOutput.name, id);

            if (midiOutput.identifier == deviceManager.getDefaultMidiOutputIdentifier())
                selector.setSelectedId (id, dontSendNotification);
        }
    }

    z0 changeListenerCallback (ChangeBroadcaster*) final { updateListOfDevices(); }

    ComboBox selector;
    AudioDeviceManager& deviceManager;
};

//==============================================================================
class AudioDeviceSettingsPanel : public Component,
                                 private ChangeListener
{
public:
    AudioDeviceSettingsPanel (AudioIODeviceType& t, AudioDeviceSetupDetails& setupDetails,
                              const b8 hideAdvancedOptionsWithButton,
                              AudioDeviceSelectorComponent& p)
        : type (t), setup (setupDetails), parent (p)
    {
        if (hideAdvancedOptionsWithButton)
        {
            showAdvancedSettingsButton = std::make_unique <TextButton> (TRANS ("Show advanced settings..."));
            addAndMakeVisible (showAdvancedSettingsButton.get());
            showAdvancedSettingsButton->setClickingTogglesState (true);
            showAdvancedSettingsButton->onClick = [this] { toggleAdvancedSettings(); };
        }

        type.scanForDevices();

        setup.manager->addChangeListener (this);

        updateAllControls();
    }

    ~AudioDeviceSettingsPanel() override
    {
        setup.manager->removeChangeListener (this);
    }

    z0 resized() override
    {
        Rectangle<i32> r (proportionOfWidth (0.35f), 0, proportionOfWidth (0.6f), 3000);

        i32k maxListBoxHeight = 100;
        i32k h = parent.getItemHeight();
        i32k space = h / 4;

        if (outputDeviceDropDown != nullptr)
        {
            auto row = r.removeFromTop (h);

            if (testButton != nullptr)
            {
                testButton->changeWidthToFitText (h);
                testButton->setBounds (row.removeFromRight (testButton->getWidth()));
                row.removeFromRight (space);
            }

            outputDeviceDropDown->setBounds (row);
            r.removeFromTop (space);
        }

        if (inputDeviceDropDown != nullptr)
        {
            auto row = r.removeFromTop (h);

            inputLevelMeter->setBounds (row.removeFromRight (testButton != nullptr ? testButton->getWidth() : row.getWidth() / 6));
            row.removeFromRight (space);
            inputDeviceDropDown->setBounds (row);
            r.removeFromTop (space);
        }

        if (outputChanList != nullptr)
        {
            outputChanList->setRowHeight (jmin (22, h));
            outputChanList->setBounds (r.removeFromTop (outputChanList->getBestHeight (maxListBoxHeight)));
            outputChanLabel->setBounds (0, outputChanList->getBounds().getCentreY() - h / 2, r.getX(), h);
            r.removeFromTop (space);
        }

        if (inputChanList != nullptr)
        {
            inputChanList->setRowHeight (jmin (22, h));
            inputChanList->setBounds (r.removeFromTop (inputChanList->getBestHeight (maxListBoxHeight)));
            inputChanLabel->setBounds (0, inputChanList->getBounds().getCentreY() - h / 2, r.getX(), h);
            r.removeFromTop (space);
        }

        r.removeFromTop (space * 2);

        if (showAdvancedSettingsButton != nullptr
            && sampleRateDropDown != nullptr && bufferSizeDropDown != nullptr)
        {
            showAdvancedSettingsButton->setBounds (r.removeFromTop (h));
            r.removeFromTop (space);
            showAdvancedSettingsButton->changeWidthToFitText();
        }

        auto advancedSettingsVisible = showAdvancedSettingsButton == nullptr
                                          || showAdvancedSettingsButton->getToggleState();

        if (sampleRateDropDown != nullptr)
        {
            sampleRateDropDown->setVisible (advancedSettingsVisible);

            if (advancedSettingsVisible)
            {
                sampleRateDropDown->setBounds (r.removeFromTop (h));
                r.removeFromTop (space);
            }
        }

        if (bufferSizeDropDown != nullptr)
        {
            bufferSizeDropDown->setVisible (advancedSettingsVisible);

            if (advancedSettingsVisible)
            {
                bufferSizeDropDown->setBounds (r.removeFromTop (h));
                r.removeFromTop (space);
            }
        }

        r.removeFromTop (space);

        if (showUIButton != nullptr || resetDeviceButton != nullptr)
        {
            auto buttons = r.removeFromTop (h);

            if (showUIButton != nullptr)
            {
                showUIButton->setVisible (advancedSettingsVisible);
                showUIButton->changeWidthToFitText (h);
                showUIButton->setBounds (buttons.removeFromLeft (showUIButton->getWidth()));
                buttons.removeFromLeft (space);
            }

            if (resetDeviceButton != nullptr)
            {
                resetDeviceButton->setVisible (advancedSettingsVisible);
                resetDeviceButton->changeWidthToFitText (h);
                resetDeviceButton->setBounds (buttons.removeFromLeft (resetDeviceButton->getWidth()));
            }

            r.removeFromTop (space);
        }

        setSize (getWidth(), r.getY());
    }

    z0 updateConfig (b8 updateOutputDevice, b8 updateInputDevice, b8 updateSampleRate, b8 updateBufferSize)
    {
        auto config = setup.manager->getAudioDeviceSetup();
        Txt error;

        if (updateOutputDevice || updateInputDevice)
        {
            if (outputDeviceDropDown != nullptr)
                config.outputDeviceName = outputDeviceDropDown->getSelectedId() < 0 ? Txt()
                                                                                    : outputDeviceDropDown->getText();

            if (inputDeviceDropDown != nullptr)
                config.inputDeviceName = inputDeviceDropDown->getSelectedId() < 0 ? Txt()
                                                                                  : inputDeviceDropDown->getText();

            if (! type.hasSeparateInputsAndOutputs())
                config.inputDeviceName = config.outputDeviceName;

            if (updateInputDevice)
                config.useDefaultInputChannels = true;
            else
                config.useDefaultOutputChannels = true;

            error = setup.manager->setAudioDeviceSetup (config, true);

            updateSelectedInput();
            updateSelectedOutput();
            updateControlPanelButton();
            resized();
        }
        else if (updateSampleRate)
        {
            if (sampleRateDropDown->getSelectedId() > 0)
            {
                config.sampleRate = sampleRateDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }
        else if (updateBufferSize)
        {
            if (bufferSizeDropDown->getSelectedId() > 0)
            {
                config.bufferSize = bufferSizeDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }

        if (error.isNotEmpty())
            messageBox = AlertWindow::showScopedAsync (MessageBoxOptions().withIconType (MessageBoxIconType::WarningIcon)
                                                                          .withTitle (TRANS ("Error when trying to open audio device!"))
                                                                          .withMessage (error)
                                                                          .withButton (TRANS ("OK")),
                                                       nullptr);
    }

    b8 showDeviceControlPanel()
    {
        if (auto* device = setup.manager->getCurrentAudioDevice())
        {
            Component modalWindow;
            modalWindow.setOpaque (true);
            modalWindow.addToDesktop (0);
            modalWindow.enterModalState();

            return device->showControlPanel();
        }

        return false;
    }

    z0 toggleAdvancedSettings()
    {
        showAdvancedSettingsButton->setButtonText ((showAdvancedSettingsButton->getToggleState() ? "Hide " : "Show ")
                                                   + Txt ("advanced settings..."));
        resized();
    }

    z0 showDeviceUIPanel()
    {
        if (showDeviceControlPanel())
        {
            setup.manager->closeAudioDevice();
            setup.manager->restartLastAudioDevice();
            getTopLevelComponent()->toFront (true);
        }
    }

    z0 playTestSound()
    {
        setup.manager->playTestSound();
    }

    z0 updateAllControls()
    {
        updateOutputsComboBox();
        updateInputsComboBox();

        updateControlPanelButton();
        updateResetButton();

        if (auto* currentDevice = setup.manager->getCurrentAudioDevice())
        {
            if (setup.maxNumOutputChannels > 0
                 && setup.minNumOutputChannels < setup.manager->getCurrentAudioDevice()->getOutputChannelNames().size())
            {
                if (outputChanList == nullptr)
                {
                    outputChanList = std::make_unique<ChannelSelectorListBox> (setup, ChannelSelectorListBox::audioOutputType,
                                                                               TRANS ("(no audio output channels found)"));
                    addAndMakeVisible (outputChanList.get());
                    outputChanLabel = std::make_unique<Label> (Txt{}, TRANS ("Active output channels:"));
                    outputChanLabel->setJustificationType (Justification::centredRight);
                    outputChanLabel->attachToComponent (outputChanList.get(), true);
                }

                outputChanList->refresh();
            }
            else
            {
                outputChanLabel.reset();
                outputChanList.reset();
            }

            if (setup.maxNumInputChannels > 0
                 && setup.minNumInputChannels < setup.manager->getCurrentAudioDevice()->getInputChannelNames().size())
            {
                if (inputChanList == nullptr)
                {
                    inputChanList = std::make_unique<ChannelSelectorListBox> (setup, ChannelSelectorListBox::audioInputType,
                                                                              TRANS ("(no audio input channels found)"));
                    addAndMakeVisible (inputChanList.get());
                    inputChanLabel = std::make_unique<Label> (Txt{}, TRANS ("Active input channels:"));
                    inputChanLabel->setJustificationType (Justification::centredRight);
                    inputChanLabel->attachToComponent (inputChanList.get(), true);
                }

                inputChanList->refresh();
            }
            else
            {
                inputChanLabel.reset();
                inputChanList.reset();
            }

            updateSampleRateComboBox (currentDevice);
            updateBufferSizeComboBox (currentDevice);
        }
        else
        {
            jassert (setup.manager->getCurrentAudioDevice() == nullptr); // not the correct device type!

            inputChanLabel.reset();
            outputChanLabel.reset();
            sampleRateLabel.reset();
            bufferSizeLabel.reset();

            inputChanList.reset();
            outputChanList.reset();
            sampleRateDropDown.reset();
            bufferSizeDropDown.reset();

            if (outputDeviceDropDown != nullptr)
                outputDeviceDropDown->setSelectedId (-1, dontSendNotification);

            if (inputDeviceDropDown != nullptr)
                inputDeviceDropDown->setSelectedId (-1, dontSendNotification);
        }

        sendLookAndFeelChange();
        resized();
        setSize (getWidth(), getLowestY() + 4);
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        updateAllControls();
    }

    z0 resetDevice()
    {
        setup.manager->closeAudioDevice();
        setup.manager->restartLastAudioDevice();
    }

private:
    AudioIODeviceType& type;
    const AudioDeviceSetupDetails setup;
    AudioDeviceSelectorComponent& parent;

    std::unique_ptr<ComboBox> outputDeviceDropDown, inputDeviceDropDown, sampleRateDropDown, bufferSizeDropDown;
    std::unique_ptr<Label> outputDeviceLabel, inputDeviceLabel, sampleRateLabel, bufferSizeLabel, inputChanLabel, outputChanLabel;
    std::unique_ptr<TextButton> testButton;
    std::unique_ptr<Component> inputLevelMeter;
    std::unique_ptr<TextButton> showUIButton, showAdvancedSettingsButton, resetDeviceButton;

    i32 findSelectedDeviceIndex (b8 isInput) const
    {
        const auto device = setup.manager->getAudioDeviceSetup();
        const auto deviceName = isInput ? device.inputDeviceName : device.outputDeviceName;
        return type.getDeviceNames (isInput).indexOf (deviceName);
    }

    z0 updateSelectedInput()
    {
        showCorrectDeviceName (inputDeviceDropDown.get(), true);
    }

    z0 updateSelectedOutput()
    {
        constexpr auto isInput = false;
        showCorrectDeviceName (outputDeviceDropDown.get(), isInput);

        if (testButton != nullptr)
            testButton->setEnabled (findSelectedDeviceIndex (isInput) >= 0);
    }

    z0 showCorrectDeviceName (ComboBox* box, b8 isInput)
    {
        if (box == nullptr)
            return;

        const auto index = findSelectedDeviceIndex (isInput);
        box->setSelectedId (index < 0 ? index : index + 1, dontSendNotification);
    }

    z0 addNamesToDeviceBox (ComboBox& combo, b8 isInputs)
    {
        const StringArray devs (type.getDeviceNames (isInputs));

        combo.clear (dontSendNotification);

        for (i32 i = 0; i < devs.size(); ++i)
            combo.addItem (devs[i], i + 1);

        combo.addItem (getNoDeviceString(), -1);
        combo.setSelectedId (-1, dontSendNotification);
    }

    i32 getLowestY() const
    {
        i32 y = 0;

        for (auto* c : getChildren())
            y = jmax (y, c->getBottom());

        return y;
    }

    z0 updateControlPanelButton()
    {
        auto* currentDevice = setup.manager->getCurrentAudioDevice();
        showUIButton.reset();

        if (currentDevice != nullptr && currentDevice->hasControlPanel())
        {
            showUIButton = std::make_unique<TextButton> (TRANS ("Control Panel"),
                                                         TRANS ("Opens the device's own control panel"));
            addAndMakeVisible (showUIButton.get());
            showUIButton->onClick = [this] { showDeviceUIPanel(); };
        }

        resized();
    }

    z0 updateResetButton()
    {
        if (auto* currentDevice = setup.manager->getCurrentAudioDevice())
        {
            if (currentDevice->hasControlPanel())
            {
                if (resetDeviceButton == nullptr)
                {
                    resetDeviceButton = std::make_unique<TextButton> (TRANS ("Reset Device"),
                                                                      TRANS ("Resets the audio interface - sometimes needed after changing a device's properties in its custom control panel"));
                    addAndMakeVisible (resetDeviceButton.get());
                    resetDeviceButton->onClick = [this] { resetDevice(); };
                    resized();
                }

                return;
            }
        }

        resetDeviceButton.reset();
    }

    z0 updateOutputsComboBox()
    {
        if (setup.maxNumOutputChannels > 0 || ! type.hasSeparateInputsAndOutputs())
        {
            if (outputDeviceDropDown == nullptr)
            {
                outputDeviceDropDown = std::make_unique<ComboBox>();
                outputDeviceDropDown->onChange = [this] { updateConfig (true, false, false, false); };

                addAndMakeVisible (outputDeviceDropDown.get());

                outputDeviceLabel = std::make_unique<Label> (Txt{}, type.hasSeparateInputsAndOutputs() ? TRANS ("Output:")
                                                                                                          : TRANS ("Device:"));
                outputDeviceLabel->attachToComponent (outputDeviceDropDown.get(), true);

                if (setup.maxNumOutputChannels > 0)
                {
                    testButton = std::make_unique<TextButton> (TRANS ("Test"), TRANS ("Plays a test tone"));
                    addAndMakeVisible (testButton.get());
                    testButton->onClick = [this] { playTestSound(); };
                }
            }

            addNamesToDeviceBox (*outputDeviceDropDown, false);
        }

        updateSelectedOutput();
    }

    z0 updateInputsComboBox()
    {
        if (setup.maxNumInputChannels > 0 && type.hasSeparateInputsAndOutputs())
        {
            if (inputDeviceDropDown == nullptr)
            {
                inputDeviceDropDown = std::make_unique<ComboBox>();
                inputDeviceDropDown->onChange = [this] { updateConfig (false, true, false, false); };
                addAndMakeVisible (inputDeviceDropDown.get());

                inputDeviceLabel = std::make_unique<Label> (Txt{}, TRANS ("Input:"));
                inputDeviceLabel->attachToComponent (inputDeviceDropDown.get(), true);

                inputLevelMeter = std::make_unique<SimpleDeviceManagerInputLevelMeter> (*setup.manager);
                addAndMakeVisible (inputLevelMeter.get());
            }

            addNamesToDeviceBox (*inputDeviceDropDown, true);
        }

        updateSelectedInput();
    }

    z0 updateSampleRateComboBox (AudioIODevice* currentDevice)
    {
        if (sampleRateDropDown == nullptr)
        {
            sampleRateDropDown = std::make_unique<ComboBox>();
            addAndMakeVisible (sampleRateDropDown.get());

            sampleRateLabel = std::make_unique<Label> (Txt{}, TRANS ("Sample rate:"));
            sampleRateLabel->attachToComponent (sampleRateDropDown.get(), true);
        }
        else
        {
            sampleRateDropDown->clear();
            sampleRateDropDown->onChange = nullptr;
        }

        const auto getFrequencyString = [] (i32 rate) { return Txt (rate) + " Hz"; };

        for (auto rate : currentDevice->getAvailableSampleRates())
        {
            const auto intRate = roundToInt (rate);
            sampleRateDropDown->addItem (getFrequencyString (intRate), intRate);
        }

        const auto intRate = roundToInt (currentDevice->getCurrentSampleRate());
        sampleRateDropDown->setText (getFrequencyString (intRate), dontSendNotification);

        sampleRateDropDown->onChange = [this] { updateConfig (false, false, true, false); };
    }

    z0 updateBufferSizeComboBox (AudioIODevice* currentDevice)
    {
        if (bufferSizeDropDown == nullptr)
        {
            bufferSizeDropDown = std::make_unique<ComboBox>();
            addAndMakeVisible (bufferSizeDropDown.get());

            bufferSizeLabel = std::make_unique<Label> (Txt{}, TRANS ("Audio buffer size:"));
            bufferSizeLabel->attachToComponent (bufferSizeDropDown.get(), true);
        }
        else
        {
            bufferSizeDropDown->clear();
            bufferSizeDropDown->onChange = nullptr;
        }

        auto currentRate = currentDevice->getCurrentSampleRate();

        if (exactlyEqual (currentRate, 0.0))
            currentRate = 48000.0;

        for (auto bs : currentDevice->getAvailableBufferSizes())
            bufferSizeDropDown->addItem (Txt (bs) + " samples (" + Txt (bs * 1000.0 / currentRate, 1) + " ms)", bs);

        bufferSizeDropDown->setSelectedId (currentDevice->getCurrentBufferSizeSamples(), dontSendNotification);
        bufferSizeDropDown->onChange = [this] { updateConfig (false, false, false, true); };
    }

public:
    //==============================================================================
    class ChannelSelectorListBox final : public ListBox,
                                         private ListBoxModel
    {
    public:
        enum BoxType
        {
            audioInputType,
            audioOutputType
        };

        //==============================================================================
        ChannelSelectorListBox (const AudioDeviceSetupDetails& setupDetails, BoxType boxType, const Txt& noItemsText)
           : ListBox ({}, nullptr), setup (setupDetails), type (boxType), noItemsMessage (noItemsText)
        {
            refresh();
            setModel (this);
            setOutlineThickness (1);
        }

        z0 refresh()
        {
            items.clear();

            if (auto* currentDevice = setup.manager->getCurrentAudioDevice())
            {
                if (type == audioInputType)
                    items = currentDevice->getInputChannelNames();
                else if (type == audioOutputType)
                    items = currentDevice->getOutputChannelNames();

                if (setup.useStereoPairs)
                {
                    StringArray pairs;

                    for (i32 i = 0; i < items.size(); i += 2)
                    {
                        auto& name = items[i];

                        if (i + 1 >= items.size())
                            pairs.add (name.trim());
                        else
                            pairs.add (getNameForChannelPair (name, items[i + 1]));
                    }

                    items = pairs;
                }
            }

            updateContent();
            repaint();
        }

        i32 getNumRows() override
        {
            return items.size();
        }

        z0 paintListBoxItem (i32 row, Graphics& g, i32 width, i32 height, b8) override
        {
            if (isPositiveAndBelow (row, items.size()))
            {
                g.fillAll (findColor (ListBox::backgroundColorId));

                auto item = items[row];
                b8 enabled = false;
                auto config = setup.manager->getAudioDeviceSetup();

                if (setup.useStereoPairs)
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels[row * 2] || config.inputChannels[row * 2 + 1];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels[row * 2] || config.outputChannels[row * 2 + 1];
                }
                else
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels[row];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels[row];
                }

                auto x = getTickX();
                auto tickW = (f32) height * 0.75f;

                getLookAndFeel().drawTickBox (g, *this, (f32) x - tickW, ((f32) height - tickW) * 0.5f, tickW, tickW,
                                              enabled, true, true, false);

                drawTextLayout (g, *this, item, { x + 5, 0, width - x - 5, height }, enabled);
            }
        }

        z0 listBoxItemClicked (i32 row, const MouseEvent& e) override
        {
            selectRow (row);

            if (e.x < getTickX())
                flipEnablement (row);
        }

        z0 listBoxItemDoubleClicked (i32 row, const MouseEvent&) override
        {
            flipEnablement (row);
        }

        z0 returnKeyPressed (i32 row) override
        {
            flipEnablement (row);
        }

        z0 paint (Graphics& g) override
        {
            ListBox::paint (g);

            if (items.isEmpty())
            {
                g.setColor (Colors::grey);
                g.setFont (0.5f * (f32) getRowHeight());
                g.drawText (noItemsMessage,
                            0, 0, getWidth(), getHeight() / 2,
                            Justification::centred, true);
            }
        }

        i32 getBestHeight (i32 maxHeight)
        {
            return getRowHeight() * jlimit (2, jmax (2, maxHeight / getRowHeight()),
                                            getNumRows())
                       + getOutlineThickness() * 2;
        }

    private:
        //==============================================================================
        const AudioDeviceSetupDetails setup;
        const BoxType type;
        const Txt noItemsMessage;
        StringArray items;

        static Txt getNameForChannelPair (const Txt& name1, const Txt& name2)
        {
            Txt commonBit;

            for (i32 j = 0; j < name1.length(); ++j)
                if (name1.substring (0, j).equalsIgnoreCase (name2.substring (0, j)))
                    commonBit = name1.substring (0, j);

            // Make sure we only split the name at a space, because otherwise, things
            // like "input 11" + "input 12" would become "input 11 + 2"
            while (commonBit.isNotEmpty() && ! CharacterFunctions::isWhitespace (commonBit.getLastCharacter()))
                commonBit = commonBit.dropLastCharacters (1);

            return name1.trim() + " + " + name2.substring (commonBit.length()).trim();
        }

        z0 flipEnablement (i32 row)
        {
            jassert (type == audioInputType || type == audioOutputType);

            if (isPositiveAndBelow (row, items.size()))
            {
                auto config = setup.manager->getAudioDeviceSetup();

                if (setup.useStereoPairs)
                {
                    BigInteger bits;
                    auto& original = (type == audioInputType ? config.inputChannels
                                                             : config.outputChannels);

                    for (i32 i = 0; i < 256; i += 2)
                        bits.setBit (i / 2, original[i] || original[i + 1]);

                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (bits, row, setup.minNumInputChannels / 2, setup.maxNumInputChannels / 2);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (bits, row, setup.minNumOutputChannels / 2, setup.maxNumOutputChannels / 2);
                    }

                    for (i32 i = 0; i < 256; ++i)
                        original.setBit (i, bits[i / 2]);
                }
                else
                {
                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (config.inputChannels, row, setup.minNumInputChannels, setup.maxNumInputChannels);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (config.outputChannels, row, setup.minNumOutputChannels, setup.maxNumOutputChannels);
                    }
                }

                setup.manager->setAudioDeviceSetup (config, true);
            }
        }

        static z0 flipBit (BigInteger& chans, i32 index, i32 minNumber, i32 maxNumber)
        {
            auto numActive = chans.countNumberOfSetBits();

            if (chans[index])
            {
                if (numActive > minNumber)
                    chans.setBit (index, false);
            }
            else
            {
                if (numActive >= maxNumber)
                {
                    auto firstActiveChan = chans.findNextSetBit (0);
                    chans.clearBit (index > firstActiveChan ? firstActiveChan : chans.getHighestBit());
                }

                chans.setBit (index, true);
            }
        }

        i32 getTickX() const
        {
            return getRowHeight();
        }

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelSelectorListBox)
    };

private:
    std::unique_ptr<ChannelSelectorListBox> inputChanList, outputChanList;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDeviceSettingsPanel)
};


//==============================================================================
AudioDeviceSelectorComponent::AudioDeviceSelectorComponent (AudioDeviceManager& dm,
                                                            i32 minInputChannelsToUse,
                                                            i32 maxInputChannelsToUse,
                                                            i32 minOutputChannelsToUse,
                                                            i32 maxOutputChannelsToUse,
                                                            b8 showMidiInputOptions,
                                                            b8 showMidiOutputSelector,
                                                            b8 showChannelsAsStereoPairsToUse,
                                                            b8 hideAdvancedOptionsWithButtonToUse)
    : deviceManager (dm),
      itemHeight (24),
      minOutputChannels (minOutputChannelsToUse),
      maxOutputChannels (maxOutputChannelsToUse),
      minInputChannels (minInputChannelsToUse),
      maxInputChannels (maxInputChannelsToUse),
      showChannelsAsStereoPairs (showChannelsAsStereoPairsToUse),
      hideAdvancedOptionsWithButton (hideAdvancedOptionsWithButtonToUse)
{
    jassert (minOutputChannels >= 0 && minOutputChannels <= maxOutputChannels);
    jassert (minInputChannels >= 0 && minInputChannels <= maxInputChannels);

    const OwnedArray<AudioIODeviceType>& types = deviceManager.getAvailableDeviceTypes();

    if (types.size() > 1)
    {
        deviceTypeDropDown = std::make_unique<ComboBox>();

        for (i32 i = 0; i < types.size(); ++i)
            deviceTypeDropDown->addItem (types.getUnchecked (i)->getTypeName(), i + 1);

        addAndMakeVisible (deviceTypeDropDown.get());
        deviceTypeDropDown->onChange = [this] { updateDeviceType(); };

        deviceTypeDropDownLabel = std::make_unique<Label> (Txt{}, TRANS ("Audio device type:"));
        deviceTypeDropDownLabel->setJustificationType (Justification::centredRight);
        deviceTypeDropDownLabel->attachToComponent (deviceTypeDropDown.get(), true);
    }

    if (showMidiInputOptions)
    {
        midiInputsList = std::make_unique <MidiInputSelectorComponentListBox> (deviceManager,
                                                                               "(" + TRANS ("No MIDI inputs available") + ")");
        addAndMakeVisible (midiInputsList.get());

        midiInputsLabel = std::make_unique<Label> (Txt{}, TRANS ("Active MIDI inputs:"));
        midiInputsLabel->setJustificationType (Justification::topRight);
        midiInputsLabel->attachToComponent (midiInputsList.get(), true);

        if (BluetoothMidiDevicePairingDialogue::isAvailable())
        {
            bluetoothButton = std::make_unique<TextButton> (TRANS ("Bluetooth MIDI"), TRANS ("Scan for bluetooth MIDI devices"));
            addAndMakeVisible (bluetoothButton.get());
            bluetoothButton->onClick = [this] { handleBluetoothButton(); };
        }
    }
    else
    {
        midiInputsList.reset();
        midiInputsLabel.reset();
        bluetoothButton.reset();
    }

    if (showMidiOutputSelector)
    {
        midiOutputSelector = std::make_unique<MidiOutputSelector> (deviceManager);
        addAndMakeVisible (midiOutputSelector.get());

        midiOutputLabel = std::make_unique<Label> ("lm", TRANS ("MIDI Output:"));
        midiOutputLabel->attachToComponent (midiOutputSelector.get(), true);
    }
    else
    {
        midiOutputSelector.reset();
        midiOutputLabel.reset();
    }

    deviceManager.addChangeListener (this);
    updateAllControls();
}

AudioDeviceSelectorComponent::~AudioDeviceSelectorComponent()
{
    deviceManager.removeChangeListener (this);
}

z0 AudioDeviceSelectorComponent::setItemHeight (i32 newItemHeight)
{
    itemHeight = newItemHeight;
    resized();
}

z0 AudioDeviceSelectorComponent::resized()
{
    Rectangle<i32> r (proportionOfWidth (0.35f), 15, proportionOfWidth (0.6f), 3000);
    auto space = itemHeight / 4;

    if (deviceTypeDropDown != nullptr)
    {
        deviceTypeDropDown->setBounds (r.removeFromTop (itemHeight));
        r.removeFromTop (space * 3);
    }

    if (audioDeviceSettingsComp != nullptr)
    {
        audioDeviceSettingsComp->resized();
        audioDeviceSettingsComp->setBounds (r.removeFromTop (audioDeviceSettingsComp->getHeight())
                                                .withX (0).withWidth (getWidth()));
        r.removeFromTop (space);
    }

    if (midiInputsList != nullptr)
    {
        midiInputsList->setRowHeight (jmin (22, itemHeight));
        midiInputsList->setBounds (r.removeFromTop (midiInputsList->getBestHeight (jmin (itemHeight * 8,
                                                                                         getHeight() - r.getY() - space - itemHeight))));
        r.removeFromTop (space);
    }

    if (bluetoothButton != nullptr)
    {
        bluetoothButton->setBounds (r.removeFromTop (24));
        r.removeFromTop (space);
    }

    if (midiOutputSelector != nullptr)
        midiOutputSelector->setBounds (r.removeFromTop (itemHeight));

    r.removeFromTop (itemHeight);
    setSize (getWidth(), r.getY());
}

z0 AudioDeviceSelectorComponent::childBoundsChanged (Component* child)
{
    if (child == audioDeviceSettingsComp.get())
        resized();
}

z0 AudioDeviceSelectorComponent::updateDeviceType()
{
    if (auto* type = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown->getSelectedId() - 1])
    {
        audioDeviceSettingsComp.reset();
        deviceManager.setCurrentAudioDeviceType (type->getTypeName(), true);
        updateAllControls(); // needed in case the type hasn't actually changed
    }
}

z0 AudioDeviceSelectorComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateAllControls();
}

z0 AudioDeviceSelectorComponent::updateAllControls()
{
    if (deviceTypeDropDown != nullptr)
        deviceTypeDropDown->setText (deviceManager.getCurrentAudioDeviceType(), dontSendNotification);

    if (audioDeviceSettingsComp == nullptr
         || audioDeviceSettingsCompType != deviceManager.getCurrentAudioDeviceType())
    {
        audioDeviceSettingsCompType = deviceManager.getCurrentAudioDeviceType();
        audioDeviceSettingsComp.reset();

        if (auto* type = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown == nullptr
                                                                   ? 0 : deviceTypeDropDown->getSelectedId() - 1])
        {
            AudioDeviceSetupDetails details;
            details.manager = &deviceManager;
            details.minNumInputChannels = minInputChannels;
            details.maxNumInputChannels = maxInputChannels;
            details.minNumOutputChannels = minOutputChannels;
            details.maxNumOutputChannels = maxOutputChannels;
            details.useStereoPairs = showChannelsAsStereoPairs;

            audioDeviceSettingsComp = std::make_unique<AudioDeviceSettingsPanel> (*type, details, hideAdvancedOptionsWithButton, *this);
            addAndMakeVisible (audioDeviceSettingsComp.get());
        }
    }

    if (midiInputsList != nullptr)
    {
        midiInputsList->updateDevices();
        midiInputsList->updateContent();
        midiInputsList->repaint();
    }

    resized();
}

z0 AudioDeviceSelectorComponent::handleBluetoothButton()
{
    if (RuntimePermissions::isGranted (RuntimePermissions::bluetoothMidi))
    {
        BluetoothMidiDevicePairingDialogue::open();
    }
    else
    {
        RuntimePermissions::request (RuntimePermissions::bluetoothMidi, [] (auto)
        {
            if (RuntimePermissions::isGranted (RuntimePermissions::bluetoothMidi))
                BluetoothMidiDevicePairingDialogue::open();
        });
    }
}

ListBox* AudioDeviceSelectorComponent::getMidiInputSelectorListBox() const noexcept
{
    return midiInputsList.get();
}

} // namespace drx
