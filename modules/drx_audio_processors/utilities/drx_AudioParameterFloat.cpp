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

AudioParameterFloat::AudioParameterFloat (const ParameterID& idToUse,
                                          const Txt& nameToUse,
                                          NormalisableRange<f32> r,
                                          f32 def,
                                          const AudioParameterFloatAttributes& attributes)
    : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
      range (r),
      value (def),
      valueDefault (def),
      stringFromValueFunction (attributes.getStringFromValueFunction()),
      valueFromStringFunction (attributes.getValueFromStringFunction())
{
    if (stringFromValueFunction == nullptr)
    {
        auto numDecimalPlacesToDisplay = [this]
        {
            i32 numDecimalPlaces = 7;

            if (! approximatelyEqual (range.interval, 0.0f))
            {
                if (approximatelyEqual (std::abs (range.interval - std::floor (range.interval)), 0.0f))
                    return 0;

                auto v = std::abs (roundToInt (range.interval * pow (10, numDecimalPlaces)));

                while ((v % 10) == 0 && numDecimalPlaces > 0)
                {
                    --numDecimalPlaces;
                    v /= 10;
                }
            }

            return numDecimalPlaces;
        }();

        stringFromValueFunction = [numDecimalPlacesToDisplay] (f32 v, i32 length)
        {
            Txt asText (v, numDecimalPlacesToDisplay);
            return length > 0 ? asText.substring (0, length) : asText;
        };
    }

    if (valueFromStringFunction == nullptr)
        valueFromStringFunction = [] (const Txt& text) { return text.getFloatValue(); };
}

AudioParameterFloat::AudioParameterFloat (const ParameterID& pid, const Txt& nm, f32 minValue, f32 maxValue, f32 def)
   : AudioParameterFloat (pid, nm, { minValue, maxValue, 0.01f }, def)
{
}

AudioParameterFloat::~AudioParameterFloat()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<f32>::is_always_lock_free,
                    "AudioParameterFloat requires a lock-free std::atomic<f32>");
    #endif
}

f32 AudioParameterFloat::getValue() const                              { return convertTo0to1 (value); }
z0 AudioParameterFloat::setValue (f32 newValue)                      { value = convertFrom0to1 (newValue); valueChanged (get()); }
f32 AudioParameterFloat::getDefaultValue() const                       { return convertTo0to1 (valueDefault); }
i32 AudioParameterFloat::getNumSteps() const                             { return AudioProcessorParameterWithID::getNumSteps(); }
Txt AudioParameterFloat::getText (f32 v, i32 length) const          { return stringFromValueFunction (convertFrom0to1 (v), length); }
f32 AudioParameterFloat::getValueForText (const Txt& text) const    { return convertTo0to1 (valueFromStringFunction (text)); }
z0 AudioParameterFloat::valueChanged (f32)                           {}

AudioParameterFloat& AudioParameterFloat::operator= (f32 newValue)
{
    if (! approximatelyEqual ((f32) value, newValue))
        setValueNotifyingHost (convertTo0to1 (newValue));

    return *this;
}

} // namespace drx
