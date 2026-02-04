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

#include "jucer_HeaderComponent.h"

#include "../../Application/jucer_Application.h"

#include "../../ProjectSaving/jucer_ProjectExporter.h"
#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
HeaderComponent::HeaderComponent (ProjectContentComponent* pcc)
    : projectContentComponent (pcc)
{
    setTitle ("Header");
    setFocusContainerType (FocusContainerType::focusContainer);

    addAndMakeVisible (configLabel);
    addAndMakeVisible (exporterBox);

    exporterBox.onChange = [this] { updateExporterButton(); };

    juceIcon.setImage (ImageCache::getFromMemory (BinaryData::drx_icon_png, BinaryData::drx_icon_pngSize), RectanglePlacement::centred);
    addAndMakeVisible (juceIcon);

    projectNameLabel.setText ({}, dontSendNotification);
    addAndMakeVisible (projectNameLabel);

    initialiseButtons();
}

//==============================================================================
z0 HeaderComponent::resized()
{
    auto bounds = getLocalBounds();
    configLabel.setFont (FontOptions { (f32) bounds.getHeight() / 3.0f });

    {
        auto headerBounds = bounds.removeFromLeft (tabsWidth);

        i32k buttonSize = 25;
        auto buttonBounds = headerBounds.removeFromRight (buttonSize);

        projectSettingsButton.setBounds (buttonBounds.removeFromBottom (buttonSize).reduced (2));

        juceIcon.setBounds (headerBounds.removeFromLeft (headerBounds.getHeight()).reduced (2));

        headerBounds.removeFromRight (5);
        projectNameLabel.setBounds (headerBounds);
    }

    {
        auto exporterWidth = jmin (400, bounds.getWidth() / 2);
        Rectangle<i32> exporterBounds (0, 0, exporterWidth, bounds.getHeight());

        exporterBounds.setCentre (bounds.getCentre());

        saveAndOpenInIDEButton.setBounds (exporterBounds.removeFromRight (exporterBounds.getHeight()).reduced (2));

        exporterBounds.removeFromRight (5);
        exporterBox.setBounds (exporterBounds.removeFromBottom (roundToInt ((f32) exporterBounds.getHeight() / 1.8f)));
        configLabel.setBounds (exporterBounds);
    }
}

z0 HeaderComponent::paint (Graphics& g)
{
    g.fillAll (findColor (backgroundColorId));
}

//==============================================================================
z0 HeaderComponent::setCurrentProject (Project* newProject)
{
    stopTimer();
    repaint();

    projectNameLabel.setText ({}, dontSendNotification);

    project = newProject;

    if (project != nullptr)
    {
        exportersTree = project->getExporters();
        exportersTree.addListener (this);
        updateExporters();

        projectNameValue.referTo (project->getProjectValue (Ids::name));
        projectNameValue.addListener (this);
        updateName();
    }
}

//==============================================================================
z0 HeaderComponent::updateExporters()
{
    auto selectedExporter = getSelectedExporter();

    exporterBox.clear();
    auto preferredExporterIndex = -1;

    i32 i = 0;
    for (Project::ExporterIterator exporter (*project); exporter.next(); ++i)
    {
        auto exporterName = exporter->getUniqueName();

        exporterBox.addItem (exporterName, i + 1);

        if (selectedExporter != nullptr && exporterName == selectedExporter->getUniqueName())
            exporterBox.setSelectedId (i + 1);

        if (exporterName.contains (ProjectExporter::getCurrentPlatformExporterTypeInfo().displayName) && preferredExporterIndex == -1)
            preferredExporterIndex = i;
    }

    if (exporterBox.getSelectedItemIndex() == -1)
    {
        if (preferredExporterIndex == -1)
        {
            i = 0;
            for (Project::ExporterIterator exporter (*project); exporter.next(); ++i)
            {
                if (exporter->canLaunchProject())
                {
                    preferredExporterIndex = i;
                    break;
                }
            }
        }

        exporterBox.setSelectedItemIndex (preferredExporterIndex != -1 ? preferredExporterIndex : 0);
    }

    updateExporterButton();
}

std::unique_ptr<ProjectExporter> HeaderComponent::getSelectedExporter() const
{
    if (project != nullptr)
    {
        i32 i = 0;
        auto selectedIndex = exporterBox.getSelectedItemIndex();

        for (Project::ExporterIterator exporter (*project); exporter.next();)
            if (i++ == selectedIndex)
                return std::move (exporter.exporter);
    }

    return nullptr;
}

b8 HeaderComponent::canCurrentExporterLaunchProject() const
{
    if (project != nullptr)
    {
        if (auto selectedExporter = getSelectedExporter())
        {
            for (Project::ExporterIterator exporter (*project); exporter.next();)
                if (exporter->canLaunchProject() && exporter->getUniqueName() == selectedExporter->getUniqueName())
                    return true;
        }
    }

    return false;
}

//==============================================================================
z0 HeaderComponent::sidebarTabsWidthChanged (i32 newWidth)
{
    tabsWidth = newWidth;
    resized();
}

z0 HeaderComponent::valueChanged (Value&)
{
    updateName();
}

z0 HeaderComponent::timerCallback()
{
    repaint();
}

//==============================================================================
z0 HeaderComponent::initialiseButtons()
{
    addAndMakeVisible (projectSettingsButton);
    projectSettingsButton.onClick = [this] { projectContentComponent->showProjectSettings(); };

    addAndMakeVisible (saveAndOpenInIDEButton);
    saveAndOpenInIDEButton.setBackgroundColor (Colors::white);
    saveAndOpenInIDEButton.setIconInset (7);
    saveAndOpenInIDEButton.onClick = [this]
    {
        if (project == nullptr)
            return;

        if (! project->isSaveAndExportDisabled())
        {
            projectContentComponent->openInSelectedIDE (true);
            return;
        }

        auto setWarningVisible = [this] (const Identifier& identifier)
        {
            auto child = project->getProjectMessages().getChildWithName (ProjectMessages::Ids::warning)
                                                      .getChildWithName (identifier);

            if (child.isValid())
                child.setProperty (ProjectMessages::Ids::isVisible, true, nullptr);
        };

        if (project->isFileModificationCheckPending())
            setWarningVisible (ProjectMessages::Ids::jucerFileModified);
    };

    updateExporterButton();
}

z0 HeaderComponent::updateName()
{
    if (project != nullptr)
        projectNameLabel.setText (project->getDocumentTitle(), dontSendNotification);
}

z0 HeaderComponent::updateExporterButton()
{
    if (auto selectedExporter = getSelectedExporter())
    {
        auto selectedName = selectedExporter->getUniqueName();

        for (auto info : ProjectExporter::getExporterTypeInfos())
        {
            if (selectedName.contains (info.displayName))
            {
                saveAndOpenInIDEButton.setImage (info.icon);
                saveAndOpenInIDEButton.repaint();
                saveAndOpenInIDEButton.setEnabled (canCurrentExporterLaunchProject());
            }
        }
    }
}
