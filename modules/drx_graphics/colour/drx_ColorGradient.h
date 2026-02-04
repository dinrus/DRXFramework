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
    Describes the layout and colours that should be used to paint a colour gradient.

    @see Graphics::setGradientFill

    @tags{Graphics}
*/
class DRX_API  ColorGradient  final
{
public:
    /** Creates an uninitialised gradient.

        If you use this constructor instead of the other one, be sure to set all the
        object's public member variables before using it!
    */
    ColorGradient() noexcept;

    ColorGradient (const ColorGradient&);
    ColorGradient (ColorGradient&&) noexcept;
    ColorGradient& operator= (const ColorGradient&);
    ColorGradient& operator= (ColorGradient&&) noexcept;

    //==============================================================================
    /** Creates a gradient object.

        (x1, y1) is the location to draw with colour1. Likewise (x2, y2) is where
        colour2 should be. In between them there's a gradient.

        If isRadial is true, the colours form a circular gradient with (x1, y1) at
        its centre.

        The alpha transparencies of the colours are used, so note that
        if you blend from transparent to a solid colour, the RGB of the transparent
        colour will become visible in parts of the gradient. e.g. blending
        from Color::transparentBlack to Colors::white will produce a
        muddy grey colour midway, but Color::transparentWhite to Colors::white
        will be white all the way across.

        @see ColorGradient
    */
    ColorGradient (Color colour1, f32 x1, f32 y1,
                    Color colour2, f32 x2, f32 y2,
                    b8 isRadial);

    /** Creates a gradient object.

        point1 is the location to draw with colour1. Likewise point2 is where
        colour2 should be. In between them there's a gradient.

        If isRadial is true, the colours form a circular gradient with point1 at
        its centre.

        The alpha transparencies of the colours are used, so note that
        if you blend from transparent to a solid colour, the RGB of the transparent
        colour will become visible in parts of the gradient. e.g. blending
        from Color::transparentBlack to Colors::white will produce a
        muddy grey colour midway, but Color::transparentWhite to Colors::white
        will be white all the way across.

        @see ColorGradient
    */
    ColorGradient (Color colour1, Point<f32> point1,
                    Color colour2, Point<f32> point2,
                    b8 isRadial);

    //==============================================================================
    /** Creates a vertical linear gradient between two Y coordinates */
    static ColorGradient vertical (Color colour1, f32 y1,
                                    Color colour2, f32 y2);

    /** Creates a horizontal linear gradient between two X coordinates */
    static ColorGradient horizontal (Color colour1, f32 x1,
                                      Color colour2, f32 x2);

    /** Creates a vertical linear gradient from top to bottom in a rectangle */
    template <typename Type>
    static ColorGradient vertical (Color colourTop, Color colourBottom, Rectangle<Type> area)
    {
        return vertical (colourTop, (f32) area.getY(), colourBottom, (f32) area.getBottom());
    }

    /** Creates a horizontal linear gradient from right to left in a rectangle */
    template <typename Type>
    static ColorGradient horizontal (Color colourLeft, Color colourRight, Rectangle<Type> area)
    {
        return horizontal (colourLeft, (f32) area.getX(), colourRight, (f32) area.getRight());
    }

    //==============================================================================
    /** Removes any colours that have been added.

        This will also remove any start and end colours, so the gradient won't work. You'll
        need to add more colours with addColor().
    */
    z0 clearColors();

    /** Adds a colour at a point along the length of the gradient.

        This allows the gradient to go through a spectrum of colours, instead of just a
        start and end colour.

        @param proportionAlongGradient      a value between 0 and 1.0, which is the proportion
                                            of the distance along the line between the two points
                                            at which the colour should occur.
        @param colour                       the colour that should be used at this point
        @returns the index at which the new point was added
    */
    i32 addColor (f64 proportionAlongGradient, Color colour);

    /** Removes one of the colours from the gradient. */
    z0 removeColor (i32 index);

    /** Multiplies the alpha value of all the colours by the given scale factor */
    z0 multiplyOpacity (f32 multiplier) noexcept;

    //==============================================================================
    /** Returns the number of colour-stops that have been added. */
    i32 getNumColors() const noexcept;

    /** Returns the position along the length of the gradient of the colour with this index.

        The index is from 0 to getNumColors() - 1. The return value will be between 0.0 and 1.0
    */
    f64 getColorPosition (i32 index) const noexcept;

    /** Returns the colour that was added with a given index.
        The index is from 0 to getNumColors() - 1.
    */
    Color getColor (i32 index) const noexcept;

    /** Changes the colour at a given index.
        The index is from 0 to getNumColors() - 1.
    */
    z0 setColor (i32 index, Color newColor) noexcept;

    /** Returns the an interpolated colour at any position along the gradient.
        @param position     the position along the gradient, between 0 and 1
    */
    Color getColorAtPosition (f64 position) const noexcept;

    //==============================================================================
    /** Creates a set of interpolated premultiplied ARGB values.
        This will resize the HeapBlock, fill it with the colours, and will return the number of
        colours that it added.
        When calling this, the ColorGradient must have at least 2 colour stops specified.
    */
    i32 createLookupTable (const AffineTransform& transform, HeapBlock<PixelARGB>& resultLookupTable) const;

    /** Creates a set of interpolated premultiplied ARGB values.
        This will fill an array of a user-specified size with the gradient, interpolating to fit.
        The numEntries argument specifies the size of the array, and this size must be greater than zero.
        When calling this, the ColorGradient must have at least 2 colour stops specified.
    */
    z0 createLookupTable (PixelARGB* resultLookupTable, i32 numEntries) const noexcept;

    /** Creates a set of interpolated premultiplied ARGB values.
        This will fill an array of a user-specified size with the gradient, interpolating to fit.
        When calling this, the ColorGradient must have at least 2 colour stops specified.
    */
    template <size_t NumEntries>
    z0 createLookupTable (PixelARGB (&resultLookupTable)[NumEntries]) const noexcept
    {
        static_assert (NumEntries != 0);
        createLookupTable (resultLookupTable, NumEntries);
    }

    /** Возвращает true, если all colours are opaque. */
    b8 isOpaque() const noexcept;

    /** Возвращает true, если all colours are completely transparent. */
    b8 isInvisible() const noexcept;

    //==============================================================================
    Point<f32> point1, point2;

    /** If true, the gradient should be filled circularly, centred around
        point1, with point2 defining a point on the circumference.

        If false, the gradient is linear between the two points.
    */
    b8 isRadial;

    b8 operator== (const ColorGradient&) const noexcept;
    b8 operator!= (const ColorGradient&) const noexcept;

    /** This comparison, and the other ordered comparisons are provided only for compatibility with
        ordered container types like std::set and std::map.
    */
    b8 operator<  (const ColorGradient&) const noexcept;
    b8 operator<= (const ColorGradient&) const noexcept;
    b8 operator>  (const ColorGradient&) const noexcept;
    b8 operator>= (const ColorGradient&) const noexcept;

private:
    //==============================================================================
    struct ColorPoint
    {
        auto tie() const { return std::tuple (position, colour.getPixelARGB().getNativeARGB()); }

        b8 operator== (ColorPoint other) const noexcept { return tie() == other.tie(); }
        b8 operator!= (ColorPoint other) const noexcept { return tie() != other.tie(); }
        b8 operator<  (ColorPoint other) const noexcept { return tie() <  other.tie(); }

        f64 position;
        Color colour;
    };

    struct ColorPointArrayComparisons;

    auto tie() const;

    Array<ColorPoint> colours;

    DRX_LEAK_DETECTOR (ColorGradient)
};

} // namespace drx
