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

 name:             WaveShaperTanhDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Wave shaper tanh demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        WaveShaperTanhDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct WaveShaperTanhDemoDSP
{
    z0 prepare (const ProcessSpec&) {}

    z0 process (const ProcessContextReplacing<f32>& context)
    {
        shapers[currentShaperIdx].process (context);
    }

    z0 reset()
    {
        for (auto&& shaper : shapers)
            shaper.reset();
    }

    z0 updateParameters()
    {
        currentShaperIdx = jmin (numElementsInArray (shapers), accuracy.getCurrentSelectedID() - 1);
    }

    //==============================================================================
    WaveShaper<f32> shapers[2] { { std::tanh }, { FastMathApproximations::tanh } };
    i32 currentShaperIdx = 0;

    ChoiceParameter accuracy {{ "No Approximation", "Use fast-math approximation" }, 1, "Accuracy" };
    std::vector<DSPDemoParameterBase*> parameters { &accuracy };  // no params for this demo
};

struct WaveShaperTanhDemo final : public Component
{
    WaveShaperTanhDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<WaveShaperTanhDemoDSP> fileReaderComponent;
};
