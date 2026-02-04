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

ToolbarButton::ToolbarButton (i32k iid, const Txt& buttonText,
                              std::unique_ptr<Drawable> normalIm,
                              std::unique_ptr<Drawable> toggledOnIm)
   : ToolbarItemComponent (iid, buttonText, true),
     normalImage (std::move (normalIm)),
     toggledOnImage (std::move (toggledOnIm))
{
    jassert (normalImage != nullptr);
}

ToolbarButton::~ToolbarButton()
{
}

//==============================================================================
b8 ToolbarButton::getToolbarItemSizes (i32 toolbarDepth, b8 /*isToolbarVertical*/, i32& preferredSize, i32& minSize, i32& maxSize)
{
    preferredSize = minSize = maxSize = toolbarDepth;
    return true;
}

z0 ToolbarButton::paintButtonArea (Graphics&, i32 /*width*/, i32 /*height*/, b8 /*isMouseOver*/, b8 /*isMouseDown*/)
{
}

z0 ToolbarButton::contentAreaChanged (const Rectangle<i32>&)
{
    buttonStateChanged();
}

z0 ToolbarButton::setCurrentImage (Drawable* const newImage)
{
    if (newImage != currentImage)
    {
        removeChildComponent (currentImage);
        currentImage = newImage;

        if (currentImage != nullptr)
        {
            enablementChanged();
            addAndMakeVisible (currentImage);
            updateDrawable();
        }
    }
}

z0 ToolbarButton::updateDrawable()
{
    if (currentImage != nullptr)
    {
        currentImage->setInterceptsMouseClicks (false, false);
        currentImage->setTransformToFit (getContentArea().toFloat(), RectanglePlacement::centred);
        currentImage->setAlpha (isEnabled() ? 1.0f : 0.5f);
    }
}

z0 ToolbarButton::resized()
{
    ToolbarItemComponent::resized();
    updateDrawable();
}

z0 ToolbarButton::enablementChanged()
{
    ToolbarItemComponent::enablementChanged();
    updateDrawable();
}

Drawable* ToolbarButton::getImageToUse() const
{
    if (getStyle() == Toolbar::textOnly)
        return nullptr;

    if (getToggleState() && toggledOnImage != nullptr)
        return toggledOnImage.get();

    return normalImage.get();
}

z0 ToolbarButton::buttonStateChanged()
{
    setCurrentImage (getImageToUse());
}

} // namespace drx
