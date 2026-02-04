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

//==============================================================================
template <typename SampleType>
LinkwitzRileyFilter<SampleType>::LinkwitzRileyFilter()
{
    update();
}

//==============================================================================
template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::setType (Type newType)
{
    filterType = newType;
}

template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::setCutoffFrequency (SampleType newCutoffFrequencyHz)
{
    jassert (isPositiveAndBelow (newCutoffFrequencyHz, static_cast<SampleType> (sampleRate * 0.5)));

    cutoffFrequency = newCutoffFrequencyHz;
    update();
}

//==============================================================================
template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    update();

    s1.resize (spec.numChannels);
    s2.resize (spec.numChannels);
    s3.resize (spec.numChannels);
    s4.resize (spec.numChannels);

    reset();
}

template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::reset()
{
    for (auto s : { &s1, &s2, &s3, &s4 })
        std::fill (s->begin(), s->end(), static_cast<SampleType> (0));
}

template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::snapToZero() noexcept
{
    for (auto s : { &s1, &s2, &s3, &s4 })
        for (auto& element : *s)
            util::snapToZero (element);
}

//==============================================================================
template <typename SampleType>
SampleType LinkwitzRileyFilter<SampleType>::processSample (i32 channel, SampleType inputValue)
{
    auto yH = (inputValue - (R2 + g) * s1[(size_t) channel] - s2[(size_t) channel]) * h;

    auto yB = g * yH + s1[(size_t) channel];
    s1[(size_t) channel] = g * yH + yB;

    auto yL = g * yB + s2[(size_t) channel];
    s2[(size_t) channel] = g * yB + yL;

    if (filterType == Type::allpass)
        return yL - R2 * yB + yH;

    auto yH2 = ((filterType == Type::lowpass ? yL : yH) - (R2 + g) * s3[(size_t) channel] - s4[(size_t) channel]) * h;

    auto yB2 = g * yH2 + s3[(size_t) channel];
    s3[(size_t) channel] = g * yH2 + yB2;

    auto yL2 = g * yB2 + s4[(size_t) channel];
    s4[(size_t) channel] = g * yB2 + yL2;

    return filterType == Type::lowpass ? yL2 : yH2;
}

template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::processSample (i32 channel, SampleType inputValue, SampleType &outputLow, SampleType &outputHigh)
{
    auto yH = (inputValue - (R2 + g) * s1[(size_t) channel] - s2[(size_t) channel]) * h;

    auto yB = g * yH + s1[(size_t) channel];
    s1[(size_t) channel] = g * yH + yB;

    auto yL = g * yB + s2[(size_t) channel];
    s2[(size_t) channel] = g * yB + yL;

    auto yH2 = (yL - (R2 + g) * s3[(size_t) channel] - s4[(size_t) channel]) * h;

    auto yB2 = g * yH2 + s3[(size_t) channel];
    s3[(size_t) channel] = g * yH2 + yB2;

    auto yL2 = g * yB2 + s4[(size_t) channel];
    s4[(size_t) channel] = g * yB2 + yL2;

    outputLow = yL2;
    outputHigh = yL - R2 * yB + yH - yL2;
}

template <typename SampleType>
z0 LinkwitzRileyFilter<SampleType>::update()
{
    g  = (SampleType) std::tan (MathConstants<f64>::pi * cutoffFrequency / sampleRate);
    R2 = (SampleType) std::sqrt (2.0);
    h  = (SampleType) (1.0 / (1.0 + R2 * g + g * g));
}

//==============================================================================
template class LinkwitzRileyFilter<f32>;
template class LinkwitzRileyFilter<f64>;

} // namespace drx::dsp
