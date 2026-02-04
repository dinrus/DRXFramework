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

class OpenGLFrameBufferImage final : public ImagePixelData
{
public:
    using Ptr = ReferenceCountedObjectPtr<OpenGLFrameBufferImage>;

    OpenGLFrameBufferImage (OpenGLContext& c, i32 w, i32 h)
        : ImagePixelData (Image::ARGB, w, h),
          context (c),
          pixelStride (4)
    {
    }

    b8 initialise()
    {
        if (! frameBuffer.initialise (context, width, height))
            return false;

        frameBuffer.clear (Colors::transparentBlack);
        return true;
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return createOpenGLGraphicsContext (context, frameBuffer);
    }

    std::unique_ptr<ImageType> createType() const override     { return std::make_unique<OpenGLImageType>(); }

    ImagePixelData::Ptr clone() override
    {
        std::unique_ptr<OpenGLFrameBufferImage> im (new OpenGLFrameBufferImage (context, width, height));

        if (! im->initialise())
            return ImagePixelData::Ptr();

        Image newImage (im.release());
        Graphics g (newImage);
        g.drawImageAt (Image (*this), 0, 0, false);

        return ImagePixelData::Ptr (newImage.getPixelData());
    }

    z0 initialiseBitmapData (Image::BitmapData& bitmapData, i32 x, i32 y, Image::BitmapData::ReadWriteMode mode) override
    {
        bitmapData.pixelFormat = pixelFormat;
        bitmapData.pixelStride = pixelStride;

        auto releaser = std::make_unique<DataReleaser> (this, Rectangle { x, y, bitmapData.width, bitmapData.height }, mode);

        bitmapData.data = (u8*) releaser->data.get();
        bitmapData.size = (size_t) bitmapData.width
                        * (size_t) bitmapData.height
                        * sizeof (PixelARGB);
        bitmapData.lineStride = (bitmapData.width * bitmapData.pixelStride + 3) & ~3;

        bitmapData.dataReleaser = std::move (releaser);

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    OpenGLContext& context;
    OpenGLFrameBuffer frameBuffer;

private:
    i32 pixelStride;

    struct DataReleaser final : public Image::BitmapData::BitmapDataReleaser
    {
        DataReleaser (Ptr selfIn, Rectangle<i32> areaIn, Image::BitmapData::ReadWriteMode modeIn)
            : self (selfIn),
              data ((size_t) (areaIn.getWidth() * areaIn.getHeight())),
              area (areaIn),
              mode (modeIn)
        {
            if (mode != Image::BitmapData::writeOnly)
                self->frameBuffer.readPixels (data.get(), getArea());
        }

        ~DataReleaser() override
        {
            if (mode != Image::BitmapData::readOnly)
                self->frameBuffer.writePixels (data, getArea());
        }

        Rectangle<i32> getArea() const
        {
            return area.withBottomY (self->frameBuffer.getHeight() - area.getY());
        }

        Ptr self;
        HeapBlock<PixelARGB> data;
        Rectangle<i32> area;
        Image::BitmapData::ReadWriteMode mode;
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLFrameBufferImage)
};

//==============================================================================
OpenGLImageType::OpenGLImageType() {}
OpenGLImageType::~OpenGLImageType() {}

i32 OpenGLImageType::getTypeID() const
{
    return 3;
}

ImagePixelData::Ptr OpenGLImageType::create (Image::PixelFormat, i32 width, i32 height, b8 /*shouldClearImage*/) const
{
    OpenGLContext* currentContext = OpenGLContext::getCurrentContext();
    jassert (currentContext != nullptr); // an OpenGL image can only be created when a valid context is active!

    std::unique_ptr<OpenGLFrameBufferImage> im (new OpenGLFrameBufferImage (*currentContext, width, height));

    if (! im->initialise())
        return ImagePixelData::Ptr();

    return *im.release();
}

OpenGLFrameBuffer* OpenGLImageType::getFrameBufferFrom (const Image& image)
{
    if (OpenGLFrameBufferImage* const glImage = dynamic_cast<OpenGLFrameBufferImage*> (image.getPixelData().get()))
        return &(glImage->frameBuffer);

    return nullptr;
}

} // namespace drx
