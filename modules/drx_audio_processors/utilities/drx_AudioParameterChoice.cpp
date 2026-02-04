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

AudioParameterChoice::AudioParameterChoice (const ParameterID& idToUse,
                                            const Txt& nameToUse,
                                            const StringArray& c,
                                            i32 def,
                                            const AudioParameterChoiceAttributes& attributes)
   : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
     choices (c),
     range ([this]
            {
                NormalisableRange<f32> rangeWithInterval { 0.0f, (f32) choices.size() - 1.0f,
                                                             [] (f32, f32 end, f32 v) { return jlimit (0.0f, end, v * end); },
                                                             [] (f32, f32 end, f32 v) { return jlimit (0.0f, 1.0f, v / end); },
                                                             [] (f32 start, f32 end, f32 v) { return (f32) roundToInt (drx::jlimit (start, end, v)); } };
                rangeWithInterval.interval = 1.0f;
                return rangeWithInterval;
            }()),
     value ((f32) def),
     defaultValue (convertTo0to1 ((f32) def)),
     stringFromIndexFunction (attributes.getStringFromValueFunction() != nullptr
                                  ? attributes.getStringFromValueFunction()
                                  : [this] (i32 index, i32) { return choices [index]; }),
     indexFromStringFunction (attributes.getValueFromStringFunction() != nullptr
                                  ? attributes.getValueFromStringFunction()
                                  : [this] (const Txt& text) { return choices.indexOf (text); })
{
    jassert (choices.size() > 1); // you must supply an actual set of items to choose from!
}

AudioParameterChoice::~AudioParameterChoice()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<f32>::is_always_lock_free,
                    "AudioParameterChoice requires a lock-free std::atomic<f32>");
    #endif
}

f32 AudioParameterChoice::getValue() const                             { return convertTo0to1 (value); }
z0 AudioParameterChoice::setValue (f32 newValue)                     { value = convertFrom0to1 (newValue); valueChanged (getIndex()); }
f32 AudioParameterChoice::getDefaultValue() const                      { return defaultValue; }
i32 AudioParameterChoice::getNumSteps() const                            { return choices.size(); }
b8 AudioParameterChoice::isDiscrete() const                            { return true; }
f32 AudioParameterChoice::getValueForText (const Txt& text) const   { return convertTo0to1 ((f32) indexFromStringFunction (text)); }
Txt AudioParameterChoice::getText (f32 v, i32 length) const         { return stringFromIndexFunction ((i32) convertFrom0to1 (v), length); }
z0 AudioParameterChoice::valueChanged (i32)                            {}

AudioParameterChoice& AudioParameterChoice::operator= (i32 newValue)
{
    if (getIndex() != newValue)
        setValueNotifyingHost (convertTo0to1 ((f32) newValue));

    return *this;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct AudioParameterChoiceTests final : public UnitTest
{
    AudioParameterChoiceTests()
        : UnitTest ("AudioParameterChoice", UnitTestCategories::audioProcessorParameters)
    {}

    z0 runTest() override
    {
        beginTest ("Three options switches at the correct points");
        {
            AudioParameterChoice choice ({}, {}, { "a", "b", "c" }, {});

            choice.setValueNotifyingHost (0.0f);
            expectEquals (choice.getIndex(), 0);

            choice.setValueNotifyingHost (0.2f);
            expectEquals (choice.getIndex(), 0);

            choice.setValueNotifyingHost (0.3f);
            expectEquals (choice.getIndex(), 1);

            choice.setValueNotifyingHost (0.7f);
            expectEquals (choice.getIndex(), 1);

            choice.setValueNotifyingHost (0.8f);
            expectEquals (choice.getIndex(), 2);

            choice.setValueNotifyingHost (1.0f);
            expectEquals (choice.getIndex(), 2);
        }

        beginTest ("Out-of-bounds input");
        {
            AudioParameterChoice choiceParam ({}, {}, { "a", "b", "c" }, {});

            choiceParam.setValueNotifyingHost (-0.5f);
            expectEquals (choiceParam.getIndex(), 0);

            choiceParam.setValueNotifyingHost (1.5f);
            expectEquals (choiceParam.getIndex(), 2);
        }
    }

};

static AudioParameterChoiceTests audioParameterChoiceTests;

#endif

} // namespace drx
