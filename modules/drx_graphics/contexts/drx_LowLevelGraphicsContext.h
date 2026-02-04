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
    Interface class for graphics context objects, used internally by the Graphics class.

    Users are not supposed to create instances of this class directly - do your drawing
    via the Graphics object instead.

    It's a base class for different types of graphics context, that may perform software-based
    or OS-accelerated rendering.

    E.g. the LowLevelGraphicsSoftwareRenderer renders onto an image in memory, but other
    subclasses could render directly to a windows HDC, a Quartz context, or an OpenGL
    context.

    @tags{Graphics}
*/
class DRX_API  LowLevelGraphicsContext
{
protected:
    //==============================================================================
    LowLevelGraphicsContext() = default;

public:
    virtual ~LowLevelGraphicsContext() = default;

    /** Возвращает true, если this device is vector-based, e.g. a printer. */
    virtual b8 isVectorDevice() const = 0;

    //==============================================================================
    /** Moves the origin to a new position.

        The coordinates are relative to the current origin, and indicate the new position
        of (0, 0).
    */
    virtual z0 setOrigin (Point<i32>) = 0;
    virtual z0 addTransform (const AffineTransform&) = 0;
    virtual f32 getPhysicalPixelScaleFactor() const = 0;

    virtual b8 clipToRectangle (const Rectangle<i32>&) = 0;
    virtual b8 clipToRectangleList (const RectangleList<i32>&) = 0;
    virtual z0 excludeClipRectangle (const Rectangle<i32>&) = 0;
    virtual z0 clipToPath (const Path&, const AffineTransform&) = 0;
    virtual z0 clipToImageAlpha (const Image&, const AffineTransform&) = 0;

    virtual b8 clipRegionIntersects (const Rectangle<i32>&) = 0;
    virtual Rectangle<i32> getClipBounds() const = 0;
    virtual b8 isClipEmpty() const = 0;

    virtual z0 saveState() = 0;
    virtual z0 restoreState() = 0;

    virtual z0 beginTransparencyLayer (f32 opacity) = 0;
    virtual z0 endTransparencyLayer() = 0;

    //==============================================================================
    virtual z0 setFill (const FillType&) = 0;
    virtual z0 setOpacity (f32) = 0;
    virtual z0 setInterpolationQuality (Graphics::ResamplingQuality) = 0;

    //==============================================================================
    virtual z0 fillAll() { fillRect (getClipBounds(), false); }
    virtual z0 fillRect (const Rectangle<i32>&, b8 replaceExistingContents) = 0;
    virtual z0 fillRect (const Rectangle<f32>&) = 0;
    virtual z0 fillRectList (const RectangleList<f32>&) = 0;
    virtual z0 fillPath (const Path&, const AffineTransform&) = 0;

    virtual z0 drawRect (const Rectangle<f32>& rect, f32 lineThickness)
    {
        auto r = rect;
        RectangleList<f32> rects;
        rects.addWithoutMerging (r.removeFromTop    (lineThickness));
        rects.addWithoutMerging (r.removeFromBottom (lineThickness));
        rects.addWithoutMerging (r.removeFromLeft   (lineThickness));
        rects.addWithoutMerging (r.removeFromRight  (lineThickness));
        fillRectList (rects);
    }

    virtual z0 strokePath (const Path& path, const PathStrokeType& strokeType, const AffineTransform& transform)
    {
        Path stroke;
        strokeType.createStrokedPath (stroke, path, transform, getPhysicalPixelScaleFactor());
        fillPath (stroke, {});
    }

    virtual z0 drawImage (const Image&, const AffineTransform&) = 0;
    virtual z0 drawLine (const Line<f32>&) = 0;

    virtual z0 drawLineWithThickness (const Line<f32>& line, f32 lineThickness)
    {
        Path p;
        p.addLineSegment (line, lineThickness);
        fillPath (p, {});
    }

    virtual z0 setFont (const Font&) = 0;
    virtual const Font& getFont() = 0;

    /** Uses the current font to draw the provided glyph numbers. */
    virtual z0 drawGlyphs (Span<u16k>,
                             Span<const Point<f32>>,
                             const AffineTransform&) = 0;

    virtual z0 drawRoundedRectangle (const Rectangle<f32>& r, f32 cornerSize, f32 lineThickness)
    {
        Path p;
        p.addRoundedRectangle (r, cornerSize);
        strokePath (p, PathStrokeType (lineThickness), {});
    }

    virtual z0 fillRoundedRectangle (const Rectangle<f32>& r, f32 cornerSize)
    {
        Path p;
        p.addRoundedRectangle (r, cornerSize);
        fillPath (p, {});
    }

    virtual z0 drawEllipse (const Rectangle<f32>& area, f32 lineThickness)
    {
        Path p;

        if (approximatelyEqual (area.getWidth(), area.getHeight()))
        {
            // For a circle, we can avoid having to generate a stroke
            p.addEllipse (area.expanded (lineThickness * 0.5f));
            p.addEllipse (area.reduced  (lineThickness * 0.5f));
            p.setUsingNonZeroWinding (false);
            fillPath (p, {});
        }
        else
        {
            p.addEllipse (area);
            strokePath (p, PathStrokeType (lineThickness), {});
        }
    }

    virtual z0 fillEllipse (const Rectangle<f32>& area)
    {
        Path p;
        p.addEllipse (area);
        fillPath (p, {});
    }

    /** Returns an integer that uniquely identifies the current frame.
        Useful for debugging/logging.
    */
    virtual zu64 getFrameId() const = 0;
};

} // namespace drx
