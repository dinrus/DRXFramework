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

DrawableRectangle::DrawableRectangle() {}
DrawableRectangle::~DrawableRectangle() {}

DrawableRectangle::DrawableRectangle (const DrawableRectangle& other)
    : DrawableShape (other),
      bounds (other.bounds),
      cornerSize (other.cornerSize)
{
    rebuildPath();
}

std::unique_ptr<Drawable> DrawableRectangle::createCopy() const
{
    return std::make_unique<DrawableRectangle> (*this);
}

//==============================================================================
z0 DrawableRectangle::setRectangle (Parallelogram<f32> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        rebuildPath();
    }
}

z0 DrawableRectangle::setCornerSize (Point<f32> newSize)
{
    if (cornerSize != newSize)
    {
        cornerSize = newSize;
        rebuildPath();
    }
}

z0 DrawableRectangle::rebuildPath()
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    Path newPath;

    if (cornerSize.x > 0 && cornerSize.y > 0)
        newPath.addRoundedRectangle (0, 0, w, h, cornerSize.x, cornerSize.y);
    else
        newPath.addRectangle (0, 0, w, h);

    newPath.applyTransform (AffineTransform::fromTargetPoints (Point<f32>(),       bounds.topLeft,
                                                               Point<f32> (w, 0),  bounds.topRight,
                                                               Point<f32> (0, h),  bounds.bottomLeft));

    if (path != newPath)
    {
        path.swapWithPath (newPath);
        pathChanged();
    }
}

} // namespace drx
