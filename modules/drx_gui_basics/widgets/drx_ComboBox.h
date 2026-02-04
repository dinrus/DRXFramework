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
    A component that lets the user choose from a drop-down list of choices.

    The combo-box has a list of text strings, each with an associated id number,
    that will be shown in the drop-down list when the user clicks on the component.

    The currently selected choice is displayed in the combo-box, and this can
    either be read-only text, or editable.

    To find out when the user selects a different item or edits the text, you
    can assign a lambda to the onChange member, or register a ComboBox::Listener
    to receive callbacks.

    @tags{GUI}
*/
class DRX_API  ComboBox  : public Component,
                            public SettableTooltipClient,
                            public Value::Listener,
                            private AsyncUpdater
{
public:
    //==============================================================================
    /** Creates a combo-box.

        On construction, the text field will be empty, so you should call the
        setSelectedId() or setText() method to choose the initial value before
        displaying it.

        @param componentName    the name to set for the component (see Component::setName())
    */
    explicit ComboBox (const Txt& componentName = {});

    /** Destructor. */
    ~ComboBox() override;

    //==============================================================================
    /** Sets whether the text in the combo-box is editable.

        The default state for a new ComboBox is non-editable, and can only be changed
        by choosing from the drop-down list.
    */
    z0 setEditableText (b8 isEditable);

    /** Возвращает true, если the text is directly editable.
        @see setEditableText
    */
    b8 isTextEditable() const noexcept;

    /** Sets the style of justification to be used for positioning the text.

        The default is Justification::centredLeft. The text is displayed using a
        Label component inside the ComboBox.
    */
    z0 setJustificationType (Justification justification);

    /** Returns the current justification for the text box.
        @see setJustificationType
    */
    Justification getJustificationType() const noexcept;

    //==============================================================================
    /** Adds an item to be shown in the drop-down list.

        @param newItemText      the text of the item to show in the list
        @param newItemId        an associated ID number that can be set or retrieved - see
                                getSelectedId() and setSelectedId(). Note that this value can not
                                be 0!
        @see setItemEnabled, addSeparator, addSectionHeading, getNumItems, getItemText, getItemId
    */
    z0 addItem (const Txt& newItemText, i32 newItemId);

    /** Adds an array of items to the drop-down list.
        The item ID of each item will be its index in the StringArray + firstItemIdOffset.
    */
    z0 addItemList (const StringArray& items, i32 firstItemIdOffset);

    /** Adds a separator line to the drop-down list.

        This is like adding a separator to a popup menu. See PopupMenu::addSeparator().
    */
    z0 addSeparator();

    /** Adds a heading to the drop-down list, so that you can group the items into
        different sections.

        The headings are indented slightly differently to set them apart from the
        items on the list, and obviously can't be selected. You might want to add
        separators between your sections too.

        @see addItem, addSeparator
    */
    z0 addSectionHeading (const Txt& headingName);

    /** This allows items in the drop-down list to be selectively disabled.

        When you add an item, it's enabled by default, but you can call this
        method to change its status.

        If you disable an item which is already selected, this won't change the
        current selection - it just stops the user choosing that item from the list.
    */
    z0 setItemEnabled (i32 itemId, b8 shouldBeEnabled);

    /** Возвращает true, если the given item is enabled. */
    b8 isItemEnabled (i32 itemId) const noexcept;

    /** Changes the text for an existing item.
    */
    z0 changeItemText (i32 itemId, const Txt& newText);

    /** Removes all the items from the drop-down list.

        If this call causes the content to be cleared, and a change-message
        will be broadcast according to the notification parameter.

        @see addItem, getNumItems
    */
    z0 clear (NotificationType notification = sendNotificationAsync);

    /** Returns the number of items that have been added to the list.

        Note that this doesn't include headers or separators.
    */
    i32 getNumItems() const noexcept;

    /** Returns the text for one of the items in the list.
        Note that this doesn't include headers or separators.
        @param index    the item's index from 0 to (getNumItems() - 1)
    */
    Txt getItemText (i32 index) const;

    /** Returns the ID for one of the items in the list.
        Note that this doesn't include headers or separators.
        @param index    the item's index from 0 to (getNumItems() - 1)
    */
    i32 getItemId (i32 index) const noexcept;

    /** Returns the index in the list of a particular item ID.
        If no such ID is found, this will return -1.
    */
    i32 indexOfItemId (i32 itemId) const noexcept;

    //==============================================================================
    /** Returns the ID of the item that's currently shown in the box.

        If no item is selected, or if the text is editable and the user
        has entered something which isn't one of the items in the list, then
        this will return 0.

        @see setSelectedId, getSelectedItemIndex, getText
    */
    i32 getSelectedId() const noexcept;

    /** Returns a Value object that can be used to get or set the selected item's ID.

        You can call Value::referTo() on this object to make the combo box control
        another Value object.
    */
    Value& getSelectedIdAsValue()                       { return currentId; }

    /** Sets one of the items to be the current selection.

        This will set the ComboBox's text to that of the item that matches
        this ID.

        @param newItemId        the new item to select
        @param notification     determines the type of change notification that will
                                be sent to listeners if the value changes
        @see getSelectedId, setSelectedItemIndex, setText
    */
    z0 setSelectedId (i32 newItemId,
                        NotificationType notification = sendNotificationAsync);

    //==============================================================================
    /** Returns the index of the item that's currently shown in the box.

        If no item is selected, or if the text is editable and the user
        has entered something which isn't one of the items in the list, then
        this will return -1.

        @see setSelectedItemIndex, getSelectedId, getText
    */
    i32 getSelectedItemIndex() const;

    /** Sets one of the items to be the current selection.

        This will set the ComboBox's text to that of the item at the given
        index in the list.

        @param newItemIndex     the new item to select
        @param notification     determines the type of change notification that will
                                be sent to listeners if the value changes
        @see getSelectedItemIndex, setSelectedId, setText
    */
    z0 setSelectedItemIndex (i32 newItemIndex,
                               NotificationType notification = sendNotificationAsync);

    //==============================================================================
    /** Returns the text that is currently shown in the combo-box's text field.

        If the ComboBox has editable text, then this text may have been edited
        by the user; otherwise it will be one of the items from the list, or
        possibly an empty string if nothing was selected.

        @see setText, getSelectedId, getSelectedItemIndex
    */
    Txt getText() const;

    /** Sets the contents of the combo-box's text field.

        The text passed-in will be set as the current text regardless of whether
        it is one of the items in the list. If the current text isn't one of the
        items, then getSelectedId() will return 0, otherwise it will return
        the appropriate ID.

        @param newText          the text to select
        @param notification     determines the type of change notification that will
                                be sent to listeners if the text changes
        @see getText
    */
    z0 setText (const Txt& newText,
                  NotificationType notification = sendNotificationAsync);

    /** Programmatically opens the text editor to allow the user to edit the current item.

        This is the same effect as when the box is clicked-on.
        @see Label::showEditor();
    */
    z0 showEditor();

    /** Pops up the combo box's list.
        This is virtual so that you can override it with your own custom popup
        mechanism if you need some really unusual behaviour.
    */
    virtual z0 showPopup();

    /** Hides the combo box's popup list, if it's currently visible. */
    z0 hidePopup();

    /** Возвращает true, если the popup menu is currently being shown. */
    b8 isPopupActive() const noexcept                 { return menuActive; }

    /** Returns the PopupMenu object associated with the ComboBox.
        Can be useful for adding sub-menus to the ComboBox standard PopupMenu
    */
    PopupMenu* getRootMenu() noexcept { return &currentMenu; }

    /** Returns the PopupMenu object associated with the ComboBox. */
    const PopupMenu* getRootMenu() const noexcept { return &currentMenu; }

    //==============================================================================
    /**
        A class for receiving events from a ComboBox.

        You can register a ComboBox::Listener with a ComboBox using the ComboBox::addListener()
        method, and it will be called when the selected item in the box changes.

        @see ComboBox::addListener, ComboBox::removeListener
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when a ComboBox has its selected item changed. */
        virtual z0 comboBoxChanged (ComboBox* comboBoxThatHasChanged) = 0;
    };

    /** Registers a listener that will be called when the box's content changes. */
    z0 addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** You can assign a lambda to this callback object to have it called when the selected ID is changed. */
    std::function<z0()> onChange;

    //==============================================================================
    /** Sets a message to display when there is no item currently selected.
        @see getTextWhenNothingSelected
    */
    z0 setTextWhenNothingSelected (const Txt& newMessage);

    /** Returns the text that is shown when no item is selected.
        @see setTextWhenNothingSelected
    */
    Txt getTextWhenNothingSelected() const;

    /** Sets the message to show when there are no items in the list, and the user clicks
        on the drop-down box.

        By default it just says "no choices", but this lets you change it to something more
        meaningful.
    */
    z0 setTextWhenNoChoicesAvailable (const Txt& newMessage);

    /** Returns the text shown when no items have been added to the list.
        @see setTextWhenNoChoicesAvailable
    */
    Txt getTextWhenNoChoicesAvailable() const;

    //==============================================================================
    /** Gives the ComboBox a tooltip. */
    z0 setTooltip (const Txt& newTooltip) override;

    /** This can be used to allow the scroll-wheel to nudge the chosen item.
        By default it's disabled, and I'd recommend leaving it disabled if there's any
        chance that the control might be inside a scrollable list or viewport.
    */
    z0 setScrollWheelEnabled (b8 enabled) noexcept;


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the combo box.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        To change the colours of the menu that pops up, you can set the colour IDs in PopupMenu::ColorIDs.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId     = 0x1000b00,   /**< The background colour to fill the box with. */
        textColorId           = 0x1000a00,   /**< The colour for the text in the box. */
        outlineColorId        = 0x1000c00,   /**< The colour for an outline around the box. */
        buttonColorId         = 0x1000d00,   /**< The base colour for the button (a LookAndFeel class will probably use variations on this). */
        arrowColorId          = 0x1000e00,   /**< The colour for the arrow shape that pops up the menu */
        focusedOutlineColorId = 0x1000f00    /**< The colour that will be used to draw a box around the edge of the component when it has focus. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        ComboBox functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawComboBox (Graphics&, i32 width, i32 height, b8 isButtonDown,
                                   i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH,
                                   ComboBox&) = 0;

        virtual Font getComboBoxFont (ComboBox&) = 0;

        virtual Label* createComboBoxTextBox (ComboBox&) = 0;

        virtual z0 positionComboBoxText (ComboBox&, Label& labelToPosition) = 0;

        virtual PopupMenu::Options getOptionsForComboBoxPopupMenu (ComboBox&, Label&) = 0;

        virtual z0 drawComboBoxTextWhenNothingSelected (Graphics&, ComboBox&, Label&) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 enablementChanged() override;
    /** @internal */
    z0 colourChanged() override;
    /** @internal */
    z0 focusGained (Component::FocusChangeType) override;
    /** @internal */
    z0 focusLost (Component::FocusChangeType) override;
    /** @internal */
    z0 handleAsyncUpdate() override;
    /** @internal */
    Txt getTooltip() override                        { return label->getTooltip(); }
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    b8 keyStateChanged (b8) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 valueChanged (Value&) override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    //==============================================================================
   #ifndef DOXYGEN
    // These methods' b8 parameters have changed: see their new method signatures.
    [[deprecated]] z0 clear (b8);
    [[deprecated]] z0 setSelectedId (i32, b8);
    [[deprecated]] z0 setSelectedItemIndex (i32, b8);
    [[deprecated]] z0 setText (const Txt&, b8);
   #endif

private:
    //==============================================================================
    enum EditableState
    {
        editableUnknown,
        labelIsNotEditable,
        labelIsEditable
    };

    PopupMenu currentMenu;
    Value currentId;
    i32 lastCurrentId = 0;
    b8 isButtonDown = false, menuActive = false, scrollWheelEnabled = false;
    f32 mouseWheelAccumulator = 0;
    ListenerList<Listener> listeners;
    std::unique_ptr<Label> label;
    Txt textWhenNothingSelected, noChoicesMessage;
    EditableState labelEditableState = editableUnknown;

    PopupMenu::Item* getItemForId (i32) const noexcept;
    PopupMenu::Item* getItemForIndex (i32) const noexcept;
    b8 selectIfEnabled (i32 index);
    b8 nudgeSelectedItem (i32 delta);
    z0 sendChange (NotificationType);
    z0 showPopupIfNotActive();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBox)
};


} // namespace drx
