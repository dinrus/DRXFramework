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

 name:             MPEDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simple MPE synthesiser application.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MPEDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class ZoneColorPicker
{
public:
    ZoneColorPicker() {}

    //==============================================================================
    Color getColorForMidiChannel (i32 midiChannel) noexcept
    {
        if (legacyModeEnabled)
            return Colors::white;

        if (zoneLayout.getLowerZone().isUsingChannelAsMemberChannel (midiChannel))
            return getColorForZone (true);

        if (zoneLayout.getUpperZone().isUsingChannelAsMemberChannel (midiChannel))
            return getColorForZone (false);

        return Colors::transparentBlack;
    }

    //==============================================================================
    Color getColorForZone (b8 isLowerZone) const noexcept
    {
        if (legacyModeEnabled)
            return Colors::white;

        if (isLowerZone)
            return Colors::blue;

        return Colors::red;
    }

    //==============================================================================
    z0 setZoneLayout (MPEZoneLayout layout) noexcept          { zoneLayout = layout; }
    z0 setLegacyModeEnabled (b8 shouldBeEnabled) noexcept   { legacyModeEnabled = shouldBeEnabled; }

private:
    //==============================================================================
    MPEZoneLayout zoneLayout;
    b8 legacyModeEnabled = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZoneColorPicker)
};

//==============================================================================
class MPESetupComponent final : public Component
{
public:
    //==============================================================================
    MPESetupComponent (MPEInstrument& instr)
        : instrument (instr)
    {
        addAndMakeVisible (isLowerZoneButton);
        isLowerZoneButton.setToggleState (true, NotificationType::dontSendNotification);

        initialiseComboBoxWithConsecutiveIntegers (memberChannels, memberChannelsLabel, 0, 16, defaultMemberChannels);
        initialiseComboBoxWithConsecutiveIntegers (masterPitchbendRange, masterPitchbendRangeLabel, 0, 96, defaultMasterPitchbendRange);
        initialiseComboBoxWithConsecutiveIntegers (notePitchbendRange, notePitchbendRangeLabel, 0, 96, defaultNotePitchbendRange);

        initialiseComboBoxWithConsecutiveIntegers (legacyStartChannel, legacyStartChannelLabel, 1, 16, 1, false);
        initialiseComboBoxWithConsecutiveIntegers (legacyEndChannel, legacyEndChannelLabel, 1, 16, 16, false);
        initialiseComboBoxWithConsecutiveIntegers (legacyPitchbendRange, legacyPitchbendRangeLabel, 0, 96, 2, false);

        addAndMakeVisible (setZoneButton);
        setZoneButton.onClick = [this] { setZoneButtonClicked(); };

        addAndMakeVisible (clearAllZonesButton);
        clearAllZonesButton.onClick = [this] { clearAllZonesButtonClicked(); };

        addAndMakeVisible (legacyModeEnabledToggle);
        legacyModeEnabledToggle.onClick = [this] { legacyModeEnabledToggleClicked(); };

        addAndMakeVisible (voiceStealingEnabledToggle);
        voiceStealingEnabledToggle.onClick = [this] { voiceStealingEnabledToggleClicked(); };

        initialiseComboBoxWithConsecutiveIntegers (numberOfVoices, numberOfVoicesLabel, 1, 20, 15);
    }

    //==============================================================================
    z0 resized() override
    {
        Rectangle<i32> r (proportionOfWidth (0.65f), 15, proportionOfWidth (0.25f), 3000);
        auto h = 24;
        auto hspace = 6;
        auto hbigspace = 18;

        isLowerZoneButton.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        memberChannels.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        notePitchbendRange.setBounds (r.removeFromTop (h));
        r.removeFromTop (hspace);
        masterPitchbendRange.setBounds (r.removeFromTop (h));

        legacyStartChannel  .setBounds (isLowerZoneButton .getBounds());
        legacyEndChannel    .setBounds (memberChannels    .getBounds());
        legacyPitchbendRange.setBounds (notePitchbendRange.getBounds());

        r.removeFromTop (hbigspace);

        auto buttonLeft = proportionOfWidth (0.5f);

        setZoneButton.setBounds (r.removeFromTop (h).withLeft (buttonLeft));
        r.removeFromTop (hspace);
        clearAllZonesButton.setBounds (r.removeFromTop (h).withLeft (buttonLeft));

        r.removeFromTop (hbigspace);

        auto toggleLeft = proportionOfWidth (0.25f);

        legacyModeEnabledToggle.setBounds (r.removeFromTop (h).withLeft (toggleLeft));
        r.removeFromTop (hspace);
        voiceStealingEnabledToggle.setBounds (r.removeFromTop (h).withLeft (toggleLeft));
        r.removeFromTop (hspace);
        numberOfVoices.setBounds (r.removeFromTop (h));
    }

    //==============================================================================
    b8 isVoiceStealingEnabled() const  { return voiceStealingEnabledToggle.getToggleState(); }
    i32 getNumVoices() const             { return numberOfVoices.getText().getIntValue(); }

    std::function<z0()> onSynthParametersChange;

private:
    //==============================================================================
    z0 initialiseComboBoxWithConsecutiveIntegers (ComboBox& comboBox, Label& labelToAttach,
                                                    i32 firstValue, i32 numValues, i32 valueToSelect,
                                                    b8 makeVisible = true)
    {
        for (auto i = 0; i < numValues; ++i)
            comboBox.addItem (Txt (i + firstValue), i + 1);

        comboBox.setSelectedId (valueToSelect - firstValue + 1);
        labelToAttach.attachToComponent (&comboBox, true);

        if (makeVisible)
            addAndMakeVisible (comboBox);
        else
            addChildComponent (comboBox);

        if (&comboBox == &numberOfVoices)
            comboBox.onChange = [this] { numberOfVoicesChanged(); };
        else if (&comboBox == &legacyPitchbendRange)
            comboBox.onChange = [this] { if (legacyModeEnabledToggle.getToggleState()) legacyModePitchbendRangeChanged(); };
        else if (&comboBox == &legacyStartChannel || &comboBox == &legacyEndChannel)
            comboBox.onChange = [this] { if (legacyModeEnabledToggle.getToggleState()) legacyModeChannelRangeChanged(); };
    }

    //==============================================================================
    z0 setZoneButtonClicked()
    {
        auto isLowerZone = isLowerZoneButton.getToggleState();
        auto numMemberChannels = memberChannels.getText().getIntValue();
        auto perNotePb = notePitchbendRange.getText().getIntValue();
        auto masterPb = masterPitchbendRange.getText().getIntValue();

        auto zoneLayout = instrument.getZoneLayout();

        if (isLowerZone)
            zoneLayout.setLowerZone (numMemberChannels, perNotePb, masterPb);
        else
            zoneLayout.setUpperZone (numMemberChannels, perNotePb, masterPb);

        instrument.setZoneLayout (zoneLayout);
    }

    z0 clearAllZonesButtonClicked()
    {
        instrument.setZoneLayout ({});
    }

    z0 legacyModeEnabledToggleClicked()
    {
        auto legacyModeEnabled = legacyModeEnabledToggle.getToggleState();

        isLowerZoneButton   .setVisible (! legacyModeEnabled);
        memberChannels      .setVisible (! legacyModeEnabled);
        notePitchbendRange  .setVisible (! legacyModeEnabled);
        masterPitchbendRange.setVisible (! legacyModeEnabled);
        setZoneButton       .setVisible (! legacyModeEnabled);
        clearAllZonesButton .setVisible (! legacyModeEnabled);

        legacyStartChannel  .setVisible (legacyModeEnabled);
        legacyEndChannel    .setVisible (legacyModeEnabled);
        legacyPitchbendRange.setVisible (legacyModeEnabled);

        if (legacyModeEnabled)
        {
            if (areLegacyModeParametersValid())
            {
                instrument.enableLegacyMode();

                instrument.setLegacyModeChannelRange   (getLegacyModeChannelRange());
                instrument.setLegacyModePitchbendRange (getLegacyModePitchbendRange());
            }
            else
            {
                handleInvalidLegacyModeParameters();
            }
        }
        else
        {
            instrument.setZoneLayout ({ MPEZone (MPEZone::Type::lower, 15) });
        }
    }

    //==============================================================================
    z0 legacyModePitchbendRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        instrument.setLegacyModePitchbendRange (getLegacyModePitchbendRange());
    }

    z0 legacyModeChannelRangeChanged()
    {
        jassert (legacyModeEnabledToggle.getToggleState() == true);

        if (areLegacyModeParametersValid())
            instrument.setLegacyModeChannelRange (getLegacyModeChannelRange());
        else
            handleInvalidLegacyModeParameters();
    }

    b8 areLegacyModeParametersValid() const
    {
        return legacyStartChannel.getText().getIntValue() <= legacyEndChannel.getText().getIntValue();
    }

    z0 handleInvalidLegacyModeParameters()
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Invalid legacy mode channel layout",
                                                         "Cannot set legacy mode start/end channel:\n"
                                                         "The end channel must not be less than the start channel!",
                                                         "Got it");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    Range<i32> getLegacyModeChannelRange() const
    {
        return { legacyStartChannel.getText().getIntValue(),
                 legacyEndChannel.getText().getIntValue() + 1 };
    }

    i32 getLegacyModePitchbendRange() const
    {
        return legacyPitchbendRange.getText().getIntValue();
    }

    //==============================================================================
    z0 voiceStealingEnabledToggleClicked()
    {
        jassert (onSynthParametersChange != nullptr);
        onSynthParametersChange();
    }

    z0 numberOfVoicesChanged()
    {
        jassert (onSynthParametersChange != nullptr);
        onSynthParametersChange();
    }

    //==============================================================================
    MPEInstrument& instrument;

    ComboBox memberChannels, masterPitchbendRange, notePitchbendRange;

    ToggleButton isLowerZoneButton  { "Lower zone" };

    Label memberChannelsLabel       { {}, "Nr. of member channels:" };
    Label masterPitchbendRangeLabel { {}, "Master pitchbend range (semitones):" };
    Label notePitchbendRangeLabel   { {}, "Note pitchbend range (semitones):" };

    TextButton setZoneButton        { "Set zone" };
    TextButton clearAllZonesButton  { "Clear all zones" };

    ComboBox legacyStartChannel, legacyEndChannel, legacyPitchbendRange;

    Label legacyStartChannelLabel   { {}, "First channel:" };
    Label legacyEndChannelLabel     { {}, "Last channel:" };
    Label legacyPitchbendRangeLabel { {}, "Pitchbend range (semitones):"};

    ToggleButton legacyModeEnabledToggle    { "Enable Legacy Mode" };
    ToggleButton voiceStealingEnabledToggle { "Enable synth voice stealing" };

    ComboBox numberOfVoices;
    Label numberOfVoicesLabel { {}, "Number of synth voices"};

    ScopedMessageBox messageBox;

    static constexpr i32 defaultMemberChannels       = 15,
                         defaultMasterPitchbendRange = 2,
                         defaultNotePitchbendRange   = 48;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESetupComponent)
};

//==============================================================================
class ZoneLayoutComponent final : public Component,
                                  private MPEInstrument::Listener
{
public:
    //==============================================================================
    ZoneLayoutComponent (MPEInstrument& instr, ZoneColorPicker& zoneColorPicker)
        : instrument (instr),
          colourPicker (zoneColorPicker)
    {
        instrument.addListener (this);
    }

    ~ZoneLayoutComponent() override
    {
        instrument.removeListener (this);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        paintBackground (g);

        if (instrument.isLegacyModeEnabled())
            paintLegacyMode (g);
        else
            paintZones (g);
    }

private:
    //==============================================================================
    z0 zoneLayoutChanged() override
    {
        repaint();
    }

    //==============================================================================
    z0 paintBackground (Graphics& g)
    {
        g.setColor (Colors::black);
        auto channelWidth = getChannelRectangleWidth();

        for (auto i = 0; i < numMidiChannels; ++i)
        {
            auto x = f32 (i) * channelWidth;
            Rectangle<i32> channelArea ((i32) x, 0, (i32) channelWidth, getHeight());

            g.drawLine ({ x, 0.0f, x, f32 (getHeight()) });
            g.drawText (Txt (i + 1), channelArea.reduced (4, 4), Justification::topLeft, false);
        }
    }

    //==============================================================================
    z0 paintZones (Graphics& g)
    {
        auto channelWidth = getChannelRectangleWidth();

        auto zoneLayout = instrument.getZoneLayout();

        Array<MPEZoneLayout::Zone> activeZones;
        if (zoneLayout.getLowerZone().isActive())  activeZones.add (zoneLayout.getLowerZone());
        if (zoneLayout.getUpperZone().isActive())  activeZones.add (zoneLayout.getUpperZone());

        for (auto zone : activeZones)
        {
            auto zoneColor = colourPicker.getColorForZone (zone.isLowerZone());

            auto xPos = zone.isLowerZone() ? 0 : zone.getLastMemberChannel() - 1;

            Rectangle<i32> zoneRect { i32 (channelWidth * (f32) xPos), 20,
                                      i32 (channelWidth * (f32) (zone.numMemberChannels + 1)), getHeight() - 20 };

            g.setColor (zoneColor);
            g.drawRect (zoneRect, 3);

            auto masterRect = zone.isLowerZone() ? zoneRect.removeFromLeft ((i32) channelWidth) : zoneRect.removeFromRight ((i32) channelWidth);

            g.setColor (zoneColor.withAlpha (0.3f));
            g.fillRect (masterRect);

            g.setColor (zoneColor.contrasting());
            g.drawText ("<>" + Txt (zone.masterPitchbendRange),  masterRect.reduced (4), Justification::top,    false);
            g.drawText ("<>" + Txt (zone.perNotePitchbendRange), masterRect.reduced (4), Justification::bottom, false);
        }
    }

    //==============================================================================
    z0 paintLegacyMode (Graphics& g)
    {
        auto channelRange = instrument.getLegacyModeChannelRange();
        auto startChannel = channelRange.getStart() - 1;
        auto numChannels  = channelRange.getEnd() - startChannel - 1;

        Rectangle<i32> zoneRect (i32 (getChannelRectangleWidth() * (f32) startChannel), 0,
                                 i32 (getChannelRectangleWidth() * (f32) numChannels), getHeight());

        zoneRect.removeFromTop (20);

        g.setColor (Colors::white);
        g.drawRect (zoneRect, 3);
        g.drawText ("LGCY", zoneRect.reduced (4, 4), Justification::topLeft, false);
        g.drawText ("<>" + Txt (instrument.getLegacyModePitchbendRange()), zoneRect.reduced (4, 4), Justification::bottomLeft, false);
    }

    //==============================================================================
    f32 getChannelRectangleWidth() const noexcept
    {
        return (f32) getWidth() / (f32) numMidiChannels;
    }

    //==============================================================================
    static constexpr i32 numMidiChannels = 16;

    MPEInstrument& instrument;
    ZoneColorPicker& colourPicker;
};

//==============================================================================
class MPEDemoSynthVoice final : public MPESynthesiserVoice
{
public:
    //==============================================================================
    MPEDemoSynthVoice() {}

    //==============================================================================
    z0 noteStarted() override
    {
        jassert (currentlyPlayingNote.isValid());
        jassert (currentlyPlayingNote.keyState == MPENote::keyDown
                 || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

        level    .setTargetValue (currentlyPlayingNote.pressure.asUnsignedFloat());
        frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
        timbre   .setTargetValue (currentlyPlayingNote.timbre.asUnsignedFloat());

        phase = 0.0;
        auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = MathConstants<f64>::twoPi * cyclesPerSample;

        tailOff = 0.0;
    }

    z0 noteStopped (b8 allowTailOff) override
    {
        jassert (currentlyPlayingNote.keyState == MPENote::off);

        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (approximatelyEqual (tailOff, 0.0)) // we only need to begin a tail-off if it's not already doing so - the
                                                   // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            clearCurrentNote();
            phaseDelta = 0.0;
        }
    }

    z0 notePressureChanged() override
    {
        level.setTargetValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    }

    z0 notePitchbendChanged() override
    {
        frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
    }

    z0 noteTimbreChanged() override
    {
        timbre.setTargetValue (currentlyPlayingNote.timbre.asUnsignedFloat());
    }

    z0 noteKeyStateChanged() override {}

    z0 setCurrentSampleRate (f64 newRate) override
    {
        if (! approximatelyEqual (currentSampleRate, newRate))
        {
            noteStopped (false);
            currentSampleRate = newRate;

            level    .reset (currentSampleRate, smoothingLengthInSeconds);
            timbre   .reset (currentSampleRate, smoothingLengthInSeconds);
            frequency.reset (currentSampleRate, smoothingLengthInSeconds);
        }
    }

    //==============================================================================
    virtual z0 renderNextBlock (AudioBuffer<f32>& outputBuffer,
                                  i32 startSample,
                                  i32 numSamples) override
    {
        if (! approximatelyEqual (phaseDelta, 0.0))
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = getNextSample() * (f32) tailOff;

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        phaseDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = getNextSample();

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;
                }
            }
        }
    }

    using MPESynthesiserVoice::renderNextBlock;

private:
    //==============================================================================
    f32 getNextSample() noexcept
    {
        auto levelDb = (level.getNextValue() - 1.0) * maxLevelDb;
        auto amplitude = pow (10.0f, 0.05f * levelDb) * maxLevel;

        // timbre is used to blend between a sine and a square.
        auto f1 = std::sin (phase);
        auto f2 = copysign (1.0, f1);
        auto a2 = timbre.getNextValue();
        auto a1 = 1.0 - a2;

        auto nextSample = f32 (amplitude * ((a1 * f1) + (a2 * f2)));

        auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = MathConstants<f64>::twoPi * cyclesPerSample;
        phase = std::fmod (phase + phaseDelta, MathConstants<f64>::twoPi);

        return nextSample;
    }

    //==============================================================================
    SmoothedValue<f64> level, timbre, frequency;

    f64 phase      = 0.0;
    f64 phaseDelta = 0.0;
    f64 tailOff    = 0.0;

    const f64 maxLevel   = 0.05;
    const f64 maxLevelDb = 31.0;
    const f64 smoothingLengthInSeconds = 0.01;
};

//==============================================================================
class MPEDemo final : public Component,
                      private AudioIODeviceCallback,
                      private MidiInputCallback,
                      private MPEInstrument::Listener
{
public:
    //==============================================================================
    MPEDemo()
    {
       #ifndef DRX_DEMO_RUNNER
        audioDeviceManager.initialise (0, 2, nullptr, true, {}, nullptr);
       #endif

        audioDeviceManager.addMidiInputDeviceCallback ({}, this);
        audioDeviceManager.addAudioCallback (this);

        addAndMakeVisible (audioSetupComp);
        addAndMakeVisible (mpeSetupComp);
        addAndMakeVisible (zoneLayoutComp);
        addAndMakeVisible (keyboardComponent);

        synth.setVoiceStealingEnabled (false);
        for (auto i = 0; i < 15; ++i)
            synth.addVoice (new MPEDemoSynthVoice());

        mpeSetupComp.onSynthParametersChange = [this]
        {
            synth.setVoiceStealingEnabled (mpeSetupComp.isVoiceStealingEnabled());

            auto numVoices = mpeSetupComp.getNumVoices();

            if (numVoices < synth.getNumVoices())
            {
                synth.reduceNumVoices (numVoices);
            }
            else
            {
                while (synth.getNumVoices() < numVoices)
                    synth.addVoice (new MPEDemoSynthVoice());
            }
        };

        instrument.addListener (this);

        setSize (880, 720);
    }

    ~MPEDemo() override
    {
        audioDeviceManager.removeMidiInputDeviceCallback ({}, this);
        audioDeviceManager.removeAudioCallback (this);
    }

    //==============================================================================
    z0 resized() override
    {
        auto zoneLayoutCompHeight = 60;
        auto audioSetupCompRelativeWidth = 0.55f;

        auto r = getLocalBounds();

        keyboardComponent.setBounds (r.removeFromBottom (150));
        r.reduce (10, 10);

        zoneLayoutComp.setBounds (r.removeFromBottom (zoneLayoutCompHeight));
        audioSetupComp.setBounds (r.removeFromLeft (proportionOfWidth (audioSetupCompRelativeWidth)));
        mpeSetupComp  .setBounds (r);
    }

    //==============================================================================
    z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData, i32 numInputChannels,
                                           f32* const* outputChannelData, i32 numOutputChannels,
                                           i32 numSamples, const AudioIODeviceCallbackContext& context) override
    {
        ignoreUnused (inputChannelData, numInputChannels, context);

        AudioBuffer<f32> buffer (outputChannelData, numOutputChannels, numSamples);
        buffer.clear();

        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }

    z0 audioDeviceAboutToStart (AudioIODevice* device) override
    {
        auto sampleRate = device->getCurrentSampleRate();
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    z0 audioDeviceStopped() override {}

private:
    //==============================================================================
    z0 handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message) override
    {
        instrument.processNextMidiEvent (message);
        midiCollector.addMessageToQueue (message);
    }

    //==============================================================================
    z0 zoneLayoutChanged() override
    {
        if (instrument.isLegacyModeEnabled())
        {
            colourPicker.setLegacyModeEnabled (true);

            synth.enableLegacyMode (instrument.getLegacyModePitchbendRange(),
                                    instrument.getLegacyModeChannelRange());
        }
        else
        {
            colourPicker.setLegacyModeEnabled (false);

            auto zoneLayout = instrument.getZoneLayout();

            if (auto* midiOutput = audioDeviceManager.getDefaultMidiOutput())
                midiOutput->sendBlockOfMessagesNow (MPEMessages::setZoneLayout (zoneLayout));

            synth.setZoneLayout (zoneLayout);
            colourPicker.setZoneLayout (zoneLayout);
        }
    }

    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    AudioDeviceSelectorComponent audioSetupComp { audioDeviceManager, 0, 0, 0, 256, true, true, true, false };
    MidiMessageCollector midiCollector;

    MPEInstrument instrument  { MPEZone (MPEZone::Type::lower, 15) };

    ZoneColorPicker colourPicker;
    MPESetupComponent mpeSetupComp      { instrument };
    ZoneLayoutComponent zoneLayoutComp  { instrument, colourPicker};

    MPESynthesiser synth                   { instrument };
    MPEKeyboardComponent keyboardComponent { instrument, MPEKeyboardComponent::horizontalKeyboard };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPEDemo)
};
