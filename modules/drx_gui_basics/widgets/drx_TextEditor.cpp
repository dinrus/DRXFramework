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
struct TextEditor::InsertAction final : public UndoableAction
{
    InsertAction (TextEditor& ed, const Txt& newText, i32 insertPos,
                  const Font& newFont, Color newColor, i32 oldCaret, i32 newCaret)
        : owner (ed),
          text (newText),
          insertIndex (insertPos),
          oldCaretPos (oldCaret),
          newCaretPos (newCaret),
          font (newFont),
          colour (newColor)
    {
    }

    b8 perform() override
    {
        owner.insert (text, insertIndex, font, colour, nullptr, newCaretPos);
        return true;
    }

    b8 undo() override
    {
        owner.remove ({ insertIndex, insertIndex + text.length() }, nullptr, oldCaretPos);
        return true;
    }

    i32 getSizeInUnits() override
    {
        return text.length() + 16;
    }

private:
    TextEditor& owner;
    const Txt text;
    i32k insertIndex, oldCaretPos, newCaretPos;
    const Font font;
    const Color colour;

    DRX_DECLARE_NON_COPYABLE (InsertAction)
};

//==============================================================================
struct TextEditor::RemoveAction final : public UndoableAction
{
    RemoveAction (TextEditor& ed, Range<i32> rangeToRemove, i32 oldCaret, i32 newCaret)
        : owner (ed),
          range (rangeToRemove),
          oldCaretPos (oldCaret),
          newCaretPos (newCaret)
    {
    }

    b8 perform() override
    {
        owner.remove (range, nullptr, newCaretPos, &removedText);
        return true;
    }

    b8 undo() override
    {
        owner.reinsert (removedText);
        owner.moveCaretTo (oldCaretPos, false);
        return true;
    }

    i32 getSizeInUnits() override
    {
        return std::accumulate (removedText.texts.begin(),
                                removedText.texts.end(),
                                0,
                                [] (auto sum, auto& value)
                                {
                                    return sum + (i32) value.getNumBytesAsUTF8();
                                });
    }

private:
    TextEditor& owner;
    const Range<i32> range;
    i32k oldCaretPos, newCaretPos;
    TextEditorStorageChunks removedText;

    DRX_DECLARE_NON_COPYABLE (RemoveAction)
};

//==============================================================================
struct TextEditor::TextHolderComponent final : public Component,
                                               public Timer,
                                               public Value::Listener
{
    TextHolderComponent (TextEditor& ed)  : owner (ed)
    {
        setWantsKeyboardFocus (false);
        setInterceptsMouseClicks (false, true);
        setMouseCursor (MouseCursor::ParentCursor);

        owner.getTextValue().addListener (this);
    }

    ~TextHolderComponent() override
    {
        owner.getTextValue().removeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        owner.drawContent (g);
    }

    z0 restartTimer()
    {
        startTimer (350);
    }

    z0 timerCallback() override
    {
        owner.timerCallbackInt();
    }

    z0 valueChanged (Value&) override
    {
        owner.textWasChangedByValue();
    }

    TextEditor& owner;

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextHolderComponent)
};

//==============================================================================
struct TextEditor::TextEditorViewport final : public Viewport
{
    TextEditorViewport (TextEditor& ed) : owner (ed) {}

    z0 visibleAreaChanged (const Rectangle<i32>&) override
    {
        if (! reentrant) // it's rare, but possible to get into a feedback loop as the viewport's scrollbars
                         // appear and disappear, causing the wrap width to change.
        {
            auto wordWrapWidth = owner.getWordWrapWidth();
            owner.updateBaseShapedTextOptions();

            if (wordWrapWidth != lastWordWrapWidth)
            {
                lastWordWrapWidth = wordWrapWidth;

                ScopedValueSetter<b8> svs (reentrant, true);
                owner.checkLayout();
            }
        }
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    TextEditor& owner;
    i32 lastWordWrapWidth = 0;
    b8 reentrant = false;

    DRX_DECLARE_NON_COPYABLE (TextEditorViewport)
};

//==============================================================================
namespace TextEditorDefs
{
    i32k textChangeMessageId = 0x10003001;
    i32k returnKeyMessageId  = 0x10003002;
    i32k escapeKeyMessageId  = 0x10003003;
    i32k focusLossMessageId  = 0x10003004;

    i32k maxActionsPerTransaction = 100;

    static i32 getCharacterCategory (t32 character) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (character)
                    ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
    }
}

//==============================================================================
TextEditor::TextEditor (const Txt& name, t32 passwordChar)
    : Component (name),
      passwordCharacter (passwordChar),
      textStorage { std::make_unique<TextEditorStorage>() },
      caretState { this }
{
    setMouseCursor (MouseCursor::IBeamCursor);

    viewport.reset (new TextEditorViewport (*this));
    addAndMakeVisible (viewport.get());
    viewport->setViewedComponent (textHolder = new TextHolderComponent (*this));
    viewport->setWantsKeyboardFocus (false);
    viewport->setScrollBarsShown (false, false);

    setWantsKeyboardFocus (true);
    recreateCaret();
}

TextEditor::~TextEditor()
{
    if (auto* peer = getPeer())
        peer->refreshTextInputTarget();

    textValue.removeListener (textHolder);
    textValue.referTo (Value());

    viewport.reset();
    textHolder = nullptr;
}

//==============================================================================
z0 TextEditor::newTransaction()
{
    lastTransactionTime = Time::getApproximateMillisecondCounter();
    undoManager.beginNewTransaction();
}

b8 TextEditor::undoOrRedo (const b8 shouldUndo)
{
    if (! isReadOnly())
    {
        newTransaction();

        if (shouldUndo ? undoManager.undo()
                       : undoManager.redo())
        {
            repaint();
            textChanged();
            scrollToMakeSureCursorIsVisible();

            return true;
        }
    }

    return false;
}

b8 TextEditor::undo()     { return undoOrRedo (true); }
b8 TextEditor::redo()     { return undoOrRedo (false); }

//==============================================================================
z0 TextEditor::setMultiLine (const b8 shouldBeMultiLine,
                               const b8 shouldWordWrap)
{
    if (multiline != shouldBeMultiLine
         || wordWrap != (shouldWordWrap && shouldBeMultiLine))
    {
        multiline = shouldBeMultiLine;
        wordWrap = shouldWordWrap && shouldBeMultiLine;
        updateBaseShapedTextOptions();

        checkLayout();

        viewport->setViewPosition (0, 0);
        resized();
        scrollToMakeSureCursorIsVisible();
    }
}

b8 TextEditor::isMultiLine() const
{
    return multiline;
}

z0 TextEditor::setScrollbarsShown (b8 shown)
{
    if (scrollbarVisible != shown)
    {
        scrollbarVisible = shown;
        checkLayout();
    }
}

z0 TextEditor::setReadOnly (b8 shouldBeReadOnly)
{
    if (readOnly != shouldBeReadOnly)
    {
        readOnly = shouldBeReadOnly;
        enablementChanged();
        invalidateAccessibilityHandler();

        if (auto* peer = getPeer())
            peer->refreshTextInputTarget();
    }
}

z0 TextEditor::setClicksOutsideDismissVirtualKeyboard (b8 newValue)
{
    clicksOutsideDismissVirtualKeyboard = newValue;
}

b8 TextEditor::isReadOnly() const noexcept
{
    return readOnly || ! isEnabled();
}

b8 TextEditor::isTextInputActive() const
{
    return ! isReadOnly() && (! clicksOutsideDismissVirtualKeyboard || globalMouseListener.lastMouseDownInEditor());
}

z0 TextEditor::setReturnKeyStartsNewLine (b8 shouldStartNewLine)
{
    returnKeyStartsNewLine = shouldStartNewLine;
}

z0 TextEditor::setTabKeyUsedAsCharacter (b8 shouldTabKeyBeUsed)
{
    tabKeyUsed = shouldTabKeyBeUsed;
}

z0 TextEditor::setPopupMenuEnabled (b8 b)
{
    popupMenuEnabled = b;
}

z0 TextEditor::setSelectAllWhenFocused (b8 b)
{
    selectAllTextWhenFocused = b;
}

z0 TextEditor::setJustification (Justification j)
{
    if (justification != j)
    {
        justification = j;

        resized();
        repaint();
    }
}

//==============================================================================
z0 TextEditor::setFont (const Font& newFont)
{
    currentFont = newFont;
    scrollToMakeSureCursorIsVisible();
}

z0 TextEditor::applyFontToAllText (const Font& newFont, b8 changeCurrentFont)
{
    if (changeCurrentFont)
        currentFont = newFont;

    textStorage->setFontForAllText (newFont);

    const auto overallColor = findColor (textColorId);
    textStorage->setColorForAllText (overallColor);

    checkLayout();
    scrollToMakeSureCursorIsVisible();
    repaint();
}

z0 TextEditor::applyColorToAllText (const Color& newColor, b8 changeCurrentTextColor)
{
    textStorage->setColorForAllText (newColor);

    if (changeCurrentTextColor)
        setColor (TextEditor::textColorId, newColor);
    else
        repaint();
}

z0 TextEditor::lookAndFeelChanged()
{
    caret.reset();
    recreateCaret();
}

z0 TextEditor::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

z0 TextEditor::enablementChanged()
{
    recreateCaret();
    repaint();
}

z0 TextEditor::setCaretVisible (b8 shouldCaretBeVisible)
{
    if (caretVisible != shouldCaretBeVisible)
    {
        caretVisible = shouldCaretBeVisible;
        recreateCaret();
    }
}

z0 TextEditor::recreateCaret()
{
    if (isCaretVisible())
    {
        if (caret == nullptr)
        {
            caret.reset (getLookAndFeel().createCaretComponent (this));
            textHolder->addChildComponent (caret.get());
            updateCaretPosition();
        }
    }
    else
    {
        caret.reset();
    }
}

z0 TextEditor::updateCaretPosition()
{
    if (caret != nullptr
        && getWidth() > 0 && getHeight() > 0)
    {
        caret->setCaretPosition (getCaretRectangle().translated (leftIndent,
                                                                 topIndent + roundToInt (getYOffset())) - getTextOffset());

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

TextEditor::LengthAndCharacterRestriction::LengthAndCharacterRestriction (i32 maxLen, const Txt& chars)
    : allowedCharacters (chars), maxLength (maxLen)
{
}

Txt TextEditor::LengthAndCharacterRestriction::filterNewText (TextEditor& ed, const Txt& newInput)
{
    Txt t (newInput);

    if (allowedCharacters.isNotEmpty())
        t = t.retainCharacters (allowedCharacters);

    if (maxLength > 0)
        t = t.substring (0, maxLength - (ed.getTotalNumChars() - ed.getHighlightedRegion().getLength()));

    return t;
}

z0 TextEditor::setInputFilter (InputFilter* newFilter, b8 takeOwnership)
{
    inputFilter.set (newFilter, takeOwnership);
}

z0 TextEditor::setInputRestrictions (i32 maxLen, const Txt& chars)
{
    setInputFilter (new LengthAndCharacterRestriction (maxLen, chars), true);
}

z0 TextEditor::setTextToShowWhenEmpty (const Txt& text, Color colourToUse)
{
    textToShowWhenEmpty = text;
    colourForTextWhenEmpty = colourToUse;
}

z0 TextEditor::setPasswordCharacter (t32 newPasswordCharacter)
{
    if (passwordCharacter != newPasswordCharacter)
    {
        passwordCharacter = newPasswordCharacter;
        applyFontToAllText (currentFont);
        updateBaseShapedTextOptions();
    }
}

z0 TextEditor::setScrollBarThickness (i32 newThicknessPixels)
{
    viewport->setScrollBarThickness (newThicknessPixels);
}

//==============================================================================
z0 TextEditor::clear()
{
    clearInternal (nullptr);
    checkLayout();
    undoManager.clearUndoHistory();
    repaint();
}

z0 TextEditor::setText (const Txt& newText, b8 sendTextChangeMessage)
{
    auto newLength = newText.length();

    if (newLength != getTotalNumChars() || getText() != newText)
    {
        if (! sendTextChangeMessage)
            textValue.removeListener (textHolder);

        textValue = newText;

        auto oldCursorPos = caretState.getPosition();
        auto cursorWasAtEnd = oldCursorPos >= getTotalNumChars();

        clearInternal (nullptr);
        insert (newText, 0, currentFont, findColor (textColorId), nullptr, caretState.getPosition());

        // if you're adding text with line-feeds to a single-line text editor, it
        // ain't gonna look right!
        jassert (multiline || ! newText.containsAnyOf ("\r\n"));

        if (cursorWasAtEnd && ! isMultiLine())
            oldCursorPos = getTotalNumChars();

        moveCaretTo (oldCursorPos, false);

        if (sendTextChangeMessage)
            textChanged();
        else
            textValue.addListener (textHolder);

        checkLayout();
        scrollToMakeSureCursorIsVisible();
        undoManager.clearUndoHistory();

        repaint();
    }
}

//==============================================================================
z0 TextEditor::updateValueFromText()
{
    if (valueTextNeedsUpdating)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }
}

Value& TextEditor::getTextValue()
{
    updateValueFromText();
    return textValue;
}

z0 TextEditor::textWasChangedByValue()
{
    if (textValue.getValueSource().getReferenceCount() > 1)
        setText (textValue.getValue());
}

//==============================================================================
z0 TextEditor::textChanged()
{
    checkLayout();

    if (listeners.size() != 0 || onTextChange != nullptr)
        postCommandMessage (TextEditorDefs::textChangeMessageId);

    if (textValue.getValueSource().getReferenceCount() > 1)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
}

z0 TextEditor::setSelection (Range<i32> newSelection) noexcept
{
    if (newSelection != selection)
    {
        selection = newSelection;

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

z0 TextEditor::returnPressed()    { postCommandMessage (TextEditorDefs::returnKeyMessageId); }
z0 TextEditor::escapePressed()    { postCommandMessage (TextEditorDefs::escapeKeyMessageId); }

z0 TextEditor::addListener (Listener* l)      { listeners.add (l); }
z0 TextEditor::removeListener (Listener* l)   { listeners.remove (l); }

//==============================================================================
z0 TextEditor::timerCallbackInt()
{
    checkFocus();

    auto now = Time::getApproximateMillisecondCounter();

    if (now > lastTransactionTime + 200)
        newTransaction();
}

z0 TextEditor::checkFocus()
{
    if (! wasFocused && hasKeyboardFocus (false) && ! isCurrentlyBlockedByAnotherModalComponent())
        wasFocused = true;
}

z0 TextEditor::repaintText (Range<i32> range)
{
    if (! range.isEmpty())
    {
        if (range.getEnd() >= getTotalNumChars())
        {
            textHolder->repaint();
            return;
        }

        const auto [anchor, lh] = getCursorEdge (caretState.withPosition (range.getStart())
                                                           .withPreferredEdge (Edge::trailing));

        auto y1 = std::trunc (anchor.y);
        i32 y2 = 0;

        if (range.getEnd() >= getTotalNumChars())
        {
            y2 = textHolder->getHeight();
        }
        else
        {
            const auto info = getCursorEdge (caretState.withPosition (range.getEnd())
                                                       .withPreferredEdge (Edge::leading));

            y2 = (i32) (info.anchor.y + lh * 2.0f);
        }

        const auto offset = getYOffset();

        textHolder->repaint (0,
                             (i32) std::floor (y1 + offset),
                             textHolder->getWidth(),
                             (i32) std::ceil ((f32) y2 - y1 + offset));
    }
}

//==============================================================================
z0 TextEditor::moveCaret (i32k newCaretPos)
{
    const auto clamped = std::clamp (newCaretPos, 0, getTotalNumChars());

    if (clamped == getCaretPosition())
        return;

    caretState.setPosition (clamped);

    if (hasKeyboardFocus (false))
        textHolder->restartTimer();

    scrollToMakeSureCursorIsVisible();
    updateCaretPosition();

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
}

i32 TextEditor::getCaretPosition() const
{
    return caretState.getPosition();
}

z0 TextEditor::setCaretPosition (i32k newIndex)
{
    moveCaretTo (newIndex, false);
}

z0 TextEditor::moveCaretToEnd()
{
    setCaretPosition (std::numeric_limits<i32>::max());
}

z0 TextEditor::scrollEditorToPositionCaret (i32k desiredCaretX,
                                              i32k desiredCaretY)

{
    updateCaretPosition();
    auto caretRect = getCaretRectangle().translated (leftIndent, topIndent);

    auto vx = caretRect.getX() - desiredCaretX;
    auto vy = caretRect.getY() - desiredCaretY;

    if (desiredCaretX < jmax (1, proportionOfWidth (0.05f)))
        vx += desiredCaretX - proportionOfWidth (0.2f);
    else if (desiredCaretX > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
        vx += desiredCaretX + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();

    vx = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), vx);

    if (! isMultiLine())
    {
        vy = viewport->getViewPositionY();
    }
    else
    {
        vy = jlimit (0, jmax (0, textHolder->getHeight() - viewport->getMaximumVisibleHeight()), vy);

        if (desiredCaretY < 0)
            vy = jmax (0, desiredCaretY + vy);
        else if (desiredCaretY > jmax (0, viewport->getMaximumVisibleHeight() - caretRect.getHeight()))
            vy += desiredCaretY + 2 + caretRect.getHeight() - viewport->getMaximumVisibleHeight();
    }

    viewport->setViewPosition (vx, vy);
}

Rectangle<i32> TextEditor::getCaretRectangleForCharIndex (i32 index) const
{
    const auto [anchor, cursorHeight] = getCursorEdge (caretState.withPosition (index));
    Rectangle<f32> caretRectangle { anchor.x, anchor.y, 2.0f, cursorHeight };
    return caretRectangle.getSmallestIntegerContainer() + getTextOffset();
}

Point<i32> TextEditor::getTextOffset() const
{
    return { getLeftIndent() + borderSize.getLeft() - viewport->getViewPositionX(),
             roundToInt ((f32) getTopIndent() + (f32) borderSize.getTop() + getYOffset()) - viewport->getViewPositionY() };
}

template <typename T>
detail::RangedValues<T> TextEditor::getGlyphRanges (const detail::RangedValues<T>& textRanges) const
{
    detail::RangedValues<T> glyphRanges;
    std::vector<Range<z64>> glyphRangesStorage;

    detail::Ranges::Operations ops;

    for (const auto [range, value, paragraph] : makeIntersectingRangedValues (&textRanges,
                                                                              textStorage.get()))
    {
        paragraph->getShapedText().getGlyphRanges (range - paragraph->getRange().getStart(),
                                                   glyphRangesStorage);

        for (const auto& glyphRange : glyphRangesStorage)
        {
            glyphRanges.set (glyphRange + paragraph->getStartingGlyph(), value, ops);
            ops.clear();
        }
    }

    return glyphRanges;
}

b8 TextEditor::isTextStorageHeightGreaterEqualThan (f32 value) const
{
    f32 height = 0.0;

    for (auto paragraphItem : *textStorage)
    {
        height += paragraphItem.value->getHeight();

        if (height >= value)
            return true;
    }

    return false;
}

f32 TextEditor::getTextStorageHeight() const
{
    const auto textHeight = std::accumulate (textStorage->begin(), textStorage->end(), 0.0f, [&] (auto acc, auto item)
    {
        return acc + item.value->getHeight();
    });

    if (! textStorage->isEmpty() && ! textStorage->back().value->getText().endsWith ("\n"))
        return textHeight;

    return textHeight + getLineSpacing() * textStorage->getLastFont().value_or (currentFont).getHeight();
}

f32 TextEditor::getYOffset() const
{
    const auto bottomY = getMaximumTextHeight();

    if (justification.testFlags (Justification::top) || isTextStorageHeightGreaterEqualThan ((f32) bottomY))
        return 0;

    auto bottom = jmax (0.0f, (f32) bottomY - getTextStorageHeight());

    if (justification.testFlags (Justification::bottom))
        return bottom;

    return bottom * 0.5f;
}

Range<z64> TextEditor::getLineRangeForIndex (i32 index)
{
    jassert (index >= 0);

    const auto indexInText = (z64) index;

    if (textStorage->isEmpty())
        return { indexInText, indexInText };

    if (const auto paragraph = textStorage->getParagraphContainingCodepointIndex (indexInText))
    {
        const auto& shapedText = paragraph->value->getShapedText();
        auto r = *shapedText.getLineTextRanges().find (indexInText - paragraph->range.getStart())
                 + paragraph->range.getStart();

        if (r.getEnd() != paragraph->range.getEnd())
            return r;

        constexpr t32 cr = 0x0d;
        constexpr t32 lf = 0x0a;

        const auto startIt = shapedText.getText().begin();
        auto endIt = shapedText.getText().end();

        for (i32 i = 0; i < 2; ++i)
        {
            if (endIt == startIt)
                break;

            auto newEnd = endIt - 1;

            if (*newEnd != cr && *newEnd != lf)
                break;

            r.setEnd (std::max (r.getStart(), r.getEnd() - 1));
            endIt = newEnd;
        }

        return r;
    }

    const auto& lastParagraphItem = textStorage->back();

    if (lastParagraphItem.value->getText().endsWith ("\n"))
        return Range<z64>::withStartAndLength (lastParagraphItem.range.getEnd(), 0);

    return lastParagraphItem.value->getShapedText().getLineTextRanges().getRanges().back()
               + lastParagraphItem.range.getStart();
}

TextEditor::CaretEdge TextEditor::getTextSelectionEdge (i32 index, Edge edge) const
{
    jassert (0 <= index && index < getTotalNumChars());
    const auto textRange = Range<z64>::withStartAndLength ((z64) index, 1);

    const auto paragraphIt = std::find_if (textStorage->begin(),
                                           textStorage->end(),
                                           [&] (const auto& p)
                                           {
                                               return p.range.contains (textRange.getStart());
                                           });

    jassert (paragraphIt != textStorage->end());

    auto& paragraph = paragraphIt->value;
    const auto& shapedText = paragraph->getShapedText();

    const auto glyphRange = std::invoke ([&]() -> Range<z64>
    {
        std::vector<Range<z64>> g;
        shapedText.getGlyphRanges (textRange - paragraph->getRange().getStart(), g);

        if (g.empty())
            return {};

        return g.front();
    });

    if (glyphRange.isEmpty())
        return getDefaultCursorEdge();

    const auto glyphsBounds = shapedText.getGlyphsBounds (glyphRange).getRectangle (0);
    const auto ltr = shapedText.isLtr (glyphRange.getStart());

    const auto anchorX = std::invoke ([&]
    {
        if (edge == Edge::leading)
            return ltr ? glyphsBounds.getX() : glyphsBounds.getRight();

        return ltr ? glyphsBounds.getRight() : glyphsBounds.getX();
    });

    const auto lineMetrics = shapedText.getLineMetricsForGlyphRange().find (glyphRange.getStart())->value;
    const auto anchorY = lineMetrics.anchor.getY();

    return { { anchorX, anchorY -lineMetrics.maxAscent + paragraph->getTop() },
             lineMetrics.maxAscent + lineMetrics.maxDescent };
}

z0 TextEditor::updateBaseShapedTextOptions()
{
    auto options = detail::ShapedText::Options{}.withTrailingWhitespacesShouldFit (true)
                                                .withJustification (getJustificationType().getOnlyHorizontalFlags());

    if (wordWrap)
        options = options.withMaxWidth ((f32) getMaximumTextWidth());
    else
        options = options.withAlignmentWidth ((f32) getMaximumTextWidth());

    textStorage->setBaseShapedTextOptions (options, passwordCharacter);
}

static auto asInt64Range (Range<i32> r)
{
    return Range<z64> { (z64) r.getStart(), (z64) r.getEnd() };
}

RectangleList<i32> TextEditor::getTextBounds (Range<i32> textRange) const
{
    RectangleList<i32> boundingBox;

    detail::RangedValues<i32> mask;
    detail::Ranges::Operations ops;
    mask.set (asInt64Range (textRange), 0, ops);

    for (auto [_1, paragraph, _2] : makeIntersectingRangedValues (textStorage.get(), &mask))
    {
        ignoreUnused (_1, _2);
        auto& shapedText = paragraph->getShapedText();

        std::vector<Range<z64>> glyphRanges;
        shapedText.getGlyphRanges (asInt64Range (textRange) - paragraph->getRange().getStart(),
                                   glyphRanges);

        for (const auto& glyphRange : glyphRanges)
            for (const auto& bounds : shapedText.getGlyphsBounds (glyphRange))
                boundingBox.add (bounds.withY (bounds.getY() + paragraph->getTop()).getSmallestIntegerContainer());
    }

    boundingBox.offsetAll (getTextOffset());
    return boundingBox;
}

//==============================================================================
// Extra space for the cursor at the right-hand-edge
constexpr i32 rightEdgeSpace = 2;

i32 TextEditor::getWordWrapWidth() const
{
    return wordWrap ? getMaximumTextWidth()
                    : std::numeric_limits<i32>::max();
}

i32 TextEditor::getMaximumTextWidth() const
{
    return jmax (1, viewport->getMaximumVisibleWidth() - leftIndent - rightEdgeSpace);
}

i32 TextEditor::getMaximumTextHeight() const
{
    return jmax (1, viewport->getMaximumVisibleHeight() - topIndent);
}

z0 TextEditor::checkLayout()
{
    if (getWordWrapWidth() > 0)
    {
        const auto textBottom = topIndent
                                + (i32) std::ceil (getYOffset() + getTextStorageHeight());

        const auto maxTextWidth = std::accumulate (textStorage->begin(), textStorage->end(), 0.0f, [&](auto pMax, auto paragraph)
                                                   {
                                                       auto& shapedText = paragraph.value->getShapedText();

                                                       const auto paragraphWidth = std::accumulate (shapedText.getLineMetricsForGlyphRange().begin(),
                                                                                                    shapedText.getLineMetricsForGlyphRange().end(),
                                                                                                    0.0f,
                                                                                                    [&] (auto lMax, auto line)
                                                                                                    {
                                                                                                        return std::max (lMax,
                                                                                                                         line.value.effectiveLineLength);
                                                                                                    });

                                                       return std::max (pMax, paragraphWidth);
                                                   });

        const auto textRight = std::max (viewport->getMaximumVisibleWidth(),
                                         (i32) std::ceil (maxTextWidth) + leftIndent + rightEdgeSpace);

        textHolder->setSize (textRight, std::max (textBottom, viewport->getHeight()));
        viewport->setScrollBarsShown (scrollbarVisible && multiline && textBottom > viewport->getMaximumVisibleHeight(),
                                      scrollbarVisible && multiline && ! wordWrap && textRight > viewport->getMaximumVisibleWidth());
    }
}

i32 TextEditor::getTextWidth() const    { return textHolder->getWidth(); }
i32 TextEditor::getTextHeight() const   { return textHolder->getHeight(); }

z0 TextEditor::setIndents (i32 newLeftIndent, i32 newTopIndent)
{
    if (leftIndent != newLeftIndent || topIndent != newTopIndent)
    {
        leftIndent = newLeftIndent;
        topIndent  = newTopIndent;

        resized();
        repaint();
    }
}

z0 TextEditor::setBorder (BorderSize<i32> border)
{
    borderSize = border;
    resized();
}

BorderSize<i32> TextEditor::getBorder() const
{
    return borderSize;
}

z0 TextEditor::setScrollToShowCursor (const b8 shouldScrollToShowCursor)
{
    keepCaretOnScreen = shouldScrollToShowCursor;
}

z0 TextEditor::scrollToMakeSureCursorIsVisible()
{
    updateCaretPosition();

    if (keepCaretOnScreen)
    {
        auto viewPos = viewport->getViewPosition();
        auto caretRect = getCaretRectangle().translated (leftIndent, topIndent) - getTextOffset();
        auto relativeCursor = caretRect.getPosition() - viewPos;

        if (relativeCursor.x < jmax (1, proportionOfWidth (0.05f)))
        {
            viewPos.x += relativeCursor.x - proportionOfWidth (0.2f);
        }
        else if (relativeCursor.x > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
        {
            viewPos.x += relativeCursor.x + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();
        }

        viewPos.x = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), viewPos.x);

        if (! isMultiLine())
        {
            viewPos.y = (getHeight() - textHolder->getHeight() - topIndent) / -2;
        }
        else if (relativeCursor.y < 0)
        {
            viewPos.y = jmax (0, relativeCursor.y + viewPos.y);
        }
        else if (relativeCursor.y > jmax (0, viewport->getMaximumVisibleHeight() - caretRect.getHeight()))
        {
            viewPos.y += relativeCursor.y + 2 + caretRect.getHeight() - viewport->getMaximumVisibleHeight();
        }

        viewport->setViewPosition (viewPos);
    }
}

z0 TextEditor::moveCaretTo (i32k newPosition, const b8 isSelecting)
{
    if (isSelecting)
    {
        moveCaret (newPosition);

        auto oldSelection = selection;

        if (dragType == notDragging)
        {
            if (std::abs (getCaretPosition() - selection.getStart()) < std::abs (getCaretPosition() - selection.getEnd()))
                dragType = draggingSelectionStart;
            else
                dragType = draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            if (getCaretPosition() >= selection.getEnd())
                dragType = draggingSelectionEnd;

            setSelection (Range<i32>::between (getCaretPosition(), selection.getEnd()));
        }
        else
        {
            if (getCaretPosition() < selection.getStart())
                dragType = draggingSelectionStart;

            setSelection (Range<i32>::between (getCaretPosition(), selection.getStart()));
        }

        repaintText (selection.getUnionWith (oldSelection));
    }
    else
    {
        dragType = notDragging;

        repaintText (selection);

        moveCaret (newPosition);
        setSelection (Range<i32>::emptyRange (getCaretPosition()));
    }
}

i32 TextEditor::getTextIndexAt (i32k x, i32k y) const
{
    const auto offset = getTextOffset();

    return indexAtPosition ((f32) (x - offset.x),
                            (f32) (y - offset.y));
}

i32 TextEditor::getTextIndexAt (const Point<i32> pt) const
{
    return getTextIndexAt (pt.x, pt.y);
}

i32 TextEditor::getCharIndexForPoint (const Point<i32> point) const
{
    return getTextIndexAt (isMultiLine() ? point : getTextBounds ({ 0, getTotalNumChars() }).getBounds().getConstrainedPoint (point));
}

z0 TextEditor::insertTextAtCaret (const Txt& t)
{
    const auto filtered = inputFilter != nullptr ? inputFilter->filterNewText (*this, t) : t;
    const auto newText = isMultiLine() ? filtered.replace ("\r\n", "\n")
                                       : filtered.replaceCharacters ("\r\n", "  ");
    const auto insertIndex = selection.getStart();
    const auto newCaretPos = insertIndex + newText.length();

    remove (selection, getUndoManager(),
            newText.isNotEmpty() ? newCaretPos - 1 : newCaretPos);

    insert (newText, insertIndex, currentFont, findColor (textColorId),
            getUndoManager(), newCaretPos);

    textChanged();
}

z0 TextEditor::setHighlightedRegion (const Range<i32>& newSelection)
{
    if (newSelection == getHighlightedRegion())
        return;

    const auto cursorAtStart = newSelection.getEnd() == getHighlightedRegion().getStart()
                            || newSelection.getEnd() == getHighlightedRegion().getEnd();
    moveCaretTo (cursorAtStart ? newSelection.getEnd() : newSelection.getStart(), false);
    moveCaretTo (cursorAtStart ? newSelection.getStart() : newSelection.getEnd(), true);
}

//==============================================================================
z0 TextEditor::copy()
{
    if (passwordCharacter == 0)
    {
        auto selectedText = getHighlightedText();

        if (selectedText.isNotEmpty())
            SystemClipboard::copyTextToClipboard (selectedText);
    }
}

z0 TextEditor::paste()
{
    if (! isReadOnly())
    {
        auto clip = SystemClipboard::getTextFromClipboard();

        if (clip.isNotEmpty())
            insertTextAtCaret (clip);
    }
}

z0 TextEditor::cut()
{
    if (! isReadOnly())
    {
        moveCaret (selection.getEnd());
        insertTextAtCaret (Txt());
    }
}

static z0 drawUnderline (Graphics& g,
                           Span<const detail::ShapedGlyph> glyphs,
                           Span<const Point<f32>> positions,
                           const Font& font,
                           const AffineTransform& transform,
                           b8 underlineWhitespace)
{
    const auto lineThickness = font.getDescent() * 0.3f;

    const auto getLeft = [&] (const auto& iter)
    {
        return *(positions.begin() + std::distance (glyphs.begin(), iter));
    };

    const auto getRight = [&] (const auto& iter)
    {
        if (iter == glyphs.end())
            return positions.back() + glyphs.back().advance;

        const auto p = *(positions.begin() + std::distance (glyphs.begin(), iter));
        return p + iter->advance;
    };

    for (auto it = glyphs.begin(), end = glyphs.end(); it != end;)
    {
        const auto adjacent = std::adjacent_find (it,
                                                  end,
                                                  [] (const auto& a, const auto& b)
                                                  {
                                                      return a.isWhitespace() != b.isWhitespace();
                                                  });

        if (! it->isWhitespace() || underlineWhitespace)
        {
            const auto left = getLeft (it);
            const auto right = getRight (adjacent);

            Path p;
            p.addRectangle (left.x, left.y + lineThickness * 2.0f, right.x - left.x, lineThickness);
            g.fillPath (p, transform);
        }

        it = adjacent + (adjacent == end ? 0 : 1);;
    }
}

struct UseClip
{
    b8 clipAtBegin = false;
    b8 clipAtEnd = false;
};

// Glyphs can reach beyond the anchor - advance defined rectangle. We shouldn't use a clip unless
// we need to partially paint a ligature.
static UseClip getDrawableGlyphs (Span<const detail::ShapedGlyph> glyphs,
                                  Span<const Point<f32>> positions,
                                  std::vector<u16>& glyphIdsOut,
                                  std::vector<Point<f32>>& positionsOut)
{
    jassert (! glyphs.empty() && glyphs.size() == positions.size());

    glyphIdsOut.clear();
    positionsOut.clear();

    UseClip useClip;

    const auto& firstGlyph = glyphs.front();

    if (firstGlyph.isPlaceholderForLigature())
    {
        useClip.clipAtBegin = true;
        glyphIdsOut.push_back ((u16) firstGlyph.glyphId);
        positionsOut.push_back (positions[0] - (f32) firstGlyph.getDistanceFromLigature() * firstGlyph.advance);
    }

    i32 remainingLigaturePlaceholders = 0;

    for (const auto [index, glyph] : enumerate (glyphs, size_t{}))
    {
        if (glyph.isLigature())
            remainingLigaturePlaceholders += glyph.getNumTrailingLigaturePlaceholders();
        else
            remainingLigaturePlaceholders = std::max (0, remainingLigaturePlaceholders - 1);

        if (! glyph.isPlaceholderForLigature())
        {
            glyphIdsOut.push_back ((u16) glyph.glyphId);
            positionsOut.push_back (positions[index]);
        }
   }

   useClip.clipAtEnd = remainingLigaturePlaceholders > 0;
   return useClip;
}

//==============================================================================
z0 TextEditor::drawContent (Graphics& g)
{
    using namespace detail;

    g.setOrigin (leftIndent, topIndent);
    f32 yOffset = getYOffset();

    Graphics::ScopedSaveState ss (g);

    detail::Ranges::Operations ops;

    const auto glyphColors = getGlyphRanges (textStorage->getColors());

    const auto selectedTextRanges = std::invoke ([&]
    {
        detail::RangedValues<i8> rv;
        rv.set (asInt64Range (selection), 1, ops);
        return getGlyphRanges (rv);
    });

    const auto textSelectionMask = std::invoke ([&]
    {
        ops.clear();

        detail::RangedValues<i8> rv;
        rv.set ({ 0, textStorage->getTotalNumGlyphs() }, 0, ops);

        for (const auto item : selectedTextRanges)
            rv.set (item.range, item.value, ops);

        return rv;
    });

    const auto underlining = std::invoke ([&]
    {
        ops.clear();

        detail::RangedValues<i32> rv;
        rv.set ({ 0, textStorage->getTotalNumChars() }, 0, ops);

        for (const auto& underlined : underlinedSections)
            rv.set (asInt64Range (underlined), 1, ops);

        return getGlyphRanges (rv);
    });

    const auto drawSelection = [&] (Span<const detail::ShapedGlyph> glyphs,
                                    Span<const Point<f32>> positions,
                                    Font font,
                                    Range<z64>,
                                    LineMetrics,
                                    i32)
    {
        g.setColor (findColor (highlightColorId).withMultipliedAlpha (hasKeyboardFocus (true) ? 1.0f : 0.5f));
        g.fillRect ({ positions.front().translated (0.0f, -font.getAscent() + yOffset),
                      positions.back().translated (glyphs.back().advance.getX(), font.getDescent() + yOffset) });
    };

    std::vector<u16> glyphIds;
    std::vector<Point<f32>> positionsForGlyphIds;

    const auto drawGlyphRuns = [&] (Span<const detail::ShapedGlyph> glyphs,
                                    Span<const Point<f32>> positions,
                                    Font font,
                                    Range<z64>,
                                    LineMetrics,
                                    Color colour,
                                    i32 isSelected,
                                    i32 hasTemporaryUnderlining)
    {
        auto& context = g.getInternalContext();

        if (context.getFont() != font)
            context.setFont (font);

        const auto transform = AffineTransform::translation (0.0f, yOffset);

        g.setColor (isSelected ? findColor (highlightedTextColorId) : colour);

        glyphIds.clear();
        positionsForGlyphIds.clear();

        const auto useClip = getDrawableGlyphs (glyphs, positions, glyphIds, positionsForGlyphIds);

        {
            // Graphics::ScopedSaveState doesn't restore the clipping regions
            context.saveState();
            const ScopeGuard restoreGraphicsContext { [&context] { context.restoreState(); } };

            if (useClip.clipAtBegin || useClip.clipAtEnd)
            {
                const auto componentBoundsInDrawBasis = getLocalBounds().toFloat().transformedBy (transform.inverted());

                // We don't really want to constrain the vertical clip, so we add/subtract a little extra,
                // because clipping right at the line 0 will still result in a visible clip border with
                // the below code.
                const auto clipTop    = componentBoundsInDrawBasis.getY() - 10.0f;
                const auto clipBottom = componentBoundsInDrawBasis.getBottom() + 10.0f;
                const auto clipX      = useClip.clipAtBegin ? positions.front().getX() : 0.0f;
                const auto clipRight  = useClip.clipAtEnd ? positions.back().getX() + glyphs.back().advance.getX()
                                                          : (f32) getRight();

                const Rectangle<f32> clipRect { { clipX, clipTop }, { clipRight, clipBottom } };
                Path clipPath;
                clipPath.addRectangle (clipRect);
                context.clipToPath (clipPath, transform);
            }

            context.drawGlyphs (glyphIds, positionsForGlyphIds, transform);
        }

        if (font.isUnderlined())
            drawUnderline (g, glyphs, positions, font,  transform, isWhitespaceUnderlined());

        if (hasTemporaryUnderlining)
        {
            const auto startX = roundToInt (positions.front().getX());
            const auto endX = roundToInt (positions.back().getX() + glyphs.back().advance.getX());
            auto baselineY = roundToInt (positions.front().getY() + 0.5f);

            Graphics::ScopedSaveState state (g);
            g.addTransform (transform);
            g.reduceClipRegion ({ startX, baselineY, endX - startX, 1 });

            g.fillCheckerBoard ({ (f32) endX, (f32) baselineY + 1.0f },
                                3.0f,
                                1.0f,
                                colour,
                                Colors::transparentBlack);
        }
    };

    const auto clip = std::invoke ([&]
    {
        auto c = g.getClipBounds();
        c.setY (roundToInt ((f32) c.getY() - yOffset));
        return c;
    });

    for (auto [range, paragraph] : *textStorage)
    {
        ignoreUnused (range);

        const auto glyphsRange = Range<z64>::withStartAndLength (paragraph->getStartingGlyph(),
                                                                   paragraph->getNumGlyphs());

        const auto top = paragraph->getTop();
        const auto bottom = top + paragraph->getHeight();

        if ((f32) clip.getY() <= bottom && top <= (f32) clip.getBottom())
        {
            paragraph->getShapedText().accessTogetherWith (drawSelection,
                                                           selectedTextRanges.getIntersectionsStartingAtZeroWith (glyphsRange));

            paragraph->getShapedText().accessTogetherWith (drawGlyphRuns,
                                                           glyphColors,
                                                           textSelectionMask.getIntersectionsStartingAtZeroWith (glyphsRange),
                                                           underlining.getIntersectionsStartingAtZeroWith (glyphsRange));
        }

        yOffset += paragraph->getHeight();
    }
}

z0 TextEditor::paint (Graphics& g)
{
    getLookAndFeel().fillTextEditorBackground (g, getWidth(), getHeight(), *this);
}

z0 TextEditor::paintOverChildren (Graphics& g)
{
    if (textToShowWhenEmpty.isNotEmpty()
         && (! hasKeyboardFocus (false))
         && getTotalNumChars() == 0)
    {
        g.setColor (colourForTextWhenEmpty);
        g.setFont (getFont());

        Rectangle<i32> textBounds (leftIndent,
                                   topIndent,
                                   viewport->getWidth() - leftIndent,
                                   getHeight() - topIndent);

        if (! textBounds.isEmpty())
            g.drawText (textToShowWhenEmpty, textBounds, justification, true);
    }

    getLookAndFeel().drawTextEditorOutline (g, getWidth(), getHeight(), *this);
}

//==============================================================================
z0 TextEditor::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    const b8 writable = ! isReadOnly();

    if (passwordCharacter == 0)
    {
        m.addItem (StandardApplicationCommandIDs::cut,   TRANS ("Cut"), writable);
        m.addItem (StandardApplicationCommandIDs::copy,  TRANS ("Copy"), ! selection.isEmpty());
    }

    m.addItem (StandardApplicationCommandIDs::paste,     TRANS ("Paste"), writable);
    m.addItem (StandardApplicationCommandIDs::del,       TRANS ("Delete"), writable);
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::selectAll, TRANS ("Select All"));
    m.addSeparator();

    if (getUndoManager() != nullptr)
    {
        m.addItem (StandardApplicationCommandIDs::undo, TRANS ("Undo"), undoManager.canUndo());
        m.addItem (StandardApplicationCommandIDs::redo, TRANS ("Redo"), undoManager.canRedo());
    }
}

z0 TextEditor::performPopupMenuAction (i32k menuItemID)
{
    switch (menuItemID)
    {
        case StandardApplicationCommandIDs::cut:        cutToClipboard(); break;
        case StandardApplicationCommandIDs::copy:       copyToClipboard(); break;
        case StandardApplicationCommandIDs::paste:      pasteFromClipboard(); break;
        case StandardApplicationCommandIDs::del:        cut(); break;
        case StandardApplicationCommandIDs::selectAll:  selectAll(); break;
        case StandardApplicationCommandIDs::undo:       undo(); break;
        case StandardApplicationCommandIDs::redo:       redo(); break;
        default: break;
    }
}

//==============================================================================
z0 TextEditor::mouseDown (const MouseEvent& e)
{
    beginDragAutoRepeat (100);
    newTransaction();

    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            caretState.setPreferredEdge (Edge::leading);
            moveCaretTo (getTextIndexAt (e.getPosition()), e.mods.isShiftDown());

            if (auto* peer = getPeer())
                peer->closeInputMethodContext();
        }
        else
        {
            PopupMenu m;
            m.setLookAndFeel (&getLookAndFeel());
            addPopupMenuItems (m, &e);

            menuActive = true;

            m.showMenuAsync (PopupMenu::Options(),
                             [safeThis = SafePointer<TextEditor> { this }] (i32 menuResult)
                             {
                                 if (auto* editor = safeThis.getComponent())
                                 {
                                     editor->menuActive = false;

                                     if (menuResult != 0)
                                         editor->performPopupMenuAction (menuResult);
                                 }
                             });
        }
    }
}

z0 TextEditor::mouseDrag (const MouseEvent& e)
{
    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            caretState.setPreferredEdge (Edge::leading);
            moveCaretTo (getTextIndexAt (e.getPosition()), true);
        }
    }
}

z0 TextEditor::mouseUp (const MouseEvent& e)
{
    newTransaction();
    textHolder->restartTimer();

    if (wasFocused || ! selectAllTextWhenFocused)
        if (e.mouseWasClicked() && ! (popupMenuEnabled && e.mods.isPopupMenu()))
            moveCaret (getTextIndexAt (e.getPosition()));

    wasFocused = true;
}

z0 TextEditor::mouseDoubleClick (const MouseEvent& e)
{
    i32 tokenEnd = getTextIndexAt (e.getPosition());
    i32 tokenStart = 0;

    if (e.getNumberOfClicks() > 3)
    {
        tokenEnd = getTotalNumChars();
    }
    else
    {
        auto t = getText();
        auto totalLength = getTotalNumChars();

        while (tokenEnd < totalLength)
        {
            auto c = t[tokenEnd];

            // (note the slight bodge here - it's because iswalnum only checks for alphabetic chars in the current locale)
            if (CharacterFunctions::isLetterOrDigit (c) || c > 128)
                ++tokenEnd;
            else
                break;
        }

        tokenStart = tokenEnd;

        while (tokenStart > 0)
        {
            auto c = t[tokenStart - 1];

            // (note the slight bodge here - it's because iswalnum only checks for alphabetic chars in the current locale)
            if (CharacterFunctions::isLetterOrDigit (c) || c > 128)
                --tokenStart;
            else
                break;
        }

        if (e.getNumberOfClicks() > 2)
        {
            while (tokenEnd < totalLength)
            {
                auto c = t[tokenEnd];

                if (c != '\r' && c != '\n')
                    ++tokenEnd;
                else
                    break;
            }

            while (tokenStart > 0)
            {
                auto c = t[tokenStart - 1];

                if (c != '\r' && c != '\n')
                    --tokenStart;
                else
                    break;
            }
        }
    }

    moveCaretTo (tokenEnd, false);
    moveCaretTo (tokenStart, true);
}

z0 TextEditor::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! viewport->useMouseWheelMoveIfNeeded (e, wheel))
        Component::mouseWheelMove (e, wheel);
}

//==============================================================================
b8 TextEditor::moveCaretWithTransaction (i32k newPos, const b8 selecting)
{
    newTransaction();
    moveCaretTo (newPos, selecting);

    if (auto* peer = getPeer())
        peer->closeInputMethodContext();

    return true;
}

b8 TextEditor::moveCaretLeft (b8 moveInWholeWordSteps, b8 selecting)
{
    auto pos = getCaretPosition();

    if (moveInWholeWordSteps)
        pos = findWordBreakBefore (pos);
    else
        --pos;

    return moveCaretWithTransaction (pos, selecting);
}

b8 TextEditor::moveCaretRight (b8 moveInWholeWordSteps, b8 selecting)
{
    auto pos = getCaretPosition();

    if (moveInWholeWordSteps)
        pos = findWordBreakAfter (pos);
    else
        ++pos;

    return moveCaretWithTransaction (pos, selecting);
}

TextEditor::Edge TextEditor::getEdgeTypeCloserToPosition (i32 indexInText, Point<f32> pos) const
{
    const auto testCaret = caretState.withPosition (indexInText);

    const auto leading = getCursorEdge (testCaret.withPreferredEdge (Edge::leading)).anchor.getDistanceFrom (pos);
    const auto trailing = getCursorEdge (testCaret.withPreferredEdge (Edge::trailing)).anchor.getDistanceFrom (pos);

    if (leading < trailing)
        return Edge::leading;

    return Edge::trailing;
}

b8 TextEditor::moveCaretUp (b8 selecting)
{
    if (! isMultiLine())
        return moveCaretToStartOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();

    const auto newY = caretPos.getY() - 1.0f;

    if (newY < 0.0f)
        return moveCaretToStartOfLine (selecting);

    const Point<f32> testPosition { caretPos.getX(), newY };
    const auto newIndex = indexAtPosition (testPosition.getX(), testPosition.getY());

    const auto edgeToUse = getEdgeTypeCloserToPosition (newIndex, testPosition);
    caretState.setPreferredEdge (edgeToUse);
    return moveCaretWithTransaction (newIndex, selecting);
}

b8 TextEditor::moveCaretDown (b8 selecting)
{
    if (! isMultiLine())
        return moveCaretToEndOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();
    const Point<f32> testPosition { caretPos.getX(), caretPos.getBottom() + 1.0f };
    const auto newIndex = indexAtPosition (testPosition.getX(), testPosition.getY());

    const auto edgeToUse = getEdgeTypeCloserToPosition (newIndex, testPosition);
    caretState.setPreferredEdge (edgeToUse);
    return moveCaretWithTransaction (newIndex, selecting);
}

b8 TextEditor::pageUp (b8 selecting)
{
    if (! isMultiLine())
        return moveCaretToStartOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getY() - (f32) viewport->getViewHeight()), selecting);
}

b8 TextEditor::pageDown (b8 selecting)
{
    if (! isMultiLine())
        return moveCaretToEndOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getBottom() + (f32) viewport->getViewHeight()), selecting);
}

z0 TextEditor::scrollByLines (i32 deltaLines)
{
    viewport->getVerticalScrollBar().moveScrollbarInSteps (deltaLines);
}

b8 TextEditor::scrollDown()
{
    scrollByLines (-1);
    return true;
}

b8 TextEditor::scrollUp()
{
    scrollByLines (1);
    return true;
}

b8 TextEditor::moveCaretToTop (b8 selecting)
{
    return moveCaretWithTransaction (0, selecting);
}

b8 TextEditor::moveCaretToStartOfLine (b8 selecting)
{
    const auto lineRange = getLineRangeForIndex (caretState.getVisualIndex());
    caretState.setPreferredEdge (Edge::leading);
    return moveCaretWithTransaction ((i32) lineRange.getStart(), selecting);
}

b8 TextEditor::moveCaretToEnd (b8 selecting)
{
    return moveCaretWithTransaction (getTotalNumChars(), selecting);
}

b8 TextEditor::moveCaretToEndOfLine (b8 selecting)
{
    const auto lineRange = getLineRangeForIndex (caretState.getVisualIndex());
    caretState.setPreferredEdge (Edge::trailing);
    return moveCaretWithTransaction ((i32) lineRange.getEnd(), selecting);
}

b8 TextEditor::deleteBackwards (b8 moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
        moveCaretTo (findWordBreakBefore (getCaretPosition()), true);
    else if (selection.isEmpty() && selection.getStart() > 0)
        setSelection ({ selection.getEnd() - 1, selection.getEnd() });

    cut();
    return true;
}

b8 TextEditor::deleteForwards (b8 /*moveInWholeWordSteps*/)
{
    if (selection.isEmpty() && selection.getStart() < getTotalNumChars())
        setSelection ({ selection.getStart(), selection.getStart() + 1 });

    cut();
    return true;
}

b8 TextEditor::copyToClipboard()
{
    newTransaction();
    copy();
    return true;
}

b8 TextEditor::cutToClipboard()
{
    newTransaction();
    copy();
    cut();
    return true;
}

b8 TextEditor::pasteFromClipboard()
{
    newTransaction();
    paste();
    return true;
}

b8 TextEditor::selectAll()
{
    newTransaction();
    moveCaretTo (getTotalNumChars(), false);
    moveCaretTo (0, true);
    return true;
}

//==============================================================================
z0 TextEditor::setEscapeAndReturnKeysConsumed (b8 shouldBeConsumed) noexcept
{
    consumeEscAndReturnKeys = shouldBeConsumed;
}

b8 TextEditor::keyPressed (const KeyPress& key)
{
    if (isReadOnly() && key != KeyPress ('c', ModifierKeys::commandModifier, 0)
                     && key != KeyPress ('a', ModifierKeys::commandModifier, 0))
        return false;

    if (! TextEditorKeyMapper<TextEditor>::invokeKeyFunction (*this, key))
    {
        if (key == KeyPress::returnKey)
        {
            newTransaction();

            if (returnKeyStartsNewLine)
            {
                insertTextAtCaret ("\n");
            }
            else
            {
                returnPressed();
                return consumeEscAndReturnKeys;
            }
        }
        else if (key.isKeyCode (KeyPress::escapeKey))
        {
            newTransaction();
            moveCaretTo (getCaretPosition(), false);
            escapePressed();
            return consumeEscAndReturnKeys;
        }
        else if (key.getTextCharacter() >= ' '
                  || (tabKeyUsed && (key.getTextCharacter() == '\t')))
        {
            insertTextAtCaret (Txt::charToString (key.getTextCharacter()));

            lastTransactionTime = Time::getApproximateMillisecondCounter();
        }
        else
        {
            return false;
        }
    }

    return true;
}

b8 TextEditor::keyStateChanged (const b8 isKeyDown)
{
    if (! isKeyDown)
        return false;

   #if DRX_WINDOWS
    if (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0).isCurrentlyDown())
        return false;  // We need to explicitly allow alt-F4 to pass through on Windows
   #endif

    if ((! consumeEscAndReturnKeys)
         && (KeyPress (KeyPress::escapeKey).isCurrentlyDown()
          || KeyPress (KeyPress::returnKey).isCurrentlyDown()))
        return false;

    // (overridden to avoid forwarding key events to the parent)
    return ! ModifierKeys::currentModifiers.isCommandDown();
}

//==============================================================================
z0 TextEditor::focusGained (FocusChangeType cause)
{
    newTransaction();

    if (selectAllTextWhenFocused)
    {
        moveCaretTo (0, false);
        moveCaretTo (getTotalNumChars(), true);
    }

    checkFocus();

    if (cause == FocusChangeType::focusChangedByMouseClick && selectAllTextWhenFocused)
        wasFocused = false;

    repaint();
    updateCaretPosition();
}

z0 TextEditor::focusLost (FocusChangeType)
{
    newTransaction();

    wasFocused = false;
    textHolder->stopTimer();

    underlinedSections.clear();

    updateCaretPosition();

    postCommandMessage (TextEditorDefs::focusLossMessageId);
    repaint();
}

//==============================================================================
z0 TextEditor::resized()
{
    viewport->setBoundsInset (borderSize);
    viewport->setSingleStepSizes (16, roundToInt (currentFont.getHeight()));
    updateBaseShapedTextOptions();

    checkLayout();

    if (isMultiLine())
        updateCaretPosition();
    else
        scrollToMakeSureCursorIsVisible();
}

z0 TextEditor::handleCommandMessage (i32k commandId)
{
    Component::BailOutChecker checker (this);

    switch (commandId)
    {
    case TextEditorDefs::textChangeMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorTextChanged (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onTextChange);

        break;

    case TextEditorDefs::returnKeyMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorReturnKeyPressed (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onReturnKey);

        break;

    case TextEditorDefs::escapeKeyMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorEscapeKeyPressed (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onEscapeKey);

        break;

    case TextEditorDefs::focusLossMessageId:
        updateValueFromText();
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorFocusLost (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onFocusLost);

        break;

    default:
        jassertfalse;
        break;
    }
}

z0 TextEditor::setTemporaryUnderlining (const Array<Range<i32>>& newUnderlinedSections)
{
    underlinedSections = newUnderlinedSections;
    repaint();
}

TextInputTarget::VirtualKeyboardType TextEditor::getKeyboardType()
{
    return passwordCharacter != 0 ? passwordKeyboard : keyboardType;
}

//==============================================================================
UndoManager* TextEditor::getUndoManager() noexcept
{
    return readOnly ? nullptr : &undoManager;
}

z0 TextEditor::clearInternal (UndoManager* const um)
{
    remove ({ 0, getTotalNumChars() }, um, getCaretPosition());
}

z0 TextEditor::insert (const Txt& text, i32 insertIndex, const Font& font,
                         Color colour, UndoManager* um, i32 caretPositionToMoveTo)
{
    if (text.isNotEmpty())
    {
        if (um != nullptr)
        {
            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new InsertAction (*this, text, insertIndex, font, colour,
                                           getCaretPosition(), caretPositionToMoveTo));
        }
        else
        {
            textStorage->set ({ insertIndex, insertIndex }, text, font, colour);
            caretState.updateEdge();

            repaintText ({ insertIndex, getTotalNumChars() }); // must do this before and after changing the data, in case
                                                               // a line gets moved due to word wrap

            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            checkLayout();
            moveCaretTo (caretPositionToMoveTo, false);

            repaintText ({ insertIndex, getTotalNumChars() });
        }
    }
}

z0 TextEditor::reinsert (const TextEditorStorageChunks& chunks)
{
    textStorage->addChunks (chunks);
    totalNumChars = -1;
    valueTextNeedsUpdating = true;
}

z0 TextEditor::remove (Range<i32> range,
                         UndoManager* const um,
                         i32k caretPositionToMoveTo,
                         TextEditorStorageChunks* removedOut)
{
    using namespace detail;

    if (! range.isEmpty())
    {
        if (um != nullptr)
        {
            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new RemoveAction (*this, range, caretState.getPosition(),
                                           caretPositionToMoveTo));
        }
        else
        {
            textStorage->remove (asInt64Range (range), removedOut);
            caretState.updateEdge();

            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            checkLayout();
            moveCaretTo (caretPositionToMoveTo, false);

            repaintText ({ range.getStart(), getTotalNumChars() });
        }
    }
}

//==============================================================================
Txt TextEditor::getText() const
{
    return textStorage->getText();
}

Txt TextEditor::getTextInRange (const Range<i32>& range) const
{
    return textStorage->getTextInRange (asInt64Range (range));
}

Txt TextEditor::getHighlightedText() const
{
    return getTextInRange (selection);
}

i32 TextEditor::getTotalNumChars() const
{
    return (i32) textStorage->getTotalNumChars();
}

b8 TextEditor::isEmpty() const
{
    return getTotalNumChars() == 0;
}

f32 TextEditor::getJustificationOffsetX() const
{
    const auto bottomRightX = (f32) getMaximumTextWidth();

    if (justification.testFlags (Justification::horizontallyCentred)) return jmax (0.0f, bottomRightX * 0.5f);
    if (justification.testFlags (Justification::right))               return jmax (0.0f, bottomRightX);

    return 0.0f;
}

TextEditor::CaretEdge TextEditor::getDefaultCursorEdge() const
{
    return { { getJustificationOffsetX(), 0.0f }, currentFont.getHeight() };
}

TextEditor::CaretEdge TextEditor::getCursorEdge (const CaretState& tempCaret) const
{
    const auto visualIndex = tempCaret.getVisualIndex();
    jassert (0 <= visualIndex && visualIndex <= getTotalNumChars());

    if (getWordWrapWidth() <= 0)
        return { {}, currentFont.getHeight() };

    if (textStorage->isEmpty())
        return getDefaultCursorEdge();

    if (visualIndex == getTotalNumChars())
    {
        const auto& lastParagraph = textStorage->back().value;

        return { { getJustificationOffsetX(), lastParagraph->getTop() + lastParagraph->getHeight() },
                 currentFont.getHeight() };
    }

    return getTextSelectionEdge (visualIndex, tempCaret.getEdge());
}

i32 TextEditor::indexAtPosition (f32 x, f32 y) const
{
    y = std::max (0.0f, y);

    if (getWordWrapWidth() <= 0)
        return getTotalNumChars();

    auto paragraphIt = textStorage->begin();
    f32 paragraphTop = 0.0f;

    while (paragraphIt != textStorage->end())
    {
        auto& paragraph = paragraphIt->value;
        const auto paragraphBottom = paragraphTop + paragraph->getHeight();

        if (paragraphTop <= y && y < paragraphBottom)
            break;

        if (y < paragraphTop)
            return {};

        paragraphTop = paragraphBottom;
        ++paragraphIt;
    }

    if (paragraphIt == textStorage->end())
        return getTotalNumChars();

    auto& shapedText = paragraphIt->value->getShapedText();
    return (i32) (shapedText.getTextIndexForCaret ({ x, y - paragraphTop }) + paragraphIt->range.getStart());
}

//==============================================================================
i32 TextEditor::findWordBreakAfter (i32k position) const
{
    auto t = getTextInRange ({ position, position + 512 });
    auto totalLength = t.length();
    i32 i = 0;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    auto type = TextEditorDefs::getCharacterCategory (t[i]);

    while (i < totalLength && type == TextEditorDefs::getCharacterCategory (t[i]))
        ++i;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    return position + i;
}

i32 TextEditor::findWordBreakBefore (i32k position) const
{
    if (position <= 0)
        return 0;

    auto startOfBuffer = jmax (0, position - 512);
    auto t = getTextInRange ({ startOfBuffer, position });

    i32 i = position - startOfBuffer;

    while (i > 0 && CharacterFunctions::isWhitespace (t [i - 1]))
        --i;

    if (i > 0)
    {
        auto type = TextEditorDefs::getCharacterCategory (t [i - 1]);

        while (i > 0 && type == TextEditorDefs::getCharacterCategory (t [i - 1]))
            --i;
    }

    jassert (startOfBuffer + i >= 0);
    return startOfBuffer + i;
}

//==============================================================================
TextEditor::CaretState::CaretState (const TextEditor* ownerIn)
    : owner { *ownerIn }
{
    updateEdge();
}

z0 TextEditor::CaretState::setPosition (i32 newPosition)
{
    if (std::exchange (position, newPosition) != newPosition)
        updateEdge();
}

z0 TextEditor::CaretState::setPreferredEdge (TextEditor::Edge newEdge)
{
    if (std::exchange (preferredEdge, newEdge) != newEdge)
        updateEdge();
}

i32 TextEditor::CaretState::getVisualIndex() const
{
    if (edge == Edge::leading)
        return position;

    return position - 1;
}

TextEditor::CaretState TextEditor::CaretState::withPosition (i32 newPosition) const
{
    auto copy = *this;
    copy.setPosition (newPosition);
    return copy;
}

TextEditor::CaretState TextEditor::CaretState::withPreferredEdge (Edge newEdge) const
{
    auto copy = *this;
    copy.setPreferredEdge (newEdge);
    return copy;
}

z0 TextEditor::CaretState::updateEdge()
{
    // The position can be temporarily outside the current text's bounds. It's the TextEditor's
    // responsibility to update the caret position after editing operations.
    const auto clampedPosition = std::clamp (position, 0, owner.getTotalNumChars());

    if (clampedPosition == 0)
    {
        edge = Edge::leading;
    }
    else if (owner.getText()[clampedPosition - 1] == '\n')
    {
        edge = Edge::leading;
    }
    else if (clampedPosition == owner.getTotalNumChars())
    {
        edge = Edge::trailing;
    }
    else
    {
        edge = preferredEdge;
    }
}

//==============================================================================
class TextEditor::EditorAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit EditorAccessibilityHandler (TextEditor& textEditorToWrap)
        : AccessibilityHandler (textEditorToWrap,
                                textEditorToWrap.isReadOnly() ? AccessibilityRole::staticText : AccessibilityRole::editableText,
                                {},
                                { std::make_unique<TextEditorTextInterface> (textEditorToWrap) }),
          textEditor (textEditorToWrap)
    {
    }

    Txt getHelp() const override  { return textEditor.getTooltip(); }

private:
    class TextEditorTextInterface final : public AccessibilityTextInterface
    {
    public:
        explicit TextEditorTextInterface (TextEditor& editor)
            : textEditor (editor)
        {
        }

        b8 isDisplayingProtectedText() const override      { return textEditor.getPasswordCharacter() != 0; }
        b8 isReadOnly() const override                     { return textEditor.isReadOnly(); }

        i32 getTotalNumCharacters() const override           { return textEditor.getText().length(); }
        Range<i32> getSelection() const override             { return textEditor.getHighlightedRegion(); }

        z0 setSelection (Range<i32> r) override
        {
            textEditor.setHighlightedRegion (r);
        }

        Txt getText (Range<i32> r) const override
        {
            if (isDisplayingProtectedText())
                return Txt::repeatedString (Txt::charToString (textEditor.getPasswordCharacter()),
                                               getTotalNumCharacters());

            return textEditor.getTextInRange (r);
        }

        z0 setText (const Txt& newText) override
        {
            textEditor.setText (newText);
        }

        i32 getTextInsertionOffset() const override          { return textEditor.getCaretPosition(); }

        RectangleList<i32> getTextBounds (Range<i32> textRange) const override
        {
            auto localRects = textEditor.getTextBounds (textRange);
            RectangleList<i32> globalRects;

            std::for_each (localRects.begin(), localRects.end(),
                           [&] (const Rectangle<i32>& r) { globalRects.add (textEditor.localAreaToGlobal (r)); });

            return globalRects;
        }

        i32 getOffsetAtPoint (Point<i32> point) const override
        {
            return textEditor.getTextIndexAt (textEditor.getLocalPoint (nullptr, point));
        }

    private:
        TextEditor& textEditor;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditorTextInterface)
    };

    TextEditor& textEditor;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorAccessibilityHandler)
};

std::unique_ptr<AccessibilityHandler> TextEditor::createAccessibilityHandler()
{
    return std::make_unique<EditorAccessibilityHandler> (*this);
}

} // namespace drx
