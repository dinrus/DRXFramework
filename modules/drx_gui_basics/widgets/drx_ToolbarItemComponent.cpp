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

ToolbarItemFactory::ToolbarItemFactory() {}
ToolbarItemFactory::~ToolbarItemFactory() {}

//==============================================================================
ToolbarItemComponent::ToolbarItemComponent (i32k itemId_,
                                            const Txt& labelText,
                                            const b8 isBeingUsedAsAButton_)
    : Button (labelText),
      itemId (itemId_),
      mode (normalMode),
      toolbarStyle (Toolbar::iconsOnly),
      dragOffsetX (0),
      dragOffsetY (0),
      isActive (true),
      isBeingDragged (false),
      isBeingUsedAsAButton (isBeingUsedAsAButton_)
{
    // Your item ID can't be 0!
    jassert (itemId_ != 0);
}

ToolbarItemComponent::~ToolbarItemComponent()
{
    overlayComp.reset();
}

Toolbar* ToolbarItemComponent::getToolbar() const
{
    return dynamic_cast<Toolbar*> (getParentComponent());
}

b8 ToolbarItemComponent::isToolbarVertical() const
{
    const Toolbar* const t = getToolbar();
    return t != nullptr && t->isVertical();
}

z0 ToolbarItemComponent::setStyle (const Toolbar::ToolbarItemStyle& newStyle)
{
    if (toolbarStyle != newStyle)
    {
        toolbarStyle = newStyle;
        repaint();
        resized();
    }
}

z0 ToolbarItemComponent::paintButton (Graphics& g, const b8 over, const b8 down)
{
    if (isBeingUsedAsAButton)
        getLookAndFeel().paintToolbarButtonBackground (g, getWidth(), getHeight(),
                                                       over, down, *this);

    if (toolbarStyle != Toolbar::iconsOnly)
    {
        auto indent = contentArea.getX();
        auto y = indent;
        auto h = getHeight() - indent * 2;

        if (toolbarStyle == Toolbar::iconsWithText)
        {
            y = contentArea.getBottom() + indent / 2;
            h -= contentArea.getHeight();
        }

        getLookAndFeel().paintToolbarButtonLabel (g, indent, y, getWidth() - indent * 2, h,
                                                  getButtonText(), *this);
    }

    if (! contentArea.isEmpty())
    {
        Graphics::ScopedSaveState ss (g);

        g.reduceClipRegion (contentArea);
        g.setOrigin (contentArea.getPosition());

        paintButtonArea (g, contentArea.getWidth(), contentArea.getHeight(), over, down);
    }
}

z0 ToolbarItemComponent::resized()
{
    if (toolbarStyle != Toolbar::textOnly)
    {
        i32k indent = jmin (proportionOfWidth (0.08f),
                                 proportionOfHeight (0.08f));

        contentArea = Rectangle<i32> (indent, indent,
                                      getWidth() - indent * 2,
                                      toolbarStyle == Toolbar::iconsWithText ? proportionOfHeight (0.55f)
                                                                             : (getHeight() - indent * 2));
    }
    else
    {
        contentArea = {};
    }

    contentAreaChanged (contentArea);
}

z0 ToolbarItemComponent::setEditingMode (const ToolbarEditingMode newMode)
{
    if (mode != newMode)
    {
        mode = newMode;
        repaint();

        if (mode == normalMode)
        {
            overlayComp.reset();
        }
        else if (overlayComp == nullptr)
        {
            overlayComp.reset (new detail::ToolbarItemDragAndDropOverlayComponent());
            addAndMakeVisible (overlayComp.get());
            overlayComp->parentSizeChanged();
        }

        resized();
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ToolbarItemComponent::createAccessibilityHandler()
{
    const auto shouldItemBeAccessible = (itemId != ToolbarItemFactory::separatorBarId
                                      && itemId != ToolbarItemFactory::spacerId
                                      && itemId != ToolbarItemFactory::flexibleSpacerId);

    if (! shouldItemBeAccessible)
        return createIgnoredAccessibilityHandler (*this);

    return std::make_unique<detail::ButtonAccessibilityHandler> (*this, AccessibilityRole::button);
}

} // namespace drx
