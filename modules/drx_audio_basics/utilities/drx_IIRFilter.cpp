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

constexpr auto minimumDecibels = -300.0f;

IIRCoefficients::IIRCoefficients() noexcept
{
    zeromem (coefficients, sizeof (coefficients));
}

IIRCoefficients::~IIRCoefficients() noexcept {}

IIRCoefficients::IIRCoefficients (const IIRCoefficients& other) noexcept
{
    memcpy (coefficients, other.coefficients, sizeof (coefficients));
}

IIRCoefficients& IIRCoefficients::operator= (const IIRCoefficients& other) noexcept
{
    memcpy (coefficients, other.coefficients, sizeof (coefficients));
    return *this;
}

IIRCoefficients::IIRCoefficients (f64 c1, f64 c2, f64 c3,
                                  f64 c4, f64 c5, f64 c6) noexcept
{
    const auto a = 1.0 / c4;

    coefficients[0] = (f32) (c1 * a);
    coefficients[1] = (f32) (c2 * a);
    coefficients[2] = (f32) (c3 * a);
    coefficients[3] = (f32) (c5 * a);
    coefficients[4] = (f32) (c6 * a);
}

IIRCoefficients IIRCoefficients::makeLowPass (f64 sampleRate,
                                              f64 frequency) noexcept
{
    return makeLowPass (sampleRate, frequency, 1.0 / MathConstants<f64>::sqrt2);
}

IIRCoefficients IIRCoefficients::makeLowPass (f64 sampleRate,
                                              f64 frequency,
                                              f64 Q) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto n = 1.0 / std::tan (MathConstants<f64>::pi * frequency / sampleRate);
    const auto nSquared = n * n;
    const auto c1 = 1.0 / (1.0 + 1.0 / Q * n + nSquared);

    return IIRCoefficients (c1,
                            c1 * 2.0,
                            c1,
                            1.0,
                            c1 * 2.0 * (1.0 - nSquared),
                            c1 * (1.0 - 1.0 / Q * n + nSquared));
}

IIRCoefficients IIRCoefficients::makeHighPass (f64 sampleRate,
                                               f64 frequency) noexcept
{
    return makeHighPass (sampleRate, frequency, 1.0 / std::sqrt (2.0));
}

IIRCoefficients IIRCoefficients::makeHighPass (f64 sampleRate,
                                               f64 frequency,
                                               f64 Q) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto n = std::tan (MathConstants<f64>::pi * frequency / sampleRate);
    const auto nSquared = n * n;
    const auto c1 = 1.0 / (1.0 + 1.0 / Q * n + nSquared);

    return IIRCoefficients (c1,
                            c1 * -2.0,
                            c1,
                            1.0,
                            c1 * 2.0 * (nSquared - 1.0),
                            c1 * (1.0 - 1.0 / Q * n + nSquared));
}

IIRCoefficients IIRCoefficients::makeBandPass (f64 sampleRate,
                                               f64 frequency) noexcept
{
    return makeBandPass (sampleRate, frequency, 1.0 / MathConstants<f64>::sqrt2);
}

IIRCoefficients IIRCoefficients::makeBandPass (f64 sampleRate,
                                               f64 frequency,
                                               f64 Q) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto n = 1.0 / std::tan (MathConstants<f64>::pi * frequency / sampleRate);
    const auto nSquared = n * n;
    const auto c1 = 1.0 / (1.0 + 1.0 / Q * n + nSquared);

    return IIRCoefficients (c1 * n / Q,
                            0.0,
                            -c1 * n / Q,
                            1.0,
                            c1 * 2.0 * (1.0 - nSquared),
                            c1 * (1.0 - 1.0 / Q * n + nSquared));
}

IIRCoefficients IIRCoefficients::makeNotchFilter (f64 sampleRate,
                                                  f64 frequency) noexcept
{
    return makeNotchFilter (sampleRate, frequency, 1.0 / MathConstants<f64>::sqrt2);
}

IIRCoefficients IIRCoefficients::makeNotchFilter (f64 sampleRate,
                                                  f64 frequency,
                                                  f64 Q) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto n = 1.0 / std::tan (MathConstants<f64>::pi * frequency / sampleRate);
    const auto nSquared = n * n;
    const auto c1 = 1.0 / (1.0 + n / Q + nSquared);

    return IIRCoefficients (c1 * (1.0 + nSquared),
                            2.0 * c1 * (1.0 - nSquared),
                            c1 * (1.0 + nSquared),
                            1.0,
                            c1 * 2.0 * (1.0 - nSquared),
                            c1 * (1.0 - n / Q + nSquared));
}

IIRCoefficients IIRCoefficients::makeAllPass (f64 sampleRate,
                                              f64 frequency) noexcept
{
    return makeAllPass (sampleRate, frequency, 1.0 / MathConstants<f64>::sqrt2);
}

IIRCoefficients IIRCoefficients::makeAllPass (f64 sampleRate,
                                              f64 frequency,
                                              f64 Q) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto n = 1.0 / std::tan (MathConstants<f64>::pi * frequency / sampleRate);
    const auto nSquared = n * n;
    const auto c1 = 1.0 / (1.0 + 1.0 / Q * n + nSquared);

    return IIRCoefficients (c1 * (1.0 - n / Q + nSquared),
                            c1 * 2.0 * (1.0 - nSquared),
                            1.0,
                            1.0,
                            c1 * 2.0 * (1.0 - nSquared),
                            c1 * (1.0 - n / Q + nSquared));
}

IIRCoefficients IIRCoefficients::makeLowShelf (f64 sampleRate,
                                               f64 cutOffFrequency,
                                               f64 Q,
                                               f32 gainFactor) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (cutOffFrequency > 0.0 && cutOffFrequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto A = std::sqrt (Decibels::gainWithLowerBound (gainFactor, minimumDecibels));
    const auto aminus1 = A - 1.0;
    const auto aplus1 = A + 1.0;
    const auto omega = (MathConstants<f64>::twoPi * jmax (cutOffFrequency, 2.0)) / sampleRate;
    const auto coso = std::cos (omega);
    const auto beta = std::sin (omega) * std::sqrt (A) / Q;
    const auto aminus1TimesCoso = aminus1 * coso;

    return IIRCoefficients (A * (aplus1 - aminus1TimesCoso + beta),
                            A * 2.0 * (aminus1 - aplus1 * coso),
                            A * (aplus1 - aminus1TimesCoso - beta),
                            aplus1 + aminus1TimesCoso + beta,
                            -2.0 * (aminus1 + aplus1 * coso),
                            aplus1 + aminus1TimesCoso - beta);
}

IIRCoefficients IIRCoefficients::makeHighShelf (f64 sampleRate,
                                                f64 cutOffFrequency,
                                                f64 Q,
                                                f32 gainFactor) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (cutOffFrequency > 0.0 && cutOffFrequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto A = std::sqrt (Decibels::gainWithLowerBound (gainFactor, minimumDecibels));
    const auto aminus1 = A - 1.0;
    const auto aplus1 = A + 1.0;
    const auto omega = (MathConstants<f64>::twoPi * jmax (cutOffFrequency, 2.0)) / sampleRate;
    const auto coso = std::cos (omega);
    const auto beta = std::sin (omega) * std::sqrt (A) / Q;
    const auto aminus1TimesCoso = aminus1 * coso;

    return IIRCoefficients (A * (aplus1 + aminus1TimesCoso + beta),
                            A * -2.0 * (aminus1 + aplus1 * coso),
                            A * (aplus1 + aminus1TimesCoso - beta),
                            aplus1 - aminus1TimesCoso + beta,
                            2.0 * (aminus1 - aplus1 * coso),
                            aplus1 - aminus1TimesCoso - beta);
}

IIRCoefficients IIRCoefficients::makePeakFilter (f64 sampleRate,
                                                 f64 frequency,
                                                 f64 Q,
                                                 f32 gainFactor) noexcept
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    const auto A = std::sqrt (Decibels::gainWithLowerBound (gainFactor, minimumDecibels));
    const auto omega = (MathConstants<f64>::twoPi * jmax (frequency, 2.0)) / sampleRate;
    const auto alpha = 0.5 * std::sin (omega) / Q;
    const auto c2 = -2.0 * std::cos (omega);
    const auto alphaTimesA = alpha * A;
    const auto alphaOverA = alpha / A;

    return IIRCoefficients (1.0 + alphaTimesA,
                            c2,
                            1.0 - alphaTimesA,
                            1.0 + alphaOverA,
                            c2,
                            1.0 - alphaOverA);
}

//==============================================================================
template <typename Mutex>
IIRFilterBase<Mutex>::IIRFilterBase() noexcept = default;

template <typename Mutex>
IIRFilterBase<Mutex>::IIRFilterBase (const IIRFilterBase& other) noexcept  : active (other.active)
{
    const typename Mutex::ScopedLockType sl (other.processLock);
    coefficients = other.coefficients;
}

//==============================================================================
template <typename Mutex>
z0 IIRFilterBase<Mutex>::makeInactive() noexcept
{
    const typename Mutex::ScopedLockType sl (processLock);
    active = false;
}

template <typename Mutex>
z0 IIRFilterBase<Mutex>::setCoefficients (const IIRCoefficients& newCoefficients) noexcept
{
    const typename Mutex::ScopedLockType sl (processLock);
    coefficients = newCoefficients;
    active = true;
}

//==============================================================================
template <typename Mutex>
z0 IIRFilterBase<Mutex>::reset() noexcept
{
    const typename Mutex::ScopedLockType sl (processLock);
    v1 = v2 = 0.0;
}

template <typename Mutex>
f32 IIRFilterBase<Mutex>::processSingleSampleRaw (f32 in) noexcept
{
    auto out = coefficients.coefficients[0] * in + v1;

    DRX_SNAP_TO_ZERO (out);

    v1 = coefficients.coefficients[1] * in - coefficients.coefficients[3] * out + v2;
    v2 = coefficients.coefficients[2] * in - coefficients.coefficients[4] * out;

    return out;
}

template <typename Mutex>
z0 IIRFilterBase<Mutex>::processSamples (f32* const samples, i32k numSamples) noexcept
{
    const typename Mutex::ScopedLockType sl (processLock);

    if (active)
    {
        auto c0 = coefficients.coefficients[0];
        auto c1 = coefficients.coefficients[1];
        auto c2 = coefficients.coefficients[2];
        auto c3 = coefficients.coefficients[3];
        auto c4 = coefficients.coefficients[4];
        auto lv1 = v1, lv2 = v2;

        for (i32 i = 0; i < numSamples; ++i)
        {
            auto in = samples[i];
            auto out = c0 * in + lv1;
            samples[i] = out;

            lv1 = c1 * in - c3 * out + lv2;
            lv2 = c2 * in - c4 * out;
        }

        DRX_SNAP_TO_ZERO (lv1);  v1 = lv1;
        DRX_SNAP_TO_ZERO (lv2);  v2 = lv2;
    }
}

template class IIRFilterBase<SpinLock>;
template class IIRFilterBase<DummyCriticalSection>;

} // namespace drx
