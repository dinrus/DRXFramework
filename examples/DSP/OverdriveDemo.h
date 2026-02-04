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

 name:             OverdriveDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Overdrive demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OverdriveDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct OverdriveDemoDSP
{
    z0 prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        auto& gainUp = overdrive.get<0>();
        gainUp.setGainDecibels (24);

        auto& bias = overdrive.get<1>();
        bias.setBias (0.4f);

        auto& wavShaper = overdrive.get<2>();
        wavShaper.functionToUse = std::tanh;

        auto& dcFilter = overdrive.get<3>();
        dcFilter.state = IIR::Coefficients<f32>::makeHighPass (sampleRate, 5.0);

        auto& gainDown = overdrive.get<4>();
        gainDown.setGainDecibels (-18.0f);

        overdrive.prepare (spec);
    }

    z0 process (const ProcessContextReplacing<f32>& context)
    {
        overdrive.process (context);
    }

    z0 reset()
    {
        overdrive.reset();
    }

    z0 updateParameters()
    {
        if (! approximatelyEqual (sampleRate, 0.0))
        {
            overdrive.get<0>().setGainDecibels (static_cast<f32> (inGainParam.getCurrentValue()));
            overdrive.get<4>().setGainDecibels (static_cast<f32> (outGainParam.getCurrentValue()));
        }
    }

    //==============================================================================
    using GainProcessor   = Gain<f32>;
    using BiasProcessor   = Bias<f32>;
    using DriveProcessor  = WaveShaper<f32>;
    using DCFilter        = ProcessorDuplicator<IIR::Filter<f32>,
                                                IIR::Coefficients<f32>>;

    ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, DCFilter, GainProcessor> overdrive;

    SliderParameter inGainParam  { { -100.0, 60.0 }, 3, 24.0,  "Input Gain",  "dB" };
    SliderParameter outGainParam { { -100.0, 20.0 }, 3, -18.0, "Output Gain", "dB" };

    std::vector<DSPDemoParameterBase*> parameters { &inGainParam, &outGainParam };
    f64 sampleRate = 0.0;
};

struct OverdriveDemo final : public Component
{
    OverdriveDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<OverdriveDemoDSP> fileReaderComponent;
};
