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

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             ValueTreesDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases value tree features.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ValueTreesDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class ValueTreeItem final : public TreeViewItem,
                            private ValueTree::Listener
{
public:
    ValueTreeItem (const ValueTree& v, UndoManager& um)
        : tree (v), undoManager (um)
    {
        tree.addListener (this);
    }

    Txt getUniqueName() const override
    {
        return tree["name"].toString();
    }

    b8 mightContainSubItems() override
    {
        return tree.getNumChildren() > 0;
    }

    z0 paintItem (Graphics& g, i32 width, i32 height) override
    {
        if (isSelected())
            g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::highlightedFill,
                                               Colors::teal));

        g.setColor (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::defaultText,
                                             Colors::black));
        g.setFont (15.0f);

        g.drawText (tree["name"].toString(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen && getNumSubItems() == 0)
            refreshSubItems();
        else
            clearSubItems();
    }

    var getDragSourceDescription() override
    {
        return "Drag Demo";
    }

    b8 isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description == "Drag Demo";
    }

    z0 itemDropped (const DragAndDropTarget::SourceDetails&, i32 insertIndex) override
    {
        OwnedArray<ValueTree> selectedTrees;
        getSelectedTreeViewItems (*getOwnerView(), selectedTrees);

        moveItems (*getOwnerView(), selectedTrees, tree, insertIndex, undoManager);
    }

    static z0 moveItems (TreeView& treeView, const OwnedArray<ValueTree>& items,
                           ValueTree newParent, i32 insertIndex, UndoManager& undoManager)
    {
        if (items.size() > 0)
        {
            std::unique_ptr<XmlElement> oldOpenness (treeView.getOpennessState (false));

            for (auto* v : items)
            {
                if (v->getParent().isValid() && newParent != *v && ! newParent.isAChildOf (*v))
                {
                    if (v->getParent() == newParent && newParent.indexOf (*v) < insertIndex)
                        --insertIndex;

                    v->getParent().removeChild (*v, &undoManager);
                    newParent.addChild (*v, insertIndex, &undoManager);
                }
            }

            if (oldOpenness != nullptr)
                treeView.restoreOpennessState (*oldOpenness, false);
        }
    }

    static z0 getSelectedTreeViewItems (TreeView& treeView, OwnedArray<ValueTree>& items)
    {
        auto numSelected = treeView.getNumSelectedItems();

        for (i32 i = 0; i < numSelected; ++i)
            if (auto* vti = dynamic_cast<ValueTreeItem*> (treeView.getSelectedItem (i)))
                items.add (new ValueTree (vti->tree));
    }

private:
    ValueTree tree;
    UndoManager& undoManager;

    z0 refreshSubItems()
    {
        clearSubItems();

        for (i32 i = 0; i < tree.getNumChildren(); ++i)
            addSubItem (new ValueTreeItem (tree.getChild (i), undoManager));
    }

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override
    {
        repaintItem();
    }

    z0 valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { treeChildrenChanged (parentTree); }
    z0 valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, i32) override  { treeChildrenChanged (parentTree); }
    z0 valueTreeChildOrderChanged (ValueTree& parentTree, i32, i32) override    { treeChildrenChanged (parentTree); }
    z0 valueTreeParentChanged (ValueTree&) override {}

    z0 treeChildrenChanged (const ValueTree& parentTree)
    {
        if (parentTree == tree)
        {
            refreshSubItems();
            treeHasChanged();
            setOpen (true);
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeItem)
};

//==============================================================================
class ValueTreesDemo final : public Component,
                             public DragAndDropContainer,
                             private Timer
{
public:
    ValueTreesDemo()
    {
        addAndMakeVisible (tree);

        tree.setTitle ("ValueTree");
        tree.setDefaultOpenness (true);
        tree.setMultiSelectEnabled (true);
        rootItem.reset (new ValueTreeItem (createRootValueTree(), undoManager));
        tree.setRootItem (rootItem.get());

        addAndMakeVisible (undoButton);
        addAndMakeVisible (redoButton);
        undoButton.onClick = [this] { undoManager.undo(); };
        redoButton.onClick = [this] { undoManager.redo(); };

        startTimer (500);

        setSize (500, 500);
    }

    ~ValueTreesDemo() override
    {
        tree.setRootItem (nullptr);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (8);

        auto buttons = r.removeFromBottom (22);
        undoButton.setBounds (buttons.removeFromLeft (100));
        buttons.removeFromLeft (6);
        redoButton.setBounds (buttons.removeFromLeft (100));

        r.removeFromBottom (4);
        tree.setBounds (r);
    }

    static ValueTree createTree (const Txt& desc)
    {
        ValueTree t ("Item");
        t.setProperty ("name", desc, nullptr);
        return t;
    }

    static ValueTree createRootValueTree()
    {
        auto vt = createTree ("This demo displays a ValueTree as a treeview.");
        vt.appendChild (createTree ("You can drag around the nodes to rearrange them"),               nullptr);
        vt.appendChild (createTree ("..and press 'delete' or 'backspace' to delete them"),            nullptr);
        vt.appendChild (createTree ("Then, you can use the undo/redo buttons to undo these changes"), nullptr);

        i32 n = 1;
        vt.appendChild (createRandomTree (n, 0), nullptr);

        return vt;
    }

    static ValueTree createRandomTree (i32& counter, i32 depth)
    {
        auto t = createTree ("Item " + Txt (counter++));

        if (depth < 3)
            for (i32 i = 1 + Random::getSystemRandom().nextInt (7); --i >= 0;)
                t.appendChild (createRandomTree (counter, depth + 1), nullptr);

        return t;
    }

    z0 deleteSelectedItems()
    {
        OwnedArray<ValueTree> selectedItems;
        ValueTreeItem::getSelectedTreeViewItems (tree, selectedItems);

        for (auto* v : selectedItems)
        {
            if (v->getParent().isValid())
                v->getParent().removeChild (*v, &undoManager);
        }
    }

    b8 keyPressed (const KeyPress& key) override
    {
        if (key == KeyPress::deleteKey || key == KeyPress::backspaceKey)
        {
            deleteSelectedItems();
            return true;
        }

        if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
        {
            undoManager.undo();
            return true;
        }

        if (key == KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
        {
            undoManager.redo();
            return true;
        }

        return Component::keyPressed (key);
    }

private:
    TreeView tree;
    TextButton undoButton  { "Undo" },
               redoButton  { "Redo" };

    std::unique_ptr<ValueTreeItem> rootItem;
    UndoManager undoManager;

    z0 timerCallback() override
    {
        undoManager.beginNewTransaction();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreesDemo)
};
