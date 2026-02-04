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

static auto operator< (const Font& a, const Font& b)
{
    return GraphicsFontHelpers::compareFont (a, b);
}

template <typename T>
static auto operator< (const Rectangle<T>& a, const Rectangle<T>& b)
{
    const auto tie = [] (auto& t) { return std::make_tuple (t.getX(), t.getY(), t.getWidth(), t.getHeight()); };
    return tie (a) < tie (b);
}

static auto operator< (const Justification& a, const Justification& b)
{
    return a.getFlags() < b.getFlags();
}

//==============================================================================
namespace
{
    template <typename ArrangementArgs>
    class GlyphArrangementCache final : public DeletedAtShutdown
    {
    public:
        GlyphArrangementCache() = default;

        ~GlyphArrangementCache() override
        {
            clearSingletonInstance();
        }

        template <typename ConfigureArrangement>
        [[nodiscard]] auto get (ArrangementArgs&& args, ConfigureArrangement&& configureArrangement)
        {
            const ScopedTryLock stl (lock);
            return stl.isLocked() ? cache.get (std::forward<ArrangementArgs> (args), std::forward<ConfigureArrangement> (configureArrangement))
                                  : configureArrangement (args);
        }

        DRX_DECLARE_SINGLETON_INLINE (GlyphArrangementCache<ArrangementArgs>, false)

    private:
        LruCache<ArrangementArgs, GlyphArrangement> cache;
        CriticalSection lock;
    };

    //==============================================================================
    template <typename Type>
    Rectangle<Type> coordsToRectangle (Type x, Type y, Type w, Type h) noexcept
    {
       #if DRX_DEBUG
        constexpr i32 maxVal = 0x3fffffff;

        jassertquiet ((i32) x >= -maxVal && (i32) x <= maxVal
                   && (i32) y >= -maxVal && (i32) y <= maxVal
                   && (i32) w >= 0 && (i32) w <= maxVal
                   && (i32) h >= 0 && (i32) h <= maxVal);
       #endif

        return { x, y, w, h };
    }
}

//==============================================================================
Graphics::Graphics (const Image& imageToDrawOnto)
    : contextHolder (imageToDrawOnto.createLowLevelContext()),
      context (*contextHolder)
{
    jassert (imageToDrawOnto.isValid()); // Can't draw into a null image!
}

Graphics::Graphics (LowLevelGraphicsContext& internalContext) noexcept
    : context (internalContext)
{
}

//==============================================================================
z0 Graphics::resetToDefaultState()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::resetToDefaultState, etw::graphicsKeyword, context.getFrameId());

    saveStateIfPending();
    context.setFill (FillType());
    context.setFont (FontOptions{}.withMetricsKind (TypefaceMetricsKind::legacy));
    context.setInterpolationQuality (Graphics::mediumResamplingQuality);
}

b8 Graphics::isVectorDevice() const
{
    return context.isVectorDevice();
}

b8 Graphics::reduceClipRegion (Rectangle<i32> area)
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::reduceClipRegionRectangle, etw::graphicsKeyword, context.getFrameId(), area)

    saveStateIfPending();
    return context.clipToRectangle (area);
}

b8 Graphics::reduceClipRegion (i32 x, i32 y, i32 w, i32 h)
{
    return reduceClipRegion (coordsToRectangle (x, y, w, h));
}

b8 Graphics::reduceClipRegion (const RectangleList<i32>& clipRegion)
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::reduceClipRegionRectangleList, etw::graphicsKeyword, context.getFrameId(), clipRegion)

    saveStateIfPending();
    return context.clipToRectangleList (clipRegion);
}

b8 Graphics::reduceClipRegion (const Path& path, const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::reduceClipRegionPath, etw::graphicsKeyword, context.getFrameId());

    saveStateIfPending();
    context.clipToPath (path, transform);
    return ! context.isClipEmpty();
}

b8 Graphics::reduceClipRegion (const Image& image, const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::reduceClipRegionImage, etw::graphicsKeyword, context.getFrameId());

    saveStateIfPending();
    context.clipToImageAlpha (image, transform);
    return ! context.isClipEmpty();
}

z0 Graphics::excludeClipRegion (Rectangle<i32> rectangleToExclude)
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::excludeClipRegion, etw::graphicsKeyword, context.getFrameId(), rectangleToExclude);

    saveStateIfPending();
    context.excludeClipRectangle (rectangleToExclude);
}

b8 Graphics::isClipEmpty() const
{
    return context.isClipEmpty();
}

Rectangle<i32> Graphics::getClipBounds() const
{
    return context.getClipBounds();
}

z0 Graphics::saveState()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::saveState, etw::graphicsKeyword, context.getFrameId());

    saveStateIfPending();
    saveStatePending = true;
}

z0 Graphics::restoreState()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::restoreState, etw::graphicsKeyword, context.getFrameId());

    if (saveStatePending)
        saveStatePending = false;
    else
        context.restoreState();
}

z0 Graphics::saveStateIfPending()
{
    if (saveStatePending)
    {
        DRX_SCOPED_TRACE_EVENT_FRAME (etw::saveState, etw::graphicsKeyword, context.getFrameId());

        saveStatePending = false;
        context.saveState();
    }
}

z0 Graphics::setOrigin (Point<i32> newOrigin)
{
    saveStateIfPending();
    context.setOrigin (newOrigin);
}

z0 Graphics::setOrigin (i32 x, i32 y)
{
    setOrigin ({ x, y });
}

z0 Graphics::addTransform (const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::addTransform, etw::graphicsKeyword, context.getFrameId());

    saveStateIfPending();
    context.addTransform (transform);
}

b8 Graphics::clipRegionIntersects (Rectangle<i32> area) const
{
    return context.clipRegionIntersects (area);
}

z0 Graphics::beginTransparencyLayer (f32 layerOpacity)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::beginTransparencyLayer, etw::graphicsKeyword, context.getFrameId());

    saveStateIfPending();
    context.beginTransparencyLayer (layerOpacity);
}

z0 Graphics::endTransparencyLayer()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::endTransparencyLayer, etw::graphicsKeyword, context.getFrameId());

    context.endTransparencyLayer();
}

//==============================================================================
z0 Graphics::setColor (Color newColor)
{
    saveStateIfPending();
    context.setFill (newColor);
}

z0 Graphics::setOpacity (f32 newOpacity)
{
    saveStateIfPending();
    context.setOpacity (newOpacity);
}

z0 Graphics::setGradientFill (const ColorGradient& gradient)
{
    setFillType (gradient);
}

z0 Graphics::setGradientFill (ColorGradient&& gradient)
{
    setFillType (std::move (gradient));
}

z0 Graphics::setTiledImageFill (const Image& imageToUse, i32k anchorX, i32k anchorY, const f32 opacity)
{
    saveStateIfPending();
    context.setFill (FillType (imageToUse, AffineTransform::translation ((f32) anchorX, (f32) anchorY)));
    context.setOpacity (opacity);
}

z0 Graphics::setFillType (const FillType& newFill)
{
    saveStateIfPending();
    context.setFill (newFill);
}

//==============================================================================
z0 Graphics::setFont (const Font& newFont)
{
    saveStateIfPending();
    context.setFont (newFont);
}

z0 Graphics::setFont (const f32 newFontHeight)
{
    setFont (context.getFont().withHeight (newFontHeight));
}

Font Graphics::getCurrentFont() const
{
    return context.getFont();
}

//==============================================================================
z0 Graphics::drawSingleLineText (const Txt& text, i32k startX, i32k baselineY,
                                   Justification justification) const
{
    if (text.isEmpty())
        return;

    // Don't pass any vertical placement flags to this method - they'll be ignored.
    jassert (justification.getOnlyVerticalFlags() == 0);

    auto flags = justification.getOnlyHorizontalFlags();

    if (flags == Justification::right && startX < context.getClipBounds().getX())
        return;

    if (flags == Justification::left && startX > context.getClipBounds().getRight())
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text); }
        b8 operator< (const ArrangementArgs& other) const { return tie() < other.tie(); }

        const Font font;
        const Txt text;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addLineOfText (args.font, args.text, 0.0f, 0.0f);
        return arrangement;
    };

    using Cache = GlyphArrangementCache<ArrangementArgs>;
    ArrangementArgs args { context.getFont(), text };
    const auto arrangement = Cache::getInstance()->get (std::move (args), std::move (configureArrangement));

    const auto transform = std::invoke ([&]
    {
        const auto t = AffineTransform::translation ((f32) startX,
                                                     (f32) baselineY);

        if (flags == Justification::left)
            return t;

        auto w = arrangement.getBoundingBox (0, -1, true).getWidth();

        if ((flags & (Justification::horizontallyCentred | Justification::horizontallyJustified)) != 0)
            w /= 2.0f;

        return t.followedBy (AffineTransform::translation (-w, 0));
    });

    arrangement.draw (*this, transform);
}

z0 Graphics::drawMultiLineText (const Txt& text, i32k startX,
                                  i32k baselineY, i32k maximumLineWidth,
                                  Justification justification, const f32 leading) const
{
    if (text.isEmpty() || startX >= context.getClipBounds().getRight())
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, maximumLineWidth, justification, leading); }
        b8 operator< (const ArrangementArgs& other) const { return tie() < other.tie(); }

        const Font font;
        const Txt text;
        i32k maximumLineWidth;
        const Justification justification;
        const f32 leading;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addJustifiedText (args.font, args.text,
                                      0.0f, 0.0f, (f32) args.maximumLineWidth,
                                      args.justification, args.leading);
        return arrangement;
    };

    ArrangementArgs args { context.getFont(),
                           text,
                           maximumLineWidth,
                           justification,
                           leading };

    using Cache = GlyphArrangementCache<ArrangementArgs>;
    Cache::getInstance()->get (std::move (args), std::move (configureArrangement))
                         .draw (*this, AffineTransform::translation ((f32) startX,
                                                                     (f32) baselineY));
}

z0 Graphics::drawText (const Txt& text, Rectangle<f32> area,
                         Justification justificationType, b8 useEllipsesIfTooBig) const
{
    if (text.isEmpty() || ! context.clipRegionIntersects (area.getSmallestIntegerContainer()))
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, width, height, justificationType, useEllipsesIfTooBig); }
        b8 operator< (const ArrangementArgs& other) const { return tie() < other.tie(); }

        const Font font;
        const Txt text;
        const f32 width;
        const f32 height;
        const Justification justificationType;
        const b8 useEllipsesIfTooBig;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addCurtailedLineOfText (args.font, args.text, 0.0f, 0.0f,
                                            args.width, args.useEllipsesIfTooBig);

        arrangement.justifyGlyphs (0, arrangement.getNumGlyphs(),
                                   0.0f, 0.0f,
                                   args.width, args.height,
                                   args.justificationType);
        return arrangement;
    };

    ArrangementArgs args { context.getFont(),
                           text,
                           area.getWidth(),
                           area.getHeight(),
                           justificationType,
                           useEllipsesIfTooBig };

    using Cache = GlyphArrangementCache<ArrangementArgs>;
    Cache::getInstance()->get (std::move (args), std::move (configureArrangement))
                         .draw (*this, AffineTransform::translation (area.getX(), area.getY()));
}

z0 Graphics::drawText (const Txt& text, Rectangle<i32> area,
                         Justification justificationType, b8 useEllipsesIfTooBig) const
{
    drawText (text, area.toFloat(), justificationType, useEllipsesIfTooBig);
}

z0 Graphics::drawText (const Txt& text, i32 x, i32 y, i32 width, i32 height,
                         Justification justificationType, const b8 useEllipsesIfTooBig) const
{
    drawText (text, coordsToRectangle (x, y, width, height), justificationType, useEllipsesIfTooBig);
}

z0 Graphics::drawFittedText (const Txt& text, Rectangle<i32> area,
                               Justification justification,
                               i32k maximumNumberOfLines,
                               const f32 minimumHorizontalScale) const
{
    if (text.isEmpty() || area.isEmpty() || ! context.clipRegionIntersects (area))
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, width, height, justification, maximumNumberOfLines, minimumHorizontalScale); }
        b8 operator< (const ArrangementArgs& other) const noexcept { return tie() < other.tie(); }

        const Font font;
        const Txt text;
        const f32 width;
        const f32 height;
        const Justification justification;
        i32k maximumNumberOfLines;
        const f32 minimumHorizontalScale;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addFittedText (args.font, args.text,
                                   0.0f, 0.0f,
                                   args.width, args.height,
                                   args.justification,
                                   args.maximumNumberOfLines,
                                   args.minimumHorizontalScale);
        return arrangement;
    };

    ArrangementArgs args { context.getFont(),
                           text,
                           (f32) area.getWidth(),
                           (f32) area.getHeight(),
                           justification,
                           maximumNumberOfLines,
                           minimumHorizontalScale };

    using Cache = GlyphArrangementCache<ArrangementArgs>;
    Cache::getInstance()->get (std::move (args), std::move (configureArrangement))
                         .draw (*this, AffineTransform::translation ((f32) area.getX(),
                                                                     (f32) area.getY()));
}

z0 Graphics::drawFittedText (const Txt& text, i32 x, i32 y, i32 width, i32 height,
                               Justification justification,
                               i32k maximumNumberOfLines,
                               const f32 minimumHorizontalScale) const
{
    drawFittedText (text, coordsToRectangle (x, y, width, height),
                    justification, maximumNumberOfLines, minimumHorizontalScale);
}

//==============================================================================
z0 Graphics::fillRect (Rectangle<i32> r) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::fillRect, etw::graphicsKeyword, context.getFrameId(), r)

    context.fillRect (r, false);
}

z0 Graphics::fillRect (Rectangle<f32> r) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_F32 (etw::fillRect, etw::graphicsKeyword, context.getFrameId(), r)

    context.fillRect (r);
}

z0 Graphics::fillRect (i32 x, i32 y, i32 width, i32 height) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::fillRect, etw::graphicsKeyword, context.getFrameId(), (Rectangle { x, y, width, height }))

    context.fillRect (coordsToRectangle (x, y, width, height), false);
}

z0 Graphics::fillRect (f32 x, f32 y, f32 width, f32 height) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_F32 (etw::fillRect, etw::graphicsKeyword, context.getFrameId(), (Rectangle { x, y, width, height }))

    fillRect (coordsToRectangle (x, y, width, height));
}

z0 Graphics::fillRectList (const RectangleList<f32>& rectangles) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_F32 (etw::fillRectList, etw::graphicsKeyword, context.getFrameId(), rectangles)

    context.fillRectList (rectangles);
}

z0 Graphics::fillRectList (const RectangleList<i32>& rects) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::fillRectList, etw::graphicsKeyword, context.getFrameId(), rects)

    RectangleList<f32> converted;

    for (const auto& r : rects)
        converted.add (r.toFloat());

    context.fillRectList (converted);
}

z0 Graphics::fillAll() const
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::fillAll, etw::graphicsKeyword, context.getFrameId())

    context.fillAll();
}

z0 Graphics::fillAll (Color colourToUse) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::fillAll, etw::graphicsKeyword, context.getFrameId())

    if (! colourToUse.isTransparent())
    {
        context.saveState();
        context.setFill (colourToUse);
        context.fillAll();
        context.restoreState();
    }
}

//==============================================================================
z0 Graphics::fillPath (const Path& path) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::fillPath, etw::graphicsKeyword, context.getFrameId());

    if (! (context.isClipEmpty() || path.isEmpty()))
        context.fillPath (path, AffineTransform());
}

z0 Graphics::fillPath (const Path& path, const AffineTransform& transform) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::fillPath, etw::graphicsKeyword, context.getFrameId())

    if (! (context.isClipEmpty() || path.isEmpty()))
        context.fillPath (path, transform);
}

z0 Graphics::strokePath (const Path& path,
                           const PathStrokeType& strokeType,
                           const AffineTransform& transform) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::strokePath, etw::graphicsKeyword, context.getFrameId())

    if (! (context.isClipEmpty() || path.isEmpty()))
        context.strokePath (path, strokeType, transform);
}

//==============================================================================
z0 Graphics::drawRect (f32 x, f32 y, f32 width, f32 height, f32 lineThickness) const
{
    drawRect (coordsToRectangle (x, y, width, height), lineThickness);
}

z0 Graphics::drawRect (i32 x, i32 y, i32 width, i32 height, i32 lineThickness) const
{
    drawRect (coordsToRectangle (x, y, width, height), lineThickness);
}

z0 Graphics::drawRect (Rectangle<i32> r, i32 lineThickness) const
{
    drawRect (r.toFloat(), (f32) lineThickness);
}

z0 Graphics::drawRect (Rectangle<f32> r, const f32 lineThickness) const
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_F32 (etw::drawRect, etw::graphicsKeyword, context.getFrameId(), r)

    jassert (r.getWidth() >= 0.0f && r.getHeight() >= 0.0f);
    context.drawRect (r, lineThickness);
}

//==============================================================================
z0 Graphics::fillEllipse (Rectangle<f32> area) const
{
    context.fillEllipse (area);
}

z0 Graphics::fillEllipse (f32 x, f32 y, f32 w, f32 h) const
{
    fillEllipse (coordsToRectangle (x, y, w, h));
}

z0 Graphics::drawEllipse (f32 x, f32 y, f32 width, f32 height, f32 lineThickness) const
{
    drawEllipse (coordsToRectangle (x, y, width, height), lineThickness);
}

z0 Graphics::drawEllipse (Rectangle<f32> area, f32 lineThickness) const
{
    context.drawEllipse (area, lineThickness);
}

z0 Graphics::fillRoundedRectangle (f32 x, f32 y, f32 width, f32 height, f32 cornerSize) const
{
    fillRoundedRectangle (coordsToRectangle (x, y, width, height), cornerSize);
}

z0 Graphics::fillRoundedRectangle (Rectangle<f32> r, const f32 cornerSize) const
{
    context.fillRoundedRectangle (r, cornerSize);
}

z0 Graphics::drawRoundedRectangle (f32 x, f32 y, f32 width, f32 height,
                                     f32 cornerSize, f32 lineThickness) const
{
    drawRoundedRectangle (coordsToRectangle (x, y, width, height), cornerSize, lineThickness);
}

z0 Graphics::drawRoundedRectangle (Rectangle<f32> r, f32 cornerSize, f32 lineThickness) const
{
    context.drawRoundedRectangle (r, cornerSize, lineThickness);
}

z0 Graphics::drawArrow (Line<f32> line, f32 lineThickness, f32 arrowheadWidth, f32 arrowheadLength) const
{
    Path p;
    p.addArrow (line, lineThickness, arrowheadWidth, arrowheadLength);
    fillPath (p);
}

z0 Graphics::fillCheckerBoard (Rectangle<f32> area, f32 checkWidth, f32 checkHeight,
                                 Color colour1, Color colour2) const
{
    jassert (checkWidth > 0 && checkHeight > 0); // can't be zero or less!

    if (checkWidth > 0 && checkHeight > 0)
    {
        context.saveState();

        if (colour1 == colour2)
        {
            context.setFill (colour1);
            context.fillRect (area);
        }
        else
        {
            auto clipped = context.getClipBounds().getIntersection (area.getSmallestIntegerContainer());

            if (! clipped.isEmpty())
            {
                i32k checkNumX = (i32) (((f32) clipped.getX() - area.getX()) / checkWidth);
                i32k checkNumY = (i32) (((f32) clipped.getY() - area.getY()) / checkHeight);
                const f32 startX = area.getX() + (f32) checkNumX * checkWidth;
                const f32 startY = area.getY() + (f32) checkNumY * checkHeight;
                const f32 right  = (f32) clipped.getRight();
                const f32 bottom = (f32) clipped.getBottom();

                for (i32 i = 0; i < 2; ++i)
                {
                    i32 cy = i;
                    RectangleList<f32> checks;

                    for (f32 y = startY; y < bottom; y += checkHeight)
                        for (f32 x = startX + (cy++ & 1) * checkWidth; x < right; x += checkWidth * 2.0f)
                            checks.addWithoutMerging ({ x, y, checkWidth, checkHeight });

                    checks.clipTo (area);
                    context.setFill (i == ((checkNumX ^ checkNumY) & 1) ? colour1 : colour2);
                    context.fillRectList (checks);
                }
            }
        }

        context.restoreState();
    }
}

//==============================================================================
z0 Graphics::drawVerticalLine (i32k x, f32 top, f32 bottom) const
{
    if (top < bottom)
        context.fillRect (Rectangle<f32> ((f32) x, top, 1.0f, bottom - top));
}

z0 Graphics::drawHorizontalLine (i32k y, f32 left, f32 right) const
{
    if (left < right)
        context.fillRect (Rectangle<f32> (left, (f32) y, right - left, 1.0f));
}

z0 Graphics::drawLine (Line<f32> line) const
{
    context.drawLine (line);
}

z0 Graphics::drawLine (f32 x1, f32 y1, f32 x2, f32 y2) const
{
    context.drawLine (Line<f32> (x1, y1, x2, y2));
}

z0 Graphics::drawLine (f32 x1, f32 y1, f32 x2, f32 y2, f32 lineThickness) const
{
    drawLine (Line<f32> (x1, y1, x2, y2), lineThickness);
}

z0 Graphics::drawLine (Line<f32> line, const f32 lineThickness) const
{
    context.drawLineWithThickness (line, lineThickness);
}

z0 Graphics::drawDashedLine (Line<f32> line, const f32* dashLengths,
                               i32 numDashLengths, f32 lineThickness, i32 n) const
{
    jassert (n >= 0 && n < numDashLengths); // your start index must be valid!

    const Point<f64> delta ((line.getEnd() - line.getStart()).toDouble());
    const f64 totalLen = delta.getDistanceFromOrigin();

    if (totalLen >= 0.1)
    {
        const f64 onePixAlpha = 1.0 / totalLen;

        for (f64 alpha = 0.0; alpha < 1.0;)
        {
            jassert (dashLengths[n] > 0); // can't have zero-length dashes!

            const f64 lastAlpha = alpha;
            alpha += dashLengths [n] * onePixAlpha;
            n = (n + 1) % numDashLengths;

            if ((n & 1) != 0)
            {
                const Line<f32> segment (line.getStart() + (delta * lastAlpha).toFloat(),
                                           line.getStart() + (delta * jmin (1.0, alpha)).toFloat());

                if (! approximatelyEqual (lineThickness, 1.0f))
                    drawLine (segment, lineThickness);
                else
                    context.drawLine (segment);
            }
        }
    }
}

//==============================================================================
z0 Graphics::setImageResamplingQuality (const Graphics::ResamplingQuality newQuality)
{
    saveStateIfPending();
    context.setInterpolationQuality (newQuality);
}

//==============================================================================
z0 Graphics::drawImageAt (const Image& imageToDraw, i32 x, i32 y, b8 fillAlphaChannel) const
{
    drawImageTransformed (imageToDraw,
                          AffineTransform::translation ((f32) x, (f32) y),
                          fillAlphaChannel);
}

z0 Graphics::drawImage (const Image& imageToDraw, Rectangle<f32> targetArea,
                          RectanglePlacement placementWithinTarget, b8 fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid())
        drawImageTransformed (imageToDraw,
                              placementWithinTarget.getTransformToFit (imageToDraw.getBounds().toFloat(), targetArea),
                              fillAlphaChannelWithCurrentBrush);
}

z0 Graphics::drawImageWithin (const Image& imageToDraw, i32 dx, i32 dy, i32 dw, i32 dh,
                                RectanglePlacement placementWithinTarget, b8 fillAlphaChannelWithCurrentBrush) const
{
    drawImage (imageToDraw, coordsToRectangle (dx, dy, dw, dh).toFloat(),
               placementWithinTarget, fillAlphaChannelWithCurrentBrush);
}

z0 Graphics::drawImage (const Image& imageToDraw,
                          i32 dx, i32 dy, i32 dw, i32 dh,
                          i32 sx, i32 sy, i32 sw, i32 sh,
                          const b8 fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid() && context.clipRegionIntersects (coordsToRectangle (dx, dy, dw, dh)))
        drawImageTransformed (imageToDraw.getClippedImage (coordsToRectangle (sx, sy, sw, sh)),
                              AffineTransform::scale ((f32) dw / (f32) sw, (f32) dh / (f32) sh)
                                              .translated ((f32) dx, (f32) dy),
                              fillAlphaChannelWithCurrentBrush);
}

z0 Graphics::drawImageTransformed (const Image& imageToDraw,
                                     const AffineTransform& transform,
                                     const b8 fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid() && ! context.isClipEmpty())
    {
        if (fillAlphaChannelWithCurrentBrush)
        {
            context.saveState();
            context.clipToImageAlpha (imageToDraw, transform);
            fillAll();
            context.restoreState();
        }
        else
        {
            context.drawImage (imageToDraw, transform);
        }
    }
}

//==============================================================================
Graphics::ScopedSaveState::ScopedSaveState (Graphics& g)  : context (g)
{
    context.saveState();
}

Graphics::ScopedSaveState::~ScopedSaveState()
{
    context.restoreState();
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class GraphicsTests : public UnitTest
{
public:
    GraphicsTests() : UnitTest ("Graphics", UnitTestCategories::graphics) {}

    z0 runTest() override
    {
        beginTest ("Render image subsection");
        {
            const SoftwareImageType softwareImageType;
            const NativeImageType nativeImageType;
            const ImageType* types[] { &softwareImageType, &nativeImageType };

            for (auto* sourceType : types)
                for (auto* targetType : types)
                    renderImageSubsection (*sourceType, *targetType);
        }
    }

private:
    z0 renderImageSubsection (const ImageType& sourceType, const ImageType& targetType)
    {
        const auto sourceColor = Colors::cyan;
        const auto sourceOffset = 49;

        const Image source { Image::ARGB, 50, 50, true, sourceType };
        const Image target { Image::ARGB, 50, 50, true, targetType };

        const auto subsection = source.getClippedImage (Rectangle { sourceOffset, sourceOffset, 1, 1 });

        Image::BitmapData { subsection, Image::BitmapData::writeOnly }.setPixelColor (0, 0, sourceColor);

        {
            // Render the subsection image so that it fills 'target'
            Graphics g { target };
            // Use low resampling quality, because we want to avoid our pixel getting blurry when it's scaled up
            g.setImageResamplingQuality (Graphics::lowResamplingQuality);
            g.drawImage (subsection,
                         0, 0, target.getWidth(), target.getHeight(),
                         0, 0, 1, 1);
        }

        {
            // Check that all pixels in 'target' match the bottom right pixel of 'source'
            const Image::BitmapData bitmap { target, Image::BitmapData::readOnly };

            i32 numFailures = 0;

            for (auto y = 0; y < bitmap.height; ++y)
            {
                for (auto x = 0; x < bitmap.width; ++x)
                {
                    const auto targetColor = bitmap.getPixelColor (x, y);

                    if (targetColor != sourceColor)
                        ++numFailures;
                }
            }

            expect (numFailures == 0);
        }
    }
};

static GraphicsTests graphicsTests;

#endif

} // namespace drx
