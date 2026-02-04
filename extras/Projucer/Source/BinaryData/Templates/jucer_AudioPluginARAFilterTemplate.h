/*
  ==============================================================================

    This file contains the basic framework code for a DRX plugin processor.

  ==============================================================================
*/

#pragma once

%%app_headers%%

//==============================================================================
/**
*/
class %%filter_class_name%%  : public drx::AudioProcessor
                            #if DrxPlugin_Enable_ARA
                             , public drx::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    %%filter_class_name%%();
    ~%%filter_class_name%%() override;

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate, i32 samplesPerBlock) override;
    z0 releaseResources() override;

   #ifndef DrxPlugin_PreferredChannelConfigurations
    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    z0 processBlock (drx::AudioBuffer<f32>&, drx::MidiBuffer&) override;

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
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%filter_class_name%%)
};
