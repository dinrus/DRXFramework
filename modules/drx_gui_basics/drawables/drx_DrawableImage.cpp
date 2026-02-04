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

DrawableImage::DrawableImage() : bounds ({ 0.0f, 0.0f, 1.0f, 1.0f })
{
}

DrawableImage::DrawableImage (const DrawableImage& other)
    : Drawable (other),
      image (other.image),
      opacity (other.opacity),
      overlayColor (other.overlayColor),
      bounds (other.bounds)
{
    setBounds (other.getBounds());
}

DrawableImage::DrawableImage (const Image& imageToUse)
{
    setImageInternal (imageToUse);
}

DrawableImage::~DrawableImage()
{
}

std::unique_ptr<Drawable> DrawableImage::createCopy() const
{
    return std::make_unique<DrawableImage> (*this);
}

//==============================================================================
z0 DrawableImage::setImage (const Image& imageToUse)
{
    if (setImageInternal (imageToUse))
        repaint();
}

z0 DrawableImage::setOpacity (const f32 newOpacity)
{
    opacity = newOpacity;
}

z0 DrawableImage::setOverlayColor (Color newOverlayColor)
{
    overlayColor = newOverlayColor;
}

z0 DrawableImage::setBoundingBox (Rectangle<f32> newBounds)
{
    setBoundingBox (Parallelogram<f32> (newBounds));
}

z0 DrawableImage::setBoundingBox (Parallelogram<f32> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;

        if (image.isValid())
        {
            auto tr = bounds.topLeft + (bounds.topRight   - bounds.topLeft) / (f32) image.getWidth();
            auto bl = bounds.topLeft + (bounds.bottomLeft - bounds.topLeft) / (f32) image.getHeight();

            auto t = AffineTransform::fromTargetPoints (bounds.topLeft.x, bounds.topLeft.y,
                                                        tr.x, tr.y,
                                                        bl.x, bl.y);

            if (t.isSingularity())
                t = {};

            setTransform (t);
        }
    }
}

//==============================================================================
z0 DrawableImage::paint (Graphics& g)
{
    if (image.isValid())
    {
        if (opacity > 0.0f && ! overlayColor.isOpaque())
        {
            g.setOpacity (opacity);
            g.drawImageAt (image, 0, 0, false);
        }

        if (! overlayColor.isTransparent())
        {
            g.setColor (overlayColor.withMultipliedAlpha (opacity));
            g.drawImageAt (image, 0, 0, true);
        }
    }
}

Rectangle<f32> DrawableImage::getDrawableBounds() const
{
    return image.getBounds().toFloat();
}

b8 DrawableImage::hitTest (i32 x, i32 y)
{
    return Drawable::hitTest (x, y) && image.isValid() && image.getPixelAt (x, y).getAlpha() >= 127;
}

Path DrawableImage::getOutlineAsPath() const
{
    return {}; // not applicable for images
}

//==============================================================================
b8 DrawableImage::setImageInternal (const Image& imageToUse)
{
    if (image != imageToUse)
    {
        image = imageToUse;
        setBounds (image.getBounds());
        setBoundingBox (image.getBounds().toFloat());
        return true;
    }

    return false;
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> DrawableImage::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);
}

} // namespace drx
