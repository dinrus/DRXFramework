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

namespace detail
{
    using ColorSpacePtr     = CFUniquePtr<CGColorSpaceRef>;
    using ContextPtr        = CFUniquePtr<CGContextRef>;
    using DataProviderPtr   = CFUniquePtr<CGDataProviderRef>;
    using ImagePtr          = CFUniquePtr<CGImageRef>;
    using GradientPtr       = CFUniquePtr<CGGradientRef>;
    using ColorPtr          = CFUniquePtr<CGColorRef>;
    using PathPtr           = CFUniquePtr<CGPathRef>;
    using MutablePathPtr    = CFUniquePtr<CGMutablePathRef>;
}

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context, f32 flipHeight);
    ~CoreGraphicsContext() override;

    //==============================================================================
    b8 isVectorDevice() const override         { return false; }

    z0 setOrigin (Point<i32>) override;
    z0 addTransform (const AffineTransform&) override;
    f32 getPhysicalPixelScaleFactor() const override;
    b8 clipToRectangle (const Rectangle<i32>&) override;
    b8 clipToRectangleList (const RectangleList<i32>&) override;
    z0 excludeClipRectangle (const Rectangle<i32>&) override;
    z0 clipToPath (const Path&, const AffineTransform&) override;
    z0 clipToImageAlpha (const Image&, const AffineTransform&) override;
    b8 clipRegionIntersects (const Rectangle<i32>&) override;
    Rectangle<i32> getClipBounds() const override;
    b8 isClipEmpty() const override;

    //==============================================================================
    z0 saveState() override;
    z0 restoreState() override;
    z0 beginTransparencyLayer (f32 opacity) override;
    z0 endTransparencyLayer() override;

    //==============================================================================
    z0 setFill (const FillType&) override;
    z0 setOpacity (f32) override;
    z0 setInterpolationQuality (Graphics::ResamplingQuality) override;

    //==============================================================================
    z0 fillAll() override;
    z0 fillRect (const Rectangle<i32>&, b8 replaceExistingContents) override;
    z0 fillRect (const Rectangle<f32>&) override;
    z0 fillRectList (const RectangleList<f32>&) override;
    z0 fillPath (const Path&, const AffineTransform&) override;
    z0 strokePath (const Path& path, const PathStrokeType& strokeType, const AffineTransform& transform) override;
    z0 drawImage (const Image& sourceImage, const AffineTransform&) override;

    //==============================================================================
    z0 drawLine (const Line<f32>&) override;
    z0 setFont (const Font&) override;
    const Font& getFont() override;
    z0 drawGlyphs (Span<u16k>,
                     Span<const Point<f32>>,
                     const AffineTransform&) override;

    zu64 getFrameId() const override { return 0; }

    z0 drawEllipse (const Rectangle<f32>& area, f32 lineThickness) override;
    z0 fillEllipse (const Rectangle<f32>& area) override;

    z0 drawRoundedRectangle (const Rectangle<f32>& r, f32 cornerSize, f32 lineThickness) override;
    z0 fillRoundedRectangle (const Rectangle<f32>& r, f32 cornerSize) override;

    z0 drawLineWithThickness (const Line<f32>& line, f32 lineThickness) override;

private:
    //==============================================================================
    detail::ContextPtr context;
    const CGFloat flipHeight;
    detail::ColorSpacePtr rgbColorSpace, greyColorSpace;
    mutable std::optional<Rectangle<i32>> lastClipRect;

    struct SavedState;
    std::unique_ptr<SavedState> state;
    OwnedArray<SavedState> stateStack;

    template <class RectType>
    CGRect convertToCGRectFlipped (RectType r) const noexcept;
    z0 setContextClipToCurrentPath (b8 useNonZeroWinding);
    z0 drawCurrentPath (CGPathDrawingMode mode);
    z0 drawGradient();
    z0 createPath (const Path&, const AffineTransform&) const;
    z0 flip() const;
    z0 applyTransform (const AffineTransform&) const;
    z0 drawImage (const Image&, const AffineTransform&, b8 fillEntireClipAsTiles);
    b8 clipToRectangleListWithoutTest (const RectangleList<f32>&);
    z0 fillCGRect (const CGRect&, b8 replaceExistingContents);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsContext)
};

} // namespace drx
