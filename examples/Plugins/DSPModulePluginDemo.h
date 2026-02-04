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

 name:             DSPModulePluginDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      An audio plugin using the DSP module.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_plugin_client, drx_audio_processors,
                   drx_audio_utils, drx_core, drx_data_structures, drx_dsp,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        DspModulePluginDemoAudioProcessor

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

namespace ID
{
   #define PARAMETER_ID(str) constexpr tukk str { #str };

    PARAMETER_ID (inputGain)
    PARAMETER_ID (outputGain)
    PARAMETER_ID (pan)
    PARAMETER_ID (distortionEnabled)
    PARAMETER_ID (distortionType)
    PARAMETER_ID (distortionOversampler)
    PARAMETER_ID (distortionLowpass)
    PARAMETER_ID (distortionHighpass)
    PARAMETER_ID (distortionInGain)
    PARAMETER_ID (distortionCompGain)
    PARAMETER_ID (distortionMix)
    PARAMETER_ID (convolutionCabEnabled)
    PARAMETER_ID (convolutionReverbEnabled)
    PARAMETER_ID (convolutionReverbMix)
    PARAMETER_ID (multiBandEnabled)
    PARAMETER_ID (multiBandFreq)
    PARAMETER_ID (multiBandLowVolume)
    PARAMETER_ID (multiBandHighVolume)
    PARAMETER_ID (compressorEnabled)
    PARAMETER_ID (compressorThreshold)
    PARAMETER_ID (compressorRatio)
    PARAMETER_ID (compressorAttack)
    PARAMETER_ID (compressorRelease)
    PARAMETER_ID (noiseGateEnabled)
    PARAMETER_ID (noiseGateThreshold)
    PARAMETER_ID (noiseGateRatio)
    PARAMETER_ID (noiseGateAttack)
    PARAMETER_ID (noiseGateRelease)
    PARAMETER_ID (limiterEnabled)
    PARAMETER_ID (limiterThreshold)
    PARAMETER_ID (limiterRelease)
    PARAMETER_ID (directDelayEnabled)
    PARAMETER_ID (directDelayType)
    PARAMETER_ID (directDelayValue)
    PARAMETER_ID (directDelaySmoothing)
    PARAMETER_ID (directDelayMix)
    PARAMETER_ID (delayEffectEnabled)
    PARAMETER_ID (delayEffectType)
    PARAMETER_ID (delayEffectValue)
    PARAMETER_ID (delayEffectSmoothing)
    PARAMETER_ID (delayEffectLowpass)
    PARAMETER_ID (delayEffectFeedback)
    PARAMETER_ID (delayEffectMix)
    PARAMETER_ID (phaserEnabled)
    PARAMETER_ID (phaserRate)
    PARAMETER_ID (phaserDepth)
    PARAMETER_ID (phaserCentreFrequency)
    PARAMETER_ID (phaserFeedback)
    PARAMETER_ID (phaserMix)
    PARAMETER_ID (chorusEnabled)
    PARAMETER_ID (chorusRate)
    PARAMETER_ID (chorusDepth)
    PARAMETER_ID (chorusCentreDelay)
    PARAMETER_ID (chorusFeedback)
    PARAMETER_ID (chorusMix)
    PARAMETER_ID (ladderEnabled)
    PARAMETER_ID (ladderCutoff)
    PARAMETER_ID (ladderResonance)
    PARAMETER_ID (ladderDrive)
    PARAMETER_ID (ladderMode)

   #undef PARAMETER_ID
}

template <typename Func, typename... Items>
constexpr z0 forEach (Func&& func, Items&&... items)
{
    (func (std::forward<Items> (items)), ...);
}

template <typename... Components>
z0 addAllAndMakeVisible (Component& target, Components&... children)
{
    forEach ([&] (Component& child) { target.addAndMakeVisible (child); }, children...);
}

template <typename... Processors>
z0 prepareAll (const dsp::ProcessSpec& spec, Processors&... processors)
{
    forEach ([&] (auto& proc) { proc.prepare (spec); }, processors...);
}

template <typename... Processors>
z0 resetAll (Processors&... processors)
{
    forEach ([] (auto& proc) { proc.reset(); }, processors...);
}

//==============================================================================
class DspModulePluginDemo : public AudioProcessor,
                            private ValueTree::Listener
{
public:
    DspModulePluginDemo()
        : DspModulePluginDemo (AudioProcessorValueTreeState::ParameterLayout{}) {}

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate, i32 samplesPerBlock) final
    {
        const auto channels = jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());

        if (channels == 0)
            return;

        chain.prepare ({ sampleRate, (u32) samplesPerBlock, (u32) channels });

        reset();
    }

    z0 reset() final
    {
        chain.reset();
        update();
    }

    z0 releaseResources() final {}

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer&) final
    {
        if (jmax (getTotalNumInputChannels(), getTotalNumOutputChannels()) == 0)
            return;

        ScopedNoDenormals noDenormals;

        if (requiresUpdate.load())
            update();

        irSize = dsp::get<convolutionIndex> (chain).reverb.getCurrentIRSize();

        const auto totalNumInputChannels  = getTotalNumInputChannels();
        const auto totalNumOutputChannels = getTotalNumOutputChannels();

        setLatencySamples (dsp::get<convolutionIndex> (chain).getLatency()
                           + (dsp::isBypassed<distortionIndex> (chain) ? 0 : roundToInt (dsp::get<distortionIndex> (chain).getLatency())));

        const auto numChannels = jmax (totalNumInputChannels, totalNumOutputChannels);

        auto inoutBlock = dsp::AudioBlock<f32> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
        chain.process (dsp::ProcessContextReplacing<f32> (inoutBlock));
    }

    z0 processBlock (AudioBuffer<f64>&, MidiBuffer&) final {}

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return nullptr; }
    b8 hasEditor() const override { return false; }

    //==============================================================================
    const Txt getName() const final { return "DSPModulePluginDemo"; }

    b8 acceptsMidi()  const final { return false; }
    b8 producesMidi() const final { return false; }
    b8 isMidiEffect() const final { return false; }

    f64 getTailLengthSeconds() const final { return 0.0; }

    //==============================================================================
    i32 getNumPrograms()    final { return 1; }
    i32 getCurrentProgram() final { return 0; }
    z0 setCurrentProgram (i32) final {}
    const Txt getProgramName (i32) final { return "None"; }

    z0 changeProgramName (i32, const Txt&) final {}

    //==============================================================================
    b8 isBusesLayoutSupported (const BusesLayout& layout) const final
    {
        return layout == BusesLayout { { AudioChannelSet::stereo() },
                                       { AudioChannelSet::stereo() } };
    }

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) final
    {
        copyXmlToBinary (*apvts.copyState().createXml(), destData);
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) final
    {
        apvts.replaceState (ValueTree::fromXml (*getXmlFromBinary (data, sizeInBytes)));
    }

    i32 getCurrentIRSize() const { return irSize; }

    using Parameter = AudioProcessorValueTreeState::Parameter;
    using Attributes = AudioProcessorValueTreeStateParameterAttributes;

    // This struct holds references to the raw parameters, so that we don't have to search
    // the APVTS (involving string comparisons and map lookups!) every time a parameter
    // changes.
    struct ParameterReferences
    {
        template <typename Param>
        static z0 add (AudioProcessorParameterGroup& group, std::unique_ptr<Param> param)
        {
            group.addChild (std::move (param));
        }

        template <typename Param>
        static z0 add (AudioProcessorValueTreeState::ParameterLayout& group, std::unique_ptr<Param> param)
        {
            group.add (std::move (param));
        }

        template <typename Param, typename Group, typename... Ts>
        static Param& addToLayout (Group& layout, Ts&&... ts)
        {
            auto param = new Param (std::forward<Ts> (ts)...);
            auto& ref = *param;
            add (layout, rawToUniquePtr (param));
            return ref;
        }

        static Txt valueToTextFunction (f32 x, i32) { return Txt (x, 2); }
        static f32 textToValueFunction (const Txt& str) { return str.getFloatValue(); }

        static auto getBasicAttributes()
        {
            return Attributes().withStringFromValueFunction (valueToTextFunction)
                               .withValueFromStringFunction (textToValueFunction);
        }

        static auto getDbAttributes()           { return getBasicAttributes().withLabel ("dB"); }
        static auto getMsAttributes()           { return getBasicAttributes().withLabel ("ms"); }
        static auto getHzAttributes()           { return getBasicAttributes().withLabel ("Hz"); }
        static auto getPercentageAttributes()   { return getBasicAttributes().withLabel ("%"); }
        static auto getRatioAttributes()        { return getBasicAttributes().withLabel (":1"); }

        static Txt valueToTextPanFunction (f32 x, i32) { return getPanningTextForValue ((x + 100.0f) / 200.0f); }
        static f32 textToValuePanFunction (const Txt& str) { return getPanningValueForText (str) * 200.0f - 100.0f; }

        struct MainGroup
        {
            explicit MainGroup (AudioProcessorParameterGroup& layout)
                : inputGain (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::inputGain, 1 },
                                                     "Input",
                                                     NormalisableRange<f32> (-40.0f, 40.0f),
                                                     0.0f,
                                                     getDbAttributes())),
                  outputGain (addToLayout<Parameter> (layout,
                                                      ParameterID { ID::outputGain, 1 },
                                                      "Output",
                                                      NormalisableRange<f32> (-40.0f, 40.0f),
                                                      0.0f,
                                                      getDbAttributes())),
                  pan (addToLayout<Parameter> (layout,
                                               ParameterID { ID::pan, 1 },
                                               "Panning",
                                               NormalisableRange<f32> (-100.0f, 100.0f),
                                               0.0f,
                                               Attributes().withStringFromValueFunction (valueToTextPanFunction)
                                                           .withValueFromStringFunction (textToValuePanFunction))) {}

            Parameter& inputGain;
            Parameter& outputGain;
            Parameter& pan;
        };

        struct DistortionGroup
        {
            explicit DistortionGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::distortionEnabled, 1 },
                                                            "Distortion",
                                                            true)),
                  type (addToLayout<AudioParameterChoice> (layout,
                                                           ParameterID { ID::distortionType, 1 },
                                                           "Waveshaper",
                                                           StringArray { "std::tanh", "Approx. tanh" },
                                                           0)),
                  inGain (addToLayout<Parameter> (layout,
                                                  ParameterID { ID::distortionInGain, 1 },
                                                  "Gain",
                                                  NormalisableRange<f32> (-40.0f, 40.0f),
                                                  0.0f,
                                                  getDbAttributes())),
                  lowpass (addToLayout<Parameter> (layout,
                                                   ParameterID { ID::distortionLowpass, 1 },
                                                   "Post Low-pass",
                                                   NormalisableRange<f32> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                   22000.0f,
                                                   getHzAttributes())),
                  highpass (addToLayout<Parameter> (layout,
                                                    ParameterID { ID::distortionHighpass, 1 },
                                                    "Pre High-pass",
                                                    NormalisableRange<f32> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                    20.0f,
                                                    getHzAttributes())),
                  compGain (addToLayout<Parameter> (layout,
                                                    ParameterID { ID::distortionCompGain, 1 },
                                                    "Compensat.",
                                                    NormalisableRange<f32> (-40.0f, 40.0f),
                                                    0.0f,
                                                    getDbAttributes())),
                  mix (addToLayout<Parameter> (layout,
                                               ParameterID { ID::distortionMix, 1 },
                                               "Mix",
                                               NormalisableRange<f32> (0.0f, 100.0f),
                                               100.0f,
                                               getPercentageAttributes())),
                  oversampler (addToLayout<AudioParameterChoice> (layout,
                                                                  ParameterID { ID::distortionOversampler, 1 },
                                                                  "Oversampling",
                                                                  StringArray { "2X",
                                                                                "4X",
                                                                                "8X",
                                                                                "2X compensated",
                                                                                "4X compensated",
                                                                                "8X compensated" },
                                                                  1)) {}

            AudioParameterBool& enabled;
            AudioParameterChoice& type;
            Parameter& inGain;
            Parameter& lowpass;
            Parameter& highpass;
            Parameter& compGain;
            Parameter& mix;
            AudioParameterChoice& oversampler;
        };

        struct MultiBandGroup
        {
            explicit MultiBandGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::multiBandEnabled, 1 },
                                                            "Multi-band",
                                                            false)),
                  freq (addToLayout<Parameter> (layout,
                                                ParameterID { ID::multiBandFreq, 1 },
                                                "Sep. Freq.",
                                                NormalisableRange<f32> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                2000.0f,
                                                getHzAttributes())),
                  lowVolume (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::multiBandLowVolume, 1 },
                                                     "Low volume",
                                                     NormalisableRange<f32> (-40.0f, 40.0f),
                                                     0.0f,
                                                     getDbAttributes())),
                  highVolume (addToLayout<Parameter> (layout,
                                                      ParameterID { ID::multiBandHighVolume, 1 },
                                                      "High volume",
                                                      NormalisableRange<f32> (-40.0f, 40.0f),
                                                      0.0f,
                                                      getDbAttributes())) {}

            AudioParameterBool& enabled;
            Parameter& freq;
            Parameter& lowVolume;
            Parameter& highVolume;
        };

        struct ConvolutionGroup
        {
            explicit ConvolutionGroup (AudioProcessorParameterGroup& layout)
                : cabEnabled (addToLayout<AudioParameterBool> (layout,
                                                               ParameterID { ID::convolutionCabEnabled, 1 },
                                                               "Cabinet",
                                                               false)),
                  reverbEnabled (addToLayout<AudioParameterBool> (layout,
                                                                  ParameterID { ID::convolutionReverbEnabled, 1 },
                                                                  "Reverb",
                                                                  false)),
                  reverbMix (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::convolutionReverbMix, 1 },
                                                     "Reverb Mix",
                                                     NormalisableRange<f32> (0.0f, 100.0f),
                                                     50.0f,
                                                     getPercentageAttributes())) {}

            AudioParameterBool& cabEnabled;
            AudioParameterBool& reverbEnabled;
            Parameter& reverbMix;
        };

        struct CompressorGroup
        {
            explicit CompressorGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::compressorEnabled, 1 },
                                                            "Comp.",
                                                            false)),
                  threshold (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::compressorThreshold, 1 },
                                                     "Threshold",
                                                     NormalisableRange<f32> (-100.0f, 0.0f),
                                                     0.0f,
                                                     getDbAttributes())),
                  ratio (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::compressorRatio, 1 },
                                                 "Ratio",
                                                 NormalisableRange<f32> (1.0f, 100.0f, 0.0f, 0.25f),
                                                 1.0f,
                                                 getRatioAttributes())),
                  attack (addToLayout<Parameter> (layout,
                                                  ParameterID { ID::compressorAttack, 1 },
                                                  "Attack",
                                                  NormalisableRange<f32> (0.01f, 1000.0f, 0.0f, 0.25f),
                                                  1.0f,
                                                  getMsAttributes())),
                  release (addToLayout<Parameter> (layout,
                                                   ParameterID { ID::compressorRelease, 1 },
                                                   "Release",
                                                   NormalisableRange<f32> (10.0f, 10000.0f, 0.0f, 0.25f),
                                                   100.0f,
                                                   getMsAttributes())) {}

            AudioParameterBool& enabled;
            Parameter& threshold;
            Parameter& ratio;
            Parameter& attack;
            Parameter& release;
        };

        struct NoiseGateGroup
        {
            explicit NoiseGateGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::noiseGateEnabled, 1 },
                                                            "Gate",
                                                            false)),
                  threshold (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::noiseGateThreshold, 1 },
                                                     "Threshold",
                                                     NormalisableRange<f32> (-100.0f, 0.0f),
                                                     -100.0f,
                                                     getDbAttributes())),
                  ratio (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::noiseGateRatio, 1 },
                                                 "Ratio",
                                                 NormalisableRange<f32> (1.0f, 100.0f, 0.0f, 0.25f),
                                                 10.0f,
                                                 getRatioAttributes())),
                  attack (addToLayout<Parameter> (layout,
                                                  ParameterID { ID::noiseGateAttack, 1 },
                                                  "Attack",
                                                  NormalisableRange<f32> (0.01f, 1000.0f, 0.0f, 0.25f),
                                                  1.0f,
                                                  getMsAttributes())),
                  release (addToLayout<Parameter> (layout,
                                                   ParameterID { ID::noiseGateRelease, 1 },
                                                   "Release",
                                                   NormalisableRange<f32> (10.0f, 10000.0f, 0.0f, 0.25f),
                                                   100.0f,
                                                   getMsAttributes())) {}

            AudioParameterBool& enabled;
            Parameter& threshold;
            Parameter& ratio;
            Parameter& attack;
            Parameter& release;
        };

        struct LimiterGroup
        {
            explicit LimiterGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::limiterEnabled, 1 },
                                                            "Limiter",
                                                            false)),
                  threshold (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::limiterThreshold, 1 },
                                                     "Threshold",
                                                     NormalisableRange<f32> (-40.0f, 0.0f),
                                                     0.0f,
                                                     getDbAttributes())),
                  release (addToLayout<Parameter> (layout,
                                                   ParameterID { ID::limiterRelease, 1 },
                                                   "Release",
                                                   NormalisableRange<f32> (10.0f, 10000.0f, 0.0f, 0.25f),
                                                   100.0f,
                                                   getMsAttributes())) {}

            AudioParameterBool& enabled;
            Parameter& threshold;
            Parameter& release;
        };

        struct DirectDelayGroup
        {
            explicit DirectDelayGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::directDelayEnabled, 1 },
                                                            "DL Dir.",
                                                            false)),
                  type (addToLayout<AudioParameterChoice> (layout,
                                                           ParameterID { ID::directDelayType, 1 },
                                                           "DL Type",
                                                           StringArray { "None", "Linear", "Lagrange", "Thiran" },
                                                           1)),
                  value (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::directDelayValue, 1 },
                                                 "Delay",
                                                 NormalisableRange<f32> (0.0f, 44100.0f),
                                                 0.0f,
                                                 getBasicAttributes().withLabel ("smps"))),
                  smoothing (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::directDelaySmoothing, 1 },
                                                     "Smooth",
                                                     NormalisableRange<f32> (20.0f, 10000.0f, 0.0f, 0.25f),
                                                     200.0f,
                                                     getMsAttributes())),
                  mix (addToLayout<Parameter> (layout,
                                               ParameterID { ID::directDelayMix, 1 },
                                               "Delay Mix",
                                               NormalisableRange<f32> (0.0f, 100.0f),
                                               50.0f,
                                               getPercentageAttributes())) {}

            AudioParameterBool& enabled;
            AudioParameterChoice& type;
            Parameter& value;
            Parameter& smoothing;
            Parameter& mix;
        };

        struct DelayEffectGroup
        {
            explicit DelayEffectGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::delayEffectEnabled, 1 },
                                                            "DL Effect",
                                                            false)),
                  type (addToLayout<AudioParameterChoice> (layout,
                                                           ParameterID { ID::delayEffectType, 1 },
                                                           "DL Type",
                                                           StringArray { "None", "Linear", "Lagrange", "Thiran" },
                                                           1)),
                  value (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::delayEffectValue, 1 },
                                                 "Delay",
                                                 NormalisableRange<f32> (0.01f, 1000.0f),
                                                 100.0f,
                                                 getMsAttributes())),
                  smoothing (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::delayEffectSmoothing, 1 },
                                                     "Smooth",
                                                     NormalisableRange<f32> (20.0f, 10000.0f, 0.0f, 0.25f),
                                                     400.0f,
                                                     getMsAttributes())),
                  lowpass (addToLayout<Parameter> (layout,
                                                   ParameterID { ID::delayEffectLowpass, 1 },
                                                   "Low-pass",
                                                   NormalisableRange<f32> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                   22000.0f,
                                                   getHzAttributes())),
                  mix (addToLayout<Parameter> (layout,
                                               ParameterID { ID::delayEffectMix, 1 },
                                               "Delay Mix",
                                               NormalisableRange<f32> (0.0f, 100.0f),
                                               50.0f,
                                               getPercentageAttributes())),
                  feedback (addToLayout<Parameter> (layout,
                                                    ParameterID { ID::delayEffectFeedback, 1 },
                                                    "Feedback",
                                                    NormalisableRange<f32> (-100.0f, 0.0f),
                                                    -100.0f,
                                                    getDbAttributes())) {}

            AudioParameterBool& enabled;
            AudioParameterChoice& type;
            Parameter& value;
            Parameter& smoothing;
            Parameter& lowpass;
            Parameter& mix;
            Parameter& feedback;
        };

        struct PhaserGroup
        {
            explicit PhaserGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::phaserEnabled, 1 },
                                                            "Phaser",
                                                            false)),
                  rate (addToLayout<Parameter> (layout,
                                                ParameterID { ID::phaserRate, 1 },
                                                "Rate",
                                                NormalisableRange<f32> (0.05f, 20.0f, 0.0f, 0.25f),
                                                1.0f,
                                                getHzAttributes())),
                  depth (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::phaserDepth, 1 },
                                                 "Depth",
                                                 NormalisableRange<f32> (0.0f, 100.0f),
                                                 50.0f,
                                                 getPercentageAttributes())),
                  centreFrequency (addToLayout<Parameter> (layout,
                                                           ParameterID { ID::phaserCentreFrequency, 1 },
                                                           "Center",
                                                           NormalisableRange<f32> (20.0f, 20000.0f, 0.0f, 0.25f),
                                                           600.0f,
                                                           getHzAttributes())),
                  feedback (addToLayout<Parameter> (layout,
                                                    ParameterID { ID::phaserFeedback, 1 },
                                                    "Feedback",
                                                    NormalisableRange<f32> (0.0f, 100.0f),
                                                    50.0f,
                                                    getPercentageAttributes())),
                  mix (addToLayout<Parameter> (layout,
                                               ParameterID { ID::phaserMix, 1 },
                                               "Mix",
                                               NormalisableRange<f32> (0.0f, 100.0f),
                                               50.0f,
                                               getPercentageAttributes())) {}

            AudioParameterBool& enabled;
            Parameter& rate;
            Parameter& depth;
            Parameter& centreFrequency;
            Parameter& feedback;
            Parameter& mix;
        };

        struct ChorusGroup
        {
            explicit ChorusGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::chorusEnabled, 1 },
                                                            "Chorus",
                                                            false)),
                  rate (addToLayout<Parameter> (layout,
                                                ParameterID { ID::chorusRate, 1 },
                                                "Rate",
                                                NormalisableRange<f32> (0.05f, 20.0f, 0.0f, 0.25f),
                                                1.0f,
                                                getHzAttributes())),
                  depth (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::chorusDepth, 1 },
                                                 "Depth",
                                                 NormalisableRange<f32> (0.0f, 100.0f),
                                                 50.0f,
                                                 getPercentageAttributes())),
                  centreDelay (addToLayout<Parameter> (layout,
                                                       ParameterID { ID::chorusCentreDelay, 1 },
                                                       "Center",
                                                       NormalisableRange<f32> (1.0f, 100.0f, 0.0f, 0.25f),
                                                       7.0f,
                                                       getMsAttributes())),
                  feedback (addToLayout<Parameter> (layout,
                                                    ParameterID { ID::chorusFeedback, 1 },
                                                    "Feedback",
                                                    NormalisableRange<f32> (0.0f, 100.0f),
                                                    50.0f,
                                                    getPercentageAttributes())),
                  mix (addToLayout<Parameter> (layout,
                                               ParameterID { ID::chorusMix, 1 },
                                               "Mix",
                                               NormalisableRange<f32> (0.0f, 100.0f),
                                               50.0f,
                                               getPercentageAttributes())) {}

            AudioParameterBool& enabled;
            Parameter& rate;
            Parameter& depth;
            Parameter& centreDelay;
            Parameter& feedback;
            Parameter& mix;
        };

        struct LadderGroup
        {
            explicit LadderGroup (AudioProcessorParameterGroup& layout)
                : enabled (addToLayout<AudioParameterBool> (layout,
                                                            ParameterID { ID::ladderEnabled, 1 },
                                                            "Ladder",
                                                            false)),
                  mode (addToLayout<AudioParameterChoice> (layout,
                                                           ParameterID { ID::ladderMode, 1 },
                                                           "Mode",
                                                           StringArray { "LP12", "LP24", "HP12", "HP24", "BP12", "BP24" },
                                                           1)),
                  cutoff (addToLayout<Parameter> (layout,
                                                  ParameterID { ID::ladderCutoff, 1 },
                                                  "Frequency",
                                                  NormalisableRange<f32> (10.0f, 22000.0f, 0.0f, 0.25f),
                                                  1000.0f,
                                                  getHzAttributes())),
                  resonance (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::ladderResonance, 1 },
                                                     "Resonance",
                                                     NormalisableRange<f32> (0.0f, 100.0f),
                                                     0.0f,
                                                     getPercentageAttributes())),
                  drive (addToLayout<Parameter> (layout,
                                                 ParameterID { ID::ladderDrive, 1 },
                                                 "Drive",
                                                 NormalisableRange<f32> (0.0f, 40.0f),
                                                 0.0f,
                                                 getDbAttributes())) {}

            AudioParameterBool& enabled;
            AudioParameterChoice& mode;
            Parameter& cutoff;
            Parameter& resonance;
            Parameter& drive;
        };

        explicit ParameterReferences (AudioProcessorValueTreeState::ParameterLayout& layout)
            : main          (addToLayout<AudioProcessorParameterGroup> (layout, "main",          "Main",          "|")),
              distortion    (addToLayout<AudioProcessorParameterGroup> (layout, "distortion",    "Distortion",    "|")),
              multiBand     (addToLayout<AudioProcessorParameterGroup> (layout, "multiband",     "Multi Band",    "|")),
              convolution   (addToLayout<AudioProcessorParameterGroup> (layout, "convolution",   "Convolution",   "|")),
              compressor    (addToLayout<AudioProcessorParameterGroup> (layout, "compressor",    "Compressor",    "|")),
              noiseGate     (addToLayout<AudioProcessorParameterGroup> (layout, "noisegate",     "Noise Gate",    "|")),
              limiter       (addToLayout<AudioProcessorParameterGroup> (layout, "limiter",       "Limiter",       "|")),
              directDelay   (addToLayout<AudioProcessorParameterGroup> (layout, "directdelay",   "Direct Delay",  "|")),
              delayEffect   (addToLayout<AudioProcessorParameterGroup> (layout, "delayeffect",   "Delay Effect",  "|")),
              phaser        (addToLayout<AudioProcessorParameterGroup> (layout, "phaser",        "Phaser",        "|")),
              chorus        (addToLayout<AudioProcessorParameterGroup> (layout, "chorus",        "Chorus",        "|")),
              ladder        (addToLayout<AudioProcessorParameterGroup> (layout, "ladder",        "Ladder",        "|")) {}

        MainGroup main;
        DistortionGroup distortion;
        MultiBandGroup multiBand;
        ConvolutionGroup convolution;
        CompressorGroup compressor;
        NoiseGateGroup noiseGate;
        LimiterGroup limiter;
        DirectDelayGroup directDelay;
        DelayEffectGroup delayEffect;
        PhaserGroup phaser;
        ChorusGroup chorus;
        LadderGroup ladder;
    };

    const ParameterReferences& getParameterValues() const noexcept { return parameters; }

    //==============================================================================
    // We store this here so that the editor retains its state if it is closed and reopened
    i32 indexTab = 0;

private:
    struct LayoutAndReferences
    {
        AudioProcessorValueTreeState::ParameterLayout layout;
        ParameterReferences references;
    };

    explicit DspModulePluginDemo (AudioProcessorValueTreeState::ParameterLayout layout)
        : AudioProcessor (BusesProperties().withInput ("In",   AudioChannelSet::stereo())
                                           .withOutput ("Out", AudioChannelSet::stereo())),
          parameters { layout },
          apvts { *this, nullptr, "state", std::move (layout) }
    {
        apvts.state.addListener (this);

        forEach ([] (dsp::Gain<f32>& gain) { gain.setRampDurationSeconds (0.05); },
                 dsp::get<inputGainIndex>  (chain),
                 dsp::get<outputGainIndex> (chain));

        dsp::get<pannerIndex> (chain).setRule (dsp::PannerRule::linear);
    }

    //==============================================================================
    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) final
    {
        requiresUpdate.store (true);
    }

    //==============================================================================
    z0 update()
    {
        {
            DistortionProcessor& distortion = dsp::get<distortionIndex> (chain);

            if (distortion.currentIndexOversampling != parameters.distortion.oversampler.getIndex())
            {
                distortion.currentIndexOversampling = parameters.distortion.oversampler.getIndex();
                prepareToPlay (getSampleRate(), getBlockSize());
                return;
            }

            distortion.currentIndexWaveshaper = parameters.distortion.type.getIndex();
            distortion.lowpass .setCutoffFrequency (parameters.distortion.lowpass.get());
            distortion.highpass.setCutoffFrequency (parameters.distortion.highpass.get());
            distortion.distGain.setGainDecibels (parameters.distortion.inGain.get());
            distortion.compGain.setGainDecibels (parameters.distortion.compGain.get());
            distortion.mixer.setWetMixProportion (parameters.distortion.mix.get() / 100.0f);
            dsp::setBypassed<distortionIndex> (chain, ! parameters.distortion.enabled);
        }

        {
            ConvolutionProcessor& convolution = dsp::get<convolutionIndex> (chain);
            convolution.cabEnabled    = parameters.convolution.cabEnabled;
            convolution.reverbEnabled = parameters.convolution.reverbEnabled;
            convolution.mixer.setWetMixProportion (parameters.convolution.reverbMix.get() / 100.0f);
        }

        dsp::get<inputGainIndex>  (chain).setGainDecibels (parameters.main.inputGain.get());
        dsp::get<outputGainIndex> (chain).setGainDecibels (parameters.main.outputGain.get());
        dsp::get<pannerIndex> (chain).setPan (parameters.main.pan.get() / 100.0f);

        {
            MultiBandProcessor& multiband = dsp::get<multiBandIndex> (chain);
            const auto multibandFreq = parameters.multiBand.freq.get();
            multiband.lowpass .setCutoffFrequency (multibandFreq);
            multiband.highpass.setCutoffFrequency (multibandFreq);
            const b8 enabled = parameters.multiBand.enabled;
            multiband.lowVolume .setGainDecibels (enabled ? parameters.multiBand.lowVolume .get() : 0.0f);
            multiband.highVolume.setGainDecibels (enabled ? parameters.multiBand.highVolume.get() : 0.0f);
            dsp::setBypassed<multiBandIndex> (chain, ! enabled);
        }

        {
            dsp::Compressor<f32>& compressor = dsp::get<compressorIndex> (chain);
            compressor.setThreshold (parameters.compressor.threshold.get());
            compressor.setRatio     (parameters.compressor.ratio.get());
            compressor.setAttack    (parameters.compressor.attack.get());
            compressor.setRelease   (parameters.compressor.release.get());
            dsp::setBypassed<compressorIndex> (chain, ! parameters.compressor.enabled);
        }

        {
            dsp::NoiseGate<f32>& noiseGate = dsp::get<noiseGateIndex> (chain);
            noiseGate.setThreshold (parameters.noiseGate.threshold.get());
            noiseGate.setRatio     (parameters.noiseGate.ratio.get());
            noiseGate.setAttack    (parameters.noiseGate.attack.get());
            noiseGate.setRelease   (parameters.noiseGate.release.get());
            dsp::setBypassed<noiseGateIndex> (chain, ! parameters.noiseGate.enabled);
        }

        {
            dsp::Limiter<f32>& limiter = dsp::get<limiterIndex> (chain);
            limiter.setThreshold (parameters.limiter.threshold.get());
            limiter.setRelease   (parameters.limiter.release.get());
            dsp::setBypassed<limiterIndex> (chain, ! parameters.limiter.enabled);
        }

        {
            DirectDelayProcessor& delay = dsp::get<directDelayIndex> (chain);
            delay.delayLineDirectType = parameters.directDelay.type.getIndex();

            std::fill (delay.delayDirectValue.begin(),
                       delay.delayDirectValue.end(),
                       (f64) parameters.directDelay.value.get());

            delay.smoothFilter.setCutoffFrequency (1000.0 / parameters.directDelay.smoothing.get());
            delay.mixer.setWetMixProportion (parameters.directDelay.mix.get() / 100.0f);
            dsp::setBypassed<directDelayIndex> (chain, ! parameters.directDelay.enabled);
        }

        {
            DelayEffectProcessor& delay = dsp::get<delayEffectIndex> (chain);
            delay.delayEffectType = parameters.delayEffect.type.getIndex();

            std::fill (delay.delayEffectValue.begin(),
                       delay.delayEffectValue.end(),
                       (f64) parameters.delayEffect.value.get() / 1000.0 * getSampleRate());

            const auto feedbackGain = Decibels::decibelsToGain (parameters.delayEffect.feedback.get(), -100.0f);

            for (auto& volume : delay.delayFeedbackVolume)
                volume.setTargetValue (feedbackGain);

            delay.smoothFilter.setCutoffFrequency (1000.0 / parameters.delayEffect.smoothing.get());
            delay.lowpass.setCutoffFrequency (parameters.delayEffect.lowpass.get());
            delay.mixer.setWetMixProportion (parameters.delayEffect.mix.get() / 100.0f);
            dsp::setBypassed<delayEffectIndex> (chain, ! parameters.delayEffect.enabled);
        }

        {
            dsp::Phaser<f32>& phaser = dsp::get<phaserIndex> (chain);
            phaser.setRate            (parameters.phaser.rate.get());
            phaser.setDepth           (parameters.phaser.depth.get() / 100.0f);
            phaser.setCentreFrequency (parameters.phaser.centreFrequency.get());
            phaser.setFeedback        (parameters.phaser.feedback.get() / 100.0f * 0.95f);
            phaser.setMix             (parameters.phaser.mix.get() / 100.0f);
            dsp::setBypassed<phaserIndex> (chain, ! parameters.phaser.enabled);
        }

        {
            dsp::Chorus<f32>& chorus = dsp::get<chorusIndex> (chain);
            chorus.setRate        (parameters.chorus.rate.get());
            chorus.setDepth       (parameters.chorus.depth.get() / 100.0f);
            chorus.setCentreDelay (parameters.chorus.centreDelay.get());
            chorus.setFeedback    (parameters.chorus.feedback.get() / 100.0f * 0.95f);
            chorus.setMix         (parameters.chorus.mix.get() / 100.0f);
            dsp::setBypassed<chorusIndex> (chain, ! parameters.chorus.enabled);
        }

        {
            dsp::LadderFilter<f32>& ladder = dsp::get<ladderIndex> (chain);

            ladder.setCutoffFrequencyHz (parameters.ladder.cutoff.get());
            ladder.setResonance         (parameters.ladder.resonance.get() / 100.0f);
            ladder.setDrive (Decibels::decibelsToGain (parameters.ladder.drive.get()));

            ladder.setMode ([&]
            {
                switch (parameters.ladder.mode.getIndex())
                {
                    case 0: return dsp::LadderFilterMode::LPF12;
                    case 1: return dsp::LadderFilterMode::LPF24;
                    case 2: return dsp::LadderFilterMode::HPF12;
                    case 3: return dsp::LadderFilterMode::HPF24;
                    case 4: return dsp::LadderFilterMode::BPF12;

                    default: break;
                }

                return dsp::LadderFilterMode::BPF24;
            }());

            dsp::setBypassed<ladderIndex> (chain, ! parameters.ladder.enabled);
        }

        requiresUpdate.store (false);
    }

    //==============================================================================
    static Txt getPanningTextForValue (f32 value)
    {
        if (approximatelyEqual (value, 0.5f))
            return "center";

        if (value < 0.5f)
            return Txt (roundToInt ((0.5f - value) * 200.0f)) + "%L";

        return Txt (roundToInt ((value - 0.5f) * 200.0f)) + "%R";
    }

    static f32 getPanningValueForText (Txt strText)
    {
        if (strText.compareIgnoreCase ("center") == 0 || strText.compareIgnoreCase ("c") == 0)
            return 0.5f;

        strText = strText.trim();

        if (strText.indexOfIgnoreCase ("%L") != -1)
        {
            auto percentage = (f32) strText.substring (0, strText.indexOf ("%")).getDoubleValue();
            return (100.0f - percentage) / 100.0f * 0.5f;
        }

        if (strText.indexOfIgnoreCase ("%R") != -1)
        {
            auto percentage = (f32) strText.substring (0, strText.indexOf ("%")).getDoubleValue();
            return percentage / 100.0f * 0.5f + 0.5f;
        }

        return 0.5f;
    }

    //==============================================================================
    struct DistortionProcessor
    {
        DistortionProcessor()
        {
            forEach ([] (dsp::Gain<f32>& gain) { gain.setRampDurationSeconds (0.05); },
                     distGain,
                     compGain);

            lowpass.setType  (dsp::FirstOrderTPTFilterType::lowpass);
            highpass.setType (dsp::FirstOrderTPTFilterType::highpass);
            mixer.setMixingRule (dsp::DryWetMixingRule::linear);
        }

        z0 prepare (const dsp::ProcessSpec& spec)
        {
            for (auto& oversampler : oversamplers)
                oversampler.initProcessing (spec.maximumBlockSize);

            prepareAll (spec, lowpass, highpass, distGain, compGain, mixer);
        }

        z0 reset()
        {
            for (auto& oversampler : oversamplers)
                oversampler.reset();

            resetAll (lowpass, highpass, distGain, compGain, mixer);
        }

        f32 getLatency() const
        {
            return oversamplers[size_t (currentIndexOversampling)].getLatencyInSamples();
        }

        template <typename Context>
        z0 process (Context& context)
        {
            if (context.isBypassed)
                return;

            const auto& inputBlock = context.getInputBlock();

            mixer.setWetLatency (getLatency());
            mixer.pushDrySamples (inputBlock);

            distGain.process (context);
            highpass.process (context);

            auto ovBlock = oversamplers[size_t (currentIndexOversampling)].processSamplesUp (inputBlock);

            dsp::ProcessContextReplacing<f32> waveshaperContext (ovBlock);

            if (isPositiveAndBelow (currentIndexWaveshaper, waveShapers.size()))
            {
                waveShapers[size_t (currentIndexWaveshaper)].process (waveshaperContext);

                if (currentIndexWaveshaper == 1)
                    clipping.process (waveshaperContext);

                waveshaperContext.getOutputBlock() *= 0.7f;
            }

            auto& outputBlock = context.getOutputBlock();
            oversamplers[size_t (currentIndexOversampling)].processSamplesDown (outputBlock);

            lowpass.process (context);
            compGain.process (context);
            mixer.mixWetSamples (outputBlock);
        }

        std::array<dsp::Oversampling<f32>, 6> oversamplers
        { {
            { 2, 1, dsp::Oversampling<f32>::filterHalfBandPolyphaseIIR, true, false },
            { 2, 2, dsp::Oversampling<f32>::filterHalfBandPolyphaseIIR, true, false },
            { 2, 3, dsp::Oversampling<f32>::filterHalfBandPolyphaseIIR, true, false },

            { 2, 1, dsp::Oversampling<f32>::filterHalfBandPolyphaseIIR, true, true },
            { 2, 2, dsp::Oversampling<f32>::filterHalfBandPolyphaseIIR, true, true },
            { 2, 3, dsp::Oversampling<f32>::filterHalfBandPolyphaseIIR, true, true },
        } };

        static f32 clip (f32 in) { return drx::jlimit (-1.0f, 1.0f, in); }

        dsp::FirstOrderTPTFilter<f32> lowpass, highpass;
        dsp::Gain<f32> distGain, compGain;
        dsp::DryWetMixer<f32> mixer { 10 };
        std::array<dsp::WaveShaper<f32>, 2> waveShapers { { { std::tanh },
                                                              { dsp::FastMathApproximations::tanh } } };
        dsp::WaveShaper<f32> clipping { clip };
        i32 currentIndexOversampling = 0;
        i32 currentIndexWaveshaper   = 0;
    };

    struct ConvolutionProcessor
    {
        ConvolutionProcessor()
        {
            loadImpulseResponse (cabinet, "guitar_amp.wav");
            loadImpulseResponse (reverb,  "reverb_ir.wav");
            mixer.setMixingRule (dsp::DryWetMixingRule::balanced);
        }

        z0 prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, cabinet, reverb, mixer);
        }

        z0 reset()
        {
            resetAll (cabinet, reverb, mixer);
        }

        template <typename Context>
        z0 process (Context& context)
        {
            auto contextConv = context;
            contextConv.isBypassed = (! cabEnabled) || context.isBypassed;
            cabinet.process (contextConv);

            if (cabEnabled)
                context.getOutputBlock().multiplyBy (4.0f);

            if (reverbEnabled)
                mixer.pushDrySamples (context.getInputBlock());

            contextConv.isBypassed = (! reverbEnabled) || context.isBypassed;
            reverb.process (contextConv);

            if (reverbEnabled)
            {
                const auto& outputBlock = context.getOutputBlock();
                outputBlock.multiplyBy (4.0f);
                mixer.mixWetSamples (outputBlock);
            }
        }

        i32 getLatency() const
        {
            auto latency = 0;

            if (cabEnabled)
                latency += cabinet.getLatency();

            if (reverbEnabled)
                latency += reverb.getLatency();

            return latency;
        }

        dsp::ConvolutionMessageQueue queue;
        dsp::Convolution cabinet { dsp::Convolution::NonUniform { 512 }, queue };
        dsp::Convolution reverb { dsp::Convolution::NonUniform { 512 }, queue };
        dsp::DryWetMixer<f32> mixer;
        b8 cabEnabled = false, reverbEnabled = false;

    private:
        static z0 loadImpulseResponse (dsp::Convolution& convolution, tukk filename)
        {
            auto stream = createAssetInputStream (filename);

            if (stream == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioFormatManager manager;
            manager.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> reader { manager.createReaderFor (std::move (stream)) };

            if (reader == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioBuffer<f32> buffer (static_cast<i32> (reader->numChannels),
                                       static_cast<i32> (reader->lengthInSamples));
            reader->read (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, buffer.getNumSamples());

            convolution.loadImpulseResponse (std::move (buffer),
                                             reader->sampleRate,
                                             dsp::Convolution::Stereo::yes,
                                             dsp::Convolution::Trim::yes,
                                             dsp::Convolution::Normalise::yes);
        }
    };

    struct MultiBandProcessor
    {
        MultiBandProcessor()
        {
            forEach ([] (dsp::Gain<f32>& gain) { gain.setRampDurationSeconds (0.05); },
                     lowVolume,
                     highVolume);

            lowpass .setType (dsp::LinkwitzRileyFilterType::lowpass);
            highpass.setType (dsp::LinkwitzRileyFilterType::highpass);
        }

        z0 prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, lowpass, highpass, lowVolume, highVolume);
            bufferSeparation.setSize (4, i32 (spec.maximumBlockSize), false, false, true);
        }

        z0 reset()
        {
            resetAll (lowpass, highpass, lowVolume, highVolume);
        }

        template <typename Context>
        z0 process (Context& context)
        {
            const auto& inputBlock = context.getInputBlock();

            const auto numSamples  = inputBlock.getNumSamples();
            const auto numChannels = inputBlock.getNumChannels();

            auto sepBlock = dsp::AudioBlock<f32> (bufferSeparation).getSubBlock (0, (size_t) numSamples);

            auto sepLowBlock  = sepBlock.getSubsetChannelBlock (0, (size_t) numChannels);
            auto sepHighBlock = sepBlock.getSubsetChannelBlock (2, (size_t) numChannels);

            sepLowBlock .copyFrom (inputBlock);
            sepHighBlock.copyFrom (inputBlock);

            auto contextLow = dsp::ProcessContextReplacing<f32> (sepLowBlock);
            contextLow.isBypassed = context.isBypassed;
            lowpass  .process (contextLow);
            lowVolume.process (contextLow);

            auto contextHigh = dsp::ProcessContextReplacing<f32> (sepHighBlock);
            contextHigh.isBypassed = context.isBypassed;
            highpass  .process (contextHigh);
            highVolume.process (contextHigh);

            if (! context.isBypassed)
            {
                sepLowBlock.add (sepHighBlock);
                context.getOutputBlock().copyFrom (sepLowBlock);
            }
        }

        dsp::LinkwitzRileyFilter<f32> lowpass, highpass;
        dsp::Gain<f32> lowVolume, highVolume;
        AudioBuffer<f32> bufferSeparation;
    };

    struct DirectDelayProcessor
    {
        DirectDelayProcessor()
        {
            smoothFilter.setType (dsp::FirstOrderTPTFilterType::lowpass);
            mixer.setMixingRule (dsp::DryWetMixingRule::linear);
        }

        z0 prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, noInterpolation, linear, lagrange, thiran, smoothFilter, mixer);
        }

        z0 reset()
        {
            resetAll (noInterpolation, linear, lagrange, thiran, smoothFilter, mixer);
        }

        template <typename Context>
        z0 process (Context& context)
        {
            if (context.isBypassed)
                return;

            const auto& inputBlock  = context.getInputBlock();
            const auto& outputBlock = context.getOutputBlock();

            mixer.pushDrySamples (inputBlock);

            const auto numChannels = inputBlock.getNumChannels();
            const auto numSamples  = inputBlock.getNumSamples();

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* samplesIn  = inputBlock .getChannelPointer (channel);
                auto* samplesOut = outputBlock.getChannelPointer (channel);

                for (size_t i = 0; i < numSamples; ++i)
                {
                    const auto delay = smoothFilter.processSample (i32 (channel), delayDirectValue[channel]);

                    samplesOut[i] = [&]
                    {
                        switch (delayLineDirectType)
                        {
                            case 0:
                                noInterpolation.pushSample (i32 (channel), samplesIn[i]);
                                noInterpolation.setDelay ((f32) delay);
                                return noInterpolation.popSample (i32 (channel));

                            case 1:
                                linear.pushSample (i32 (channel), samplesIn[i]);
                                linear.setDelay ((f32) delay);
                                return linear.popSample (i32 (channel));

                            case 2:
                                lagrange.pushSample (i32 (channel), samplesIn[i]);
                                lagrange.setDelay ((f32) delay);
                                return lagrange.popSample (i32 (channel));

                            case 3:
                                thiran.pushSample (i32 (channel), samplesIn[i]);
                                thiran.setDelay ((f32) delay);
                                return thiran.popSample (i32 (channel));

                            default:
                                break;
                        }

                        jassertfalse;
                        return 0.0f;
                    }();
                }
            }

            mixer.mixWetSamples (outputBlock);
        }

        static constexpr auto directDelayBufferSize = 44100;
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::None>        noInterpolation { directDelayBufferSize };
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::Linear>      linear          { directDelayBufferSize };
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::Lagrange3rd> lagrange        { directDelayBufferSize };
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::Thiran>      thiran          { directDelayBufferSize };

        // Double precision to avoid some approximation issues
        dsp::FirstOrderTPTFilter<f64> smoothFilter;

        dsp::DryWetMixer<f32> mixer;
        std::array<f64, 2> delayDirectValue { {} };

        i32 delayLineDirectType = 1;
    };

    struct DelayEffectProcessor
    {
        DelayEffectProcessor()
        {
            smoothFilter.setType (dsp::FirstOrderTPTFilterType::lowpass);
            lowpass.setType      (dsp::FirstOrderTPTFilterType::lowpass);
            mixer.setMixingRule (dsp::DryWetMixingRule::linear);
        }

        z0 prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, noInterpolation, linear, lagrange, thiran, smoothFilter, lowpass, mixer);

            for (auto& volume : delayFeedbackVolume)
                volume.reset (spec.sampleRate, 0.05);
        }

        z0 reset()
        {
            resetAll (noInterpolation, linear, lagrange, thiran, smoothFilter, lowpass, mixer);
            std::fill (lastDelayEffectOutput.begin(), lastDelayEffectOutput.end(), 0.0f);
        }

        template <typename Context>
        z0 process (Context& context)
        {
            if (context.isBypassed)
                return;

            const auto& inputBlock  = context.getInputBlock();
            const auto& outputBlock = context.getOutputBlock();
            const auto numSamples  = inputBlock.getNumSamples();
            const auto numChannels = inputBlock.getNumChannels();

            mixer.pushDrySamples (inputBlock);

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* samplesIn  = inputBlock .getChannelPointer (channel);
                auto* samplesOut = outputBlock.getChannelPointer (channel);

                for (size_t i = 0; i < numSamples; ++i)
                {
                    auto input = samplesIn[i] - lastDelayEffectOutput[channel];

                    auto delay = smoothFilter.processSample (i32 (channel), delayEffectValue[channel]);

                    const auto output = [&]
                    {
                        switch (delayEffectType)
                        {
                            case 0:
                                noInterpolation.pushSample (i32 (channel), input);
                                noInterpolation.setDelay ((f32) delay);
                                return noInterpolation.popSample (i32 (channel));

                            case 1:
                                linear.pushSample (i32 (channel), input);
                                linear.setDelay ((f32) delay);
                                return linear.popSample (i32 (channel));

                            case 2:
                                lagrange.pushSample (i32 (channel), input);
                                lagrange.setDelay ((f32) delay);
                                return lagrange.popSample (i32 (channel));

                            case 3:
                                thiran.pushSample (i32 (channel), input);
                                thiran.setDelay ((f32) delay);
                                return thiran.popSample (i32 (channel));

                            default:
                                break;
                        }

                        jassertfalse;
                        return 0.0f;
                    }();

                    const auto processed = lowpass.processSample (i32 (channel), output);

                    samplesOut[i] = processed;
                    lastDelayEffectOutput[channel] = processed * delayFeedbackVolume[channel].getNextValue();
                }
            }

            mixer.mixWetSamples (outputBlock);
        }

        static constexpr auto effectDelaySamples = 192000;
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::None>        noInterpolation { effectDelaySamples };
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::Linear>      linear          { effectDelaySamples };
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::Lagrange3rd> lagrange        { effectDelaySamples };
        dsp::DelayLine<f32, dsp::DelayLineInterpolationTypes::Thiran>      thiran          { effectDelaySamples };

        // Double precision to avoid some approximation issues
        dsp::FirstOrderTPTFilter<f64> smoothFilter;

        std::array<f64, 2> delayEffectValue;

        std::array<LinearSmoothedValue<f32>, 2> delayFeedbackVolume;
        dsp::FirstOrderTPTFilter<f32> lowpass;
        dsp::DryWetMixer<f32> mixer;
        std::array<f32, 2> lastDelayEffectOutput;

        i32 delayEffectType = 1;
    };

    ParameterReferences parameters;
    AudioProcessorValueTreeState apvts;

    using Chain = dsp::ProcessorChain<dsp::NoiseGate<f32>,
                                      dsp::Gain<f32>,
                                      DirectDelayProcessor,
                                      MultiBandProcessor,
                                      dsp::Compressor<f32>,
                                      dsp::Phaser<f32>,
                                      dsp::Chorus<f32>,
                                      DistortionProcessor,
                                      dsp::LadderFilter<f32>,
                                      DelayEffectProcessor,
                                      ConvolutionProcessor,
                                      dsp::Limiter<f32>,
                                      dsp::Gain<f32>,
                                      dsp::Panner<f32>>;
    Chain chain;

    // We use this enum to index into the chain above
    enum ProcessorIndices
    {
        noiseGateIndex,
        inputGainIndex,
        directDelayIndex,
        multiBandIndex,
        compressorIndex,
        phaserIndex,
        chorusIndex,
        distortionIndex,
        ladderIndex,
        delayEffectIndex,
        convolutionIndex,
        limiterIndex,
        outputGainIndex,
        pannerIndex
    };

    //==============================================================================
    std::atomic<b8> requiresUpdate { true };
    std::atomic<i32> irSize { 0 };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemo)
};

//==============================================================================
class DspModulePluginDemoEditor final : public AudioProcessorEditor
{
public:
    explicit DspModulePluginDemoEditor (DspModulePluginDemo& p)
        : AudioProcessorEditor (&p),
          proc (p)
    {
        comboEffect.addSectionHeading ("Main");
        comboEffect.addItem ("Distortion", TabDistortion);
        comboEffect.addItem ("Convolution", TabConvolution);
        comboEffect.addItem ("Multi-band", TabMultiBand);

        comboEffect.addSectionHeading ("Dynamics");
        comboEffect.addItem ("Compressor", TabCompressor);
        comboEffect.addItem ("Noise gate", TabNoiseGate);
        comboEffect.addItem ("Limiter", TabLimiter);

        comboEffect.addSectionHeading ("Delay");
        comboEffect.addItem ("Delay line direct", TabDelayLineDirect);
        comboEffect.addItem ("Delay line effect", TabDelayLineEffect);

        comboEffect.addSectionHeading ("Others");
        comboEffect.addItem ("Phaser", TabPhaser);
        comboEffect.addItem ("Chorus", TabChorus);
        comboEffect.addItem ("Ladder filter", TabLadder);

        comboEffect.setSelectedId (proc.indexTab + 1, dontSendNotification);
        comboEffect.onChange = [this]
        {
            proc.indexTab = comboEffect.getSelectedId() - 1;
            updateVisibility();
        };

        addAllAndMakeVisible (*this,
                              comboEffect,
                              labelEffect,
                              basicControls,
                              distortionControls,
                              convolutionControls,
                              multibandControls,
                              compressorControls,
                              noiseGateControls,
                              limiterControls,
                              directDelayControls,
                              delayEffectControls,
                              phaserControls,
                              chorusControls,
                              ladderControls);
        labelEffect.setJustificationType (Justification::centredRight);
        labelEffect.attachToComponent (&comboEffect, true);

        updateVisibility();

        setSize (800, 430);
        setResizable (false, false);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        auto rect = getLocalBounds();

        auto rectTop    = rect.removeFromTop (topSize);
        auto rectBottom = rect.removeFromBottom (bottomSize);

        auto rectEffects = rect.removeFromBottom (tabSize);
        auto rectChoice  = rect.removeFromBottom (midSize);

        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
        g.fillRect (rect);

        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).brighter (0.2f));
        g.fillRect (rectEffects);

        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).darker (0.2f));
        g.fillRect (rectTop);
        g.fillRect (rectBottom);
        g.fillRect (rectChoice);

        g.setColor (Colors::white);
        g.setFont (Font (FontOptions (20.0f)).italicised().withExtraKerningFactor (0.1f));
        g.drawFittedText ("DSP MODULE DEMO", rectTop.reduced (10, 0), Justification::centredLeft, 1);

        g.setFont (FontOptions (14.0f));
        Txt strText = "IR length (reverb): " + Txt (proc.getCurrentIRSize()) + " samples";
        g.drawFittedText (strText, rectBottom.reduced (10, 0), Justification::centredRight, 1);
    }

    z0 resized() override
    {
        auto rect = getLocalBounds();
        rect.removeFromTop (topSize);
        rect.removeFromBottom (bottomSize);

        auto rectEffects = rect.removeFromBottom (tabSize);
        auto rectChoice  = rect.removeFromBottom (midSize);

        comboEffect.setBounds (rectChoice.withSizeKeepingCentre (200, 24));

        rect.reduce (80, 0);
        rectEffects.reduce (20, 0);

        basicControls.setBounds (rect);

        forEach ([&] (Component& comp) { comp.setBounds (rectEffects); },
                 distortionControls,
                 convolutionControls,
                 multibandControls,
                 compressorControls,
                 noiseGateControls,
                 limiterControls,
                 directDelayControls,
                 delayEffectControls,
                 phaserControls,
                 chorusControls,
                 ladderControls);
    }

    /*  Called by VST3 and AAX hosts to determine which parameter is under the mouse. */
    i32 getControlParameterIndex (Component& comp) override
    {
        if (auto* parent = findParentComponentWithParamMenu (&comp))
            return parent->getParameterIndex();

        return -1;
    }

private:
    class ComponentWithParamMenu : public Component
    {
    public:
        ComponentWithParamMenu (AudioProcessorEditor& editorIn, RangedAudioParameter& paramIn)
            : editor (editorIn), param (paramIn) {}

        z0 mouseUp (const MouseEvent& e) override
        {
            if (e.mods.isRightButtonDown())
                if (auto* c = editor.getHostContext())
                    if (auto menuInfo = c->getContextMenuForParameter (&param))
                        menuInfo->getEquivalentPopupMenu().showMenuAsync (PopupMenu::Options{}.withTargetComponent (this)
                                                                                              .withMousePosition());
        }

        i32 getParameterIndex() const
        {
            return param.getParameterIndex();
        }

    private:
        AudioProcessorEditor& editor;
        RangedAudioParameter& param;
    };

    static ComponentWithParamMenu* findParentComponentWithParamMenu (Component* c)
    {
        if (c == nullptr)
            return nullptr;

        if (auto* derived = dynamic_cast<ComponentWithParamMenu*> (c))
            return derived;

        return findParentComponentWithParamMenu (c->getParentComponent());
    }

    class AttachedSlider final : public ComponentWithParamMenu
    {
    public:
        AttachedSlider (AudioProcessorEditor& editorIn, RangedAudioParameter& paramIn)
            : ComponentWithParamMenu (editorIn, paramIn),
              label ("", paramIn.name),
              attachment (paramIn, slider)
        {
            slider.addMouseListener (this, true);

            addAllAndMakeVisible (*this, slider, label);

            slider.setTextValueSuffix (" " + paramIn.label);

            label.attachToComponent (&slider, false);
            label.setJustificationType (Justification::centred);
        }

        z0 resized() override { slider.setBounds (getLocalBounds().reduced (0, 40)); }

    private:
        Slider slider { Slider::RotaryVerticalDrag, Slider::TextBoxBelow };
        Label label;
        SliderParameterAttachment attachment;
    };

    class AttachedToggle final : public ComponentWithParamMenu
    {
    public:
        AttachedToggle (AudioProcessorEditor& editorIn, RangedAudioParameter& paramIn)
            : ComponentWithParamMenu (editorIn, paramIn),
              toggle (paramIn.name),
              attachment (paramIn, toggle)
        {
            toggle.addMouseListener (this, true);
            addAndMakeVisible (toggle);
        }

        z0 resized() override { toggle.setBounds (getLocalBounds()); }

    private:
        ToggleButton toggle;
        ButtonParameterAttachment attachment;
    };

    class AttachedCombo final : public ComponentWithParamMenu
    {
    public:
        AttachedCombo (AudioProcessorEditor& editorIn, RangedAudioParameter& paramIn)
            : ComponentWithParamMenu (editorIn, paramIn),
              combo (paramIn),
              label ("", paramIn.name),
              attachment (paramIn, combo)
        {
            combo.addMouseListener (this, true);

            addAllAndMakeVisible (*this, combo, label);

            label.attachToComponent (&combo, false);
            label.setJustificationType (Justification::centred);
        }

        z0 resized() override
        {
            combo.setBounds (getLocalBounds().withSizeKeepingCentre (jmin (getWidth(), 150), 24));
        }

    private:
        struct ComboWithItems final : public ComboBox
        {
            explicit ComboWithItems (RangedAudioParameter& param)
            {
                // Adding the list here in the constructor means that the combo
                // is already populated when we construct the attachment below
                addItemList (dynamic_cast<AudioParameterChoice&> (param).choices, 1);
            }
        };

        ComboWithItems combo;
        Label label;
        ComboBoxParameterAttachment attachment;
    };

    //==============================================================================
    z0 updateVisibility()
    {
        const auto indexEffect = comboEffect.getSelectedId();

        const auto op = [&] (const std::tuple<Component&, i32>& tup)
        {
            Component& comp    = std::get<0> (tup);
            i32k tabIndex = std::get<1> (tup);
            comp.setVisible (tabIndex == indexEffect);
        };

        forEach (op,
                 std::forward_as_tuple (distortionControls,  TabDistortion),
                 std::forward_as_tuple (convolutionControls, TabConvolution),
                 std::forward_as_tuple (multibandControls,   TabMultiBand),
                 std::forward_as_tuple (compressorControls,  TabCompressor),
                 std::forward_as_tuple (noiseGateControls,   TabNoiseGate),
                 std::forward_as_tuple (limiterControls,     TabLimiter),
                 std::forward_as_tuple (directDelayControls, TabDelayLineDirect),
                 std::forward_as_tuple (delayEffectControls, TabDelayLineEffect),
                 std::forward_as_tuple (phaserControls,      TabPhaser),
                 std::forward_as_tuple (chorusControls,      TabChorus),
                 std::forward_as_tuple (ladderControls,      TabLadder));
    }

    enum EffectsTabs
    {
        TabDistortion = 1,
        TabConvolution,
        TabMultiBand,
        TabCompressor,
        TabNoiseGate,
        TabLimiter,
        TabDelayLineDirect,
        TabDelayLineEffect,
        TabPhaser,
        TabChorus,
        TabLadder
    };

    //==============================================================================
    ComboBox comboEffect;
    Label labelEffect { {}, "Audio effect: " };

    struct GetTrackInfo
    {
        // Combo boxes need a lot of room
        Grid::TrackInfo operator() (AttachedCombo&)             const { return 120_px; }

        // Toggles are a bit smaller
        Grid::TrackInfo operator() (AttachedToggle&)            const { return 80_px; }

        // Sliders take up as much room as they can
        Grid::TrackInfo operator() (AttachedSlider&)            const { return 1_fr; }
    };

    template <typename... Components>
    static z0 performLayout (const Rectangle<i32>& bounds, Components&... components)
    {
        Grid grid;
        using Track = Grid::TrackInfo;

        grid.autoColumns     = Track (1_fr);
        grid.autoRows        = Track (1_fr);
        grid.columnGap       = Grid::Px (10);
        grid.rowGap          = Grid::Px (0);
        grid.autoFlow        = Grid::AutoFlow::column;

        grid.templateColumns = { GetTrackInfo{} (components)... };
        grid.items           = { GridItem (components)... };

        grid.performLayout (bounds);
    }

    struct BasicControls final : public Component
    {
        explicit BasicControls (AudioProcessorEditor& editor,
                                const DspModulePluginDemo::ParameterReferences::MainGroup& state)
            : pan       (editor, state.pan),
              input     (editor, state.inputGain),
              output    (editor, state.outputGain)
        {
            addAllAndMakeVisible (*this, pan, input, output);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), input, output, pan);
        }

        AttachedSlider pan, input, output;
    };

    struct DistortionControls final : public Component
    {
        explicit DistortionControls (AudioProcessorEditor& editor,
                                     const DspModulePluginDemo::ParameterReferences::DistortionGroup& state)
            : toggle       (editor, state.enabled),
              lowpass      (editor, state.lowpass),
              highpass     (editor, state.highpass),
              mix          (editor, state.mix),
              gain         (editor, state.inGain),
              compv        (editor, state.compGain),
              type         (editor, state.type),
              oversampling (editor, state.oversampler)
        {
            addAllAndMakeVisible (*this, toggle, type, lowpass, highpass, mix, gain, compv, oversampling);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, type, gain, highpass, lowpass, compv, mix, oversampling);
        }

        AttachedToggle toggle;
        AttachedSlider lowpass, highpass, mix, gain, compv;
        AttachedCombo type, oversampling;
    };

    struct ConvolutionControls final : public Component
    {
        explicit ConvolutionControls (AudioProcessorEditor& editor,
                                      const DspModulePluginDemo::ParameterReferences::ConvolutionGroup& state)
            : cab    (editor, state.cabEnabled),
              reverb (editor, state.reverbEnabled),
              mix    (editor, state.reverbMix)
        {
            addAllAndMakeVisible (*this, cab, reverb, mix);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), cab, reverb, mix);
        }

        AttachedToggle cab, reverb;
        AttachedSlider mix;
    };

    struct MultiBandControls final : public Component
    {
        explicit MultiBandControls (AudioProcessorEditor& editor,
                                    const DspModulePluginDemo::ParameterReferences::MultiBandGroup& state)
            : toggle (editor, state.enabled),
              low    (editor, state.lowVolume),
              high   (editor, state.highVolume),
              lRFreq (editor, state.freq)
        {
            addAllAndMakeVisible (*this, toggle, low, high, lRFreq);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, lRFreq, low, high);
        }

        AttachedToggle toggle;
        AttachedSlider low, high, lRFreq;
    };

    struct CompressorControls final : public Component
    {
        explicit CompressorControls (AudioProcessorEditor& editor,
                                     const DspModulePluginDemo::ParameterReferences::CompressorGroup& state)
            : toggle    (editor, state.enabled),
              threshold (editor, state.threshold),
              ratio     (editor, state.ratio),
              attack    (editor, state.attack),
              release   (editor, state.release)
        {
            addAllAndMakeVisible (*this, toggle, threshold, ratio, attack, release);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, threshold, ratio, attack, release);
        }

        AttachedToggle toggle;
        AttachedSlider threshold, ratio, attack, release;
    };

    struct NoiseGateControls final : public Component
    {
        explicit NoiseGateControls (AudioProcessorEditor& editor,
                                    const DspModulePluginDemo::ParameterReferences::NoiseGateGroup& state)
            : toggle    (editor, state.enabled),
              threshold (editor, state.threshold),
              ratio     (editor, state.ratio),
              attack    (editor, state.attack),
              release   (editor, state.release)
        {
            addAllAndMakeVisible (*this, toggle, threshold, ratio, attack, release);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, threshold, ratio, attack, release);
        }

        AttachedToggle toggle;
        AttachedSlider threshold, ratio, attack, release;
    };

    struct LimiterControls final : public Component
    {
        explicit LimiterControls (AudioProcessorEditor& editor,
                                  const DspModulePluginDemo::ParameterReferences::LimiterGroup& state)
            : toggle    (editor, state.enabled),
              threshold (editor, state.threshold),
              release   (editor, state.release)
        {
            addAllAndMakeVisible (*this, toggle, threshold, release);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, threshold, release);
        }

        AttachedToggle toggle;
        AttachedSlider threshold, release;
    };

    struct DirectDelayControls final : public Component
    {
        explicit DirectDelayControls (AudioProcessorEditor& editor,
                                      const DspModulePluginDemo::ParameterReferences::DirectDelayGroup& state)
            : toggle (editor, state.enabled),
              type   (editor, state.type),
              delay  (editor, state.value),
              smooth (editor, state.smoothing),
              mix    (editor, state.mix)
        {
            addAllAndMakeVisible (*this, toggle, type, delay, smooth, mix);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, type, delay, smooth, mix);
        }

        AttachedToggle toggle;
        AttachedCombo type;
        AttachedSlider delay, smooth, mix;
    };

    struct DelayEffectControls final : public Component
    {
        explicit DelayEffectControls (AudioProcessorEditor& editor,
                                      const DspModulePluginDemo::ParameterReferences::DelayEffectGroup& state)
            : toggle   (editor, state.enabled),
              type     (editor, state.type),
              value    (editor, state.value),
              smooth   (editor, state.smoothing),
              lowpass  (editor, state.lowpass),
              feedback (editor, state.feedback),
              mix      (editor, state.mix)
        {
            addAllAndMakeVisible (*this, toggle, type, value, smooth, lowpass, feedback, mix);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, type, value, smooth, lowpass, feedback, mix);
        }

        AttachedToggle toggle;
        AttachedCombo type;
        AttachedSlider value, smooth, lowpass, feedback, mix;
    };

    struct PhaserControls final : public Component
    {
        explicit PhaserControls (AudioProcessorEditor& editor,
                                 const DspModulePluginDemo::ParameterReferences::PhaserGroup& state)
            : toggle   (editor, state.enabled),
              rate     (editor, state.rate),
              depth    (editor, state.depth),
              centre   (editor, state.centreFrequency),
              feedback (editor, state.feedback),
              mix      (editor, state.mix)
        {
            addAllAndMakeVisible (*this, toggle, rate, depth, centre, feedback, mix);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, rate, depth, centre, feedback, mix);
        }

        AttachedToggle toggle;
        AttachedSlider rate, depth, centre, feedback, mix;
    };

    struct ChorusControls final : public Component
    {
        explicit ChorusControls (AudioProcessorEditor& editor,
                                 const DspModulePluginDemo::ParameterReferences::ChorusGroup& state)
            : toggle   (editor, state.enabled),
              rate     (editor, state.rate),
              depth    (editor, state.depth),
              centre   (editor, state.centreDelay),
              feedback (editor, state.feedback),
              mix      (editor, state.mix)
        {
            addAllAndMakeVisible (*this, toggle, rate, depth, centre, feedback, mix);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, rate, depth, centre, feedback, mix);
        }

        AttachedToggle toggle;
        AttachedSlider rate, depth, centre, feedback, mix;
    };

    struct LadderControls final : public Component
    {
        explicit LadderControls (AudioProcessorEditor& editor,
                                 const DspModulePluginDemo::ParameterReferences::LadderGroup& state)
            : toggle    (editor, state.enabled),
              mode      (editor, state.mode),
              freq      (editor, state.cutoff),
              resonance (editor, state.resonance),
              drive     (editor, state.drive)
        {
            addAllAndMakeVisible (*this, toggle, mode, freq, resonance, drive);
        }

        z0 resized() override
        {
            performLayout (getLocalBounds(), toggle, mode, freq, resonance, drive);
        }

        AttachedToggle toggle;
        AttachedCombo mode;
        AttachedSlider freq, resonance, drive;
    };

    //==============================================================================
    static constexpr auto topSize    = 40,
                          bottomSize = 40,
                          midSize    = 40,
                          tabSize    = 155;

    //==============================================================================
    DspModulePluginDemo& proc;

    BasicControls       basicControls       { *this, proc.getParameterValues().main };
    DistortionControls  distortionControls  { *this, proc.getParameterValues().distortion };
    ConvolutionControls convolutionControls { *this, proc.getParameterValues().convolution };
    MultiBandControls   multibandControls   { *this, proc.getParameterValues().multiBand };
    CompressorControls  compressorControls  { *this, proc.getParameterValues().compressor };
    NoiseGateControls   noiseGateControls   { *this, proc.getParameterValues().noiseGate };
    LimiterControls     limiterControls     { *this, proc.getParameterValues().limiter };
    DirectDelayControls directDelayControls { *this, proc.getParameterValues().directDelay };
    DelayEffectControls delayEffectControls { *this, proc.getParameterValues().delayEffect };
    PhaserControls      phaserControls      { *this, proc.getParameterValues().phaser };
    ChorusControls      chorusControls      { *this, proc.getParameterValues().chorus };
    LadderControls      ladderControls      { *this, proc.getParameterValues().ladder };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemoEditor)
};

struct DspModulePluginDemoAudioProcessor final : public DspModulePluginDemo
{
    AudioProcessorEditor* createEditor() override
    {
        return new DspModulePluginDemoEditor (*this);
    }

    b8 hasEditor() const override { return true; }
};
