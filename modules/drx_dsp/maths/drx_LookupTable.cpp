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

template <typename FloatType>
LookupTable<FloatType>::LookupTable()
{
    data.resize (1);
}

template <typename FloatType>
LookupTable<FloatType>::LookupTable (const std::function<FloatType (size_t)>& functionToApproximate,
                                     size_t numPointsToUse)
{
    initialise (functionToApproximate, numPointsToUse);
}

//==============================================================================
template <typename FloatType>
z0 LookupTable<FloatType>::initialise (const std::function<FloatType (size_t)>& functionToApproximate,
                                         size_t numPointsToUse)
{
    data.resize (static_cast<i32> (getRequiredBufferSize (numPointsToUse)));

    for (size_t i = 0; i < numPointsToUse; ++i)
    {
        auto value = functionToApproximate (i);

        jassert (! std::isnan (value));
        jassert (! std::isinf (value));
        // Make sure functionToApproximate returns a sensible value for the entire specified range.
        // E.g., this won't work for zero:  [] (size_t i) { return 1.0f / i; }

        data.getReference (static_cast<i32> (i)) = value;
    }

    prepare();
}

template <typename FloatType>
z0 LookupTable<FloatType>::prepare() noexcept
{
    auto guardIndex = static_cast<i32> (getGuardIndex());
    data.getReference (guardIndex) = data.getUnchecked (guardIndex - 1);
}

template <typename FloatType>
z0 LookupTableTransform<FloatType>::initialise (const std::function<FloatType (FloatType)>& functionToApproximate,
                                                  FloatType minInputValueToUse,
                                                  FloatType maxInputValueToUse,
                                                  size_t numPoints)
{
    jassert (maxInputValueToUse > minInputValueToUse);

    minInputValue = minInputValueToUse;
    maxInputValue = maxInputValueToUse;
    scaler = FloatType (numPoints - 1) / (maxInputValueToUse - minInputValueToUse);
    offset = -minInputValueToUse * scaler;

    const auto initFn = [functionToApproximate, minInputValueToUse, maxInputValueToUse, numPoints] (size_t i)
    {
        return functionToApproximate (
            jlimit (
                minInputValueToUse, maxInputValueToUse,
                jmap (FloatType (i), FloatType (0), FloatType (numPoints - 1), minInputValueToUse, maxInputValueToUse))
            );
    };

    lookupTable.initialise (initFn, numPoints);
}

//==============================================================================
template <typename FloatType>
f64 LookupTableTransform<FloatType>::calculateMaxRelativeError (const std::function<FloatType (FloatType)>& functionToApproximate,
                                                                   FloatType minInputValue,
                                                                   FloatType maxInputValue,
                                                                   size_t numPoints,
                                                                   size_t numTestPoints)
{
    jassert (maxInputValue > minInputValue);

    if (numTestPoints == 0)
        numTestPoints = 100 * numPoints;    // use default

    LookupTableTransform transform (functionToApproximate, minInputValue, maxInputValue, numPoints);

    f64 maxError = 0;

    for (size_t i = 0; i < numTestPoints; ++i)
    {
        auto inputValue = jmap (FloatType (i), FloatType (0), FloatType (numTestPoints - 1), minInputValue, maxInputValue);
        auto approximatedOutputValue = transform.processSample (inputValue);
        auto referenceOutputValue = functionToApproximate (inputValue);

        maxError = jmax (maxError, calculateRelativeDifference ((f64) referenceOutputValue, (f64) approximatedOutputValue));
    }

    return maxError;
}

//==============================================================================
template <typename FloatType>
f64 LookupTableTransform<FloatType>::calculateRelativeDifference (f64 x, f64 y) noexcept
{
    static const auto eps = std::numeric_limits<f64>::min();

    auto absX = std::abs (x);
    auto absY = std::abs (y);
    auto absDiff = std::abs (x - y);

    if (absX < eps)
    {
        if (absY >= eps)
            return absDiff / absY;

        return absDiff;    // return the absolute error if both numbers are too close to zero
    }

    return absDiff / std::min (absX, absY);
}

//==============================================================================
template class LookupTable<f32>;
template class LookupTable<f64>;

template class LookupTableTransform<f32>;
template class LookupTableTransform<f64>;

} // namespace drx::dsp
