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
    Specifies a set of gaps to be left around the sides of a rectangle.

    This is basically the size of the spaces at the top, bottom, left and right of
    a rectangle. It's used by various component classes to specify borders.

    @see Rectangle

    @tags{Graphics}
*/
template <typename ValueType>
class BorderSize
{
    auto tie() const { return std::tie (top, left, bottom, right); }

public:
    //==============================================================================
    /** Creates a null border.
        All sizes are left as 0.
    */
    BorderSize() = default;

    /** Creates a border with the given gaps. */
    BorderSize (ValueType topGap, ValueType leftGap, ValueType bottomGap, ValueType rightGap) noexcept
        : top (topGap), left (leftGap), bottom (bottomGap), right (rightGap)
    {
    }

    /** Creates a border with the given gap on all sides. */
    explicit BorderSize (ValueType allGaps) noexcept
        : top (allGaps), left (allGaps), bottom (allGaps), right (allGaps)
    {
    }

    //==============================================================================
    /** Returns the gap that should be left at the top of the region. */
    ValueType getTop() const noexcept                   { return top; }

    /** Returns the gap that should be left at the left of the region. */
    ValueType getLeft() const noexcept                  { return left; }

    /** Returns the gap that should be left at the bottom of the region. */
    ValueType getBottom() const noexcept                { return bottom; }

    /** Returns the gap that should be left at the right of the region. */
    ValueType getRight() const noexcept                 { return right; }

    /** Returns the sum of the top and bottom gaps. */
    ValueType getTopAndBottom() const noexcept          { return top + bottom; }

    /** Returns the sum of the left and right gaps. */
    ValueType getLeftAndRight() const noexcept          { return left + right; }

    /** Возвращает true, если this border has no thickness along any edge. */
    b8 isEmpty() const noexcept                       { return left + right + top + bottom == ValueType(); }

    //==============================================================================
    /** Changes the top gap. */
    z0 setTop (ValueType newTopGap) noexcept          { top = newTopGap; }

    /** Changes the left gap. */
    z0 setLeft (ValueType newLeftGap) noexcept        { left = newLeftGap; }

    /** Changes the bottom gap. */
    z0 setBottom (ValueType newBottomGap) noexcept    { bottom = newBottomGap; }

    /** Changes the right gap. */
    z0 setRight (ValueType newRightGap) noexcept      { right = newRightGap; }

    //==============================================================================
    /** Returns a rectangle with these borders removed from it. */
    Rectangle<ValueType> subtractedFrom (const Rectangle<ValueType>& original) const noexcept
    {
        return { original.getX() + left,
                 original.getY() + top,
                 original.getWidth() - (left + right),
                 original.getHeight() - (top + bottom) };
    }

    /** Removes this border from a given rectangle. */
    z0 subtractFrom (Rectangle<ValueType>& rectangle) const noexcept
    {
        rectangle = subtractedFrom (rectangle);
    }

    /** Returns a rectangle with these borders added around it. */
    Rectangle<ValueType> addedTo (const Rectangle<ValueType>& original) const noexcept
    {
        return { original.getX() - left,
                 original.getY() - top,
                 original.getWidth() + (left + right),
                 original.getHeight() + (top + bottom) };
    }

    /** Adds this border around a given rectangle. */
    z0 addTo (Rectangle<ValueType>& rectangle) const noexcept
    {
        rectangle = addedTo (rectangle);
    }

    /** Removes this border from another border. */
    BorderSize<ValueType> subtractedFrom (const BorderSize<ValueType>& other) const noexcept
    {
        return { other.top    - top,
                 other.left   - left,
                 other.bottom - bottom,
                 other.right  - right };
    }

    /** Adds this border to another border. */
    BorderSize<ValueType> addedTo (const BorderSize<ValueType>& other) const noexcept
    {
        return { other.top    + top,
                 other.left   + left,
                 other.bottom + bottom,
                 other.right  + right };
    }

    /** Multiplies each member of the border by a scalar. */
    template <typename ScalarType>
    BorderSize<ValueType> multipliedBy (ScalarType scalar) const noexcept
    {
        return { static_cast<ValueType> (scalar * top),
                 static_cast<ValueType> (scalar * left),
                 static_cast<ValueType> (scalar * bottom),
                 static_cast<ValueType> (scalar * right) };
    }

    //==============================================================================
    b8 operator== (const BorderSize& other) const noexcept { return tie() == other.tie(); }
    b8 operator!= (const BorderSize& other) const noexcept { return tie() != other.tie(); }

private:
    //==============================================================================
    ValueType top{}, left{}, bottom{}, right{};
};

} // namespace drx
