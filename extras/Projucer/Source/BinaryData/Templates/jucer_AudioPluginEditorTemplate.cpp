/*
  ==============================================================================

    This file contains the basic framework code for a DRX plugin editor.

  ==============================================================================
*/

%%editor_cpp_headers%%

//==============================================================================
%%editor_class_name%%::%%editor_class_name%% (%%filter_class_name%%& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

%%editor_class_name%%::~%%editor_class_name%%()
{
}

//==============================================================================
z0 %%editor_class_name%%::paint (drx::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColor (drx::ResizableWindow::backgroundColorId));

    g.setColor (drx::Colors::white);
    g.setFont (drx::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), drx::Justification::centred, 1);
}

z0 %%editor_class_name%%::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
