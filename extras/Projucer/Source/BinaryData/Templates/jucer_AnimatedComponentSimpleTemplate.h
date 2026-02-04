#pragma once

%%include_juce%%

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class %%content_component_class%%  : public drx::AnimatedAppComponent
{
public:
    //==============================================================================
    %%content_component_class%%()
    {
        // Make sure you set the size of the component after
        // you add any child components.
        setSize (800, 600);
        setFramesPerSecond (60); // This sets the frequency of the update calls.
    }

    ~%%content_component_class%%() override
    {
    }

    //==============================================================================
    z0 update() override
    {
        // This function is called at the frequency specified by the setFramesPerSecond() call
        // in the constructor. You can use it to update counters, animate values, etc.
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
