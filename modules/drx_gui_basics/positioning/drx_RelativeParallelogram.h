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

//==============================================================================
/**
    A parallelogram defined by three RelativePoint positions.

    @see RelativePoint, RelativeCoordinate

    @tags{GUI}
*/
class DRX_API  RelativeParallelogram
{
public:
    //==============================================================================
    RelativeParallelogram();
    RelativeParallelogram (const Rectangle<f32>& simpleRectangle);
    RelativeParallelogram (const RelativePoint& topLeft, const RelativePoint& topRight, const RelativePoint& bottomLeft);
    RelativeParallelogram (const Txt& topLeft, const Txt& topRight, const Txt& bottomLeft);
    ~RelativeParallelogram();

    //==============================================================================
    z0 resolveThreePoints (Point<f32>* points, Expression::Scope* scope) const;
    z0 resolveFourCorners (Point<f32>* points, Expression::Scope* scope) const;
    const Rectangle<f32> getBounds (Expression::Scope* scope) const;
    z0 getPath (Path& path, Expression::Scope* scope) const;
    AffineTransform resetToPerpendicular (Expression::Scope* scope);
    b8 isDynamic() const;

    b8 operator== (const RelativeParallelogram&) const noexcept;
    b8 operator!= (const RelativeParallelogram&) const noexcept;

    static Point<f32> getInternalCoordForPoint (const Point<f32>* parallelogramCorners, Point<f32> point) noexcept;
    static Point<f32> getPointForInternalCoord (const Point<f32>* parallelogramCorners, Point<f32> internalPoint) noexcept;
    static Rectangle<f32> getBoundingBox (const Point<f32>* parallelogramCorners) noexcept;

    //==============================================================================
    RelativePoint topLeft, topRight, bottomLeft;
};

} // namespace drx
