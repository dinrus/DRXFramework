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
    A dialog-box style window.

    This class is a convenient way of creating a DocumentWindow with a close button
    that can be triggered by pressing the escape key.

    Any of the methods available to a DocumentWindow or ResizableWindow are also
    available to this, so it can be made resizable, have a menu bar, etc.

    You can either override or use an instance of the DialogWindow class directly,
    or you can use a DialogWindow::LaunchOptions structure to quickly set up and
    launch a box containing a content component.

    If you use the class directly, you'll need to override the
    DocumentWindow::closeButtonPressed() method to handle the user clicking the close
    button - for more info, see the DocumentWindow help.

    @see DocumentWindow, ResizableWindow

    @tags{GUI}
*/
class DRX_API  DialogWindow   : public DocumentWindow
{
public:
    //==============================================================================
    /** Creates a DialogWindow.

        @param name                 the name to give the component - this is also
                                    the title shown at the top of the window. To change
                                    this later, use setName()
        @param backgroundColor     the colour to use for filling the window's background.
        @param escapeKeyTriggersCloseButton if true, then pressing the escape key will cause the
                                            close button to be triggered
        @param addToDesktop         if true, the window will be automatically added to the
                                    desktop; if false, you can use it as a child component
        @param desktopScale         specifies the scale to use when drawing the window. In a plugin,
                                    the host controls the scale used to render the plugin editor.
                                    You should query the editor scale with
                                    Component::getApproximateScaleFactorForComponent() and pass the
                                    result here. You can ignore this parameter in a standalone app
    */
    DialogWindow (const Txt& name,
                  Color backgroundColor,
                  b8 escapeKeyTriggersCloseButton,
                  b8 addToDesktop = true,
                  f32 desktopScale = 1.0f);

    /** Destructor.
        If a content component has been set with setContentOwned(), it will be deleted.
    */
    ~DialogWindow() override;

    //==============================================================================
    /** This class defines a collection of settings to be used to open a DialogWindow.

        The easiest way to open a DialogWindow is to create yourself a LaunchOptions structure,
        initialise its fields with the appropriate details, and then call its launchAsync()
        method to launch the dialog.
    */
    struct DRX_API  LaunchOptions
    {
        LaunchOptions() noexcept;

        /** The title to give the window. */
        Txt dialogTitle;

        /** The background colour for the window. */
        Color dialogBackgroundColor = Colors::lightgrey;

        /** The content component to show in the window. This must not be null!
            Using an OptionalScopedPointer to hold this pointer lets you indicate whether
            you'd like the dialog to automatically delete the component when the dialog
            has terminated.
        */
        OptionalScopedPointer<Component> content;

        /** If this is not a nullptr, it indicates a component that you'd like to position this
            dialog box in front of. See the DocumentWindow::centreAroundComponent() method for
            more info about this parameter.
        */
        Component* componentToCentreAround = nullptr;

        /** If true, then the escape key will trigger the dialog's close button. */
        b8 escapeKeyTriggersCloseButton = true;
        /** If true, the dialog will use a native title bar. See TopLevelWindow::setUsingNativeTitleBar() */
        b8 useNativeTitleBar = true;
        /** If true, the window will be resizable. See ResizableWindow::setResizable() */
        b8 resizable = true;
        /** Indicates whether to use a border or corner resizer component. See ResizableWindow::setResizable() */
        b8 useBottomRightCornerResizer = false;

        /** Launches a new modal dialog window.
            This will create a dialog based on the settings in this structure,
            launch it modally, and return immediately. The window that is returned
            will be automatically deleted when the modal state is terminated.

            When the dialog's close button is clicked, it'll automatically terminate its
            modal state, but you can also do this programmatically by calling
            exitModalState (returnValue) on the DialogWindow.

            If your content component needs to find the dialog window that it is
            contained in, a quick trick is to do this:
            @code
            if (DialogWindow* dw = contentComponent->findParentComponentOfClass<DialogWindow>())
                dw->exitModalState (1234);
            @endcode
        */
        DialogWindow* launchAsync();

        /** Creates a new DialogWindow instance with these settings.
            This method simply creates the window, it doesn't run it modally. In most cases
            you'll want to use launchAsync() or runModal() instead.
        */
        DialogWindow* create();

       #if DRX_MODAL_LOOPS_PERMITTED
        /** Launches and runs the dialog modally, returning the status code that was
            used to terminate the modal loop.

            Note that running modal loops inline is a BAD technique. If possible, always
            use launchAsync() instead of this method.
        */
        i32 runModal();
       #endif

        DRX_DECLARE_NON_COPYABLE (LaunchOptions)
    };

    //==============================================================================
    /** Easy way of quickly showing a dialog box containing a given component.

        Note: This method has been superseded by the DialogWindow::LaunchOptions structure,
        which does the same job with some extra flexibility. The showDialog method is here
        for backwards compatibility, but please use DialogWindow::LaunchOptions in new code.

        This will open and display a DialogWindow containing a given component, making it
        modal, but returning immediately to allow the dialog to finish in its own time. If
        you want to block and run a modal loop until the dialog is dismissed, use showModalDialog()
        instead.

        To close the dialog programmatically, you should call exitModalState (returnValue) on
        the DialogWindow that is created. To find a pointer to this window from your
        contentComponent, you can do something like this:
        @code
        if (DialogWindow* dw = contentComponent->findParentComponentOfClass<DialogWindow>())
            dw->exitModalState (1234);
        @endcode

        @param dialogTitle          the dialog box's title
        @param contentComponent     the content component for the dialog box. Make sure
                                    that this has been set to the size you want it to
                                    be before calling this method. The component won't
                                    be deleted by this call, so you can re-use it or delete
                                    it afterwards
        @param componentToCentreAround  if this is not a nullptr, it indicates a component that
                                    you'd like to show this dialog box in front of. See the
                                    DocumentWindow::centreAroundComponent() method for more
                                    info on this parameter
        @param backgroundColor     a colour to use for the dialog box's background colour
        @param escapeKeyTriggersCloseButton if true, then pressing the escape key will cause the
                                            close button to be triggered
        @param shouldBeResizable    if true, the dialog window has either a resizable border, or
                                    a corner resizer
        @param useBottomRightCornerResizer     if shouldBeResizable is true, this indicates whether
                                    to use a border or corner resizer component. See ResizableWindow::setResizable()
    */
    static z0 showDialog (const Txt& dialogTitle,
                            Component* contentComponent,
                            Component* componentToCentreAround,
                            Color backgroundColor,
                            b8 escapeKeyTriggersCloseButton,
                            b8 shouldBeResizable = false,
                            b8 useBottomRightCornerResizer = false);

   #if DRX_MODAL_LOOPS_PERMITTED
    /** Easy way of quickly showing a dialog box containing a given component.

        Note: This method has been superseded by the DialogWindow::LaunchOptions structure,
        which does the same job with some extra flexibility. The showDialog method is here
        for backwards compatibility, but please use DialogWindow::LaunchOptions in new code.

        This will open and display a DialogWindow containing a given component, returning
        when the user clicks its close button.

        It returns the value that was returned by the dialog box's runModalLoop() call.

        To close the dialog programmatically, you should call exitModalState (returnValue) on
        the DialogWindow that is created. To find a pointer to this window from your
        contentComponent, you can do something like this:
        @code
        if (DialogWindow* dw = contentComponent->findParentComponentOfClass<DialogWindow>())
            dw->exitModalState (1234);
        @endcode

        @param dialogTitle          the dialog box's title
        @param contentComponent     the content component for the dialog box. Make sure
                                    that this has been set to the size you want it to
                                    be before calling this method. The component won't
                                    be deleted by this call, so you can re-use it or delete
                                    it afterwards
        @param componentToCentreAround  if this is not a nullptr, it indicates a component that
                                    you'd like to show this dialog box in front of. See the
                                    DocumentWindow::centreAroundComponent() method for more
                                    info on this parameter
        @param backgroundColor     a colour to use for the dialog box's background colour
        @param escapeKeyTriggersCloseButton if true, then pressing the escape key will cause the
                                            close button to be triggered
        @param shouldBeResizable    if true, the dialog window has either a resizable border, or
                                    a corner resizer
        @param useBottomRightCornerResizer     if shouldBeResizable is true, this indicates whether
                                    to use a border or corner resizer component. See ResizableWindow::setResizable()
    */
    static i32 showModalDialog (const Txt& dialogTitle,
                                Component* contentComponent,
                                Component* componentToCentreAround,
                                Color backgroundColor,
                                b8 escapeKeyTriggersCloseButton,
                                b8 shouldBeResizable = false,
                                b8 useBottomRightCornerResizer = false);
   #endif


    /** Called when the escape key is pressed.
        This can be overridden to do things other than the default behaviour, which is to hide
        the window. Return true if the key has been used, or false if it was ignored.
    */
    virtual b8 escapeKeyPressed();

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** @internal */
    z0 resized() override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    f32 getDesktopScaleFactor() const override { return desktopScale * Desktop::getInstance().getGlobalScaleFactor(); }

private:
    f32 desktopScale = 1.0f;
    b8 escapeKeyTriggersCloseButton;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DialogWindow)
};

} // namespace drx
