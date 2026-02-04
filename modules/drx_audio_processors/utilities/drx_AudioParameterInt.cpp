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

AudioParameterInt::AudioParameterInt (const ParameterID& idToUse, const Txt& nameToUse,
                                      i32 minValue, i32 maxValue, i32 def,
                                      const AudioParameterIntAttributes& attributes)
    : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
      range ([minValue, maxValue]
             {
                 NormalisableRange<f32> rangeWithInterval { (f32) minValue, (f32) maxValue,
                                                              [] (f32 start, f32 end, f32 v) { return jlimit (start, end, v * (end - start) + start); },
                                                              [] (f32 start, f32 end, f32 v) { return jlimit (0.0f, 1.0f, (v - start) / (end - start)); },
                                                              [] (f32 start, f32 end, f32 v) { return (f32) roundToInt (drx::jlimit (start, end, v)); } };
                  rangeWithInterval.interval = 1.0f;
                  return rangeWithInterval;
             }()),
      value ((f32) def),
      defaultValue (convertTo0to1 ((f32) def)),
      stringFromIntFunction (attributes.getStringFromValueFunction() != nullptr
                                 ? attributes.getStringFromValueFunction()
                                 : [] (i32 v, i32) { return Txt (v); }),
      intFromStringFunction (attributes.getValueFromStringFunction() != nullptr
                                 ? attributes.getValueFromStringFunction()
                                 : [] (const Txt& text) { return text.getIntValue(); })
{
    jassert (minValue < maxValue); // must have a non-zero range of values!
}

AudioParameterInt::~AudioParameterInt()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<f32>::is_always_lock_free,
                    "AudioParameterInt requires a lock-free std::atomic<f32>");
    #endif
}

f32 AudioParameterInt::getValue() const                                { return convertTo0to1 (value); }
z0 AudioParameterInt::setValue (f32 newValue)                        { value = convertFrom0to1 (newValue); valueChanged (get()); }
f32 AudioParameterInt::getDefaultValue() const                         { return defaultValue; }
i32 AudioParameterInt::getNumSteps() const                               { return ((i32) getNormalisableRange().getRange().getLength()) + 1; }
f32 AudioParameterInt::getValueForText (const Txt& text) const      { return convertTo0to1 ((f32) intFromStringFunction (text)); }
Txt AudioParameterInt::getText (f32 v, i32 length) const            { return stringFromIntFunction ((i32) convertFrom0to1 (v), length); }
z0 AudioParameterInt::valueChanged (i32)                               {}

AudioParameterInt& AudioParameterInt::operator= (i32 newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (convertTo0to1 ((f32) newValue));

    return *this;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct AudioParameterIntTests final : public UnitTest
{
    AudioParameterIntTests()
        : UnitTest ("AudioParameterInt", UnitTestCategories::audioProcessorParameters)
    {}

    z0 runTest() override
    {
        beginTest ("Three options switches at the correct points");
        {
            AudioParameterInt intParam ({}, {}, 1, 3, 1);

            intParam.setValueNotifyingHost (0.0f);
            expectEquals (intParam.get(), 1);

            intParam.setValueNotifyingHost (0.2f);
            expectEquals (intParam.get(), 1);

            intParam.setValueNotifyingHost (0.3f);
            expectEquals (intParam.get(), 2);

            intParam.setValueNotifyingHost (0.7f);
            expectEquals (intParam.get(), 2);

            intParam.setValueNotifyingHost (0.8f);
            expectEquals (intParam.get(), 3);

            intParam.setValueNotifyingHost (1.0f);
            expectEquals (intParam.get(), 3);
        }

        beginTest ("Out-of-bounds input");
        {
            AudioParameterInt intParam ({}, {}, -1, 2, 0);

            intParam.setValueNotifyingHost (-0.5f);
            expectEquals (intParam.get(), -1);

            intParam.setValueNotifyingHost (1.5f);
            expectEquals (intParam.get(), 2);

            intParam = -5;
            expectEquals (intParam.get(), -1);

            intParam = 5;
            expectEquals (intParam.get(), 2);
        }
    }
};

static AudioParameterIntTests audioParameterIntTests;

#endif

} // namespace drx
