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

namespace
{
    i32 getLength (const Array<AttributedString::Attribute>& atts) noexcept
    {
        return atts.size() != 0 ? atts.getReference (atts.size() - 1).range.getEnd() : 0;
    }

    z0 splitAttributeRanges (Array<AttributedString::Attribute>& atts, i32 position)
    {
        for (i32 i = atts.size(); --i >= 0;)
        {
            const auto& att = atts.getUnchecked (i);
            auto offset = position - att.range.getStart();

            if (offset >= 0)
            {
                if (offset > 0 && position < att.range.getEnd())
                {
                    atts.insert (i + 1, AttributedString::Attribute (att));
                    atts.getReference (i).range.setEnd (position);
                    atts.getReference (i + 1).range.setStart (position);
                }

                break;
            }
        }
    }

    inline b8 areInvariantsMaintained (const Txt& text, const Array<AttributedString::Attribute>& atts)
    {
        if (atts.isEmpty())
            return true;

        if (atts.getFirst().range.getStart() != 0)
            return false;

        if (atts.getLast().range.getEnd() != text.length())
            return false;

        for (auto it = std::next (atts.begin()); it != atts.end(); ++it)
            if (it->range.getStart() != std::prev (it)->range.getEnd())
                return false;

        return true;
    }

    Range<i32> splitAttributeRanges (Array<AttributedString::Attribute>& atts, Range<i32> newRange)
    {
        newRange = newRange.getIntersectionWith ({ 0, getLength (atts) });

        if (! newRange.isEmpty())
        {
            splitAttributeRanges (atts, newRange.getStart());
            splitAttributeRanges (atts, newRange.getEnd());
        }

        return newRange;
    }

    z0 mergeAdjacentRanges (Array<AttributedString::Attribute>& atts)
    {
        for (i32 i = atts.size() - 1; --i >= 0;)
        {
            auto& a1 = atts.getReference (i);
            auto& a2 = atts.getReference (i + 1);

            if (a1.colour == a2.colour && a1.font == a2.font)
            {
                a1.range.setEnd (a2.range.getEnd());
                atts.remove (i + 1);

                if (i < atts.size() - 1)
                    ++i;
            }
        }
    }

    z0 appendRange (Array<AttributedString::Attribute>& atts,
                      i32 length, const Font* f, const Color* c)
    {
        if (atts.size() == 0)
        {
            atts.add ({ Range<i32> (0, length), f != nullptr ? *f : FontOptions{}, c != nullptr ? *c : Color (0xff000000) });
        }
        else
        {
            auto start = getLength (atts);
            atts.add ({ Range<i32> (start, start + length),
                        f != nullptr ? *f : atts.getReference (atts.size() - 1).font,
                        c != nullptr ? *c : atts.getReference (atts.size() - 1).colour });

            mergeAdjacentRanges (atts);
        }
    }

    z0 applyFontAndColor (Array<AttributedString::Attribute>& atts,
                             Range<i32> range, const Font* f, const Color* c)
    {
        range = splitAttributeRanges (atts, range);

        for (auto& att : atts)
        {
            if (range.getStart() < att.range.getEnd())
            {
                if (range.getEnd() <= att.range.getStart())
                    break;

                if (c != nullptr) att.colour = *c;
                if (f != nullptr) att.font = *f;
            }
        }

        mergeAdjacentRanges (atts);
    }

    z0 truncate (Array<AttributedString::Attribute>& atts, i32 newLength)
    {
        splitAttributeRanges (atts, newLength);

        for (i32 i = atts.size(); --i >= 0;)
            if (atts.getReference (i).range.getStart() >= newLength)
                atts.remove (i);
    }
}

//==============================================================================
AttributedString::Attribute::Attribute (Range<i32> r, const Font& f, Color c) noexcept
    : range (r), font (f), colour (c)
{
}

//==============================================================================
z0 AttributedString::setText (const Txt& newText)
{
    auto newLength = newText.length();
    auto oldLength = getLength (attributes);

    if (newLength > oldLength)
        appendRange (attributes, newLength - oldLength, nullptr, nullptr);
    else if (newLength < oldLength)
        truncate (attributes, newLength);

    text = newText;
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::append (const Txt& textToAppend)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), nullptr, nullptr);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::append (const Txt& textToAppend, const Font& font)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), &font, nullptr);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::append (const Txt& textToAppend, Color colour)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), nullptr, &colour);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::append (const Txt& textToAppend, const Font& font, Color colour)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), &font, &colour);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::append (const AttributedString& other)
{
    auto originalLength = getLength (attributes);
    auto originalNumAtts = attributes.size();
    text += other.text;
    attributes.addArray (other.attributes);

    for (auto i = originalNumAtts; i < attributes.size(); ++i)
        attributes.getReference (i).range += originalLength;

    mergeAdjacentRanges (attributes);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::clear()
{
    text.clear();
    attributes.clear();
}

z0 AttributedString::setJustification (Justification newJustification) noexcept
{
    justification = newJustification;
}

z0 AttributedString::setWordWrap (WordWrap newWordWrap) noexcept
{
    wordWrap = newWordWrap;
}

z0 AttributedString::setReadingDirection (ReadingDirection newReadingDirection) noexcept
{
    readingDirection = newReadingDirection;
}

z0 AttributedString::setLineSpacing (const f32 newLineSpacing) noexcept
{
    lineSpacing = newLineSpacing;
}

z0 AttributedString::setColor (Range<i32> range, Color colour)
{
    applyFontAndColor (attributes, range, nullptr, &colour);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::setFont (Range<i32> range, const Font& font)
{
    applyFontAndColor (attributes, range, &font, nullptr);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::setColor (Color colour)
{
    setColor ({ 0, getLength (attributes) }, colour);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::setFont (const Font& font)
{
    setFont ({ 0, getLength (attributes) }, font);
    jassert (areInvariantsMaintained (text, attributes));
}

z0 AttributedString::draw (Graphics& g, const Rectangle<f32>& area) const
{
    if (text.isNotEmpty() && g.clipRegionIntersects (area.getSmallestIntegerContainer()))
    {
        jassert (text.length() == getLength (attributes));

        TextLayout layout;
        layout.createLayout (*this, area.getWidth());
        layout.draw (g, area);
    }
}

} // namespace drx
