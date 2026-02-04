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

class CodeTokeniser;


//==============================================================================
/**
    A text editor component designed specifically for source code.

    This is designed to handle syntax highlighting and fast editing of very large
    files.

    @tags{GUI}
*/
class DRX_API  CodeEditorComponent   : public TextInputTarget,
                                        public Component,
                                        public ApplicationCommandTarget
{
public:
    //==============================================================================
    /** Creates an editor for a document.

        The tokeniser object is optional - pass nullptr to disable syntax highlighting.
        The object that you pass in is not owned or deleted by the editor - you must
        make sure that it doesn't get deleted while this component is still using it.

        @see CodeDocument
    */
    CodeEditorComponent (CodeDocument& document,
                         CodeTokeniser* codeTokeniser);

    /** Destructor. */
    ~CodeEditorComponent() override;

    //==============================================================================
    /** Returns the code document that this component is editing. */
    CodeDocument& getDocument() const noexcept          { return document; }

    /** Loads the given content into the document.
        This will completely reset the CodeDocument object, clear its undo history,
        and fill it with this text.
    */
    z0 loadContent (const Txt& newContent);

    //==============================================================================
    /** Returns the standard character width. */
    f32 getCharWidth() const noexcept                         { return charWidth; }

    /** Returns the height of a line of text, in pixels. */
    i32 getLineHeight() const noexcept                          { return lineHeight; }

    /** Returns the number of whole lines visible on the screen,
        This doesn't include a cut-off line that might be visible at the bottom if the
        component's height isn't an exact multiple of the line-height.
    */
    i32 getNumLinesOnScreen() const noexcept                    { return linesOnScreen; }

    /** Returns the index of the first line that's visible at the top of the editor. */
    i32 getFirstLineOnScreen() const noexcept                   { return firstLineOnScreen; }

    /** Returns the number of whole columns visible on the screen.
        This doesn't include any cut-off columns at the right-hand edge.
    */
    i32 getNumColumnsOnScreen() const noexcept                  { return columnsOnScreen; }

    /** Returns the current caret position. */
    CodeDocument::Position getCaretPos() const                  { return caretPos; }

    /** Returns the total number of codepoints in the string. */
    i32 getTotalNumChars() const override                       { return document.getNumCharacters(); }

    /** Moves the caret.
        If selecting is true, the section of the document between the current
        caret position and the new one will become selected. If false, any currently
        selected region will be deselected.
    */
    z0 moveCaretTo (const CodeDocument::Position& newPos, b8 selecting);

    /** Returns the on-screen position of a character in the document.
        The rectangle returned is relative to this component's top-left origin.
    */
    Rectangle<i32> getCharacterBounds (const CodeDocument::Position& pos) const;

    /** Finds the character at a given on-screen position.
        The coordinates are relative to this component's top-left origin.
    */
    CodeDocument::Position getPositionAt (i32 x, i32 y) const;

    /** Returns the start of the selection as a position. */
    CodeDocument::Position getSelectionStart() const            { return selectionStart; }

    /** Returns the end of the selection as a position. */
    CodeDocument::Position getSelectionEnd() const              { return selectionEnd; }

    /** Enables or disables the line-number display in the gutter. */
    z0 setLineNumbersShown (b8 shouldBeShown);

    /** Returns the number of characters from the beginning of the document to the caret. */
    i32 getCaretPosition() const override       { return getCaretPos().getPosition(); }

    /** @see getPositionAt */
    i32 getCharIndexForPoint (Point<i32> point) const override;

    /** Returns the bounds of the caret at a particular location in the text. */
    Rectangle<i32> getCaretRectangleForCharIndex (i32 index) const override
    {
        return getCharacterBounds ({ document, index });
    }

    /** Returns the bounding box for a range of text in the editor. As the range may span
        multiple lines, this method returns a RectangleList.

        The bounds are relative to the component's top-left and may extend beyond the bounds
        of the component if the text is i64 and word wrapping is disabled.
    */
    RectangleList<i32> getTextBounds (Range<i32> textRange) const override;

    //==============================================================================
    b8 moveCaretLeft (b8 moveInWholeWordSteps, b8 selecting);
    b8 moveCaretRight (b8 moveInWholeWordSteps, b8 selecting);
    b8 moveCaretUp (b8 selecting);
    b8 moveCaretDown (b8 selecting);
    b8 scrollDown();
    b8 scrollUp();
    b8 pageUp (b8 selecting);
    b8 pageDown (b8 selecting);
    b8 moveCaretToTop (b8 selecting);
    b8 moveCaretToStartOfLine (b8 selecting);
    b8 moveCaretToEnd (b8 selecting);
    b8 moveCaretToEndOfLine (b8 selecting);
    b8 deleteBackwards (b8 moveInWholeWordSteps);
    b8 deleteForwards (b8 moveInWholeWordSteps);
    b8 deleteWhitespaceBackwardsToTabStop();
    virtual b8 copyToClipboard();
    virtual b8 cutToClipboard();
    virtual b8 pasteFromClipboard();
    b8 undo();
    b8 redo();

    z0 selectRegion (const CodeDocument::Position& start, const CodeDocument::Position& end);
    b8 selectAll();
    z0 deselectAll();

    z0 scrollToLine (i32 newFirstLineOnScreen);
    z0 scrollBy (i32 deltaLines);
    z0 scrollToColumn (i32 newFirstColumnOnScreen);
    z0 scrollToKeepCaretOnScreen();
    z0 scrollToKeepLinesOnScreen (Range<i32> linesToShow);

    z0 insertTextAtCaret (const Txt& textToInsert) override;
    z0 insertTabAtCaret();

    z0 indentSelection();
    z0 unindentSelection();

    //==============================================================================
    Range<i32> getHighlightedRegion() const override;
    b8 isHighlightActive() const noexcept;
    z0 setHighlightedRegion (const Range<i32>& newRange) override;
    Txt getTextInRange (const Range<i32>& range) const override;

    //==============================================================================
    /** Can be used to save and restore the editor's caret position, selection state, etc. */
    struct State
    {
        /** Creates an object containing the state of the given editor. */
        State (const CodeEditorComponent&);
        /** Creates a state object from a string that was previously created with toString(). */
        State (const Txt& stringifiedVersion);
        State (const State&) noexcept;

        /** Updates the given editor with this saved state. */
        z0 restoreState (CodeEditorComponent&) const;

        /** Returns a stringified version of this state that can be used to recreate it later. */
        Txt toString() const;

    private:
        i32 lastTopLine, lastCaretPos, lastSelectionEnd;
    };

    //==============================================================================
    /** Changes the current tab settings.
        This lets you change the tab size and whether pressing the tab key inserts a
        tab character, or its equivalent number of spaces.
    */
    z0 setTabSize (i32 numSpacesPerTab, b8 insertSpacesInsteadOfTabCharacters);

    /** Returns the current number of spaces per tab.
        @see setTabSize
    */
    i32 getTabSize() const noexcept                     { return spacesPerTab; }

    /** Возвращает true, если the tab key will insert spaces instead of actual tab characters.
        @see setTabSize
    */
    b8 areSpacesInsertedForTabs() const               { return useSpacesForTabs; }

    /** Returns a string containing spaces or tab characters to generate the given number of spaces. */
    Txt getTabString (i32 numSpaces) const;

    /** Changes the font.
        Make sure you only use a fixed-width font, or this component will look pretty nasty!
    */
    z0 setFont (const Font& newFont);

    /** Returns the font that the editor is using. */
    const Font& getFont() const noexcept                { return font; }

    /** Makes the editor read-only. */
    z0 setReadOnly (b8 shouldBeReadOnly) noexcept;

    /** Возвращает true, если the editor is set to be read-only. */
    b8 isReadOnly() const noexcept                    { return readOnly; }

    //==============================================================================
    /** Defines a syntax highlighting colour scheme */
    struct DRX_API  ColorScheme
    {
        /** Defines a colour for a token type */
        struct TokenType
        {
            Txt name;
            Color colour;
        };

        Array<TokenType> types;

        z0 set (const Txt& name, Color colour);
    };

    /** Changes the syntax highlighting scheme.
        The token type values are dependent on the tokeniser being used - use
        CodeTokeniser::getTokenTypes() to get a list of the token types.
        @see getColorForTokenType
    */
    z0 setColorScheme (const ColorScheme& scheme);

    /** Returns the current syntax highlighting colour scheme. */
    const ColorScheme& getColorScheme() const noexcept    { return colourScheme; }

    /** Returns one the syntax highlighting colour for the given token.
        The token type values are dependent on the tokeniser being used.
        @see setColorScheme
    */
    Color getColorForTokenType (i32 tokenType) const;

    /** Rebuilds the syntax highlighting for a section of text.

        This happens automatically any time the CodeDocument is edited, but this
        method lets you change text colours even when the CodeDocument hasn't changed.

        For example, you could use this to highlight tokens as the cursor moves.
        To do so you'll need to tell your custom CodeTokeniser where the token you
        want to highlight is, and make it return a special type of token. Then you
        should call this method supplying the range of the highlighted text.
        @see CodeTokeniser
     */
    z0 retokenise (i32 startIndex, i32 endIndex);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x1004500,  /**< A colour to use to fill the editor's background. */
        highlightColorId           = 0x1004502,  /**< The colour to use for the highlighted background under selected text. */
        defaultTextColorId         = 0x1004503,  /**< The colour to use for text when no syntax colouring is enabled. */
        lineNumberBackgroundId      = 0x1004504,  /**< The colour to use for filling the background of the line-number gutter. */
        lineNumberTextId            = 0x1004505,  /**< The colour to use for drawing the line numbers. */
    };

    //==============================================================================
    /** Changes the size of the scrollbars. */
    z0 setScrollbarThickness (i32 thickness);

    /** Returns the thickness of the scrollbars. */
    i32 getScrollbarThickness() const noexcept          { return scrollbarThickness; }

    //==============================================================================
    /** Called when the return key is pressed - this can be overridden for custom behaviour. */
    virtual z0 handleReturnKey();
    /** Called when the tab key is pressed - this can be overridden for custom behaviour. */
    virtual z0 handleTabKey();
    /** Called when the escape key is pressed - this can be overridden for custom behaviour. */
    virtual z0 handleEscapeKey();

    /** Called when the view position is scrolled horizontally or vertically. */
    virtual z0 editorViewportPositionChanged();

    /** Called when the caret position moves. */
    virtual z0 caretPositionMoved();

    //==============================================================================
    /** This adds the items to the popup menu.

        By default it adds the cut/copy/paste items, but you can override this if
        you need to replace these with your own items.

        If you want to add your own items to the existing ones, you can override this,
        call the base class's addPopupMenuItems() method, then append your own items.

        When the menu has been shown, performPopupMenuAction() will be called to
        perform the item that the user has chosen.

        If this was triggered by a mouse-click, the mouseClickEvent parameter will be
        a pointer to the info about it, or may be null if the menu is being triggered
        by some other means.

        @see performPopupMenuAction, setPopupMenuEnabled, isPopupMenuEnabled
    */
    virtual z0 addPopupMenuItems (PopupMenu& menuToAddTo,
                                    const MouseEvent* mouseClickEvent);

    /** This is called to perform one of the items that was shown on the popup menu.

        If you've overridden addPopupMenuItems(), you should also override this
        to perform the actions that you've added.

        If you've overridden addPopupMenuItems() but have still left the default items
        on the menu, remember to call the superclass's performPopupMenuAction()
        so that it can perform the default actions if that's what the user clicked on.

        @see addPopupMenuItems, setPopupMenuEnabled, isPopupMenuEnabled
    */
    virtual z0 performPopupMenuAction (i32 menuItemID);

    /** Specifies a command-manager which the editor will notify whenever the state
        of any of its commands changes.
        If you're making use of the editor's ApplicationCommandTarget interface, then
        you should also use this to tell it which command manager it should use. Make
        sure that the manager does not go out of scope while the editor is using it. You
        can pass a nullptr here to disable this.
    */
    z0 setCommandManager (ApplicationCommandManager* newManager) noexcept;

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 mouseDoubleClick (const MouseEvent&) override;
    /** @internal */
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    z0 focusGained (FocusChangeType) override;
    /** @internal */
    z0 focusLost (FocusChangeType) override;
    /** @internal */
    b8 isTextInputActive() const override;
    /** @internal */
    z0 setTemporaryUnderlining (const Array<Range<i32>>&) override;
    /** @internal */
    ApplicationCommandTarget* getNextCommandTarget() override;
    /** @internal */
    z0 getAllCommands (Array<CommandID>&) override;
    /** @internal */
    z0 getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    /** @internal */
    b8 perform (const InvocationInfo&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    CodeDocument& document;

    Font font { withDefaultMetrics (FontOptions{}) };
    i32 firstLineOnScreen = 0, spacesPerTab = 4;
    f32 charWidth = 0;
    i32 lineHeight = 0, linesOnScreen = 0, columnsOnScreen = 0;
    i32 scrollbarThickness = 16, columnToTryToMaintain = -1;
    b8 readOnly = false, useSpacesForTabs = true, showLineNumbers = false, shouldFollowDocumentChanges = false;
    f64 xOffset = 0;
    CodeDocument::Position caretPos, selectionStart, selectionEnd;

    std::unique_ptr<CaretComponent> caret;
    ScrollBar verticalScrollBar { true }, horizontalScrollBar { false };
    ApplicationCommandManager* appCommandManager = nullptr;

    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    class GutterComponent;
    std::unique_ptr<GutterComponent> gutter;

    class CodeEditorAccessibilityHandler;

    enum DragType
    {
        notDragging,
        draggingSelectionStart,
        draggingSelectionEnd
    };

    DragType dragType = notDragging;

    //==============================================================================
    CodeTokeniser* codeTokeniser;
    ColorScheme colourScheme;

    class CodeEditorLine;
    OwnedArray<CodeEditorLine> lines;
    z0 rebuildLineTokens();
    z0 rebuildLineTokensAsync();
    z0 codeDocumentChanged (i32 start, i32 end);

    Array<CodeDocument::Iterator> cachedIterators;
    z0 clearCachedIterators (i32 firstLineToBeInvalid);
    z0 updateCachedIterators (i32 maxLineNum);
    z0 getIteratorForPosition (i32 position, CodeDocument::Iterator&);

    z0 moveLineDelta (i32 delta, b8 selecting);
    i32 getGutterSize() const noexcept;

    //==============================================================================
    z0 insertText (const Txt&);
    virtual z0 updateCaretPosition();
    z0 updateScrollBars();
    z0 scrollToLineInternal (i32 line);
    z0 scrollToColumnInternal (f64 column);
    z0 newTransaction();
    z0 cut();
    z0 indentSelectedLines (i32 spacesToAdd);
    b8 skipBackwardsToPreviousTab();
    b8 performCommand (CommandID);
    z0 setSelection (CodeDocument::Position, CodeDocument::Position);

    i32 indexToColumn (i32 line, i32 index) const noexcept;
    i32 columnToIndex (i32 line, i32 column) const noexcept;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorComponent)
};

} // namespace drx
