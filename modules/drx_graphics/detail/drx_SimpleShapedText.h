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

/** Types of text direction. This may also be applied to characters. */
enum class TextDirection
{
    ltr, // This text reads left to right.
    rtl  // This text reads right to left.
};

class ShapedTextOptions
{
private:
    auto tie() const
    {
        return std::tie (justification,
                         readingDir,
                         maxWidth,
                         alignmentWidth,
                         height,
                         fontsForRange,
                         language,
                         firstLineIndent,
                         leading,
                         additiveLineSpacing,
                         baselineAtZero,
                         allowBreakingInsideWord,
                         trailingWhitespacesShouldFit,
                         maxNumLines,
                         ellipsis);
    }

public:
    b8 operator== (const ShapedTextOptions& other) const { return tie() == other.tie(); }
    b8 operator!= (const ShapedTextOptions& other) const { return tie() != other.tie(); }

    //==============================================================================
    [[nodiscard]] ShapedTextOptions withJustification (Justification x) const
    {
        return withMember (*this, &ShapedTextOptions::justification, x);
    }

    /*  This option will use soft wrapping for lines that are longer than the specified value,
        and it will also align each line to this width, using the Justification provided in
        withJustification.

        The alignment width can be overriden using withAlignmentWidth, but currently we only need
        to do this for the TextEditor.
    */
    [[nodiscard]] ShapedTextOptions withMaxWidth (f32 x) const
    {
        return withMember (*this, &ShapedTextOptions::maxWidth, x);
    }

    /*  With this option each line will be aligned only if it's shorter or equal to the alignment
        width. Otherwise, the line's x anchor will be 0.0f. This is in contrast to using
        withMaxWidth only, which will modify the x anchor of RTL lines that are too i64, to ensure
        that it's the logical end of the text that falls outside the visible bounds.

        The alignment width is also a distinct value from the value used for soft wrapping which is
        specified using withMaxWidth.

        This option is specifically meant to support an existing TextEditor behaviour, where text
        can be aligned even when word wrapping is off. You probably don't need to use this function,
        unless you want to reproduce the particular behaviour seen in the TextEditor, and should
        only use withMaxWidth, if alignment is required.

        With this option off, text is either not aligned, or aligned to the width specified using
        withMaxWidth.

        When this option is in use, it overrides the width specified in withMaxWidth for alignment
        purposes, but not for line wrapping purposes.

        It also accommodates the fact that the TextEditor has a scrolling feature and text never
        becomes unreachable, even if the lines are longer than the viewport's width.
    */
    [[nodiscard]] ShapedTextOptions withAlignmentWidth (f32 x) const
    {
        return withMember (*this, &ShapedTextOptions::alignmentWidth, x);
    }

    [[nodiscard]] ShapedTextOptions withHeight (f32 x) const
    {
        return withMember (*this, &ShapedTextOptions::height, x);
    }

    [[nodiscard]] ShapedTextOptions withFont (Font x) const
    {
        RangedValues<Font> fonts;
        detail::Ranges::Operations ops;
        fonts.set ({ 0, std::numeric_limits<z64>::max() }, x, ops);

        return withMember (*this, &ShapedTextOptions::fontsForRange, std::move (fonts));
    }

    [[nodiscard]] ShapedTextOptions withFonts (const detail::RangedValues<Font>& x) const
    {
        return withMember (*this, &ShapedTextOptions::fontsForRange, x);
    }

    [[nodiscard]] ShapedTextOptions withLanguage (StringRef x) const
    {
        return withMember (*this, &ShapedTextOptions::language, x);
    }

    [[nodiscard]] ShapedTextOptions withFirstLineIndent (f32 x) const
    {
        return withMember (*this, &ShapedTextOptions::firstLineIndent, x);
    }

    /*  This controls the space between lines using a proportional value, with a default of 1.0,
        meaning single line spacing i.e. the descender of the current line + ascender of the next
        line. This value is multiplied by the leading provided here.
    */
    [[nodiscard]] ShapedTextOptions withLeading (f32 x) const
    {
        return withMember (*this, &ShapedTextOptions::leading, x);
    }

    /*  This controls the space between lines using an additive absolute value, with a default of 0.0.
        This value is added to the spacing between each two lines.
    */
    [[nodiscard]] ShapedTextOptions withAdditiveLineSpacing (f32 x) const
    {
        return withMember (*this, &ShapedTextOptions::additiveLineSpacing, x);
    }

    [[nodiscard]] ShapedTextOptions withBaselineAtZero (b8 x = true) const
    {
        return withMember (*this, &ShapedTextOptions::baselineAtZero, x);
    }

    [[nodiscard]] ShapedTextOptions withTrailingWhitespacesShouldFit (b8 x = true) const
    {
        return withMember (*this, &ShapedTextOptions::trailingWhitespacesShouldFit, x);
    }

    [[nodiscard]] ShapedTextOptions withMaxNumLines (z64 x) const
    {
        return withMember (*this, &ShapedTextOptions::maxNumLines, x);
    }

    [[nodiscard]] ShapedTextOptions withEllipsis (Txt x = Txt::charToString ((t32) 0x2026)) const
    {
        return withMember (*this, &ShapedTextOptions::ellipsis, std::move (x));
    }

    [[nodiscard]] ShapedTextOptions withReadingDirection (std::optional<TextDirection> x) const
    {
        return withMember (*this, &ShapedTextOptions::readingDir, x);
    }

    [[nodiscard]] ShapedTextOptions withAllowBreakingInsideWord (b8 x = true) const
    {
        return withMember (*this, &ShapedTextOptions::allowBreakingInsideWord, x);
    }

    const auto& getReadingDirection() const             { return readingDir; }
    const auto& getJustification() const                { return justification; }
    const auto& getMaxWidth() const                     { return maxWidth; }
    const auto& getAlignmentWidth() const               { return alignmentWidth; }
    const auto& getHeight() const                       { return height; }
    const auto& getFontsForRange() const                { return fontsForRange; }
    const auto& getLanguage() const                     { return language; }
    const auto& getFirstLineIndent() const              { return firstLineIndent; }
    const auto& getLeading() const                      { return leading; }
    const auto& getAdditiveLineSpacing() const          { return additiveLineSpacing; }
    const auto& isBaselineAtZero() const                { return baselineAtZero; }
    const auto& getTrailingWhitespacesShouldFit() const { return trailingWhitespacesShouldFit; }
    const auto& getMaxNumLines() const                  { return maxNumLines; }
    const auto& getEllipsis() const                     { return ellipsis; }
    const auto& getAllowBreakingInsideWord() const      { return allowBreakingInsideWord; }

private:
    Justification justification { Justification::topLeft };
    std::optional<TextDirection> readingDir;
    std::optional<f32> maxWidth;
    std::optional<f32> alignmentWidth;
    std::optional<f32> height;

    detail::RangedValues<Font> fontsForRange = std::invoke ([&]
    {
        detail::RangedValues<Font> result;
        detail::Ranges::Operations ops;
        result.set ({ 0, std::numeric_limits<z64>::max() }, FontOptions { 15.0f }, ops);
        return result;
    });

    Txt language = SystemStats::getDisplayLanguage();
    f32 firstLineIndent = 0.0f;
    f32 leading = 1.0f;
    f32 additiveLineSpacing = 0.0f;
    b8 baselineAtZero = false;
    b8 allowBreakingInsideWord = false;
    b8 trailingWhitespacesShouldFit = true;
    z64 maxNumLines = std::numeric_limits<z64>::max();
    Txt ellipsis;
};

struct ShapedGlyph
{
    ShapedGlyph (u32 glyphIdIn,
                 z64 clusterIn,
                 b8 unsafeToBreakIn,
                 b8 whitespaceIn,
                 b8 newlineIn,
                 i8 distanceFromLigatureIn,
                 Point<f32> advanceIn,
                 Point<f32> offsetIn)
        : advance (advanceIn),
          offset (offsetIn),
          cluster (clusterIn),
          glyphId (glyphIdIn),
          unsafeToBreak (unsafeToBreakIn),
          whitespace (whitespaceIn),
          newline (newlineIn),
          distanceFromLigature (distanceFromLigatureIn) {}

    b8 isUnsafeToBreak() const { return unsafeToBreak; }
    b8 isWhitespace() const { return whitespace; }
    b8 isNewline() const { return newline; }

    b8 isNonLigature() const { return distanceFromLigature == 0; }
    b8 isLigature() const { return distanceFromLigature < 0; }
    b8 isPlaceholderForLigature() const { return distanceFromLigature > 0; }

    i8 getDistanceFromLigature() const { return distanceFromLigature; }
    i8 getNumTrailingLigaturePlaceholders() const { return -distanceFromLigature; }

    Point<f32> advance;
    Point<f32> offset;
    z64 cluster;
    u32 glyphId;

private:
    // These are effectively bools, pack into a single i32 once we have more than four flags.
    i8 unsafeToBreak;
    i8 whitespace;
    i8 newline;
    i8 distanceFromLigature;
};

struct GlyphLookupEntry
{
    Range<z64> glyphRange;
    b8 ltr = true;
};

class SimpleShapedText
{
public:
    /*  Shapes and lays out the first contiguous sequence of ranges specified in the fonts
        parameter.
    */
    SimpleShapedText (const Txt* data,
                      const ShapedTextOptions& options);

    const auto& getLineNumbersForGlyphRanges() const { return lineNumbersForGlyphRanges; }

    const auto& getLineTextRanges() const { return lineTextRanges; }

    const auto& getResolvedFonts() const { return resolvedFonts; }

    Range<z64> getTextRange (z64 glyphIndex) const;

    /*  Возвращает true, если the specified glyph is inside to an LTR run.
    */
    b8 isLtr (z64 glyphIndex) const;

    /*  This function may fail to return an out range, even if the provided textRange falls inside
        the string range used for the creation of the ShapedText object.

        This is because the shaping process could fail due to insufficient glyph resolution to the
        point, where it will produce zero glyphs for the provided string.
    */
    z0 getGlyphRanges (Range<z64> textRange, std::vector<Range<z64>>& outRanges) const;

    /*  Returns the input codepoint index that follows the glyph in a logical sense. So for LTR text
        this is the cluster number of the glyph to the right. For RTL text it's the one on the left.

        If there is no subsequent glyph, the returned number is the first Unicode codepoint index
        that isn't covered by the cluster to which the selected glyph belongs, so for the glyph 'o'
        in "hello" this would be 5, given there are no ligatures in use.
    */
    z64 getTextIndexAfterGlyph (z64 glyphIndex) const;

    z64 getNumLines() const { return (z64) lineNumbersForGlyphRanges.getRanges().size(); }
    z64 getNumGlyphs() const { return (z64) glyphsInVisualOrder.size(); }

    t32 getCodepoint (z64 glyphIndex) const;

    Span<const ShapedGlyph> getGlyphs (Range<z64> glyphRange) const;

    Span<const ShapedGlyph> getGlyphs() const;

    const auto& getGlyphLookup() const { return glyphLookup; }

private:
    z0 shape (const Txt& data,
                const ShapedTextOptions& options);

    const Txt& string;
    std::vector<ShapedGlyph> glyphsInVisualOrder;
    detail::RangedValues<z64> lineNumbersForGlyphRanges;
    detail::Ranges lineTextRanges;
    detail::RangedValues<Font> resolvedFonts;
    detail::RangedValues<GlyphLookupEntry> glyphLookup;

    DRX_LEAK_DETECTOR (SimpleShapedText)
};

} // namespace drx::detail
