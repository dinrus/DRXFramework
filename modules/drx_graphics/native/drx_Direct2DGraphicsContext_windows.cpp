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

struct ScopedBlendCopy
{
    explicit ScopedBlendCopy (ComSmartPtr<ID2D1DeviceContext1> c)
        : ctx (c)
    {
        ctx->SetPrimitiveBlend (D2D1_PRIMITIVE_BLEND_COPY);
    }

    ~ScopedBlendCopy()
    {
        ctx->SetPrimitiveBlend (blend);
    }

    ComSmartPtr<ID2D1DeviceContext1> ctx;
    D2D1_PRIMITIVE_BLEND blend = ctx->GetPrimitiveBlend();
};

class PushedLayers
{
public:
    PushedLayers() { pushedLayers.reserve (32); }
    PushedLayers (const PushedLayers&) { pushedLayers.reserve (32); }

   #if DRX_DEBUG
    ~PushedLayers()
    {
        jassert (pushedLayers.empty());
    }
   #endif

    z0 push (ComSmartPtr<ID2D1DeviceContext1> context, const D2D1_LAYER_PARAMETERS1& layerParameters)
    {
        pushedLayers.emplace_back (OwningLayer { layerParameters });
        pushedLayers.back().push (context);
    }

    z0 push (ComSmartPtr<ID2D1DeviceContext1> context, const Rectangle<f32>& r)
    {
        pushedLayers.emplace_back (r);
        pushedLayers.back().push (context);
    }

    z0 popOne (ComSmartPtr<ID2D1DeviceContext1> context)
    {
        if (pushedLayers.empty())
            return;

        pushedLayers.back().pop (context);
        pushedLayers.pop_back();
    }

    b8 isEmpty() const
    {
        return pushedLayers.empty();
    }

    z0 fillGeometryWithNoLayersActive (ComSmartPtr<ID2D1DeviceContext1> ctx,
                                         ComSmartPtr<ID2D1Geometry> geo,
                                         ComSmartPtr<ID2D1Brush> brush)
    {
        ComSmartPtr<ID2D1Factory> factory;
        ctx->GetFactory (factory.resetAndGetPointerAddress());

        const auto hasGeoLayer = std::any_of (pushedLayers.begin(),
                                              pushedLayers.end(),
                                              [] (const auto& x) { return std::holds_alternative<OwningLayer> (x.var); });

        const auto intersection = [&]() -> ComSmartPtr<ID2D1Geometry>
        {
            if (! hasGeoLayer)
                return {};

            const auto contextSize = ctx->GetPixelSize();

            ComSmartPtr<ID2D1RectangleGeometry> rect;
            factory->CreateRectangleGeometry (D2D1::RectF (0.0f,
                                                           0.0f,
                                                           (f32) contextSize.width,
                                                           (f32) contextSize.height),
                                              rect.resetAndGetPointerAddress());

            ComSmartPtr<ID2D1Geometry> clip = rect;

            for (const auto& layer : pushedLayers)
            {
                ScopedGeometryWithSink scope { factory, D2D1_FILL_MODE_WINDING };

                if (auto* l = std::get_if<OwningLayer> (&layer.var))
                {
                    clip->CombineWithGeometry (l->geometry,
                                               D2D1_COMBINE_MODE_INTERSECT,
                                               l->params.maskTransform,
                                               scope.sink);
                }
                else if (auto* r = std::get_if<Rectangle<f32>> (&layer.var))
                {
                    ComSmartPtr<ID2D1RectangleGeometry> temporaryRect;
                    factory->CreateRectangleGeometry (D2DUtilities::toRECT_F (*r),
                                                      temporaryRect.resetAndGetPointerAddress());
                    clip->CombineWithGeometry (temporaryRect,
                                               D2D1_COMBINE_MODE_INTERSECT,
                                               D2D1::Matrix3x2F::Identity(),
                                               scope.sink);
                }

                clip = scope.geometry;
            }

            return clip;
        }();

        const auto clipWithGeo = [&]() -> ComSmartPtr<ID2D1Geometry>
        {
            if (intersection == nullptr)
                return geo;

            ScopedGeometryWithSink scope { factory, D2D1_FILL_MODE_WINDING };
            intersection->CombineWithGeometry (geo,
                                               D2D1_COMBINE_MODE_INTERSECT,
                                               D2D1::Matrix3x2F::Identity(),
                                               scope.sink);
            return scope.geometry;
        }();

        if (intersection != nullptr)
        {
            std::for_each (pushedLayers.rbegin(),
                           pushedLayers.rend(),
                           [&] (const auto& layer) { layer.pop (ctx); });
        }

        {
            const ScopedBlendCopy scope { ctx };
            ctx->FillGeometry (clipWithGeo, brush);
        }

        if (intersection != nullptr)
        {
            pushedLayers.clear();

            auto newLayer = D2D1::LayerParameters1();
            newLayer.geometricMask = intersection;
            push (ctx, newLayer);
        }
    }

private:
    struct OwningLayer
    {
        explicit OwningLayer (const D2D1_LAYER_PARAMETERS1& p) : params (p) {}

        D2D1_LAYER_PARAMETERS1 params;
        ComSmartPtr<ID2D1Geometry> geometry = params.geometricMask != nullptr ? addComSmartPtrOwner (params.geometricMask) : nullptr;
        ComSmartPtr<ID2D1Brush> brush = params.opacityBrush != nullptr ? addComSmartPtrOwner (params.opacityBrush) : nullptr;
    };

    struct Layer
    {
        explicit Layer (std::variant<OwningLayer, Rectangle<f32>> v) : var (std::move (v)) {}

        z0 push (ComSmartPtr<ID2D1DeviceContext1> context) const
        {
            if (auto* layer = std::get_if<OwningLayer> (&var))
                context->PushLayer (layer->params, nullptr);
            else if (auto* rect = std::get_if<Rectangle<f32>> (&var))
                context->PushAxisAlignedClip (D2DUtilities::toRECT_F (*rect), D2D1_ANTIALIAS_MODE_ALIASED);
        }

        z0 pop (ComSmartPtr<ID2D1DeviceContext1> context) const
        {
            if (std::holds_alternative<OwningLayer> (var))
                context->PopLayer();
            else if (std::holds_alternative<Rectangle<f32>> (var))
                context->PopAxisAlignedClip();
        }

        std::variant<OwningLayer, Rectangle<f32>> var;
    };

    std::vector<Layer> pushedLayers;

    //==============================================================================
    // PushedLayer represents a Direct2D clipping or transparency layer
    //
    // D2D layers have to be pushed into the device context. Every push has to be
    // matched with a pop.
    //
    // D2D has special layers called "axis aligned clip layers" which clip to an
    // axis-aligned rectangle. Pushing an axis-aligned clip layer must be matched
    // with a call to deviceContext->PopAxisAlignedClip() in the reverse order
    // in which the layers were pushed.
    //
    // So if the pushed layer stack is built like this:
    //
    // PushLayer()
    // PushLayer()
    // PushAxisAlignedClip()
    // PushLayer()
    //
    // the layer stack must be popped like this:
    //
    // PopLayer()
    // PopAxisAlignedClip()
    // PopLayer()
    // PopLayer()
    //
    // PushedLayer, PushedAxisAlignedClipLayer, and LayerPopper all exist just to unwind the
    // layer stack accordingly.
};

struct PagesAndArea
{
    Image imageHandle;
    Span<const Direct2DPixelDataPage> pages;
    Rectangle<i32> area;

    static PagesAndArea make (const Image& image, ComSmartPtr<ID2D1Device1> device)
    {
        using GetImage = Image (*) (const Image&);
        constexpr GetImage converters[] { [] (const Image& i) { return i; },
                                          [] (const Image& i) { return NativeImageType{}.convert (i); } };

        for (auto* getImage : converters)
        {
            const auto converted = getImage (image);
            const auto native = converted.getPixelData()->getNativeExtensions();

            if (auto pages = native.getPages (device); ! pages.empty())
                return PagesAndArea { converted, std::move (pages), converted.getBounds().withPosition (native.getTopLeft()) };
        }

        // Not sure how this could happen unless the NativeImageType no longer provides Windows native details...
        jassertfalse;
        return {};
    }
};

struct Direct2DGraphicsContext::SavedState
{
public:
    // Constructor for first stack entry
    SavedState (Direct2DGraphicsContext& ownerIn,
                Rectangle<i32> frameSizeIn,
                ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                ComSmartPtr<ID2D1SolidColorBrush>& colourBrushIn,
                Direct2DDeviceResources& deviceResourcesIn)
        : owner (ownerIn),
          context (deviceContext),
          currentBrush (colourBrushIn),
          colourBrush (colourBrushIn),
          deviceResources (deviceResourcesIn),
          deviceSpaceClipList (frameSizeIn.toFloat())
    {
    }

    z0 pushLayer (const D2D1_LAYER_PARAMETERS1& layerParameters)
    {
        layers.push (context, layerParameters);
    }

    z0 pushGeometryClipLayer (ComSmartPtr<ID2D1Geometry> geometry)
    {
        if (geometry != nullptr)
            pushLayer (D2D1::LayerParameters1 (D2D1::InfiniteRect(), geometry));
    }

    z0 pushTransformedRectangleGeometryClipLayer (ComSmartPtr<ID2D1RectangleGeometry> geometry, const AffineTransform& transform)
    {
        DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, pushGeometryLayerTime)

        jassert (geometry != nullptr);
        auto layerParameters = D2D1::LayerParameters1 (D2D1::InfiniteRect(), geometry);
        layerParameters.maskTransform = D2DUtilities::transformToMatrix (transform);
        pushLayer (layerParameters);
    }

    z0 pushAliasedAxisAlignedClipLayer (const Rectangle<f32>& r)
    {
        DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, pushAliasedAxisAlignedLayerTime)

        layers.push (context, r);
    }

    z0 pushTransparencyLayer (f32 opacity)
    {
        pushLayer ({ D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::IdentityMatrix(), opacity, {}, {} });
    }

    z0 popLayers()
    {
        while (! layers.isEmpty())
            layers.popOne (context);
    }

    z0 popTopLayer()
    {
        layers.popOne (context);
    }

    z0 setFont (const Font& newFont)
    {
        font = newFont;
    }

    z0 setOpacity (f32 newOpacity)
    {
        fillType.setOpacity (newOpacity);
    }

    z0 clearFill()
    {
        linearGradient = nullptr;
        radialGradient = nullptr;
        bitmapBrush = nullptr;
        currentBrush = nullptr;
    }

    /** Translate a DRX FillType to a Direct2D brush */
    z0 updateCurrentBrush()
    {
        if (fillType.isColor())
        {
            // Reuse the same colour brush
            currentBrush = colourBrush;
        }
        else if (fillType.isTiledImage())
        {
            if (fillType.image.isNull())
                return;

            const auto device = D2DUtilities::getDeviceForContext (context);
            const auto imageFormat = fillType.image.getFormat();
            const auto targetFormat = imageFormat == Image::SingleChannel ? Image::ARGB : imageFormat;
            const auto pagesAndArea = PagesAndArea::make (fillType.image.convertedToFormat (targetFormat), device);

            if (pagesAndArea.pages.empty())
                return;

            const auto bitmap = pagesAndArea.pages.front().bitmap;

            if (bitmap == nullptr)
                return;

            D2D1_BRUSH_PROPERTIES brushProps { fillType.getOpacity(), D2DUtilities::transformToMatrix (fillType.transform) };
            auto bmProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);
            const auto hr = context->CreateBitmapBrush (bitmap,
                                                        bmProps,
                                                        brushProps,
                                                        bitmapBrush.resetAndGetPointerAddress());

            if (FAILED (hr))
                return;

            currentBrush = bitmapBrush;
        }
        else if (fillType.isGradient())
        {
            if (fillType.gradient->isRadial)
            {
                radialGradient = deviceResources.radialGradientCache.get (*fillType.gradient, context, owner.metrics.get());
                currentBrush = radialGradient;
            }
            else
            {
                linearGradient = deviceResources.linearGradientCache.get (*fillType.gradient, context, owner.metrics.get());
                currentBrush = linearGradient;
            }
        }

        updateColorBrush();
    }

    z0 updateColorBrush()
    {
        if (colourBrush && fillType.isColor())
        {
            auto colour = D2DUtilities::toCOLOR_F (fillType.colour);
            colourBrush->SetColor (colour);
        }
    }

    enum BrushTransformFlags
    {
        noTransforms = 0,
        applyWorldTransform = 1,
        applyInverseWorldTransform = 2,
        applyFillTypeTransform = 4,
        applyWorldAndFillTypeTransforms = applyFillTypeTransform | applyWorldTransform
    };

    ComSmartPtr<ID2D1Brush> getBrush (i32 flags = applyWorldAndFillTypeTransforms)
    {
        if (fillType.isInvisible())
            return nullptr;

        if (! fillType.isGradient() && ! fillType.isTiledImage())
            return currentBrush;

        Point<f32> translation{};
        AffineTransform transform{};

        if (fillType.isGradient())
        {
            if ((flags & BrushTransformFlags::applyWorldTransform) != 0)
            {
                if (currentTransform.isOnlyTranslated)
                    translation = currentTransform.offset.toFloat();
                else
                    transform = currentTransform.getTransform();
            }

            if ((flags & BrushTransformFlags::applyFillTypeTransform) != 0)
            {
                if (fillType.transform.isOnlyTranslation())
                    translation += Point (fillType.transform.getTranslationX(), fillType.transform.getTranslationY());
                else
                    transform = transform.followedBy (fillType.transform);
            }

            if ((flags & BrushTransformFlags::applyInverseWorldTransform) != 0)
            {
                if (currentTransform.isOnlyTranslated)
                    translation -= currentTransform.offset.toFloat();
                else
                    transform = transform.followedBy (currentTransform.getTransform().inverted());
            }

            const auto p1 = fillType.gradient->point1 + translation;
            const auto p2 = fillType.gradient->point2 + translation;

            if (fillType.gradient->isRadial)
            {
                const auto radius = p2.getDistanceFrom (p1);
                radialGradient->SetRadiusX (radius);
                radialGradient->SetRadiusY (radius);
                radialGradient->SetCenter ({ p1.x, p1.y });
            }
            else
            {
                linearGradient->SetStartPoint ({ p1.x, p1.y });
                linearGradient->SetEndPoint ({ p2.x, p2.y });
            }
        }
        else if (fillType.isTiledImage())
        {
            if ((flags & BrushTransformFlags::applyWorldTransform) != 0)
                transform = currentTransform.getTransform();

            if ((flags & BrushTransformFlags::applyFillTypeTransform) != 0)
                transform = transform.followedBy (fillType.transform);

            if ((flags & BrushTransformFlags::applyInverseWorldTransform) != 0)
                transform = transform.followedBy (currentTransform.getTransform().inverted());
        }

        currentBrush->SetTransform (D2DUtilities::transformToMatrix (transform));
        currentBrush->SetOpacity (fillType.getOpacity());

        return currentBrush;
    }

    b8 doesIntersectClipList (Rectangle<i32> r) const noexcept
    {
        return deviceSpaceClipList.intersects (r.toFloat());
    }

    b8 doesIntersectClipList (Rectangle<f32> r) const noexcept
    {
        return deviceSpaceClipList.intersects (r);
    }

    b8 doesIntersectClipList (Line<f32> r) const noexcept
    {
        return doesIntersectClipList (Rectangle { r.getStart(), r.getEnd() }.expanded (1.0f));
    }

    b8 doesIntersectClipList (const RectangleList<f32>& other) const noexcept
    {
        return deviceSpaceClipList.intersects (other);
    }

    b8 isCurrentTransformAxisAligned() const noexcept
    {
        return currentTransform.isOnlyTranslated || (currentTransform.complexTransform.mat01 == 0.0f && currentTransform.complexTransform.mat10 == 0.0f);
    }

    static Txt toString (const RenderingHelpers::TranslationOrTransform& t)
    {
        Txt s;
        s << "Offset " << t.offset.toString() << newLine;
        s << "Transform " << t.complexTransform.mat00 << " " << t.complexTransform.mat01 << " " << t.complexTransform.mat02 << " / ";
        s << "          " << t.complexTransform.mat10 << " " << t.complexTransform.mat11 << " " << t.complexTransform.mat12 << newLine;
        return s;
    }

    PushedLayers layers;

    Direct2DGraphicsContext& owner;

    ComSmartPtr<ID2D1DeviceContext1> context;
    ComSmartPtr<ID2D1Brush> currentBrush;
    ComSmartPtr<ID2D1SolidColorBrush>& colourBrush; // reference to shared colour brush
    ComSmartPtr<ID2D1BitmapBrush> bitmapBrush;
    ComSmartPtr<ID2D1LinearGradientBrush> linearGradient;
    ComSmartPtr<ID2D1RadialGradientBrush> radialGradient;

    RenderingHelpers::TranslationOrTransform currentTransform;

    Direct2DDeviceResources& deviceResources;
    RectangleList<f32> deviceSpaceClipList;

    Font font { FontOptions{} };

    FillType fillType;

    D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;

    DRX_LEAK_DETECTOR (SavedState)
};

static Line<f32> operator+ (Line<f32> a, Point<f32> b)
{
    return { a.getStart() + b, a.getEnd() + b };
}

static RectangleList<f32> operator+ (RectangleList<f32> a, Point<f32> b)
{
    a.offsetAll (b);
    return a;
}

//==============================================================================
struct Direct2DGraphicsContext::Pimpl : private DxgiAdapterListener
{
protected:
    Direct2DGraphicsContext& owner;
    SharedResourcePointer<DirectX> directX;
    SharedResourcePointer<Direct2DFactories> directWrite;

    std::optional<Direct2DDeviceResources> deviceResources;

    std::vector<std::unique_ptr<Direct2DGraphicsContext::SavedState>> savedClientStates;

    virtual b8 prepare()
    {
        if (! deviceResources.has_value())
            deviceResources = Direct2DDeviceResources::create (getDeviceContext());

        return deviceResources.has_value();
    }

    virtual z0 teardown()
    {
        deviceResources.reset();
    }

    virtual b8 checkPaintReady()
    {
        return deviceResources.has_value();
    }

public:
    explicit Pimpl (Direct2DGraphicsContext& ownerIn)
        : owner (ownerIn)
    {
        directX->adapters.addListener (*this);
    }

    ~Pimpl() override
    {
        directX->adapters.removeListener (*this);

        popAllSavedStates();
    }

    virtual SavedState* startFrame()
    {
        prepare();

        // Anything to paint?
        const auto paintAreas = getPaintAreas();
        const auto paintBounds = paintAreas.getBounds();

        if (! getFrameSize().intersects (paintBounds) || paintBounds.isEmpty() || paintAreas.isEmpty())
            return nullptr;

        // Is Direct2D ready to paint?
        if (! checkPaintReady())
            return nullptr;

       #if DRX_DIRECT2D_METRICS
        owner.metrics->startFrame();
       #endif

        DRX_TRACE_EVENT_INT_RECT_LIST (etw::startD2DFrame, etw::direct2dKeyword, owner.getFrameId(), paintAreas);

        const auto deviceContext = getDeviceContext();

        // Init device context transform
        resetTransform (deviceContext);

        // Start drawing
        deviceContext->SetTarget (getDeviceContextTarget());
        deviceContext->BeginDraw();

        // Init the save state stack and return the first saved state
        return pushFirstSavedState (paintBounds);
    }

    virtual HRESULT finishFrame()
    {
        // Fully pop the state stack
        popAllSavedStates();

        // Finish drawing
        // SetTarget(nullptr) so the device context doesn't hold a reference to the swap chain buffer
        HRESULT hr = S_OK;
        {
            DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, endDrawDuration)
            DRX_SCOPED_TRACE_EVENT_FRAME (etw::endDraw, etw::direct2dKeyword, owner.getFrameId());

            const auto deviceContext = getDeviceContext();
            hr = deviceContext->EndDraw();
            deviceContext->SetTarget (nullptr);
        }

        jassert (SUCCEEDED (hr));

        if (FAILED (hr))
            teardown();

       #if DRX_DIRECT2D_METRICS
        owner.metrics->finishFrame();
       #endif

        return hr;
    }

    SavedState* getCurrentSavedState() const
    {
        return ! savedClientStates.empty() ? savedClientStates.back().get() : nullptr;
    }

    SavedState* pushFirstSavedState (Rectangle<i32> initialClipRegion)
    {
        jassert (savedClientStates.empty());

        savedClientStates.push_back (std::make_unique<SavedState> (owner,
                                                                   initialClipRegion,
                                                                   getDeviceContext(),
                                                                   deviceResources->colourBrush,
                                                                   *deviceResources));

        return getCurrentSavedState();
    }

    SavedState* pushSavedState()
    {
        jassert (! savedClientStates.empty());

        savedClientStates.push_back (std::make_unique<SavedState> (*savedClientStates.back()));

        return getCurrentSavedState();
    }

    SavedState* popSavedState()
    {
        savedClientStates.back()->popLayers();
        savedClientStates.pop_back();

        return getCurrentSavedState();
    }

    z0 popAllSavedStates()
    {
        while (! savedClientStates.empty())
            popSavedState();
    }

    virtual RectangleList<i32> getPaintAreas() const = 0;
    virtual Rectangle<i32> getFrameSize() const = 0;
    virtual ComSmartPtr<ID2D1DeviceContext1> getDeviceContext() const = 0;
    virtual ComSmartPtr<ID2D1Image> getDeviceContextTarget() const = 0;

    z0 setDeviceContextTransform (AffineTransform transform)
    {
        setTransform (getDeviceContext(), transform);
    }

    z0 resetDeviceContextTransform()
    {
        resetTransform (getDeviceContext());
    }

    auto getDirect2DFactory()
    {
        return directX->getD2DFactory();
    }

    auto getDirectWriteFactory()
    {
        return directWrite->getDWriteFactory();
    }

    auto getDirectWriteFactory4()
    {
        return directWrite->getDWriteFactory4();
    }

    auto& getFontCollection()
    {
        return directWrite->getFonts();
    }

    b8 fillSpriteBatch (const RectangleList<f32>& list)
    {
        if (! owner.currentState->fillType.isColor())
            return false;

        auto* rectangleListSpriteBatch = deviceResources->rectangleListSpriteBatch.get();

        if (rectangleListSpriteBatch == nullptr)
            return false;

        const auto deviceContext = getDeviceContext();

        if (deviceContext == nullptr)
            return false;

        owner.applyPendingClipList();

        const auto& transform = owner.currentState->currentTransform;

        if (transform.isOnlyTranslated)
        {
            auto translateRectangle = [&] (const Rectangle<f32>& r) -> Rectangle<f32>
            {
                return transform.translated (r);
            };

            rectangleListSpriteBatch->fillRectangles (deviceContext,
                                                      list,
                                                      owner.currentState->fillType.colour,
                                                      translateRectangle,
                                                      owner.metrics.get());
            return true;
        }

        if (owner.currentState->isCurrentTransformAxisAligned())
        {
            auto transformRectangle = [&] (const Rectangle<f32>& r) -> Rectangle<f32>
            {
                return transform.boundsAfterTransform (r);
            };

            rectangleListSpriteBatch->fillRectangles (deviceContext,
                                                      list,
                                                      owner.currentState->fillType.colour,
                                                      transformRectangle,
                                                      owner.metrics.get());
            return true;
        }

        auto checkRectangleWithoutTransforming = [&] (const Rectangle<f32>& r) -> Rectangle<f32>
        {
            return r;
        };

        ScopedTransform scopedTransform { *this, owner.currentState };
        rectangleListSpriteBatch->fillRectangles (deviceContext,
                                                  list,
                                                  owner.currentState->fillType.colour,
                                                  checkRectangleWithoutTransforming,
                                                  owner.metrics.get());

        return true;
    }

    template <typename Shape, typename Fn>
    z0 paintPrimitive (const Shape& shape, Fn&& primitiveOp)
    {
        const auto& transform = owner.currentState->currentTransform;

        owner.applyPendingClipList();

        auto deviceContext = getDeviceContext();

        if (deviceContext == nullptr)
            return;

        const auto fillTransform = transform.isOnlyTranslated
                                 ? SavedState::BrushTransformFlags::applyWorldAndFillTypeTransforms
                                 : SavedState::BrushTransformFlags::applyFillTypeTransform;

        const auto brush = owner.currentState->getBrush (fillTransform);

        if (transform.isOnlyTranslated)
        {
            const auto translated = shape + transform.offset.toFloat();

            if (owner.currentState->doesIntersectClipList (translated))
                primitiveOp (translated, deviceContext, brush);
        }
        else if (owner.currentState->doesIntersectClipList (transform.boundsAfterTransform (shape)))
        {
            ScopedTransform scopedTransform { *this, owner.currentState };
            primitiveOp (shape, deviceContext, brush);
        }
    }

    DirectWriteGlyphRun glyphRun;

private:
    static z0 resetTransform (ID2D1DeviceContext1* context)
    {
        context->SetTransform (D2D1::IdentityMatrix());
    }

    static z0 setTransform (ID2D1DeviceContext1* context, AffineTransform newTransform)
    {
        context->SetTransform (D2DUtilities::transformToMatrix (newTransform));
    }

    DxgiAdapter::Ptr findAdapter() const
    {
        return Direct2DDeviceResources::findAdapter (directX->adapters, getDeviceContext());
    }

    z0 adapterCreated (DxgiAdapter::Ptr newAdapter) override
    {
        const auto adapter = findAdapter();

        if (adapter == nullptr || ! adapter->uniqueIDMatches (newAdapter))
            teardown();
    }

    z0 adapterRemoved (DxgiAdapter::Ptr expiringAdapter) override
    {
        const auto adapter = findAdapter();

        if (adapter != nullptr && adapter->uniqueIDMatches (expiringAdapter))
            teardown();
    }

    HWND hwnd = nullptr;

   #if DRX_DIRECT2D_METRICS
    z64 paintStartTicks = 0;
   #endif

    DRX_DECLARE_WEAK_REFERENCEABLE (Pimpl)
};

//==============================================================================
Direct2DGraphicsContext::Direct2DGraphicsContext() = default;
Direct2DGraphicsContext::~Direct2DGraphicsContext() = default;

b8 Direct2DGraphicsContext::startFrame (f32 dpiScale)
{
    const auto pimpl = getPimpl();
    const auto paintAreas = pimpl->getPaintAreas();
    currentState = pimpl->startFrame();

    if (currentState == nullptr)
        return false;

    if (auto deviceContext = pimpl->getDeviceContext())
    {
        resetPendingClipList();

        clipToRectangleList (paintAreas);

        // Clear the buffer *after* setting the clip region
        clearTargetBuffer();

        // Init font & brush
        setFont (currentState->font);
        currentState->updateCurrentBrush();

        addTransform (AffineTransform::scale (dpiScale));
    }

    return true;
}

z0 Direct2DGraphicsContext::endFrame()
{
    getPimpl()->finishFrame();

    currentState = nullptr;
    ++frame;
}

z0 Direct2DGraphicsContext::setOrigin (Point<i32> o)
{
    applyPendingClipList();

    currentState->currentTransform.setOrigin (o);

    resetPendingClipList();
}

z0 Direct2DGraphicsContext::addTransform (const AffineTransform& transform)
{
    // The pending clip list is based on the transform stored in currentState, so apply the pending clip list before adding the transform
    applyPendingClipList();

    currentState->currentTransform.addTransform (transform);

    resetPendingClipList();
}

b8 Direct2DGraphicsContext::clipToRectangle (const Rectangle<i32>& r)
{
    const auto& transform = currentState->currentTransform;
    auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

    DRX_TRACE_EVENT_INT_RECT_LIST (etw::clipToRectangle, etw::direct2dKeyword, getFrameId(), r);

    // The renderer needs to keep track of the aggregate clip rectangles in order to correctly report the
    // clip region to the caller. The renderer also needs to push Direct2D clip layers to the device context
    // to perform the actual clipping. The reported clip region will not necessarily match the Direct2D clip region
    // if the clip region is transformed, or the clip region is an image or a path.
    //
    // Pushing Direct2D clip layers is expensive and there's no need to clip until something is actually drawn.
    // So - pendingClipList is a list of the areas that need to actually be clipped. Each fill or
    // draw method then applies any pending clip areas before drawing.
    //
    // Also - calling ID2D1DeviceContext::SetTransform is expensive, so check the current transform to see
    // if the renderer can pre-transform the clip rectangle instead.
    if (transform.isOnlyTranslated)
    {
        // The current transform is only a translation, so save a few cycles by just adding the
        // offset instead of transforming the rectangle; the software renderer does something similar.
        auto translatedR = r.toFloat() + transform.offset.toFloat();
        deviceSpaceClipList.clipTo (translatedR);

        pendingClipList.clipTo (translatedR);
    }
    else if (currentState->isCurrentTransformAxisAligned())
    {
        // The current transform is a simple scale + translation, so pre-transform the rectangle
        auto transformedR = transform.boundsAfterTransform (r.toFloat());
        deviceSpaceClipList.clipTo (transformedR);

        pendingClipList.clipTo (transformedR);
    }
    else
    {
        deviceSpaceClipList = getPimpl()->getFrameSize().toFloat();

        // The current transform is too complex to pre-transform the rectangle, so just add the
        // rectangle to the clip list. The renderer will need to call ID2D1DeviceContext::SetTransform
        // before applying the clip layer.
        pendingClipList.clipTo (r.toFloat());
    }

    return ! isClipEmpty();
}

b8 Direct2DGraphicsContext::clipToRectangleList (const RectangleList<i32>& newClipList)
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::clipToRectangleList, etw::direct2dKeyword, getFrameId(), newClipList)

    const auto& transform = currentState->currentTransform;
    auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

    // This works a lot like clipToRect

    // Just one rectangle?
    if (newClipList.getNumRectangles() == 1)
        return clipToRectangle (newClipList.getRectangle (0));

    if (transform.isIdentity())
    {
        deviceSpaceClipList.clipTo (newClipList);

        pendingClipList.clipTo (newClipList);
    }
    else if (currentState->currentTransform.isOnlyTranslated)
    {
        RectangleList<i32> offsetList (newClipList);
        offsetList.offsetAll (transform.offset);
        deviceSpaceClipList.clipTo (offsetList);

        pendingClipList.clipTo (offsetList);
    }
    else if (currentState->isCurrentTransformAxisAligned())
    {
        RectangleList<f32> scaledList;

        for (auto& i : newClipList)
            scaledList.add (transform.boundsAfterTransform (i.toFloat()));

        deviceSpaceClipList.clipTo (scaledList);
        pendingClipList.clipTo (scaledList);
    }
    else
    {
        deviceSpaceClipList = getPimpl()->getFrameSize().toFloat();

        pendingClipList.clipTo (newClipList);
    }

    return ! isClipEmpty();
}

z0 Direct2DGraphicsContext::excludeClipRectangle (const Rectangle<i32>& userSpaceExcludedRectangle)
{
    DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::excludeClipRectangle, etw::direct2dKeyword, getFrameId(), userSpaceExcludedRectangle)

    auto& transform = currentState->currentTransform;
    auto& deviceSpaceClipList = currentState->deviceSpaceClipList;
    const auto frameSize = getPimpl()->getFrameSize().toFloat();

    if (transform.isOnlyTranslated)
    {
        // Just a translation; pre-translate the exclusion area
        auto translatedR = transform.translated (userSpaceExcludedRectangle.toFloat()).getLargestIntegerWithin().toFloat();

        if (! translatedR.contains (frameSize))
        {
            deviceSpaceClipList.subtract (translatedR);
            pendingClipList.subtract (translatedR);
        }
    }
    else if (currentState->isCurrentTransformAxisAligned())
    {
        // Just a scale + translation; pre-transform the exclusion area
        auto transformedR = transform.boundsAfterTransform (userSpaceExcludedRectangle.toFloat()).getLargestIntegerWithin().toFloat();

        if (! transformedR.contains (frameSize))
        {
            deviceSpaceClipList.subtract (transformedR);
            pendingClipList.subtract (transformedR);
        }
    }
    else
    {
        applyPendingClipList();

        deviceSpaceClipList = frameSize;
        pendingClipList.subtract (userSpaceExcludedRectangle.toFloat());
    }
}

z0 Direct2DGraphicsContext::resetPendingClipList()
{
    auto& transform = currentState->currentTransform;

    const auto frameSize = transform.isOnlyTranslated || currentState->isCurrentTransformAxisAligned()
                         ? getPimpl()->getFrameSize()
                         : getPimpl()->getFrameSize().transformedBy (transform.getTransform().inverted());

    pendingClipList.reset (frameSize.toFloat());
}

z0 Direct2DGraphicsContext::applyPendingClipList()
{
    if (! pendingClipList.isClipApplied())
        return;

    auto& transform = currentState->currentTransform;
    const auto axisAligned = currentState->isCurrentTransformAxisAligned();
    const auto list = pendingClipList.getList();

    // Clip if the pending clip list is not empty and smaller than the frame size
    if (! list.containsRectangle (getPimpl()->getFrameSize().toFloat()) && ! list.isEmpty())
    {
        if (list.getNumRectangles() == 1 && axisAligned)
        {
            auto r = list.getRectangle (0);
            currentState->pushAliasedAxisAlignedClipLayer (r);
        }
        else
        {
            auto clipTransform = axisAligned ? AffineTransform{} : transform.getTransform();
            if (auto clipGeometry = D2DHelpers::rectListToPathGeometry (getPimpl()->getDirect2DFactory(),
                                                                        list,
                                                                        clipTransform,
                                                                        D2D1_FILL_MODE_WINDING,
                                                                        D2D1_FIGURE_BEGIN_FILLED,
                                                                        metrics.get()))
            {
                currentState->pushGeometryClipLayer (clipGeometry);
            }
        }

        resetPendingClipList();
    }
}

z0 Direct2DGraphicsContext::clipToPath (const Path& path, const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::clipToPath, etw::direct2dKeyword, getFrameId());

    applyPendingClipList();

    // Set the clip list to the full size of the frame to match
    // the software renderer
    auto pathTransform = currentState->currentTransform.getTransformWith (transform);
    auto transformedBounds = path.getBounds().transformedBy (pathTransform);
    currentState->deviceSpaceClipList.clipTo (transformedBounds);

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        currentState->pushGeometryClipLayer (D2DHelpers::pathToPathGeometry (getPimpl()->getDirect2DFactory(),
                                                                             path,
                                                                             pathTransform,
                                                                             D2D1_FIGURE_BEGIN_FILLED,
                                                                             metrics.get()));
    }
}

z0 Direct2DGraphicsContext::clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::clipToImageAlpha, etw::direct2dKeyword, getFrameId());

    if (sourceImage.isNull())
        return;

    applyPendingClipList();

    // Put a rectangle clip layer under the image clip layer
    // The D2D bitmap brush will extend past the boundaries of sourceImage, so clip
    // to the sourceImage bounds
    auto brushTransform = currentState->currentTransform.getTransformWith (transform);

    if (D2DHelpers::isTransformAxisAligned (brushTransform))
    {
        currentState->pushAliasedAxisAlignedClipLayer (sourceImage.getBounds().toFloat().transformedBy (brushTransform));
    }
    else
    {
        const auto sourceImageRectF = D2DUtilities::toRECT_F (sourceImage.getBounds());
        ComSmartPtr<ID2D1RectangleGeometry> geometry;

        if (const auto hr = getPimpl()->getDirect2DFactory()->CreateRectangleGeometry (sourceImageRectF, geometry.resetAndGetPointerAddress());
            SUCCEEDED (hr) && geometry != nullptr)
        {
            currentState->pushTransformedRectangleGeometryClipLayer (geometry, brushTransform);
        }
    }

    // Set the clip list to the full size of the frame to match
    // the software renderer
    currentState->deviceSpaceClipList = getPimpl()->getFrameSize().toFloat();

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        const auto maxDim = (i32) deviceContext->GetMaximumBitmapSize();

        if (sourceImage.getWidth() > maxDim || sourceImage.getHeight() > maxDim)
        {
            // The Direct2D renderer doesn't currently support clipping to very large images
            jassertfalse;
            return;
        }

        const auto device = D2DUtilities::getDeviceForContext (deviceContext);
        const auto pagesAndArea = PagesAndArea::make (sourceImage, device);

        if (pagesAndArea.pages.empty())
            return;

        const auto bitmap = pagesAndArea.pages.front().bitmap;

        if (bitmap == nullptr)
            return;

        // Make a transformed bitmap brush using the bitmap
        // As usual, apply the current transform first *then* the transform parameter
        ComSmartPtr<ID2D1BitmapBrush> brush;
        auto matrix = D2DUtilities::transformToMatrix (brushTransform);
        D2D1_BRUSH_PROPERTIES brushProps = { 1.0f, matrix };

        auto bitmapBrushProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP);
        auto hr = deviceContext->CreateBitmapBrush (bitmap, bitmapBrushProps, brushProps, brush.resetAndGetPointerAddress());

        if (FAILED (hr))
            return;

        // Push the clipping layer onto the layer stack
        // Don't set maskTransform in the LayerParameters struct; that only applies to geometry clipping
        // Do set the contentBounds member, transformed appropriately
        auto layerParams = D2D1::LayerParameters1();
        auto transformedBounds = sourceImage.getBounds().toFloat().transformedBy (brushTransform);
        layerParams.contentBounds = D2DUtilities::toRECT_F (transformedBounds);
        layerParams.opacityBrush = brush;

        currentState->pushLayer (layerParams);
    }
}

b8 Direct2DGraphicsContext::clipRegionIntersects (const Rectangle<i32>& r)
{
    const auto rect = currentState->currentTransform.isOnlyTranslated ? currentState->currentTransform.translated (r.toFloat())
                                                                      : currentState->currentTransform.boundsAfterTransform (r.toFloat());
    return currentState->deviceSpaceClipList.intersectsRectangle (rect);
}

Rectangle<i32> Direct2DGraphicsContext::getClipBounds() const
{
    return currentState->currentTransform.deviceSpaceToUserSpace (currentState->deviceSpaceClipList.getBounds()).getSmallestIntegerContainer();
}

b8 Direct2DGraphicsContext::isClipEmpty() const
{
    return getClipBounds().isEmpty();
}

z0 Direct2DGraphicsContext::saveState()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::saveState, etw::direct2dKeyword, getFrameId());

    applyPendingClipList();

    currentState = getPimpl()->pushSavedState();
}

z0 Direct2DGraphicsContext::restoreState()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::restoreState, etw::direct2dKeyword, getFrameId());

    currentState = getPimpl()->popSavedState();

    currentState->updateColorBrush();
    jassert (currentState);

    resetPendingClipList();
}

z0 Direct2DGraphicsContext::beginTransparencyLayer (f32 opacity)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::beginTransparencyLayer, etw::direct2dKeyword, getFrameId());

    applyPendingClipList();

    if (auto deviceContext = getPimpl()->getDeviceContext())
        currentState->pushTransparencyLayer (opacity);
}

z0 Direct2DGraphicsContext::endTransparencyLayer()
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::endTransparencyLayer, etw::direct2dKeyword, getFrameId());

    if (auto deviceContext = getPimpl()->getDeviceContext())
        currentState->popTopLayer();
}

z0 Direct2DGraphicsContext::setFill (const FillType& fillType)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::setFill, etw::direct2dKeyword, getFrameId());

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        currentState->fillType = fillType;
        currentState->updateCurrentBrush();
    }
}

z0 Direct2DGraphicsContext::setOpacity (f32 newOpacity)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::setOpacity, etw::direct2dKeyword, getFrameId());

    currentState->setOpacity (newOpacity);

    if (auto deviceContext = getPimpl()->getDeviceContext())
        currentState->updateCurrentBrush();
}

z0 Direct2DGraphicsContext::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    switch (quality)
    {
        case Graphics::ResamplingQuality::lowResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
            break;

        case Graphics::ResamplingQuality::mediumResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;
            break;

        case Graphics::ResamplingQuality::highResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;
            break;
    }
}

z0 Direct2DGraphicsContext::fillRect (const Rectangle<i32>& r, b8 replaceExistingContents)
{
    if (r.isEmpty())
        return;

    if (replaceExistingContents)
    {
        applyPendingClipList();

        const auto asRectF = D2DUtilities::toRECT_F (r.toFloat());
        ComSmartPtr<ID2D1RectangleGeometry> rectGeometry;
        getPimpl()->getDirect2DFactory()->CreateRectangleGeometry (asRectF,
                                                                   rectGeometry.resetAndGetPointerAddress());

        const auto matrix = D2DUtilities::transformToMatrix (currentState->currentTransform.getTransform());
        ComSmartPtr<ID2D1TransformedGeometry> geo;
        getPimpl()->getDirect2DFactory()->CreateTransformedGeometry (rectGeometry,
                                                                     matrix,
                                                                     geo.resetAndGetPointerAddress());

        const auto brush = currentState->fillType.isInvisible() ? currentState->currentBrush : currentState->getBrush();
        currentState->layers.fillGeometryWithNoLayersActive (getPimpl()->getDeviceContext(), geo, brush);
        return;
    }

    const auto fill = [] (Rectangle<f32> rect,
                          ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                          ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush != nullptr)
            deviceContext->FillRectangle (D2DUtilities::toRECT_F (rect), brush);
    };

    getPimpl()->paintPrimitive (r.toFloat(), fill);
}

z0 Direct2DGraphicsContext::fillRect (const Rectangle<f32>& r)
{
    if (r.isEmpty())
        return;

    auto fill = [] (Rectangle<f32> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush != nullptr)
            deviceContext->FillRectangle (D2DUtilities::toRECT_F (rect), brush);
    };

    getPimpl()->paintPrimitive (r, fill);
}

z0 Direct2DGraphicsContext::fillRectList (const RectangleList<f32>& list)
{
    if (getPimpl()->fillSpriteBatch (list))
        return;

    auto fill = [] (const RectangleList<f32>& l, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush != nullptr)
            for (const auto& r : l)
                deviceContext->FillRectangle (D2DUtilities::toRECT_F (r), brush);
    };

    getPimpl()->paintPrimitive (list, fill);
}

z0 Direct2DGraphicsContext::drawRect (const Rectangle<f32>& r, f32 lineThickness)
{
    auto draw = [&] (Rectangle<f32> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        // ID2D1DeviceContext::DrawRectangle centers the stroke around the edges of the specified rectangle, but
        // the software renderer contains the stroke within the rectangle
        // To match the software renderer, reduce the rectangle by half the stroke width
        if (brush != nullptr)
            deviceContext->DrawRectangle (D2DUtilities::toRECT_F (rect.reduced (lineThickness * 0.5f)), brush, lineThickness);
    };

    getPimpl()->paintPrimitive (r, draw);
}

z0 Direct2DGraphicsContext::fillPath (const Path& p, const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::fillPath, etw::direct2dKeyword, getFrameId());

    if (p.isEmpty())
        return;

    applyPendingClipList();

    const auto deviceContext = getPimpl()->getDeviceContext();
    const auto brush = currentState->getBrush (SavedState::applyFillTypeTransform);
    const auto factory = getPimpl()->getDirect2DFactory();
    const auto geometry = D2DHelpers::pathToPathGeometry (factory,
                                                          p,
                                                          transform,
                                                          D2D1_FIGURE_BEGIN_FILLED,
                                                          metrics.get());

    if (deviceContext == nullptr || brush == nullptr || geometry == nullptr)
        return;

    DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, fillGeometryTime)

    ScopedTransform scopedTransform { *getPimpl(), currentState };
    deviceContext->FillGeometry (geometry, brush);
}

z0 Direct2DGraphicsContext::strokePath (const Path& p, const PathStrokeType& strokeType, const AffineTransform& transform)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::drawPath, etw::direct2dKeyword, getFrameId());

    if (p.getBounds().withZeroOrigin() == Rectangle<f32>{})
        return;

    applyPendingClipList();

    const auto deviceContext = getPimpl()->getDeviceContext();
    const auto brush = currentState->getBrush (SavedState::applyFillTypeTransform);
    const auto factory = getPimpl()->getDirect2DFactory();
    const auto strokeStyle = D2DHelpers::pathStrokeTypeToStrokeStyle (factory, strokeType);
    const auto geometry = D2DHelpers::pathToPathGeometry (factory,
                                                          p,
                                                          transform,
                                                          D2D1_FIGURE_BEGIN_HOLLOW,
                                                          metrics.get());

    if (deviceContext == nullptr || brush == nullptr || geometry == nullptr || strokeStyle == nullptr)
        return;

    DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawGeometryTime)

    ScopedTransform scopedTransform { *getPimpl(), currentState };
    deviceContext->DrawGeometry (geometry, brush, strokeType.getStrokeThickness(), strokeStyle);
}

z0 Direct2DGraphicsContext::drawImage (const Image& imageIn, const AffineTransform& transform)
{
    DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawImageTime)

    DRX_SCOPED_TRACE_EVENT_FRAME (etw::drawImage, etw::direct2dKeyword, getFrameId());

    if (imageIn.isNull())
        return;

    applyPendingClipList();

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        const auto device = D2DUtilities::getDeviceForContext (deviceContext);
        const auto pagesAndArea = PagesAndArea::make (imageIn, device);

        if (pagesAndArea.pages.empty())
        {
            jassertfalse;
            return;
        }

        const auto imageTransform = currentState->currentTransform.getTransformWith (transform);

        auto drawTiles = [&] (auto&& getRect)
        {
            for (const auto& page : pagesAndArea.pages)
            {
                if (page.bitmap == nullptr)
                    continue;

                const auto pageBounds = page.getBounds();
                const auto intersection = pageBounds.toFloat().getIntersection (pagesAndArea.area.toFloat());

                if (intersection.isEmpty())
                    continue;

                const auto src = intersection - pageBounds.getPosition().toFloat();
                const auto dst = getRect (intersection - pagesAndArea.area.getPosition().toFloat());
                const auto [srcConverted, dstConverted] = std::tuple (D2DUtilities::toRECT_F (src),
                                                                      D2DUtilities::toRECT_F (dst));

                if (page.bitmap->GetPixelFormat().format == DXGI_FORMAT_A8_UNORM)
                {
                    const auto lastColor = currentState->colourBrush->GetColor();
                    const auto lastMode = deviceContext->GetAntialiasMode();

                    currentState->colourBrush->SetColor (D2D1::ColorF (1.0f, 1.0f, 1.0f, currentState->fillType.getOpacity()));
                    deviceContext->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
                    deviceContext->FillOpacityMask (page.bitmap,
                                                    currentState->colourBrush,
                                                    dstConverted,
                                                    srcConverted);

                    deviceContext->SetAntialiasMode (lastMode);
                    currentState->colourBrush->SetColor (lastColor);
                }
                else
                {
                    deviceContext->DrawBitmap (page.bitmap,
                                               dstConverted,
                                               currentState->fillType.getOpacity(),
                                               currentState->interpolationMode,
                                               srcConverted,
                                               {});
                }
            }
        };

        if (imageTransform.isOnlyTranslation() || D2DHelpers::isTransformAxisAligned (imageTransform))
        {
            drawTiles ([&] (auto intersection)
            {
                return intersection.transformedBy (imageTransform);
            });

            return;
        }

        ScopedTransform scopedTransform { *getPimpl(), currentState, transform };

        drawTiles ([] (auto intersection)
        {
            return intersection;
        });
    }
}

z0 Direct2DGraphicsContext::drawLine (const Line<f32>& line)
{
    drawLineWithThickness (line, 1.0f);
}

z0 Direct2DGraphicsContext::drawLineWithThickness (const Line<f32>& line, f32 lineThickness)
{
    auto draw = [&] (Line<f32> l, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        const auto makePoint = [] (const auto& x) { return D2D1::Point2F (x.getX(), x.getY()); };
        deviceContext->DrawLine (makePoint (l.getStart()),
                                 makePoint (l.getEnd()),
                                 brush,
                                 lineThickness);
    };

    getPimpl()->paintPrimitive (line, draw);
}

z0 Direct2DGraphicsContext::setFont (const Font& newFont)
{
    DRX_SCOPED_TRACE_EVENT_FRAME (etw::setFont, etw::direct2dKeyword, getFrameId());

    currentState->setFont (newFont);
}

const Font& Direct2DGraphicsContext::getFont()
{
    return currentState->font;
}

f32 Direct2DGraphicsContext::getPhysicalPixelScaleFactor() const
{
    if (currentState != nullptr)
        return currentState->currentTransform.getPhysicalPixelScaleFactor();

    // If this is hit, there's no frame in progress, so the scale factor isn't meaningful
    jassertfalse;
    return 1.0f;
}

z0 Direct2DGraphicsContext::drawRoundedRectangle (const Rectangle<f32>& area, f32 cornerSize, f32 lineThickness)
{
    auto draw = [&] (Rectangle<f32> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        D2D1_ROUNDED_RECT roundedRect { D2DUtilities::toRECT_F (rect), cornerSize, cornerSize };
        deviceContext->DrawRoundedRectangle (roundedRect, brush, lineThickness);
    };

    getPimpl()->paintPrimitive (area, draw);
}

z0 Direct2DGraphicsContext::fillRoundedRectangle (const Rectangle<f32>& area, f32 cornerSize)
{
    auto fill = [&] (Rectangle<f32> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        D2D1_ROUNDED_RECT roundedRect { D2DUtilities::toRECT_F (rect), cornerSize, cornerSize };
        deviceContext->FillRoundedRectangle (roundedRect, brush);
    };

    getPimpl()->paintPrimitive (area, fill);
}

z0 Direct2DGraphicsContext::drawEllipse (const Rectangle<f32>& area, f32 lineThickness)
{
    auto draw = [&] (Rectangle<f32> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        auto centre = rect.getCentre();
        D2D1_ELLIPSE ellipse { { centre.x, centre.y }, rect.proportionOfWidth (0.5f), rect.proportionOfHeight (0.5f) };
        deviceContext->DrawEllipse (ellipse, brush, lineThickness);
    };

    getPimpl()->paintPrimitive (area, draw);
}

z0 Direct2DGraphicsContext::fillEllipse (const Rectangle<f32>& area)
{
    auto fill = [&] (Rectangle<f32> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        auto centre = rect.getCentre();
        D2D1_ELLIPSE ellipse { { centre.x, centre.y }, rect.proportionOfWidth (0.5f), rect.proportionOfHeight (0.5f) };
        deviceContext->FillEllipse (ellipse, brush);
    };

    getPimpl()->paintPrimitive (area, fill);
}

z0 Direct2DGraphicsContext::drawGlyphs (Span<u16k> glyphNumbers,
                                          Span<const Point<f32>> positions,
                                          const AffineTransform& transform)
{
    jassert (glyphNumbers.size() == positions.size());

    DRX_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawGlyphRunTime)

    DRX_SCOPED_TRACE_EVENT_FRAME (etw::drawGlyphRun, etw::direct2dKeyword, getFrameId());

    if (currentState->fillType.isInvisible() || glyphNumbers.empty() || positions.empty())
        return;

    const auto& font = currentState->font;

    const auto deviceContext = getPimpl()->getDeviceContext();

    if (! deviceContext)
        return;

    const auto typeface = font.getTypefacePtr();
    const auto fontFace = [&]() -> ComSmartPtr<IDWriteFontFace>
    {
        if (auto* x = dynamic_cast<WindowsDirectWriteTypeface*> (typeface.get()))
            return x->getIDWriteFontFace();

        return {};
    }();

    if (fontFace == nullptr)
        return;

    const auto fontScale = font.getHorizontalScale();
    const auto textTransform = AffineTransform::scale (fontScale, 1.0f).followedBy (transform);
    const auto worldTransform = currentState->currentTransform.getTransform();
    const auto textAndWorldTransform = textTransform.followedBy (worldTransform);
    const auto onlyTranslated = textAndWorldTransform.isOnlyTranslation();

    const auto fillTransform = onlyTranslated
                             ? SavedState::BrushTransformFlags::applyWorldAndFillTypeTransforms
                             : SavedState::BrushTransformFlags::applyFillTypeTransform;

    auto brush = currentState->getBrush (fillTransform);

    if (brush == nullptr)
        return;

    applyPendingClipList();

    D2D1_POINT_2F baselineOrigin { 0.0f, 0.0f };

    if (onlyTranslated)
    {
        baselineOrigin = { textAndWorldTransform.getTranslationX(), textAndWorldTransform.getTranslationY() };
    }
    else
    {
        D2D1::Matrix3x2F matrix{};
        brush->GetTransform (&matrix);
        const auto brushTransform = D2DUtilities::matrixToTransform (matrix);
        brush->SetTransform (D2DUtilities::transformToMatrix (brushTransform.followedBy (textTransform.inverted())));
        getPimpl()->setDeviceContextTransform (textAndWorldTransform);
    }

    auto& run = getPimpl()->glyphRun;
    run.replace (positions, fontScale);

    DWRITE_GLYPH_RUN directWriteGlyphRun;
    directWriteGlyphRun.fontFace = fontFace;
    directWriteGlyphRun.fontEmSize = font.getHeightInPoints();
    directWriteGlyphRun.glyphCount = (UINT32) glyphNumbers.size();
    directWriteGlyphRun.glyphIndices = glyphNumbers.data();
    directWriteGlyphRun.glyphAdvances = run.getAdvances();
    directWriteGlyphRun.glyphOffsets = run.getOffsets();
    directWriteGlyphRun.isSideways = FALSE;
    directWriteGlyphRun.bidiLevel = 0;

    const auto tryDrawColorGlyphs = [&]
    {
        // There's a helpful colour glyph rendering sample at
        // https://github.com/microsoft/Windows-universal-samples/blob/main/Samples/DWriteColorGlyph/cpp/CustomTextRenderer.cpp
        const auto factory = getPimpl()->getDirectWriteFactory4();

        if (factory == nullptr)
            return false;

        const auto ctx = deviceContext.getInterface<ID2D1DeviceContext4>();

        if (ctx == nullptr)
            return false;

        ComSmartPtr<IDWriteColorGlyphRunEnumerator1> enumerator;

        constexpr auto formats = DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE
                               | DWRITE_GLYPH_IMAGE_FORMATS_CFF
                               | DWRITE_GLYPH_IMAGE_FORMATS_COLR
                               | DWRITE_GLYPH_IMAGE_FORMATS_PNG
                               | DWRITE_GLYPH_IMAGE_FORMATS_JPEG
                               | DWRITE_GLYPH_IMAGE_FORMATS_TIFF
                               | DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8;

        if (const auto hr = factory->TranslateColorGlyphRun (baselineOrigin,
                                                             &directWriteGlyphRun,
                                                             nullptr,
                                                             formats,
                                                             DWRITE_MEASURING_MODE_NATURAL,
                                                             nullptr,
                                                             0,
                                                             enumerator.resetAndGetPointerAddress());
                FAILED (hr) || enumerator == nullptr)
        {
            // NOCOLOR is expected if the font has no colour glyphs. Other errors are not expected.
            jassert (hr == DWRITE_E_NOCOLOR && enumerator == nullptr);
            return false;
        }

        for (BOOL gotRun = false; SUCCEEDED (enumerator->MoveNext (&gotRun)) && gotRun;)
        {
            const DWRITE_COLOR_GLYPH_RUN1* colourRun = nullptr;

            if (FAILED (enumerator->GetCurrentRun (&colourRun)) || colourRun == nullptr)
                break;

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
            switch (colourRun->glyphImageFormat)
            {
                case DWRITE_GLYPH_IMAGE_FORMATS_PNG:
                case DWRITE_GLYPH_IMAGE_FORMATS_JPEG:
                case DWRITE_GLYPH_IMAGE_FORMATS_TIFF:
                case DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8:
                    ctx->DrawColorBitmapGlyphRun (colourRun->glyphImageFormat,
                                                  { colourRun->baselineOriginX, colourRun->baselineOriginY },
                                                  &colourRun->glyphRun,
                                                  colourRun->measuringMode);
                    break;

                case DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE:
                case DWRITE_GLYPH_IMAGE_FORMATS_CFF:
                case DWRITE_GLYPH_IMAGE_FORMATS_COLR:
                default:
                {
                    const auto useForeground = colourRun->paletteIndex == 0xffff;
                    const auto lastColor = currentState->colourBrush->GetColor();
                    const auto colourBrush = currentState->colourBrush;

                    if (! useForeground)
                        colourBrush->SetColor (colourRun->runColor);

                    const auto brushToUse = useForeground ? ComSmartPtr<ID2D1Brush> (brush)
                                                          : ComSmartPtr<ID2D1Brush> (colourBrush);

                    ctx->DrawGlyphRun ({ colourRun->baselineOriginX, colourRun->baselineOriginY },
                                       &colourRun->glyphRun,
                                       colourRun->glyphRunDescription,
                                       brushToUse,
                                       colourRun->measuringMode);

                    if (! useForeground)
                        colourBrush->SetColor (lastColor);

                    break;
                }
            }
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
        }

        return true;
    };

    if (! tryDrawColorGlyphs())
        deviceContext->DrawGlyphRun (baselineOrigin, &directWriteGlyphRun, brush);

    if (! onlyTranslated)
        getPimpl()->resetDeviceContextTransform();
}

Direct2DGraphicsContext::ScopedTransform::ScopedTransform (Pimpl& pimplIn, SavedState* stateIn)
    : pimpl (pimplIn), state (stateIn)
{
    pimpl.setDeviceContextTransform (stateIn->currentTransform.getTransform());
}

Direct2DGraphicsContext::ScopedTransform::ScopedTransform (Pimpl& pimplIn, SavedState* stateIn, const AffineTransform& transform)
    : pimpl (pimplIn), state (stateIn)
{
    pimpl.setDeviceContextTransform (stateIn->currentTransform.getTransformWith (transform));
}

Direct2DGraphicsContext::ScopedTransform::~ScopedTransform()
{
    pimpl.resetDeviceContextTransform();
}

//==============================================================================
//==============================================================================

#if DRX_UNIT_TESTS

class Direct2DGraphicsContextTests : public UnitTest
{
public:
    Direct2DGraphicsContextTests() : UnitTest ("Direct2D Graphics Context", UnitTestCategories::graphics) {}

    z0 runTest() override
    {
        const auto imageWidth = 1 << 15;
        const auto imageHeight = 128;
        Image largeImageSoftware { Image::RGB, imageWidth, imageHeight, false, SoftwareImageType{} };

        {
            Graphics g { largeImageSoftware };
            g.setGradientFill ({ Colors::red, 0, 0, Colors::cyan, (f32) largeImageSoftware.getWidth(), 0, false });
            g.fillAll();
        }

        constexpr auto targetDim = 512;

        const auto largeImageNative = NativeImageType{}.convert (largeImageSoftware);
        const auto subsection = largeImageNative.getClippedImage (largeImageNative.getBounds().withSizeKeepingCentre (1 << 14, 64));

        beginTest ("Render large images");
        {
            for (const auto& imageToDraw : { largeImageNative, subsection })
            {
                const AffineTransform transformsToTest[]
                {
                    {},
                    AffineTransform::translation ((f32) targetDim - (f32) imageToDraw.getWidth(), 0),
                    AffineTransform::translation (0, (f32) targetDim - (f32) imageToDraw.getHeight()),
                    AffineTransform::scale ((f32) targetDim / imageWidth),
                    AffineTransform::scale ((f32) targetDim / imageWidth)
                            .followedBy (AffineTransform::translation (32, 64)),
                    AffineTransform::scale (1.1f),
                    AffineTransform::scale ((f32) targetDim / imageWidth,
                                            (f32) targetDim / imageHeight),
                    AffineTransform::rotation (MathConstants<f32>::pi * 0.25f),
                    AffineTransform::rotation (MathConstants<f32>::pi * 0.25f, imageWidth * 0.5f, 0)
                            .followedBy (AffineTransform::translation (-imageWidth * 0.5f, 0)),
                };

                for (const auto& transform : transformsToTest)
                {
                    Image targetNative { Image::RGB, targetDim, targetDim, true, NativeImageType{} };
                    Image targetSoftware { Image::RGB, targetDim, targetDim, true, SoftwareImageType{} };

                    for (auto& image : { &targetNative, &targetSoftware })
                    {
                        Graphics g { *image };
                        g.drawImageTransformed (imageToDraw, transform);
                    }

                    compareImages (targetNative, targetSoftware);
                }
            }
        }
    }

    z0 compareImages (const Image& a, const Image& b)
    {
        expect (a.getBounds() == b.getBounds());

        const Image::BitmapData bitmapA { a, Image::BitmapData::readOnly };
        const Image::BitmapData bitmapB { b, Image::BitmapData::readOnly };

        z64 accumulatedError{};
        z64 numSamples{};

        for (auto y = 0; y < a.getHeight(); y += 16)
        {
            for (auto x = 0; x < a.getWidth(); x += 16)
            {
                const auto expected = bitmapA.getPixelColor (x, y);
                const auto actual   = bitmapB.getPixelColor (x, y);

                for (auto& fn : { &Color::getRed, &Color::getGreen, &Color::getBlue, &Color::getAlpha })
                {
                    accumulatedError += ((z64) (actual.*fn)() - (z64) (expected.*fn)());
                    ++numSamples;
                }
            }
        }

        const auto averageError = (f64) accumulatedError / (f64) numSamples;
        expect (std::abs (averageError) < 1.0);
    }
};

static Direct2DGraphicsContextTests direct2DGraphicsContextTests;

#endif

} // namespace drx
