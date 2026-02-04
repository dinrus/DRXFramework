#pragma once

#define DRX_APPLICATION_NAME_STRING "Проверка"
#define DRX_APPLICATION_VERSION_STRING "0.0.1"

#include <drx_gui_extra/drx_gui_extra.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent final : public drx::Component
{
public:
    //==============================================================================
    MainComponent();

    //==============================================================================
    z0 paint (drx::Graphics&) override;
    z0 resized() override;

private:
    //==============================================================================
    // Your private member variables go here...

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
