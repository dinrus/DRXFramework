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

#include <drx_core/system/drx_TargetPlatform.h>
#include <drx_core/system/drx_CompilerWarnings.h>

//==============================================================================
#if DrxPlugin_Build_VST3

#if DRX_VST3_CAN_REPLACE_VST2 && ! DRX_FORCE_USE_LEGACY_PARAM_IDS && ! DRX_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING

 // If you encounter this error there may be an issue migrating parameter
 // automation between sessions saved using the VST2 and VST3 versions of this
 // plugin.
 //
 // If you have released neither a VST2 or VST3 version of the plugin,
 // consider only releasing a VST3 version and disabling DRX_VST3_CAN_REPLACE_VST2.
 //
 // If you have released a VST2 version of the plugin but have not yet released
 // a VST3 version of the plugin, consider enabling DRX_FORCE_USE_LEGACY_PARAM_IDS.
 // This will ensure that the parameter IDs remain compatible between both the
 // VST2 and VST3 versions of the plugin in all hosts.
 //
 // If you have released a VST3 version of the plugin but have not released a
 // VST2 version of the plugin, enable DRX_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING.
 // DO NOT change the DRX_VST3_CAN_REPLACE_VST2 or DRX_FORCE_USE_LEGACY_PARAM_IDS
 // values as this will break compatibility with currently released VST3
 // versions of the plugin.
 //
 // If you have already released a VST2 and VST3 version of the plugin you may
 // find in some hosts when a session containing automation data is saved using
 // the VST2 or VST3 version, and is later loaded using the other version, the
 // automation data will fail to control any of the parameters in the plugin as
 // the IDs for these parameters are different. To fix parameter automation for
 // the VST3 plugin when a session was saved with the VST2 plugin, implement
 // VST3ClientExtensions::getCompatibleParameterIds() and enable
 // DRX_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING.

 #error You may have a conflict with parameter automation between VST2 and VST3 versions of your plugin. See the comment above for more details.
#endif

DRX_BEGIN_NO_SANITIZE ("vptr")

#if DRX_PLUGINHOST_VST3
 #if DRX_MAC
  #include <CoreFoundation/CoreFoundation.h>
 #endif
 #undef DRX_VST3HEADERS_INCLUDE_HEADERS_ONLY
 #define DRX_VST3HEADERS_INCLUDE_HEADERS_ONLY 1
#endif

#include <drx_audio_processors/format_types/drx_VST3Headers.h>

#undef DRX_VST3HEADERS_INCLUDE_HEADERS_ONLY
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1

#include <drx_audio_plugin_client/detail/drx_CheckSettingMacros.h>
#include <drx_audio_plugin_client/detail/drx_IncludeSystemHeaders.h>
#include <drx_audio_plugin_client/detail/drx_PluginUtilities.h>
#include <drx_audio_plugin_client/detail/drx_LinuxMessageThread.h>
#include <drx_audio_plugin_client/detail/drx_VSTWindowUtilities.h>
#include <drx_gui_basics/native/drx_WindowsHooks_windows.h>

#include <drx_audio_processors/format_types/drx_LegacyAudioParameter.cpp>
#include <drx_audio_processors/utilities/drx_FlagCache.h>
#include <drx_audio_processors/format_types/drx_VST3Common.h>

#ifndef DRX_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS
 #if DrxPlugin_WantsMidiInput
  #define DRX_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS 1
 #endif
#endif

#if DRX_LINUX || DRX_BSD
 #include <drx_events/native/drx_EventLoopInternal_linux.h>
 #include <unordered_map>
#endif

#if DRX_MAC
 #include <drx_core/native/drx_CFHelpers_mac.h>
#endif

//==============================================================================
#if DrxPlugin_Enable_ARA
 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpragma-pack")
 #include <ARA_API/ARAVST3.h>
 DRX_END_IGNORE_WARNINGS_GCC_LIKE

 #if ARA_SUPPORT_VERSION_1
  #error "Unsupported ARA version - only ARA version 2 and onward are supported by the current implementation"
 #endif

 DEF_CLASS_IID (ARA::IPlugInEntryPoint)
 DEF_CLASS_IID (ARA::IPlugInEntryPoint2)
 DEF_CLASS_IID (ARA::IMainFactory)
#endif

namespace drx
{

using VST3InterfaceType = VST3ClientExtensions::InterfaceType;
using VST3InterfaceId = VST3ClientExtensions::InterfaceId;

using namespace Steinberg;

static FUID toSteinbergUID (const VST3InterfaceId& uid)
{
    return FUID::fromTUID ((tukk) (uid.data()));
}

static VST3InterfaceId toVST3InterfaceId (const TUID uid)
{
    VST3InterfaceId iid;
    std::memcpy (iid.data(), uid, iid.size());
    return iid;
}

static VST3InterfaceId getInterfaceId (VST3InterfaceType interfaceType)
{
   #if DRX_VST3_CAN_REPLACE_VST2
    if (interfaceType == VST3InterfaceType::controller || interfaceType == VST3InterfaceType::component)
        return VST3ClientExtensions::convertVST2PluginId (DrxPlugin_VSTUniqueID, DrxPlugin_Name, interfaceType);
   #endif

    return VST3ClientExtensions::convertDrxPluginId (DrxPlugin_ManufacturerCode, DrxPlugin_PluginCode, interfaceType);
}

//==============================================================================
#if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
 f64 getScaleFactorForWindow (HWND);
#endif

//==============================================================================
#if DRX_LINUX || DRX_BSD

enum class HostMessageThreadAttached { no, yes };

class HostMessageThreadState
{
public:
    template <typename Callback>
    z0 setStateWithAction (HostMessageThreadAttached stateIn, Callback&& action)
    {
        const std::lock_guard<std::mutex> lock { m };
        state = stateIn;
        action();
    }

    z0 assertHostMessageThread()
    {
        const std::lock_guard<std::mutex> lock { m };

        if (state == HostMessageThreadAttached::no)
            return;

        DRX_ASSERT_MESSAGE_THREAD
    }

private:
    HostMessageThreadAttached state = HostMessageThreadAttached::no;
    std::mutex m;
};

class EventHandler final  : public Linux::IEventHandler,
                            private LinuxEventLoopInternal::Listener
{
public:
    EventHandler()
    {
        LinuxEventLoopInternal::registerLinuxEventLoopListener (*this);
    }

    ~EventHandler() override
    {
        jassert (hostRunLoops.empty());

        LinuxEventLoopInternal::deregisterLinuxEventLoopListener (*this);

        if (! messageThread->isRunning())
            hostMessageThreadState.setStateWithAction (HostMessageThreadAttached::no,
                                                       [this]() { messageThread->start(); });
    }

    DRX_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API queryInterface (const TUID targetIID, uk* obj) override
    {
        return testFor (*this, targetIID, UniqueBase<Linux::IEventHandler>{}).extract (obj);
    }

    z0 PLUGIN_API onFDIsSet (Linux::FileDescriptor fd) override
    {
        updateCurrentMessageThread();
        LinuxEventLoopInternal::invokeEventLoopCallbackForFd (fd);
    }

    //==============================================================================
    z0 registerHandlerForRunLoop (Linux::IRunLoop* l)
    {
        if (l == nullptr)
            return;

        refreshAttachedEventLoop ([this, l] { hostRunLoops.insert (l); });
        updateCurrentMessageThread();
    }

    z0 unregisterHandlerForRunLoop (Linux::IRunLoop* l)
    {
        if (l == nullptr)
            return;

        refreshAttachedEventLoop ([this, l]
        {
            const auto it = hostRunLoops.find (l);

            if (it != hostRunLoops.end())
                hostRunLoops.erase (it);
        });
    }

    /* Asserts if it can be established that the calling thread is different from the host's message
       thread.

       On Linux this can only be determined if the host has already registered its run loop. Until
       then DRX messages are serviced by a background thread internal to the plugin.
    */
    static z0 assertHostMessageThread()
    {
        hostMessageThreadState.assertHostMessageThread();
    }

private:
    //==============================================================================
    /*  Connects all known FDs to a single host event loop instance. */
    class AttachedEventLoop
    {
    public:
        AttachedEventLoop() = default;

        AttachedEventLoop (Linux::IRunLoop* loopIn, Linux::IEventHandler* handlerIn)
            : loop (loopIn), handler (handlerIn)
        {
            for (auto& fd : LinuxEventLoopInternal::getRegisteredFds())
                loop->registerEventHandler (handler, fd);
        }

        AttachedEventLoop (AttachedEventLoop&& other) noexcept
        {
            swap (other);
        }

        AttachedEventLoop& operator= (AttachedEventLoop&& other) noexcept
        {
            swap (other);
            return *this;
        }

        AttachedEventLoop (const AttachedEventLoop&) = delete;
        AttachedEventLoop& operator= (const AttachedEventLoop&) = delete;

        ~AttachedEventLoop()
        {
            if (loop == nullptr)
                return;

            loop->unregisterEventHandler (handler);
        }

    private:
        z0 swap (AttachedEventLoop& other)
        {
            std::swap (other.loop, loop);
            std::swap (other.handler, handler);
        }

        Linux::IRunLoop* loop = nullptr;
        Linux::IEventHandler* handler = nullptr;
    };

    //==============================================================================
    z0 updateCurrentMessageThread()
    {
        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            if (messageThread->isRunning())
                messageThread->stop();

            hostMessageThreadState.setStateWithAction (HostMessageThreadAttached::yes,
                                                       [] { MessageManager::getInstance()->setCurrentThreadAsMessageThread(); });
        }
    }

    z0 fdCallbacksChanged() override
    {
        // The set of active FDs has changed, so deregister from the current event loop and then
        // re-register the current set of FDs.
        refreshAttachedEventLoop ([]{});
    }

    /*  Deregisters from any attached event loop, updates the set of known event loops, and then
        attaches all FDs to the first known event loop.

        The same event loop instance is shared between all plugin instances. Every time an event
        loop is added or removed, this function should be called to register all FDs with a
        suitable event loop.

        Note that there's no API to deregister a single FD for a given event loop. Instead, we must
        deregister all FDs, and then register all known FDs again.
    */
    template <typename Callback>
    z0 refreshAttachedEventLoop (Callback&& modifyKnownRunLoops)
    {
        // Deregister the old event loop.
        // It's important to call the destructor from the old attached loop before calling the
        // constructor of the new attached loop.
        attachedEventLoop = AttachedEventLoop();

        modifyKnownRunLoops();

        // If we still know about an extant event loop, attach to it.
        if (hostRunLoops.begin() != hostRunLoops.end())
            attachedEventLoop = AttachedEventLoop (*hostRunLoops.begin(), this);
    }

    SharedResourcePointer<detail::MessageThread> messageThread;

    std::atomic<i32> refCount { 1 };

    std::multiset<Linux::IRunLoop*> hostRunLoops;
    AttachedEventLoop attachedEventLoop;

    static inline HostMessageThreadState hostMessageThreadState;

    //==============================================================================
    DRX_DECLARE_NON_MOVEABLE (EventHandler)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventHandler)
};

#endif

static z0 assertHostMessageThread()
{
   #if DRX_LINUX || DRX_BSD
    EventHandler::assertHostMessageThread();
   #else
    DRX_ASSERT_MESSAGE_THREAD
   #endif
}

//==============================================================================
class InParameterChangedCallbackSetter
{
public:
    explicit InParameterChangedCallbackSetter (b8& ref)
        : inner ([&]() -> auto& { jassert (! ref); return ref; }(), true, false) {}

private:
    ScopedValueSetter<b8> inner;
};

template <typename Member>
static QueryInterfaceResult queryAdditionalInterfaces (AudioProcessor* processor,
                                                       const TUID targetIID,
                                                       Member&& member)
{
    if (processor == nullptr)
        return {};

    uk obj = nullptr;

    if (auto* extensions = processor->getVST3ClientExtensions())
    {
        const auto result = (extensions->*member) (targetIID, &obj);
        return { result, obj };
    }

    return {};
}

static tresult extractResult (const QueryInterfaceResult& userInterface,
                              const InterfaceResultWithDeferredAddRef& juceInterface,
                              uk* obj)
{
    if (userInterface.isOk() && juceInterface.isOk())
    {
        // If you hit this assertion, you've provided a custom implementation of an interface
        // that DRX implements already. As a result, your plugin may not behave correctly.
        // Consider removing your custom implementation.
        jassertfalse;

        return userInterface.extract (obj);
    }

    if (userInterface.isOk())
        return userInterface.extract (obj);

    return juceInterface.extract (obj);
}

//==============================================================================
class DrxAudioProcessor final : public Vst::IUnitInfo
{
public:
    explicit DrxAudioProcessor (AudioProcessor* source) noexcept
        : audioProcessor (source)
    {
        setupParameters();
    }

    AudioProcessor* get() const noexcept      { return audioProcessor.get(); }

    DRX_DECLARE_VST3_COM_QUERY_METHODS
    DRX_DECLARE_VST3_COM_REF_METHODS

    //==============================================================================
    enum InternalParameters
    {
        paramPreset               = 0x70727374, // 'prst'
        paramMidiControllerOffset = 0x6d636d00, // 'mdm*'
        paramBypass               = 0x62797073  // 'byps'
    };

    //==============================================================================
    Steinberg::i32 PLUGIN_API getUnitCount() override
    {
        return parameterGroups.size() + 1;
    }

    tresult PLUGIN_API getUnitInfo (Steinberg::i32 unitIndex, Vst::UnitInfo& info) override
    {
        if (unitIndex == 0)
        {
            info.id             = Vst::kRootUnitId;
            info.parentUnitId   = Vst::kNoParentUnitId;
            info.programListId  = getProgramListCount() > 0
                                ? static_cast<Vst::ProgramListID> (programParamID)
                                : Vst::kNoProgramListId;

            toString128 (info.name, TRANS ("Root Unit"));

            return kResultTrue;
        }

        if (auto* group = parameterGroups[unitIndex - 1])
        {
            info.id             = DrxAudioProcessor::getUnitID (group);
            info.parentUnitId   = DrxAudioProcessor::getUnitID (group->getParent());
            info.programListId  = Vst::kNoProgramListId;

            toString128 (info.name, group->getName());

            return kResultTrue;
        }

        return kResultFalse;
    }

    Steinberg::i32 PLUGIN_API getProgramListCount() override
    {
        if (audioProcessor->getNumPrograms() > 0)
            return 1;

        return 0;
    }

    tresult PLUGIN_API getProgramListInfo (Steinberg::i32 listIndex, Vst::ProgramListInfo& info) override
    {
        if (listIndex == 0)
        {
            info.id = static_cast<Vst::ProgramListID> (programParamID);
            info.programCount = static_cast<Steinberg::i32> (audioProcessor->getNumPrograms());

            toString128 (info.name, TRANS ("Factory Presets"));

            return kResultTrue;
        }

        jassertfalse;
        zerostruct (info);
        return kResultFalse;
    }

    tresult PLUGIN_API getProgramName (Vst::ProgramListID listId, Steinberg::i32 programIndex, Vst::String128 name) override
    {
        if (listId == static_cast<Vst::ProgramListID> (programParamID)
            && isPositiveAndBelow ((i32) programIndex, audioProcessor->getNumPrograms()))
        {
            toString128 (name, audioProcessor->getProgramName ((i32) programIndex));
            return kResultTrue;
        }

        jassertfalse;
        toString128 (name, drx::Txt());
        return kResultFalse;
    }

    tresult PLUGIN_API hasProgramPitchNames (Vst::ProgramListID, Steinberg::i32) override
    {
        for (i32 i = 0; i <= 127; ++i)
            if (audioProcessor->getNameForMidiNoteNumber (i, 1))
                return kResultTrue;

        return kResultFalse;
    }

    tresult PLUGIN_API getProgramPitchName (Vst::ProgramListID, Steinberg::i32, Steinberg::i16 midiNote, Vst::String128 nameOut) override
    {
        if (auto name = audioProcessor->getNameForMidiNoteNumber (midiNote, 1))
        {
            toString128 (nameOut, *name);
            return kResultTrue;
        }

        return kResultFalse;
    }

    tresult PLUGIN_API getProgramInfo (Vst::ProgramListID, Steinberg::i32, Vst::CString, Vst::String128) override             { return kNotImplemented; }
    tresult PLUGIN_API selectUnit (Vst::UnitID) override                                                                        { return kNotImplemented; }
    tresult PLUGIN_API setUnitProgramData (Steinberg::i32, Steinberg::i32, IBStream*) override                              { return kNotImplemented; }
    Vst::UnitID PLUGIN_API getSelectedUnit() override                                                                           { return Vst::kRootUnitId; }

    tresult PLUGIN_API getUnitByBus (Vst::MediaType, Vst::BusDirection, Steinberg::i32, Steinberg::i32, Vst::UnitID& unitId) override
    {
        unitId = Vst::kRootUnitId;
        return kResultOk;
    }

    //==============================================================================
    inline Vst::ParamID getVSTParamIDForIndex (i32 paramIndex) const noexcept
    {
       #if DRX_FORCE_USE_LEGACY_PARAM_IDS
        return static_cast<Vst::ParamID> (paramIndex);
       #else
        jassert (paramIndex < vstParamIDs.size());
        return vstParamIDs.getReference (paramIndex);
       #endif
    }

    AudioProcessorParameter* getParamForVSTParamID (Vst::ParamID paramID) const noexcept
    {
        const auto iter = paramMap.find (paramID);
        return iter != paramMap.end() ? iter->second : nullptr;
    }

    AudioProcessorParameter* getBypassParameter() const noexcept
    {
        return getParamForVSTParamID (bypassParamID);
    }

    AudioProcessorParameter* getProgramParameter() const noexcept
    {
        return getParamForVSTParamID (programParamID);
    }

    static Vst::UnitID getUnitID (const AudioProcessorParameterGroup* group)
    {
        if (group == nullptr || group->getParent() == nullptr)
            return Vst::kRootUnitId;

        // From the VST3 docs (also applicable to unit IDs!):
        // Up to 2^31 parameters can be exported with id range [0, 2147483648]
        // (the range [2147483649, 429496729] is reserved for host application).
        auto unitID = group->getID().hashCode() & 0x7fffffff;

        // If you hit this assertion then your group ID is hashing to a value
        // reserved by the VST3 SDK. Please use a different group ID.
        jassert (unitID != Vst::kRootUnitId);

        return unitID;
    }

    const Array<Vst::ParamID>& getParamIDs() const noexcept { return vstParamIDs; }
    Vst::ParamID getBypassParamID()          const noexcept { return bypassParamID; }
    Vst::ParamID getProgramParamID()         const noexcept { return programParamID; }
    b8 isBypassRegularParameter()          const noexcept { return bypassIsRegularParameter; }

    i32 findCacheIndexForParamID (Vst::ParamID paramID) const noexcept { return vstParamIDs.indexOf (paramID); }

    z0 setParameterValue (Steinberg::i32 paramIndex, f32 value)
    {
        cachedParamValues.set (paramIndex, value);
    }

    template <typename Callback>
    z0 forAllChangedParameters (Callback&& callback)
    {
        cachedParamValues.ifSet ([&] (Steinberg::i32 index, f32 value)
        {
            callback (cachedParamValues.getParamID (index), value);
        });
    }

    b8 isUsingManagedParameters() const noexcept    { return juceParameters.isUsingManagedParameters(); }

    std::map<Vst::ParamID, AudioProcessorParameter*> getParameterMap (const VST3InterfaceId& pluginId) const
    {
        const auto iter = compatibleParameterIdMap.find (pluginId);
        return iter != compatibleParameterIdMap.end() ? iter->second
                                                      : std::map<Vst::ParamID, AudioProcessorParameter*>{};
    }

    AudioProcessorParameter* getParameter (const Txt& juceParamId) const
    {
        const auto iter = juceIdParameterMap.find (juceParamId);
        return iter != juceIdParameterMap.end() ? iter->second : nullptr;
    }

    z0 updateParameterMapping()
    {
        static const auto currentPluginId = getInterfaceId (VST3InterfaceType::component);

        compatibleParameterIdMap = {};
        compatibleParameterIdMap[currentPluginId] = paramMap;

        if (const auto* ext = audioProcessor->getVST3ClientExtensions())
        {
            for (auto& compatibleClass : ext->getCompatibleClasses())
            {
                auto& parameterIdMap = compatibleParameterIdMap[compatibleClass];

                for (auto [oldParamId, newParamId] : ext->getCompatibleParameterIds (compatibleClass))
                {
                    auto* parameter = getParameter (newParamId);
                    parameterIdMap[oldParamId] = parameter;

                    // This means a parameter ID returned by getCompatibleParameterIds()
                    // does not match any parameters declared in the plugin. All IDs must
                    // match an existing parameter, or return an empty string to indicate
                    // there is no parameter to map to.
                    jassert (parameter != nullptr || newParamId.isEmpty());

                    // This means getCompatibleParameterIds() returned a parameter mapping
                    // that will hide a parameter in the current plugin! If this is due to
                    // an ID collision between plugin versions, you may be able to determine
                    // the mapping to report based on setStateInformation(). If you've
                    // already done this you can safely ignore this warning. If there is no
                    // way to determine the difference between the two plugin versions in
                    // setStateInformation() the best course of action is to remove the
                    // problematic parameter from the mapping.
                    jassert (compatibleClass != currentPluginId
                             || getParamForVSTParamID (oldParamId) == nullptr
                             || parameter == getParamForVSTParamID (oldParamId));
                }
            }
        }
    }

    //==============================================================================
    inline static const FUID iid = toSteinbergUID (getInterfaceId (VST3InterfaceType::processor));

private:
    //==============================================================================
    z0 setupParameters()
    {
        parameterGroups = audioProcessor->getParameterTree().getSubgroups (true);

       #if DRX_ASSERTIONS_ENABLED_OR_LOGGED
        auto allGroups = parameterGroups;
        allGroups.add (&audioProcessor->getParameterTree());
        std::unordered_set<Vst::UnitID> unitIDs;

        for (auto* group : allGroups)
        {
            auto insertResult = unitIDs.insert (getUnitID (group));

            // If you hit this assertion then either a group ID is not unique or
            // you are very unlucky and a hashed group ID is not unique
            jassert (insertResult.second);
        }
       #endif

       #if DRX_FORCE_USE_LEGACY_PARAM_IDS
        const b8 forceLegacyParamIDs = true;
       #else
        const b8 forceLegacyParamIDs = false;
       #endif

        juceParameters.update (*audioProcessor, forceLegacyParamIDs);
        auto numParameters = juceParameters.getNumParameters();

        b8 vst3WrapperProvidedBypassParam = false;
        auto* bypassParameter = audioProcessor->getBypassParameter();

        if (bypassParameter == nullptr)
        {
            vst3WrapperProvidedBypassParam = true;
            ownedBypassParameter.reset (new AudioParameterBool ("byps", "Bypass", false));
            bypassParameter = ownedBypassParameter.get();
        }

        // if the bypass parameter is not part of the exported parameters that the plug-in supports
        // then add it to the end of the list as VST3 requires the bypass parameter to be exported!
        bypassIsRegularParameter = juceParameters.contains (audioProcessor->getBypassParameter());

        if (! bypassIsRegularParameter)
            juceParameters.addNonOwning (bypassParameter);

        i32 i = 0;
        for (auto* juceParam : juceParameters)
        {
            b8 isBypassParameter = (juceParam == bypassParameter);

            Vst::ParamID vstParamID = forceLegacyParamIDs ? static_cast<Vst::ParamID> (i++)
                                                          : generateVSTParamIDForParam (juceParam);

            if (isBypassParameter)
            {
                // we need to remain backward compatible with the old bypass id
                if (vst3WrapperProvidedBypassParam)
                {
                    DRX_BEGIN_IGNORE_WARNINGS_MSVC (6240)
                    vstParamID = static_cast<Vst::ParamID> ((isUsingManagedParameters() && ! forceLegacyParamIDs) ? paramBypass : numParameters);
                    DRX_END_IGNORE_WARNINGS_MSVC
                }

                bypassParamID = vstParamID;
            }

            vstParamIDs.add (vstParamID);
            paramMap[vstParamID] = juceParam;
            juceIdParameterMap[LegacyAudioParameter::getParamID (juceParam, false)] = juceParam;
        }

        auto numPrograms = audioProcessor->getNumPrograms();

        if (numPrograms > 1)
        {
            ownedProgramParameter = std::make_unique<AudioParameterInt> ("juceProgramParameter", "Program",
                                                                         0, numPrograms - 1,
                                                                         audioProcessor->getCurrentProgram());

            juceParameters.addNonOwning (ownedProgramParameter.get());

            if (forceLegacyParamIDs)
                programParamID = static_cast<Vst::ParamID> (i++);

            vstParamIDs.add (programParamID);
            paramMap[programParamID] = ownedProgramParameter.get();
        }

        cachedParamValues = CachedParamValues { { vstParamIDs.begin(), vstParamIDs.end() } };
    }

    Vst::ParamID generateVSTParamIDForParam (const AudioProcessorParameter* param)
    {
        const auto juceParamID = LegacyAudioParameter::getParamID (param, false);

       #if DRX_FORCE_USE_LEGACY_PARAM_IDS
        return static_cast<Vst::ParamID> (juceParamID.getIntValue());
       #else
        return VST3ClientExtensions::convertDrxParameterId (juceParamID, DRX_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS);
       #endif
    }

    //==============================================================================
    Array<Vst::ParamID> vstParamIDs;
    CachedParamValues cachedParamValues;
    Vst::ParamID bypassParamID = 0, programParamID = static_cast<Vst::ParamID> (paramPreset);
    b8 bypassIsRegularParameter = false;
    std::map<VST3InterfaceId, std::map<Vst::ParamID, AudioProcessorParameter*>> compatibleParameterIdMap;
    std::map<Txt, AudioProcessorParameter*> juceIdParameterMap;

    //==============================================================================
    std::atomic<i32> refCount { 0 };
    std::unique_ptr<AudioProcessor> audioProcessor;

    //==============================================================================
    LegacyAudioParametersWrapper juceParameters;
    std::map<Vst::ParamID, AudioProcessorParameter*> paramMap;
    std::unique_ptr<AudioProcessorParameter> ownedBypassParameter, ownedProgramParameter;
    Array<const AudioProcessorParameterGroup*> parameterGroups;

    DrxAudioProcessor() = delete;
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrxAudioProcessor)
};

#if DRX_LINUX || DRX_BSD
using RunLoop = VSTComSmartPtr<Linux::IRunLoop>;

class ScopedRunLoop
{
public:
    explicit ScopedRunLoop (const RunLoop& l)
        : runLoop (l)
    {
        eventHandler->registerHandlerForRunLoop (runLoop.get());
    }

    ~ScopedRunLoop()
    {
        eventHandler->unregisterHandlerForRunLoop (runLoop.get());
    }

    RunLoop get() const { return runLoop; }

    DRX_DECLARE_NON_COPYABLE (ScopedRunLoop)
    DRX_DECLARE_NON_MOVEABLE (ScopedRunLoop)

    static RunLoop getRunLoopFromFrame (IPlugFrame* plugFrame)
    {
        VSTComSmartPtr<Linux::IRunLoop> result;
        result.loadFrom (plugFrame);
        return result;
    }

private:
    ScopedDrxInitialiser_GUI libraryInitialiser;
    SharedResourcePointer<detail::MessageThread> messageThread;
    SharedResourcePointer<EventHandler> eventHandler;
    RunLoop runLoop;
};
#else
struct RunLoop
{
    z0 loadFrom (FUnknown*) {}
};

class ScopedRunLoop
{
public:
    explicit ScopedRunLoop (const RunLoop&) {}
    RunLoop get() const { return {}; }
    static RunLoop getRunLoopFromFrame (IPlugFrame*) { return {}; }

private:
    ScopedDrxInitialiser_GUI libraryInitialiser;
};
#endif

class DrxVST3Component;

static thread_local b8 inParameterChangedCallback = false;

static z0 setValueAndNotifyIfChanged (AudioProcessorParameter& param, f32 newValue)
{
    if (approximatelyEqual (param.getValue(), newValue))
        return;

    const InParameterChangedCallbackSetter scopedSetter { inParameterChangedCallback };
    param.setValueNotifyingHost (newValue);
}

//==============================================================================
class DrxVST3EditController final : public Vst::EditController,
                                     public Vst::IMidiMapping,
                                     public Vst::IUnitInfo,
                                     public Vst::IRemapParamID,
                                     public Vst::ChannelContext::IInfoListener,
                                    #if DrxPlugin_Enable_ARA
                                     public Presonus::IPlugInViewEmbedding,
                                    #endif
                                     public AudioProcessorListener,
                                     private ComponentRestarter::Listener
{
public:
    DrxVST3EditController (const VSTComSmartPtr<Vst::IHostApplication>& host,
                            const RunLoop& l)
        : scopedRunLoop (l)
    {
        if (host != nullptr)
            host->queryInterface (FUnknown::iid, (uk*) &hostContext);

        blueCatPatchwork |= isBlueCatHost (host.get());
    }

    //==============================================================================
    inline static const FUID iid = toSteinbergUID (getInterfaceId (VST3InterfaceType::controller));

    //==============================================================================
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Winconsistent-missing-override")

    REFCOUNT_METHODS (ComponentBase)

    DRX_END_IGNORE_WARNINGS_GCC_LIKE

    tresult PLUGIN_API queryInterface (const TUID targetIID, uk* obj) override
    {
        const auto userProvidedInterface = queryAdditionalInterfaces (getPluginInstance(),
                                                                      targetIID,
                                                                      &VST3ClientExtensions::queryIEditController);

        const auto juceProvidedInterface = queryInterfaceInternal (targetIID);

        return extractResult (userProvidedInterface, juceProvidedInterface, obj);
    }

    //==============================================================================
    tresult PLUGIN_API initialize (FUnknown* context) override
    {
        if (hostContext != context)
            hostContext = context;

        blueCatPatchwork |= isBlueCatHost (context);

        return kResultTrue;
    }

    tresult PLUGIN_API terminate() override
    {
        if (auto* pluginInstance = getPluginInstance())
            pluginInstance->removeListener (this);

        audioProcessor = nullptr;

        return EditController::terminate();
    }

    //==============================================================================
    struct Param final : public Vst::Parameter
    {
        Param (DrxVST3EditController& editController, AudioProcessorParameter& p,
               Vst::ParamID vstParamID, Vst::UnitID vstUnitID,
               b8 isBypassParameter)
            : owner (editController), param (p)
        {
            info.id = vstParamID;
            info.unitId = vstUnitID;

            updateParameterInfo();

            // Is this a meter?
            if ((((u32) param.getCategory() & 0xffff0000) >> 16) == 2)
                info.flags = Vst::ParameterInfo::kIsReadOnly;
            else
                info.flags = param.isAutomatable() ? Vst::ParameterInfo::kCanAutomate : 0;

            if (isBypassParameter)
                info.flags |= Vst::ParameterInfo::kIsBypass;

            valueNormalized = info.defaultNormalizedValue;
        }

        b8 updateParameterInfo()
        {
            auto updateParamIfChanged = [] (Vst::String128& paramToUpdate, const Txt& newValue)
            {
                if (drx::toString (paramToUpdate) == newValue)
                    return false;

                toString128 (paramToUpdate, newValue);
                return true;
            };

            const auto updateParamIfScalarChanged = [] (auto& toChange, const auto newValue)
            {
                return ! exactlyEqual (std::exchange (toChange, newValue), newValue);
            };

            const auto newStepCount = [&]
            {
               #if ! DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
                if (! param.isDiscrete())
                    return 0;
               #endif

                const auto numSteps = param.getNumSteps();
                return (Steinberg::i32) (0 < numSteps && numSteps < 0x7fffffff ? numSteps - 1 : 0);
            }();

            auto anyUpdated = updateParamIfChanged (info.title, param.getName (128));
            anyUpdated |= updateParamIfChanged (info.shortTitle, param.getName (8));
            anyUpdated |= updateParamIfChanged (info.units, param.getLabel());
            anyUpdated |= updateParamIfScalarChanged (info.stepCount, newStepCount);
            anyUpdated |= updateParamIfScalarChanged (info.defaultNormalizedValue, (f64) param.getDefaultValue());

            jassert (0 <= info.defaultNormalizedValue && info.defaultNormalizedValue <= 1.0);

            return anyUpdated;
        }

        b8 setNormalized (Vst::ParamValue v) override
        {
            v = jlimit (0.0, 1.0, v);

            if (! approximatelyEqual (v, valueNormalized))
            {
                valueNormalized = v;

                // Only update the AudioProcessor here if we're not playing,
                // otherwise we get parallel streams of parameter value updates
                // during playback
                if (! owner.vst3IsPlaying)
                    setValueAndNotifyIfChanged (param, (f32) v);

                changed();
                return true;
            }

            return false;
        }

        z0 toString (Vst::ParamValue value, Vst::String128 result) const override
        {
            if (LegacyAudioParameter::isLegacy (&param))
                // remain backward-compatible with old DRX code
                toString128 (result, param.getCurrentValueAsText());
            else
                toString128 (result, param.getText ((f32) value, 128));
        }

        b8 fromString (const Vst::TChar* text, Vst::ParamValue& outValueNormalized) const override
        {
            if (! LegacyAudioParameter::isLegacy (&param))
            {
                outValueNormalized = param.getValueForText (getStringFromVstTChars (text));
                return true;
            }

            return false;
        }

        static Txt getStringFromVstTChars (const Vst::TChar* text)
        {
            return drx::Txt (drx::CharPointer_UTF16 (reinterpret_cast<const drx::CharPointer_UTF16::CharType*> (text)));
        }

        Vst::ParamValue toPlain (Vst::ParamValue v) const override       { return v; }
        Vst::ParamValue toNormalized (Vst::ParamValue v) const override  { return v; }

    private:
        DrxVST3EditController& owner;
        AudioProcessorParameter& param;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Param)
    };

    //==============================================================================
    struct ProgramChangeParameter final : public Vst::Parameter
    {
        ProgramChangeParameter (AudioProcessor& p, Vst::ParamID vstParamID)
            : owner (p)
        {
            jassert (owner.getNumPrograms() > 1);

            info.id = vstParamID;
            toString128 (info.title, "Program");
            toString128 (info.shortTitle, "Program");
            toString128 (info.units, "");
            info.stepCount = owner.getNumPrograms() - 1;
            info.defaultNormalizedValue = static_cast<Vst::ParamValue> (owner.getCurrentProgram())
                                            / static_cast<Vst::ParamValue> (info.stepCount);
            info.unitId = Vst::kRootUnitId;
            info.flags = Vst::ParameterInfo::kIsProgramChange | Vst::ParameterInfo::kCanAutomate;
        }

        ~ProgramChangeParameter() override = default;

        b8 setNormalized (Vst::ParamValue v) override
        {
            const auto programValue = getProgramValueFromNormalised (v);

            if (programValue != owner.getCurrentProgram())
                owner.setCurrentProgram (programValue);

            if (! approximatelyEqual (valueNormalized, v))
            {
                valueNormalized = v;
                changed();

                return true;
            }

            return false;
        }

        z0 toString (Vst::ParamValue value, Vst::String128 result) const override
        {
            toString128 (result, owner.getProgramName (roundToInt (value * info.stepCount)));
        }

        b8 fromString (const Vst::TChar* text, Vst::ParamValue& outValueNormalized) const override
        {
            auto paramValueString = getStringFromVstTChars (text);
            auto n = owner.getNumPrograms();

            for (i32 i = 0; i < n; ++i)
            {
                if (paramValueString == owner.getProgramName (i))
                {
                    outValueNormalized = static_cast<Vst::ParamValue> (i) / info.stepCount;
                    return true;
                }
            }

            return false;
        }

        static Txt getStringFromVstTChars (const Vst::TChar* text)
        {
            return Txt (CharPointer_UTF16 (reinterpret_cast<const CharPointer_UTF16::CharType*> (text)));
        }

        Steinberg::i32 getProgramValueFromNormalised (Vst::ParamValue v) const
        {
            return jmin (info.stepCount, (Steinberg::i32) (v * (info.stepCount + 1)));
        }

        Vst::ParamValue toPlain (Vst::ParamValue v) const override       { return getProgramValueFromNormalised (v); }
        Vst::ParamValue toNormalized (Vst::ParamValue v) const override  { return v / info.stepCount; }

    private:
        AudioProcessor& owner;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramChangeParameter)
    };

    //==============================================================================
    tresult PLUGIN_API getCompatibleParamID (const TUID pluginToReplaceUID,
                                             Vst::ParamID oldParamID,
                                             Vst::ParamID& newParamID) override
    {
        const auto parameterMap = audioProcessor->getParameterMap (toVST3InterfaceId (pluginToReplaceUID));
        const auto iter = parameterMap.find (oldParamID);

        if (iter == parameterMap.end())
        {
            // This suggests a host is trying to load a plugin and parameter ID
            // combination that hasn't been accounted for in getCompatibleParameterIds().
            // Override this method in VST3ClientExtensions and return a suitable
            // parameter mapping to silence this warning.
            jassertfalse;
            return kResultFalse;
        }

        const auto* parameter = iter->second;
        newParamID = parameter != nullptr ? audioProcessor->getVSTParamIDForIndex (parameter->getParameterIndex())
                                          : 0xffffffff;
        return kResultTrue;
    }

    //==============================================================================
    tresult PLUGIN_API setChannelContextInfos (Vst::IAttributeList* list) override
    {
        if (auto* instance = getPluginInstance())
        {
            if (list != nullptr)
            {
                AudioProcessor::TrackProperties trackProperties;

                {
                    Vst::String128 channelName;
                    if (list->getString (Vst::ChannelContext::kChannelNameKey, channelName, sizeof (channelName)) == kResultTrue)
                        trackProperties.name = std::make_optional (toString (channelName));
                }

                {
                    Steinberg::z64 colour;
                    if (list->getInt (Vst::ChannelContext::kChannelColorKey, colour) == kResultTrue)
                        trackProperties.colour = std::make_optional (Color (Vst::ChannelContext::GetRed ((u32) colour),  Vst::ChannelContext::GetGreen ((u32) colour),
                                                                             Vst::ChannelContext::GetBlue ((u32) colour), Vst::ChannelContext::GetAlpha ((u32) colour)));
                }



                if (MessageManager::getInstance()->isThisTheMessageThread())
                    instance->updateTrackProperties (trackProperties);
                else
                    MessageManager::callAsync ([trackProperties, instance]
                                               { instance->updateTrackProperties (trackProperties); });
            }
        }

        return kResultOk;
    }

    //==============================================================================
   #if DrxPlugin_Enable_ARA
    Steinberg::TBool PLUGIN_API isViewEmbeddingSupported() override
    {
        if (auto* pluginInstance = getPluginInstance())
            return (Steinberg::TBool) dynamic_cast<AudioProcessorARAExtension*> (pluginInstance)->isEditorView();
        return (Steinberg::TBool) false;
    }

    Steinberg::tresult PLUGIN_API setViewIsEmbedded (Steinberg::IPlugView* /*view*/, Steinberg::TBool /*embedded*/) override
    {
        return kResultOk;
    }
   #endif

    //==============================================================================
    tresult PLUGIN_API setComponentState (IBStream*) override
    {
        // As an IEditController member, the host should only call this from the message thread.
        assertHostMessageThread();

        if (auto* pluginInstance = getPluginInstance())
        {
            for (auto vstParamId : audioProcessor->getParamIDs())
            {
                auto paramValue = [&]
                {
                    if (vstParamId == audioProcessor->getProgramParamID())
                        return EditController::plainParamToNormalized (audioProcessor->getProgramParamID(),
                                                                       pluginInstance->getCurrentProgram());

                    return (f64) audioProcessor->getParamForVSTParamID (vstParamId)->getValue();
                }();

                setParamNormalized (vstParamId, paramValue);
            }
        }

        audioProcessor->updateParameterMapping();

        if (auto* handler = getComponentHandler())
            handler->restartComponent (Vst::kParamValuesChanged | Vst::kParamIDMappingChanged);

        return kResultOk;
    }

    z0 setAudioProcessor (DrxAudioProcessor* audioProc)
    {
        if (audioProcessor.get() != audioProc)
            installAudioProcessor (addVSTComSmartPtrOwner (audioProc));
    }

    tresult PLUGIN_API connect (IConnectionPoint* other) override
    {
        if (other != nullptr && audioProcessor == nullptr)
        {
            auto result = ComponentBase::connect (other);

            if (! audioProcessor.loadFrom (other))
                sendIntMessage ("DrxVST3EditController", (Steinberg::z64) (pointer_sized_int) this);
            else
                installAudioProcessor (audioProcessor);

            return result;
        }

        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API getMidiControllerAssignment ([[maybe_unused]] Steinberg::i32 busIndex,
                                                    [[maybe_unused]] Steinberg::i16 channel,
                                                    [[maybe_unused]] Vst::CtrlNumber midiControllerNumber,
                                                    [[maybe_unused]] Vst::ParamID& resultID) override
    {
       #if DRX_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS
        resultID = midiControllerToParameter[channel][midiControllerNumber];
        return kResultTrue; // Returning false makes some hosts stop asking for further MIDI Controller Assignments
       #else
        return kResultFalse;
       #endif
    }

    // Converts an incoming parameter index to a MIDI controller:
    b8 getMidiControllerForParameter (Vst::ParamID index, i32& channel, i32& ctrlNumber)
    {
        auto mappedIndex = static_cast<i32> (index - parameterToMidiControllerOffset);

        if (isPositiveAndBelow (mappedIndex, numElementsInArray (parameterToMidiController)))
        {
            auto& mc = parameterToMidiController[mappedIndex];

            if (mc.channel != -1 && mc.ctrlNumber != -1)
            {
                channel = jlimit (1, 16, mc.channel + 1);
                ctrlNumber = mc.ctrlNumber;
                return true;
            }
        }

        return false;
    }

    inline b8 isMidiControllerParamID (Vst::ParamID paramID) const noexcept
    {
        return (paramID >= parameterToMidiControllerOffset
                    && isPositiveAndBelow (paramID - parameterToMidiControllerOffset,
                                           static_cast<Vst::ParamID> (numElementsInArray (parameterToMidiController))));
    }

    //==============================================================================
    Steinberg::i32 PLUGIN_API getUnitCount() override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getUnitCount();

        jassertfalse;
        return 1;
    }

    tresult PLUGIN_API getUnitInfo (Steinberg::i32 unitIndex, Vst::UnitInfo& info) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getUnitInfo (unitIndex, info);

        jassertfalse;
        if (unitIndex == 0)
        {
            info.id             = Vst::kRootUnitId;
            info.parentUnitId   = Vst::kNoParentUnitId;
            info.programListId  = Vst::kNoProgramListId;

            toString128 (info.name, TRANS ("Root Unit"));

            return kResultTrue;
        }

        zerostruct (info);
        return kResultFalse;
    }

    Steinberg::i32 PLUGIN_API getProgramListCount() override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getProgramListCount();

        jassertfalse;
        return 0;
    }

    tresult PLUGIN_API getProgramListInfo (Steinberg::i32 listIndex, Vst::ProgramListInfo& info) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getProgramListInfo (listIndex, info);

        jassertfalse;
        zerostruct (info);
        return kResultFalse;
    }

    tresult PLUGIN_API getProgramName (Vst::ProgramListID listId, Steinberg::i32 programIndex, Vst::String128 name) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getProgramName (listId, programIndex, name);

        jassertfalse;
        toString128 (name, drx::Txt());
        return kResultFalse;
    }

    tresult PLUGIN_API getProgramInfo (Vst::ProgramListID listId, Steinberg::i32 programIndex,
                                       Vst::CString attributeId, Vst::String128 attributeValue) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getProgramInfo (listId, programIndex, attributeId, attributeValue);

        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API hasProgramPitchNames (Vst::ProgramListID listId, Steinberg::i32 programIndex) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->hasProgramPitchNames (listId, programIndex);

        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API getProgramPitchName (Vst::ProgramListID listId, Steinberg::i32 programIndex,
                                            Steinberg::i16 midiPitch, Vst::String128 name) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getProgramPitchName (listId, programIndex, midiPitch, name);

        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API selectUnit (Vst::UnitID unitId) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->selectUnit (unitId);

        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API setUnitProgramData (Steinberg::i32 listOrUnitId, Steinberg::i32 programIndex,
                                           IBStream* data) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->setUnitProgramData (listOrUnitId, programIndex, data);

        jassertfalse;
        return kResultFalse;
    }

    Vst::UnitID PLUGIN_API getSelectedUnit() override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getSelectedUnit();

        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API getUnitByBus (Vst::MediaType type, Vst::BusDirection dir, Steinberg::i32 busIndex,
                                     Steinberg::i32 channel, Vst::UnitID& unitId) override
    {
        if (audioProcessor != nullptr)
            return audioProcessor->getUnitByBus (type, dir, busIndex, channel, unitId);

        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    IPlugView* PLUGIN_API createView (tukk name) override
    {
        if (auto* pluginInstance = getPluginInstance())
        {
            const auto mayCreateEditor = pluginInstance->hasEditor()
                                      && name != nullptr
                                      && std::strcmp (name, Vst::ViewType::kEditor) == 0
                                      && (pluginInstance->getActiveEditor() == nullptr
                                          || detail::PluginUtilities::getHostType().isAdobeAudition()
                                          || detail::PluginUtilities::getHostType().isPremiere());

            if (mayCreateEditor)
                return new DrxVST3Editor (*this, *audioProcessor);
        }

        return nullptr;
    }

    //==============================================================================
    z0 beginGesture (Vst::ParamID vstParamId)
    {
        if (! inSetState && MessageManager::getInstance()->isThisTheMessageThread())
            beginEdit (vstParamId);
    }

    z0 endGesture (Vst::ParamID vstParamId)
    {
        if (! inSetState && MessageManager::getInstance()->isThisTheMessageThread())
            endEdit (vstParamId);
    }

    z0 paramChanged (Steinberg::i32 parameterIndex, Vst::ParamID vstParamId, f64 newValue)
    {
        if (inParameterChangedCallback || inSetState)
            return;

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            // NB: Cubase has problems if performEdit is called without setParamNormalized
            EditController::setParamNormalized (vstParamId, newValue);
            performEdit (vstParamId, newValue);
        }
        else
        {
            audioProcessor->setParameterValue (parameterIndex, (f32) newValue);
        }
    }

    //==============================================================================
    z0 audioProcessorParameterChangeGestureBegin (AudioProcessor*, i32 index) override
    {
        beginGesture (audioProcessor->getVSTParamIDForIndex (index));
    }

    z0 audioProcessorParameterChangeGestureEnd (AudioProcessor*, i32 index) override
    {
        endGesture (audioProcessor->getVSTParamIDForIndex (index));
    }

    z0 audioProcessorParameterChanged (AudioProcessor*, i32 index, f32 newValue) override
    {
        paramChanged (index, audioProcessor->getVSTParamIDForIndex (index), newValue);
    }

    z0 audioProcessorChanged (AudioProcessor*, const ChangeDetails& details) override
    {
        i32 flags = 0;

        if (details.parameterInfoChanged)
        {
            for (i32 i = 0; i < parameters.getParameterCount(); ++i)
                if (auto* param = dynamic_cast<Param*> (parameters.getParameterByIndex (i)))
                    if (param->updateParameterInfo())
                        flags |= Vst::kParamTitlesChanged;
        }

        if (auto* pluginInstance = getPluginInstance())
        {
            if (details.programChanged)
            {
                const auto programParameterId = audioProcessor->getProgramParamID();

                if (audioProcessor->getParamForVSTParamID (programParameterId) != nullptr)
                {
                    const auto currentProgram = pluginInstance->getCurrentProgram();
                    const auto paramValue = roundToInt (EditController::normalizedParamToPlain (programParameterId,
                                                                                                EditController::getParamNormalized (programParameterId)));

                    if (currentProgram != paramValue)
                    {
                        beginGesture (programParameterId);
                        paramChanged (audioProcessor->findCacheIndexForParamID (programParameterId),
                                      programParameterId,
                                      EditController::plainParamToNormalized (programParameterId, currentProgram));
                        endGesture (programParameterId);

                        flags |= Vst::kParamValuesChanged;
                    }
                }
            }

            auto latencySamples = pluginInstance->getLatencySamples();

           #if DrxPlugin_Enable_ARA
            jassert (latencySamples == 0 || ! dynamic_cast<AudioProcessorARAExtension*> (pluginInstance)->isBoundToARA());
           #endif

            if (details.latencyChanged && latencySamples != lastLatencySamples)
            {
                flags |= Vst::kLatencyChanged;
                lastLatencySamples = latencySamples;
            }
        }

        if (details.nonParameterStateChanged)
            flags |= pluginShouldBeMarkedDirtyFlag;

        if (inSetupProcessing)
            flags &= Vst::kLatencyChanged;

        componentRestarter.restart (flags);
    }

    //==============================================================================
    AudioProcessor* getPluginInstance() const noexcept
    {
        if (audioProcessor != nullptr)
            return audioProcessor->get();

        return nullptr;
    }

    static constexpr auto pluginShouldBeMarkedDirtyFlag = 1 << 16;

private:
    b8 isBlueCatHost (FUnknown* context) const
    {
        // We can't use the normal PluginHostType mechanism here because that will give us the name
        // of the host process. However, this plugin instance might be loaded in an instance of
        // the BlueCat PatchWork host, which might itself be a plugin.

        VSTComSmartPtr<Vst::IHostApplication> host;
        host.loadFrom (context);

        if (host == nullptr)
            return false;

        Vst::String128 name;

        if (host->getName (name) != kResultOk)
            return false;

        const auto hostName = toString (name);
        return hostName.contains ("Blue Cat's VST3 Host");
    }

    friend DrxVST3Component;
    friend Param;

    //==============================================================================
    ScopedRunLoop scopedRunLoop;
    VSTComSmartPtr<DrxAudioProcessor> audioProcessor;

    struct MidiController
    {
        i32 channel = -1, ctrlNumber = -1;
    };

    ComponentRestarter componentRestarter { *this };

    enum { numMIDIChannels = 16 };
    Vst::ParamID parameterToMidiControllerOffset;
    MidiController parameterToMidiController[(i32) numMIDIChannels * (i32) Vst::kCountCtrlNumber];
    Vst::ParamID midiControllerToParameter[numMIDIChannels][Vst::kCountCtrlNumber];

    z0 restartComponentOnMessageThread (i32 flags) override
    {
        if ((flags & pluginShouldBeMarkedDirtyFlag) != 0)
            setDirty (true);

        flags &= ~pluginShouldBeMarkedDirtyFlag;

        if (auto* handler = componentHandler.get())
            handler->restartComponent (flags);
    }

    //==============================================================================
    struct OwnedParameterListener  final : public AudioProcessorParameter::Listener
    {
        OwnedParameterListener (DrxVST3EditController& editController,
                                AudioProcessorParameter& parameter,
                                Vst::ParamID paramID,
                                i32 cacheIndex)
            : owner (editController),
              vstParamID (paramID),
              parameterIndex (cacheIndex)
        {
            // We shouldn't be using an OwnedParameterListener for parameters that have
            // been added directly to the AudioProcessor. We observe those via the
            // normal audioProcessorParameterChanged mechanism.
            jassert (parameter.getParameterIndex() == -1);
            // The parameter must have a non-negative index in the parameter cache.
            jassert (parameterIndex >= 0);
            parameter.addListener (this);
        }

        z0 parameterValueChanged (i32, f32 newValue) override
        {
            owner.paramChanged (parameterIndex, vstParamID, newValue);
        }

        z0 parameterGestureChanged (i32, b8 gestureIsStarting) override
        {
            if (gestureIsStarting)
                owner.beginGesture (vstParamID);
            else
                owner.endGesture (vstParamID);
        }

        DrxVST3EditController& owner;
        const Vst::ParamID vstParamID = Vst::kNoParamId;
        i32k parameterIndex = -1;
    };

    std::vector<std::unique_ptr<OwnedParameterListener>> ownedParameterListeners;

    //==============================================================================
    b8 inSetState = false;
    std::atomic<b8> vst3IsPlaying     { false },
                      inSetupProcessing { false };

    i32 lastLatencySamples = 0;
    b8 blueCatPatchwork = isBlueCatHost (hostContext.get());

   #if ! DRX_MAC
    f32 lastScaleFactorReceived = 1.0f;
   #endif

    InterfaceResultWithDeferredAddRef queryInterfaceInternal (const TUID targetIID)
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<FObject>{},
                                             UniqueBase<DrxVST3EditController>{},
                                             UniqueBase<Vst::IEditController>{},
                                             UniqueBase<Vst::IEditController2>{},
                                             UniqueBase<Vst::IConnectionPoint>{},
                                             UniqueBase<Vst::IMidiMapping>{},
                                             UniqueBase<Vst::IUnitInfo>{},
                                             UniqueBase<Vst::IRemapParamID>{},
                                             UniqueBase<Vst::ChannelContext::IInfoListener>{},
                                             SharedBase<IPluginBase, Vst::IEditController>{},
                                             UniqueBase<IDependent>{},
                                            #if DrxPlugin_Enable_ARA
                                             UniqueBase<Presonus::IPlugInViewEmbedding>{},
                                            #endif
                                             SharedBase<FUnknown, Vst::IEditController>{});

        if (result.isOk())
            return result;

        if (doUIDsMatch (targetIID, DrxAudioProcessor::iid))
            return { kResultOk, audioProcessor.get() };

        return {};
    }

    z0 installAudioProcessor (const VSTComSmartPtr<DrxAudioProcessor>& newAudioProcessor)
    {
        audioProcessor = newAudioProcessor;

        if (auto* extensions = audioProcessor->get()->getVST3ClientExtensions())
        {
            extensions->setIComponentHandler (componentHandler);
            extensions->setIHostApplication (hostContext.get());
        }

        if (auto* pluginInstance = getPluginInstance())
        {
            lastLatencySamples = pluginInstance->getLatencySamples();

            pluginInstance->addListener (this);

            // as the bypass is not part of the regular parameters we need to listen for it explicitly
            if (! audioProcessor->isBypassRegularParameter())
            {
                const auto paramID = audioProcessor->getBypassParamID();
                ownedParameterListeners.push_back (std::make_unique<OwnedParameterListener> (*this,
                                                                                             *audioProcessor->getParamForVSTParamID (paramID),
                                                                                             paramID,
                                                                                             audioProcessor->findCacheIndexForParamID (paramID)));
            }

            if (parameters.getParameterCount() <= 0)
            {
                auto n = audioProcessor->getParamIDs().size();

                for (i32 i = 0; i < n; ++i)
                {
                    auto vstParamID = audioProcessor->getVSTParamIDForIndex (i);

                    if (vstParamID == audioProcessor->getProgramParamID())
                        continue;

                    auto* juceParam = audioProcessor->getParamForVSTParamID (vstParamID);
                    auto* parameterGroup = pluginInstance->getParameterTree().getGroupsForParameter (juceParam).getLast();
                    auto unitID = DrxAudioProcessor::getUnitID (parameterGroup);

                    parameters.addParameter (new Param (*this, *juceParam, vstParamID, unitID,
                                                        (vstParamID == audioProcessor->getBypassParamID())));
                }

                const auto programParamId = audioProcessor->getProgramParamID();

                if (auto* programParam = audioProcessor->getParamForVSTParamID (programParamId))
                {
                    ownedParameterListeners.push_back (std::make_unique<OwnedParameterListener> (*this,
                                                                                                 *programParam,
                                                                                                 programParamId,
                                                                                                 audioProcessor->findCacheIndexForParamID (programParamId)));

                    parameters.addParameter (new ProgramChangeParameter (*pluginInstance, audioProcessor->getProgramParamID()));
                }
            }

           #if DRX_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS
            parameterToMidiControllerOffset = static_cast<Vst::ParamID> (audioProcessor->isUsingManagedParameters() ? DrxAudioProcessor::paramMidiControllerOffset
                                                                                                                    : parameters.getParameterCount());

            initialiseMidiControllerMappings();
           #endif

            audioProcessorChanged (pluginInstance, ChangeDetails().withParameterInfoChanged (true));
        }
    }

    z0 initialiseMidiControllerMappings()
    {
        for (i32 c = 0, p = 0; c < numMIDIChannels; ++c)
        {
            for (i32 i = 0; i < Vst::kCountCtrlNumber; ++i, ++p)
            {
                midiControllerToParameter[c][i] = static_cast<Vst::ParamID> (p) + parameterToMidiControllerOffset;
                parameterToMidiController[p].channel = c;
                parameterToMidiController[p].ctrlNumber = i;

                parameters.addParameter (new Vst::Parameter (toString ("MIDI CC " + Txt (c) + "|" + Txt (i)),
                                         static_cast<Vst::ParamID> (p) + parameterToMidiControllerOffset, nullptr, 0, 0,
                                         0, Vst::kRootUnitId));
            }
        }
    }

    z0 sendIntMessage (tukk idTag, const Steinberg::z64 value)
    {
        jassert (hostContext != nullptr);

        if (auto message = becomeVSTComSmartPtrOwner (allocateMessage()))
        {
            message->setMessageID (idTag);
            message->getAttributes()->setInt (idTag, value);
            sendMessage (message.get());
        }
    }

    class EditorContextMenu final : public HostProvidedContextMenu
    {
    public:
        EditorContextMenu (AudioProcessorEditor& editorIn,
                           VSTComSmartPtr<Vst::IContextMenu> contextMenuIn)
            : editor (editorIn), contextMenu (contextMenuIn) {}

        PopupMenu getEquivalentPopupMenu() const override
        {
            using MenuItem   = Vst::IContextMenuItem;
            using MenuTarget = Vst::IContextMenuTarget;

            struct Submenu
            {
                PopupMenu menu;
                Txt name;
                b8 enabled;
            };

            std::vector<Submenu> menuStack (1);

            for (i32 i = 0, end = contextMenu->getItemCount(); i < end; ++i)
            {
                MenuItem item{};
                MenuTarget* target = nullptr;
                contextMenu->getItem (i, item, &target);

                if ((item.flags & MenuItem::kIsGroupStart) == MenuItem::kIsGroupStart)
                {
                    menuStack.push_back ({ PopupMenu{},
                                           toString (item.name),
                                           (item.flags & MenuItem::kIsDisabled) == 0 });
                }
                else if ((item.flags & MenuItem::kIsGroupEnd) == MenuItem::kIsGroupEnd)
                {
                    const auto back = menuStack.back();
                    menuStack.pop_back();

                    if (menuStack.empty())
                    {
                        // malformed menu
                        jassertfalse;
                        return {};
                    }

                    menuStack.back().menu.addSubMenu (back.name, back.menu, back.enabled);
                }
                else if ((item.flags & MenuItem::kIsSeparator) == MenuItem::kIsSeparator)
                {
                    menuStack.back().menu.addSeparator();
                }
                else
                {
                    const auto callback = [menu = contextMenu, i]
                    {
                        MenuItem localItem{};
                        MenuTarget* localTarget = nullptr;

                        if (menu->getItem (i, localItem, &localTarget) == kResultOk && localTarget != nullptr)
                            localTarget->executeMenuItem (localItem.tag);
                    };

                    menuStack.back().menu.addItem (toString (item.name),
                                                   (item.flags & MenuItem::kIsDisabled) == 0,
                                                   (item.flags & MenuItem::kIsChecked) != 0,
                                                   callback);
                }
            }

            if (menuStack.size() != 1)
            {
                // malformed menu
                jassertfalse;
                return {};
            }

            return menuStack.back().menu;
        }

        z0 showNativeMenu (Point<i32> pos) const override
        {
            const auto scaled = pos * Component::getApproximateScaleFactorForComponent (&editor);
            contextMenu->popup (scaled.x, scaled.y);
        }

    private:
        AudioProcessorEditor& editor;
        VSTComSmartPtr<Vst::IContextMenu> contextMenu;
    };

    class EditorHostContext final : public AudioProcessorEditorHostContext
    {
    public:
        EditorHostContext (DrxAudioProcessor& processorIn,
                           AudioProcessorEditor& editorIn,
                           Vst::IComponentHandler* handler,
                           IPlugView* viewIn)
            : processor (processorIn), editor (editorIn), componentHandler (handler), view (viewIn) {}

        std::unique_ptr<HostProvidedContextMenu> getContextMenuForParameter (const AudioProcessorParameter* parameter) const override
        {
            if (componentHandler == nullptr || view == nullptr)
                return {};

            FUnknownPtr<Vst::IComponentHandler3> handler (componentHandler);

            if (handler == nullptr)
                return {};

            const auto idToUse = parameter != nullptr ? processor.getVSTParamIDForIndex (parameter->getParameterIndex()) : 0;
            const auto menu = becomeVSTComSmartPtrOwner (handler->createContextMenu (view, &idToUse));
            return std::make_unique<EditorContextMenu> (editor, menu);
        }

    private:
        DrxAudioProcessor& processor;
        AudioProcessorEditor& editor;
        Vst::IComponentHandler* componentHandler = nullptr;
        IPlugView* view = nullptr;
    };

    //==============================================================================
    class DrxVST3Editor final : public Vst::EditorView,
                                 public Vst::IParameterFinder,
                                 public IPlugViewContentScaleSupport,
                                 private Timer
    {
    public:
        DrxVST3Editor (DrxVST3EditController& ec, DrxAudioProcessor& p)
            : EditorView (&ec, nullptr),
              owner (addVSTComSmartPtrOwner (&ec)),
              pluginInstance (*p.get())
        {
            createContentWrapperComponentIfNeeded();

           #if DRX_MAC
            if (detail::PluginUtilities::getHostType().type == PluginHostType::SteinbergCubase10)
                cubase10Workaround.reset (new Cubase10WindowResizeWorkaround (*this));
           #endif
        }

        ~DrxVST3Editor() override = default; // NOLINT

        tresult PLUGIN_API queryInterface (const TUID targetIID, uk* obj) override
        {
            const auto result = testForMultiple (*this,
                                                 targetIID,
                                                 UniqueBase<Vst::IParameterFinder>{},
                                                 UniqueBase<IPlugViewContentScaleSupport>{});

            if (result.isOk())
                return result.extract (obj);

            return Vst::EditorView::queryInterface (targetIID, obj);
        }

        // NOLINTBEGIN
        REFCOUNT_METHODS (Vst::EditorView)
        // NOLINTEND

        //==============================================================================
        tresult PLUGIN_API isPlatformTypeSupported (FIDString type) override
        {
            if (type != nullptr && pluginInstance.hasEditor())
            {
               #if DRX_WINDOWS
                if (strcmp (type, kPlatformTypeHWND) == 0)
               #elif DRX_MAC
                if (strcmp (type, kPlatformTypeNSView) == 0 || strcmp (type, kPlatformTypeHIView) == 0)
               #elif DRX_LINUX || DRX_BSD
                if (strcmp (type, kPlatformTypeX11EmbedWindowID) == 0)
               #endif
                    return kResultTrue;
            }

            return kResultFalse;
        }

        tresult PLUGIN_API attached (uk parent, FIDString type) override
        {
            if (parent == nullptr || isPlatformTypeSupported (type) == kResultFalse)
                return kResultFalse;

            viewRunLoop.emplace (ScopedRunLoop::getRunLoopFromFrame (plugFrame));

            systemWindow = parent;

            createContentWrapperComponentIfNeeded();

            const auto desktopFlags = detail::PluginUtilities::getDesktopFlags (component->pluginEditor.get());

           #if DRX_WINDOWS || DRX_LINUX || DRX_BSD
            // If the plugin was last opened at a particular scale, try to reapply that scale here.
            // Note that we do this during attach(), rather than in DrxVST3Editor(). During the
            // constructor, we don't have a host plugFrame, so
            // ContentWrapperComponent::resizeHostWindow() won't do anything, and the content
            // wrapper component will be left at the wrong size.
            applyScaleFactor (StoredScaleFactor{}.withInternal (owner->lastScaleFactorReceived));

            // Check the host scale factor *before* calling addToDesktop, so that the initial
            // window size during addToDesktop is correct for the current platform scale factor.
            #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
             component->checkHostWindowScaleFactor();
            #endif

            component->setOpaque (true);
            component->addToDesktop (desktopFlags, systemWindow);
            component->setVisible (true);

            #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
             component->startTimer (500);
            #endif

           #else
            macHostWindow = detail::VSTWindowUtilities::attachComponentToWindowRefVST (component.get(), desktopFlags, parent);
           #endif

            component->resizeHostWindow();
            attachedToParent();

            // Life's too short to faff around with wave lab
            if (detail::PluginUtilities::getHostType().isWavelab())
                startTimer (200);

            return kResultTrue;
        }

        tresult PLUGIN_API removed() override
        {
            if (component != nullptr)
            {
               #if DRX_WINDOWS
                component->removeFromDesktop();
               #elif DRX_MAC
                if (macHostWindow != nullptr)
                {
                    detail::VSTWindowUtilities::detachComponentFromWindowRefVST (component.get(), macHostWindow);
                    macHostWindow = nullptr;
                }
               #endif

                component = nullptr;
                lastReportedSize.reset();
            }

            viewRunLoop.reset();

            return CPluginView::removed();
        }

        tresult PLUGIN_API onSize (ViewRect* newSize) override
        {
            if (newSize == nullptr)
            {
                jassertfalse;
                return kResultFalse;
            }

            lastReportedSize.reset();
            rect = roundToViewRect (convertFromHostBounds (*newSize));

            if (component == nullptr)
                return kResultTrue;

            component->setSize (rect.getWidth(), rect.getHeight());

           #if DRX_MAC
            if (cubase10Workaround != nullptr)
            {
                cubase10Workaround->triggerAsyncUpdate();
            }
            else
           #endif
            {
                if (auto* peer = component->getPeer())
                    peer->updateBounds();
            }

            return kResultTrue;
        }

        tresult PLUGIN_API getSize (ViewRect* size) override
        {
           #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
            if (detail::PluginUtilities::getHostType().isAbletonLive() && systemWindow == nullptr)
                return kResultFalse;
           #endif

            if (size == nullptr || component == nullptr)
                return kResultFalse;

            const auto editorBounds = component->getSizeToContainChild();
            const auto sizeToReport = lastReportedSize.has_value()
                                    ? *lastReportedSize
                                    : convertToHostBounds (editorBounds.withZeroOrigin().toFloat());

            lastReportedSize = *size = sizeToReport;
            return kResultTrue;
        }

        tresult PLUGIN_API canResize() override
        {
            if (component != nullptr)
                if (auto* editor = component->pluginEditor.get())
                    if (editor->isResizable())
                        return kResultTrue;

            return kResultFalse;
        }

        tresult PLUGIN_API checkSizeConstraint (ViewRect* rectToCheck) override
        {
            if (rectToCheck != nullptr && component != nullptr)
            {
                if (auto* editor = component->pluginEditor.get())
                {
                    if (canResize() == kResultFalse)
                    {
                        // Ableton Live will call checkSizeConstraint even if the view returns false
                        // from canResize. Set the out param to an appropriate size for the editor
                        // and return.
                        auto constrainedRect = component->getLocalArea (editor, editor->getLocalBounds())
                                                        .getSmallestIntegerContainer();

                        *rectToCheck = roundToViewRect (convertFromHostBounds (*rectToCheck));
                        rectToCheck->right  = rectToCheck->left + roundToInt (constrainedRect.getWidth());
                        rectToCheck->bottom = rectToCheck->top  + roundToInt (constrainedRect.getHeight());
                        *rectToCheck = convertToHostBounds (createRectangle (*rectToCheck));
                    }
                    else if (auto* constrainer = editor->getConstrainer())
                    {
                        const auto clientBounds = convertFromHostBounds (*rectToCheck);
                        const auto editorBounds = editor->getLocalArea (component.get(), clientBounds);

                        auto minW = (f32) constrainer->getMinimumWidth();
                        auto maxW = (f32) constrainer->getMaximumWidth();
                        auto minH = (f32) constrainer->getMinimumHeight();
                        auto maxH = (f32) constrainer->getMaximumHeight();

                        auto width  = jlimit (minW, maxW, editorBounds.getWidth());
                        auto height = jlimit (minH, maxH, editorBounds.getHeight());

                        auto aspectRatio = (f32) constrainer->getFixedAspectRatio();

                        if (! approximatelyEqual (aspectRatio, 0.0f))
                        {
                            b8 adjustWidth = (width / height > aspectRatio);

                            if (detail::PluginUtilities::getHostType().type == PluginHostType::SteinbergCubase9)
                            {
                                auto currentEditorBounds = editor->getBounds().toFloat();

                                if (approximatelyEqual (currentEditorBounds.getWidth(), width) && ! approximatelyEqual (currentEditorBounds.getHeight(), height))
                                    adjustWidth = true;
                                else if (approximatelyEqual (currentEditorBounds.getHeight(), height) && ! approximatelyEqual (currentEditorBounds.getWidth(), width))
                                    adjustWidth = false;
                            }

                            if (adjustWidth)
                            {
                                width = height * aspectRatio;

                                if (width > maxW || width < minW)
                                {
                                    width = jlimit (minW, maxW, width);
                                    height = width / aspectRatio;
                                }
                            }
                            else
                            {
                                height = width / aspectRatio;

                                if (height > maxH || height < minH)
                                {
                                    height = jlimit (minH, maxH, height);
                                    width = height * aspectRatio;
                                }
                            }
                        }

                        auto constrainedRect = component->getLocalArea (editor, Rectangle<f32> (width, height));

                        *rectToCheck = convertToHostBounds (clientBounds.withWidth (constrainedRect.getWidth())
                                                                        .withHeight (constrainedRect.getHeight()));
                    }
                }

                return kResultTrue;
            }

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API setContentScaleFactor ([[maybe_unused]] const IPlugViewContentScaleSupport::ScaleFactor factor) override
        {
           #if ! DRX_MAC
            const auto scaleToApply = [&]
            {
               #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
                // Cubase 10 only sends integer scale factors, so correct this for fractional scales
                if (detail::PluginUtilities::getHostType().type != PluginHostType::SteinbergCubase10)
                    return factor;

                const auto hostWindowScale = (IPlugViewContentScaleSupport::ScaleFactor) getScaleFactorForWindow (static_cast<HWND> (systemWindow));

                if (hostWindowScale <= 0.0 || approximatelyEqual (factor, hostWindowScale))
                    return factor;

                return hostWindowScale;
               #else
                return factor;
               #endif
            }();

            applyScaleFactor (scaleFactor.withHost (scaleToApply));

            return kResultTrue;
           #else
            return kResultFalse;
           #endif
        }

        tresult PLUGIN_API findParameter (i32 xPos, i32 yPos, Vst::ParamID& resultTag) override
        {
            if (const auto paramId = findParameterImpl (xPos, yPos))
            {
                resultTag = *paramId;
                return kResultTrue;
            }

            return kResultFalse;
        }

    private:
        std::optional<Vst::ParamID> findParameterImpl (i32 xPos, i32 yPos) const
        {
            auto* wrapper = component.get();

            if (wrapper == nullptr)
                return {};

            auto* componentAtPosition = wrapper->getComponentAt (xPos, yPos);

            if (componentAtPosition == nullptr)
                return {};

            auto* editor = wrapper->pluginEditor.get();

            if (editor == nullptr)
                return {};

            const auto parameterIndex = editor->getControlParameterIndex (*componentAtPosition);

            if (parameterIndex < 0)
                return {};

            auto processor = owner->audioProcessor;

            if (processor == nullptr)
                return {};

            return processor->getVSTParamIDForIndex (parameterIndex);
        }

        z0 timerCallback() override
        {
            stopTimer();

            ViewRect viewRect;
            getSize (&viewRect);
            onSize (&viewRect);
        }

        static ViewRect roundToViewRect (Rectangle<f32> r)
        {
            const auto rounded = r.toNearestIntEdges();
            return { rounded.getX(),
                     rounded.getY(),
                     rounded.getRight(),
                     rounded.getBottom() };
        }

        static Rectangle<f32> createRectangle (ViewRect viewRect)
        {
            return Rectangle<f32>::leftTopRightBottom ((f32) viewRect.left,
                                                         (f32) viewRect.top,
                                                         (f32) viewRect.right,
                                                         (f32) viewRect.bottom);
        }

        static ViewRect convertToHostBounds (Rectangle<f32> pluginRect)
        {
            const auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();
            return roundToViewRect (approximatelyEqual (desktopScale, 1.0f) ? pluginRect
                                                                            : pluginRect * desktopScale);
        }

        static Rectangle<f32> convertFromHostBounds (ViewRect hostViewRect)
        {
            const auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();
            const auto hostRect = createRectangle (hostViewRect);

            return approximatelyEqual (desktopScale, 1.0f) ? hostRect
                                                           : (hostRect / desktopScale);
        }

        //==============================================================================
        struct ContentWrapperComponent final : public Component
                                            #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
                                             , public Timer
                                            #endif
        {
            ContentWrapperComponent (DrxVST3Editor& editor)  : owner (editor)
            {
                setOpaque (true);
                setBroughtToFrontOnMouseClick (true);
            }

            ~ContentWrapperComponent() override
            {
                if (pluginEditor != nullptr)
                {
                    PopupMenu::dismissAllActiveMenus();
                    pluginEditor->processor.editorBeingDeleted (pluginEditor.get());
                }
            }

            z0 createEditor (AudioProcessor& plugin)
            {
                pluginEditor.reset (plugin.createEditorIfNeeded());

               #if DrxPlugin_Enable_ARA
                jassert (dynamic_cast<AudioProcessorEditorARAExtension*> (pluginEditor.get()) != nullptr);
                // for proper view embedding, ARA plug-ins must be resizable
                jassert (pluginEditor->isResizable());
               #endif

                if (pluginEditor != nullptr)
                {
                    editorHostContext = std::make_unique<EditorHostContext> (*owner.owner->audioProcessor,
                                                                             *pluginEditor,
                                                                             owner.owner->getComponentHandler(),
                                                                             &owner);

                    pluginEditor->setHostContext (editorHostContext.get());
                   #if ! DRX_MAC
                    pluginEditor->setScaleFactor (owner.scaleFactor.get());
                   #endif

                    addAndMakeVisible (pluginEditor.get());
                    pluginEditor->setTopLeftPosition (0, 0);

                    lastBounds = getSizeToContainChild();

                    {
                        const ScopedValueSetter<b8> resizingParentSetter (resizingParent, true);
                        setBounds (lastBounds);
                    }

                    resizeHostWindow();
                }
                else
                {
                    // if hasEditor() returns true then createEditorIfNeeded has to return a valid editor
                    jassertfalse;
                }
            }

            z0 paint (Graphics& g) override
            {
                g.fillAll (Colors::black);
            }

            drx::Rectangle<i32> getSizeToContainChild()
            {
                if (pluginEditor != nullptr)
                    return getLocalArea (pluginEditor.get(), pluginEditor->getLocalBounds());

                return {};
            }

            z0 childBoundsChanged (Component*) override
            {
                if (resizingChild)
                    return;

                auto newBounds = getSizeToContainChild();

                if (newBounds != lastBounds)
                {
                    resizeHostWindow();

                   #if DRX_LINUX || DRX_BSD
                    if (detail::PluginUtilities::getHostType().isBitwigStudio())
                        repaint();
                   #endif

                    lastBounds = newBounds;
                }
            }

            z0 resized() override
            {
                if (pluginEditor != nullptr)
                {
                    if (! resizingParent)
                    {
                        auto newBounds = getLocalBounds();

                        {
                            const ScopedValueSetter<b8> resizingChildSetter (resizingChild, true);
                            pluginEditor->setBounds (pluginEditor->getLocalArea (this, newBounds).withZeroOrigin());
                        }

                        lastBounds = newBounds;
                    }
                }
            }

            z0 parentSizeChanged() override
            {
                if (pluginEditor != nullptr)
                {
                    resizeHostWindow();
                    pluginEditor->repaint();
                }
            }

            z0 resizeHostWindow()
            {
                if (pluginEditor != nullptr)
                {
                    if (owner.plugFrame != nullptr)
                    {
                        auto editorBounds = getSizeToContainChild();
                        auto newSize = convertToHostBounds (editorBounds.withZeroOrigin().toFloat());

                        {
                            const ScopedValueSetter<b8> resizingParentSetter (resizingParent, true);
                            owner.plugFrame->resizeView (&owner, &newSize);
                        }

                        auto host = detail::PluginUtilities::getHostType();

                       #if DRX_MAC
                        if (host.isWavelab() || host.isReaper() || owner.owner->blueCatPatchwork)
                       #else
                        if (host.isWavelab() || host.isAbletonLive() || host.isBitwigStudio() || owner.owner->blueCatPatchwork)
                       #endif
                            setBounds (editorBounds.withZeroOrigin());
                    }
                }
            }

            z0 setEditorScaleFactor (f32 scale)
            {
                if (pluginEditor != nullptr)
                {
                    auto prevEditorBounds = pluginEditor->getLocalArea (this, lastBounds);

                    {
                        const ScopedValueSetter<b8> resizingChildSetter (resizingChild, true);

                        pluginEditor->setScaleFactor (scale);
                        pluginEditor->setBounds (prevEditorBounds.withZeroOrigin());
                    }

                    lastBounds = getSizeToContainChild();

                    resizeHostWindow();
                    repaint();
                }
            }

           #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
            z0 checkHostWindowScaleFactor()
            {
                const auto estimatedScale = (f32) getScaleFactorForWindow (static_cast<HWND> (owner.systemWindow));

                if (estimatedScale > 0.0)
                    owner.applyScaleFactor (owner.scaleFactor.withInternal (estimatedScale));
            }

            z0 timerCallback() override
            {
                checkHostWindowScaleFactor();
            }
           #endif

            std::unique_ptr<AudioProcessorEditor> pluginEditor;

        private:
            DrxVST3Editor& owner;
            std::unique_ptr<EditorHostContext> editorHostContext;
            Rectangle<i32> lastBounds;
            b8 resizingChild = false, resizingParent = false;

            DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentWrapperComponent)
        };

        z0 createContentWrapperComponentIfNeeded()
        {
            if (component == nullptr)
            {
               #if DRX_LINUX || DRX_BSD
                const MessageManagerLock mmLock;
               #endif

                component.reset (new ContentWrapperComponent (*this));
                component->createEditor (pluginInstance);
            }
        }

        //==============================================================================
        std::optional<ScopedRunLoop> viewRunLoop;
        std::optional<ViewRect> lastReportedSize;

        VSTComSmartPtr<DrxVST3EditController> owner;
        AudioProcessor& pluginInstance;

       #if DRX_LINUX || DRX_BSD
        struct MessageManagerLockedDeleter
        {
            template <typename ObjectType>
            z0 operator() (ObjectType* object) const noexcept
            {
                const MessageManagerLock mmLock;
                delete object;
            }
        };

        std::unique_ptr<ContentWrapperComponent, MessageManagerLockedDeleter> component;
       #else
        std::unique_ptr<ContentWrapperComponent> component;
       #endif

        friend ContentWrapperComponent;

       #if DRX_MAC
        uk macHostWindow = nullptr;

        // On macOS Cubase 10 resizes the host window after calling onSize() resulting in the peer
        // bounds being a step behind the plug-in. Calling updateBounds() asynchronously seems to fix things...
        struct Cubase10WindowResizeWorkaround final : public AsyncUpdater
        {
            Cubase10WindowResizeWorkaround (DrxVST3Editor& o)  : owner (o) {}

            z0 handleAsyncUpdate() override
            {
                if (owner.component != nullptr)
                    if (auto* peer = owner.component->getPeer())
                        peer->updateBounds();
            }

            DrxVST3Editor& owner;
        };

        std::unique_ptr<Cubase10WindowResizeWorkaround> cubase10Workaround;
       #else
        class StoredScaleFactor
        {
        public:
            StoredScaleFactor withHost     (f32 x) const { return withMember (*this, &StoredScaleFactor::host,     x); }
            StoredScaleFactor withInternal (f32 x) const { return withMember (*this, &StoredScaleFactor::internal, x); }
            f32 get() const { return host.value_or (internal); }

        private:
            std::optional<f32> host;
            f32 internal = 1.0f;
        };

        z0 applyScaleFactor (const StoredScaleFactor newFactor)
        {
            const auto previous = std::exchange (scaleFactor, newFactor).get();

            if (approximatelyEqual (previous, scaleFactor.get()))
                return;

            if (owner != nullptr)
                owner->lastScaleFactorReceived = scaleFactor.get();

            if (component != nullptr)
            {
               #if DRX_LINUX || DRX_BSD
                const MessageManagerLock mmLock;
               #endif
                component->setEditorScaleFactor (scaleFactor.get());
            }
        }

        StoredScaleFactor scaleFactor;

        #if DRX_WINDOWS
         detail::WindowsHooks hooks;
        #endif

       #endif

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrxVST3Editor)
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrxVST3EditController)
};


//==============================================================================
#if DrxPlugin_Enable_ARA
 class DrxARAFactory final : public ARA::IMainFactory
 {
 public:
    DrxARAFactory() = default;
    virtual ~DrxARAFactory() = default;

    DRX_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API queryInterface (const ::Steinberg::TUID targetIID, uk* obj) override
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<ARA::IMainFactory>{},
                                             UniqueBase<FUnknown>{});

        if (result.isOk())
            return result.extract (obj);

        if (doUIDsMatch (targetIID, DrxARAFactory::iid))
        {
            addRef();
            *obj = this;
            return kResultOk;
        }

        *obj = nullptr;
        return kNoInterface;
    }

    //---from ARA::IMainFactory-------
    const ARA::ARAFactory* PLUGIN_API getFactory() SMTG_OVERRIDE
    {
        return createARAFactory();
    }

    inline static const FUID iid = toSteinbergUID (getInterfaceId (VST3InterfaceType::ara));

 private:
     //==============================================================================
     std::atomic<i32> refCount { 1 };
 };
#endif

//==============================================================================
class DrxVST3Component final : public Vst::IComponent,
                                public Vst::IAudioProcessor,
                                public Vst::IUnitInfo,
                                public Vst::IConnectionPoint,
                                public Vst::IProcessContextRequirements,
                               #if DrxPlugin_Enable_ARA
                                public ARA::IPlugInEntryPoint,
                                public ARA::IPlugInEntryPoint2,
                               #endif
                                public AudioPlayHead
{
public:
    DrxVST3Component (const VSTComSmartPtr<Vst::IHostApplication>& h,
                       const RunLoop& l)
        : scopedRunLoop (l),
          pluginInstance (createPluginFilterOfType (AudioProcessor::wrapperType_VST3).release()),
          host (h)
    {
        inParameterChangedCallback = false;

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = { DrxPlugin_PreferredChannelConfigurations };
        [[maybe_unused]] i32k numConfigs = numElementsInArray (configs);

        jassert (numConfigs > 0 && (configs[0][0] > 0 || configs[0][1] > 0));

        pluginInstance->setPlayConfigDetails (configs[0][0], configs[0][1], 44100.0, 1024);
       #endif

        // VST-3 requires your default layout to be non-discrete!
        // For example, your default layout must be mono, stereo, quadrophonic
        // and not AudioChannelSet::discreteChannels (2) etc.
        jassert (checkBusFormatsAreNotDiscrete());

        comPluginInstance = addVSTComSmartPtrOwner (new DrxAudioProcessor (pluginInstance));

        zerostruct (processContext);

        processSetup.maxSamplesPerBlock = 1024;
        processSetup.processMode = Vst::kRealtime;
        processSetup.sampleRate = 44100.0;
        processSetup.symbolicSampleSize = Vst::kSample32;

        pluginInstance->setPlayHead (this);

        // Constructing the underlying static object involves dynamic allocation.
        // This call ensures that the construction won't happen on the audio thread.
        detail::PluginUtilities::getHostType();
    }

    ~DrxVST3Component() override
    {
        if (juceVST3EditController != nullptr)
            juceVST3EditController->vst3IsPlaying = false;

        if (pluginInstance != nullptr)
            if (pluginInstance->getPlayHead() == this)
                pluginInstance->setPlayHead (nullptr);
    }

    //==============================================================================
    AudioProcessor& getPluginInstance() const noexcept { return *pluginInstance; }

    //==============================================================================
    inline static const FUID iid = toSteinbergUID (getInterfaceId (VST3InterfaceType::component));

    DRX_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API queryInterface (const TUID targetIID, uk* obj) override
    {
        const auto userProvidedInterface = queryAdditionalInterfaces (&getPluginInstance(),
                                                                      targetIID,
                                                                      &VST3ClientExtensions::queryIAudioProcessor);

        const auto juceProvidedInterface = queryInterfaceInternal (targetIID);

        return extractResult (userProvidedInterface, juceProvidedInterface, obj);
    }

    enum class CallPrepareToPlay { no, yes };

    //==============================================================================
    tresult PLUGIN_API initialize (FUnknown* hostContext) override
    {
        if (host.get() != hostContext)
            host.loadFrom (hostContext);

        processContext.sampleRate = processSetup.sampleRate;
        preparePlugin (processSetup.sampleRate, (i32) processSetup.maxSamplesPerBlock, CallPrepareToPlay::no);

        return kResultTrue;
    }

    tresult PLUGIN_API terminate() override
    {
        getPluginInstance().releaseResources();
        return kResultTrue;
    }

    //==============================================================================
    tresult PLUGIN_API connect (IConnectionPoint* other) override
    {
        if (other != nullptr && juceVST3EditController == nullptr)
            juceVST3EditController.loadFrom (other);

        return kResultTrue;
    }

    tresult PLUGIN_API disconnect (IConnectionPoint*) override
    {
        if (juceVST3EditController != nullptr)
            juceVST3EditController->vst3IsPlaying = false;

        juceVST3EditController = {};
        return kResultTrue;
    }

    tresult PLUGIN_API notify (Vst::IMessage* message) override
    {
        if (message != nullptr && juceVST3EditController == nullptr)
        {
            Steinberg::z64 value = 0;

            if (message->getAttributes()->getInt ("DrxVST3EditController", value) == kResultTrue)
            {
                juceVST3EditController = addVSTComSmartPtrOwner ((DrxVST3EditController*) (pointer_sized_int) value);

                if (juceVST3EditController != nullptr)
                    juceVST3EditController->setAudioProcessor (comPluginInstance.get());
                else
                    jassertfalse;
            }
        }

        return kResultTrue;
    }

    tresult PLUGIN_API getControllerClassId (TUID classID) override
    {
        memcpy (classID, DrxVST3EditController::iid, sizeof (TUID));
        return kResultTrue;
    }

    //==============================================================================
    tresult PLUGIN_API setActive (TBool state) override
    {
        const FLStudioDIYSpecificationEnforcementLock lock (flStudioDIYSpecificationEnforcementMutex);

        const auto willBeActive = (state != 0);

        active = false;
        // Some hosts may call setBusArrangements in response to calls made during prepareToPlay
        // or releaseResources. Specifically, Wavelab 11.1 calls setBusArrangements in the same
        // call stack when the AudioProcessor calls setLatencySamples inside prepareToPlay.
        // In order for setBusArrangements to return successfully, the plugin must not be activated
        // until after prepareToPlay has completely finished.
        const ScopeGuard scope { [&] { active = willBeActive; } };

        if (willBeActive)
        {
            const auto sampleRate = processSetup.sampleRate > 0.0
                                  ? processSetup.sampleRate
                                  : getPluginInstance().getSampleRate();

            const auto bufferSize = processSetup.maxSamplesPerBlock > 0
                                  ? (i32) processSetup.maxSamplesPerBlock
                                  : getPluginInstance().getBlockSize();

            preparePlugin (sampleRate, bufferSize, CallPrepareToPlay::yes);
        }
        else
        {
            getPluginInstance().releaseResources();
        }

        return kResultOk;
    }

    tresult PLUGIN_API setIoMode (Vst::IoMode) override                                 { return kNotImplemented; }
    tresult PLUGIN_API getRoutingInfo (Vst::RoutingInfo&, Vst::RoutingInfo&) override   { return kNotImplemented; }

    //==============================================================================
    b8 isBypassed() const
    {
        if (auto* bypassParam = comPluginInstance->getBypassParameter())
            return bypassParam->getValue() >= 0.5f;

        return false;
    }

    z0 setBypassed (b8 shouldBeBypassed)
    {
        if (auto* bypassParam = comPluginInstance->getBypassParameter())
            setValueAndNotifyIfChanged (*bypassParam, shouldBeBypassed ? 1.0f : 0.0f);
    }

    //==============================================================================
    z0 writeDrxPrivateStateInformation (MemoryOutputStream& out)
    {
        if (pluginInstance->getBypassParameter() == nullptr)
        {
            ValueTree privateData (kDrxPrivateDataIdentifier);

            // for now we only store the bypass value
            privateData.setProperty ("Bypass", var (isBypassed()), nullptr);
            privateData.writeToStream (out);
        }
    }

    z0 setDrxPrivateStateInformation (ukk data, i32 sizeInBytes)
    {
        if (pluginInstance->getBypassParameter() == nullptr)
        {
            if (comPluginInstance->getBypassParameter() != nullptr)
            {
                auto privateData = ValueTree::readFromData (data, static_cast<size_t> (sizeInBytes));
                setBypassed (static_cast<b8> (privateData.getProperty ("Bypass", var (false))));
            }
        }
    }

    z0 getStateInformation (MemoryBlock& destData)
    {
        pluginInstance->getStateInformation (destData);

        // With bypass support, DRX now needs to store private state data.
        // Put this at the end of the plug-in state and add a few null characters
        // so that plug-ins built with older versions of DRX will hopefully ignore
        // this data. Additionally, we need to add some sort of magic identifier
        // at the very end of the private data so that DRX has some sort of
        // way to figure out if the data was stored with a newer DRX version.
        MemoryOutputStream extraData;

        extraData.writeInt64 (0);
        writeDrxPrivateStateInformation (extraData);
        auto privateDataSize = (z64) (extraData.getDataSize() - sizeof (z64));
        extraData.writeInt64 (privateDataSize);
        extraData << kDrxPrivateDataIdentifier;

        // write magic string
        destData.append (extraData.getData(), extraData.getDataSize());
    }

    z0 setStateInformation (ukk data, i32 sizeAsInt)
    {
        b8 unusedState = false;
        auto& flagToSet = juceVST3EditController != nullptr ? juceVST3EditController->inSetState : unusedState;
        const ScopedValueSetter<b8> scope (flagToSet, true);

        auto size = (zu64) sizeAsInt;

        // Check if this data was written with a newer DRX version
        // and if it has the DRX private data magic code at the end
        auto jucePrivDataIdentifierSize = std::strlen (kDrxPrivateDataIdentifier);

        if ((size_t) size >= jucePrivDataIdentifierSize + sizeof (z64))
        {
            auto buffer = static_cast<tukk> (data);

            Txt magic (CharPointer_UTF8 (buffer + size - jucePrivDataIdentifierSize),
                          CharPointer_UTF8 (buffer + size));

            if (magic == kDrxPrivateDataIdentifier)
            {
                // found a DRX private data section
                zu64 privateDataSize;

                std::memcpy (&privateDataSize,
                             buffer + ((size_t) size - jucePrivDataIdentifierSize - sizeof (zu64)),
                             sizeof (zu64));

                privateDataSize = ByteOrder::swapIfBigEndian (privateDataSize);
                size -= privateDataSize + jucePrivDataIdentifierSize + sizeof (zu64);

                if (privateDataSize > 0)
                    setDrxPrivateStateInformation (buffer + size, static_cast<i32> (privateDataSize));

                size -= sizeof (zu64);
            }
        }

        if (size > 0)
            pluginInstance->setStateInformation (data, static_cast<i32> (size));
    }

    //==============================================================================
    b8 shouldTryToLoadVst2State()
    {
       #if DRX_VST3_CAN_REPLACE_VST2
        return true;
       #else
        if (auto extensions = pluginInstance->getVST3ClientExtensions())
            return ! extensions->getCompatibleClasses().empty();

        return false;
       #endif
    }

    b8 shouldWriteStateWithVst2Compatibility()
    {
       #if DRX_VST3_CAN_REPLACE_VST2
        return true;
       #else
        return false;
       #endif
    }

    b8 readFromMemoryStream (IBStream* state)
    {
        FUnknownPtr<ISizeableStream> s (state);
        Steinberg::z64 size = 0;

        if (s != nullptr
             && s->getStreamSize (size) == kResultOk
             && size > 0
             && size < 1024 * 1024 * 100) // (some hosts seem to return junk for the size)
        {
            MemoryBlock block (static_cast<size_t> (size));

            // turns out that Cubase 9 might give you the incorrect stream size :-(
            Steinberg::i32 bytesRead = 1;
            i32 len;

            for (len = 0; bytesRead > 0 && len < static_cast<i32> (block.getSize()); len += bytesRead)
                if (state->read (block.getData(), static_cast<i32> (block.getSize()), &bytesRead) != kResultOk)
                    break;

            if (len == 0)
                return false;

            block.setSize (static_cast<size_t> (len));

            // Adobe Audition CS6 hack to avoid trying to use corrupted streams:
            if (detail::PluginUtilities::getHostType().isAdobeAudition())
                if (block.getSize() >= 5 && memcmp (block.getData(), "VC2!E", 5) == 0)
                    return false;

            setStateInformation (block.getData(), (i32) block.getSize());
            return true;
        }

        return false;
    }

    b8 readFromUnknownStream (IBStream* state)
    {
        MemoryOutputStream allData;

        {
            const size_t bytesPerBlock = 4096;
            HeapBlock<t8> buffer (bytesPerBlock);

            for (;;)
            {
                Steinberg::i32 bytesRead = 0;
                auto status = state->read (buffer, (Steinberg::i32) bytesPerBlock, &bytesRead);

                if (bytesRead <= 0 || (status != kResultTrue && ! detail::PluginUtilities::getHostType().isWavelab()))
                    break;

                allData.write (buffer, static_cast<size_t> (bytesRead));
            }
        }

        const size_t dataSize = allData.getDataSize();

        if (dataSize <= 0 || dataSize >= 0x7fffffff)
            return false;

        setStateInformation (allData.getData(), (i32) dataSize);
        return true;
    }

    b8 readVst2State (IBStream* state)
    {
        if (auto vst2State = VST3::tryVst2StateLoad (*state))
        {
            setStateInformation (vst2State->chunk.data(), (i32) vst2State->chunk.size());
            return true;
        }

        return false;
    }

    tresult PLUGIN_API setState (IBStream* state) override
    {
        // The VST3 spec requires that this function is called from the UI thread.
        // If this assertion fires, your host is misbehaving!
        assertHostMessageThread();

        if (state == nullptr)
            return kInvalidArgument;

        FUnknownPtr<IBStream> stateRefHolder (state); // just in case the caller hasn't properly ref-counted the stream object

        const auto seekToBeginningOfStream = [&]
        {
            return state->seek (0, IBStream::kIBSeekSet, nullptr) == kResultTrue;
        };

        if (seekToBeginningOfStream() && shouldTryToLoadVst2State() && readVst2State (state))
            return kResultTrue;

        if (seekToBeginningOfStream() && ! detail::PluginUtilities::getHostType().isFruityLoops() && readFromMemoryStream (state))
            return kResultTrue;

        if (seekToBeginningOfStream() && readFromUnknownStream (state))
            return kResultTrue;

        return kResultFalse;
    }

    tresult getStateWithVst2Compatibility (const MemoryBlock& dataChunk, IBStream& outState)
    {
        VST3::Vst2xState vst2State;

        vst2State.chunk.resize (dataChunk.getSize());
        std::copy (dataChunk.begin(), dataChunk.end(), vst2State.chunk.begin());

        vst2State.fxUniqueID = DrxPlugin_VSTUniqueID;
        vst2State.fxVersion = DrxPlugin_VersionCode;
        vst2State.isBypassed = isBypassed();

        if (VST3::writeVst2State (vst2State, outState))
            return kResultTrue;

        // Please inform the DRX team if you hit this assertion
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API getState (IBStream* state) override
    {
       if (state == nullptr)
           return kInvalidArgument;

        MemoryBlock mem;
        getStateInformation (mem);

        if (mem.isEmpty())
            return kResultFalse;

        if (shouldWriteStateWithVst2Compatibility())
            return getStateWithVst2Compatibility (mem, *state);

        return state->write (mem.getData(), (Steinberg::i32) mem.getSize());
    }

    //==============================================================================
    Steinberg::i32 PLUGIN_API getUnitCount() override                                                                         { return comPluginInstance->getUnitCount(); }
    tresult PLUGIN_API getUnitInfo (Steinberg::i32 unitIndex, Vst::UnitInfo& info) override                                   { return comPluginInstance->getUnitInfo (unitIndex, info); }
    Steinberg::i32 PLUGIN_API getProgramListCount() override                                                                  { return comPluginInstance->getProgramListCount(); }
    tresult PLUGIN_API getProgramListInfo (Steinberg::i32 listIndex, Vst::ProgramListInfo& info) override                     { return comPluginInstance->getProgramListInfo (listIndex, info); }
    tresult PLUGIN_API getProgramName (Vst::ProgramListID listId, Steinberg::i32 programIndex, Vst::String128 name) override  { return comPluginInstance->getProgramName (listId, programIndex, name); }
    tresult PLUGIN_API getProgramInfo (Vst::ProgramListID listId, Steinberg::i32 programIndex,
                                       Vst::CString attributeId, Vst::String128 attributeValue) override                        { return comPluginInstance->getProgramInfo (listId, programIndex, attributeId, attributeValue); }
    tresult PLUGIN_API hasProgramPitchNames (Vst::ProgramListID listId, Steinberg::i32 programIndex) override                 { return comPluginInstance->hasProgramPitchNames (listId, programIndex); }
    tresult PLUGIN_API getProgramPitchName (Vst::ProgramListID listId, Steinberg::i32 programIndex,
                                            Steinberg::i16 midiPitch, Vst::String128 name) override                           { return comPluginInstance->getProgramPitchName (listId, programIndex, midiPitch, name); }
    tresult PLUGIN_API selectUnit (Vst::UnitID unitId) override                                                                 { return comPluginInstance->selectUnit (unitId); }
    tresult PLUGIN_API setUnitProgramData (Steinberg::i32 listOrUnitId, Steinberg::i32 programIndex,
                                           IBStream* data) override                                                             { return comPluginInstance->setUnitProgramData (listOrUnitId, programIndex, data); }
    Vst::UnitID PLUGIN_API getSelectedUnit() override                                                                           { return comPluginInstance->getSelectedUnit(); }
    tresult PLUGIN_API getUnitByBus (Vst::MediaType type, Vst::BusDirection dir, Steinberg::i32 busIndex,
                                     Steinberg::i32 channel, Vst::UnitID& unitId) override                                    { return comPluginInstance->getUnitByBus (type, dir, busIndex, channel, unitId); }

    //==============================================================================
    Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        info.setTimeInSamples (jmax ((Steinberg::z64) 0, processContext.projectTimeSamples));
        info.setTimeInSeconds (static_cast<f64> (*info.getTimeInSamples()) / processContext.sampleRate);
        info.setIsRecording ((processContext.state & Vst::ProcessContext::kRecording) != 0);
        info.setIsPlaying ((processContext.state & Vst::ProcessContext::kPlaying) != 0);
        info.setIsLooping ((processContext.state & Vst::ProcessContext::kCycleActive) != 0);

        info.setBpm ((processContext.state & Vst::ProcessContext::kTempoValid) != 0
                     ? makeOptional (processContext.tempo)
                     : nullopt);

        info.setTimeSignature ((processContext.state & Vst::ProcessContext::kTimeSigValid) != 0
                               ? makeOptional (TimeSignature { processContext.timeSigNumerator, processContext.timeSigDenominator })
                               : nullopt);

        info.setLoopPoints ((processContext.state & Vst::ProcessContext::kCycleValid) != 0
                            ? makeOptional (LoopPoints { processContext.cycleStartMusic, processContext.cycleEndMusic })
                            : nullopt);

        info.setPpqPosition ((processContext.state & Vst::ProcessContext::kProjectTimeMusicValid) != 0
                             ? makeOptional (processContext.projectTimeMusic)
                             : nullopt);

        info.setPpqPositionOfLastBarStart ((processContext.state & Vst::ProcessContext::kBarPositionValid) != 0
                                           ? makeOptional (processContext.barPositionMusic)
                                           : nullopt);

        info.setFrameRate ((processContext.state & Vst::ProcessContext::kSmpteValid) != 0
                           ? makeOptional (FrameRate().withBaseRate ((i32) processContext.frameRate.framesPerSecond)
                                                      .withDrop ((processContext.frameRate.flags & Vst::FrameRate::kDropRate) != 0)
                                                      .withPullDown ((processContext.frameRate.flags & Vst::FrameRate::kPullDownRate) != 0))
                           : nullopt);

        info.setEditOriginTime (info.getFrameRate().hasValue()
                                ? makeOptional ((f64) processContext.smpteOffsetSubframes / (80.0 * info.getFrameRate()->getEffectiveRate()))
                                : nullopt);

        info.setHostTimeNs ((processContext.state & Vst::ProcessContext::kSystemTimeValid) != 0
                            ? makeOptional ((zu64) processContext.systemTime)
                            : nullopt);

        return info;
    }

    //==============================================================================
    i32 getNumAudioBuses (b8 isInput) const
    {
        i32 busCount = pluginInstance->getBusCount (isInput);

      #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = {DrxPlugin_PreferredChannelConfigurations};
        i32k numConfigs = numElementsInArray (configs);

        b8 hasOnlyZeroChannels = true;

        for (i32 i = 0; i < numConfigs && hasOnlyZeroChannels == true; ++i)
            if (configs[i][isInput ? 0 : 1] != 0)
                hasOnlyZeroChannels = false;

        busCount = jmin (busCount, hasOnlyZeroChannels ? 0 : 1);
       #endif

        return busCount;
    }

    //==============================================================================
    Steinberg::i32 PLUGIN_API getBusCount (Vst::MediaType type, Vst::BusDirection dir) override
    {
        if (type == Vst::kAudio)
            return getNumAudioBuses (dir == Vst::kInput);

        if (type == Vst::kEvent)
        {
           #if DrxPlugin_WantsMidiInput
            if (dir == Vst::kInput)
                return 1;
           #endif

           #if DrxPlugin_ProducesMidiOutput
            if (dir == Vst::kOutput)
                return 1;
           #endif
        }

        return 0;
    }

    tresult PLUGIN_API getBusInfo (Vst::MediaType type, Vst::BusDirection dir,
                                   Steinberg::i32 index, Vst::BusInfo& info) override
    {
        if (type == Vst::kAudio)
        {
            if (index < 0 || index >= getNumAudioBuses (dir == Vst::kInput))
                return kResultFalse;

            if (auto* bus = pluginInstance->getBus (dir == Vst::kInput, index))
            {
                info.mediaType = Vst::kAudio;
                info.direction = dir;
                info.channelCount = bus->getLastEnabledLayout().size();

                [[maybe_unused]] const auto lastEnabledVst3Layout = getVst3SpeakerArrangement (bus->getLastEnabledLayout());
                jassert (lastEnabledVst3Layout.has_value() && info.channelCount == Vst::SpeakerArr::getChannelCount (*lastEnabledVst3Layout));
                toString128 (info.name, bus->getName());

                info.busType = [&]
                {
                    const auto isFirstBus = (index == 0);

                    if (dir == Vst::kInput)
                    {
                        if (isFirstBus)
                        {
                            if (auto* extensions = pluginInstance->getVST3ClientExtensions())
                                return extensions->getPluginHasMainInput() ? Vst::kMain : Vst::kAux;

                            return Vst::kMain;
                        }

                        return Vst::kAux;
                    }

                   #if DrxPlugin_IsSynth
                    return Vst::kMain;
                   #else
                    return isFirstBus ? Vst::kMain : Vst::kAux;
                   #endif
                }();

               #ifdef DrxPlugin_PreferredChannelConfigurations
                info.flags = Vst::BusInfo::kDefaultActive;
               #else
                info.flags = (bus->isEnabledByDefault()) ? Vst::BusInfo::kDefaultActive : 0;
               #endif

                return kResultTrue;
            }
        }

        if (type == Vst::kEvent)
        {
            info.flags = Vst::BusInfo::kDefaultActive;

           #if DrxPlugin_WantsMidiInput
            if (dir == Vst::kInput && index == 0)
            {
                info.mediaType = Vst::kEvent;
                info.direction = dir;

               #ifdef DrxPlugin_VSTNumMidiInputs
                info.channelCount = DrxPlugin_VSTNumMidiInputs;
               #else
                info.channelCount = 16;
               #endif

                toString128 (info.name, TRANS ("MIDI Input"));
                info.busType = Vst::kMain;
                return kResultTrue;
            }
           #endif

           #if DrxPlugin_ProducesMidiOutput
            if (dir == Vst::kOutput && index == 0)
            {
                info.mediaType = Vst::kEvent;
                info.direction = dir;

               #ifdef DrxPlugin_VSTNumMidiOutputs
                info.channelCount = DrxPlugin_VSTNumMidiOutputs;
               #else
                info.channelCount = 16;
               #endif

                toString128 (info.name, TRANS ("MIDI Output"));
                info.busType = Vst::kMain;
                return kResultTrue;
            }
           #endif
        }

        zerostruct (info);
        return kResultFalse;
    }

    tresult PLUGIN_API activateBus (Vst::MediaType type,
                                    Vst::BusDirection dir,
                                    Steinberg::i32 index,
                                    TBool state) override
    {
        const FLStudioDIYSpecificationEnforcementLock lock (flStudioDIYSpecificationEnforcementMutex);

        // The host is misbehaving! The plugin must be deactivated before setting new arrangements.
        jassert (! active);

        if (type == Vst::kEvent)
        {
           #if DrxPlugin_WantsMidiInput
            if (index == 0 && dir == Vst::kInput)
            {
                isMidiInputBusEnabled = (state != 0);
                return kResultTrue;
            }
           #endif

           #if DrxPlugin_ProducesMidiOutput
            if (index == 0 && dir == Vst::kOutput)
            {
                isMidiOutputBusEnabled = (state != 0);
                return kResultTrue;
            }
           #endif

            return kResultFalse;
        }

        if (type == Vst::kAudio)
        {
            const auto numPublicInputBuses  = getNumAudioBuses (true);
            const auto numPublicOutputBuses = getNumAudioBuses (false);

            if (! isPositiveAndBelow (index, dir == Vst::kInput ? numPublicInputBuses : numPublicOutputBuses))
                return kResultFalse;

            // The host is allowed to enable/disable buses as it sees fit, so the plugin needs to be
            // able to handle any set of enabled/disabled buses, including layouts for which
            // AudioProcessor::isBusesLayoutSupported would return false.
            // Our strategy is to keep track of the layout that the host last requested, and to
            // attempt to apply that layout directly.
            // If the layout isn't supported by the processor, we'll try enabling all the buses
            // instead.
            // If the host enables a bus that the processor refused to enable, then we'll ignore
            // that bus (and return silence for output buses). If the host disables a bus that the
            // processor refuses to disable, the wrapper will provide the processor with silence for
            // input buses, and ignore the contents of output buses.
            // Note that some hosts (old bitwig and cakewalk) may incorrectly call this function
            // when the plugin is in an activated state.
            if (dir == Vst::kInput)
                bufferMapper.setInputBusHostActive ((size_t) index, state != 0);
            else
                bufferMapper.setOutputBusHostActive ((size_t) index, state != 0);

            AudioProcessor::BusesLayout desiredLayout;

            for (const auto isInput : { true, false })
            {
                const auto numPublicBuses = isInput ? numPublicInputBuses : numPublicOutputBuses;
                auto& layoutBuses = isInput ? desiredLayout.inputBuses : desiredLayout.outputBuses;

                for (auto i = 0; i < numPublicBuses; ++i)
                {
                    layoutBuses.add (isInput ? bufferMapper.getRequestedLayoutForInputBus ((size_t) i)
                                             : bufferMapper.getRequestedLayoutForOutputBus ((size_t) i));
                }

                while (layoutBuses.size() < pluginInstance->getBusCount (isInput))
                    layoutBuses.add (AudioChannelSet::disabled());
            }

            const auto prev = pluginInstance->getBusesLayout();

            const auto busesLayoutSupported = [&]
            {
               #ifdef DrxPlugin_PreferredChannelConfigurations
                struct ChannelPair
                {
                    short ins, outs;

                    auto tie() const { return std::tie (ins, outs); }
                    b8 operator== (ChannelPair x) const { return tie() == x.tie(); }
                };

                const auto countChannels = [] (auto& range)
                {
                    return std::accumulate (range.begin(), range.end(), 0, [] (auto acc, auto set)
                    {
                        return acc + set.size();
                    });
                };

                const auto toShort = [] (i32 x)
                {
                    jassert (0 <= x && x <= std::numeric_limits<short>::max());
                    return (short) x;
                };

                const ChannelPair requested { toShort (countChannels (desiredLayout.inputBuses)),
                                              toShort (countChannels (desiredLayout.outputBuses)) };
                const ChannelPair configs[] = { DrxPlugin_PreferredChannelConfigurations };
                return std::find (std::begin (configs), std::end (configs), requested) != std::end (configs);
               #else
                return pluginInstance->checkBusesLayoutSupported (desiredLayout);
               #endif
            }();

            if (busesLayoutSupported)
                pluginInstance->setBusesLayout (desiredLayout);
            else
                pluginInstance->enableAllBuses();

            bufferMapper.updateActiveClientBuses (pluginInstance->getBusesLayout());

            return kResultTrue;
        }

        return kResultFalse;
    }

    b8 checkBusFormatsAreNotDiscrete()
    {
        auto numInputBuses  = pluginInstance->getBusCount (true);
        auto numOutputBuses = pluginInstance->getBusCount (false);

        for (i32 i = 0; i < numInputBuses; ++i)
        {
            auto layout = pluginInstance->getChannelLayoutOfBus (true,  i);

            if (layout.isDiscreteLayout() && ! layout.isDisabled())
                return false;
        }

        for (i32 i = 0; i < numOutputBuses; ++i)
        {
            auto layout = pluginInstance->getChannelLayoutOfBus (false,  i);

            if (layout.isDiscreteLayout() && ! layout.isDisabled())
                return false;
        }

        return true;
    }

    tresult PLUGIN_API setBusArrangements (Vst::SpeakerArrangement* inputs, Steinberg::i32 numIns,
                                           Vst::SpeakerArrangement* outputs, Steinberg::i32 numOuts) override
    {
        const FLStudioDIYSpecificationEnforcementLock lock (flStudioDIYSpecificationEnforcementMutex);

        if (active)
        {
            // The host is misbehaving! The plugin must be deactivated before setting new arrangements.
            jassertfalse;
            return kResultFalse;
        }

        auto numInputBuses  = pluginInstance->getBusCount (true);
        auto numOutputBuses = pluginInstance->getBusCount (false);

        if (numIns > numInputBuses || numOuts > numOutputBuses)
            return kResultFalse;

        // see the following documentation to understand the correct way to react to this callback
        // https://steinbergmedia.github.io/vst3_doc/vstinterfaces/classSteinberg_1_1Vst_1_1IAudioProcessor.html#ad3bc7bac3fd3b194122669be2a1ecc42

        const auto toLayoutsArray = [] (auto begin, auto end) -> std::optional<Array<AudioChannelSet>>
        {
            Array<AudioChannelSet> result;

            for (auto it = begin; it != end; ++it)
            {
                const auto set = getChannelSetForSpeakerArrangement (*it);

                if (! set.has_value())
                    return {};

                result.add (*set);
            }

            return result;
        };

        const auto optionalRequestedLayout = [&]() -> std::optional<AudioProcessor::BusesLayout>
        {
            const auto ins  = toLayoutsArray (inputs,  inputs  + numIns);
            const auto outs = toLayoutsArray (outputs, outputs + numOuts);

            if (! ins.has_value() || ! outs.has_value())
                return {};

            AudioProcessor::BusesLayout result;
            result.inputBuses  = *ins;
            result.outputBuses = *outs;
            return result;
        }();

        if (! optionalRequestedLayout.has_value())
            return kResultFalse;

        const auto& requestedLayout = *optionalRequestedLayout;

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = { DrxPlugin_PreferredChannelConfigurations };
        if (! AudioProcessor::containsLayout (requestedLayout, configs))
            return kResultFalse;
       #endif

        if (pluginInstance->checkBusesLayoutSupported (requestedLayout))
        {
            if (! pluginInstance->setBusesLayoutWithoutEnabling (requestedLayout))
                return kResultFalse;

            bufferMapper.updateFromProcessor (*pluginInstance);
            return kResultTrue;
        }

        // apply layout changes in reverse order as Steinberg says we should prioritize main buses
        const auto nextBest = [this, numInputBuses, numOutputBuses, &requestedLayout]
        {
            auto layout = pluginInstance->getBusesLayout();

            for (auto busIdx = jmax (numInputBuses, numOutputBuses) - 1; busIdx >= 0; --busIdx)
                for (const auto isInput : { true, false })
                    if (auto* bus = pluginInstance->getBus (isInput, busIdx))
                        bus->isLayoutSupported (requestedLayout.getChannelSet (isInput, busIdx), &layout);

            return layout;
        }();

        if (pluginInstance->setBusesLayoutWithoutEnabling (nextBest))
            bufferMapper.updateFromProcessor (*pluginInstance);

        return kResultFalse;
    }

    tresult PLUGIN_API getBusArrangement (Vst::BusDirection dir, Steinberg::i32 index, Vst::SpeakerArrangement& arr) override
    {
        if (auto* bus = pluginInstance->getBus (dir == Vst::kInput, index))
        {
            if (const auto arrangement = getVst3SpeakerArrangement (bus->getLastEnabledLayout()))
            {
                arr = *arrangement;
                return kResultTrue;
            }

            // There's a bus here, but we can't represent its layout in terms of VST3 speakers!
            jassertfalse;
        }

        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API canProcessSampleSize (Steinberg::i32 symbolicSampleSize) override
    {
        return (symbolicSampleSize == Vst::kSample32
                 || (getPluginInstance().supportsDoublePrecisionProcessing()
                       && symbolicSampleSize == Vst::kSample64)) ? kResultTrue : kResultFalse;
    }

    Steinberg::u32 PLUGIN_API getLatencySamples() override
    {
        return (Steinberg::u32) jmax (0, getPluginInstance().getLatencySamples());
    }

    tresult PLUGIN_API setupProcessing (Vst::ProcessSetup& newSetup) override
    {
        ScopedInSetupProcessingSetter inSetupProcessingSetter (juceVST3EditController.get());

        if (canProcessSampleSize (newSetup.symbolicSampleSize) != kResultTrue)
            return kResultFalse;

        processSetup = newSetup;
        processContext.sampleRate = processSetup.sampleRate;

        getPluginInstance().setProcessingPrecision (newSetup.symbolicSampleSize == Vst::kSample64
                                                        ? AudioProcessor::doublePrecision
                                                        : AudioProcessor::singlePrecision);
        getPluginInstance().setNonRealtime (newSetup.processMode == Vst::kOffline);

        preparePlugin (processSetup.sampleRate, processSetup.maxSamplesPerBlock, CallPrepareToPlay::no);

        return kResultTrue;
    }

    tresult PLUGIN_API setProcessing (TBool state) override
    {
        if (! state)
            getPluginInstance().reset();

        return kResultTrue;
    }

    Steinberg::u32 PLUGIN_API getTailSamples() override
    {
        auto tailLengthSeconds = getPluginInstance().getTailLengthSeconds();

        if (tailLengthSeconds <= 0.0 || processSetup.sampleRate <= 0.0)
            return Vst::kNoTail;

        if (std::isinf (tailLengthSeconds))
            return Vst::kInfiniteTail;

        return (Steinberg::u32) roundToIntAccurate (tailLengthSeconds * processSetup.sampleRate);
    }

    //==============================================================================
    z0 processParameterChanges (Vst::IParameterChanges& paramChanges)
    {
        jassert (pluginInstance != nullptr);

        struct ParamChangeInfo
        {
            Steinberg::i32 offsetSamples = 0;
            f64 value = 0.0;
        };

        const auto getPointFromQueue = [] (Vst::IParamValueQueue* queue, Steinberg::i32 index)
        {
            ParamChangeInfo result;
            return queue->getPoint (index, result.offsetSamples, result.value) == kResultTrue
                   ? makeOptional (result)
                   : nullopt;
        };

        const auto numParamsChanged = paramChanges.getParameterCount();

        for (Steinberg::i32 i = 0; i < numParamsChanged; ++i)
        {
            if (auto* paramQueue = paramChanges.getParameterData (i))
            {
                const auto vstParamID = paramQueue->getParameterId();
                const auto numPoints  = paramQueue->getPointCount();

               #if DRX_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS
                if (juceVST3EditController != nullptr && juceVST3EditController->isMidiControllerParamID (vstParamID))
                {
                    for (Steinberg::i32 point = 0; point < numPoints; ++point)
                    {
                        if (const auto change = getPointFromQueue (paramQueue, point))
                            addParameterChangeToMidiBuffer (change->offsetSamples, vstParamID, change->value);
                    }
                }
                else
               #endif
                if (const auto change = getPointFromQueue (paramQueue, numPoints - 1))
                {
                    if (auto* param = comPluginInstance->getParamForVSTParamID (vstParamID))
                        setValueAndNotifyIfChanged (*param, (f32) change->value);
                }
            }
        }
    }

    z0 addParameterChangeToMidiBuffer (const Steinberg::i32 offsetSamples, const Vst::ParamID id, const f64 value)
    {
        // If the parameter is mapped to a MIDI CC message then insert it into the midiBuffer.
        i32 channel, ctrlNumber;

        if (juceVST3EditController->getMidiControllerForParameter (id, channel, ctrlNumber))
        {
            if (ctrlNumber == Vst::kAfterTouch)
                midiBuffer.addEvent (MidiMessage::channelPressureChange (channel,
                                                                         jlimit (0, 127, (i32) (value * 128.0))), offsetSamples);
            else if (ctrlNumber == Vst::kPitchBend)
                midiBuffer.addEvent (MidiMessage::pitchWheel (channel,
                                                              jlimit (0, 0x3fff, (i32) (value * 0x4000))), offsetSamples);
            else
                midiBuffer.addEvent (MidiMessage::controllerEvent (channel,
                                                                   jlimit (0, 127, ctrlNumber),
                                                                   jlimit (0, 127, (i32) (value * 128.0))), offsetSamples);
        }
    }

    tresult PLUGIN_API process (Vst::ProcessData& data) override
    {
        const FLStudioDIYSpecificationEnforcementLock lock (flStudioDIYSpecificationEnforcementMutex);

        if (pluginInstance == nullptr)
            return kResultFalse;

        if ((processSetup.symbolicSampleSize == Vst::kSample64) != pluginInstance->isUsingDoublePrecision())
            return kResultFalse;

        if (data.processContext != nullptr)
        {
            processContext = *data.processContext;

            if (juceVST3EditController != nullptr)
                juceVST3EditController->vst3IsPlaying = (processContext.state & Vst::ProcessContext::kPlaying) != 0;
        }
        else
        {
            zerostruct (processContext);

            if (juceVST3EditController != nullptr)
                juceVST3EditController->vst3IsPlaying = false;
        }

        midiBuffer.clear();

        if (data.inputParameterChanges != nullptr)
            processParameterChanges (*data.inputParameterChanges);

       #if DrxPlugin_WantsMidiInput
        if (isMidiInputBusEnabled && data.inputEvents != nullptr)
            MidiEventList::toMidiBuffer (midiBuffer, *data.inputEvents);
       #endif

        if (detail::PluginUtilities::getHostType().isWavelab())
        {
            i32k numInputChans  = (data.inputs  != nullptr && data.inputs[0].channelBuffers32 != nullptr)  ? (i32) data.inputs[0].numChannels  : 0;
            i32k numOutputChans = (data.outputs != nullptr && data.outputs[0].channelBuffers32 != nullptr) ? (i32) data.outputs[0].numChannels : 0;

            if ((pluginInstance->getTotalNumInputChannels() + pluginInstance->getTotalNumOutputChannels()) > 0
                 && (numInputChans + numOutputChans) == 0)
                return kResultFalse;
        }

        // If all of these are zero, the host is attempting to flush parameters without processing audio.
        if (data.numSamples != 0 || data.numInputs != 0 || data.numOutputs != 0)
        {
            if      (processSetup.symbolicSampleSize == Vst::kSample32) processAudio<f32>  (data);
            else if (processSetup.symbolicSampleSize == Vst::kSample64) processAudio<f64> (data);
            else jassertfalse;
        }

        if (auto* changes = data.outputParameterChanges)
        {
            comPluginInstance->forAllChangedParameters ([&] (Vst::ParamID paramID, f32 value)
                                                        {
                                                            Steinberg::i32 queueIndex = 0;

                                                            if (auto* queue = changes->addParameterData (paramID, queueIndex))
                                                            {
                                                                Steinberg::i32 pointIndex = 0;
                                                                queue->addPoint (0, value, pointIndex);
                                                            }
                                                        });
        }

       #if DrxPlugin_ProducesMidiOutput
        if (isMidiOutputBusEnabled && data.outputEvents != nullptr)
            MidiEventList::pluginToHostEventList (*data.outputEvents, midiBuffer);
       #endif

        return kResultTrue;
    }

private:
    /*  FL's Patcher implements the VST3 specification incorrectly, calls process() before/during
        setActive().
    */
    class [[nodiscard]] FLStudioDIYSpecificationEnforcementLock
    {
    public:
        explicit FLStudioDIYSpecificationEnforcementLock (CriticalSection& mutex)
        {
            static const auto lockRequired = PluginHostType().isFruityLoops();

            if (lockRequired)
                lock.emplace (mutex);
        }

    private:
        std::optional<ScopedLock> lock;
    };

    InterfaceResultWithDeferredAddRef queryInterfaceInternal (const TUID targetIID)
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<IPluginBase>{},
                                             UniqueBase<DrxVST3Component>{},
                                             UniqueBase<Vst::IComponent>{},
                                             UniqueBase<Vst::IAudioProcessor>{},
                                             UniqueBase<Vst::IUnitInfo>{},
                                             UniqueBase<Vst::IConnectionPoint>{},
                                             UniqueBase<Vst::IProcessContextRequirements>{},
                                            #if DrxPlugin_Enable_ARA
                                             UniqueBase<ARA::IPlugInEntryPoint>{},
                                             UniqueBase<ARA::IPlugInEntryPoint2>{},
                                            #endif
                                             SharedBase<FUnknown, Vst::IComponent>{});

        if (result.isOk())
            return result;

        if (doUIDsMatch (targetIID, DrxAudioProcessor::iid))
            return { kResultOk, comPluginInstance.get() };

        return {};
    }

    //==============================================================================
    struct ScopedInSetupProcessingSetter
    {
        ScopedInSetupProcessingSetter (DrxVST3EditController* c)
            : controller (c)
        {
            if (controller != nullptr)
                controller->inSetupProcessing = true;
        }

        ~ScopedInSetupProcessingSetter()
        {
            if (controller != nullptr)
                controller->inSetupProcessing = false;
        }

    private:
        DrxVST3EditController* controller = nullptr;
    };

    //==============================================================================
    template <typename FloatType>
    z0 processAudio (Vst::ProcessData& data)
    {
        ClientRemappedBuffer<FloatType> remappedBuffer { bufferMapper, data };
        auto& buffer = remappedBuffer.buffer;

        jassert ((i32) buffer.getNumChannels() == jmax (pluginInstance->getTotalNumInputChannels(),
                                                        pluginInstance->getTotalNumOutputChannels()));

        {
            const ScopedLock sl (pluginInstance->getCallbackLock());

            pluginInstance->setNonRealtime (data.processMode == Vst::kOffline);

           #if DRX_DEBUG && ! DrxPlugin_ProducesMidiOutput
            i32k numMidiEventsComingIn = midiBuffer.getNumEvents();
           #endif

            if (pluginInstance->isSuspended())
            {
                buffer.clear();
            }
            else
            {
                // processBlockBypassed should only ever be called if the AudioProcessor doesn't
                // return a valid parameter from getBypassParameter
                if (pluginInstance->getBypassParameter() == nullptr && comPluginInstance->getBypassParameter()->getValue() >= 0.5f)
                    pluginInstance->processBlockBypassed (buffer, midiBuffer);
                else
                    pluginInstance->processBlock (buffer, midiBuffer);
            }

           #if DRX_DEBUG && (! DrxPlugin_ProducesMidiOutput)
            /*  This assertion is caused when you've added some events to the
                midiMessages array in your processBlock() method, which usually means
                that you're trying to send them somewhere. But in this case they're
                getting thrown away.

                If your plugin does want to send MIDI messages, you'll need to set
                the DrxPlugin_ProducesMidiOutput macro to 1 in your
                DrxPluginCharacteristics.h file.

                If you don't want to produce any MIDI output, then you should clear the
                midiMessages array at the end of your processBlock() method, to
                indicate that you don't want any of the events to be passed through
                to the output.
            */
            jassert (midiBuffer.getNumEvents() <= numMidiEventsComingIn);
           #endif
        }
    }

    //==============================================================================
    Steinberg::u32 PLUGIN_API getProcessContextRequirements() override
    {
        return kNeedSystemTime
             | kNeedContinousTimeSamples
             | kNeedProjectTimeMusic
             | kNeedBarPositionMusic
             | kNeedCycleMusic
             | kNeedSamplesToNextClock
             | kNeedTempo
             | kNeedTimeSignature
             | kNeedChord
             | kNeedFrameRate
             | kNeedTransportState;
    }

    z0 preparePlugin (f64 sampleRate, i32 bufferSize, CallPrepareToPlay callPrepareToPlay)
    {
        auto& p = getPluginInstance();

        p.setRateAndBufferSizeDetails (sampleRate, bufferSize);

        if (callPrepareToPlay == CallPrepareToPlay::yes)
            p.prepareToPlay (sampleRate, bufferSize);

        midiBuffer.ensureSize (2048);
        midiBuffer.clear();

        bufferMapper.updateFromProcessor (p);
        bufferMapper.prepare (bufferSize);
    }

    //==============================================================================
   #if DrxPlugin_Enable_ARA
    const ARA::ARAFactory* PLUGIN_API getFactory() SMTG_OVERRIDE
    {
        return createARAFactory();
    }

    const ARA::ARAPlugInExtensionInstance* PLUGIN_API bindToDocumentController (ARA::ARADocumentControllerRef /*controllerRef*/) SMTG_OVERRIDE
    {
        ARA_VALIDATE_API_STATE (false && "call is deprecated in ARA 2, host must not call this");
        return nullptr;
    }

    const ARA::ARAPlugInExtensionInstance* PLUGIN_API bindToDocumentControllerWithRoles (ARA::ARADocumentControllerRef documentControllerRef,
                                                                                         ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles) SMTG_OVERRIDE
    {
        AudioProcessorARAExtension* araAudioProcessorExtension = dynamic_cast<AudioProcessorARAExtension*> (pluginInstance);
        return araAudioProcessorExtension->bindToARA (documentControllerRef, knownRoles, assignedRoles);
    }
   #endif

    //==============================================================================
    ScopedRunLoop scopedRunLoop;
    std::atomic<i32> refCount { 1 };
    AudioProcessor* pluginInstance = nullptr;

   #if DRX_LINUX || DRX_BSD
    template <class T>
    struct LockedVSTComSmartPtr
    {
        LockedVSTComSmartPtr() = default;
        LockedVSTComSmartPtr (const VSTComSmartPtr<T>& ptrIn) : ptr (ptrIn)  {}
        LockedVSTComSmartPtr (const LockedVSTComSmartPtr&) = default;
        LockedVSTComSmartPtr& operator= (const LockedVSTComSmartPtr&) = default;

        ~LockedVSTComSmartPtr()
        {
            const MessageManagerLock mmLock;
            ptr = {};
        }

        T* operator->() const         { return ptr.operator->(); }
        T* get() const noexcept       { return ptr.get(); }
        operator T*() const noexcept  { return ptr.get(); }

        template <typename... Args>
        b8 loadFrom (Args&&... args)  { return ptr.loadFrom (std::forward<Args> (args)...); }

    private:
        VSTComSmartPtr<T> ptr;
    };

    LockedVSTComSmartPtr<Vst::IHostApplication> host;
    LockedVSTComSmartPtr<DrxAudioProcessor> comPluginInstance;
    LockedVSTComSmartPtr<DrxVST3EditController> juceVST3EditController;
   #else
    VSTComSmartPtr<Vst::IHostApplication> host;
    VSTComSmartPtr<DrxAudioProcessor> comPluginInstance;
    VSTComSmartPtr<DrxVST3EditController> juceVST3EditController;
   #endif

    /**
        Since VST3 does not provide a way of knowing the buffer size and sample rate at any point,
        this object needs to be copied on every call to process() to be up-to-date...
    */
    Vst::ProcessContext processContext;
    Vst::ProcessSetup processSetup;

    MidiBuffer midiBuffer;
    ClientBufferMapper bufferMapper;

    b8 active = false;

   #if DrxPlugin_WantsMidiInput
    std::atomic<b8> isMidiInputBusEnabled { true };
   #endif
   #if DrxPlugin_ProducesMidiOutput
    std::atomic<b8> isMidiOutputBusEnabled { true };
   #endif

    inline static constexpr tukk kDrxPrivateDataIdentifier = "DRXPrivateData";
    CriticalSection flStudioDIYSpecificationEnforcementMutex;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrxVST3Component)
};

//==============================================================================
b8 initModule();
b8 initModule()
{
    return true;
}

b8 shutdownModule();
b8 shutdownModule()
{
    return true;
}

#undef DRX_EXPORTED_FUNCTION

#if DRX_WINDOWS
 #define DRX_EXPORTED_FUNCTION
#else
 #define DRX_EXPORTED_FUNCTION extern "C" __attribute__ ((visibility ("default")))
#endif

#if DRX_WINDOWS
 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

 extern "C" __declspec (dllexport) b8 InitDll()   { return initModule(); }
 extern "C" __declspec (dllexport) b8 ExitDll()   { return shutdownModule(); }

 DRX_END_IGNORE_WARNINGS_GCC_LIKE
#elif DRX_LINUX || DRX_BSD
 uk moduleHandle = nullptr;
 i32 moduleEntryCounter = 0;

 DRX_EXPORTED_FUNCTION b8 ModuleEntry (uk sharedLibraryHandle);
 DRX_EXPORTED_FUNCTION b8 ModuleEntry (uk sharedLibraryHandle)
 {
     if (++moduleEntryCounter == 1)
     {
         moduleHandle = sharedLibraryHandle;
         return initModule();
     }

     return true;
 }

 DRX_EXPORTED_FUNCTION b8 ModuleExit();
 DRX_EXPORTED_FUNCTION b8 ModuleExit()
 {
     if (--moduleEntryCounter == 0)
     {
         moduleHandle = nullptr;
         return shutdownModule();
     }

     return true;
 }
#elif DRX_MAC
 CFBundleRef globalBundleInstance = nullptr;
 drx::u32 numBundleRefs = 0;
 drx::Array<CFBundleRef> bundleRefs;

 enum { MaxPathLength = 2048 };
 t8 modulePath[MaxPathLength] = { 0 };
 uk moduleHandle = nullptr;

 DRX_EXPORTED_FUNCTION b8 bundleEntry (CFBundleRef ref);
 DRX_EXPORTED_FUNCTION b8 bundleEntry (CFBundleRef ref)
 {
     if (ref != nullptr)
     {
         ++numBundleRefs;
         CFRetain (ref);

         bundleRefs.add (ref);

         if (moduleHandle == nullptr)
         {
             globalBundleInstance = ref;
             moduleHandle = ref;

             CFUniquePtr<CFURLRef> tempURL (CFBundleCopyBundleURL (ref));
             CFURLGetFileSystemRepresentation (tempURL.get(), true, (UInt8*) modulePath, MaxPathLength);
         }
     }

     return initModule();
 }

 DRX_EXPORTED_FUNCTION b8 bundleExit();
 DRX_EXPORTED_FUNCTION b8 bundleExit()
 {
     if (shutdownModule())
     {
         if (--numBundleRefs == 0)
         {
             for (i32 i = 0; i < bundleRefs.size(); ++i)
                 CFRelease (bundleRefs.getUnchecked (i));

             bundleRefs.clear();
         }

         return true;
     }

     return false;
 }
#endif

// See https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Compatibility+with+VST+2.x+or+VST+1.html
class DrxPluginCompatibility final : public IPluginCompatibility
{
public:
    virtual ~DrxPluginCompatibility() = default;

    DRX_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API getCompatibilityJSON (IBStream* stream) override
    {
        const ScopedDrxInitialiser_GUI libraryInitialiser;

        auto filter = createPluginFilterOfType (AudioProcessor::WrapperType::wrapperType_VST3);
        auto* extensions = filter->getVST3ClientExtensions();

        const auto compatibilityObjects = [&]
        {
            if (extensions == nullptr || extensions->getCompatibleClasses().empty())
                return Array<var>();

            DynamicObject::Ptr object { new DynamicObject };

            // New iid is the ID of our Audio Effect class
            object->setProperty ("New", Txt (VST3::UID (DrxVST3Component::iid).toString()));
            object->setProperty ("Old", [&]
            {
                Array<var> oldArray;

                for (const auto& uid : extensions->getCompatibleClasses())
                    oldArray.add (Txt::toHexString (uid.data(), (i32) uid.size(), 0));

                return oldArray;
            }());

            return Array<var> { object.get() };
        }();

        MemoryOutputStream memory;
        JSON::writeToStream (memory, var { compatibilityObjects });
        return stream->write (memory.getMemoryBlock().getData(), (Steinberg::i32) memory.getDataSize());
    }

    tresult PLUGIN_API queryInterface (const TUID targetIID, uk* obj) override
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<IPluginCompatibility>{},
                                             UniqueBase<FUnknown>{});

        if (result.isOk())
            return result.extract (obj);

        jassertfalse; // Something new?
        *obj = nullptr;
        return kNotImplemented;
    }

    inline static const FUID iid = toSteinbergUID (getInterfaceId (VST3InterfaceType::compatibility));

private:
    std::atomic<i32> refCount { 1 };
};

//==============================================================================
using CreateFunction = FUnknown* (*) (const VSTComSmartPtr<Vst::IHostApplication>&,
                                      const RunLoop&);

//==============================================================================
struct DrxPluginFactory final : public IPluginFactory3
{
    DrxPluginFactory()
        : factoryInfo (DrxPlugin_Manufacturer, DrxPlugin_ManufacturerWebsite,
                       DrxPlugin_ManufacturerEmail, Vst::kDefaultFactoryFlags) {}

    virtual ~DrxPluginFactory() = default;

    //==============================================================================
    DRX_DECLARE_VST3_COM_REF_METHODS

    tresult PLUGIN_API queryInterface (const TUID targetIID, uk* obj) override
    {
        const auto result = testForMultiple (*this,
                                             targetIID,
                                             UniqueBase<IPluginFactory3>{},
                                             UniqueBase<IPluginFactory2>{},
                                             UniqueBase<IPluginFactory>{},
                                             UniqueBase<FUnknown>{});

        if (result.isOk())
            return result.extract (obj);

        jassertfalse; // Something new?
        *obj = nullptr;
        return kNotImplemented;
    }

    //==============================================================================
    Steinberg::i32 PLUGIN_API countClasses() override
    {
        return (Steinberg::i32) getClassEntries().size();
    }

    tresult PLUGIN_API getFactoryInfo (PFactoryInfo* info) override
    {
        if (info == nullptr)
            return kInvalidArgument;

        memcpy (info, &factoryInfo, sizeof (PFactoryInfo));
        return kResultOk;
    }

    tresult PLUGIN_API getClassInfo (Steinberg::i32 index, PClassInfo* info) override
    {
        return getPClassInfo<PClassInfo> (index, info);
    }

    tresult PLUGIN_API getClassInfo2 (Steinberg::i32 index, PClassInfo2* info) override
    {
        return getPClassInfo<PClassInfo2> (index, info);
    }

    tresult PLUGIN_API getClassInfoUnicode (Steinberg::i32 index, PClassInfoW* info) override
    {
        if (info != nullptr)
        {
            memcpy (info, &getClassEntries()[static_cast<size_t> (index)].infoW, sizeof (PClassInfoW));
            return kResultOk;
        }

        return kInvalidArgument;
    }

    tresult PLUGIN_API createInstance (FIDString cid, FIDString sourceIid, uk* obj) override
    {
        const ScopedRunLoop scope { runLoop };

        *obj = nullptr;

        TUID tuid;
        memcpy (tuid, sourceIid, sizeof (TUID));

       #if VST_VERSION >= 0x030608
        auto sourceFuid = FUID::fromTUID (tuid);
       #else
        FUID sourceFuid;
        sourceFuid = tuid;
       #endif

        if (cid == nullptr || sourceIid == nullptr || ! sourceFuid.isValid())
        {
            jassertfalse; // The host you're running in has severe implementation issues!
            return kInvalidArgument;
        }

        TUID iidToQuery;
        sourceFuid.toTUID (iidToQuery);

        for (auto& entry : getClassEntries())
        {
            if (doUIDsMatch (entry.infoW.cid, cid))
            {
                if (auto instance = becomeVSTComSmartPtrOwner (entry.createFunction (host, runLoop)))
                {
                    if (instance->queryInterface (iidToQuery, obj) == kResultOk)
                        return kResultOk;
                }

                break;
            }
        }

        return kNoInterface;
    }

    tresult PLUGIN_API setHostContext (FUnknown* context) override
    {
        runLoop.loadFrom (context);
        host.loadFrom (context);

        if (host != nullptr)
        {
            Vst::String128 name;
            host->getName (name);

            return kResultTrue;
        }

        return kNotImplemented;
    }

private:
    //==============================================================================
    std::atomic<i32> refCount { 1 };
    const PFactoryInfo factoryInfo;
    VSTComSmartPtr<Vst::IHostApplication> host;
    RunLoop runLoop;

    //==============================================================================
    struct ClassEntry
    {
        ClassEntry (const PClassInfo2& info, CreateFunction fn) noexcept
            : info2 (info), createFunction (fn)
        {
            infoW.fromAscii (info);
        }

        PClassInfo2 info2;
        PClassInfoW infoW;
        CreateFunction createFunction = {};

    private:
        DRX_DECLARE_NON_COPYABLE (ClassEntry)
    };

    static Span<const ClassEntry> getClassEntries()
    {
      #ifndef DrxPlugin_Vst3ComponentFlags
       #if DrxPlugin_IsSynth
        #define DrxPlugin_Vst3ComponentFlags Vst::kSimpleModeSupported
       #else
        #define DrxPlugin_Vst3ComponentFlags 0
       #endif
      #endif

      #ifndef DrxPlugin_Vst3Category
       #if DrxPlugin_IsSynth
        #define DrxPlugin_Vst3Category Vst::PlugType::kInstrumentSynth
       #else
        #define DrxPlugin_Vst3Category Vst::PlugType::kFx
       #endif
      #endif

        static const PClassInfo2 compatibilityClass { DrxPluginCompatibility::iid,
                                                      PClassInfo::kManyInstances,
                                                      kPluginCompatibilityClass,
                                                      DrxPlugin_Name,
                                                      0,
                                                      "",
                                                      DrxPlugin_Manufacturer,
                                                      DrxPlugin_VersionString,
                                                      kVstVersionString };

        static const PClassInfo2 componentClass { DrxVST3Component::iid,
                                                  PClassInfo::kManyInstances,
                                                  kVstAudioEffectClass,
                                                  DrxPlugin_Name,
                                                  DrxPlugin_Vst3ComponentFlags,
                                                  DrxPlugin_Vst3Category,
                                                  DrxPlugin_Manufacturer,
                                                  DrxPlugin_VersionString,
                                                  kVstVersionString };

        static const PClassInfo2 controllerClass { DrxVST3EditController::iid,
                                                   PClassInfo::kManyInstances,
                                                   kVstComponentControllerClass,
                                                   DrxPlugin_Name,
                                                   DrxPlugin_Vst3ComponentFlags,
                                                   DrxPlugin_Vst3Category,
                                                   DrxPlugin_Manufacturer,
                                                   DrxPlugin_VersionString,
                                                   kVstVersionString };
       #if DrxPlugin_Enable_ARA
        static const PClassInfo2 araFactoryClass { DrxARAFactory::iid,
                                                   PClassInfo::kManyInstances,
                                                   kARAMainFactoryClass,
                                                   DrxPlugin_Name,
                                                   DrxPlugin_Vst3ComponentFlags,
                                                   DrxPlugin_Vst3Category,
                                                   DrxPlugin_Manufacturer,
                                                   DrxPlugin_VersionString,
                                                   kVstVersionString };
       #endif

        static const ClassEntry classEntries[]
        {
            ClassEntry { componentClass, [] (const VSTComSmartPtr<Vst::IHostApplication>& h,
                                             const RunLoop& l) -> FUnknown*
            {
                return static_cast<Vst::IAudioProcessor*> (new DrxVST3Component (h, l));
            } },
            ClassEntry { controllerClass, [] (const VSTComSmartPtr<Vst::IHostApplication>& h,
                                              const RunLoop& l) -> FUnknown*
            {
                return static_cast<Vst::IEditController*> (new DrxVST3EditController (h, l));
            } },
            ClassEntry { compatibilityClass, [] (const VSTComSmartPtr<Vst::IHostApplication>&,
                                                 const RunLoop&) -> FUnknown*
            {
                return new DrxPluginCompatibility;
            } },
           #if DrxPlugin_Enable_ARA
            ClassEntry { araFactoryClass, [] (const VSTComSmartPtr<Vst::IHostApplication>&,
                                              const RunLoop&) -> FUnknown*
            {
                return static_cast<ARA::IMainFactory*> (new DrxARAFactory);
            } },
           #endif
        };

        return Span { classEntries };
    }

    //==============================================================================
    template <class PClassInfoType>
    tresult PLUGIN_API getPClassInfo (Steinberg::i32 index, PClassInfoType* info)
    {
        if (info != nullptr)
        {
            zerostruct (*info);
            memcpy (info, (PClassInfoType*) &getClassEntries()[static_cast<size_t> (index)].info2, sizeof (PClassInfoType));
            return kResultOk;
        }

        jassertfalse;
        return kInvalidArgument;
    }

    //==============================================================================
    // no leak detector here to prevent it firing on shutdown when running in hosts that
    // don't release the factory object correctly...
    DRX_DECLARE_NON_COPYABLE (DrxPluginFactory)
};

} // namespace drx

//==============================================================================
using namespace drx;

//==============================================================================
// The VST3 plugin entry point.
extern "C" SMTG_EXPORT_SYMBOL IPluginFactory* PLUGIN_API GetPluginFactory()
{
   #if (DRX_MSVC || (DRX_WINDOWS && DRX_CLANG)) && DRX_32BIT
    // Cunning trick to force this function to be exported. Life's too short to
    // faff around creating .def files for this kind of thing.
    // Unnecessary for 64-bit builds because those don't use decorated function names.
    #pragma comment(linker, "/EXPORT:GetPluginFactory=_GetPluginFactory@0")
   #endif

    return new DrxPluginFactory();
}

//==============================================================================
#if DRX_WINDOWS
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID) { if (reason == DLL_PROCESS_ATTACH) Process::setCurrentModuleInstanceHandle (instance); return true; }

DRX_END_IGNORE_WARNINGS_GCC_LIKE
#endif

DRX_END_NO_SANITIZE

#endif //DrxPlugin_Build_VST3
