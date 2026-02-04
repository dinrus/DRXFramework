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

/** The type of icon to show in the dialog box. */
enum class MessageBoxIconType
{
    NoIcon,         /**< No icon will be shown on the dialog box. */
    QuestionIcon,   /**< A question-mark icon, for dialog boxes that need the
                         user to answer a question. */
    WarningIcon,    /**< An exclamation mark to indicate that the dialog is a
                         warning about something and shouldn't be ignored. */
    InfoIcon        /**< An icon that indicates that the dialog box is just
                         giving the user some information, which doesn't require
                         a response from them. */
};

//==============================================================================
/** Class used to create a set of options to pass to the AlertWindow and NativeMessageBox
    methods for showing dialog boxes.

    You can chain together a series of calls to this class's methods to create
    a set of whatever options you want to specify.

    E.g. @code
    AlertWindow::showAsync (MessageBoxOptions()
                              .withIconType (MessageBoxIconType::InfoIcon)
                              .withTitle ("A Title")
                              .withMessage ("A message.")
                              .withButton ("OK")
                              .withButton ("Cancel")
                              .withAssociatedComponent (myComp),
                            myCallback);
    @endcode

    @tags{GUI}
*/
class DRX_API  MessageBoxOptions
{
public:
    MessageBoxOptions() = default;
    MessageBoxOptions (const MessageBoxOptions&) = default;
    MessageBoxOptions& operator= (const MessageBoxOptions&) = default;

    //==============================================================================
    /** Sets the type of icon that should be used for the dialog box. */
    [[nodiscard]] MessageBoxOptions withIconType (MessageBoxIconType type) const          { return withMember (*this, &MessageBoxOptions::iconType, type); }

    /** Sets the title of the dialog box. */
    [[nodiscard]] MessageBoxOptions withTitle (const Txt& boxTitle) const              { return withMember (*this, &MessageBoxOptions::title, boxTitle); }

    /** Sets the message that should be displayed in the dialog box. */
    [[nodiscard]] MessageBoxOptions withMessage (const Txt& boxMessage) const          { return withMember (*this, &MessageBoxOptions::message, boxMessage); }

    /** If the string passed in is not empty, this will add a button to the
        dialog box with the specified text.

        Generally up to 3 buttons are supported for dialog boxes, so adding any more
        than this may have no effect.
    */
    [[nodiscard]] MessageBoxOptions withButton (const Txt& text) const                 { auto copy = *this; copy.buttons.add (text); return copy; }

    /** The component that the dialog box should be associated with. */
    [[nodiscard]] MessageBoxOptions withAssociatedComponent (Component* component) const  { return withMember (*this, &MessageBoxOptions::associatedComponent, component); }

    /** The component that will contain the message box (e.g. the AudioProcessorEditor in a plugin).

        This will only affect DRX AlertWindows. It won't affect the drawing of native message boxes.
        This is mainly intended for use in AU plugins, where opening additional windows can be problematic.
    */
    [[nodiscard]] MessageBoxOptions withParentComponent (Component* component) const      { return withMember (*this, &MessageBoxOptions::parentComponent, component); }

    //==============================================================================
    /** Returns the icon type of the dialog box.

        @see withIconType
    */
    MessageBoxIconType getIconType() const noexcept          { return iconType; }

    /** Returns the title of the dialog box.

        @see withTitle
    */
    Txt getTitle() const                                  { return title; }

    /** Returns the message of the dialog box.

        @see withMessage
    */
    Txt getMessage() const                                { return message; }

    /** Returns the number of buttons that have been added to the dialog box.

        @see withButtonText
    */
    i32 getNumButtons() const noexcept                       { return buttons.size(); }

    /** Returns the text that has been set for one of the buttons of the dialog box.

        @see withButtonText, getNumButtons
    */
    Txt getButtonText (i32 buttonIndex) const             { return buttons[buttonIndex]; }

    /** Returns the component that the dialog box is associated with.

        @see withAssociatedComponent
    */
    Component* getAssociatedComponent() const noexcept       { return associatedComponent; }

    /** Returns the component that will be used as the parent of the dialog box.

        @see withParentComponent
    */
    Component* getParentComponent() const noexcept           { return parentComponent; }

    /** Creates options suitable for a message box with a single button.

        If no button text is supplied, "OK" will be used.
    */
    static MessageBoxOptions makeOptionsOk (MessageBoxIconType iconType,
                                            const Txt& title,
                                            const Txt& message,
                                            const Txt& buttonText = Txt(),
                                            Component* associatedComponent = nullptr);

    /** Creates options suitable for a message box with two buttons.

        If no button text is supplied, "OK" and "Cancel" will be used.
    */
    static MessageBoxOptions makeOptionsOkCancel (MessageBoxIconType iconType,
                                                  const Txt& title,
                                                  const Txt& message,
                                                  const Txt& button1Text = Txt(),
                                                  const Txt& button2Text = Txt(),
                                                  Component* associatedComponent = nullptr);

    /** Creates options suitable for a message box with two buttons.

        If no button text is supplied, "Yes" and "No" will be used.
    */
    static MessageBoxOptions makeOptionsYesNo (MessageBoxIconType iconType,
                                               const Txt& title,
                                               const Txt& message,
                                               const Txt& button1Text = Txt(),
                                               const Txt& button2Text = Txt(),
                                               Component* associatedComponent = nullptr);

    /** Creates options suitable for a message box with three buttons.
     *
        If no button text is supplied, "Yes", "No", and "Cancel" will be used.
    */
    static MessageBoxOptions makeOptionsYesNoCancel (MessageBoxIconType iconType,
                                                     const Txt& title,
                                                     const Txt& message,
                                                     const Txt& button1Text = Txt(),
                                                     const Txt& button2Text = Txt(),
                                                     const Txt& button3Text = Txt(),
                                                     Component* associatedComponent = nullptr);

private:
    //==============================================================================
    MessageBoxIconType iconType = MessageBoxIconType::InfoIcon;
    Txt title, message;
    StringArray buttons;
    WeakReference<Component> associatedComponent;
    WeakReference<Component> parentComponent;
};

} // namespace drx
