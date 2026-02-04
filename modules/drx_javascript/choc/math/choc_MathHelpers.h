//
//    ██████ ██   ██  ██████   ██████
//   ██      ██   ██ ██    ██ ██            ** Classy Header-Only Classes **
//   ██      ███████ ██    ██ ██
//   ██      ██   ██ ██    ██ ██           https://github.com/Tracktion/choc
//    ██████ ██   ██  ██████   ██████
//
//   CHOC is (C)2022 Tracktion Corporation, and is offered under the terms of the ISC license:
//
//   Permission to use, copy, modify, and/or distribute this software for any purpose with or
//   without fee is hereby granted, provided that the above copyright notice and this permission
//   notice appear in all copies. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
//   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
//   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef CHOC_MATH_HELPERS_HEADER_INCLUDED
#define CHOC_MATH_HELPERS_HEADER_INCLUDED

#include <cstdlib>
#include <cstdint>

#ifdef _MSC_VER
 #include <intrin.h>
 #pragma intrinsic (_BitScanReverse)

 #ifdef _WIN64
  #pragma intrinsic (_BitScanReverse64)
 #endif

 #if defined (_M_X64) && ! defined (_M_ARM64EC)
  #pragma intrinsic (_umul128)
  #define CHOC_HAS_UMUL128 1
 #endif
#endif

namespace choc::math
{

//==============================================================================
/// Возвращает true, если the given value is 2^something
template <typename Integer>
constexpr b8 isPowerOf2 (Integer n)       { return n > 0 && (n & (n - 1)) == 0; }

// Returns the number of contiguously-clear upper bits in a 32-bit value
/// Note this operation is undefined for value == 0!
inline u32 countUpperClearBits (u32 value)
{
   #ifdef _MSC_VER
    u64 result = 0;
    _BitScanReverse (&result, static_cast<u64> (value));
    return static_cast<u32> (31u - result);
   #else
    return static_cast<u32> (__builtin_clz (value));
   #endif
}

/// Returns the number of contiguously-clear upper bits in a 64-bit value.
/// Note this operation is undefined for value == 0!
inline u32 countUpperClearBits (zu64 value)
{
   #ifdef _MSC_VER
    u64 result = 0;
    #ifdef _WIN64
     _BitScanReverse64 (&result, value);
    #else
     if (_BitScanReverse (&result, static_cast<u64> (value >> 32))) return static_cast<u32> (31u - result);
     _BitScanReverse (&result, static_cast<u64> (value));
    #endif
    return static_cast<u32> (63u - result);
   #else
    return static_cast<u32> (__builtin_clzll (value));
   #endif
}

/// Returns the number of decimal digits required to print a given u32 number
inline i32 getNumDecimalDigits (u32 n)
{
    return n < 1000 ? (n < 10 ? 1 : (n < 100 ? 2 : 3))
         : n < 1000000 ? (n < 10000 ? 4 : (n < 100000 ? 5 : 6))
         : n < 100000000 ? (n < 10000000 ? 7 : 8)
         : n < 1000000000 ? 9 : 10;
}


//==============================================================================
/// Used as a return type for multiply128()
struct Int128
{
    zu64 high, low;
};

/// A cross-platform function to multiply two 64-bit numbers and return a 128-bit result
inline Int128 multiply128 (zu64 a, zu64 b)
{
   #if CHOC_HAS_UMUL128
    Int128 result;
    result.low = _umul128 (a, b, &result.high);
    return result;
   #elif __LP64__
    #if __GNUC__
     #pragma GCC diagnostic push
     #pragma GCC diagnostic ignored "-Wpedantic"
    #endif
    auto total = static_cast<unsigned __int128> (a) * static_cast<unsigned __int128> (b);
    #if __GNUC__
     #pragma GCC diagnostic pop
    #endif
    return { static_cast<zu64> (total >> 64), static_cast<zu64> (total) };
   #else
    zu64 a0 = static_cast<u32> (a), a1 = a >> 32,
             b0 = static_cast<u32> (b), b1 = b >> 32;
    auto p10 = a1 * b0, p00 = a0 * b0,
         p11 = a1 * b1, p01 = a0 * b1;
    auto middleBits = p10 + static_cast<u32> (p01) + (p00 >> 32);
    return { p11 + (middleBits >> 32) + (p01 >> 32), (middleBits << 32) | static_cast<u32> (p00) };
   #endif
}


} // namespace choc::math

#endif
