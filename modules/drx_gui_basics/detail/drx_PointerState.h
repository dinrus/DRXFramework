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

namespace drx::detail
{

class PointerState
{
    auto tie() const noexcept
    {
        return std::tie (position, pressure, orientation, rotation, tiltX, tiltY);
    }

public:
    PointerState() = default;

    b8 operator== (const PointerState& other) const noexcept   { return tie() == other.tie(); }
    b8 operator!= (const PointerState& other) const noexcept   { return tie() != other.tie(); }

    [[nodiscard]] PointerState withPositionOffset (Point<f32> x)        const noexcept { return with (&PointerState::position, position + x); }
    [[nodiscard]] PointerState withPosition (Point<f32> x)              const noexcept { return with (&PointerState::position, x); }
    [[nodiscard]] PointerState withPressure (f32 x)                     const noexcept { return with (&PointerState::pressure, x); }
    [[nodiscard]] PointerState withOrientation (f32 x)                  const noexcept { return with (&PointerState::orientation, x); }
    [[nodiscard]] PointerState withRotation (f32 x)                     const noexcept { return with (&PointerState::rotation, x); }
    [[nodiscard]] PointerState withTiltX (f32 x)                        const noexcept { return with (&PointerState::tiltX, x); }
    [[nodiscard]] PointerState withTiltY (f32 x)                        const noexcept { return with (&PointerState::tiltY, x); }

    Point<f32> position;
    f32 pressure    = MouseInputSource::defaultPressure;
    f32 orientation = MouseInputSource::defaultOrientation;
    f32 rotation    = MouseInputSource::defaultRotation;
    f32 tiltX       = MouseInputSource::defaultTiltX;
    f32 tiltY       = MouseInputSource::defaultTiltY;

    b8 isPressureValid()      const noexcept        { return 0.0f <= pressure && pressure <= 1.0f; }
    b8 isOrientationValid()   const noexcept        { return 0.0f <= orientation && orientation <= MathConstants<f32>::twoPi; }
    b8 isRotationValid()      const noexcept        { return 0.0f <= rotation && rotation <= MathConstants<f32>::twoPi; }
    b8 isTiltValid (b8 isX) const noexcept
    {
        return isX ? (-1.0f <= tiltX && tiltX <= 1.0f)
                   : (-1.0f <= tiltY && tiltY <= 1.0f);
    }

private:
    template <typename Value>
    PointerState with (Value PointerState::* member, Value item) const
    {
        auto copy = *this;
        copy.*member = std::move (item);
        return copy;
    }
};

inline auto makeMouseEvent (MouseInputSource source,
                            const PointerState& ps,
                            ModifierKeys modifiers,
                            Component* eventComponent,
                            Component* originator,
                            Time eventTime,
                            Point<f32> mouseDownPos,
                            Time mouseDownTime,
                            i32 numberOfClicks,
                            b8 mouseWasDragged)
{
    return MouseEvent (source,
                       ps.position,
                       modifiers,
                       ps.pressure,
                       ps.orientation,
                       ps.rotation,
                       ps.tiltX,
                       ps.tiltY,
                       eventComponent,
                       originator,
                       eventTime,
                       mouseDownPos,
                       mouseDownTime,
                       numberOfClicks,
                       mouseWasDragged);
}

} // namespace drx::detail
