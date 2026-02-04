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

ColorGradient::ColorGradient() noexcept  : isRadial (false)
{
   #if DRX_DEBUG
    point1.setX (987654.0f);
    #define DRX_COLOURGRADIENT_CHECK_COORDS_INITIALISED jassert (! exactlyEqual (point1.x, 987654.0f));
   #else
    #define DRX_COLOURGRADIENT_CHECK_COORDS_INITIALISED
   #endif
}

ColorGradient::ColorGradient (const ColorGradient& other)
    : point1 (other.point1), point2 (other.point2), isRadial (other.isRadial), colours (other.colours)
{}

ColorGradient::ColorGradient (ColorGradient&& other) noexcept
    : point1 (other.point1), point2 (other.point2), isRadial (other.isRadial),
      colours (std::move (other.colours))
{}

ColorGradient& ColorGradient::operator= (const ColorGradient& other)
{
    point1 = other.point1;
    point2 = other.point2;
    isRadial = other.isRadial;
    colours = other.colours;
    return *this;
}

ColorGradient& ColorGradient::operator= (ColorGradient&& other) noexcept
{
    point1 = other.point1;
    point2 = other.point2;
    isRadial = other.isRadial;
    colours = std::move (other.colours);
    return *this;
}

ColorGradient::ColorGradient (Color colour1, f32 x1, f32 y1,
                                Color colour2, f32 x2, f32 y2, b8 radial)
    : ColorGradient (colour1, Point<f32> (x1, y1),
                      colour2, Point<f32> (x2, y2), radial)
{
}

ColorGradient::ColorGradient (Color colour1, Point<f32> p1,
                                Color colour2, Point<f32> p2, b8 radial)
    : point1 (p1),
      point2 (p2),
      isRadial (radial)
{
    colours.add (ColorPoint { 0.0, colour1 },
                 ColorPoint { 1.0, colour2 });
}

ColorGradient ColorGradient::vertical (Color c1, f32 y1, Color c2, f32 y2)
{
    return { c1, 0, y1, c2, 0, y2, false };
}

ColorGradient ColorGradient::horizontal (Color c1, f32 x1, Color c2, f32 x2)
{
    return { c1, x1, 0, c2, x2, 0, false };
}

struct PointComparisons
{
    auto tie() const { return std::tie (point->x, point->y); }

    b8 operator== (const PointComparisons& other) const { return tie() == other.tie(); }
    b8 operator!= (const PointComparisons& other) const { return tie() != other.tie(); }
    b8 operator<  (const PointComparisons& other) const { return tie() <  other.tie(); }

    const Point<f32>* point = nullptr;
};

struct ColorGradient::ColorPointArrayComparisons
{
    b8 operator== (const ColorPointArrayComparisons& other) const { return *array == *other.array; }
    b8 operator!= (const ColorPointArrayComparisons& other) const { return *array != *other.array; }

    b8 operator<  (const ColorPointArrayComparisons& other) const
    {
        return std::lexicographical_compare (array->begin(), array->end(), other.array->begin(), other.array->end());
    }

    const Array<ColorGradient::ColorPoint>* array = nullptr;
};

auto ColorGradient::tie() const
{
    return std::tuple (PointComparisons { &point1 },
                       PointComparisons { &point2 },
                       isRadial,
                       ColorPointArrayComparisons { &colours });
}

b8 ColorGradient::operator== (const ColorGradient& other) const noexcept { return tie() == other.tie(); }
b8 ColorGradient::operator!= (const ColorGradient& other) const noexcept { return tie() != other.tie(); }

b8 ColorGradient::operator<  (const ColorGradient& other) const noexcept { return tie() <  other.tie(); }
b8 ColorGradient::operator<= (const ColorGradient& other) const noexcept { return tie() <= other.tie(); }
b8 ColorGradient::operator>  (const ColorGradient& other) const noexcept { return tie() >  other.tie(); }
b8 ColorGradient::operator>= (const ColorGradient& other) const noexcept { return tie() >= other.tie(); }

//==============================================================================
z0 ColorGradient::clearColors()
{
    colours.clear();
}

i32 ColorGradient::addColor (const f64 proportionAlongGradient, Color colour)
{
    // must be within the two end-points
    jassert (proportionAlongGradient >= 0 && proportionAlongGradient <= 1.0);

    if (proportionAlongGradient <= 0)
    {
        colours.set (0, { 0.0, colour });
        return 0;
    }

    auto pos = jmin (1.0, proportionAlongGradient);

    i32 i;
    for (i = 0; i < colours.size(); ++i)
        if (colours.getReference (i).position > pos)
            break;

    colours.insert (i, { pos, colour });
    return i;
}

z0 ColorGradient::removeColor (i32 index)
{
    jassert (isPositiveAndBelow (index, colours.size()));
    colours.remove (index);
}

z0 ColorGradient::multiplyOpacity (const f32 multiplier) noexcept
{
    for (auto& c : colours)
        c.colour = c.colour.withMultipliedAlpha (multiplier);
}

//==============================================================================
i32 ColorGradient::getNumColors() const noexcept
{
    return colours.size();
}

f64 ColorGradient::getColorPosition (i32 index) const noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        return colours.getReference (index).position;

    return 0;
 }

Color ColorGradient::getColor (i32 index) const noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        return colours.getReference (index).colour;

    return {};
}

z0 ColorGradient::setColor (i32 index, Color newColor) noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        colours.getReference (index).colour = newColor;
}

Color ColorGradient::getColorAtPosition (f64 position) const noexcept
{
    jassert (approximatelyEqual (colours.getReference (0).position, 0.0)); // the first colour specified has to go at position 0

    if (position <= 0 || colours.size() <= 1)
        return colours.getReference (0).colour;

    i32 i = colours.size() - 1;
    while (position < colours.getReference (i).position)
        --i;

    auto& p1 = colours.getReference (i);

    if (i >= colours.size() - 1)
        return p1.colour;

    auto& p2 = colours.getReference (i + 1);

    return p1.colour.interpolatedWith (p2.colour, (f32) ((position - p1.position) / (p2.position - p1.position)));
}

//==============================================================================
z0 ColorGradient::createLookupTable (PixelARGB* const lookupTable, i32k numEntries) const noexcept
{
    DRX_COLOURGRADIENT_CHECK_COORDS_INITIALISED // Trying to use this object without setting its coordinates?
    jassert (colours.size() >= 2);
    jassert (numEntries > 0);
    jassert (approximatelyEqual (colours.getReference (0).position, 0.0)); // The first colour specified has to go at position 0

    i32 index = 0;

    for (i32 j = 0; j < colours.size() - 1; ++j)
    {
        const auto& o = colours.getReference (j + 0);
        const auto& p = colours.getReference (j + 1);
        const auto numToDo = roundToInt (p.position * (numEntries - 1)) - index;
        const auto pix1 = o.colour.getNonPremultipliedPixelARGB();
        const auto pix2 = p.colour.getNonPremultipliedPixelARGB();

        for (auto i = 0; i < numToDo; ++i)
        {
            auto blended = pix1;
            blended.tween (pix2, (u32) ((i << 8) / numToDo));
            blended.premultiply();

            jassert (0 <= index && index < numEntries);
            lookupTable[index++] = blended;
        }
    }

    std::fill (lookupTable + index, lookupTable + numEntries, colours.getLast().colour.getPixelARGB());
}

i32 ColorGradient::createLookupTable (const AffineTransform& transform, HeapBlock<PixelARGB>& lookupTable) const
{
    DRX_COLOURGRADIENT_CHECK_COORDS_INITIALISED // Trying to use this object without setting its coordinates?
    jassert (colours.size() >= 2);

    auto numEntries = jlimit (1, jmax (1, (colours.size() - 1) << 8),
                              3 * (i32) point1.transformedBy (transform)
                                              .getDistanceFrom (point2.transformedBy (transform)));
    lookupTable.malloc (numEntries);
    createLookupTable (lookupTable, numEntries);
    return numEntries;
}

b8 ColorGradient::isOpaque() const noexcept
{
    for (auto& c : colours)
        if (! c.colour.isOpaque())
            return false;

    return true;
}

b8 ColorGradient::isInvisible() const noexcept
{
    for (auto& c : colours)
        if (! c.colour.isTransparent())
            return false;

    return true;
}

} // namespace drx
