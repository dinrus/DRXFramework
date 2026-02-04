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
class ExporterItem final : public ProjectTreeItemBase,
                           private Value::Listener
{
public:
    ExporterItem (Project& p, ProjectExporter* e, i32 index)
        : project (p), exporter (e), configListTree (exporter->getConfigurations()),
          exporterIndex (index)
    {
        exporter->initialiseDependencyPathValues();
        configListTree.addListener (this);
        targetLocationValue.referTo (exporter->getTargetLocationValue());
        targetLocationValue.addListener (this);
    }

    i32 getItemHeight() const override        { return 25; }
    b8 canBeSelected() const override       { return true; }
    b8 mightContainSubItems() override      { return exporter->getNumConfigurations() > 0; }
    Txt getUniqueName() const override     { return "exporter_" + Txt (exporterIndex); }
    Txt getRenamingName() const override   { return getDisplayName(); }
    Txt getDisplayName() const override    { return exporter->getUniqueName(); }
    z0 setName (const Txt&) override     {}
    b8 isMissing() const override           { return false; }
    Txt getTooltip() override              { return getDisplayName(); }

    static Icon getIconForExporter (ProjectExporter* e)
    {
        if (e != nullptr)
        {
            if         (e->isXcode())        return Icon (getIcons().xcode,        Colors::transparentBlack);
            else if    (e->isVisualStudio()) return Icon (getIcons().visualStudio, Colors::transparentBlack);
            else if    (e->isAndroid())      return Icon (getIcons().android,      Colors::transparentBlack);
            else if    (e->isMakefile())     return Icon (getIcons().linux,        Colors::transparentBlack);
        }

        return Icon();
    }

    Icon getIcon() const override
    {
        return getIconForExporter (exporter.get()).withColor (getContentColor (true));
    }

    z0 showDocument() override
    {
        showSettingsPage (new SettingsComp (*exporter));
    }

    z0 deleteItem() override
    {
        auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                               "Delete Exporter",
                                                               "Are you sure you want to delete this export target?");
        messageBox = AlertWindow::showScopedAsync (options, [safeThis = WeakReference { this }] (i32 result)
        {
            if (safeThis == nullptr || result == 0)
                return;

            safeThis->closeSettingsPage();

            auto parent = safeThis->exporter->settings.getParent();
            parent.removeChild (safeThis->exporter->settings,
                                safeThis->project.getUndoManagerFor (parent));
        });
    }

    z0 addSubItems() override
    {
        for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            addSubItem (new ConfigItem (config.config, *exporter));
    }

    z0 showPopupMenu (Point<i32> p) override
    {
        PopupMenu menu;
        menu.addItem (1, "Add a new configuration", exporter->supportsUserDefinedConfigurations());
        menu.addItem (2, "Save this exporter");
        menu.addSeparator();
        menu.addItem (3, "Delete this exporter");

        launchPopupMenu (menu, p);
    }

    z0 showAddMenu (Point<i32> p) override
    {
        PopupMenu menu;
        menu.addItem (1, "Add a new configuration", exporter->supportsUserDefinedConfigurations());

        launchPopupMenu (menu, p);
    }

    z0 handlePopupMenuResult (i32 resultCode) override
    {
        if (resultCode == 1)
            exporter->addNewConfiguration (false);
        else if (resultCode == 2)
            project.saveProject (Async::yes, exporter.get(), nullptr);
        else if (resultCode == 3)
            deleteAllSelectedItems();
    }

    var getDragSourceDescription() override
    {
        return getParentItem()->getUniqueName() + "/" + Txt (exporterIndex);
    }

    b8 isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description.toString().startsWith (getUniqueName());
    }

    z0 itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, i32 insertIndex) override
    {
        auto oldIndex = indexOfConfig (dragSourceDetails.description.toString().fromLastOccurrenceOf ("||", false, false));

        if (oldIndex >= 0)
            configListTree.moveChild (oldIndex, insertIndex, project.getUndoManagerFor (configListTree));
    }

    i32 indexOfConfig (const Txt& configName)
    {
        i32 i = 0;
        for (ProjectExporter::ConfigIterator config (*exporter); config.next(); ++i)
            if (config->getName() == configName)
                return i;

        return -1;
    }

    //==============================================================================
    z0 valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    z0 valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, i32) override  { refreshIfNeeded (parentTree); }
    z0 valueTreeChildOrderChanged (ValueTree& parentTree, i32, i32) override    { refreshIfNeeded (parentTree); }

    z0 refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == configListTree)
            refreshSubItems();
    }

private:
    Project& project;
    std::unique_ptr<ProjectExporter> exporter;
    ValueTree configListTree;
    i32 exporterIndex;

    Value targetLocationValue;

    ScopedMessageBox messageBox;

    z0 valueChanged (Value& value) override
    {
        if (value == exporter->getTargetLocationValue())
            refreshSubItems();
    }

    //==============================================================================
    struct SettingsComp final : public Component
    {
        SettingsComp (ProjectExporter& exp)
            : group (exp.getUniqueName(),
                     ExporterItem::getIconForExporter (&exp),
                     exp.getDescription())
        {
            addAndMakeVisible (group);

            PropertyListBuilder props;
            exp.createPropertyEditors (props);
            group.setProperties (props);
            parentSizeChanged();
        }

        z0 parentSizeChanged() override   { updateSize (*this, group); }
        z0 resized() override             { group.setBounds (getLocalBounds().withTrimmedLeft (12)); }

        PropertyGroupComponent group;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterItem)
    DRX_DECLARE_WEAK_REFERENCEABLE (ExporterItem)
};


//==============================================================================
class ConfigItem final : public ProjectTreeItemBase
{
public:
    ConfigItem (const ProjectExporter::BuildConfiguration::Ptr& conf, ProjectExporter& e)
        : config (conf), exporter (e), configTree (config->config)
    {
        jassert (config != nullptr);
        configTree.addListener (this);
    }

    b8 isMissing() const override                 { return false; }
    b8 canBeSelected() const override             { return true; }
    b8 mightContainSubItems() override            { return false; }
    Txt getUniqueName() const override           { return "config_" + config->getName(); }
    Txt getRenamingName() const override         { return getDisplayName(); }
    Txt getDisplayName() const override          { return config->getName(); }
    z0 setName (const Txt&) override           {}
    Icon getIcon() const override                   { return Icon (getIcons().config, getContentColor (true)); }
    z0 itemOpennessChanged (b8) override        {}

    z0 showDocument() override
    {
        showSettingsPage (new SettingsComp (*config));
    }

    z0 deleteItem() override
    {
        auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                               "Delete Configuration",
                                                               "Are you sure you want to delete this configuration?");
        messageBox = AlertWindow::showScopedAsync (options, [parent = WeakReference { this }] (i32 result)
        {
            if (parent == nullptr)
                return;

            if (result == 0)
                return;

            parent->closeSettingsPage();
            parent->config->removeFromExporter();
        });
    }

    z0 showPopupMenu (Point<i32> p) override
    {
        b8 enabled = exporter.supportsUserDefinedConfigurations();

        PopupMenu menu;
        menu.addItem (1, "Create a copy of this configuration", enabled);
        menu.addSeparator();
        menu.addItem (2, "Delete this configuration", enabled);

        launchPopupMenu (menu, p);
    }

    z0 handlePopupMenuResult (i32 resultCode) override
    {
        if (resultCode == 1)
            exporter.addNewConfigurationFromExisting (*config);
        else if (resultCode == 2)
            deleteAllSelectedItems();
    }

    var getDragSourceDescription() override
    {
        return getParentItem()->getUniqueName() + "||" + config->getName();
    }

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override  { repaintItem(); }

private:
    ProjectExporter::BuildConfiguration::Ptr config;
    ProjectExporter& exporter;
    ValueTree configTree;
    ScopedMessageBox messageBox;

    //==============================================================================
    class SettingsComp final : public Component
    {
    public:
        SettingsComp (ProjectExporter::BuildConfiguration& conf)
            : group (conf.exporter.getUniqueName() + " - " + conf.getName(), Icon (getIcons().config, Colors::transparentBlack))
        {
            addAndMakeVisible (group);

            PropertyListBuilder props;
            conf.createPropertyEditors (props);
            group.setProperties (props);
            parentSizeChanged();
        }

        z0 parentSizeChanged() override  { updateSize (*this, group); }

        z0 resized() override { group.setBounds (getLocalBounds().withTrimmedLeft (12)); }

    private:
        PropertyGroupComponent group;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigItem)
    DRX_DECLARE_WEAK_REFERENCEABLE (ConfigItem)
};

//==============================================================================
class ExportersTreeRoot final : public ProjectTreeItemBase
{
public:
    ExportersTreeRoot (Project& p)
        : project (p),
          exportersTree (project.getExporters())
    {
        exportersTree.addListener (this);
    }

    b8 isRoot() const override                     { return true; }
    b8 canBeSelected() const override              { return true; }
    b8 isMissing() const override                  { return false; }
    b8 mightContainSubItems() override             { return project.getNumExporters() > 0; }
    Txt getUniqueName() const override            { return "exporters"; }
    Txt getRenamingName() const override          { return getDisplayName(); }
    Txt getDisplayName() const override           { return "Exporters"; }
    z0 setName (const Txt&) override            {}
    Icon getIcon() const override                    { return project.getMainGroup().getIcon (isOpen()).withColor (getContentColor (true)); }

    z0 showPopupMenu (Point<i32>) override
    {
        if (auto* pcc = getProjectContentComponent())
            pcc->showNewExporterMenu();
    }

    z0 addSubItems() override
    {
        i32 i = 0;
        for (Project::ExporterIterator exporter (project); exporter.next(); ++i)
            addSubItem (new TreeItemTypes::ExporterItem (project, exporter.exporter.release(), i));
    }

    b8 isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description.toString().startsWith (getUniqueName());
    }

    z0 itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, i32 insertIndex) override
    {
        i32 oldIndex = dragSourceDetails.description.toString().getTrailingIntValue();
        exportersTree.moveChild (oldIndex, jmax (0, insertIndex), project.getUndoManagerFor (exportersTree));
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen)
            refreshSubItems();
    }

    z0 removeExporter (i32 index)
    {
        if (auto* exporter = dynamic_cast<TreeItemTypes::ExporterItem*> (getSubItem (index)))
            exporter->deleteItem();
    }

private:
    Project& project;
    ValueTree exportersTree;

    //==============================================================================
    z0 valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    z0 valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, i32) override  { refreshIfNeeded (parentTree); }
    z0 valueTreeChildOrderChanged (ValueTree& parentTree, i32, i32) override    { refreshIfNeeded (parentTree); }

    z0 refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == exportersTree)
            refreshSubItems();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExportersTreeRoot)
};
