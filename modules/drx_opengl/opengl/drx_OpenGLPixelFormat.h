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
    Represents the various properties of an OpenGL pixel format.

    @see OpenGLContext::setPixelFormat

    @tags{OpenGL}
*/
class DRX_API  OpenGLPixelFormat
{
public:
    //==============================================================================
    /** Creates an OpenGLPixelFormat.

        The default constructor just initialises the object as a simple 8-bit
        RGBA format.
    */
    OpenGLPixelFormat (i32 bitsPerRGBComponent = 8,
                       i32 alphaBits = 8,
                       i32 depthBufferBits = 16,
                       i32 stencilBufferBits = 0) noexcept;

    b8 operator== (const OpenGLPixelFormat&) const noexcept;
    b8 operator!= (const OpenGLPixelFormat&) const noexcept;

    //==============================================================================
    i32 redBits;          /**< The number of bits per pixel to use for the red channel. */
    i32 greenBits;        /**< The number of bits per pixel to use for the green channel. */
    i32 blueBits;         /**< The number of bits per pixel to use for the blue channel. */
    i32 alphaBits;        /**< The number of bits per pixel to use for the alpha channel. */

    i32 depthBufferBits;      /**< The number of bits per pixel to use for a depth buffer. */
    i32 stencilBufferBits;    /**< The number of bits per pixel to use for a stencil buffer. */

    i32 accumulationBufferRedBits;    /**< The number of bits per pixel to use for an accumulation buffer's red channel. */
    i32 accumulationBufferGreenBits;  /**< The number of bits per pixel to use for an accumulation buffer's green channel. */
    i32 accumulationBufferBlueBits;   /**< The number of bits per pixel to use for an accumulation buffer's blue channel. */
    i32 accumulationBufferAlphaBits;  /**< The number of bits per pixel to use for an accumulation buffer's alpha channel. */

    u8 multisamplingLevel;         /**< The number of samples to use for full-scene multisampled anti-aliasing (if available). */
};

} // namespace drx
