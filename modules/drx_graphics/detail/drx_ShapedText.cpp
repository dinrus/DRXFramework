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

namespace drx::detail
{

class ShapedText::Impl
{
public:
    Impl (Txt textIn, Options optionsIn)
        : options { std::move (optionsIn) },
          text { std::move (textIn) }
    {
    }

    z0 draw (const Graphics& g, AffineTransform transform) const
    {
        drawJustifiedText (justifiedText, g, transform);
    }

    f32 getHeight() const
    {
        return justifiedText.getHeight();
    }

    z64 getNumGlyphs() const
    {
        return simpleShapedText.getNumGlyphs();
    }

    const detail::RangedValues<LineMetrics>& getLineMetricsForGlyphRange() const
    {
        return justifiedText.getLineMetricsForGlyphRange();
    }

    const detail::Ranges& getLineTextRanges() const
    {
        return simpleShapedText.getLineTextRanges();
    }

    auto& getText() const
    {
        return text;
    }

    auto getTextRange (z64 glyphIndex) const
    {
        return simpleShapedText.getTextRange (glyphIndex);
    }

    auto isLtr (z64 glyphIndex) const
    {
        return simpleShapedText.isLtr (glyphIndex);
    }

    z64 getTextIndexForCaret (Point<f32> p) const
    {
        const auto getGlyph = [&] (z64 i)
        {
            return simpleShapedText.getGlyphs()[(size_t) i];
        };

        if (getNumGlyphs() == 0)
            return 0;

        const auto glyphOnTheRight = justifiedText.getGlyphIndexToTheRightOf (p);

        if (glyphOnTheRight >= getNumGlyphs())
        {
            const auto glyphOnTheLeft = glyphOnTheRight - 1;
            const auto ltr = simpleShapedText.getGlyphLookup().find (getGlyph (glyphOnTheLeft).cluster)->value.ltr;

            if (ltr)
                return simpleShapedText.getTextIndexAfterGlyph (glyphOnTheLeft);

            return simpleShapedText.getGlyphs()[(size_t) glyphOnTheLeft].cluster;
        }

        const auto ltr = simpleShapedText.getGlyphLookup().find (getGlyph (glyphOnTheRight).cluster)->value.ltr;

        if (ltr)
            return simpleShapedText.getGlyphs()[(size_t) glyphOnTheRight].cluster;

        return simpleShapedText.getTextIndexAfterGlyph (glyphOnTheRight);
    }

    z0 getGlyphRanges (Range<z64> textRange, std::vector<Range<z64>>& outRanges) const
    {
        simpleShapedText.getGlyphRanges (textRange, outRanges);
    }

    RectangleList<f32> getGlyphsBounds (Range<z64> glyphRange) const
    {
        return justifiedText.getGlyphsBounds (glyphRange);
    }

    auto getGlyphAnchor (z64 index) const
    {
        return justifiedText.getGlyphAnchor (index);
    }

    Span<const f32> getMinimumRequiredWidthForLines() const
    {
        return justifiedText.getMinimumRequiredWidthForLines();
    }

    //==============================================================================
    auto& getSimpleShapedText() const { return simpleShapedText; }

    auto& getJustifiedText() const { return justifiedText; }

private:
    ShapedTextOptions options;
    Txt text;
    SimpleShapedText simpleShapedText { &text, options };
    JustifiedText justifiedText { &simpleShapedText, options };
};

ShapedText::ShapedText() : ShapedText ("", {})
{
}

ShapedText::ShapedText (Txt text) : ShapedText (std::move (text), {})
{
}

ShapedText::ShapedText (Txt text, Options options)
{
    impl = std::make_shared<Impl> (std::move (text), std::move (options));
}

z0 ShapedText::draw (const Graphics& g, AffineTransform transform) const
{
    impl->draw (g, transform);
}

f32 ShapedText::getHeight() const
{
    return impl->getHeight();
}

z64 ShapedText::getNumGlyphs() const
{
    return impl->getNumGlyphs();
}

const detail::RangedValues<LineMetrics>& ShapedText::getLineMetricsForGlyphRange() const
{
    return impl->getLineMetricsForGlyphRange();
}

const detail::Ranges& ShapedText::getLineTextRanges() const
{
    return impl->getLineTextRanges();
}

const Txt& ShapedText::getText() const
{
    return impl->getText();
}

Range<z64> ShapedText::getTextRange (z64 glyphIndex) const
{
    return impl->getTextRange (glyphIndex);
}

b8 ShapedText::isLtr (z64 glyphIndex) const
{
    return impl->isLtr (glyphIndex);
}

z64 ShapedText::getTextIndexForCaret (Point<f32> p) const
{
    return impl->getTextIndexForCaret (p);
}

z0 ShapedText::getGlyphRanges (Range<z64> textRange, std::vector<Range<z64>>& outRanges) const
{
    return impl->getGlyphRanges (textRange, outRanges);
}

RectangleList<f32> ShapedText::getGlyphsBounds (Range<z64> glyphRange) const
{
    return impl->getGlyphsBounds (glyphRange);
}

GlyphAnchorResult ShapedText::getGlyphAnchor (z64 index) const
{
    return impl->getGlyphAnchor (index);
}

Span<const f32> ShapedText::getMinimumRequiredWidthForLines() const
{
    return impl->getMinimumRequiredWidthForLines();
}

const JustifiedText& ShapedText::getJustifiedText() const { return impl->getJustifiedText(); }

const SimpleShapedText& ShapedText::getSimpleShapedText() const { return impl->getSimpleShapedText(); }

} // namespace drx::detail
