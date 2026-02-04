/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Application/jucer_Headers.h"
#include "../../Utility/UI/jucer_IconButton.h"

class Project;
class ProjectContentComponent;
class ProjectExporter;

//==============================================================================
class HeaderComponent final : public Component,
                              private ValueTree::Listener,
                              private Value::Listener,
                              private Timer
{
public:
    HeaderComponent (ProjectContentComponent* projectContentComponent);

    //==============================================================================
    z0 resized() override;
    z0 paint (Graphics&) override;

    //==============================================================================
    z0 setCurrentProject (Project*);

    z0 updateExporters();
    std::unique_ptr<ProjectExporter> getSelectedExporter() const;
    b8 canCurrentExporterLaunchProject() const;

    z0 sidebarTabsWidthChanged (i32 newWidth);

private:
    //==============================================================================
    z0 valueChanged (Value&) override;
    z0 timerCallback() override;

    //==============================================================================
    z0 valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override        { updateIfNeeded (parentTree); }
    z0 valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, i32) override { updateIfNeeded (parentTree); }
    z0 valueTreeChildOrderChanged (ValueTree& parentTree, i32, i32) override   { updateIfNeeded (parentTree); }

    z0 updateIfNeeded (ValueTree tree)
    {
        if (tree == exportersTree)
            updateExporters();
    }

    //==============================================================================
    z0 initialiseButtons();

    z0 updateName();
    z0 updateExporterButton();

    //==============================================================================
    i32 tabsWidth = 200;

    ProjectContentComponent* projectContentComponent = nullptr;
    Project* project = nullptr;
    ValueTree exportersTree;

    Value projectNameValue;

    ComboBox exporterBox;
    Label configLabel  { "Config Label", "Selected exporter" }, projectNameLabel;

    ImageComponent juceIcon;

    IconButton projectSettingsButton { "Project Settings", getIcons().settings },
               saveAndOpenInIDEButton { "Save and Open in IDE", Image() };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};
