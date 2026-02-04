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
MouseInputSource::MouseInputSource (detail::MouseInputSourceImpl* s) noexcept   : pimpl (s)  {}
MouseInputSource::MouseInputSource (const MouseInputSource& other) noexcept : pimpl (other.pimpl)  {}
MouseInputSource::~MouseInputSource() noexcept {}

MouseInputSource& MouseInputSource::operator= (const MouseInputSource& other) noexcept
{
    pimpl = other.pimpl;
    return *this;
}

MouseInputSource::InputSourceType MouseInputSource::getType() const noexcept    { return pimpl->inputType; }
b8 MouseInputSource::isMouse() const noexcept                                 { return (getType() == MouseInputSource::InputSourceType::mouse); }
b8 MouseInputSource::isTouch() const noexcept                                 { return (getType() == MouseInputSource::InputSourceType::touch); }
b8 MouseInputSource::isPen() const noexcept                                   { return (getType() == MouseInputSource::InputSourceType::pen); }
b8 MouseInputSource::canHover() const noexcept                                { return ! isTouch(); }
b8 MouseInputSource::hasMouseWheel() const noexcept                           { return ! isTouch(); }
i32 MouseInputSource::getIndex() const noexcept                                 { return pimpl->index; }
b8 MouseInputSource::isDragging() const noexcept                              { return pimpl->isDragging(); }
Point<f32> MouseInputSource::getScreenPosition() const noexcept               { return pimpl->getScreenPosition(); }
Point<f32> MouseInputSource::getRawScreenPosition() const noexcept            { return pimpl->getRawScreenPosition();  }
ModifierKeys MouseInputSource::getCurrentModifiers() const noexcept             { return pimpl->getCurrentModifiers(); }
f32 MouseInputSource::getCurrentPressure() const noexcept                     { return pimpl->lastPointerState.pressure; }
b8 MouseInputSource::isPressureValid() const noexcept                         { return pimpl->lastPointerState.isPressureValid(); }
f32 MouseInputSource::getCurrentOrientation() const noexcept                  { return pimpl->lastPointerState.orientation; }
b8 MouseInputSource::isOrientationValid() const noexcept                      { return pimpl->lastPointerState.isOrientationValid(); }
f32 MouseInputSource::getCurrentRotation() const noexcept                     { return pimpl->lastPointerState.rotation; }
b8 MouseInputSource::isRotationValid() const noexcept                         { return pimpl->lastPointerState.isRotationValid(); }
f32 MouseInputSource::getCurrentTilt (b8 tiltX) const noexcept              { return tiltX ? pimpl->lastPointerState.tiltX : pimpl->lastPointerState.tiltY; }
b8 MouseInputSource::isTiltValid (b8 isX) const noexcept                    { return pimpl->lastPointerState.isTiltValid (isX); }
Component* MouseInputSource::getComponentUnderMouse() const                     { return pimpl->getComponentUnderMouse(); }
z0 MouseInputSource::triggerFakeMove() const                                  { pimpl->triggerFakeMove(); }
i32 MouseInputSource::getNumberOfMultipleClicks() const noexcept                { return pimpl->getNumberOfMultipleClicks(); }
Time MouseInputSource::getLastMouseDownTime() const noexcept                    { return pimpl->getLastMouseDownTime(); }
Point<f32> MouseInputSource::getLastMouseDownPosition() const noexcept        { return pimpl->getLastMouseDownPosition(); }
b8 MouseInputSource::isLongPressOrDrag() const noexcept                       { return pimpl->isLongPressOrDrag(); }
b8 MouseInputSource::hasMovedSignificantlySincePressed() const noexcept       { return pimpl->hasMovedSignificantlySincePressed(); }
b8 MouseInputSource::canDoUnboundedMovement() const noexcept                  { return ! isTouch(); }
z0 MouseInputSource::enableUnboundedMouseMovement (b8 isEnabled, b8 keepCursorVisibleUntilOffscreen) const
                                                                         { pimpl->enableUnboundedMouseMovement (isEnabled, keepCursorVisibleUntilOffscreen); }
b8 MouseInputSource::isUnboundedMouseMovementEnabled() const           { return pimpl->isUnboundedMouseModeOn; }
b8 MouseInputSource::hasMouseCursor() const noexcept                   { return ! isTouch(); }
z0 MouseInputSource::showMouseCursor (const MouseCursor& cursor)       { pimpl->showMouseCursor (cursor, false); }
z0 MouseInputSource::hideCursor()                                      { pimpl->hideCursor(); }
z0 MouseInputSource::revealCursor()                                    { pimpl->revealCursor (false); }
z0 MouseInputSource::forceMouseCursorUpdate()                          { pimpl->revealCursor (true); }
z0 MouseInputSource::setScreenPosition (Point<f32> p)                { pimpl->setScreenPosition (p); }

z0 MouseInputSource::handleEvent (ComponentPeer& peer, Point<f32> pos, z64 time, ModifierKeys mods,
                                    f32 pressure, f32 orientation, const PenDetails& penDetails)
{
    pimpl->handleEvent (peer, pos, Time (time), mods.withOnlyMouseButtons(), pressure, orientation, penDetails);
}

z0 MouseInputSource::handleWheel (ComponentPeer& peer, Point<f32> pos, z64 time, const MouseWheelDetails& wheel)
{
    pimpl->handleWheel (peer, pos, Time (time), wheel);
}

z0 MouseInputSource::handleMagnifyGesture (ComponentPeer& peer, Point<f32> pos, z64 time, f32 scaleFactor)
{
    pimpl->handleMagnifyGesture (peer, pos, Time (time), scaleFactor);
}

const f32 MouseInputSource::invalidPressure = 0.0f;
const f32 MouseInputSource::invalidOrientation = 0.0f;
const f32 MouseInputSource::invalidRotation = 0.0f;

const f32 MouseInputSource::invalidTiltX = 0.0f;
const f32 MouseInputSource::invalidTiltY = 0.0f;

const Point<f32> MouseInputSource::offscreenMousePos { -10.0f, -10.0f };

// Deprecated method
b8 MouseInputSource::hasMouseMovedSignificantlySincePressed() const noexcept  { return pimpl->hasMouseMovedSignificantlySincePressed(); }

} // namespace drx
