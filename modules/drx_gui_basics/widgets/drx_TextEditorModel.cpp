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

class TextEditor::ParagraphStorage
{
public:
    ParagraphStorage (Txt s, const TextEditorStorage* storageIn)
        : text { std::move (s) },
          numBytesAsUTF8 { text.getNumBytesAsUTF8() },
          storage { *storageIn }
    {
        updatePasswordReplacementText();
    }

    const Txt& getText() const
    {
        return text;
    }

    const Txt& getTextForDisplay() const;

    size_t getNumBytesAsUTF8() const;

    z0 setRange (Range<z64> rangeIn)
    {
        range = rangeIn;
    }

    auto getRange() const
    {
        return range;
    }

    const detail::ShapedText& getShapedText();

    f32 getHeight()
    {
        if (! height.has_value())
            height = getShapedText().getHeight();

        return *height;
    }

    z64 getNumGlyphs()
    {
        if (! numGlyphs.has_value())
            numGlyphs = getShapedText().getNumGlyphs();

        return *numGlyphs;
    }

    f32 getTop();

    z64 getStartingGlyph();

    z0 clearShapedText();

private:
    z0 updatePasswordReplacementText();

    Txt text;
    std::optional<Txt> passwordReplacementText;
    size_t numBytesAsUTF8;
    Range<z64> range;
    const TextEditorStorage& storage;
    std::optional<detail::ShapedText> shapedText;
    std::optional<f32> height;
    std::optional<z64> numGlyphs;
};

//==============================================================================
class TextEditor::ParagraphsModel
{
public:
    using ParagraphItem = detail::RangedValuesIteratorItem<const std::unique_ptr<ParagraphStorage>>;

    explicit ParagraphsModel (const TextEditorStorage* ownerIn)
        : owner { *ownerIn }
    {}

    z0 set (Range<z64> range, const Txt& text)
    {
        using namespace detail;

        const auto codepointBeforeRange = getTextInRange (Range<z64>::withStartAndLength (range.getStart() - 1, 1));

        Ranges::Operations ops;

        ranges.drop (range, ops);

        if (! text.isEmpty())
        {
            ranges.insert ({ range.getStart(), range.getStart() + text.length() }, ops);
            mergeForward (ranges, *ranges.getIndexForEnclosingRange (range.getStart()), ops);
        }

        if (const auto newParagraphIndex = ranges.getIndexForEnclosingRange (range.getStart()))
            ranges.mergeBack (*newParagraphIndex, ops);

        const auto splitBeforeOffset = range.getStart() + 1 - (codepointBeforeRange.isEmpty() ? 0 : 1);

        for (auto breakAfterIndex : UnicodeHelpers::getLineBreaks (codepointBeforeRange + text))
            ranges.split (breakAfterIndex + splitBeforeOffset, ops);

        handleOps (ops, text);
    }

    Txt getText() const
    {
        const auto numBytes = std::accumulate (storage.begin(),
                                               storage.end(),
                                               (size_t) 0,
                                               [] (auto sum, auto& p)
                                               {
                                                   return sum + p->getNumBytesAsUTF8();
                                               });

        MemoryOutputStream mo;
        mo.preallocate (numBytes);

        for (const auto& paragraph : storage)
            mo << paragraph->getText();

        return mo.toUTF8();
    }

    Txt getTextInRange (Range<z64> range) const
    {
        Txt text;

        for (const auto& partialRange : ranges.getIntersectionsWith (range))
        {
            const auto i = *ranges.getIndexForEnclosingRange (partialRange.getStart());
            const auto fullRange = ranges.get (i);
            auto& paragraph = *storage[i];
            const auto startInParagraph = (i32) (partialRange.getStart() - fullRange.getStart());
            text += paragraph.getText().substring (startInParagraph,
                                                   startInParagraph + (i32) partialRange.getLength());
        }

        return text;
    }

    auto begin() const
    {
        return detail::RangedValuesIterator<const std::unique_ptr<ParagraphStorage>> { storage.data(),
                                                                                       ranges.data(),
                                                                                       ranges.data() };
    }

    auto end() const
    {
        return detail::RangedValuesIterator<const std::unique_ptr<ParagraphStorage>> { storage.data(),
                                                                                       ranges.data(),
                                                                                       ranges.data() + ranges.size() };
    }

    std::optional<ParagraphItem> getParagraphContainingCodepointIndex (z64 index)
    {
        const auto paragraphIndex = ranges.getIndexForEnclosingRange (index);

        if (! paragraphIndex.has_value())
            return std::nullopt;

        return ParagraphItem { ranges.get (*paragraphIndex),
                               storage[*paragraphIndex] };
    }

    b8 isEmpty() const { return storage.empty(); }

    ParagraphItem back() const
    {
        jassert (! ranges.isEmpty());
        return { ranges.get (ranges.size() - 1), storage.back() };
    }

    z64 getTotalNumChars() const
    {
        if (ranges.isEmpty())
            return 0;

        return ranges.getRanges().back().getEnd();
    }

    z64 getTotalNumGlyphs() const
    {
        return std::accumulate (storage.begin(),
                                storage.end(),
                                (z64) 0,
                                [] (const auto& sum, const auto& item)
                                {
                                    return sum + item->getNumGlyphs();
                                });
    }

private:
    static z0 mergeForward (detail::Ranges& ranges, size_t index, detail::Ranges::Operations& ops)
    {
        if (ranges.size() > index + 1)
            return ranges.mergeBack (index + 1, ops);
    }

    z0 handleOps (const detail::Ranges::Operations& ops, const Txt& text)
    {
        using namespace detail;

        for (const auto& op : ops)
        {
            if (auto* newOp = std::get_if<Ranges::Ops::New> (&op))
            {
                storage.insert (iteratorWithAdvance (storage.begin(), newOp->index),
                                createParagraph (text));
            }
            else if (auto* split = std::get_if<Ranges::Ops::Split> (&op))
            {
                const auto& splitValue = storage[split->index]->getText();
                const auto localLeftRange = split->leftRange.movedToStartAt (0);
                const auto localRightRange = split->rightRange.movedToStartAt (localLeftRange.getEnd());

                auto leftSplitValue = splitValue.substring ((i32) localLeftRange.getStart(),
                                                            (i32) localLeftRange.getEnd());

                auto rightSplitValue = splitValue.substring ((i32) localRightRange.getStart(),
                                                             (i32) localRightRange.getEnd());

                storage[split->index] = createParagraph (std::move (leftSplitValue));

                storage.insert (iteratorWithAdvance (storage.begin(), split->index + 1),
                                createParagraph (std::move (rightSplitValue)));
            }
            else if (auto* erased = std::get_if<Ranges::Ops::Erase> (&op))
            {
                storage.erase (iteratorWithAdvance (storage.begin(), erased->range.getStart()),
                               iteratorWithAdvance (storage.begin(), erased->range.getEnd()));
            }
            else if (auto* changed = std::get_if<Ranges::Ops::Change> (&op))
            {
                const auto oldRange = changed->oldRange;
                const auto newRange = changed->newRange;

                // This happens when a range just gets shifted due to drop or insert operations
                if (oldRange.getLength() == newRange.getLength())
                    continue;

                auto deltaStart = (i32) (newRange.getStart() - oldRange.getStart());
                auto deltaEnd   = (i32) (newRange.getEnd() - oldRange.getEnd());

                auto& paragraph = storage[changed->index];
                const auto& oldText = paragraph->getText();

                jassert (deltaStart >= 0);

                if (deltaEnd <= 0)
                {
                    paragraph = createParagraph (oldText.substring (deltaStart, oldText.length() + deltaEnd));
                }
                else
                {
                    jassert (changed->index + 1 < storage.size());
                    paragraph = createParagraph (oldText.substring (deltaStart, oldText.length())
                                                 + storage[changed->index + 1]->getText().substring (0, deltaEnd));
                }
            }
        }

        for (const auto [index, range] : enumerate (ranges, size_t{}))
            storage[index]->setRange (range);
    }

    std::unique_ptr<ParagraphStorage> createParagraph (Txt s) const
    {
        return std::make_unique<ParagraphStorage> (s, &owner);
    }

    const TextEditorStorage& owner;
    detail::Ranges ranges;
    std::vector<std::unique_ptr<ParagraphStorage>> storage;
};

//==============================================================================
struct TextEditor::TextEditorStorageChunks
{
    std::vector<z64> positions;
    std::vector<Txt> texts;
    std::vector<Font> fonts;
    std::vector<Color> colours;
};

//==============================================================================
class TextEditor::TextEditorStorage
{
public:
    z0 set (Range<z64> range, const Txt& text, const Font& font, const Color& colour)
    {
        paragraphs.set (range, text);

        detail::Ranges::Operations ops;

        fonts.drop (range, ops);
        colours.drop (range, ops);
        ops.clear();

        const auto insertionRange = Range<z64>::withStartAndLength (range.getStart(),
                                                                      (z64) text.length());
        fonts.insert (insertionRange, font, ops);
        colours.insert (insertionRange, colour, ops);
    }

    z0 setFontForAllText (const Font& font)
    {
        detail::Ranges::Operations ops;
        fonts.set ({ 0, paragraphs.getTotalNumChars() }, font, ops);
        clearShapedTexts();
    }

    z0 setColorForAllText (const Color& colour)
    {
        detail::Ranges::Operations ops;
        colours.set ({ 0, paragraphs.getTotalNumChars() }, colour, ops);
        clearShapedTexts();
    }

    z0 remove (Range<z64> range, TextEditorStorageChunks* removedOut)
    {
        using namespace detail;

        detail::Ranges::Operations ops;

        RangedValues<z64> rangeConstraint;
        rangeConstraint.set (range, 0, ops);

        if (removedOut != nullptr)
        {
            for (const auto [r, font, colour, _] : makeIntersectingRangedValues (&fonts, &colours, &rangeConstraint))
            {
                ignoreUnused (_);

                removedOut->positions.push_back (r.getStart());
                removedOut->texts.push_back (getTextInRange (r));
                removedOut->fonts.push_back (font);
                removedOut->colours.push_back (colour);
            }
        }

        paragraphs.set (range, "");
        ops.clear();
        fonts.drop (range, ops);
        colours.drop (range, ops);
    }

    z0 addChunks (const TextEditorStorageChunks& chunks)
    {
        for (size_t i = 0; i < chunks.positions.size(); ++i)
        {
            set (Range<z64>::withStartAndLength (chunks.positions[i], 0),
                 chunks.texts[i],
                 chunks.fonts[i],
                 chunks.colours[i]);
        }
    }

    Txt getText() const
    {
        return paragraphs.getText();
    }

    Txt getTextInRange (Range<z64> range) const
    {
        return paragraphs.getTextInRange (range);
    }

    detail::RangedValues<Font> getFonts (Range<z64> range) const
    {
        return fonts.getIntersectionsStartingAtZeroWith (range);
    }

    const auto& getColors() const
    {
        return colours;
    }

    auto begin() const { return paragraphs.begin(); }
    auto end() const { return paragraphs.end(); }

    auto isEmpty() const { return paragraphs.isEmpty(); }
    auto back() const { return paragraphs.back(); }

    std::optional<Font> getLastFont() const
    {
        if (fonts.isEmpty())
            return std::nullopt;

        return fonts.back().value;
    }

    z64 getTotalNumChars() const
    {
        return paragraphs.getTotalNumChars();
    }

    z64 getTotalNumGlyphs() const
    {
        return paragraphs.getTotalNumGlyphs();
    }

    z0 setBaseShapedTextOptions (detail::ShapedTextOptions options, t32 passwordCharacterIn)
    {
        if (std::exchange (baseShapedTextOptions, options) != options)
            clearShapedTexts();

        if (std::exchange (passwordCharacter, passwordCharacterIn) != passwordCharacterIn)
            clearShapedTexts();
    }

    detail::ShapedTextOptions getShapedTextOptions (Range<z64> range) const
    {
        return baseShapedTextOptions.withFonts (getFonts (range));
    }

    t32 getPasswordCharacter() const
    {
        return passwordCharacter;
    }

    auto getParagraphContainingCodepointIndex (z64 index)
    {
        return paragraphs.getParagraphContainingCodepointIndex (index);
    }

private:
    z0 clearShapedTexts()
    {
        for (auto p : paragraphs)
            p.value->clearShapedText();
    }

    detail::RangedValues<Font> fonts;
    detail::RangedValues<Color> colours;
    ParagraphsModel paragraphs { this };
    detail::ShapedTextOptions baseShapedTextOptions;
    t32 passwordCharacter = 0;
};

//==============================================================================
const Txt& TextEditor::ParagraphStorage::getTextForDisplay() const
{
    if (passwordReplacementText.has_value())
        return *passwordReplacementText;

    return text;
}

size_t TextEditor::ParagraphStorage::getNumBytesAsUTF8() const
{
    return numBytesAsUTF8;
}

const detail::ShapedText& TextEditor::ParagraphStorage::getShapedText()
{
    if (! shapedText.has_value())
        shapedText.emplace (getTextForDisplay(), storage.getShapedTextOptions (range));

    return *shapedText;
}

f32 TextEditor::ParagraphStorage::getTop()
{
    f32 top = 0.0f;

    for (const auto paragraphItem : storage)
    {
        if (paragraphItem.value.get() == this)
            break;

        top += paragraphItem.value->getHeight();
    }

    return top;
}

z64 TextEditor::ParagraphStorage::getStartingGlyph()
{
    z64 startingGlyph = 0;

    for (const auto paragraph : storage)
    {
        if (paragraph.value.get() == this)
            break;

        startingGlyph += paragraph.value->getNumGlyphs();
    }

    return startingGlyph;
}

z0 TextEditor::ParagraphStorage::updatePasswordReplacementText()
{
    const auto passwordChar = storage.getPasswordCharacter();

    if (passwordChar == 0)
    {
        passwordReplacementText.reset();
        return;
    }

    constexpr t32 cr = 0x0d;
    constexpr t32 lf = 0x0a;

    const auto startIt = text.begin();
    auto endIt = text.end();

    for (i32 i = 0; i < 2; ++i)
    {
        if (endIt == startIt)
            break;

        auto newEnd = endIt - 1;

        if (*newEnd != cr && *newEnd != lf)
            break;

        endIt = newEnd;
    }

    passwordReplacementText = Txt::repeatedString (Txt::charToString (passwordChar),
                                                      (i32) startIt.lengthUpTo (endIt))
                                  + Txt { endIt, text.end() };
}

z0 TextEditor::ParagraphStorage::clearShapedText()
{
    shapedText.reset();
    height.reset();
    numGlyphs.reset();
    updatePasswordReplacementText();
}

}
