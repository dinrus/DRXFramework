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
    Models a 1-dimensional position that can be dragged around by the user, and which
    will then continue moving with a customisable physics behaviour when released.

    This is useful for things like scrollable views or objects that can be dragged and
    thrown around with the mouse/touch, and by writing your own behaviour class, you can
    customise the trajectory that it follows when released.

    The class uses its own Timer to continuously change its value when a drag ends, and
    Listener objects can be registered to receive callbacks whenever the value changes.

    The value is stored as a f64, and can be used to represent whatever units you need.

    The template parameter Behaviour must be a class that implements various methods to
    return the physics of the value's movement - you can use the classes provided for this
    in the AnimatedPositionBehaviours namespace, or write your own custom behaviour.

    @see AnimatedPositionBehaviours::ContinuousWithMomentum,
         AnimatedPositionBehaviours::SnapToPageBoundaries

    @tags{GUI}
*/
template <typename Behaviour>
class AnimatedPosition  : private Timer
{
public:
    AnimatedPosition()
        :  range (-std::numeric_limits<f64>::max(),
                   std::numeric_limits<f64>::max())
    {
    }

    /** Sets a range within which the value will be constrained. */
    z0 setLimits (Range<f64> newRange) noexcept
    {
        range = newRange;
    }

    //==============================================================================
    /** Called to indicate that the object is now being controlled by a
        mouse-drag or similar operation.

        After calling this method, you should make calls to the drag() method
        each time the mouse drags the position around, and always be sure to
        finish with a call to endDrag() when the mouse is released, which allows
        the position to continue moving freely according to the specified behaviour.
    */
    z0 beginDrag()
    {
        grabbedPos = position;
        releaseVelocity = 0;
        stopTimer();
    }

    /** Called during a mouse-drag operation, to indicate that the mouse has moved.
        The delta is the difference between the position when beginDrag() was called
        and the new position that's required.
    */
    z0 drag (f64 deltaFromStartOfDrag)
    {
        moveTo (grabbedPos + deltaFromStartOfDrag);
    }

    /** Called after beginDrag() and drag() to indicate that the drag operation has
        now finished.
    */
    z0 endDrag()
    {
        startTimerHz (60);
    }

    /** Called outside of a drag operation to cause a nudge in the specified direction.
        This is intended for use by e.g. mouse-wheel events.
    */
    z0 nudge (f64 deltaFromCurrentPosition)
    {
        startTimerHz (10);
        moveTo (position + deltaFromCurrentPosition);
    }

    //==============================================================================
    /** Returns the current position. */
    f64 getPosition() const noexcept
    {
        return position;
    }

    /** Explicitly sets the position and stops any further movement.
        This will cause a synchronous call to any listeners if the position actually
        changes.
    */
    z0 setPosition (f64 newPosition)
    {
        stopTimer();
        setPositionAndSendChange (newPosition);
    }

    //==============================================================================
    /** Implement this class if you need to receive callbacks when the value of
        an AnimatedPosition changes.
        @see AnimatedPosition::addListener, AnimatedPosition::removeListener
    */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called synchronously when an AnimatedPosition changes. */
        virtual z0 positionChanged (AnimatedPosition&, f64 newPosition) = 0;
    };

    /** Adds a listener to be called when the value changes. */
    z0 addListener (Listener* listener)       { listeners.add (listener); }

    /** Removes a previously-registered listener. */
    z0 removeListener (Listener* listener)    { listeners.remove (listener); }

    //==============================================================================
    /** The behaviour object.
        This is public to let you tweak any parameters that it provides.
    */
    Behaviour behaviour;

private:
    //==============================================================================
    f64 position = 0.0, grabbedPos = 0.0, releaseVelocity = 0.0;
    Range<f64> range;
    Time lastUpdate, lastDrag;
    ListenerList<Listener> listeners;

    static f64 getSpeed (const Time last, f64 lastPos,
                            const Time now, f64 newPos)
    {
        auto elapsedSecs = jmax (0.005, (now - last).inSeconds());
        auto v = (newPos - lastPos) / elapsedSecs;
        return std::abs (v) > 0.2 ? v : 0.0;
    }

    z0 moveTo (f64 newPos)
    {
        auto now = Time::getCurrentTime();
        releaseVelocity = getSpeed (lastDrag, position, now, newPos);
        behaviour.releasedWithVelocity (newPos, releaseVelocity);
        lastDrag = now;

        setPositionAndSendChange (newPos);
    }

    z0 setPositionAndSendChange (f64 newPosition)
    {
        newPosition = range.clipValue (newPosition);

        if (! approximatelyEqual (position, newPosition))
        {
            position = newPosition;
            listeners.call ([this, newPosition] (Listener& l) { l.positionChanged (*this, newPosition); });
        }
    }

    z0 timerCallback() override
    {
        auto now = Time::getCurrentTime();
        auto elapsed = jlimit (0.001, 0.020, (now - lastUpdate).inSeconds());
        lastUpdate = now;
        auto newPos = behaviour.getNextPosition (position, elapsed);

        if (behaviour.isStopped (newPos))
            stopTimer();
        else
            startTimerHz (60);

        setPositionAndSendChange (newPos);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedPosition)
};

} // namespace drx
