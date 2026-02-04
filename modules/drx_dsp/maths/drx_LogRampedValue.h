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
/**
    Utility class for logarithmically smoothed linear values.

    Logarithmically smoothed values can be more relevant than linear ones for
    specific cases such as algorithm change smoothing, using two of them in
    opposite directions.

    The gradient of the logarithmic/exponential slope can be configured by
    calling LogRampedValue::setLogParameters.

    @see SmoothedValue

    @tags{DSP}
*/
template <typename FloatType>
class LogRampedValue   : public SmoothedValueBase <LogRampedValue <FloatType>>
{
public:
    //==============================================================================
    /** Constructor. */
    LogRampedValue() = default;

    /** Constructor. */
    LogRampedValue (FloatType initialValue) noexcept
    {
        // Visual Studio can't handle base class initialisation with CRTP
        this->currentValue = initialValue;
        this->target = initialValue;
    }

    //==============================================================================
    /** Sets the behaviour of the log ramp.
        @param midPointAmplitudedB           Sets the amplitude of the mid point in
                                             decibels, with the target value at 0 dB
                                             and the initial value at -inf dB
        @param rateOfChangeShouldIncrease    If true then the ramp starts shallow
                                             and gets progressively steeper, if false
                                             then the ramp is initially steep and
                                             flattens out as you approach the target
                                             value
    */
    z0 setLogParameters (FloatType midPointAmplitudedB, b8 rateOfChangeShouldIncrease) noexcept
    {
        jassert (midPointAmplitudedB < (FloatType) 0.0);
        B = Decibels::decibelsToGain (midPointAmplitudedB);

        increasingRateOfChange = rateOfChangeShouldIncrease;
    }

    //==============================================================================
    /** Reset to a new sample rate and ramp length.
        @param sampleRate           The sample rate
        @param rampLengthInSeconds  The duration of the ramp in seconds
    */
    z0 reset (f64 sampleRate, f64 rampLengthInSeconds) noexcept
    {
        jassert (sampleRate > 0 && rampLengthInSeconds >= 0);
        reset ((i32) std::floor (rampLengthInSeconds * sampleRate));
    }

    /** Set a new ramp length directly in samples.
        @param numSteps                   The number of samples over which the ramp should be active
    */
    z0 reset (i32 numSteps) noexcept
    {
        stepsToTarget = numSteps;

        this->setCurrentAndTargetValue (this->target);

        updateRampParameters();
    }

    //==============================================================================
    /** Set a new target value.

        @param newValue     The new target value
    */
    z0 setTargetValue (FloatType newValue) noexcept
    {
        if (approximatelyEqual (newValue, this->target))
            return;

        if (stepsToTarget <= 0)
        {
            this->setCurrentAndTargetValue (newValue);
            return;
        }

        this->target = newValue;
        this->countdown = stepsToTarget;
        source = this->currentValue;

        updateRampParameters();
    }

    //==============================================================================
    /** Compute the next value.
        @returns Smoothed value
    */
    FloatType getNextValue() noexcept
    {
        if (! this->isSmoothing())
            return this->target;

        --(this->countdown);

        temp *= r; temp += d;
        this->currentValue = jmap (temp, source, this->target);

        return this->currentValue;
    }

    //==============================================================================
    /** Skip the next numSamples samples.

        This is identical to calling getNextValue numSamples times.
        @see getNextValue
    */
    FloatType skip (i32 numSamples) noexcept
    {
        if (numSamples >= this->countdown)
        {
            this->setCurrentAndTargetValue (this->target);
            return this->target;
        }

        this->countdown -= numSamples;

        auto rN = (FloatType) std::pow (r, numSamples);
        temp *= rN;
        temp += d * (rN - (FloatType) 1) / (r - (FloatType) 1);

        this->currentValue = jmap (temp, source, this->target);
        return this->currentValue;
    }

private:
    //==============================================================================
    z0 updateRampParameters()
    {
        auto D = increasingRateOfChange ? B : (FloatType) 1 - B;
        auto base = ((FloatType) 1 / D) - (FloatType) 1;
        r = std::pow (base, (FloatType) 2 / (FloatType) stepsToTarget);
        auto rN = std::pow (r, (FloatType) stepsToTarget);
        d = (r - (FloatType) 1) / (rN - (FloatType) 1);
        temp = 0;
    }

    //==============================================================================
    b8 increasingRateOfChange = true;
    FloatType B = Decibels::decibelsToGain ((FloatType) -40);

    i32 stepsToTarget = 0;
    FloatType temp = 0, source = 0, r = 0, d = 1;
};

} // namespace drx::dsp
