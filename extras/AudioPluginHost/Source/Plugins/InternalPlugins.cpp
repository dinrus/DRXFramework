/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include <DrxHeader.h>

#include <drx_audio_plugin_client/drx_audio_plugin_client.h>

#include "InternalPlugins.h"
#include "PluginGraph.h"

#define PIP_DEMO_UTILITIES_INCLUDED 1

// An alternative version of createAssetInputStream from the demo utilities header
// that fetches resources from embedded binary data instead of files
static std::unique_ptr<InputStream> createAssetInputStream (tukk resourcePath)
{
    for (i32 i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        if (Txt (BinaryData::originalFilenames[i]) == Txt (resourcePath))
        {
            i32 dataSizeInBytes;
            auto* resource = BinaryData::getNamedResource (BinaryData::namedResourceList[i], dataSizeInBytes);
            return std::make_unique<MemoryInputStream> (resource, dataSizeInBytes, false);
        }
    }

    return {};
}

#include "../../../../examples/Plugins/AUv3SynthPluginDemo.h"
#include "../../../../examples/Plugins/ArpeggiatorPluginDemo.h"
#include "../../../../examples/Plugins/AudioPluginDemo.h"
#include "../../../../examples/Plugins/DSPModulePluginDemo.h"
#include "../../../../examples/Plugins/GainPluginDemo.h"
#include "../../../../examples/Plugins/MidiLoggerPluginDemo.h"
#include "../../../../examples/Plugins/MultiOutSynthPluginDemo.h"
#include "../../../../examples/Plugins/NoiseGatePluginDemo.h"
#include "../../../../examples/Plugins/SamplerPluginDemo.h"
#include "../../../../examples/Plugins/SurroundPluginDemo.h"

//==============================================================================
class InternalPlugin final : public AudioPluginInstance
{
public:
    explicit InternalPlugin (std::unique_ptr<AudioProcessor> innerIn)
        : inner (std::move (innerIn))
    {
        jassert (inner != nullptr);

        for (auto isInput : { true, false })
            matchChannels (isInput);

        setBusesLayout (inner->getBusesLayout());
    }

    //==============================================================================
    const Txt getName() const override                                         { return inner->getName(); }
    StringArray getAlternateDisplayNames() const override                         { return inner->getAlternateDisplayNames(); }
    f64 getTailLengthSeconds() const override                                  { return inner->getTailLengthSeconds(); }
    b8 acceptsMidi() const override                                             { return inner->acceptsMidi(); }
    b8 producesMidi() const override                                            { return inner->producesMidi(); }
    AudioProcessorEditor* createEditor() override                                 { return inner->createEditor(); }
    b8 hasEditor() const override                                               { return inner->hasEditor(); }
    i32 getNumPrograms() override                                                 { return inner->getNumPrograms(); }
    i32 getCurrentProgram() override                                              { return inner->getCurrentProgram(); }
    z0 setCurrentProgram (i32 i) override                                       { inner->setCurrentProgram (i); }
    const Txt getProgramName (i32 i) override                                  { return inner->getProgramName (i); }
    z0 changeProgramName (i32 i, const Txt& n) override                      { inner->changeProgramName (i, n); }
    z0 getStateInformation (drx::MemoryBlock& b) override                      { inner->getStateInformation (b); }
    z0 setStateInformation (ukk d, i32 s) override                      { inner->setStateInformation (d, s); }
    z0 getCurrentProgramStateInformation (drx::MemoryBlock& b) override        { inner->getCurrentProgramStateInformation (b); }
    z0 setCurrentProgramStateInformation (ukk d, i32 s) override        { inner->setCurrentProgramStateInformation (d, s); }
    z0 prepareToPlay (f64 sr, i32 bs) override                               { inner->setRateAndBufferSizeDetails (sr, bs); inner->prepareToPlay (sr, bs); }
    z0 releaseResources() override                                              { inner->releaseResources(); }
    z0 memoryWarningReceived() override                                         { inner->memoryWarningReceived(); }
    z0 processBlock (AudioBuffer<f32>& a, MidiBuffer& m) override             { inner->processBlock (a, m); }
    z0 processBlock (AudioBuffer<f64>& a, MidiBuffer& m) override            { inner->processBlock (a, m); }
    z0 processBlockBypassed (AudioBuffer<f32>& a, MidiBuffer& m) override     { inner->processBlockBypassed (a, m); }
    z0 processBlockBypassed (AudioBuffer<f64>& a, MidiBuffer& m) override    { inner->processBlockBypassed (a, m); }
    b8 supportsDoublePrecisionProcessing() const override                       { return inner->supportsDoublePrecisionProcessing(); }
    b8 supportsMPE() const override                                             { return inner->supportsMPE(); }
    b8 isMidiEffect() const override                                            { return inner->isMidiEffect(); }
    z0 reset() override                                                         { inner->reset(); }
    z0 setNonRealtime (b8 b) noexcept override                                { inner->setNonRealtime (b); }
    z0 refreshParameterList() override                                          { inner->refreshParameterList(); }
    z0 numChannelsChanged() override                                            { inner->numChannelsChanged(); }
    z0 numBusesChanged() override                                               { inner->numBusesChanged(); }
    z0 processorLayoutsChanged() override                                       { inner->processorLayoutsChanged(); }
    z0 setPlayHead (AudioPlayHead* p) override                                  { inner->setPlayHead (p); }
    z0 updateTrackProperties (const TrackProperties& p) override                { inner->updateTrackProperties (p); }
    b8 isBusesLayoutSupported (const BusesLayout& layout) const override        { return inner->checkBusesLayoutSupported (layout); }
    b8 applyBusLayouts (const BusesLayout& layouts) override                    { return inner->setBusesLayout (layouts) && AudioPluginInstance::applyBusLayouts (layouts); }

    b8 canAddBus (b8) const override                                          { return true; }
    b8 canRemoveBus (b8) const override                                       { return true; }

    //==============================================================================
    z0 fillInPluginDescription (PluginDescription& description) const override
    {
        description = getPluginDescription (*inner);
    }

private:
    static PluginDescription getPluginDescription (const AudioProcessor& proc)
    {
        const auto ins                  = proc.getTotalNumInputChannels();
        const auto outs                 = proc.getTotalNumOutputChannels();
        const auto identifier           = proc.getName();
        const auto registerAsGenerator  = ins == 0;
        const auto acceptsMidi          = proc.acceptsMidi();

        PluginDescription descr;

        descr.name              = identifier;
        descr.descriptiveName   = identifier;
        descr.pluginFormatName  = InternalPluginFormat::getIdentifier();
        descr.category          = (registerAsGenerator ? (acceptsMidi ? "Synth" : "Generator") : "Effect");
        descr.manufacturerName  = "DRX";
        descr.version           = ProjectInfo::versionString;
        descr.fileOrIdentifier  = identifier;
        descr.isInstrument      = (acceptsMidi && registerAsGenerator);
        descr.numInputChannels  = ins;
        descr.numOutputChannels = outs;

        descr.uniqueId = descr.deprecatedUid = identifier.hashCode();

        return descr;
    }

    z0 matchChannels (b8 isInput)
    {
        const auto inBuses = inner->getBusCount (isInput);

        while (getBusCount (isInput) < inBuses)
            addBus (isInput);

        while (inBuses < getBusCount (isInput))
            removeBus (isInput);
    }

    std::unique_ptr<AudioProcessor> inner;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternalPlugin)
};

//==============================================================================
class SineWaveSynth final : public AudioProcessor
{
public:
    SineWaveSynth()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo()))
    {
        i32k numVoices = 8;

        // Add some voices...
        for (i32 i = numVoices; --i >= 0;)
            synth.addVoice (new SineWaveVoice());

        // ..and give the synth a sound to play
        synth.addSound (new SineWaveSound());
    }

    static Txt getIdentifier()
    {
        return "Sine Wave Synth";
    }

    //==============================================================================
    z0 prepareToPlay (f64 newSampleRate, i32) override
    {
        synth.setCurrentPlaybackSampleRate (newSampleRate);
    }

    z0 releaseResources() override {}

    //==============================================================================
    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        i32k numSamples = buffer.getNumSamples();

        buffer.clear();
        synth.renderNextBlock (buffer, midiMessages, 0, numSamples);
        buffer.applyGain (0.8f);
    }

    using AudioProcessor::processBlock;

    const Txt getName() const override                                           { return getIdentifier(); }
    f64 getTailLengthSeconds() const override                                    { return 0.0; }
    b8 acceptsMidi() const override                                               { return true; }
    b8 producesMidi() const override                                              { return true; }
    AudioProcessorEditor* createEditor() override                                   { return nullptr; }
    b8 hasEditor() const override                                                 { return false; }
    i32 getNumPrograms() override                                                   { return 1; }
    i32 getCurrentProgram() override                                                { return 0; }
    z0 setCurrentProgram (i32) override                                           {}
    const Txt getProgramName (i32) override                                      { return {}; }
    z0 changeProgramName (i32, const Txt&) override                            {}
    z0 getStateInformation (drx::MemoryBlock&) override                          {}
    z0 setStateInformation (ukk, i32) override                            {}

private:
    //==============================================================================
    struct SineWaveSound final : public SynthesiserSound
    {
        SineWaveSound() = default;

        b8 appliesToNote (i32 /*midiNoteNumber*/) override    { return true; }
        b8 appliesToChannel (i32 /*midiChannel*/) override    { return true; }
    };

    struct SineWaveVoice final : public SynthesiserVoice
    {
        SineWaveVoice() = default;

        b8 canPlaySound (SynthesiserSound* sound) override
        {
            return dynamic_cast<SineWaveSound*> (sound) != nullptr;
        }

        z0 startNote (i32 midiNoteNumber, f32 velocity,
                        SynthesiserSound* /*sound*/,
                        i32 /*currentPitchWheelPosition*/) override
        {
            currentAngle = 0.0;
            level = velocity * 0.15;
            tailOff = 0.0;

            f64 cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
            f64 cyclesPerSample = cyclesPerSecond / getSampleRate();

            angleDelta = cyclesPerSample * 2.0 * MathConstants<f64>::pi;
        }

        z0 stopNote (f32 /*velocity*/, b8 allowTailOff) override
        {
            if (allowTailOff)
            {
                // start a tail-off by setting this flag. The render callback will pick up on
                // this and do a fade out, calling clearCurrentNote() when it's finished.

                if (approximatelyEqual (tailOff, 0.0)) // we only need to begin a tail-off if it's not already doing so - the
                                                       // stopNote method could be called more than once.
                    tailOff = 1.0;
            }
            else
            {
                // we're being told to stop playing immediately, so reset everything..

                clearCurrentNote();
                angleDelta = 0.0;
            }
        }

        z0 pitchWheelMoved (i32 /*newValue*/) override
        {
            // not implemented for the purposes of this demo!
        }

        z0 controllerMoved (i32 /*controllerNumber*/, i32 /*newValue*/) override
        {
            // not implemented for the purposes of this demo!
        }

        z0 renderNextBlock (AudioBuffer<f32>& outputBuffer, i32 startSample, i32 numSamples) override
        {
            if (! approximatelyEqual (angleDelta, 0.0))
            {
                if (tailOff > 0)
                {
                    while (--numSamples >= 0)
                    {
                        const f32 currentSample = (f32) (sin (currentAngle) * level * tailOff);

                        for (i32 i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample (i, startSample, currentSample);

                        currentAngle += angleDelta;
                        ++startSample;

                        tailOff *= 0.99;

                        if (tailOff <= 0.005)
                        {
                            // tells the synth that this voice has stopped
                            clearCurrentNote();

                            angleDelta = 0.0;
                            break;
                        }
                    }
                }
                else
                {
                    while (--numSamples >= 0)
                    {
                        const f32 currentSample = (f32) (sin (currentAngle) * level);

                        for (i32 i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample (i, startSample, currentSample);

                        currentAngle += angleDelta;
                        ++startSample;
                    }
                }
            }
        }

        using SynthesiserVoice::renderNextBlock;

    private:
        f64 currentAngle = 0, angleDelta = 0, level = 0, tailOff = 0;
    };

    //==============================================================================
    Synthesiser synth;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWaveSynth)
};

//==============================================================================
class ReverbPlugin final : public AudioProcessor
{
public:
    ReverbPlugin()
        : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                           .withOutput ("Output", AudioChannelSet::stereo()))
    {}

    static Txt getIdentifier()
    {
        return "Reverb";
    }

    z0 prepareToPlay (f64 newSampleRate, i32) override
    {
        reverb.setSampleRate (newSampleRate);
    }

    z0 reset() override
    {
        reverb.reset();
    }

    z0 releaseResources() override {}

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer&) override
    {
        auto numChannels = buffer.getNumChannels();

        if (numChannels == 1)
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
        else
            reverb.processStereo (buffer.getWritePointer (0),
                                  buffer.getWritePointer (1),
                                  buffer.getNumSamples());

        for (i32 ch = 2; ch < numChannels; ++ch)
            buffer.clear (ch, 0, buffer.getNumSamples());
    }

    using AudioProcessor::processBlock;

    const Txt getName() const override                                           { return getIdentifier(); }
    f64 getTailLengthSeconds() const override                                    { return 0.0; }
    b8 acceptsMidi() const override                                               { return false; }
    b8 producesMidi() const override                                              { return false; }
    AudioProcessorEditor* createEditor() override                                   { return nullptr; }
    b8 hasEditor() const override                                                 { return false; }
    i32 getNumPrograms() override                                                   { return 1; }
    i32 getCurrentProgram() override                                                { return 0; }
    z0 setCurrentProgram (i32) override                                           {}
    const Txt getProgramName (i32) override                                      { return {}; }
    z0 changeProgramName (i32, const Txt&) override                            {}
    z0 getStateInformation (drx::MemoryBlock&) override                          {}
    z0 setStateInformation (ukk, i32) override                            {}

private:
    Reverb reverb;
};

//==============================================================================

InternalPluginFormat::InternalPluginFactory::InternalPluginFactory (const std::initializer_list<Constructor>& constructorsIn)
    : constructors (constructorsIn),
      descriptions ([&]
      {
          std::vector<PluginDescription> result;

          for (const auto& constructor : constructors)
              result.push_back (constructor()->getPluginDescription());

          return result;
      }())
{}

std::unique_ptr<AudioPluginInstance> InternalPluginFormat::InternalPluginFactory::createInstance (const Txt& name) const
{
    const auto begin = descriptions.begin();
    const auto it = std::find_if (begin,
                                  descriptions.end(),
                                  [&] (const PluginDescription& desc) { return name.equalsIgnoreCase (desc.name); });

    if (it == descriptions.end())
        return nullptr;

    const auto index = (size_t) std::distance (begin, it);
    return constructors[index]();
}

InternalPluginFormat::InternalPluginFormat()
    : factory {
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode); },
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode); },
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode); },
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode); },

        [] { return std::make_unique<InternalPlugin> (std::make_unique<SineWaveSynth>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<ReverbPlugin>()); },

        [] { return std::make_unique<InternalPlugin> (std::make_unique<AUv3SynthProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<Arpeggiator>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<DspModulePluginDemoAudioProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<GainProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<DrxDemoPluginAudioProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<MidiLoggerPluginDemoProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<MultiOutSynth>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<NoiseGate>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<SamplerAudioProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<SurroundProcessor>()); }
    }
{
}

std::unique_ptr<AudioPluginInstance> InternalPluginFormat::createInstance (const Txt& name)
{
    return factory.createInstance (name);
}

z0 InternalPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                 f64 /*initialSampleRate*/, i32 /*initialBufferSize*/,
                                                 PluginCreationCallback callback)
{
    if (auto p = createInstance (desc.name))
        callback (std::move (p), {});
    else
        callback (nullptr, NEEDS_TRANS ("Invalid internal plugin name"));
}

b8 InternalPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

const std::vector<PluginDescription>& InternalPluginFormat::getAllTypes() const
{
    return factory.getDescriptions();
}
