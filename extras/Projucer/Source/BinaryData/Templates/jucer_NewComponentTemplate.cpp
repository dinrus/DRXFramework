/*
  ==============================================================================

    %%filename%%
    Created: %%date%%
    Author:  %%author%%

  ==============================================================================
*/

%%include_juce%%
%%include_corresponding_header%%

//==============================================================================
%%component_class%%::%%component_class%%()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

%%component_class%%::~%%component_class%%()
{
}

z0 %%component_class%%::paint (drx::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));   // clear the background

    g.setColor (drx::Colors::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColor (drx::Colors::white);
    g.setFont (drx::FontOptions (14.0f));
    g.drawText ("%%component_class%%", getLocalBounds(),
                drx::Justification::centred, true);   // draw some placeholder text
}

z0 %%component_class%%::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
