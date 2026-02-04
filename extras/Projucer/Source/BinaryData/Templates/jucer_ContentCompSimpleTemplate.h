#pragma once

%%include_juce%%

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class %%content_component_class%%  : public drx::Component
{
public:
    //==============================================================================
    %%content_component_class%%()
    {
        setSize (600, 400);
    }

    ~%%content_component_class%%() override
    {
    }

    //==============================================================================
    z0 paint (drx::Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));

        g.setFont (drx::FontOptions (16.0f));
        g.setColor (drx::Colors::white);
        g.drawText ("Hello World!", getLocalBounds(), drx::Justification::centred, true);
    }

    z0 resized() override
    {
        // This is called when the %%content_component_class%% is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    // Your private member variables go here...


    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%content_component_class%%)
};
