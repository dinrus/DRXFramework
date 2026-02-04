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
    %%content_component_class%%()
    {
        // Make sure you set the size of the component after
        // you add any child components.
        setSize (800, 600);

        // Some platforms require permissions to open input channels so request that here
        if (drx::RuntimePermissions::isRequired (drx::RuntimePermissions::recordAudio)
            && ! drx::RuntimePermissions::isGranted (drx::RuntimePermissions::recordAudio))
        {
            drx::RuntimePermissions::request (drx::RuntimePermissions::recordAudio,
                                               [&] (b8 granted) { setAudioChannels (granted ? 2 : 0, 2); });
        }
        else
        {
            // Specify the number of input and output channels that we want to open
            setAudioChannels (2, 2);
        }
    }

    ~%%content_component_class%%() override
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    //==============================================================================
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
    }

    z0 getNextAudioBlock (const drx::AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        bufferToFill.clearActiveBufferRegion();
    }

    z0 releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //==============================================================================
    z0 paint (drx::Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));

        // You can add your drawing code here!
    }

    z0 resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    // Your private member variables go here...


    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%content_component_class%%)
};
