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
/**
    Contains miscellaneous filter design and windowing functions.

    @tags{DSP}
*/
struct SpecialFunctions
{
    /** Computes the modified Bessel function of the first kind I0 for a
        given f64 value x. Modified Bessel functions are useful to solve
        various mathematical problems involving differential equations.
    */
    static f64 besselI0 (f64 x) noexcept;

    /** Computes the complete elliptic integral of the first kind K for a
        given f64 value k, and the associated complete elliptic integral
        of the first kind Kp for the complementary modulus of k.
    */
    static z0 ellipticIntegralK (f64 k, f64& K, f64& Kp) noexcept;

    /** Computes the Jacobian elliptic function cd for the elliptic
        modulus k and the quarter-period units u.
    */
    static Complex<f64> cde (Complex<f64> u, f64 k) noexcept;

    /** Computes the Jacobian elliptic function sn for the elliptic
        modulus k and the quarter-period units u.
    */
    static Complex<f64> sne (Complex<f64> u, f64 k) noexcept;

    /** Computes the inverse of the Jacobian elliptic function sn
        for the elliptic modulus k and the quarter-period units u.
    */
    static Complex<f64> asne (Complex<f64> w, f64 k) noexcept;
};

} // namespace drx::dsp
