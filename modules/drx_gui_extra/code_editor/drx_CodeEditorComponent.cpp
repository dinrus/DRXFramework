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
class CodeEditorComponent::CodeEditorAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit CodeEditorAccessibilityHandler (CodeEditorComponent& codeEditorComponentToWrap)
        : AccessibilityHandler (codeEditorComponentToWrap,
                                codeEditorComponentToWrap.isReadOnly() ? AccessibilityRole::staticText
                                                                       : AccessibilityRole::editableText,
                                {},
                                { std::make_unique<CodeEditorComponentTextInterface> (codeEditorComponentToWrap) })
    {
    }

private:
    class CodeEditorComponentTextInterface final : public AccessibilityTextInterface
    {
    public:
        explicit CodeEditorComponentTextInterface (CodeEditorComponent& codeEditorComponentToWrap)
            : codeEditorComponent (codeEditorComponentToWrap)
        {
        }

        b8 isDisplayingProtectedText() const override
        {
            return false;
        }

        b8 isReadOnly() const override
        {
            return codeEditorComponent.isReadOnly();
        }

        i32 getTotalNumCharacters() const override
        {
            return codeEditorComponent.document.getAllContent().length();
        }

        Range<i32> getSelection() const override
        {
            return { codeEditorComponent.selectionStart.getPosition(),
                     codeEditorComponent.selectionEnd.getPosition() };
        }

        z0 setSelection (Range<i32> r) override
        {
            codeEditorComponent.setHighlightedRegion (r);
        }

        Txt getText (Range<i32> r) const override
        {
            auto& doc = codeEditorComponent.document;

            return doc.getTextBetween (CodeDocument::Position (doc, r.getStart()),
                                       CodeDocument::Position (doc, r.getEnd()));
        }

        z0 setText (const Txt& newText) override
        {
            codeEditorComponent.document.replaceAllContent (newText);
        }

        i32 getTextInsertionOffset() const override
        {
            return codeEditorComponent.caretPos.getPosition();
        }

        RectangleList<i32> getTextBounds (Range<i32> textRange) const override
        {
            const auto localRects = codeEditorComponent.getTextBounds (textRange);

            RectangleList<i32> globalRects;

            for (auto r : localRects)
                globalRects.add (codeEditorComponent.localAreaToGlobal (r));

            return globalRects;
        }

        i32 getOffsetAtPoint (Point<i32> point) const override
        {
            return codeEditorComponent.getPositionAt (point.x, point.y).getPosition();
        }

    private:
        CodeEditorComponent& codeEditorComponent;
    };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorAccessibilityHandler)
};

//==============================================================================
class CodeEditorComponent::CodeEditorLine
{
public:
    CodeEditorLine() noexcept {}

    b8 update (CodeDocument& codeDoc, i32 lineNum,
                 CodeDocument::Iterator& source,
                 CodeTokeniser* tokeniser, i32k tabSpaces,
                 const CodeDocument::Position& selStart,
                 const CodeDocument::Position& selEnd)
    {
        Array<SyntaxToken> newTokens;
        newTokens.ensureStorageAllocated (8);

        if (tokeniser == nullptr)
        {
            auto line = codeDoc.getLine (lineNum);
            addToken (newTokens, line, line.length(), -1);
        }
        else if (lineNum < codeDoc.getNumLines())
        {
            const CodeDocument::Position pos (codeDoc, lineNum, 0);
            createTokens (pos.getPosition(), pos.getLineText(),
                          source, *tokeniser, newTokens);
        }

        replaceTabsWithSpaces (newTokens, tabSpaces);

        i32 newHighlightStart = 0;
        i32 newHighlightEnd = 0;

        if (selStart.getLineNumber() <= lineNum && selEnd.getLineNumber() >= lineNum)
        {
            auto line = codeDoc.getLine (lineNum);

            CodeDocument::Position lineStart (codeDoc, lineNum, 0), lineEnd (codeDoc, lineNum + 1, 0);
            newHighlightStart = indexToColumn (jmax (0, selStart.getPosition() - lineStart.getPosition()),
                                               line, tabSpaces);
            newHighlightEnd = indexToColumn (jmin (lineEnd.getPosition() - lineStart.getPosition(), selEnd.getPosition() - lineStart.getPosition()),
                                             line, tabSpaces);
        }

        if (newHighlightStart != highlightColumnStart || newHighlightEnd != highlightColumnEnd)
        {
            highlightColumnStart = newHighlightStart;
            highlightColumnEnd = newHighlightEnd;
        }
        else if (tokens == newTokens)
        {
            return false;
        }

        tokens.swapWith (newTokens);
        return true;
    }

    Optional<Rectangle<f32>> getHighlightArea (f32 x, i32 y, i32 lineH, f32 characterWidth) const
    {
        return getHighlightArea (x, y, lineH, characterWidth, { highlightColumnStart, highlightColumnEnd });
    }

    Optional<Rectangle<f32>> getHighlightArea (f32 x,
                                                 i32 y,
                                                 i32 lineH,
                                                 f32 characterWidth,
                                                 Range<i32> highlightColumns) const
    {
        if (highlightColumns.isEmpty())
            return {};

        return Rectangle<f32> (x + (f32) highlightColumns.getStart() * characterWidth - 1.0f, (f32) y - 0.5f,
                                 (f32) (highlightColumns.getEnd() - highlightColumns.getStart()) * characterWidth + 1.5f, (f32) lineH + 1.0f);

    }

    z0 draw (CodeEditorComponent& owner, Graphics& g, const Font& fontToUse,
               const f32 rightClip, const f32 x, i32k y,
               i32k lineH, const f32 characterWidth) const
    {
        AttributedString as;
        as.setJustification (Justification::centredLeft);

        i32 column = 0;

        for (auto& token : tokens)
        {
            const f32 tokenX = x + (f32) column * characterWidth;
            if (tokenX > rightClip)
                break;

            as.append (token.text.initialSectionNotContaining ("\r\n"), fontToUse, owner.getColorForTokenType (token.tokenType));
            column += token.length;
        }

        as.draw (g, { x, (f32) y, (f32) column * characterWidth + 10.0f, (f32) lineH });
    }

private:
    struct SyntaxToken
    {
        SyntaxToken (const Txt& t, i32k len, i32k type) noexcept
            : text (t), length (len), tokenType (type)
        {}

        b8 operator== (const SyntaxToken& other) const noexcept
        {
            return tokenType == other.tokenType
                    && length == other.length
                    && text == other.text;
        }

        Txt text;
        i32 length;
        i32 tokenType;
    };

    Array<SyntaxToken> tokens;
    i32 highlightColumnStart = 0, highlightColumnEnd = 0;

    static z0 createTokens (i32 startPosition, const Txt& lineText,
                              CodeDocument::Iterator& source,
                              CodeTokeniser& tokeniser,
                              Array<SyntaxToken>& newTokens)
    {
        CodeDocument::Iterator lastIterator (source);
        i32k lineLength = lineText.length();

        for (;;)
        {
            i32 tokenType = tokeniser.readNextToken (source);
            i32 tokenStart = lastIterator.getPosition();
            i32 tokenEnd = source.getPosition();

            if (tokenEnd <= tokenStart)
                break;

            tokenEnd -= startPosition;

            if (tokenEnd > 0)
            {
                tokenStart -= startPosition;
                i32k start = jmax (0, tokenStart);
                addToken (newTokens, lineText.substring (start, tokenEnd),
                          tokenEnd - start, tokenType);

                if (tokenEnd >= lineLength)
                    break;
            }

            lastIterator = source;
        }

        source = lastIterator;
    }

    static z0 replaceTabsWithSpaces (Array<SyntaxToken>& tokens, i32k spacesPerTab)
    {
        i32 x = 0;

        for (auto& t : tokens)
        {
            for (;;)
            {
                i32k tabPos = t.text.indexOfChar ('\t');
                if (tabPos < 0)
                    break;

                i32k spacesNeeded = spacesPerTab - ((tabPos + x) % spacesPerTab);
                t.text = t.text.replaceSection (tabPos, 1, Txt::repeatedString (" ", spacesNeeded));
                t.length = t.text.length();
            }

            x += t.length;
        }
    }

    i32 indexToColumn (i32 index, const Txt& line, i32 tabSpaces) const noexcept
    {
        jassert (index <= line.length());

        auto t = line.getCharPointer();
        i32 col = 0;

        for (i32 i = 0; i < index; ++i)
        {
            if (t.getAndAdvance() != '\t')
                ++col;
            else
                col += tabSpaces - (col % tabSpaces);
        }

        return col;
    }

    static z0 addToken (Array<SyntaxToken>& dest, const Txt& text, i32 length, i32 type)
    {
        if (length > 1000)
        {
            // subdivide very i64 tokens to avoid unwieldy glyph sequences
            addToken (dest, text.substring (0, length / 2), length / 2, type);
            addToken (dest, text.substring (length / 2), length - length / 2, type);
        }
        else
        {
            dest.add (SyntaxToken (text, length, type));
        }
    }
};

namespace CodeEditorHelpers
{
    static i32 findFirstNonWhitespaceChar (StringRef line) noexcept
    {
        auto t = line.text;
        i32 i = 0;

        while (! t.isEmpty())
        {
            if (! t.isWhitespace())
                return i;

            ++t;
            ++i;
        }

        return 0;
    }
}

//==============================================================================
class CodeEditorComponent::Pimpl   : public Timer,
                                     public AsyncUpdater,
                                     public ScrollBar::Listener,
                                     public CodeDocument::Listener
{
public:
    Pimpl (CodeEditorComponent& ed) : owner (ed) {}

private:
    CodeEditorComponent& owner;

    z0 timerCallback() override        { owner.newTransaction(); }
    z0 handleAsyncUpdate() override    { owner.rebuildLineTokens(); }

    z0 scrollBarMoved (ScrollBar* scrollBarThatHasMoved, f64 newRangeStart) override
    {
        if (scrollBarThatHasMoved->isVertical())
            owner.scrollToLineInternal ((i32) newRangeStart);
        else
            owner.scrollToColumnInternal (newRangeStart);
    }

    z0 codeDocumentTextInserted (const Txt& newText, i32 pos) override
    {
        owner.codeDocumentChanged (pos, pos + newText.length());
    }

    z0 codeDocumentTextDeleted (i32 start, i32 end) override
    {
        owner.codeDocumentChanged (start, end);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
class CodeEditorComponent::GutterComponent final : public Component
{
public:
    GutterComponent() {}

    z0 paint (Graphics& g) override
    {
        jassert (dynamic_cast<CodeEditorComponent*> (getParentComponent()) != nullptr);
        auto& editor = *static_cast<CodeEditorComponent*> (getParentComponent());

        g.fillAll (editor.findColor (CodeEditorComponent::backgroundColorId)
                    .overlaidWith (editor.findColor (lineNumberBackgroundId)));

        auto clip = g.getClipBounds();
        i32k lineH = editor.lineHeight;
        const f32 lineHeightFloat = (f32) lineH;
        i32k firstLineToDraw = jmax (0, clip.getY() / lineH);
        i32k lastLineToDraw = jmin (editor.lines.size(), clip.getBottom() / lineH + 1,
                                         lastNumLines - editor.firstLineOnScreen);

        auto lineNumberFont = editor.getFont().withHeight (jmin (13.0f, lineHeightFloat * 0.8f));
        auto w = (f32) getWidth() - 2.0f;
        GlyphArrangement ga;

        for (i32 i = firstLineToDraw; i < lastLineToDraw; ++i)
            ga.addFittedText (lineNumberFont, Txt (editor.firstLineOnScreen + i + 1),
                              0, (f32) (lineH * i), w, lineHeightFloat,
                              Justification::centredRight, 1, 0.2f);

        g.setColor (editor.findColor (lineNumberTextId));
        ga.draw (g);
    }

    z0 documentChanged (CodeDocument& doc, i32 newFirstLine)
    {
        auto newNumLines = doc.getNumLines();

        if (newNumLines != lastNumLines || firstLine != newFirstLine)
        {
            firstLine = newFirstLine;
            lastNumLines = newNumLines;
            repaint();
        }
    }

private:
    i32 firstLine = 0, lastNumLines = 0;
};


//==============================================================================
CodeEditorComponent::CodeEditorComponent (CodeDocument& doc, CodeTokeniser* const tokeniser)
    : document (doc),
      caretPos (doc, 0, 0),
      selectionStart (doc, 0, 0),
      selectionEnd (doc, 0, 0),
      codeTokeniser (tokeniser)
{
    pimpl.reset (new Pimpl (*this));

    caretPos.setPositionMaintained (true);
    selectionStart.setPositionMaintained (true);
    selectionEnd.setPositionMaintained (true);

    setOpaque (true);
    setMouseCursor (MouseCursor::IBeamCursor);
    setWantsKeyboardFocus (true);

    addAndMakeVisible (verticalScrollBar);
    verticalScrollBar.setSingleStepSize (1.0);

    addAndMakeVisible (horizontalScrollBar);
    horizontalScrollBar.setSingleStepSize (1.0);

    Font f (withDefaultMetrics (FontOptions { 12.0f }));
    f.setTypefaceName (Font::getDefaultMonospacedFontName());
    setFont (f);

    if (codeTokeniser != nullptr)
        setColorScheme (codeTokeniser->getDefaultColorScheme());

    setLineNumbersShown (true);

    verticalScrollBar.addListener (pimpl.get());
    horizontalScrollBar.addListener (pimpl.get());
    document.addListener (pimpl.get());

    lookAndFeelChanged();
}

CodeEditorComponent::~CodeEditorComponent()
{
    if (auto* peer = getPeer())
        peer->refreshTextInputTarget();

    document.removeListener (pimpl.get());
}

i32 CodeEditorComponent::getGutterSize() const noexcept
{
    return showLineNumbers ? 35 : 5;
}

z0 CodeEditorComponent::loadContent (const Txt& newContent)
{
    clearCachedIterators (0);
    document.replaceAllContent (newContent);
    document.clearUndoHistory();
    document.setSavePoint();
    caretPos.setPosition (0);
    selectionStart.setPosition (0);
    selectionEnd.setPosition (0);
    scrollToLine (0);
}

b8 CodeEditorComponent::isTextInputActive() const
{
    return true;
}

z0 CodeEditorComponent::setTemporaryUnderlining (const Array<Range<i32>>&)
{
    // TODO IME composition ranges not yet supported for this component
}

z0 CodeEditorComponent::setLineNumbersShown (const b8 shouldBeShown)
{
    if (showLineNumbers != shouldBeShown)
    {
        showLineNumbers = shouldBeShown;
        gutter.reset();

        if (shouldBeShown)
        {
            gutter.reset (new GutterComponent());
            addAndMakeVisible (gutter.get());
        }

        resized();
    }
}

z0 CodeEditorComponent::setReadOnly (b8 b) noexcept
{
    if (readOnly != b)
    {
        readOnly = b;

        if (b)
            removeChildComponent (caret.get());
        else
            addAndMakeVisible (caret.get());

        invalidateAccessibilityHandler();
    }
}

//==============================================================================
z0 CodeEditorComponent::resized()
{
    auto visibleWidth = getWidth() - scrollbarThickness - getGutterSize();
    linesOnScreen   = jmax (1, (getHeight() - scrollbarThickness) / lineHeight);
    columnsOnScreen = jmax (1, (i32) ((f32) visibleWidth / charWidth));
    lines.clear();
    rebuildLineTokens();
    updateCaretPosition();

    if (gutter != nullptr)
        gutter->setBounds (0, 0, getGutterSize() - 2, getHeight());

    verticalScrollBar.setBounds (getWidth() - scrollbarThickness, 0,
                                 scrollbarThickness, getHeight() - scrollbarThickness);

    horizontalScrollBar.setBounds (getGutterSize(), getHeight() - scrollbarThickness,
                                   visibleWidth, scrollbarThickness);
    updateScrollBars();
}

z0 CodeEditorComponent::paint (Graphics& g)
{
    g.fillAll (findColor (CodeEditorComponent::backgroundColorId));

    const auto gutterSize = getGutterSize();
    const auto bottom = horizontalScrollBar.isVisible() ? horizontalScrollBar.getY() : getHeight();
    const auto right  = verticalScrollBar.isVisible()   ? verticalScrollBar.getX()   : getWidth();

    g.reduceClipRegion (gutterSize, 0, right - gutterSize, bottom);

    g.setFont (font);

    const auto clip = g.getClipBounds();
    const auto firstLineToDraw = jmax (0, clip.getY() / lineHeight);
    const auto lastLineToDraw  = jmin (lines.size(), clip.getBottom() / lineHeight + 1);
    const auto x = (f32) (gutterSize - xOffset * charWidth);
    const auto rightClip = (f32) clip.getRight();

    {
        RectangleList<f32> highlightArea;

        for (i32 i = firstLineToDraw; i < lastLineToDraw; ++i)
            if (const auto area = lines.getUnchecked (i)->getHighlightArea (x, lineHeight * i, lineHeight, charWidth))
                highlightArea.add (*area);

        g.setColor (findColor (CodeEditorComponent::highlightColorId));
        g.fillRectList (highlightArea);
    }

    for (i32 i = firstLineToDraw; i < lastLineToDraw; ++i)
        lines.getUnchecked (i)->draw (*this, g, font, rightClip, x, lineHeight * i, lineHeight, charWidth);
}

z0 CodeEditorComponent::setScrollbarThickness (i32k thickness)
{
    if (scrollbarThickness != thickness)
    {
        scrollbarThickness = thickness;
        resized();
    }
}

z0 CodeEditorComponent::rebuildLineTokensAsync()
{
    pimpl->triggerAsyncUpdate();
}

z0 CodeEditorComponent::rebuildLineTokens()
{
    pimpl->cancelPendingUpdate();

    auto numNeeded = linesOnScreen + 1;
    auto minLineToRepaint = numNeeded;
    i32 maxLineToRepaint = 0;

    if (numNeeded != lines.size())
    {
        lines.clear();

        for (i32 i = numNeeded; --i >= 0;)
            lines.add (new CodeEditorLine());

        minLineToRepaint = 0;
        maxLineToRepaint = numNeeded;
    }

    jassert (numNeeded == lines.size());

    CodeDocument::Iterator source (document);
    getIteratorForPosition (CodeDocument::Position (document, firstLineOnScreen, 0).getPosition(), source);

    for (i32 i = 0; i < numNeeded; ++i)
    {
        if (lines.getUnchecked (i)->update (document, firstLineOnScreen + i, source, codeTokeniser,
                                           spacesPerTab, selectionStart, selectionEnd))
        {
            minLineToRepaint = jmin (minLineToRepaint, i);
            maxLineToRepaint = jmax (maxLineToRepaint, i);
        }
    }

    if (minLineToRepaint <= maxLineToRepaint)
        repaint (0, lineHeight * minLineToRepaint - 1,
                 verticalScrollBar.getX(), lineHeight * (1 + maxLineToRepaint - minLineToRepaint) + 2);

    if (gutter != nullptr)
        gutter->documentChanged (document, firstLineOnScreen);
}

z0 CodeEditorComponent::codeDocumentChanged (i32k startIndex, i32k endIndex)
{
    const CodeDocument::Position affectedTextStart (document, startIndex);
    const CodeDocument::Position affectedTextEnd (document, endIndex);

    retokenise (startIndex, endIndex);

    updateCaretPosition();
    columnToTryToMaintain = -1;

    if (affectedTextEnd.getPosition() >= selectionStart.getPosition()
         && affectedTextStart.getPosition() <= selectionEnd.getPosition())
        deselectAll();

    if (shouldFollowDocumentChanges)
        if (caretPos.getPosition() > affectedTextEnd.getPosition()
            || caretPos.getPosition() < affectedTextStart.getPosition())
            moveCaretTo (affectedTextStart, false);

    updateScrollBars();
}

z0 CodeEditorComponent::retokenise (i32 startIndex, [[maybe_unused]] i32 endIndex)
{
    const CodeDocument::Position affectedTextStart (document, startIndex);

    clearCachedIterators (affectedTextStart.getLineNumber());

    rebuildLineTokensAsync();
}

//==============================================================================
z0 CodeEditorComponent::updateCaretPosition()
{
    if (caret != nullptr)
    {
        caret->setCaretPosition (getCharacterBounds (getCaretPos()));

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

z0 CodeEditorComponent::moveCaretTo (const CodeDocument::Position& newPos, const b8 highlighting)
{
    caretPos = newPos;
    columnToTryToMaintain = -1;
    b8 selectionWasActive = isHighlightActive();

    if (highlighting)
    {
        if (dragType == notDragging)
        {
            auto oldCaretPos = caretPos.getPosition();
            auto isStart = std::abs (oldCaretPos - selectionStart.getPosition())
                            < std::abs (oldCaretPos - selectionEnd.getPosition());

            dragType = isStart ? draggingSelectionStart : draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            if (selectionEnd.getPosition() < caretPos.getPosition())
            {
                setSelection (selectionEnd, caretPos);
                dragType = draggingSelectionEnd;
            }
            else
            {
                setSelection (caretPos, selectionEnd);
            }
        }
        else
        {
            if (caretPos.getPosition() < selectionStart.getPosition())
            {
                setSelection (caretPos, selectionStart);
                dragType = draggingSelectionStart;
            }
            else
            {
                setSelection (selectionStart, caretPos);
            }
        }

        rebuildLineTokensAsync();
    }
    else
    {
        deselectAll();
    }

    updateCaretPosition();
    scrollToKeepCaretOnScreen();
    updateScrollBars();
    caretPositionMoved();

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);

    if (appCommandManager != nullptr && selectionWasActive != isHighlightActive())
        appCommandManager->commandStatusChanged();
}

z0 CodeEditorComponent::deselectAll()
{
    if (isHighlightActive())
        rebuildLineTokensAsync();

    setSelection (caretPos, caretPos);
    dragType = notDragging;
}

z0 CodeEditorComponent::updateScrollBars()
{
    verticalScrollBar.setRangeLimits (0, jmax (document.getNumLines(), firstLineOnScreen + linesOnScreen));
    verticalScrollBar.setCurrentRange (firstLineOnScreen, linesOnScreen);

    horizontalScrollBar.setRangeLimits (0, jmax ((f64) document.getMaximumLineLength(), xOffset + columnsOnScreen));
    horizontalScrollBar.setCurrentRange (xOffset, columnsOnScreen);
}

z0 CodeEditorComponent::scrollToLineInternal (i32 newFirstLineOnScreen)
{
    newFirstLineOnScreen = jlimit (0, jmax (0, document.getNumLines() - 1),
                                   newFirstLineOnScreen);

    if (newFirstLineOnScreen != firstLineOnScreen)
    {
        firstLineOnScreen = newFirstLineOnScreen;
        updateCaretPosition();

        updateCachedIterators (firstLineOnScreen);
        rebuildLineTokensAsync();
        pimpl->handleUpdateNowIfNeeded();

        editorViewportPositionChanged();
    }
}

z0 CodeEditorComponent::scrollToColumnInternal (f64 column)
{
    const f64 newOffset = jlimit (0.0, document.getMaximumLineLength() + 3.0, column);

    if (! approximatelyEqual (xOffset, newOffset))
    {
        xOffset = newOffset;
        updateCaretPosition();
        repaint();
    }
}

z0 CodeEditorComponent::scrollToLine (i32 newFirstLineOnScreen)
{
    scrollToLineInternal (newFirstLineOnScreen);
    updateScrollBars();
}

z0 CodeEditorComponent::scrollToColumn (i32 newFirstColumnOnScreen)
{
    scrollToColumnInternal (newFirstColumnOnScreen);
    updateScrollBars();
}

z0 CodeEditorComponent::scrollBy (i32 deltaLines)
{
    scrollToLine (firstLineOnScreen + deltaLines);
}

z0 CodeEditorComponent::scrollToKeepLinesOnScreen (Range<i32> rangeToShow)
{
    if (rangeToShow.getStart() < firstLineOnScreen)
        scrollBy (rangeToShow.getStart() - firstLineOnScreen);
    else if (rangeToShow.getEnd() >= firstLineOnScreen + linesOnScreen)
        scrollBy (rangeToShow.getEnd() - (firstLineOnScreen + linesOnScreen - 1));
}

z0 CodeEditorComponent::scrollToKeepCaretOnScreen()
{
    if (getWidth() > 0 && getHeight() > 0)
    {
        auto caretLine = caretPos.getLineNumber();
        scrollToKeepLinesOnScreen ({ caretLine, caretLine });

        auto column = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());

        if (column >= xOffset + columnsOnScreen - 1)
            scrollToColumn (column + 1 - columnsOnScreen);
        else if (column < xOffset)
            scrollToColumn (column);
    }
}

Rectangle<i32> CodeEditorComponent::getCharacterBounds (const CodeDocument::Position& pos) const
{
    return { roundToInt ((getGutterSize() - xOffset * charWidth) + (f32) indexToColumn (pos.getLineNumber(), pos.getIndexInLine()) * charWidth),
             (pos.getLineNumber() - firstLineOnScreen) * lineHeight,
             roundToInt (charWidth),
             lineHeight };
}

CodeDocument::Position CodeEditorComponent::getPositionAt (i32 x, i32 y) const
{
    i32k line = y / lineHeight + firstLineOnScreen;
    i32k column = roundToInt ((x - (getGutterSize() - xOffset * charWidth)) / charWidth);
    i32k index = columnToIndex (line, column);

    return CodeDocument::Position (document, line, index);
}

i32 CodeEditorComponent::getCharIndexForPoint (Point<i32> point) const
{
    return getPositionAt (point.x, point.y).getPosition();
}

RectangleList<i32> CodeEditorComponent::getTextBounds (Range<i32> textRange) const
{
    RectangleList<i32> localRects;

    const CodeDocument::Position startPosition (document, textRange.getStart());
    const CodeDocument::Position endPosition   (document, textRange.getEnd());

    for (i32 line = startPosition.getLineNumber(); line <= endPosition.getLineNumber(); ++line)
    {
        const CodeDocument::Position lineStartColumn0 { document, line, 0 };

        const auto lineStart = line == startPosition.getLineNumber() ? lineStartColumn0.movedBy (startPosition.getIndexInLine())
                                                                     : lineStartColumn0;

        const CodeDocument::Position lineEnd { document, line, line == endPosition.getLineNumber() ? endPosition.getIndexInLine()
                                                                                                   : document.getLine (line).length() };

        const auto startPos = getCharacterBounds (lineStart).getTopLeft();
        const auto endPos   = getCharacterBounds (lineEnd)  .getTopLeft();

        localRects.add (startPos.x, startPos.y, jmax (1, endPos.x - startPos.x), getLineHeight());
    }

    return localRects;
}

//==============================================================================
z0 CodeEditorComponent::insertTextAtCaret (const Txt& newText)
{
    insertText (newText);
}

z0 CodeEditorComponent::insertText (const Txt& newText)
{
    if (! readOnly)
    {
        document.deleteSection (selectionStart, selectionEnd);

        if (newText.isNotEmpty())
            document.insertText (caretPos, newText);

        scrollToKeepCaretOnScreen();
        caretPositionMoved();

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
    }
}

z0 CodeEditorComponent::insertTabAtCaret()
{
    if (! readOnly)
    {
        if (CharacterFunctions::isWhitespace (caretPos.getCharacter())
             && caretPos.getLineNumber() == caretPos.movedBy (1).getLineNumber())
        {
            moveCaretTo (document.findWordBreakAfter (caretPos), false);
        }

        if (useSpacesForTabs)
        {
            auto caretCol = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());
            auto spacesNeeded = spacesPerTab - (caretCol % spacesPerTab);
            insertTextAtCaret (Txt::repeatedString (" ", spacesNeeded));
        }
        else
        {
            insertTextAtCaret ("\t");
        }
    }
}

b8 CodeEditorComponent::deleteWhitespaceBackwardsToTabStop()
{
    if (getHighlightedRegion().isEmpty() && ! readOnly)
    {
        for (;;)
        {
            auto currentColumn = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());

            if (currentColumn <= 0 || (currentColumn % spacesPerTab) == 0)
                break;

            moveCaretLeft (false, true);
        }

        auto selected = getTextInRange (getHighlightedRegion());

        if (selected.isNotEmpty() && selected.trim().isEmpty())
        {
            cut();
            return true;
        }
    }

    return false;
}

z0 CodeEditorComponent::indentSelection()     { indentSelectedLines ( spacesPerTab); }
z0 CodeEditorComponent::unindentSelection()   { indentSelectedLines (-spacesPerTab); }

z0 CodeEditorComponent::indentSelectedLines (i32k spacesToAdd)
{
    if (! readOnly)
    {
        newTransaction();

        CodeDocument::Position oldSelectionStart (selectionStart), oldSelectionEnd (selectionEnd), oldCaret (caretPos);
        oldSelectionStart.setPositionMaintained (true);
        oldSelectionEnd.setPositionMaintained (true);
        oldCaret.setPositionMaintained (true);

        i32k lineStart = selectionStart.getLineNumber();
        i32 lineEnd = selectionEnd.getLineNumber();

        if (lineEnd > lineStart && selectionEnd.getIndexInLine() == 0)
            --lineEnd;

        for (i32 line = lineStart; line <= lineEnd; ++line)
        {
            auto lineText = document.getLine (line);
            auto nonWhitespaceStart = CodeEditorHelpers::findFirstNonWhitespaceChar (lineText);

            if (nonWhitespaceStart > 0 || lineText.trimStart().isNotEmpty())
            {
                const CodeDocument::Position wsStart (document, line, 0);
                const CodeDocument::Position wsEnd   (document, line, nonWhitespaceStart);

                i32k numLeadingSpaces = indexToColumn (line, wsEnd.getIndexInLine());
                i32k newNumLeadingSpaces = jmax (0, numLeadingSpaces + spacesToAdd);

                if (newNumLeadingSpaces != numLeadingSpaces)
                {
                    document.deleteSection (wsStart, wsEnd);
                    document.insertText (wsStart, getTabString (newNumLeadingSpaces));
                }
            }
        }

        setSelection (oldSelectionStart, oldSelectionEnd);

        if (caretPos != oldCaret)
        {
            caretPos = oldCaret;

            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
        }
    }
}

z0 CodeEditorComponent::cut()
{
    insertText ({});
}

b8 CodeEditorComponent::copyToClipboard()
{
    newTransaction();
    auto selection = document.getTextBetween (selectionStart, selectionEnd);

    if (selection.isNotEmpty())
        SystemClipboard::copyTextToClipboard (selection);

    return true;
}

b8 CodeEditorComponent::cutToClipboard()
{
    copyToClipboard();
    cut();
    newTransaction();
    return true;
}

b8 CodeEditorComponent::pasteFromClipboard()
{
    newTransaction();
    auto clip = SystemClipboard::getTextFromClipboard();

    if (clip.isNotEmpty())
        insertText (clip);

    newTransaction();
    return true;
}

b8 CodeEditorComponent::moveCaretLeft (const b8 moveInWholeWordSteps, const b8 selecting)
{
    newTransaction();

    if (selecting && dragType == notDragging)
    {
        selectRegion (CodeDocument::Position (selectionEnd), CodeDocument::Position (selectionStart));
        dragType = draggingSelectionStart;
    }

    if (isHighlightActive() && ! (selecting || moveInWholeWordSteps))
    {
        moveCaretTo (selectionStart, false);
        return true;
    }

    if (moveInWholeWordSteps)
        moveCaretTo (document.findWordBreakBefore (caretPos), selecting);
    else
        moveCaretTo (caretPos.movedBy (-1), selecting);

    return true;
}

b8 CodeEditorComponent::moveCaretRight (const b8 moveInWholeWordSteps, const b8 selecting)
{
    newTransaction();

    if (selecting && dragType == notDragging)
    {
        selectRegion (CodeDocument::Position (selectionStart), CodeDocument::Position (selectionEnd));
        dragType = draggingSelectionEnd;
    }

    if (isHighlightActive() && ! (selecting || moveInWholeWordSteps))
    {
        moveCaretTo (selectionEnd, false);
        return true;
    }

    if (moveInWholeWordSteps)
        moveCaretTo (document.findWordBreakAfter (caretPos), selecting);
    else
        moveCaretTo (caretPos.movedBy (1), selecting);

    return true;
}

z0 CodeEditorComponent::moveLineDelta (i32k delta, const b8 selecting)
{
    CodeDocument::Position pos (caretPos);
    auto newLineNum = pos.getLineNumber() + delta;

    if (columnToTryToMaintain < 0)
        columnToTryToMaintain = indexToColumn (pos.getLineNumber(), pos.getIndexInLine());

    pos.setLineAndIndex (newLineNum, columnToIndex (newLineNum, columnToTryToMaintain));

    auto colToMaintain = columnToTryToMaintain;
    moveCaretTo (pos, selecting);
    columnToTryToMaintain = colToMaintain;
}

b8 CodeEditorComponent::moveCaretDown (const b8 selecting)
{
    newTransaction();

    if (caretPos.getLineNumber() == document.getNumLines() - 1)
        moveCaretTo (CodeDocument::Position (document, std::numeric_limits<i32>::max(), std::numeric_limits<i32>::max()), selecting);
    else
        moveLineDelta (1, selecting);

    return true;
}

b8 CodeEditorComponent::moveCaretUp (const b8 selecting)
{
    newTransaction();

    if (caretPos.getLineNumber() == 0)
        moveCaretTo (CodeDocument::Position (document, 0, 0), selecting);
    else
        moveLineDelta (-1, selecting);

    return true;
}

b8 CodeEditorComponent::pageDown (const b8 selecting)
{
    newTransaction();
    scrollBy (jlimit (0, linesOnScreen, 1 + document.getNumLines() - firstLineOnScreen - linesOnScreen));
    moveLineDelta (linesOnScreen, selecting);
    return true;
}

b8 CodeEditorComponent::pageUp (const b8 selecting)
{
    newTransaction();
    scrollBy (-linesOnScreen);
    moveLineDelta (-linesOnScreen, selecting);
    return true;
}

b8 CodeEditorComponent::scrollUp()
{
    newTransaction();
    scrollBy (1);

    if (caretPos.getLineNumber() < firstLineOnScreen)
        moveLineDelta (1, false);

    return true;
}

b8 CodeEditorComponent::scrollDown()
{
    newTransaction();
    scrollBy (-1);

    if (caretPos.getLineNumber() >= firstLineOnScreen + linesOnScreen)
        moveLineDelta (-1, false);

    return true;
}

b8 CodeEditorComponent::moveCaretToTop (const b8 selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (document, 0, 0), selecting);
    return true;
}

b8 CodeEditorComponent::moveCaretToStartOfLine (const b8 selecting)
{
    newTransaction();

    i32 index = CodeEditorHelpers::findFirstNonWhitespaceChar (caretPos.getLineText());

    if (index >= caretPos.getIndexInLine() && caretPos.getIndexInLine() > 0)
        index = 0;

    moveCaretTo (CodeDocument::Position (document, caretPos.getLineNumber(), index), selecting);
    return true;
}

b8 CodeEditorComponent::moveCaretToEnd (const b8 selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (document, std::numeric_limits<i32>::max(),
                                         std::numeric_limits<i32>::max()), selecting);
    return true;
}

b8 CodeEditorComponent::moveCaretToEndOfLine (const b8 selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (document, caretPos.getLineNumber(),
                                         std::numeric_limits<i32>::max()), selecting);
    return true;
}

b8 CodeEditorComponent::deleteBackwards (const b8 moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
    {
        cut(); // in case something is already highlighted
        moveCaretTo (document.findWordBreakBefore (caretPos), true);
    }
    else if (selectionStart == selectionEnd && ! skipBackwardsToPreviousTab())
    {
        selectionStart.moveBy (-1);
    }

    cut();
    return true;
}

b8 CodeEditorComponent::skipBackwardsToPreviousTab()
{
    auto currentLineText = caretPos.getLineText().removeCharacters ("\r\n");
    auto currentIndex = caretPos.getIndexInLine();

    if (currentLineText.isNotEmpty() && currentLineText.length() == currentIndex)
    {
        i32k currentLine = caretPos.getLineNumber();
        i32k currentColumn = indexToColumn (currentLine, currentIndex);
        i32k previousTabColumn = (currentColumn - 1) - ((currentColumn - 1) % spacesPerTab);
        i32k previousTabIndex = columnToIndex (currentLine, previousTabColumn);

        if (currentLineText.substring (previousTabIndex, currentIndex).trim().isEmpty())
        {
            selectionStart.moveBy (previousTabIndex - currentIndex);
            return true;
        }
    }

    return false;
}

b8 CodeEditorComponent::deleteForwards (const b8 moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
    {
        cut(); // in case something is already highlighted
        moveCaretTo (document.findWordBreakAfter (caretPos), true);
    }
    else
    {
        if (selectionStart == selectionEnd)
            selectionEnd.moveBy (1);
        else
            newTransaction();
    }

    cut();
    return true;
}

b8 CodeEditorComponent::selectAll()
{
    newTransaction();
    selectRegion (CodeDocument::Position (document, std::numeric_limits<i32>::max(),
                                          std::numeric_limits<i32>::max()),
                  CodeDocument::Position (document, 0, 0));
    return true;
}

z0 CodeEditorComponent::selectRegion (const CodeDocument::Position& start,
                                        const CodeDocument::Position& end)
{
    moveCaretTo (start, false);
    moveCaretTo (end, true);
}

//==============================================================================
b8 CodeEditorComponent::undo()
{
    if (readOnly)
        return false;

    ScopedValueSetter<b8> svs (shouldFollowDocumentChanges, true, false);
    document.undo();
    scrollToKeepCaretOnScreen();
    return true;
}

b8 CodeEditorComponent::redo()
{
    if (readOnly)
        return false;

    ScopedValueSetter<b8> svs (shouldFollowDocumentChanges, true, false);
    document.redo();
    scrollToKeepCaretOnScreen();
    return true;
}

z0 CodeEditorComponent::newTransaction()
{
    document.newTransaction();
    pimpl->startTimer (600);
}

z0 CodeEditorComponent::setCommandManager (ApplicationCommandManager* newManager) noexcept
{
    appCommandManager = newManager;
}

//==============================================================================
Range<i32> CodeEditorComponent::getHighlightedRegion() const
{
    return { selectionStart.getPosition(),
             selectionEnd.getPosition() };
}

b8 CodeEditorComponent::isHighlightActive() const noexcept
{
    return selectionStart != selectionEnd;
}

z0 CodeEditorComponent::setHighlightedRegion (const Range<i32>& newRange)
{
    if (newRange == getHighlightedRegion())
        return;

    const auto cursorAtStart = newRange.getEnd() == getHighlightedRegion().getStart()
                            || newRange.getEnd() == getHighlightedRegion().getEnd();
    selectRegion (CodeDocument::Position (document, cursorAtStart ? newRange.getEnd() : newRange.getStart()),
                  CodeDocument::Position (document, cursorAtStart ? newRange.getStart() : newRange.getEnd()));
}

Txt CodeEditorComponent::getTextInRange (const Range<i32>& range) const
{
    return document.getTextBetween (CodeDocument::Position (document, range.getStart()),
                                    CodeDocument::Position (document, range.getEnd()));
}

//==============================================================================
b8 CodeEditorComponent::keyPressed (const KeyPress& key)
{
    if (! TextEditorKeyMapper<CodeEditorComponent>::invokeKeyFunction (*this, key))
    {
        if (readOnly)
            return false;

        if (key == KeyPress::tabKey || key.getTextCharacter() == '\t')      handleTabKey();
        else if (key == KeyPress::returnKey)                                handleReturnKey();
        else if (key == KeyPress::escapeKey)                                handleEscapeKey();
        else if (key == KeyPress ('[', ModifierKeys::commandModifier, 0))   unindentSelection();
        else if (key == KeyPress (']', ModifierKeys::commandModifier, 0))   indentSelection();
        else if (key.getTextCharacter() >= ' ')                             insertTextAtCaret (Txt::charToString (key.getTextCharacter()));
        else                                                                return false;
    }

    pimpl->handleUpdateNowIfNeeded();
    return true;
}

z0 CodeEditorComponent::handleReturnKey()
{
    insertTextAtCaret (document.getNewLineCharacters());
}

z0 CodeEditorComponent::handleTabKey()
{
    insertTabAtCaret();
}

z0 CodeEditorComponent::handleEscapeKey()
{
    newTransaction();
}

z0 CodeEditorComponent::editorViewportPositionChanged()
{
}

z0 CodeEditorComponent::caretPositionMoved()
{
}

//==============================================================================
ApplicationCommandTarget* CodeEditorComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

z0 CodeEditorComponent::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = { StandardApplicationCommandIDs::cut,
                              StandardApplicationCommandIDs::copy,
                              StandardApplicationCommandIDs::paste,
                              StandardApplicationCommandIDs::del,
                              StandardApplicationCommandIDs::selectAll,
                              StandardApplicationCommandIDs::undo,
                              StandardApplicationCommandIDs::redo };

    commands.addArray (ids, numElementsInArray (ids));
}

z0 CodeEditorComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    const b8 anythingSelected = isHighlightActive();

    switch (commandID)
    {
        case StandardApplicationCommandIDs::cut:
            result.setInfo (TRANS ("Cut"), TRANS ("Copies the currently selected text to the clipboard and deletes it."), "Editing", 0);
            result.setActive (anythingSelected && ! readOnly);
            result.defaultKeypresses.add (KeyPress ('x', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::copy:
            result.setInfo (TRANS ("Copy"), TRANS ("Copies the currently selected text to the clipboard."), "Editing", 0);
            result.setActive (anythingSelected);
            result.defaultKeypresses.add (KeyPress ('c', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::paste:
            result.setInfo (TRANS ("Paste"), TRANS ("Inserts text from the clipboard."), "Editing", 0);
            result.setActive (! readOnly);
            result.defaultKeypresses.add (KeyPress ('v', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::del:
            result.setInfo (TRANS ("Delete"), TRANS ("Deletes any selected text."), "Editing", 0);
            result.setActive (anythingSelected && ! readOnly);
            break;

        case StandardApplicationCommandIDs::selectAll:
            result.setInfo (TRANS ("Select All"), TRANS ("Selects all the text in the editor."), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('a', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::undo:
            result.setInfo (TRANS ("Undo"), TRANS ("Undo"), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier, 0));
            result.setActive (document.getUndoManager().canUndo() && ! readOnly);
            break;

        case StandardApplicationCommandIDs::redo:
            result.setInfo (TRANS ("Redo"), TRANS ("Redo"), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
            result.setActive (document.getUndoManager().canRedo() && ! readOnly);
            break;

        default:
            break;
    }
}

b8 CodeEditorComponent::perform (const InvocationInfo& info)
{
    return performCommand (info.commandID);
}

z0 CodeEditorComponent::lookAndFeelChanged()
{
    caret.reset (getLookAndFeel().createCaretComponent (this));
    addAndMakeVisible (caret.get());
}

b8 CodeEditorComponent::performCommand (const CommandID commandID)
{
    switch (commandID)
    {
        case StandardApplicationCommandIDs::cut:        cutToClipboard(); break;
        case StandardApplicationCommandIDs::copy:       copyToClipboard(); break;
        case StandardApplicationCommandIDs::paste:      pasteFromClipboard(); break;
        case StandardApplicationCommandIDs::del:        cut(); break;
        case StandardApplicationCommandIDs::selectAll:  selectAll(); break;
        case StandardApplicationCommandIDs::undo:       undo(); break;
        case StandardApplicationCommandIDs::redo:       redo(); break;
        default:                                        return false;
    }

    return true;
}

z0 CodeEditorComponent::setSelection (CodeDocument::Position newSelectionStart,
                                        CodeDocument::Position newSelectionEnd)
{
    if (selectionStart != newSelectionStart
        || selectionEnd != newSelectionEnd)
    {
        selectionStart = newSelectionStart;
        selectionEnd = newSelectionEnd;

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

//==============================================================================
z0 CodeEditorComponent::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    m.addItem (StandardApplicationCommandIDs::cut,   TRANS ("Cut"), isHighlightActive() && ! readOnly);
    m.addItem (StandardApplicationCommandIDs::copy,  TRANS ("Copy"), ! getHighlightedRegion().isEmpty());
    m.addItem (StandardApplicationCommandIDs::paste, TRANS ("Paste"), ! readOnly);
    m.addItem (StandardApplicationCommandIDs::del,   TRANS ("Delete"), ! readOnly);
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::selectAll, TRANS ("Select All"));
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::undo,  TRANS ("Undo"), document.getUndoManager().canUndo());
    m.addItem (StandardApplicationCommandIDs::redo,  TRANS ("Redo"), document.getUndoManager().canRedo());
}

z0 CodeEditorComponent::performPopupMenuAction (i32k menuItemID)
{
    performCommand (menuItemID);
}

static z0 codeEditorMenuCallback (i32 menuResult, CodeEditorComponent* editor)
{
    if (editor != nullptr && menuResult != 0)
        editor->performPopupMenuAction (menuResult);
}

//==============================================================================
z0 CodeEditorComponent::mouseDown (const MouseEvent& e)
{
    newTransaction();
    dragType = notDragging;

    if (e.mods.isPopupMenu())
    {
        setMouseCursor (MouseCursor::NormalCursor);

        if (getHighlightedRegion().isEmpty())
        {
            CodeDocument::Position start, end;
            document.findTokenContaining (getPositionAt (e.x, e.y), start, end);

            if (start.getPosition() < end.getPosition())
                selectRegion (start, end);
        }

        PopupMenu m;
        m.setLookAndFeel (&getLookAndFeel());
        addPopupMenuItems (m, &e);

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (codeEditorMenuCallback, this));
    }
    else
    {
        beginDragAutoRepeat (100);
        moveCaretTo (getPositionAt (e.x, e.y), e.mods.isShiftDown());
    }
}

z0 CodeEditorComponent::mouseDrag (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
        moveCaretTo (getPositionAt (e.x, e.y), true);
}

z0 CodeEditorComponent::mouseUp (const MouseEvent&)
{
    newTransaction();
    beginDragAutoRepeat (0);
    dragType = notDragging;
    setMouseCursor (MouseCursor::IBeamCursor);
}

z0 CodeEditorComponent::mouseDoubleClick (const MouseEvent& e)
{
    CodeDocument::Position tokenStart (getPositionAt (e.x, e.y));
    CodeDocument::Position tokenEnd (tokenStart);

    if (e.getNumberOfClicks() > 2)
        document.findLineContaining (tokenStart, tokenStart, tokenEnd);
    else
        document.findTokenContaining (tokenStart, tokenStart, tokenEnd);

    selectRegion (tokenStart, tokenEnd);
    dragType = notDragging;
}

z0 CodeEditorComponent::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if ((verticalScrollBar.isVisible() && ! approximatelyEqual (wheel.deltaY, 0.0f))
         || (horizontalScrollBar.isVisible() && ! approximatelyEqual (wheel.deltaX, 0.0f)))
    {
        {
            MouseWheelDetails w (wheel);
            w.deltaX = 0;
            verticalScrollBar.mouseWheelMove (e, w);
        }

        {
            MouseWheelDetails w (wheel);
            w.deltaY = 0;
            horizontalScrollBar.mouseWheelMove (e, w);
        }
    }
    else
    {
        Component::mouseWheelMove (e, wheel);
    }
}

//==============================================================================
z0 CodeEditorComponent::focusGained (FocusChangeType)     { updateCaretPosition(); }
z0 CodeEditorComponent::focusLost (FocusChangeType)       { updateCaretPosition(); }

//==============================================================================
z0 CodeEditorComponent::setTabSize (i32k numSpaces, const b8 insertSpaces)
{
    useSpacesForTabs = insertSpaces;

    if (spacesPerTab != numSpaces)
    {
        spacesPerTab = numSpaces;
        rebuildLineTokensAsync();
    }
}

Txt CodeEditorComponent::getTabString (i32k numSpaces) const
{
    return Txt::repeatedString (useSpacesForTabs ? " " : "\t",
                                   useSpacesForTabs ? numSpaces
                                                    : (numSpaces / spacesPerTab));
}

i32 CodeEditorComponent::indexToColumn (i32 lineNum, i32 index) const noexcept
{
    auto line = document.getLine (lineNum);
    auto t = line.getCharPointer();
    i32 col = 0;

    for (i32 i = 0; i < index; ++i)
    {
        if (t.isEmpty())
        {
            jassertfalse;
            break;
        }

        if (t.getAndAdvance() != '\t')
            ++col;
        else
            col += getTabSize() - (col % getTabSize());
    }

    return col;
}

i32 CodeEditorComponent::columnToIndex (i32 lineNum, i32 column) const noexcept
{
    auto line = document.getLine (lineNum);
    auto t = line.getCharPointer();
    i32 i = 0, col = 0;

    while (! t.isEmpty())
    {
        if (t.getAndAdvance() != '\t')
            ++col;
        else
            col += getTabSize() - (col % getTabSize());

        if (col > column)
            break;

        ++i;
    }

    return i;
}

//==============================================================================
z0 CodeEditorComponent::setFont (const Font& newFont)
{
    font = newFont;

    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    charWidth = font.getStringWidthFloat ("0");
    DRX_END_IGNORE_DEPRECATION_WARNINGS

    lineHeight = roundToInt (font.getHeight());
    resized();
}

z0 CodeEditorComponent::ColorScheme::set (const Txt& name, Color colour)
{
    for (auto& tt : types)
    {
        if (tt.name == name)
        {
            tt.colour = colour;
            return;
        }
    }

    TokenType tt;
    tt.name = name;
    tt.colour = colour;
    types.add (tt);
}

z0 CodeEditorComponent::setColorScheme (const ColorScheme& scheme)
{
    colourScheme = scheme;
    repaint();
}

Color CodeEditorComponent::getColorForTokenType (i32k tokenType) const
{
    return isPositiveAndBelow (tokenType, colourScheme.types.size())
                ? colourScheme.types.getReference (tokenType).colour
                : findColor (CodeEditorComponent::defaultTextColorId);
}

z0 CodeEditorComponent::clearCachedIterators (i32k firstLineToBeInvalid)
{
    i32 i;
    for (i = cachedIterators.size(); --i >= 0;)
        if (cachedIterators.getUnchecked (i).getLine() < firstLineToBeInvalid)
            break;

    cachedIterators.removeRange (jmax (0, i - 1), cachedIterators.size());
}

z0 CodeEditorComponent::updateCachedIterators (i32 maxLineNum)
{
    i32k maxNumCachedPositions = 5000;
    i32k linesBetweenCachedSources = jmax (10, document.getNumLines() / maxNumCachedPositions);

    if (cachedIterators.size() == 0)
        cachedIterators.add (CodeDocument::Iterator (document));

    if (codeTokeniser != nullptr)
    {
        for (;;)
        {
            const auto last = cachedIterators.getLast();

            if (last.getLine() >= maxLineNum)
                break;

            cachedIterators.add (CodeDocument::Iterator (last));
            auto& t = cachedIterators.getReference (cachedIterators.size() - 1);
            i32k targetLine = jmin (maxLineNum, last.getLine() + linesBetweenCachedSources);

            for (;;)
            {
                codeTokeniser->readNextToken (t);

                if (t.getLine() >= targetLine)
                    break;

                if (t.isEOF())
                    return;
            }
        }
    }
}

z0 CodeEditorComponent::getIteratorForPosition (i32 position, CodeDocument::Iterator& source)
{
    if (codeTokeniser != nullptr)
    {
        for (i32 i = cachedIterators.size(); --i >= 0;)
        {
            auto& t = cachedIterators.getReference (i);

            if (t.getPosition() <= position)
            {
                source = t;
                break;
            }
        }

        while (source.getPosition() < position)
        {
            const CodeDocument::Iterator original (source);
            codeTokeniser->readNextToken (source);

            if (source.getPosition() > position || source.isEOF())
            {
                source = original;
                break;
            }
        }
    }
}

CodeEditorComponent::State::State (const CodeEditorComponent& editor)
    : lastTopLine (editor.getFirstLineOnScreen()),
      lastCaretPos (editor.getCaretPos().getPosition()),
      lastSelectionEnd (lastCaretPos)
{
    auto selection = editor.getHighlightedRegion();

    if (lastCaretPos == selection.getStart())
        lastSelectionEnd = selection.getEnd();
    else
        lastSelectionEnd = selection.getStart();
}

CodeEditorComponent::State::State (const State& other) noexcept
    : lastTopLine (other.lastTopLine),
      lastCaretPos (other.lastCaretPos),
      lastSelectionEnd (other.lastSelectionEnd)
{
}

z0 CodeEditorComponent::State::restoreState (CodeEditorComponent& editor) const
{
    editor.selectRegion (CodeDocument::Position (editor.getDocument(), lastSelectionEnd),
                         CodeDocument::Position (editor.getDocument(), lastCaretPos));

    if (lastTopLine > 0 && lastTopLine < editor.getDocument().getNumLines())
        editor.scrollToLine (lastTopLine);
}

CodeEditorComponent::State::State (const Txt& s)
{
    auto tokens = StringArray::fromTokens (s, ":", {});

    lastTopLine      = tokens[0].getIntValue();
    lastCaretPos     = tokens[1].getIntValue();
    lastSelectionEnd = tokens[2].getIntValue();
}

Txt CodeEditorComponent::State::toString() const
{
    return Txt (lastTopLine) + ":" + Txt (lastCaretPos) + ":" + Txt (lastSelectionEnd);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> CodeEditorComponent::createAccessibilityHandler()
{
    return std::make_unique<CodeEditorAccessibilityHandler> (*this);
}

} // namespace drx
