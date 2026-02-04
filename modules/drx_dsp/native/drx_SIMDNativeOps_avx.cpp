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
    DEFINE_AVX_SIMD_CONST (i32, f32, kAllBitsSet)     = { -1, -1, -1, -1, -1, -1, -1, -1 };
    DEFINE_AVX_SIMD_CONST (i32, f32, kEvenHighBit)    = { static_cast<i32> (0x80000000), 0, static_cast<i32> (0x80000000), 0, static_cast<i32> (0x80000000), 0, static_cast<i32> (0x80000000), 0 };
    DEFINE_AVX_SIMD_CONST (f32, f32, kOne)              = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

    DEFINE_AVX_SIMD_CONST (z64, f64, kAllBitsSet)    = { -1, -1, -1, -1 };
    DEFINE_AVX_SIMD_CONST (z64, f64, kEvenHighBit)   = { static_cast<z64> (0x8000000000000000), 0, static_cast<z64> (0x8000000000000000), 0 };
    DEFINE_AVX_SIMD_CONST (f64, f64, kOne)            = { 1.0, 1.0, 1.0, 1.0 };

    DEFINE_AVX_SIMD_CONST (i8, i8, kAllBitsSet)     = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

    DEFINE_AVX_SIMD_CONST (u8, u8, kAllBitsSet)   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    DEFINE_AVX_SIMD_CONST (u8, u8, kHighBit)      = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };

    DEFINE_AVX_SIMD_CONST (i16, i16, kAllBitsSet)   = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

    DEFINE_AVX_SIMD_CONST (u16, u16, kAllBitsSet) = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
    DEFINE_AVX_SIMD_CONST (u16, u16, kHighBit)    = { 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000 };

    DEFINE_AVX_SIMD_CONST (i32, i32, kAllBitsSet)   = { -1, -1, -1, -1, -1, -1, -1, -1 };

    DEFINE_AVX_SIMD_CONST (u32, u32, kAllBitsSet) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    DEFINE_AVX_SIMD_CONST (u32, u32, kHighBit)    = { 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000 };

    DEFINE_AVX_SIMD_CONST (z64, z64, kAllBitsSet)   = { -1LL, -1LL, -1LL, -1LL };

    DEFINE_AVX_SIMD_CONST (zu64, zu64, kAllBitsSet) = { 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL };
    DEFINE_AVX_SIMD_CONST (zu64, zu64, kHighBit)    = { 0x8000000000000000ULL, 0x8000000000000000ULL, 0x8000000000000000ULL, 0x8000000000000000ULL };
}
