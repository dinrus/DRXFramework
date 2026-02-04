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

namespace BitmapDataDetail
{
    template <auto T>
    using FormatConstant = std::integral_constant<decltype (T), T>;

    using ARGB = FormatConstant<Image::PixelFormat::ARGB>;
    using RGB  = FormatConstant<Image::PixelFormat::RGB>;
    using A    = FormatConstant<Image::PixelFormat::SingleChannel>;

    static Color getPixelColor (u8k* pixel, A)
    {
        return Color (*((const PixelAlpha*) pixel));
    }

    static Color getPixelColor (u8k* pixel, RGB)
    {
        return Color (*((const PixelRGB*) pixel));
    }

    static Color getPixelColor (u8k* pixel, ARGB)
    {
        return Color (((const PixelARGB*) pixel)->getUnpremultiplied());
    }

    static z0 setPixelColor (u8* pixel, PixelARGB col, A)
    {
        ((PixelAlpha*) pixel)->set (col);
    }

    static z0 setPixelColor (u8* pixel, PixelARGB col, RGB)
    {
        ((PixelRGB*) pixel)->set (col);
    }

    static z0 setPixelColor (u8* pixel, PixelARGB col, ARGB)
    {
        ((PixelARGB*) pixel)->set (col);
    }

    using ConverterFn = z0 (*) (const Image::BitmapData& src, const Image::BitmapData& dst, i32 w, i32 h);

    template <typename From, typename To>
    static constexpr ConverterFn makeConverterFn (From, To)
    {
        struct GetPixel
        {
            explicit GetPixel (const Image::BitmapData& bd)
                : data (bd.data),
                  lineStride ((size_t) bd.lineStride),
                  pixelStride ((size_t) bd.pixelStride) {}

            u8* operator() (i32 x, i32 y) const
            {
                return data + (size_t) y * (size_t) lineStride + (size_t) x * (size_t) pixelStride;
            }

            u8* data;
            size_t lineStride, pixelStride;
        };

        return [] (const Image::BitmapData& src, const Image::BitmapData& dst, i32 w, i32 h)
        {
            const GetPixel getSrc { src }, getDst { dst };

            for (i32 y = 0; y < h; ++y)
            {
                for (i32 x = 0; x < w; ++x)
                {
                    const auto srcColor = getPixelColor (getSrc (x, y), From{});
                    setPixelColor (getDst (x, y), srcColor.getPixelARGB(), To{});
                }
            }
        };
    }

    template <typename From, typename... To>
    static constexpr auto makeConverterFns (From from, To... to)
    {
        return std::array<ConverterFn, sizeof... (To)> { makeConverterFn (from, to)... };
    }

    /** This structure holds a 2D array of function pointers.
        The structure is indexed by source format and destination format, where the function
        at that index will convert an entire BitmapData array between those two formats.

        This approach is designed to avoid branching, especially switch statements, from
        the inner loop of the conversion. At time of writing, compilers cannot automatically
        vectorise loops containing switch statements. Therefore, it's often faster to move
        switches or table lookups outside tight loops.

        This is the old, slow approach:

        @code
            for (i32 y = 0; y < dest.height; ++y)
                for (i32 x = 0; x < dest.width; ++x)
                    dest.setPixelColor (x, y, src.getPixelColor (x, y));
        @endcode

        This is a faster way to write the same thing:

        @code
            if (const auto* converter = converterFnTable.getConverterFor (src.pixelFormat, dest.pixelFormat))
                converter (src, dest, dest.width, dest.height);
        @endcode
    */
    template <typename... Formats>
    class ConverterFnTable
    {
    public:
        constexpr ConverterFn getConverterFor (Image::PixelFormat src, Image::PixelFormat dst) const
        {
            const auto srcIndex = toIndex (src, Formats{}...);
            const auto dstIndex = toIndex (dst, Formats{}...);

            if (srcIndex >= sizeof... (Formats) || dstIndex >= sizeof... (Formats))
                return nullptr;

            return table[srcIndex][dstIndex];
        }

    private:
        static size_t toIndex (Image::PixelFormat)
        {
            return 0;
        }

        template <typename Head, typename... Tail>
        static size_t toIndex (Image::PixelFormat src, Head head, Tail... tail)
        {
            return src == head() ? 0 : 1 + toIndex (src, tail...);
        }

        std::array<std::array<ConverterFn, sizeof... (Formats)>, sizeof... (Formats)> table
        {
            makeConverterFns (Formats{}, Formats{}...)...
        };
    };

    static b8 convert (const Image::BitmapData& src, Image::BitmapData& dest)
    {
        if (std::tuple (src.width, src.height) != std::tuple (dest.width, dest.height))
            return false;

        static constexpr auto converterFnTable = ConverterFnTable<RGB, ARGB, A>{};

        if (src.pixelStride == dest.pixelStride && src.pixelFormat == dest.pixelFormat)
        {
            for (i32 y = 0; y < dest.height; ++y)
                memcpy (dest.getLinePointer (y), src.getLinePointer (y), (size_t) dest.pixelStride * (size_t) dest.width);
        }
        else
        {
            if (auto* converter = converterFnTable.getConverterFor (src.pixelFormat, dest.pixelFormat))
                converter (src, dest, dest.width, dest.height);
        }

        return true;
    }

    static Image convert (const Image::BitmapData& src, const ImageType& type)
    {
        Image result (type.create (src.pixelFormat, src.width, src.height, false));
        Image::BitmapData (result, Image::BitmapData::writeOnly).convertFrom (src);

        return result;
    }

    static z0 blurDataTriplets (u8* d, i32 num, i32k delta) noexcept
    {
        u32 last = d[0];
        d[0] = (u8) ((d[0] + d[delta] + 1) / 3);
        d += delta;

        num -= 2;

        do
        {
            u32k newLast = d[0];
            d[0] = (u8) ((last + d[0] + d[delta] + 1) / 3);
            d += delta;
            last = newLast;
        }
        while (--num > 0);

        d[0] = (u8) ((last + d[0] + 1) / 3);
    }

    static z0 blurSingleChannelImage (u8* const data, i32k w, i32k h,
                                        i32k lineStride, i32k repetitions) noexcept
    {
        jassert (w > 2 && h > 2);

        for (i32 y = 0; y < h; ++y)
            for (i32 i = repetitions; --i >= 0;)
                blurDataTriplets (data + lineStride * y, w, 1);

        for (i32 x = 0; x < w; ++x)
            for (i32 i = repetitions; --i >= 0;)
                blurDataTriplets (data + x, h, lineStride);
    }

    template <class PixelType>
    struct PixelIterator
    {
        template <class PixelOperation>
        static z0 iterate (const Image::BitmapData& data, const PixelOperation& pixelOp)
        {
            for (i32 y = 0; y < data.height; ++y)
                for (i32 x = 0; x < data.width; ++x)
                    pixelOp (*reinterpret_cast<PixelType*> (data.getPixelPointer (x, y)));
        }
    };

    template <class PixelOperation>
    static z0 performPixelOp (const Image::BitmapData& data, const PixelOperation& pixelOp)
    {
        switch (data.pixelFormat)
        {
            case Image::ARGB:           PixelIterator<PixelARGB> ::iterate (data, pixelOp); break;
            case Image::RGB:            PixelIterator<PixelRGB>  ::iterate (data, pixelOp); break;
            case Image::SingleChannel:  PixelIterator<PixelAlpha>::iterate (data, pixelOp); break;
            case Image::UnknownFormat:
            default:                    jassertfalse; break;
        }
    }
}

//==============================================================================
/*  Allows access to ImagePixelData implementation details by LowLevelGraphicsContext instances.
    The internal templating is mainly to facilitate returning a type with dynamic implementation by value.
*/
class ImagePixelDataNativeExtensions
{
public:
    template <typename Impl>
    explicit ImagePixelDataNativeExtensions (Impl x)
        : impl (std::make_unique<Concrete<Impl>> (std::move (x))) {}

    /*  For subsection images, this returns the top-left pixel inside the root image */
    Point<i32> getTopLeft() const { return impl->getTopLeft(); }

   #if DRX_WINDOWS
    Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1> x) const { return impl->getPages (x); }
   #endif

   #if DRX_MAC || DRX_IOS
    CGContextRef getCGContext() const { return impl->getCGContext(); }
    CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef x) const { return impl->getCGImage (x); }
   #endif

private:
    struct Base
    {
        virtual ~Base() = default;
        virtual Point<i32> getTopLeft() const = 0;

       #if DRX_WINDOWS
        virtual Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1>) const = 0;
       #endif

       #if DRX_MAC || DRX_IOS
        virtual CGContextRef getCGContext() const = 0;
        virtual CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef x) const = 0;
       #endif
    };

    template <typename Impl>
    class Concrete : public Base
    {
    public:
        explicit Concrete (Impl x)
            : impl (std::move (x)) {}

        Point<i32> getTopLeft() const override { return impl.getTopLeft(); }

       #if DRX_WINDOWS
        Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1> x) const override { return impl.getPages (x); }
       #endif

       #if DRX_MAC || DRX_IOS
        CGContextRef getCGContext() const override { return impl.getCGContext(); }
        CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef x) const override { return impl.getCGImage (x); }
       #endif

    private:
        Impl impl;
    };

    std::unique_ptr<Base> impl;
};

//==============================================================================
class SubsectionPixelData : public ImagePixelData
{
public:
    using Ptr = ReferenceCountedObjectPtr<SubsectionPixelData>;

    SubsectionPixelData (ImagePixelData::Ptr source, Rectangle<i32> r)
        : ImagePixelData (source->pixelFormat, r.getWidth(), r.getHeight()),
          sourceImage (std::move (source)),
          area (r)
    {
    }

    Rectangle<i32>      getSubsection()      const { return area; }
    ImagePixelData::Ptr getSourcePixelData() const { return sourceImage; }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        auto g = sourceImage->createLowLevelContext();
        g->clipToRectangle (area);
        g->setOrigin (area.getPosition());
        return g;
    }

    z0 initialiseBitmapData (Image::BitmapData& bitmap, i32 x, i32 y, Image::BitmapData::ReadWriteMode mode) override
    {
        sourceImage->initialiseBitmapData (bitmap, x + area.getX(), y + area.getY(), mode);

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData::Ptr clone() override
    {
        jassert (getReferenceCount() > 0); // (This method can't be used on an unowned pointer, as it will end up self-deleting)
        auto type = createType();
        auto result = type->create (pixelFormat, area.getWidth(), area.getHeight(), pixelFormat != Image::RGB);

        {
            Graphics g { Image { result } };
            g.drawImageAt (Image (*this), 0, 0);
        }

        return result;
    }

    std::unique_ptr<ImageType> createType() const override { return sourceImage->createType(); }

    z0 applySingleChannelBoxBlurEffectInArea (Rectangle<i32> b, i32 radius) override
    {
        sourceImage->applySingleChannelBoxBlurEffectInArea (getIntersection (b), radius);
    }

    z0 applyGaussianBlurEffectInArea (Rectangle<i32> b, f32 radius) override
    {
        sourceImage->applyGaussianBlurEffectInArea (getIntersection (b), radius);
    }

    z0 multiplyAllAlphasInArea (Rectangle<i32> b, f32 amount) override
    {
        sourceImage->multiplyAllAlphasInArea (getIntersection (b), amount);
    }

    z0 desaturateInArea (Rectangle<i32> b) override
    {
        sourceImage->desaturateInArea (getIntersection (b));
    }

    /* as we always hold a reference to image, don't f64 count */
    i32 getSharedCount() const noexcept override { return getReferenceCount() + sourceImage->getSharedCount() - 1; }

    NativeExtensions getNativeExtensions() override
    {
        struct Wrapped
        {
            explicit Wrapped (Ptr selfIn)
                : self (selfIn) {}

           #if DRX_WINDOWS
            Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1> x) const
            {
                return self->sourceImage->getNativeExtensions().getPages (x);
            }
           #endif

           #if DRX_MAC || DRX_IOS
            CGContextRef getCGContext() const
            {
                return self->sourceImage->getNativeExtensions().getCGContext();
            }

            CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef colourSpace) const
            {
                const auto& parentNative = self->sourceImage->getNativeExtensions();
                const auto parentImage = parentNative.getCGImage (colourSpace);
                return CFUniquePtr<CGImageRef> { CGImageCreateWithImageInRect (parentImage.get(),
                                                                               makeCGRect (self->area + parentNative.getTopLeft())) };
            }
           #endif

            Point<i32> getTopLeft() const
            {
                return self->sourceImage->getNativeExtensions().getTopLeft() + self->area.getTopLeft();
            }

            Ptr self;
        };

        return NativeExtensions { Wrapped { this } };
    }

private:
    Rectangle<i32> getIntersection (Rectangle<i32> b) const
    {
        return area.getIntersection (b + area.getTopLeft());
    }

    friend class Image;
    const ImagePixelData::Ptr sourceImage;
    const Rectangle<i32> area;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubsectionPixelData)
};

//==============================================================================
ImagePixelData::ImagePixelData (Image::PixelFormat format, i32 w, i32 h)
    : pixelFormat (format), width (w), height (h)
{
    jassert (format == Image::RGB || format == Image::ARGB || format == Image::SingleChannel);
    jassert (w > 0 && h > 0); // It's illegal to create a zero-sized image!
}

ImagePixelData::~ImagePixelData()
{
    listeners.call ([this] (Listener& l) { l.imageDataBeingDeleted (this); });
}

z0 ImagePixelData::sendDataChangeMessage()
{
    listeners.call ([this] (Listener& l) { l.imageDataChanged (this); });
}

i32 ImagePixelData::getSharedCount() const noexcept
{
    return getReferenceCount();
}

z0 ImagePixelData::applySingleChannelBoxBlurEffectInArea (Rectangle<i32> bounds, i32 radius)
{
    if (pixelFormat == Image::SingleChannel)
    {
        const Image::BitmapData bm (Image { this }, bounds, Image::BitmapData::readWrite);
        BitmapDataDetail::blurSingleChannelImage (bm.data, bm.width, bm.height, bm.lineStride, 2 * radius);
    }
}

z0 ImagePixelData::applyGaussianBlurEffectInArea (Rectangle<i32> bounds, f32 radius)
{
    ImageConvolutionKernel blurKernel (roundToInt (radius * 2.0f));
    blurKernel.createGaussianBlur (radius);

    Image target { this };
    blurKernel.applyToImage (target, Image { this }.createCopy(), bounds);
}

z0 ImagePixelData::multiplyAllAlphasInArea (Rectangle<i32> b, f32 amount)
{
    if (pixelFormat == Image::ARGB || pixelFormat == Image::SingleChannel)
    {
        const Image::BitmapData destData (Image { this }, b, Image::BitmapData::readWrite);
        BitmapDataDetail::performPixelOp (destData, [&] (auto& p) { p.multiplyAlpha (amount); });
    }
}

z0 ImagePixelData::desaturateInArea (Rectangle<i32> b)
{
    if (pixelFormat == Image::ARGB || pixelFormat == Image::RGB)
    {
        const Image::BitmapData destData (Image { this }, b, Image::BitmapData::readWrite);
        BitmapDataDetail::performPixelOp (destData, [] (auto& p) { p.desaturate(); });
    }
}

auto ImagePixelData::getNativeExtensions() -> NativeExtensions
{
    struct Wrapped
    {
        Point<i32> getTopLeft() const { return {}; }

       #if DRX_WINDOWS
        Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1>) const { return {}; }
       #endif

       #if DRX_MAC || DRX_IOS
        CGContextRef getCGContext() const { return {}; }
        CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef) const { return {}; }
       #endif
    };

    return NativeExtensions { Wrapped{} };
}

//==============================================================================
ImageType::ImageType() = default;
ImageType::~ImageType() = default;

Image ImageType::convert (const Image& source) const
{
    if (source.isNull() || getTypeID() == source.getPixelData()->createType()->getTypeID())
        return source;

    const Image::BitmapData src (source, Image::BitmapData::readOnly);

    if (src.data == nullptr)
        return {};

    return BitmapDataDetail::convert (src, *this);
}

//==============================================================================
class SoftwarePixelData : public ImagePixelData
{
public:
    SoftwarePixelData (Image::PixelFormat formatToUse, i32 w, i32 h, b8 clearImage)
        : ImagePixelData (formatToUse, w, h),
          pixelStride (formatToUse == Image::RGB ? 3 : ((formatToUse == Image::ARGB) ? 4 : 1)),
          lineStride ((pixelStride * jmax (1, w) + 3) & ~3)
    {
        imageData.allocate ((size_t) lineStride * (size_t) jmax (1, h), clearImage);
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (*this));
    }

    z0 initialiseBitmapData (Image::BitmapData& bitmap, i32 x, i32 y, Image::BitmapData::ReadWriteMode mode) override
    {
        const auto offset = (size_t) x * (size_t) pixelStride + (size_t) y * (size_t) lineStride;
        bitmap.data = imageData + offset;
        bitmap.size = (size_t) (height * lineStride) - offset;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    Ptr clone() override
    {
        auto s = new SoftwarePixelData (pixelFormat, width, height, false);
        memcpy (s->imageData, imageData, (size_t) lineStride * (size_t) height);
        return *s;
    }

    std::unique_ptr<ImageType> createType() const override    { return std::make_unique<SoftwareImageType>(); }

private:
    HeapBlock<u8> imageData;
    i32k pixelStride, lineStride;

    DRX_LEAK_DETECTOR (SoftwarePixelData)
};

SoftwareImageType::SoftwareImageType() = default;
SoftwareImageType::~SoftwareImageType() = default;

ImagePixelData::Ptr SoftwareImageType::create (Image::PixelFormat format, i32 width, i32 height, b8 clearImage) const
{
    return *new SoftwarePixelData (format, width, height, clearImage);
}

i32 SoftwareImageType::getTypeID() const
{
    return 2;
}

//==============================================================================
NativeImageType::NativeImageType() = default;
NativeImageType::~NativeImageType() = default;

i32 NativeImageType::getTypeID() const
{
    return 1;
}

#if DRX_LINUX || DRX_BSD
ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, i32 width, i32 height, b8 clearImage) const
{
    return new SoftwarePixelData (format, width, height, clearImage);
}
#endif

//==============================================================================

Image Image::getClippedImage (const Rectangle<i32>& area) const
{
    if (area.contains (getBounds()))
        return *this;

    auto validArea = area.getIntersection (getBounds());

    if (validArea.isEmpty())
        return {};

    return Image { ImagePixelData::Ptr { new SubsectionPixelData { image, validArea } } };
}

//==============================================================================
Image::Image() noexcept = default;

Image::Image (ReferenceCountedObjectPtr<ImagePixelData> instance) noexcept
    : image (std::move (instance))
{
}

Image::Image (PixelFormat format, i32 width, i32 height, b8 clearImage)
    : image (NativeImageType().create (format, width, height, clearImage))
{
}

Image::Image (PixelFormat format, i32 width, i32 height, b8 clearImage, const ImageType& type)
    : image (type.create (format, width, height, clearImage))
{
}

Image::Image (const Image& other) noexcept
    : image (other.image)
{
}

Image& Image::operator= (const Image& other)
{
    image = other.image;
    return *this;
}

Image::Image (Image&& other) noexcept
    : image (std::move (other.image))
{
}

Image& Image::operator= (Image&& other) noexcept
{
    image = std::move (other.image);
    return *this;
}

Image::~Image() = default;

i32 Image::getReferenceCount() const noexcept           { return image == nullptr ? 0 : image->getSharedCount(); }

b8 Image::isValid() const noexcept
{
    return image != nullptr;
}

i32 Image::getWidth() const noexcept                    { return image == nullptr ? 0 : image->width; }
i32 Image::getHeight() const noexcept                   { return image == nullptr ? 0 : image->height; }
Rectangle<i32> Image::getBounds() const noexcept        { return image == nullptr ? Rectangle<i32>() : Rectangle<i32> (image->width, image->height); }
Image::PixelFormat Image::getFormat() const noexcept    { return image == nullptr ? UnknownFormat : image->pixelFormat; }
b8 Image::isARGB() const noexcept                     { return getFormat() == ARGB; }
b8 Image::isRGB() const noexcept                      { return getFormat() == RGB; }
b8 Image::isSingleChannel() const noexcept            { return getFormat() == SingleChannel; }
b8 Image::hasAlphaChannel() const noexcept            { return getFormat() != RGB; }

std::unique_ptr<LowLevelGraphicsContext> Image::createLowLevelContext() const
{
    if (image != nullptr)
        return image->createLowLevelContext();

    return {};
}

z0 Image::duplicateIfShared()
{
    if (getReferenceCount() > 1)
        image = image->clone();
}

Image Image::createCopy() const
{
    if (image != nullptr)
        return Image (image->clone());

    return {};
}

Image Image::rescaled (i32 newWidth, i32 newHeight, Graphics::ResamplingQuality quality) const
{
    if (image == nullptr || (image->width == newWidth && image->height == newHeight))
        return *this;

    auto type = image->createType();
    Image newImage (type->create (image->pixelFormat, newWidth, newHeight, hasAlphaChannel()));

    Graphics g (newImage);
    g.setImageResamplingQuality (quality);
    g.drawImageTransformed (*this, AffineTransform::scale ((f32) newWidth  / (f32) image->width,
                                                           (f32) newHeight / (f32) image->height), false);
    return newImage;
}

Image Image::convertedToFormat (PixelFormat newFormat) const
{
    if (image == nullptr || newFormat == image->pixelFormat)
        return *this;

    auto w = image->width, h = image->height;

    auto type = image->createType();
    Image newImage (type->create (newFormat, w, h, false));

    if (newFormat == SingleChannel)
    {
        if (! hasAlphaChannel())
        {
            newImage.clear (getBounds(), Colors::black);
        }
        else
        {
            const BitmapData destData (newImage, { w, h }, BitmapData::writeOnly);
            const BitmapData srcData (*this, { w, h }, BitmapData::readOnly);

            for (i32 y = 0; y < h; ++y)
            {
                auto src = reinterpret_cast<const PixelARGB*> (srcData.getLinePointer (y));
                auto dst = destData.getLinePointer (y);

                for (i32 x = 0; x < w; ++x)
                    dst[x] = src[x].getAlpha();
            }
        }
    }
    else if (image->pixelFormat == SingleChannel && newFormat == Image::ARGB)
    {
        const BitmapData destData (newImage, { w, h }, BitmapData::writeOnly);
        const BitmapData srcData (*this, { w, h }, BitmapData::readOnly);

        for (i32 y = 0; y < h; ++y)
        {
            auto src = reinterpret_cast<const PixelAlpha*> (srcData.getLinePointer (y));
            auto dst = reinterpret_cast<PixelARGB*> (destData.getLinePointer (y));

            for (i32 x = 0; x < w; ++x)
                dst[x].set (src[x]);
        }
    }
    else
    {
        if (hasAlphaChannel())
            newImage.clear (getBounds());

        Graphics g (newImage);
        g.drawImageAt (*this, 0, 0);
    }

    return newImage;
}

NamedValueSet* Image::getProperties() const
{
    return image == nullptr ? nullptr : &(image->userData);
}

//==============================================================================
Image::BitmapData::BitmapData (Image& im, i32 x, i32 y, i32 w, i32 h, ReadWriteMode mode)
    : BitmapData (im, { x, y, w, h }, mode)
{
}

Image::BitmapData::BitmapData (const Image& im, Rectangle<i32> bounds, ReadWriteMode mode)
    : width (bounds.getWidth()), height (bounds.getHeight())
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (im.image != nullptr);
    jassert (bounds.getX() >= 0);
    jassert (bounds.getY() >= 0);
    jassert (bounds.getWidth() > 0);
    jassert (bounds.getHeight() > 0);
    jassert (bounds.getRight() <= im.getWidth());
    jassert (bounds.getBottom() <= im.getHeight());

    im.image->initialiseBitmapData (*this, bounds.getX(), bounds.getY(), mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::BitmapData (const Image& im, i32 x, i32 y, i32 w, i32 h)
    : width (w), height (h)
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (im.image != nullptr);
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= im.getWidth() && y + h <= im.getHeight());

    im.image->initialiseBitmapData (*this, x, y, readOnly);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::BitmapData (const Image& im, ReadWriteMode mode)
    : width (im.getWidth()),
      height (im.getHeight())
{
    // The BitmapData class must be given a valid image!
    jassert (im.image != nullptr);

    im.image->initialiseBitmapData (*this, 0, 0, mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Color Image::BitmapData::getPixelColor (i32 x, i32 y) const noexcept
{
    auto* pixel = getPixelPointer (x, y);

    switch (pixelFormat)
    {
        case ARGB:           return BitmapDataDetail::getPixelColor (pixel, BitmapDataDetail::ARGB{});
        case RGB:            return BitmapDataDetail::getPixelColor (pixel, BitmapDataDetail::RGB{});
        case SingleChannel:  return BitmapDataDetail::getPixelColor (pixel, BitmapDataDetail::A{});
        case UnknownFormat:
        default:             jassertfalse; break;
    }

    return {};
}

z0 Image::BitmapData::setPixelColor (i32 x, i32 y, Color colour) const noexcept
{
    auto* pixel = getPixelPointer (x, y);
    auto col = colour.getPixelARGB();

    switch (pixelFormat)
    {
        case ARGB:           return BitmapDataDetail::setPixelColor (pixel, col, BitmapDataDetail::ARGB{});
        case RGB:            return BitmapDataDetail::setPixelColor (pixel, col, BitmapDataDetail::RGB{});
        case SingleChannel:  return BitmapDataDetail::setPixelColor (pixel, col, BitmapDataDetail::A{});
        case UnknownFormat:
        default:             jassertfalse; break;
    }
}

b8 Image::BitmapData::convertFrom (const BitmapData& source)
{
    return BitmapDataDetail::convert (source, *this);
}

b8 Image::setBackupEnabled (b8 enabled)
{
    if (auto ptr = image)
    {
        if (auto* ext = ptr->getBackupExtensions())
        {
            ext->setBackupEnabled (enabled);
            return true;
        }
    }

    return false;
}

//==============================================================================
z0 Image::clear (const Rectangle<i32>& area, Color colourToClearTo)
{
    if (image == nullptr)
        return;

    auto g = image->createLowLevelContext();
    g->setFill (colourToClearTo);
    g->fillRect (area, true);
}

//==============================================================================
Color Image::getPixelAt (i32 x, i32 y) const
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight()))
    {
        const BitmapData srcData (*this, x, y, 1, 1);
        return srcData.getPixelColor (0, 0);
    }

    return {};
}

z0 Image::setPixelAt (i32 x, i32 y, Color colour)
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight()))
    {
        const BitmapData destData (*this, x, y, 1, 1, BitmapData::writeOnly);
        destData.setPixelColor (0, 0, colour);
    }
}

z0 Image::multiplyAlphaAt (i32 x, i32 y, f32 multiplier)
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight())
         && hasAlphaChannel())
    {
        const BitmapData destData (*this, x, y, 1, 1, BitmapData::readWrite);

        if (isARGB())
            reinterpret_cast<PixelARGB*> (destData.data)->multiplyAlpha (multiplier);
        else
            *(destData.data) = (u8) (*(destData.data) * multiplier);
    }
}

z0 Image::multiplyAllAlphas (f32 amountToMultiplyBy)
{
    if (auto ptr = image)
        ptr->multiplyAllAlphas (amountToMultiplyBy);
}

z0 Image::desaturate()
{
    if (auto ptr = image)
        ptr->desaturate();
}

z0 Image::createSolidAreaMask (RectangleList<i32>& result, f32 alphaThreshold) const
{
    if (hasAlphaChannel())
    {
        auto threshold = (u8) jlimit (0, 255, roundToInt (alphaThreshold * 255.0f));
        SparseSet<i32> pixelsOnRow;

        const BitmapData srcData (*this, 0, 0, getWidth(), getHeight());

        for (i32 y = 0; y < srcData.height; ++y)
        {
            pixelsOnRow.clear();
            auto lineData = srcData.getLinePointer (y);

            if (isARGB())
            {
                for (i32 x = 0; x < srcData.width; ++x)
                {
                    if (reinterpret_cast<const PixelARGB*> (lineData)->getAlpha() >= threshold)
                        pixelsOnRow.addRange (Range<i32> (x, x + 1));

                    lineData += srcData.pixelStride;
                }
            }
            else
            {
                for (i32 x = 0; x < srcData.width; ++x)
                {
                    if (*lineData >= threshold)
                        pixelsOnRow.addRange (Range<i32> (x, x + 1));

                    lineData += srcData.pixelStride;
                }
            }

            for (i32 i = 0; i < pixelsOnRow.getNumRanges(); ++i)
            {
                auto range = pixelsOnRow.getRange (i);
                result.add (Rectangle<i32> (range.getStart(), y, range.getLength(), 1));
            }

            result.consolidate();
        }
    }
    else
    {
        result.add (0, 0, getWidth(), getHeight());
    }
}

z0 Image::moveImageSection (i32 dx, i32 dy,
                              i32 sx, i32 sy,
                              i32 w, i32 h)
{
    if (dx < 0)
    {
        w += dx;
        sx -= dx;
        dx = 0;
    }

    if (dy < 0)
    {
        h += dy;
        sy -= dy;
        dy = 0;
    }

    if (sx < 0)
    {
        w += sx;
        dx -= sx;
        sx = 0;
    }

    if (sy < 0)
    {
        h += sy;
        dy -= sy;
        sy = 0;
    }

    i32k minX = jmin (dx, sx);
    i32k minY = jmin (dy, sy);

    w = jmin (w, getWidth()  - jmax (sx, dx));
    h = jmin (h, getHeight() - jmax (sy, dy));

    if (w > 0 && h > 0)
    {
        auto maxX = jmax (dx, sx) + w;
        auto maxY = jmax (dy, sy) + h;

        const BitmapData destData (*this, minX, minY, maxX - minX, maxY - minY, BitmapData::readWrite);

        auto dst = destData.getPixelPointer (dx - minX, dy - minY);
        auto src = destData.getPixelPointer (sx - minX, sy - minY);

        auto lineSize = (size_t) destData.pixelStride * (size_t) w;

        if (dy > sy)
        {
            while (--h >= 0)
            {
                i32k offset = h * destData.lineStride;
                memmove (dst + offset, src + offset, lineSize);
            }
        }
        else if (dst != src)
        {
            while (--h >= 0)
            {
                memmove (dst, src, lineSize);
                dst += destData.lineStride;
                src += destData.lineStride;
            }
        }
    }
}

//==============================================================================
#if DRX_ALLOW_STATIC_NULL_VARIABLES

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

const Image Image::null;

DRX_END_IGNORE_DEPRECATION_WARNINGS

#endif

} // namespace drx
