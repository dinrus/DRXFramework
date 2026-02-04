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

/** Properties of an AudioParameterBool.

    @see AudioParameterBool(), RangedAudioParameterAttributes()

    @tags{Audio}
*/
class AudioParameterBoolAttributes : public RangedAudioParameterAttributes<AudioParameterBoolAttributes, b8> {};

//==============================================================================
/**
    Provides a class of AudioProcessorParameter that can be used as a boolean value.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterChoice

    @tags{Audio}
*/
class DRX_API AudioParameterBool  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterBool with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterBoolAttributes().withStringFromValueFunction ([] (auto x, auto) { return x ? "On" : "Off"; })
                                                        .withLabel ("enabled");
        auto param = std::make_unique<AudioParameterBool> ("paramID", "Parameter Name", false, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param defaultValue        The default value
        @param attributes          Optional characteristics
    */
    AudioParameterBool (const ParameterID& parameterID,
                        const Txt& parameterName,
                        b8 defaultValue,
                        const AudioParameterBoolAttributes& attributes = {});

    /** Creates a AudioParameterBool with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param defaultValue        The default value
        @param parameterLabel      An optional label for the parameter's value
        @param stringFromBool      An optional lambda function that converts a b8
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param boolFromString      An optional lambda function that parses a string and
                                   converts it into a b8 value. Some hosts use this
                                   to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterBool (const ParameterID& parameterID,
                        const Txt& parameterName,
                        b8 defaultValue,
                        const Txt& parameterLabel,
                        std::function<Txt (b8 value, i32 maximumStringLength)> stringFromBool = nullptr,
                        std::function<b8 (const Txt& text)> boolFromString = nullptr)
        : AudioParameterBool (parameterID,
                              parameterName,
                              defaultValue,
                              AudioParameterBoolAttributes().withLabel (parameterLabel)
                                                            .withStringFromValueFunction (std::move (stringFromBool))
                                                            .withValueFromStringFunction (std::move (boolFromString)))
    {
    }

    /** Destructor. */
    ~AudioParameterBool() override;

    /** Returns the parameter's current boolean value. */
    b8 get() const noexcept          { return value >= 0.5f; }

    /** Returns the parameter's current boolean value. */
    operator b8() const noexcept     { return get(); }

    /** Changes the parameter's current value to a new boolean. */
    AudioParameterBool& operator= (b8 newValue);

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<f32>& getNormalisableRange() const override   { return range; }

protected:
    /** Override this method if you are interested in receiving callbacks
        when the parameter value changes.
    */
    virtual z0 valueChanged (b8 newValue);

private:
    //==============================================================================
    f32 getValue() const override;
    z0 setValue (f32 newValue) override;
    f32 getDefaultValue() const override;
    i32 getNumSteps() const override;
    b8 isDiscrete() const override;
    b8 isBoolean() const override;
    Txt getText (f32, i32) const override;
    f32 getValueForText (const Txt&) const override;

    const NormalisableRange<f32> range { 0.0f, 1.0f, 1.0f };
    std::atomic<f32> value;
    const f32 valueDefault;
    std::function<Txt (b8, i32)> stringFromBoolFunction;
    std::function<b8 (const Txt&)> boolFromStringFunction;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterBool)
};

} // namespace drx
