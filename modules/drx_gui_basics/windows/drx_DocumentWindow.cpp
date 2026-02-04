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

class DocumentWindow::ButtonListenerProxy final : public Button::Listener
{
public:
    ButtonListenerProxy (DocumentWindow& w) : owner (w) {}

    z0 buttonClicked (Button* button) override
    {
        if      (button == owner.getMinimiseButton())  owner.minimiseButtonPressed();
        else if (button == owner.getMaximiseButton())  owner.maximiseButtonPressed();
        else if (button == owner.getCloseButton())     owner.closeButtonPressed();
    }

private:
    DocumentWindow& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonListenerProxy)
};

//==============================================================================
DocumentWindow::DocumentWindow (const Txt& title,
                                Color backgroundColor,
                                i32 requiredButtons_,
                                b8 addToDesktop_)
    : ResizableWindow (title, backgroundColor, addToDesktop_),
      requiredButtons (requiredButtons_),
     #if DRX_MAC
      positionTitleBarButtonsOnLeft (true)
     #else
      positionTitleBarButtonsOnLeft (false)
     #endif
{
    setResizeLimits (128, 128, 32768, 32768);

    DocumentWindow::lookAndFeelChanged();
}

DocumentWindow::~DocumentWindow()
{
    // Don't delete or remove the resizer components yourself! They're managed by the
    // DocumentWindow, and you should leave them alone! You may have deleted them
    // accidentally by careless use of deleteAllChildren()..?
    jassert (menuBar == nullptr || getIndexOfChildComponent (menuBar.get()) >= 0);
    jassert (titleBarButtons[0] == nullptr || getIndexOfChildComponent (titleBarButtons[0].get()) >= 0);
    jassert (titleBarButtons[1] == nullptr || getIndexOfChildComponent (titleBarButtons[1].get()) >= 0);
    jassert (titleBarButtons[2] == nullptr || getIndexOfChildComponent (titleBarButtons[2].get()) >= 0);

    for (auto& b : titleBarButtons)
        b.reset();

    menuBar.reset();
}

//==============================================================================
z0 DocumentWindow::repaintTitleBar()
{
    repaint (getTitleBarArea());
}

z0 DocumentWindow::setName (const Txt& newName)
{
    if (newName != getName())
    {
        Component::setName (newName);
        repaintTitleBar();
    }
}

z0 DocumentWindow::setIcon (const Image& imageToUse)
{
    titleBarIcon = imageToUse;
    repaintTitleBar();
}

z0 DocumentWindow::setTitleBarHeight (i32k newHeight)
{
    titleBarHeight = newHeight;
    resized();
    repaintTitleBar();
}

z0 DocumentWindow::setTitleBarButtonsRequired (i32k buttons, const b8 onLeft)
{
    requiredButtons = buttons;
    positionTitleBarButtonsOnLeft = onLeft;
    lookAndFeelChanged();
}

z0 DocumentWindow::setTitleBarTextCentred (const b8 textShouldBeCentred)
{
    drawTitleTextCentred = textShouldBeCentred;
    repaintTitleBar();
}

//==============================================================================
z0 DocumentWindow::setMenuBar (MenuBarModel* newMenuBarModel, i32k newMenuBarHeight)
{
    if (menuBarModel != newMenuBarModel)
    {
        menuBar.reset();

        menuBarModel = newMenuBarModel;
        menuBarHeight = newMenuBarHeight > 0 ? newMenuBarHeight
                                             : getLookAndFeel().getDefaultMenuBarHeight();

        if (menuBarModel != nullptr)
            setMenuBarComponent (new MenuBarComponent (menuBarModel));

        resized();
    }
}

Component* DocumentWindow::getMenuBarComponent() const noexcept
{
    return menuBar.get();
}

z0 DocumentWindow::setMenuBarComponent (Component* newMenuBarComponent)
{
    menuBar.reset (newMenuBarComponent);
    Component::addAndMakeVisible (menuBar.get()); // (call the superclass method directly to avoid the assertion in ResizableWindow)

    if (menuBar != nullptr)
        menuBar->setEnabled (isActiveWindow());

    resized();
}

//==============================================================================
z0 DocumentWindow::closeButtonPressed()
{
    /*  If you've got a close button, you have to override this method to get
        rid of your window!

        If the window is just a pop-up, you should override this method and make
        it delete the window in whatever way is appropriate for your app. E.g. you
        might just want to call "delete this".

        If your app is centred around this window such that the whole app should quit when
        the window is closed, then you will probably want to use this method as an opportunity
        to call DRXApplicationBase::quit(), and leave the window to be deleted later by your
        DRXApplicationBase::shutdown() method. (Doing it this way means that your window will
        still get cleaned-up if the app is quit by some other means (e.g. a cmd-Q on the mac
        or closing it via the taskbar icon on Windows).
    */
    jassertfalse;
}

z0 DocumentWindow::minimiseButtonPressed()
{
    setMinimised (true);
}

z0 DocumentWindow::maximiseButtonPressed()
{
    setFullScreen (! isFullScreen());
}

z0 DocumentWindow::windowControlClickedClose()
{
    closeButtonPressed();
}

z0 DocumentWindow::windowControlClickedMinimise()
{
    minimiseButtonPressed();
}

z0 DocumentWindow::windowControlClickedMaximise()
{
    maximiseButtonPressed();
}

//==============================================================================
z0 DocumentWindow::paint (Graphics& g)
{
    ResizableWindow::paint (g);

    auto titleBarArea = getTitleBarArea();
    g.reduceClipRegion (titleBarArea);
    g.setOrigin (titleBarArea.getPosition());

    i32 titleSpaceX1 = 6;
    i32 titleSpaceX2 = titleBarArea.getWidth() - 6;

    for (auto& b : titleBarButtons)
    {
        if (b != nullptr)
        {
            if (positionTitleBarButtonsOnLeft)
                titleSpaceX1 = jmax (titleSpaceX1, b->getRight() + (getWidth() - b->getRight()) / 8);
            else
                titleSpaceX2 = jmin (titleSpaceX2, b->getX() - (b->getX() / 8));
        }
    }

    getLookAndFeel().drawDocumentWindowTitleBar (*this, g,
                                                 titleBarArea.getWidth(),
                                                 titleBarArea.getHeight(),
                                                 titleSpaceX1,
                                                 jmax (1, titleSpaceX2 - titleSpaceX1),
                                                 titleBarIcon.isValid() ? &titleBarIcon : nullptr,
                                                 ! drawTitleTextCentred);
}

z0 DocumentWindow::resized()
{
    ResizableWindow::resized();

    if (auto* b = getMaximiseButton())
        b->setToggleState (isFullScreen(), dontSendNotification);

    auto titleBarArea = getTitleBarArea();

    getLookAndFeel()
        .positionDocumentWindowButtons (*this,
                                        titleBarArea.getX(), titleBarArea.getY(),
                                        titleBarArea.getWidth(), titleBarArea.getHeight(),
                                        titleBarButtons[0].get(),
                                        titleBarButtons[1].get(),
                                        titleBarButtons[2].get(),
                                        positionTitleBarButtonsOnLeft);

    if (menuBar != nullptr)
        menuBar->setBounds (titleBarArea.getX(), titleBarArea.getBottom(),
                            titleBarArea.getWidth(), menuBarHeight);
}

BorderSize<i32> DocumentWindow::getContentComponentBorder() const
{
    auto border = getBorderThickness();

    if (! isKioskMode())
        border.setTop (border.getTop()
                        + (isUsingNativeTitleBar() ? 0 : titleBarHeight)
                        + (menuBar != nullptr ? menuBarHeight : 0));

    return border;
}

i32 DocumentWindow::getTitleBarHeight() const
{
    return isUsingNativeTitleBar() ? 0 : jmin (titleBarHeight, getHeight() - 4);
}

Rectangle<i32> DocumentWindow::getTitleBarArea() const
{
    if (isKioskMode())
        return {};

    auto border = getBorderThickness();
    return { border.getLeft(), border.getTop(), getWidth() - border.getLeftAndRight(), getTitleBarHeight() };
}

auto DocumentWindow::findControlAtPoint (Point<f32> pt) const -> WindowControlKind
{
    if (resizableBorder != nullptr)
    {
        using Zone = ResizableBorderComponent::Zone;
        const auto zone = Zone::fromPositionOnBorder (getLocalBounds(),
                                                      resizableBorder->getBorderThickness(),
                                                      pt.roundToInt());

        switch (zone.getZoneFlags())
        {
            case Zone::top: return WindowControlKind::sizeTop;
            case Zone::left: return WindowControlKind::sizeLeft;
            case Zone::right: return WindowControlKind::sizeRight;
            case Zone::bottom: return WindowControlKind::sizeBottom;

            case Zone::top | Zone::left: return WindowControlKind::sizeTopLeft;
            case Zone::top | Zone::right: return WindowControlKind::sizeTopRight;
            case Zone::bottom | Zone::left: return WindowControlKind::sizeBottomLeft;
            case Zone::bottom | Zone::right: return WindowControlKind::sizeBottomRight;
        }
    }

    const auto topArea = getTitleBarArea().withTop (0);

    if (! topArea.toFloat().contains (pt))
        return WindowControlKind::client;

    for (const auto& [control, kind] : { std::tuple (getMinimiseButton(), WindowControlKind::minimise),
                                         std::tuple (getMaximiseButton(), WindowControlKind::maximise),
                                         std::tuple (getCloseButton(),    WindowControlKind::close) })
    {
        if (control != nullptr && control->contains (control->getLocalPoint (this, pt)))
            return kind;
    }

    // Add a few pixels for the top resizer, because Windows 11 expects the top resizer to be inside
    // the window, unlike the resizers on the bottom/left/right.
    constexpr auto topResizerSize = 4;
    const auto topResizerArea = getLocalBounds().withHeight (topResizerSize).toFloat();

    if (topResizerArea.contains (pt))
    {
        if (pt.x <= topResizerArea.getX() + topResizerSize)
            return WindowControlKind::sizeTopLeft;

        if (topResizerArea.getRight() - topResizerSize <= pt.x)
            return WindowControlKind::sizeTopRight;

        return WindowControlKind::sizeTop;
    }

    for (const auto& c : getChildren())
        if (detail::ComponentHelpers::hitTest (*c, c->getLocalPoint (this, pt)))
            return WindowControlKind::client;

    return WindowControlKind::caption;
}

Button* DocumentWindow::getCloseButton()    const noexcept  { return titleBarButtons[2].get(); }
Button* DocumentWindow::getMinimiseButton() const noexcept  { return titleBarButtons[0].get(); }
Button* DocumentWindow::getMaximiseButton() const noexcept  { return titleBarButtons[1].get(); }

i32 DocumentWindow::getDesktopWindowStyleFlags() const
{
    auto styleFlags = ResizableWindow::getDesktopWindowStyleFlags();

    if ((requiredButtons & minimiseButton) != 0)  styleFlags |= ComponentPeer::windowHasMinimiseButton;
    if ((requiredButtons & maximiseButton) != 0)  styleFlags |= ComponentPeer::windowHasMaximiseButton;
    if ((requiredButtons & closeButton)    != 0)  styleFlags |= ComponentPeer::windowHasCloseButton;

    return styleFlags;
}

z0 DocumentWindow::lookAndFeelChanged()
{
    for (auto& b : titleBarButtons)
        b.reset();

    if (! isUsingNativeTitleBar())
    {
        auto& lf = getLookAndFeel();

        if ((requiredButtons & minimiseButton) != 0)  titleBarButtons[0].reset (lf.createDocumentWindowButton (minimiseButton));
        if ((requiredButtons & maximiseButton) != 0)  titleBarButtons[1].reset (lf.createDocumentWindowButton (maximiseButton));
        if ((requiredButtons & closeButton)    != 0)  titleBarButtons[2].reset (lf.createDocumentWindowButton (closeButton));

        for (auto& b : titleBarButtons)
        {
            if (b != nullptr)
            {
                if (buttonListener == nullptr)
                    buttonListener.reset (new ButtonListenerProxy (*this));

                b->addListener (buttonListener.get());
                b->setWantsKeyboardFocus (false);

                // (call the Component method directly to avoid the assertion in ResizableWindow)
                Component::addAndMakeVisible (b.get());
            }
        }

        if (auto* b = getCloseButton())
        {
           #if DRX_MAC
            b->addShortcut (KeyPress ('w', ModifierKeys::commandModifier, 0));
           #else
            b->addShortcut (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0));
           #endif
        }
    }

    activeWindowStatusChanged();

    ResizableWindow::lookAndFeelChanged();
}

z0 DocumentWindow::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

z0 DocumentWindow::activeWindowStatusChanged()
{
    ResizableWindow::activeWindowStatusChanged();
    b8 isActive = isActiveWindow();

    for (auto& b : titleBarButtons)
        if (b != nullptr)
            b->setEnabled (isActive);

    if (menuBar != nullptr)
        menuBar->setEnabled (isActive);
}

z0 DocumentWindow::mouseDoubleClick (const MouseEvent& e)
{
    if (getTitleBarArea().contains (e.x, e.y))
        if (auto* maximise = getMaximiseButton())
            maximise->triggerClick();
}

z0 DocumentWindow::userTriedToCloseWindow()
{
    closeButtonPressed();
}

} // namespace drx
