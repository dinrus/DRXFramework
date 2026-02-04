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


//==============================================================================
class ModulesInformationComponent final : public Component,
                                          private ListBoxModel,
                                          private ValueTree::Listener
{
public:
    ModulesInformationComponent (Project& p)
        : project (p),
          modulesValueTree (project.getEnabledModules().getState())
    {
        auto tempHeader = std::make_unique<ListBoxHeader> (Array<Txt> { "Module", "Version", "Make Local Copy", "Paths" },
                                                           Array<f32> { 0.25f, 0.2f, 0.2f, 0.35f });
        listHeader = tempHeader.get();
        list.setHeaderComponent (std::move (tempHeader));
        list.setModel (this);
        list.setColor (ListBox::backgroundColorId, Colors::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (30);
        list.setMultipleSelectionEnabled (true);

        addAndMakeVisible (header);

        addAndMakeVisible (setCopyModeButton);
        setCopyModeButton.setTriggeredOnMouseDown (true);
        setCopyModeButton.onClick = [this] { showCopyModeMenu(); };

        addAndMakeVisible (copyPathButton);
        copyPathButton.setTriggeredOnMouseDown (true);
        copyPathButton.onClick = [this] { showSetPathsMenu(); };

        addAndMakeVisible (globalPathsButton);
        globalPathsButton.onClick = [this] { showGlobalPathsMenu(); };

        modulesValueTree.addListener (this);
        lookAndFeelChanged();
    }

    z0 paint (Graphics& g) override
    {
        g.setColor (findColor (secondaryBackgroundColorId));
        g.fillRect (getLocalBounds().reduced (12, 0));
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds().reduced (12, 0);

        header.setBounds (bounds.removeFromTop (40));

        bounds.reduce (10, 0);
        list.setBounds (bounds.removeFromTop (list.getRowPosition (getNumRows() - 1, true).getBottom() + 20));

        if (bounds.getHeight() < 35)
        {
            parentSizeChanged();
        }
        else
        {
            auto buttonRow = bounds.removeFromTop (35);
            setCopyModeButton.setBounds (buttonRow.removeFromLeft (jmin (200, bounds.getWidth() / 3)));
            buttonRow.removeFromLeft (8);
            copyPathButton.setBounds (buttonRow.removeFromLeft (jmin (200, bounds.getWidth() / 3)));
            buttonRow.removeFromLeft (8);
            globalPathsButton.setBounds (buttonRow.removeFromLeft (jmin (200, bounds.getWidth() / 3)));
        }
    }

    z0 parentSizeChanged() override
    {
        auto width = jmax (550, getParentWidth());
        auto y = list.getRowPosition (getNumRows() - 1, true).getBottom() + 200;

        y = jmax (getParentHeight(), y);

        setSize (width, y);
    }

    i32 getNumRows() override
    {
        return project.getEnabledModules().getNumModules();
    }

    z0 paintListBoxItem (i32 rowNumber, Graphics& g, i32 width, i32 height, b8 rowIsSelected) override
    {
        ignoreUnused (height);

        Rectangle<i32> bounds (0, 0, width, height);

        g.setColor (rowIsSelected ? findColor (defaultHighlightColorId) : findColor (rowNumber % 2 == 0 ? widgetBackgroundColorId
                                                                                                            : secondaryWidgetBackgroundColorId));
        g.fillRect (bounds.withTrimmedBottom (1));

        bounds.removeFromLeft (5);
        g.setColor (rowIsSelected ? findColor (defaultHighlightedTextColorId) : findColor (widgetTextColorId));

        //==============================================================================
        auto moduleID = project.getEnabledModules().getModuleID (rowNumber);

        g.drawFittedText (moduleID, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (0) * (f32) width)), Justification::centredLeft, 1);

        //==============================================================================
        auto version = project.getEnabledModules().getModuleInfo (moduleID).getVersion();
        if (version.isEmpty())
            version = "?";

        g.drawFittedText (version, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (1) * (f32) width)), Justification::centredLeft, 1);

        //==============================================================================
        g.drawFittedText (Txt (project.getEnabledModules().shouldCopyModuleFilesLocally (moduleID) ? "Yes" : "No"),
                          bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (2) * (f32) width)), Justification::centredLeft, 1);

        //==============================================================================
        Txt pathText;

        if (project.getEnabledModules().shouldUseGlobalPath (moduleID))
        {
            pathText = "Global";
        }
        else
        {
            StringArray paths;

            for (Project::ExporterIterator exporter (project); exporter.next();)
                paths.addIfNotAlreadyThere (exporter->getPathForModuleString (moduleID).trim());

            paths.removeEmptyStrings();
            paths.removeDuplicates (true);

            pathText = paths.joinIntoString (", ");
        }

        g.drawFittedText (pathText, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (3) * (f32) width)), Justification::centredLeft, 1);
    }

    z0 listBoxItemDoubleClicked (i32 row, const MouseEvent&) override
    {
        auto moduleID = project.getEnabledModules().getModuleID (row);

        if (moduleID.isNotEmpty())
            if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->showModule (moduleID);
    }

    z0 deleteKeyPressed (i32 row) override
    {
        project.getEnabledModules().removeModule (project.getEnabledModules().getModuleID (row));
    }

    z0 lookAndFeelChanged() override
    {
        setCopyModeButton.setColor (TextButton::buttonColorId, findColor (secondaryButtonBackgroundColorId));
        copyPathButton.setColor    (TextButton::buttonColorId, findColor (defaultButtonBackgroundColorId));
        globalPathsButton.setColor (TextButton::buttonColorId, findColor (defaultButtonBackgroundColorId));
    }

private:
    enum
    {
        nameCol = 1,
        versionCol,
        copyCol,
        pathCol
    };

    Project& project;
    ValueTree modulesValueTree;

    ContentViewHeader header  { "Modules", { getIcons().modules, Colors::transparentBlack } };
    ListBox list;
    ListBoxHeader* listHeader;

    TextButton setCopyModeButton  { "Set copy-mode for all modules..." };
    TextButton copyPathButton     { "Set paths for all modules..." };
    TextButton globalPathsButton  { "Enable/disable global path for modules..." };

    std::map<Txt, var> modulePathClipboard;

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32) override         { itemChanged(); }
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32) override           { itemChanged(); }
    z0 valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

    z0 itemChanged()
    {
        list.updateContent();
        resized();
        repaint();
    }

    static z0 setLocalCopyModeForAllModules (Project& project, b8 copyLocally)
    {
        auto& modules = project.getEnabledModules();

        for (auto i = modules.getNumModules(); --i >= 0;)
           modules.shouldCopyModuleFilesLocallyValue (modules.getModuleID (i)) = copyLocally;
    }

    z0 showCopyModeMenu()
    {
        PopupMenu m;

        m.addItem (PopupMenu::Item ("Set all modules to copy locally")
                     .setAction ([&] { setLocalCopyModeForAllModules (project, true); }));

        m.addItem (PopupMenu::Item ("Set all modules to not copy locally")
                     .setAction ([&] { setLocalCopyModeForAllModules (project, false); }));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (setCopyModeButton));
    }

    static z0 setAllModulesToUseGlobalPaths (Project& project, b8 useGlobal)
    {
        auto& modules = project.getEnabledModules();

        for (auto moduleID : modules.getAllModules())
            modules.shouldUseGlobalPathValue (moduleID) = useGlobal;
    }

    static z0 setSelectedModulesToUseGlobalPaths (Project& project, SparseSet<i32> selected, b8 useGlobal)
    {
        auto& modules = project.getEnabledModules();

        for (i32 i = 0; i < selected.size(); ++i)
            modules.shouldUseGlobalPathValue (modules.getModuleID (selected[i])) = useGlobal;
    }

    z0 showGlobalPathsMenu()
    {
        PopupMenu m;

        m.addItem (PopupMenu::Item ("Set all modules to use global paths")
                    .setAction ([&] { setAllModulesToUseGlobalPaths (project, true); }));

        m.addItem (PopupMenu::Item ("Set all modules to not use global paths")
                    .setAction ([&] { setAllModulesToUseGlobalPaths (project, false); }));

        m.addItem (PopupMenu::Item ("Set selected modules to use global paths")
                    .setEnabled (list.getNumSelectedRows() > 0)
                    .setAction ([&] { setSelectedModulesToUseGlobalPaths (project, list.getSelectedRows(), true); }));

        m.addItem (PopupMenu::Item ("Set selected modules to not use global paths")
                    .setEnabled (list.getNumSelectedRows() > 0)
                    .setAction ([&] { setSelectedModulesToUseGlobalPaths (project, list.getSelectedRows(), false); }));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (globalPathsButton));
    }

    z0 showSetPathsMenu()
    {
        PopupMenu m;
        auto moduleToCopy = project.getEnabledModules().getModuleID (list.getSelectedRow());

        if (moduleToCopy.isNotEmpty())
        {
            m.addItem (PopupMenu::Item ("Copy the paths from the module '" + moduleToCopy + "' to all other modules")
                         .setAction ([this, moduleToCopy]
                                     {
                                         auto& modulesList = project.getEnabledModules();

                                         for (Project::ExporterIterator exporter (project); exporter.next();)
                                         {
                                             for (i32 i = 0; i < modulesList.getNumModules(); ++i)
                                             {
                                                 auto modID = modulesList.getModuleID (i);

                                                 if (modID != moduleToCopy)
                                                     exporter->getPathForModuleValue (modID) = exporter->getPathForModuleValue (moduleToCopy).get();
                                             }
                                         }

                                         list.repaint();
                                     }));

            m.addItem (PopupMenu::Item ("Copy paths from selected module")
                         .setEnabled (list.getNumSelectedRows() == 1)
                         .setAction ([this, moduleToCopy]
                                     {
                                         modulePathClipboard.clear();

                                         for (Project::ExporterIterator exporter (project); exporter.next();)
                                             modulePathClipboard[exporter->getUniqueName()] = exporter->getPathForModuleValue (moduleToCopy).get();

                                         list.repaint();
                                     }));

            m.addItem (PopupMenu::Item ("Paste paths to selected modules")
                         .setEnabled (! modulePathClipboard.empty())
                         .setAction ([this]
                                     {
                                         for (i32 selectionId = 0; selectionId < list.getNumSelectedRows(); ++selectionId)
                                         {
                                             auto rowNumber = list.getSelectedRow (selectionId);
                                             auto modID = project.getEnabledModules().getModuleID (rowNumber);

                                             for (Project::ExporterIterator exporter (project); exporter.next();)
                                                 exporter->getPathForModuleValue (modID) = modulePathClipboard[exporter->getUniqueName()];
                                         }

                                         list.repaint();
                                     }));
        }
        else
        {
            m.addItem (PopupMenu::Item ("(Select a module in the list above to use this option)")
                         .setEnabled (false));
        }

        m.showMenuAsync (PopupMenu::Options()
                           .withDeletionCheck (*this)
                           .withTargetComponent (copyPathButton));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesInformationComponent)
};
