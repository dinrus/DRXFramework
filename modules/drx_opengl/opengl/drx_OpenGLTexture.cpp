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

OpenGLTexture::OpenGLTexture()
    : textureID (0), width (0), height (0), ownerContext (nullptr)
{
}

OpenGLTexture::~OpenGLTexture()
{
    release();
}

b8 OpenGLTexture::isValidSize (i32 width, i32 height)
{
    return isPowerOfTwo (width) && isPowerOfTwo (height);
}

z0 OpenGLTexture::create (i32k w, i32k h, ukk pixels, GLenum type, b8 topLeft)
{
    ownerContext = OpenGLContext::getCurrentContext();

    // Texture objects can only be created when the current thread has an active OpenGL
    // context. You'll need to create this object in one of the OpenGLContext's callbacks.
    jassert (ownerContext != nullptr);

    if (textureID == 0)
    {
        DRX_CHECK_OPENGL_ERROR
        glGenTextures (1, &textureID);
        glBindTexture (GL_TEXTURE_2D, textureID);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        auto glMagFilter = (GLint) (ownerContext->texMagFilter == OpenGLContext::linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        DRX_CHECK_OPENGL_ERROR
    }
    else
    {
        glBindTexture (GL_TEXTURE_2D, textureID);
        DRX_CHECK_OPENGL_ERROR;
    }

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    DRX_CHECK_OPENGL_ERROR

    const auto textureNpotSupported = ownerContext->isTextureNpotSupported();

    const auto getAllowedTextureSize = [&] (i32 n)
    {
        return textureNpotSupported ? n : nextPowerOfTwo (n);
    };

    width  = getAllowedTextureSize (w);
    height = getAllowedTextureSize (h);

    const GLint internalformat = type == GL_ALPHA ? GL_ALPHA : GL_RGBA;

    if (width != w || height != h)
    {
        glTexImage2D (GL_TEXTURE_2D, 0, internalformat,
                      width, height, 0, type, GL_UNSIGNED_BYTE, nullptr);

        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, topLeft ? (height - h) : 0, w, h,
                         type, GL_UNSIGNED_BYTE, pixels);
    }
    else
    {
        glTexImage2D (GL_TEXTURE_2D, 0, internalformat,
                      w, h, 0, type, GL_UNSIGNED_BYTE, pixels);
    }

    DRX_CHECK_OPENGL_ERROR
}

template <class PixelType>
struct Flipper
{
    static z0 flip (HeapBlock<PixelARGB>& dataCopy,
                      u8k* srcData,
                      i32k lineStride,
                      i32k pixelStride,
                      i32k w,
                      i32k h)
    {
        dataCopy.malloc (w * h);

        for (i32 y = 0; y < h; ++y)
        {
            auto* srcLine = srcData + lineStride * y;
            auto* dstLine = dataCopy.get() + w * (h - 1 - y);

            for (i32 x = 0; x < w; ++x)
            {
                auto* srcPixel = srcLine + x * pixelStride;
                dstLine[x].set (*unalignedPointerCast<const PixelType*> (srcPixel));
            }
        }
    }
};

z0 OpenGLTexture::loadImage (const Image& image)
{
    i32k imageW = image.getWidth();
    i32k imageH = image.getHeight();

    HeapBlock<PixelARGB> dataCopy;
    Image::BitmapData srcData (image, Image::BitmapData::readOnly);

    switch (srcData.pixelFormat)
    {
        case Image::ARGB:           Flipper<PixelARGB> ::flip (dataCopy, srcData.data, srcData.lineStride, srcData.pixelStride, imageW, imageH); break;
        case Image::RGB:            Flipper<PixelRGB>  ::flip (dataCopy, srcData.data, srcData.lineStride, srcData.pixelStride, imageW, imageH); break;
        case Image::SingleChannel:  Flipper<PixelAlpha>::flip (dataCopy, srcData.data, srcData.lineStride, srcData.pixelStride, imageW, imageH); break;
        case Image::UnknownFormat:
        default: break;
    }

    create (imageW, imageH, dataCopy, DRX_RGBA_FORMAT, true);
}

z0 OpenGLTexture::loadARGB (const PixelARGB* pixels, i32k w, i32k h)
{
    create (w, h, pixels, DRX_RGBA_FORMAT, false);
}

z0 OpenGLTexture::loadAlpha (u8k* pixels, i32 w, i32 h)
{
    create (w, h, pixels, GL_ALPHA, false);
}

z0 OpenGLTexture::loadARGBFlipped (const PixelARGB* pixels, i32 w, i32 h)
{
    HeapBlock<PixelARGB> flippedCopy;
    Flipper<PixelARGB>::flip (flippedCopy, (u8k*) pixels, 4 * w, 4, w, h);

    create (w, h, flippedCopy, DRX_RGBA_FORMAT, true);
}

z0 OpenGLTexture::release()
{
    if (textureID != 0)
    {
        // If the texture is deleted while the owner context is not active, it's
        // impossible to delete it, so this will be a leak until the context itself
        // is deleted.
        jassert (ownerContext == OpenGLContext::getCurrentContext());

        if (ownerContext == OpenGLContext::getCurrentContext())
        {
            glDeleteTextures (1, &textureID);

            textureID = 0;
            width = 0;
            height = 0;
        }
    }
}

z0 OpenGLTexture::bind() const
{
    glBindTexture (GL_TEXTURE_2D, textureID);
}

z0 OpenGLTexture::unbind() const
{
    glBindTexture (GL_TEXTURE_2D, 0);
}

} // namespace drx
