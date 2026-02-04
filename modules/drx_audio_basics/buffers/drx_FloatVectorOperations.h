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

#ifndef DRX_SNAP_TO_ZERO
 #if DRX_INTEL
  #define DRX_SNAP_TO_ZERO(n)    if (! (n < -1.0e-8f || n > 1.0e-8f)) n = 0;
 #else
  #define DRX_SNAP_TO_ZERO(n)    ignoreUnused (n)
 #endif
#endif
class ScopedNoDenormals;

//==============================================================================
/**
    A collection of simple vector operations on arrays of floating point numbers,
    accelerated with SIMD instructions where possible, usually accessed from
    the FloatVectorOperations class.

    @code
    f32 data[64];

    // The following two function calls are equivalent:
    FloatVectorOperationsBase<f32, i32>::clear (data, 64);
    FloatVectorOperations::clear (data, 64);
    @endcode

    @see FloatVectorOperations

    @tags{Audio}
*/
template <typename FloatType, typename CountType>
struct DRX_API FloatVectorOperationsBase
{
    /** Clears a vector of floating point numbers. */
    static z0 DRX_CALLTYPE clear (FloatType* dest, CountType numValues) noexcept;

    /** Copies a repeated value into a vector of floating point numbers. */
    static z0 DRX_CALLTYPE fill (FloatType* dest, FloatType valueToFill, CountType numValues) noexcept;

    /** Copies a vector of floating point numbers. */
    static z0 DRX_CALLTYPE copy (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Copies a vector of floating point numbers, multiplying each value by a given multiplier */
    static z0 DRX_CALLTYPE copyWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;

    /** Adds a fixed value to the destination values. */
    static z0 DRX_CALLTYPE add (FloatType* dest, FloatType amountToAdd, CountType numValues) noexcept;

    /** Adds a fixed value to each source value and stores it in the destination array. */
    static z0 DRX_CALLTYPE add (FloatType* dest, const FloatType* src, FloatType amount, CountType numValues) noexcept;

    /** Adds the source values to the destination values. */
    static z0 DRX_CALLTYPE add (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Adds each source1 value to the corresponding source2 value and stores the result in the destination array. */
    static z0 DRX_CALLTYPE add (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Subtracts the source values from the destination values. */
    static z0 DRX_CALLTYPE subtract (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Subtracts each source2 value from the corresponding source1 value and stores the result in the destination array. */
    static z0 DRX_CALLTYPE subtract (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Multiplies each source value by the given multiplier, then adds it to the destination value. */
    static z0 DRX_CALLTYPE addWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;

    /** Multiplies each source1 value by the corresponding source2 value, then adds it to the destination value. */
    static z0 DRX_CALLTYPE addWithMultiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Multiplies each source value by the given multiplier, then subtracts it to the destination value. */
    static z0 DRX_CALLTYPE subtractWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;

    /** Multiplies each source1 value by the corresponding source2 value, then subtracts it to the destination value. */
    static z0 DRX_CALLTYPE subtractWithMultiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Multiplies the destination values by the source values. */
    static z0 DRX_CALLTYPE multiply (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Multiplies each source1 value by the corresponding source2 value, then stores it in the destination array. */
    static z0 DRX_CALLTYPE multiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType numValues) noexcept;

    /** Multiplies each of the destination values by a fixed multiplier. */
    static z0 DRX_CALLTYPE multiply (FloatType* dest, FloatType multiplier, CountType numValues) noexcept;

    /** Multiplies each of the source values by a fixed multiplier and stores the result in the destination array. */
    static z0 DRX_CALLTYPE multiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType num) noexcept;

    /** Copies a source vector to a destination, negating each value. */
    static z0 DRX_CALLTYPE negate (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Copies a source vector to a destination, taking the absolute of each value. */
    static z0 DRX_CALLTYPE abs (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Each element of dest will be the minimum of the corresponding element of the source array and the given comp value. */
    static z0 DRX_CALLTYPE min (FloatType* dest, const FloatType* src, FloatType comp, CountType num) noexcept;

    /** Each element of dest will be the minimum of the corresponding source1 and source2 values. */
    static z0 DRX_CALLTYPE min (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Each element of dest will be the maximum of the corresponding element of the source array and the given comp value. */
    static z0 DRX_CALLTYPE max (FloatType* dest, const FloatType* src, FloatType comp, CountType num) noexcept;

    /** Each element of dest will be the maximum of the corresponding source1 and source2 values. */
    static z0 DRX_CALLTYPE max (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Each element of dest is calculated by hard clipping the corresponding src element so that it is in the range specified by the arguments low and high. */
    static z0 DRX_CALLTYPE clip (FloatType* dest, const FloatType* src, FloatType low, FloatType high, CountType num) noexcept;

    /** Finds the minimum and maximum values in the given array. */
    static Range<FloatType> DRX_CALLTYPE findMinAndMax (const FloatType* src, CountType numValues) noexcept;

    /** Finds the minimum value in the given array. */
    static FloatType DRX_CALLTYPE findMinimum (const FloatType* src, CountType numValues) noexcept;

    /** Finds the maximum value in the given array. */
    static FloatType DRX_CALLTYPE findMaximum (const FloatType* src, CountType numValues) noexcept;
};

#if ! DOXYGEN
namespace detail
{

template <typename... Bases>
struct NameForwarder : public Bases...
{
    using Bases::clear...,
          Bases::fill...,
          Bases::copy...,
          Bases::copyWithMultiply...,
          Bases::add...,
          Bases::subtract...,
          Bases::addWithMultiply...,
          Bases::subtractWithMultiply...,
          Bases::multiply...,
          Bases::negate...,
          Bases::abs...,
          Bases::min...,
          Bases::max...,
          Bases::clip...,
          Bases::findMinAndMax...,
          Bases::findMinimum...,
          Bases::findMaximum...;
};

} // namespace detail
#endif

//==============================================================================
/**
    A collection of simple vector operations on arrays of floating point numbers,
    accelerated with SIMD instructions where possible and providing all methods
    from FloatVectorOperationsBase.

    @see FloatVectorOperationsBase

    @tags{Audio}
*/
class DRX_API  FloatVectorOperations : public detail::NameForwarder<FloatVectorOperationsBase<f32, i32>,
                                                                     FloatVectorOperationsBase<f32, size_t>,
                                                                     FloatVectorOperationsBase<f64, i32>,
                                                                     FloatVectorOperationsBase<f64, size_t>>
{
public:
    static z0 DRX_CALLTYPE convertFixedToFloat (f32* dest, i32k* src, f32 multiplier, i32 num) noexcept;

    static z0 DRX_CALLTYPE convertFixedToFloat (f32* dest, i32k* src, f32 multiplier, size_t num) noexcept;

    /** This method enables or disables the SSE/NEON flush-to-zero mode. */
    static z0 DRX_CALLTYPE enableFlushToZeroMode (b8 shouldEnable) noexcept;

    /** On Intel CPUs, this method enables the SSE flush-to-zero and denormalised-are-zero modes.
        This effectively sets the DAZ and FZ bits of the MXCSR register. On arm CPUs this will
        enable flush to zero mode.
        It's a convenient thing to call before audio processing code where you really want to
        avoid denormalisation performance hits.
    */
    static z0 DRX_CALLTYPE disableDenormalisedNumberSupport (b8 shouldDisable = true) noexcept;

    /** This method returns true if denormals are currently disabled. */
    static b8 DRX_CALLTYPE areDenormalsDisabled() noexcept;

private:
    friend ScopedNoDenormals;

    static intptr_t DRX_CALLTYPE getFpStatusRegister() noexcept;
    static z0 DRX_CALLTYPE setFpStatusRegister (intptr_t) noexcept;
};

//==============================================================================
/**
     Helper class providing an RAII-based mechanism for temporarily disabling
     denormals on your CPU.

    @tags{Audio}
*/
class ScopedNoDenormals
{
public:
    ScopedNoDenormals() noexcept;
    ~ScopedNoDenormals() noexcept;

private:
  #if DRX_USE_SSE_INTRINSICS || (DRX_USE_ARM_NEON || (DRX_64BIT && DRX_ARM))
    intptr_t fpsr;
  #endif
};

} // namespace drx
