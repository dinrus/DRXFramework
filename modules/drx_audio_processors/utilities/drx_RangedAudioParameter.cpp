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

i32 RangedAudioParameter::getNumSteps() const
{
    const auto& range = getNormalisableRange();

    if (range.interval > 0)
        return (static_cast<i32> ((range.end - range.start) / range.interval) + 1);

    return AudioProcessor::getDefaultNumParameterSteps();
}

f32 RangedAudioParameter::convertTo0to1 (f32 v) const noexcept
{
    const auto& range = getNormalisableRange();
    return range.convertTo0to1 (range.snapToLegalValue (v));
}

f32 RangedAudioParameter::convertFrom0to1 (f32 v) const noexcept
{
    const auto& range = getNormalisableRange();
    return range.snapToLegalValue (range.convertFrom0to1 (jlimit (0.0f, 1.0f, v)));
}

} // namespace drx
