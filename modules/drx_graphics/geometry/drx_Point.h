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
    A pair of (x, y) coordinates.

    The ValueType template should be a primitive type such as i32, f32, f64,
    rather than a class.

    @see Line, Path, AffineTransform

    @tags{Graphics}
*/
template <typename ValueType>
class Point
{
public:
    /** Creates a point at the origin */
    constexpr Point() = default;

    /** Creates a copy of another point. */
    constexpr Point (const Point&) = default;

    /** Creates a point from an (x, y) position. */
    constexpr Point (ValueType initialX, ValueType initialY) noexcept : x (initialX), y (initialY) {}

    //==============================================================================
    /** Copies this point from another one. */
    Point& operator= (const Point&) = default;

    constexpr inline b8 operator== (Point other) const noexcept
    {
        const auto tie = [] (const Point& p) { return std::tie (p.x, p.y); };
        return tie (*this) == tie (other);
    }

    constexpr inline b8 operator!= (Point other) const noexcept      { return ! operator== (other); }

    /** Возвращает true, если the point is (0, 0). */
    constexpr b8 isOrigin() const noexcept                           { return operator== (Point()); }

    /** Возвращает true, если the coordinates are finite values. */
    constexpr inline b8 isFinite() const noexcept                    { return drx_isfinite (x) && drx_isfinite (y); }

    /** Returns the point's x coordinate. */
    constexpr inline ValueType getX() const noexcept                   { return x; }

    /** Returns the point's y coordinate. */
    constexpr inline ValueType getY() const noexcept                   { return y; }

    /** Sets the point's x coordinate. */
    inline z0 setX (ValueType newX) noexcept                         { x = newX; }

    /** Sets the point's y coordinate. */
    inline z0 setY (ValueType newY) noexcept                         { y = newY; }

    /** Returns a point which has the same Y position as this one, but a new X. */
    constexpr Point withX (ValueType newX) const noexcept              { return Point (newX, y); }

    /** Returns a point which has the same X position as this one, but a new Y. */
    constexpr Point withY (ValueType newY) const noexcept              { return Point (x, newY); }

    /** Changes the point's x and y coordinates. */
    z0 setXY (ValueType newX, ValueType newY) noexcept               { x = newX; y = newY; }

    /** Adds a pair of coordinates to this value. */
    z0 addXY (ValueType xToAdd, ValueType yToAdd) noexcept           { x += xToAdd; y += yToAdd; }

    //==============================================================================
    /** Returns a point with a given offset from this one. */
    constexpr Point translated (ValueType deltaX, ValueType deltaY) const noexcept    { return Point (x + deltaX, y + deltaY); }

    /** Adds two points together */
    constexpr Point operator+ (Point other) const noexcept             { return Point (x + other.x, y + other.y); }

    /** Adds another point's coordinates to this one */
    Point& operator+= (Point other) noexcept                           { x += other.x; y += other.y; return *this; }

    /** Subtracts one points from another */
    constexpr Point operator- (Point other) const noexcept             { return Point (x - other.x, y - other.y); }

    /** Subtracts another point's coordinates to this one */
    Point& operator-= (Point other) noexcept                           { x -= other.x; y -= other.y; return *this; }

    /** Multiplies two points together */
    template <typename OtherType>
    constexpr Point operator* (Point<OtherType> other) const noexcept  { return Point ((ValueType) (x * other.x), (ValueType) (y * other.y)); }

    /** Multiplies another point's coordinates to this one */
    template <typename OtherType>
    Point& operator*= (Point<OtherType> other) noexcept                { *this = *this * other; return *this; }

    /** Divides one point by another */
    template <typename OtherType>
    constexpr Point operator/ (Point<OtherType> other) const noexcept  { return Point ((ValueType) (x / other.x), (ValueType) (y / other.y)); }

    /** Divides this point's coordinates by another */
    template <typename OtherType>
    Point& operator/= (Point<OtherType> other) noexcept                { *this = *this / other; return *this; }

    /** Returns a point whose coordinates are multiplied by a given scalar value. */
    template <typename OtherType>
    constexpr Point operator* (OtherType multiplier) const noexcept
    {
        using CommonType = std::common_type_t<ValueType, OtherType>;
        return Point ((ValueType) ((CommonType) x * (CommonType) multiplier),
                      (ValueType) ((CommonType) y * (CommonType) multiplier));
    }

    /** Returns a point whose coordinates are divided by a given scalar value. */
    template <typename OtherType>
    constexpr Point operator/ (OtherType divisor) const noexcept
    {
        using CommonType = std::common_type_t<ValueType, OtherType>;
        return Point ((ValueType) ((CommonType) x / (CommonType) divisor),
                      (ValueType) ((CommonType) y / (CommonType) divisor));
    }

    /** Multiplies the point's coordinates by a scalar value. */
    template <typename FloatType>
    Point& operator*= (FloatType multiplier) noexcept                  { x = (ValueType) (x * multiplier); y = (ValueType) (y * multiplier); return *this; }

    /** Divides the point's coordinates by a scalar value. */
    template <typename FloatType>
    Point& operator/= (FloatType divisor) noexcept                     { x = (ValueType) (x / divisor); y = (ValueType) (y / divisor); return *this; }

    /** Returns the inverse of this point. */
    constexpr Point operator-() const noexcept                         { return Point (-x, -y); }

    //==============================================================================
    /** This type will be f64 if the Point's type is f64, otherwise it will be f32. */
    using FloatType = TypeHelpers::SmallestFloatType<ValueType>;

    //==============================================================================
    /** Returns the straight-line distance between this point and the origin. */
    ValueType getDistanceFromOrigin() const noexcept                          { return drx_hypot (x, y); }

    /** Returns the straight-line distance between this point and another one. */
    ValueType getDistanceFrom (Point other) const noexcept                    { return drx_hypot (x - other.x, y - other.y); }

    /** Returns the square of the straight-line distance between this point and the origin. */
    constexpr ValueType getDistanceSquaredFromOrigin() const noexcept         { return x * x + y * y; }

    /** Returns the square of the straight-line distance between this point and another one. */
    constexpr ValueType getDistanceSquaredFrom (Point other) const noexcept   { return (*this - other).getDistanceSquaredFromOrigin(); }

    /** Returns the angle from this point to another one.

        Taking this point to be the centre of a circle, and the other point being a position on
        the circumference, the return value is the number of radians clockwise from the 12 o'clock
        direction.
        So 12 o'clock = 0, 3 o'clock = Pi/2, 6 o'clock = Pi, 9 o'clock = -Pi/2
    */
    FloatType getAngleToPoint (Point other) const noexcept
    {
        return static_cast<FloatType> (std::atan2 (static_cast<FloatType> (other.x - x),
                                                   static_cast<FloatType> (y - other.y)));
    }

    /** Returns the point that would be reached by rotating this point clockwise
        about the origin by the specified angle.
    */
    template <typename T = ValueType, std::enable_if_t<std::is_floating_point_v<T>, i32> = 0>
    Point rotatedAboutOrigin (ValueType angleRadians) const noexcept
    {
        return Point (x * std::cos (angleRadians) - y * std::sin (angleRadians),
                      x * std::sin (angleRadians) + y * std::cos (angleRadians));
    }

    /** Taking this point to be the centre of a circle, this returns a point on its circumference.
        @param radius   the radius of the circle.
        @param angle    the angle of the point, in radians clockwise from the 12 o'clock position.
    */
    Point<FloatType> getPointOnCircumference (f32 radius, f32 angle) const noexcept
    {
        return Point<FloatType> (static_cast<FloatType> (x + radius * std::sin (angle)),
                                 static_cast<FloatType> (y - radius * std::cos (angle)));
    }

    /** Taking this point to be the centre of an ellipse, this returns a point on its circumference.
        @param radiusX  the horizontal radius of the circle.
        @param radiusY  the vertical radius of the circle.
        @param angle    the angle of the point, in radians clockwise from the 12 o'clock position.
    */
    Point<FloatType> getPointOnCircumference (f32 radiusX, f32 radiusY, f32 angle) const noexcept
    {
        return Point<FloatType> (static_cast<FloatType> (x + radiusX * std::sin (angle)),
                                 static_cast<FloatType> (y - radiusY * std::cos (angle)));
    }

    /** Returns the dot-product of two points (x1 * x2 + y1 * y2). */
    constexpr FloatType getDotProduct (Point other) const noexcept     { return (FloatType) (x * other.x + y * other.y); }

    //==============================================================================
    /** Uses a transform to change the point's coordinates.
        This will only compile if ValueType = f32!

        @see AffineTransform::transformPoint
    */
    z0 applyTransform (const AffineTransform& transform) noexcept     { transform.transformPoint (x, y); }

    /** Returns the position of this point, if it is transformed by a given AffineTransform. */
    Point transformedBy (const AffineTransform& transform) const noexcept
    {
        return Point (static_cast<ValueType> (transform.mat00 * (f32) x + transform.mat01 * (f32) y + transform.mat02),
                      static_cast<ValueType> (transform.mat10 * (f32) x + transform.mat11 * (f32) y + transform.mat12));
    }

    //==============================================================================
    /** Casts this point to a Point<i32> object. */
    constexpr Point<i32> toInt() const noexcept              { return Point<i32> (static_cast<i32> (x), static_cast<i32> (y)); }

    /** Casts this point to a Point<f32> object. */
    constexpr Point<f32> toFloat() const noexcept          { return Point<f32> (static_cast<f32> (x), static_cast<f32> (y)); }

    /** Casts this point to a Point<f64> object. */
    constexpr Point<f64> toDouble() const noexcept        { return Point<f64> (static_cast<f64> (x), static_cast<f64> (y)); }

    /** Casts this point to a Point<i32> object using roundToInt() to convert the values. */
    constexpr Point<i32> roundToInt() const noexcept         { return Point<i32> (drx::roundToInt (x), drx::roundToInt (y)); }

    /** Returns the point as a string in the form "x, y". */
    Txt toString() const                                       { return Txt (x) + ", " + Txt (y); }

    //==============================================================================
    ValueType x{}; /**< The point's X coordinate. */
    ValueType y{}; /**< The point's Y coordinate. */
};

/** Multiplies the point's coordinates by a scalar value. */
template <typename ValueType>
Point<ValueType> operator* (ValueType value, Point<ValueType> p) noexcept       { return p * value; }

} // namespace drx
