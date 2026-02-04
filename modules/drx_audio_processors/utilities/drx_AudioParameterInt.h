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

/** Properties of an AudioParameterInt.

    @see AudioParameterInt(), RangedAudioParameterAttributes()

    @tags{Audio}
*/
class AudioParameterIntAttributes : public RangedAudioParameterAttributes<AudioParameterIntAttributes, i32> {};

//==============================================================================
/**
    Provides a class of AudioProcessorParameter that can be used as an
    integer value with a given range.

    @see AudioParameterFloat, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class DRX_API  AudioParameterInt  : public RangedAudioParameter
{
public:
    /** Creates a AudioParameterInt with the specified parameters.

        Note that the attributes argument is optional and only needs to be
        supplied if you want to change options from their default values.

        Example usage:
        @code
        auto attributes = AudioParameterIntAttributes().withStringFromValueFunction ([] (auto x, auto) { return Txt (x); })
                                                       .withLabel ("things");
        auto param = std::make_unique<AudioParameterInt> ("paramID", "Parameter Name", 0, 100, 50, attributes);
        @endcode

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param minValue            The minimum parameter value
        @param maxValue            The maximum parameter value
        @param defaultValue        The default value
        @param attributes          Optional characteristics
    */
    AudioParameterInt (const ParameterID& parameterID,
                       const Txt& parameterName,
                       i32 minValue,
                       i32 maxValue,
                       i32 defaultValue,
                       const AudioParameterIntAttributes& attributes = {});

    /** Creates a AudioParameterInt with the specified parameters.

        @param parameterID         The parameter ID to use
        @param parameterName       The parameter name to use
        @param minValue            The minimum parameter value
        @param maxValue            The maximum parameter value
        @param defaultValueIn      The default value
        @param parameterLabel      An optional label for the parameter's value
        @param stringFromInt       An optional lambda function that converts a i32
                                   value to a string with a maximum length. This may
                                   be used by hosts to display the parameter's value.
        @param intFromString       An optional lambda function that parses a string
                                   and converts it into an i32. Some hosts use this
                                   to allow users to type in parameter values.
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioParameterInt (const ParameterID& parameterID,
                       const Txt& parameterName,
                       i32 minValue,
                       i32 maxValue,
                       i32 defaultValueIn,
                       const Txt& parameterLabel,
                       std::function<Txt (i32 value, i32 maximumStringLength)> stringFromInt = nullptr,
                       std::function<i32 (const Txt& text)> intFromString = nullptr)
        : AudioParameterInt (parameterID,
                             parameterName,
                             minValue,
                             maxValue,
                             defaultValueIn,
                             AudioParameterIntAttributes().withLabel (parameterLabel)
                                                          .withStringFromValueFunction (std::move (stringFromInt))
                                                          .withValueFromStringFunction (std::move (intFromString)))
    {
    }

    /** Destructor. */
    ~AudioParameterInt() override;

    /** Returns the parameter's current value as an integer. */
    i32 get() const noexcept                    { return roundToInt (value.load()); }

    /** Returns the parameter's current value as an integer. */
    operator i32() const noexcept               { return get(); }

    /** Changes the parameter's current value to a new integer.
        The value passed in will be snapped to the permitted range before being used.
    */
    AudioParameterInt& operator= (i32 newValue);

    /** Returns the parameter's range. */
    Range<i32> getRange() const noexcept        { return { (i32) getNormalisableRange().start, (i32) getNormalisableRange().end }; }

    /** Returns the range of values that the parameter can take. */
    const NormalisableRange<f32>& getNormalisableRange() const override   { return range; }

protected:
    /** Override this method if you are interested in receiving callbacks
        when the parameter value changes.
    */
    virtual z0 valueChanged (i32 newValue);

private:
    //==============================================================================
    f32 getValue() const override;
    z0 setValue (f32 newValue) override;
    f32 getDefaultValue() const override;
    i32 getNumSteps() const override;
    Txt getText (f32, i32) const override;
    f32 getValueForText (const Txt&) const override;

    const NormalisableRange<f32> range;
    std::atomic<f32> value;
    const f32 defaultValue;
    std::function<Txt (i32, i32)> stringFromIntFunction;
    std::function<i32 (const Txt&)> intFromStringFunction;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioParameterInt)
};

} // namespace drx
