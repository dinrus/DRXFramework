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

TextLayout::Glyph::Glyph (i32 glyph, Point<f32> anch, f32 w) noexcept
    : glyphCode (glyph), anchor (anch), width (w)
{
}

//==============================================================================
TextLayout::Run::Run (Range<i32> range, i32 numGlyphsToPreallocate)
    : stringRange (range)
{
    glyphs.ensureStorageAllocated (numGlyphsToPreallocate);
}

Range<f32> TextLayout::Run::getRunBoundsX() const noexcept
{
    Range<f32> range;
    b8 isFirst = true;

    for (auto& glyph : glyphs)
    {
        Range<f32> r (glyph.anchor.x, glyph.anchor.x + glyph.width);

        if (isFirst)
        {
            isFirst = false;
            range = r;
        }
        else
        {
            range = range.getUnionWith (r);
        }
    }

    return range;
}

//==============================================================================
TextLayout::Line::Line (Range<i32> range, Point<f32> o, f32 asc, f32 desc,
                        f32 lead, i32 numRunsToPreallocate)
    : stringRange (range), lineOrigin (o),
      ascent (asc), descent (desc), leading (lead)
{
    runs.ensureStorageAllocated (numRunsToPreallocate);
}

TextLayout::Line::Line (const Line& other)
    : stringRange (other.stringRange), lineOrigin (other.lineOrigin),
      ascent (other.ascent), descent (other.descent), leading (other.leading)
{
    runs.addCopiesOf (other.runs);
}

TextLayout::Line& TextLayout::Line::operator= (const Line& other)
{
    auto copy = other;
    swap (copy);
    return *this;
}

Range<f32> TextLayout::Line::getLineBoundsX() const noexcept
{
    Range<f32> range;
    b8 isFirst = true;

    for (auto* run : runs)
    {
        auto runRange = run->getRunBoundsX();

        if (isFirst)
        {
            isFirst = false;
            range = runRange;
        }
        else
        {
            range = range.getUnionWith (runRange);
        }
    }

    return range + lineOrigin.x;
}

Range<f32> TextLayout::Line::getLineBoundsY() const noexcept
{
    return { lineOrigin.y - ascent,
             lineOrigin.y + descent };
}

Rectangle<f32> TextLayout::Line::getLineBounds() const noexcept
{
    auto x = getLineBoundsX();
    auto y = getLineBoundsY();

    return { x.getStart(), y.getStart(), x.getLength(), y.getLength() };
}

z0 TextLayout::Line::swap (Line& other) noexcept
{
    std::swap (other.runs,          runs);
    std::swap (other.stringRange,   stringRange);
    std::swap (other.lineOrigin,    lineOrigin);
    std::swap (other.ascent,        ascent);
    std::swap (other.descent,       descent);
    std::swap (other.leading,       leading);
}

//==============================================================================
TextLayout::TextLayout()
    : width (0), height (0), justification (Justification::topLeft)
{
}

TextLayout::TextLayout (const TextLayout& other)
    : width (other.width), height (other.height),
      justification (other.justification)
{
    lines.addCopiesOf (other.lines);
}

TextLayout::TextLayout (TextLayout&& other) noexcept
    : lines (std::move (other.lines)),
      width (other.width), height (other.height),
      justification (other.justification)
{
}

TextLayout& TextLayout::operator= (TextLayout&& other) noexcept
{
    lines = std::move (other.lines);
    width = other.width;
    height = other.height;
    justification = other.justification;
    return *this;
}

TextLayout& TextLayout::operator= (const TextLayout& other)
{
    width = other.width;
    height = other.height;
    justification = other.justification;
    lines.clear();
    lines.addCopiesOf (other.lines);
    return *this;
}

TextLayout::~TextLayout()
{
}

TextLayout::Line& TextLayout::getLine (i32 index) const noexcept
{
    return *lines.getUnchecked (index);
}

z0 TextLayout::ensureStorageAllocated (i32 numLinesNeeded)
{
    lines.ensureStorageAllocated (numLinesNeeded);
}

z0 TextLayout::addLine (std::unique_ptr<Line> line)
{
    lines.add (line.release());
}

z0 TextLayout::draw (Graphics& g, Rectangle<f32> area) const
{
    auto origin = justification.appliedToRectangle (Rectangle<f32> (width, getHeight()), area).getPosition();

    auto& context   = g.getInternalContext();
    context.saveState();

    auto clip       = context.getClipBounds();
    auto clipTop    = (f32) clip.getY()      - origin.y;
    auto clipBottom = (f32) clip.getBottom() - origin.y;

    std::vector<u16> glyphNumbers;
    std::vector<Point<f32>> positions;

    for (auto& line : *this)
    {
        auto lineRangeY = line.getLineBoundsY();

        if (lineRangeY.getEnd() < clipTop)
            continue;

        if (lineRangeY.getStart() > clipBottom)
            break;

        auto lineOrigin = origin + line.lineOrigin;

        for (auto* run : line.runs)
        {
            context.setFont (run->font);
            context.setFill (run->colour);

            const auto& glyphs = run->glyphs;

            glyphNumbers.resize ((size_t) glyphs.size());
            std::transform (glyphs.begin(), glyphs.end(), glyphNumbers.begin(), [&] (const Glyph& x)
            {
                return (u16) x.glyphCode;
            });

            positions.resize ((size_t) glyphs.size());
            std::transform (glyphs.begin(), glyphs.end(), positions.begin(), [&] (const Glyph& x)
            {
                return x.anchor;
            });

            context.drawGlyphs (glyphNumbers, positions, AffineTransform::translation (lineOrigin));

            if (run->font.isUnderlined())
            {
                const auto runExtent = run->getRunBoundsX();
                const auto lineThickness = run->font.getDescent() * 0.3f;

                context.fillRect ({ runExtent.getStart() + lineOrigin.x,
                                    lineOrigin.y + lineThickness * 2.0f,
                                    runExtent.getLength(),
                                    lineThickness });
            }
        }
    }

    context.restoreState();
}

z0 TextLayout::createLayout (const AttributedString& text, f32 maxWidth)
{
    createLayout (text, maxWidth, 1.0e7f);
}

z0 TextLayout::createLayout (const AttributedString& text, f32 maxWidth, f32 maxHeight)
{
    lines.clear();
    width = maxWidth;
    height = maxHeight;
    justification = text.getJustification();

    createStandardLayout (text);

    recalculateSize();
}

z0 TextLayout::createLayoutWithBalancedLineLengths (const AttributedString& text, f32 maxWidth)
{
    createLayoutWithBalancedLineLengths (text, maxWidth, 1.0e7f);
}

z0 TextLayout::createLayoutWithBalancedLineLengths (const AttributedString& text, f32 maxWidth, f32 maxHeight)
{
    auto minimumWidth = maxWidth / 2.0f;
    auto bestWidth = maxWidth;
    f32 bestLineProportion = 0.0f;

    while (maxWidth > minimumWidth)
    {
        createLayout (text, maxWidth, maxHeight);

        if (getNumLines() < 2)
            return;

        auto line1 = lines.getUnchecked (lines.size() - 1)->getLineBoundsX().getLength();
        auto line2 = lines.getUnchecked (lines.size() - 2)->getLineBoundsX().getLength();
        auto shortest = jmin (line1, line2);
        auto longest  = jmax (line1, line2);
        auto prop = shortest > 0 ? longest / shortest : 1.0f;

        if (prop > 0.9f && prop < 1.1f)
            return;

        if (prop > bestLineProportion)
        {
            bestLineProportion = prop;
            bestWidth = maxWidth;
        }

        maxWidth -= 10.0f;
    }

    if (! approximatelyEqual (bestWidth, maxWidth))
        createLayout (text, bestWidth, maxHeight);
}

//==============================================================================
template <typename T, typename U>
static auto castTo (const Range<U>& r)
{
    return Range<T> (static_cast<T> (r.getStart()), static_cast<T> (r.getEnd()));
}

static Range<z64> getInputRange (const detail::ShapedText& st, Range<z64> glyphRange)
{
    if (glyphRange.isEmpty())
    {
        jassertfalse;
        return {};
    }

    const auto startInputRange = st.getTextRange (glyphRange.getStart());
    const auto endInputRange   = st.getTextRange (glyphRange.getEnd() - 1);

    // The glyphRange is always in visual order and could have an opposite direction to the text
    return { std::min (startInputRange.getStart(), endInputRange.getStart()),
             std::max (startInputRange.getEnd(), endInputRange.getEnd()) };
}

static Range<z64> getLineInputRange (const detail::ShapedText& st, z64 lineNumber)
{
    using namespace detail;

    return getInputRange (st, st.getSimpleShapedText()
                                .getLineNumbersForGlyphRanges()
                                .getItem ((size_t) lineNumber).range);
}

struct MaxFontAscentAndDescent
{
    f32 ascent{}, descent{};
};

static MaxFontAscentAndDescent getMaxFontAscentAndDescentInEnclosingLine (const detail::ShapedText& st,
                                                                          Range<z64> lineChunkRange)
{
    const auto sst = st.getSimpleShapedText();

    const auto lineRange = sst.getLineNumbersForGlyphRanges()
                              .getItemWithEnclosingRange (lineChunkRange.getStart())->range;

    const auto fonts = sst.getResolvedFonts().getIntersectionsWith (lineRange);

    MaxFontAscentAndDescent result;

    for (const auto pair : fonts)
    {
        result.ascent = std::max (result.ascent, pair.value.getAscent());
        result.descent = std::max (result.descent, pair.value.getDescent());
    }

    return result;
}

static std::optional<detail::TextDirection> getTextDirection (const AttributedString& text)
{
    using namespace detail;

    using ReadingDirection = AttributedString::ReadingDirection;

    const auto dir = text.getReadingDirection();

    if (dir == ReadingDirection::leftToRight)
        return TextDirection::ltr;

    if (dir == ReadingDirection::rightToLeft)
        return TextDirection::rtl;

    return std::nullopt;
}

z0 TextLayout::createStandardLayout (const AttributedString& text)
{
    using namespace detail;

    detail::Ranges::Operations ops;

    RangedValues<Font> fonts;
    RangedValues<Color> colours;

    for (auto i = 0, iMax = text.getNumAttributes(); i < iMax; ++i)
    {
        const auto& attribute = text.getAttribute (i);
        const auto range = castTo<z64> (attribute.range);
        fonts.set (range, attribute.font, ops);
        colours.set (range, attribute.colour, ops);
        ops.clear();
    }

    auto shapedTextOptions = ShapedTextOptions{}.withFonts (fonts)
                                                .withLanguage (SystemStats::getUserLanguage())
                                                .withTrailingWhitespacesShouldFit (false)
                                                .withJustification (justification)
                                                .withReadingDirection (getTextDirection (text))
                                                .withAdditiveLineSpacing (text.getLineSpacing());

    if (text.getWordWrap() != AttributedString::none)
        shapedTextOptions = shapedTextOptions.withMaxWidth (width);

    ShapedText st { text.getText(), shapedTextOptions };

    std::optional<z64> lastLineNumber;
    std::unique_ptr<Line> line;

    st.accessTogetherWith ([&] (Span<const ShapedGlyph> glyphs,
                                Span<const Point<f32>> positions,
                                Font font,
                                Range<z64> glyphRange,
                                LineMetrics lineMetrics,
                                Color colour)
                           {
                               if (std::exchange (lastLineNumber, lineMetrics.lineNumber) != lineMetrics.lineNumber)
                               {
                                   if (line != nullptr)
                                       addLine (std::move (line));

                                   const auto ascentAndDescent = getMaxFontAscentAndDescentInEnclosingLine (st,
                                                                                                            glyphRange);

                                   line = std::make_unique<Line> (castTo<i32> (getLineInputRange (st, lineMetrics.lineNumber)),
                                                                  positions[0],
                                                                  ascentAndDescent.ascent,
                                                                  ascentAndDescent.descent,
                                                                  0.0f,
                                                                  0);
                               }

                               auto run = std::make_unique<Run> (castTo<i32> (getInputRange (st, glyphRange)), 0);

                               run->font = font;
                               run->colour = colour;

                               const auto beyondLastNonWhitespace = [&]
                               {
                                   auto i = glyphs.size();

                                   for (auto it  = std::reverse_iterator { glyphs.end() },
                                             end = std::reverse_iterator { glyphs.begin() };
                                        it != end && it->isWhitespace();
                                        ++it)
                                   {
                                       --i;
                                   }

                                   return i;
                               }();

                               for (size_t i = 0; i < beyondLastNonWhitespace; ++i)
                               {
                                   if (glyphs[i].isPlaceholderForLigature())
                                       continue;

                                   run->glyphs.add ({ (i32) glyphs[i].glyphId,
                                                      positions[i] - line->lineOrigin,
                                                      glyphs[i].advance.x });
                               }

                               line->runs.add (std::move (run));
                           },
                           colours);

    if (line != nullptr)
        addLine (std::move (line));
}

z0 TextLayout::recalculateSize()
{
    if (! lines.isEmpty())
    {
        auto bounds = lines.getFirst()->getLineBounds();

        for (auto* line : lines)
            bounds = bounds.getUnion (line->getLineBounds());

        for (auto* line : lines)
            line->lineOrigin.x -= bounds.getX();

        width  = bounds.getWidth();
        height = bounds.getHeight();
    }
    else
    {
        width = 0;
        height = 0;
    }
}

} // namespace drx
