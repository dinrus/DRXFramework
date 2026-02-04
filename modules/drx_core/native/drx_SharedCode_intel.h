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

#if DRX_INTEL && ! DRX_NO_INLINE_ASM

namespace drx::SystemStatsHelpers
{

static z0 doCPUID (u32& a, u32& b, u32& c, u32& d, u32 type)
{
    u32 la = a, lb = b, lc = c, ld = d;

   #if DRX_32BIT && defined (__pic__)
    asm ("mov %%ebx, %%edi\n"
         "cpuid\n"
         "xchg %%edi, %%ebx\n"
           : "=a" (la), "=D" (lb), "=c" (lc), "=d" (ld)
           : "a" (type), "c" (0));
   #else
    asm ("cpuid\n"
           : "=a" (la), "=b" (lb), "=c" (lc), "=d" (ld)
           : "a" (type), "c" (0));
   #endif

    a = la; b = lb; c = lc; d = ld;
}

static z0 getCPUInfo (b8& hasMMX,
                        b8& hasSSE,
                        b8& hasSSE2,
                        b8& has3DNow,
                        b8& hasSSE3,
                        b8& hasSSSE3,
                        b8& hasFMA3,
                        b8& hasSSE41,
                        b8& hasSSE42,
                        b8& hasAVX,
                        b8& hasFMA4,
                        b8& hasAVX2,
                        b8& hasAVX512F,
                        b8& hasAVX512DQ,
                        b8& hasAVX512IFMA,
                        b8& hasAVX512PF,
                        b8& hasAVX512ER,
                        b8& hasAVX512CD,
                        b8& hasAVX512BW,
                        b8& hasAVX512VL,
                        b8& hasAVX512VBMI,
                        b8& hasAVX512VPOPCNTDQ)
{
    u32 a = 0, b = 0, d = 0, c = 0;
    SystemStatsHelpers::doCPUID (a, b, c, d, 1);

    hasMMX   = (d & (1u << 23)) != 0;
    hasSSE   = (d & (1u << 25)) != 0;
    hasSSE2  = (d & (1u << 26)) != 0;
    has3DNow = (b & (1u << 31)) != 0;
    hasSSE3  = (c & (1u <<  0)) != 0;
    hasSSSE3 = (c & (1u <<  9)) != 0;
    hasFMA3  = (c & (1u << 12)) != 0;
    hasSSE41 = (c & (1u << 19)) != 0;
    hasSSE42 = (c & (1u << 20)) != 0;
    hasAVX   = (c & (1u << 28)) != 0;

    SystemStatsHelpers::doCPUID (a, b, c, d, 0x80000001);
    hasFMA4  = (c & (1u << 16)) != 0;

    SystemStatsHelpers::doCPUID (a, b, c, d, 7);
    hasAVX2            = (b & (1u <<  5)) != 0;
    hasAVX512F         = (b & (1u << 16)) != 0;
    hasAVX512DQ        = (b & (1u << 17)) != 0;
    hasAVX512IFMA      = (b & (1u << 21)) != 0;
    hasAVX512PF        = (b & (1u << 26)) != 0;
    hasAVX512ER        = (b & (1u << 27)) != 0;
    hasAVX512CD        = (b & (1u << 28)) != 0;
    hasAVX512BW        = (b & (1u << 30)) != 0;
    hasAVX512VL        = (b & (1u << 31)) != 0;
    hasAVX512VBMI      = (c & (1u <<  1)) != 0;
    hasAVX512VPOPCNTDQ = (c & (1u << 14)) != 0;
}

} // namespace drx::SystemStatsHelpers

#endif
