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

//==============================================================================
/**
    A class that measures various statistics about a series of floating point
    values that it is given.

    @tags{Core}
*/
template <typename FloatType>
class StatisticsAccumulator
{
public:
    //==============================================================================
    /** Constructs a new StatisticsAccumulator. */
    StatisticsAccumulator() = default;

    //==============================================================================
    /** Add a new value to the accumulator.
        This will update all running statistics accordingly.
    */
    z0 addValue (FloatType v) noexcept
    {
        jassert (drx_isfinite (v));

        sum += v;
        sumSquares += v * v;
        ++count;

        if (v > maximum) maximum = v;
        if (v < minimum) minimum = v;
    }

    /** Reset the accumulator.
        This will reset all currently saved statistcs.
    */
    z0 reset() noexcept               { *this = StatisticsAccumulator<FloatType>(); }

    //==============================================================================
    /** Returns the average (arithmetic mean) of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getAverage() const noexcept
    {
        return count > 0 ? sum / (FloatType) count
                         : FloatType();
    }

    /** Returns the variance of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getVariance() const noexcept
    {
        return count > 0 ? (sumSquares - sum * sum / (FloatType) count) / (FloatType) count
                         : FloatType();
    }

    /** Returns the standard deviation of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getStandardDeviation() const noexcept
    {
        return std::sqrt (getVariance());
    }

    /** Returns the smallest of all previously added values.
        If no values have been added yet, this will return positive infinity.
    */
    FloatType getMinValue() const noexcept
    {
        return minimum;
    }

    /** Returns the largest of all previously added values.
        If no values have been added yet, this will return negative infinity.
    */
    FloatType getMaxValue() const noexcept
    {
        return maximum;
    }

    /** Returns how many values have been added to this accumulator. */
    size_t getCount() const noexcept
    {
        return count;
    }

private:
    //==============================================================================
    struct KahanSum
    {
        KahanSum() = default;
        operator FloatType() const noexcept             { return sum; }

        z0 DRX_NO_ASSOCIATIVE_MATH_OPTIMISATIONS operator+= (FloatType value) noexcept
        {
            FloatType correctedValue = value - error;
            FloatType newSum = sum + correctedValue;
            error = (newSum - sum) - correctedValue;
            sum = newSum;
        }

        FloatType sum{}, error{};
    };

    //==============================================================================
    size_t count { 0 };
    KahanSum sum, sumSquares;
    FloatType minimum {  std::numeric_limits<FloatType>::infinity() },
              maximum { -std::numeric_limits<FloatType>::infinity() };
};

} // namespace drx
