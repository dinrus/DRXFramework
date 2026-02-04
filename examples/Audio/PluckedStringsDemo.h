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

 name:             PluckedStringsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simulation of a plucked string sound.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        PluckedStringsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/**
    A very basic generator of a simulated plucked string sound, implementing
    the Karplus-Strong algorithm.

    Not performance-optimised!
*/
class StringSynthesiser
{
public:
    //==============================================================================
    /** Constructor.

        @param sampleRate      The audio sample rate to use.
        @param frequencyInHz   The fundamental frequency of the simulated string in
                               Hertz.
    */
    StringSynthesiser (f64 sampleRate, f64 frequencyInHz)
    {
        doPluckForNextBuffer.set (false);
        prepareSynthesiserState (sampleRate, frequencyInHz);
    }

    //==============================================================================
    /** Excite the simulated string by plucking it at a given position.

        @param pluckPosition The position of the plucking, relative to the length
                             of the string. Must be between 0 and 1.
    */
    z0 stringPlucked (f32 pluckPosition)
    {
        jassert (pluckPosition >= 0.0 && pluckPosition <= 1.0);

        // we choose a very simple approach to communicate with the audio thread:
        // simply tell the synth to perform the plucking excitation at the beginning
        // of the next buffer (= when generateAndAddData is called the next time).

        if (doPluckForNextBuffer.compareAndSetBool (1, 0))
        {
            // plucking in the middle gives the largest amplitude;
            // plucking at the very ends will do nothing.
            amplitude = std::sin (MathConstants<f32>::pi * pluckPosition);
        }
    }

    //==============================================================================
    /** Generate next chunk of mono audio output and add it into a buffer.

        @param outBuffer  Buffer to fill (one channel only). New sound will be
                          added to existing content of the buffer (instead of
                          replacing it).
        @param numSamples Number of samples to generate (make sure that outBuffer
                          has enough space).
    */
    z0 generateAndAddData (f32* outBuffer, i32 numSamples)
    {
        if (doPluckForNextBuffer.compareAndSetBool (0, 1))
            exciteInternalBuffer();

        // cycle through the delay line and apply a simple averaging filter
        for (auto i = 0; i < numSamples; ++i)
        {
            auto nextPos = (pos + 1) % delayLine.size();

            delayLine[nextPos] = (f32) (decay * 0.5 * (delayLine[nextPos] + delayLine[pos]));
            outBuffer[i] += delayLine[pos];

            pos = nextPos;
        }
    }

private:
    //==============================================================================
    z0 prepareSynthesiserState (f64 sampleRate, f64 frequencyInHz)
    {
        auto delayLineLength = (size_t) roundToInt (sampleRate / frequencyInHz);

        // we need a minimum delay line length to get a reasonable synthesis.
        // if you hit this assert, increase sample rate or decrease frequency!
        jassert (delayLineLength > 50);

        delayLine.resize (delayLineLength);
        std::fill (delayLine.begin(), delayLine.end(), 0.0f);

        excitationSample.resize (delayLineLength);

        // as the excitation sample we use random noise between -1 and 1
        // (as a simple approximation to a plucking excitation)

        std::generate (excitationSample.begin(),
                       excitationSample.end(),
                       [] { return (Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f; } );
    }

    z0 exciteInternalBuffer()
    {
        // fill the buffer with the precomputed excitation sound (scaled with amplitude)

        jassert (delayLine.size() >= excitationSample.size());

        std::transform (excitationSample.begin(),
                        excitationSample.end(),
                        delayLine.begin(),
                        [this] (f64 sample) { return static_cast<f32> (amplitude * sample); } );
    }

    //==============================================================================
    const f64 decay = 0.998;
    f64 amplitude = 0.0;

    Atomic<i32> doPluckForNextBuffer;

    std::vector<f32> excitationSample, delayLine;
    size_t pos = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringSynthesiser)
};

//==============================================================================
/*
    This component represents a horizontal vibrating musical string of fixed height
    and variable length. The string can be excited by calling stringPlucked().
*/
class StringComponent final : public Component,
                              private Timer
{
public:
    StringComponent (i32 lengthInPixels, Color stringColor)
        : length (lengthInPixels), colour (stringColor)
    {
        // ignore mouse-clicks so that our parent can get them instead.
        setInterceptsMouseClicks (false, false);
        setSize (length, height);
        startTimerHz (60);
    }

    //==============================================================================
    z0 stringPlucked (f32 pluckPositionRelative)
    {
        amplitude = maxAmplitude * std::sin (pluckPositionRelative * MathConstants<f32>::pi);
        phase = MathConstants<f32>::pi;
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.setColor (colour);
        g.strokePath (generateStringPath(), PathStrokeType (2.0f));
    }

    Path generateStringPath() const
    {
        auto y = (f32) height / 2.0f;

        Path stringPath;
        stringPath.startNewSubPath (0, y);
        stringPath.quadraticTo ((f32) length / 2.0f, y + (std::sin (phase) * amplitude), (f32) length, y);
        return stringPath;
    }

    //==============================================================================
    z0 timerCallback() override
    {
        updateAmplitude();
        updatePhase();
        repaint();
    }

    z0 updateAmplitude()
    {
        // this determines the decay of the visible string vibration.
        amplitude *= 0.99f;
    }

    z0 updatePhase()
    {
        // this determines the visible vibration frequency.
        // just an arbitrary number chosen to look OK:
        auto phaseStep = 400.0f / (f32) length;

        phase += phaseStep;

        if (phase >= MathConstants<f32>::twoPi)
            phase -= MathConstants<f32>::twoPi;
    }

private:
    //==============================================================================
    i32 length;
    Color colour;

    i32 height = 20;
    f32 amplitude = 0.0f;
    const f32 maxAmplitude = 12.0f;
    f32 phase = 0.0f;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringComponent)
};

//==============================================================================
class PluckedStringsDemo final : public AudioAppComponent
{
public:
    PluckedStringsDemo()
       #ifdef DRX_DEMO_RUNNER
        : AudioAppComponent (getSharedAudioDeviceManager (0, 2))
       #endif
    {
        createStringComponents();
        setSize (800, 560);

        // specify the number of input and output channels that we want to open
        auto audioDevice = deviceManager.getCurrentAudioDevice();
        auto numInputChannels  = (audioDevice != nullptr ? audioDevice->getActiveInputChannels() .countNumberOfSetBits() : 0);
        auto numOutputChannels = jmax (audioDevice != nullptr ? audioDevice->getActiveOutputChannels().countNumberOfSetBits() : 2, 2);

        setAudioChannels (numInputChannels, numOutputChannels);
    }

    ~PluckedStringsDemo() override
    {
        shutdownAudio();
    }

    //==============================================================================
    z0 prepareToPlay (i32 /*samplesPerBlockExpected*/, f64 sampleRate) override
    {
        generateStringSynths (sampleRate);
    }

    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto* channelData = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);

            if (channel == 0)
            {
                for (auto synth : stringSynths)
                    synth->generateAndAddData (channelData, bufferToFill.numSamples);
            }
            else
            {
                memcpy (channelData,
                        bufferToFill.buffer->getReadPointer (0),
                        ((size_t) bufferToFill.numSamples) * sizeof (f32));
            }
        }
    }

    z0 releaseResources() override
    {
        stringSynths.clear();
    }

    //==============================================================================
    z0 paint (Graphics&) override {}

    z0 resized() override
    {
        auto xPos = 20;
        auto yPos = 20;
        auto yDistance = 50;

        for (auto stringLine : stringLines)
        {
            stringLine->setTopLeftPosition (xPos, yPos);
            yPos += yDistance;
            addAndMakeVisible (stringLine);
        }
    }

private:
    z0 mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        for (auto i = 0; i < stringLines.size(); ++i)
        {
            auto* stringLine = stringLines.getUnchecked (i);

            if (stringLine->getBounds().contains (e.getPosition()))
            {
                auto position = (e.position.x - (f32) stringLine->getX()) / (f32) stringLine->getWidth();

                stringLine->stringPlucked (position);
                stringSynths.getUnchecked (i)->stringPlucked (position);
            }
        }
    }

    //==============================================================================
    struct StringParameters
    {
        StringParameters (i32 midiNote)
            : frequencyInHz (MidiMessage::getMidiNoteInHertz (midiNote)),
              lengthInPixels ((i32) (760 / (frequencyInHz / MidiMessage::getMidiNoteInHertz (42))))
        {}

        f64 frequencyInHz;
        i32 lengthInPixels;
    };

    static Array<StringParameters> getDefaultStringParameters()
    {
        return Array<StringParameters> (42, 44, 46, 49, 51, 54, 56, 58, 61, 63, 66, 68, 70);
    }

    z0 createStringComponents()
    {
        for (auto stringParams : getDefaultStringParameters())
        {
            stringLines.add (new StringComponent (stringParams.lengthInPixels,
                                                  Color::fromHSV (Random().nextFloat(), 0.6f, 0.9f, 1.0f)));
        }
    }

    z0 generateStringSynths (f64 sampleRate)
    {
        stringSynths.clear();

        for (auto stringParams : getDefaultStringParameters())
        {
            stringSynths.add (new StringSynthesiser (sampleRate, stringParams.frequencyInHz));
        }
    }

    //==============================================================================
    OwnedArray<StringComponent> stringLines;
    OwnedArray<StringSynthesiser> stringSynths;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluckedStringsDemo)
};
