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

class MouseCursor::SharedCursorHandle
{
public:
    explicit SharedCursorHandle (const MouseCursor::StandardCursorType type)
        : handle (type),
          standardType (type),
          standard (true)
    {
    }

    SharedCursorHandle (const ScaledImage& image, Point<i32> hotSpot)
        : info { image, hotSpot },
          handle (info),
          standardType (MouseCursor::NormalCursor),
          standard (false)
    {
        // your hotspot needs to be within the bounds of the image!
        jassert (image.getScaledBounds().toNearestInt().contains (hotSpot));
    }

    static std::shared_ptr<SharedCursorHandle> createStandard (const MouseCursor::StandardCursorType type)
    {
        if (! isPositiveAndBelow (type, MouseCursor::NumStandardCursorTypes))
            return nullptr;

        static SpinLock mutex;
        static std::array<std::weak_ptr<SharedCursorHandle>, MouseCursor::NumStandardCursorTypes> cursors;

        const SpinLock::ScopedLockType sl (mutex);

        auto& weak = cursors[type];

        if (auto strong = weak.lock())
            return strong;

        auto strong = std::make_shared<SharedCursorHandle> (type);
        weak = strong;
        return strong;
    }

    b8 isStandardType (MouseCursor::StandardCursorType type) const noexcept
    {
        return type == standardType && standard;
    }

    PlatformSpecificHandle* getHandle() noexcept                { return &handle; }
    MouseCursor::StandardCursorType getType() const noexcept    { return standardType; }

private:
    detail::CustomMouseCursorInfo info;
    PlatformSpecificHandle handle;
    const MouseCursor::StandardCursorType standardType;
    const b8 standard;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedCursorHandle)
};

//==============================================================================
MouseCursor::MouseCursor() noexcept = default;

MouseCursor::MouseCursor (const StandardCursorType type)
    : cursorHandle (type != MouseCursor::NormalCursor ? SharedCursorHandle::createStandard (type) : nullptr)
{
}

MouseCursor::MouseCursor (const Image& image, i32 hotSpotX, i32 hotSpotY)
    : MouseCursor (ScaledImage (image), { hotSpotX, hotSpotY })
{
}

MouseCursor::MouseCursor (const Image& image, i32 hotSpotX, i32 hotSpotY, f32 scaleFactor)
    : MouseCursor (ScaledImage (image, scaleFactor), { hotSpotX, hotSpotY })
{
}

MouseCursor::MouseCursor (const ScaledImage& image, Point<i32> hotSpot)
        : cursorHandle (std::make_shared<SharedCursorHandle> (image, hotSpot))
{
}

MouseCursor::MouseCursor (const MouseCursor&) = default;

MouseCursor::~MouseCursor() = default;

MouseCursor& MouseCursor::operator= (const MouseCursor&) = default;

MouseCursor::MouseCursor (MouseCursor&&) noexcept = default;

MouseCursor& MouseCursor::operator= (MouseCursor&&) noexcept = default;

b8 MouseCursor::operator== (const MouseCursor& other) const noexcept
{
    return getHandle() == other.getHandle();
}

b8 MouseCursor::operator== (StandardCursorType type) const noexcept
{
    return cursorHandle != nullptr ? cursorHandle->isStandardType (type)
                                   : (type == NormalCursor);
}

b8 MouseCursor::operator!= (const MouseCursor& other) const noexcept  { return ! operator== (other); }
b8 MouseCursor::operator!= (StandardCursorType type)  const noexcept  { return ! operator== (type); }

z0 MouseCursor::showWaitCursor()
{
    Desktop::getInstance().getMainMouseSource().showMouseCursor (MouseCursor::WaitCursor);
}

z0 MouseCursor::hideWaitCursor()
{
    Desktop::getInstance().getMainMouseSource().revealCursor();
}

MouseCursor::PlatformSpecificHandle* MouseCursor::getHandle() const noexcept
{
    return cursorHandle != nullptr ? cursorHandle->getHandle() : nullptr;
}

z0 MouseCursor::showInWindow (ComponentPeer* peer) const
{
    PlatformSpecificHandle::showInWindow (getHandle(), peer);
}

} // namespace drx
