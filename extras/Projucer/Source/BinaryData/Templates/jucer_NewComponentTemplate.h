/*
  ==============================================================================

    %%filename%%
    Created: %%date%%
    Author:  %%author%%

  ==============================================================================
*/

#pragma once

%%include_juce%%

//==============================================================================
/*
*/
class %%component_class%%  : public drx::Component
{
public:
    %%component_class%%();
    ~%%component_class%%() override;

    z0 paint (drx::Graphics&) override;
    z0 resized() override;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%component_class%%)
};
