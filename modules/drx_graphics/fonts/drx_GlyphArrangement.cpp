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

static constexpr b8 isNonBreakingSpace (const t32 c)
{
    return c == 0x00a0
        || c == 0x2007
        || c == 0x202f
        || c == 0x2060;
}

static b8 areAllRequiredWidthsSmallerThanMax (const detail::ShapedText& shapedText, f32 width)
{
    const auto lineWidths = shapedText.getMinimumRequiredWidthForLines();
    return std::all_of (lineWidths.begin(), lineWidths.end(), [width] (auto& w) { return w <= width; });
}

// ShapedText truncates the last line by default, even if it requires larger width than the maximum
// allowed.
static b8 areAllRequiredWidthsExceptTheLastSmallerThanMax (const detail::ShapedText& shapedText, f32 width)
{
    const auto lineWidths = shapedText.getMinimumRequiredWidthForLines();

    if (lineWidths.empty())
        return true;

    return std::all_of (lineWidths.begin(), std::prev (lineWidths.end()), [width] (auto& w) { return w <= width; });
}

PositionedGlyph::PositionedGlyph() noexcept
    : character (0), glyph (0), x (0), y (0), w (0), whitespace (false)
{
}

PositionedGlyph::PositionedGlyph (const Font& font_, t32 character_, i32 glyphNumber,
                                  f32 anchorX, f32 baselineY, f32 width, b8 whitespace_)
    : font (font_), character (character_), glyph (glyphNumber),
      x (anchorX), y (baselineY), w (width), whitespace (whitespace_)
{
}

z0 PositionedGlyph::draw (Graphics& g) const
{
    draw (g, {});
}

z0 PositionedGlyph::draw (Graphics& g, AffineTransform transform) const
{
    if (isWhitespace())
        return;

    auto& context = g.getInternalContext();
    context.setFont (font);
    u16k glyphs[] { static_cast<u16> (glyph) };
    const Point<f32> positions[] { Point { x, y } };
    context.drawGlyphs (glyphs, positions, transform);
}

z0 PositionedGlyph::createPath (Path& path) const
{
    if (! isWhitespace())
    {
        if (auto t = font.getTypefacePtr())
        {
            Path p;
            t->getOutlineForGlyph (font.getMetricsKind(), glyph, p);

            path.addPath (p, AffineTransform::scale (font.getHeight() * font.getHorizontalScale(), font.getHeight())
                                             .translated (x, y));
        }
    }
}

b8 PositionedGlyph::hitTest (f32 px, f32 py) const
{
    if (getBounds().contains (px, py) && ! isWhitespace())
    {
        if (auto t = font.getTypefacePtr())
        {
            Path p;
            t->getOutlineForGlyph (font.getMetricsKind(), glyph, p);

            AffineTransform::translation (-x, -y)
                            .scaled (1.0f / (font.getHeight() * font.getHorizontalScale()), 1.0f / font.getHeight())
                            .transformPoint (px, py);

            return p.contains (px, py);
        }
    }

    return false;
}

z0 PositionedGlyph::moveBy (f32 deltaX, f32 deltaY)
{
    x += deltaX;
    y += deltaY;
}


//==============================================================================
GlyphArrangement::GlyphArrangement()
{
    glyphs.ensureStorageAllocated (128);
}

//==============================================================================
z0 GlyphArrangement::clear()
{
    glyphs.clear();
}

PositionedGlyph& GlyphArrangement::getGlyph (i32 index) noexcept
{
    return glyphs.getReference (index);
}

//==============================================================================
z0 GlyphArrangement::addGlyphArrangement (const GlyphArrangement& other)
{
    glyphs.addArray (other.glyphs);
}

z0 GlyphArrangement::addGlyph (const PositionedGlyph& glyph)
{
    glyphs.add (glyph);
}

z0 GlyphArrangement::removeRangeOfGlyphs (i32 startIndex, i32 num)
{
    glyphs.removeRange (startIndex, num < 0 ? glyphs.size() : num);
}

//==============================================================================
z0 GlyphArrangement::addLineOfText (const Font& font, const Txt& text, f32 xOffset, f32 yOffset)
{
    addCurtailedLineOfText (font, text, xOffset, yOffset, 1.0e10f, false);
}

static z0 addGlyphsFromShapedText (GlyphArrangement& ga, const detail::ShapedText& st, f32 x, f32 y)
{
    st.accessTogetherWith ([&] (auto shapedGlyphs, auto positions, auto font, auto glyphRange, auto)
                           {
                               for (size_t i = 0; i < shapedGlyphs.size(); ++i)
                               {
                                   const auto glyphIndex = (z64) i + glyphRange.getStart();

                                   auto& glyph = shapedGlyphs[i];
                                   auto& position = positions[i];

                                   if (glyph.isPlaceholderForLigature())
                                       continue;

                                   PositionedGlyph pg { font,
                                                        st.getText()[(i32) st.getTextRange (glyphIndex).getStart()],
                                                        (i32) glyph.glyphId,
                                                        position.getX() + x,
                                                        position.getY() + y,
                                                        glyph.advance.getX(),
                                                        glyph.isWhitespace() };

                                   ga.addGlyph (std::move (pg));
                               }
                           });
}

z0 GlyphArrangement::addCurtailedLineOfText (const Font& font, const Txt& text,
                                               f32 xOffset, f32 yOffset,
                                               f32 maxWidthPixels, b8 useEllipsis)
{
    using namespace detail;

    auto options = ShapedText::Options{}.withMaxNumLines (1)
                                        .withMaxWidth (maxWidthPixels)
                                        .withFont (font)
                                        .withBaselineAtZero()
                                        .withTrailingWhitespacesShouldFit (false);

    if (useEllipsis)
        options = options.withEllipsis();

    ShapedText st { text, options };

    addGlyphsFromShapedText (*this, st, xOffset, yOffset);
}

z0 GlyphArrangement::addJustifiedText (const Font& font, const Txt& text,
                                         f32 x, f32 y, f32 maxLineWidth,
                                         Justification horizontalLayout,
                                         f32 leading)
{
    using namespace detail;

    ShapedText st { text, ShapedText::Options{}.withMaxWidth (maxLineWidth)
                                               .withJustification (horizontalLayout)
                                               .withFont (font)
                                               .withLeading (1.0f + leading / font.getHeight())
                                               .withBaselineAtZero()
                                               .withTrailingWhitespacesShouldFit (false) };

    addGlyphsFromShapedText (*this, st, x, y);
}

static auto createFittedText (const Font& f,
                              const Txt& text,
                              f32 width,
                              f32 height,
                              Justification layout,
                              i32 maximumLines,
                              f32 minimumRelativeHorizontalScale,
                              detail::ShapedText::Options baseOptions = {})
{
    using namespace detail;

    if (! layout.testFlags (Justification::bottom | Justification::top))
        layout = layout.getOnlyHorizontalFlags() | Justification::verticallyCentred;

    if (approximatelyEqual (minimumRelativeHorizontalScale, 0.0f))
        minimumRelativeHorizontalScale = Font::getDefaultMinimumHorizontalScaleFactor();

    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (0 < minimumRelativeHorizontalScale && minimumRelativeHorizontalScale <= 1.0f);

    if (text.containsAnyOf ("\r\n"))
    {
        ShapedText st { text,
                        baseOptions
                            .withMaxWidth (width)
                            .withHeight (height)
                            .withJustification (layout)
                            .withFont (f)
                            .withTrailingWhitespacesShouldFit (false) };

        return st;
    }

    const auto trimmed = text.trim();

    constexpr auto widthFittingTolerance = 0.01f;

    // First attempt: try to squash the entire text on a single line
    {
        ShapedText st { trimmed, baseOptions.withFont (f)
                                            .withMaxWidth (width)
                                            .withHeight (height)
                                            .withMaxNumLines (1)
                                            .withJustification (layout)
                                            .withTrailingWhitespacesShouldFit (false) };

        const auto requiredWidths = st.getMinimumRequiredWidthForLines();

        if (requiredWidths.empty() || requiredWidths.front() <= width)
            return st;

        // If we can fit the entire line, squash by just enough and insert
        if (requiredWidths.front() * minimumRelativeHorizontalScale < width)
        {
            const auto requiredRelativeScale = width / (requiredWidths.front() + widthFittingTolerance);

            ShapedText squashed { trimmed,
                                  baseOptions
                                      .withFont (f.withHorizontalScale (f.getHorizontalScale() * requiredRelativeScale))
                                      .withMaxWidth (width)
                                      .withHeight (height)
                                      .withJustification (layout)
                                      .withTrailingWhitespacesShouldFit (false)};

            return squashed;
        }
    }

    const auto minimumHorizontalScale = minimumRelativeHorizontalScale * f.getHorizontalScale();

    if (maximumLines <= 1)
    {
        ShapedText squashed { trimmed,
                              baseOptions
                                  .withFont (f.withHorizontalScale (minimumHorizontalScale))
                                  .withMaxWidth (width)
                                  .withHeight (height)
                                  .withJustification (layout)
                                  .withMaxNumLines (1)
                                  .withEllipsis() };

        return squashed;
    }

    // Keep reshaping the text constantly decreasing the fontsize and increasing the number of lines
    // until all text fits.

    auto length  = trimmed.length();
    i32 numLines = 1;

    if (length <= 12 && ! trimmed.containsAnyOf (" -\t\r\n"))
        maximumLines = 1;

    maximumLines = jmin (maximumLines, length);

    auto font = f;
    auto cumulativeLineLengths = font.getHeight() * 1.4f;

    while (numLines < maximumLines)
    {
        ++numLines;
        auto newFontHeight = height / (f32) numLines;

        if (newFontHeight < font.getHeight())
            font.setHeight (jmax (8.0f, newFontHeight));

        ShapedText squashed { trimmed,
                              baseOptions
                                  .withFont (font)
                                  .withMaxWidth (width)
                                  .withHeight (height)
                                  .withMaxNumLines (numLines)
                                  .withJustification (layout)
                                  .withTrailingWhitespacesShouldFit (false) };

        if (areAllRequiredWidthsSmallerThanMax (squashed, width))
            return squashed;

        const auto lineWidths = squashed.getMinimumRequiredWidthForLines();

        // We're trying to guess how much horizontal space the text would need to fit, and we
        // need to have an allowance for line end whitespaces which take up additional room
        // when not falling at the end of lines.
        cumulativeLineLengths = std::accumulate (lineWidths.begin(), lineWidths.end(), 0.0f)
                                + font.getHeight() * (f32) numLines * 1.4f;

        if (newFontHeight < 8.0f)
            break;
    }

    //==============================================================================
    // We run an iterative interval halving algorithm to find the largest scale that can fit all
    // text
    auto makeShapedText = [&] (f32 horizontalScale)
    {
        return ShapedText { trimmed,
                            baseOptions
                                .withFont (font.withHorizontalScale (horizontalScale))
                                .withMaxWidth (width)
                                .withHeight (height)
                                .withMaxNumLines (numLines)
                                .withJustification (layout)
                                .withTrailingWhitespacesShouldFit (false)
                                .withEllipsis() };
    };

    auto lowerScaleBound = minimumHorizontalScale;
    auto upperScaleBound = jlimit (minimumHorizontalScale,
                                   1.0f,
                                   (f32) numLines * width / cumulativeLineLengths);

    if (auto st = makeShapedText (upperScaleBound);
        areAllRequiredWidthsSmallerThanMax (st, width)
        || approximatelyEqual (upperScaleBound, minimumHorizontalScale))
    {
        return st;
    }

    struct Candidate
    {
        f32 scale{};
        ShapedText shapedText;
    };

    Candidate candidate { minimumHorizontalScale, makeShapedText (minimumHorizontalScale) };

    for (i32 i = 0, numApproximatingIterations = 3; i < numApproximatingIterations; ++i)
    {
        auto scale = jmap (0.5f, lowerScaleBound, upperScaleBound);

        if (auto st = makeShapedText (scale);
            areAllRequiredWidthsSmallerThanMax (st, width))
        {
            lowerScaleBound = std::max (lowerScaleBound, scale);

            if (candidate.scale < scale)
            {
                candidate.scale = scale;
                candidate.shapedText = std::move (st);
            }
        }
        else
        {
            upperScaleBound = std::min (upperScaleBound, scale);
        }
    }

    const auto scalePerfectlyFittingTheLongestLine = [&]
    {
        const auto lineWidths = candidate.shapedText.getMinimumRequiredWidthForLines();
        const auto greatestLineWidth = std::accumulate (lineWidths.begin(),
                                                        lineWidths.end(),
                                                        0.0f,
                                                        [] (auto acc, auto w) { return std::max (acc, w); });

        if (exactlyEqual (greatestLineWidth, 0.0f))
            return candidate.scale;

        return jlimit (candidate.scale,
                       1.0f,
                       candidate.scale * width / (greatestLineWidth + widthFittingTolerance));
    }();

    if (candidate.scale < scalePerfectlyFittingTheLongestLine)
    {
        if (auto st = makeShapedText (scalePerfectlyFittingTheLongestLine);
            areAllRequiredWidthsSmallerThanMax (st, width))
        {
            candidate.scale = scalePerfectlyFittingTheLongestLine;
            candidate.shapedText = std::move (st);
        }
    }

    return candidate.shapedText;
}

z0 GlyphArrangement::addFittedText (const Font& f,
                                      const Txt& text,
                                      f32 x,
                                      f32 y,
                                      f32 width,
                                      f32 height,
                                      Justification layout,
                                      i32 maximumLines,
                                      f32 minimumHorizontalScale)
{
    const auto st = createFittedText (f, text, width, height, layout, maximumLines, minimumHorizontalScale);

    // ShapedText has the feature for visually truncating the last line, and createFittedText() uses
    // it. Hence if it's only the last line that requires a larger width, ShapedText will take care
    // of it. If lines other than the last one require more width than the minimum, it means they
    // contain a single unbreakable word, and the shaping needs to be redone with breaks inside
    // words allowed.
    if (areAllRequiredWidthsExceptTheLastSmallerThanMax (st, width))
    {
        addGlyphsFromShapedText (*this, st, x, y);
        return;
    }

    const auto stWithWordBreaks = createFittedText (f,
                                                    text,
                                                    width,
                                                    height,
                                                    layout,
                                                    maximumLines,
                                                    minimumHorizontalScale,
                                                    detail::ShapedText::Options{}.withAllowBreakingInsideWord());

    addGlyphsFromShapedText (*this, stWithWordBreaks, x, y);
}

//==============================================================================
z0 GlyphArrangement::moveRangeOfGlyphs (i32 startIndex, i32 num, const f32 dx, const f32 dy)
{
    jassert (startIndex >= 0);

    if (! approximatelyEqual (dx, 0.0f) || ! approximatelyEqual (dy, 0.0f))
    {
        if (num < 0 || startIndex + num > glyphs.size())
            num = glyphs.size() - startIndex;

        while (--num >= 0)
            glyphs.getReference (startIndex++).moveBy (dx, dy);
    }
}

z0 GlyphArrangement::stretchRangeOfGlyphs (i32 startIndex, i32 num, f32 horizontalScaleFactor)
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    if (num > 0)
    {
        auto xAnchor = glyphs.getReference (startIndex).getLeft();

        while (--num >= 0)
        {
            auto& pg = glyphs.getReference (startIndex++);

            pg.x = xAnchor + (pg.x - xAnchor) * horizontalScaleFactor;
            pg.font.setHorizontalScale (pg.font.getHorizontalScale() * horizontalScaleFactor);
            pg.w *= horizontalScaleFactor;
        }
    }
}

Rectangle<f32> GlyphArrangement::getBoundingBox (i32 startIndex, i32 num, b8 includeWhitespace) const
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    Rectangle<f32> result;

    while (--num >= 0)
    {
        auto& pg = glyphs.getReference (startIndex++);

        if (includeWhitespace || ! pg.isWhitespace())
            result = result.getUnion (pg.getBounds());
    }

    return result;
}

z0 GlyphArrangement::justifyGlyphs (i32 startIndex, i32 num,
                                      f32 x, f32 y, f32 width, f32 height,
                                      Justification justification)
{
    jassert (num >= 0 && startIndex >= 0);

    if (glyphs.size() > 0 && num > 0)
    {
        auto bb = getBoundingBox (startIndex, num, ! justification.testFlags (Justification::horizontallyJustified
                                                                               | Justification::horizontallyCentred));
        f32 deltaX = x, deltaY = y;

        if (justification.testFlags (Justification::horizontallyJustified))     deltaX -= bb.getX();
        else if (justification.testFlags (Justification::horizontallyCentred))  deltaX += (width - bb.getWidth()) * 0.5f - bb.getX();
        else if (justification.testFlags (Justification::right))                deltaX += width - bb.getRight();
        else                                                                    deltaX -= bb.getX();

        if (justification.testFlags (Justification::top))                       deltaY -= bb.getY();
        else if (justification.testFlags (Justification::bottom))               deltaY += height - bb.getBottom();
        else                                                                    deltaY += (height - bb.getHeight()) * 0.5f - bb.getY();

        moveRangeOfGlyphs (startIndex, num, deltaX, deltaY);

        if (justification.testFlags (Justification::horizontallyJustified))
        {
            i32 lineStart = 0;
            auto baseY = glyphs.getReference (startIndex).getBaselineY();

            i32 i;
            for (i = 0; i < num; ++i)
            {
                auto glyphY = glyphs.getReference (startIndex + i).getBaselineY();

                if (! approximatelyEqual (glyphY, baseY))
                {
                    spreadOutLine (startIndex + lineStart, i - lineStart, width);

                    lineStart = i;
                    baseY = glyphY;
                }
            }

            if (i > lineStart)
                spreadOutLine (startIndex + lineStart, i - lineStart, width);
        }
    }
}

z0 GlyphArrangement::spreadOutLine (i32 start, i32 num, f32 targetWidth)
{
    if (start + num < glyphs.size()
         && glyphs.getReference (start + num - 1).getCharacter() != '\r'
         && glyphs.getReference (start + num - 1).getCharacter() != '\n')
    {
        i32 numSpaces = 0;
        i32 spacesAtEnd = 0;

        for (i32 i = 0; i < num; ++i)
        {
            if (glyphs.getReference (start + i).isWhitespace())
            {
                ++spacesAtEnd;
                ++numSpaces;
            }
            else
            {
                spacesAtEnd = 0;
            }
        }

        numSpaces -= spacesAtEnd;

        if (numSpaces > 0)
        {
            auto startX = glyphs.getReference (start).getLeft();
            auto endX   = glyphs.getReference (start + num - 1 - spacesAtEnd).getRight();

            auto extraPaddingBetweenWords = (targetWidth - (endX - startX)) / (f32) numSpaces;
            f32 deltaX = 0.0f;

            for (i32 i = 0; i < num; ++i)
            {
                glyphs.getReference (start + i).moveBy (deltaX, 0.0f);

                if (glyphs.getReference (start + i).isWhitespace())
                    deltaX += extraPaddingBetweenWords;
            }
        }
    }
}

//==============================================================================
z0 GlyphArrangement::drawGlyphUnderline (const Graphics& g,
                                           i32 i,
                                           AffineTransform transform) const
{
    const auto pg = glyphs.getReference (i);

    if (! pg.font.isUnderlined())
        return;

    const auto lineThickness = (pg.font.getDescent()) * 0.3f;

    auto nextX = pg.x + pg.w;

    if (i < glyphs.size() - 1 && approximatelyEqual (glyphs.getReference (i + 1).y, pg.y))
        nextX = glyphs.getReference (i + 1).x;

    Path p;
    p.addRectangle (pg.x, pg.y + lineThickness * 2.0f, nextX - pg.x, lineThickness);
    g.fillPath (p, transform);
}

z0 GlyphArrangement::draw (const Graphics& g) const
{
    draw (g, {});
}

z0 GlyphArrangement::draw (const Graphics& g, AffineTransform transform) const
{
    std::vector<u16> glyphNumbers;
    std::vector<Point<f32>> positions;

    glyphNumbers.reserve (static_cast<size_t> (glyphs.size()));
    positions.reserve (static_cast<size_t> (glyphs.size()));

    auto& ctx = g.getInternalContext();
    ctx.saveState();
    const ScopeGuard guard { [&ctx] { ctx.restoreState(); } };

    for (auto it = glyphs.begin(), end = glyphs.end(); it != end;)
    {
        const auto adjacent = std::adjacent_find (it, end, [] (const auto& a, const auto& b)
        {
            return a.font != b.font;
        });

        const auto next = adjacent + (adjacent == end ? 0 : 1);

        glyphNumbers.clear();
        std::transform (it, next, std::back_inserter (glyphNumbers), [] (const PositionedGlyph& x)
        {
            return (u16) x.glyph;
        });

        positions.clear();
        std::transform (it, next, std::back_inserter (positions), [] (const PositionedGlyph& x)
        {
            return Point { x.x, x.y };
        });

        ctx.setFont (it->font);
        ctx.drawGlyphs (glyphNumbers, positions, transform);

        it = next;
    }

    for (const auto pair : enumerate (glyphs))
        drawGlyphUnderline (g, static_cast<i32> (pair.index), transform);
}

z0 GlyphArrangement::createPath (Path& path) const
{
    for (auto& g : glyphs)
        g.createPath (path);
}

i32 GlyphArrangement::findGlyphIndexAt (f32 x, f32 y) const
{
    for (i32 i = 0; i < glyphs.size(); ++i)
        if (glyphs.getReference (i).hitTest (x, y))
            return i;

    return -1;
}

} // namespace drx
