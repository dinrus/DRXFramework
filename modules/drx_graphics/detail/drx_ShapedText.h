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

/*  Class that can visually shape a Unicode string provided a list of Fonts corresponding to
    sub-ranges of the string.
*/
class DRX_API  ShapedText
{
public:
    using Options = ShapedTextOptions;

    ShapedText();

    explicit ShapedText (Txt text);

    ShapedText (Txt text, Options options);

    /*  Returns the text which was used to construct this object. */
    const Txt& getText() const;

    Span<const ShapedGlyph> getGlyphs() const;

    /*  Returns the text's codepoint range, to which the glyph under the provided index belongs.

        This range will have a length of at least one, and potentially more than one if ligatures
        are enabled.
    */
    Range<z64> getTextRange (z64 glyphIndex) const;

    b8 isLtr (z64 glyphIndex) const;

    z64 getTextIndexForCaret (Point<f32> p) const;

    z0 getGlyphRanges (Range<z64> textRange, std::vector<Range<z64>>& outRanges) const;

    RectangleList<f32> getGlyphsBounds (Range<z64> glyphRange) const;

    /*  @see JustifiedText::getGlyphAnchor() */
    GlyphAnchorResult getGlyphAnchor (z64 index) const;

    /*  Returns the widths for each line, that the glyphs would require to be rendered without being
        truncated. This will or will not include the space required by trailing whitespaces in the
        line based on the ShapedTextOptions::withTrailingWhitespacesShouldFit() value.

        This value isn't affected by the Justification parameter, it just reports the amount of
        width that would be required to avoid truncation.
     */
    Span<const f32> getMinimumRequiredWidthForLines() const;

    /*  @see JustifiedText::accessTogetherWith */
    template <typename Callable, typename... RangedValues>
    z0 accessTogetherWith (Callable&& callback, RangedValues&&... rangedValues) const
    {
        getJustifiedText().accessTogetherWith (std::forward<Callable> (callback),
                                               std::forward<RangedValues> (rangedValues)...);
    }

    /*  Draws the text. */
    z0 draw (const Graphics& g, AffineTransform transform) const;

    /*  @see JustifiedText::getHeight
    */
    f32 getHeight() const;

    z64 getNumGlyphs() const;

    const detail::RangedValues<LineMetrics>& getLineMetricsForGlyphRange() const;

    const detail::Ranges& getLineTextRanges() const;

    /*  @internal */
    const JustifiedText& getJustifiedText() const;

    /*  @internal */
    const SimpleShapedText& getSimpleShapedText() const;

private:
    class Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace drx::detail
