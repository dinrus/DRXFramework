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
class StartPageTreeHolder final : public Component
{
public:
    enum class Open { no, yes };

    StartPageTreeHolder (const Txt& title,
                         const StringArray& headerNames,
                         const std::vector<StringArray>& itemNames,
                         std::function<z0 (i32, i32)>&& selectedCallback,
                         Open shouldBeOpen)
        : headers (headerNames),
          items (itemNames),
          itemSelectedCallback (std::move (selectedCallback))
    {
        jassert (headers.size() == (i32) items.size());

        tree.setTitle (title);
        tree.setRootItem (new TreeRootItem (*this));
        tree.setRootItemVisible (false);
        tree.setIndentSize (15);
        tree.setDefaultOpenness (shouldBeOpen == Open::yes);

        addAndMakeVisible (tree);
    }

    ~StartPageTreeHolder() override
    {
        tree.deleteRootItem();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (secondaryBackgroundColorId));
    }

    z0 resized() override
    {
        tree.setBounds (getLocalBounds());
    }

    z0 setSelectedItem (const Txt& category, i32 index)
    {
        auto* root = tree.getRootItem();

        for (i32 i = root->getNumSubItems(); --i >=0;)
        {
            if (auto* item = root->getSubItem (i))
            {
                if (item->getUniqueName() == category)
                    item->getSubItem (index)->setSelected (true, true);
            }
        }
    }

private:
    //==============================================================================
    class TreeSubItem final : public TreeViewItem
    {
    public:
        TreeSubItem (StartPageTreeHolder& o, const Txt& n, const StringArray& subItemsIn)
            : owner (o), name (n), isHeader (subItemsIn.size() > 0)
        {
            for (auto& s : subItemsIn)
                addSubItem (new TreeSubItem (owner, s, {}));
        }

        b8 mightContainSubItems() override    { return isHeader; }
        b8 canBeSelected() const override     { return ! isHeader; }

        i32 getItemWidth() const override       { return -1; }
        i32 getItemHeight() const override      { return 25; }

        Txt getUniqueName() const override   { return name; }
        Txt getAccessibilityName() override  { return getUniqueName(); }

        z0 paintOpenCloseButton (Graphics& g, const Rectangle<f32>& area, Color, b8 isMouseOver) override
        {
            g.setColor (getOwnerView()->findColor (isSelected() ? defaultHighlightedTextColorId
                                                                  : treeIconColorId));

            TreeViewItem::paintOpenCloseButton (g, area, getOwnerView()->findColor (defaultIconColorId), isMouseOver);
        }

        z0 paintItem (Graphics& g, i32 w, i32 h) override
        {
            Rectangle<i32> bounds (w, h);

            auto shouldBeHighlighted = isSelected();

            if (shouldBeHighlighted)
            {
                g.setColor (getOwnerView()->findColor (defaultHighlightColorId));
                g.fillRect (bounds);
            }

            g.setColor (shouldBeHighlighted ? getOwnerView()->findColor (defaultHighlightedTextColorId)
                                             : getOwnerView()->findColor (defaultTextColorId));

            g.drawFittedText (name, bounds.reduced (5).withTrimmedLeft (10), Justification::centredLeft, 1);
        }

        z0 itemClicked (const MouseEvent& e) override
        {
            if (isSelected())
                itemSelectionChanged (true);

            if (e.mods.isPopupMenu() && mightContainSubItems())
                setOpen (! isOpen());
        }

        z0 itemSelectionChanged (b8 isNowSelected) override
        {
            jassert (! isHeader);

            if (isNowSelected)
                owner.itemSelectedCallback (getParentItem()->getIndexInParent(), getIndexInParent());
        }

    private:
        StartPageTreeHolder& owner;
        Txt name;
        b8 isHeader = false;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeSubItem)
    };

    struct TreeRootItem final : public TreeViewItem
    {
        explicit TreeRootItem (StartPageTreeHolder& o)
            : owner (o)
        {
            for (i32 i = 0; i < owner.headers.size(); ++i)
                addSubItem (new TreeSubItem (owner, owner.headers[i], owner.items[(size_t) i]));
        }

        b8 mightContainSubItems() override { return ! owner.headers.isEmpty();}

        StartPageTreeHolder& owner;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeRootItem)
    };

    //==============================================================================
    TreeView tree;
    StringArray headers;
    std::vector<StringArray> items;

    std::function<z0 (i32, i32)> itemSelectedCallback;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartPageTreeHolder)
};
