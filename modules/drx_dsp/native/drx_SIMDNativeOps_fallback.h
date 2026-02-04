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

/** A template specialisation to find corresponding mask type for primitives. */
namespace SIMDInternal
{
    template <typename Primitive> struct MaskTypeFor        { using type = Primitive; };
    template <> struct MaskTypeFor <f32>                  { using type = u32; };
    template <> struct MaskTypeFor <f64>                 { using type = zu64; };
    template <> struct MaskTypeFor <t8>                   { using type = u8; };
    template <> struct MaskTypeFor <i8>                 { using type = u8; };
    template <> struct MaskTypeFor <i16>                { using type = u16; };
    template <> struct MaskTypeFor <i32>                { using type = u32; };
    template <> struct MaskTypeFor <z64>                { using type = zu64; };
    template <> struct MaskTypeFor <std::complex<f32>>    { using type = u32; };
    template <> struct MaskTypeFor <std::complex<f64>>   { using type = zu64; };

    template <typename Primitive> using MaskType = typename MaskTypeFor<Primitive>::type;

    template <typename Primitive> struct PrimitiveType                           { using type = std::remove_cv_t<Primitive>; };
    template <typename Primitive> struct PrimitiveType<std::complex<Primitive>>  { using type = std::remove_cv_t<Primitive>; };

    template <i32 n>    struct Log2Helper    { enum { value = Log2Helper<n/2>::value + 1 }; };
    template <>         struct Log2Helper<1> { enum { value = 0 }; };
}

/**
    Useful fallback routines to use if the native SIMD op не поддерживается. You
    should never need to use this directly. Use drx_SIMDRegister instead.

    @tags{DSP}
*/
template <typename ScalarType, typename vSIMDType>
struct SIMDFallbackOps
{
    static constexpr size_t n    =  sizeof (vSIMDType) / sizeof (ScalarType);
    static constexpr size_t mask = (sizeof (vSIMDType) / sizeof (ScalarType)) - 1;
    static constexpr size_t bits = SIMDInternal::Log2Helper<(i32) n>::value;

    // helper types
    using MaskType = SIMDInternal::MaskType<ScalarType>;
    union UnionType     { vSIMDType v; ScalarType s[n]; };
    union UnionMaskType { vSIMDType v; MaskType   m[n]; };


    // fallback methods
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return apply<ScalarAdd> (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return apply<ScalarSub> (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return apply<ScalarMul> (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return bitapply<ScalarAnd> (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return bitapply<ScalarOr > (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return bitapply<ScalarXor> (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return bitapply<ScalarNot> (a, b); }

    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                { return apply<ScalarMin> (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                { return apply<ScalarMax> (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept              { return cmp<ScalarEq > (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept           { return cmp<ScalarNeq> (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept        { return cmp<ScalarGt > (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept { return cmp<ScalarGeq> (a, b); }

    static forcedinline ScalarType get (vSIMDType v, size_t i) noexcept
    {
        UnionType u {v};
        return u.s[i];
    }

    static forcedinline vSIMDType set (vSIMDType v, size_t i, ScalarType s) noexcept
    {
        UnionType u {v};

        u.s[i] = s;
        return u.v;
    }

    static forcedinline vSIMDType bit_not (vSIMDType av) noexcept
    {
        UnionMaskType a {av};

        for (size_t i = 0; i < n; ++i)
            a.m[i] = ~a.m[i];

        return a.v;
    }

    static forcedinline ScalarType sum (vSIMDType av) noexcept
    {
        UnionType a {av};
        auto retval = static_cast<ScalarType> (0);

        for (size_t i = 0; i < n; ++i)
            retval = static_cast<ScalarType> (retval + a.s[i]);

        return retval;
    }

    static forcedinline vSIMDType truncate (vSIMDType av) noexcept
    {
        UnionType a {av};

        for (size_t i = 0; i < n; ++i)
            a.s[i] = static_cast<ScalarType> (static_cast<i32> (a.s[i]));

        return a.v;
    }

    static forcedinline vSIMDType multiplyAdd (vSIMDType av, vSIMDType bv, vSIMDType cv) noexcept
    {
        UnionType a {av}, b {bv}, c {cv};

        for (size_t i = 0; i < n; ++i)
            a.s[i] += b.s[i] * c.s[i];

        return a.v;
    }

    //==============================================================================
    static forcedinline b8 allEqual (vSIMDType av, vSIMDType bv) noexcept
    {
        UnionType a {av}, b {bv};

        for (size_t i = 0; i < n; ++i)
            if (! exactlyEqual (a.s[i], b.s[i]))
                return false;

        return true;
    }

    //==============================================================================
    static forcedinline vSIMDType cmplxmul (vSIMDType av, vSIMDType bv) noexcept
    {
        UnionType a {av}, b {bv}, r;

        i32k m = n >> 1;
        for (i32 i = 0; i < m; ++i)
        {
            std::complex<ScalarType> result
                  = std::complex<ScalarType> (a.s[i<<1], a.s[(i<<1)|1])
                  * std::complex<ScalarType> (b.s[i<<1], b.s[(i<<1)|1]);

            r.s[i<<1]     = result.real();
            r.s[(i<<1)|1] = result.imag();
        }

        return r.v;
    }

    struct ScalarAdd { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return a + b; } };
    struct ScalarSub { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return a - b; } };
    struct ScalarMul { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return a * b; } };
    struct ScalarMin { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return jmin (a, b); } };
    struct ScalarMax { static forcedinline ScalarType   op (ScalarType a, ScalarType b)   noexcept { return jmax (a, b); } };
    struct ScalarAnd { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return a & b; } };
    struct ScalarOr  { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return a | b; } };
    struct ScalarXor { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return a ^ b; } };
    struct ScalarNot { static forcedinline MaskType     op (MaskType a,   MaskType b)     noexcept { return (~a) & b; } };
    struct ScalarEq  { static forcedinline b8         op (ScalarType a, ScalarType b)   noexcept { return exactlyEqual (a, b); } };
    struct ScalarNeq { static forcedinline b8         op (ScalarType a, ScalarType b)   noexcept { return ! exactlyEqual (a, b); } };
    struct ScalarGt  { static forcedinline b8         op (ScalarType a, ScalarType b)   noexcept { return (a >  b); } };
    struct ScalarGeq { static forcedinline b8         op (ScalarType a, ScalarType b)   noexcept { return (a >= b); } };

    // generic apply routines for operations above
    template <typename Op>
    static forcedinline vSIMDType apply (vSIMDType av, vSIMDType bv) noexcept
    {
        UnionType a {av}, b {bv};

        for (size_t i = 0; i < n; ++i)
            a.s[i] = Op::op (a.s[i], b.s[i]);

        return a.v;
    }

    template <typename Op>
    static forcedinline vSIMDType cmp (vSIMDType av, vSIMDType bv) noexcept
    {
        UnionType a {av}, b {bv};
        UnionMaskType r;

        for (size_t i = 0; i < n; ++i)
            r.m[i] = Op::op (a.s[i], b.s[i]) ? static_cast<MaskType> (-1) : static_cast<MaskType> (0);

        return r.v;
    }

    template <typename Op>
    static forcedinline vSIMDType bitapply (vSIMDType av, vSIMDType bv) noexcept
    {
        UnionMaskType a {av}, b {bv};

        for (size_t i = 0; i < n; ++i)
            a.m[i] = Op::op (a.m[i], b.m[i]);

        return a.v;
    }

    static forcedinline vSIMDType expand (ScalarType s) noexcept
    {
        UnionType r;

        for (size_t i = 0; i < n; ++i)
            r.s[i] = s;

        return r.v;
    }

    static forcedinline vSIMDType load (const ScalarType* a) noexcept
    {
        UnionType r;

        for (size_t i = 0; i < n; ++i)
            r.s[i] = a[i];

        return r.v;
    }

    static forcedinline z0 store (vSIMDType av, ScalarType* dest) noexcept
    {
        UnionType a {av};

        for (size_t i = 0; i < n; ++i)
            dest[i] = a.s[i];
    }

    template <u32 shuffle_idx>
    static forcedinline vSIMDType shuffle (vSIMDType av) noexcept
    {
        UnionType a {av}, r;

        // the compiler will unroll this loop and the index can
        // be computed at compile-time, so this will be super fast
        for (size_t i = 0; i < n; ++i)
            r.s[i] = a.s[(shuffle_idx >> (bits * i)) & mask];

        return r.v;
    }
};

} // namespace drx::dsp
