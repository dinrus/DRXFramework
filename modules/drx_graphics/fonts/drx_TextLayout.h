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
    A Pre-formatted piece of text, which may contain multiple fonts and colours.

    A TextLayout is created from an AttributedString, and once created can be
    quickly drawn into a Graphics context.

    @see AttributedString

    @tags{Graphics}
*/
class DRX_API  TextLayout  final
{
private:
    template <typename Iterator>
    class DereferencingIterator
    {
    public:
        using value_type = std::remove_reference_t<decltype (**std::declval<Iterator>())>;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;

        explicit DereferencingIterator (Iterator in) : iterator (std::move (in)) {}

        DereferencingIterator& operator+= (difference_type distance)
        {
            iterator += distance;
            return *this;
        }

        friend DereferencingIterator operator+ (DereferencingIterator i, difference_type d) { return i += d; }
        friend DereferencingIterator operator+ (difference_type d, DereferencingIterator i) { return i += d; }

        DereferencingIterator& operator-= (difference_type distance)
        {
            iterator -= distance;
            return *this;
        }

        friend DereferencingIterator operator- (DereferencingIterator i, difference_type d) { return i -= d; }

        friend difference_type operator- (DereferencingIterator a, DereferencingIterator b)   { return a.iterator - b.iterator; }

        reference operator[] (difference_type d) const { return *iterator[d]; }

        friend b8 operator<  (DereferencingIterator a, DereferencingIterator b) { return a.iterator <  b.iterator; }
        friend b8 operator<= (DereferencingIterator a, DereferencingIterator b) { return a.iterator <= b.iterator; }
        friend b8 operator>  (DereferencingIterator a, DereferencingIterator b) { return a.iterator >  b.iterator; }
        friend b8 operator>= (DereferencingIterator a, DereferencingIterator b) { return a.iterator >= b.iterator; }
        friend b8 operator== (DereferencingIterator a, DereferencingIterator b) { return a.iterator == b.iterator; }
        friend b8 operator!= (DereferencingIterator a, DereferencingIterator b) { return a.iterator != b.iterator; }

        DereferencingIterator& operator++()           { ++iterator; return *this; }
        DereferencingIterator& operator--()           { --iterator; return *this; }
        DereferencingIterator  operator++ (i32) const { DereferencingIterator copy (*this); ++(*this); return copy; }
        DereferencingIterator  operator-- (i32) const { DereferencingIterator copy (*this); --(*this); return copy; }

        reference operator*()  const { return **iterator; }
        pointer   operator->() const { return  *iterator; }

    private:
        Iterator iterator;
    };

public:
    /** Creates an empty layout.
        Having created a TextLayout, you can populate it using createLayout() or
        createLayoutWithBalancedLineLengths().
    */
    TextLayout();
    TextLayout (const TextLayout&);
    TextLayout& operator= (const TextLayout&);
    TextLayout (TextLayout&&) noexcept;
    TextLayout& operator= (TextLayout&&) noexcept;

    /** Destructor. */
    ~TextLayout();

    //==============================================================================
    /** Creates a layout from the given attributed string.
        This will replace any data that is currently stored in the layout.
    */
    z0 createLayout (const AttributedString&, f32 maxWidth);

    /** Creates a layout from the given attributed string, given some size constraints.
        This will replace any data that is currently stored in the layout.
    */
    z0 createLayout (const AttributedString&, f32 maxWidth, f32 maxHeight);

    /** Creates a layout, attempting to choose a width which results in lines
        of a similar length.

        This will be slower than the normal createLayout method, but produces a
        tidier result.
    */
    z0 createLayoutWithBalancedLineLengths (const AttributedString&, f32 maxWidth);

    /** Creates a layout, attempting to choose a width which results in lines
        of a similar length.

        This will be slower than the normal createLayout method, but produces a
        tidier result.
    */
    z0 createLayoutWithBalancedLineLengths (const AttributedString&, f32 maxWidth, f32 maxHeight);

    /** Draws the layout within the specified area.
        The position of the text within the rectangle is controlled by the justification
        flags set in the original AttributedString that was used to create this layout.
    */
    z0 draw (Graphics&, Rectangle<f32> area) const;

    //==============================================================================
    /** A positioned glyph. */
    class DRX_API  Glyph
    {
    public:
        Glyph (i32 glyphCode, Point<f32> anchor, f32 width) noexcept;

        /** The code number of this glyph. */
        i32 glyphCode;

        /** The glyph's anchor point - this is relative to the line's origin.
            @see TextLayout::Line::lineOrigin
        */
        Point<f32> anchor;

        f32 width;

    private:
        DRX_LEAK_DETECTOR (Glyph)
    };

    //==============================================================================
    /** A sequence of glyphs with a common font and colour. */
    class DRX_API  Run
    {
    public:
        Run() = default;
        Run (Range<i32> stringRange, i32 numGlyphsToPreallocate);

        /** Returns the X position range which contains all the glyphs in this run. */
        Range<f32> getRunBoundsX() const noexcept;

        Font font { FontOptions{} };    /**< The run's font. */
        Color colour { 0xff000000 };   /**< The run's colour. */
        Array<Glyph> glyphs;            /**< The glyphs in this run. */
        Range<i32> stringRange;         /**< The character range that this run represents in the
                                             original string that was used to create it. */
    private:
        DRX_LEAK_DETECTOR (Run)
    };

    //==============================================================================
    /** A line containing a sequence of glyph-runs. */
    class DRX_API  Line
    {
    public:
        Line() = default;
        Line (Range<i32> stringRange, Point<f32> lineOrigin,
              f32 ascent, f32 descent, f32 leading, i32 numRunsToPreallocate);

        Line (const Line&);
        Line& operator= (const Line&);

        Line (Line&&) noexcept = default;
        Line& operator= (Line&&) noexcept = default;

        ~Line() noexcept = default;

        /** Returns the X position range which contains all the glyphs in this line. */
        Range<f32> getLineBoundsX() const noexcept;

        /** Returns the Y position range which contains all the glyphs in this line. */
        Range<f32> getLineBoundsY() const noexcept;

        /** Returns the smallest rectangle which contains all the glyphs in this line. */
        Rectangle<f32> getLineBounds() const noexcept;

        z0 swap (Line& other) noexcept;

        OwnedArray<Run> runs;           /**< The glyph-runs in this line. */
        Range<i32> stringRange;         /**< The character range that this line represents in the
                                             original string that was used to create it. */
        Point<f32> lineOrigin;        /**< The line's baseline origin. */
        f32 ascent = 0.0f, descent = 0.0f, leading = 0.0f;

    private:
        DRX_LEAK_DETECTOR (Line)
    };

    //==============================================================================
    /** Returns the maximum width of the content. */
    f32 getWidth() const noexcept     { return width; }

    /** Returns the maximum height of the content. */
    f32 getHeight() const noexcept    { return height; }

    /** Returns the number of lines in the layout. */
    i32 getNumLines() const noexcept    { return lines.size(); }

    /** Returns one of the lines. */
    Line& getLine (i32 index) const noexcept;

    /** Adds a line to the layout. The layout will take ownership of this line object
        and will delete it when it is no longer needed. */
    z0 addLine (std::unique_ptr<Line>);

    /** Pre-allocates space for the specified number of lines. */
    z0 ensureStorageAllocated (i32 numLinesNeeded);

    using       iterator = DereferencingIterator<      Line* const*>;
    using const_iterator = DereferencingIterator<const Line* const*>;

    /** Returns an iterator over the lines of content */
          iterator  begin()       { return       iterator (lines.begin()); }
    const_iterator  begin() const { return const_iterator (lines.begin()); }
    const_iterator cbegin() const { return const_iterator (lines.begin()); }

    /** Returns an iterator over the lines of content */
          iterator  end()       { return       iterator (lines.end()); }
    const_iterator  end() const { return const_iterator (lines.end()); }
    const_iterator cend() const { return const_iterator (lines.end()); }

    /** If you modify the TextLayout after creating it, call this to compute
        the new dimensions of the content.
    */
    z0 recalculateSize();

    /** This convenience function adds an AttributedString to a TextLayout
        and returns the bounding box of the text after shaping.

        The returned bounding box is positioned with its origin at the left end of the text's
        baseline.
    */
    static Rectangle<f32> getStringBounds (const AttributedString& string)
    {
        TextLayout layout;
        layout.createLayout (string, std::numeric_limits<f32>::max());

        if (layout.getNumLines() == 0)
            return {};

        return layout.getLine (0).getLineBounds();
    }

    /** This convenience function adds text to a TextLayout using the specified font
        and returns the bounding box of the text after shaping.

        The returned bounding box is positioned with its origin at the left end of the text's
        baseline.
    */
    static Rectangle<f32> getStringBounds (const Font& font, StringRef text)
    {
        AttributedString string;
        string.append (text, font);
        return getStringBounds (string);
    }

    /** This convenience function adds an AttributedString to a TextLayout
        and returns the bounding box of the text after shaping.
    */
    static f32 getStringWidth (const AttributedString& string)
    {
        return getStringBounds (string).getWidth();
    }

    /** This convenience function adds text to a TextLayout using the specified font
        and returns the width of the bounding box of the text after shaping.
    */
    static f32 getStringWidth (const Font& font, StringRef text)
    {
        return getStringBounds (font, text).getWidth();
    }

private:
    OwnedArray<Line> lines;
    f32 width, height;
    Justification justification;

    z0 createStandardLayout (const AttributedString&);

    DRX_LEAK_DETECTOR (TextLayout)
};

} // namespace drx
