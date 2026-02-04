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

#ifndef DOXYGEN

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wignored-attributes")

#ifdef _MSC_VER
 #define DECLARE_SSE_SIMD_CONST(type, name) \
    static __declspec (align (16)) const type name [16 / sizeof (type)]

 #define DEFINE_SSE_SIMD_CONST(type, class_type, name) \
    __declspec (align (16)) const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)]

#else
 #define DECLARE_SSE_SIMD_CONST(type, name) \
    static const type name [16 / sizeof (type)] __attribute__ ((aligned (16)))

 #define DEFINE_SSE_SIMD_CONST(type, class_type, name) \
    const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)] __attribute__ ((aligned (16)))

#endif

template <typename type>
struct SIMDNativeOps;

//==============================================================================
/** Single-precision floating point SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<f32>
{
    //==============================================================================
    using vSIMDType = __m128;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (i32, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (i32, kEvenHighBit);
    DECLARE_SSE_SIMD_CONST (f32, kOne);

    //==============================================================================
    static forcedinline __m128 DRX_VECTOR_CALLTYPE expand (f32 s) noexcept                            { return _mm_load1_ps (&s); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE load (const f32* a) noexcept                       { return _mm_load_ps (a); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m128 value, f32* dest) noexcept             { _mm_store_ps (dest, value); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE add (__m128 a, __m128 b) noexcept                    { return _mm_add_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE sub (__m128 a, __m128 b) noexcept                    { return _mm_sub_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE mul (__m128 a, __m128 b) noexcept                    { return _mm_mul_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE bit_and (__m128 a, __m128 b) noexcept                { return _mm_and_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE bit_or  (__m128 a, __m128 b) noexcept                { return _mm_or_ps  (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE bit_xor (__m128 a, __m128 b) noexcept                { return _mm_xor_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE bit_notand (__m128 a, __m128 b) noexcept             { return _mm_andnot_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE bit_not (__m128 a) noexcept                          { return bit_notand (a, _mm_loadu_ps ((f32*) kAllBitsSet)); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE min (__m128 a, __m128 b) noexcept                    { return _mm_min_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE max (__m128 a, __m128 b) noexcept                    { return _mm_max_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE equal (__m128 a, __m128 b) noexcept                  { return _mm_cmpeq_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE notEqual (__m128 a, __m128 b) noexcept               { return _mm_cmpneq_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE greaterThan (__m128 a, __m128 b) noexcept            { return _mm_cmpgt_ps (a, b); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128 a, __m128 b) noexcept     { return _mm_cmpge_ps (a, b); }
    static forcedinline b8   DRX_VECTOR_CALLTYPE allEqual (__m128 a, __m128 b ) noexcept              { return (_mm_movemask_ps (equal (a, b)) == 0xf); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE multiplyAdd (__m128 a, __m128 b, __m128 c) noexcept  { return _mm_add_ps (a, _mm_mul_ps (b, c)); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE dupeven (__m128 a) noexcept                          { return _mm_shuffle_ps (a, a, _MM_SHUFFLE (2, 2, 0, 0)); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE dupodd (__m128 a) noexcept                           { return _mm_shuffle_ps (a, a, _MM_SHUFFLE (3, 3, 1, 1)); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE swapevenodd (__m128 a) noexcept                      { return _mm_shuffle_ps (a, a, _MM_SHUFFLE (2, 3, 0, 1)); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE oddevensum (__m128 a) noexcept                       { return _mm_add_ps (_mm_shuffle_ps (a, a, _MM_SHUFFLE (1, 0, 3, 2)), a); }
    static forcedinline f32  DRX_VECTOR_CALLTYPE get (__m128 v, size_t i) noexcept                    { return SIMDFallbackOps<f32, __m128>::get (v, i); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE set (__m128 v, size_t i, f32 s) noexcept           { return SIMDFallbackOps<f32, __m128>::set (v, i, s); }
    static forcedinline __m128 DRX_VECTOR_CALLTYPE truncate (__m128 a) noexcept                         { return _mm_cvtepi32_ps (_mm_cvttps_epi32 (a)); }

    //==============================================================================
    static forcedinline __m128 DRX_VECTOR_CALLTYPE cmplxmul (__m128 a, __m128 b) noexcept
    {
        __m128 rr_ir = mul (a, dupeven (b));
        __m128 ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, _mm_loadu_ps ((f32*) kEvenHighBit)));
    }

    static forcedinline f32 DRX_VECTOR_CALLTYPE sum (__m128 a) noexcept
    {
       #if defined (__SSE4__)
        const auto retval = _mm_dp_ps (a, _mm_loadu_ps (kOne), 0xff);
       #elif defined (__SSE3__)
        const auto shuffled = _mm_movehdup_ps (a);
        const auto sums = _mm_add_ps (a, shuffled);
        const auto retval = _mm_add_ss (sums, _mm_movehl_ps (shuffled, sums));
       #else
        auto retval = _mm_add_ps (_mm_shuffle_ps (a, a, 0x4e), a);
        retval = _mm_add_ps (retval, _mm_shuffle_ps (retval, retval, 0xb1));
       #endif
        return _mm_cvtss_f32 (retval);
    }
};

//==============================================================================
/** Double-precision floating point SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<f64>
{
    //==============================================================================
    using vSIMDType = __m128d;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (z64, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (z64, kEvenHighBit);
    DECLARE_SSE_SIMD_CONST (f64, kOne);

    //==============================================================================
    static forcedinline __m128d DRX_VECTOR_CALLTYPE vconst (const f64* a) noexcept                       { return load (a); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE vconst (const z64* a) noexcept                      { return _mm_castsi128_pd (_mm_load_si128 (reinterpret_cast<const __m128i*> (a))); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE expand (f64 s) noexcept                              { return _mm_load1_pd (&s); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE load (const f64* a) noexcept                         { return _mm_load_pd (a); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m128d value, f64* dest) noexcept               { _mm_store_pd (dest, value); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE add (__m128d a, __m128d b) noexcept                     { return _mm_add_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE sub (__m128d a, __m128d b) noexcept                     { return _mm_sub_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE mul (__m128d a, __m128d b) noexcept                     { return _mm_mul_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE bit_and (__m128d a, __m128d b) noexcept                 { return _mm_and_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE bit_or  (__m128d a, __m128d b) noexcept                 { return _mm_or_pd  (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE bit_xor (__m128d a, __m128d b) noexcept                 { return _mm_xor_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE bit_notand (__m128d a, __m128d b) noexcept              { return _mm_andnot_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE bit_not (__m128d a) noexcept                            { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE min (__m128d a, __m128d b) noexcept                     { return _mm_min_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE max (__m128d a, __m128d b) noexcept                     { return _mm_max_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE equal (__m128d a, __m128d b) noexcept                   { return _mm_cmpeq_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE notEqual (__m128d a, __m128d b) noexcept                { return _mm_cmpneq_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE greaterThan (__m128d a, __m128d b) noexcept             { return _mm_cmpgt_pd (a, b); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128d a, __m128d b) noexcept      { return _mm_cmpge_pd (a, b); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m128d a, __m128d b ) noexcept               { return (_mm_movemask_pd (equal (a, b)) == 0x3); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE multiplyAdd (__m128d a, __m128d b, __m128d c) noexcept  { return _mm_add_pd (a, _mm_mul_pd (b, c)); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE dupeven (__m128d a) noexcept                            { return _mm_shuffle_pd (a, a, _MM_SHUFFLE2 (0, 0)); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE dupodd (__m128d a) noexcept                             { return _mm_shuffle_pd (a, a, _MM_SHUFFLE2 (1, 1)); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE swapevenodd (__m128d a) noexcept                        { return _mm_shuffle_pd (a, a, _MM_SHUFFLE2 (0, 1)); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE oddevensum (__m128d a) noexcept                         { return a; }
    static forcedinline f64  DRX_VECTOR_CALLTYPE get (__m128d v, size_t i) noexcept                      { return SIMDFallbackOps<f64, __m128d>::get (v, i); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE set (__m128d v, size_t i, f64 s) noexcept            { return SIMDFallbackOps<f64, __m128d>::set (v, i, s); }
    static forcedinline __m128d DRX_VECTOR_CALLTYPE truncate (__m128d a) noexcept                           { return _mm_cvtepi32_pd (_mm_cvttpd_epi32 (a)); }

    //==============================================================================
    static forcedinline __m128d DRX_VECTOR_CALLTYPE cmplxmul (__m128d a, __m128d b) noexcept
    {
        __m128d rr_ir = mul (a, dupeven (b));
        __m128d ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vconst (kEvenHighBit)));
    }

    static forcedinline f64 DRX_VECTOR_CALLTYPE sum (__m128d a) noexcept
    {
       #if defined (__SSE4__)
        __m128d retval = _mm_dp_pd (a, vconst (kOne), 0xff);
       #elif defined (__SSE3__)
        __m128d retval = _mm_hadd_pd (a, a);
       #else
        __m128d retval = _mm_add_pd (_mm_shuffle_pd (a, a, 0x01), a);
       #endif
        return _mm_cvtsd_f64 (retval);
    }
};

//==============================================================================
/** Signed 8-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<i8>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (i8, kAllBitsSet);

    static forcedinline __m128i DRX_VECTOR_CALLTYPE vconst (const i8* a) noexcept                       { return load (a); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE load (const i8* a) noexcept                         { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0    DRX_VECTOR_CALLTYPE store (__m128i v, i8* p) noexcept                   { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE expand (i8 s) noexcept                              { return _mm_set1_epi8 (s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
   #if defined (__SSE4__)
    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epi8 (a, b); }
   #else
    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
   #endif
    static forcedinline __m128i DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline i8  DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<i8, __m128i>::get (v, i); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, i8 s) noexcept            { return SIMDFallbackOps<i8, __m128i>::set (v, i, s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline i8 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i lo = _mm_unpacklo_epi8 (a, _mm_setzero_si128());
        __m128i hi = _mm_unpackhi_epi8 (a, _mm_setzero_si128());

        for (i32 i = 0; i < 3; ++i)
        {
            lo = _mm_hadd_epi16 (lo, lo);
            hi = _mm_hadd_epi16 (hi, hi);
        }

        return static_cast<i8> ((_mm_cvtsi128_si32 (lo) & 0xff) + (_mm_cvtsi128_si32 (hi) & 0xff));
       #else
        return SIMDFallbackOps<i8, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b)
    {
        // unpack and multiply
        __m128i even = _mm_mullo_epi16 (a, b);
        __m128i odd  = _mm_mullo_epi16 (_mm_srli_epi16 (a, 8), _mm_srli_epi16 (b, 8));

        return _mm_or_si128 (_mm_slli_epi16 (odd, 8),
                             _mm_srli_epi16 (_mm_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Unsigned 8-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<u8>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (u8, kHighBit);
    DECLARE_SSE_SIMD_CONST (u8, kAllBitsSet);

    static forcedinline __m128i DRX_VECTOR_CALLTYPE vconst (u8k* a) noexcept                      { return load (a); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE load (u8k* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m128i v, u8* p) noexcept                     { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE expand (u8 s) noexcept                             { return _mm_set1_epi8 ((i8) s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epu8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epu8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi8 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi8 (ssign (a), ssign (b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline u8 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<u8, __m128i>::get (v, i); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, u8 s) noexcept           { return SIMDFallbackOps<u8, __m128i>::set (v, i, s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline u8 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i lo = _mm_unpacklo_epi8 (a, _mm_setzero_si128());
        __m128i hi = _mm_unpackhi_epi8 (a, _mm_setzero_si128());

        for (i32 i = 0; i < 3; ++i)
        {
            lo = _mm_hadd_epi16 (lo, lo);
            hi = _mm_hadd_epi16 (hi, hi);
        }

        return static_cast<u8> ((static_cast<u32> (_mm_cvtsi128_si32 (lo)) & 0xffu)
                                   + (static_cast<u32> (_mm_cvtsi128_si32 (hi)) & 0xffu));
       #else
        return SIMDFallbackOps<u8, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b)
    {
        // unpack and multiply
        __m128i even = _mm_mullo_epi16 (a, b);
        __m128i odd  = _mm_mullo_epi16 (_mm_srli_epi16 (a, 8), _mm_srli_epi16 (b, 8));

        return _mm_or_si128 (_mm_slli_epi16 (odd, 8),
                             _mm_srli_epi16 (_mm_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Signed 16-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<i16>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (i16, kAllBitsSet);

    //==============================================================================
    static forcedinline __m128i DRX_VECTOR_CALLTYPE vconst (i16k* a) noexcept                      { return load (a); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE load (i16k* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0    DRX_VECTOR_CALLTYPE store (__m128i v, i16* p) noexcept                  { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE expand (i16 s) noexcept                             { return _mm_set1_epi16 (s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return _mm_mullo_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi16 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline i16 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<i16, __m128i>::get (v, i); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, i16 s) noexcept           { return SIMDFallbackOps<i16, __m128i>::set (v, i, s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline i16 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi16 (a, a);
        tmp = _mm_hadd_epi16 (tmp, tmp);
        tmp = _mm_hadd_epi16 (tmp, tmp);

        return static_cast<i16> (_mm_cvtsi128_si32 (tmp) & 0xffff);
       #else
        return SIMDFallbackOps<i16, __m128i>::sum (a);
       #endif
    }
};

//==============================================================================
/** Unsigned 16-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<u16>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (u16, kHighBit);
    DECLARE_SSE_SIMD_CONST (u16, kAllBitsSet);

    //==============================================================================
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE vconst (u16k* a) noexcept                     { return load (a); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE load (u16k* a) noexcept                       { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0     DRX_VECTOR_CALLTYPE store (__m128i v, u16* p) noexcept                 { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE expand (u16 s) noexcept                            { return _mm_set1_epi16 ((i16) s); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi16 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi16 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return _mm_mullo_epi16 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
   #if defined (__SSE4__)
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epu16 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epu16 (a, b); }
   #else
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
   #endif
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi16 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi16 (ssign (a), ssign (b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8     DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline u16 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<u16, __m128i>::get (v, i); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, u16 s) noexcept          { return SIMDFallbackOps<u16, __m128i>::set (v, i, s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline u16 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi16 (a, a);
        tmp = _mm_hadd_epi16 (tmp, tmp);
        tmp = _mm_hadd_epi16 (tmp, tmp);

        return static_cast<u16> (static_cast<u32> (_mm_cvtsi128_si32 (tmp)) & 0xffffu);
       #else
        return SIMDFallbackOps<u16, __m128i>::sum (a);
       #endif
    }
};

//==============================================================================
/** Signed 32-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<i32>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (i32, kAllBitsSet);

    //==============================================================================
    static forcedinline __m128i DRX_VECTOR_CALLTYPE vconst (const i32* a) noexcept                      { return load (a); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE load (const i32* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0    DRX_VECTOR_CALLTYPE store (__m128i v, i32* p) noexcept                  { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE expand (i32 s) noexcept                             { return _mm_set1_epi32 (s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi32 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi32 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi32 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi32 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline i32 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<i32, __m128i>::get (v, i); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, i32 s) noexcept           { return SIMDFallbackOps<i32, __m128i>::set (v, i, s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline i32 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi32 (a, a);
        return _mm_cvtsi128_si32 (_mm_hadd_epi32 (tmp, tmp));
       #else
        return SIMDFallbackOps<i32, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_mullo_epi32 (a, b);
       #else
        __m128i even = _mm_mul_epu32 (a,b);
        __m128i odd = _mm_mul_epu32 (_mm_srli_si128 (a,4), _mm_srli_si128 (b,4));
        return _mm_unpacklo_epi32 (_mm_shuffle_epi32 (even, _MM_SHUFFLE (0,0,2,0)),
                                   _mm_shuffle_epi32 (odd,  _MM_SHUFFLE (0,0,2,0)));
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_min_epi32 (a, b);
       #else
        __m128i lt = greaterThan (b, a);
        return bit_or (bit_and (lt, a), bit_andnot (lt, b));
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_max_epi32 (a, b);
       #else
        __m128i gt = greaterThan (a, b);
        return bit_or (bit_and (gt, a), bit_andnot (gt, b));
       #endif
    }
};

//==============================================================================
/** Unsigned 32-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<u32>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (u32, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (u32, kHighBit);

    //==============================================================================
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE vconst (u32k* a) noexcept                     { return load (a); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE load (u32k* a) noexcept                       { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0     DRX_VECTOR_CALLTYPE store (__m128i v, u32* p) noexcept                 { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE expand (u32 s) noexcept                            { return _mm_set1_epi32 ((i32) s); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi32 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi32 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi32 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi32 (ssign (a), ssign (b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8     DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline u32 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<u32, __m128i>::get (v, i); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, u32 s) noexcept          { return SIMDFallbackOps<u32, __m128i>::set (v, i, s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline u32 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi32 (a, a);
        return static_cast<u32> (_mm_cvtsi128_si32 (_mm_hadd_epi32 (tmp, tmp)));
       #else
        return SIMDFallbackOps<u32, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_mullo_epi32 (a, b);
       #else
        __m128i even = _mm_mul_epu32 (a,b);
        __m128i odd = _mm_mul_epu32 (_mm_srli_si128 (a,4), _mm_srli_si128 (b,4));
        return _mm_unpacklo_epi32 (_mm_shuffle_epi32 (even, _MM_SHUFFLE (0,0,2,0)),
                                   _mm_shuffle_epi32 (odd,  _MM_SHUFFLE (0,0,2,0)));
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_min_epi32 (a, b);
       #else
        __m128i lt = greaterThan (b, a);
        return bit_or (bit_and (lt, a), bit_andnot (lt, b));
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_max_epi32 (a, b);
       #else
        __m128i gt = greaterThan (a, b);
        return bit_or (bit_and (gt, a), bit_andnot (gt, b));
       #endif
    }
};

//==============================================================================
/** Signed 64-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<z64>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (z64, kAllBitsSet);

    static forcedinline __m128i DRX_VECTOR_CALLTYPE vconst (const z64* a) noexcept                      { return load (a); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE expand (z64 s) noexcept                             { return _mm_set1_epi64x (s); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE load (const z64* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0    DRX_VECTOR_CALLTYPE store (__m128i v, z64* p) noexcept                  { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi64 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi64 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline z64 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<z64, __m128i>::get (v, i); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, z64 s) noexcept           { return SIMDFallbackOps<z64, __m128i>::set (v, i, s); }
    static forcedinline z64 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept                                { return SIMDFallbackOps<z64, __m128i>::sum (a); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return SIMDFallbackOps<z64, __m128i>::mul (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_cmpeq_epi64 (a, b);
       #else
        __m128i bitmask = _mm_cmpeq_epi32 (a, b);
        bitmask = _mm_and_si128 (bitmask, _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 3, 0, 1)));
        return _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 2, 0, 0));
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_2__)
        return _mm_cmpgt_epi64 (a, b);
       #else
        return SIMDFallbackOps<z64, __m128i>::greaterThan (a, b);
       #endif
    }
};

//==============================================================================
/** Unsigned 64-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<zu64>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (zu64, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (zu64, kHighBit);

    static forcedinline __m128i  DRX_VECTOR_CALLTYPE vconst (const zu64* a) noexcept                     { return load (a); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE expand (zu64 s) noexcept                            { return _mm_set1_epi64x ((z64) s); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE load (const zu64* a) noexcept                       { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline z0     DRX_VECTOR_CALLTYPE store (__m128i v, zu64* p) noexcept                 { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi64 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi64 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8     DRX_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline zu64 DRX_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<zu64, __m128i>::get (v, i); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE set (__m128i v, size_t i, zu64 s) noexcept          { return SIMDFallbackOps<zu64, __m128i>::set (v, i, s); }
    static forcedinline zu64 DRX_VECTOR_CALLTYPE sum (__m128i a) noexcept                                { return SIMDFallbackOps<zu64, __m128i>::sum (a); }
    static forcedinline __m128i  DRX_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return SIMDFallbackOps<zu64, __m128i>::mul (a, b); }
    static forcedinline __m128i DRX_VECTOR_CALLTYPE truncate (__m128i a) noexcept                            { return a; }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_1__)
        return _mm_cmpeq_epi64 (a, b);
       #else
        __m128i bitmask = _mm_cmpeq_epi32 (a, b);
        bitmask = _mm_and_si128 (bitmask, _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 3, 0, 1)));
        return _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 2, 0, 0));
       #endif
    }

    static forcedinline __m128i DRX_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept
    {
       #if defined (__SSE4_2__)
        return _mm_cmpgt_epi64 (ssign (a), ssign (b));
       #else
        return SIMDFallbackOps<zu64, __m128i>::greaterThan (a, b);
       #endif
    }
};

#endif

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx::dsp
