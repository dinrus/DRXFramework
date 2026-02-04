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

OpenGLPixelFormat::OpenGLPixelFormat (i32k bitsPerRGBComponent,
                                      i32k alphaBits_,
                                      i32k depthBufferBits_,
                                      i32k stencilBufferBits_) noexcept
    : redBits (bitsPerRGBComponent),
      greenBits (bitsPerRGBComponent),
      blueBits (bitsPerRGBComponent),
      alphaBits (alphaBits_),
      depthBufferBits (depthBufferBits_),
      stencilBufferBits (stencilBufferBits_),
      accumulationBufferRedBits (0),
      accumulationBufferGreenBits (0),
      accumulationBufferBlueBits (0),
      accumulationBufferAlphaBits (0),
      multisamplingLevel (0)
{
}

static auto tie (const OpenGLPixelFormat& fmt)
{
    return std::tie (fmt.redBits,
                     fmt.greenBits,
                     fmt.blueBits,
                     fmt.alphaBits,
                     fmt.depthBufferBits,
                     fmt.stencilBufferBits,
                     fmt.accumulationBufferRedBits,
                     fmt.accumulationBufferGreenBits,
                     fmt.accumulationBufferBlueBits,
                     fmt.accumulationBufferAlphaBits,
                     fmt.multisamplingLevel);
}

b8 OpenGLPixelFormat::operator== (const OpenGLPixelFormat& other) const noexcept
{
    return tie (*this) == tie (other);
}

b8 OpenGLPixelFormat::operator!= (const OpenGLPixelFormat& other) const noexcept
{
    return ! operator== (other);
}

} // namespace drx
