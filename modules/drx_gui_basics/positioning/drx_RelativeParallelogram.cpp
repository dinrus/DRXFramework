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

RelativeParallelogram::RelativeParallelogram()
{
}

RelativeParallelogram::RelativeParallelogram (const Rectangle<f32>& r)
    : topLeft (r.getTopLeft()), topRight (r.getTopRight()), bottomLeft (r.getBottomLeft())
{
}

RelativeParallelogram::RelativeParallelogram (const RelativePoint& topLeft_, const RelativePoint& topRight_, const RelativePoint& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::RelativeParallelogram (const Txt& topLeft_, const Txt& topRight_, const Txt& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::~RelativeParallelogram()
{
}

z0 RelativeParallelogram::resolveThreePoints (Point<f32>* points, Expression::Scope* const scope) const
{
    points[0] = topLeft.resolve (scope);
    points[1] = topRight.resolve (scope);
    points[2] = bottomLeft.resolve (scope);
}

z0 RelativeParallelogram::resolveFourCorners (Point<f32>* points, Expression::Scope* const scope) const
{
    resolveThreePoints (points, scope);
    points[3] = points[1] + (points[2] - points[0]);
}

const Rectangle<f32> RelativeParallelogram::getBounds (Expression::Scope* const scope) const
{
    Point<f32> points[4];
    resolveFourCorners (points, scope);
    return Rectangle<f32>::findAreaContainingPoints (points, 4);
}

z0 RelativeParallelogram::getPath (Path& path, Expression::Scope* const scope) const
{
    Point<f32> points[4];
    resolveFourCorners (points, scope);

    path.startNewSubPath (points[0]);
    path.lineTo (points[1]);
    path.lineTo (points[3]);
    path.lineTo (points[2]);
    path.closeSubPath();
}

AffineTransform RelativeParallelogram::resetToPerpendicular (Expression::Scope* const scope)
{
    Point<f32> corners[3];
    resolveThreePoints (corners, scope);

    const Line<f32> top (corners[0], corners[1]);
    const Line<f32> left (corners[0], corners[2]);
    const Point<f32> newTopRight (corners[0] + Point<f32> (top.getLength(), 0.0f));
    const Point<f32> newBottomLeft (corners[0] + Point<f32> (0.0f, left.getLength()));

    topRight.moveToAbsolute (newTopRight, scope);
    bottomLeft.moveToAbsolute (newBottomLeft, scope);

    return AffineTransform::fromTargetPoints (corners[0], corners[0],
                                              corners[1], newTopRight,
                                              corners[2], newBottomLeft);
}

b8 RelativeParallelogram::isDynamic() const
{
    return topLeft.isDynamic() || topRight.isDynamic() || bottomLeft.isDynamic();
}

b8 RelativeParallelogram::operator== (const RelativeParallelogram& other) const noexcept
{
    return topLeft == other.topLeft && topRight == other.topRight && bottomLeft == other.bottomLeft;
}

b8 RelativeParallelogram::operator!= (const RelativeParallelogram& other) const noexcept
{
    return ! operator== (other);
}

Point<f32> RelativeParallelogram::getInternalCoordForPoint (const Point<f32>* const corners, Point<f32> target) noexcept
{
    const Point<f32> tr (corners[1] - corners[0]);
    const Point<f32> bl (corners[2] - corners[0]);
    target -= corners[0];

    return Point<f32> (Line<f32> (Point<f32>(), tr).getIntersection (Line<f32> (target, target - bl)).getDistanceFromOrigin(),
                         Line<f32> (Point<f32>(), bl).getIntersection (Line<f32> (target, target - tr)).getDistanceFromOrigin());
}

Point<f32> RelativeParallelogram::getPointForInternalCoord (const Point<f32>* const corners, const Point<f32> point) noexcept
{
    return corners[0]
            + Line<f32> (Point<f32>(), corners[1] - corners[0]).getPointAlongLine (point.x)
            + Line<f32> (Point<f32>(), corners[2] - corners[0]).getPointAlongLine (point.y);
}

Rectangle<f32> RelativeParallelogram::getBoundingBox (const Point<f32>* const p) noexcept
{
    const Point<f32> points[] = { p[0], p[1], p[2], p[1] + (p[2] - p[0]) };
    return Rectangle<f32>::findAreaContainingPoints (points, 4);
}

} // namespace drx
