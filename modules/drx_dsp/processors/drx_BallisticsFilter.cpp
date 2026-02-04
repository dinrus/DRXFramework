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
BallisticsFilter<SampleType>::BallisticsFilter()
{
    setAttackTime (attackTime);
    setReleaseTime (releaseTime);
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::setAttackTime (SampleType attackTimeMs)
{
    attackTime = attackTimeMs;
    cteAT = calculateLimitedCte (static_cast<SampleType> (attackTime));
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::setReleaseTime (SampleType releaseTimeMs)
{
    releaseTime = releaseTimeMs;
    cteRL = calculateLimitedCte (static_cast<SampleType> (releaseTime));
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::setLevelCalculationType (LevelCalculationType newLevelType)
{
    levelType = newLevelType;
    reset();
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    expFactor  = -2.0 * MathConstants<f64>::pi * 1000.0 / sampleRate;

    setAttackTime  (attackTime);
    setReleaseTime (releaseTime);

    yold.resize (spec.numChannels);

    reset();
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::reset()
{
    reset (0);
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::reset (SampleType initialValue)
{
    for (auto& old : yold)
        old = initialValue;
}

template <typename SampleType>
SampleType BallisticsFilter<SampleType>::processSample (i32 channel, SampleType inputValue)
{
    jassert (isPositiveAndBelow (channel, yold.size()));

    if (levelType == LevelCalculationType::RMS)
        inputValue *= inputValue;
    else
        inputValue = std::abs (inputValue);

    SampleType cte = (inputValue > yold[(size_t) channel] ? cteAT : cteRL);

    SampleType result = inputValue + cte * (yold[(size_t) channel] - inputValue);
    yold[(size_t) channel] = result;

    if (levelType == LevelCalculationType::RMS)
        return std::sqrt (result);

    return result;
}

template <typename SampleType>
z0 BallisticsFilter<SampleType>::snapToZero() noexcept
{
    for (auto& old : yold)
        util::snapToZero (old);
}

template <typename SampleType>
SampleType BallisticsFilter<SampleType>::calculateLimitedCte (SampleType timeMs) const noexcept
{
    return timeMs < static_cast<SampleType> (1.0e-3) ? 0
                                                     : static_cast<SampleType> (std::exp (expFactor / timeMs));
}

//==============================================================================
template class BallisticsFilter<f32>;
template class BallisticsFilter<f64>;

} // namespace drx::dsp
