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

ResizableWindow::ResizableWindow (const Txt& name, b8 shouldAddToDesktop)
    : TopLevelWindow (name, shouldAddToDesktop)
{
    initialise (shouldAddToDesktop);
}

ResizableWindow::ResizableWindow (const Txt& name, Color bkgnd, b8 shouldAddToDesktop)
    : TopLevelWindow (name, shouldAddToDesktop)
{
    setBackgroundColor (bkgnd);
    initialise (shouldAddToDesktop);
}

ResizableWindow::~ResizableWindow()
{
    // Don't delete or remove the resizer components yourself! They're managed by the
    // ResizableWindow, and you should leave them alone! You may have deleted them
    // accidentally by careless use of deleteAllChildren()..?
    jassert (resizableCorner == nullptr || getIndexOfChildComponent (resizableCorner.get()) >= 0);
    jassert (resizableBorder == nullptr || getIndexOfChildComponent (resizableBorder.get()) >= 0);

    resizableCorner.reset();
    resizableBorder.reset();
    clearContentComponent();

    // have you been adding your own components directly to this window..? tut tut tut.
    // Read the instructions for using a ResizableWindow!
    jassert (getNumChildComponents() == 0);
}

z0 ResizableWindow::initialise (const b8 shouldAddToDesktop)
{
    defaultConstrainer.setMinimumOnscreenAmounts (0x10000, 16, 24, 16);

    lastNonFullScreenPos.setBounds (50, 50, 256, 256);

    if (shouldAddToDesktop)
        addToDesktop();
}

i32 ResizableWindow::getDesktopWindowStyleFlags() const
{
    i32 styleFlags = TopLevelWindow::getDesktopWindowStyleFlags();

    if (isResizable() && Desktop::getInstance().supportsBorderlessNonClientResize())
        styleFlags |= ComponentPeer::windowIsResizable;

    return styleFlags;
}

//==============================================================================
z0 ResizableWindow::clearContentComponent()
{
    if (ownsContentComponent)
    {
        contentComponent.deleteAndZero();
    }
    else
    {
        removeChildComponent (contentComponent);
        contentComponent = nullptr;
    }
}

z0 ResizableWindow::setContent (Component* newContentComponent,
                                  b8 takeOwnership,
                                  b8 resizeToFitWhenContentChangesSize)
{
    if (newContentComponent != contentComponent)
    {
        clearContentComponent();

        contentComponent = newContentComponent;
        Component::addAndMakeVisible (contentComponent);
    }

    ownsContentComponent = takeOwnership;
    resizeToFitContent = resizeToFitWhenContentChangesSize;

    if (resizeToFitWhenContentChangesSize)
        childBoundsChanged (contentComponent);

    resized(); // must always be called to position the new content comp
}

z0 ResizableWindow::setContentOwned (Component* newContentComponent, const b8 resizeToFitWhenContentChangesSize)
{
    setContent (newContentComponent, true, resizeToFitWhenContentChangesSize);
}

z0 ResizableWindow::setContentNonOwned (Component* newContentComponent, const b8 resizeToFitWhenContentChangesSize)
{
    setContent (newContentComponent, false, resizeToFitWhenContentChangesSize);
}

z0 ResizableWindow::setContentComponent (Component* const newContentComponent,
                                           const b8 deleteOldOne,
                                           const b8 resizeToFitWhenContentChangesSize)
{
    if (newContentComponent != contentComponent)
    {
        if (deleteOldOne)
        {
            contentComponent.deleteAndZero();
        }
        else
        {
            removeChildComponent (contentComponent);
            contentComponent = nullptr;
        }
    }

    setContent (newContentComponent, true, resizeToFitWhenContentChangesSize);
}

z0 ResizableWindow::setContentComponentSize (i32 width, i32 height)
{
    jassert (width > 0 && height > 0); // not a great idea to give it a zero size..

    auto border = getContentComponentBorder();

    setSize (width + border.getLeftAndRight(),
             height + border.getTopAndBottom());
}

BorderSize<i32> ResizableWindow::getBorderThickness() const
{
    if (isUsingNativeTitleBar() || isKioskMode())
        return {};

    return BorderSize<i32> ((resizableBorder != nullptr && ! isFullScreen()) ? 4 : 1);
}

BorderSize<i32> ResizableWindow::getContentComponentBorder() const
{
    return getBorderThickness();
}

z0 ResizableWindow::moved()
{
    updateLastPosIfShowing();
}

z0 ResizableWindow::visibilityChanged()
{
    TopLevelWindow::visibilityChanged();
    updateLastPosIfShowing();
}

z0 ResizableWindow::resized()
{
    const b8 resizerHidden = isFullScreen() || isKioskMode() || isUsingNativeTitleBar();

    if (resizableBorder != nullptr)
    {
        resizableBorder->setVisible (! resizerHidden);
        resizableBorder->setBorderThickness (getBorderThickness());
        resizableBorder->setSize (getWidth(), getHeight());
        resizableBorder->toBack();
    }

    if (resizableCorner != nullptr)
    {
        resizableCorner->setVisible (! resizerHidden);

        i32k resizerSize = 18;
        resizableCorner->setBounds (getWidth() - resizerSize,
                                    getHeight() - resizerSize,
                                    resizerSize, resizerSize);
    }

    if (contentComponent != nullptr)
    {
        // The window expects to be able to be able to manage the size and position
        // of its content component, so you can't arbitrarily add a transform to it!
        jassert (! contentComponent->isTransformed());

        contentComponent->setBoundsInset (getContentComponentBorder());
    }

    updateLastPosIfShowing();

   #if DRX_DEBUG
    hasBeenResized = true;
   #endif
}

z0 ResizableWindow::childBoundsChanged (Component* child)
{
    if ((child == contentComponent) && (child != nullptr) && resizeToFitContent)
    {
        auto borders = getContentComponentBorder();

        setSize (child->getWidth() + borders.getLeftAndRight(),
                 child->getHeight() + borders.getTopAndBottom());
    }
}


//==============================================================================
z0 ResizableWindow::activeWindowStatusChanged()
{
    auto border = getContentComponentBorder();
    auto area = getLocalBounds();

    repaint (area.removeFromTop (border.getTop()));
    repaint (area.removeFromLeft (border.getLeft()));
    repaint (area.removeFromRight (border.getRight()));
    repaint (area.removeFromBottom (border.getBottom()));
}

//==============================================================================
z0 ResizableWindow::setResizable (const b8 shouldBeResizable,
                                    const b8 useBottomRightCornerResizer)
{
    resizable = shouldBeResizable;

    if (shouldBeResizable)
    {
        if (useBottomRightCornerResizer)
        {
            resizableBorder.reset();

            if (resizableCorner == nullptr)
            {
                resizableCorner.reset (new ResizableCornerComponent (this, constrainer));
                Component::addChildComponent (resizableCorner.get());
                resizableCorner->setAlwaysOnTop (true);
            }
        }
        else
        {
            resizableCorner.reset();

            if (resizableBorder == nullptr && (! isOnDesktop() || ! Desktop::getInstance().supportsBorderlessNonClientResize()))
            {
                resizableBorder.reset (new ResizableBorderComponent (this, constrainer));
                Component::addChildComponent (resizableBorder.get());
            }
        }
    }
    else
    {
        resizableCorner.reset();
        resizableBorder.reset();
    }

    if (isOnDesktop())
        recreateDesktopWindow();

    childBoundsChanged (contentComponent);
    resized();
}

b8 ResizableWindow::isResizable() const noexcept
{
    return resizable;
}

z0 ResizableWindow::setResizeLimits (i32 newMinimumWidth,
                                       i32 newMinimumHeight,
                                       i32 newMaximumWidth,
                                       i32 newMaximumHeight) noexcept
{
    // if you've set up a custom constrainer then these settings won't have any effect..
    jassert (constrainer == &defaultConstrainer || constrainer == nullptr);

    if (constrainer == nullptr)
        setConstrainer (&defaultConstrainer);

    defaultConstrainer.setSizeLimits (newMinimumWidth, newMinimumHeight,
                                      newMaximumWidth, newMaximumHeight);

    setBoundsConstrained (getBounds());
}

z0 ResizableWindow::setDraggable (b8 shouldBeDraggable) noexcept
{
    canDrag = shouldBeDraggable;
}

z0 ResizableWindow::setConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        constrainer = newConstrainer;

        const b8 useBottomRightCornerResizer = resizableCorner != nullptr;

        resizableCorner.reset();
        resizableBorder.reset();

        setResizable (isResizable(), useBottomRightCornerResizer);
        updatePeerConstrainer();
    }
}

z0 ResizableWindow::setBoundsConstrained (const Rectangle<i32>& newBounds)
{
    if (constrainer != nullptr)
        constrainer->setBoundsForComponent (this, newBounds, false, false, false, false);
    else
        setBounds (newBounds);
}

//==============================================================================
z0 ResizableWindow::paint (Graphics& g)
{
    auto& lf = getLookAndFeel();

    lf.fillResizableWindowBackground (g, getWidth(), getHeight(),
                                      getBorderThickness(), *this);

    if (! isFullScreen())
        lf.drawResizableWindowBorder (g, getWidth(), getHeight(),
                                      getBorderThickness(), *this);

   #if DRX_DEBUG
    /* If this fails, then you've probably written a subclass with a resized()
       callback but forgotten to make it call its parent class's resized() method.

       It's important when you override methods like resized(), moved(),
       etc., that you make sure the base class methods also get called.

       Of course you shouldn't really be overriding ResizableWindow::resized() anyway,
       because your content should all be inside the content component - and it's the
       content component's resized() method that you should be using to do your
       layout.
    */
    jassert (hasBeenResized || (getWidth() == 0 && getHeight() == 0));
   #endif
}

z0 ResizableWindow::lookAndFeelChanged()
{
    resized();

    if (isOnDesktop())
    {
        Component::addToDesktop (getDesktopWindowStyleFlags());
        updatePeerConstrainer();
    }
}

Color ResizableWindow::getBackgroundColor() const noexcept
{
    return findColor (backgroundColorId, false);
}

z0 ResizableWindow::setBackgroundColor (Color newColor)
{
    auto backgroundColor = newColor;

    if (! Desktop::canUseSemiTransparentWindows())
        backgroundColor = newColor.withAlpha (1.0f);

    setColor (backgroundColorId, backgroundColor);
    setOpaque (backgroundColor.isOpaque());
    repaint();
}

//==============================================================================
b8 ResizableWindow::isFullScreen() const
{
    if (isOnDesktop())
    {
        auto* peer = getPeer();
        return peer != nullptr && peer->isFullScreen();
    }

    return fullscreen;
}

z0 ResizableWindow::setFullScreen (const b8 shouldBeFullScreen)
{
    if (shouldBeFullScreen != isFullScreen())
    {
        updateLastPosIfShowing();
        fullscreen = shouldBeFullScreen;

        if (isOnDesktop())
        {
            if (auto* peer = getPeer())
            {
                // keep a copy of this intact in case the real one gets messed-up while we're un-maximising
                auto lastPos = lastNonFullScreenPos;

                peer->setFullScreen (shouldBeFullScreen);

                if ((! shouldBeFullScreen) && ! lastPos.isEmpty())
                    setBounds (lastPos);
            }
            else
            {
                jassertfalse;
            }
        }
        else
        {
            if (shouldBeFullScreen)
                setBounds (0, 0, getParentWidth(), getParentHeight());
            else
                setBounds (lastNonFullScreenPos);
        }

        resized();
    }
}

b8 ResizableWindow::isMinimised() const
{
    if (auto* peer = getPeer())
        return peer->isMinimised();

    return false;
}

z0 ResizableWindow::setMinimised (const b8 shouldMinimise)
{
    if (shouldMinimise != isMinimised())
    {
        if (auto* peer = getPeer())
        {
            updateLastPosIfShowing();
            peer->setMinimised (shouldMinimise);
        }
        else
        {
            jassertfalse;
        }
    }
}

b8 ResizableWindow::isKioskMode() const
{
    if (isOnDesktop())
        if (auto* peer = getPeer())
            return peer->isKioskMode();

    return Desktop::getInstance().getKioskModeComponent() == this;
}

z0 ResizableWindow::updateLastPosIfShowing()
{
    if (isShowing())
    {
        updateLastPosIfNotFullScreen();
        updatePeerConstrainer();
    }
}

z0 ResizableWindow::updateLastPosIfNotFullScreen()
{
    if (! (isFullScreen() || isMinimised() || isKioskMode()))
        lastNonFullScreenPos = getBounds();
}

z0 ResizableWindow::updatePeerConstrainer()
{
    if (isOnDesktop())
        if (auto* peer = getPeer())
            peer->setConstrainer (constrainer);
}

z0 ResizableWindow::parentSizeChanged()
{
    if (isFullScreen() && getParentComponent() != nullptr)
        setBounds (getParentComponent()->getLocalBounds());
}

//==============================================================================
Txt ResizableWindow::getWindowStateAsString()
{
    updateLastPosIfShowing();
    auto stateString = (isFullScreen() && ! isKioskMode() ? "fs " : "") + lastNonFullScreenPos.toString();

   #if DRX_LINUX
    if (auto* peer = isOnDesktop() ? getPeer() : nullptr)
    {
        if (const auto optionalFrameSize = peer->getFrameSizeIfPresent())
        {
            const auto& frameSize = *optionalFrameSize;
            stateString << " frame " << frameSize.getTop() << ' ' << frameSize.getLeft()
                        << ' ' << frameSize.getBottom() << ' ' << frameSize.getRight();
        }
    }
   #endif

    return stateString;
}

b8 ResizableWindow::restoreWindowStateFromString (const Txt& s)
{
    StringArray tokens;
    tokens.addTokens (s, false);
    tokens.removeEmptyStrings();
    tokens.trim();

    const b8 fs = tokens[0].startsWithIgnoreCase ("fs");
    i32k firstCoord = fs ? 1 : 0;

    if (tokens.size() < firstCoord + 4)
        return false;

    Rectangle<i32> newPos (tokens[firstCoord].getIntValue(),
                           tokens[firstCoord + 1].getIntValue(),
                           tokens[firstCoord + 2].getIntValue(),
                           tokens[firstCoord + 3].getIntValue());

    if (newPos.isEmpty())
        return false;

    auto* peer = isOnDesktop() ? getPeer() : nullptr;

    if (peer != nullptr)
    {
        if (const auto frameSize = peer->getFrameSizeIfPresent())
            frameSize->addTo (newPos);
    }

   #if DRX_LINUX
    if (peer == nullptr || ! peer->getFrameSizeIfPresent())
    {
        // We need to adjust for the frame size before we create a peer, as X11
        // doesn't provide this information at construction time.
        if (tokens[firstCoord + 4] == "frame" && tokens.size() == firstCoord + 9)
        {
            BorderSize<i32> frame { tokens[firstCoord + 5].getIntValue(),
                                    tokens[firstCoord + 6].getIntValue(),
                                    tokens[firstCoord + 7].getIntValue(),
                                    tokens[firstCoord + 8].getIntValue() };

            newPos.setX (newPos.getX() - frame.getLeft());
            newPos.setY (newPos.getY() - frame.getTop());

            setBounds (newPos);
        }
    }
   #endif

    {
        auto& desktop = Desktop::getInstance();
        auto allMonitors = desktop.getDisplays().getRectangleList (true);
        allMonitors.clipTo (newPos);
        auto onScreenArea = allMonitors.getBounds();

        if (onScreenArea.getWidth() * onScreenArea.getHeight() < 32 * 32)
        {
            auto screen = desktop.getDisplays().getDisplayForRect (newPos)->userArea;

            newPos.setSize (jmin (newPos.getWidth(),  screen.getWidth()),
                            jmin (newPos.getHeight(), screen.getHeight()));

            newPos.setPosition (jlimit (screen.getX(), screen.getRight()  - newPos.getWidth(),  newPos.getX()),
                                jlimit (screen.getY(), screen.getBottom() - newPos.getHeight(), newPos.getY()));
        }
    }

    if (peer != nullptr)
    {
        if (const auto frameSize = peer->getFrameSizeIfPresent())
            frameSize->subtractFrom (newPos);

        peer->setNonFullScreenBounds (newPos);
    }

    updateLastPosIfNotFullScreen();

    if (fs)
        setBoundsConstrained (newPos);

    setFullScreen (fs);

    if (! fs)
        setBoundsConstrained (newPos);

    return true;
}

//==============================================================================
z0 ResizableWindow::mouseDown (const MouseEvent& e)
{
    if (canDrag && ! isFullScreen())
    {
        dragStarted = true;
        dragger.startDraggingComponent (this, e);
    }
}

z0 ResizableWindow::mouseDrag (const MouseEvent& e)
{
    if (dragStarted)
        dragger.dragComponent (this, e, constrainer);
}

z0 ResizableWindow::mouseUp (const MouseEvent&)
{
    dragStarted = false;
}

//==============================================================================
#if DRX_DEBUG
z0 ResizableWindow::addChildComponent (Component* const child, i32 zOrder)
{
    /* Agh! You shouldn't add components directly to a ResizableWindow - this class
       manages its child components automatically, and if you add your own it'll cause
       trouble. Instead, use setContentComponent() to give it a component which
       will be automatically resized and kept in the right place - then you can add
       subcomponents to the content comp. See the notes for the ResizableWindow class
       for more info.

       If you really know what you're doing and want to avoid this assertion, just call
       Component::addChildComponent directly.
    */
    jassertfalse;

    Component::addChildComponent (child, zOrder);
}

z0 ResizableWindow::addAndMakeVisible (Component* const child, i32 zOrder)
{
    /* Agh! You shouldn't add components directly to a ResizableWindow - this class
       manages its child components automatically, and if you add your own it'll cause
       trouble. Instead, use setContentComponent() to give it a component which
       will be automatically resized and kept in the right place - then you can add
       subcomponents to the content comp. See the notes for the ResizableWindow class
       for more info.

       If you really know what you're doing and want to avoid this assertion, just call
       Component::addAndMakeVisible directly.
    */
    jassertfalse;

    Component::addAndMakeVisible (child, zOrder);
}
#endif

} // namespace drx
