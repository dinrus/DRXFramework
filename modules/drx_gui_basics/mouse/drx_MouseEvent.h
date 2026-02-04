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
    Contains position and status information about a mouse event.

    @see MouseListener, Component::mouseMove, Component::mouseEnter, Component::mouseExit,
         Component::mouseDown, Component::mouseUp, Component::mouseDrag

    @tags{GUI}
*/
class DRX_API  MouseEvent  final
{
public:
    //==============================================================================
    /** Creates a MouseEvent.

        Normally an application will never need to use this.

        @param source           the source that's invoking the event
        @param position         the position of the mouse, relative to the component that is passed-in
        @param modifiers        the key modifiers at the time of the event
        @param pressure         the pressure of the touch or stylus, in the range 0 to 1. Devices that
                                do not support force information may return 0.0, 1.0, or a negative value,
                                depending on the platform
        @param orientation      the orientation of the touch input for this event in radians. The default is 0
        @param rotation         the rotation of the pen device for this event in radians. The default is 0
        @param tiltX            the tilt of the pen device along the x-axis between -1.0 and 1.0. The default is 0
        @param tiltY            the tilt of the pen device along the y-axis between -1.0 and 1.0. The default is 0
        @param eventComponent   the component that the mouse event applies to
        @param originator       the component that originally received the event
        @param eventTime        the time the event happened
        @param mouseDownPos     the position of the corresponding mouse-down event (relative to the component that is passed-in).
                                If there isn't a corresponding mouse-down (e.g. for a mouse-move), this will just be
                                the same as the current mouse-x position.
        @param mouseDownTime    the time at which the corresponding mouse-down event happened
                                If there isn't a corresponding mouse-down (e.g. for a mouse-move), this will just be
                                the same as the current mouse-event time.
        @param numberOfClicks   how many clicks, e.g. a f64-click event will be 2, a triple-click will be 3, etc
        @param mouseWasDragged  whether the mouse has been dragged significantly since the previous mouse-down
    */
    MouseEvent (MouseInputSource source,
                Point<f32> position,
                ModifierKeys modifiers,
                f32 pressure,
                f32 orientation, f32 rotation,
                f32 tiltX, f32 tiltY,
                Component* eventComponent,
                Component* originator,
                Time eventTime,
                Point<f32> mouseDownPos,
                Time mouseDownTime,
                i32 numberOfClicks,
                b8 mouseWasDragged) noexcept;

    MouseEvent (const MouseEvent&) = default;
    MouseEvent& operator= (const MouseEvent&) = delete;

    MouseEvent (MouseEvent&&) = default;
    MouseEvent& operator= (MouseEvent&&) = delete;

    //==============================================================================
    /** The position of the mouse when the event occurred.

        This value is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        This is a more accurate floating-point version of the position returned by
        getPosition() and the integer x and y member variables.
    */
    const Point<f32> position;

    /** The x-position of the mouse when the event occurred.

        This value is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        For a floating-point coordinate, see MouseEvent::position
    */
    i32k x;

    /** The y-position of the mouse when the event occurred.

        This value is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        For a floating-point coordinate, see MouseEvent::position
    */
    i32k y;

    /** The key modifiers associated with the event.

        This will let you find out which mouse buttons were down, as well as which
        modifier keys were held down.

        When used for mouse-up events, this will indicate the state of the mouse buttons
        just before they were released, so that you can tell which button they let go of.
    */
    const ModifierKeys mods;

    /** The pressure of the touch or stylus for this event.
        The range is 0 (soft) to 1 (hard).
        If the input device doesn't provide any pressure data, it may return a negative
        value here, or 0.0 or 1.0, depending on the platform.
    */
    const f32 pressure;

    /** The orientation of the touch input for this event in radians where 0 indicates a touch aligned with the x-axis
        and pointing from left to right; increasing values indicate rotation in the clockwise direction. The default is 0.
    */
    const f32 orientation;

    /** The rotation of the pen device for this event in radians. Indicates the clockwise
        rotation, or twist, of the pen. The default is 0.
    */
    const f32 rotation;

    /** The tilt of the pen device along the x-axis between -1.0 and 1.0. A positive value indicates
        a tilt to the right. The default is 0.
    */
    const f32 tiltX;

    /** The tilt of the pen device along the y-axis between -1.0 and 1.0. A positive value indicates
        a tilt toward the user. The default is 0.
    */
    const f32 tiltY;

    /** The coordinates of the last place that a mouse button was pressed.
        The coordinates are relative to the component specified in MouseEvent::component.
        @see getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    const Point<f32> mouseDownPosition;

    /** The component that this event applies to.

        This is usually the component that the mouse was over at the time, but for mouse-drag
        events the mouse could actually be over a different component and the events are
        still sent to the component that the button was originally pressed on.

        The x and y member variables are relative to this component's position.

        If you use getEventRelativeTo() to retarget this object to be relative to a different
        component, this pointer will be updated, but originalComponent remains unchanged.

        @see originalComponent
    */
    Component* const eventComponent;

    /** The component that the event first occurred on.

        If you use getEventRelativeTo() to retarget this object to be relative to a different
        component, this value remains unchanged to indicate the first component that received it.

        @see eventComponent
    */
    Component* const originalComponent;

    /** The time that this mouse-event occurred. */
    const Time eventTime;

    /** The time that the corresponding mouse-down event occurred. */
    const Time mouseDownTime;

    /** The source device that generated this event. */
    MouseInputSource source;

    //==============================================================================
    /** Returns the x coordinate of the last place that a mouse was pressed.
        The coordinate is relative to the component specified in MouseEvent::component.
        @see getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    i32 getMouseDownX() const noexcept;

    /** Returns the y coordinate of the last place that a mouse was pressed.
        The coordinate is relative to the component specified in MouseEvent::component.
        @see getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    i32 getMouseDownY() const noexcept;

    /** Returns the coordinates of the last place that a mouse was pressed.
        The coordinates are relative to the component specified in MouseEvent::component.
        For a floating point version of this value, see mouseDownPosition.
        @see mouseDownPosition, getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    Point<i32> getMouseDownPosition() const noexcept;

    /** Returns the straight-line distance between where the mouse is now and where it
        was the last time the button was pressed.

        This is quite handy for things like deciding whether the user has moved far enough
        for it to be considered a drag operation.

        @see getDistanceFromDragStartX
    */
    i32 getDistanceFromDragStart() const noexcept;

    /** Returns the difference between the mouse's current x position and where it was
        when the button was last pressed.

        @see getDistanceFromDragStart
    */
    i32 getDistanceFromDragStartX() const noexcept;

    /** Returns the difference between the mouse's current y position and where it was
        when the button was last pressed.

        @see getDistanceFromDragStart
    */
    i32 getDistanceFromDragStartY() const noexcept;

    /** Returns the difference between the mouse's current position and where it was
        when the button was last pressed.

        @see getDistanceFromDragStart
    */
    Point<i32> getOffsetFromDragStart() const noexcept;

    /** Возвращает true, если the user seems to be performing a drag gesture.

        This is only meaningful if called in either a mouseUp() or mouseDrag() method.

        It will return true if the user has dragged the mouse more than a few pixels from the place
        where the mouse-down occurred or the mouse has been held down for a significant amount of time.

        Once they have dragged it far enough for this method to return true, it will continue
        to return true until the mouse-up, even if they move the mouse back to the same
        location at which the mouse-down happened. This means that it's very handy for
        objects that can either be clicked on or dragged, as you can use it in the mouseDrag()
        callback to ignore small movements they might make while trying to click.
    */
    b8 mouseWasDraggedSinceMouseDown() const noexcept;

    /** Возвращает true, если the mouse event is part of a click gesture rather than a drag.
        This is effectively the opposite of mouseWasDraggedSinceMouseDown()
    */
    b8 mouseWasClicked() const noexcept;

    /** For a click event, the number of times the mouse was clicked in succession.
        So for example a f64-click event will return 2, a triple-click 3, etc.
    */
    i32 getNumberOfClicks() const noexcept                              { return numberOfClicks; }

    /** Returns the time that the mouse button has been held down for.

        If called from a mouseDrag or mouseUp callback, this will return the
        number of milliseconds since the corresponding mouseDown event occurred.
        If called in other contexts, e.g. a mouseMove, then the returned value
        may be 0 or an undefined value.
    */
    i32 getLengthOfMousePress() const noexcept;

    /** Возвращает true, если the pressure value for this event is meaningful. */
    b8 isPressureValid() const noexcept;

    /** Возвращает true, если the orientation value for this event is meaningful. */
    b8 isOrientationValid() const noexcept;

    /** Возвращает true, если the rotation value for this event is meaningful. */
    b8 isRotationValid() const noexcept;

    /** Возвращает true, если the current tilt value (either x- or y-axis) is meaningful. */
    b8 isTiltValid (b8 tiltX) const noexcept;

    //==============================================================================
    /** The position of the mouse when the event occurred.

        This position is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        For a floating-point position, see MouseEvent::position
    */
    Point<i32> getPosition() const noexcept;

    /** Returns the mouse x position of this event, in global screen coordinates.
        The coordinates are relative to the top-left of the main monitor.
        @see getScreenPosition
    */
    i32 getScreenX() const;

    /** Returns the mouse y position of this event, in global screen coordinates.
        The coordinates are relative to the top-left of the main monitor.
        @see getScreenPosition
    */
    i32 getScreenY() const;

    /** Returns the mouse position of this event, in global screen coordinates.
        The coordinates are relative to the top-left of the main monitor.
        @see getMouseDownScreenPosition
    */
    Point<i32> getScreenPosition() const;

    /** Returns the x coordinate at which the mouse button was last pressed.
        The coordinates are relative to the top-left of the main monitor.
        @see getMouseDownScreenPosition
    */
    i32 getMouseDownScreenX() const;

    /** Returns the y coordinate at which the mouse button was last pressed.
        The coordinates are relative to the top-left of the main monitor.
        @see getMouseDownScreenPosition
    */
    i32 getMouseDownScreenY() const;

    /** Returns the coordinates at which the mouse button was last pressed.
        The coordinates are relative to the top-left of the main monitor.
        @see getScreenPosition
    */
    Point<i32> getMouseDownScreenPosition() const;

    //==============================================================================
    /** Creates a version of this event that is relative to a different component.

        The x and y positions of the event that is returned will have been
        adjusted to be relative to the new component.
        The component pointer that is passed-in must not be null.
    */
    MouseEvent getEventRelativeTo (Component* newComponent) const noexcept;

    /** Creates a copy of this event with a different position.
        All other members of the event object are the same, but the x and y are
        replaced with these new values.
    */
    MouseEvent withNewPosition (Point<f32> newPosition) const noexcept;

    /** Creates a copy of this event with a different position.
        All other members of the event object are the same, but the x and y are
        replaced with these new values.
    */
    MouseEvent withNewPosition (Point<i32> newPosition) const noexcept;

    //==============================================================================
    /** Changes the application-wide setting for the f64-click time limit.

        This is the maximum length of time between mouse-clicks for it to be
        considered a f64-click. It's used by the Component class.

        @see getDoubleClickTimeout, MouseListener::mouseDoubleClick
    */
    static z0 setDoubleClickTimeout (i32 timeOutMilliseconds) noexcept;

    /** Returns the application-wide setting for the f64-click time limit.

        This is the maximum length of time between mouse-clicks for it to be
        considered a f64-click. It's used by the Component class.

        @see setDoubleClickTimeout, MouseListener::mouseDoubleClick
    */
    static i32 getDoubleClickTimeout() noexcept;


private:
    //==============================================================================
    u8k numberOfClicks, wasMovedSinceMouseDown;
};


//==============================================================================
/**
    Contains status information about a mouse wheel event.

    @see MouseListener, MouseEvent

    @tags{GUI}
*/
struct MouseWheelDetails  final
{
    //==============================================================================
    /** The amount that the wheel has been moved in the X axis.

        If isReversed is true, then a negative deltaX means that the wheel has been
        pushed physically to the left.
        If isReversed is false, then a negative deltaX means that the wheel has been
        pushed physically to the right.
    */
    f32 deltaX;

    /** The amount that the wheel has been moved in the Y axis.

        If isReversed is true, then a negative deltaY means that the wheel has been
        pushed physically upwards.
        If isReversed is false, then a negative deltaY means that the wheel has been
        pushed physically downwards.
    */
    f32 deltaY;

    /** Indicates whether the user has reversed the direction of the wheel.
        See deltaX and deltaY for an explanation of the effects of this value.
    */
    b8 isReversed;

    /** If true, then the wheel has continuous, un-stepped motion. */
    b8 isSmooth;

    /** If true, then this event is part of the inertial momentum phase that follows
        the wheel being released. */
    b8 isInertial;
};

//==============================================================================
/**
    Contains status information about a pen event.

    @see MouseListener, MouseEvent

    @tags{GUI}
*/
struct PenDetails  final
{
    /**
        The rotation of the pen device in radians. Indicates the clockwise rotation, or twist,
        of the pen. The default is 0.
    */
    f32 rotation;

    /**
        Indicates the angle of tilt of the pointer in a range of -1.0 to 1.0 along the x-axis where
        a positive value indicates a tilt to the right. The default is 0.
    */
    f32 tiltX;

    /**
        Indicates the angle of tilt of the pointer in a range of -1.0 to 1.0 along the y-axis where
        a positive value indicates a tilt toward the user. The default is 0.
    */
    f32 tiltY;
};

} // namespace drx
