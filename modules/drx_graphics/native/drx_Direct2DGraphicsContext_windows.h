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

class Direct2DGraphicsContext : public LowLevelGraphicsContext
{
public:
    Direct2DGraphicsContext();
    ~Direct2DGraphicsContext() override;

    //==============================================================================
    b8 isVectorDevice() const override { return false; }

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
    z0 fillRect (const Rectangle<i32>&, b8 replaceExistingContents) override;
    z0 fillRect (const Rectangle<f32>&) override;
    z0 fillRectList (const RectangleList<f32>&) override;
    z0 fillPath (const Path&, const AffineTransform&) override;
    z0 drawImage (const Image& sourceImage, const AffineTransform&) override;

    //==============================================================================
    z0 drawLine (const Line<f32>&) override;
    z0 setFont (const Font&) override;
    const Font& getFont() override;
    z0 drawGlyphs (Span<u16k>,
                     Span<const Point<f32>>,
                     const AffineTransform&) override;

    //==============================================================================
    // These methods were not originally part of the LowLevelGraphicsContext; they
    // were added because Direct2D supports these drawing primitives directly.
    // The specialised functions are more efficient than emulating the same behaviour, e.g.
    // by drawing paths.
    z0 drawLineWithThickness (const Line<f32>&, f32) override;

    z0 drawEllipse (const Rectangle<f32>& area, f32 lineThickness) override;
    z0 fillEllipse (const Rectangle<f32>& area) override;

    z0 drawRect (const Rectangle<f32>&, f32) override;
    z0 strokePath (const Path&, const PathStrokeType& strokeType, const AffineTransform&) override;

    z0 drawRoundedRectangle (const Rectangle<f32>& area, f32 cornerSize, f32 lineThickness) override;
    z0 fillRoundedRectangle (const Rectangle<f32>& area, f32 cornerSize) override;

    //==============================================================================
    b8 startFrame (f32 dpiScale);
    z0 endFrame();

    virtual Image createSnapshot() const { return {}; }

    zu64 getFrameId() const override { return frame; }

    Direct2DMetrics::Ptr metrics;

protected:
    struct SavedState;
    SavedState* currentState = nullptr;

    class PendingClipList
    {
    public:
        z0 clipTo (Rectangle<f32> i)
        {
            if (std::exchange (clipApplied, true))
                list.clipTo (i);
            else
                list = i;
        }

        template <typename Numeric>
        z0 clipTo (const RectangleList<Numeric>& other)
        {
            if (std::exchange (clipApplied, true))
            {
                list.clipTo (other);
            }
            else
            {
                list.clear();

                for (const auto& r : other)
                    list.add (r.toFloat());
            }
        }

        z0 subtract (Rectangle<f32> i)
        {
            list.subtract (i);
            clipApplied = true;
        }

        RectangleList<f32> getList() const { return list; }
        b8 isClipApplied() const { return clipApplied; }

        z0 reset (Rectangle<f32> maxBounds)
        {
            list = maxBounds;
            clipApplied = false;
        }

    private:
        RectangleList<f32> list;
        b8 clipApplied = false;
    };

    PendingClipList pendingClipList;

    struct Pimpl;
    virtual Pimpl* getPimpl() const noexcept = 0;

    z0 resetPendingClipList();
    z0 applyPendingClipList();
    virtual z0 clearTargetBuffer() = 0;

    struct ScopedTransform
    {
        ScopedTransform (Pimpl&, SavedState*);
        ScopedTransform (Pimpl&, SavedState*, const AffineTransform& transform);
        ~ScopedTransform();

        Pimpl& pimpl;
        SavedState* state = nullptr;
    };

    zu64 frame = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DGraphicsContext)
};

} // namespace drx
