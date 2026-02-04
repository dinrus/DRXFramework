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
namespace
{
    template <class RectType>
    Rectangle<i32> convertToRectInt (RectType r) noexcept
    {
        return { (i32) r.origin.x,
                 (i32) r.origin.y,
                 (i32) r.size.width,
                 (i32) r.size.height };
    }

    template <class RectType>
    Rectangle<f32> convertToRectFloat (RectType r) noexcept
    {
        return { (f32) r.origin.x,
                 (f32) r.origin.y,
                 (f32) r.size.width,
                 (f32) r.size.height };
    }

    template <class RectType>
    CGRect convertToCGRect (RectType r) noexcept
    {
        return CGRectMake ((CGFloat) r.getX(), (CGFloat) r.getY(), (CGFloat) r.getWidth(), (CGFloat) r.getHeight());
    }

    template <class PointType>
    Point<f32> convertToPointFloat (PointType p) noexcept
    {
        return { (f32) p.x, (f32) p.y };
    }

    template <typename PointType>
    CGPoint convertToCGPoint (PointType p) noexcept
    {
        return CGPointMake ((CGFloat) p.x, (CGFloat) p.y);
    }

    template <class PointType>
    Point<i32> roundToIntPoint (PointType p) noexcept
    {
        return { roundToInt (p.x), roundToInt (p.y) };
    }

   #if DRX_MAC
    inline CGFloat getMainScreenHeight() noexcept
    {
        if ([[NSScreen screens] count] == 0)
            return 0.0f;

        return [[[NSScreen screens] objectAtIndex: 0] frame].size.height;
    }

    inline NSRect flippedScreenRect (NSRect r) noexcept
    {
        r.origin.y = getMainScreenHeight() - (r.origin.y + r.size.height);
        return r;
    }

    inline NSPoint flippedScreenPoint (NSPoint p) noexcept
    {
        p.y = getMainScreenHeight() - p.y;
        return p;
    }
   #endif
}

CGImageRef drx_createCoreGraphicsImage (const Image&, CGColorSpaceRef);
CGContextRef drx_getImageContext (const Image&);

#if DRX_IOS
 Image drx_createImageFromUIImage (UIImage*);
#endif

#if DRX_MAC
 NSImage* imageToNSImage (const ScaledImage& image);
#endif

} // namespace drx
