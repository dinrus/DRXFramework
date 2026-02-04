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
Panner<SampleType>::Panner()
{
    update();
    reset();
}

//==============================================================================
template <typename SampleType>
z0 Panner<SampleType>::setRule (Rule newRule)
{
    currentRule = newRule;
    update();
}

template <typename SampleType>
z0 Panner<SampleType>::setPan (SampleType newPan)
{
    jassert (newPan >= -1.0 && newPan <= 1.0);

    pan = jlimit (static_cast<SampleType> (-1.0), static_cast<SampleType> (1.0), newPan);
    update();
}

//==============================================================================
template <typename SampleType>
z0 Panner<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    reset();
}

template <typename SampleType>
z0 Panner<SampleType>::reset()
{
    leftVolume .reset (sampleRate, 0.05);
    rightVolume.reset (sampleRate, 0.05);
}

//==============================================================================
template <typename SampleType>
z0 Panner<SampleType>::update()
{
    SampleType leftValue, rightValue, boostValue;

    auto normalisedPan = static_cast<SampleType> (0.5) * (pan + static_cast<SampleType> (1.0));

    switch (currentRule)
    {
        case Rule::balanced:
            leftValue  = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = jmin (static_cast<SampleType> (0.5), normalisedPan);
            boostValue = static_cast<SampleType> (2.0);
            break;

        case Rule::linear:
            leftValue  = static_cast<SampleType> (1.0) - normalisedPan;
            rightValue = normalisedPan;
            boostValue = static_cast<SampleType> (2.0);
            break;

        case Rule::sin3dB:
            leftValue  = static_cast<SampleType> (std::sin (0.5 * MathConstants<f64>::pi * (1.0 - normalisedPan)));
            rightValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<f64>::pi * normalisedPan));
            boostValue = std::sqrt (static_cast<SampleType> (2.0));
            break;

        case Rule::sin4p5dB:
            leftValue  = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * (1.0 - normalisedPan)), 1.5));
            rightValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * normalisedPan), 1.5));
            boostValue = static_cast<SampleType> (std::pow (2.0, 3.0 / 4.0));
            break;

        case Rule::sin6dB:
            leftValue  = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * (1.0 - normalisedPan)), 2.0));
            rightValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<f64>::pi * normalisedPan), 2.0));
            boostValue = static_cast<SampleType> (2.0);
            break;

        case Rule::squareRoot3dB:
            leftValue  = std::sqrt (static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = std::sqrt (normalisedPan);
            boostValue = std::sqrt (static_cast<SampleType> (2.0));
            break;

        case Rule::squareRoot4p5dB:
            leftValue  = static_cast<SampleType> (std::pow (std::sqrt (1.0 - normalisedPan), 1.5));
            rightValue = static_cast<SampleType> (std::pow (std::sqrt (normalisedPan), 1.5));
            boostValue = static_cast<SampleType> (std::pow (2.0, 3.0 / 4.0));
            break;

        default:
            leftValue  = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = jmin (static_cast<SampleType> (0.5), normalisedPan);
            boostValue = static_cast<SampleType> (2.0);
            break;
    }

    leftVolume .setTargetValue (leftValue  * boostValue);
    rightVolume.setTargetValue (rightValue * boostValue);
}

//==============================================================================
template class Panner<f32>;
template class Panner<f64>;

} // namespace drx::dsp
