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

class ProjectContentComponent;
class Project;

//==============================================================================
class JucerTreeViewBase : public TreeViewItem,
                          public TooltipClient
{
public:
    JucerTreeViewBase();
    ~JucerTreeViewBase() override = default;

    i32 getItemWidth() const override                   { return -1; }
    i32 getItemHeight() const override                  { return 25; }

    z0 paintOpenCloseButton (Graphics&, const Rectangle<f32>& area, Color backgroundColor, b8 isMouseOver) override;
    z0 paintItem (Graphics& g, i32 width, i32 height) override;
    z0 itemClicked (const MouseEvent& e) override;
    z0 itemSelectionChanged (b8 isNowSelected) override;
    z0 itemDoubleClicked (const MouseEvent&) override;
    std::unique_ptr<Component> createItemComponent() override;
    Txt getTooltip() override            { return {}; }
    Txt getAccessibilityName() override  { return getDisplayName(); }

    z0 cancelDelayedSelectionTimer();

    //==============================================================================
    virtual b8 isRoot() const                                   { return false; }
    virtual Font getFont() const;
    virtual Txt getRenamingName() const = 0;
    virtual Txt getDisplayName() const = 0;
    virtual z0 setName (const Txt& newName) = 0;
    virtual b8 isMissing() const = 0;
    virtual b8 hasWarnings() const                              { return false; }
    virtual Icon getIcon() const = 0;
    virtual b8 isIconCrossedOut() const                         { return false; }
    virtual z0 paintIcon (Graphics& g, Rectangle<f32> area);
    virtual z0 paintContent (Graphics& g, Rectangle<i32> area);
    virtual i32 getRightHandButtonSpace() { return 0; }
    virtual Color getContentColor (b8 isIcon) const;
    virtual i32 getMillisecsAllowedForDragGesture()               { return 120; }
    virtual File getDraggableFile() const                         { return {}; }

    z0 refreshSubItems();
    z0 showRenameBox();

    virtual z0 deleteItem()                              {}
    virtual z0 deleteAllSelectedItems()                  {}
    virtual z0 showDocument()                            {}
    virtual z0 showMultiSelectionPopupMenu (Point<i32>)  {}
    virtual z0 showPopupMenu (Point<i32>)                {}
    virtual z0 showAddMenu (Point<i32>)                  {}
    virtual z0 handlePopupMenuResult (i32)               {}
    virtual z0 setSearchFilter (const Txt&)           {}

    z0 launchPopupMenu (PopupMenu&, Point<i32>); // runs asynchronously, and produces a callback to handlePopupMenuResult().

    //==============================================================================
    // To handle situations where an item gets deleted before openness is
    // restored for it, this OpennessRestorer keeps only a pointer to the
    // topmost tree item.
    struct WholeTreeOpennessRestorer final : public OpennessRestorer
    {
        WholeTreeOpennessRestorer (TreeViewItem& item)  : OpennessRestorer (getTopLevelItem (item))
        {}

    private:
        static TreeViewItem& getTopLevelItem (TreeViewItem& item)
        {
            if (TreeViewItem* const p = item.getParentItem())
                return getTopLevelItem (*p);

            return item;
        }
    };

    i32 textX = 0;

protected:
    ProjectContentComponent* getProjectContentComponent() const;
    virtual z0 addSubItems() {}

private:
    class ItemSelectionTimer;
    std::unique_ptr<Timer> delayedSelectionTimer;

    z0 invokeShowDocument();

    DRX_DECLARE_WEAK_REFERENCEABLE (JucerTreeViewBase)
};

//==============================================================================
class TreePanelBase : public Component
{
public:
    TreePanelBase (const Project* p, const Txt& treeviewID)
        : project (p), opennessStateKey (treeviewID)
    {
        addAndMakeVisible (tree);

        tree.setRootItemVisible (true);
        tree.setDefaultOpenness (true);
        tree.setColor (TreeView::backgroundColorId, Colors::transparentBlack);
        tree.setIndentSize (14);
        tree.getViewport()->setScrollBarThickness (6);

        tree.addMouseListener (this, true);
    }

    ~TreePanelBase() override
    {
        tree.setRootItem (nullptr);
    }

    z0 setRoot (std::unique_ptr<JucerTreeViewBase>);
    z0 saveOpenness();

    virtual z0 deleteSelectedItems()
    {
        if (rootItem != nullptr)
            rootItem->deleteAllSelectedItems();
    }

    z0 setEmptyTreeMessage (const Txt& newMessage)
    {
        if (emptyTreeMessage != newMessage)
        {
            emptyTreeMessage = newMessage;
            repaint();
        }
    }

    static z0 drawEmptyPanelMessage (Component& comp, Graphics& g, const Txt& message)
    {
        i32k fontHeight = 13;
        const Rectangle<i32> area (comp.getLocalBounds());
        g.setColor (comp.findColor (defaultTextColorId));
        g.setFont ((f32) fontHeight);
        g.drawFittedText (message, area.reduced (4, 2), Justification::centred, area.getHeight() / fontHeight);
    }

    z0 paint (Graphics& g) override
    {
        if (emptyTreeMessage.isNotEmpty() && (rootItem == nullptr || rootItem->getNumSubItems() == 0))
            drawEmptyPanelMessage (*this, g, emptyTreeMessage);
    }

    z0 resized() override
    {
        tree.setBounds (getAvailableBounds());
    }

    Rectangle<i32> getAvailableBounds() const
    {
        return Rectangle<i32> (0, 2, getWidth() - 2, getHeight() - 2);
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        if (e.eventComponent == &tree)
        {
            tree.clearSelectedItems();

            if (e.mods.isRightButtonDown())
                rootItem->showPopupMenu (e.getMouseDownScreenPosition());
        }
    }

    const Project* project;
    TreeView tree;
    std::unique_ptr<JucerTreeViewBase> rootItem;

private:
    Txt opennessStateKey, emptyTreeMessage;
};

//==============================================================================
class TreeItemComponent final : public Component
{
public:
    TreeItemComponent (JucerTreeViewBase& i)  : item (&i)
    {
        setAccessible (false);
        setInterceptsMouseClicks (false, true);
        item->textX = iconWidth;
    }

    z0 paint (Graphics& g) override
    {
        if (item == nullptr)
            return;

        auto bounds = getLocalBounds().toFloat();
        auto iconBounds = bounds.removeFromLeft ((f32) iconWidth).reduced (7, 5);

        bounds.removeFromRight ((f32) buttons.size() * bounds.getHeight());

        item->paintIcon    (g, iconBounds);
        item->paintContent (g, bounds.toNearestInt());
    }

    z0 resized() override
    {
        auto r = getLocalBounds();

        for (i32 i = buttons.size(); --i >= 0;)
            buttons.getUnchecked (i)->setBounds (r.removeFromRight (r.getHeight()));
    }

    z0 addRightHandButton (Component* button)
    {
        buttons.add (button);
        addAndMakeVisible (button);
    }

    WeakReference<JucerTreeViewBase> item;
    OwnedArray<Component> buttons;

    i32k iconWidth = 25;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeItemComponent)
};
