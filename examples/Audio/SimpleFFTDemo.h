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

 name:             SimpleFFTDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simple FFT application.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        SimpleFFTDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class SimpleFFTDemo final : public AudioAppComponent,
                            private Timer
{
public:
    SimpleFFTDemo() :
         #ifdef DRX_DEMO_RUNNER
          AudioAppComponent (getSharedAudioDeviceManager (1, 0)),
         #endif
          forwardFFT (fftOrder),
          spectrogramImage (Image::RGB, 512, 512, true)
    {
        setOpaque (true);

       #ifndef DRX_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (b8 granted)
                                     {
                                         i32 numInputChannels = granted ? 2 : 0;
                                         setAudioChannels (numInputChannels, 2);
                                     });
       #else
        setAudioChannels (2, 2);
       #endif

        startTimerHz (60);
        setSize (700, 500);
    }

    ~SimpleFFTDemo() override
    {
        shutdownAudio();
    }

    //==============================================================================
    z0 prepareToPlay (i32 /*samplesPerBlockExpected*/, f64 /*newSampleRate*/) override
    {
        // (nothing to do here)
    }

    z0 releaseResources() override
    {
        // (nothing to do here)
    }

    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (bufferToFill.buffer->getNumChannels() > 0)
        {
            const auto* channelData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
                pushNextSampleIntoFifo (channelData[i]);

            bufferToFill.clearActiveBufferRegion();
        }
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);

        g.setOpacity (1.0f);
        g.drawImage (spectrogramImage, getLocalBounds().toFloat());
    }

    z0 timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    z0 pushNextSampleIntoFifo (f32 sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next line should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample;
    }

    z0 drawNextLineOfSpectrogram()
    {
        auto rightHandEdge = spectrogramImage.getWidth() - 1;
        auto imageHeight   = spectrogramImage.getHeight();

        // first, shuffle our image leftwards by 1 pixel..
        spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);

        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform (fftData);

        // find the range of values produced, so we can scale our rendering to
        // show up the detail clearly
        auto maxLevel = FloatVectorOperations::findMinAndMax (fftData, fftSize / 2);

        Image::BitmapData bitmap { spectrogramImage, rightHandEdge, 0, 1, imageHeight, Image::BitmapData::writeOnly };

        for (auto y = 1; y < imageHeight; ++y)
        {
            auto skewedProportionY = 1.0f - std::exp (std::log ((f32) y / (f32) imageHeight) * 0.2f);
            auto fftDataIndex = jlimit (0, fftSize / 2, (i32) (skewedProportionY * (i32) fftSize / 2));
            auto level = jmap (fftData[fftDataIndex], 0.0f, jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

            bitmap.setPixelColor (0, y, Color::fromHSV (level, 1.0f, level, 1.0f));
        }
    }

    enum
    {
        fftOrder = 10,
        fftSize  = 1 << fftOrder
    };

private:
    dsp::FFT forwardFFT;
    Image spectrogramImage;

    f32 fifo [fftSize];
    f32 fftData [2 * fftSize];
    i32 fifoIndex = 0;
    b8 nextFFTBlockReady = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFFTDemo)
};
