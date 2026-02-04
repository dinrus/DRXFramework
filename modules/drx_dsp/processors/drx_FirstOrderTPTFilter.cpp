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
FirstOrderTPTFilter<SampleType>::FirstOrderTPTFilter()
{
    update();
}

//==============================================================================
template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::setType (Type newValue)
{
    filterType = newValue;
}

template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::setCutoffFrequency (SampleType newValue)
{
    jassert (isPositiveAndBelow (newValue, static_cast<SampleType> (sampleRate * 0.5)));

    cutoffFrequency = newValue;
    update();
}

//==============================================================================
template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    s1.resize (spec.numChannels);

    update();
    reset();
}

template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::reset()
{
    reset (static_cast<SampleType> (0));
}

template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::reset (SampleType newValue)
{
    std::fill (s1.begin(), s1.end(), newValue);
}

//==============================================================================
template <typename SampleType>
SampleType FirstOrderTPTFilter<SampleType>::processSample (i32 channel, SampleType inputValue)
{
    auto& s = s1[(size_t) channel];

    auto v = G * (inputValue - s);
    auto y = v + s;
    s = y + v;

    switch (filterType)
    {
        case Type::lowpass:   return y;
        case Type::highpass:  return inputValue - y;
        case Type::allpass:   return 2 * y - inputValue;
        default:              break;
    }

    jassertfalse;
    return y;
}

template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::snapToZero() noexcept
{
    for (auto& s : s1)
        util::snapToZero (s);
}

//==============================================================================
template <typename SampleType>
z0 FirstOrderTPTFilter<SampleType>::update()
{
    auto g = SampleType (std::tan (drx::MathConstants<f64>::pi * cutoffFrequency / sampleRate));
    G = g / (1 + g);
}

//==============================================================================
template class FirstOrderTPTFilter<f32>;
template class FirstOrderTPTFilter<f64>;

} // namespace drx::dsp
