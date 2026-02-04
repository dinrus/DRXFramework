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

 name:             FIRFilterDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      FIR filter demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FIRFilterDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct FIRFilterDemoDSP
{
    z0 prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        fir.state = FilterDesign<f32>::designFIRLowpassWindowMethod (440.0f, sampleRate, 21,
                                                                       WindowingFunction<f32>::blackman);
        fir.prepare (spec);
    }

    z0 process (const ProcessContextReplacing<f32>& context)
    {
        fir.process (context);
    }

    z0 reset()
    {
        fir.reset();
    }

    z0 updateParameters()
    {
        if (! approximatelyEqual (sampleRate, 0.0))
        {
            auto cutoff = static_cast<f32> (cutoffParam.getCurrentValue());
            auto windowingMethod = static_cast<WindowingFunction<f32>::WindowingMethod> (typeParam.getCurrentSelectedID() - 1);

            *fir.state = *FilterDesign<f32>::designFIRLowpassWindowMethod (cutoff, sampleRate, 21, windowingMethod);
        }
    }

    //==============================================================================
    ProcessorDuplicator<FIR::Filter<f32>, FIR::Coefficients<f32>> fir;

    f64 sampleRate = 0.0;

    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.4, 440.0f, "Cutoff", "Hz" };
    ChoiceParameter typeParam { { "Rectangular", "Triangular", "Hann", "Hamming", "Blackman", "Blackman-Harris", "Flat Top", "Kaiser" },
                                5, "Windowing Function" };

    std::vector<DSPDemoParameterBase*> parameters { &cutoffParam, &typeParam };
};

struct FIRFilterDemo final : public Component
{
    FIRFilterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<FIRFilterDemoDSP> fileReaderComponent;
};
