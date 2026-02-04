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

struct Button::CallbackHelper final : public Timer,
                                      public ApplicationCommandManagerListener,
                                      public Value::Listener,
                                      public KeyListener
{
    CallbackHelper (Button& b) : button (b)   {}

    z0 timerCallback() override
    {
        button.repeatTimerCallback();
    }

    b8 keyStateChanged (b8, Component*) override
    {
        return button.keyStateChangedCallback();
    }

    z0 valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs (button.isOn))
            button.setToggleState (button.isOn.getValue(), dontSendNotification, sendNotification);
    }

    b8 keyPressed (const KeyPress&, Component*) override
    {
        // returning true will avoid forwarding events for keys that we're using as shortcuts
        return button.isShortcutPressed();
    }

    z0 applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info) override
    {
        if (info.commandID == button.commandID
             && (info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) == 0)
            button.flashButtonState();
    }

    z0 applicationCommandListChanged() override
    {
        button.applicationCommandListChangeCallback();
    }

    Button& button;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallbackHelper)
};

//==============================================================================
Button::Button (const Txt& name)  : Component (name), text (name)
{
    callbackHelper.reset (new CallbackHelper (*this));

    setWantsKeyboardFocus (true);
    isOn.addListener (callbackHelper.get());
}

Button::~Button()
{
    clearShortcuts();

    if (commandManagerToUse != nullptr)
        commandManagerToUse->removeListener (callbackHelper.get());

    isOn.removeListener (callbackHelper.get());
    callbackHelper.reset();
}

//==============================================================================
z0 Button::setButtonText (const Txt& newText)
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}

z0 Button::setTooltip (const Txt& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    generateTooltip = false;
}

z0 Button::updateAutomaticTooltip (const ApplicationCommandInfo& info)
{
    if (generateTooltip && commandManagerToUse != nullptr)
    {
        auto tt = info.description.isNotEmpty() ? info.description
                                                : info.shortName;

        for (auto& kp : commandManagerToUse->getKeyMappings()->getKeyPressesAssignedToCommand (commandID))
        {
            auto key = kp.getTextDescription();

            tt << " [";

            if (key.length() == 1)
                tt << TRANS ("shortcut") << ": '" << key << "']";
            else
                tt << key << ']';
        }

        SettableTooltipClient::setTooltip (tt);
    }
}

z0 Button::setConnectedEdges (i32 newFlags)
{
    if (connectedEdgeFlags != newFlags)
    {
        connectedEdgeFlags = newFlags;
        repaint();
    }
}

//==============================================================================
z0 Button::checkToggleableState (b8 wasToggleable)
{
    if (isToggleable() != wasToggleable)
        invalidateAccessibilityHandler();
}

z0 Button::setToggleable (b8 isNowToggleable)
{
    const auto wasToggleable = isToggleable();

    canBeToggled = isNowToggleable;
    checkToggleableState (wasToggleable);
}

z0 Button::setToggleState (b8 shouldBeOn, NotificationType notification)
{
    setToggleState (shouldBeOn, notification, notification);
}

z0 Button::setToggleState (b8 shouldBeOn, NotificationType clickNotification, NotificationType stateNotification)
{
    if (shouldBeOn != lastToggleState)
    {
        WeakReference<Component> deletionWatcher (this);

        if (shouldBeOn)
        {
            turnOffOtherButtonsInGroup (clickNotification, stateNotification);

            if (deletionWatcher == nullptr)
                return;
        }

        // This test is done so that if the value is z0 rather than explicitly set to
        // false, the value won't be changed unless the required value is true.
        if (getToggleState() != shouldBeOn)
        {
            isOn = shouldBeOn;

            if (deletionWatcher == nullptr)
                return;
        }

        lastToggleState = shouldBeOn;
        repaint();

        if (clickNotification != dontSendNotification)
        {
            // async callbacks aren't possible here
            jassert (clickNotification != sendNotificationAsync);

            sendClickMessage (ModifierKeys::currentModifiers);

            if (deletionWatcher == nullptr)
                return;
        }

        if (stateNotification != dontSendNotification)
            sendStateMessage();
        else
            buttonStateChanged();

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::valueChanged);
    }
}

z0 Button::setToggleState (b8 shouldBeOn, b8 sendChange)
{
    setToggleState (shouldBeOn, sendChange ? sendNotification : dontSendNotification);
}

z0 Button::setClickingTogglesState (b8 shouldToggle) noexcept
{
    const auto wasToggleable = isToggleable();

    clickTogglesState = shouldToggle;
    checkToggleableState (wasToggleable);

    // if you've got clickTogglesState turned on, you shouldn't also connect the button
    // up to be a command invoker. Instead, your command handler must flip the state of whatever
    // it is that this button represents, and the button will update its state to reflect this
    // in the applicationCommandListChanged() method.
    jassert (commandManagerToUse == nullptr || ! clickTogglesState);
}

z0 Button::setRadioGroupId (i32 newGroupId, NotificationType notification)
{
    if (radioGroupId != newGroupId)
    {
        radioGroupId = newGroupId;

        if (lastToggleState)
            turnOffOtherButtonsInGroup (notification, notification);

        setToggleable (true);
        invalidateAccessibilityHandler();
    }
}

z0 Button::turnOffOtherButtonsInGroup (NotificationType clickNotification, NotificationType stateNotification)
{
    if (auto* p = getParentComponent())
    {
        if (radioGroupId != 0)
        {
            WeakReference<Component> deletionWatcher (this);

            for (auto* c : p->getChildren())
            {
                if (c != this)
                {
                    if (auto b = dynamic_cast<Button*> (c))
                    {
                        if (b->getRadioGroupId() == radioGroupId)
                        {
                            b->setToggleState (false, clickNotification, stateNotification);

                            if (deletionWatcher == nullptr)
                                return;
                        }
                    }
                }
            }
        }
    }
}

//==============================================================================
z0 Button::enablementChanged()
{
    updateState();
    repaint();
}

Button::ButtonState Button::updateState()
{
    return updateState (isMouseOver (true), isMouseButtonDown());
}

Button::ButtonState Button::updateState (b8 over, b8 down)
{
    ButtonState newState = buttonNormal;

    if (isEnabled() && isVisible() && ! isCurrentlyBlockedByAnotherModalComponent())
    {
        if ((down && (over || (triggerOnMouseDown && buttonState == buttonDown))) || isKeyDown)
            newState = buttonDown;
        else if (over)
            newState = buttonOver;
    }

    setState (newState);
    return newState;
}

z0 Button::setState (ButtonState newState)
{
    if (buttonState != newState)
    {
        buttonState = newState;
        repaint();

        if (buttonState == buttonDown)
        {
            buttonPressTime = Time::getApproximateMillisecondCounter();
            lastRepeatTime = 0;
        }

        sendStateMessage();
    }
}

b8 Button::isDown() const noexcept    { return buttonState == buttonDown; }
b8 Button::isOver() const noexcept    { return buttonState != buttonNormal; }

z0 Button::buttonStateChanged() {}

u32 Button::getMillisecondsSinceButtonDown() const noexcept
{
    auto now = Time::getApproximateMillisecondCounter();
    return now > buttonPressTime ? now - buttonPressTime : 0;
}

z0 Button::setTriggeredOnMouseDown (b8 isTriggeredOnMouseDown) noexcept
{
    triggerOnMouseDown = isTriggeredOnMouseDown;
}

b8 Button::getTriggeredOnMouseDown() const noexcept
{
    return triggerOnMouseDown;
}

//==============================================================================
z0 Button::clicked()
{
}

z0 Button::clicked (const ModifierKeys&)
{
    clicked();
}

enum { clickMessageId = 0x2f3f4f99 };

z0 Button::triggerClick()
{
    postCommandMessage (clickMessageId);
}

z0 Button::internalClickCallback (const ModifierKeys& modifiers)
{
    if (clickTogglesState)
    {
        const b8 shouldBeOn = (radioGroupId != 0 || ! lastToggleState);

        if (shouldBeOn != getToggleState())
        {
            setToggleState (shouldBeOn, sendNotification);
            return;
        }
    }

    sendClickMessage (modifiers);
}

z0 Button::flashButtonState()
{
    if (isEnabled())
    {
        needsToRelease = true;
        setState (buttonDown);
        callbackHelper->startTimer (100);
    }
}

z0 Button::handleCommandMessage (i32 commandId)
{
    if (commandId == clickMessageId)
    {
        if (isEnabled())
        {
            flashButtonState();
            internalClickCallback (ModifierKeys::currentModifiers);
        }
    }
    else
    {
        Component::handleCommandMessage (commandId);
    }
}

//==============================================================================
z0 Button::addListener (Listener* l)      { buttonListeners.add (l); }
z0 Button::removeListener (Listener* l)   { buttonListeners.remove (l); }

z0 Button::sendClickMessage (const ModifierKeys& modifiers)
{
    Component::BailOutChecker checker (this);

    if (commandManagerToUse != nullptr && commandID != 0)
    {
        ApplicationCommandTarget::InvocationInfo info (commandID);
        info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromButton;
        info.originatingComponent = this;

        commandManagerToUse->invoke (info, true);
    }

    clicked (modifiers);

    if (checker.shouldBailOut())
        return;

    buttonListeners.callChecked (checker, [this] (Listener& l) { l.buttonClicked (this); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onClick);
}

z0 Button::sendStateMessage()
{
    Component::BailOutChecker checker (this);

    buttonStateChanged();

    if (checker.shouldBailOut())
        return;

    buttonListeners.callChecked (checker, [this] (Listener& l) { l.buttonStateChanged (this); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onStateChange);
}

//==============================================================================
z0 Button::paint (Graphics& g)
{
    if (needsToRelease && isEnabled())
    {
        needsToRelease = false;
        needsRepainting = true;
    }

    paintButton (g, isOver(), isDown());
    lastStatePainted = buttonState;
}

//==============================================================================
z0 Button::mouseEnter (const MouseEvent&)     { updateState (true,  false); }
z0 Button::mouseExit (const MouseEvent&)      { updateState (false, false); }

z0 Button::mouseDown (const MouseEvent& e)
{
    updateState (true, true);

    if (isDown())
    {
        if (autoRepeatDelay >= 0)
            callbackHelper->startTimer (autoRepeatDelay);

        if (triggerOnMouseDown)
            internalClickCallback (e.mods);
    }
}

z0 Button::mouseUp (const MouseEvent& e)
{
    const auto wasDown = isDown();
    const auto wasOver = isOver();
    updateState (isMouseSourceOver (e), false);

    if (wasDown && wasOver && ! triggerOnMouseDown)
    {
        if (lastStatePainted != buttonDown)
            flashButtonState();

        WeakReference<Component> deletionWatcher (this);

        internalClickCallback (e.mods);

        if (deletionWatcher != nullptr)
            updateState (isMouseSourceOver (e), false);
    }
}

z0 Button::mouseDrag (const MouseEvent& e)
{
    auto oldState = buttonState;
    updateState (isMouseSourceOver (e), true);

    if (autoRepeatDelay >= 0 && buttonState != oldState && isDown())
        callbackHelper->startTimer (autoRepeatSpeed);
}

b8 Button::isMouseSourceOver (const MouseEvent& e)
{
    if (e.source.isTouch() || e.source.isPen())
        return getLocalBounds().toFloat().contains (e.position);

    return isMouseOver();
}

z0 Button::focusGained (FocusChangeType)
{
    updateState();
    repaint();
}

z0 Button::focusLost (FocusChangeType)
{
    updateState();
    repaint();
}

z0 Button::visibilityChanged()
{
    needsToRelease = false;
    updateState();
}

z0 Button::parentHierarchyChanged()
{
    auto* newKeySource = shortcuts.isEmpty() ? nullptr : getTopLevelComponent();

    if (newKeySource != keySource.get())
    {
        if (keySource != nullptr)
            keySource->removeKeyListener (callbackHelper.get());

        keySource = newKeySource;

        if (keySource != nullptr)
            keySource->addKeyListener (callbackHelper.get());
    }
}

//==============================================================================
z0 Button::setCommandToTrigger (ApplicationCommandManager* newCommandManager,
                                  CommandID newCommandID, b8 generateTip)
{
    commandID = newCommandID;
    generateTooltip = generateTip;

    if (commandManagerToUse != newCommandManager)
    {
        if (commandManagerToUse != nullptr)
            commandManagerToUse->removeListener (callbackHelper.get());

        commandManagerToUse = newCommandManager;

        if (commandManagerToUse != nullptr)
            commandManagerToUse->addListener (callbackHelper.get());

        // if you've got clickTogglesState turned on, you shouldn't also connect the button
        // up to be a command invoker. Instead, your command handler must flip the state of whatever
        // it is that this button represents, and the button will update its state to reflect this
        // in the applicationCommandListChanged() method.
        jassert (commandManagerToUse == nullptr || ! clickTogglesState);
    }

    if (commandManagerToUse != nullptr)
        applicationCommandListChangeCallback();
    else
        setEnabled (true);
}

z0 Button::applicationCommandListChangeCallback()
{
    if (commandManagerToUse != nullptr)
    {
        ApplicationCommandInfo info (0);

        if (commandManagerToUse->getTargetForCommand (commandID, info) != nullptr)
        {
            updateAutomaticTooltip (info);
            setEnabled ((info.flags & ApplicationCommandInfo::isDisabled) == 0);
            setToggleState ((info.flags & ApplicationCommandInfo::isTicked) != 0, dontSendNotification);
        }
        else
        {
            setEnabled (false);
        }
    }
}

//==============================================================================
z0 Button::addShortcut (const KeyPress& key)
{
    if (key.isValid())
    {
        jassert (! isRegisteredForShortcut (key));  // already registered!

        shortcuts.add (key);
        parentHierarchyChanged();
    }
}

z0 Button::clearShortcuts()
{
    shortcuts.clear();
    parentHierarchyChanged();
}

b8 Button::isShortcutPressed() const
{
    if (isShowing() && ! isCurrentlyBlockedByAnotherModalComponent())
        for (auto& s : shortcuts)
            if (s.isCurrentlyDown())
                return true;

    return false;
}

b8 Button::isRegisteredForShortcut (const KeyPress& key) const
{
    for (auto& s : shortcuts)
        if (key == s)
            return true;

    return false;
}

b8 Button::keyStateChangedCallback()
{
    if (! isEnabled())
        return false;

    const b8 wasDown = isKeyDown;
    isKeyDown = isShortcutPressed();

    if (autoRepeatDelay >= 0 && (isKeyDown && ! wasDown))
        callbackHelper->startTimer (autoRepeatDelay);

    updateState();

    if (isEnabled() && wasDown && ! isKeyDown)
    {
        internalClickCallback (ModifierKeys::currentModifiers);

        // (return immediately - this button may now have been deleted)
        return true;
    }

    return wasDown || isKeyDown;
}

b8 Button::keyPressed (const KeyPress& key)
{
    if (isEnabled() && key.isKeyCode (KeyPress::returnKey))
    {
        triggerClick();
        return true;
    }

    return false;
}

//==============================================================================
z0 Button::setRepeatSpeed (i32 initialDelayMillisecs,
                             i32 repeatMillisecs,
                             i32 minimumDelayInMillisecs) noexcept
{
    autoRepeatDelay = initialDelayMillisecs;
    autoRepeatSpeed = repeatMillisecs;
    autoRepeatMinimumDelay = jmin (autoRepeatSpeed, minimumDelayInMillisecs);
}

z0 Button::repeatTimerCallback()
{
    if (needsRepainting)
    {
        callbackHelper->stopTimer();
        updateState();
        needsRepainting = false;
    }
    else if (autoRepeatSpeed > 0 && (isKeyDown || (updateState() == buttonDown)))
    {
        auto repeatSpeed = autoRepeatSpeed;

        if (autoRepeatMinimumDelay >= 0)
        {
            auto timeHeldDown = jmin (1.0, getMillisecondsSinceButtonDown() / 4000.0);
            timeHeldDown *= timeHeldDown;

            repeatSpeed = repeatSpeed + (i32) (timeHeldDown * (autoRepeatMinimumDelay - repeatSpeed));
        }

        repeatSpeed = jmax (1, repeatSpeed);

        auto now = Time::getMillisecondCounter();

        // if we've been blocked from repeating often enough, speed up the repeat timer to compensate..
        if (lastRepeatTime != 0 && (i32) (now - lastRepeatTime) > repeatSpeed * 2)
            repeatSpeed = jmax (1, repeatSpeed / 2);

        lastRepeatTime = now;
        callbackHelper->startTimer (repeatSpeed);

        internalClickCallback (ModifierKeys::currentModifiers);
    }
    else if (! needsToRelease)
    {
        callbackHelper->stopTimer();
    }
}

std::unique_ptr<AccessibilityHandler> Button::createAccessibilityHandler()
{
    return std::make_unique<detail::ButtonAccessibilityHandler> (*this, AccessibilityRole::button);
}

} // namespace drx
