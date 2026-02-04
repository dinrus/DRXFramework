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
class ConcertinaHeader final : public Component,
                               public ChangeBroadcaster
{
public:
    ConcertinaHeader (Txt n, Path p)
        : Component (n), name (n), iconPath (p)
    {
        setTitle (getName());

        panelIcon = Icon (iconPath, Colors::white);

        nameLabel.setText (name, dontSendNotification);
        nameLabel.setJustificationType (Justification::centredLeft);
        nameLabel.setInterceptsMouseClicks (false, false);
        nameLabel.setAccessible (false);
        nameLabel.setColor (Label::textColorId, Colors::white);

        addAndMakeVisible (nameLabel);
    }

    z0 resized() override
    {
        auto b = getLocalBounds().toFloat();

        iconBounds = b.removeFromLeft (b.getHeight()).reduced (7, 7);
        arrowBounds = b.removeFromRight (b.getHeight());
        nameLabel.setBounds (b.toNearestInt());
    }

    z0 paint (Graphics& g) override
    {
        g.setColor (findColor (defaultButtonBackgroundColorId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2, 3).toFloat(), 2.0f);

        g.setColor (Colors::white);
        g.fillPath (ProjucerLookAndFeel::getArrowPath (arrowBounds,
                                                       getParentComponent()->getBoundsInParent().getY() == yPosition ? 2 : 0,
                                                       true, Justification::centred));

        panelIcon.draw (g, iconBounds.toFloat(), false);
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        if (! e.mouseWasDraggedSinceMouseDown())
            sendChangeMessage();
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::button,
                                                       AccessibilityActions().addAction (AccessibilityActionType::press,
                                                                                         [this] { sendChangeMessage(); }));
    }

    i32 direction = 0;
    i32 yPosition = 0;

private:
    Txt name;
    Label nameLabel;

    Path iconPath;
    Icon panelIcon;

    Rectangle<f32> arrowBounds, iconBounds;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaHeader)
};

//==============================================================================
class FindPanel final : public Component,
                        private Timer,
                        private FocusChangeListener
{
public:
    FindPanel (std::function<z0 (const Txt&)> cb)
        : callback (cb)
    {
        addAndMakeVisible (editor);
        editor.onTextChange = [this] { startTimer (250); };
        editor.onFocusLost  = [this]
        {
            isFocused = false;
            repaint();
        };

        Desktop::getInstance().addFocusChangeListener (this);

        lookAndFeelChanged();
    }

    ~FindPanel() override
    {
        Desktop::getInstance().removeFocusChangeListener (this);
    }

    z0 paintOverChildren (Graphics& g) override
    {
        if (! isFocused)
            return;

        g.setColor (findColor (defaultHighlightColorId));

        Path p;
        p.addRoundedRectangle (getLocalBounds().reduced (2), 3.0f);
        g.strokePath (p, PathStrokeType (2.0f));
    }


    z0 resized() override
    {
        editor.setBounds (getLocalBounds().reduced (2));
    }

private:
    TextEditor editor;
    b8 isFocused = false;
    std::function<z0 (const Txt&)> callback;

    //==============================================================================
    z0 lookAndFeelChanged() override
    {
        editor.setTextToShowWhenEmpty ("Filter...", findColor (widgetTextColorId).withAlpha (0.3f));
    }

    z0 globalFocusChanged (Component* focusedComponent) override
    {
        if (focusedComponent == &editor)
        {
            isFocused = true;
            repaint();
        }
    }

    z0 timerCallback() override
    {
        stopTimer();
        callback (editor.getText());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FindPanel)
};

//==============================================================================
class ConcertinaTreeComponent final : public Component
{
public:
    class AdditionalComponents
    {
    public:
        enum Type
        {
            addButton      = (1 << 0),
            settingsButton = (1 << 1),
            findPanel      = (1 << 2)
        };

        [[nodiscard]] AdditionalComponents with (Type t)
        {
            auto copy = *this;
            copy.componentTypes |= t;

            return copy;
        }

        b8 has (Type t) const noexcept
        {
            return (componentTypes & t) != 0;
        }

    private:
        i32 componentTypes = 0;
    };

    ConcertinaTreeComponent (const Txt& name,
                             TreePanelBase* tree,
                             AdditionalComponents additionalComponents)
         : Component (name),
           treeToDisplay (tree)
    {
        setTitle (getName());
        setFocusContainerType (FocusContainerType::focusContainer);

        if (additionalComponents.has (AdditionalComponents::addButton))
        {
            addButton = std::make_unique<IconButton> ("Add", getIcons().plus);
            addAndMakeVisible (addButton.get());
            addButton->onClick = [this] { showAddMenu(); };
        }

        if (additionalComponents.has (AdditionalComponents::settingsButton))
        {
            settingsButton = std::make_unique<IconButton> ("Settings", getIcons().settings);
            addAndMakeVisible (settingsButton.get());
            settingsButton->onClick = [this] { showSettings(); };
        }

        if (additionalComponents.has (AdditionalComponents::findPanel))
        {
            findPanel = std::make_unique<FindPanel> ([this] (const Txt& filter) { treeToDisplay->rootItem->setSearchFilter (filter); });
            addAndMakeVisible (findPanel.get());
        }

        addAndMakeVisible (treeToDisplay.get());
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        if (addButton != nullptr || settingsButton != nullptr || findPanel != nullptr)
        {
            auto bottomSlice = bounds.removeFromBottom (25);
            bottomSlice.removeFromRight (3);

            if (addButton != nullptr)
                addButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (settingsButton != nullptr)
                settingsButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (findPanel != nullptr)
                findPanel->setBounds (bottomSlice.reduced (2));
        }

        treeToDisplay->setBounds (bounds);
    }

    TreePanelBase* getTree() const noexcept    { return treeToDisplay.get(); }

private:
    std::unique_ptr<TreePanelBase> treeToDisplay;
    std::unique_ptr<IconButton> addButton, settingsButton;
    std::unique_ptr<FindPanel> findPanel;

    z0 showAddMenu()
    {
        auto numSelected = treeToDisplay->tree.getNumSelectedItems();

        if (numSelected > 1)
            return;

        if (numSelected == 0)
        {
            if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
                root->showPopupMenu (addButton->getScreenBounds().getCentre());
        }
        else
        {
            if (auto* item = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getSelectedItem (0)))
                item->showAddMenu (addButton->getScreenBounds().getCentre());
        }
    }

    z0 showSettings()
    {
        if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
        {
            treeToDisplay->tree.clearSelectedItems();
            root->showDocument();
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaTreeComponent)
};


//==============================================================================
struct ProjectSettingsComponent final : public Component,
                                        private ChangeListener
{
    ProjectSettingsComponent (Project& p)
        : project (p),
          group (project.getProjectFilenameRootString(),
                 Icon (getIcons().settings, Colors::transparentBlack))
    {
        setTitle ("Project Settings");
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (group);

        updatePropertyList();
        project.addChangeListener (this);
    }

    ~ProjectSettingsComponent() override
    {
        project.removeChangeListener (this);
    }

    z0 resized() override
    {
        group.updateSize (12, 0, getWidth() - 24);
        group.setBounds (getLocalBounds().reduced (12, 0));
    }

    z0 updatePropertyList()
    {
        PropertyListBuilder props;
        project.createPropertyEditors (props);
        group.setProperties (props);
        group.setName ("Project Settings");

        lastProjectType = project.getProjectTypeString();
        parentSizeChanged();
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        if (lastProjectType != project.getProjectTypeString())
            updatePropertyList();
    }

    z0 parentSizeChanged() override
    {
        auto width = jmax (550, getParentWidth());
        auto y = group.updateSize (12, 0, width - 12);

        y = jmax (getParentHeight(), y);

        setSize (width, y);
    }

    Project& project;
    var lastProjectType;
    PropertyGroupComponent group;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSettingsComponent)
};

//==============================================================================
struct FileTreePanel final : public TreePanelBase
{
    FileTreePanel (Project& p)
        : TreePanelBase (&p, "fileTreeState")
    {
        tree.setMultiSelectEnabled (true);
        setRoot (std::make_unique<TreeItemTypes::GroupItem> (p.getMainGroup()));
        tree.setRootItemVisible (false);
    }

    z0 updateMissingFileStatuses()
    {
        if (auto* p = dynamic_cast<TreeItemTypes::FileTreeItemBase*> (rootItem.get()))
            p->checkFileStatus();
    }
};

struct ModuleTreePanel final : public TreePanelBase
{
    ModuleTreePanel (Project& p)
        : TreePanelBase (&p, "moduleTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (std::make_unique<TreeItemTypes::EnabledModulesItem> (p));
        tree.setRootItemVisible (false);
    }
};

struct ExportersTreePanel final : public TreePanelBase
{
    ExportersTreePanel (Project& p)
        : TreePanelBase (&p, "exportersTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (std::make_unique<TreeItemTypes::ExportersTreeRoot> (p));
        tree.setRootItemVisible (false);
    }
};

//==============================================================================
class Sidebar final : public Component,
                      private ChangeListener
{
public:
    Sidebar (Project* p)
        : project (p)
    {
        setFocusContainerType (FocusContainerType::focusContainer);

        if (project != nullptr)
            buildConcertina();
    }

    ~Sidebar() override
    {
        TreePanelBase* panels[] = { getFileTreePanel(), getModuleTreePanel(), getExportersTreePanel() };

        for (auto* panel : panels)
            if (panel != nullptr)
                panel->saveOpenness();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (secondaryBackgroundColorId));
    }

    z0 resized() override
    {
        concertinaPanel.setBounds (getLocalBounds().withTrimmedBottom (3));
    }

    TreePanelBase* getTreeWithSelectedItems()
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
        {
            if (auto* treeComponent = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (i)))
            {
                if (auto* base = treeComponent->getTree())
                    if (base->tree.getNumSelectedItems() != 0)
                        return base;
            }
        }

        return nullptr;
    }

    FileTreePanel*      getFileTreePanel()        { return getPanel<FileTreePanel>      (0); }
    ModuleTreePanel*    getModuleTreePanel()      { return getPanel<ModuleTreePanel>    (1); }
    ExportersTreePanel* getExportersTreePanel()   { return getPanel<ExportersTreePanel> (2); }

    z0 showPanel (i32 panelIndex)
    {
        jassert (isPositiveAndBelow (panelIndex, concertinaPanel.getNumPanels()));

        concertinaPanel.expandPanelFully (concertinaPanel.getPanel (panelIndex), true);
    }

private:
    //==============================================================================
    template <typename PanelType>
    PanelType* getPanel (i32 panelIndex)
    {
        if (auto* panel = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (panelIndex)))
            return dynamic_cast<PanelType*> (panel->getTree());

        return nullptr;
    }

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        const auto pointerMatches = [source] (const std::unique_ptr<ConcertinaHeader>& header) { return header.get() == source; };
        const auto it = std::find_if (headers.begin(), headers.end(), pointerMatches);
        const auto index = (i32) std::distance (headers.begin(), it);

        if (index != (i32) headers.size())
            concertinaPanel.expandPanelFully (concertinaPanel.getPanel (index), true);
    }

    z0 buildConcertina()
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0 ; --i)
            concertinaPanel.removePanel (concertinaPanel.getPanel (i));

        headers.clear();

        auto addPanel = [this] (const Txt& name,
                                TreePanelBase* tree,
                                ConcertinaTreeComponent::AdditionalComponents components,
                                const Path& icon)
        {
            if (project != nullptr)
                concertinaPanel.addPanel (-1, new ConcertinaTreeComponent (name, tree, components), true);

            headers.push_back (std::make_unique<ConcertinaHeader> (name, icon));
        };

        using AdditionalComponents = ConcertinaTreeComponent::AdditionalComponents;

        addPanel ("File Explorer", new FileTreePanel (*project),
                  AdditionalComponents{}
                      .with (AdditionalComponents::addButton)
                      .with (AdditionalComponents::findPanel),
                  getIcons().fileExplorer);

        addPanel ("Modules", new ModuleTreePanel (*project),
                  AdditionalComponents{}
                      .with (AdditionalComponents::addButton)
                      .with (AdditionalComponents::settingsButton),
                  getIcons().modules);

        addPanel ("Exporters", new ExportersTreePanel (*project),
                  AdditionalComponents{}.with (AdditionalComponents::addButton),
                  getIcons().exporter);

        for (i32 i = 0; i < concertinaPanel.getNumPanels(); ++i)
        {
            auto* p = concertinaPanel.getPanel (i);
            auto* h = headers[(size_t) i].get();
            p->addMouseListener (this, true);

            h->addChangeListener (this);
            h->yPosition = i * 30;

            concertinaPanel.setCustomPanelHeader (p, h, false);
            concertinaPanel.setPanelHeaderSize (p, 30);
        }

        addAndMakeVisible (concertinaPanel);
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
        {
            if (auto* p = concertinaPanel.getPanel (i))
            {
                if (! (p->isParentOf (e.eventComponent)))
                {
                    auto* base = dynamic_cast<TreePanelBase*> (p);

                    if (base == nullptr)
                        if (auto* concertina = dynamic_cast<ConcertinaTreeComponent*> (p))
                            base = concertina->getTree();

                    if (base != nullptr)
                        base->tree.clearSelectedItems();
                }
            }
        }
    }

    //==============================================================================
    std::vector<std::unique_ptr<ConcertinaHeader>> headers;
    ConcertinaPanel concertinaPanel;
    Project* project = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sidebar)
};
