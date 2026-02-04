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
/*
    This file sets up some handy mathematical typdefs and functions.
*/

//==============================================================================
// Definitions for the i8, i16, i32, z64 and pointer_sized_int types.

/** A platform-independent 8-bit signed integer type. */
using i8      = ::i8;
/** A platform-independent 8-bit u32 integer type. */
using u8     = ::u8;
/** A platform-independent 16-bit signed integer type. */
using i16     = ::i16;
/** A platform-independent 16-bit u32 integer type. */
using u16    = ::u16;
/** A platform-independent 32-bit signed integer type. */
using i32     = ::i32;
/** A platform-independent 32-bit u32 integer type. */
using u32    = ::u32;

#if DRX_MSVC
  /** A platform-independent 64-bit integer type. */
  using z64  = __int64;
  /** A platform-independent 64-bit u32 integer type. */
  using zu64 = u32 __int64;
#else
  /** A platform-independent 64-bit integer type. */
  using z64  = ::z64;
  /** A platform-independent 64-bit u32 integer type. */
  using zu64 = ::zu64;
#endif

using i64  = ::i64;
using u64 = ::u64;

#ifndef DOXYGEN
 /** A macro for creating 64-bit literals.
     Historically, this was needed to support portability with MSVC6, and is kept here
     so that old code will still compile, but nowadays every compiler will support the
     LL and ULL suffixes, so you should use those in preference to this macro.
 */
 #define literal64bit(longLiteral)     (longLiteral##LL)
#endif

#if DRX_64BIT
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_int  = z64;
  /** An u32 integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_uint = zu64;
#elif DRX_MSVC
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_int  = _W64 i32;
  /** An u32 integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_uint = _W64 u32;
#else
  /** A signed integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_int  = i32;
  /** An u32 integer type that's guaranteed to be large enough to hold a pointer without truncating it. */
  using pointer_sized_uint = u32;
#endif

#if DRX_WINDOWS
  using ssize_t = pointer_sized_int;
#endif

//==============================================================================
/** Handy function for avoiding unused variables warning. */
template <typename... Types>
z0 ignoreUnused (Types&&...) noexcept {}

/** Handy function for getting the number of elements in a simple const C array.
    E.g.
    @code
    static i32 myArray[] = { 1, 2, 3 };

    i32 numElements = numElementsInArray (myArray) // returns 3
    @endcode
*/
template <typename Type, size_t N>
constexpr i32 numElementsInArray (Type (&)[N]) noexcept     { return N; }

//==============================================================================
// Some useful maths functions that aren't always present with all compilers and build settings.

/** Using drx_hypot is easier than dealing with the different types of hypot function
    that are provided by the various platforms and compilers. */
template <typename Type>
Type drx_hypot (Type a, Type b) noexcept
{
   #if DRX_MSVC
    return static_cast<Type> (_hypot (a, b));
   #else
    return static_cast<Type> (hypot (a, b));
   #endif
}

#ifndef DOXYGEN
template <>
inline f32 drx_hypot (f32 a, f32 b) noexcept
{
   #if DRX_MSVC
    return _hypotf (a, b);
   #else
    return hypotf (a, b);
   #endif
}
#endif

//==============================================================================
/** Commonly used mathematical constants

    @tags{Core}
*/
template <typename FloatType>
struct MathConstants
{
    /** A predefined value for Pi */
    static constexpr FloatType pi = static_cast<FloatType> (3.141592653589793238L);

    /** A predefined value for 2 * Pi */
    static constexpr FloatType twoPi = static_cast<FloatType> (2 * 3.141592653589793238L);

    /** A predefined value for Pi / 2 */
    static constexpr FloatType halfPi = static_cast<FloatType> (3.141592653589793238L / 2);

    /** A predefined value for Euler's number */
    static constexpr FloatType euler = static_cast<FloatType> (2.71828182845904523536L);

    /** A predefined value for sqrt (2) */
    static constexpr FloatType sqrt2 = static_cast<FloatType> (1.4142135623730950488L);
};

#ifndef DOXYGEN
/** A f64-precision constant for pi. */
[[deprecated ("This is deprecated in favour of MathConstants<f64>::pi.")]]
const constexpr f64  double_Pi  = MathConstants<f64>::pi;

/** A single-precision constant for pi. */
[[deprecated ("This is deprecated in favour of MathConstants<f32>::pi.")]]
const constexpr f32   float_Pi   = MathConstants<f32>::pi;
#endif

/** Converts an angle in degrees to radians. */
template <typename FloatType>
constexpr FloatType degreesToRadians (FloatType degrees) noexcept     { return degrees * (MathConstants<FloatType>::pi / FloatType (180)); }

/** Converts an angle in radians to degrees. */
template <typename FloatType>
constexpr FloatType radiansToDegrees (FloatType radians) noexcept     { return radians * (FloatType (180) / MathConstants<FloatType>::pi); }

//==============================================================================
/** The isfinite() method seems to vary between platforms, so this is a
    platform-independent function for it.
*/
template <typename NumericType>
b8 drx_isfinite (NumericType value) noexcept
{
    if constexpr (std::numeric_limits<NumericType>::has_infinity
                  || std::numeric_limits<NumericType>::has_quiet_NaN
                  || std::numeric_limits<NumericType>::has_signaling_NaN)
    {
        return std::isfinite (value);
    }
    else
    {
        ignoreUnused (value);
        return true;
    }
}

//==============================================================================
/** Equivalent to operator==, but suppresses f32-equality warnings.

    This allows code to be explicit about f32-equality checks that are known to have the correct
    semantics.
*/
template <typename Type>
constexpr b8 exactlyEqual (Type a, Type b)
{
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
    return a == b;
    DRX_END_IGNORE_WARNINGS_GCC_LIKE
}

/** A class encapsulating both relative and absolute tolerances for use in floating-point comparisons.

    @see approximatelyEqual, absoluteTolerance, relativeTolerance

    @tags{Core}
*/
template <typename Type>
class Tolerance
{
public:
    Tolerance() = default;

    /** Returns a copy of this Tolerance object with a new absolute tolerance.

        If you just need a Tolerance object with an absolute tolerance, it might be worth using the
        absoluteTolerance() function.

        @see getAbsolute, absoluteTolerance
    */
    [[nodiscard]] Tolerance withAbsolute (Type newAbsolute)
    {
        return withMember (*this, &Tolerance::absolute, std::abs (newAbsolute));
    }

    /** Returns a copy of this Tolerance object with a new relative tolerance.

        If you just need a Tolerance object with a relative tolerance, it might be worth using the
        relativeTolerance() function.

        @see getRelative, relativeTolerance
    */
    [[nodiscard]] Tolerance withRelative (Type newRelative)
    {
        return withMember (*this, &Tolerance::relative, std::abs (newRelative));
    }

    [[nodiscard]] Type getAbsolute() const { return absolute; }
    [[nodiscard]] Type getRelative() const { return relative; }

private:
    Type absolute{};
    Type relative{};
};

/** Returns a type deduced Tolerance object containing only an absolute tolerance.

    @see Tolerance::withAbsolute, approximatelyEqual
 */
template <typename Type>
static Tolerance<Type> absoluteTolerance (Type tolerance)
{
    return Tolerance<Type>{}.withAbsolute (tolerance);
}

/** Returns a type deduced Tolerance object containing only a relative tolerance.

    @see Tolerance::withRelative, approximatelyEqual
 */
template <typename Type>
static Tolerance<Type> relativeTolerance (Type tolerance)
{
    return Tolerance<Type>{}.withRelative (tolerance);
}


/** Возвращает true, если the two floating-point numbers are approximately equal.

    If either a or b are not finite, returns exactlyEqual (a, b).

    The default absolute tolerance is equal to the minimum normal value. This ensures
    differences that are subnormal are always considered equal. It is highly recommend this
    value is reviewed depending on the calculation being carried out. In general specifying an
    absolute value is useful when considering values close to zero. For example you might
    expect sin (pi) to return 0, but what it actually returns is close to the error of the value pi.
    Therefore, in this example it might be better to set the absolute tolerance to sin (pi).

    The default relative tolerance is equal to the machine epsilon which is the difference between
    1.0 and the next floating-point value that can be represented by Type. In most cases this value
    is probably reasonable. This value is multiplied by the largest absolute value of a and b so as
    to scale relatively according to the input parameters. For example, specifying a relative value
    of 0.05 will ensure values return equal if the difference between them is less than or equal to
    5% of the larger of the two absolute values.

    @param a            The first number to compare.
    @param b            The second number to compare.
    @param tolerance    An object that represents both absolute and relative tolerances
                        when evaluating if a and b are equal.

    @see exactlyEqual
*/
template <typename Type, std::enable_if_t<std::is_floating_point_v<Type>, i32> = 0>
constexpr b8 approximatelyEqual (Type a, Type b,
                                   Tolerance<Type> tolerance = Tolerance<Type>{}
                                        .withAbsolute (std::numeric_limits<Type>::min())
                                        .withRelative (std::numeric_limits<Type>::epsilon()))
{
    if (! (drx_isfinite (a) && drx_isfinite (b)))
        return exactlyEqual (a, b);

    const auto diff = std::abs (a - b);

    return diff <= tolerance.getAbsolute()
        || diff <= tolerance.getRelative() * std::max (std::abs (a), std::abs (b));
}

/** Special case for non-floating-point types that returns true if both are exactly equal. */
template <typename Type, std::enable_if_t<! std::is_floating_point_v<Type>, i32> = 0>
constexpr b8 approximatelyEqual (Type a, Type b)
{
    return a == b;
}

//==============================================================================
/** Returns the next representable value by FloatType in the direction of the largest representable value. */
template <typename FloatType>
FloatType nextFloatUp (FloatType value) noexcept
{
    return std::nextafter (value, std::numeric_limits<FloatType>::max());
}

/** Returns the next representable value by FloatType in the direction of the lowest representable value. */
template <typename FloatType>
FloatType nextFloatDown (FloatType value) noexcept
{
    return std::nextafter (value, std::numeric_limits<FloatType>::lowest());
}

//==============================================================================
// Some indispensable min/max functions

/** Returns the larger of two values. */
template <typename Type>
constexpr Type jmax (Type a, Type b)                                   { return a < b ? b : a; }

/** Returns the larger of three values. */
template <typename Type>
constexpr Type jmax (Type a, Type b, Type c)                           { return a < b ? (b < c ? c : b) : (a < c ? c : a); }

/** Returns the larger of four values. */
template <typename Type>
constexpr Type jmax (Type a, Type b, Type c, Type d)                   { return jmax (a, jmax (b, c, d)); }

/** Returns the smaller of two values. */
template <typename Type>
constexpr Type jmin (Type a, Type b)                                   { return b < a ? b : a; }

/** Returns the smaller of three values. */
template <typename Type>
constexpr Type jmin (Type a, Type b, Type c)                           { return b < a ? (c < b ? c : b) : (c < a ? c : a); }

/** Returns the smaller of four values. */
template <typename Type>
constexpr Type jmin (Type a, Type b, Type c, Type d)                   { return jmin (a, jmin (b, c, d)); }

/** Remaps a normalised value (between 0 and 1) to a target range.
    This effectively returns (targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin)).
*/
template <typename Type>
constexpr Type jmap (Type value0To1, Type targetRangeMin, Type targetRangeMax)
{
    return targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin);
}

/** Remaps a value from a source range to a target range. */
template <typename Type>
Type jmap (Type sourceValue, Type sourceRangeMin, Type sourceRangeMax, Type targetRangeMin, Type targetRangeMax)
{
    jassert (! approximatelyEqual (sourceRangeMax, sourceRangeMin)); // mapping from a range of zero will produce NaN!
    return targetRangeMin + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
}

/** Remaps a normalised value (between 0 and 1) to a logarithmic target range.

    The entire target range must be greater than zero.

    @see mapFromLog10

    @code
    mapToLog10 (0.5, 0.4, 40.0) == 4.0
    @endcode
*/
template <typename Type>
Type mapToLog10 (Type value0To1, Type logRangeMin, Type logRangeMax)
{
    jassert (logRangeMin > 0);
    jassert (logRangeMax > 0);

    auto logMin = std::log10 (logRangeMin);
    auto logMax = std::log10 (logRangeMax);

    return std::pow ((Type) 10.0, value0To1 * (logMax - logMin) + logMin);
}

/** Remaps a logarithmic value in a target range to a normalised value (between 0 and 1).

    The entire target range must be greater than zero.

    @see mapToLog10

    @code
    mapFromLog10 (4.0, 0.4, 40.0) == 0.5
    @endcode
*/
template <typename Type>
Type mapFromLog10 (Type valueInLogRange, Type logRangeMin, Type logRangeMax)
{
    jassert (logRangeMin > 0);
    jassert (logRangeMax > 0);

    auto logMin = std::log10 (logRangeMin);
    auto logMax = std::log10 (logRangeMax);

    return (std::log10 (valueInLogRange) - logMin) / (logMax - logMin);
}

/** Scans an array of values, returning the minimum value that it contains. */
template <typename Type, typename Size>
Type findMinimum (const Type* data, Size numValues)
{
    if (numValues <= 0)
        return Type (0);

    auto result = *data++;

    while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
    {
        auto v = *data++;

        if (v < result)
            result = v;
    }

    return result;
}

/** Scans an array of values, returning the maximum value that it contains. */
template <typename Type, typename Size>
Type findMaximum (const Type* values, Size numValues)
{
    if (numValues <= 0)
        return Type (0);

    auto result = *values++;

    while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
    {
        auto v = *values++;

        if (result < v)
            result = v;
    }

    return result;
}

/** Scans an array of values, returning the minimum and maximum values that it contains. */
template <typename Type>
z0 findMinAndMax (const Type* values, i32 numValues, Type& lowest, Type& highest)
{
    if (numValues <= 0)
    {
        lowest = Type (0);
        highest = Type (0);
    }
    else
    {
        auto mn = *values++;
        auto mx = mn;

        while (--numValues > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
        {
            auto v = *values++;

            if (mx < v)  mx = v;
            if (v < mn)  mn = v;
        }

        lowest = mn;
        highest = mx;
    }
}

//==============================================================================
/** Constrains a value to keep it within a given range.

    This will check that the specified value lies between the lower and upper bounds
    specified, and if not, will return the nearest value that would be in-range. Effectively,
    it's like calling jmax (lowerLimit, jmin (upperLimit, value)).

    Note that it expects that lowerLimit <= upperLimit. If this isn't true,
    the results will be unpredictable.

    @param lowerLimit           the minimum value to return
    @param upperLimit           the maximum value to return
    @param valueToConstrain     the value to try to return
    @returns    the closest value to valueToConstrain which lies between lowerLimit
                and upperLimit (inclusive)
    @see jmin, jmax, jmap
*/
template <typename Type>
Type jlimit (Type lowerLimit,
             Type upperLimit,
             Type valueToConstrain) noexcept
{
    jassert (lowerLimit <= upperLimit); // if these are in the wrong order, results are unpredictable..

    return valueToConstrain < lowerLimit ? lowerLimit
                                         : (upperLimit < valueToConstrain ? upperLimit
                                                                          : valueToConstrain);
}

/** Возвращает true, если a value is at least zero, and also below a specified upper limit.
    This is basically a quicker way to write:
    @code valueToTest >= 0 && valueToTest < upperLimit
    @endcode
*/
template <typename Type1, typename Type2>
b8 isPositiveAndBelow (Type1 valueToTest, Type2 upperLimit) noexcept
{
    jassert (Type1() <= static_cast<Type1> (upperLimit)); // makes no sense to call this if the upper limit is itself below zero..
    return Type1() <= valueToTest && valueToTest < static_cast<Type1> (upperLimit);
}

template <typename Type>
b8 isPositiveAndBelow (i32 valueToTest, Type upperLimit) noexcept
{
    jassert (upperLimit >= 0); // makes no sense to call this if the upper limit is itself below zero..
    return static_cast<u32> (valueToTest) < static_cast<u32> (upperLimit);
}

/** Возвращает true, если a value is at least zero, and also less than or equal to a specified upper limit.
    This is basically a quicker way to write:
    @code valueToTest >= 0 && valueToTest <= upperLimit
    @endcode
*/
template <typename Type1, typename Type2>
b8 isPositiveAndNotGreaterThan (Type1 valueToTest, Type2 upperLimit) noexcept
{
    jassert (Type1() <= static_cast<Type1> (upperLimit)); // makes no sense to call this if the upper limit is itself below zero..
    return Type1() <= valueToTest && valueToTest <= static_cast<Type1> (upperLimit);
}

template <typename Type>
b8 isPositiveAndNotGreaterThan (i32 valueToTest, Type upperLimit) noexcept
{
    jassert (upperLimit >= 0); // makes no sense to call this if the upper limit is itself below zero..
    return static_cast<u32> (valueToTest) <= static_cast<u32> (upperLimit);
}

/** Computes the absolute difference between two values and returns true if it is less than or equal
    to a given tolerance, otherwise it returns false.
*/
template <typename Type>
b8 isWithin (Type a, Type b, Type tolerance) noexcept
{
    return std::abs (a - b) <= tolerance;
}

//==============================================================================
#if DRX_MSVC
 #pragma optimize ("t", off)
 #ifndef __INTEL_COMPILER
  #pragma float_control (precise, on, push)
 #endif
#endif

/** Fast floating-point-to-integer conversion.

    This is faster than using the normal c++ cast to convert a f32 to an i32, and
    it will round the value to the nearest integer, rather than rounding it down
    like the normal cast does.

    Note that this routine gets its speed at the expense of some accuracy, and when
    rounding values whose floating point component is exactly 0.5, odd numbers and
    even numbers will be rounded up or down differently.
*/
template <typename FloatType>
i32 roundToInt (const FloatType value) noexcept
{
  #ifdef __INTEL_COMPILER
   #pragma float_control (precise, on, push)
  #endif

    union { i32 asInt[2]; f64 asDouble; } n;
    n.asDouble = ((f64) value) + 6755399441055744.0;

   #if DRX_BIG_ENDIAN
    return n.asInt [1];
   #else
    return n.asInt [0];
   #endif
}

inline i32 roundToInt (i32 value) noexcept
{
    return value;
}

#if DRX_MSVC
 #ifndef __INTEL_COMPILER
  #pragma float_control (pop)
 #endif
 #pragma optimize ("", on)  // resets optimisations to the project defaults
#endif

/** Fast floating-point-to-integer conversion.

    This is a slightly slower and slightly more accurate version of roundToInt(). It works
    fine for values above zero, but negative numbers are rounded the wrong way.
*/
inline i32 roundToIntAccurate (f64 value) noexcept
{
   #ifdef __INTEL_COMPILER
    #pragma float_control (pop)
   #endif

    return roundToInt (value + 1.5e-8);
}

//==============================================================================
/** Truncates a positive floating-point number to an u32.

    This is generally faster than static_cast<u32> (std::floor (x))
    but it only works for positive numbers small enough to be represented as an
    u32.
*/
template <typename FloatType>
u32 truncatePositiveToUnsignedInt (FloatType value) noexcept
{
    jassert (value >= static_cast<FloatType> (0));
    jassert (static_cast<FloatType> (value)
             <= static_cast<FloatType> (std::numeric_limits<u32>::max()));

    return static_cast<u32> (value);
}

//==============================================================================
/** Возвращает true, если the specified integer is a power-of-two. */
template <typename IntegerType>
constexpr b8 isPowerOfTwo (IntegerType value)
{
   return (value & (value - 1)) == 0;
}

/** Returns the smallest power-of-two which is equal to or greater than the given integer. */
inline i32 nextPowerOfTwo (i32 n) noexcept
{
    --n;
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    return n + 1;
}

/** Returns the index of the highest set bit in a (non-zero) number.
    So for n=3 this would return 1, for n=7 it returns 2, etc.
    An input value of 0 is illegal!
*/
i32 findHighestSetBit (u32 n) noexcept;

/** Returns the number of bits in a 32-bit integer. */
constexpr i32 countNumberOfBits (u32 n) noexcept
{
    n -= ((n >> 1) & 0x55555555);
    n =  (((n >> 2) & 0x33333333) + (n & 0x33333333));
    n =  (((n >> 4) + n) & 0x0f0f0f0f);
    n += (n >> 8);
    n += (n >> 16);
    return (i32) (n & 0x3f);
}

/** Returns the number of bits in a 64-bit integer. */
constexpr i32 countNumberOfBits (zu64 n) noexcept
{
    return countNumberOfBits ((u32) n) + countNumberOfBits ((u32) (n >> 32));
}

/** Performs a modulo operation, but can cope with the dividend being negative.
    The divisor must be greater than zero.
*/
template <typename IntegerType>
IntegerType negativeAwareModulo (IntegerType dividend, const IntegerType divisor) noexcept
{
    jassert (divisor > 0);
    dividend %= divisor;
    return (dividend < 0) ? (dividend + divisor) : dividend;
}

/** Returns the square of its argument. */
template <typename NumericType>
inline constexpr NumericType square (NumericType n) noexcept
{
    return n * n;
}

//==============================================================================
/** Writes a number of bits into a memory buffer at a given bit index.
    The buffer is treated as a sequence of 8-bit bytes, and the value is encoded in little-endian order,
    so for example if startBit = 10, and numBits = 11 then the lower 6 bits of the value would be written
    into bits 2-8 of targetBuffer[1], and the upper 5 bits of value into bits 0-5 of targetBuffer[2].

    @see readLittleEndianBitsInBuffer
*/
z0 writeLittleEndianBitsInBuffer (uk targetBuffer, u32 startBit, u32 numBits, u32 value) noexcept;

/** Reads a number of bits from a buffer at a given bit index.
    The buffer is treated as a sequence of 8-bit bytes, and the value is encoded in little-endian order,
    so for example if startBit = 10, and numBits = 11 then the lower 6 bits of the result would be read
    from bits 2-8 of sourceBuffer[1], and the upper 5 bits of the result from bits 0-5 of sourceBuffer[2].

    @see writeLittleEndianBitsInBuffer
*/
u32 readLittleEndianBitsInBuffer (ukk sourceBuffer, u32 startBit, u32 numBits) noexcept;


//==============================================================================
#if DRX_INTEL || DOXYGEN
 /** This macro can be applied to a f32 variable to check whether it contains a denormalised
     value, and to normalise it if necessary.
     On CPUs that aren't vulnerable to denormalisation problems, this will have no effect.
 */
 #define DRX_UNDENORMALISE(x)   { (x) += 0.1f; (x) -= 0.1f; }
#else
 #define DRX_UNDENORMALISE(x)
#endif

//==============================================================================
/** This namespace contains a few template classes for helping work out class type variations.
*/
namespace TypeHelpers
{
    /** The ParameterType struct is used to find the best type to use when passing some kind
        of object as a parameter.

        Of course, this is only likely to be useful in certain esoteric template situations.

        E.g. "myFunction (typename TypeHelpers::ParameterType<i32>::type, typename TypeHelpers::ParameterType<MyObject>::type)"
        would evaluate to "myfunction (i32, const MyObject&)", keeping any primitive types as
        pass-by-value, but passing objects as a const reference, to avoid copying.

        @tags{Core}
    */
    template <typename Type> struct ParameterType                   { using type = const Type&; };

   #ifndef DOXYGEN
    template <typename Type> struct ParameterType <Type&>           { using type = Type&; };
    template <typename Type> struct ParameterType <Type*>           { using type = Type*; };
    template <>              struct ParameterType <t8>            { using type = t8; };
    template <>              struct ParameterType <u8>   { using type = u8; };
    template <>              struct ParameterType <short>           { using type = short; };
    template <>              struct ParameterType <u16>  { using type = u16; };
    template <>              struct ParameterType <i32>             { using type = i32; };
    template <>              struct ParameterType <u32>    { using type = u32; };
    template <>              struct ParameterType <i64>            { using type = i64; };
    template <>              struct ParameterType <u64>   { using type = u64; };
    template <>              struct ParameterType <z64>  { using type = z64; };
    template <>              struct ParameterType <zu64>{ using type = zu64; };
    template <>              struct ParameterType <b8>            { using type = b8; };
    template <>              struct ParameterType <f32>           { using type = f32; };
    template <>              struct ParameterType <f64>          { using type = f64; };
   #endif

    /** These templates are designed to take a type, and if it's a f64, they return a f64
        type; for anything else, they return a f32 type.

        @tags{Core}
    */
    template <typename Type>
    using SmallestFloatType = std::conditional_t<std::is_same_v<Type, f64>, f64, f32>;

    /** These templates are designed to take an integer type, and return an u32
        version with the same size.

        @tags{Core}
    */
    template <i32 bytes>     struct UnsignedTypeWithSize            {};

   #ifndef DOXYGEN
    template <>              struct UnsignedTypeWithSize<1>         { using type = u8; };
    template <>              struct UnsignedTypeWithSize<2>         { using type = u16; };
    template <>              struct UnsignedTypeWithSize<4>         { using type = u32; };
    template <>              struct UnsignedTypeWithSize<8>         { using type = zu64; };
   #endif
}

//==============================================================================
#ifndef DOXYGEN
 [[deprecated ("Use roundToInt instead.")]] inline i32 roundDoubleToInt (f64 value) noexcept  { return roundToInt (value); }
 [[deprecated ("Use roundToInt instead.")]] inline i32 roundFloatToInt  (f32  value) noexcept  { return roundToInt (value); }
 [[deprecated ("Use std::abs() instead.")]] inline z64 abs64 (z64 n) noexcept                { return std::abs (n); }
#endif

/** Converts an enum to its underlying integral type.
    Similar to std::to_underlying, which is only available in C++23 and above.
*/
template <typename T>
constexpr auto toUnderlyingType (T t) -> std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>>
{
    return static_cast<std::underlying_type_t<T>> (t);
}

} // namespace drx
