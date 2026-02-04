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

struct ScalingHelpers
{
    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (f32 scale, PointOrRect pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? pos / scale : pos;
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (f32 scale, PointOrRect pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? pos * scale : pos;
    }

    // For these, we need to avoid getSmallestIntegerContainer being used, which causes
    // judder when moving windows
    static Rectangle<i32> unscaledScreenPosToScaled (f32 scale, Rectangle<i32> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<i32> (roundToInt ((f32) pos.getX() / scale),
                                                                    roundToInt ((f32) pos.getY() / scale),
                                                                    roundToInt ((f32) pos.getWidth() / scale),
                                                                    roundToInt ((f32) pos.getHeight() / scale)) : pos;
    }

    static Rectangle<i32> scaledScreenPosToUnscaled (f32 scale, Rectangle<i32> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<i32> (roundToInt ((f32) pos.getX() * scale),
                                                                    roundToInt ((f32) pos.getY() * scale),
                                                                    roundToInt ((f32) pos.getWidth() * scale),
                                                                    roundToInt ((f32) pos.getHeight() * scale)) : pos;
    }

    static Rectangle<f32> unscaledScreenPosToScaled (f32 scale, Rectangle<f32> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<f32> (pos.getX() / scale,
                                                                      pos.getY() / scale,
                                                                      pos.getWidth() / scale,
                                                                      pos.getHeight() / scale) : pos;
    }

    static Rectangle<f32> scaledScreenPosToUnscaled (f32 scale, Rectangle<f32> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<f32> (pos.getX() * scale,
                                                                      pos.getY() * scale,
                                                                      pos.getWidth() * scale,
                                                                      pos.getHeight() * scale) : pos;
    }

    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (PointOrRect pos) noexcept
    {
        return unscaledScreenPosToScaled (Desktop::getInstance().getGlobalScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (PointOrRect pos) noexcept
    {
        return scaledScreenPosToUnscaled (Desktop::getInstance().getGlobalScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (const Component& comp, PointOrRect pos) noexcept
    {
        return unscaledScreenPosToScaled (comp.getDesktopScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (const Component& comp, PointOrRect pos) noexcept
    {
        return scaledScreenPosToUnscaled (comp.getDesktopScaleFactor(), pos);
    }

    static Point<i32>       addPosition      (Point<i32> p,       const Component& c) noexcept  { return p + c.getPosition(); }
    static Rectangle<i32>   addPosition      (Rectangle<i32> p,   const Component& c) noexcept  { return p + c.getPosition(); }
    static Point<f32>     addPosition      (Point<f32> p,     const Component& c) noexcept  { return p + c.getPosition().toFloat(); }
    static Rectangle<f32> addPosition      (Rectangle<f32> p, const Component& c) noexcept  { return p + c.getPosition().toFloat(); }
    static Point<i32>       subtractPosition (Point<i32> p,       const Component& c) noexcept  { return p - c.getPosition(); }
    static Rectangle<i32>   subtractPosition (Rectangle<i32> p,   const Component& c) noexcept  { return p - c.getPosition(); }
    static Point<f32>     subtractPosition (Point<f32> p,     const Component& c) noexcept  { return p - c.getPosition().toFloat(); }
    static Rectangle<f32> subtractPosition (Rectangle<f32> p, const Component& c) noexcept  { return p - c.getPosition().toFloat(); }

    static Point<f32> screenPosToLocalPos (Component& comp, Point<f32> pos)
    {
        if (auto* peer = comp.getPeer())
        {
            pos = peer->globalToLocal (pos);
            auto& peerComp = peer->getComponent();
            return comp.getLocalPoint (&peerComp, unscaledScreenPosToScaled (peerComp, pos));
        }

        return comp.getLocalPoint (nullptr, unscaledScreenPosToScaled (comp, pos));
    }
};

} // namespace drx::detail
