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
    Creates an openGL texture from an Image.

    @tags{OpenGL}
*/
class DRX_API  OpenGLTexture
{
public:
    OpenGLTexture();
    ~OpenGLTexture();

    /** Creates a texture from the given image.

        Note that if the image's dimensions aren't a power-of-two, the texture may
        be created with a larger size.

        The image will be arranged so that its top-left corner is at texture
        coordinate (0, 1).
    */
    z0 loadImage (const Image& image);

    /** Creates a texture from a raw array of pixels.
        If width and height are not powers-of-two, the texture will be created with a
        larger size, and only the subsection (0, 0, width, height) will be initialised.
        The data is sent directly to the OpenGL driver without being flipped vertically,
        so the first pixel will be mapped onto texture coordinate (0, 0).
    */
    z0 loadARGB (const PixelARGB* pixels, i32 width, i32 height);

    /** Creates a texture from a raw array of pixels.
        This is like loadARGB, but will vertically flip the data so that the first
        pixel ends up at texture coordinate (0, 1), and if the width and height are
        not powers-of-two, it will compensate by using a larger texture size.
    */
    z0 loadARGBFlipped (const PixelARGB* pixels, i32 width, i32 height);

    /** Creates an alpha-channel texture from an array of alpha values.
        If width and height are not powers-of-two, the texture will be created with a
        larger size, and only the subsection (0, 0, width, height) will be initialised.
        The data is sent directly to the OpenGL driver without being flipped vertically,
        so the first pixel will be mapped onto texture coordinate (0, 0).
    */
    z0 loadAlpha (u8k* pixels, i32 width, i32 height);

    /** Frees the texture, if there is one. */
    z0 release();

    /** Binds the texture to the currently active openGL context. */
    z0 bind() const;

    /** Unbinds the texture to the currently active openGL context. */
    z0 unbind() const;

    /** Returns the GL texture ID number. */
    GLuint getTextureID() const noexcept        { return textureID; }

    i32 getWidth() const noexcept               { return width; }
    i32 getHeight() const noexcept              { return height; }

    /** Возвращает true, если a texture can be created with the given size.
        Some systems may require that the sizes are powers-of-two.
    */
    static b8 isValidSize (i32 width, i32 height);

private:
    GLuint textureID;
    i32 width, height;
    OpenGLContext* ownerContext;

    z0 create (i32 w, i32 h, ukk, GLenum, b8 topLeft);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLTexture)
};

} // namespace drx
