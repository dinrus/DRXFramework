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

 name:             BouncingBallWavetableDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Wavetable synthesis with a bouncing ball.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        BouncingBallWavetableDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class BouncingBallWavetableDemo final : public AudioAppComponent,
                                        private Timer
{
public:
    //==============================================================================
    BouncingBallWavetableDemo()
       #ifdef DRX_DEMO_RUNNER
        : AudioAppComponent (getSharedAudioDeviceManager (0, 2))
       #endif
    {
        setSize (600, 600);

        for (auto i = 0; i < numElementsInArray (waveValues); ++i)
            zeromem (waveValues[i], sizeof (waveValues[i]));

        // specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
        startTimerHz (60);
    }

    ~BouncingBallWavetableDemo() override
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

        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            auto ind = waveTableIndex;

            auto* channelData = bufferToFill.buffer->getWritePointer (chan, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
            {
                if (isPositiveAndBelow (chan, numElementsInArray (waveValues)))
                {
                    channelData[i] = waveValues[chan][ind % wavetableSize];
                    ++ind;
                }
            }
        }

        waveTableIndex = (i32) (waveTableIndex + bufferToFill.numSamples) % wavetableSize;
    }

    z0 releaseResources() override
    {
        // This gets automatically called when audio device parameters change
        // or device is restarted.
        stopTimer();
    }


    //==============================================================================
    z0 paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));

        auto nextPos = pos + delta;

        if (nextPos.x < 10 || nextPos.x + 10 > (f32) getWidth())
        {
            delta.x = -delta.x;
            nextPos.x = pos.x + delta.x;
        }

        if (nextPos.y < 50 || nextPos.y + 10 > (f32) getHeight())
        {
            delta.y = -delta.y;
            nextPos.y = pos.y + delta.y;
        }

        if (! dragging)
        {
            writeInterpolatedValue (pos, nextPos);
            pos = nextPos;
        }
        else
        {
            pos = lastMousePosition;
        }

        // draw a circle
        g.setColor (getLookAndFeel().findColor (Slider::thumbColorId));
        g.fillEllipse (pos.x, pos.y, 20, 20);

        drawWaveform (g, 20.0f, 0);
        drawWaveform (g, 40.0f, 1);
    }

    z0 drawWaveform (Graphics& g, f32 y, i32 channel) const
    {
        auto pathWidth = 2000;

        Path wavePath;
        wavePath.startNewSubPath (0.0f, y);

        for (auto i = 1; i < pathWidth; ++i)
            wavePath.lineTo ((f32) i, (1.0f + waveValues[channel][i * numElementsInArray (waveValues[0]) / pathWidth]) * 10.0f);

        g.strokePath (wavePath, PathStrokeType (1.0f),
                      wavePath.getTransformToScaleToFit (Rectangle<f32> (0.0f, y, (f32) getWidth(), 20.0f), false));
    }

    // Mouse handling..
    z0 mouseDown (const MouseEvent& e) override
    {
        lastMousePosition = e.position;
        mouseDrag (e);
        dragging = true;
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        dragging = true;

        if (e.position != lastMousePosition)
        {
            // calculate movement vector
            delta = e.position - lastMousePosition;

            waveValues[0][bufferIndex % wavetableSize] = xToAmplitude (e.position.x);
            waveValues[1][bufferIndex % wavetableSize] = yToAmplitude (e.position.y);

            ++bufferIndex;
            lastMousePosition = e.position;
        }
    }

    z0 mouseUp (const MouseEvent&) override
    {
        dragging = false;
    }

    z0 writeInterpolatedValue (Point<f32> lastPosition,
                                 Point<f32> currentPosition)
    {
        Point<f32> start, finish;

        if (lastPosition.getX() > currentPosition.getX())
        {
            finish = lastPosition;
            start  = currentPosition;
        }
        else
        {
            start  = lastPosition;
            finish = currentPosition;
        }

        for (auto i = 0; i < steps; ++i)
        {
            auto p = start + ((finish - start) * i) / (i32) steps;

            auto index = (bufferIndex + i) % wavetableSize;
            waveValues[1][index] = yToAmplitude (p.y);
            waveValues[0][index] = xToAmplitude (p.x);
        }

        bufferIndex = (bufferIndex + steps) % wavetableSize;
    }

    f32 indexToX (i32 indexValue) const noexcept
    {
        return (f32) indexValue;
    }

    f32 amplitudeToY (f32 amp) const noexcept
    {
        return (f32) getHeight() - (amp + 1.0f) * (f32) getHeight() / 2.0f;
    }

    f32 xToAmplitude (f32 x) const noexcept
    {
        return jlimit (-1.0f, 1.0f, 2.0f * ((f32) getWidth() - x) / (f32) getWidth() - 1.0f);
    }

    f32 yToAmplitude (f32 y) const noexcept
    {
        return jlimit (-1.0f, 1.0f, 2.0f * ((f32) getHeight() - y) / (f32) getHeight() - 1.0f);
    }

    z0 timerCallback() override
    {
        repaint();
    }

private:
    //==============================================================================
    enum
    {
        wavetableSize = 36000,
        steps = 10
    };

    Point<f32> pos   = { 299.0f, 299.0f };
    Point<f32> delta = { 0.0f, 0.0f };
    i32 waveTableIndex = 0;
    i32 bufferIndex    = 0;
    f64 sampleRate  = 0.0;
    i32 expectedSamplesPerBlock = 0;
    Point<f32> lastMousePosition;
    f32 waveValues[2][wavetableSize];
    b8 dragging = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBallWavetableDemo)
};
