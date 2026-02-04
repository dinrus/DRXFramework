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
    An editable text box.

    A TextEditor can either be in single- or multi-line mode, and supports mixed
    fonts and colours.

    @see TextEditor::Listener, Label

    @tags{GUI}
*/
class DRX_API  TextEditor  : public TextInputTarget,
                              public Component,
                              public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates a new, empty text editor.

        @param componentName        the name to pass to the component for it to use as its name
        @param passwordCharacter    if this is not zero, this character will be used as a replacement
                                    for all characters that are drawn on screen - e.g. to create
                                    a password-style textbox containing circular blobs instead of text,
                                    you could set this value to 0x25cf, which is the unicode character
                                    for a black splodge (not all fonts include this, though), or 0x2022,
                                    which is a bullet (probably the best choice for linux).
    */
    explicit TextEditor (const Txt& componentName = Txt(),
                         t32 passwordCharacter = 0);

    /** Destructor. */
    ~TextEditor() override;

    //==============================================================================
    /** Puts the editor into either multi- or single-line mode.

        By default, the editor will be in single-line mode, so use this if you need a multi-line
        editor.

        See also the setReturnKeyStartsNewLine() method, which will also need to be turned
        on if you want a multi-line editor with line-breaks.

        @param shouldBeMultiLine whether the editor should be multi- or single-line.
        @param shouldWordWrap    sets whether i64 lines should be broken up in multi-line editors.
                                 If this is false and scrollbars are enabled a horizontal scrollbar
                                 will be shown.

        @see isMultiLine, setReturnKeyStartsNewLine, setScrollbarsShown
    */
    z0 setMultiLine (b8 shouldBeMultiLine,
                       b8 shouldWordWrap = true);

    /** Возвращает true, если the editor is in multi-line mode. */
    b8 isMultiLine() const;

    //==============================================================================
    /** Changes the behaviour of the return key.

        If set to true, the return key will insert a new-line into the text; if false
        it will trigger a call to the TextEditor::Listener::textEditorReturnKeyPressed()
        method. By default this is set to false, and when true it will only insert
        new-lines when in multi-line mode (see setMultiLine()).
    */
    z0 setReturnKeyStartsNewLine (b8 shouldStartNewLine);

    /** Returns the value set by setReturnKeyStartsNewLine().
        See setReturnKeyStartsNewLine() for more info.
    */
    b8 getReturnKeyStartsNewLine() const                      { return returnKeyStartsNewLine; }

    /** Indicates whether the tab key should be accepted and used to input a tab character,
        or whether it gets ignored.

        By default the tab key is ignored, so that it can be used to switch keyboard focus
        between components.
    */
    z0 setTabKeyUsedAsCharacter (b8 shouldTabKeyBeUsed);

    /** Возвращает true, если the tab key is being used for input.
        @see setTabKeyUsedAsCharacter
    */
    b8 isTabKeyUsedAsCharacter() const                        { return tabKeyUsed; }

    /** This can be used to change whether escape and return keypress events are
        propagated up to the parent component.
        The default here is true, meaning that these events are not allowed to reach the
        parent, but you may want to allow them through so that they can trigger other
        actions, e.g. closing a dialog box, etc.
    */
    z0 setEscapeAndReturnKeysConsumed (b8 shouldBeConsumed) noexcept;

    //==============================================================================
    /** Changes the editor to read-only mode.

        By default, the text editor is not read-only. If you're making it read-only, you
        might also want to call setCaretVisible (false) to get rid of the caret.

        The text can still be highlighted and copied when in read-only mode.

        @see isReadOnly, setCaretVisible
    */
    z0 setReadOnly (b8 shouldBeReadOnly);

    /** Возвращает true, если the editor is in read-only mode. */
    b8 isReadOnly() const noexcept;

    //==============================================================================
    /** Makes the caret visible or invisible.
        By default the caret is visible.
        @see setCaretColor, setCaretPosition
    */
    z0 setCaretVisible (b8 shouldBeVisible);

    /** Возвращает true, если the caret is enabled.
        @see setCaretVisible
    */
    b8 isCaretVisible() const noexcept                            { return caretVisible && ! isReadOnly(); }

    //==============================================================================
    /** Enables or disables scrollbars (this only applies when in multi-line mode).

        When the text gets too i64 to fit in the component, a scrollbar can appear to
        allow it to be scrolled. Even when this is enabled, the scrollbar will be hidden
        unless it's needed.

        By default scrollbars are enabled.
    */
    z0 setScrollbarsShown (b8 shouldBeEnabled);

    /** Возвращает true, если scrollbars are enabled.
        @see setScrollbarsShown
    */
    b8 areScrollbarsShown() const noexcept                        { return scrollbarVisible; }

    /** Changes the password character used to disguise the text.

        @param passwordCharacter    if this is not zero, this character will be used as a replacement
                                    for all characters that are drawn on screen - e.g. to create
                                    a password-style textbox containing circular blobs instead of text,
                                    you could set this value to 0x25cf, which is the unicode character
                                    for a black splodge (not all fonts include this, though), or 0x2022,
                                    which is a bullet (probably the best choice for linux).
    */
    z0 setPasswordCharacter (t32 passwordCharacter);

    /** Returns the current password character.
        @see setPasswordCharacter
    */
    t32 getPasswordCharacter() const noexcept                { return passwordCharacter; }

    //==============================================================================
    /** Allows a right-click menu to appear for the editor.

        (This defaults to being enabled).

        If enabled, right-clicking (or command-clicking on the Mac) will pop up a menu
        of options such as cut/copy/paste, undo/redo, etc.
    */
    z0 setPopupMenuEnabled (b8 menuEnabled);

    /** Возвращает true, если the right-click menu is enabled.
        @see setPopupMenuEnabled
    */
    b8 isPopupMenuEnabled() const noexcept                        { return popupMenuEnabled; }

    /** Возвращает true, если a popup-menu is currently being displayed. */
    b8 isPopupMenuCurrentlyActive() const noexcept                { return menuActive; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        NB: You can also set the caret colour using CaretComponent::caretColorId

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId       = 0x1000200, /**< The colour to use for the text component's background - this can be
                                                   transparent if necessary. */

        textColorId             = 0x1000201, /**< The colour that will be used when text is added to the editor. Note
                                                   that because the editor can contain multiple colours, calling this
                                                   method won't change the colour of existing text - to do that, use
                                                   the applyColorToAllText() method */

        highlightColorId        = 0x1000202, /**< The colour with which to fill the background of highlighted sections of
                                                   the text - this can be transparent if you don't want to show any
                                                   highlighting.*/

        highlightedTextColorId  = 0x1000203, /**< The colour with which to draw the text in highlighted sections. */

        outlineColorId          = 0x1000205, /**< If this is non-transparent, it will be used to draw a box around
                                                   the edge of the component. */

        focusedOutlineColorId   = 0x1000206, /**< If this is non-transparent, it will be used to draw a box around
                                                   the edge of the component when it has focus. */

        shadowColorId           = 0x1000207, /**< If this is non-transparent, it'll be used to draw an inner shadow
                                                   around the edge of the editor. */
    };

    //==============================================================================
    /** Sets the font to use for newly added text.

        This will change the font that will be used next time any text is added or entered
        into the editor. It won't change the font of any existing text - to do that, use
        applyFontToAllText() instead.

        @see applyFontToAllText
    */
    z0 setFont (const Font& newFont);

    /** Applies a font to all the text in the editor.

        This function also calls
        applyColorToAllText (findColor (TextEditor::ColorIds::textColorId), false);

        If the changeCurrentFont argument is true then this will also set the
        new font as the font to be used for any new text that's added.

        @see setFont
    */
    z0 applyFontToAllText (const Font& newFont, b8 changeCurrentFont = true);

    /** Returns the font that's currently being used for new text.

        @see setFont
    */
    const Font& getFont() const noexcept  { return currentFont; }

    /** Applies a colour to all the text in the editor.

        If the changeCurrentTextColor argument is true then this will also set the
        new colour as the colour to be used for any new text that's added.
    */
    z0 applyColorToAllText (const Color& newColor, b8 changeCurrentTextColor = true);

    /** Sets whether whitespace should be underlined when the editor font is underlined.

        @see isWhitespaceUnderlined
    */
    z0 setWhitespaceUnderlined (b8 shouldUnderlineWhitespace) noexcept  { underlineWhitespace = shouldUnderlineWhitespace; }

    /** Возвращает true, если whitespace is underlined for underlined fonts.

        @see setWhitespaceIsUnderlined
    */
    b8 isWhitespaceUnderlined() const noexcept                            { return underlineWhitespace; }

    //==============================================================================
    /** If set to true, focusing on the editor will highlight all its text.

        (Set to false by default).

        This is useful for boxes where you expect the user to re-enter all the
        text when they focus on the component, rather than editing what's already there.
    */
    z0 setSelectAllWhenFocused (b8 shouldSelectAll);

    /** When the text editor is empty, it can be set to display a message.

        This is handy for things like telling the user what to type in the box - the
        string is only displayed, it's not taken to actually be the contents of
        the editor.
    */
    z0 setTextToShowWhenEmpty (const Txt& text, Color colourToUse);

    /** Returns the text that will be shown when the text editor is empty.

        @see setTextToShowWhenEmpty
    */
    Txt getTextToShowWhenEmpty() const noexcept    { return textToShowWhenEmpty; }

    //==============================================================================
    /** Changes the size of the scrollbars that are used.
        Handy if you need smaller scrollbars for a small text box.
    */
    z0 setScrollBarThickness (i32 newThicknessPixels);

    //==============================================================================
    /**
        Receives callbacks from a TextEditor component when it changes.

        @see TextEditor::addListener
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the user changes the text in some way. */
        virtual z0 textEditorTextChanged (TextEditor&) {}

        /** Called when the user presses the return key. */
        virtual z0 textEditorReturnKeyPressed (TextEditor&) {}

        /** Called when the user presses the escape key. */
        virtual z0 textEditorEscapeKeyPressed (TextEditor&) {}

        /** Called when the text editor loses focus. */
        virtual z0 textEditorFocusLost (TextEditor&) {}
    };

    /** Registers a listener to be told when things happen to the text.
        @see removeListener
    */
    z0 addListener (Listener* newListener);

    /** Deregisters a listener.
        @see addListener
    */
    z0 removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** You can assign a lambda to this callback object to have it called when the text is changed. */
    std::function<z0()> onTextChange;

    /** You can assign a lambda to this callback object to have it called when the return key is pressed. */
    std::function<z0()> onReturnKey;

    /** You can assign a lambda to this callback object to have it called when the escape key is pressed. */
    std::function<z0()> onEscapeKey;

    /** You can assign a lambda to this callback object to have it called when the editor loses key focus. */
    std::function<z0()> onFocusLost;

    //==============================================================================
    /** Returns the entire contents of the editor. */
    Txt getText() const;

    /** Returns a section of the contents of the editor. */
    Txt getTextInRange (const Range<i32>& textRange) const override;

    /** Возвращает true, если there are no characters in the editor.
        This is far more efficient than calling getText().isEmpty().
    */
    b8 isEmpty() const;

    /** Sets the entire content of the editor.

        This will clear the editor and insert the given text (using the current text colour
        and font). You can set the current text colour using
        @code setColor (TextEditor::textColorId, ...);
        @endcode

        @param newText                  the text to add
        @param sendTextChangeMessage    if true, this will cause a change message to
                                        be sent to all the listeners.
        @see insertTextAtCaret
    */
    z0 setText (const Txt& newText,
                  b8 sendTextChangeMessage = true);

    /** Returns a Value object that can be used to get or set the text.

        Bear in mind that this operate quite slowly if your text box contains large
        amounts of text, as it needs to dynamically build the string that's involved.
        It's best used for small text boxes.
    */
    Value& getTextValue();

    /** Inserts some text at the current caret position.

        If a section of the text is highlighted, it will be replaced by
        this string, otherwise it will be inserted.

        To delete a section of text, you can use setHighlightedRegion() to
        highlight it, and call insertTextAtCaret (Txt()).

        @see setCaretPosition, getCaretPosition, setHighlightedRegion
    */
    z0 insertTextAtCaret (const Txt& textToInsert) override;

    /** Deletes all the text from the editor. */
    z0 clear();

    /** Deletes the currently selected region.
        This doesn't copy the deleted section to the clipboard - if you need to do that, call copy() first.
        @see copy, paste, SystemClipboard
    */
    z0 cut();

    /** Copies the currently selected region to the clipboard.
        @see cut, paste, SystemClipboard
    */
    z0 copy();

    /** Pastes the contents of the clipboard into the editor at the caret position.
        @see cut, copy, SystemClipboard
    */
    z0 paste();

    //==============================================================================
    /** Returns the current index of the caret.
        @see setCaretPosition
    */
    i32 getCaretPosition() const override;

    /** Moves the caret to be in front of a given character.
        @see getCaretPosition, moveCaretToEnd
    */
    z0 setCaretPosition (i32 newIndex);

    /** Attempts to scroll the text editor so that the caret ends up at
        a specified position.

        This won't affect the caret's position within the text, it tries to scroll
        the entire editor vertically and horizontally so that the caret is sitting
        at the given position (relative to the top-left of this component).

        Depending on the amount of text available, it might not be possible to
        scroll far enough for the caret to reach this exact position, but it
        will go as far as it can in that direction.
    */
    z0 scrollEditorToPositionCaret (i32 desiredCaretX, i32 desiredCaretY);

    /** Get the graphical position of the caret for a particular index in the text.

        The rectangle returned is relative to the component's top-left corner.
    */
    Rectangle<i32> getCaretRectangleForCharIndex (i32 index) const override;

    /** Selects a section of the text. */
    z0 setHighlightedRegion (const Range<i32>& newSelection) override;

    /** Returns the range of characters that are selected.
        If nothing is selected, this will return an empty range.
        @see setHighlightedRegion
    */
    Range<i32> getHighlightedRegion() const override            { return selection; }

    /** Returns the section of text that is currently selected. */
    Txt getHighlightedText() const;

    /** Finds the index of the character at a given position.
        The coordinates are relative to the component's top-left.
    */
    i32 getTextIndexAt (i32 x, i32 y) const;

    /** Finds the index of the character at a given position.
        The coordinates are relative to the component's top-left.
    */
    i32 getTextIndexAt (Point<i32>) const;

    /** Like getTextIndexAt, but doesn't snap to the beginning/end of the range for
        points vertically outside the text.
    */
    i32 getCharIndexForPoint (Point<i32> point) const override;

    /** Counts the number of characters in the text.

        This is quicker than getting the text as a string if you just need to know
        the length.
    */
    i32 getTotalNumChars() const override;

    /** Returns the total width of the text, as it is currently laid-out.

        This may be larger than the size of the TextEditor, and can change when
        the TextEditor is resized or the text changes.
    */
    i32 getTextWidth() const;

    /** Returns the maximum height of the text, as it is currently laid-out.

        This may be larger than the size of the TextEditor, and can change when
        the TextEditor is resized or the text changes.
    */
    i32 getTextHeight() const;

    /** Changes the size of the gap at the top and left-edge of the editor.
        By default there's a gap of 4 pixels.
    */
    z0 setIndents (i32 newLeftIndent, i32 newTopIndent);

    /** Returns the gap at the top edge of the editor.
        @see setIndents
    */
    i32 getTopIndent() const noexcept   { return topIndent; }

    /** Returns the gap at the left edge of the editor.
        @see setIndents
    */
    i32 getLeftIndent() const noexcept  { return leftIndent; }

    /** Changes the size of border left around the edge of the component.
        @see getBorder
    */
    z0 setBorder (BorderSize<i32> border);

    /** Returns the size of border around the edge of the component.
        @see setBorder
    */
    BorderSize<i32> getBorder() const;

    /** Used to disable the auto-scrolling which keeps the caret visible.

        If true (the default), the editor will scroll when the caret moves offscreen. If
        set to false, it won't.
    */
    z0 setScrollToShowCursor (b8 shouldScrollToShowCaret);

    /** Modifies the justification of the text within the editor window. */
    z0 setJustification (Justification newJustification);

    /** Returns the type of justification, as set in setJustification(). */
    Justification getJustificationType() const noexcept             { return justification; }

    /** Sets the line spacing of the TextEditor.
        The default (and minimum) value is 1.0 and values > 1.0 will increase the line spacing as a
        multiple of the line height e.g. for f64-spacing call this method with an argument of 2.0.
    */
    z0 setLineSpacing (f32 newLineSpacing) noexcept             { lineSpacing = jmax (1.0f, newLineSpacing); }

    /** Returns the current line spacing of the TextEditor. */
    f32 getLineSpacing() const noexcept                           { return lineSpacing; }

    /** Returns the bounding box for a range of text in the editor. As the range may span
        multiple lines, this method returns a RectangleList.

        The bounds are relative to the component's top-left and may extend beyond the bounds
        of the component if the text is i64 and word wrapping is disabled.
    */
    RectangleList<i32> getTextBounds (Range<i32> textRange) const override;

    //==============================================================================
    z0 moveCaretToEnd();
    b8 moveCaretLeft (b8 moveInWholeWordSteps, b8 selecting);
    b8 moveCaretRight (b8 moveInWholeWordSteps, b8 selecting);
    b8 moveCaretUp (b8 selecting);
    b8 moveCaretDown (b8 selecting);
    b8 pageUp (b8 selecting);
    b8 pageDown (b8 selecting);
    b8 scrollDown();
    b8 scrollUp();
    b8 moveCaretToTop (b8 selecting);
    b8 moveCaretToStartOfLine (b8 selecting);
    b8 moveCaretToEnd (b8 selecting);
    b8 moveCaretToEndOfLine (b8 selecting);
    b8 deleteBackwards (b8 moveInWholeWordSteps);
    b8 deleteForwards (b8 moveInWholeWordSteps);
    b8 copyToClipboard();
    b8 cutToClipboard();
    b8 pasteFromClipboard();
    b8 selectAll();
    b8 undo();
    b8 redo();

    //==============================================================================
    /** This adds the items to the popup menu.

        By default it adds the cut/copy/paste items, but you can override this if
        you need to replace these with your own items.

        If you want to add your own items to the existing ones, you can override this,
        call the base class's addPopupMenuItems() method, then append your own items.

        When the menu has been shown, performPopupMenuAction() will be called to
        perform the item that the user has chosen.

        The default menu items will be added using item IDs from the
        StandardApplicationCommandIDs namespace.

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

    //==============================================================================
    /** Base class for input filters that can be applied to a TextEditor to restrict
        the text that can be entered.
    */
    class DRX_API  InputFilter
    {
    public:
        InputFilter() = default;
        virtual ~InputFilter() = default;

        /** This method is called whenever text is entered into the editor.
            An implementation of this class should check the input string,
            and return an edited version of it that should be used.
        */
        virtual Txt filterNewText (TextEditor&, const Txt& newInput) = 0;
    };

    /** An input filter for a TextEditor that limits the length of text and/or the
        characters that it may contain.
    */
    class DRX_API  LengthAndCharacterRestriction  : public InputFilter
    {
    public:
        /** Creates a filter that limits the length of text, and/or the characters that it can contain.
            @param maxNumChars          if this is > 0, it sets a maximum length limit; if <= 0, no
                                        limit is set
            @param allowedCharacters    if this is non-empty, then only characters that occur in
                                        this string are allowed to be entered into the editor.
        */
        LengthAndCharacterRestriction (i32 maxNumChars, const Txt& allowedCharacters);

        Txt filterNewText (TextEditor&, const Txt&) override;

    private:
        Txt allowedCharacters;
        i32 maxLength;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LengthAndCharacterRestriction)
    };

    /** Sets an input filter that should be applied to this editor.
        The filter can be nullptr, to remove any existing filters.
        If takeOwnership is true, then the filter will be owned and deleted by the editor
        when no longer needed.
    */
    z0 setInputFilter (InputFilter* newFilter, b8 takeOwnership);

    /** Returns the current InputFilter, as set by setInputFilter(). */
    InputFilter* getInputFilter() const noexcept                { return inputFilter; }

    /** Sets limits on the characters that can be entered.
        This is just a shortcut that passes an instance of the LengthAndCharacterRestriction
        class to setInputFilter().

        @param maxTextLength        if this is > 0, it sets a maximum length limit; if 0, no
                                    limit is set
        @param allowedCharacters    if this is non-empty, then only characters that occur in
                                    this string are allowed to be entered into the editor.
    */
    z0 setInputRestrictions (i32 maxTextLength,
                               const Txt& allowedCharacters = Txt());

    /** Sets the type of virtual keyboard that should be displayed when this editor has
        focus.
    */
    z0 setKeyboardType (VirtualKeyboardType type) noexcept    { keyboardType = type; }

    /** Sets the behaviour of mouse/touch interactions outside this component.

        If true, then presses outside of the TextEditor will dismiss the virtual keyboard.
        If false, then the virtual keyboard will remain onscreen for as i64 as the TextEditor has
        keyboard focus.
    */
    z0 setClicksOutsideDismissVirtualKeyboard (b8);

    /** Возвращает true, если the editor is configured to hide the virtual keyboard when the mouse is
        pressed on another component.
    */
    b8 getClicksOutsideDismissVirtualKeyboard() const     { return clicksOutsideDismissVirtualKeyboard; }

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        TextEditor drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 fillTextEditorBackground (Graphics&, i32 width, i32 height, TextEditor&) = 0;
        virtual z0 drawTextEditorOutline (Graphics&, i32 width, i32 height, TextEditor&) = 0;

        virtual CaretComponent* createCaretComponent (Component* keyFocusOwner) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 paintOverChildren (Graphics&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseDoubleClick (const MouseEvent&) override;
    /** @internal */
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    b8 keyStateChanged (b8) override;
    /** @internal */
    z0 focusGained (FocusChangeType) override;
    /** @internal */
    z0 focusLost (FocusChangeType) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 enablementChanged() override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    b8 isTextInputActive() const override;
    /** @internal */
    z0 setTemporaryUnderlining (const Array<Range<i32>>&) override;
    /** @internal */
    VirtualKeyboardType getKeyboardType() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** Scrolls the minimum distance needed to get the caret into view. */
    z0 scrollToMakeSureCursorIsVisible();

    /** Used internally to dispatch a text-change message. */
    z0 textChanged();

    /** Begins a new transaction in the UndoManager. */
    z0 newTransaction();

    /** Can be overridden to intercept return key presses directly */
    virtual z0 returnPressed();

    /** Can be overridden to intercept escape key presses directly */
    virtual z0 escapePressed();

private:
    //==============================================================================
    struct TextHolderComponent;
    struct TextEditorViewport;
    struct InsertAction;
    struct RemoveAction;
    class EditorAccessibilityHandler;

    class GlobalMouseListener : private MouseListener
    {
    public:
        explicit GlobalMouseListener (Component& e) : editor (e) { Desktop::getInstance().addGlobalMouseListener    (this); }
        ~GlobalMouseListener() override                          { Desktop::getInstance().removeGlobalMouseListener (this); }

        b8 lastMouseDownInEditor() const { return mouseDownInEditor; }

    private:
        z0 mouseDown (const MouseEvent& event) override { mouseDownInEditor = event.originalComponent == &editor; }

        Component& editor;
        b8 mouseDownInEditor = false;
    };

    std::unique_ptr<Viewport> viewport;
    TextHolderComponent* textHolder;
    BorderSize<i32> borderSize { 1, 1, 1, 3 };
    Justification justification { Justification::topLeft };
    const GlobalMouseListener globalMouseListener { *this };

    b8 readOnly = false;
    b8 caretVisible = true;
    b8 multiline = false;
    b8 wordWrap = false;
    b8 returnKeyStartsNewLine = false;
    b8 popupMenuEnabled = true;
    b8 selectAllTextWhenFocused = false;
    b8 scrollbarVisible = true;
    b8 wasFocused = false;
    b8 keepCaretOnScreen = true;
    b8 tabKeyUsed = false;
    b8 menuActive = false;
    b8 valueTextNeedsUpdating = false;
    b8 consumeEscAndReturnKeys = true;
    b8 underlineWhitespace = true;
    b8 clicksOutsideDismissVirtualKeyboard = false;

    UndoManager undoManager;
    std::unique_ptr<CaretComponent> caret;
    Range<i32> selection;
    i32 leftIndent = 4, topIndent = 4;
    u32 lastTransactionTime = 0;
    Font currentFont { withDefaultMetrics (FontOptions { 14.0f }) };
    mutable i32 totalNumChars = 0;

    //==============================================================================
    enum class Edge
    {
        leading,
        trailing
    };

    //==============================================================================
    struct CaretState
    {
    public:
        explicit CaretState (const TextEditor* ownerIn);

        i32 getPosition() const { return position; }
        Edge getEdge() const { return edge; }

        z0 setPosition (i32 newPosition);

        /*  Not all visual edge positions are permitted e.g. a trailing caret after a newline
            is not allowed. getVisualIndex() and getEdge() will return the closest permitted
            values to the preferred one.
        */
        z0 setPreferredEdge (Edge newEdge);

        /*  The returned value is in the range [0, TextEditor::getTotalNumChars()]. It returns the
            glyph index to which the caret is closest visually. This is significant when
            differentiating between the end of one line and the beginning of the next.
        */
        i32 getVisualIndex() const;

        z0 updateEdge();

        //==============================================================================
        CaretState withPosition (i32 newPosition) const;
        CaretState withPreferredEdge (Edge newEdge) const;

    private:
        const TextEditor& owner;
        i32 position = 0;
        Edge edge = Edge::trailing;
        Edge preferredEdge = Edge::trailing;
    };

    //==============================================================================
    Txt textToShowWhenEmpty;
    Color colourForTextWhenEmpty;
    t32 passwordCharacter;
    OptionalScopedPointer<InputFilter> inputFilter;
    Value textValue;
    VirtualKeyboardType keyboardType = TextInputTarget::textKeyboard;
    f32 lineSpacing = 1.0f;

    enum DragType
    {
        notDragging,
        draggingSelectionStart,
        draggingSelectionEnd
    };

    DragType dragType = notDragging;

    ListenerList<Listener> listeners;
    Array<Range<i32>> underlinedSections;

    class ParagraphStorage;
    class ParagraphsModel;
    struct TextEditorStorageChunks;
    class TextEditorStorage;

    z0 moveCaret (i32 newCaretPos);
    z0 moveCaretTo (i32 newPosition, b8 isSelecting);
    z0 recreateCaret();
    z0 handleCommandMessage (i32) override;
    z0 clearInternal (UndoManager*);
    z0 insert (const Txt&, i32 insertIndex, const Font&, Color, UndoManager*, i32 newCaretPos);
    z0 reinsert (const TextEditorStorageChunks& chunks);
    z0 remove (Range<i32>, UndoManager*, i32 caretPositionToMoveTo, TextEditorStorageChunks* removedOut = nullptr);

    struct CaretEdge
    {
        Point<f32> anchor;
        f32 height{};
    };

    f32 getJustificationOffsetX() const;
    CaretEdge getDefaultCursorEdge() const;
    CaretEdge getTextSelectionEdge (i32 index, Edge edge) const;
    CaretEdge getCursorEdge (const CaretState& caret) const;
    z0 updateCaretPosition();
    z0 updateValueFromText();
    z0 textWasChangedByValue();
    i32 indexAtPosition (f32 x, f32 y) const;
    i32 findWordBreakAfter (i32 position) const;
    i32 findWordBreakBefore (i32 position) const;
    b8 moveCaretWithTransaction (i32 newPos, b8 selecting);
    z0 drawContent (Graphics&);
    z0 checkLayout();
    i32 getWordWrapWidth() const;
    i32 getMaximumTextWidth() const;
    i32 getMaximumTextHeight() const;
    z0 timerCallbackInt();
    z0 checkFocus();
    z0 repaintText (Range<i32>);
    z0 scrollByLines (i32 deltaLines);
    b8 undoOrRedo (b8 shouldUndo);
    UndoManager* getUndoManager() noexcept;
    z0 setSelection (Range<i32>) noexcept;
    Point<i32> getTextOffset() const;

    Edge getEdgeTypeCloserToPosition (i32 indexInText, Point<f32> pos) const;

    std::unique_ptr<TextEditorStorage> textStorage;
    CaretState caretState;

    b8 isTextStorageHeightGreaterEqualThan (f32 value) const;
    f32 getTextStorageHeight() const;
    f32 getYOffset() const;
    z0 updateBaseShapedTextOptions();
    Range<z64> getLineRangeForIndex (i32 index);

    template <typename T>
    detail::RangedValues<T> getGlyphRanges (const detail::RangedValues<T>& textRanges) const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditor)
};


} // namespace drx
