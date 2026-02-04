%%include_corresponding_header%%

//==============================================================================
%%content_component_class%%::%%content_component_class%%()
{
    setSize (600, 400);
}

%%content_component_class%%::~%%content_component_class%%()
{
}

//==============================================================================
z0 %%content_component_class%%::paint (drx::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));

    g.setFont (drx::FontOptions (16.0f));
    g.setColor (drx::Colors::white);
    g.drawText ("Hello World!", getLocalBounds(), drx::Justification::centred, true);
}

z0 %%content_component_class%%::resized()
{
    // This is called when the %%content_component_class%% is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
