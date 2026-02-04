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

 name:             AudioAppDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simple audio application.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioAppDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class AudioAppDemo final : public AudioAppComponent
{
public:
    //==============================================================================
    AudioAppDemo()
       #ifdef DRX_DEMO_RUNNER
        : AudioAppComponent (getSharedAudioDeviceManager (0, 2))
       #endif
    {
        setAudioChannels (0, 2);

        setSize (800, 600);
    }

    ~AudioAppDemo() override
    {
        shutdownAudio();
    }

    //==============================================================================
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 newSampleRate) override
    {
        sampleRate = newSampleRate;
        expectedSamplesPerBlock = samplesPerBlockExpected;
    }

    /*  This method generates the actual audio samples.
        In this example the buffer is filled with a sine wave whose frequency and
        amplitude are controlled by the mouse position.
     */
    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
        auto originalPhase = phase;

        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            phase = originalPhase;

            auto* channelData = bufferToFill.buffer->getWritePointer (chan, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples ; ++i)
            {
                channelData[i] = amplitude * std::sin (phase);

                // increment the phase step for the next sample
                phase = std::fmod (phase + phaseDelta, MathConstants<f32>::twoPi);
            }
        }
    }

    z0 releaseResources() override
    {
        // This gets automatically called when audio device parameters change
        // or device is restarted.
    }


    //==============================================================================
    z0 paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));

        auto centreY = (f32) getHeight() / 2.0f;
        auto radius = amplitude * 200.0f;

        if (radius >= 0.0f)
        {
            // Draw an ellipse based on the mouse position and audio volume
            g.setColor (Colors::lightgreen);

            g.fillEllipse (jmax (0.0f, lastMousePosition.x) - radius / 2.0f,
                           jmax (0.0f, lastMousePosition.y) - radius / 2.0f,
                           radius, radius);
        }

        // Draw a representative sine wave.
        Path wavePath;
        wavePath.startNewSubPath (0, centreY);

        for (auto x = 1.0f; x < (f32) getWidth(); ++x)
            wavePath.lineTo (x, centreY + amplitude * (f32) getHeight() * 2.0f
                                            * std::sin (x * frequency * 0.0001f));

        g.setColor (getLookAndFeel().findColor (Slider::thumbColorId));
        g.strokePath (wavePath, PathStrokeType (2.0f));
    }

    // Mouse handling..
    z0 mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        lastMousePosition = e.position;

        frequency = (f32) (getHeight() - e.y) * 10.0f;
        amplitude = jmin (0.9f, 0.2f * e.position.x / (f32) getWidth());

        phaseDelta = (f32) (MathConstants<f64>::twoPi * frequency / sampleRate);

        repaint();
    }

    z0 mouseUp (const MouseEvent&) override
    {
        amplitude = 0.0f;
        repaint();
    }

    z0 resized() override
    {
        // This is called when the component is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    f32 phase       = 0.0f;
    f32 phaseDelta  = 0.0f;
    f32 frequency   = 5000.0f;
    f32 amplitude   = 0.2f;

    f64 sampleRate = 0.0;
    i32 expectedSamplesPerBlock = 0;
    Point<f32> lastMousePosition;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAppDemo)
};
