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
    struct GraphicsFontHelpers
    {
        static auto compareFont (const Font& a, const Font& b) { return Font::compare (a, b); }
    };
}

namespace drx::RenderingHelpers
{

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4127)

//==============================================================================
/** Holds either a simple integer translation, or an affine transform.

    @tags{Graphics}
*/
class TranslationOrTransform
{
public:
    TranslationOrTransform() = default;
    TranslationOrTransform (Point<i32> origin) noexcept  : offset (origin) {}

    TranslationOrTransform (const TranslationOrTransform& other) = default;

    AffineTransform getTransform() const noexcept
    {
        return isOnlyTranslated ? AffineTransform::translation (offset)
                                : complexTransform;
    }

    AffineTransform getTransformWith (const AffineTransform& userTransform) const noexcept
    {
        return isOnlyTranslated ? userTransform.translated (offset)
                                : userTransform.followedBy (complexTransform);
    }

    b8 isIdentity() const noexcept
    {
        return isOnlyTranslated && offset.isOrigin();
    }

    z0 setOrigin (Point<i32> delta) noexcept
    {
        if (isOnlyTranslated)
            offset += delta;
        else
            complexTransform = AffineTransform::translation (delta)
                                               .followedBy (complexTransform);
    }

    z0 addTransform (const AffineTransform& t) noexcept
    {
        if (isOnlyTranslated && t.isOnlyTranslation())
        {
            auto tx = (i32) (t.getTranslationX() * 256.0f);
            auto ty = (i32) (t.getTranslationY() * 256.0f);

            if (((tx | ty) & 0xf8) == 0)
            {
                offset += Point<i32> (tx >> 8, ty >> 8);
                return;
            }
        }

        complexTransform = getTransformWith (t);
        isOnlyTranslated = false;
        isRotated = (! approximatelyEqual (complexTransform.mat01, 0.0f)
                     || ! approximatelyEqual (complexTransform.mat10, 0.0f)
                     || complexTransform.mat00 < 0
                     || complexTransform.mat11 < 0);
    }

    f32 getPhysicalPixelScaleFactor() const noexcept
    {
        return isOnlyTranslated ? 1.0f : std::sqrt (std::abs (complexTransform.getDeterminant()));
    }

    z0 moveOriginInDeviceSpace (Point<i32> delta) noexcept
    {
        if (isOnlyTranslated)
            offset += delta;
        else
            complexTransform = complexTransform.translated (delta);
    }

    Rectangle<i32> translated (Rectangle<i32> r) const noexcept
    {
        jassert (isOnlyTranslated);
        return r + offset;
    }

    Rectangle<f32> translated (Rectangle<f32> r) const noexcept
    {
        jassert (isOnlyTranslated);
        return r + offset.toFloat();
    }

    auto boundsAfterTransform (Rectangle<f32> r) const noexcept
    {
        jassert (! isOnlyTranslated);
        return r.transformedBy (complexTransform);
    }

    template <typename RectangleOrPoint>
    auto transformed (RectangleOrPoint r) const noexcept
    {
        jassert (! isOnlyTranslated);
        return r.transformedBy (complexTransform);
    }

    auto boundsAfterTransform (const RectangleList<f32>& r) const noexcept
    {
        jassert (! isOnlyTranslated);
        return boundsAfterTransform (r.getBounds());
    }

    auto boundsAfterTransform (Line<f32> r) const noexcept
    {
        jassert (! isOnlyTranslated);
        return Line { transformed (r.getStart()), transformed (r.getEnd()) };
    }

    template <typename Type>
    Rectangle<f32> deviceSpaceToUserSpace (Rectangle<Type> r) const noexcept
    {
        return isOnlyTranslated ? r.toFloat() - offset.toFloat()
                                : r.toFloat().transformedBy (complexTransform.inverted());
    }

    AffineTransform complexTransform;
    Point<i32> offset;
    b8 isOnlyTranslated = true, isRotated = false;
};

//==============================================================================
/** Holds a cache of recently-used glyph objects of some type.

    @tags{Graphics}
*/
class GlyphCache  : private DeletedAtShutdown
{
public:
    GlyphCache() = default;

    ~GlyphCache() override
    {
        getSingletonPointer() = nullptr;
    }

    static GlyphCache& getInstance()
    {
        auto& g = getSingletonPointer();

        if (g == nullptr)
            g = new GlyphCache();

        return *g;
    }

    //==============================================================================
    z0 reset()
    {
        const ScopedLock sl { lock };
        cache = {};
    }

    const auto& get (const Font& font, i32k glyphNumber)
    {
        const ScopedLock sl { lock };
        return cache.get (Key { font, glyphNumber }, [] (const auto& key)
        {
            auto fontHeight = key.font.getHeight();
            auto typeface = key.font.getTypefacePtr();
            return typeface->getLayersForGlyph (key.font.getMetricsKind(),
                                                key.glyph,
                                                AffineTransform::scale (fontHeight * key.font.getHorizontalScale(),
                                                                        fontHeight),
                                                fontHeight);
        });
    }

private:
    struct Key
    {
        Font font;
        i32 glyph;

        b8 operator< (const Key& other) const
        {
            if (glyph < other.glyph)
                return true;

            if (other.glyph < glyph)
                return false;

            return GraphicsFontHelpers::compareFont (font, other.font);
        }
    };

    LruCache<Key, std::vector<GlyphLayer>> cache;
    CriticalSection lock;

    static GlyphCache*& getSingletonPointer() noexcept
    {
        static GlyphCache* g = nullptr;
        return g;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphCache)
};

//==============================================================================
/** Calculates the alpha values and positions for rendering the edges of a
    non-pixel-aligned rectangle.

    @tags{Graphics}
*/
struct FloatRectangleRasterisingInfo
{
    FloatRectangleRasterisingInfo (Rectangle<f32> area)
        : left   (roundToInt (256.0f * area.getX())),
          top    (roundToInt (256.0f * area.getY())),
          right  (roundToInt (256.0f * area.getRight())),
          bottom (roundToInt (256.0f * area.getBottom()))
    {
        if ((top >> 8) == (bottom >> 8))
        {
            topAlpha = bottom - top;
            bottomAlpha = 0;
            totalTop = top >> 8;
            totalBottom = bottom = top = totalTop + 1;
        }
        else
        {
            if ((top & 255) == 0)
            {
                topAlpha = 0;
                top = totalTop = (top >> 8);
            }
            else
            {
                topAlpha = 255 - (top & 255);
                totalTop = (top >> 8);
                top = totalTop + 1;
            }

            bottomAlpha = bottom & 255;
            bottom >>= 8;
            totalBottom = bottom + (bottomAlpha != 0 ? 1 : 0);
        }

        if ((left >> 8) == (right >> 8))
        {
            leftAlpha = right - left;
            rightAlpha = 0;
            totalLeft = (left >> 8);
            totalRight = right = left = totalLeft + 1;
        }
        else
        {
            if ((left & 255) == 0)
            {
                leftAlpha = 0;
                left = totalLeft = (left >> 8);
            }
            else
            {
                leftAlpha = 255 - (left & 255);
                totalLeft = (left >> 8);
                left = totalLeft + 1;
            }

            rightAlpha = right & 255;
            right >>= 8;
            totalRight = right + (rightAlpha != 0 ? 1 : 0);
        }
    }

    template <class Callback>
    z0 iterate (Callback& callback) const
    {
        if (topAlpha != 0)       callback (totalLeft, totalTop, totalRight - totalLeft, 1, topAlpha);
        if (bottomAlpha != 0)    callback (totalLeft, bottom,   totalRight - totalLeft, 1, bottomAlpha);
        if (leftAlpha != 0)      callback (totalLeft, totalTop, 1, totalBottom - totalTop, leftAlpha);
        if (rightAlpha != 0)     callback (right,     totalTop, 1, totalBottom - totalTop, rightAlpha);

        callback (left, top, right - left, bottom - top, 255);
    }

    inline b8 isOnePixelWide() const noexcept            { return right - left == 1 && leftAlpha + rightAlpha == 0; }

    inline i32 getTopLeftCornerAlpha() const noexcept      { return (topAlpha * leftAlpha) >> 8; }
    inline i32 getTopRightCornerAlpha() const noexcept     { return (topAlpha * rightAlpha) >> 8; }
    inline i32 getBottomLeftCornerAlpha() const noexcept   { return (bottomAlpha * leftAlpha) >> 8; }
    inline i32 getBottomRightCornerAlpha() const noexcept  { return (bottomAlpha * rightAlpha) >> 8; }

    //==============================================================================
    i32 left, top, right, bottom;  // bounds of the solid central area, excluding anti-aliased edges
    i32 totalTop, totalLeft, totalBottom, totalRight; // bounds of the total area, including edges
    i32 topAlpha, leftAlpha, bottomAlpha, rightAlpha; // alpha of each anti-aliased edge
};

// Line::findNearestPointTo will always return a point between the line's start and end, whereas
// this version assumes that the line is infinite.
static Point<f32> closestPointOnInfiniteLine (const Line<f32>& line, const Point<f32>& point)
{
    const Line perpendicularThroughPoint { point, point + line.getPointAlongLine (0.0f, 1.0f) - line.getStart() };
    return line.getIntersection (perpendicularThroughPoint);
}

//==============================================================================
/** Contains classes for calculating the colour of pixels within various types of gradient. */
namespace GradientPixelIterators
{
    /** Iterates the colour of pixels in a linear gradient */
    struct Linear
    {
        Linear (const ColorGradient& gradient, const AffineTransform& transform,
                const PixelARGB* colours, i32 numColors)
            : lookupTable (colours),
              numEntries (numColors)
        {
            jassert (numColors >= 0);
            auto p1 = gradient.point1;
            auto p2 = gradient.point2;

            if (! transform.isIdentity())
            {
                auto p3 = Line<f32> (p2, p1).getPointAlongLine (0.0f, 100.0f);

                p1.applyTransform (transform);
                p2.applyTransform (transform);
                p3.applyTransform (transform);

                p2 = closestPointOnInfiniteLine ({ p2, p3 }, p1);
            }

            vertical   = std::abs (p1.x - p2.x) < 0.001f;
            horizontal = std::abs (p1.y - p2.y) < 0.001f;

            if (vertical)
            {
                scale = roundToInt ((f64) ((z64) numEntries << (i32) numScaleBits) / (f64) (p2.y - p1.y));
                start = roundToInt (p1.y * (f32) scale);
            }
            else if (horizontal)
            {
                scale = roundToInt ((f64) ((z64) numEntries << (i32) numScaleBits) / (f64) (p2.x - p1.x));
                start = roundToInt (p1.x * (f32) scale);
            }
            else
            {
                grad = (p2.y - p1.y) / (f64) (p1.x - p2.x);
                yTerm = p1.y - (p1.x / grad);
                scale = roundToInt ((f64) ((z64) numEntries << (i32) numScaleBits) / (yTerm * grad - (p2.y * grad - p2.x)));
                grad *= scale;
            }
        }

        forcedinline z0 setY (i32 y) noexcept
        {
            if (vertical)
                linePix = lookupTable[jlimit (0, numEntries, (y * scale - start) >> (i32) numScaleBits)];
            else if (! horizontal)
                start = roundToInt ((y - yTerm) * grad);
        }

        inline PixelARGB getPixel (i32 x) const noexcept
        {
            return vertical ? linePix
                            : lookupTable[jlimit (0, numEntries, (x * scale - start) >> (i32) numScaleBits)];
        }

        const PixelARGB* const lookupTable;
        i32k numEntries;
        PixelARGB linePix;
        i32 start, scale;
        f64 grad, yTerm;
        b8 vertical, horizontal;
        enum { numScaleBits = 12 };

        DRX_DECLARE_NON_COPYABLE (Linear)
    };

    //==============================================================================
    /** Iterates the colour of pixels in a circular radial gradient */
    struct Radial
    {
        Radial (const ColorGradient& gradient, const AffineTransform&,
                const PixelARGB* colours, i32 numColors)
            : lookupTable (colours),
              numEntries (numColors),
              gx1 (gradient.point1.x),
              gy1 (gradient.point1.y)
        {
            jassert (numColors >= 0);
            auto diff = gradient.point1 - gradient.point2;
            maxDist = diff.x * diff.x + diff.y * diff.y;
            invScale = numEntries / std::sqrt (maxDist);
            jassert (roundToInt (std::sqrt (maxDist) * invScale) <= numEntries);
        }

        forcedinline z0 setY (i32 y) noexcept
        {
            dy = y - gy1;
            dy *= dy;
        }

        inline PixelARGB getPixel (i32 px) const noexcept
        {
            auto x = px - gx1;
            x *= x;
            x += dy;

            return lookupTable[x >= maxDist ? numEntries : roundToInt (std::sqrt (x) * invScale)];
        }

        const PixelARGB* const lookupTable;
        i32k numEntries;
        const f64 gx1, gy1;
        f64 maxDist, invScale, dy;

        DRX_DECLARE_NON_COPYABLE (Radial)
    };

    //==============================================================================
    /** Iterates the colour of pixels in a skewed radial gradient */
    struct TransformedRadial   : public Radial
    {
        TransformedRadial (const ColorGradient& gradient, const AffineTransform& transform,
                           const PixelARGB* colours, i32 numColors)
            : Radial (gradient, transform, colours, numColors),
              inverseTransform (transform.inverted())
        {
            tM10 = inverseTransform.mat10;
            tM00 = inverseTransform.mat00;
        }

        forcedinline z0 setY (i32 y) noexcept
        {
            auto floatY = (f32) y;
            lineYM01 = inverseTransform.mat01 * floatY + inverseTransform.mat02 - gx1;
            lineYM11 = inverseTransform.mat11 * floatY + inverseTransform.mat12 - gy1;
        }

        inline PixelARGB getPixel (i32 px) const noexcept
        {
            f64 x = px;
            auto y = tM10 * x + lineYM11;
            x = tM00 * x + lineYM01;
            x *= x;
            x += y * y;

            if (x >= maxDist)
                return lookupTable[numEntries];

            return lookupTable[jmin (numEntries, roundToInt (std::sqrt (x) * invScale))];
        }

    private:
        f64 tM10, tM00, lineYM01, lineYM11;
        const AffineTransform inverseTransform;

        DRX_DECLARE_NON_COPYABLE (TransformedRadial)
    };
}

#define DRX_PERFORM_PIXEL_OP_LOOP(op) \
{ \
    i32k destStride = destData.pixelStride;  \
    do { dest->op; dest = addBytesToPointer (dest, destStride); } while (--width > 0); \
}

//==============================================================================
/** Contains classes for filling edge tables with various fill types. */
namespace EdgeTableFillers
{
    /** Fills an edge-table with a solid colour. */
    template <class PixelType, b8 replaceExisting = false>
    struct SolidColor
    {
        SolidColor (const Image::BitmapData& image, PixelARGB colour)
            : destData (image), sourceColor (colour)
        {
            if (sizeof (PixelType) == 3 && (size_t) destData.pixelStride == sizeof (PixelType))
                areRGBComponentsEqual = sourceColor.getRed() == sourceColor.getGreen()
                                            && sourceColor.getGreen() == sourceColor.getBlue();
            else
                areRGBComponentsEqual = false;
        }

        forcedinline z0 setEdgeTableYPos (i32 y) noexcept
        {
            linePixels = (PixelType*) destData.getLinePointer (y);
        }

        forcedinline z0 handleEdgeTablePixel (i32 x, i32 alphaLevel) const noexcept
        {
            if (replaceExisting)
                getPixel (x)->set (sourceColor);
            else
                getPixel (x)->blend (sourceColor, (u32) alphaLevel);
        }

        forcedinline z0 handleEdgeTablePixelFull (i32 x) const noexcept
        {
            if (replaceExisting)
                getPixel (x)->set (sourceColor);
            else
                getPixel (x)->blend (sourceColor);
        }

        forcedinline z0 handleEdgeTableLine (i32 x, i32 width, i32 alphaLevel) const noexcept
        {
            auto p = sourceColor;
            p.multiplyAlpha (alphaLevel);

            auto* dest = getPixel (x);

            if (replaceExisting || p.getAlpha() >= 0xff)
                replaceLine (dest, p, width);
            else
                blendLine (dest, p, width);
        }

        forcedinline z0 handleEdgeTableLineFull (i32 x, i32 width) const noexcept
        {
            auto* dest = getPixel (x);

            if (replaceExisting || sourceColor.getAlpha() >= 0xff)
                replaceLine (dest, sourceColor, width);
            else
                blendLine (dest, sourceColor, width);
        }

        z0 handleEdgeTableRectangle (i32 x, i32 y, i32 width, i32 height, i32 alphaLevel) noexcept
        {
            auto p = sourceColor;
            p.multiplyAlpha (alphaLevel);

            setEdgeTableYPos (y);
            auto* dest = getPixel (x);

            if (replaceExisting || p.getAlpha() >= 0xff)
            {
                while (--height >= 0)
                {
                    replaceLine (dest, p, width);
                    dest = addBytesToPointer (dest, destData.lineStride);
                }
            }
            else
            {
                while (--height >= 0)
                {
                    blendLine (dest, p, width);
                    dest = addBytesToPointer (dest, destData.lineStride);
                }
            }
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 y, i32 width, i32 height) noexcept
        {
            handleEdgeTableRectangle (x, y, width, height, 255);
        }

    private:
        const Image::BitmapData& destData;
        PixelType* linePixels;
        PixelARGB sourceColor;
        b8 areRGBComponentsEqual;

        forcedinline PixelType* getPixel (i32 x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        inline z0 blendLine (PixelType* dest, PixelARGB colour, i32 width) const noexcept
        {
            DRX_PERFORM_PIXEL_OP_LOOP (blend (colour))
        }

        forcedinline z0 replaceLine (PixelRGB* dest, PixelARGB colour, i32 width) const noexcept
        {
            if ((size_t) destData.pixelStride == sizeof (*dest) && areRGBComponentsEqual)
                memset ((uk) dest, colour.getRed(), (size_t) width * 3);   // if all the component values are the same, we can cheat..
            else
                DRX_PERFORM_PIXEL_OP_LOOP (set (colour));
        }

        forcedinline z0 replaceLine (PixelAlpha* dest, const PixelARGB colour, i32 width) const noexcept
        {
            if ((size_t) destData.pixelStride == sizeof (*dest))
                memset ((uk) dest, colour.getAlpha(), (size_t) width);
            else
                DRX_PERFORM_PIXEL_OP_LOOP (setAlpha (colour.getAlpha()))
        }

        forcedinline z0 replaceLine (PixelARGB* dest, const PixelARGB colour, i32 width) const noexcept
        {
            DRX_PERFORM_PIXEL_OP_LOOP (set (colour))
        }

        DRX_DECLARE_NON_COPYABLE (SolidColor)
    };

    //==============================================================================
    /** Fills an edge-table with a gradient. */
    template <class PixelType, class GradientType>
    struct Gradient  : public GradientType
    {
        Gradient (const Image::BitmapData& dest, const ColorGradient& gradient, const AffineTransform& transform,
                  const PixelARGB* colours, i32 numColors)
            : GradientType (gradient, transform, colours, numColors - 1),
              destData (dest)
        {
        }

        forcedinline z0 setEdgeTableYPos (i32 y) noexcept
        {
            linePixels = (PixelType*) destData.getLinePointer (y);
            GradientType::setY (y);
        }

        forcedinline z0 handleEdgeTablePixel (i32 x, i32 alphaLevel) const noexcept
        {
            getPixel (x)->blend (GradientType::getPixel (x), (u32) alphaLevel);
        }

        forcedinline z0 handleEdgeTablePixelFull (i32 x) const noexcept
        {
            getPixel (x)->blend (GradientType::getPixel (x));
        }

        z0 handleEdgeTableLine (i32 x, i32 width, i32 alphaLevel) const noexcept
        {
            auto* dest = getPixel (x);

            if (alphaLevel < 0xff)
                DRX_PERFORM_PIXEL_OP_LOOP (blend (GradientType::getPixel (x++), (u32) alphaLevel))
            else
                DRX_PERFORM_PIXEL_OP_LOOP (blend (GradientType::getPixel (x++)))
        }

        z0 handleEdgeTableLineFull (i32 x, i32 width) const noexcept
        {
            auto* dest = getPixel (x);
            DRX_PERFORM_PIXEL_OP_LOOP (blend (GradientType::getPixel (x++)))
        }

        z0 handleEdgeTableRectangle (i32 x, i32 y, i32 width, i32 height, i32 alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 y, i32 width, i32 height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

    private:
        const Image::BitmapData& destData;
        PixelType* linePixels;

        forcedinline PixelType* getPixel (i32 x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        DRX_DECLARE_NON_COPYABLE (Gradient)
    };

    //==============================================================================
    /** Fills an edge-table with a non-transformed image. */
    template <class DestPixelType, class SrcPixelType, b8 repeatPattern>
    struct ImageFill
    {
        ImageFill (const Image::BitmapData& dest, const Image::BitmapData& src, i32 alpha, i32 x, i32 y)
            : destData (dest),
              srcData (src),
              extraAlpha (alpha + 1),
              xOffset (repeatPattern ? negativeAwareModulo (x, src.width)  - src.width  : x),
              yOffset (repeatPattern ? negativeAwareModulo (y, src.height) - src.height : y)
        {
        }

        forcedinline z0 setEdgeTableYPos (i32 y) noexcept
        {
            linePixels = (DestPixelType*) destData.getLinePointer (y);
            y -= yOffset;

            if (repeatPattern)
            {
                jassert (y >= 0);
                y %= srcData.height;
            }

            sourceLineStart = (SrcPixelType*) srcData.getLinePointer (y);
        }

        forcedinline z0 handleEdgeTablePixel (i32 x, i32 alphaLevel) const noexcept
        {
            alphaLevel = (alphaLevel * extraAlpha) >> 8;

            getDestPixel (x)->blend (*getSrcPixel (repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)), (u32) alphaLevel);
        }

        forcedinline z0 handleEdgeTablePixelFull (i32 x) const noexcept
        {
            getDestPixel (x)->blend (*getSrcPixel (repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)), (u32) extraAlpha);
        }

        z0 handleEdgeTableLine (i32 x, i32 width, i32 alphaLevel) const noexcept
        {
            auto* dest = getDestPixel (x);
            alphaLevel = (alphaLevel * extraAlpha) >> 8;
            x -= xOffset;

            if (repeatPattern)
            {
                if (alphaLevel < 0xfe)
                    DRX_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width), (u32) alphaLevel))
                else
                    DRX_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width)))
            }
            else
            {
                jassert (x >= 0 && x + width <= srcData.width);

                if (alphaLevel < 0xfe)
                    DRX_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++), (u32) alphaLevel))
                else
                    copyRow (dest, getSrcPixel (x), width);
            }
        }

        z0 handleEdgeTableLineFull (i32 x, i32 width) const noexcept
        {
            auto* dest = getDestPixel (x);
            x -= xOffset;

            if (repeatPattern)
            {
                if (extraAlpha < 0xfe)
                    DRX_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width), (u32) extraAlpha))
                else
                    DRX_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width)))
            }
            else
            {
                jassert (x >= 0 && x + width <= srcData.width);

                if (extraAlpha < 0xfe)
                    DRX_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++), (u32) extraAlpha))
                else
                    copyRow (dest, getSrcPixel (x), width);
            }
        }

        z0 handleEdgeTableRectangle (i32 x, i32 y, i32 width, i32 height, i32 alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 y, i32 width, i32 height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

        z0 clipEdgeTableLine (EdgeTable& et, i32 x, i32 y, i32 width)
        {
            jassert (x - xOffset >= 0 && x + width - xOffset <= srcData.width);
            auto* s = (SrcPixelType*) srcData.getLinePointer (y - yOffset);
            auto* mask = (u8*) (s + x - xOffset);

            if (sizeof (SrcPixelType) == sizeof (PixelARGB))
                mask += PixelARGB::indexA;

            et.clipLineToMask (x, y, mask, sizeof (SrcPixelType), width);
        }

    private:
        const Image::BitmapData& destData;
        const Image::BitmapData& srcData;
        i32k extraAlpha, xOffset, yOffset;
        DestPixelType* linePixels;
        SrcPixelType* sourceLineStart;

        forcedinline DestPixelType* getDestPixel (i32 x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        forcedinline SrcPixelType const* getSrcPixel (i32 x) const noexcept
        {
            return addBytesToPointer (sourceLineStart, x * srcData.pixelStride);
        }

        forcedinline z0 copyRow (DestPixelType* dest, SrcPixelType const* src, i32 width) const noexcept
        {
            auto destStride = destData.pixelStride;
            auto srcStride  = srcData.pixelStride;

            if (destStride == srcStride
                 && srcData.pixelFormat  == Image::RGB
                 && destData.pixelFormat == Image::RGB)
            {
                memcpy ((uk) dest, src, (size_t) (width * srcStride));
            }
            else
            {
                do
                {
                    dest->blend (*src);
                    dest = addBytesToPointer (dest, destStride);
                    src  = addBytesToPointer (src, srcStride);
                } while (--width > 0);
            }
        }

        DRX_DECLARE_NON_COPYABLE (ImageFill)
    };

    //==============================================================================
    /** Fills an edge-table with a transformed image. */
    template <class DestPixelType, class SrcPixelType, b8 repeatPattern>
    struct TransformedImageFill
    {
        TransformedImageFill (const Image::BitmapData& dest, const Image::BitmapData& src,
                              const AffineTransform& transform, i32 alpha, Graphics::ResamplingQuality q)
            : interpolator (transform,
                            q != Graphics::lowResamplingQuality ? 0.5f : 0.0f,
                            q != Graphics::lowResamplingQuality ? -128 : 0),
              destData (dest),
              srcData (src),
              extraAlpha (alpha + 1),
              quality (q),
              maxX (src.width  - 1),
              maxY (src.height - 1)
        {
            scratchBuffer.malloc (scratchSize);
        }

        forcedinline z0 setEdgeTableYPos (i32 newY) noexcept
        {
            currentY = newY;
            linePixels = (DestPixelType*) destData.getLinePointer (newY);
        }

        forcedinline z0 handleEdgeTablePixel (i32 x, i32 alphaLevel) noexcept
        {
            SrcPixelType p;
            generate (&p, x, 1);

            getDestPixel (x)->blend (p, (u32) (alphaLevel * extraAlpha) >> 8);
        }

        forcedinline z0 handleEdgeTablePixelFull (i32 x) noexcept
        {
            SrcPixelType p;
            generate (&p, x, 1);

            getDestPixel (x)->blend (p, (u32) extraAlpha);
        }

        z0 handleEdgeTableLine (i32 x, i32 width, i32 alphaLevel) noexcept
        {
            if (width > (i32) scratchSize)
            {
                scratchSize = (size_t) width;
                scratchBuffer.malloc (scratchSize);
            }

            SrcPixelType* span = scratchBuffer;
            generate (span, x, width);

            auto* dest = getDestPixel (x);
            alphaLevel *= extraAlpha;
            alphaLevel >>= 8;

            if (alphaLevel < 0xfe)
                DRX_PERFORM_PIXEL_OP_LOOP (blend (*span++, (u32) alphaLevel))
            else
                DRX_PERFORM_PIXEL_OP_LOOP (blend (*span++))
        }

        forcedinline z0 handleEdgeTableLineFull (i32 x, i32 width) noexcept
        {
            handleEdgeTableLine (x, width, 255);
        }

        z0 handleEdgeTableRectangle (i32 x, i32 y, i32 width, i32 height, i32 alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 y, i32 width, i32 height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

        z0 clipEdgeTableLine (EdgeTable& et, i32 x, i32 y, i32 width)
        {
            if (width > (i32) scratchSize)
            {
                scratchSize = (size_t) width;
                scratchBuffer.malloc (scratchSize);
            }

            currentY = y;
            generate (scratchBuffer.get(), x, width);

            et.clipLineToMask (x, y,
                               reinterpret_cast<u8*> (scratchBuffer.get()) + SrcPixelType::indexA,
                               sizeof (SrcPixelType), width);
        }

    private:
        forcedinline DestPixelType* getDestPixel (i32 x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        //==============================================================================
        template <class PixelType>
        z0 generate (PixelType* dest, i32 x, i32 numPixels) noexcept
        {
            this->interpolator.setStartOfLine ((f32) x, (f32) currentY, numPixels);

            do
            {
                i32 hiResX, hiResY;
                this->interpolator.next (hiResX, hiResY);

                i32 loResX = hiResX >> 8;
                i32 loResY = hiResY >> 8;

                if (repeatPattern)
                {
                    loResX = negativeAwareModulo (loResX, srcData.width);
                    loResY = negativeAwareModulo (loResY, srcData.height);
                }

                if (quality != Graphics::lowResamplingQuality)
                {
                    if (isPositiveAndBelow (loResX, maxX))
                    {
                        if (isPositiveAndBelow (loResY, maxY))
                        {
                            // In the centre of the image..
                            render4PixelAverage (dest, this->srcData.getPixelPointer (loResX, loResY),
                                                 hiResX & 255, hiResY & 255);
                            ++dest;
                            continue;
                        }

                        if (! repeatPattern)
                        {
                            // At a top or bottom edge..
                            if (loResY < 0)
                                render2PixelAverageX (dest, this->srcData.getPixelPointer (loResX, 0), hiResX & 255);
                            else
                                render2PixelAverageX (dest, this->srcData.getPixelPointer (loResX, maxY), hiResX & 255);

                            ++dest;
                            continue;
                        }
                    }
                    else
                    {
                        if (isPositiveAndBelow (loResY, maxY) && ! repeatPattern)
                        {
                            // At a left or right hand edge..
                            if (loResX < 0)
                                render2PixelAverageY (dest, this->srcData.getPixelPointer (0, loResY), hiResY & 255);
                            else
                                render2PixelAverageY (dest, this->srcData.getPixelPointer (maxX, loResY), hiResY & 255);

                            ++dest;
                            continue;
                        }
                    }
                }

                if (! repeatPattern)
                {
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

                dest->set (*(const PixelType*) this->srcData.getPixelPointer (loResX, loResY));
                ++dest;

            } while (--numPixels > 0);
        }

        //==============================================================================
        z0 render4PixelAverage (PixelARGB* dest, u8k* src, i32 subPixelX, i32 subPixelY) noexcept
        {
            u32 c[4] = { 256 * 128, 256 * 128, 256 * 128, 256 * 128 };

            auto weight = (u32) ((256 - subPixelX) * (256 - subPixelY));
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.pixelStride;

            weight = (u32) (subPixelX * (256 - subPixelY));
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.lineStride;

            weight = (u32) (subPixelX * subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src -= this->srcData.pixelStride;

            weight = (u32) ((256 - subPixelX) * subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            dest->setARGB ((u8) (c[PixelARGB::indexA] >> 16),
                           (u8) (c[PixelARGB::indexR] >> 16),
                           (u8) (c[PixelARGB::indexG] >> 16),
                           (u8) (c[PixelARGB::indexB] >> 16));
        }

        z0 render2PixelAverageX (PixelARGB* dest, u8k* src, u32 subPixelX) noexcept
        {
            u32 c[4] = { 128, 128, 128, 128 };

            u32 weight = 256 - subPixelX;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.pixelStride;

            weight = subPixelX;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            dest->setARGB ((u8) (c[PixelARGB::indexA] >> 8),
                           (u8) (c[PixelARGB::indexR] >> 8),
                           (u8) (c[PixelARGB::indexG] >> 8),
                           (u8) (c[PixelARGB::indexB] >> 8));
        }

        z0 render2PixelAverageY (PixelARGB* dest, u8k* src, u32 subPixelY) noexcept
        {
            u32 c[4] = { 128, 128, 128, 128 };

            u32 weight = 256 - subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.lineStride;

            weight = subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            dest->setARGB ((u8) (c[PixelARGB::indexA] >> 8),
                           (u8) (c[PixelARGB::indexR] >> 8),
                           (u8) (c[PixelARGB::indexG] >> 8),
                           (u8) (c[PixelARGB::indexB] >> 8));
        }

        //==============================================================================
        z0 render4PixelAverage (PixelRGB* dest, u8k* src, u32 subPixelX, u32 subPixelY) noexcept
        {
            u32 c[3] = { 256 * 128, 256 * 128, 256 * 128 };

            u32 weight = (256 - subPixelX) * (256 - subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.pixelStride;

            weight = subPixelX * (256 - subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.lineStride;

            weight = subPixelX * subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src -= this->srcData.pixelStride;

            weight = (256 - subPixelX) * subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            dest->setARGB ((u8) 255,
                           (u8) (c[PixelRGB::indexR] >> 16),
                           (u8) (c[PixelRGB::indexG] >> 16),
                           (u8) (c[PixelRGB::indexB] >> 16));
        }

        z0 render2PixelAverageX (PixelRGB* dest, u8k* src, u32 subPixelX) noexcept
        {
            u32 c[3] = { 128, 128, 128 };

            u32k weight = 256 - subPixelX;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.pixelStride;

            c[0] += subPixelX * src[0];
            c[1] += subPixelX * src[1];
            c[2] += subPixelX * src[2];

            dest->setARGB ((u8) 255,
                           (u8) (c[PixelRGB::indexR] >> 8),
                           (u8) (c[PixelRGB::indexG] >> 8),
                           (u8) (c[PixelRGB::indexB] >> 8));
        }

        z0 render2PixelAverageY (PixelRGB* dest, u8k* src, u32 subPixelY) noexcept
        {
            u32 c[3] = { 128, 128, 128 };

            u32k weight = 256 - subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.lineStride;

            c[0] += subPixelY * src[0];
            c[1] += subPixelY * src[1];
            c[2] += subPixelY * src[2];

            dest->setARGB ((u8) 255,
                           (u8) (c[PixelRGB::indexR] >> 8),
                           (u8) (c[PixelRGB::indexG] >> 8),
                           (u8) (c[PixelRGB::indexB] >> 8));
        }

        //==============================================================================
        z0 render4PixelAverage (PixelAlpha* dest, u8k* src, u32 subPixelX, u32 subPixelY) noexcept
        {
            u32 c = 256 * 128;
            c += src[0] * ((256 - subPixelX) * (256 - subPixelY));
            src += this->srcData.pixelStride;
            c += src[0] * (subPixelX * (256 - subPixelY));
            src += this->srcData.lineStride;
            c += src[0] * (subPixelX * subPixelY);
            src -= this->srcData.pixelStride;

            c += src[0] * ((256 - subPixelX) * subPixelY);

            *((u8*) dest) = (u8) (c >> 16);
        }

        z0 render2PixelAverageX (PixelAlpha* dest, u8k* src, u32 subPixelX) noexcept
        {
            u32 c = 128;
            c += src[0] * (256 - subPixelX);
            src += this->srcData.pixelStride;
            c += src[0] * subPixelX;
            *((u8*) dest) = (u8) (c >> 8);
        }

        z0 render2PixelAverageY (PixelAlpha* dest, u8k* src, u32 subPixelY) noexcept
        {
            u32 c = 128;
            c += src[0] * (256 - subPixelY);
            src += this->srcData.lineStride;
            c += src[0] * subPixelY;
            *((u8*) dest) = (u8) (c >> 8);
        }

        //==============================================================================
        struct TransformedImageSpanInterpolator
        {
            TransformedImageSpanInterpolator (const AffineTransform& transform, f32 offsetFloat, i32 offsetInt) noexcept
                : inverseTransform (transform.inverted()),
                  pixelOffset (offsetFloat), pixelOffsetInt (offsetInt)
            {}

            z0 setStartOfLine (f32 sx, f32 sy, i32 numPixels) noexcept
            {
                jassert (numPixels > 0);

                sx += pixelOffset;
                sy += pixelOffset;
                auto x1 = sx, y1 = sy;
                sx += (f32) numPixels;
                inverseTransform.transformPoints (x1, y1, sx, sy);

                xBresenham.set ((i32) (x1 * 256.0f), (i32) (sx * 256.0f), numPixels, pixelOffsetInt);
                yBresenham.set ((i32) (y1 * 256.0f), (i32) (sy * 256.0f), numPixels, pixelOffsetInt);
            }

            z0 next (i32& px, i32& py) noexcept
            {
                px = xBresenham.n;  xBresenham.stepToNext();
                py = yBresenham.n;  yBresenham.stepToNext();
            }

        private:
            struct BresenhamInterpolator
            {
                BresenhamInterpolator() = default;

                z0 set (i32 n1, i32 n2, i32 steps, i32 offsetInt) noexcept
                {
                    numSteps = steps;
                    step = (n2 - n1) / numSteps;
                    remainder = modulo = (n2 - n1) % numSteps;
                    n = n1 + offsetInt;

                    if (modulo <= 0)
                    {
                        modulo += numSteps;
                        remainder += numSteps;
                        --step;
                    }

                    modulo -= numSteps;
                }

                forcedinline z0 stepToNext() noexcept
                {
                    modulo += remainder;
                    n += step;

                    if (modulo > 0)
                    {
                        modulo -= numSteps;
                        ++n;
                    }
                }

                i32 n;

            private:
                i32 numSteps, step, modulo, remainder;
            };

            const AffineTransform inverseTransform;
            BresenhamInterpolator xBresenham, yBresenham;
            const f32 pixelOffset;
            i32k pixelOffsetInt;

            DRX_DECLARE_NON_COPYABLE (TransformedImageSpanInterpolator)
        };

        //==============================================================================
        TransformedImageSpanInterpolator interpolator;
        const Image::BitmapData& destData;
        const Image::BitmapData& srcData;
        i32k extraAlpha;
        const Graphics::ResamplingQuality quality;
        i32k maxX, maxY;
        i32 currentY;
        DestPixelType* linePixels;
        HeapBlock<SrcPixelType> scratchBuffer;
        size_t scratchSize = 2048;

        DRX_DECLARE_NON_COPYABLE (TransformedImageFill)
    };


    //==============================================================================
    template <class Iterator>
    z0 renderImageTransformed (Iterator& iter, const Image::BitmapData& destData, const Image::BitmapData& srcData,
                                 i32 alpha, const AffineTransform& transform, Graphics::ResamplingQuality quality, b8 tiledFill)
    {
        switch (destData.pixelFormat)
        {
        case Image::ARGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFill<PixelARGB, PixelARGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelARGB, PixelARGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFill<PixelARGB, PixelRGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelARGB, PixelRGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { TransformedImageFill<PixelARGB, PixelAlpha, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelARGB, PixelAlpha, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            }
            break;

        case Image::RGB:
        {
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFill<PixelRGB, PixelARGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelRGB, PixelARGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFill<PixelRGB, PixelRGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelRGB, PixelRGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { TransformedImageFill<PixelRGB, PixelAlpha, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelRGB, PixelAlpha, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            }
            break;
        }

        case Image::SingleChannel:
        case Image::UnknownFormat:
        default:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFill<PixelAlpha, PixelARGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelAlpha, PixelARGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFill<PixelAlpha, PixelRGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelAlpha, PixelRGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { TransformedImageFill<PixelAlpha, PixelAlpha, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelAlpha, PixelAlpha, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            }
            break;
        }
    }

    template <class Iterator>
    z0 renderImageUntransformed (Iterator& iter, const Image::BitmapData& destData, const Image::BitmapData& srcData, i32 alpha, i32 x, i32 y, b8 tiledFill)
    {
        switch (destData.pixelFormat)
        {
        case Image::ARGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFill<PixelARGB, PixelARGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelARGB, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFill<PixelARGB, PixelRGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelARGB, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { ImageFill<PixelARGB, PixelAlpha, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelARGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;

        case Image::RGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFill<PixelRGB, PixelARGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelRGB, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFill<PixelRGB, PixelRGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelRGB, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { ImageFill<PixelRGB, PixelAlpha, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelRGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;

        case Image::SingleChannel:
        case Image::UnknownFormat:
        default:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFill<PixelAlpha, PixelARGB, true>   r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelAlpha, PixelARGB, false>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFill<PixelAlpha, PixelRGB, true>    r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelAlpha, PixelRGB, false>   r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { ImageFill<PixelAlpha, PixelAlpha, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelAlpha, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;
        }
    }

    template <class Iterator, class DestPixelType>
    z0 renderSolidFill (Iterator& iter, const Image::BitmapData& destData, PixelARGB fillColor, b8 replaceContents, DestPixelType*)
    {
        if (replaceContents)
        {
            EdgeTableFillers::SolidColor<DestPixelType, true> r (destData, fillColor);
            iter.iterate (r);
        }
        else
        {
            EdgeTableFillers::SolidColor<DestPixelType, false> r (destData, fillColor);
            iter.iterate (r);
        }
    }

    template <class Iterator, class DestPixelType>
    z0 renderGradient (Iterator& iter, const Image::BitmapData& destData, const ColorGradient& g, const AffineTransform& transform,
                         const PixelARGB* lookupTable, i32 numLookupEntries, b8 isIdentity, DestPixelType*)
    {
        if (g.isRadial)
        {
            if (isIdentity)
            {
                EdgeTableFillers::Gradient<DestPixelType, GradientPixelIterators::Radial> renderer (destData, g, transform, lookupTable, numLookupEntries);
                iter.iterate (renderer);
            }
            else
            {
                EdgeTableFillers::Gradient<DestPixelType, GradientPixelIterators::TransformedRadial> renderer (destData, g, transform, lookupTable, numLookupEntries);
                iter.iterate (renderer);
            }
        }
        else
        {
            EdgeTableFillers::Gradient<DestPixelType, GradientPixelIterators::Linear> renderer (destData, g, transform, lookupTable, numLookupEntries);
            iter.iterate (renderer);
        }
    }
}

//==============================================================================
namespace ClipRegions
{
    template <typename SavedStateType>
    struct Base  : public SingleThreadedReferenceCountedObject
    {
        Base() = default;
        ~Base() override = default;

        using Ptr = ReferenceCountedObjectPtr<Base>;

        virtual Ptr clone() const = 0;
        virtual Ptr applyClipTo (const Ptr& target) const = 0;

        virtual Ptr clipToRectangle (Rectangle<i32>) = 0;
        virtual Ptr clipToRectangleList (const RectangleList<i32>&) = 0;
        virtual Ptr excludeClipRectangle (Rectangle<i32>) = 0;
        virtual Ptr clipToPath (const Path&, const AffineTransform&) = 0;
        virtual Ptr clipToEdgeTable (const EdgeTable&) = 0;
        virtual Ptr clipToImageAlpha (const Image&, const AffineTransform&, Graphics::ResamplingQuality) = 0;
        virtual z0 translate (Point<i32> delta) = 0;

        virtual b8 clipRegionIntersects (Rectangle<i32>) const = 0;
        virtual Rectangle<i32> getClipBounds() const = 0;

        virtual z0 fillRectWithColor (SavedStateType&, Rectangle<i32>, PixelARGB colour, b8 replaceContents) const = 0;
        virtual z0 fillRectWithColor (SavedStateType&, Rectangle<f32>, PixelARGB colour) const = 0;
        virtual z0 fillAllWithColor (SavedStateType&, PixelARGB colour, b8 replaceContents) const = 0;
        virtual z0 fillAllWithGradient (SavedStateType&, ColorGradient&, const AffineTransform&, b8 isIdentity) const = 0;
        virtual z0 renderImageTransformed (SavedStateType&, const Image&, i32 alpha, const AffineTransform&, Graphics::ResamplingQuality, b8 tiledFill) const = 0;
        virtual z0 renderImageUntransformed (SavedStateType&, const Image&, i32 alpha, i32 x, i32 y, b8 tiledFill) const = 0;
    };

    //==============================================================================
    template <typename SavedStateType>
    struct EdgeTableRegion  : public Base<SavedStateType>
    {
        EdgeTableRegion (const EdgeTable& e)            : edgeTable (e) {}
        EdgeTableRegion (Rectangle<i32> r)              : edgeTable (r) {}
        EdgeTableRegion (Rectangle<f32> r)            : edgeTable (r) {}
        EdgeTableRegion (const RectangleList<i32>& r)   : edgeTable (r) {}
        EdgeTableRegion (const RectangleList<f32>& r) : edgeTable (r) {}
        EdgeTableRegion (Rectangle<i32> bounds, const Path& p, const AffineTransform& t) : edgeTable (bounds, p, t) {}

        EdgeTableRegion (const EdgeTableRegion& other)  : edgeTable (other.edgeTable) {}
        EdgeTableRegion& operator= (const EdgeTableRegion&) = delete;

        using Ptr = typename Base<SavedStateType>::Ptr;

        Ptr clone() const override                           { return *new EdgeTableRegion (*this); }
        Ptr applyClipTo (const Ptr& target) const override   { return target->clipToEdgeTable (edgeTable); }

        Ptr clipToRectangle (Rectangle<i32> r) override
        {
            edgeTable.clipToRectangle (r);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToRectangleList (const RectangleList<i32>& r) override
        {
            RectangleList<i32> inverse (edgeTable.getMaximumBounds());

            if (inverse.subtract (r))
                for (auto& i : inverse)
                    edgeTable.excludeRectangle (i);

            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr excludeClipRectangle (Rectangle<i32> r) override
        {
            edgeTable.excludeRectangle (r);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToPath (const Path& p, const AffineTransform& transform) override
        {
            EdgeTable et (edgeTable.getMaximumBounds(), p, transform);
            edgeTable.clipToEdgeTable (et);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToEdgeTable (const EdgeTable& et) override
        {
            edgeTable.clipToEdgeTable (et);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToImageAlpha (const Image& image, const AffineTransform& transform, Graphics::ResamplingQuality quality) override
        {
            const Image::BitmapData srcData (image, Image::BitmapData::readOnly);

            if (transform.isOnlyTranslation())
            {
                // If our translation doesn't involve any distortion, just use a simple blit..
                auto tx = (i32) (transform.getTranslationX() * 256.0f);
                auto ty = (i32) (transform.getTranslationY() * 256.0f);

                if (quality == Graphics::lowResamplingQuality || ((tx | ty) & 224) == 0)
                {
                    auto imageX = ((tx + 128) >> 8);
                    auto imageY = ((ty + 128) >> 8);

                    if (image.getFormat() == Image::ARGB)
                        straightClipImage (srcData, imageX, imageY, (PixelARGB*) nullptr);
                    else
                        straightClipImage (srcData, imageX, imageY, (PixelAlpha*) nullptr);

                    return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
                }
            }

            if (transform.isSingularity())
                return Ptr();

            {
                Path p;
                p.addRectangle (0, 0, (f32) srcData.width, (f32) srcData.height);
                EdgeTable et2 (edgeTable.getMaximumBounds(), p, transform);
                edgeTable.clipToEdgeTable (et2);
            }

            if (! edgeTable.isEmpty())
            {
                if (image.getFormat() == Image::ARGB)
                    transformedClipImage (srcData, transform, quality, (PixelARGB*) nullptr);
                else
                    transformedClipImage (srcData, transform, quality, (PixelAlpha*) nullptr);
            }

            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        z0 translate (Point<i32> delta) override
        {
            edgeTable.translate ((f32) delta.x, delta.y);
        }

        b8 clipRegionIntersects (Rectangle<i32> r) const override
        {
            return edgeTable.getMaximumBounds().intersects (r);
        }

        Rectangle<i32> getClipBounds() const override
        {
            return edgeTable.getMaximumBounds();
        }

        z0 fillRectWithColor (SavedStateType& state, Rectangle<i32> area, PixelARGB colour, b8 replaceContents) const override
        {
            fillRectWithColorImpl (state, area, colour, replaceContents);
        }

        z0 fillRectWithColor (SavedStateType& state, Rectangle<f32> area, PixelARGB colour) const override
        {
            fillRectWithColorImpl (state, area, colour, false);
        }

        z0 fillAllWithColor (SavedStateType& state, PixelARGB colour, b8 replaceContents) const override
        {
            state.fillWithSolidColor (edgeTable, colour, replaceContents);
        }

        z0 fillAllWithGradient (SavedStateType& state, ColorGradient& gradient, const AffineTransform& transform, b8 isIdentity) const override
        {
            state.fillWithGradient (edgeTable, gradient, transform, isIdentity);
        }

        z0 renderImageTransformed (SavedStateType& state, const Image& src, i32 alpha, const AffineTransform& transform, Graphics::ResamplingQuality quality, b8 tiledFill) const override
        {
            state.renderImageTransformed (edgeTable, src, alpha, transform, quality, tiledFill);
        }

        z0 renderImageUntransformed (SavedStateType& state, const Image& src, i32 alpha, i32 x, i32 y, b8 tiledFill) const override
        {
            state.renderImageUntransformed (edgeTable, src, alpha, x, y, tiledFill);
        }

        EdgeTable edgeTable;

    private:
        template <typename Value>
        z0 fillRectWithColorImpl (SavedStateType& state, Rectangle<Value> area, PixelARGB colour, b8 replace) const
        {
            auto totalClip = edgeTable.getMaximumBounds().template toType<Value>();
            auto clipped = totalClip.getIntersection (area);

            if (clipped.isEmpty())
                return;

            EdgeTableRegion et (clipped);
            et.edgeTable.clipToEdgeTable (edgeTable);
            state.fillWithSolidColor (et.edgeTable, colour, replace);
        }

        template <class SrcPixelType>
        z0 transformedClipImage (const Image::BitmapData& srcData, const AffineTransform& transform, Graphics::ResamplingQuality quality, const SrcPixelType*)
        {
            EdgeTableFillers::TransformedImageFill<SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, transform, 255, quality);

            for (i32 y = 0; y < edgeTable.getMaximumBounds().getHeight(); ++y)
                renderer.clipEdgeTableLine (edgeTable, edgeTable.getMaximumBounds().getX(), y + edgeTable.getMaximumBounds().getY(),
                                            edgeTable.getMaximumBounds().getWidth());
        }

        template <class SrcPixelType>
        z0 straightClipImage (const Image::BitmapData& srcData, i32 imageX, i32 imageY, const SrcPixelType*)
        {
            Rectangle<i32> r (imageX, imageY, srcData.width, srcData.height);
            edgeTable.clipToRectangle (r);

            EdgeTableFillers::ImageFill<SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, 255, imageX, imageY);

            for (i32 y = 0; y < r.getHeight(); ++y)
                renderer.clipEdgeTableLine (edgeTable, r.getX(), y + r.getY(), r.getWidth());
        }
    };

    //==============================================================================
    template <typename SavedStateType>
    class RectangleListRegion  : public Base<SavedStateType>
    {
    public:
        RectangleListRegion (Rectangle<i32> r) : clip (r) {}
        RectangleListRegion (const RectangleList<i32>& r)  : clip (r) {}
        RectangleListRegion (const RectangleListRegion& other) : clip (other.clip) {}

        using Ptr = typename Base<SavedStateType>::Ptr;

        Ptr clone() const override                           { return *new RectangleListRegion (*this); }
        Ptr applyClipTo (const Ptr& target) const override   { return target->clipToRectangleList (clip); }

        Ptr clipToRectangle (Rectangle<i32> r) override
        {
            clip.clipTo (r);
            return clip.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToRectangleList (const RectangleList<i32>& r) override
        {
            clip.clipTo (r);
            return clip.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr excludeClipRectangle (Rectangle<i32> r) override
        {
            clip.subtract (r);
            return clip.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToPath (const Path& p, const AffineTransform& transform) override  { return toEdgeTable()->clipToPath (p, transform); }
        Ptr clipToEdgeTable (const EdgeTable& et) override                         { return toEdgeTable()->clipToEdgeTable (et); }

        Ptr clipToImageAlpha (const Image& image, const AffineTransform& transform, Graphics::ResamplingQuality quality) override
        {
            return toEdgeTable()->clipToImageAlpha (image, transform, quality);
        }

        z0 translate (Point<i32> delta) override                    { clip.offsetAll (delta); }
        b8 clipRegionIntersects (Rectangle<i32> r) const override   { return clip.intersects (r); }
        Rectangle<i32> getClipBounds() const override                 { return clip.getBounds(); }

        z0 fillRectWithColor (SavedStateType& state, Rectangle<i32> area, PixelARGB colour, b8 replaceContents) const override
        {
            SubRectangleIterator iter (clip, area);
            state.fillWithSolidColor (iter, colour, replaceContents);
        }

        z0 fillRectWithColor (SavedStateType& state, Rectangle<f32> area, PixelARGB colour) const override
        {
            SubRectangleIteratorFloat iter (clip, area);
            state.fillWithSolidColor (iter, colour, false);
        }

        z0 fillAllWithColor (SavedStateType& state, PixelARGB colour, b8 replaceContents) const override
        {
            state.fillWithSolidColor (*this, colour, replaceContents);
        }

        z0 fillAllWithGradient (SavedStateType& state, ColorGradient& gradient, const AffineTransform& transform, b8 isIdentity) const override
        {
            state.fillWithGradient (*this, gradient, transform, isIdentity);
        }

        z0 renderImageTransformed (SavedStateType& state, const Image& src, i32 alpha, const AffineTransform& transform, Graphics::ResamplingQuality quality, b8 tiledFill) const override
        {
            state.renderImageTransformed (*this, src, alpha, transform, quality, tiledFill);
        }

        z0 renderImageUntransformed (SavedStateType& state, const Image& src, i32 alpha, i32 x, i32 y, b8 tiledFill) const override
        {
            state.renderImageUntransformed (*this, src, alpha, x, y, tiledFill);
        }

        RectangleList<i32> clip;

        //==============================================================================
        template <class Renderer>
        z0 iterate (Renderer& r) const noexcept
        {
            for (auto& i : clip)
            {
                auto x = i.getX();
                auto w = i.getWidth();
                jassert (w > 0);
                auto bottom = i.getBottom();

                for (i32 y = i.getY(); y < bottom; ++y)
                {
                    r.setEdgeTableYPos (y);
                    r.handleEdgeTableLineFull (x, w);
                }
            }
        }

    private:
        //==============================================================================
        class SubRectangleIterator
        {
        public:
            SubRectangleIterator (const RectangleList<i32>& clipList, Rectangle<i32> clipBounds)
                : clip (clipList), area (clipBounds)
            {}

            template <class Renderer>
            z0 iterate (Renderer& r) const noexcept
            {
                for (auto& i : clip)
                {
                    auto rect = i.getIntersection (area);

                    if (! rect.isEmpty())
                        r.handleEdgeTableRectangleFull (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
                }
            }

        private:
            const RectangleList<i32>& clip;
            const Rectangle<i32> area;

            DRX_DECLARE_NON_COPYABLE (SubRectangleIterator)
        };

        //==============================================================================
        class SubRectangleIteratorFloat
        {
        public:
            SubRectangleIteratorFloat (const RectangleList<i32>& clipList, Rectangle<f32> clipBounds) noexcept
                : clip (clipList), area (clipBounds)
            {
            }

            template <class Renderer>
            z0 iterate (Renderer& r) const noexcept
            {
                const RenderingHelpers::FloatRectangleRasterisingInfo f (area);

                for (auto& i : clip)
                {
                    auto clipLeft   = i.getX();
                    auto clipRight  = i.getRight();
                    auto clipTop    = i.getY();
                    auto clipBottom = i.getBottom();

                    if (f.totalBottom > clipTop && f.totalTop < clipBottom
                         && f.totalRight > clipLeft && f.totalLeft < clipRight)
                    {
                        if (f.isOnePixelWide())
                        {
                            if (f.topAlpha != 0 && f.totalTop >= clipTop)
                            {
                                r.setEdgeTableYPos (f.totalTop);
                                r.handleEdgeTablePixel (f.left, f.topAlpha);
                            }

                            auto y1 = jmax (clipTop, f.top);
                            auto y2 = jmin (f.bottom, clipBottom);
                            auto h = y2 - y1;

                            if (h > 0)
                                r.handleEdgeTableRectangleFull (f.left, y1, 1, h);

                            if (f.bottomAlpha != 0 && f.bottom < clipBottom)
                            {
                                r.setEdgeTableYPos (f.bottom);
                                r.handleEdgeTablePixel (f.left, f.bottomAlpha);
                            }
                        }
                        else
                        {
                            auto clippedLeft   = jmax (f.left, clipLeft);
                            auto clippedWidth  = jmin (f.right, clipRight) - clippedLeft;
                            b8 doLeftAlpha  = f.leftAlpha != 0 && f.totalLeft >= clipLeft;
                            b8 doRightAlpha = f.rightAlpha != 0 && f.right < clipRight;

                            if (f.topAlpha != 0 && f.totalTop >= clipTop)
                            {
                                r.setEdgeTableYPos (f.totalTop);

                                if (doLeftAlpha)        r.handleEdgeTablePixel (f.totalLeft, f.getTopLeftCornerAlpha());
                                if (clippedWidth > 0)   r.handleEdgeTableLine (clippedLeft, clippedWidth, f.topAlpha);
                                if (doRightAlpha)       r.handleEdgeTablePixel (f.right, f.getTopRightCornerAlpha());
                            }

                            auto y1 = jmax (clipTop, f.top);
                            auto y2 = jmin (f.bottom, clipBottom);
                            auto h = y2 - y1;

                            if (h > 0)
                            {
                                if (h == 1)
                                {
                                    r.setEdgeTableYPos (y1);

                                    if (doLeftAlpha)        r.handleEdgeTablePixel (f.totalLeft, f.leftAlpha);
                                    if (clippedWidth > 0)   r.handleEdgeTableLineFull (clippedLeft, clippedWidth);
                                    if (doRightAlpha)       r.handleEdgeTablePixel (f.right, f.rightAlpha);
                                }
                                else
                                {
                                    if (doLeftAlpha)        r.handleEdgeTableRectangle (f.totalLeft, y1, 1, h, f.leftAlpha);
                                    if (clippedWidth > 0)   r.handleEdgeTableRectangleFull (clippedLeft, y1, clippedWidth, h);
                                    if (doRightAlpha)       r.handleEdgeTableRectangle (f.right, y1, 1, h, f.rightAlpha);
                                }
                            }

                            if (f.bottomAlpha != 0 && f.bottom < clipBottom)
                            {
                                r.setEdgeTableYPos (f.bottom);

                                if (doLeftAlpha)        r.handleEdgeTablePixel (f.totalLeft, f.getBottomLeftCornerAlpha());
                                if (clippedWidth > 0)   r.handleEdgeTableLine (clippedLeft, clippedWidth, f.bottomAlpha);
                                if (doRightAlpha)       r.handleEdgeTablePixel (f.right, f.getBottomRightCornerAlpha());
                            }
                        }
                    }
                }
            }

        private:
            const RectangleList<i32>& clip;
            Rectangle<f32> area;

            DRX_DECLARE_NON_COPYABLE (SubRectangleIteratorFloat)
        };

        Ptr toEdgeTable() const   { return *new EdgeTableRegion<SavedStateType> (clip); }

        RectangleListRegion& operator= (const RectangleListRegion&) = delete;
    };
}

//==============================================================================
template <class SavedStateType>
class SavedStateBase
{
public:
    using BaseRegionType           = typename ClipRegions::Base<SavedStateType>;
    using EdgeTableRegionType      = typename ClipRegions::EdgeTableRegion<SavedStateType>;
    using RectangleListRegionType  = typename ClipRegions::RectangleListRegion<SavedStateType>;

    SavedStateBase (Rectangle<i32> initialClip)
        : clip (new RectangleListRegionType (initialClip)),
          interpolationQuality (Graphics::mediumResamplingQuality), transparencyLayerAlpha (1.0f)
    {
    }

    SavedStateBase (const RectangleList<i32>& clipList, Point<i32> origin)
        : clip (new RectangleListRegionType (clipList)), transform (origin),
          interpolationQuality (Graphics::mediumResamplingQuality), transparencyLayerAlpha (1.0f)
    {
    }

    SavedStateBase (const SavedStateBase& other)
        : clip (other.clip), transform (other.transform), fillType (other.fillType),
          interpolationQuality (other.interpolationQuality),
          transparencyLayerAlpha (other.transparencyLayerAlpha)
    {
    }

    SavedStateType& getThis() noexcept  { return *static_cast<SavedStateType*> (this); }

    b8 clipToRectangle (Rectangle<i32> r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.translated (r));
            }
            else if (! transform.isRotated)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.transformed (r));
            }
            else
            {
                Path p;
                p.addRectangle (r);
                clipToPath (p, {});
            }
        }

        return clip != nullptr;
    }

    b8 clipToRectangleList (const RectangleList<i32>& r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();

                if (transform.isIdentity())
                {
                    clip = clip->clipToRectangleList (r);
                }
                else
                {
                    RectangleList<i32> offsetList (r);
                    offsetList.offsetAll (transform.offset);
                    clip = clip->clipToRectangleList (offsetList);
                }
            }
            else
            {
                clipToPath (r.toPath(), {});
            }
        }

        return clip != nullptr;
    }

    b8 excludeClipRectangle (Rectangle<i32> r)
    {
        if (clip != nullptr)
        {
            cloneClipIfMultiplyReferenced();

            if (transform.isOnlyTranslated)
            {
                clip = clip->excludeClipRectangle (transform.translated (r.toFloat()).getLargestIntegerWithin());
            }
            else if (! transform.isRotated)
            {
                clip = clip->excludeClipRectangle (transform.boundsAfterTransform (r.toFloat()).getLargestIntegerWithin());
            }
            else
            {
                Path p;
                p.addRectangle (r.toFloat());
                p.applyTransform (transform.complexTransform);
                p.addRectangle (clip->getClipBounds().toFloat());
                p.setUsingNonZeroWinding (false);
                clip = clip->clipToPath (p, {});
            }
        }

        return clip != nullptr;
    }

    z0 clipToPath (const Path& p, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            cloneClipIfMultiplyReferenced();
            clip = clip->clipToPath (p, transform.getTransformWith (t));
        }
    }

    z0 clipToImageAlpha (const Image& sourceImage, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            if (sourceImage.hasAlphaChannel())
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToImageAlpha (sourceImage, transform.getTransformWith (t), interpolationQuality);
            }
            else
            {
                Path p;
                p.addRectangle (sourceImage.getBounds());
                clipToPath (p, t);
            }
        }
    }

    b8 clipRegionIntersects (Rectangle<i32> r) const
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
                return clip->clipRegionIntersects (transform.translated (r));

            return getClipBounds().intersects (r);
        }

        return false;
    }

    Rectangle<i32> getClipBounds() const
    {
        return clip != nullptr ? transform.deviceSpaceToUserSpace (clip->getClipBounds()).getSmallestIntegerContainer()
                               : Rectangle<i32>();
    }

    z0 setFillType (const FillType& newFill)
    {
        fillType = newFill;
    }

    z0 fillTargetRect (Rectangle<i32> r, b8 replaceContents)
    {
        if (fillType.isColor())
        {
            clip->fillRectWithColor (getThis(), r, fillType.colour.getPixelARGB(), replaceContents);
        }
        else
        {
            auto clipped = clip->getClipBounds().getIntersection (r);

            if (! clipped.isEmpty())
                fillShape (*new RectangleListRegionType (clipped), false);
        }
    }

    z0 fillTargetRect (Rectangle<f32> r)
    {
        if (fillType.isColor())
        {
            clip->fillRectWithColor (getThis(), r, fillType.colour.getPixelARGB());
        }
        else
        {
            auto clipped = clip->getClipBounds().toFloat().getIntersection (r);

            if (! clipped.isEmpty())
                fillShape (*new EdgeTableRegionType (clipped), false);
        }
    }

    template <typename CoordType>
    z0 fillRectAsPath (Rectangle<CoordType> r)
    {
        Path p;
        p.addRectangle (r);
        fillPath (p, {});
    }

    z0 fillRect (Rectangle<i32> r, b8 replaceContents)
    {
        if (r.isEmpty())
            return;

        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                fillTargetRect (transform.translated (r), replaceContents);
            }
            else if (! transform.isRotated)
            {
                if (replaceContents)
                    fillTargetRect (transform.boundsAfterTransform (r.toFloat()).toNearestInt(), true);
                else
                    fillTargetRect (transform.boundsAfterTransform (r.toFloat()));
            }
            else
            {
                jassert (! replaceContents); // not implemented
                fillRectAsPath (r);
            }
        }
    }

    z0 fillRect (Rectangle<f32> r)
    {
        if (r.isEmpty())
            return;

        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
                fillTargetRect (transform.translated (r));
            else if (! transform.isRotated)
                fillTargetRect (transform.boundsAfterTransform (r));
            else
                fillRectAsPath (r);
        }
    }

    z0 fillRectList (const RectangleList<f32>& list)
    {
        if (clip != nullptr)
        {
            if (list.getNumRectangles() == 1)
                return fillRect (*list.begin());

            if (transform.isIdentity())
            {
                fillShape (*new EdgeTableRegionType (list), false);
            }
            else if (! transform.isRotated)
            {
                RectangleList<f32> transformed (list);

                if (transform.isOnlyTranslated)
                    transformed.offsetAll (transform.offset.toFloat());
                else
                    transformed.transformAll (transform.getTransform());

                fillShape (*new EdgeTableRegionType (transformed), false);
            }
            else
            {
                fillPath (list.toPath(), {});
            }
        }
    }

    z0 fillPath (const Path& path, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            auto trans = transform.getTransformWith (t);
            auto clipRect = clip->getClipBounds();

            if (path.getBoundsTransformed (trans).getSmallestIntegerContainer().intersects (clipRect))
                fillShape (*new EdgeTableRegionType (clipRect, path, trans), false);
        }
    }

    z0 fillEdgeTable (const EdgeTable& edgeTable, f32 x, i32 y)
    {
        if (clip != nullptr)
        {
            auto* edgeTableClip = new EdgeTableRegionType (edgeTable);
            edgeTableClip->edgeTable.translate (x, y);

            fillShape (*edgeTableClip, false);
        }
    }

    z0 drawLine (Line<f32> line)
    {
        Path p;
        p.addLineSegment (line, 1.0f);
        fillPath (p, {});
    }

    z0 drawImage (const Image& sourceImage, const AffineTransform& trans)
    {
        if (clip != nullptr && ! fillType.colour.isTransparent())
            renderImage (sourceImage, trans, {});
    }

    static b8 isOnlyTranslationAllowingError (const AffineTransform& t, f32 tolerance) noexcept
    {
        return std::abs (t.mat01) < tolerance
            && std::abs (t.mat10) < tolerance
            && std::abs (t.mat00 - 1.0f) < tolerance
            && std::abs (t.mat11 - 1.0f) < tolerance;
    }

    z0 renderImage (const Image& sourceImage, const AffineTransform& trans, const BaseRegionType* tiledFillClipRegion)
    {
        auto t = transform.getTransformWith (trans);
        auto alpha = fillType.colour.getAlpha();

        if (isOnlyTranslationAllowingError (t, 0.002f))
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            auto tx = (i32) (t.getTranslationX() * 256.0f);
            auto ty = (i32) (t.getTranslationY() * 256.0f);

            if (interpolationQuality == Graphics::lowResamplingQuality || ((tx | ty) & 224) == 0)
            {
                tx = ((tx + 128) >> 8);
                ty = ((ty + 128) >> 8);

                if (tiledFillClipRegion != nullptr)
                {
                    tiledFillClipRegion->renderImageUntransformed (getThis(), sourceImage, alpha, tx, ty, true);
                }
                else
                {
                    Rectangle<i32> area (tx, ty, sourceImage.getWidth(), sourceImage.getHeight());
                    area = area.getIntersection (getThis().getMaximumBounds());

                    if (! area.isEmpty())
                        if (auto c = clip->applyClipTo (*new EdgeTableRegionType (area)))
                            c->renderImageUntransformed (getThis(), sourceImage, alpha, tx, ty, false);
                }

                return;
            }
        }

        if (! t.isSingularity())
        {
            if (tiledFillClipRegion != nullptr)
            {
                tiledFillClipRegion->renderImageTransformed (getThis(), sourceImage, alpha,
                                                             t, interpolationQuality, true);
            }
            else
            {
                Path p;
                p.addRectangle (sourceImage.getBounds());

                if (auto c = clip->clone()->clipToPath (p, t))
                    c->renderImageTransformed (getThis(), sourceImage, alpha,
                                               t, interpolationQuality, false);
            }
        }
    }

    z0 fillShape (typename BaseRegionType::Ptr shapeToFill, b8 replaceContents)
    {
        jassert (clip != nullptr);
        shapeToFill = clip->applyClipTo (shapeToFill);

        if (shapeToFill != nullptr)
        {
            if (fillType.isGradient())
            {
                jassert (! replaceContents); // that option is just for solid colours

                auto g2 = *(fillType.gradient);
                g2.multiplyOpacity (fillType.getOpacity());
                auto t = transform.getTransformWith (fillType.transform).translated (-0.5f, -0.5f);

                b8 isIdentity = t.isOnlyTranslation();

                if (isIdentity)
                {
                    // If our translation doesn't involve any distortion, we can speed it up..
                    g2.point1.applyTransform (t);
                    g2.point2.applyTransform (t);
                    t = {};
                }

                shapeToFill->fillAllWithGradient (getThis(), g2, t, isIdentity);
            }
            else if (fillType.isTiledImage())
            {
                renderImage (fillType.image, fillType.transform, shapeToFill.get());
            }
            else
            {
                shapeToFill->fillAllWithColor (getThis(), fillType.colour.getPixelARGB(), replaceContents);
            }
        }
    }

    z0 cloneClipIfMultiplyReferenced()
    {
        if (clip->getReferenceCount() > 1)
            clip = clip->clone();
    }

    typename BaseRegionType::Ptr clip;
    RenderingHelpers::TranslationOrTransform transform;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;
    f32 transparencyLayerAlpha;
};

//==============================================================================
class SoftwareRendererSavedState  : public SavedStateBase<SoftwareRendererSavedState>
{
    using BaseClass = SavedStateBase<SoftwareRendererSavedState>;

public:
    SoftwareRendererSavedState (const Image& im, Rectangle<i32> clipBounds)
        : BaseClass (clipBounds), image (im)
    {
    }

    SoftwareRendererSavedState (const Image& im, const RectangleList<i32>& clipList, Point<i32> origin)
        : BaseClass (clipList, origin), image (im)
    {
    }

    SoftwareRendererSavedState (const SoftwareRendererSavedState& other) = default;

    SoftwareRendererSavedState* beginTransparencyLayer (f32 opacity)
    {
        auto* s = new SoftwareRendererSavedState (*this);

        if (clip != nullptr)
        {
            auto layerBounds = clip->getClipBounds();

            s->image = Image (Image::ARGB, layerBounds.getWidth(), layerBounds.getHeight(), true);
            s->transparencyLayerAlpha = opacity;
            s->transform.moveOriginInDeviceSpace (-layerBounds.getPosition());
            s->cloneClipIfMultiplyReferenced();
            s->clip->translate (-layerBounds.getPosition());
        }

        return s;
    }

    z0 endTransparencyLayer (SoftwareRendererSavedState& finishedLayerState)
    {
        if (clip != nullptr)
        {
            auto layerBounds = clip->getClipBounds();

            auto g = image.createLowLevelContext();
            g->setOpacity (finishedLayerState.transparencyLayerAlpha);
            g->drawImage (finishedLayerState.image, AffineTransform::translation (layerBounds.getPosition()));
        }
    }

    static z0 clearGlyphCache()
    {
        GlyphCache::getInstance().reset();
    }

    //==============================================================================
    Rectangle<i32> getMaximumBounds() const     { return image.getBounds(); }

    //==============================================================================
    template <typename IteratorType>
    z0 renderImageTransformed (IteratorType& iter, const Image& src, i32 alpha, const AffineTransform& trans, Graphics::ResamplingQuality quality, b8 tiledFill) const
    {
        Image::BitmapData destData (image, Image::BitmapData::readWrite);
        const Image::BitmapData srcData (src, Image::BitmapData::readOnly);
        EdgeTableFillers::renderImageTransformed (iter, destData, srcData, alpha, trans, quality, tiledFill);
    }

    template <typename IteratorType>
    z0 renderImageUntransformed (IteratorType& iter, const Image& src, i32 alpha, i32 x, i32 y, b8 tiledFill) const
    {
        Image::BitmapData destData (image, Image::BitmapData::readWrite);
        const Image::BitmapData srcData (src, Image::BitmapData::readOnly);
        EdgeTableFillers::renderImageUntransformed (iter, destData, srcData, alpha, x, y, tiledFill);
    }

    template <typename IteratorType>
    z0 fillWithSolidColor (IteratorType& iter, PixelARGB colour, b8 replaceContents) const
    {
        Image::BitmapData destData (image, Image::BitmapData::readWrite);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   EdgeTableFillers::renderSolidFill (iter, destData, colour, replaceContents, (PixelARGB*) nullptr); break;
            case Image::RGB:    EdgeTableFillers::renderSolidFill (iter, destData, colour, replaceContents, (PixelRGB*) nullptr); break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:            EdgeTableFillers::renderSolidFill (iter, destData, colour, replaceContents, (PixelAlpha*) nullptr); break;
        }
    }

    template <typename IteratorType>
    z0 fillWithGradient (IteratorType& iter, ColorGradient& gradient, const AffineTransform& trans, b8 isIdentity) const
    {
        HeapBlock<PixelARGB> lookupTable;
        auto numLookupEntries = gradient.createLookupTable (trans, lookupTable);
        jassert (numLookupEntries > 0);

        Image::BitmapData destData (image, Image::BitmapData::readWrite);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   EdgeTableFillers::renderGradient (iter, destData, gradient, trans, lookupTable, numLookupEntries, isIdentity, (PixelARGB*) nullptr); break;
            case Image::RGB:    EdgeTableFillers::renderGradient (iter, destData, gradient, trans, lookupTable, numLookupEntries, isIdentity, (PixelRGB*) nullptr); break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:            EdgeTableFillers::renderGradient (iter, destData, gradient, trans, lookupTable, numLookupEntries, isIdentity, (PixelAlpha*) nullptr); break;
        }
    }

    //==============================================================================
    Image image;
    Font font { FontOptions{} };

private:
    SoftwareRendererSavedState& operator= (const SoftwareRendererSavedState&) = delete;
};

//==============================================================================
template <class StateObjectType>
class SavedStateStack
{
public:
    SavedStateStack (StateObjectType* initialState) noexcept
        : currentState (initialState)
    {}

    SavedStateStack() = default;

    z0 initialise (StateObjectType* state)
    {
        currentState.reset (state);
    }

    inline StateObjectType* operator->() const noexcept     { return currentState.get(); }
    inline StateObjectType& operator*()  const noexcept     { return *currentState; }

    z0 save()
    {
        stack.add (new StateObjectType (*currentState));
    }

    z0 restore()
    {
        if (auto* top = stack.getLast())
        {
            currentState.reset (top);
            stack.removeLast (1, false);
        }
        else
        {
            jassertfalse; // trying to pop with an empty stack!
        }
    }

    z0 beginTransparencyLayer (f32 opacity)
    {
        save();
        currentState.reset (currentState->beginTransparencyLayer (opacity));
    }

    z0 endTransparencyLayer()
    {
        std::unique_ptr<StateObjectType> finishedTransparencyLayer (currentState.release());
        restore();
        currentState->endTransparencyLayer (*finishedTransparencyLayer);
    }

private:
    std::unique_ptr<StateObjectType> currentState;
    OwnedArray<StateObjectType> stack;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavedStateStack)
};

//==============================================================================
template <class SavedStateType>
class StackBasedLowLevelGraphicsContext  : public LowLevelGraphicsContext
{
public:
    explicit StackBasedLowLevelGraphicsContext (zu64 frameIn)
        : frame (frameIn)
    {
    }

    b8 isVectorDevice()                                              const override { return false; }
    Rectangle<i32> getClipBounds()                                     const override { return stack->getClipBounds(); }
    b8 isClipEmpty()                                                 const override { return stack->clip == nullptr; }

    z0 setOrigin (Point<i32> o)                                            override { stack->transform.setOrigin (o); }
    z0 addTransform (const AffineTransform& t)                             override { stack->transform.addTransform (t); }
    f32 getPhysicalPixelScaleFactor() const                                override { return stack->transform.getPhysicalPixelScaleFactor(); }
    b8 clipRegionIntersects (const Rectangle<i32>& r)                      override { return stack->clipRegionIntersects (r); }
    b8 clipToRectangle (const Rectangle<i32>& r)                           override { return stack->clipToRectangle (r); }
    b8 clipToRectangleList (const RectangleList<i32>& r)                   override { return stack->clipToRectangleList (r); }
    z0 excludeClipRectangle (const Rectangle<i32>& r)                      override { stack->excludeClipRectangle (r); }
    z0 clipToPath (const Path& path, const AffineTransform& t)             override { stack->clipToPath (path, t); }
    z0 clipToImageAlpha (const Image& im, const AffineTransform& t)        override { stack->clipToImageAlpha (im, t); }
    z0 saveState()                                                         override { stack.save(); }
    z0 restoreState()                                                      override { stack.restore(); }
    z0 beginTransparencyLayer (f32 opacity)                              override { stack.beginTransparencyLayer (opacity); }
    z0 endTransparencyLayer()                                              override { stack.endTransparencyLayer(); }
    z0 setFill (const FillType& fillType)                                  override { stack->setFillType (fillType); }
    z0 setOpacity (f32 newOpacity)                                       override { stack->fillType.setOpacity (newOpacity); }
    z0 setInterpolationQuality (Graphics::ResamplingQuality quality)       override { stack->interpolationQuality = quality; }
    z0 fillRect (const Rectangle<i32>& r, b8 replace)                    override { stack->fillRect (r, replace); }
    z0 fillRect (const Rectangle<f32>& r)                                override { stack->fillRect (r); }
    z0 fillRectList (const RectangleList<f32>& list)                     override { stack->fillRectList (list); }
    z0 fillPath (const Path& path, const AffineTransform& t)               override { stack->fillPath (path, t); }
    z0 drawImage (const Image& im, const AffineTransform& t)               override { stack->drawImage (im, t); }
    z0 drawLine (const Line<f32>& line)                                  override { stack->drawLine (line); }
    z0 setFont (const Font& newFont)                                       override { stack->font = newFont; }
    const Font& getFont()                                                    override { return stack->font; }
    zu64 getFrameId()                                              const override { return frame; }

    z0 drawGlyphs (Span<u16k> glyphs,
                     Span<const Point<f32>> positions,
                     const AffineTransform& t) override
    {
        jassert (glyphs.size() == positions.size());

        for (const auto [index, glyph] : enumerate (glyphs))
            drawGlyph (glyph, AffineTransform::translation (positions[(size_t) index]).followedBy (t));
    }

protected:
    z0 drawGlyph (u16 i, const AffineTransform& t)
    {
        if (stack->clip == nullptr)
            return;

        const auto [layers, drawPosition] = [&]
        {
            if (t.isOnlyTranslation() && ! stack->transform.isRotated)
            {
                auto& cache = RenderingHelpers::GlyphCache::getInstance();
                const Point pos (t.getTranslationX(), t.getTranslationY());

                if (this->stack->transform.isOnlyTranslated)
                {
                    const auto drawPos = pos + stack->transform.offset.toFloat();
                    return std::tuple (cache.get (stack->font, i), drawPos);
                }

                auto f = stack->font;
                f.setHeight (f.getHeight() * stack->transform.complexTransform.mat11);

                auto xScale = stack->transform.complexTransform.mat00 / stack->transform.complexTransform.mat11;

                if (std::abs (xScale - 1.0f) > 0.01f)
                    f.setHorizontalScale (xScale);

                const auto drawPos = stack->transform.transformed (pos);
                return std::tuple (cache.get (f, i), drawPos);
            }

            const auto fontHeight = stack->font.getHeight();
            const auto fontTransform = AffineTransform::scale (fontHeight * stack->font.getHorizontalScale(),
                                                               fontHeight).followedBy (t);
            const auto fullTransform = stack->transform.getTransformWith (fontTransform);
            return std::tuple (stack->font.getTypefacePtr()->getLayersForGlyph (stack->font.getMetricsKind(), i, fullTransform, fontHeight), Point<f32>{});
        }();

        const auto initialFill = stack->fillType;
        const ScopeGuard scope { [&] { this->stack->setFillType (initialFill); } };

        for (const auto& layer : layers)
        {
            if (auto* colourLayer = std::get_if<ColorLayer> (&layer.layer))
            {
                if (auto fill = colourLayer->colour)
                    stack->setFillType (*fill);

                stack->fillEdgeTable (colourLayer->clip, drawPosition.x, (i32) drawPosition.y);
            }
            else if (auto* imageLayer = std::get_if<ImageLayer> (&layer.layer))
            {
                // The position arguments to fillEdgeTable are in physical screen-space,
                // and do not take the current context transform into account.
                // However, drawImage *does* apply the context transform internally.
                // We apply the inverse context transform here so that after the
                // real context transform is applied, the image will be painted at the
                // physical position specified by drawPosition.
                const auto imageTransform = imageLayer->transform.translated (drawPosition)
                                                                 .followedBy (stack->transform.getTransform().inverted());
                stack->drawImage (imageLayer->image, imageTransform);
            }
        }
    }

    explicit StackBasedLowLevelGraphicsContext (SavedStateType* initialState) : stack (initialState) {}
    StackBasedLowLevelGraphicsContext() = default;

    RenderingHelpers::SavedStateStack<SavedStateType> stack;
    zu64 frame = 0;
};

DRX_END_IGNORE_WARNINGS_MSVC

} // namespace drx::RenderingHelpers
