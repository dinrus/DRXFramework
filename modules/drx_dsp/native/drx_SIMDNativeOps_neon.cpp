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
    DEFINE_NEON_SIMD_CONST (i32, f32, kAllBitsSet)     = { -1, -1, -1, -1 };
    DEFINE_NEON_SIMD_CONST (i32, f32, kEvenHighBit)    = { static_cast<i32> (0x80000000), 0, static_cast<i32> (0x80000000), 0 };
    DEFINE_NEON_SIMD_CONST (f32, f32, kOne)              = { 1.0f, 1.0f, 1.0f, 1.0f };

   #if DRX_64BIT
    DEFINE_NEON_SIMD_CONST (z64, f64, kAllBitsSet)    = { -1, -1 };
    DEFINE_NEON_SIMD_CONST (f64, f64, kOne)            = { 1.0, 1.0 };
   #endif

    DEFINE_NEON_SIMD_CONST (i8, i8, kAllBitsSet)     = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    DEFINE_NEON_SIMD_CONST (u8, u8, kAllBitsSet)   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    DEFINE_NEON_SIMD_CONST (i16, i16, kAllBitsSet)   = { -1, -1, -1, -1, -1, -1, -1, -1 };
    DEFINE_NEON_SIMD_CONST (u16, u16, kAllBitsSet) = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
    DEFINE_NEON_SIMD_CONST (i32, i32, kAllBitsSet)   = { -1, -1, -1, -1 };
    DEFINE_NEON_SIMD_CONST (u32, u32, kAllBitsSet) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    DEFINE_NEON_SIMD_CONST (z64, z64, kAllBitsSet)   = { -1, -1 };
    DEFINE_NEON_SIMD_CONST (zu64, zu64, kAllBitsSet) = { 0xffffffffffffffff, 0xffffffffffffffff };
}
