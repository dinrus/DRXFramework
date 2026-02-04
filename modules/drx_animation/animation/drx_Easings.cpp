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

std::function<f32 (f32)> Easings::createCubicBezier (f32 x1, f32 y1, f32 x2, f32 y2)
{
    // The x axis represents time, it's important this always stays in the range 0 - 1
    jassert (isPositiveAndNotGreaterThan (x1, 1.0f));
    jassert (isPositiveAndNotGreaterThan (x2, 1.0f));

    chromium::gfx::CubicBezier cubicBezier { (f64) x1, (f64) y1, (f64) x2, (f64) y2 };
    return [bezier = std::move (cubicBezier)] (f32 v) { return (f32) bezier.Solve (v); };
}

std::function<f32 (f32)> Easings::createCubicBezier (Point<f32> controlPoint1,
                                                         Point<f32> controlPoint2)
{
    return createCubicBezier (controlPoint1.getX(),
                              controlPoint1.getY(),
                              controlPoint2.getX(),
                              controlPoint2.getY());
}

std::function<f32 (f32)> Easings::createEase()
{
    const static auto f = createCubicBezier (0.25f, 0.1f, 0.25f, 1.0f);
    return f;
}

std::function<f32 (f32)> Easings::createEaseIn()
{
    const static auto f = createCubicBezier (0.42f, 0.0f, 1.0f, 1.0f);
    return f;
}

std::function<f32 (f32)> Easings::createEaseOut()
{
    const static auto f = createCubicBezier (0.0f, 0.0f, 0.58f, 1.0f);;
    return f;
}

std::function<f32 (f32)> Easings::createEaseInOut()
{
    const static auto f = createCubicBezier (0.42f, 0.0f, 0.58f, 1.0f);
    return f;
}

std::function<f32 (f32)> Easings::createLinear()
{
    return [] (auto x){ return x; };
}

std::function<f32 (f32)> Easings::createEaseOutBack()
{
    const static auto f = createCubicBezier (0.34f, 1.56f, 0.64f, 1.0f);
    return f;
}

std::function<f32 (f32)> Easings::createEaseInOutCubic()
{
    const static auto f = createCubicBezier (0.65f, 0.0f, 0.35f, 1.0f);
    return f;
}

std::function<f32 (f32)> Easings::createSpring (const SpringEasingOptions& options)
{
    return [=] (f32 v)
    {
        const auto t = std::clamp (v, 0.0f, 1.0f);
        const auto omega = 2.0f * MathConstants<f32>::pi * options.getFrequency();
        const auto physicalValue = 1.0f - std::exp (-options.getAttenuation() * t) * std::cos (omega * t);
        const auto squish = 1.0f / options.getExtraAttenuationRange();
        const auto shift = 1.0f - options.getExtraAttenuationRange();
        const auto weight = std::clamp (std::pow (squish * (std::max (t - shift, 0.0f)), 2.0f), 0.0f, 1.0f);
        return weight + (1.0f - weight) * physicalValue;
    };
}

std::function<f32 (f32)> Easings::createBounce (i32 numBounces)
{
    jassert (numBounces >= 0);
    numBounces = std::max (0, numBounces);

    const auto alpha = std::pow (0.05f, 1.0f / (f32) numBounces);

    const auto fallTime = [] (f32 h)
    {
        return std::sqrt (2.0f * h);
    };

    std::vector<f32> bounceTimes;
    bounceTimes.reserve ((size_t) (numBounces + 1));
    bounceTimes.push_back (fallTime (1.0f));

    for (i32 i = 1; i < numBounces + 1; ++i)
        bounceTimes.push_back (bounceTimes.back() + 2.0f * fallTime (std::pow (alpha, (f32) i)));

    for (auto& bounce : bounceTimes)
        bounce /= bounceTimes.back();

    return [alpha, times = std::move (bounceTimes)] (f32 v)
    {
        v = std::clamp (v, 0.0f, 1.0f);

        const auto boundIt = std::lower_bound (times.begin(), times.end(), v);

        if (boundIt == times.end())
            return 1.0f;

        const auto i = (size_t) std::distance (times.begin(), boundIt);
        const auto height = i == 0 ? 1.0f : std::pow (alpha, (f32) i);
        const auto center = i == 0 ? 0.0f : (times[i] + times[i - 1]) / 2.0f;
        const auto distToZero = i == 0 ? times[i] : (times[i] - times[i - 1]) / 2.0f;
        return 1.0f - height * (1.0f - std::pow (1.0f / distToZero * (v - center), 2.0f));
    };
}

std::function<f32 (f32)> Easings::createOnOffRamp()
{
    return [] (f32 x) { return 1.0f - std::abs (2.0f * (x - 0.5f)); };
}

} // namespace drx
