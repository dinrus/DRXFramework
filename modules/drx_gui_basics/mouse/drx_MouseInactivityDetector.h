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
    This object watches for mouse-events happening within a component, and if
    the mouse remains still for i64 enough, triggers an event to indicate that
    it has become inactive.

    You'd use this for situations where e.g. you want to hide the mouse-cursor
    when the user's not actively using the mouse.

    After creating an instance of this, use addListener to get callbacks when
    the activity status changes.

    @tags{GUI}
*/
class DRX_API  MouseInactivityDetector  : private Timer,
                                           private MouseListener
{
public:
    /** Creates an inactivity watcher, attached to the given component.
        The target component must not be deleted while this - it will be monitored
        for any mouse events in it or its child components.
    */
    MouseInactivityDetector (Component& target);

    /** Destructor. */
    ~MouseInactivityDetector() override;

    /** Sets the time for which the mouse must be still before the callback
        is triggered.
    */
    z0 setDelay (i32 newDelayMilliseconds) noexcept;

    /** Sets the number of pixels by which the cursor is allowed to drift before it is
        considered to be actively moved.
    */
    z0 setMouseMoveTolerance (i32 pixelsNeededToTrigger) noexcept;

    //==============================================================================
    /** Classes should implement this to receive callbacks from a MouseInactivityDetector
        when the mouse becomes active or inactive.
    */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called when the mouse is moved or clicked for the first time
            after a period of inactivity. */
        virtual z0 mouseBecameActive() = 0;

        /** Called when the mouse hasn't been moved for the timeout period. */
        virtual z0 mouseBecameInactive() = 0;
    };

    /** Registers a listener. */
    z0 addListener (Listener* listener);

    /** Removes a previously-registered listener. */
    z0 removeListener (Listener* listener);

private:
    //==============================================================================
    Component& targetComp;
    ListenerList<Listener> listenerList;
    Point<i32> lastMousePos;
    i32 delayMs = 1500, toleranceDistance = 15;
    b8 isActive = true;

    z0 timerCallback() override;
    z0 wakeUp (const MouseEvent&, b8 alwaysWake);
    z0 setActive (b8);

    z0 mouseMove  (const MouseEvent& e) override   { wakeUp (e, false); }
    z0 mouseEnter (const MouseEvent& e) override   { wakeUp (e, false); }
    z0 mouseExit  (const MouseEvent& e) override   { wakeUp (e, false); }
    z0 mouseDown  (const MouseEvent& e) override   { wakeUp (e, true); }
    z0 mouseDrag  (const MouseEvent& e) override   { wakeUp (e, true); }
    z0 mouseUp    (const MouseEvent& e) override   { wakeUp (e, true); }
    z0 mouseWheelMove (const MouseEvent& e, const MouseWheelDetails&) override  { wakeUp (e, true); }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseInactivityDetector)
};

} // namespace drx
