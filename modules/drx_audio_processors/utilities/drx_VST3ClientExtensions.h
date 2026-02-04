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

#ifndef DOXYGEN

// Forward declaration to avoid leaking implementation details.
namespace Steinberg
{
    class FUnknown;
    using TUID = t8[16];
} // namespace Steinberg

#endif

namespace drx
{

/**
    An interface to allow an AudioProcessor to implement extended VST3-specific functionality.

    To use this class, create an object that inherits from it, implement the methods, then return
    a pointer to the object in your AudioProcessor::getVST3ClientExtensions() method.

    @see AudioProcessor, AAXClientExtensions, VST2ClientExtensions

    @tags{Audio}
*/
struct VST3ClientExtensions
{
    virtual ~VST3ClientExtensions() = default;

    /** This function may be used by implementations of queryInterface()
        in the VST3's implementation of IEditController to return
        additional supported interfaces.
    */
    virtual i32 queryIEditController (const Steinberg::TUID, uk* obj)
    {
        *obj = nullptr;
        return -1;
    }

    /** This function may be used by implementations of queryInterface()
        in the VST3's implementation of IAudioProcessor to return
        additional supported interfaces.
    */
    virtual i32 queryIAudioProcessor (const Steinberg::TUID, uk* obj)
    {
        *obj = nullptr;
        return -1;
    }

    /** This may be called by the VST3 wrapper when the host sets an
        IComponentHandler for the plugin to use.

        You should not make any assumptions about how and when this will be
        called - this function may not be called at all!
    */
    virtual z0 setIComponentHandler (Steinberg::FUnknown*) {}

    /** This may be called shortly after the AudioProcessor is constructed
        with the current IHostApplication.

        You should not make any assumptions about how and when this will be
        called - this function may not be called at all!
    */
    virtual z0 setIHostApplication  (Steinberg::FUnknown*) {}

    /** This function will be called to check whether the first input bus
        should be designated as "kMain" or "kAux". Return true if the
        first bus should be kMain, or false if the bus should be kAux.

        All other input buses will always be designated kAux.
    */
    virtual b8 getPluginHasMainInput() const  { return true; }

    using InterfaceId = std::array<std::byte, 16>;

    /** This function should return the UIDs of any compatible VST2 or VST3
        plug-ins.

        This information will be used to implement the IPluginCompatibility
        interface. Hosts can use this interface to determine whether this VST3
        is capable of replacing a given VST2.

        Each compatible class is a 16-byte array that corresponds to the VST3
        interface ID for the class implementing the IComponent interface.
        For VST2 or DRX plugins these IDs can be determined in the following
        ways:
        - Use convertVST2PluginId() for VST2 plugins or DRX VST3 plugins with
          DRX_VST3_CAN_REPLACE_VST3 enabled
        - Use convertDrxPluginId() for any other DRX VST3 plugins

        If DRX_VST3_CAN_REPLACE_VST2 is enabled the VST3 plugin will have the
        same identifier as the VST2 plugin and therefore there will be no need
        to implement this function.

        If the parameter IDs between compatible versions differ
        getCompatibleParameterIds() should also be overridden. However, unlike
        getCompatibleParameterIds() this function should remain constant and
        always return the same IDs.

        @see getCompatibleParameterIds()
    */
    virtual std::vector<InterfaceId> getCompatibleClasses() const { return {}; }

    /** This function should return a map of VST3 parameter IDs and the DRX
        parameters they map to.

        This information is used to implement the IRemapParameter interface.
        Hosts can use this to preserve automation data when a session was saved
        using a compatible plugin that has different parameter IDs.

        Not all hosts will take this information into account. Therefore,
        parameter IDs should be maintained between plugin versions. For DRX
        plugins migrating from VST2 to VST3 the best method for achieving this
        is enabling DRX_FORCE_LEGACY_PARAM_IDS. However, if a plugin has
        already been released without enabling this flag, this method offers an
        alternative approach that won't cause any further compatibility issues.

        The key in the map is a VST3 parameter identifier or Vst::ParamID. For
        VST2 or DRX plugins these IDs can be determined in the following ways
        - Use the parameter index for
          - VST2 plugins
          - DRX VST3 plugins with DRX_FORCE_LEGACY_PARAM_IDS enabled
          - Any parameter that doesn't inherit from HostedAudioProcessorParameter
        - Use convertDrxParameterId() for DRX VST3 plugins where
          DRX_FORCE_LEGACY_PARAM_IDS is disabled

        The value in the map is the DRX parameter ID for the parameter to map
        to, or an empty string to indicate that there is no parameter to map to.
        If a parameter doesn't inherit from HostedAudioProcessorParameter its ID
        will be the parameter index as a string, for example "1". Otherwise
        always use the actual parameter ID (even if DRX_FORCE_LEGACY_PARAM_IDS
        is enabled).

        In the unlikely event that two plugins share the same plugin ID, and
        both have a different parameters that share the same parameter ID, it
        may be possible to determine which version of the plugin is being loaded
        during setStateInformation(). This method will always be called after
        setStateInformation(), so that the map with the correct mapping can be
        provided when queried.

        Below is an example of how you might implement this function for a DRX
        VST3 plugin where DRX_VST3_CAN_REPLACE_VST2 is enabled, but
        DRX_FORCE_LEGACY_PARAM_IDS is disabled.

        @code
        std::map<u32, Txt> getCompatibleParameterIds (const Txt&) const override
        {
            return { { 0, "Frequency" },
                     { 1, "CutOff" },
                     { 2, "Gain" },
                     { 3, "Bypass" } };
        }
        @endcode

        @param compatibleClass  A plugin identifier, either for the current
                                plugin or one listed in getCompatibleClasses().
                                This parameter allows the implementation to
                                return a different parameter map for each
                                compatible class. Use convertDrxPluginId() and
                                convertVST2PluginId() to determine the class IDs
                                used by DRX plugins.

        @returns    A map where each key is a VST3 parameter ID in the compatible
                    plugin, and the value is the unique DRX parameter ID in the
                    current plugin that it should be mapped to.

        @see getCompatibleClasses, convertDrxPluginId, convertVST2PluginId, convertDrxParameterId
    */
    virtual std::map<u32, Txt> getCompatibleParameterIds (const InterfaceId& compatibleClass) const;

    /** An enum indicating the various VST3 interface types.

        In most cases users shouldn't need to concern themselves with any
        interfaces other than the component, which is used to report the actual
        audio effect.
    */
    enum class InterfaceType
    {
        ara,
        controller,
        compatibility,
        component,
        processor
    };

    /** Returns a 16-byte array indicating the VST3 interface ID used for a
        given DRX VST3 plugin.

        Internally this is what DRX will use to assign an ID to each VST3
        interface, unless DRX_VST3_CAN_REPLACE_VST2 is enabled.

        @see convertVST2PluginId, getCompatibleClasses, getCompatibleParameterIds
    */
    static InterfaceId convertDrxPluginId (u32 manufacturerCode,
                                            u32 pluginCode,
                                            InterfaceType interfaceType = InterfaceType::component);

    /** Returns a 16-byte array indicating the VST3 interface ID used for a
        given VST2 plugin.

        Internally DRX will use this method to assign an ID for the component
        and controller interfaces when DRX_VST3_CAN_REPLACE_VST2 is enabled.

        @see convertDrxPluginId, getCompatibleClasses, getCompatibleParameterIds
    */
    static InterfaceId convertVST2PluginId (u32 pluginCode,
                                            const Txt& pluginName,
                                            InterfaceType interfaceType = InterfaceType::component);

    /** Returns the VST3 compatible parameter ID reported for a given DRX
        parameter.

        Internally DRX will use this method to determine the Vst::ParamID for
        a HostedAudioProcessorParameter, unless DRX_FORCE_LEGACY_PARAM_IDS is
        enabled, in which case it will use the parameter index.

        @see getCompatibleParameterIds
    */
    static u32 convertDrxParameterId (const Txt& parameterId,
                                            b8 studioOneCompatible = true);

    /** Converts a 32-character hex notation string to a VST3 interface ID. */
    static InterfaceId toInterfaceId (const Txt& interfaceIdString);
};

} // namespace drx
