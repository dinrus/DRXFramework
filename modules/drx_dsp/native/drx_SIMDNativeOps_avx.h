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
 #define DECLARE_AVX_SIMD_CONST(type, name) \
    static __declspec (align (32)) const type name[32 / sizeof (type)]

 #define DEFINE_AVX_SIMD_CONST(type, class_type, name) \
    __declspec (align (32)) const type SIMDNativeOps<class_type>:: name[32 / sizeof (type)]

#else
 #define DECLARE_AVX_SIMD_CONST(type, name) \
    static const type name[32 / sizeof (type)] __attribute__ ((aligned (32)))

 #define DEFINE_AVX_SIMD_CONST(type, class_type, name) \
    const type SIMDNativeOps<class_type>:: name[32 / sizeof (type)] __attribute__ ((aligned (32)))

#endif

template <typename type>
struct SIMDNativeOps;

//==============================================================================
/** Single-precision floating point AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<f32>
{
    using vSIMDType = __m256;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (i32, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (i32, kEvenHighBit);
    DECLARE_AVX_SIMD_CONST (f32, kOne);

    //==============================================================================
    static forcedinline __m256 DRX_VECTOR_CALLTYPE vconst (const f32* a) noexcept                     { return load (a); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE vconst (const i32* a) noexcept                   { return _mm256_castsi256_ps (_mm256_load_si256 (reinterpret_cast <const __m256i*> (a))); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE expand (f32 s) noexcept                            { return _mm256_broadcast_ss (&s); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE load (const f32* a) noexcept                       { return _mm256_load_ps (a); }
    static forcedinline z0   DRX_VECTOR_CALLTYPE store (__m256 value, f32* dest) noexcept           { _mm256_store_ps (dest, value); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE add (__m256 a, __m256 b) noexcept                    { return _mm256_add_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE sub (__m256 a, __m256 b) noexcept                    { return _mm256_sub_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE mul (__m256 a, __m256 b) noexcept                    { return _mm256_mul_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE bit_and (__m256 a, __m256 b) noexcept                { return _mm256_and_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE bit_or  (__m256 a, __m256 b) noexcept                { return _mm256_or_ps  (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE bit_xor (__m256 a, __m256 b) noexcept                { return _mm256_xor_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE bit_notand (__m256 a, __m256 b) noexcept             { return _mm256_andnot_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE bit_not (__m256 a) noexcept                          { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE min (__m256 a, __m256 b) noexcept                    { return _mm256_min_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE max (__m256 a, __m256 b) noexcept                    { return _mm256_max_ps (a, b); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE equal (__m256 a, __m256 b) noexcept                  { return _mm256_cmp_ps (a, b, _CMP_EQ_OQ); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE notEqual (__m256 a, __m256 b) noexcept               { return _mm256_cmp_ps (a, b, _CMP_NEQ_OQ); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE greaterThan (__m256 a, __m256 b) noexcept            { return _mm256_cmp_ps (a, b, _CMP_GT_OQ); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256 a, __m256 b) noexcept     { return _mm256_cmp_ps (a, b, _CMP_GE_OQ); }
    static forcedinline b8   DRX_VECTOR_CALLTYPE allEqual (__m256 a, __m256 b) noexcept               { return (_mm256_movemask_ps (equal (a, b)) == 0xff); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE dupeven (__m256 a) noexcept                          { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (2, 2, 0, 0)); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE dupodd (__m256 a) noexcept                           { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (3, 3, 1, 1)); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE swapevenodd (__m256 a) noexcept                      { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (2, 3, 0, 1)); }
    static forcedinline f32  DRX_VECTOR_CALLTYPE get (__m256 v, size_t i) noexcept                    { return SIMDFallbackOps<f32, __m256>::get (v, i); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE set (__m256 v, size_t i, f32 s) noexcept           { return SIMDFallbackOps<f32, __m256>::set (v, i, s); }
    static forcedinline __m256 DRX_VECTOR_CALLTYPE truncate (__m256 a) noexcept                         { return _mm256_cvtepi32_ps (_mm256_cvttps_epi32 (a)); }

    static forcedinline __m256 DRX_VECTOR_CALLTYPE multiplyAdd (__m256 a, __m256 b, __m256 c) noexcept
    {
       #if __FMA__
        return _mm256_fmadd_ps (b, c, a);
       #else
        return add (a, mul (b, c));
       #endif
    }

    static forcedinline __m256 DRX_VECTOR_CALLTYPE oddevensum (__m256 a) noexcept
    {
        a = _mm256_add_ps (_mm256_shuffle_ps (a, a, _MM_SHUFFLE (1, 0, 3, 2)), a);
        return add (_mm256_permute2f128_ps (a, a, 1), a);
    }

    //==============================================================================
    static forcedinline __m256 DRX_VECTOR_CALLTYPE cmplxmul (__m256 a, __m256 b) noexcept
    {
        __m256 rr_ir = mul (a, dupeven (b));
        __m256 ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vconst (kEvenHighBit)));
    }

    static forcedinline f32 DRX_VECTOR_CALLTYPE sum (__m256 a) noexcept
    {
       __m256 retval = _mm256_dp_ps (a, vconst (kOne), 0xff);
       __m256 tmp = _mm256_permute2f128_ps (retval, retval, 1);
       retval = _mm256_add_ps (retval, tmp);

      #if DRX_GCC
       return retval[0];
      #else
       return _mm256_cvtss_f32 (retval);
      #endif
    }
};

//==============================================================================
/** Double-precision floating point AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<f64>
{
    using vSIMDType = __m256d;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (z64, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (z64, kEvenHighBit);
    DECLARE_AVX_SIMD_CONST (f64, kOne);

    //==============================================================================
    static forcedinline __m256d DRX_VECTOR_CALLTYPE vconst (const f64* a) noexcept                      { return load (a); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE vconst (const z64* a) noexcept                     { return _mm256_castsi256_pd (_mm256_load_si256 (reinterpret_cast <const __m256i*> (a))); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE expand (f64 s) noexcept                             { return _mm256_broadcast_sd (&s); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE load (const f64* a) noexcept                        { return _mm256_load_pd (a); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m256d value, f64* dest) noexcept              { _mm256_store_pd (dest, value); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE add (__m256d a, __m256d b) noexcept                    { return _mm256_add_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE sub (__m256d a, __m256d b) noexcept                    { return _mm256_sub_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE mul (__m256d a, __m256d b) noexcept                    { return _mm256_mul_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE bit_and (__m256d a, __m256d b) noexcept                { return _mm256_and_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE bit_or  (__m256d a, __m256d b) noexcept                { return _mm256_or_pd  (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE bit_xor (__m256d a, __m256d b) noexcept                { return _mm256_xor_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE bit_notand (__m256d a, __m256d b) noexcept             { return _mm256_andnot_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE bit_not (__m256d a) noexcept                           { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE min (__m256d a, __m256d b) noexcept                    { return _mm256_min_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE max (__m256d a, __m256d b) noexcept                    { return _mm256_max_pd (a, b); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE equal (__m256d a, __m256d b) noexcept                  { return _mm256_cmp_pd (a, b, _CMP_EQ_OQ); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE notEqual (__m256d a, __m256d b) noexcept               { return _mm256_cmp_pd (a, b, _CMP_NEQ_OQ); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE greaterThan (__m256d a, __m256d b) noexcept            { return _mm256_cmp_pd (a, b, _CMP_GT_OQ); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256d a, __m256d b) noexcept     { return _mm256_cmp_pd (a, b, _CMP_GE_OQ); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m256d a, __m256d b) noexcept               { return (_mm256_movemask_pd (equal (a, b)) == 0xf); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE multiplyAdd (__m256d a, __m256d b, __m256d c) noexcept { return _mm256_add_pd (a, _mm256_mul_pd (b, c)); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE dupeven (__m256d a) noexcept                           { return _mm256_shuffle_pd (a, a, 0); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE dupodd (__m256d a) noexcept                            { return _mm256_shuffle_pd (a, a, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE swapevenodd (__m256d a) noexcept                       { return _mm256_shuffle_pd (a, a, (1 << 0) | (0 << 1) | (1 << 2) | (0 << 3)); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE oddevensum (__m256d a) noexcept                        { return _mm256_add_pd (_mm256_permute2f128_pd (a, a, 1), a); }
    static forcedinline f64  DRX_VECTOR_CALLTYPE get (__m256d v, size_t i) noexcept                     { return SIMDFallbackOps<f64, __m256d>::get (v, i); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE set (__m256d v, size_t i, f64 s) noexcept           { return SIMDFallbackOps<f64, __m256d>::set (v, i, s); }
    static forcedinline __m256d DRX_VECTOR_CALLTYPE truncate (__m256d a) noexcept                          { return _mm256_cvtepi32_pd (_mm256_cvttpd_epi32 (a)); }

    //==============================================================================
    static forcedinline __m256d DRX_VECTOR_CALLTYPE cmplxmul (__m256d a, __m256d b) noexcept
    {
        __m256d rr_ir = mul (a, dupeven (b));
        __m256d ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vconst (kEvenHighBit)));
    }

    static forcedinline f64 DRX_VECTOR_CALLTYPE sum (__m256d a) noexcept
    {
        __m256d retval = _mm256_hadd_pd (a, a);
        __m256d tmp = _mm256_permute2f128_pd (retval, retval, 1);
        retval = _mm256_add_pd (retval, tmp);

       #if DRX_GCC
        return retval[0];
       #else
        return _mm256_cvtsd_f64 (retval);
       #endif
    }
};

//==============================================================================
/** Signed 8-bit integer AVX intrinsics

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<i8>
{
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (i8, kAllBitsSet);

    static forcedinline __m256i DRX_VECTOR_CALLTYPE expand (i8 s) noexcept                             { return _mm256_set1_epi8 (s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE load (const i8* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m256i value, i8* dest) noexcept              { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                    { return _mm256_add_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                    { return _mm256_sub_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept             { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                           { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                    { return _mm256_min_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                    { return _mm256_max_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                  { return _mm256_cmpeq_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept            { return _mm256_cmpgt_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept     { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept               { return _mm256_movemask_epi8 (equal (a, b)) == -1; }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept { return add (a, mul (b, c)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline i8  DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                     { return SIMDFallbackOps<i8, __m256i>::get (v, i); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, i8 s) noexcept           { return SIMDFallbackOps<i8, __m256i>::set (v, i, s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                          { return a; }

    //==============================================================================
    static forcedinline i8 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i lo = _mm256_unpacklo_epi8 (a, _mm256_setzero_si256());
        __m256i hi = _mm256_unpackhi_epi8 (a, _mm256_setzero_si256());

        for (i32 i = 0; i < 3; ++i)
        {
            lo = _mm256_hadd_epi16 (lo, lo);
            hi = _mm256_hadd_epi16 (hi, hi);
        }

       #if DRX_GCC
        return (i8) ((lo[0] & 0xff) +
                         (hi[0] & 0xff) +
                         (lo[2] & 0xff) +
                         (hi[2] & 0xff));
       #else
        constexpr i32 mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (i8) ((_mm256_cvtsi256_si32 (lo) & 0xff) +
                         (_mm256_cvtsi256_si32 (hi) & 0xff) +
                         (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (lo, mask)) & 0xff) +
                         (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (hi, mask)) & 0xff));
       #endif
    }

    static forcedinline __m256i DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b)
    {
        // unpack and multiply
        __m256i even = _mm256_mullo_epi16 (a, b);
        __m256i odd  = _mm256_mullo_epi16 (_mm256_srli_epi16 (a, 8), _mm256_srli_epi16 (b, 8));

        return _mm256_or_si256 (_mm256_slli_epi16 (odd, 8),
                             _mm256_srli_epi16 (_mm256_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Unsigned 8-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<u8>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (u8, kHighBit);
    DECLARE_AVX_SIMD_CONST (u8, kAllBitsSet);

    static forcedinline __m256i DRX_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE expand (u8 s) noexcept                             { return _mm256_set1_epi8 ((i8) s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE load (u8k* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m256i value, u8* dest) noexcept              { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi8 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi8 (ssign (a), ssign (b)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline u8 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<u8, __m256i>::get (v, i); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, u8 s) noexcept           { return SIMDFallbackOps<u8, __m256i>::set (v, i, s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline u8 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i lo = _mm256_unpacklo_epi8 (a, _mm256_setzero_si256());
        __m256i hi = _mm256_unpackhi_epi8 (a, _mm256_setzero_si256());

        for (i32 i = 0; i < 3; ++i)
        {
            lo = _mm256_hadd_epi16 (lo, lo);
            hi = _mm256_hadd_epi16 (hi, hi);
        }

       #if DRX_GCC
        return (u8) ((static_cast<u32> (lo[0]) & 0xffu) +
                          (static_cast<u32> (hi[0]) & 0xffu) +
                          (static_cast<u32> (lo[2]) & 0xffu) +
                          (static_cast<u32> (hi[2]) & 0xffu));
       #else
        constexpr i32 mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (u8) ((static_cast<u32> (_mm256_cvtsi256_si32 (lo)) & 0xffu) +
                          (static_cast<u32> (_mm256_cvtsi256_si32 (hi)) & 0xffu) +
                          (static_cast<u32> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (lo, mask))) & 0xffu) +
                          (static_cast<u32> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (hi, mask))) & 0xffu));
       #endif
    }

    static forcedinline __m256i DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b)
    {
        // unpack and multiply
        __m256i even = _mm256_mullo_epi16 (a, b);
        __m256i odd  = _mm256_mullo_epi16 (_mm256_srli_epi16 (a, 8), _mm256_srli_epi16 (b, 8));

        return _mm256_or_si256 (_mm256_slli_epi16 (odd, 8),
                             _mm256_srli_epi16 (_mm256_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Signed 16-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<i16>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (i16, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i DRX_VECTOR_CALLTYPE expand (i16 s) noexcept                             { return _mm256_set1_epi16 (s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE load (i16k* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0 DRX_VECTOR_CALLTYPE store (__m256i value, i16* dest) noexcept              { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi16 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline i16 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<i16, __m256i>::get (v, i); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, i16 s) noexcept           { return SIMDFallbackOps<i16, __m256i>::set (v, i, s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline i16 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi16 (a, a);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        tmp = _mm256_hadd_epi16 (tmp, tmp);

       #if DRX_GCC
        return (i16) ((tmp[0] & 0xffff) + (tmp[2] & 0xffff));
       #else
        constexpr i32 mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (i16) ((_mm256_cvtsi256_si32 (tmp) & 0xffff) +
                          (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask)) & 0xffff));
       #endif
    }
};

//==============================================================================
/** Unsigned 16-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<u16>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (u16, kHighBit);
    DECLARE_AVX_SIMD_CONST (u16, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE expand (u16 s) noexcept                            { return _mm256_set1_epi16 ((i16) s); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE load (u16k* p) noexcept                       { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0     DRX_VECTOR_CALLTYPE store (__m256i value, u16* dest) noexcept          { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi16 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi16 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi16 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu16 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu16 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi16 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi16 (ssign (a), ssign (b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8     DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline u16 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<u16, __m256i>::get (v, i); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, u16 s) noexcept          { return SIMDFallbackOps<u16, __m256i>::set (v, i, s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline u16 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi16 (a, a);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        tmp = _mm256_hadd_epi16 (tmp, tmp);

       #if DRX_GCC
        return (u16) ((static_cast<u32> (tmp[0]) & 0xffffu) +
                           (static_cast<u32> (tmp[2]) & 0xffffu));
       #else
        constexpr i32 mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (u16) ((static_cast<u32> (_mm256_cvtsi256_si32 (tmp)) & 0xffffu) +
                           (static_cast<u32> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask))) & 0xffffu));
       #endif
    }
};

//==============================================================================
/** Signed 32-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<i32>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (i32, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i DRX_VECTOR_CALLTYPE expand (i32 s) noexcept                             { return _mm256_set1_epi32 (s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE load (const i32* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0    DRX_VECTOR_CALLTYPE store (__m256i value, i32* dest) noexcept           { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi32 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline i32 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<i32, __m256i>::get (v, i); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, i32 s) noexcept           { return SIMDFallbackOps<i32, __m256i>::set (v, i, s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline i32 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi32 (a, a);
        tmp = _mm256_hadd_epi32 (tmp, tmp);

       #if DRX_GCC
        return (i32) (tmp[0] + tmp[2]);
       #else
        constexpr i32 mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return _mm256_cvtsi256_si32 (tmp) + _mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask));
       #endif
    }
};

//==============================================================================
/** Unsigned 32-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<u32>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (u32, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (u32, kHighBit);

    //==============================================================================
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE expand (u32 s) noexcept                            { return _mm256_set1_epi32 ((i32) s); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE load (u32k* p) noexcept                       { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0     DRX_VECTOR_CALLTYPE store (__m256i value, u32* dest) noexcept          { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi32 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi32 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi32 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu32 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu32 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi32 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi32 (ssign (a), ssign (b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8     DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline u32 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<u32, __m256i>::get (v, i); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, u32 s) noexcept          { return SIMDFallbackOps<u32, __m256i>::set (v, i, s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline u32 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi32 (a, a);
        tmp = _mm256_hadd_epi32 (tmp, tmp);

       #if DRX_GCC
        return static_cast<u32> (tmp[0]) + static_cast<u32> (tmp[2]);
       #else
        constexpr i32 mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return static_cast<u32> (_mm256_cvtsi256_si32 (tmp))
            + static_cast<u32> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask)));
       #endif
    }
};

//==============================================================================
/** Signed 64-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<z64>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (z64, kAllBitsSet);

    static forcedinline __m256i DRX_VECTOR_CALLTYPE expand (z64 s) noexcept                             { return _mm256_set1_epi64x ((z64) s); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE load (const z64* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0    DRX_VECTOR_CALLTYPE store (__m256i value, z64* dest) noexcept           { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi64 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi64 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { __m256i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { __m256i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi64 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi64 (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8    DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline z64 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<z64, __m256i>::get (v, i); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, z64 s) noexcept           { return SIMDFallbackOps<z64, __m256i>::set (v, i, s); }
    static forcedinline z64 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept                                { return SIMDFallbackOps<z64, __m256i>::sum (a); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return SIMDFallbackOps<z64, __m256i>::mul (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }
};

//==============================================================================
/** Unsigned 64-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<zu64>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (zu64, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (zu64, kHighBit);

    static forcedinline __m256i  DRX_VECTOR_CALLTYPE expand (zu64 s) noexcept                            { return _mm256_set1_epi64x ((z64) s); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE load (const zu64* p) noexcept                       { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline z0     DRX_VECTOR_CALLTYPE store (__m256i value, zu64* dest) noexcept          { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi64 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi64 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { __m256i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { __m256i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi64 (a, b); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi64 (ssign (a), ssign (b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline b8     DRX_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline zu64 DRX_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<zu64, __m256i>::get (v, i); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE set (__m256i v, size_t i, zu64 s) noexcept          { return SIMDFallbackOps<zu64, __m256i>::set (v, i, s); }
    static forcedinline zu64 DRX_VECTOR_CALLTYPE sum (__m256i a) noexcept                                { return SIMDFallbackOps<zu64, __m256i>::sum (a); }
    static forcedinline __m256i  DRX_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return SIMDFallbackOps<zu64, __m256i>::mul (a, b); }
    static forcedinline __m256i DRX_VECTOR_CALLTYPE truncate (__m256i a) noexcept                            { return a; }
};

#endif

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx::dsp
