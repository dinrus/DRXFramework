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

 name:             SurroundPlugin
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Surround audio plugin.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_plugin_client, drx_audio_processors,
                   drx_audio_utils, drx_core, drx_data_structures,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        SurroundProcessor

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class ProcessorWithLevels : public AudioProcessor,
                            private AsyncUpdater,
                            private Timer
{
public:
    ProcessorWithLevels()
        : AudioProcessor (BusesProperties().withInput  ("Input", AudioChannelSet::stereo())
                                           .withInput  ("Aux", AudioChannelSet::stereo(), false)
                                           .withOutput ("Output", AudioChannelSet::stereo())
                                           .withOutput ("Aux", AudioChannelSet::stereo(), false))
    {
        startTimerHz (60);
        applyBusLayouts (getBusesLayout());
    }

    ~ProcessorWithLevels() override
    {
        stopTimer();
        cancelPendingUpdate();
    }

    z0 prepareToPlay (f64, i32) override
    {
        samplesToPlay = (i32) getSampleRate();
        reset();
    }

    z0 processBlock (AudioBuffer<f32>&  audio, MidiBuffer&) override { processAudio (audio); }
    z0 processBlock (AudioBuffer<f64>& audio, MidiBuffer&) override { processAudio (audio); }

    z0 releaseResources() override { reset(); }

    f32 getLevel (i32 bus, i32 channel) const
    {
        return readableLevels[(size_t) getChannelIndexInProcessBlockBuffer (true, bus, channel)];
    }

    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto isSetValid = [] (const AudioChannelSet& set)
        {
            return ! set.isDisabled()
                   && ! (set.isDiscreteLayout() && set.getChannelIndexForType (AudioChannelSet::discreteChannel0) == -1);
        };

        return isSetValid (layouts.getMainOutputChannelSet())
               && isSetValid (layouts.getMainInputChannelSet());
    }

    z0 reset() override
    {
        channelClicked = 0;
        samplesPlayed = samplesToPlay;
    }

    b8 applyBusLayouts (const BusesLayout& layouts) final
    {
        // Some very badly-behaved hosts will call this during processing!
        const SpinLock::ScopedLockType lock (levelMutex);

        const auto result = AudioProcessor::applyBusLayouts (layouts);

        size_t numInputChannels = 0;

        for (auto i = 0; i < getBusCount (true); ++i)
            numInputChannels += (size_t) getBus (true, i)->getLastEnabledLayout().size();

        incomingLevels = readableLevels = std::vector<f32> (numInputChannels, 0.0f);

        triggerAsyncUpdate();
        return result;
    }

    //==============================================================================
    const Txt getName() const override                  { return "Surround PlugIn"; }
    b8 acceptsMidi() const override                      { return false; }
    b8 producesMidi() const override                     { return false; }
    f64 getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    i32 getNumPrograms() override                          { return 1; }
    i32 getCurrentProgram() override                       { return 0; }
    z0 setCurrentProgram (i32) override                  {}
    const Txt getProgramName (i32) override             { return "None"; }
    z0 changeProgramName (i32, const Txt&) override   {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock&) override       {}
    z0 setStateInformation (ukk, i32) override   {}

    z0 channelButtonClicked (i32 bus, i32 channelIndex)
    {
        channelClicked = getChannelIndexInProcessBlockBuffer (false, bus, channelIndex);
        samplesPlayed = 0;
    }

    std::function<z0()> updateEditor;

private:
    z0 handleAsyncUpdate() override
    {
        NullCheckedInvocation::invoke (updateEditor);
    }

    template <typename Float>
    z0 processAudio (AudioBuffer<Float>& audio)
    {
        {
            SpinLock::ScopedTryLockType lock (levelMutex);

            if (lock.isLocked())
            {
                const auto numInputChannels = (size_t) getTotalNumInputChannels();

                for (size_t i = 0; i < numInputChannels; ++i)
                {
                    const auto minMax = audio.findMinMax ((i32) i, 0, audio.getNumSamples());
                    const auto newMax = (f32) std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd()));

                    auto& toUpdate = incomingLevels[i];
                    toUpdate = jmax (toUpdate, newMax);
                }
            }
        }

        audio.clear (0, audio.getNumSamples());

        auto fillSamples = jmin (samplesToPlay - samplesPlayed, audio.getNumSamples());

        if (isPositiveAndBelow (channelClicked, audio.getNumChannels()))
        {
            auto* channelBuffer = audio.getWritePointer (channelClicked);
            auto freq = (f32) (440.0 / getSampleRate());

            for (auto i = 0; i < fillSamples; ++i)
                channelBuffer[i] += std::sin (MathConstants<f32>::twoPi * freq * (f32) samplesPlayed++);
        }
    }

    z0 timerCallback() override
    {
        const SpinLock::ScopedLockType lock (levelMutex);

        for (size_t i = 0; i < readableLevels.size(); ++i)
            readableLevels[i] = std::max (readableLevels[i] * 0.95f, std::exchange (incomingLevels[i], 0.0f));
    }

    SpinLock levelMutex;
    std::vector<f32> incomingLevels;
    std::vector<f32> readableLevels;

    i32 channelClicked;
    i32 samplesPlayed;
    i32 samplesToPlay;
};

//==============================================================================
const Color textColor = Colors::white.withAlpha (0.8f);

inline z0 drawBackground (Component& comp, Graphics& g)
{
    g.setColor (comp.getLookAndFeel().findColor (ResizableWindow::backgroundColorId).darker (0.8f));
    g.fillRoundedRectangle (comp.getLocalBounds().toFloat(), 4.0f);
}

inline z0 configureLabel (Label& label, const AudioProcessor::Bus* layout)
{
    const auto text = layout != nullptr
                          ? (layout->getName() + ": " + layout->getCurrentLayout().getDescription())
                          : "";
    label.setText (text, dontSendNotification);
    label.setJustificationType (Justification::centred);
    label.setColor (Label::textColorId, textColor);
}

class InputBusViewer final : public Component,
                             private Timer
{
public:
    InputBusViewer (ProcessorWithLevels& proc, i32 busNumber)
        : processor (proc),
          bus (busNumber)
    {
        configureLabel (layoutName, processor.getBus (true, bus));
        addAndMakeVisible (layoutName);

        startTimerHz (60);
    }

    ~InputBusViewer() override
    {
        stopTimer();
    }

    z0 paint (Graphics& g) override
    {
        drawBackground (*this, g);

        auto* layout = processor.getBus (true, bus);

        if (layout == nullptr)
            return;

        const auto channelSet = layout->getCurrentLayout();
        const auto numChannels = channelSet.size();

        Grid grid;

        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo (Grid::Fr (1));
        grid.items.insertMultiple (0, GridItem(), numChannels);
        grid.performLayout (getLocalBounds());

        const auto minDb = -50.0f;
        const auto maxDb = 6.0f;

        for (auto i = 0; i < numChannels; ++i)
        {
            g.setColor (Colors::orange.darker());

            const auto levelInDb = Decibels::gainToDecibels (processor.getLevel (bus, i), minDb);
            const auto fractionOfHeight = jmap (levelInDb, minDb, maxDb, 0.0f, 1.0f);
            const auto bounds = grid.items[i].currentBounds;
            const auto trackBounds = bounds.withSizeKeepingCentre (16, bounds.getHeight() - 10).toFloat();
            g.fillRect (trackBounds.withHeight (trackBounds.proportionOfHeight (fractionOfHeight)).withBottomY (trackBounds.getBottom()));

            g.setColor (textColor);

            g.drawText (channelSet.getAbbreviatedChannelTypeName (channelSet.getTypeOfChannel (i)),
                        bounds,
                        Justification::centredBottom);
        }
    }

    z0 resized() override
    {
        layoutName.setBounds (getLocalBounds().removeFromTop (20));
    }

    i32 getNumChannels() const
    {
        if (auto* b = processor.getBus (true, bus))
            return b->getCurrentLayout().size();

        return 0;
    }

private:
    z0 timerCallback() override { repaint(); }

    ProcessorWithLevels& processor;
    i32 bus = 0;
    Label layoutName;
};

//==============================================================================
class OutputBusViewer final : public Component
{
public:
    OutputBusViewer (ProcessorWithLevels& proc, i32 busNumber)
        : processor (proc),
          bus (busNumber)
    {
        auto* layout = processor.getBus (false, bus);

        configureLabel (layoutName, layout);
        addAndMakeVisible (layoutName);

        if (layout == nullptr)
            return;

        const auto& channelSet = layout->getCurrentLayout();

        const auto numChannels = channelSet.size();

        for (auto i = 0; i < numChannels; ++i)
        {
            const auto channelName = channelSet.getAbbreviatedChannelTypeName (channelSet.getTypeOfChannel (i));

            channelButtons.emplace_back (channelName, channelName);

            auto& newButton = channelButtons.back();
            newButton.onClick = [&p = processor, b = bus, i] { p.channelButtonClicked (b, i); };
            addAndMakeVisible (newButton);
        }

        resized();
    }

    z0 paint (Graphics& g) override
    {
        drawBackground (*this, g);
    }

    z0 resized() override
    {
        auto b = getLocalBounds();

        layoutName.setBounds (b.removeFromBottom (20));

        Grid grid;
        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo (Grid::Fr (1));

        for (auto& channelButton : channelButtons)
            grid.items.add (GridItem (channelButton));

        grid.performLayout (b.reduced (2));
    }

    i32 getNumChannels() const
    {
        if (auto* b = processor.getBus (false, bus))
            return b->getCurrentLayout().size();

        return 0;
    }

private:
    ProcessorWithLevels& processor;
    i32 bus = 0;
    Label layoutName;
    std::list<TextButton> channelButtons;
};

//==============================================================================
class SurroundEditor final : public AudioProcessorEditor
{
public:
    explicit SurroundEditor (ProcessorWithLevels& parent)
        : AudioProcessorEditor (parent),
          customProcessor (parent),
          scopedUpdateEditor (customProcessor.updateEditor, [this] { updateGUI(); })
    {
        updateGUI();
        setResizable (true, true);
    }

    z0 resized() override
    {
        auto r = getLocalBounds();
        doLayout (inputViewers, r.removeFromTop (proportionOfHeight (0.5f)));
        doLayout (outputViewers, r);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
    }

private:
    template <typename Range>
    z0 doLayout (Range& range, Rectangle<i32> bounds) const
    {
        FlexBox fb;

        for (auto& viewer : range)
        {
            if (viewer.getNumChannels() != 0)
            {
                fb.items.add (FlexItem (viewer)
                                  .withFlex ((f32) viewer.getNumChannels())
                                  .withMargin (4.0f));
            }
        }

        fb.performLayout (bounds);
    }

    z0 updateGUI()
    {
        inputViewers.clear();
        outputViewers.clear();

        const auto inputBuses = getAudioProcessor()->getBusCount (true);

        for (auto i = 0; i < inputBuses; ++i)
        {
            inputViewers.emplace_back (customProcessor, i);
            addAndMakeVisible (inputViewers.back());
        }

        const auto outputBuses = getAudioProcessor()->getBusCount (false);

        for (auto i = 0; i < outputBuses; ++i)
        {
            outputViewers.emplace_back (customProcessor, i);
            addAndMakeVisible (outputViewers.back());
        }

        const auto channels = jmax (processor.getTotalNumInputChannels(),
                                    processor.getTotalNumOutputChannels());
        setSize (jmax (150, channels * 40), 200);

        resized();
    }

    ProcessorWithLevels& customProcessor;
    ScopedValueSetter<std::function<z0()>> scopedUpdateEditor;
    std::list<InputBusViewer> inputViewers;
    std::list<OutputBusViewer> outputViewers;
};

//==============================================================================
struct SurroundProcessor final : public ProcessorWithLevels
{
    AudioProcessorEditor* createEditor() override { return new SurroundEditor (*this); }
    b8 hasEditor() const override               { return true; }
};
