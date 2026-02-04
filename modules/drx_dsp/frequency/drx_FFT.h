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

namespace drx::dsp
{

/**
    Performs a fast fourier transform.

    This is only a simple low-footprint implementation and isn't tuned for speed - it may
    be useful for simple applications where one of the more complex FFT libraries would be
    overkill. (But in the future it may end up becoming optimised of course...)

    The FFT class itself contains lookup tables, so there's some overhead in creating
    one, you should create and cache an FFT object for each size/direction of transform
    that you need, and re-use them to perform the actual operation.

    @tags{DSP}
*/
class DRX_API  FFT
{
public:
    //==============================================================================
    /** Initialises an object for performing forward and inverse FFT with the given size.
        The number of points the FFT will operate on will be 2 ^ order.
    */
    FFT (i32 order);

    /** Move constructor. */
    FFT (FFT&&) noexcept;

    /** Move assignment operator. */
    FFT& operator= (FFT&&) noexcept;

    /** Destructor. */
    ~FFT();

    //==============================================================================
    /** Performs an out-of-place FFT, either forward or inverse.
        The arrays must contain at least getSize() elements.
    */
    z0 perform (const Complex<f32>* input, Complex<f32>* output, b8 inverse) const noexcept;

    /** Performs an in-place forward transform on a block of real data.

        As the coefficients of the negative frequencies (frequencies higher than
        N/2 or pi) are the complex conjugate of their positive counterparts,
        it may not be necessary to calculate them for your particular application.
        You can use onlyCalculateNonNegativeFrequencies to let the FFT
        engine know that you do not plan on using them. Note that this is only a
        hint: some FFT engines (currently only the Fallback engine), will still
        calculate the negative frequencies even if onlyCalculateNonNegativeFrequencies
        is true.

        The size of the array passed in must be 2 * getSize(), and the first half
        should contain your raw input sample data. On return, if
        onlyCalculateNonNegativeFrequencies is false, the array will contain size
        complex real + imaginary parts data interleaved. If
        onlyCalculateNonNegativeFrequencies is true, the array will contain at least
        (size / 2) + 1 complex numbers. Both outputs can be passed to
        performRealOnlyInverseTransform() in order to convert it back to reals.
    */
    z0 performRealOnlyForwardTransform (f32* inputOutputData,
                                          b8 onlyCalculateNonNegativeFrequencies = false) const noexcept;

    /** Performs a reverse operation to data created in performRealOnlyForwardTransform().

        Although performRealOnlyInverseTransform will only use the first ((size / 2) + 1)
        complex numbers, the size of the array passed in must still be 2 * getSize(), as some
        FFT engines require the extra space for the calculation. On return, the first half of the
        array will contain the reconstituted samples.
    */
    z0 performRealOnlyInverseTransform (f32* inputOutputData) const noexcept;

    /** Takes an array and simply transforms it to the magnitude frequency response
        spectrum. This may be handy for things like frequency displays or analysis.
        The size of the array passed in must be 2 * getSize().

        On return, if onlyCalculateNonNegativeFrequencies is false, the array will contain size
        magnitude values. If onlyCalculateNonNegativeFrequencies is true, the array will contain
        at least size / 2 + 1 magnitude values.
    */
    z0 performFrequencyOnlyForwardTransform (f32* inputOutputData,
                                               b8 onlyCalculateNonNegativeFrequencies = false) const noexcept;

    /** Returns the number of data points that this FFT was created to work with. */
    i32 getSize() const noexcept            { return size; }

    //==============================================================================
   #ifndef DOXYGEN
    /* internal */
    struct Instance;
    template <typename> struct EngineImpl;
   #endif

private:
    //==============================================================================
    struct Engine;

    std::unique_ptr<Instance> engine;
    i32 size;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFT)
};

} // namespace drx::dsp
