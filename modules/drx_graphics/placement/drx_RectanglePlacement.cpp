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

b8 RectanglePlacement::operator== (const RectanglePlacement& other) const noexcept
{
    return flags == other.flags;
}

b8 RectanglePlacement::operator!= (const RectanglePlacement& other) const noexcept
{
    return flags != other.flags;
}

z0 RectanglePlacement::applyTo (f64& x, f64& y, f64& w, f64& h,
                                  const f64 dx, const f64 dy, const f64 dw, const f64 dh) const noexcept
{
    if (approximatelyEqual (w, 0.0) || approximatelyEqual (h, 0.0))
        return;

    if ((flags & stretchToFit) != 0)
    {
        x = dx;
        y = dy;
        w = dw;
        h = dh;
    }
    else
    {
        f64 scale = (flags & fillDestination) != 0 ? jmax (dw / w, dh / h)
                                                      : jmin (dw / w, dh / h);

        if ((flags & onlyReduceInSize) != 0)
            scale = jmin (scale, 1.0);

        if ((flags & onlyIncreaseInSize) != 0)
            scale = jmax (scale, 1.0);

        w *= scale;
        h *= scale;

        if ((flags & xLeft) != 0)
            x = dx;
        else if ((flags & xRight) != 0)
            x = dx + dw - w;
        else
            x = dx + (dw - w) * 0.5;

        if ((flags & yTop) != 0)
            y = dy;
        else if ((flags & yBottom) != 0)
            y = dy + dh - h;
        else
            y = dy + (dh - h) * 0.5;
    }
}

AffineTransform RectanglePlacement::getTransformToFit (const Rectangle<f32>& source, const Rectangle<f32>& destination) const noexcept
{
    if (source.isEmpty())
        return AffineTransform();

    f32 newX = destination.getX();
    f32 newY = destination.getY();

    f32 scaleX = destination.getWidth() / source.getWidth();
    f32 scaleY = destination.getHeight() / source.getHeight();

    if ((flags & stretchToFit) == 0)
    {
        scaleX = (flags & fillDestination) != 0 ? jmax (scaleX, scaleY)
                                                : jmin (scaleX, scaleY);

        if ((flags & onlyReduceInSize) != 0)
            scaleX = jmin (scaleX, 1.0f);

        if ((flags & onlyIncreaseInSize) != 0)
            scaleX = jmax (scaleX, 1.0f);

        scaleY = scaleX;

        if ((flags & xRight) != 0)
            newX += destination.getWidth() - source.getWidth() * scaleX;             // right
        else if ((flags & xLeft) == 0)
            newX += (destination.getWidth() - source.getWidth() * scaleX) / 2.0f;    // centre

        if ((flags & yBottom) != 0)
            newY += destination.getHeight() - source.getHeight() * scaleX;             // bottom
        else if ((flags & yTop) == 0)
            newY += (destination.getHeight() - source.getHeight() * scaleX) / 2.0f;    // centre
    }

    return AffineTransform::translation (-source.getX(), -source.getY())
                .scaled (scaleX, scaleY)
                .translated (newX, newY);
}

} // namespace drx
