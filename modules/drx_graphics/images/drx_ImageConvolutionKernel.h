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
    Represents a filter kernel to use in convoluting an image.

    @see Image::applyConvolution

    @tags{Graphics}
*/
class DRX_API  ImageConvolutionKernel
{
public:
    //==============================================================================
    /** Creates an empty convolution kernel.

        @param size     the length of each dimension of the kernel, so e.g. if the size
                        is 5, it will create a 5x5 kernel
    */
    ImageConvolutionKernel (i32 size);

    /** Destructor. */
    ~ImageConvolutionKernel();

    //==============================================================================
    /** Resets all values in the kernel to zero. */
    z0 clear();

    /** Returns one of the kernel values. */
    f32 getKernelValue (i32 x, i32 y) const noexcept;

    /** Sets the value of a specific cell in the kernel.

        The x and y parameters must be in the range 0 < x < getKernelSize().

        @see setOverallSum
    */
    z0 setKernelValue (i32 x, i32 y, f32 value) noexcept;

    /** Rescales all values in the kernel to make the total add up to a fixed value.

        This will multiply all values in the kernel by (desiredTotalSum / currentTotalSum).
    */
    z0 setOverallSum (f32 desiredTotalSum);

    /** Multiplies all values in the kernel by a value. */
    z0 rescaleAllValues (f32 multiplier);

    /** Initialises the kernel for a gaussian blur.

        @param blurRadius   this may be larger or smaller than the kernel's actual
                            size but this will obviously be wasteful or clip at the
                            edges. Ideally the kernel should be just larger than
                            (blurRadius * 2).
    */
    z0 createGaussianBlur (f32 blurRadius);

    //==============================================================================
    /** Returns the size of the kernel.

        E.g. if it's a 3x3 kernel, this returns 3.
    */
    i32 getKernelSize() const               { return size; }

    //==============================================================================
    /** Applies the kernel to an image.

        @param destImage        the image that will receive the resultant convoluted pixels.
        @param sourceImage      the source image to read from - this can be the same image as
                                the destination, but if different, it must be exactly the same
                                size and format.
        @param destinationArea  the region of the image to apply the filter to
    */
    z0 applyToImage (Image& destImage,
                       const Image& sourceImage,
                       const Rectangle<i32>& destinationArea) const;

private:
    //==============================================================================
    HeapBlock<f32> values;
    i32k size;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageConvolutionKernel)
};

} // namespace drx
