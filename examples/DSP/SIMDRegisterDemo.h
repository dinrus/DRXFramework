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

 name:             SIMDRegisterDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      SIMD register demo using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_dsp, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        SIMDRegisterDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

template <typename T>
static T* toBasePointer (SIMDRegister<T>* r) noexcept
{
    return reinterpret_cast<T*> (r);
}

constexpr auto registerSize = dsp::SIMDRegister<f32>::size();

//==============================================================================
struct SIMDRegisterDemoDSP
{
    z0 prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        iirCoefficients = IIR::Coefficients<f32>::makeLowPass (sampleRate, 440.0f);
        iir.reset (new IIR::Filter<SIMDRegister<f32>> (iirCoefficients));

        interleaved = AudioBlock<SIMDRegister<f32>> (interleavedBlockData, 1, spec.maximumBlockSize);
        zero        = AudioBlock<f32> (zeroData, SIMDRegister<f32>::size(), spec.maximumBlockSize);

        zero.clear();

        auto monoSpec = spec;
        monoSpec.numChannels = 1;
        iir->prepare (monoSpec);
    }

    template <typename SampleType>
    auto prepareChannelPointers (const AudioBlock<SampleType>& block)
    {
        std::array<SampleType*, registerSize> result {};

        for (size_t ch = 0; ch < result.size(); ++ch)
            result[ch] = (ch < block.getNumChannels() ? block.getChannelPointer (ch) : zero.getChannelPointer (ch));

        return result;
    }

    z0 process (const ProcessContextReplacing<f32>& context)
    {
        jassert (context.getInputBlock().getNumSamples()  == context.getOutputBlock().getNumSamples());
        jassert (context.getInputBlock().getNumChannels() == context.getOutputBlock().getNumChannels());

        const auto& input  = context.getInputBlock();
        const auto numSamples = (i32) input.getNumSamples();

        auto inChannels = prepareChannelPointers (input);

        using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

        AudioData::interleaveSamples (AudioData::NonInterleavedSource<Format> { inChannels.data(),                                 registerSize, },
                                      AudioData::InterleavedDest<Format>      { toBasePointer (interleaved.getChannelPointer (0)), registerSize },
                                      numSamples);

        iir->process (ProcessContextReplacing<SIMDRegister<f32>> (interleaved));

        auto outChannels = prepareChannelPointers (context.getOutputBlock());

        AudioData::deinterleaveSamples (AudioData::InterleavedSource<Format>  { toBasePointer (interleaved.getChannelPointer (0)), registerSize },
                                        AudioData::NonInterleavedDest<Format> { outChannels.data(),                                registerSize },
                                        numSamples);
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
                case 1:   *iirCoefficients = IIR::ArrayCoefficients<f32>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:   *iirCoefficients = IIR::ArrayCoefficients<f32>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:   *iirCoefficients = IIR::ArrayCoefficients<f32>::makeBandPass (sampleRate, cutoff, qVal); break;
                default:  break;
            }
        }
    }

    //==============================================================================
    IIR::Coefficients<f32>::Ptr iirCoefficients;
    std::unique_ptr<IIR::Filter<SIMDRegister<f32>>> iir;

    AudioBlock<SIMDRegister<f32>> interleaved;
    AudioBlock<f32> zero;

    HeapBlock<t8> interleavedBlockData, zeroData;

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass" }, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 0.7, "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    f64 sampleRate = 0.0;
};

struct SIMDRegisterDemo final : public Component
{
    SIMDRegisterDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    z0 resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<SIMDRegisterDemoDSP> fileReaderComponent;
};
