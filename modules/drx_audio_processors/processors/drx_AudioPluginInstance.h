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

// MSVC does not like it if you override a deprecated method even if you
// keep the deprecation attribute. Other compilers are more forgiving.
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4996)

//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the AudioProcessor class, and adds some extra functionality
    that helps when wrapping dynamically loaded plugins.

    This class is not needed when writing plugins, and you should never need to derive
    your own sub-classes from it. The plugin hosting classes use it internally and will
    return AudioPluginInstance objects which wrap external plugins.

    @see AudioProcessor, AudioPluginFormat

    @tags{Audio}
*/
class DRX_API  AudioPluginInstance   : public AudioProcessor
{
public:
    //==============================================================================
    /** Destructor.

        Make sure that you delete any UI components that belong to this plugin before
        deleting the plugin.
    */
    ~AudioPluginInstance() override = default;

    //==============================================================================
    /** Fills-in the appropriate parts of this plugin description object. */
    virtual z0 fillInPluginDescription (PluginDescription&) const = 0;

    /** Returns a PluginDescription for this plugin.
        This is just a convenience method to avoid calling fillInPluginDescription.
    */
    PluginDescription getPluginDescription() const;

    /** Allows retrieval of information related to the inner workings of a particular plugin format,
        such as the AEffect* of a VST, or the handle of an AudioUnit.

        To use this, create a new class derived from ExtensionsVisitor, and override
        each of the visit member functions. If this AudioPluginInstance wraps a VST3 plugin
        the visitVST3() member will be called, while if the AudioPluginInstance wraps an
        unknown format the visitUnknown() member will be called. The argument of the visit function
        can be queried to extract information related to the AudioPluginInstance's implementation.
    */
    virtual z0 getExtensions (ExtensionsVisitor&) const;

    using HostedParameter = HostedAudioProcessorParameter;

    /** Adds a parameter to this instance.

        @see AudioProcessor::addParameter()
    */
    z0 addHostedParameter (std::unique_ptr<HostedParameter>);

    /** Adds multiple parameters to this instance.

        In debug mode, this will also check that all added parameters derive from
        HostedParameter.

        @see AudioProcessor::addParameterGroup()
    */
    z0 addHostedParameterGroup (std::unique_ptr<AudioProcessorParameterGroup>);

    /** Adds multiple parameters to this instance.

        In debug mode, this will also check that all added parameters derive from
        HostedParameter.

        @see AudioProcessor::setParameterTree()
    */
    z0 setHostedParameterTree (AudioProcessorParameterGroup);

    /** Gets the parameter at a particular index.

        If you want to find lots of parameters by their IDs, you should probably build and
        use a map<Txt, HostedParameter*> by looping through all parameters.
    */
    HostedParameter* getHostedParameter (i32 index) const;

   #ifndef DOXYGEN
    /** Use the new typesafe visitor-based interface rather than this function.

        Returns a pointer to some kind of platform-specific data about the plugin.
        E.g. For a VST, this value can be cast to an AEffect*. For an AudioUnit, it can be
        cast to an AudioUnit handle.
    */
    [[deprecated ("Use the new typesafe visitor-based interface rather than this function.")]]
    virtual uk getPlatformSpecificData();

    // Rather than using these methods you should call the corresponding methods
    // on the AudioProcessorParameter objects returned from getParameters().
    // See the implementations of the methods below for some examples of how to
    // do this.
    //
    // In addition to being marked as deprecated these methods will assert on
    // the first call.
    [[deprecated]] Txt getParameterID (i32 index) override;
    [[deprecated]] f32 getParameter (i32 parameterIndex) override;
    [[deprecated]] z0 setParameter (i32 parameterIndex, f32 newValue) override;
    [[deprecated]] const Txt getParameterName (i32 parameterIndex) override;
    [[deprecated]] Txt getParameterName (i32 parameterIndex, i32 maximumStringLength) override;
    [[deprecated]] const Txt getParameterText (i32 parameterIndex) override;
    [[deprecated]] Txt getParameterText (i32 parameterIndex, i32 maximumStringLength) override;
    [[deprecated]] i32 getParameterNumSteps (i32 parameterIndex) override;
    [[deprecated]] b8 isParameterDiscrete (i32 parameterIndex) const override;
    [[deprecated]] b8 isParameterAutomatable (i32 parameterIndex) const override;
    [[deprecated]] f32 getParameterDefaultValue (i32 parameterIndex) override;
    [[deprecated]] Txt getParameterLabel (i32 parameterIndex) const override;
    [[deprecated]] b8 isParameterOrientationInverted (i32 parameterIndex) const override;
    [[deprecated]] b8 isMetaParameter (i32 parameterIndex) const override;
    [[deprecated]] AudioProcessorParameter::Category getParameterCategory (i32 parameterIndex) const override;
   #endif

protected:
    //==============================================================================
    /** Structure used to describe plugin parameters */
    struct Parameter   : public HostedParameter
    {
    public:
        Parameter();

        Txt getText (f32 value, i32 maximumStringLength) const override;
        f32 getValueForText (const Txt& text) const override;

    private:
        const StringArray onStrings, offStrings;
    };

    AudioPluginInstance() = default;
    AudioPluginInstance (const BusesProperties& ioLayouts) : AudioProcessor (ioLayouts) {}
    template <size_t numLayouts>
    AudioPluginInstance (const short channelLayoutList[numLayouts][2]) : AudioProcessor (channelLayoutList) {}

private:
    // It's not safe to add a plain AudioProcessorParameter to an AudioPluginInstance.
    // Instead, all parameters must be HostedParameters.
    using AudioProcessor::addParameter;
    using AudioProcessor::addParameterGroup;
    using AudioProcessor::setParameterTree;

    z0 assertOnceOnDeprecatedMethodUse() const noexcept;

    static b8 deprecationAssertiontriggered;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginInstance)
};

DRX_END_IGNORE_WARNINGS_MSVC

} // namespace drx
