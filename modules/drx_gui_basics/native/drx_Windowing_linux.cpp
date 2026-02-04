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
class LinuxComponentPeer final : public ComponentPeer,
                                 private XWindowSystemUtilities::XSettings::Listener
{
public:
    LinuxComponentPeer (Component& comp, i32 windowStyleFlags, ::Window parentToAddTo)
        : ComponentPeer (comp, windowStyleFlags),
          isAlwaysOnTop (comp.isAlwaysOnTop())
    {
        // it's dangerous to create a window on a thread other than the message thread.
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        const auto* instance = XWindowSystem::getInstance();

        if (! instance->isX11Available())
            return;

        if (isAlwaysOnTop)
            ++WindowUtilsInternal::numAlwaysOnTopPeers;

        repainter = std::make_unique<LinuxRepaintManager> (*this);

        windowH = instance->createWindow (parentToAddTo, this);
        parentWindow = parentToAddTo;

        setTitle (component.getName());

        if (auto* xSettings = instance->getXSettings())
            xSettings->addListener (this);

        getNativeRealtimeModifiers = []() -> ModifierKeys { return XWindowSystem::getInstance()->getNativeRealtimeModifiers(); };

        updateVBlankTimer();
    }

    ~LinuxComponentPeer() override
    {
        // it's dangerous to delete a window on a thread other than the message thread.
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        auto* instance = XWindowSystem::getInstance();

        repainter = nullptr;
        instance->destroyWindow (windowH);

        if (auto* xSettings = instance->getXSettings())
            xSettings->removeListener (this);

        if (isAlwaysOnTop)
            --WindowUtilsInternal::numAlwaysOnTopPeers;
    }

    ::Window getWindowHandle() const noexcept
    {
        return windowH;
    }

    //==============================================================================
    uk getNativeHandle() const override
    {
        return reinterpret_cast<uk> (getWindowHandle());
    }

    //==============================================================================
    z0 forceSetBounds (const Rectangle<i32>& correctedNewBounds, b8 isNowFullScreen)
    {
        bounds = correctedNewBounds;

        updateScaleFactorFromNewBounds (bounds, false);

        auto physicalBounds = parentWindow == 0 ? Desktop::getInstance().getDisplays().logicalToPhysical (bounds)
                                                : bounds * currentScaleFactor;

        WeakReference<Component> deletionChecker (&component);

        // If we are in a ConfigureNotify handler then forceSetBounds is being called as a
        // consequence of X11 telling us what the window size is. There's no need to report this
        // size back again to X11. By this we are avoiding a pitfall, when we get many subsequent
        // ConfigureNotify events, many of which has stale size information. By not calling
        // XWindowSystem::setBounds we are not actualising these old, incorrect sizes.
        if (! inConfigureNotifyHandler)
            XWindowSystem::getInstance()->setBounds (windowH, physicalBounds, isNowFullScreen);

        fullScreen = isNowFullScreen;

        if (deletionChecker != nullptr)
        {
            updateBorderSize();
            handleMovedOrResized();
        }
    }

    z0 setBounds (const Rectangle<i32>& newBounds, b8 isNowFullScreen) override
    {
        const auto correctedNewBounds = newBounds.withSize (jmax (1, newBounds.getWidth()),
                                                            jmax (1, newBounds.getHeight()));

        if (bounds != correctedNewBounds || fullScreen != isNowFullScreen)
            forceSetBounds (correctedNewBounds, isNowFullScreen);
    }

    Point<i32> getScreenPosition (b8 physical) const
    {
        auto physicalParentPosition = XWindowSystem::getInstance()->getPhysicalParentScreenPosition();
        auto parentPosition = parentWindow == 0 ? Desktop::getInstance().getDisplays().physicalToLogical (physicalParentPosition)
                                                : physicalParentPosition / currentScaleFactor;

        auto screenBounds = parentWindow == 0 ? bounds
                                              : bounds.translated (parentPosition.x, parentPosition.y);

        if (physical)
            return parentWindow == 0 ? Desktop::getInstance().getDisplays().logicalToPhysical (screenBounds.getTopLeft())
                                     : screenBounds.getTopLeft() * currentScaleFactor;

        return screenBounds.getTopLeft();
    }

    Rectangle<i32> getBounds() const override
    {
        return bounds;
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        return windowBorder;
    }

    BorderSize<i32> getFrameSize() const override
    {
        const auto optionalBorderSize = getFrameSizeIfPresent();
        return optionalBorderSize ? (*optionalBorderSize) : BorderSize<i32>();
    }

    Point<f32> localToGlobal (Point<f32> relativePosition) override
    {
        return localToGlobal (*this, relativePosition);
    }

    Point<f32> globalToLocal (Point<f32> screenPosition) override
    {
        return globalToLocal (*this, screenPosition);
    }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    //==============================================================================
    StringArray getAvailableRenderingEngines() override
    {
        return { "Software Renderer" };
    }

    z0 setVisible (b8 shouldBeVisible) override
    {
        XWindowSystem::getInstance()->setVisible (windowH, shouldBeVisible);
    }

    z0 setTitle (const Txt& title) override
    {
        XWindowSystem::getInstance()->setTitle (windowH, title);
    }

    z0 setMinimised (b8 shouldBeMinimised) override
    {
        if (shouldBeMinimised)
            XWindowSystem::getInstance()->setMinimised (windowH, shouldBeMinimised);
        else
            setVisible (true);
    }

    b8 isMinimised() const override
    {
        return XWindowSystem::getInstance()->isMinimised (windowH);
    }

    b8 isShowing() const override
    {
        return ! XWindowSystem::getInstance()->isMinimised (windowH);
    }

    z0 setFullScreen (b8 shouldBeFullScreen) override
    {
        auto r = lastNonFullscreenBounds; // (get a copy of this before de-minimising)

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            const auto usingNativeTitleBar = ((styleFlags & windowHasTitleBar) != 0);

            if (usingNativeTitleBar)
                XWindowSystem::getInstance()->setMaximised (windowH, shouldBeFullScreen);

            if (shouldBeFullScreen)
                r = usingNativeTitleBar ? XWindowSystem::getInstance()->getWindowBounds (windowH, parentWindow)
                                        : Desktop::getInstance().getDisplays().getDisplayForRect (bounds)->userArea;

            if (! r.isEmpty())
                setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

            component.repaint();
        }
    }

    b8 isFullScreen() const override
    {
        return fullScreen;
    }

    b8 contains (Point<i32> localPos, b8 trueIfInAChildWindow) const override
    {
        if (! bounds.withZeroOrigin().contains (localPos))
            return false;

        for (i32 i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            auto* c = Desktop::getInstance().getComponent (i);

            if (c == &component)
                break;

            if (! c->isVisible())
                continue;

            auto* otherPeer = c->getPeer();
            jassert (otherPeer == nullptr || dynamic_cast<LinuxComponentPeer*> (c->getPeer()) != nullptr);

            if (auto* peer = static_cast<LinuxComponentPeer*> (otherPeer))
                if (peer->contains (globalToLocal (*peer, localToGlobal (*this, localPos.toFloat())).roundToInt(), true))
                    return false;
        }

        if (trueIfInAChildWindow)
            return true;

        return XWindowSystem::getInstance()->contains (windowH, localPos * currentScaleFactor);
    }

    z0 toFront (b8 makeActive) override
    {
        if (makeActive)
        {
            setVisible (true);
            grabFocus();
        }

        XWindowSystem::getInstance()->toFront (windowH, makeActive);
        handleBroughtToFront();
    }

    z0 toBehind (ComponentPeer* other) override
    {
        if (auto* otherPeer = dynamic_cast<LinuxComponentPeer*> (other))
        {
            if (otherPeer->styleFlags & windowIsTemporary)
                return;

            setMinimised (false);
            XWindowSystem::getInstance()->toBehind (windowH, otherPeer->windowH);
        }
        else
        {
            jassertfalse; // wrong type of window?
        }
    }

    b8 isFocused() const override
    {
        return XWindowSystem::getInstance()->isFocused (windowH);
    }

    z0 grabFocus() override
    {
        if (XWindowSystem::getInstance()->grabFocus (windowH))
            isActiveApplication = true;
    }

    //==============================================================================
    z0 repaint (const Rectangle<i32>& area) override
    {
        if (repainter != nullptr)
            repainter->repaint (area.getIntersection (bounds.withZeroOrigin()));
    }

    z0 performAnyPendingRepaintsNow() override
    {
        if (repainter != nullptr)
            repainter->performAnyPendingRepaintsNow();
    }

    z0 setIcon (const Image& newIcon) override
    {
        XWindowSystem::getInstance()->setIcon (windowH, newIcon);
    }

    f64 getPlatformScaleFactor() const noexcept override
    {
        return currentScaleFactor;
    }

    z0 setAlpha (f32) override                                  {}
    b8 setAlwaysOnTop (b8) override                             { return false; }
    z0 textInputRequired (Point<i32>, TextInputTarget&) override  {}

    //==============================================================================
    z0 addOpenGLRepaintListener (Component* dummy)
    {
        if (dummy != nullptr)
            glRepaintListeners.addIfNotAlreadyThere (dummy);
    }

    z0 removeOpenGLRepaintListener (Component* dummy)
    {
        if (dummy != nullptr)
            glRepaintListeners.removeAllInstancesOf (dummy);
    }

    z0 repaintOpenGLContexts()
    {
        for (auto* c : glRepaintListeners)
            c->handleCommandMessage (0);
    }

    //==============================================================================
    ::Window getParentWindow()                         { return parentWindow; }
    z0 setParentWindow (::Window newParent)          { parentWindow = newParent; }

    //==============================================================================
    b8 isConstrainedNativeWindow() const
    {
        return constrainer != nullptr
            && (styleFlags & (windowHasTitleBar | windowIsResizable)) == (windowHasTitleBar | windowIsResizable)
            && ! isKioskMode();
    }

    z0 updateWindowBounds()
    {
        if (windowH == 0)
        {
            jassertfalse;
            return;
        }

        if (isConstrainedNativeWindow())
            XWindowSystem::getInstance()->updateConstraints (windowH);

        auto physicalBounds = XWindowSystem::getInstance()->getWindowBounds (windowH, parentWindow);

        updateScaleFactorFromNewBounds (physicalBounds, true);

        bounds = parentWindow == 0 ? Desktop::getInstance().getDisplays().physicalToLogical (physicalBounds)
                                   : physicalBounds / currentScaleFactor;

        updateVBlankTimer();
    }

    z0 updateBorderSize()
    {
        if ((styleFlags & windowHasTitleBar) == 0)
        {
            windowBorder = ComponentPeer::OptionalBorderSize { BorderSize<i32>() };
        }
        else if (! windowBorder
                 || ((*windowBorder).getTopAndBottom() == 0 && (*windowBorder).getLeftAndRight() == 0))
        {
            windowBorder = [&]()
            {
                if (auto unscaledBorderSize = XWindowSystem::getInstance()->getBorderSize (windowH))
                    return OptionalBorderSize { (*unscaledBorderSize).multipliedBy (1.0 / currentScaleFactor) };

                return OptionalBorderSize {};
            }();
        }
    }

    b8 setWindowAssociation (::Window windowIn)
    {
        clearWindowAssociation();
        association = { this, windowIn };
        return association.isValid();
    }

    z0 clearWindowAssociation() { association = {}; }

    z0 startHostManagedResize (Point<i32>, ResizableBorderComponent::Zone zone) override
    {
        XWindowSystem::getInstance()->startHostManagedResize (windowH, zone);
    }

    //==============================================================================
    static b8 isActiveApplication;
    b8 focused = false;
    b8 inConfigureNotifyHandler = false;

private:
    //==============================================================================
    class LinuxRepaintManager
    {
    public:
        LinuxRepaintManager (LinuxComponentPeer& p)
            : peer (p),
              isSemiTransparentWindow ((peer.getStyleFlags() & ComponentPeer::windowIsSemiTransparent) != 0)
        {
        }

        z0 dispatchDeferredRepaints()
        {
            XWindowSystem::getInstance()->processPendingPaintsForWindow (peer.windowH);

            if (XWindowSystem::getInstance()->getNumPaintsPendingForWindow (peer.windowH) > 0)
                return;

            if (! regionsNeedingRepaint.isEmpty())
                performAnyPendingRepaintsNow();
            else if (Time::getApproximateMillisecondCounter() > lastTimeImageUsed + 3000)
                image = Image();
        }

        z0 repaint (Rectangle<i32> area)
        {
            regionsNeedingRepaint.add (area * peer.currentScaleFactor);
        }

        z0 performAnyPendingRepaintsNow()
        {
            if (XWindowSystem::getInstance()->getNumPaintsPendingForWindow (peer.windowH) > 0)
                return;

            auto originalRepaintRegion = regionsNeedingRepaint;
            regionsNeedingRepaint.clear();
            auto totalArea = originalRepaintRegion.getBounds();

            if (! totalArea.isEmpty())
            {
                const auto wasImageNull = image.isNull();

                if (wasImageNull || image.getWidth() < totalArea.getWidth()
                     || image.getHeight() < totalArea.getHeight())
                {
                    image = XWindowSystem::getInstance()->createImage (isSemiTransparentWindow,
                                                                       totalArea.getWidth(), totalArea.getHeight(),
                                                                       useARGBImagesForRendering);
                    if (wasImageNull)
                    {
                        // After calling createImage() XWindowSystem::getWindowBounds() will return
                        // changed coordinates that look like the result of some position
                        // defaulting mechanism. If we handle a configureNotifyEvent after
                        // createImage() and before we would issue new, valid coordinates, we will
                        // apply these default, unwanted coordinates to our window. To avoid that
                        // we immediately send another positioning message to guarantee that the
                        // next configureNotifyEvent will read valid values.
                        //
                        // This issue only occurs right after peer creation, when the image is
                        // null. Updating when only the width or height is changed would lead to
                        // incorrect behaviour.
                        peer.forceSetBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (peer.component, peer.component.getBoundsInParent()),
                                             peer.isFullScreen());
                    }
                }

                RectangleList<i32> adjustedList (originalRepaintRegion);
                adjustedList.offsetAll (-totalArea.getX(), -totalArea.getY());

                if (XWindowSystem::getInstance()->canUseARGBImages())
                    for (auto& i : originalRepaintRegion)
                        image.clear (i - totalArea.getPosition());

                {
                    auto context = peer.getComponent().getLookAndFeel()
                                     .createGraphicsContext (image, -totalArea.getPosition(), adjustedList);

                    context->addTransform (AffineTransform::scale ((f32) peer.currentScaleFactor));
                    peer.handlePaint (*context);
                }

                for (auto& i : originalRepaintRegion)
                   XWindowSystem::getInstance()->blitToWindow (peer.windowH, image, i, totalArea);
            }

            lastTimeImageUsed = Time::getApproximateMillisecondCounter();
        }

    private:
        LinuxComponentPeer& peer;
        const b8 isSemiTransparentWindow;
        Image image;
        u32 lastTimeImageUsed = 0;
        RectangleList<i32> regionsNeedingRepaint;

        b8 useARGBImagesForRendering = XWindowSystem::getInstance()->canUseARGBImages();

        DRX_DECLARE_NON_COPYABLE (LinuxRepaintManager)
    };

    //==============================================================================
    template <typename This>
    static Point<f32> localToGlobal (This& t, Point<f32> relativePosition)
    {
        return relativePosition + t.getScreenPosition (false).toFloat();
    }

    template <typename This>
    static Point<f32> globalToLocal (This& t, Point<f32> screenPosition)
    {
        return screenPosition - t.getScreenPosition (false).toFloat();
    }

    //==============================================================================
    z0 settingChanged (const XWindowSystemUtilities::XSetting& settingThatHasChanged) override
    {
        static StringArray possibleSettings { XWindowSystem::getWindowScalingFactorSettingName(),
                                              "Gdk/UnscaledDPI",
                                              "Xft/DPI" };

        if (possibleSettings.contains (settingThatHasChanged.name))
            forceDisplayUpdate();
    }

    z0 updateScaleFactorFromNewBounds (const Rectangle<i32>& newBounds, b8 isPhysical)
    {
        Point<i32> translation = (parentWindow != 0 ? getScreenPosition (isPhysical) : Point<i32>());
        const auto& desktop = Desktop::getInstance();

        if (auto* display = desktop.getDisplays().getDisplayForRect (newBounds.translated (translation.x, translation.y),
                                                                     isPhysical))
        {
            auto newScaleFactor = display->scale / desktop.getGlobalScaleFactor();

            if (! approximatelyEqual (newScaleFactor, currentScaleFactor))
            {
                currentScaleFactor = newScaleFactor;
                scaleFactorListeners.call ([&] (ScaleFactorListener& l) { l.nativeScaleFactorChanged (currentScaleFactor); });
            }
        }
    }

    z0 onVBlank()
    {
        const auto timestampSec = Time::getMillisecondCounterHiRes() / 1000.0;
        callVBlankListeners (timestampSec);

        if (repainter != nullptr)
            repainter->dispatchDeferredRepaints();
    }

    z0 updateVBlankTimer()
    {
        if (auto* display = Desktop::getInstance().getDisplays().getDisplayForRect (bounds))
        {
            // Some systems fail to set an explicit refresh rate, or ask for a refresh rate of 0
            // (observed on Raspbian Bullseye over VNC). In these situations, use a fallback value.
            const auto newIntFrequencyHz = roundToInt (display->verticalFrequencyHz.value_or (0.0));
            const auto frequencyToUse = newIntFrequencyHz != 0 ? newIntFrequencyHz : 100;

            if (vBlankManager.getTimerInterval() != frequencyToUse)
                vBlankManager.startTimerHz (frequencyToUse);
        }
    }

    //==============================================================================
    std::unique_ptr<LinuxRepaintManager> repainter;
    TimedCallback vBlankManager { [this]() { onVBlank(); } };

    ::Window windowH = {}, parentWindow = {};
    Rectangle<i32> bounds;
    ComponentPeer::OptionalBorderSize windowBorder;
    b8 fullScreen = false, isAlwaysOnTop = false;
    f64 currentScaleFactor = 1.0;
    Array<Component*> glRepaintListeners;
    ScopedWindowAssociation association;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinuxComponentPeer)
};

b8 LinuxComponentPeer::isActiveApplication = false;

//==============================================================================
ComponentPeer* Component::createNewPeer (i32 styleFlags, uk nativeWindowToAttachTo)
{
    return new LinuxComponentPeer (*this, styleFlags, (::Window) nativeWindowToAttachTo);
}

//==============================================================================
DRX_API b8 DRX_CALLTYPE Process::isForegroundProcess()    { return LinuxComponentPeer::isActiveApplication; }

DRX_API z0 DRX_CALLTYPE Process::makeForegroundProcess()  {}
DRX_API z0 DRX_CALLTYPE Process::hide()                   {}

//==============================================================================
z0 Desktop::setKioskComponent (Component* comp, b8 enableOrDisable, b8)
{
    if (enableOrDisable)
        comp->setBounds (getDisplays().getDisplayForRect (comp->getScreenBounds())->totalArea);
}

z0 Displays::findDisplays (f32 masterScale)
{
    if (XWindowSystem::getInstance()->getDisplay() != nullptr)
    {
        displays = XWindowSystem::getInstance()->findDisplays (masterScale);

        if (! displays.isEmpty())
            updateToLogical();
    }
}

b8 Desktop::canUseSemiTransparentWindows() noexcept
{
    return XWindowSystem::getInstance()->canUseSemiTransparentWindows();
}

class Desktop::NativeDarkModeChangeDetectorImpl  : private XWindowSystemUtilities::XSettings::Listener
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
        const auto* windowSystem = XWindowSystem::getInstance();

        if (auto* xSettings = windowSystem->getXSettings())
            xSettings->addListener (this);

        darkModeEnabled = windowSystem->isDarkModeActive();
    }

    ~NativeDarkModeChangeDetectorImpl() override
    {
        if (auto* windowSystem = XWindowSystem::getInstanceWithoutCreating())
            if (auto* xSettings = windowSystem->getXSettings())
                xSettings->removeListener (this);
    }

    b8 isDarkModeEnabled() const noexcept  { return darkModeEnabled; }

private:
    z0 settingChanged (const XWindowSystemUtilities::XSetting& settingThatHasChanged) override
    {
        if (settingThatHasChanged.name == XWindowSystem::getThemeNameSettingName())
        {
            const auto wasDarkModeEnabled = std::exchange (darkModeEnabled, XWindowSystem::getInstance()->isDarkModeActive());

            if (darkModeEnabled != wasDarkModeEnabled)
                Desktop::getInstance().darkModeChanged();
        }
    }

    b8 darkModeEnabled = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
}

b8 Desktop::isDarkModeActive() const
{
    return nativeDarkModeChangeDetectorImpl->isDarkModeEnabled();
}

static b8 screenSaverAllowed = true;

z0 Desktop::setScreenSaverEnabled (b8 isEnabled)
{
    if (screenSaverAllowed != isEnabled)
    {
        screenSaverAllowed = isEnabled;
        XWindowSystem::getInstance()->setScreenSaverEnabled (screenSaverAllowed);
    }
}

b8 Desktop::isScreenSaverEnabled()
{
    return screenSaverAllowed;
}

f64 Desktop::getDefaultMasterScale()                             { return 1.0; }

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const  { return upright; }
z0 Desktop::allowedOrientationsChanged()                          {}

//==============================================================================
b8 detail::MouseInputSourceList::addSource()
{
    if (sources.isEmpty())
    {
        addSource (0, MouseInputSource::InputSourceType::mouse);
        return true;
    }

    return false;
}

b8 detail::MouseInputSourceList::canUseTouch() const
{
    return false;
}

Point<f32> MouseInputSource::getCurrentRawMousePosition()
{
    return Desktop::getInstance().getDisplays().physicalToLogical (XWindowSystem::getInstance()->getCurrentMousePosition());
}

z0 MouseInputSource::setRawMousePosition (Point<f32> newPosition)
{
    XWindowSystem::getInstance()->setMousePosition (Desktop::getInstance().getDisplays().logicalToPhysical (newPosition));
}

//==============================================================================
class MouseCursor::PlatformSpecificHandle
{
public:
    explicit PlatformSpecificHandle (const MouseCursor::StandardCursorType type)
        : cursorHandle (makeHandle (type)) {}

    explicit PlatformSpecificHandle (const detail::CustomMouseCursorInfo& info)
        : cursorHandle (makeHandle (info)) {}

    ~PlatformSpecificHandle()
    {
        if (cursorHandle != Cursor{})
            XWindowSystem::getInstance()->deleteMouseCursor (cursorHandle);
    }

    static z0 showInWindow (PlatformSpecificHandle* handle, ComponentPeer* peer)
    {
        const auto cursor = handle != nullptr ? handle->cursorHandle : Cursor{};

        if (peer != nullptr)
            XWindowSystem::getInstance()->showCursor ((::Window) peer->getNativeHandle(), cursor);
    }

private:
    static Cursor makeHandle (const detail::CustomMouseCursorInfo& info)
    {
        const auto image = info.image.getImage();
        return XWindowSystem::getInstance()->createCustomMouseCursorInfo (image.rescaled ((i32) (image.getWidth()  / info.image.getScale()),
                                                                                          (i32) (image.getHeight() / info.image.getScale())), info.hotspot);
    }

    static Cursor makeHandle (MouseCursor::StandardCursorType type)
    {
        return XWindowSystem::getInstance()->createStandardMouseCursor (type);
    }

    Cursor cursorHandle;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE (PlatformSpecificHandle)
    DRX_DECLARE_NON_MOVEABLE (PlatformSpecificHandle)
};

//==============================================================================
static LinuxComponentPeer* getPeerForDragEvent (Component* sourceComp)
{
    if (sourceComp == nullptr)
        if (auto* draggingSource = Desktop::getInstance().getDraggingMouseSource (0))
            sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp != nullptr)
        if (auto* lp = dynamic_cast<LinuxComponentPeer*> (sourceComp->getPeer()))
            return lp;

    jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
    return nullptr;
}

b8 DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, b8 canMoveFiles,
                                                           Component* sourceComp, std::function<z0()> callback)
{
    if (files.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent (sourceComp))
        return XWindowSystem::getInstance()->externalDragFileInit (peer, files, canMoveFiles, std::move (callback));

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

b8 DragAndDropContainer::performExternalDragDropOfText (const Txt& text, Component* sourceComp,
                                                          std::function<z0()> callback)
{
    if (text.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent (sourceComp))
        return XWindowSystem::getInstance()->externalDragTextInit (peer, text, std::move (callback));

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

//==============================================================================
z0 SystemClipboard::copyTextToClipboard (const Txt& clipText)
{
    XWindowSystem::getInstance()->copyTextToClipboard (clipText);
}

Txt SystemClipboard::getTextFromClipboard()
{
    return XWindowSystem::getInstance()->getTextFromClipboard();
}

//==============================================================================
b8 KeyPress::isKeyCurrentlyDown (i32 keyCode)
{
    return XWindowSystem::getInstance()->isKeyCurrentlyDown (keyCode);
}

z0 LookAndFeel::playAlertSound()
{
    std::cout << "\a" << std::flush;
}

//==============================================================================
Image detail::WindowingHelpers::createIconForFile (const File&)
{
    return {};
}

z0 drx_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy);
z0 drx_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->addOpenGLRepaintListener (dummy);
}

z0 drx_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy);
z0 drx_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy)
{
    if (auto* linuxPeer = dynamic_cast<LinuxComponentPeer*> (peer))
        linuxPeer->removeOpenGLRepaintListener (dummy);
}

} // namespace drx
