#pragma once

%%include_juce%%

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class %%content_component_class%%  : public drx::AudioAppComponent
{
public:
    //==============================================================================
    %%content_component_class%%();
    ~%%content_component_class%%() override;

    //==============================================================================
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override;
    z0 getNextAudioBlock (const drx::AudioSourceChannelInfo& bufferToFill) override;
    z0 releaseResources() override;

    //==============================================================================
    z0 paint (drx::Graphics& g) override;
    z0 resized() override;

private:
    //==============================================================================
    // Your private member variables go here...


    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%content_component_class%%)
};
