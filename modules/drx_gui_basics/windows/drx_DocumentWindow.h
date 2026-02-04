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
    A resizable window with a title bar and maximise, minimise and close buttons.

    This subclass of ResizableWindow creates a fairly standard type of window with
    a title bar and various buttons. The name of the component is shown in the
    title bar, and an icon can optionally be specified with setIcon().

    All the methods available to a ResizableWindow are also available to this,
    so it can easily be made resizable, minimised, maximised, etc.

    It's not advisable to add child components directly to a DocumentWindow: put them
    inside your content component instead. And overriding methods like resized(), moved(), etc
    is also not recommended - instead override these methods for your content component.
    (If for some obscure reason you do need to override these methods, always remember to
    call the super-class's resized() method too, otherwise it'll fail to lay out the window
    decorations correctly).

    You can also automatically add a menu bar to the window, using the setMenuBar()
    method.

    @see ResizableWindow, DialogWindow

    @tags{GUI}
*/
class DRX_API  DocumentWindow   : public ResizableWindow
{
public:
    //==============================================================================
    /** The set of available button-types that can be put on the title bar.

        @see setTitleBarButtonsRequired
    */
    enum TitleBarButtons
    {
        minimiseButton = 1,
        maximiseButton = 2,
        closeButton = 4,

        /** A combination of all the buttons above. */
        allButtons = 7
    };

    //==============================================================================
    /** Creates a DocumentWindow.

        @param name             the name to give the component - this is also
                                the title shown at the top of the window. To change
                                this later, use setName()
        @param backgroundColor the colour to use for filling the window's background.
        @param requiredButtons  specifies which of the buttons (close, minimise, maximise)
                                should be shown on the title bar. This value is a bitwise
                                combination of values from the TitleBarButtons enum. Note
                                that it can be "allButtons" to get them all. You
                                can change this later with the setTitleBarButtonsRequired()
                                method, which can also specify where they are positioned.
                                The behaviour of native titlebars on macOS is slightly different:
                                the maximiseButton flag controls whether or not the window can enter
                                native fullscreen mode, and the zoom button can be disabled by
                                making the window non-resizable.
        @param addToDesktop     if true, the window will be automatically added to the
                                desktop; if false, you can use it as a child component
        @see TitleBarButtons
    */
    DocumentWindow (const Txt& name,
                    Color backgroundColor,
                    i32 requiredButtons,
                    b8 addToDesktop = true);

    /** Destructor.
        If a content component has been set with setContentOwned(), it will be deleted.
    */
    ~DocumentWindow() override;

    //==============================================================================
    /** Changes the component's name.

        (This is overridden from Component::setName() to cause a repaint, as
        the name is what gets drawn across the window's title bar).
    */
    z0 setName (const Txt& newName) override;

    /** Sets an icon to show in the title bar, next to the title.

        A copy is made internally of the image, so the caller can delete the
        image after calling this. If an empty Image is passed-in, any existing icon
        will be removed.
    */
    z0 setIcon (const Image& imageToUse);

    /** Changes the height of the title-bar. */
    z0 setTitleBarHeight (i32 newHeight);

    /** Returns the current title bar height. */
    i32 getTitleBarHeight() const;

    /** Changes the set of title-bar buttons being shown.

        @param requiredButtons  specifies which of the buttons (close, minimise, maximise)
                                should be shown on the title bar. This value is a bitwise
                                combination of values from the TitleBarButtons enum. Note
                                that it can be "allButtons" to get them all.
                                The behaviour of native titlebars on macOS is slightly different:
                                the maximiseButton flag controls whether or not the window can enter
                                native fullscreen mode, and the zoom button can be disabled by
                                making the window non-resizable.
        @param positionTitleBarButtonsOnLeft    if true, the buttons should go at the
                                left side of the bar; if false, they'll be placed at the right
    */
    z0 setTitleBarButtonsRequired (i32 requiredButtons,
                                     b8 positionTitleBarButtonsOnLeft);

    /** Sets whether the title should be centred within the window.

        If true, the title text is shown in the middle of the title-bar; if false,
        it'll be shown at the left of the bar.
    */
    z0 setTitleBarTextCentred (b8 textShouldBeCentred);

    //==============================================================================
    /** Creates a menu inside this window.

        @param menuBarModel     this specifies a MenuBarModel that should be used to
                                generate the contents of a menu bar that will be placed
                                just below the title bar, and just above any content
                                component. If this value is a nullptr, any existing menu bar
                                will be removed from the component; if it is not a nullptr,
                                one will be added if it's required.
        @param menuBarHeight    the height of the menu bar component, if one is needed. Pass a value of zero
                                or less to use the look-and-feel's default size.
    */
    z0 setMenuBar (MenuBarModel* menuBarModel,
                     i32 menuBarHeight = 0);

    /** Returns the current menu bar component, or null if there isn't one.
        This is probably a MenuBarComponent, unless a custom one has been set using
        setMenuBarComponent().
    */
    Component* getMenuBarComponent() const noexcept;

    /** Replaces the current menu bar with a custom component.
        The component will be owned and deleted by the document window.
    */
    z0 setMenuBarComponent (Component* newMenuBarComponent);

    //==============================================================================
    /** This method is called when the user tries to close the window.

        This is triggered by the user clicking the close button, or using some other
        OS-specific key shortcut or OS menu for getting rid of a window.

        If the window is just a pop-up, you should override this closeButtonPressed()
        method and make it delete the window in whatever way is appropriate for your
        app. E.g. you might just want to call "delete this".

        If your app is centred around this window such that the whole app should quit when
        the window is closed, then you will probably want to use this method as an opportunity
        to call DRXApplicationBase::quit(), and leave the window to be deleted later by your
        DRXApplicationBase::shutdown() method. (Doing it this way means that your window will
        still get cleaned-up if the app is quit by some other means (e.g. a cmd-Q on the mac
        or closing it via the taskbar icon on Windows).

        (Note that the DocumentWindow class overrides Component::userTriedToCloseWindow() and
        redirects it to call this method, so any methods of closing the window that are
        caught by userTriedToCloseWindow() will also end up here).
    */
    virtual z0 closeButtonPressed();

    /** Callback that is triggered when the minimise button is pressed.

        This function is only called when using a non-native titlebar.

        The default implementation of this calls ResizableWindow::setMinimised(), but
        you can override it to do more customised behaviour.
    */
    virtual z0 minimiseButtonPressed();

    /** Callback that is triggered when the maximise button is pressed, or when the
        title-bar is f64-clicked.

        This function is only called when using a non-native titlebar.

        The default implementation of this calls ResizableWindow::setFullScreen(), but
        you can override it to do more customised behaviour.
    */
    virtual z0 maximiseButtonPressed();

    //==============================================================================
    /** Returns the close button, (or nullptr if there isn't one). */
    Button* getCloseButton() const noexcept;

    /** Returns the minimise button, (or nullptr if there isn't one). */
    Button* getMinimiseButton() const noexcept;

    /** Returns the maximise button, (or nullptr if there isn't one). */
    Button* getMaximiseButton() const noexcept;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the window.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        textColorId                = 0x1005701,  /**< The colour to draw any text with. It's up to the look
                                                       and feel class how this is used. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        window drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawDocumentWindowTitleBar (DocumentWindow&,
                                                 Graphics&, i32 w, i32 h,
                                                 i32 titleSpaceX, i32 titleSpaceW,
                                                 const Image* icon,
                                                 b8 drawTitleTextOnLeft) = 0;

        virtual Button* createDocumentWindowButton (i32 buttonType) = 0;

        virtual z0 positionDocumentWindowButtons (DocumentWindow&,
                                                    i32 titleBarX, i32 titleBarY, i32 titleBarW, i32 titleBarH,
                                                    Button* minimiseButton,
                                                    Button* maximiseButton,
                                                    Button* closeButton,
                                                    b8 positionTitleBarButtonsOnLeft) = 0;
    };

    //==============================================================================
   #ifndef DOXYGEN
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    BorderSize<i32> getContentComponentBorder() const override;
    /** @internal */
    z0 mouseDoubleClick (const MouseEvent&) override;
    /** @internal */
    z0 userTriedToCloseWindow() override;
    /** @internal */
    z0 activeWindowStatusChanged() override;
    /** @internal */
    i32 getDesktopWindowStyleFlags() const override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    Rectangle<i32> getTitleBarArea() const;
    /** @internal */
    WindowControlKind findControlAtPoint (Point<f32>) const override;
    /** @internal */
    z0 windowControlClickedClose() override;
    /** @internal */
    z0 windowControlClickedMinimise() override;
    /** @internal */
    z0 windowControlClickedMaximise() override;
   #endif

private:
    //==============================================================================
    i32 titleBarHeight = 26, menuBarHeight = 24, requiredButtons;
    b8 positionTitleBarButtonsOnLeft, drawTitleTextCentred = true;
    std::unique_ptr<Button> titleBarButtons [3];
    Image titleBarIcon;
    std::unique_ptr<Component> menuBar;
    MenuBarModel* menuBarModel = nullptr;

    class ButtonListenerProxy;
    std::unique_ptr<ButtonListenerProxy> buttonListener;

    z0 repaintTitleBar();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentWindow)
};

} // namespace drx
