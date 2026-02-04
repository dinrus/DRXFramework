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
/** A window that displays a message and has buttons for the user to react to it.

    For simple dialog boxes with just a couple of buttons on them, there are
    some static methods for running these.

    For more complex dialogs, an AlertWindow can be created, then it can have some
    buttons and components added to it, and its enterModalState() method is used to
    show it. The value returned to the ModalComponentManager::Callback shows
    which button the user pressed to dismiss the box.

    @see ThreadWithProgressWindow, Component::enterModalState

    @tags{GUI}
*/
class DRX_API  AlertWindow  : public TopLevelWindow
{
public:
    //==============================================================================
    /** Creates an AlertWindow.

        @param title    the headline to show at the top of the dialog box
        @param message  a longer, more descriptive message to show underneath the
                        headline
        @param iconType the type of icon to display
        @param associatedComponent   if this is non-null, it specifies the component that the
                        alert window should be associated with. Depending on the look
                        and feel, this might be used for positioning of the alert window.
    */
    AlertWindow (const Txt& title,
                 const Txt& message,
                 MessageBoxIconType iconType,
                 Component* associatedComponent = nullptr);

    /** Destroys the AlertWindow */
    ~AlertWindow() override;

    //==============================================================================
    /** Returns the type of alert icon that was specified when the window
        was created. */
    MessageBoxIconType getAlertType() const noexcept  { return alertIconType; }

    //==============================================================================
    /** Changes the dialog box's message.

        This will also resize the window to fit the new message if required.
    */
    z0 setMessage (const Txt& message);

    //==============================================================================
    /** Adds a button to the window.

        @param name         the text to show on the button
        @param returnValue  the value that should be returned from runModalLoop()
                            if this is the button that the user presses.
        @param shortcutKey1 an optional key that can be pressed to trigger this button
        @param shortcutKey2 a second optional key that can be pressed to trigger this button
    */
    z0 addButton (const Txt& name,
                    i32 returnValue,
                    const KeyPress& shortcutKey1 = KeyPress(),
                    const KeyPress& shortcutKey2 = KeyPress());

    /** Returns the number of buttons that the window currently has. */
    i32 getNumButtons() const;

    /** Returns a Button that was added to the AlertWindow.

        @param index   the index of the button in order that it was added with the addButton() method.
        @returns the Button component, or nullptr if the index is out of bounds.

        @see getNumButtons
    */
    Button* getButton (i32 index) const;

    /** Returns a Button that was added to the AlertWindow.

        @param buttonName   the name that was passed into the addButton() method
        @returns the Button component, or nullptr if none was found for the given name.
    */
    Button* getButton (const Txt& buttonName) const;

    /** Invokes a click of one of the buttons. */
    z0 triggerButtonClick (const Txt& buttonName);

    /** If set to true and the window contains no buttons, then pressing the escape key will make
        the alert cancel its modal state.
        By default this setting is true - turn it off if you don't want the box to respond to
        the escape key. Note that it is ignored if you have any buttons, and in that case you
        should give the buttons appropriate keypresses to trigger cancelling if you want to.
    */
    z0 setEscapeKeyCancels (b8 shouldEscapeKeyCancel);

    //==============================================================================
    /** Adds a textbox to the window for entering strings.

        @param name             an internal name for the text-box. This is the name to pass to
                                the getTextEditorContents() method to find out what the
                                user typed-in.
        @param initialContents  a string to show in the text box when it's first shown
        @param onScreenLabel    if this is non-empty, it will be displayed next to the
                                text-box to label it.
        @param isPasswordBox    if true, the text editor will display asterisks instead of
                                the actual text
        @see getTextEditorContents
    */
    z0 addTextEditor (const Txt& name,
                        const Txt& initialContents,
                        const Txt& onScreenLabel = Txt(),
                        b8 isPasswordBox = false);

    /** Returns the contents of a named textbox.

        After showing an AlertWindow that contains a text editor, this can be
        used to find out what the user has typed into it.

        @param nameOfTextEditor     the name of the text box that you're interested in
        @see addTextEditor
    */
    Txt getTextEditorContents (const Txt& nameOfTextEditor) const;

    /** Returns a pointer to a textbox that was added with addTextEditor(). */
    TextEditor* getTextEditor (const Txt& nameOfTextEditor) const;

    //==============================================================================
    /** Adds a drop-down list of choices to the box.

        After the box has been shown, the getComboBoxComponent() method can
        be used to find out which item the user picked.

        @param name     the label to use for the drop-down list
        @param items    the list of items to show in it
        @param onScreenLabel    if this is non-empty, it will be displayed next to the
                                combo-box to label it.
        @see getComboBoxComponent
    */
    z0 addComboBox (const Txt& name,
                      const StringArray& items,
                      const Txt& onScreenLabel = Txt());

    /** Returns a drop-down list that was added to the AlertWindow.

        @param nameOfList   the name that was passed into the addComboBox() method
                            when creating the drop-down
        @returns the ComboBox component, or nullptr if none was found for the given name.
    */
    ComboBox* getComboBoxComponent (const Txt& nameOfList) const;

    //==============================================================================
    /** Adds a block of text.

        This is handy for adding a multi-line note next to a textbox or combo-box,
        to provide more details about what's going on.
    */
    z0 addTextBlock (const Txt& text);

    //==============================================================================
    /** Adds a progress-bar to the window.

        @param progressValue    a variable that will be repeatedly checked while the
                                dialog box is visible, to see how far the process has
                                got. The value should be in the range 0 to 1.0
        @param style            determines the style the ProgressBar should adopt.
                                By default this use a style automatically chosen by
                                the LookAndFeel, but you can force a particular style
                                by passing a non-optional value.
        @see ProgressBar::setStyle
    */
    z0 addProgressBarComponent (f64& progressValue, std::optional<ProgressBar::Style> style = std::nullopt);

    //==============================================================================
    /** Adds a user-defined component to the dialog box.

        @param component    the component to add - its size should be set up correctly
                            before it is passed in. The caller is responsible for deleting
                            the component later on - the AlertWindow won't delete it.
    */
    z0 addCustomComponent (Component* component);

    /** Returns the number of custom components in the dialog box.
        @see getCustomComponent, addCustomComponent
    */
    i32 getNumCustomComponents() const;

    /** Returns one of the custom components in the dialog box.

        @param index    a value 0 to (getNumCustomComponents() - 1).
                        Out-of-range indexes will return nullptr
        @see getNumCustomComponents, addCustomComponent
    */
    Component* getCustomComponent (i32 index) const;

    /** Removes one of the custom components in the dialog box.
        Note that this won't delete it, it just removes the component from the window

        @param index    a value 0 to (getNumCustomComponents() - 1).
                        Out-of-range indexes will return nullptr
        @returns        the component that was removed (or null)
        @see getNumCustomComponents, addCustomComponent
    */
    Component* removeCustomComponent (i32 index);

    //==============================================================================
    /** Возвращает true, если the window contains any components other than just buttons.*/
    b8 containsAnyExtraComponents() const;

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    /** Shows a dialog box that just has a message and a single button to get rid of it.

        The box is shown modally, and the method will block until the user has clicked the
        button (or pressed the escape or return keys).

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param buttonText   the text to show in the button - if this string is empty, the
                            default string "OK" (or a localised version) will be used.
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
    */
    static z0 DRX_CALLTYPE showMessageBox (MessageBoxIconType iconType,
                                              const Txt& title,
                                              const Txt& message,
                                              const Txt& buttonText = Txt(),
                                              Component* associatedComponent = nullptr);

    /** Shows a dialog box using the specified options.

        The box is shown modally, and the method will block until the user dismisses it.

        @param options  the options to use when creating the dialog.

        @returns  the index of the button that was clicked.

        @see MessageBoxOptions
    */
    static i32 DRX_CALLTYPE show (const MessageBoxOptions& options);
   #endif

    /** Shows a dialog box using the specified options.

        The box will be displayed and placed into a modal state, but this method will return
        immediately, and the callback will be invoked later when the user dismisses the box.

        @param options   the options to use when creating the dialog.
        @param callback  if this is non-null, the callback will receive a call to its
                         modalStateFinished() when the box is dismissed with the index of the
                         button that was clicked as its argument.
                         The callback object will be owned and deleted by the system, so make sure
                         that it works safely and doesn't keep any references to objects that might
                         be deleted before it gets called.

        @see MessageBoxOptions
    */
    static z0 DRX_CALLTYPE showAsync (const MessageBoxOptions& options,
                                         ModalComponentManager::Callback* callback);

    /** Shows a dialog box using the specified options.

        The box will be displayed and placed into a modal state, but this method will return
        immediately, and the callback will be invoked later when the user dismisses the box.

        @param options   the options to use when creating the dialog.
        @param callback  if this is non-null, the callback will be called when the box is
                         dismissed with the index of the button that was clicked as its argument.

        @see MessageBoxOptions
    */
    static z0 DRX_CALLTYPE showAsync (const MessageBoxOptions& options,
                                         std::function<z0 (i32)> callback);

    /** Shows a dialog box that just has a message and a single button to get rid of it.

        The box will be displayed and placed into a modal state, but this method will
        return immediately, and if a callback was supplied, it will be invoked later
        when the user dismisses the box.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param buttonText   the text to show in the button - if this string is empty, the
                            default string "OK" (or a localised version) will be used.
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the callback will receive a call to its
                            modalStateFinished() when the box is dismissed. The callback object
                            will be owned and deleted by the system, so make sure that it works
                            safely and doesn't keep any references to objects that might be deleted
                            before it gets called.
    */
    static z0 DRX_CALLTYPE showMessageBoxAsync (MessageBoxIconType iconType,
                                                   const Txt& title,
                                                   const Txt& message,
                                                   const Txt& buttonText = Txt(),
                                                   Component* associatedComponent = nullptr,
                                                   ModalComponentManager::Callback* callback = nullptr);

    /** Shows a dialog box with two buttons.

        Ideal for ok/cancel or yes/no choices. The return key can also be used
        to trigger the first button, and the escape key for the second button.

        If DRX_MODAL_LOOPS_PERMITTED is not defined or the callback parameter is non-null,
        this function will return immediately. The object passed as the callback argument will
        receive the result of the alert window asynchronously.
        Otherwise, if DRX_MODAL_LOOPS_PERMITTED is defined and the callback parameter is null,
        the box is shown modally, and the method will block until the user has clicked the button
        (or pressed the escape or return keys). This mode of operation can cause problems,
        especially in plugins, so it is not recommended.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param button1Text  the text to show in the first button - if this string is
                            empty, the default string "OK" (or a localised version of it)
                            will be used.
        @param button2Text  the text to show in the second button - if this string is
                            empty, the default string "cancel" (or a localised version of it)
                            will be used.
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the menu will be launched asynchronously,
                            returning immediately, and the callback will receive a call to its
                            modalStateFinished() when the box is dismissed, with its parameter
                            being 1 if the ok button was pressed, or 0 for cancel. The callback object
                            will be owned and deleted by the system, so make sure that it works
                            safely and doesn't keep any references to objects that might be deleted
                            before it gets called.
        @returns true if button 1 was clicked, false if it was button 2. If the callback parameter
                 is not null, the method always returns false, and the user's choice is delivered
                 later by the callback.
    */
    static b8 DRX_CALLTYPE showOkCancelBox (MessageBoxIconType iconType,
                                               const Txt& title,
                                               const Txt& message,
                                              #if DRX_MODAL_LOOPS_PERMITTED
                                               const Txt& button1Text = Txt(),
                                               const Txt& button2Text = Txt(),
                                               Component* associatedComponent = nullptr,
                                               ModalComponentManager::Callback* callback = nullptr);
                                              #else
                                               const Txt& button1Text,
                                               const Txt& button2Text,
                                               Component* associatedComponent,
                                               ModalComponentManager::Callback* callback);
                                              #endif

    /** Shows a dialog box with three buttons.

        Ideal for yes/no/cancel boxes.

        The escape key can be used to trigger the third button.

        If DRX_MODAL_LOOPS_PERMITTED is not defined or the callback parameter is non-null,
        this function will return immediately. The object passed as the callback argument will
        receive the result of the alert window asynchronously.
        Otherwise, if DRX_MODAL_LOOPS_PERMITTED is defined and the callback parameter is null,
        the box is shown modally, and the method will block until the user has clicked the button
        (or pressed the escape or return keys). This mode of operation can cause problems,
        especially in plugins, so it is not recommended.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param button1Text  the text to show in the first button - if an empty string, then
                            "yes" will be used (or a localised version of it)
        @param button2Text  the text to show in the first button - if an empty string, then
                            "no" will be used (or a localised version of it)
        @param button3Text  the text to show in the first button - if an empty string, then
                            "cancel" will be used (or a localised version of it)
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the menu will be launched asynchronously,
                            returning immediately, and the callback will receive a call to its
                            modalStateFinished() when the box is dismissed, with its parameter
                            being 1 if the "yes" button was pressed, 2 for the "no" button, or 0
                            if it was cancelled. The callback object will be owned and deleted by the
                            system, so make sure that it works safely and doesn't keep any references
                            to objects that might be deleted before it gets called.

        @returns If the callback parameter has been set, this returns 0. Otherwise, it
                 returns one of the following values:
                 - 0 if the third button was pressed (normally used for 'cancel')
                 - 1 if the first button was pressed (normally used for 'yes')
                 - 2 if the middle button was pressed (normally used for 'no')
    */
    static i32 DRX_CALLTYPE showYesNoCancelBox (MessageBoxIconType iconType,
                                                 const Txt& title,
                                                 const Txt& message,
                                                #if DRX_MODAL_LOOPS_PERMITTED
                                                 const Txt& button1Text = Txt(),
                                                 const Txt& button2Text = Txt(),
                                                 const Txt& button3Text = Txt(),
                                                 Component* associatedComponent = nullptr,
                                                 ModalComponentManager::Callback* callback = nullptr);
                                                #else
                                                 const Txt& button1Text,
                                                 const Txt& button2Text,
                                                 const Txt& button3Text,
                                                 Component* associatedComponent,
                                                 ModalComponentManager::Callback* callback);
                                                #endif

    /** Shows an alert window using the specified options.

        The box will be displayed and placed into a modal state, but this method will return
        immediately, and the callback will be invoked later when the user dismisses the box.

        This function is always asynchronous, even if the callback is null.

        The result codes returned by the alert window are as follows.
        - One button:
            - button[0] returns 0
        - Two buttons:
            - button[0] returns 1
            - button[1] returns 0
        - Three buttons:
            - button[0] returns 1
            - button[1] returns 2
            - button[2] returns 0

        @param options   the options to use when creating the dialog.
        @param callback  if this is non-null, the callback will receive a call to its
                         modalStateFinished() when the box is dismissed with the index of the
                         button that was clicked as its argument.
                         The callback object will be owned and deleted by the system, so make sure
                         that it works safely and doesn't keep any references to objects that might
                         be deleted before it gets called.
        @returns         a ScopedMessageBox instance. The message box will remain visible for no
                         longer than the ScopedMessageBox remains alive.

        @see MessageBoxOptions
    */
    [[nodiscard]] static ScopedMessageBox showScopedAsync (const MessageBoxOptions& options,
                                                           std::function<z0 (i32)> callback);

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED && ! defined (DOXYGEN)
    /** Shows an operating-system native dialog box.

        @param title        the title to use at the top
        @param bodyText     the longer message to show
        @param isOkCancel   if true, this will show an ok/cancel box, if false,
                            it'll show a box with just an ok button
        @returns true if the ok button was pressed, false if they pressed cancel.
    */
    [[deprecated ("Use the NativeMessageBox methods instead for more options")]]
    static b8 DRX_CALLTYPE showNativeDialogBox (const Txt& title,
                                                   const Txt& bodyText,
                                                   b8 isOkCancel);
   #endif


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the alert box.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x1001800,  /**< The background colour for the window. */
        textColorId                = 0x1001810,  /**< The colour for the text. */
        outlineColorId             = 0x1001820   /**< An optional colour to use to draw a border around the window. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        alert-window drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual AlertWindow* createAlertWindow (const Txt& title, const Txt& message,
                                                const Txt& button1,
                                                const Txt& button2,
                                                const Txt& button3,
                                                MessageBoxIconType iconType,
                                                i32 numButtons,
                                                Component* associatedComponent) = 0;

        virtual z0 drawAlertBox (Graphics&, AlertWindow&, const Rectangle<i32>& textArea, TextLayout&) = 0;

        virtual i32 getAlertBoxWindowFlags() = 0;

        virtual Array<i32> getWidthsForTextButtons (AlertWindow&, const Array<TextButton*>&) = 0;
        virtual i32 getAlertWindowButtonHeight() = 0;

        virtual Font getAlertWindowTitleFont() = 0;
        virtual Font getAlertWindowMessageFont() = 0;
        virtual Font getAlertWindowFont() = 0;
    };

    //==============================================================================
    using AlertIconType = MessageBoxIconType;

    static constexpr auto NoIcon       = MessageBoxIconType::NoIcon;
    static constexpr auto QuestionIcon = MessageBoxIconType::QuestionIcon;
    static constexpr auto WarningIcon  = MessageBoxIconType::WarningIcon;
    static constexpr auto InfoIcon     = MessageBoxIconType::InfoIcon;

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 userTriedToCloseWindow() override;
    /** @internal */
    i32 getDesktopWindowStyleFlags() const override;
    /** @internal */
    f32 getDesktopScaleFactor() const override { return desktopScale * Desktop::getInstance().getGlobalScaleFactor(); }

private:
    //==============================================================================
    Txt text;
    TextLayout textLayout;
    Label accessibleMessageLabel;
    MessageBoxIconType alertIconType;
    ComponentBoundsConstrainer constrainer;
    ComponentDragger dragger;
    Rectangle<i32> textArea;
    OwnedArray<TextButton> buttons;
    OwnedArray<TextEditor> textBoxes;
    OwnedArray<ComboBox> comboBoxes;
    OwnedArray<ProgressBar> progressBars;
    Array<Component*> customComps;
    OwnedArray<Component> textBlocks;
    Array<Component*> allComps;
    StringArray textboxNames, comboBoxNames;
    Component* const associatedComponent;
    b8 escapeKeyCancels = true;
    f32 desktopScale = 1.0f;

    z0 exitAlert (Button* button);
    z0 updateLayout (b8 onlyIncreaseSize);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlertWindow)
};

} // namespace drx
