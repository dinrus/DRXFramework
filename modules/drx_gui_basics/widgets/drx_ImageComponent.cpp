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

ImageComponent::ImageComponent (const Txt& name)
    : Component (name),
      placement (RectanglePlacement::centred)
{
}

ImageComponent::~ImageComponent()
{
}

z0 ImageComponent::setImage (const Image& newImage)
{
    if (image != newImage)
    {
        image = newImage;
        repaint();
    }
}

z0 ImageComponent::setImage (const Image& newImage, RectanglePlacement placementToUse)
{
    if (image != newImage || placement != placementToUse)
    {
        image = newImage;
        placement = placementToUse;
        repaint();
    }
}

z0 ImageComponent::setImagePlacement (RectanglePlacement newPlacement)
{
    if (placement != newPlacement)
    {
        placement = newPlacement;
        repaint();
    }
}

const Image& ImageComponent::getImage() const
{
    return image;
}

RectanglePlacement ImageComponent::getImagePlacement() const
{
    return placement;
}

z0 ImageComponent::paint (Graphics& g)
{
    g.setOpacity (1.0f);
    g.drawImage (image, getLocalBounds().toFloat(), placement);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ImageComponent::createAccessibilityHandler()
{
    class ImageComponentAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        explicit ImageComponentAccessibilityHandler (ImageComponent& imageComponentToWrap)
            : AccessibilityHandler (imageComponentToWrap, AccessibilityRole::image),
              imageComponent (imageComponentToWrap)
        {
        }

        Txt getHelp() const override   { return imageComponent.getTooltip(); }

    private:
        ImageComponent& imageComponent;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComponentAccessibilityHandler)
    };

    return std::make_unique<ImageComponentAccessibilityHandler> (*this);
}

} // namespace drx
