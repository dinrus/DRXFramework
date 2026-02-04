%%include_corresponding_header%%

//==============================================================================
%%content_component_class%%::%%content_component_class%%()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
}

%%content_component_class%%::~%%content_component_class%%()
{
    // This shuts down the GL system and stops the rendering calls.
    shutdownOpenGL();
}

//==============================================================================
z0 %%content_component_class%%::initialise()
{
    // Initialise GL objects for rendering here.
}

z0 %%content_component_class%%::shutdown()
{
    // Free any GL objects created for rendering here.
}

z0 %%content_component_class%%::render()
{
    // This clears the context with a black background.
    drx::OpenGLHelpers::clear (drx::Colors::black);

    // Add your rendering code here...
}

//==============================================================================
z0 %%content_component_class%%::paint (drx::Graphics& g)
{
    // You can add your component specific drawing code here!
    // This will draw over the top of the openGL background.
}

z0 %%content_component_class%%::resized()
{
    // This is called when the %%content_component_class%% is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
