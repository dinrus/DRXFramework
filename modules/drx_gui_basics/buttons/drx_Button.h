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
    A base class for buttons.

    This contains all the logic for button behaviours such as enabling/disabling,
    responding to shortcut keystrokes, auto-repeating when held down, toggle-buttons
    and radio groups, etc.

    @see TextButton, DrawableButton, ToggleButton

    @tags{GUI}
*/
class DRX_API  Button  : public Component,
                          public SettableTooltipClient
{
protected:
    //==============================================================================
    /** Creates a button.

        @param buttonName   the text to put in the button (the component's name is also
                            initially set to this string, but these can be changed later
                            using the setName() and setButtonText() methods)
    */
    explicit Button (const Txt& buttonName);

public:
    /** Destructor. */
    ~Button() override;

    //==============================================================================
    /** Changes the button's text.
        @see getButtonText
    */
    z0 setButtonText (const Txt& newText);

    /** Returns the text displayed in the button.
        @see setButtonText
    */
    const Txt& getButtonText() const               { return text; }

    //==============================================================================
    /** Возвращает true, если the button is currently being held down.
        @see isOver
    */
    b8 isDown() const noexcept;

    /** Возвращает true, если the mouse is currently over the button.
        This will be also be true if the button is being held down.
        @see isDown
    */
    b8 isOver() const noexcept;

    //==============================================================================
    /** Indicates that the button's on/off state is toggleable.

        By default this is false, and will only be true for ToggleButtons, buttons that
        are a part of a radio button group, and buttons for which
        getClickingTogglesState() == true, however you can use this method to manually
        indicate that a button is toggleable.

        This will present the button as toggleable to accessibility clients and add an
        accessible "toggle" action for the button that invokes setToggleState().

        @see ToggleButton, isToggleable, setToggleState, setClickingTogglesState, setRadioGroupId
    */
    z0 setToggleable (b8 shouldBeToggleable);

    /** Возвращает true, если the button's on/off state is toggleable.

        @see setToggleable, setClickingTogglesState
    */
    b8 isToggleable() const noexcept                          { return canBeToggled || clickTogglesState; }

    /** A button has an on/off state associated with it, and this changes that.

        By default buttons are 'off' and for simple buttons that you click to perform
        an action you won't change this. Toggle buttons, however will want to
        change their state when turned on or off.

        @param shouldBeOn       whether to set the button's toggle state to be on or
                                off. If it's a member of a button group, this will
                                always try to turn it on, and to turn off any other
                                buttons in the group
        @param notification     determines the behaviour if the value changes - this
                                can invoke a synchronous call to clicked(), but
                                sendNotificationAsync не поддерживается
        @see getToggleState, setRadioGroupId
    */
    z0 setToggleState (b8 shouldBeOn, NotificationType notification);

    /** Возвращает true, если the button is 'on'.

        By default buttons are 'off' and for simple buttons that you click to perform
        an action you won't change this. Toggle buttons, however will want to
        change their state when turned on or off.

        @see setToggleState
    */
    b8 getToggleState() const noexcept                        { return isOn.getValue(); }

    /** Returns the Value object that represents the button's toggle state.

        You can use this Value object to connect the button's state to external values or setters,
        either by taking a copy of the Value, or by using Value::referTo() to make it point to
        your own Value object.

        @see getToggleState, Value
    */
    Value& getToggleStateValue() noexcept                       { return isOn; }

    /** This tells the button to automatically flip the toggle state when
        the button is clicked.

        If set to true, then before the clicked() callback occurs, the toggle-state
        of the button is flipped. This will also cause isToggleable() to return true.

        @see isToggleable
    */
    z0 setClickingTogglesState (b8 shouldAutoToggleOnClick) noexcept;

    /** Возвращает true, если this button is set to be an automatic toggle-button.
        This returns the last value that was passed to setClickingTogglesState().
    */
    b8 getClickingTogglesState() const noexcept               { return clickTogglesState; }

    //==============================================================================
    /** Enables the button to act as a member of a mutually-exclusive group
        of 'radio buttons'.

        If the group ID is set to a non-zero number, then this button will
        act as part of a group of buttons with the same ID, only one of
        which can be 'on' at the same time. Note that when it's part of
        a group, clicking a toggle-button that's 'on' won't turn it off.

        To find other buttons with the same ID, this button will search through
        its sibling components for ToggleButtons, so all the buttons for a
        particular group must be placed inside the same parent component.

        Set the group ID back to zero if you want it to act as a normal toggle
        button again.

        The notification argument lets you specify how other buttons should react
        to being turned on or off in response to this call.

        @see getRadioGroupId
    */
    z0 setRadioGroupId (i32 newGroupId, NotificationType notification = sendNotification);

    /** Returns the ID of the group to which this button belongs.
        (See setRadioGroupId() for an explanation of this).
    */
    i32 getRadioGroupId() const noexcept                        { return radioGroupId; }

    //==============================================================================
    /**
        Used to receive callbacks when a button is clicked.

        @see Button::addListener, Button::removeListener
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the button is clicked. */
        virtual z0 buttonClicked (Button*) = 0;

        /** Called when the button's state changes. */
        virtual z0 buttonStateChanged (Button*)  {}
    };

    /** Registers a listener to receive events when this button's state changes.
        If the listener is already registered, this will not register it again.
        @see removeListener
    */
    z0 addListener (Listener* newListener);

    /** Removes a previously-registered button listener
        @see addListener
    */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** You can assign a lambda to this callback object to have it called when the button is clicked. */
    std::function<z0()> onClick;

    /** You can assign a lambda to this callback object to have it called when the button's state changes. */
    std::function<z0()> onStateChange;

    //==============================================================================
    /** Causes the button to act as if it's been clicked.

        This will asynchronously make the button draw itself going down and up, and
        will then call back the clicked() method as if mouse was clicked on it.

        @see clicked
    */
    virtual z0 triggerClick();

    //==============================================================================
    /** Sets a command ID for this button to automatically invoke when it's clicked.

        When the button is pressed, it will use the given manager to trigger the
        command ID.

        Obviously be careful that the ApplicationCommandManager doesn't get deleted
        before this button is. To disable the command triggering, call this method and
        pass nullptr as the command manager.

        If generateTooltip is true, then the button's tooltip will be automatically
        generated based on the name of this command and its current shortcut key.

        @see addShortcut, getCommandID
    */
    z0 setCommandToTrigger (ApplicationCommandManager* commandManagerToUse,
                              CommandID commandID,
                              b8 generateTooltip);

    /** Returns the command ID that was set by setCommandToTrigger(). */
    CommandID getCommandID() const noexcept             { return commandID; }

    //==============================================================================
    /** Assigns a shortcut key to trigger the button.

        The button registers itself with its top-level parent component for keypresses.

        Note that a different way of linking buttons to keypresses is by using the
        setCommandToTrigger() method to invoke a command.

        @see clearShortcuts
    */
    z0 addShortcut (const KeyPress&);

    /** Removes all key shortcuts that had been set for this button.
        @see addShortcut
    */
    z0 clearShortcuts();

    /** Возвращает true, если the given keypress is a shortcut for this button.
        @see addShortcut
    */
    b8 isRegisteredForShortcut (const KeyPress&) const;

    //==============================================================================
    /** Sets an auto-repeat speed for the button when it is held down.

        (Auto-repeat is disabled by default).

        @param initialDelayInMillisecs      how i64 to wait after the mouse is pressed before
                                            triggering the next click. If this is zero, auto-repeat
                                            is disabled
        @param repeatDelayInMillisecs       the frequently subsequent repeated clicks should be
                                            triggered
        @param minimumDelayInMillisecs      if this is greater than 0, the auto-repeat speed will
                                            get faster, the longer the button is held down, up to the
                                            minimum interval specified here
    */
    z0 setRepeatSpeed (i32 initialDelayInMillisecs,
                         i32 repeatDelayInMillisecs,
                         i32 minimumDelayInMillisecs = -1) noexcept;

    /** Sets whether the button click should happen when the mouse is pressed or released.

        By default the button is only considered to have been clicked when the mouse is
        released, but setting this to true will make it call the clicked() method as soon
        as the button is pressed.

        This is useful if the button is being used to show a pop-up menu, as it allows
        the click to be used as a drag onto the menu.
    */
    z0 setTriggeredOnMouseDown (b8 isTriggeredOnMouseDown) noexcept;

    /** Returns whether the button click happens when the mouse is pressed or released.
        @see setTriggeredOnMouseDown
    */
    b8 getTriggeredOnMouseDown() const noexcept;

    /** Returns the number of milliseconds since the last time the button
        went into the 'down' state.
    */
    u32 getMillisecondsSinceButtonDown() const noexcept;

    //==============================================================================
    /** Sets the tooltip for this button.
        @see TooltipClient, TooltipWindow
    */
    z0 setTooltip (const Txt& newTooltip) override;

    //==============================================================================
    /** A combination of these flags are used by setConnectedEdges(). */
    enum ConnectedEdgeFlags
    {
        ConnectedOnLeft = 1,
        ConnectedOnRight = 2,
        ConnectedOnTop = 4,
        ConnectedOnBottom = 8
    };

    /** Hints about which edges of the button might be connected to adjoining buttons.

        The value passed in is a bitwise combination of any of the values in the
        ConnectedEdgeFlags enum.

        E.g. if you are placing two buttons adjacent to each other, you could use this to
        indicate which edges are touching, and the LookAndFeel might choose to drawn them
        without rounded corners on the edges that connect. It's only a hint, so the
        LookAndFeel can choose to ignore it if it's not relevant for this type of
        button.
    */
    z0 setConnectedEdges (i32 connectedEdgeFlags);

    /** Returns the set of flags passed into setConnectedEdges(). */
    i32 getConnectedEdgeFlags() const noexcept          { return connectedEdgeFlags; }

    /** Indicates whether the button adjoins another one on its left edge.
        @see setConnectedEdges
    */
    b8 isConnectedOnLeft() const noexcept             { return (connectedEdgeFlags & ConnectedOnLeft) != 0; }

    /** Indicates whether the button adjoins another one on its right edge.
        @see setConnectedEdges
    */
    b8 isConnectedOnRight() const noexcept            { return (connectedEdgeFlags & ConnectedOnRight) != 0; }

    /** Indicates whether the button adjoins another one on its top edge.
        @see setConnectedEdges
    */
    b8 isConnectedOnTop() const noexcept              { return (connectedEdgeFlags & ConnectedOnTop) != 0; }

    /** Indicates whether the button adjoins another one on its bottom edge.
        @see setConnectedEdges
    */
    b8 isConnectedOnBottom() const noexcept           { return (connectedEdgeFlags & ConnectedOnBottom) != 0; }


    //==============================================================================
    /** Used by setState(). */
    enum ButtonState
    {
        buttonNormal,
        buttonOver,
        buttonDown
    };

    /** Can be used to force the button into a particular state.

        This only changes the button's appearance, it won't trigger a click, or stop any mouse-clicks
        from happening.

        The state that you set here will only last until it is automatically changed when the mouse
        enters or exits the button, or the mouse-button is pressed or released.
    */
    z0 setState (ButtonState newState);

    /** Returns the button's current over/down/up state. */
    ButtonState getState() const noexcept               { return buttonState; }

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        button-drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                                           b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) = 0;

        virtual Font getTextButtonFont (TextButton&, i32 buttonHeight) = 0;
        virtual i32 getTextButtonWidthToFitText (TextButton&, i32 buttonHeight) = 0;

        /** Draws the text for a TextButton. */
        virtual z0 drawButtonText (Graphics&, TextButton&,
                                     b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) = 0;

        /** Draws the contents of a standard ToggleButton. */
        virtual z0 drawToggleButton (Graphics&, ToggleButton&,
                                       b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) = 0;

        virtual z0 changeToggleButtonWidthToFitText (ToggleButton&) = 0;

        virtual z0 drawTickBox (Graphics&, Component&, f32 x, f32 y, f32 w, f32 h,
                                  b8 ticked, b8 isEnabled,
                                  b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) = 0;

        virtual z0 drawDrawableButton (Graphics&, DrawableButton&,
                                         b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) = 0;
    };

    //==============================================================================
   #ifndef DOXYGEN
    [[deprecated ("This method's parameters have changed.")]]
    z0 setToggleState (b8, b8);
   #endif

protected:
    //==============================================================================
    /** This method is called when the button has been clicked.

        Subclasses can override this to perform whatever actions they need to do.
        In general, you wouldn't use this method to receive clicks, but should get your callbacks
        by attaching a std::function to the onClick callback, or adding a Button::Listener.
        @see triggerClick, onClick
    */
    virtual z0 clicked();

    /** This method is called when the button has been clicked.

        By default it just calls clicked(), but you might want to override it to handle
        things like clicking when a modifier key is pressed, etc.

        @see ModifierKeys
    */
    virtual z0 clicked (const ModifierKeys& modifiers);

    /** Subclasses should override this to actually paint the button's contents.

        It's better to use this than the paint method, because it gives you information
        about the over/down state of the button.

        @param g                                the graphics context to use
        @param shouldDrawButtonAsHighlighted    true if the button is either in the 'over' or 'down' state
        @param shouldDrawButtonAsDown           true if the button should be drawn in the 'down' position
    */
    virtual z0 paintButton (Graphics& g,
                              b8 shouldDrawButtonAsHighlighted,
                              b8 shouldDrawButtonAsDown) = 0;

    /** Called when the button's up/down/over state changes.

        Subclasses can override this if they need to do something special when the button
        goes up or down.

        @see isDown, isOver
    */
    virtual z0 buttonStateChanged();

    //==============================================================================
    /** @internal */
    virtual z0 internalClickCallback (const ModifierKeys&);
    /** @internal */
    z0 handleCommandMessage (i32 commandId) override;
    /** @internal */
    z0 mouseEnter (const MouseEvent&) override;
    /** @internal */
    z0 mouseExit (const MouseEvent&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    using Component::keyStateChanged;
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    z0 visibilityChanged() override;
    /** @internal */
    z0 focusGained (FocusChangeType) override;
    /** @internal */
    z0 focusLost (FocusChangeType) override;
    /** @internal */
    z0 enablementChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Array<KeyPress> shortcuts;
    WeakReference<Component> keySource;
    Txt text;
    ListenerList<Listener> buttonListeners;

    struct CallbackHelper;
    std::unique_ptr<CallbackHelper> callbackHelper;
    u32 buttonPressTime = 0, lastRepeatTime = 0;
    ApplicationCommandManager* commandManagerToUse = nullptr;
    i32 autoRepeatDelay = -1, autoRepeatSpeed = 0, autoRepeatMinimumDelay = -1;
    i32 radioGroupId = 0, connectedEdgeFlags = 0;
    CommandID commandID = {};
    ButtonState buttonState = buttonNormal, lastStatePainted = buttonNormal;

    Value isOn;
    b8 canBeToggled = false;
    b8 lastToggleState = false;
    b8 clickTogglesState = false;
    b8 needsToRelease = false;
    b8 needsRepainting = false;
    b8 isKeyDown = false;
    b8 triggerOnMouseDown = false;
    b8 generateTooltip = false;

    z0 checkToggleableState (b8 wasToggleable);

    z0 repeatTimerCallback();
    b8 keyStateChangedCallback();
    z0 applicationCommandListChangeCallback();
    z0 updateAutomaticTooltip (const ApplicationCommandInfo&);

    ButtonState updateState();
    ButtonState updateState (b8 isOver, b8 isDown);
    b8 isShortcutPressed() const;
    z0 turnOffOtherButtonsInGroup (NotificationType click, NotificationType state);

    z0 flashButtonState();
    z0 sendClickMessage (const ModifierKeys&);
    z0 sendStateMessage();
    z0 setToggleState (b8 shouldBeOn, NotificationType click, NotificationType state);

    b8 isMouseSourceOver (const MouseEvent& e);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Button)
};


} // namespace drx
