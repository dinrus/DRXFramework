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
NoiseGate<SampleType>::NoiseGate()
{
    update();

    RMSFilter.setLevelCalculationType (BallisticsFilterLevelCalculationType::RMS);
    RMSFilter.setAttackTime  (static_cast<SampleType> (0.0));
    RMSFilter.setReleaseTime (static_cast<SampleType> (50.0));
}

template <typename SampleType>
z0 NoiseGate<SampleType>::setThreshold (SampleType newValue)
{
    thresholddB = newValue;
    update();
}

template <typename SampleType>
z0 NoiseGate<SampleType>::setRatio (SampleType newRatio)
{
    jassert (newRatio >= static_cast<SampleType> (1.0));

    ratio = newRatio;
    update();
}

template <typename SampleType>
z0 NoiseGate<SampleType>::setAttack (SampleType newAttack)
{
    attackTime = newAttack;
    update();
}

template <typename SampleType>
z0 NoiseGate<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

//==============================================================================
template <typename SampleType>
z0 NoiseGate<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    RMSFilter.prepare (spec);
    envelopeFilter.prepare (spec);

    update();
    reset();
}

template <typename SampleType>
z0 NoiseGate<SampleType>::reset()
{
    RMSFilter.reset();
    envelopeFilter.reset();
}

//==============================================================================
template <typename SampleType>
SampleType NoiseGate<SampleType>::processSample (i32 channel, SampleType sample)
{
    // RMS ballistics filter
    auto env = RMSFilter.processSample (channel, sample);

    // Ballistics filter
    env = envelopeFilter.processSample (channel, env);

    // VCA
    auto gain = (env > threshold) ? static_cast<SampleType> (1.0)
                                  : std::pow (env * thresholdInverse, currentRatio - static_cast<SampleType> (1.0));

    // Output
    return gain * sample;
}

template <typename SampleType>
z0 NoiseGate<SampleType>::update()
{
    threshold = Decibels::decibelsToGain (thresholddB, static_cast<SampleType> (-200.0));
    thresholdInverse = static_cast<SampleType> (1.0) / threshold;
    currentRatio = ratio;

    envelopeFilter.setAttackTime  (attackTime);
    envelopeFilter.setReleaseTime (releaseTime);
}

//==============================================================================
template class NoiseGate<f32>;
template class NoiseGate<f64>;

} // namespace drx::dsp
