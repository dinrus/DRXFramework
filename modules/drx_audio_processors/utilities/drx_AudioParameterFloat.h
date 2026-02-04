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

/** Properties of an AudioParameterFloat.

    @see AudioParameterFloat(), RangedAudioParameterAttributes()

    @tags{Audio}
*/
class AudioParameterFloatAttributes : public RangedAudioParameterAttributes<AudioParameterFloatAttributes, f32> {};

//==============================================================================
/**
    A subclass of AudioProcessorParameter that provides an easy way to create a
    parameter which maps onto a given NormalisableRange.

    @see AudioParameterInt, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class DRX_API  AudioParameterFloat  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterFloat with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterFloatAttributes().withStringFromValueFunction ([] (auto x, auto) { return Txt (x * 100); })
                                                         .withLabel ("%");
        auto param = std::make_unique<AudioParameterFloat> ("paramID", "Parameter Name", NormalisableRange<f32>(), 0.5f, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param normalisableRange   The NormalisableRange to use
        @param defaultValue        The non-normalised default value
        @param attributes          Optional characteristics
    */
    AudioParameterFloat (const ParameterID& parameterID,
                         const Txt& parameterName,
                         NormalisableRange<f32> normalisableRange,
                         f32 defaultValue,
                         const AudioParameterFloatAttributes& attributes = {});

    /** Creates a AudioParameterFloat with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param normalisableRange   The NormalisableRange to use
        @param defaultValue        The non-normalised default value
        @param parameterLabel      An optional label for the parameter's value
        @param parameterCategory   An optional parameter category
        @param stringFromValue     An optional lambda function that converts a non-normalised
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param valueFromString     An optional lambda function that parses a string and
                                   converts it into a non-normalised value. Some hosts use
                                   this to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterFloat (const ParameterID& parameterID,
                         const Txt& parameterName,
                         NormalisableRange<f32> normalisableRange,
                         f32 defaultValue,
                         const Txt& parameterLabel,
                         Category parameterCategory = AudioProcessorParameter::genericParameter,
                         std::function<Txt (f32 value, i32 maximumStringLength)> stringFromValue = nullptr,
                         std::function<f32 (const Txt& text)> valueFromString = nullptr)
        : AudioParameterFloat (parameterID,
                               parameterName,
                               std::move (normalisableRange),
                               defaultValue,
                               AudioParameterFloatAttributes().withLabel (parameterLabel)
                                                              .withCategory (parameterCategory)
                                                              .withStringFromValueFunction (std::move (stringFromValue))
                                                              .withValueFromStringFunction (std::move (valueFromString)))
    {
    }

    /** Creates a AudioParameterFloat with an ID, name, and range.
        On creation, its value is set to the default value.
        For control over skew factors, you can use the other
        constructor and provide a NormalisableRange.
    */
    AudioParameterFloat (const ParameterID& parameterID,
                         const Txt& parameterName,
                         f32 minValue,
                         f32 maxValue,
                         f32 defaultValue);

    /** Destructor. */
    ~AudioParameterFloat() override;

    /** Returns the parameter's current value. */
    f32 get() const noexcept                  { return value; }

    /** Returns the parameter's current value. */
    operator f32() const noexcept             { return value; }

    /** Changes the parameter's current value. */
    AudioParameterFloat& operator= (f32 newValue);

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<f32>& getNormalisableRange() const override   { return range; }

    /** Provides access to the parameter's range. */
    NormalisableRange<f32> range;

protected:
    /** Override this method if you are interested in receiving callbacks
        when the parameter value changes.
    */
    virtual z0 valueChanged (f32 newValue);

private:
    //==============================================================================
    f32 getValue() const override;
    z0 setValue (f32 newValue) override;
    f32 getDefaultValue() const override;
    i32 getNumSteps() const override;
    Txt getText (f32, i32) const override;
    f32 getValueForText (const Txt&) const override;

    std::atomic<f32> value;
    const f32 valueDefault;
    std::function<Txt (f32, i32)> stringFromValueFunction;
    std::function<f32 (const Txt&)> valueFromStringFunction;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterFloat)
};

} // namespace drx
