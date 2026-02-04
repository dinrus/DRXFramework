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

namespace drx
{

ToolbarItemPalette::ToolbarItemPalette (ToolbarItemFactory& tbf, Toolbar& bar)
    : factory (tbf), toolbar (bar)
{
    auto* itemHolder = new Component();
    viewport.setViewedComponent (itemHolder);

    Array<i32> allIds;
    factory.getAllToolbarItemIds (allIds);

    for (auto& i : allIds)
        addComponent (i, -1);

    addAndMakeVisible (viewport);
}

ToolbarItemPalette::~ToolbarItemPalette()
{
}

//==============================================================================
z0 ToolbarItemPalette::addComponent (i32k itemId, i32k index)
{
    if (auto* tc = Toolbar::createItem (factory, itemId))
    {
        items.insert (index, tc);
        viewport.getViewedComponent()->addAndMakeVisible (tc, index);
        tc->setEditingMode (ToolbarItemComponent::editableOnPalette);
    }
    else
    {
        jassertfalse;
    }
}

z0 ToolbarItemPalette::replaceComponent (ToolbarItemComponent& comp)
{
    auto index = items.indexOf (&comp);
    jassert (index >= 0);
    items.removeObject (&comp, false);

    addComponent (comp.getItemId(), index);
    resized();
}

z0 ToolbarItemPalette::resized()
{
    viewport.setBoundsInset (BorderSize<i32> (1));

    auto* itemHolder = viewport.getViewedComponent();

    i32k indent = 8;
    i32k preferredWidth = viewport.getWidth() - viewport.getScrollBarThickness() - indent;
    i32k height = toolbar.getThickness();
    auto x = indent;
    auto y = indent;
    i32 maxX = 0;

    for (auto* tc : items)
    {
        tc->setStyle (toolbar.getStyle());

        i32 preferredSize = 1, minSize = 1, maxSize = 1;

        if (tc->getToolbarItemSizes (height, false, preferredSize, minSize, maxSize))
        {
            if (x + preferredSize > preferredWidth && x > indent)
            {
                x = indent;
                y += height;
            }

            tc->setBounds (x, y, preferredSize, height);

            x += preferredSize + 8;
            maxX = jmax (maxX, x);
        }
    }

    itemHolder->setSize (maxX, y + height + 8);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ToolbarItemPalette::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
