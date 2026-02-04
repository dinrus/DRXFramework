#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
}

//==============================================================================
z0 MainComponent::paint (drx::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));

    g.setFont (drx::FontOptions (16.0f));
    g.setColor (drx::Colors::white);
    g.drawText ("Hello World! Миру - мир!", getLocalBounds(), drx::Justification::centred, true);
}

z0 MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
