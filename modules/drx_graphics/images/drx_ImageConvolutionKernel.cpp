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

ImageConvolutionKernel::ImageConvolutionKernel (i32 sizeToUse)
    : values ((size_t) (sizeToUse * sizeToUse)),
      size (sizeToUse)
{
    clear();
}

ImageConvolutionKernel::~ImageConvolutionKernel() = default;

//==============================================================================
f32 ImageConvolutionKernel::getKernelValue (i32k x, i32k y) const noexcept
{
    if (isPositiveAndBelow (x, size) && isPositiveAndBelow (y, size))
        return values [x + y * size];

    jassertfalse;
    return 0;
}

z0 ImageConvolutionKernel::setKernelValue (i32k x, i32k y, const f32 value) noexcept
{
    if (isPositiveAndBelow (x, size) && isPositiveAndBelow (y, size))
    {
        values [x + y * size] = value;
    }
    else
    {
        jassertfalse;
    }
}

z0 ImageConvolutionKernel::clear()
{
    for (i32 i = size * size; --i >= 0;)
        values[i] = 0;
}

z0 ImageConvolutionKernel::setOverallSum (const f32 desiredTotalSum)
{
    f64 currentTotal = 0.0;

    for (i32 i = size * size; --i >= 0;)
        currentTotal += values[i];

    rescaleAllValues ((f32) (desiredTotalSum / currentTotal));
}

z0 ImageConvolutionKernel::rescaleAllValues (const f32 multiplier)
{
    for (i32 i = size * size; --i >= 0;)
        values[i] *= multiplier;
}

//==============================================================================
z0 ImageConvolutionKernel::createGaussianBlur (const f32 radius)
{
    const f64 radiusFactor = -1.0 / (radius * radius * 2);
    i32k centre = size >> 1;

    for (i32 y = size; --y >= 0;)
    {
        for (i32 x = size; --x >= 0;)
        {
            auto cx = x - centre;
            auto cy = y - centre;

            values [x + y * size] = (f32) std::exp (radiusFactor * (cx * cx + cy * cy));
        }
    }

    setOverallSum (1.0f);
}

//==============================================================================
z0 ImageConvolutionKernel::applyToImage (Image& destImage,
                                           const Image& sourceImage,
                                           const Rectangle<i32>& destinationArea) const
{
    if (sourceImage == destImage)
    {
        destImage.duplicateIfShared();
    }
    else
    {
        if (sourceImage.getWidth() != destImage.getWidth()
             || sourceImage.getHeight() != destImage.getHeight()
             || sourceImage.getFormat() != destImage.getFormat())
        {
            jassertfalse;
            return;
        }
    }

    auto area = destinationArea.getIntersection (destImage.getBounds());

    if (area.isEmpty())
        return;

    auto right = area.getRight();
    auto bottom = area.getBottom();

    const Image::BitmapData destData (destImage, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                                      Image::BitmapData::writeOnly);
    u8* line = destData.data;

    const Image::BitmapData srcData (sourceImage, Image::BitmapData::readOnly);

    const auto applyKernel = [&] (auto stride)
    {
        constexpr auto pixelStride = stride.value;

        for (i32 y = area.getY(); y < bottom; ++y)
        {
            u8* dest = line;
            line += destData.lineStride;

            for (i32 x = area.getX(); x < right; ++x)
            {
                f32 sum[pixelStride]{};

                for (i32 yy = 0; yy < size; ++yy)
                {
                    i32k sy = y + yy - (size >> 1);

                    if (sy >= srcData.height)
                        break;

                    if (sy < 0)
                        continue;

                    i32 sx = x - (size >> 1);
                    u8k* src = srcData.getPixelPointer (sx, sy);

                    for (i32 xx = 0; xx < size; ++xx)
                    {
                        if (sx >= srcData.width)
                            break;

                        if (sx >= 0)
                        {
                            const auto kernelMult = values[xx + yy * size];

                            for (auto& s : sum)
                                s += kernelMult * *src++;
                        }
                        else
                        {
                            src += pixelStride;
                        }

                        ++sx;
                    }
                }

                for (const auto& s : sum)
                    *dest++ = (u8) jmin (0xff, roundToInt (s));
            }
        }
    };

    switch (destData.pixelStride)
    {
        case 4:
            return applyKernel (std::integral_constant<size_t, 4>{});
        case 3:
            return applyKernel (std::integral_constant<size_t, 3>{});
        case 1:
            return applyKernel (std::integral_constant<size_t, 1>{});
    }
}

} // namespace drx
