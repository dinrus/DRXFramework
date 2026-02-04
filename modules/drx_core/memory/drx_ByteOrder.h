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
/** Contains static methods for converting the byte order between different
    endiannesses.

    @tags{Core}
*/
class DRX_API  ByteOrder
{
public:
    //==============================================================================
    /** Swaps the upper and lower bytes of a 16-bit integer. */
    constexpr static u16 swap (u16 value) noexcept;

    /** Swaps the upper and lower bytes of a 16-bit integer. */
    constexpr static i16 swap (i16 value) noexcept;

    /** Reverses the order of the 4 bytes in a 32-bit integer. */
    static u32 swap (u32 value) noexcept;

    /** Reverses the order of the 4 bytes in a 32-bit integer. */
    static i32 swap (i32 value) noexcept;

    /** Reverses the order of the 8 bytes in a 64-bit integer. */
    static zu64 swap (zu64 value) noexcept;

    /** Reverses the order of the 8 bytes in a 64-bit integer. */
    static z64 swap (z64 value) noexcept;

    /** Returns a garbled f32 which has the reverse byte-order of the original. */
    static f32 swap (f32 value) noexcept;

    /** Returns a garbled f64 which has the reverse byte-order of the original. */
    static f64 swap (f64 value) noexcept;

    //==============================================================================
    /** Swaps the byte order of a signed or u32 integer if the CPU is big-endian */
    template <typename Type>
    static Type swapIfBigEndian (Type value) noexcept
    {
       #if DRX_LITTLE_ENDIAN
        return value;
       #else
        return swap (value);
       #endif
    }

    /** Swaps the byte order of a signed or u32 integer if the CPU is little-endian */
    template <typename Type>
    static Type swapIfLittleEndian (Type value) noexcept
    {
       #if DRX_LITTLE_ENDIAN
        return swap (value);
       #else
        return value;
       #endif
    }

    //==============================================================================
    /** Turns 4 bytes into a little-endian integer. */
    constexpr static u32 littleEndianInt (ukk bytes) noexcept;

    /** Turns 8 bytes into a little-endian integer. */
    constexpr static zu64 littleEndianInt64 (ukk bytes) noexcept;

    /** Turns 2 bytes into a little-endian integer. */
    constexpr static u16 littleEndianShort (ukk bytes) noexcept;

    /** Converts 3 little-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
    constexpr static i32 littleEndian24Bit (ukk bytes) noexcept;

    /** Copies a 24-bit number to 3 little-endian bytes. */
    static z0 littleEndian24BitToChars (i32 value, uk destBytes) noexcept;

    //==============================================================================
    /** Turns 4 bytes into a big-endian integer. */
    constexpr static u32 bigEndianInt (ukk bytes) noexcept;

    /** Turns 8 bytes into a big-endian integer. */
    constexpr static zu64 bigEndianInt64 (ukk bytes) noexcept;

    /** Turns 2 bytes into a big-endian integer. */
    constexpr static u16 bigEndianShort (ukk bytes) noexcept;

    /** Converts 3 big-endian bytes into a signed 24-bit value (which is sign-extended to 32 bits). */
    constexpr static i32 bigEndian24Bit (ukk bytes) noexcept;

    /** Copies a 24-bit number to 3 big-endian bytes. */
    static z0 bigEndian24BitToChars (i32 value, uk destBytes) noexcept;

    //==============================================================================
    /** Constructs a 16-bit integer from its constituent bytes, in order of significance. */
    constexpr static u16 makeInt (u8 leastSig, u8 mostSig) noexcept;

    /** Constructs a 32-bit integer from its constituent bytes, in order of significance. */
    constexpr static u32 makeInt (u8 leastSig, u8 byte1, u8 byte2, u8 mostSig) noexcept;

    /** Constructs a 64-bit integer from its constituent bytes, in order of significance. */
    constexpr static zu64 makeInt (u8 leastSig, u8 byte1, u8 byte2, u8 byte3,
                                     u8 byte4, u8 byte5, u8 byte6, u8 mostSig) noexcept;

    //==============================================================================
    /** Возвращает true, если the current CPU is big-endian. */
    constexpr static b8 isBigEndian() noexcept
    {
       #if DRX_LITTLE_ENDIAN
        return false;
       #else
        return true;
       #endif
    }

private:
    ByteOrder() = delete;
};


//==============================================================================
constexpr inline u16 ByteOrder::swap (u16 v) noexcept         { return static_cast<u16> ((v << 8) | (v >> 8)); }
constexpr inline i16  ByteOrder::swap (i16  v) noexcept         { return static_cast<i16> (swap (static_cast<u16> (v))); }
inline i32  ByteOrder::swap (i32 v) noexcept                    { return static_cast<i32> (swap (static_cast<u32> (v))); }
inline z64  ByteOrder::swap (z64 v) noexcept                    { return static_cast<z64> (swap (static_cast<zu64> (v))); }
inline f32  ByteOrder::swap (f32 v) noexcept                    { union { u32 asUInt; f32 asFloat;  } n; n.asFloat = v; n.asUInt = swap (n.asUInt); return n.asFloat; }
inline f64 ByteOrder::swap (f64 v) noexcept                   { union { zu64 asUInt; f64 asFloat; } n; n.asFloat = v; n.asUInt = swap (n.asUInt); return n.asFloat; }

#if DRX_MSVC && ! defined (__INTEL_COMPILER)
 #pragma intrinsic (_byteswap_ulong)
#endif

inline u32 ByteOrder::swap (u32 n) noexcept
{
   #if DRX_MAC || DRX_IOS
    return OSSwapInt32 (n);
   #elif (DRX_GCC  || DRX_CLANG) && DRX_INTEL && ! DRX_NO_INLINE_ASM
    asm("bswap %%eax" : "=a"(n) : "a"(n));
    return n;
   #elif DRX_MSVC
    return _byteswap_ulong (n);
   #elif DRX_ANDROID
    return bswap_32 (n);
   #else
    return (n << 24) | (n >> 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8);
   #endif
}

inline zu64 ByteOrder::swap (zu64 value) noexcept
{
   #if DRX_MAC || DRX_IOS
    return OSSwapInt64 (value);
   #elif DRX_MSVC
    return _byteswap_uint64 (value);
   #else
    return (((zu64) swap ((u32) value)) << 32) | swap ((u32) (value >> 32));
   #endif
}

constexpr inline u16 ByteOrder::makeInt (u8 b0, u8 b1) noexcept
{
    return static_cast<u16> (static_cast<u16> (b0) | (static_cast<u16> (b1) << 8));
}

constexpr inline u32 ByteOrder::makeInt (u8 b0, u8 b1, u8 b2, u8 b3) noexcept
{
    return static_cast<u32> (b0)        | (static_cast<u32> (b1) << 8)
        | (static_cast<u32> (b2) << 16) | (static_cast<u32> (b3) << 24);
}

constexpr inline zu64 ByteOrder::makeInt (u8 b0, u8 b1, u8 b2, u8 b3, u8 b4, u8 b5, u8 b6, u8 b7) noexcept
{
    return static_cast<zu64> (b0)        | (static_cast<zu64> (b1) << 8)  | (static_cast<zu64> (b2) << 16) | (static_cast<zu64> (b3) << 24)
        | (static_cast<zu64> (b4) << 32) | (static_cast<zu64> (b5) << 40) | (static_cast<zu64> (b6) << 48) | (static_cast<zu64> (b7) << 56);
}

constexpr inline u16 ByteOrder::littleEndianShort (ukk bytes) noexcept         { return makeInt (static_cast<u8k*> (bytes)[0], static_cast<u8k*> (bytes)[1]); }
constexpr inline u32 ByteOrder::littleEndianInt   (ukk bytes) noexcept         { return makeInt (static_cast<u8k*> (bytes)[0], static_cast<u8k*> (bytes)[1],
                                                                                                            static_cast<u8k*> (bytes)[2], static_cast<u8k*> (bytes)[3]); }
constexpr inline zu64 ByteOrder::littleEndianInt64 (ukk bytes) noexcept         { return makeInt (static_cast<u8k*> (bytes)[0], static_cast<u8k*> (bytes)[1],
                                                                                                            static_cast<u8k*> (bytes)[2], static_cast<u8k*> (bytes)[3],
                                                                                                            static_cast<u8k*> (bytes)[4], static_cast<u8k*> (bytes)[5],
                                                                                                            static_cast<u8k*> (bytes)[6], static_cast<u8k*> (bytes)[7]); }

constexpr inline u16 ByteOrder::bigEndianShort    (ukk bytes) noexcept         { return makeInt (static_cast<u8k*> (bytes)[1], static_cast<u8k*> (bytes)[0]); }
constexpr inline u32 ByteOrder::bigEndianInt      (ukk bytes) noexcept         { return makeInt (static_cast<u8k*> (bytes)[3], static_cast<u8k*> (bytes)[2],
                                                                                                            static_cast<u8k*> (bytes)[1], static_cast<u8k*> (bytes)[0]); }
constexpr inline zu64 ByteOrder::bigEndianInt64    (ukk bytes) noexcept         { return makeInt (static_cast<u8k*> (bytes)[7], static_cast<u8k*> (bytes)[6],
                                                                                                            static_cast<u8k*> (bytes)[5], static_cast<u8k*> (bytes)[4],
                                                                                                            static_cast<u8k*> (bytes)[3], static_cast<u8k*> (bytes)[2],
                                                                                                            static_cast<u8k*> (bytes)[1], static_cast<u8k*> (bytes)[0]); }

constexpr inline i32 ByteOrder::littleEndian24Bit  (ukk bytes) noexcept         { return (i32) ((((u32) static_cast<const i8*> (bytes)[2]) << 16) | (((u32) static_cast<u8k*> (bytes)[1]) << 8) | ((u32) static_cast<u8k*> (bytes)[0])); }
constexpr inline i32 ByteOrder::bigEndian24Bit     (ukk bytes) noexcept         { return (i32) ((((u32) static_cast<const i8*> (bytes)[0]) << 16) | (((u32) static_cast<u8k*> (bytes)[1]) << 8) | ((u32) static_cast<u8k*> (bytes)[2])); }

inline z0 ByteOrder::littleEndian24BitToChars (i32 value, uk destBytes) noexcept   { static_cast<u8*> (destBytes)[0] = (u8) value;         static_cast<u8*> (destBytes)[1] = (u8) (value >> 8); static_cast<u8*> (destBytes)[2] = (u8) (value >> 16); }
inline z0 ByteOrder::bigEndian24BitToChars    (i32 value, uk destBytes) noexcept   { static_cast<u8*> (destBytes)[0] = (u8) (value >> 16); static_cast<u8*> (destBytes)[1] = (u8) (value >> 8); static_cast<u8*> (destBytes)[2] = (u8) value; }

} // namespace drx
