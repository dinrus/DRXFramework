/*
  ==============================================================================

    This file contains the basic framework code for a DRX plugin editor.

  ==============================================================================
*/

#pragma once

%%editor_headers%%

//==============================================================================
/**
*/
class %%editor_class_name%%  : public drx::AudioProcessorEditor
                            #if DrxPlugin_Enable_ARA
                             , public drx::AudioProcessorEditorARAExtension
                            #endif
{
public:
    %%editor_class_name%% (%%filter_class_name%%&);
    ~%%editor_class_name%%() override;

    //==============================================================================
    z0 paint (drx::Graphics&) override;
    z0 resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    %%filter_class_name%%& audioProcessor;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%editor_class_name%%)
};
