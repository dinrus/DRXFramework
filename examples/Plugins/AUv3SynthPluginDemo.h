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

 name:                  AUv3SynthPlugin
 version:               1.0.0
 vendor:                DRX
 website:               http://drx.com
 description:           AUv3 synthesiser audio plugin.

 dependencies:          drx_audio_basics, drx_audio_devices, drx_audio_formats,
                        drx_audio_plugin_client, drx_audio_processors,
                        drx_audio_utils, drx_core, drx_data_structures,
                        drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:             xcode_mac, xcode_iphone

 moduleFlags:           DRX_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             AUv3SynthProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn
 extraPluginFormats:    AUv3

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class MaterialLookAndFeel final : public LookAndFeel_V4
{
public:
    //==============================================================================
    MaterialLookAndFeel()
    {
        setColor (ResizableWindow::backgroundColorId, windowBackgroundColor);
        setColor (TextButton::buttonOnColorId,        brightButtonColor);
        setColor (TextButton::buttonColorId,          disabledButtonColor);
    }

    //==============================================================================
    z0 drawButtonBackground (Graphics& g,
                               Button& button,
                               const Color& /*backgroundColor*/,
                               b8 /*isMouseOverButton*/,
                               b8 isButtonDown) override
    {
        auto buttonRect = button.getLocalBounds().toFloat();

        if (isButtonDown)
            g.setColor (brightButtonColor.withAlpha (0.7f));
        else if (! button.isEnabled())
            g.setColor (disabledButtonColor);
        else
            g.setColor (brightButtonColor);

        g.fillRoundedRectangle (buttonRect, 5.0f);
    }

    //==============================================================================
    z0 drawButtonText (Graphics& g, TextButton& button, b8 isMouseOverButton, b8 isButtonDown) override
    {
        ignoreUnused (isMouseOverButton, isButtonDown);

        Font font (getTextButtonFont (button, button.getHeight()));
        g.setFont (font);

        if (button.isEnabled())
            g.setColor (Colors::white);
        else
            g.setColor (backgroundColor);

        g.drawFittedText (button.getButtonText(), 0, 0,
                          button.getWidth(),
                          button.getHeight(),
                          Justification::centred, 2);
    }

    //==============================================================================
    z0 drawLinearSlider (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                           Slider::SliderStyle style, Slider& slider) override
    {
        ignoreUnused (style, minSliderPos, maxSliderPos);

        auto r = Rectangle<i32> (x + haloRadius, y, width - (haloRadius * 2), height);
        auto backgroundBar = r.withSizeKeepingCentre (r.getWidth(), 2);

        sliderPos = (sliderPos - minSliderPos) / static_cast<f32> (width);

        auto knobPos = static_cast<i32> (sliderPos * (f32) r.getWidth());

        g.setColor (sliderActivePart);
        g.fillRect (backgroundBar.removeFromLeft (knobPos));

        g.setColor (sliderInactivePart);
        g.fillRect (backgroundBar);

        if (slider.isMouseOverOrDragging())
        {
            auto haloBounds = r.withTrimmedLeft (knobPos - haloRadius)
                               .withWidth (haloRadius * 2)
                               .withSizeKeepingCentre (haloRadius * 2, haloRadius * 2);

            g.setColor (sliderActivePart.withAlpha (0.5f));
            g.fillEllipse (haloBounds.toFloat());
        }

        auto knobRadius = slider.isMouseOverOrDragging() ? knobActiveRadius : knobInActiveRadius;
        auto knobBounds = r.withTrimmedLeft (knobPos - knobRadius)
                           .withWidth (knobRadius * 2)
                           .withSizeKeepingCentre (knobRadius * 2, knobRadius * 2);

        g.setColor (sliderActivePart);
        g.fillEllipse (knobBounds.toFloat());
    }

    //==============================================================================
    Font getTextButtonFont (TextButton& button, i32 buttonHeight) override
    {
        return LookAndFeel_V3::getTextButtonFont (button, buttonHeight).withHeight (buttonFontSize);
    }

    Font getLabelFont (Label& label) override
    {
        return LookAndFeel_V3::getLabelFont (label).withHeight (labelFontSize);
    }

    //==============================================================================
    enum
    {
        labelFontSize  = 12,
        buttonFontSize = 15
    };

    //==============================================================================
    enum
    {
        knobActiveRadius   = 12,
        knobInActiveRadius = 8,
        haloRadius         = 18
    };

    //==============================================================================
    const Color windowBackgroundColor = Color (0xff262328);
    const Color backgroundColor       = Color (0xff4d4d4d);
    const Color brightButtonColor     = Color (0xff80cbc4);
    const Color disabledButtonColor   = Color (0xffe4e4e4);
    const Color sliderInactivePart     = Color (0xff545d62);
    const Color sliderActivePart       = Color (0xff80cbc4);
};

//==============================================================================
class AUv3SynthEditor final : public AudioProcessorEditor,
                              private Timer
{
public:
    //==============================================================================
    AUv3SynthEditor (AudioProcessor& processorIn)
        : AudioProcessorEditor (processorIn),
          roomSizeSlider (Slider::LinearHorizontal, Slider::NoTextBox)
    {
        setLookAndFeel (&materialLookAndFeel);

        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);

        recordButton.onClick = [this] { startRecording(); };
        addAndMakeVisible (recordButton);

        roomSizeSlider.onValueChange = [this] { setParameterValue ("roomSize", (f32) roomSizeSlider.getValue()); };
        roomSizeSlider.setRange (0.0, 1.0);
        addAndMakeVisible (roomSizeSlider);

        if (auto fileStream = createAssetInputStream ("proaudio.path"))
        {
            Path proAudioPath;
            proAudioPath.loadPathFromStream (*fileStream);
            proAudioIcon.setPath (proAudioPath);
            addAndMakeVisible (proAudioIcon);

            auto proAudioIconColor = findColor (TextButton::buttonOnColorId);
            proAudioIcon.setFill (FillType (proAudioIconColor));
        }

        setSize (600, 400);
        startTimer (100);
    }

    ~AUv3SynthEditor() override
    {
        setLookAndFeel (nullptr);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (ResizableWindow::backgroundColorId));
    }

    z0 resized() override
    {
        auto r = getLocalBounds();

        auto guiElementAreaHeight = r.getHeight() / 3;

        proAudioIcon.setTransformToFit (r.removeFromLeft (proportionOfWidth (0.25))
                                         .withSizeKeepingCentre (guiElementAreaHeight, guiElementAreaHeight)
                                         .toFloat(),
                                        RectanglePlacement::fillDestination);

        auto margin = guiElementAreaHeight / 4;
        r.reduce (margin, margin);

        auto buttonHeight = guiElementAreaHeight - margin;

        recordButton  .setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
        roomSizeSlider.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
    }

    //==============================================================================
    z0 startRecording()
    {
        recordButton.setEnabled (false);
        setParameterValue ("isRecording", 1.0f);
    }

private:
    //==============================================================================
    z0 timerCallback() override
    {
        auto isRecordingNow = (getParameterValue ("isRecording") >= 0.5f);

        recordButton.setEnabled (! isRecordingNow);
        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);
    }

    //==============================================================================
    AudioProcessorParameter* getParameter (const Txt& paramId)
    {
        if (auto* audioProcessor = getAudioProcessor())
        {
            auto& params = audioProcessor->getParameters();

            for (auto p : params)
            {
                if (auto* param = dynamic_cast<AudioProcessorParameterWithID*> (p))
                {
                    if (param->paramID == paramId)
                        return param;
                }
            }
        }

        return nullptr;
    }

    //==============================================================================
    f32 getParameterValue (const Txt& paramId)
    {
        if (auto* param = getParameter (paramId))
            return param->getValue();

        return 0.0f;
    }

    z0 setParameterValue (const Txt& paramId, f32 value)
    {
        if (auto* param = getParameter (paramId))
            param->setValueNotifyingHost (value);
    }

    //==============================================================================
    MaterialLookAndFeel materialLookAndFeel;

    //==============================================================================
    TextButton recordButton { "Record" };
    Slider roomSizeSlider;
    DrawablePath proAudioIcon;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUv3SynthEditor)
};

//==============================================================================
class AUv3SynthProcessor final : public AudioProcessor
{
public:
    AUv3SynthProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
          currentRecording (1, 1), currentProgram (0)
    {
        // initialize parameters
        addParameter (isRecordingParam = new AudioParameterBool  ({ "isRecording", 1 }, "Is Recording", false));
        addParameter (roomSizeParam    = new AudioParameterFloat ({ "roomSize", 1 }, "Room Size", 0.0f, 1.0f, 0.5f));

        formatManager.registerBasicFormats();

        for (auto i = 0; i < maxNumVoices; ++i)
            synth.addVoice (new SamplerVoice());

        loadNewSample (createAssetInputStream ("singing.ogg"), "ogg");
    }

    //==============================================================================
    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return (layouts.getMainOutputChannels() <= 2);
    }

    z0 prepareToPlay (f64 sampleRate, i32 estimatedMaxSizeOfBuffer) override
    {
        ignoreUnused (estimatedMaxSizeOfBuffer);

        lastSampleRate = sampleRate;

        currentRecording.setSize (1, static_cast<i32> (std::ceil (maxDurationOfRecording * lastSampleRate)));
        samplesRecorded = 0;

        synth.setCurrentPlaybackSampleRate (lastSampleRate);
        reverb.setSampleRate (lastSampleRate);
    }

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        Reverb::Parameters reverbParameters;
        reverbParameters.roomSize = roomSizeParam->get();

        reverb.setParameters (reverbParameters);
        synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

        if (getMainBusNumOutputChannels() == 1)
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
        else if (getMainBusNumOutputChannels() == 2)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    z0 releaseResources() override                                            { currentRecording.setSize (1, 1); }

    //==============================================================================
    b8 acceptsMidi() const override                                           { return true; }
    b8 producesMidi() const override                                          { return false; }
    f64 getTailLengthSeconds() const override                                { return 0.0; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override                               { return new AUv3SynthEditor (*this); }
    b8 hasEditor() const override                                             { return true; }

    //==============================================================================
    const Txt getName() const override                                       { return "AUv3 Synth"; }
    i32 getNumPrograms() override                                               { return 4; }
    i32 getCurrentProgram() override                                            { return currentProgram; }
    z0 setCurrentProgram (i32 index) override                                 { currentProgram = index; }

    const Txt getProgramName (i32 index) override
    {
        switch (index)
        {
            case 0:  return "Piano";
            case 1:  return "Singing";
            case 2:  return "Pinched Balloon";
            case 3:  return "Gazeebo";
            default: break;
        }

        return "<Unknown>";
    }

    //==============================================================================
    z0 changeProgramName (i32 /*index*/, const Txt& /*name*/) override     {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*isRecordingParam);
        stream.writeFloat (*roomSizeParam);
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

        isRecordingParam->setValueNotifyingHost (stream.readFloat());
        roomSizeParam->setValueNotifyingHost    (stream.readFloat());

    }

private:
    //==============================================================================
    z0 loadNewSampleBinary (ukk data, i32 dataSize, tukk format)
    {
        auto soundBuffer = std::make_unique<MemoryInputStream> (data, static_cast<std::size_t> (dataSize), false);
        loadNewSample (std::move (soundBuffer), format);
    }

    z0 loadNewSample (std::unique_ptr<InputStream> soundBuffer, tukk format)
    {
        std::unique_ptr<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension (format)->createReaderFor (soundBuffer.release(), true));

        BigInteger midiNotes;
        midiNotes.setRange (0, 126, true);
        SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);
        synth.removeSound (0);
        sound = newSound;
        synth.addSound (sound);
    }

    z0 swapSamples()
    {
        MemoryBlock mb;
        auto* stream = new MemoryOutputStream (mb, true);

        {
            std::unique_ptr<AudioFormatWriter> writer (formatManager.findFormatForFileExtension ("wav")->createWriterFor (stream, lastSampleRate, 1, 16,
                                                                                                                          StringPairArray(), 0));
            writer->writeFromAudioSampleBuffer (currentRecording, 0, currentRecording.getNumSamples());
            writer->flush();
            stream->flush();
        }

        loadNewSampleBinary (mb.getData(), static_cast<i32> (mb.getSize()), "wav");
    }

    //==============================================================================
    static constexpr i32 maxNumVoices = 5;
    static constexpr f64 maxDurationOfRecording = 1.0;

    //==============================================================================
    AudioFormatManager formatManager;

    i32 samplesRecorded;
    f64 lastSampleRate;
    AudioBuffer<f32> currentRecording;

    Reverb reverb;
    Synthesiser synth;
    SynthesiserSound::Ptr sound;

    AudioParameterBool* isRecordingParam;
    AudioParameterFloat* roomSizeParam;

    i32 currentProgram;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUv3SynthProcessor)
};
