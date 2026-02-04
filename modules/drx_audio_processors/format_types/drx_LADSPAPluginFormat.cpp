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

#if DRX_PLUGINHOST_LADSPA && (DRX_LINUX || DRX_BSD)

#include <ladspa.h>

namespace drx
{

static i32 shellLADSPAUIDToCreate = 0;
static i32 insideLADSPACallback = 0;

#define DRX_LADSPA_LOGGING 1

#if DRX_LADSPA_LOGGING
 #define DRX_LADSPA_LOG(x) Logger::writeToLog (x);
#else
 #define DRX_LADSPA_LOG(x)
#endif

//==============================================================================
class LADSPAModuleHandle final : public ReferenceCountedObject
{
public:
    LADSPAModuleHandle (const File& f)
        : file (f)
    {
        getActiveModules().add (this);
    }

    ~LADSPAModuleHandle()
    {
        getActiveModules().removeFirstMatchingValue (this);
        close();
    }

    using Ptr = ReferenceCountedObjectPtr<LADSPAModuleHandle>;

    static Array<LADSPAModuleHandle*>& getActiveModules()
    {
        static Array<LADSPAModuleHandle*> activeModules;
        return activeModules;
    }

    static LADSPAModuleHandle* findOrCreateModule (const File& file)
    {
        for (auto i = getActiveModules().size(); --i >= 0;)
        {
            auto* module = getActiveModules().getUnchecked (i);

            if (module->file == file)
                return module;
        }

        ++insideLADSPACallback;
        shellLADSPAUIDToCreate = 0;

        DRX_LADSPA_LOG ("Loading LADSPA module: " + file.getFullPathName());

        std::unique_ptr<LADSPAModuleHandle> m (new LADSPAModuleHandle (file));

        if (! m->open())
            m = nullptr;

        --insideLADSPACallback;

        return m.release();
    }

    File file;
    LADSPA_Descriptor_Function moduleMain = nullptr;

private:
    DynamicLibrary module;

    b8 open()
    {
        module.open (file.getFullPathName());
        moduleMain = (LADSPA_Descriptor_Function) module.getFunction ("ladspa_descriptor");

        return (moduleMain != nullptr);
    }

    z0 close()
    {
        module.close();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAModuleHandle)
};

//==============================================================================
class LADSPAPluginInstance final    : public AudioPluginInstance
{
public:
    LADSPAPluginInstance (const LADSPAModuleHandle::Ptr& m)
        : module (m)
    {
        ++insideLADSPACallback;

        name = module->file.getFileNameWithoutExtension();

        DRX_LADSPA_LOG ("Creating LADSPA instance: " + name);

        if (module->moduleMain != nullptr)
        {
            plugin = module->moduleMain ((size_t) shellLADSPAUIDToCreate);

            if (plugin == nullptr)
            {
                DRX_LADSPA_LOG ("Cannot find any valid descriptor in shared library");
                --insideLADSPACallback;
                return;
            }
        }
        else
        {
            DRX_LADSPA_LOG ("Cannot find any valid plugin in shared library");
            --insideLADSPACallback;
            return;
        }

        const auto sampleRate = getSampleRate() > 0 ? getSampleRate()
                                                    : 44100.0;

        handle = plugin->instantiate (plugin, (u32) sampleRate);

        --insideLADSPACallback;
    }

    ~LADSPAPluginInstance() override
    {
        const ScopedLock sl (lock);

        jassert (insideLADSPACallback == 0);

        if (handle != nullptr && plugin != nullptr)
            NullCheckedInvocation::invoke (plugin->cleanup, handle);

        initialised = false;
        module = nullptr;
        plugin = nullptr;
        handle = nullptr;
    }

    z0 initialise (f64 initialSampleRate, i32 initialBlockSize)
    {
        setPlayConfigDetails (inputs.size(), outputs.size(), initialSampleRate, initialBlockSize);

        if (initialised || plugin == nullptr || handle == nullptr)
            return;

        DRX_LADSPA_LOG ("Initialising LADSPA: " + name);

        initialised = true;

        inputs.clear();
        outputs.clear();
        AudioProcessorParameterGroup newTree;

        for (u32 i = 0; i < plugin->PortCount; ++i)
        {
            const auto portDesc = plugin->PortDescriptors[i];

            if ((portDesc & LADSPA_PORT_CONTROL) != 0)
                newTree.addChild (std::make_unique<LADSPAParameter> (*this, (i32) i,
                                                                     Txt (plugin->PortNames[i]).trim(),
                                                                     (portDesc & LADSPA_PORT_INPUT) != 0));

            if ((portDesc & LADSPA_PORT_AUDIO) != 0)
            {
                if ((portDesc & LADSPA_PORT_INPUT) != 0)    inputs.add ((i32) i);
                if ((portDesc & LADSPA_PORT_OUTPUT) != 0)   outputs.add ((i32) i);
            }
        }

        setHostedParameterTree (std::move (newTree));

        for (auto* param : getParameters())
            if (auto* ladspaParam = dynamic_cast<LADSPAParameter*> (param))
                plugin->connect_port (handle, (size_t) ladspaParam->paramID, &(ladspaParam->paramValue.scaled));

        setPlayConfigDetails (inputs.size(), outputs.size(), initialSampleRate, initialBlockSize);

        setCurrentProgram (0);
        setLatencySamples (0);

        // Some plugins crash if this doesn't happen:
        NullCheckedInvocation::invoke (plugin->activate, handle);
        NullCheckedInvocation::invoke (plugin->deactivate, handle);
    }

    //==============================================================================
    // AudioPluginInstance methods:

    z0 fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = getName();
        desc.fileOrIdentifier = module->file.getFullPathName();
        desc.uniqueId = desc.deprecatedUid = getUID();
        desc.lastFileModTime = module->file.getLastModificationTime();
        desc.lastInfoUpdateTime = Time::getCurrentTime();
        desc.pluginFormatName = "LADSPA";
        desc.category = getCategory();
        desc.manufacturerName = plugin != nullptr ? Txt (plugin->Maker) : Txt();
        desc.version = getVersion();
        desc.numInputChannels  = getTotalNumInputChannels();
        desc.numOutputChannels = getTotalNumOutputChannels();
        desc.isInstrument = false;
    }

    const Txt getName() const override
    {
        if (plugin != nullptr && plugin->Label != nullptr)
            return plugin->Label;

        return name;
    }

    i32 getUID() const
    {
        if (plugin != nullptr && plugin->UniqueID != 0)
            return (i32) plugin->UniqueID;

        return module->file.hashCode();
    }

    Txt getVersion() const                         { return LADSPA_VERSION; }
    Txt getCategory() const                        { return "Effect"; }

    b8 acceptsMidi()  const override                { return false; }
    b8 producesMidi() const override                { return false; }

    f64 getTailLengthSeconds() const override      { return 0.0; }

    //==============================================================================
    z0 prepareToPlay (f64 newSampleRate, i32 samplesPerBlockExpected) override
    {
        setLatencySamples (0);

        initialise (newSampleRate, samplesPerBlockExpected);

        if (initialised)
        {
            tempBuffer.setSize (jmax (1, outputs.size()), samplesPerBlockExpected);

            // dodgy hack to force some plugins to initialise the sample rate..
            if (auto* firstParam = getParameters()[0])
            {
                const auto old = firstParam->getValue();
                firstParam->setValue ((old < 0.5f) ? 1.0f : 0.0f);
                firstParam->setValue (old);
            }

            NullCheckedInvocation::invoke (plugin->activate, handle);
        }
    }

    z0 releaseResources() override
    {
        if (handle != nullptr && plugin->deactivate != nullptr)
            NullCheckedInvocation::invoke (plugin->deactivate, handle);

        tempBuffer.setSize (1, 1);
    }

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer&) override
    {
        auto numSamples = buffer.getNumSamples();

        if (initialised && plugin != nullptr && handle != nullptr)
        {
            for (i32 i = 0; i < inputs.size(); ++i)
                plugin->connect_port (handle, (size_t) inputs[i],
                                      i < buffer.getNumChannels() ? buffer.getWritePointer (i) : nullptr);

            if (plugin->run != nullptr)
            {
                for (i32 i = 0; i < outputs.size(); ++i)
                    plugin->connect_port (handle, (size_t) outputs.getUnchecked (i),
                                          i < buffer.getNumChannels() ? buffer.getWritePointer (i) : nullptr);

                plugin->run (handle, (size_t) numSamples);
                return;
            }

            if (plugin->run_adding != nullptr)
            {
                tempBuffer.setSize (outputs.size(), numSamples);
                tempBuffer.clear();

                for (i32 i = 0; i < outputs.size(); ++i)
                    plugin->connect_port (handle, (size_t) outputs.getUnchecked (i), tempBuffer.getWritePointer (i));

                plugin->run_adding (handle, (size_t) numSamples);

                for (i32 i = 0; i < outputs.size(); ++i)
                    if (i < buffer.getNumChannels())
                        buffer.copyFrom (i, 0, tempBuffer, i, 0, numSamples);

                return;
            }

            jassertfalse; // no callback to use?
        }

        for (auto i = getTotalNumInputChannels(), e = getTotalNumOutputChannels(); i < e; ++i)
            buffer.clear (i, 0, numSamples);
    }

    using AudioPluginInstance::processBlock;

    b8 isInputChannelStereoPair  (i32 index) const override    { return isPositiveAndBelow (index, getTotalNumInputChannels()); }
    b8 isOutputChannelStereoPair (i32 index) const override    { return isPositiveAndBelow (index, getTotalNumOutputChannels()); }

    const Txt getInputChannelName (i32k index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return Txt (plugin->PortNames [inputs [index]]).trim();

        return {};
    }

    const Txt getOutputChannelName (i32k index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return Txt (plugin->PortNames [outputs [index]]).trim();

        return {};
    }

    //==============================================================================
    i32 getNumPrograms() override                            { return 0; }
    i32 getCurrentProgram() override                         { return 0; }

    z0 setCurrentProgram (i32) override
    {
        for (auto* param : getParameters())
            if (auto* ladspaParam = dynamic_cast<LADSPAParameter*> (param))
                ladspaParam->reset();
    }

    const Txt getProgramName (i32) override             { return {}; }
    z0 changeProgramName (i32, const Txt&) override   {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        auto numParameters = getParameters().size();
        destData.setSize ((size_t) numParameters * sizeof (f32));
        destData.fillWith (0);

        auto* p = unalignedPointerCast<f32*> (destData.getData());

        for (i32 i = 0; i < numParameters; ++i)
            if (auto* param = getParameters()[i])
                p[i] = param->getValue();
    }

    z0 getCurrentProgramStateInformation (MemoryBlock& destData) override               { getStateInformation (destData); }
    z0 setCurrentProgramStateInformation (ukk data, i32 sizeInBytes) override   { setStateInformation (data, sizeInBytes); }

    z0 setStateInformation (ukk data, [[maybe_unused]] i32 sizeInBytes) override
    {
        auto* p = static_cast<const f32*> (data);

        for (i32 i = 0; i < getParameters().size(); ++i)
            if (auto* param = getParameters()[i])
                param->setValue (p[i]);
    }

    b8 hasEditor() const override                 { return false; }
    AudioProcessorEditor* createEditor() override   { return nullptr; }

    b8 isValid() const                            { return handle != nullptr; }

    //==============================================================================
    LADSPAModuleHandle::Ptr module;
    const LADSPA_Descriptor* plugin = nullptr;

private:
    //==============================================================================
    struct LADSPAParameter final   : public Parameter
    {
        struct ParameterValue
        {
            inline ParameterValue() noexcept                                               {}
            inline ParameterValue (f32 s, f32 u) noexcept  : scaled (s), unscaled (u)  {}

            f32 scaled = 0, unscaled = 0;
        };

        LADSPAParameter (LADSPAPluginInstance& parent, i32 parameterID,
                         const Txt& parameterName, b8 parameterIsAutomatable)
            : pluginInstance (parent),
              paramID (parameterID),
              name (parameterName),
              automatable (parameterIsAutomatable)
        {
            reset();
        }

        f32 getValue() const override
        {
            if (pluginInstance.plugin != nullptr)
            {
                const ScopedLock sl (pluginInstance.lock);

                return paramValue.unscaled;
            }

            return 0.0f;
        }

        Txt getCurrentValueAsText() const override
        {
            if (auto* interface = pluginInstance.plugin)
            {
                const auto& hint = interface->PortRangeHints[paramID];

                if (LADSPA_IS_HINT_INTEGER (hint.HintDescriptor))
                    return Txt ((i32) paramValue.scaled);

                return Txt (paramValue.scaled, 4);
            }

            return {};
        }

        z0 setValue (f32 newValue) override
        {
            if (auto* interface = pluginInstance.plugin)
            {
                const ScopedLock sl (pluginInstance.lock);

                if (! approximatelyEqual (paramValue.unscaled, newValue))
                    paramValue = ParameterValue (getNewParamScaled (interface->PortRangeHints [paramID], newValue), newValue);
            }
        }

        f32 getDefaultValue() const override
        {
            return defaultValue;
        }

        ParameterValue getDefaultParamValue() const
        {
            if (auto* interface = pluginInstance.plugin)
            {
                const auto& hint = interface->PortRangeHints[paramID];
                const auto& desc = hint.HintDescriptor;

                if (LADSPA_IS_HINT_HAS_DEFAULT (desc))
                {
                    if (LADSPA_IS_HINT_DEFAULT_0 (desc))    return {};
                    if (LADSPA_IS_HINT_DEFAULT_1 (desc))    return { 1.0f, 1.0f };
                    if (LADSPA_IS_HINT_DEFAULT_100 (desc))  return { 100.0f, 0.5f };
                    if (LADSPA_IS_HINT_DEFAULT_440 (desc))  return { 440.0f, 0.5f };

                    const auto scale = LADSPA_IS_HINT_SAMPLE_RATE (desc) ? (f32) pluginInstance.getSampleRate()
                    : 1.0f;
                    const auto lower = hint.LowerBound * scale;
                    const auto upper = hint.UpperBound * scale;

                    if (LADSPA_IS_HINT_BOUNDED_BELOW (desc) && LADSPA_IS_HINT_DEFAULT_MINIMUM (desc))   return { lower, 0.0f };
                    if (LADSPA_IS_HINT_BOUNDED_ABOVE (desc) && LADSPA_IS_HINT_DEFAULT_MAXIMUM (desc))   return { upper, 1.0f };

                    if (LADSPA_IS_HINT_BOUNDED_BELOW (desc))
                    {
                        auto useLog = LADSPA_IS_HINT_LOGARITHMIC (desc);

                        if (LADSPA_IS_HINT_DEFAULT_LOW    (desc))  return { scaledValue (lower, upper, 0.25f, useLog), 0.25f };
                        if (LADSPA_IS_HINT_DEFAULT_MIDDLE (desc))  return { scaledValue (lower, upper, 0.50f, useLog), 0.50f };
                        if (LADSPA_IS_HINT_DEFAULT_HIGH   (desc))  return { scaledValue (lower, upper, 0.75f, useLog), 0.75f };
                    }
                }
            }

            return {};
        }

        z0 reset()
        {
            paramValue = getDefaultParamValue();
            defaultValue = paramValue.unscaled;
        }

        Txt getName (i32 /*maximumStringLength*/) const override    { return name; }
        Txt getLabel() const override                               { return {}; }

        b8 isAutomatable() const override                            { return automatable; }

        Txt getParameterID() const override
        {
            return Txt (paramID);
        }

        static f32 scaledValue (f32 low, f32 high, f32 alpha, b8 useLog) noexcept
        {
            if (useLog && low > 0 && high > 0)
                return expf (logf (low) * (1.0f - alpha) + logf (high) * alpha);

            return low + (high - low) * alpha;
        }

        static f32 toIntIfNecessary (const LADSPA_PortRangeHintDescriptor& desc, f32 value)
        {
            return LADSPA_IS_HINT_INTEGER (desc) ? ((f32) (i32) value) : value;
        }

        f32 getNewParamScaled (const LADSPA_PortRangeHint& hint, f32 newValue) const
        {
            const auto& desc = hint.HintDescriptor;

            if (LADSPA_IS_HINT_TOGGLED (desc))
                return (newValue < 0.5f) ? 0.0f : 1.0f;

            const auto scale = LADSPA_IS_HINT_SAMPLE_RATE (desc) ? (f32) pluginInstance.getSampleRate()
            : 1.0f;
            const auto lower = hint.LowerBound * scale;
            const auto upper = hint.UpperBound * scale;

            if (LADSPA_IS_HINT_BOUNDED_BELOW (desc) && LADSPA_IS_HINT_BOUNDED_ABOVE (desc))
                return toIntIfNecessary (desc, scaledValue (lower, upper, newValue, LADSPA_IS_HINT_LOGARITHMIC (desc)));

            if (LADSPA_IS_HINT_BOUNDED_BELOW (desc))   return toIntIfNecessary (desc, newValue);
            if (LADSPA_IS_HINT_BOUNDED_ABOVE (desc))   return toIntIfNecessary (desc, newValue * upper);

            return 0.0f;
        }

        LADSPAPluginInstance& pluginInstance;
        i32k paramID;
        const Txt name;
        const b8 automatable;

        ParameterValue paramValue;
        f32 defaultValue = 0.0f;
    };

    //==============================================================================
    LADSPA_Handle handle = nullptr;
    Txt name;
    CriticalSection lock;
    b8 initialised = false;
    AudioBuffer<f32> tempBuffer { 1, 1 };
    Array<i32> inputs, outputs;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAPluginInstance)
};


//==============================================================================
LADSPAPluginFormat::LADSPAPluginFormat() {}
LADSPAPluginFormat::~LADSPAPluginFormat() {}

z0 LADSPAPluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results, const Txt& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uniqueId = desc.deprecatedUid = 0;

    auto createdInstance = createInstanceFromDescription (desc, 44100.0, 512);
    auto instance = dynamic_cast<LADSPAPluginInstance*> (createdInstance.get());

    if (instance == nullptr || ! instance->isValid())
        return;

    instance->initialise (44100.0, 512);
    instance->fillInPluginDescription (desc);

    if (instance->module->moduleMain != nullptr)
    {
        for (i32 uid = 0;; ++uid)
        {
            if (auto* plugin = instance->module->moduleMain ((size_t) uid))
            {
                desc.uniqueId = desc.deprecatedUid = uid;
                desc.name = plugin->Name != nullptr ? plugin->Name : "Unknown";

                if (! arrayContainsPlugin (results, desc))
                    results.add (new PluginDescription (desc));
            }
            else
            {
                break;
            }
        }
    }
}

z0 LADSPAPluginFormat::createPluginInstance (const PluginDescription& desc,
                                               f64 sampleRate, i32 blockSize,
                                               PluginCreationCallback callback)
{
    std::unique_ptr<LADSPAPluginInstance> result;

    if (fileMightContainThisPluginType (desc.fileOrIdentifier))
    {
        auto file = File (desc.fileOrIdentifier);

        auto previousWorkingDirectory = File::getCurrentWorkingDirectory();
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        const LADSPAModuleHandle::Ptr module (LADSPAModuleHandle::findOrCreateModule (file));

        if (module != nullptr)
        {
            shellLADSPAUIDToCreate = desc.uniqueId != 0 ? desc.uniqueId : desc.deprecatedUid;

            result.reset (new LADSPAPluginInstance (module));

            if (result->plugin != nullptr && result->isValid())
                result->initialise (sampleRate, blockSize);
            else
                result = nullptr;
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    Txt errorMsg;

    if (result == nullptr)
        errorMsg = TRANS ("Unable to load XXX plug-in file").replace ("XXX", "LADSPA");

    callback (std::move (result), errorMsg);
}

b8 LADSPAPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

b8 LADSPAPluginFormat::fileMightContainThisPluginType (const Txt& fileOrIdentifier)
{
    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);
    return f.existsAsFile() && f.hasFileExtension (".so");
}

Txt LADSPAPluginFormat::getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier)
{
    return fileOrIdentifier;
}

b8 LADSPAPluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).getLastModificationTime() != desc.lastFileModTime;
}

b8 LADSPAPluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    return File::createFileWithoutCheckingPath (desc.fileOrIdentifier).exists();
}

StringArray LADSPAPluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const b8 recursive, b8)
{
    StringArray results;

    for (i32 j = 0; j < directoriesToSearch.getNumPaths(); ++j)
        recursiveFileSearch (results, directoriesToSearch[j], recursive);

    return results;
}

z0 LADSPAPluginFormat::recursiveFileSearch (StringArray& results, const File& dir, const b8 recursive)
{

    for (const auto& iter : RangedDirectoryIterator (dir, false, "*", File::findFilesAndDirectories))
    {
        auto f = iter.getFile();
        b8 isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath LADSPAPluginFormat::getDefaultLocationsToSearch()
{
    return  { SystemStats::getEnvironmentVariable ("LADSPA_PATH", "/usr/lib/ladspa;/usr/local/lib/ladspa;~/.ladspa").replace (":", ";") };
}

} // namespace drx

#endif
