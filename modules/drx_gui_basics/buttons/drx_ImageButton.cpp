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

ImageButton::ImageButton (const Txt& text_)
    : Button (text_),
      scaleImageToFit (true),
      preserveProportions (true),
      alphaThreshold (0)
{
}

ImageButton::~ImageButton()
{
}

z0 ImageButton::setImages (const b8 resizeButtonNowToFitThisImage,
                             const b8 rescaleImagesWhenButtonSizeChanges,
                             const b8 preserveImageProportions,
                             const Image& normalImage_,
                             const f32 imageOpacityWhenNormal,
                             Color overlayColorWhenNormal,
                             const Image& overImage_,
                             const f32 imageOpacityWhenOver,
                             Color overlayColorWhenOver,
                             const Image& downImage_,
                             const f32 imageOpacityWhenDown,
                             Color overlayColorWhenDown,
                             const f32 hitTestAlphaThreshold)
{
    normalImage = normalImage_;
    overImage = overImage_;
    downImage = downImage_;

    if (resizeButtonNowToFitThisImage && normalImage.isValid())
    {
        imageBounds.setSize (normalImage.getWidth(),
                             normalImage.getHeight());

        setSize (imageBounds.getWidth(), imageBounds.getHeight());
    }

    scaleImageToFit = rescaleImagesWhenButtonSizeChanges;
    preserveProportions = preserveImageProportions;

    normalOpacity = imageOpacityWhenNormal;
    normalOverlay = overlayColorWhenNormal;
    overOpacity   = imageOpacityWhenOver;
    overOverlay   = overlayColorWhenOver;
    downOpacity   = imageOpacityWhenDown;
    downOverlay   = overlayColorWhenDown;

    alphaThreshold = (u8) jlimit (0, 0xff, roundToInt (255.0f * hitTestAlphaThreshold));

    repaint();
}

Image ImageButton::getCurrentImage() const
{
    if (isDown() || getToggleState())
        return getDownImage();

    if (isOver())
        return getOverImage();

    return getNormalImage();
}

Image ImageButton::getNormalImage() const
{
    return normalImage;
}

Image ImageButton::getOverImage() const
{
    return overImage.isValid() ? overImage
                               : normalImage;
}

Image ImageButton::getDownImage() const
{
    return downImage.isValid() ? downImage
                               : getOverImage();
}

z0 ImageButton::paintButton (Graphics& g,
                               b8 shouldDrawButtonAsHighlighted,
                               b8 shouldDrawButtonAsDown)
{
    if (! isEnabled())
    {
        shouldDrawButtonAsHighlighted = false;
        shouldDrawButtonAsDown = false;
    }

    Image im (getCurrentImage());

    if (im.isValid())
    {
        i32k iw = im.getWidth();
        i32k ih = im.getHeight();
        i32 w = getWidth();
        i32 h = getHeight();
        i32 x = (w - iw) / 2;
        i32 y = (h - ih) / 2;

        if (scaleImageToFit)
        {
            if (preserveProportions)
            {
                i32 newW, newH;
                const f32 imRatio = (f32) ih / (f32) iw;
                const f32 destRatio = (f32) h / (f32) w;

                if (imRatio > destRatio)
                {
                    newW = roundToInt ((f32) h / imRatio);
                    newH = h;
                }
                else
                {
                    newW = w;
                    newH = roundToInt ((f32) w * imRatio);
                }

                x = (w - newW) / 2;
                y = (h - newH) / 2;
                w = newW;
                h = newH;
            }
            else
            {
                x = 0;
                y = 0;
            }
        }

        if (! scaleImageToFit)
        {
            w = iw;
            h = ih;
        }

        imageBounds.setBounds (x, y, w, h);

        const b8 useDownImage = shouldDrawButtonAsDown || getToggleState();

        getLookAndFeel().drawImageButton (g, &im, x, y, w, h,
                                          useDownImage ? downOverlay
                                                       : (shouldDrawButtonAsHighlighted ? overOverlay
                                                                            : normalOverlay),
                                          useDownImage ? downOpacity
                                                       : (shouldDrawButtonAsHighlighted ? overOpacity
                                                                            : normalOpacity),
                                          *this);
    }
}

b8 ImageButton::hitTest (i32 x, i32 y)
{
    if (! Component::hitTest (x, y)) // handle setInterceptsMouseClicks
        return false;

    if (alphaThreshold == 0)
        return true;

    Image im (getCurrentImage());

    return im.isNull() || ((! imageBounds.isEmpty())
                            && alphaThreshold < im.getPixelAt (((x - imageBounds.getX()) * im.getWidth()) / imageBounds.getWidth(),
                                                               ((y - imageBounds.getY()) * im.getHeight()) / imageBounds.getHeight()).getAlpha());
}

} // namespace drx
