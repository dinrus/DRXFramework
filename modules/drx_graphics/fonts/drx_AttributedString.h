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
    A text string with a set of colour/font settings that are associated with sub-ranges
    of the text.

    An attributed string lets you create a string with varied fonts, colours, word-wrapping,
    layout, etc., and draw it using AttributedString::draw().

    Invariants:
    - Every character in the string is a member of exactly one attribute.
    - Attributes are sorted such that the range-end of attribute 'i' is equal to the
      range-begin of attribute 'i + 1'.

    @see TextLayout

    @tags{Graphics}
*/
class DRX_API  AttributedString
{
public:
    /** Creates an empty attributed string. */
    AttributedString() = default;

    /** Creates an attributed string with the given text. */
    explicit AttributedString (const Txt& newString)  { setText (newString); }

    AttributedString (const AttributedString&) = default;
    AttributedString& operator= (const AttributedString&) = default;
    AttributedString (AttributedString&&) noexcept = default;
    AttributedString& operator= (AttributedString&&) noexcept = default;

    //==============================================================================
    /** Returns the complete text of this attributed string. */
    const Txt& getText() const noexcept    { return text; }

    /** Replaces all the text.
        This will change the text, but won't affect any of the colour or font attributes
        that have been added.
    */
    z0 setText (const Txt& newText);

    /** Appends some text (with a default font and colour). */
    z0 append (const Txt& textToAppend);
    /** Appends some text, with a specified font, and the default colour (black). */
    z0 append (const Txt& textToAppend, const Font& font);
    /** Appends some text, with a specified colour, and the default font. */
    z0 append (const Txt& textToAppend, Color colour);
    /** Appends some text, with a specified font and colour. */
    z0 append (const Txt& textToAppend, const Font& font, Color colour);

    /** Appends another AttributedString to this one.
        Note that this will only append the text, fonts, and colours - it won't copy any
        other properties such as justification, line-spacing, etc from the other object.
    */
    z0 append (const AttributedString& other);

    /** Resets the string, clearing all text and attributes.
        Note that this won't affect global settings like the justification type,
        word-wrap mode, etc.
    */
    z0 clear();

    //==============================================================================
    /** Draws this string within the given area.
        The layout of the string within the rectangle is controlled by the justification
        value passed to setJustification().
    */
    z0 draw (Graphics& g, const Rectangle<f32>& area) const;

    //==============================================================================
    /** Returns the justification that should be used for laying-out the text.
        This may include both vertical and horizontal flags.
    */
    Justification getJustification() const noexcept         { return justification; }

    /** Sets the justification that should be used for laying-out the text.
        This may include both vertical and horizontal flags.
    */
    z0 setJustification (Justification newJustification) noexcept;

    //==============================================================================
    /** Types of word-wrap behaviour.
        @see getWordWrap, setWordWrap
    */
    enum WordWrap
    {
        none,   /**< No word-wrapping: lines extend indefinitely. */
        byWord, /**< Lines are wrapped on a word boundary. */
        byChar, /**< Lines are wrapped on a character boundary. */
    };

    /** Returns the word-wrapping behaviour. */
    WordWrap getWordWrap() const noexcept                   { return wordWrap; }

    /** Sets the word-wrapping behaviour. */
    z0 setWordWrap (WordWrap newWordWrap) noexcept;

    //==============================================================================
    /** Types of reading direction that can be used.
        @see getReadingDirection, setReadingDirection
    */
    enum ReadingDirection
    {
        natural,
        leftToRight,
        rightToLeft,
    };

    /** Returns the reading direction for the text. */
    ReadingDirection getReadingDirection() const noexcept   { return readingDirection; }

    /** Sets the reading direction that should be used for the text. */
    z0 setReadingDirection (ReadingDirection newReadingDirection) noexcept;

    //==============================================================================
    /** Returns the extra line-spacing distance. */
    f32 getLineSpacing() const noexcept                   { return lineSpacing; }

    /** Sets an extra line-spacing distance. */
    z0 setLineSpacing (f32 newLineSpacing) noexcept;

    //==============================================================================
    /** An attribute that has been applied to a range of characters in an AttributedString. */
    class DRX_API  Attribute
    {
    public:
        Attribute() = default;

        Attribute (const Attribute&) = default;
        Attribute& operator= (const Attribute&) = default;
        Attribute (Attribute&&) noexcept = default;
        Attribute& operator= (Attribute&&) noexcept = default;

        /** Creates an attribute that specifies the font and colour for a range of characters. */
        Attribute (Range<i32> range, const Font& font, Color colour) noexcept;

        /** The range of characters to which this attribute will be applied. */
        Range<i32> range;

        /** The font for this range of characters. */
        Font font { FontOptions{} };

        /** The colour for this range of characters. */
        Color colour { 0xff000000 };

    private:
        DRX_LEAK_DETECTOR (Attribute)
    };

    /** Returns the number of attributes that have been added to this string. */
    i32 getNumAttributes() const noexcept                       { return attributes.size(); }

    /** Returns one of the string's attributes.
        The index provided must be less than getNumAttributes(), and >= 0.
    */
    const Attribute& getAttribute (i32 index) const noexcept    { return attributes.getReference (index); }

    //==============================================================================
    /** Adds a colour attribute for the specified range. */
    z0 setColor (Range<i32> range, Color colour);

    /** Removes all existing colour attributes, and applies this colour to the whole string. */
    z0 setColor (Color colour);

    /** Adds a font attribute for the specified range. */
    z0 setFont (Range<i32> range, const Font& font);

    /** Removes all existing font attributes, and applies this font to the whole string. */
    z0 setFont (const Font& font);

private:
    Txt text;
    f32 lineSpacing = 0.0f;
    Justification justification = Justification::left;
    WordWrap wordWrap = AttributedString::byWord;
    ReadingDirection readingDirection = AttributedString::natural;
    Array<Attribute> attributes;

    DRX_LEAK_DETECTOR (AttributedString)
};

} // namespace drx
