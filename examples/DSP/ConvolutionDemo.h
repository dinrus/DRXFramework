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

 name:             ConvolutionDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Convolution demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ConvolutionDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct ConvolutionDemoDSP
{
    z0 prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        convolution.prepare (spec);
        updateParameters();
    }

    z0 process (ProcessContextReplacing<f32> context)
    {
        context.isBypassed = bypass;

        // Load a new IR if there's one available. Note that this doesn't lock or allocate!
        bufferTransfer.get ([this] (BufferWithSampleRate& buf)
        {
            convolution.loadImpulseResponse (std::move (buf.buffer),
                                             buf.sampleRate,
                                             Convolution::Stereo::yes,
                                             Convolution::Trim::yes,
                                             Convolution::Normalise::yes);
        });

        convolution.process (context);
    }

    z0 reset()
    {
        convolution.reset();
    }

    z0 updateParameters()
    {
        auto* cabinetTypeParameter = dynamic_cast<ChoiceParameter*> (parameters[0]);

        if (cabinetTypeParameter == nullptr)
        {
            jassertfalse;
            return;
        }

        if (cabinetTypeParameter->getCurrentSelectedID() == 1)
        {
            bypass = true;
        }
        else
        {
            bypass = false;

            auto selectedType = cabinetTypeParameter->getCurrentSelectedID();
            auto assetName = (selectedType == 2 ? "guitar_amp.wav" : "cassette_recorder.wav");

            auto assetInputStream = createAssetInputStream (assetName);

            if (assetInputStream == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioFormatManager manager;
            manager.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> reader { manager.createReaderFor (std::move (assetInputStream)) };

            if (reader == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioBuffer<f32> buffer (static_cast<i32> (reader->numChannels),
                                       static_cast<i32> (reader->lengthInSamples));
            reader->read (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, buffer.getNumSamples());

            bufferTransfer.set (BufferWithSampleRate { std::move (buffer), reader->sampleRate });
        }
    }

    //==============================================================================
    struct BufferWithSampleRate
    {
        BufferWithSampleRate() = default;

        BufferWithSampleRate (AudioBuffer<f32>&& bufferIn, f64 sampleRateIn)
            : buffer (std::move (bufferIn)), sampleRate (sampleRateIn) {}

        AudioBuffer<f32> buffer;
        f64 sampleRate = 0.0;
    };

    class BufferTransfer
    {
    public:
        z0 set (BufferWithSampleRate&& p)
        {
            const SpinLock::ScopedLockType lock (mutex);
            buffer = std::move (p);
            newBuffer = true;
        }

        // Call `fn` passing the new buffer, if there's one available
        template <typename Fn>
        z0 get (Fn&& fn)
        {
            const SpinLock::ScopedTryLockType lock (mutex);

            if (lock.isLocked() && newBuffer)
            {
                fn (buffer);
                newBuffer = false;
            }
        }

    private:
        BufferWithSampleRate buffer;
        b8 newBuffer = false;
        SpinLock mutex;
    };

    f64 sampleRate = 0.0;
    b8 bypass = false;

    MemoryBlock currentCabinetData;
    Convolution convolution;

    BufferTransfer bufferTransfer;

    ChoiceParameter cabinetParam { { "Bypass", "Guitar amplifier 8''", "Cassette recorder" }, 1, "Cabinet Type" };

    std::vector<DSPDemoParameterBase*> parameters { &cabinetParam };
};

struct ConvolutionDemo final : public Component
{
    ConvolutionDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<ConvolutionDemoDSP> fileReaderComponent;
};
