/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#pragma once

#include <DrxHeader.h>
#include "DemoContentComponent.h"

//==============================================================================
class MainComponent final : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    z0 paint (Graphics&) override;
    z0 resized() override;

    //==============================================================================
    SidePanel& getSidePanel()              { return demosPanel; }

    //==============================================================================
    z0 homeButtonClicked();
    z0 settingsButtonClicked();

    //==============================================================================
    StringArray getRenderingEngines()      { return renderingEngines; }
    i32 getCurrentRenderingEngine()        { return currentRenderingEngineIdx; }
    z0 setRenderingEngine (i32 index);

private:
    z0 parentHierarchyChanged() override;
    z0 updateRenderingEngine (i32 index);

    //==============================================================================
    std::unique_ptr<DemoContentComponent> contentComponent;
    SidePanel demosPanel  { "Demos", 250, true };

    OpenGLContext openGLContext;
    ComponentPeer* peer = nullptr;
    StringArray renderingEngines;
    i32 currentRenderingEngineIdx = -1;

    TextButton showDemosButton      { "Browse Demos" };

    b8 isShowingHeavyweightDemo = false;
    i32 sidePanelWidth = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
