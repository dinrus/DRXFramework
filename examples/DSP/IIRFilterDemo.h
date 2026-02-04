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

 name:             IIRFilterDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      IIR filter demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        IIRFilterDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct IIRFilterDemoDSP
{
    z0 prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        iir.state = IIR::Coefficients<f32>::makeLowPass (sampleRate, 440.0);
        iir.prepare (spec);
    }

    z0 process (const ProcessContextReplacing<f32>& context)
    {
        iir.process (context);
    }

    z0 reset()
    {
        iir.reset();
    }

    z0 updateParameters()
    {
        if (! approximatelyEqual (sampleRate, 0.0))
        {
            auto cutoff = static_cast<f32> (cutoffParam.getCurrentValue());
            auto qVal   = static_cast<f32> (qParam.getCurrentValue());

            switch (typeParam.getCurrentSelectedID())
            {
                case 1:     *iir.state = IIR::ArrayCoefficients<f32>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:     *iir.state = IIR::ArrayCoefficients<f32>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:     *iir.state = IIR::ArrayCoefficients<f32>::makeBandPass (sampleRate, cutoff, qVal); break;
                default:    break;
            }
        }
    }

    //==============================================================================
    ProcessorDuplicator<IIR::Filter<f32>, IIR::Coefficients<f32>> iir;

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass" }, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 1.0 / std::sqrt (2.0), "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    f64 sampleRate = 0.0;
};

struct IIRFilterDemo final : public Component
{
    IIRFilterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<IIRFilterDemoDSP> fileReaderComponent;
};
