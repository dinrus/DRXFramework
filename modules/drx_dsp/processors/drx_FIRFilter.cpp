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

template <typename NumericType>
f64 FIR::Coefficients<NumericType>::Coefficients::getMagnitudeForFrequency (f64 frequency, f64 theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);
    jassert (frequency >= 0.0 && frequency <= theSampleRate * 0.5);

    constexpr Complex<f64> j (0, 1);
    auto order = getFilterOrder();

    Complex<f64> numerator = 0.0, factor = 1.0;
    Complex<f64> jw = std::exp (-MathConstants<f64>::twoPi * frequency * j / theSampleRate);

    const auto* coefs = coefficients.begin();

    for (size_t n = 0; n <= order; ++n)
    {
        numerator += static_cast<f64> (coefs[n]) * factor;
        factor *= jw;
    }

    return std::abs (numerator);
}

//==============================================================================
template <typename NumericType>
z0 FIR::Coefficients<NumericType>::Coefficients::getMagnitudeForFrequencyArray (f64* frequencies, f64* magnitudes,
                                                                        size_t numSamples, f64 theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);

    constexpr Complex<f64> j (0, 1);
    const auto* coefs = coefficients.begin();
    auto order = getFilterOrder();

    for (size_t i = 0; i < numSamples; ++i)
    {
        jassert (frequencies[i] >= 0.0 && frequencies[i] <= theSampleRate * 0.5);

        Complex<f64> numerator = 0.0;
        Complex<f64> factor = 1.0;
        Complex<f64> jw = std::exp (-MathConstants<f64>::twoPi * frequencies[i] * j / theSampleRate);

        for (size_t n = 0; n <= order; ++n)
        {
            numerator += static_cast<f64> (coefs[n]) * factor;
            factor *= jw;
        }

        magnitudes[i] = std::abs (numerator);
    }
}

//==============================================================================
template <typename NumericType>
f64 FIR::Coefficients<NumericType>::Coefficients::getPhaseForFrequency (f64 frequency, f64 theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);
    jassert (frequency >= 0.0 && frequency <= theSampleRate * 0.5);

    constexpr Complex<f64> j (0, 1);

    Complex<f64> numerator = 0.0;
    Complex<f64> factor = 1.0;
    Complex<f64> jw = std::exp (-MathConstants<f64>::twoPi * frequency * j / theSampleRate);

    const auto* coefs = coefficients.begin();
    auto order = getFilterOrder();

    for (size_t n = 0; n <= order; ++n)
    {
        numerator += static_cast<f64> (coefs[n]) * factor;
        factor *= jw;
    }

    return std::arg (numerator);
}

//==============================================================================
template <typename NumericType>
z0 FIR::Coefficients<NumericType>::Coefficients::getPhaseForFrequencyArray (f64* frequencies, f64* phases,
                                                                    size_t numSamples, f64 theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);

    constexpr Complex<f64> j (0, 1);
    const auto* coefs = coefficients.begin();
    auto order = getFilterOrder();

    for (size_t i = 0; i < numSamples; ++i)
    {
        jassert (frequencies[i] >= 0.0 && frequencies[i] <= theSampleRate * 0.5);

        Complex<f64> numerator = 0.0, factor = 1.0;
        Complex<f64> jw = std::exp (-MathConstants<f64>::twoPi * frequencies[i] * j / theSampleRate);

        for (size_t n = 0; n <= order; ++n)
        {
            numerator += static_cast<f64> (coefs[n]) * factor;
            factor *= jw;
        }

        phases[i] = std::arg (numerator);
    }
}

//==============================================================================
template <typename NumericType>
z0 FIR::Coefficients<NumericType>::Coefficients::normalise() noexcept
{
    auto magnitude = static_cast<NumericType> (0);

    auto* coefs = coefficients.getRawDataPointer();
    auto n = static_cast<size_t> (coefficients.size());

    for (size_t i = 0; i < n; ++i)
    {
        auto c = coefs[i];
        magnitude += c * c;
    }

    auto magnitudeInv = 1 / (4 * std::sqrt (magnitude));

    FloatVectorOperations::multiply (coefs, magnitudeInv, static_cast<i32> (n));
}

//==============================================================================
template struct FIR::Coefficients<f32>;
template struct FIR::Coefficients<f64>;

} // namespace drx::dsp
