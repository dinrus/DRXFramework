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
template <typename SampleType, typename InterpolationType>
DelayLine<SampleType, InterpolationType>::DelayLine()
    : DelayLine (0)
{
}

template <typename SampleType, typename InterpolationType>
DelayLine<SampleType, InterpolationType>::DelayLine (i32 maximumDelayInSamples)
{
    jassert (maximumDelayInSamples >= 0);

    sampleRate = 44100.0;

    setMaximumDelayInSamples (maximumDelayInSamples);
}

//==============================================================================
template <typename SampleType, typename InterpolationType>
z0 DelayLine<SampleType, InterpolationType>::setDelay (SampleType newDelayInSamples)
{
    auto upperLimit = (SampleType) getMaximumDelayInSamples();
    jassert (isPositiveAndNotGreaterThan (newDelayInSamples, upperLimit));

    delay     = jlimit ((SampleType) 0, upperLimit, newDelayInSamples);
    delayInt  = static_cast<i32> (std::floor (delay));
    delayFrac = delay - (SampleType) delayInt;

    updateInternalVariables();
}

template <typename SampleType, typename InterpolationType>
SampleType DelayLine<SampleType, InterpolationType>::getDelay() const
{
    return delay;
}

//==============================================================================
template <typename SampleType, typename InterpolationType>
z0 DelayLine<SampleType, InterpolationType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.numChannels > 0);

    bufferData.setSize ((i32) spec.numChannels, totalSize, false, false, true);

    writePos.resize (spec.numChannels);
    readPos.resize  (spec.numChannels);

    v.resize (spec.numChannels);
    sampleRate = spec.sampleRate;

    reset();
}

template <typename SampleType, typename InterpolationType>
z0 DelayLine<SampleType, InterpolationType>::setMaximumDelayInSamples (i32 maxDelayInSamples)
{
    jassert (maxDelayInSamples >= 0);
    totalSize = jmax (4, maxDelayInSamples + 2);
    bufferData.setSize ((i32) bufferData.getNumChannels(), totalSize, false, false, true);
    reset();
}

template <typename SampleType, typename InterpolationType>
z0 DelayLine<SampleType, InterpolationType>::reset()
{
    for (auto vec : { &writePos, &readPos })
        std::fill (vec->begin(), vec->end(), 0);

    std::fill (v.begin(), v.end(), static_cast<SampleType> (0));

    bufferData.clear();
}

//==============================================================================
template <typename SampleType, typename InterpolationType>
z0 DelayLine<SampleType, InterpolationType>::pushSample (i32 channel, SampleType sample)
{
    bufferData.setSample (channel, writePos[(size_t) channel], sample);
    writePos[(size_t) channel] = (writePos[(size_t) channel] + totalSize - 1) % totalSize;
}

template <typename SampleType, typename InterpolationType>
SampleType DelayLine<SampleType, InterpolationType>::popSample (i32 channel, SampleType delayInSamples, b8 updateReadPointer)
{
    if (delayInSamples >= 0)
        setDelay (delayInSamples);

    auto result = interpolateSample (channel);

    if (updateReadPointer)
        readPos[(size_t) channel] = (readPos[(size_t) channel] + totalSize - 1) % totalSize;

    return result;
}

//==============================================================================
template class DelayLine<f32,  DelayLineInterpolationTypes::None>;
template class DelayLine<f64, DelayLineInterpolationTypes::None>;
template class DelayLine<f32,  DelayLineInterpolationTypes::Linear>;
template class DelayLine<f64, DelayLineInterpolationTypes::Linear>;
template class DelayLine<f32,  DelayLineInterpolationTypes::Lagrange3rd>;
template class DelayLine<f64, DelayLineInterpolationTypes::Lagrange3rd>;
template class DelayLine<f32,  DelayLineInterpolationTypes::Thiran>;
template class DelayLine<f64, DelayLineInterpolationTypes::Thiran>;

} // namespace drx::dsp
