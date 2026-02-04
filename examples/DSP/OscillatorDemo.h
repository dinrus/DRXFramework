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

 name:             OscillatorDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Oscillator demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OscillatorDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct OscillatorDemoDSP
{
    z0 prepare (const ProcessSpec& spec)
    {
        gain.setGainDecibels (-6.0f);

        for (auto&& oscillator : oscillators)
        {
            oscillator.setFrequency (440.f);
            oscillator.prepare (spec);
        }

        updateParameters();

        tempBuffer = AudioBlock<f32> (tempBufferMemory, spec.numChannels, spec.maximumBlockSize);
    }

    z0 process (const ProcessContextReplacing<f32>& context)
    {
        tempBuffer.copyFrom (context.getInputBlock());
        tempBuffer.multiplyBy (static_cast<f32> (fileMix));

        oscillators[currentOscillatorIdx].process (context);
        context.getOutputBlock().multiplyBy (static_cast<f32> (1.0 - fileMix));

        context.getOutputBlock().add (tempBuffer);

        gain.process (context);
    }

    z0 reset()
    {
        oscillators[currentOscillatorIdx].reset();
    }

    z0 updateParameters()
    {
        currentOscillatorIdx = jmin (numElementsInArray (oscillators),
                                     3 * (accuracy.getCurrentSelectedID() - 1) + (typeParam.getCurrentSelectedID() - 1));

        auto freq = static_cast<f32> (freqParam.getCurrentValue());

        for (auto&& oscillator : oscillators)
            oscillator.setFrequency (freq);

        gain.setGainDecibels (static_cast<f32> (gainParam.getCurrentValue()));

        fileMix = mixParam.getCurrentValue();
    }

    //==============================================================================
    Oscillator<f32> oscillators[6] =
    {
        // No Approximation
        {[] (f32 x) { return std::sin (x); }},                   // sine
        {[] (f32 x) { return x / MathConstants<f32>::pi; }},   // saw
        {[] (f32 x) { return x < 0.0f ? -1.0f : 1.0f; }},        // square

        // Approximated by a wave-table
        {[] (f32 x) { return std::sin (x); }, 100},                 // sine
        {[] (f32 x) { return x / MathConstants<f32>::pi; }, 100}, // saw
        {[] (f32 x) { return x < 0.0f ? -1.0f : 1.0f; }, 100}       // square
    };

    i32 currentOscillatorIdx = 0;
    Gain<f32> gain;

    ChoiceParameter typeParam { {"sine", "saw", "square"}, 1, "Type" };
    ChoiceParameter accuracy  { {"No Approximation", "Use Wavetable"}, 1, "Accuracy" };
    SliderParameter freqParam { { 20.0, 24000.0 }, 0.4, 440.0, "Frequency", "Hz" };
    SliderParameter gainParam { { -100.0, 20.0 }, 3.0, -20.0, "Gain", "dB" };
    SliderParameter mixParam  { { 0.0, 1.0 }, 1.0, 0.0, "File mix" };

    HeapBlock<t8> tempBufferMemory;
    AudioBlock<f32> tempBuffer;
    f64 fileMix;

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &accuracy, &freqParam, &gainParam, &mixParam };
};

struct OscillatorDemo final : public Component
{
    OscillatorDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<OscillatorDemoDSP> fileReaderComponent;
};
