#pragma once

#include <drx_audio_processors/drx_audio_processors.h>

//==============================================================================
class AudioPluginAudioProcessor final : public drx::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate, i32 samplesPerBlock) override;
    z0 releaseResources() override;

    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override;

    z0 processBlock (drx::AudioBuffer<f32>&, drx::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    drx::AudioProcessorEditor* createEditor() override;
    b8 hasEditor() const override;

    //==============================================================================
    const drx::Txt getName() const override;

    b8 acceptsMidi() const override;
    b8 producesMidi() const override;
    b8 isMidiEffect() const override;
    f64 getTailLengthSeconds() const override;

    //==============================================================================
    i32 getNumPrograms() override;
    i32 getCurrentProgram() override;
    z0 setCurrentProgram (i32 index) override;
    const drx::Txt getProgramName (i32 index) override;
    z0 changeProgramName (i32 index, const drx::Txt& newName) override;

    //==============================================================================
    z0 getStateInformation (drx::MemoryBlock& destData) override;
    z0 setStateInformation (ukk data, i32 sizeInBytes) override;

private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
