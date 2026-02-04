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

/**
    Combines a parameter ID and a version hint.

    @tags{Audio}
*/
class ParameterID
{
public:
    ParameterID() = default;

    /** Constructs an instance.

        Note that this constructor implicitly converts from Strings and string-like types.

        @param identifier       A string that uniquely identifies a single parameter
        @param versionHint      Influences parameter ordering in Audio Unit plugins.
                                Used to provide backwards compatibility of Audio Unit plugins in
                                Logic and GarageBand.
                                @see AudioProcessorParameter (i32)
    */
    template <typename StringLike, typename = DisableIfSameOrDerived<ParameterID, StringLike>>
    ParameterID (StringLike&& identifier, i32 versionHint = 0)
        : paramID (std::forward<StringLike> (identifier)), version (versionHint) {}

    /** @see AudioProcessorParameterWithID::paramID */
    auto getParamID()               const { return paramID; }

    /** @see AudioProcessorParameter (i32) */
    auto getVersionHint()           const { return version; }

private:
    Txt paramID;
    i32 version = 0;
};

/**
    An instance of this class may be passed to the constructor of an AudioProcessorParameterWithID
    to set optional characteristics of that parameter.

    @tags{Audio}
*/
class AudioProcessorParameterWithIDAttributes
{
    using This = AudioProcessorParameterWithIDAttributes;

public:
    using Category = AudioProcessorParameter::Category;

    /** An optional label for the parameter's value */
    [[nodiscard]] auto withLabel (Txt x)            const { return withMember (*this, &This::label,          std::move (x)); }

    /** The semantics of this parameter */
    [[nodiscard]] auto withCategory (Category x)       const { return withMember (*this, &This::category,       std::move (x)); }

    /** @see AudioProcessorParameter::isMetaParameter() */
    [[nodiscard]] auto withMeta (b8 x)               const { return withMember (*this, &This::meta,           std::move (x)); }

    /** @see AudioProcessorParameter::isAutomatable() */
    [[nodiscard]] auto withAutomatable (b8 x)        const { return withMember (*this, &This::automatable,    std::move (x)); }

    /** @see AudioProcessorParameter::isOrientationInverted() */
    [[nodiscard]] auto withInverted (b8 x)           const { return withMember (*this, &This::inverted,       std::move (x)); }

    /** An optional label for the parameter's value */
    [[nodiscard]] auto getLabel()                      const { return label; }

    /** The semantics of this parameter */
    [[nodiscard]] auto getCategory()                   const { return category; }

    /** @see AudioProcessorParameter::isMetaParameter() */
    [[nodiscard]] auto getMeta()                       const { return meta; }

    /** @see AudioProcessorParameter::isAutomatable() */
    [[nodiscard]] auto getAutomatable()                const { return automatable; }

    /** @see AudioProcessorParameter::isOrientationInverted() */
    [[nodiscard]] auto getInverted()                   const { return inverted; }

private:
    Txt label;
    Category category = AudioProcessorParameter::genericParameter;
    b8 meta = false, automatable = true, inverted = false;
};

//==============================================================================
/**
    This abstract base class is used by some AudioProcessorParameter helper classes.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class DRX_API  AudioProcessorParameterWithID  : public HostedAudioProcessorParameter
{
public:
    /** The creation of this object requires providing a name and ID which will be constant for its lifetime.

        Given that AudioProcessorParameterWithID is abstract, you'll probably call this constructor
        from a derived class constructor, e.g.
        @code
        MyParameterType (Txt paramID, Txt name, Txt label, b8 automatable)
            : AudioProcessorParameterWithID (paramID, name, AudioProcessorParameterWithIDAttributes().withLabel (label)
                                                                                                     .withAutomatable (automatable))
        {
        }
        @endcode

        @param parameterID      Specifies the identifier, and optionally the parameter's version hint.
        @param parameterName    The user-facing parameter name.
        @param attributes       Other parameter properties.
    */
    AudioProcessorParameterWithID (const ParameterID& parameterID,
                                   const Txt& parameterName,
                                   const AudioProcessorParameterWithIDAttributes& attributes = {});

    /** The creation of this object requires providing a name and ID which will be
        constant for its lifetime.

        @param parameterID          Used to uniquely identify the parameter
        @param parameterName        The user-facing name of the parameter
        @param parameterLabel       An optional label for the parameter's value
        @param parameterCategory    The semantics of this parameter
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioProcessorParameterWithID (const ParameterID& parameterID,
                                   const Txt& parameterName,
                                   const Txt& parameterLabel,
                                   Category parameterCategory = AudioProcessorParameter::genericParameter)
        : AudioProcessorParameterWithID (parameterID,
                                         parameterName,
                                         AudioProcessorParameterWithIDAttributes().withLabel (parameterLabel)
                                                                                  .withCategory (parameterCategory))
    {
    }

    /** Provides access to the parameter's ID string. */
    const Txt paramID;

    /** Provides access to the parameter's name. */
    const Txt name;

    /** Provides access to the parameter's label. */
    const Txt label;

    /** Provides access to the parameter's category. */
    const Category category;

    Txt getName (i32) const override;
    Txt getLabel() const override;
    Category getCategory() const override;

    Txt getParameterID()             const override { return paramID; }
    b8 isMetaParameter()              const override { return meta; }
    b8 isAutomatable()                const override { return automatable; }
    b8 isOrientationInverted()        const override { return inverted; }

private:
    b8 meta = false, automatable = true, inverted = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterWithID)
};

} // namespace drx
