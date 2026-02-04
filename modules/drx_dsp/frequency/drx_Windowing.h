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
    A class which provides multiple windowing functions useful for filter design
    and spectrum analyzers.

    The different functions provided here can be used by creating either a
    WindowingFunction object, or a static function to fill an array with the
    windowing method samples.

    @tags{DSP}
*/
template <typename FloatType>
class DRX_API  WindowingFunction
{
public:
    //==============================================================================
    /** The windowing methods available. */
    enum WindowingMethod
    {
        rectangular = 0,
        triangular,
        hann,
        hamming,
        blackman,
        blackmanHarris,
        flatTop,
        kaiser,
        numWindowingMethods
    };

    //==============================================================================
    /** This constructor automatically fills a buffer of the specified size using
        the fillWindowingTables function and the specified arguments.

        @see fillWindowingTables
    */
    WindowingFunction (size_t size, WindowingMethod,
                       b8 normalise = true, FloatType beta = 0);

    //==============================================================================
    /** Fills the content of the object array with a given windowing method table.

        @param size         the size of the destination buffer allocated in the object
        @param type         the type of windowing method being used
        @param normalise    if the result must be normalised, creating a DC amplitude
                            response of one
        @param beta         an optional argument useful only for Kaiser's method
                            which must be positive and sets the properties of the
                            method (bandwidth and attenuation increases with beta)
    */
    z0 fillWindowingTables (size_t size, WindowingMethod type,
                              b8 normalise = true, FloatType beta = 0) noexcept;

    /** Fills the content of an array with a given windowing method table.

        @param samples      the destination buffer pointer
        @param size         the size of the destination buffer allocated in the object
        @param normalise    if the result must be normalised, creating a DC amplitude
                            response of one
        @param beta         an optional argument useful only for Kaiser's method,
                            which must be positive and sets the properties of the
                            method (bandwidth and attenuation increases with beta)
    */
    static z0 fillWindowingTables (FloatType* samples, size_t size, WindowingMethod,
                                     b8 normalise = true, FloatType beta = 0) noexcept;

    /** Multiplies the content of a buffer with the given window. */
    z0 multiplyWithWindowingTable (FloatType* samples, size_t size) const noexcept;

    /** Returns the name of a given windowing method. */
    static tukk getWindowingMethodName (WindowingMethod) noexcept;


private:
    //==============================================================================
    Array<FloatType> windowTable;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowingFunction)
};

} // namespace drx::dsp
