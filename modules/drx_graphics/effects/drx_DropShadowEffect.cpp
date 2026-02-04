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

DropShadow::DropShadow (Color shadowColor, i32k r, Point<i32> o) noexcept
    : colour (shadowColor), radius (r), offset (o)
{
    jassert (radius > 0);
}

z0 DropShadow::drawForImage (Graphics& g, const Image& srcImage) const
{
    jassert (radius > 0);

    if (! srcImage.isValid())
        return;

    auto blurred = srcImage.convertedToFormat (Image::SingleChannel);
    blurred.setBackupEnabled (false);

    blurred.getPixelData()->applySingleChannelBoxBlurEffect (radius);

    g.setColor (colour);
    g.drawImageAt (blurred, offset.x, offset.y, true);
}

z0 DropShadow::drawForPath (Graphics& g, const Path& path) const
{
    jassert (radius > 0);

    auto area = (path.getBounds().getSmallestIntegerContainer() + offset)
            .expanded (radius + 1)
            .getIntersection (g.getClipBounds().expanded (radius + 1));

    if (area.getWidth() <= 2 || area.getHeight() <= 2)
        return;

    Image pathImage { Image::SingleChannel, area.getWidth(), area.getHeight(), true };
    pathImage.setBackupEnabled (false);

    {
        Graphics g2 (pathImage);
        g2.setColor (Colors::white);
        g2.fillPath (path, AffineTransform::translation ((f32) (offset.x - area.getX()),
                                                         (f32) (offset.y - area.getY())));
    }

    pathImage.getPixelData()->applySingleChannelBoxBlurEffect (radius);

    g.setColor (colour);
    g.drawImageAt (pathImage, area.getX(), area.getY(), true);
}

static z0 drawShadowSection (Graphics& g, ColorGradient& cg, Rectangle<f32> area,
                               b8 isCorner, f32 centreX, f32 centreY, f32 edgeX, f32 edgeY)
{
    cg.point1 = area.getRelativePoint (centreX, centreY);
    cg.point2 = area.getRelativePoint (edgeX, edgeY);
    cg.isRadial = isCorner;

    g.setGradientFill (cg);
    g.fillRect (area);
}

z0 DropShadow::drawForRectangle (Graphics& g, const Rectangle<i32>& targetArea) const
{
    ColorGradient cg (colour, 0, 0, colour.withAlpha (0.0f), 0, 0, false);

    for (f32 i = 0.05f; i < 1.0f; i += 0.1f)
        cg.addColor (1.0 - i, colour.withMultipliedAlpha (i * i));

    const f32 radiusInset = (f32) radius / 2.0f;
    const f32 expandedRadius = (f32) radius + radiusInset;

    auto area = targetArea.toFloat().reduced (radiusInset) + offset.toFloat();

    auto r = area.expanded (expandedRadius);
    auto top = r.removeFromTop (expandedRadius);
    auto bottom = r.removeFromBottom (expandedRadius);

    drawShadowSection (g, cg, top.removeFromLeft  (expandedRadius), true, 1.0f, 1.0f, 0, 1.0f);
    drawShadowSection (g, cg, top.removeFromRight (expandedRadius), true, 0, 1.0f, 1.0f, 1.0f);
    drawShadowSection (g, cg, top, false, 0, 1.0f, 0, 0);

    drawShadowSection (g, cg, bottom.removeFromLeft  (expandedRadius), true, 1.0f, 0, 0, 0);
    drawShadowSection (g, cg, bottom.removeFromRight (expandedRadius), true, 0, 0, 1.0f, 0);
    drawShadowSection (g, cg, bottom, false, 0, 0, 0, 1.0f);

    drawShadowSection (g, cg, r.removeFromLeft  (expandedRadius), false, 1.0f, 0, 0, 0);
    drawShadowSection (g, cg, r.removeFromRight (expandedRadius), false, 0, 0, 1.0f, 0);

    g.setColor (colour);
    g.fillRect (area);
}

//==============================================================================
DropShadowEffect::DropShadowEffect() = default;
DropShadowEffect::~DropShadowEffect() = default;

z0 DropShadowEffect::setShadowProperties (const DropShadow& newShadow)
{
    shadow = newShadow;
}

z0 DropShadowEffect::applyEffect (Image& image, Graphics& g, f32 scaleFactor, f32 alpha)
{
    DropShadow s (shadow);
    s.radius = roundToInt ((f32) s.radius * scaleFactor);
    s.colour = s.colour.withMultipliedAlpha (alpha);
    s.offset.x = roundToInt ((f32) s.offset.x * scaleFactor);
    s.offset.y = roundToInt ((f32) s.offset.y * scaleFactor);

    s.drawForImage (g, image);

    g.setOpacity (alpha);
    g.drawImageAt (image, 0, 0);
}

} // namespace drx
