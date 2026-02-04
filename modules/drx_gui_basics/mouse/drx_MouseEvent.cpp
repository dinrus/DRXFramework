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

MouseEvent::MouseEvent (MouseInputSource inputSource,
                        Point<f32> pos,
                        ModifierKeys modKeys,
                        f32 force,
                        f32 o, f32 r,
                        f32 tX, f32 tY,
                        Component* const eventComp,
                        Component* const originator,
                        Time time,
                        Point<f32> downPos,
                        Time downTime,
                        i32k numClicks,
                        const b8 mouseWasDragged) noexcept
    : position (pos),
      x (roundToInt (pos.x)),
      y (roundToInt (pos.y)),
      mods (modKeys),
      pressure (force),
      orientation (o), rotation (r),
      tiltX (tX), tiltY (tY),
      mouseDownPosition (downPos),
      eventComponent (eventComp),
      originalComponent (originator),
      eventTime (time),
      mouseDownTime (downTime),
      source (inputSource),
      numberOfClicks ((u8) numClicks),
      wasMovedSinceMouseDown ((u8) (mouseWasDragged ? 1 : 0))
{
}

//==============================================================================
MouseEvent MouseEvent::getEventRelativeTo (Component* const otherComponent) const noexcept
{
    jassert (otherComponent != nullptr);

    return MouseEvent (source, otherComponent->getLocalPoint (eventComponent, position),
                       mods, pressure, orientation, rotation, tiltX, tiltY,
                       otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPosition),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<f32> newPosition) const noexcept
{
    return MouseEvent (source, newPosition, mods, pressure, orientation, rotation, tiltX, tiltY,
                       eventComponent, originalComponent, eventTime, mouseDownPosition, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<i32> newPosition) const noexcept
{
    return MouseEvent (source, newPosition.toFloat(), mods, pressure, orientation, rotation,
                       tiltX, tiltY, eventComponent,  originalComponent, eventTime, mouseDownPosition,
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

//==============================================================================
b8 MouseEvent::mouseWasDraggedSinceMouseDown() const noexcept
{
    return wasMovedSinceMouseDown != 0;
}

b8 MouseEvent::mouseWasClicked() const noexcept
{
    return ! mouseWasDraggedSinceMouseDown();
}

i32 MouseEvent::getLengthOfMousePress() const noexcept
{
    if (mouseDownTime.toMilliseconds() > 0)
        return jmax (0, (i32) (eventTime - mouseDownTime).inMilliseconds());

    return 0;
}

//==============================================================================
Point<i32> MouseEvent::getPosition() const noexcept             { return Point<i32> (x, y); }
Point<i32> MouseEvent::getScreenPosition() const                { return eventComponent->localPointToGlobal (getPosition()); }

Point<i32> MouseEvent::getMouseDownPosition() const noexcept    { return mouseDownPosition.roundToInt(); }
Point<i32> MouseEvent::getMouseDownScreenPosition() const       { return eventComponent->localPointToGlobal (mouseDownPosition).roundToInt(); }

Point<i32> MouseEvent::getOffsetFromDragStart() const noexcept  { return (position - mouseDownPosition).roundToInt(); }
i32 MouseEvent::getDistanceFromDragStart() const noexcept       { return roundToInt (mouseDownPosition.getDistanceFrom (position)); }

i32 MouseEvent::getMouseDownX() const noexcept                  { return roundToInt (mouseDownPosition.x); }
i32 MouseEvent::getMouseDownY() const noexcept                  { return roundToInt (mouseDownPosition.y); }

i32 MouseEvent::getDistanceFromDragStartX() const noexcept      { return getOffsetFromDragStart().x; }
i32 MouseEvent::getDistanceFromDragStartY() const noexcept      { return getOffsetFromDragStart().y; }

i32 MouseEvent::getScreenX() const                              { return getScreenPosition().x; }
i32 MouseEvent::getScreenY() const                              { return getScreenPosition().y; }

i32 MouseEvent::getMouseDownScreenX() const                     { return getMouseDownScreenPosition().x; }
i32 MouseEvent::getMouseDownScreenY() const                     { return getMouseDownScreenPosition().y; }

b8 MouseEvent::isPressureValid() const noexcept               { return pressure > 0.0f && pressure < 1.0f; }
b8 MouseEvent::isOrientationValid() const noexcept            { return orientation >= 0.0f && orientation <= MathConstants<f32>::twoPi; }
b8 MouseEvent::isRotationValid() const noexcept               { return rotation >= 0 && rotation <= MathConstants<f32>::twoPi; }
b8 MouseEvent::isTiltValid (b8 isX) const noexcept          { return isX ? (tiltX >= -1.0f && tiltX <= 1.0f) : (tiltY >= -1.0f && tiltY <= 1.0f); }

//==============================================================================
static i32 doubleClickTimeOutMs = 400;

i32 MouseEvent::getDoubleClickTimeout() noexcept                        { return doubleClickTimeOutMs; }
z0 MouseEvent::setDoubleClickTimeout (i32k newTime) noexcept     { doubleClickTimeOutMs = newTime; }

} // namespace drx
