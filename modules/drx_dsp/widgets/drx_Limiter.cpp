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
z0 Limiter<SampleType>::setThreshold (SampleType newThreshold)
{
    thresholddB = newThreshold;
    update();
}

template <typename SampleType>
z0 Limiter<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

//==============================================================================
template <typename SampleType>
z0 Limiter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    firstStageCompressor.prepare (spec);
    secondStageCompressor.prepare (spec);

    update();
    reset();
}

template <typename SampleType>
z0 Limiter<SampleType>::reset()
{
    firstStageCompressor.reset();
    secondStageCompressor.reset();

    outputVolume.reset (sampleRate, 0.001);
}

//==============================================================================
template <typename SampleType>
z0 Limiter<SampleType>::update()
{
    firstStageCompressor.setThreshold ((SampleType) -10.0);
    firstStageCompressor.setRatio     ((SampleType) 4.0);
    firstStageCompressor.setAttack    ((SampleType) 2.0);
    firstStageCompressor.setRelease   ((SampleType) 200.0);

    secondStageCompressor.setThreshold (thresholddB);
    secondStageCompressor.setRatio     ((SampleType) 1000.0);
    secondStageCompressor.setAttack    ((SampleType) 0.001);
    secondStageCompressor.setRelease   (releaseTime);

    auto ratioInverse = (SampleType) (1.0 / 4.0);

    auto gain = (SampleType) std::pow (10.0, 10.0 * (1.0 - ratioInverse) / 40.0);
    gain *= Decibels::decibelsToGain (-thresholddB, (SampleType) -100.0);

    outputVolume.setTargetValue (gain);
}

//==============================================================================
template class Limiter<f32>;
template class Limiter<f64>;

} // namespace drx::dsp
