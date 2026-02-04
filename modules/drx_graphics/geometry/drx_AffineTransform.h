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
    Represents a 2D affine-transformation matrix.

    An affine transformation is a transformation such as a rotation, scale, shear,
    resize or translation.

    These are used for various 2D transformation tasks, e.g. with Path objects.

    @see Path, Point, Line

    @tags{Graphics}
*/
class DRX_API  AffineTransform  final
{
public:
    //==============================================================================
    /** Creates an identity transform. */
    AffineTransform() = default;

    /** Creates a copy of another transform. */
    AffineTransform (const AffineTransform&) = default;

    /** Creates a transform from a set of raw matrix values.

        The resulting matrix is:

            (mat00 mat01 mat02)
            (mat10 mat11 mat12)
            (  0     0     1  )
    */
    AffineTransform (f32 mat00, f32 mat01, f32 mat02,
                     f32 mat10, f32 mat11, f32 mat12) noexcept;

    /** Copies from another AffineTransform object */
    AffineTransform& operator= (const AffineTransform&) = default;

    /** Compares two transforms. */
    b8 operator== (const AffineTransform& other) const noexcept;

    /** Compares two transforms. */
    b8 operator!= (const AffineTransform& other) const noexcept;

    //==============================================================================
    /** Transforms a 2D coordinate using this matrix. */
    template <typename ValueType>
    z0 transformPoint (ValueType& x, ValueType& y) const noexcept
    {
        auto oldX = x;
        x = static_cast<ValueType> (mat00 * oldX + mat01 * y + mat02);
        y = static_cast<ValueType> (mat10 * oldX + mat11 * y + mat12);
    }

    /** Transforms two 2D coordinates using this matrix.
        This is just a shortcut for calling transformPoint() on each of these pairs of
        coordinates in turn. (And putting all the calculations into one function hopefully
        also gives the compiler a bit more scope for pipelining it).
    */
    template <typename ValueType>
    z0 transformPoints (ValueType& x1, ValueType& y1,
                          ValueType& x2, ValueType& y2) const noexcept
    {
        auto oldX1 = x1, oldX2 = x2;
        x1 = static_cast<ValueType> (mat00 * oldX1 + mat01 * y1 + mat02);
        y1 = static_cast<ValueType> (mat10 * oldX1 + mat11 * y1 + mat12);
        x2 = static_cast<ValueType> (mat00 * oldX2 + mat01 * y2 + mat02);
        y2 = static_cast<ValueType> (mat10 * oldX2 + mat11 * y2 + mat12);
    }

    /** Transforms three 2D coordinates using this matrix.
        This is just a shortcut for calling transformPoint() on each of these pairs of
        coordinates in turn. (And putting all the calculations into one function hopefully
        also gives the compiler a bit more scope for pipelining it).
    */
    template <typename ValueType>
    z0 transformPoints (ValueType& x1, ValueType& y1,
                          ValueType& x2, ValueType& y2,
                          ValueType& x3, ValueType& y3) const noexcept
    {
        auto oldX1 = x1, oldX2 = x2, oldX3 = x3;
        x1 = static_cast<ValueType> (mat00 * oldX1 + mat01 * y1 + mat02);
        y1 = static_cast<ValueType> (mat10 * oldX1 + mat11 * y1 + mat12);
        x2 = static_cast<ValueType> (mat00 * oldX2 + mat01 * y2 + mat02);
        y2 = static_cast<ValueType> (mat10 * oldX2 + mat11 * y2 + mat12);
        x3 = static_cast<ValueType> (mat00 * oldX3 + mat01 * y3 + mat02);
        y3 = static_cast<ValueType> (mat10 * oldX3 + mat11 * y3 + mat12);
    }

    //==============================================================================
    /** Returns a new transform which is the same as this one followed by a translation. */
    AffineTransform translated (f32 deltaX,
                                f32 deltaY) const noexcept;

    /** Returns a new transform which is the same as this one followed by a translation. */
    template <typename PointType>
    AffineTransform translated (PointType delta) const noexcept
    {
        return translated ((f32) delta.x, (f32) delta.y);
    }

    /** Returns a new transform which is a translation. */
    static AffineTransform translation (f32 deltaX,
                                        f32 deltaY) noexcept;

    /** Returns a new transform which is a translation. */
    template <typename PointType>
    static AffineTransform translation (PointType delta) noexcept
    {
        return translation ((f32) delta.x, (f32) delta.y);
    }

    /** Returns a copy of this transform with the specified translation matrix values. */
    AffineTransform withAbsoluteTranslation (f32 translationX,
                                             f32 translationY) const noexcept;

    /** Returns a transform which is the same as this one followed by a rotation.

        The rotation is specified by a number of radians to rotate clockwise, centred around
        the origin (0, 0).
    */
    AffineTransform rotated (f32 angleInRadians) const noexcept;

    /** Returns a transform which is the same as this one followed by a rotation about a given point.

        The rotation is specified by a number of radians to rotate clockwise, centred around
        the coordinates passed in.
    */
    AffineTransform rotated (f32 angleInRadians,
                             f32 pivotX,
                             f32 pivotY) const noexcept;

    /** Returns a new transform which is a rotation about (0, 0). */
    static AffineTransform rotation (f32 angleInRadians) noexcept;

    /** Returns a new transform which is a rotation about a given point. */
    static AffineTransform rotation (f32 angleInRadians,
                                     f32 pivotX,
                                     f32 pivotY) noexcept;

    /** Returns a transform which is the same as this one followed by a re-scaling.
        The scaling is centred around the origin (0, 0).
    */
    AffineTransform scaled (f32 factorX,
                            f32 factorY) const noexcept;

    /** Returns a transform which is the same as this one followed by a re-scaling.
        The scaling is centred around the origin (0, 0).
    */
    AffineTransform scaled (f32 factor) const noexcept;

    /** Returns a transform which is the same as this one followed by a re-scaling.
        The scaling is centred around the origin provided.
    */
    AffineTransform scaled (f32 factorX, f32 factorY,
                            f32 pivotX, f32 pivotY) const noexcept;

    /** Returns a new transform which is a re-scale about the origin. */
    static AffineTransform scale (f32 factorX,
                                  f32 factorY) noexcept;

    /** Returns a new transform which is a re-scale about the origin. */
    static AffineTransform scale (f32 factor) noexcept;

    /** Returns a new transform which is a re-scale centred around the point provided. */
    static AffineTransform scale (f32 factorX, f32 factorY,
                                  f32 pivotX, f32 pivotY) noexcept;

    /** Returns a transform which is the same as this one followed by a shear.
        The shear is centred around the origin (0, 0).
    */
    AffineTransform sheared (f32 shearX, f32 shearY) const noexcept;

    /** Returns a shear transform, centred around the origin (0, 0). */
    static AffineTransform shear (f32 shearX, f32 shearY) noexcept;

    /** Returns a transform that will flip coordinates vertically within a window of the given height.
        This is handy for converting between upside-down coordinate systems such as OpenGL or CoreGraphics.
    */
    static AffineTransform verticalFlip (f32 height) noexcept;

    /** Returns a matrix which is the inverse operation of this one.

        Some matrices don't have an inverse - in this case, the method will just return
        an identity transform.
    */
    AffineTransform inverted() const noexcept;

    /** Returns the transform that will map three known points onto three coordinates
        that are supplied.

        This returns the transform that will transform (0, 0) into (x00, y00),
        (1, 0) to (x10, y10), and (0, 1) to (x01, y01).
    */
    static AffineTransform fromTargetPoints (f32 x00, f32 y00,
                                             f32 x10, f32 y10,
                                             f32 x01, f32 y01) noexcept;

    /** Returns the transform that will map three specified points onto three target points. */
    static AffineTransform fromTargetPoints (f32 sourceX1, f32 sourceY1, f32 targetX1, f32 targetY1,
                                             f32 sourceX2, f32 sourceY2, f32 targetX2, f32 targetY2,
                                             f32 sourceX3, f32 sourceY3, f32 targetX3, f32 targetY3) noexcept;

    /** Returns the transform that will map three specified points onto three target points. */
    template <typename PointType>
    static AffineTransform fromTargetPoints (PointType source1, PointType target1,
                                             PointType source2, PointType target2,
                                             PointType source3, PointType target3) noexcept
    {
        return fromTargetPoints (source1.x, source1.y, target1.x, target1.y,
                                 source2.x, source2.y, target2.x, target2.y,
                                 source3.x, source3.y, target3.x, target3.y);
    }

    //==============================================================================
    /** Returns the result of concatenating another transformation after this one. */
    AffineTransform followedBy (const AffineTransform& other) const noexcept;

    /** Возвращает true, если this transform has no effect on points. */
    b8 isIdentity() const noexcept;

    /** Возвращает true, если this transform maps to a singularity - i.e. if it has no inverse. */
    b8 isSingularity() const noexcept;

    /** Возвращает true, если the transform only translates, and doesn't scale or rotate the points. */
    b8 isOnlyTranslation() const noexcept;

    /** Возвращает true, если the transform only translates and/or scales. */
    b8 isOnlyTranslationOrScale() const noexcept;

    /** If this transform is only a translation, this returns the X offset.
        @see isOnlyTranslation
    */
    f32 getTranslationX() const noexcept                  { return mat02; }

    /** If this transform is only a translation, this returns the X offset.
        @see isOnlyTranslation
    */
    f32 getTranslationY() const noexcept                  { return mat12; }

    /** Returns the determinant of the transform. */
    f32 getDeterminant() const noexcept;

    //==============================================================================
   #ifndef DOXYGEN
    /** This method has been deprecated.

        You can calculate the scale factor using:
        @code
        std::sqrt (std::abs (AffineTransform::getDeterminant()))
        @endcode

        This method produces incorrect values for transforms containing rotations.

        Returns the approximate scale factor by which lengths will be transformed.
        Obviously a length may be scaled by entirely different amounts depending on its
        direction, so this is only appropriate as a rough guide.
    */
    [[deprecated ("This method produces incorrect values for transforms containing rotations. "
                 "See the method docs for a code example on how to calculate the correct scale factor.")]]
    f32 getScaleFactor() const noexcept;

    [[deprecated ("If you need an identity transform, just use AffineTransform() or {}.")]]
    static const AffineTransform identity;
   #endif

    //==============================================================================
    /* The transform matrix is:

        (mat00 mat01 mat02)
        (mat10 mat11 mat12)
        (  0     0     1  )
    */
    f32 mat00 { 1.0f }, mat01 { 0.0f }, mat02 { 0.0f };
    f32 mat10 { 0.0f }, mat11 { 1.0f }, mat12 { 0.0f };
};

} // namespace drx
