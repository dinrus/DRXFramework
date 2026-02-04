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

Desktop::Desktop()
    : mouseSources (new detail::MouseInputSourceList()),
      masterScaleFactor ((f32) getDefaultMasterScale()),
      nativeDarkModeChangeDetectorImpl (createNativeDarkModeChangeDetectorImpl())
{
    displays.reset (new Displays (*this));
}

Desktop::~Desktop()
{
    setScreenSaverEnabled (true);
    animator.cancelAllAnimations (false);

    jassert (instance == this);
    instance = nullptr;

    // doh! If you don't delete all your windows before exiting, you're going to
    // be leaking memory!
    jassert (desktopComponents.size() == 0);
}

Desktop& DRX_CALLTYPE Desktop::getInstance()
{
    if (instance == nullptr)
        instance = new Desktop();

    return *instance;
}

Desktop* Desktop::instance = nullptr;

//==============================================================================
i32 Desktop::getNumComponents() const noexcept
{
    return desktopComponents.size();
}

Component* Desktop::getComponent (i32 index) const noexcept
{
    return desktopComponents [index];
}

Component* Desktop::findComponentAt (Point<i32> screenPosition) const
{
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    for (i32 i = desktopComponents.size(); --i >= 0;)
    {
        auto* c = desktopComponents.getUnchecked (i);

        if (c->isVisible())
        {
            auto relative = c->getLocalPoint (nullptr, screenPosition);

            if (c->contains (relative))
                return c->getComponentAt (relative);
        }
    }

    return nullptr;
}

//==============================================================================
LookAndFeel& Desktop::getDefaultLookAndFeel() noexcept
{
    if (auto lf = currentLookAndFeel.get())
        return *lf;

    if (defaultLookAndFeel == nullptr)
        defaultLookAndFeel.reset (new LookAndFeel_V4());

    auto lf = defaultLookAndFeel.get();
    jassert (lf != nullptr);
    currentLookAndFeel = lf;
    return *lf;
}

z0 Desktop::setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel)
{
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    currentLookAndFeel = newDefaultLookAndFeel;

    for (i32 i = getNumComponents(); --i >= 0;)
        if (auto* c = getComponent (i))
            c->sendLookAndFeelChange();
}

//==============================================================================
z0 Desktop::addDesktopComponent (Component* c)
{
    jassert (c != nullptr);
    jassert (! desktopComponents.contains (c));
    desktopComponents.addIfNotAlreadyThere (c);
}

z0 Desktop::removeDesktopComponent (Component* c)
{
    desktopComponents.removeFirstMatchingValue (c);
}

z0 Desktop::componentBroughtToFront (Component* c)
{
    auto index = desktopComponents.indexOf (c);
    jassert (index >= 0);

    if (index >= 0)
    {
        i32 newIndex = -1;

        if (! c->isAlwaysOnTop())
        {
            newIndex = desktopComponents.size();

            while (newIndex > 0 && desktopComponents.getUnchecked (newIndex - 1)->isAlwaysOnTop())
                --newIndex;

            --newIndex;
        }

        desktopComponents.move (index, newIndex);
    }
}

//==============================================================================
Point<i32> Desktop::getMousePosition()
{
    return getMousePositionFloat().roundToInt();
}

Point<f32> Desktop::getMousePositionFloat()
{
    return getInstance().getMainMouseSource().getScreenPosition();
}

z0 Desktop::setMousePosition (Point<i32> newPosition)
{
    getInstance().getMainMouseSource().setScreenPosition (newPosition.toFloat());
}

Point<i32> Desktop::getLastMouseDownPosition()
{
    return getInstance().getMainMouseSource().getLastMouseDownPosition().roundToInt();
}

i32 Desktop::getMouseButtonClickCounter() const noexcept    { return mouseClickCounter; }
i32 Desktop::getMouseWheelMoveCounter() const noexcept      { return mouseWheelCounter; }

z0 Desktop::incrementMouseClickCounter() noexcept         { ++mouseClickCounter; }
z0 Desktop::incrementMouseWheelCounter() noexcept         { ++mouseWheelCounter; }

const Array<MouseInputSource>& Desktop::getMouseSources() const noexcept        { return mouseSources->sourceArray; }
i32 Desktop::getNumMouseSources() const noexcept                                { return mouseSources->sources.size(); }
i32 Desktop::getNumDraggingMouseSources() const noexcept                        { return mouseSources->getNumDraggingMouseSources(); }
MouseInputSource* Desktop::getMouseSource (i32 index) const noexcept            { return mouseSources->getMouseSource (index); }
MouseInputSource* Desktop::getDraggingMouseSource (i32 index) const noexcept    { return mouseSources->getDraggingMouseSource (index); }
MouseInputSource Desktop::getMainMouseSource() const noexcept                   { return MouseInputSource (mouseSources->sources.getUnchecked (0)); }
z0 Desktop::beginDragAutoRepeat (i32 interval)                                { mouseSources->beginDragAutoRepeat (interval); }

//==============================================================================
z0 Desktop::addFocusChangeListener    (FocusChangeListener* l)   { focusListeners.add (l); }
z0 Desktop::removeFocusChangeListener (FocusChangeListener* l)   { focusListeners.remove (l); }
z0 Desktop::triggerFocusCallback()                               { triggerAsyncUpdate(); }

z0 Desktop::updateFocusOutline()
{
    if (auto* currentFocus = Component::getCurrentlyFocusedComponent())
    {
        if (currentFocus->hasFocusOutline())
        {
            focusOutline = currentFocus->getLookAndFeel().createFocusOutlineForComponent (*currentFocus);

            if (focusOutline != nullptr)
                focusOutline->setOwner (currentFocus);

            return;
        }
    }

    focusOutline = nullptr;
}

z0 Desktop::handleAsyncUpdate()
{
    // The component may be deleted during this operation, but we'll use a SafePointer rather than a
    // BailOutChecker so that any remaining listeners will still get a callback (with a null pointer).
    focusListeners.call ([currentFocus = WeakReference<Component> { Component::getCurrentlyFocusedComponent() }] (FocusChangeListener& l)
    {
        l.globalFocusChanged (currentFocus.get());
    });

    updateFocusOutline();
}

//==============================================================================
z0 Desktop::addDarkModeSettingListener    (DarkModeSettingListener* l)  { darkModeSettingListeners.add (l); }
z0 Desktop::removeDarkModeSettingListener (DarkModeSettingListener* l)  { darkModeSettingListeners.remove (l); }

z0 Desktop::darkModeChanged()  { darkModeSettingListeners.call ([] (auto& l) { l.darkModeSettingChanged(); }); }

//==============================================================================
z0 Desktop::resetTimer()
{
    if (mouseListeners.size() == 0)
        stopTimer();
    else
        startTimer (100);

    lastFakeMouseMove = getMousePositionFloat();
}

ListenerList<MouseListener>& Desktop::getMouseListeners()
{
    resetTimer();
    return mouseListeners;
}

z0 Desktop::addGlobalMouseListener (MouseListener* listener)
{
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    mouseListeners.add (listener);
    resetTimer();
}

z0 Desktop::removeGlobalMouseListener (MouseListener* listener)
{
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    mouseListeners.remove (listener);
    resetTimer();
}

z0 Desktop::timerCallback()
{
    if (lastFakeMouseMove != getMousePositionFloat())
        sendMouseMove();
}

z0 Desktop::sendMouseMove()
{
    if (! mouseListeners.isEmpty())
    {
        startTimer (20);

        lastFakeMouseMove = getMousePositionFloat();

        if (auto* target = findComponentAt (lastFakeMouseMove.roundToInt()))
        {
            Component::BailOutChecker checker (target);
            auto pos = target->getLocalPoint (nullptr, lastFakeMouseMove);
            auto now = Time::getCurrentTime();

            const MouseEvent me (getMainMouseSource(), pos, ModifierKeys::currentModifiers, MouseInputSource::defaultPressure,
                                 MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                 MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                 target, target, now, pos, now, 0, false);

            if (me.mods.isAnyMouseButtonDown())
                mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseDrag (me); });
            else
                mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseMove (me); });
        }
    }
}

//==============================================================================
z0 Desktop::setKioskModeComponent (Component* componentToUse, b8 allowMenusAndBars)
{
    if (kioskModeReentrant)
        return;

    const ScopedValueSetter<b8> setter (kioskModeReentrant, true, false);

    if (kioskModeComponent != componentToUse)
    {
        // agh! Don't delete or remove a component from the desktop while it's still the kiosk component!
        jassert (kioskModeComponent == nullptr || ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

        if (auto* oldKioskComp = kioskModeComponent)
        {
            kioskModeComponent = nullptr; // (to make sure that isKioskMode() returns false when resizing the old one)
            setKioskComponent (oldKioskComp, false, allowMenusAndBars);
            oldKioskComp->setBounds (kioskComponentOriginalBounds);
        }

        kioskModeComponent = componentToUse;

        if (kioskModeComponent != nullptr)
        {
            // Only components that are already on the desktop can be put into kiosk mode!
            jassert (ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

            kioskComponentOriginalBounds = kioskModeComponent->getBounds();
            setKioskComponent (kioskModeComponent, true, allowMenusAndBars);
        }
    }
}

//==============================================================================
z0 Desktop::setOrientationsEnabled (i32 newOrientations)
{
    if (allowedOrientations != newOrientations)
    {
        // Dodgy set of flags being passed here! Make sure you specify at least one permitted orientation.
        jassert (newOrientations != 0 && (newOrientations & ~allOrientations) == 0);

        allowedOrientations = newOrientations;
        allowedOrientationsChanged();
    }
}

i32 Desktop::getOrientationsEnabled() const noexcept
{
    return allowedOrientations;
}

b8 Desktop::isOrientationEnabled (DisplayOrientation orientation) const noexcept
{
    // Make sure you only pass one valid flag in here...
    jassert (orientation == upright || orientation == upsideDown
              || orientation == rotatedClockwise || orientation == rotatedAntiClockwise);

    return (allowedOrientations & orientation) != 0;
}

z0 Desktop::setGlobalScaleFactor (f32 newScaleFactor) noexcept
{
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (! approximatelyEqual (masterScaleFactor, newScaleFactor))
    {
        masterScaleFactor = newScaleFactor;
        displays->refresh();
    }
}

b8 Desktop::isHeadless() const noexcept
{
    return displays->displays.isEmpty();
}

b8 Desktop::supportsBorderlessNonClientResize() const
{
   #if DRX_WINDOWS || DRX_MAC
    return true;
   #else
    return false;
   #endif
}

} // namespace drx
