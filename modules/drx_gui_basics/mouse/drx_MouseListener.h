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
    A MouseListener can be registered with a component to receive callbacks
    about mouse events that happen to that component.

    @see Component::addMouseListener, Component::removeMouseListener

    @tags{GUI}
*/
class DRX_API  MouseListener
{
public:
    /** Destructor. */
    virtual ~MouseListener() = default;

    /** Called when the mouse moves inside a component.

        If the mouse button isn't pressed and the mouse moves over a component,
        this will be called to let the component react to this.

        A component will always get a mouseEnter callback before a mouseMove.

        @param event details about the position and status of the mouse event, including
                     the source component in which it occurred
        @see mouseEnter, mouseExit, mouseDrag, contains
    */
    virtual z0 mouseMove (const MouseEvent& event);

    /** Called when the mouse first enters a component.

        If the mouse button isn't pressed and the mouse moves into a component,
        this will be called to let the component react to this.

        When the mouse button is pressed and held down while being moved in
        or out of a component, no mouseEnter or mouseExit callbacks are made - only
        mouseDrag messages are sent to the component that the mouse was originally
        clicked on, until the button is released.

        @param event details about the position and status of the mouse event, including
                     the source component in which it occurred
        @see mouseExit, mouseDrag, mouseMove, contains
    */
    virtual z0 mouseEnter (const MouseEvent& event);

    /** Called when the mouse moves out of a component.

        This will be called when the mouse moves off the edge of this
        component.

        If the mouse button was pressed, and it was then dragged off the
        edge of the component and released, then this callback will happen
        when the button is released, after the mouseUp callback.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseEnter, mouseDrag, mouseMove, contains
    */
    virtual z0 mouseExit (const MouseEvent& event);

    /** Called when a mouse button is pressed.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        Once a button is held down, the mouseDrag method will be called when the
        mouse moves, until the button is released.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseUp, mouseDrag, mouseDoubleClick, contains
    */
    virtual z0 mouseDown (const MouseEvent& event);

    /** Called when the mouse is moved while a button is held down.

        When a mouse button is pressed inside a component, that component
        receives mouseDrag callbacks each time the mouse moves, even if the
        mouse strays outside the component's bounds.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseDown, mouseUp, mouseMove, contains, setDragRepeatInterval
    */
    virtual z0 mouseDrag (const MouseEvent& event);

    /** Called when a mouse button is released.

        A mouseUp callback is sent to the component in which a button was pressed
        even if the mouse is actually over a different component when the
        button is released.

        The MouseEvent object passed in contains lots of methods for finding out
        which buttons were down just before they were released.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseDown, mouseDrag, mouseDoubleClick, contains
    */
    virtual z0 mouseUp (const MouseEvent& event);

    /** Called when a mouse button has been f64-clicked on a component.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseDown, mouseUp
    */
    virtual z0 mouseDoubleClick (const MouseEvent& event);

    /** Called when the mouse-wheel is moved.

        This callback is sent to the component that the mouse is over when the
        wheel is moved.

        If not overridden, a component will forward this message to its parent, so
        that parent components can collect mouse-wheel messages that happen to
        child components which aren't interested in them.

        @param event   details about the mouse event
        @param wheel   details about the wheel movement
    */
    virtual z0 mouseWheelMove (const MouseEvent& event,
                                 const MouseWheelDetails& wheel);

    /** Called when a pinch-to-zoom mouse-gesture is used.

        If not overridden, a component will forward this message to its parent, so
        that parent components can collect gesture messages that are unused by child
        components.

        @param event   details about the mouse event
        @param scaleFactor  a multiplier to indicate by how much the size of the target
                            should be changed. A value of 1.0 would indicate no change,
                            values greater than 1.0 mean it should be enlarged.
    */
    virtual z0 mouseMagnify (const MouseEvent& event, f32 scaleFactor);
};

} // namespace drx
