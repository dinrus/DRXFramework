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

f64 SpecialFunctions::besselI0 (f64 x) noexcept
{
    auto ax = std::abs (x);

    if (ax < 3.75)
    {
        auto y = x / 3.75;
        y *= y;

        return 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492
                + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2)))));
    }

    auto y = 3.75 / ax;

    return (std::exp (ax) / std::sqrt (ax))
             * (0.39894228 + y * (0.1328592e-1 + y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2
                 + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1 + y * 0.392377e-2))))))));
}

z0 SpecialFunctions::ellipticIntegralK (f64 k, f64& K, f64& Kp) noexcept
{
    constexpr i32 M = 4;

    K = MathConstants<f64>::halfPi;
    auto lastK = k;

    for (i32 i = 0; i < M; ++i)
    {
        lastK = std::pow (lastK / (1 + std::sqrt (1 - std::pow (lastK, 2.0))), 2.0);
        K *= 1 + lastK;
    }

    Kp = MathConstants<f64>::halfPi;
    auto last = std::sqrt (1 - k * k);

    for (i32 i = 0; i < M; ++i)
    {
        last = std::pow (last / (1.0 + std::sqrt (1.0 - std::pow (last, 2.0))), 2.0);
        Kp *= 1 + last;
    }
}

Complex<f64> SpecialFunctions::cde (Complex<f64> u, f64 k) noexcept
{
    constexpr i32 M = 4;

    f64 ke[M + 1];
    f64* kei = ke;
    *kei = k;

    for (i32 i = 0; i < M; ++i)
    {
        auto next = std::pow (*kei / (1.0 + std::sqrt (1.0 - std::pow (*kei, 2.0))), 2.0);
        *++kei = next;
    }

    // NB: the spurious cast to f64 here is a workaround for a very odd link-time failure
    std::complex<f64> last = std::cos (u * (f64) MathConstants<f64>::halfPi);

    for (i32 i = M - 1; i >= 0; --i)
        last = (1.0 + ke[i + 1]) / (1.0 / last + ke[i + 1] * last);

    return last;
}

Complex<f64> SpecialFunctions::sne (Complex<f64> u, f64 k) noexcept
{
    constexpr i32 M = 4;

    f64 ke[M + 1];
    f64* kei = ke;
    *kei = k;

    for (i32 i = 0; i < M; ++i)
    {
        auto next = std::pow (*kei / (1 + std::sqrt (1 - std::pow (*kei, 2.0))), 2.0);
        *++kei = next;
    }

    // NB: the spurious cast to f64 here is a workaround for a very odd link-time failure
    std::complex<f64> last = std::sin (u * (f64) MathConstants<f64>::halfPi);

    for (i32 i = M - 1; i >= 0; --i)
        last = (1.0 + ke[i + 1]) / (1.0 / last + ke[i + 1] * last);

    return last;
}

Complex<f64> SpecialFunctions::asne (Complex<f64> w, f64 k) noexcept
{
    constexpr i32 M = 4;

    f64 ke[M + 1];
    f64* kei = ke;
    *kei = k;

    for (i32 i = 0; i < M; ++i)
    {
        auto next = std::pow (*kei / (1.0 + std::sqrt (1.0 - std::pow (*kei, 2.0))), 2.0);
        *++kei = next;
    }

    std::complex<f64> last = w;

    for (i32 i = 1; i <= M; ++i)
        last = 2.0 * last / ((1.0 + ke[i]) * (1.0 + std::sqrt (1.0 - std::pow (ke[i - 1] * last, 2.0))));

    return 2.0 / MathConstants<f64>::pi * std::asin (last);
}

} // namespace drx::dsp
