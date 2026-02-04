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

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassWindowMethod (FloatType frequency,
                                                           f64 sampleRate, size_t order,
                                                           WindowingMethod type,
                                                           FloatType beta)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);

    auto* result = new typename FIR::Coefficients<FloatType> (order + 1u);

    auto* c = result->getRawCoefficients();
    auto normalisedFrequency = frequency / sampleRate;

    for (size_t i = 0; i <= order; ++i)
    {
        if (i == order / 2)
        {
            c[i] = static_cast<FloatType> (normalisedFrequency * 2);
        }
        else
        {
            auto indice = MathConstants<f64>::pi * (static_cast<f64> (i) - 0.5 * static_cast<f64> (order));
            c[i] = static_cast<FloatType> (std::sin (2.0 * indice * normalisedFrequency) / indice);
        }
    }

    WindowingFunction<FloatType> theWindow (order + 1, type, false, beta);
    theWindow.multiplyWithWindowingTable (c, order + 1);

    return *result;
}

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassKaiserMethod (FloatType frequency, f64 sampleRate,
                                                           FloatType normalisedTransitionWidth,
                                                           FloatType amplitudedB)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalisedTransitionWidth > 0 && normalisedTransitionWidth <= 0.5);
    jassert (amplitudedB >= -100 && amplitudedB <= 0);

    FloatType beta = 0;

    if (amplitudedB < -50)
        beta = static_cast<FloatType> (0.1102 * (-amplitudedB - 8.7));
    else if (amplitudedB <= -21)
        beta = static_cast<FloatType> (0.5842 * std::pow (-amplitudedB - 21, 0.4) + 0.07886 * (-amplitudedB - 21));

    i32 order = amplitudedB < -21 ? roundToInt (std::ceil ((-amplitudedB - 7.95) / (2.285 * normalisedTransitionWidth * MathConstants<f64>::twoPi)))
                                    : roundToInt (std::ceil (5.79 / (normalisedTransitionWidth * MathConstants<f64>::twoPi)));

    jassert (order >= 0);

    return designFIRLowpassWindowMethod (frequency, sampleRate, static_cast<size_t> (order),
                                         WindowingFunction<FloatType>::kaiser, beta);
}


template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassTransitionMethod (FloatType frequency, f64 sampleRate, size_t order,
                                                               FloatType normalisedTransitionWidth, FloatType spline)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalisedTransitionWidth > 0 && normalisedTransitionWidth <= 0.5);
    jassert (spline >= 1.0 && spline <= 4.0);

    auto normalisedFrequency = frequency / static_cast<FloatType> (sampleRate);

    auto* result = new typename FIR::Coefficients<FloatType> (order + 1u);
    auto* c = result->getRawCoefficients();

    for (size_t i = 0; i <= order; ++i)
    {
        if (i == order / 2 && order % 2 == 0)
        {
            c[i] = static_cast<FloatType> (2 * normalisedFrequency);
        }
        else
        {
            auto indice  = MathConstants<f64>::pi * ((f64) i - 0.5 * (f64) order);
            auto indice2 = MathConstants<f64>::pi * normalisedTransitionWidth * ((f64) i - 0.5 * (f64) order) / spline;
            c[i] = static_cast<FloatType> (std::sin (2 * indice * normalisedFrequency)
                                            / indice * std::pow (std::sin (indice2) / indice2, spline));
        }
    }

    return *result;
}

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassLeastSquaresMethod (FloatType frequency,
                                                                 f64 sampleRate, size_t order,
                                                                 FloatType normalisedTransitionWidth,
                                                                 FloatType stopBandWeight)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalisedTransitionWidth > 0 && normalisedTransitionWidth <= 0.5);
    jassert (stopBandWeight >= 1.0 && stopBandWeight <= 100.0);

    auto normalisedFrequency = static_cast<f64> (frequency) / sampleRate;

    auto wp = MathConstants<f64>::twoPi * (static_cast<f64> (normalisedFrequency - normalisedTransitionWidth / 2.0));
    auto ws = MathConstants<f64>::twoPi * (static_cast<f64> (normalisedFrequency + normalisedTransitionWidth / 2.0));

    auto N = order + 1;

    auto* result = new typename FIR::Coefficients<FloatType> (static_cast<size_t> (N));
    auto* c = result->getRawCoefficients();

    auto sinc = [] (f64 x)
    {
        return approximatelyEqual (x, 0.0) ? 1 : std::sin (x * MathConstants<f64>::pi) / (MathConstants<f64>::pi * x);
    };

    if (N % 2 == 1)
    {
        // Type I
        auto M = (N - 1) / 2;

        Matrix<f64> b (M + 1, 1),
                       q (2 * M + 1, 1);

        auto factorp = wp / MathConstants<f64>::pi;
        auto factors = ws / MathConstants<f64>::pi;

        for (size_t i = 0; i <= M; ++i)
            b (i, 0) = factorp * sinc (factorp * (f64) i);

        q (0, 0) = factorp + stopBandWeight * (1.0 - factors);

        for (size_t i = 1; i <= 2 * M; ++i)
            q (i, 0) = factorp * sinc (factorp * (f64) i) - stopBandWeight * factors * sinc (factors * (f64) i);

        auto Q1 = Matrix<f64>::toeplitz (q, M + 1);
        auto Q2 = Matrix<f64>::hankel (q, M + 1, 0);

        Q1 += Q2; Q1 *= 0.5;

        Q1.solve (b);

        c[M] = static_cast<FloatType> (b (0, 0));

        for (size_t i = 1; i <= M; ++i)
        {
            c[M - i] = static_cast<FloatType> (b (i, 0) * 0.5);
            c[M + i] = static_cast<FloatType> (b (i, 0) * 0.5);
        }
    }
    else
    {
        // Type II
        auto M = N / 2;

        Matrix<f64> b (M, 1);
        Matrix<f64> qp (2 * M, 1);
        Matrix<f64> qs (2 * M, 1);

        auto factorp = wp / MathConstants<f64>::pi;
        auto factors = ws / MathConstants<f64>::pi;

        for (size_t i = 0; i < M; ++i)
            b (i, 0) = factorp * sinc (factorp * ((f64) i + 0.5));

        for (size_t i = 0; i < 2 * M; ++i)
        {
            qp (i, 0) = 0.25 * factorp * sinc (factorp * (f64) i);
            qs (i, 0) = -0.25 * stopBandWeight * factors * sinc (factors * (f64) i);
        }

        auto Q1p = Matrix<f64>::toeplitz (qp, M);
        auto Q2p = Matrix<f64>::hankel (qp, M, 1);
        auto Q1s = Matrix<f64>::toeplitz (qs, M);
        auto Q2s = Matrix<f64>::hankel (qs, M, 1);

        auto Id = Matrix<f64>::identity (M);
        Id *= (0.25 * stopBandWeight);

        Q1p += Q2p;
        Q1s += Q2s;
        Q1s += Id;

        auto& Q = Q1s;
        Q += Q1p;

        Q.solve (b);

        for (size_t i = 0; i < M; ++i)
        {
            c[M - i - 1] = static_cast<FloatType> (b (i, 0) * 0.25);
            c[M + i]     = static_cast<FloatType> (b (i, 0) * 0.25);
        }
    }

    return *result;
}

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassHalfBandEquirippleMethod (FloatType normalisedTransitionWidth,
                                                                       FloatType amplitudedB)
{
    jassert (normalisedTransitionWidth > 0 && normalisedTransitionWidth <= 0.5);
    jassert (amplitudedB >= -300 && amplitudedB <= -10);

    auto wpT = (0.5 - normalisedTransitionWidth) * MathConstants<f64>::pi;

    auto n = roundToInt (std::ceil ((amplitudedB - 18.18840664 * wpT + 33.64775300) / (18.54155181 * wpT - 29.13196871)));
    auto kp = (n * wpT - 1.57111377 * n + 0.00665857) / (-1.01927560 * n + 0.37221484);
    auto A = (0.01525753 * n + 0.03682344 + 9.24760314 / (f64) n) * kp + 1.01701407 + 0.73512298 / (f64) n;
    auto B = (0.00233667 * n - 1.35418408 + 5.75145813 / (f64) n) * kp + 1.02999650 - 0.72759508 / (f64) n;

    auto hn  = FilterDesign<FloatType>::getPartialImpulseResponseHn (n, kp);
    auto hnm = FilterDesign<FloatType>::getPartialImpulseResponseHn (n - 1, kp);

    auto diff = (hn.size() - hnm.size()) / 2;

    for (i32 i = 0; i < diff; ++i)
    {
        hnm.add (0.0);
        hnm.insert (0, 0.0);
    }

    auto hh = hn;

    for (i32 i = 0; i < hn.size(); ++i)
        hh.setUnchecked (i, A * hh[i] + B * hnm[i]);

    auto* result = new typename FIR::Coefficients<FloatType> (static_cast<size_t> (hh.size()));
    auto* c = result->getRawCoefficients();

    for (i32 i = 0; i < hh.size(); ++i)
        c[i] = (f32) hh[i];

    auto NN = [&]
    {
        if (n % 2 == 0)
            return 2.0 * result->getMagnitudeForFrequency (0.5, 1.0);

        auto w01 = std::sqrt (kp * kp + (1 - kp * kp) * std::pow (std::cos (MathConstants<f64>::pi / (2.0 * n + 1.0)), 2.0));

        if (std::abs (w01) > 1.0)
            return 2.0 * result->getMagnitudeForFrequency (0.5, 1.0);

        auto om01 = std::acos (-w01);
        return -2.0 * result->getMagnitudeForFrequency (om01 / MathConstants<f64>::twoPi, 1.0);
    }();

    for (i32 i = 0; i < hh.size(); ++i)
        c[i] = static_cast<FloatType> ((A * hn[i] + B * hnm[i]) / NN);

    c[2 * n + 1] = static_cast<FloatType> (0.5);

    return *result;
}

template <typename FloatType>
Array<f64> FilterDesign<FloatType>::getPartialImpulseResponseHn (i32 n, f64 kp)
{
    Array<f64> alpha;
    alpha.resize (2 * n + 1);

    alpha.setUnchecked (2 * n, 1.0 / std::pow (1.0 - kp * kp, n));

    if (n > 0)
        alpha.setUnchecked (2 * n - 2, -(2 * n * kp * kp + 1) * alpha[2 * n]);

    if (n > 1)
        alpha.setUnchecked (2 * n - 4, -(4 * n + 1 + (n - 1) * (2 * n - 1) * kp * kp) / (2.0 * n) * alpha[2 * n - 2]
                             - (2 * n + 1) * ((n + 1) * kp * kp + 1) / (2.0 * n) * alpha[2 * n]);

    for (i32 k = n; k >= 3; --k)
    {
        auto c1 = (3 * (n*(n + 2) - k * (k - 2)) + 2 * k - 3 + 2 * (k - 2)*(2 * k - 3) * kp * kp) * alpha[2 * k - 4];
        auto c2 = (3 * (n*(n + 2) - (k - 1) * (k + 1)) + 2 * (2 * k - 1) + 2 * k*(2 * k - 1) * kp * kp) * alpha[2 * k - 2];
        auto c3 = (n * (n + 2) - (k - 1) * (k + 1)) * alpha[2 * k];
        auto c4 = (n * (n + 2) - (k - 3) * (k - 1));

        alpha.setUnchecked (2 * k - 6, -(c1 + c2 + c3) / c4);
    }

    Array<f64> ai;
    ai.resize (2 * n + 1 + 1);

    for (i32 k = 0; k <= n; ++k)
        ai.setUnchecked (2 * k + 1, alpha[2 * k] / (2.0 * k + 1.0));

    Array<f64> hn;
    hn.resize (2 * n + 1 + 2 * n + 1 + 1);

    for (i32 k = 0; k <= n; ++k)
    {
        hn.setUnchecked (2 * n + 1 + (2 * k + 1), 0.5 * ai[2 * k + 1]);
        hn.setUnchecked (2 * n + 1 - (2 * k + 1), 0.5 * ai[2 * k + 1]);
    }

    return hn;
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderButterworthMethod (FloatType frequency, f64 sampleRate,
                                                                         FloatType normalisedTransitionWidth,
                                                                         FloatType passbandAmplitudedB,
                                                                         FloatType stopbandAmplitudedB)
{
    return designIIRLowpassHighOrderGeneralMethod (0, frequency, sampleRate, normalisedTransitionWidth,
                                                   passbandAmplitudedB, stopbandAmplitudedB);
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderChebyshev1Method (FloatType frequency, f64 sampleRate,
                                                                        FloatType normalisedTransitionWidth,
                                                                        FloatType passbandAmplitudedB,
                                                                        FloatType stopbandAmplitudedB)
{
    return designIIRLowpassHighOrderGeneralMethod (1, frequency, sampleRate, normalisedTransitionWidth,
                                                   passbandAmplitudedB, stopbandAmplitudedB);
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderChebyshev2Method (FloatType frequency, f64 sampleRate,
                                                                        FloatType normalisedTransitionWidth,
                                                                        FloatType passbandAmplitudedB,
                                                                        FloatType stopbandAmplitudedB)
{
    return designIIRLowpassHighOrderGeneralMethod (2, frequency, sampleRate, normalisedTransitionWidth,
                                                   passbandAmplitudedB, stopbandAmplitudedB);
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderEllipticMethod (FloatType frequency, f64 sampleRate,
                                                                      FloatType normalisedTransitionWidth,
                                                                      FloatType passbandAmplitudedB,
                                                                      FloatType stopbandAmplitudedB)
{
    return designIIRLowpassHighOrderGeneralMethod (3, frequency, sampleRate, normalisedTransitionWidth,
                                                   passbandAmplitudedB, stopbandAmplitudedB);
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderGeneralMethod (i32 type, FloatType frequency, f64 sampleRate,
                                                                     FloatType normalisedTransitionWidth,
                                                                     FloatType passbandAmplitudedB,
                                                                     FloatType stopbandAmplitudedB)
{
    jassert (0 < sampleRate);
    jassert (0 < frequency && frequency <= sampleRate * 0.5);
    jassert (0 < normalisedTransitionWidth && normalisedTransitionWidth <= 0.5);
    jassert (-20 < passbandAmplitudedB && passbandAmplitudedB < 0);
    jassert (-300 < stopbandAmplitudedB && stopbandAmplitudedB < -20);

    auto normalisedFrequency = frequency / sampleRate;

    auto fp = normalisedFrequency - normalisedTransitionWidth / 2;
    jassert (0.0 < fp && fp < 0.5);

    auto fs = normalisedFrequency + normalisedTransitionWidth / 2;
    jassert (0.0 < fs && fs < 0.5);

    f64 Ap = passbandAmplitudedB;
    f64 As = stopbandAmplitudedB;
    auto Gp = Decibels::decibelsToGain (Ap, -300.0);
    auto Gs = Decibels::decibelsToGain (As, -300.0);
    auto epsp = std::sqrt (1.0 / (Gp * Gp) - 1.0);
    auto epss = std::sqrt (1.0 / (Gs * Gs) - 1.0);

    auto omegap = std::tan (MathConstants<f64>::pi * fp);
    auto omegas = std::tan (MathConstants<f64>::pi * fs);
    constexpr auto halfPi = MathConstants<f64>::halfPi;

    auto k = omegap / omegas;
    auto k1 = epsp / epss;

    i32 N;

    if (type == 0)
    {
        N = roundToInt (std::ceil (std::log (1.0 / k1) / std::log (1.0 / k)));
    }
    else if (type == 1 || type == 2)
    {
        N = roundToInt (std::ceil (std::acosh (1.0 / k1) / std::acosh (1.0 / k)));
    }
    else
    {
        f64 K, Kp, K1, K1p;

        SpecialFunctions::ellipticIntegralK (k, K, Kp);
        SpecialFunctions::ellipticIntegralK (k1, K1, K1p);

        N = roundToInt (std::ceil ((K1p * K) / (K1 * Kp)));
    }

    i32k r = N % 2;
    i32k L = (N - r) / 2;
    const f64 H0 = (type == 1 || type == 3) ? std::pow (Gp, 1.0 - r) : 1.0;

    Array<Complex<f64>> pa, za;
    Complex<f64> j (0, 1);

    if (type == 0)
    {
        if (r == 1)
            pa.add (-omegap * std::pow (epsp, -1.0 / (f64) N));

        for (i32 i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (f64) N;
            pa.add (omegap * std::pow (epsp, -1.0 / (f64) N) * j * exp (ui * halfPi * j));
        }
    }
    else if (type == 1)
    {
        auto v0 = std::asinh (1.0 / epsp) / (N * halfPi);

        if (r == 1)
            pa.add (-omegap * std::sinh (v0 * halfPi));

        for (i32 i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (f64) N;
            pa.add (omegap * j * std::cos ((ui - j * v0) * halfPi));
        }
    }
    else if (type == 2)
    {
        auto v0 = std::asinh (epss) / (N * halfPi);

        if (r == 1)
            pa.add (-1.0 / (k / omegap * std::sinh (v0 * halfPi)));

        for (i32 i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (f64) N;

            pa.add (1.0 / (k / omegap * j * std::cos ((ui - j * v0) * halfPi)));
            za.add (1.0 / (k / omegap * j * std::cos (ui * halfPi)));
        }
    }
    else
    {
        auto v0 = -j * (SpecialFunctions::asne (j / epsp, k1) / (f64) N);

        if (r == 1)
            pa.add (omegap * j * SpecialFunctions::sne (j * v0, k));

        for (i32 i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (f64) N;
            auto zetai = SpecialFunctions::cde (ui, k);

            pa.add (omegap * j * SpecialFunctions::cde (ui - j * v0, k));
            za.add (omegap * j / (k * zetai));
        }
    }

    Array<Complex<f64>> p, z, g;

    if (r == 1)
    {
        p.add ((1.0 + pa[0]) / (1.0 - pa[0]));
        g.add (0.5 * (1.0 - p[0]));
    }

    for (i32 i = 0; i < L; ++i)
    {
        p.add ((1.0 + pa[i + r]) / (1.0 - pa[i + r]));
        z.add (za.size() == 0 ? -1.0 : (1.0 + za[i]) / (1.0 - za[i]));
        g.add ((1.0 - p[i + r]) / (1.0 - z[i]));
    }

    ReferenceCountedArray<IIR::Coefficients<FloatType>> cascadedCoefficients;

    if (r == 1)
    {
        auto b0 = static_cast<FloatType> (H0 * std::real (g[0]));
        auto b1 = b0;
        auto a1 = static_cast<FloatType> (-std::real (p[0]));

        cascadedCoefficients.add (new IIR::Coefficients<FloatType> (b0, b1, 1.0f, a1));
    }

    for (i32 i = 0; i < L; ++i)
    {
        auto gain = std::pow (std::abs (g[i + r]), 2.0);

        auto b0 = static_cast<FloatType> (gain);
        auto b1 = static_cast<FloatType> (std::real (-z[i] - std::conj (z[i])) * gain);
        auto b2 = static_cast<FloatType> (std::real ( z[i] * std::conj (z[i])) * gain);

        auto a1 = static_cast<FloatType> (std::real (-p[i+r] - std::conj (p[i + r])));
        auto a2 = static_cast<FloatType> (std::real ( p[i+r] * std::conj (p[i + r])));

        cascadedCoefficients.add (new IIR::Coefficients<FloatType> (b0, b1, b2, 1, a1, a2));
    }

    return cascadedCoefficients;
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderButterworthMethod (FloatType frequency,
                                                                         f64 sampleRate, i32 order)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (order > 0);

    ReferenceCountedArray<IIR::Coefficients<FloatType>> arrayFilters;

    if (order % 2 == 1)
    {
        arrayFilters.add (*IIR::Coefficients<FloatType>::makeFirstOrderLowPass (sampleRate, frequency));

        for (i32 i = 0; i < order / 2; ++i)
        {
            auto Q = 1.0 / (2.0 * std::cos ((i + 1.0) * MathConstants<f64>::pi / order));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeLowPass (sampleRate, frequency,
                                                                          static_cast<FloatType> (Q)));
        }
    }
    else
    {
        for (i32 i = 0; i < order / 2; ++i)
        {
            auto Q = 1.0 / (2.0 * std::cos ((2.0 * i + 1.0) * MathConstants<f64>::pi / (order * 2.0)));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeLowPass (sampleRate, frequency,
                                                                          static_cast<FloatType> (Q)));
        }
    }

    return arrayFilters;
}

template <typename FloatType>
ReferenceCountedArray<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRHighpassHighOrderButterworthMethod (FloatType frequency,
                                                                          f64 sampleRate, i32 order)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (order > 0);

    ReferenceCountedArray<IIR::Coefficients<FloatType>> arrayFilters;

    if (order % 2 == 1)
    {
        arrayFilters.add (*IIR::Coefficients<FloatType>::makeFirstOrderHighPass (sampleRate, frequency));

        for (i32 i = 0; i < order / 2; ++i)
        {
            auto Q = 1.0 / (2.0 * std::cos ((i + 1.0) * MathConstants<f64>::pi / order));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeHighPass (sampleRate, frequency,
                                                                           static_cast<FloatType> (Q)));
        }
    }
    else
    {
        for (i32 i = 0; i < order / 2; ++i)
        {
            auto Q = 1.0 / (2.0 * std::cos ((2.0 * i + 1.0) * MathConstants<f64>::pi / (order * 2.0)));
            arrayFilters.add (*IIR::Coefficients<FloatType>::makeHighPass (sampleRate, frequency,
                                                                           static_cast<FloatType> (Q)));
        }
    }

    return arrayFilters;
}

template <typename FloatType>
typename FilterDesign<FloatType>::IIRPolyphaseAllpassStructure
    FilterDesign<FloatType>::designIIRLowpassHalfBandPolyphaseAllpassMethod (FloatType normalisedTransitionWidth,
                                                                             FloatType stopbandAmplitudedB)
{
    jassert (normalisedTransitionWidth > 0 && normalisedTransitionWidth <= 0.5);
    jassert (stopbandAmplitudedB > -300 && stopbandAmplitudedB < -10);

    const f64 wt = MathConstants<f64>::twoPi * normalisedTransitionWidth;
    const f64 ds = Decibels::decibelsToGain (stopbandAmplitudedB, static_cast<FloatType> (-300.0));

    auto k = std::pow (std::tan ((MathConstants<f64>::pi - wt) / 4), 2.0);
    auto kp = std::sqrt (1.0 - k * k);
    auto e = (1 - std::sqrt (kp)) / (1 + std::sqrt (kp)) * 0.5;
    auto q = e + 2 * std::pow (e, 5.0) + 15 * std::pow (e, 9.0) + 150 * std::pow (e, 13.0);

    auto k1 = ds * ds / (1 - ds * ds);
    i32 n = roundToInt (std::ceil (std::log (k1 * k1 / 16) / std::log (q)));

    if (n % 2 == 0)
        ++n;

    if (n == 1)
        n = 3;

    auto q1 = std::pow (q, (f64) n);
    k1 = 4 * std::sqrt (q1);

    i32k N = (n - 1) / 2;
    Array<f64> ai;

    for (i32 i = 1; i <= N; ++i)
    {
        f64 num = 0.0;
        f64 delta = 1.0;
        i32 m = 0;

        while (std::abs (delta) > 1e-100)
        {
            delta = std::pow (-1, m) * std::pow (q, m * (m + 1))
                     * std::sin ((2 * m + 1) * MathConstants<f64>::pi * i / (f64) n);
            num += delta;
            m++;
        }

        num *= 2 * std::pow (q, 0.25);

        f64 den = 0.0;
        delta = 1.0;
        m = 1;

        while (std::abs (delta) > 1e-100)
        {
            delta = std::pow (-1, m) * std::pow (q, m * m)
                     * std::cos (m * MathConstants<f64>::twoPi * i / (f64) n);
            den += delta;
            ++m;
        }

        den = 1 + 2 * den;

        auto wi = num / den;
        auto api = std::sqrt ((1 - wi * wi * k) * (1 - wi * wi / k)) / (1 + wi * wi);

        ai.add ((1 - api) / (1 + api));
    }

    IIRPolyphaseAllpassStructure structure;

    for (i32 i = 0; i < N; i += 2)
        structure.directPath.add (new IIR::Coefficients<FloatType> (static_cast<FloatType> (ai[i]),
                                                                    0, 1, 1, 0, static_cast<FloatType> (ai[i])));

    structure.delayedPath.add (new IIR::Coefficients<FloatType> (0, 1, 1, 0));

    for (i32 i = 1; i < N; i += 2)
        structure.delayedPath.add (new IIR::Coefficients<FloatType> (static_cast<FloatType> (ai[i]),
                                                                     0, 1, 1, 0, static_cast<FloatType> (ai[i])));

    structure.alpha.addArray (ai);

    return structure;
}


template struct FilterDesign<f32>;
template struct FilterDesign<f64>;

} // namespace drx::dsp
