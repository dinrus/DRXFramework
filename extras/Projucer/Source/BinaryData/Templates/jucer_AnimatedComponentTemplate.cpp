%%include_corresponding_header%%

//==============================================================================
%%content_component_class%%::%%content_component_class%%()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
    setFramesPerSecond (60); // This sets the frequency of the update calls.
}

%%content_component_class%%::~%%content_component_class%%()
{
}

//==============================================================================
z0 %%content_component_class%%::update()
{
    // This function is called at the frequency specified by the setFramesPerSecond() call
    // in the constructor. You can use it to update counters, animate values, etc.
}

//==============================================================================
z0 %%content_component_class%%::paint (drx::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));

    // You can add your drawing code here!
}

z0 %%content_component_class%%::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
