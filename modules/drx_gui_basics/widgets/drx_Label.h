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
    A component that displays a text string, and can optionally become a text
    editor when clicked.

    @tags{GUI}
*/
class DRX_API  Label  : public Component,
                         public SettableTooltipClient,
                         protected TextEditor::Listener,
                         private ComponentListener,
                         private Value::Listener
{
public:
    //==============================================================================
    /** Creates a Label.

        @param componentName    the name to give the component
        @param labelText        the text to show in the label
    */
    Label (const Txt& componentName = Txt(),
           const Txt& labelText = Txt());

    /** Destructor. */
    ~Label() override;

    //==============================================================================
    /** Changes the label text.

        The NotificationType parameter indicates whether to send a change message to
        any Label::Listener objects if the new text is different.
    */
    z0 setText (const Txt& newText,
                  NotificationType notification);

    /** Returns the label's current text.

        @param returnActiveEditorContents   if this is true and the label is currently
                                            being edited, then this method will return the
                                            text as it's being shown in the editor. If false,
                                            then the value returned here won't be updated until
                                            the user has finished typing and pressed the return
                                            key.
    */
    Txt getText (b8 returnActiveEditorContents = false) const;

    /** Returns the text content as a Value object.
        You can call Value::referTo() on this object to make the label read and control
        a Value object that you supply.
    */
    Value& getTextValue() noexcept                          { return textValue; }

    //==============================================================================
    /** Changes the font to use to draw the text.
        @see getFont
    */
    z0 setFont (const Font& newFont);

    /** Returns the font currently being used.
        This may be the one set by setFont(), unless it has been overridden by the current LookAndFeel
        @see setFont
    */
    Font getFont() const noexcept;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the label.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        Note that you can also use the constants from TextEditor::ColorIds to change the
        colour of the text editor that is opened when a label is editable.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId             = 0x1000280, /**< The background colour to fill the label with. */
        textColorId                   = 0x1000281, /**< The colour for the text. */
        outlineColorId                = 0x1000282, /**< An optional colour to use to draw a border around the label.
                                                         Leave this transparent to not have an outline. */
        backgroundWhenEditingColorId  = 0x1000283, /**< The background colour when the label is being edited. */
        textWhenEditingColorId        = 0x1000284, /**< The colour for the text when the label is being edited. */
        outlineWhenEditingColorId     = 0x1000285  /**< An optional border colour when the label is being edited. */
    };

    //==============================================================================
    /** Sets the style of justification to be used for positioning the text.
        (The default is Justification::centredLeft)
    */
    z0 setJustificationType (Justification justification);

    /** Returns the type of justification, as set in setJustificationType(). */
    Justification getJustificationType() const noexcept                         { return justification; }

    /** Changes the border that is left between the edge of the component and the text.
        By default there's a small gap left at the sides of the component to allow for
        the drawing of the border, but you can change this if necessary.
    */
    z0 setBorderSize (BorderSize<i32> newBorderSize);

    /** Returns the size of the border to be left around the text. */
    BorderSize<i32> getBorderSize() const noexcept                              { return border; }

    /** Makes this label "stick to" another component.

        This will cause the label to follow another component around, staying
        either to its left or above it.

        @param owner    the component to follow
        @param onLeft   if true, the label will stay on the left of its component; if
                        false, it will stay above it.
    */
    z0 attachToComponent (Component* owner, b8 onLeft);

    /** If this label has been attached to another component using attachToComponent, this
        returns the other component.

        Returns nullptr if the label is not attached.
    */
    Component* getAttachedComponent() const;

    /** If the label is attached to the left of another component, this returns true.

        Returns false if the label is above the other component. This is only relevant if
        attachToComponent() has been called.
    */
    b8 isAttachedOnLeft() const noexcept                                      { return leftOfOwnerComp; }

    /** Specifies the minimum amount that the font can be squashed horizontally before it starts
        using ellipsis. Use a value of 0 for a default value.

        @see Graphics::drawFittedText
    */
    z0 setMinimumHorizontalScale (f32 newScale);

    /** Specifies the amount that the font can be squashed horizontally. */
    f32 getMinimumHorizontalScale() const noexcept                            { return minimumHorizontalScale; }

    /** Set a keyboard type for use when the text editor is shown. */
    z0 setKeyboardType (TextInputTarget::VirtualKeyboardType type) noexcept   { keyboardType = type; }

    //==============================================================================
    /**
        A class for receiving events from a Label.

        You can register a Label::Listener with a Label using the Label::addListener()
        method, and it will be called when the text of the label changes, either because
        of a call to Label::setText() or by the user editing the text (if the label is
        editable).

        @see Label::addListener, Label::removeListener
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when a Label's text has changed. */
        virtual z0 labelTextChanged (Label* labelThatHasChanged) = 0;

        /** Called when a Label goes into editing mode and displays a TextEditor. */
        virtual z0 editorShown (Label*, TextEditor&) {}

        /** Called when a Label is about to delete its TextEditor and exit editing mode. */
        virtual z0 editorHidden (Label*, TextEditor&) {}
    };

    /** Registers a listener that will be called when the label's text changes. */
    z0 addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** You can assign a lambda to this callback object to have it called when the label text is changed. */
    std::function<z0()> onTextChange;

    /** You can assign a lambda to this callback object to have it called when the label's editor is shown. */
    std::function<z0()> onEditorShow;

    /** You can assign a lambda to this callback object to have it called when the label's editor is hidden. */
    std::function<z0()> onEditorHide;

    //==============================================================================
    /** Makes the label turn into a TextEditor when clicked.

        By default this is turned off.

        If turned on, then single- or f64-clicking will turn the label into
        an editor. If the user then changes the text, then the ChangeBroadcaster
        base class will be used to send change messages to any listeners that
        have registered.

        If the user changes the text, the textWasEdited() method will be called
        afterwards, and subclasses can override this if they need to do anything
        special.

        @param editOnSingleClick            if true, just clicking once on the label will start editing the text
        @param editOnDoubleClick            if true, a f64-click is needed to start editing
        @param lossOfFocusDiscardsChanges   if true, clicking somewhere else while the text is being
                                            edited will discard any changes; if false, then this will
                                            commit the changes.
        @see showEditor, setEditorColors, TextEditor
    */
    z0 setEditable (b8 editOnSingleClick,
                      b8 editOnDoubleClick = false,
                      b8 lossOfFocusDiscardsChanges = false);

    /** Возвращает true, если this option was set using setEditable(). */
    b8 isEditableOnSingleClick() const noexcept                       { return editSingleClick; }

    /** Возвращает true, если this option was set using setEditable(). */
    b8 isEditableOnDoubleClick() const noexcept                       { return editDoubleClick; }

    /** Возвращает true, если this option has been set in a call to setEditable(). */
    b8 doesLossOfFocusDiscardChanges() const noexcept                 { return lossOfFocusDiscardsChanges; }

    /** Возвращает true, если the user can edit this label's text. */
    b8 isEditable() const noexcept                                    { return editSingleClick || editDoubleClick; }

    /** Makes the editor appear as if the label had been clicked by the user.
        @see textWasEdited, setEditable
    */
    z0 showEditor();

    /** Hides the editor if it was being shown.

        @param discardCurrentEditorContents     if true, the label's text will be
                                                reset to whatever it was before the editor
                                                was shown; if false, the current contents of the
                                                editor will be used to set the label's text
                                                before it is hidden.
    */
    z0 hideEditor (b8 discardCurrentEditorContents);

    /** Возвращает true, если the editor is currently focused and active. */
    b8 isBeingEdited() const noexcept;

    /** Returns the currently-visible text editor, or nullptr if none is open. */
    TextEditor* getCurrentTextEditor() const noexcept;

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        label drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawLabel (Graphics&, Label&) = 0;
        virtual Font getLabelFont (Label&) = 0;
        virtual BorderSize<i32> getLabelBorderSize (Label&) = 0;
    };

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** Creates the TextEditor component that will be used when the user has clicked on the label.
        Subclasses can override this if they need to customise this component in some way.
    */
    virtual TextEditor* createEditorComponent();

    /** Called after the user changes the text. */
    virtual z0 textWasEdited();

    /** Called when the text has been altered. */
    virtual z0 textWasChanged();

    /** Called when the text editor has just appeared, due to a user click or other focus change. */
    virtual z0 editorShown (TextEditor*);

    /** Called when the text editor is going to be deleted, after editing has finished. */
    virtual z0 editorAboutToBeHidden (TextEditor*);

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 mouseDoubleClick (const MouseEvent&) override;
    /** @internal */
    z0 componentMovedOrResized (Component&, b8 wasMoved, b8 wasResized) override;
    /** @internal */
    z0 componentParentHierarchyChanged (Component&) override;
    /** @internal */
    z0 componentVisibilityChanged (Component&) override;
    /** @internal */
    z0 inputAttemptWhenModal() override;
    /** @internal */
    z0 focusGained (FocusChangeType) override;
    /** @internal */
    z0 enablementChanged() override;
    /** @internal */
    std::unique_ptr<ComponentTraverser> createKeyboardFocusTraverser() override;
    /** @internal */
    z0 textEditorTextChanged (TextEditor&) override;
    /** @internal */
    z0 textEditorReturnKeyPressed (TextEditor&) override;
    /** @internal */
    z0 textEditorEscapeKeyPressed (TextEditor&) override;
    /** @internal */
    z0 textEditorFocusLost (TextEditor&) override;
    /** @internal */
    z0 colourChanged() override;
    /** @internal */
    z0 valueChanged (Value&) override;
    /** @internal */
    z0 callChangeListeners();

private:
    //==============================================================================
    Value textValue;
    Txt lastTextValue;
    Font font { withDefaultMetrics (FontOptions { 15.0f }) };
    Justification justification = Justification::centredLeft;
    std::unique_ptr<TextEditor> editor;
    ListenerList<Listener> listeners;
    WeakReference<Component> ownerComponent;
    BorderSize<i32> border { 1, 5, 1, 5 };
    f32 minimumHorizontalScale = 0;
    TextInputTarget::VirtualKeyboardType keyboardType = TextInputTarget::textKeyboard;
    b8 editSingleClick = false;
    b8 editDoubleClick = false;
    b8 lossOfFocusDiscardsChanges = false;
    b8 leftOfOwnerComp = false;

    b8 updateFromTextEditorContents (TextEditor&);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Label)
};


} // namespace drx
