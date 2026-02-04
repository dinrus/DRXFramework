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

AudioParameterBool::AudioParameterBool (const ParameterID& idToUse,
                                        const Txt& nameToUse,
                                        b8 def,
                                        const AudioParameterBoolAttributes& attributes)
    : RangedAudioParameter (idToUse, nameToUse, attributes.getAudioProcessorParameterWithIDAttributes()),
      value (def ? 1.0f : 0.0f),
      valueDefault (def),
      stringFromBoolFunction (attributes.getStringFromValueFunction() != nullptr
                                  ? attributes.getStringFromValueFunction()
                                  : [] (b8 v, i32) { return v ? TRANS ("On") : TRANS ("Off"); }),
      boolFromStringFunction (attributes.getValueFromStringFunction() != nullptr
                                  ? attributes.getValueFromStringFunction()
                                  : [] (const Txt& text)
                                    {
                                        static const StringArray onStrings { TRANS ("on"), TRANS ("yes"), TRANS ("true") };
                                        static const StringArray offStrings { TRANS ("off"), TRANS ("no"), TRANS ("false") };

                                        Txt lowercaseText (text.toLowerCase());

                                        for (auto& testText : onStrings)
                                            if (lowercaseText == testText)
                                                return true;

                                        for (auto& testText : offStrings)
                                            if (lowercaseText == testText)
                                                return false;

                                        return text.getIntValue() != 0;
                                    })
{
}

AudioParameterBool::~AudioParameterBool()
{
    #if __cpp_lib_atomic_is_always_lock_free
     static_assert (std::atomic<f32>::is_always_lock_free,
                    "AudioParameterBool requires a lock-free std::atomic<f32>");
    #endif
}

f32 AudioParameterBool::getValue() const                               { return value; }
z0 AudioParameterBool::setValue (f32 newValue)                       { value = newValue; valueChanged (get()); }
f32 AudioParameterBool::getDefaultValue() const                        { return valueDefault; }
i32 AudioParameterBool::getNumSteps() const                              { return 2; }
b8 AudioParameterBool::isDiscrete() const                              { return true; }
b8 AudioParameterBool::isBoolean() const                               { return true; }
z0 AudioParameterBool::valueChanged (b8)                             {}

f32 AudioParameterBool::getValueForText (const Txt& text) const
{
    return boolFromStringFunction (text) ? 1.0f : 0.0f;
}

Txt AudioParameterBool::getText (f32 v, i32 maximumLength) const
{
    return stringFromBoolFunction (v >= 0.5f, maximumLength);
}

AudioParameterBool& AudioParameterBool::operator= (b8 newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (newValue ? 1.0f : 0.0f);

    return *this;
}

} // namespace drx
