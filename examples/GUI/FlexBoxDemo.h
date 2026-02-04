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

 name:             FlexBoxDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Responsive layouts using FlexBox.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FlexBoxDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct DemoFlexPanel final : public Component
{
    DemoFlexPanel (Color col, FlexItem& item)
        : flexItem (item), colour (col)
    {
        i32 x = 70;
        i32 y = 3;

        setupTextEditor (flexOrderEditor, { x, y, 20, 18 }, "0", [this] { flexItem.order = (i32) flexOrderEditor.getText().getFloatValue(); });
        addLabel ("order", flexOrderEditor);
        y += 20;

        setupTextEditor (flexGrowEditor, { x, y, 20, 18 }, "0", [this] { flexItem.flexGrow = flexGrowEditor.getText().getFloatValue(); });
        addLabel ("flex-grow", flexGrowEditor);
        y += 20;

        setupTextEditor (flexShrinkEditor, { x, y, 20, 18 }, "1", [this] { flexItem.flexShrink = flexShrinkEditor.getText().getFloatValue(); });
        addLabel ("flex-shrink", flexShrinkEditor);
        y += 20;

        setupTextEditor (flexBasisEditor, { x, y, 33, 18 }, "100", [this] { flexItem.flexBasis = flexBasisEditor.getText().getFloatValue(); });
        addLabel ("flex-basis", flexBasisEditor);
        y += 20;

        alignSelfCombo.addItem ("auto",       1);
        alignSelfCombo.addItem ("flex-start", 2);
        alignSelfCombo.addItem ("flex-end",   3);
        alignSelfCombo.addItem ("center",     4);
        alignSelfCombo.addItem ("stretch",    5);

        alignSelfCombo.setBounds (x, y, 90, 18);
        alignSelfCombo.onChange = [this] { updateAssignSelf(); };
        alignSelfCombo.setSelectedId (5);
        alignSelfCombo.setColor (ComboBox::outlineColorId, Colors::transparentBlack);
        addAndMakeVisible (alignSelfCombo);
        addLabel ("align-self", alignSelfCombo);
    }

    z0 setupTextEditor (TextEditor& te, Rectangle<i32> b, StringRef initialText, std::function<z0()> updateFn)
    {
        te.setBounds (b);
        te.setText (initialText);

        te.onTextChange = [this, updateFn]
        {
            updateFn();
            refreshLayout();
        };

        addAndMakeVisible (te);
    }

    z0 addLabel (const Txt& name, Component& target)
    {
        auto label = new Label (name, name);
        label->attachToComponent (&target, true);
        labels.add (label);
        addAndMakeVisible (label);
    }

    z0 updateAssignSelf()
    {
        switch (alignSelfCombo.getSelectedId())
        {
            case 1:  flexItem.alignSelf = FlexItem::AlignSelf::autoAlign; break;
            case 2:  flexItem.alignSelf = FlexItem::AlignSelf::flexStart; break;
            case 3:  flexItem.alignSelf = FlexItem::AlignSelf::flexEnd;   break;
            case 4:  flexItem.alignSelf = FlexItem::AlignSelf::center;    break;
            case 5:  flexItem.alignSelf = FlexItem::AlignSelf::stretch;   break;
            default: break;
        }

        refreshLayout();
    }

    z0 refreshLayout()
    {
        if (auto parent = getParentComponent())
            parent->resized();
    }

    z0 paint (Graphics& g) override
    {
        auto r = getLocalBounds();

        g.setColor (colour);
        g.fillRect (r);

        g.setColor (Colors::black);
        g.drawFittedText ("w: " + Txt (r.getWidth()) + newLine + "h: " + Txt (r.getHeight()),
                          r.reduced (4), Justification::bottomRight, 2);
    }

    z0 lookAndFeelChanged() override
    {
        flexOrderEditor .applyFontToAllText (flexOrderEditor .getFont());
        flexGrowEditor  .applyFontToAllText (flexGrowEditor  .getFont());
        flexShrinkEditor.applyFontToAllText (flexShrinkEditor.getFont());
        flexBasisEditor .applyFontToAllText (flexBasisEditor .getFont());
    }

    FlexItem& flexItem;

    TextEditor flexOrderEditor, flexGrowEditor, flexShrinkEditor, flexBasisEditor;
    ComboBox alignSelfCombo;

    Color colour;
    OwnedArray<Label> labels;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoFlexPanel)

};

//==============================================================================
struct FlexBoxDemo final : public drx::Component
{
    FlexBoxDemo()
    {
        setupPropertiesPanel();
        setupFlexBoxItems();

        setSize (1000, 500);
    }

    z0 resized() override
    {
        flexBox.performLayout (getFlexBoxBounds());
    }

    Rectangle<f32> getFlexBoxBounds() const
    {
        return getLocalBounds().withTrimmedLeft (300)
                               .reduced (10)
                               .toFloat();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Colors::lightgrey));
        g.setColor (Colors::white);
        g.fillRect (getFlexBoxBounds());
    }

    z0 setupPropertiesPanel()
    {
        auto directionGroup = addControl (new GroupComponent ("direction", "flex-direction"));
        directionGroup->setBounds (10, 30, 140, 110);

        i32 i = 0;
        i32 groupID    = 1234;
        i32 leftMargin = 15;
        i32 topMargin  = 45;

        createToggleButton ("row",            groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.flexDirection = FlexBox::Direction::row; }).setToggleState (true, dontSendNotification);
        createToggleButton ("row-reverse",    groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexDirection = FlexBox::Direction::rowReverse; });
        createToggleButton ("column",         groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexDirection = FlexBox::Direction::column; });
        createToggleButton ("column-reverse", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexDirection = FlexBox::Direction::columnReverse; });

        auto wrapGroup = addControl (new GroupComponent ("wrap", "flex-wrap"));
        wrapGroup->setBounds (160, 30, 140, 110);

        i = 0;
        ++groupID;
        leftMargin = 165;

        createToggleButton ("nowrap",       groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexWrap = FlexBox::Wrap::noWrap; });
        createToggleButton ("wrap",         groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.flexWrap = FlexBox::Wrap::wrap; });
        createToggleButton ("wrap-reverse", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexWrap = FlexBox::Wrap::wrapReverse; });

        auto justifyGroup = addControl (new GroupComponent ("justify", "justify-content"));
        justifyGroup->setBounds (10, 150, 140, 140);

        i = 0;
        ++groupID;
        leftMargin = 15;
        topMargin  = 165;

        createToggleButton ("flex-start",    groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.justifyContent = FlexBox::JustifyContent::flexStart; });
        createToggleButton ("flex-end",      groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::flexEnd; });
        createToggleButton ("center",        groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::center; });
        createToggleButton ("space-between", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::spaceBetween; });
        createToggleButton ("space-around",  groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::spaceAround; });

        auto alignGroup = addControl (new GroupComponent ("align", "align-items"));
        alignGroup->setBounds (160, 150, 140, 140);

        i = 0;
        ++groupID;
        leftMargin = 165;
        topMargin  = 165;

        createToggleButton ("stretch",    groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.alignItems = FlexBox::AlignItems::stretch; });
        createToggleButton ("flex-start", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignItems = FlexBox::AlignItems::flexStart; });
        createToggleButton ("flex-end",   groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignItems = FlexBox::AlignItems::flexEnd; });
        createToggleButton ("center",     groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignItems = FlexBox::AlignItems::center; });

        auto alignContentGroup = addControl (new GroupComponent ("content", "align-content"));
        alignContentGroup->setBounds (10, 300, 140, 160);

        i = 0;
        ++groupID;
        leftMargin = 15;
        topMargin  = 315;

        createToggleButton ("stretch",       groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.alignContent = FlexBox::AlignContent::stretch; });
        createToggleButton ("flex-start",    groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::flexStart; });
        createToggleButton ("flex-end",      groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::flexEnd; });
        createToggleButton ("center",        groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::center; });
        createToggleButton ("space-between", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::spaceBetween; });
        createToggleButton ("space-around",  groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::spaceAround; });
    }

    z0 setupFlexBoxItems()
    {
        addItem (Colors::orange);
        addItem (Colors::aqua);
        addItem (Colors::lightcoral);
        addItem (Colors::aquamarine);
        addItem (Colors::forestgreen);
    }

    z0 addItem (Color colour)
    {
        flexBox.items.add (FlexItem (100, 150)
                             .withMargin (10)
                             .withWidth (200));

        auto& flexItem = flexBox.items.getReference (flexBox.items.size() - 1);

        auto panel = panels.add (new DemoFlexPanel (colour, flexItem));
        flexItem.associatedComponent = panel;
        addAndMakeVisible (panel);
    }

    ToggleButton& createToggleButton (StringRef text, i32 groupID, i32 x, i32 y, b8 toggleOn, std::function<z0()> fn)
    {
        auto* tb = buttons.add (new ToggleButton());
        tb->setButtonText (text);
        tb->setRadioGroupId (groupID);
        tb->setToggleState (toggleOn, dontSendNotification);

        tb->onClick = [this, fn]
        {
            fn();
            resized();
        };

        tb->setBounds (x, y, 130, 22);
        addAndMakeVisible (tb);
        return *tb;
    }

    template <typename ComponentType>
    ComponentType* addControl (ComponentType* newControlComp)
    {
        controls.add (newControlComp);
        addAndMakeVisible (newControlComp);
        return newControlComp;
    }

    FlexBox flexBox;

    OwnedArray<DemoFlexPanel> panels;
    OwnedArray<Component> controls;
    OwnedArray<ToggleButton> buttons;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlexBoxDemo)
};
