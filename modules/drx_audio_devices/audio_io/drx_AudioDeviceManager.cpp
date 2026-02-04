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

template <typename Setup>
static auto getSetupInfo (Setup& s, b8 isInput)
{
    struct SetupInfo
    {
        // f64 brackets so that we get the expression type, i.e. a (possibly const) reference
        decltype ((s.inputDeviceName)) name;
        decltype ((s.inputChannels)) channels;
        decltype ((s.useDefaultInputChannels)) useDefault;
    };

    return isInput ? SetupInfo { s.inputDeviceName,  s.inputChannels,  s.useDefaultInputChannels }
                   : SetupInfo { s.outputDeviceName, s.outputChannels, s.useDefaultOutputChannels };
}

static auto tie (const AudioDeviceManager::AudioDeviceSetup& s)
{
    return std::tie (s.outputDeviceName,
                     s.inputDeviceName,
                     s.sampleRate,
                     s.bufferSize,
                     s.inputChannels,
                     s.useDefaultInputChannels,
                     s.outputChannels,
                     s.useDefaultOutputChannels);
}

b8 AudioDeviceManager::AudioDeviceSetup::operator== (const AudioDeviceManager::AudioDeviceSetup& other) const
{
    return tie (*this) == tie (other);
}

b8 AudioDeviceManager::AudioDeviceSetup::operator!= (const AudioDeviceManager::AudioDeviceSetup& other) const
{
    return tie (*this) != tie (other);
}

//==============================================================================
class AudioDeviceManager::CallbackHandler final : public AudioIODeviceCallback,
                                                  public MidiInputCallback,
                                                  public AudioIODeviceType::Listener
{
public:
    CallbackHandler (AudioDeviceManager& adm) noexcept  : owner (adm) {}

private:
    z0 audioDeviceIOCallbackWithContext (const f32* const* ins,
                                           i32 numIns,
                                           f32* const* outs,
                                           i32 numOuts,
                                           i32 numSamples,
                                           const AudioIODeviceCallbackContext& context) override
    {
        owner.audioDeviceIOCallbackInt (ins, numIns, outs, numOuts, numSamples, context);
    }

    z0 audioDeviceAboutToStart (AudioIODevice* device) override
    {
        owner.audioDeviceAboutToStartInt (device);
    }

    z0 audioDeviceStopped() override
    {
        owner.audioDeviceStoppedInt();
    }

    z0 audioDeviceError (const Txt& message) override
    {
        owner.audioDeviceErrorInt (message);
    }

    z0 handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) override
    {
        owner.handleIncomingMidiMessageInt (source, message);
    }

    z0 audioDeviceListChanged() override
    {
        owner.audioDeviceListChanged();
    }

    AudioDeviceManager& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallbackHandler)
};

//==============================================================================
AudioDeviceManager::AudioDeviceManager()
{
    callbackHandler.reset (new CallbackHandler (*this));
}

AudioDeviceManager::~AudioDeviceManager()
{
    currentAudioDevice.reset();
    defaultMidiOutput.reset();
}

//==============================================================================
z0 AudioDeviceManager::createDeviceTypesIfNeeded()
{
    if (availableDeviceTypes.size() == 0)
    {
        OwnedArray<AudioIODeviceType> types;
        createAudioDeviceTypes (types);

        for (auto* t : types)
            addAudioDeviceType (std::unique_ptr<AudioIODeviceType> (t));

        types.clear (false);

        for (auto* type : availableDeviceTypes)
            type->scanForDevices();

        pickCurrentDeviceTypeWithDevices();
    }
}

z0 AudioDeviceManager::pickCurrentDeviceTypeWithDevices()
{
    const auto deviceTypeHasDevices = [] (const AudioIODeviceType* ptr)
    {
        return ! ptr->getDeviceNames (true) .isEmpty()
            || ! ptr->getDeviceNames (false).isEmpty();
    };

    if (auto* type = findType (currentDeviceType))
        if (deviceTypeHasDevices (type))
            return;

    const auto iter = std::find_if (availableDeviceTypes.begin(),
                                    availableDeviceTypes.end(),
                                    deviceTypeHasDevices);

    if (iter != availableDeviceTypes.end())
        currentDeviceType = (*iter)->getTypeName();
}

const OwnedArray<AudioIODeviceType>& AudioDeviceManager::getAvailableDeviceTypes()
{
    scanDevicesIfNeeded();
    return availableDeviceTypes;
}

z0 AudioDeviceManager::updateCurrentSetup()
{
    if (currentAudioDevice != nullptr)
    {
        currentSetup.sampleRate     = currentAudioDevice->getCurrentSampleRate();
        currentSetup.bufferSize     = currentAudioDevice->getCurrentBufferSizeSamples();
        currentSetup.inputChannels  = currentAudioDevice->getActiveInputChannels();
        currentSetup.outputChannels = currentAudioDevice->getActiveOutputChannels();
    }
}

z0 AudioDeviceManager::audioDeviceListChanged()
{
    if (currentAudioDevice != nullptr)
    {
        auto currentDeviceStillAvailable = [&]
        {
            auto currentTypeName = currentAudioDevice->getTypeName();
            auto currentDeviceName = currentAudioDevice->getName();

            for (auto* deviceType : availableDeviceTypes)
            {
                if (currentTypeName == deviceType->getTypeName())
                {
                    for (auto& deviceName : deviceType->getDeviceNames (true))
                        if (currentDeviceName == deviceName)
                            return true;

                    for (auto& deviceName : deviceType->getDeviceNames (false))
                        if (currentDeviceName == deviceName)
                            return true;
                }
            }

            return false;
        }();

        if (! currentDeviceStillAvailable)
        {
            closeAudioDevice();

            if (auto e = createStateXml())
                initialiseFromXML (*e, true, preferredDeviceName, &currentSetup);
            else
                initialiseDefault (preferredDeviceName, &currentSetup);
        }

        updateCurrentSetup();
    }

    sendChangeMessage();
}

z0 AudioDeviceManager::midiDeviceListChanged()
{
    openLastRequestedMidiDevices (midiDeviceInfosFromXml, defaultMidiOutputDeviceInfo);
    sendChangeMessage();
}

//==============================================================================
static z0 addIfNotNull (OwnedArray<AudioIODeviceType>& list, AudioIODeviceType* const device)
{
    if (device != nullptr)
        list.add (device);
}

z0 AudioDeviceManager::createAudioDeviceTypes (OwnedArray<AudioIODeviceType>& list)
{
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode::shared));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode::exclusive));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode::sharedLowLatency));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_DirectSound());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_ASIO());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_iOSAudio());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Bela());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_ALSA());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_JACK());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Oboe());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_OpenSLES());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Android());
}

z0 AudioDeviceManager::addAudioDeviceType (std::unique_ptr<AudioIODeviceType> newDeviceType)
{
    if (newDeviceType != nullptr)
    {
        jassert (lastDeviceTypeConfigs.size() == availableDeviceTypes.size());

        availableDeviceTypes.add (newDeviceType.release());
        lastDeviceTypeConfigs.add (new AudioDeviceSetup());

        availableDeviceTypes.getLast()->addListener (callbackHandler.get());
    }
}

z0 AudioDeviceManager::removeAudioDeviceType (AudioIODeviceType* deviceTypeToRemove)
{
    if (deviceTypeToRemove != nullptr)
    {
        jassert (lastDeviceTypeConfigs.size() == availableDeviceTypes.size());

        auto index = availableDeviceTypes.indexOf (deviceTypeToRemove);

        if (auto removed = std::unique_ptr<AudioIODeviceType> (availableDeviceTypes.removeAndReturn (index)))
        {
            removed->removeListener (callbackHandler.get());
            lastDeviceTypeConfigs.remove (index, true);
        }
    }
}

static b8 deviceListContains (AudioIODeviceType* type, b8 isInput, const Txt& name)
{
    for (auto& deviceName : type->getDeviceNames (isInput))
        if (deviceName.trim().equalsIgnoreCase (name.trim()))
            return true;

    return false;
}

//==============================================================================
Txt AudioDeviceManager::initialise (i32k numInputChannelsNeeded,
                                       i32k numOutputChannelsNeeded,
                                       const XmlElement* const xml,
                                       const b8 selectDefaultDeviceOnFailure,
                                       const Txt& preferredDefaultDeviceName,
                                       const AudioDeviceSetup* preferredSetupOptions)
{
    scanDevicesIfNeeded();
    pickCurrentDeviceTypeWithDevices();

    numInputChansNeeded = numInputChannelsNeeded;
    numOutputChansNeeded = numOutputChannelsNeeded;
    preferredDeviceName = preferredDefaultDeviceName;

    if (xml != nullptr && xml->hasTagName ("DEVICESETUP"))
        return initialiseFromXML (*xml, selectDefaultDeviceOnFailure,
                                  preferredDeviceName, preferredSetupOptions);

    return initialiseDefault (preferredDeviceName, preferredSetupOptions);
}

Txt AudioDeviceManager::initialiseDefault (const Txt& preferredDefaultDeviceName,
                                              const AudioDeviceSetup* preferredSetupOptions)
{
    AudioDeviceSetup setup;

    if (preferredSetupOptions != nullptr)
    {
        setup = *preferredSetupOptions;
    }
    else if (preferredDefaultDeviceName.isNotEmpty())
    {
        const auto nameMatches = [&preferredDefaultDeviceName] (const Txt& name)
        {
            return name.matchesWildcard (preferredDefaultDeviceName, true);
        };

        struct WildcardMatch
        {
            Txt value;
            b8 successful;
        };

        const auto getWildcardMatch = [&nameMatches] (const StringArray& names)
        {
            const auto iter = std::find_if (names.begin(), names.end(), nameMatches);
            return WildcardMatch { iter != names.end() ? *iter : Txt(), iter != names.end() };
        };

        struct WildcardMatches
        {
            WildcardMatch input, output;
        };

        const auto getMatchesForType = [&getWildcardMatch] (const AudioIODeviceType* type)
        {
            return WildcardMatches { getWildcardMatch (type->getDeviceNames (true)),
                                     getWildcardMatch (type->getDeviceNames (false)) };
        };

        struct SearchResult
        {
            Txt type, input, output;
        };

        const auto result = [&]
        {
            // First, look for a device type with an input and output which matches the preferred name
            for (auto* type : availableDeviceTypes)
            {
                const auto matches = getMatchesForType (type);

                if (matches.input.successful && matches.output.successful)
                    return SearchResult { type->getTypeName(), matches.input.value, matches.output.value };
            }

            // No device type has matching ins and outs, so fall back to a device where either the
            // input or output match
            for (auto* type : availableDeviceTypes)
            {
                const auto matches = getMatchesForType (type);

                if (matches.input.successful || matches.output.successful)
                    return SearchResult { type->getTypeName(), matches.input.value, matches.output.value };
            }

            // No devices match the query, so just use the default devices from the current type
            return SearchResult { currentDeviceType, {}, {} };
        }();

        currentDeviceType = result.type;
        setup.inputDeviceName = result.input;
        setup.outputDeviceName = result.output;
    }

    insertDefaultDeviceNames (setup);
    return setAudioDeviceSetup (setup, false);
}

Txt AudioDeviceManager::initialiseFromXML (const XmlElement& xml,
                                              b8 selectDefaultDeviceOnFailure,
                                              const Txt& preferredDefaultDeviceName,
                                              const AudioDeviceSetup* preferredSetupOptions)
{
    lastExplicitSettings.reset (new XmlElement (xml));

    Txt error;
    AudioDeviceSetup setup;

    if (preferredSetupOptions != nullptr)
        setup = *preferredSetupOptions;

    if (xml.getStringAttribute ("audioDeviceName").isNotEmpty())
    {
        setup.inputDeviceName = setup.outputDeviceName
            = xml.getStringAttribute ("audioDeviceName");
    }
    else
    {
        setup.inputDeviceName  = xml.getStringAttribute ("audioInputDeviceName");
        setup.outputDeviceName = xml.getStringAttribute ("audioOutputDeviceName");
    }

    currentDeviceType = xml.getStringAttribute ("deviceType");

    if (findType (currentDeviceType) == nullptr)
    {
        if (auto* type = findType (setup.inputDeviceName, setup.outputDeviceName))
            currentDeviceType = type->getTypeName();
        else if (auto* firstType = availableDeviceTypes.getFirst())
            currentDeviceType = firstType->getTypeName();
    }

    setup.bufferSize = xml.getIntAttribute ("audioDeviceBufferSize", setup.bufferSize);
    setup.sampleRate = xml.getDoubleAttribute ("audioDeviceRate", setup.sampleRate);

    setup.inputChannels .parseString (xml.getStringAttribute ("audioDeviceInChans",  "11"), 2);
    setup.outputChannels.parseString (xml.getStringAttribute ("audioDeviceOutChans", "11"), 2);

    setup.useDefaultInputChannels  = ! xml.hasAttribute ("audioDeviceInChans");
    setup.useDefaultOutputChannels = ! xml.hasAttribute ("audioDeviceOutChans");

    error = setAudioDeviceSetup (setup, true);

    if (error.isNotEmpty() && selectDefaultDeviceOnFailure)
        error = initialise (numInputChansNeeded, numOutputChansNeeded, nullptr, false, preferredDefaultDeviceName);

    enabledMidiInputs.clear();

    const auto midiInputs = [&]
    {
        Array<MidiDeviceInfo> result;

        for (auto* c : xml.getChildWithTagNameIterator ("MIDIINPUT"))
            result.add ({ c->getStringAttribute ("name"), c->getStringAttribute ("identifier") });

        return result;
    }();

    const MidiDeviceInfo defaultOutputDeviceInfo (xml.getStringAttribute ("defaultMidiOutput"),
                                                  xml.getStringAttribute ("defaultMidiOutputDevice"));

    openLastRequestedMidiDevices (midiInputs, defaultOutputDeviceInfo);

    return error;
}

z0 AudioDeviceManager::openLastRequestedMidiDevices (const Array<MidiDeviceInfo>& desiredInputs, const MidiDeviceInfo& defaultOutput)
{
    const auto openDeviceIfAvailable = [&] (const Array<MidiDeviceInfo>& devices,
                                            const MidiDeviceInfo& deviceToOpen,
                                            auto&& doOpen)
    {
        const auto iterWithMatchingIdentifier = std::find_if (devices.begin(), devices.end(), [&] (const auto& x)
        {
            return x.identifier == deviceToOpen.identifier;
        });

        if (iterWithMatchingIdentifier != devices.end())
        {
            doOpen (deviceToOpen.identifier);
            return;
        }

        const auto iterWithMatchingName = std::find_if (devices.begin(), devices.end(), [&] (const auto& x)
        {
            return x.name == deviceToOpen.name;
        });

        if (iterWithMatchingName != devices.end())
            doOpen (iterWithMatchingName->identifier);
    };

    midiDeviceInfosFromXml = desiredInputs;

    const auto inputs = MidiInput::getAvailableDevices();

    for (const auto& info : midiDeviceInfosFromXml)
        openDeviceIfAvailable (inputs, info, [&] (const auto identifier) { setMidiInputDeviceEnabled (identifier, true); });

    const auto outputs = MidiOutput::getAvailableDevices();

    openDeviceIfAvailable (outputs, defaultOutput, [&] (const auto identifier) { setDefaultMidiOutputDevice (identifier); });
}

Txt AudioDeviceManager::initialiseWithDefaultDevices (i32 numInputChannelsNeeded,
                                                         i32 numOutputChannelsNeeded)
{
    lastExplicitSettings.reset();

    return initialise (numInputChannelsNeeded, numOutputChannelsNeeded,
                       nullptr, false, {}, nullptr);
}

z0 AudioDeviceManager::insertDefaultDeviceNames (AudioDeviceSetup& setup) const
{
    enum class Direction { out, in };

    if (auto* type = getCurrentDeviceTypeObject())
    {
        // We avoid selecting a device pair that doesn't share a matching sample rate, if possible.
        // If not, other parts of the AudioDeviceManager and AudioIODevice classes should generate
        // an appropriate error message when opening or starting these devices.
        const auto getDevicesToTestForMatchingSampleRate = [&setup, type, this] (Direction dir)
        {
            const auto isInput = dir == Direction::in;
            const auto info = getSetupInfo (setup, isInput);

            if (! info.name.isEmpty())
                return StringArray { info.name };

            const auto numChannelsNeeded = isInput ? numInputChansNeeded : numOutputChansNeeded;
            auto deviceNames = numChannelsNeeded > 0 ? type->getDeviceNames (isInput) : StringArray {};
            deviceNames.move (type->getDefaultDeviceIndex (isInput), 0);

            return deviceNames;
        };

        std::map<std::pair<Direction, Txt>, Array<f64>> sampleRatesCache;

        const auto getSupportedSampleRates = [&sampleRatesCache, type] (Direction dir, const Txt& deviceName)
        {
            const auto key = std::make_pair (dir, deviceName);

            auto& entry = [&]() -> auto&
            {
                auto it = sampleRatesCache.find (key);

                if (it != sampleRatesCache.end())
                    return it->second;

                auto& elem = sampleRatesCache[key];
                auto tempDevice = rawToUniquePtr (type->createDevice ((dir == Direction::in) ? "" : deviceName,
                                                                      (dir == Direction::in) ? deviceName : ""));
                if (tempDevice != nullptr)
                    elem = tempDevice->getAvailableSampleRates();

                return elem;
            }();

            return entry;
        };

        const auto validate = [&getSupportedSampleRates] (const Txt& outputDeviceName, const Txt& inputDeviceName)
        {
            jassert (! outputDeviceName.isEmpty() && ! inputDeviceName.isEmpty());

            const auto outputSampleRates = getSupportedSampleRates (Direction::out, outputDeviceName);
            const auto inputSampleRates  = getSupportedSampleRates (Direction::in,  inputDeviceName);

            return std::any_of (inputSampleRates.begin(),
                                inputSampleRates.end(),
                                [&] (auto inputSampleRate) { return outputSampleRates.contains (inputSampleRate); });
        };

        auto outputsToTest = getDevicesToTestForMatchingSampleRate (Direction::out);
        auto inputsToTest  = getDevicesToTestForMatchingSampleRate (Direction::in);

        // We set default device names, so in case no in-out pair passes the validation, we still
        // produce the same result as before
        if (setup.outputDeviceName.isEmpty() && ! outputsToTest.isEmpty())
            setup.outputDeviceName = outputsToTest[0];

        if (setup.inputDeviceName.isEmpty() && ! inputsToTest.isEmpty())
            setup.inputDeviceName = inputsToTest[0];

        // No pairs to validate
        if (outputsToTest.size() < 2 && inputsToTest.size() < 2)
            return;

        // We check all possible in-out pairs until the first validation pass. If no pair passes we
        // leave the setup unchanged.
        for (const auto& out : outputsToTest)
        {
            for (const auto& in : inputsToTest)
            {
                if (validate (out, in))
                {
                    setup.outputDeviceName = out;
                    setup.inputDeviceName  = in;

                    return;
                }
            }
        }
    }
}

std::unique_ptr<XmlElement> AudioDeviceManager::createStateXml() const
{
    if (lastExplicitSettings != nullptr)
        return std::make_unique<XmlElement> (*lastExplicitSettings);

    return {};
}

//==============================================================================
z0 AudioDeviceManager::scanDevicesIfNeeded()
{
    if (listNeedsScanning)
    {
        listNeedsScanning = false;

        createDeviceTypesIfNeeded();

        for (auto* type : availableDeviceTypes)
            type->scanForDevices();
    }
}

AudioIODeviceType* AudioDeviceManager::findType (const Txt& typeName)
{
    scanDevicesIfNeeded();

    for (auto* type : availableDeviceTypes)
        if (type->getTypeName() == typeName)
            return type;

    return {};
}

AudioIODeviceType* AudioDeviceManager::findType (const Txt& inputName, const Txt& outputName)
{
    scanDevicesIfNeeded();

    for (auto* type : availableDeviceTypes)
        if ((inputName.isNotEmpty() && deviceListContains (type, true, inputName))
             || (outputName.isNotEmpty() && deviceListContains (type, false, outputName)))
            return type;

    return {};
}

AudioDeviceManager::AudioDeviceSetup AudioDeviceManager::getAudioDeviceSetup() const
{
    return currentSetup;
}

z0 AudioDeviceManager::getAudioDeviceSetup (AudioDeviceSetup& setup) const
{
    setup = currentSetup;
}

z0 AudioDeviceManager::deleteCurrentDevice()
{
    currentAudioDevice.reset();
    currentSetup.inputDeviceName.clear();
    currentSetup.outputDeviceName.clear();
}

z0 AudioDeviceManager::setCurrentAudioDeviceType (const Txt& type, b8 treatAsChosenDevice)
{
    for (i32 i = 0; i < availableDeviceTypes.size(); ++i)
    {
        if (availableDeviceTypes.getUnchecked (i)->getTypeName() == type
             && currentDeviceType != type)
        {
            if (currentAudioDevice != nullptr)
            {
                closeAudioDevice();
                Thread::sleep (1500); // allow a moment for OS devices to sort themselves out, to help
                                      // avoid things like DirectSound/ASIO clashes
            }

            currentDeviceType = type;

            AudioDeviceSetup s (*lastDeviceTypeConfigs.getUnchecked (i));
            insertDefaultDeviceNames (s);

            setAudioDeviceSetup (s, treatAsChosenDevice);

            sendChangeMessage();
            break;
        }
    }
}

AudioWorkgroup AudioDeviceManager::getDeviceAudioWorkgroup() const
{
    return currentAudioDevice != nullptr ? currentAudioDevice->getWorkgroup() : AudioWorkgroup{};
}

AudioIODeviceType* AudioDeviceManager::getCurrentDeviceTypeObject() const
{
    for (auto* type : availableDeviceTypes)
        if (type->getTypeName() == currentDeviceType)
            return type;

    return availableDeviceTypes.getFirst();
}

static z0 updateSetupChannels (AudioDeviceManager::AudioDeviceSetup& setup, i32 defaultNumIns, i32 defaultNumOuts)
{
    auto updateChannels = [] (const Txt& deviceName, BigInteger& channels, i32 defaultNumChannels)
    {
        if (deviceName.isEmpty())
        {
            channels.clear();
        }
        else if (defaultNumChannels != -1)
        {
            channels.clear();
            channels.setRange (0, defaultNumChannels, true);
        }
    };

    updateChannels (setup.inputDeviceName,  setup.inputChannels,  setup.useDefaultInputChannels  ? defaultNumIns  : -1);
    updateChannels (setup.outputDeviceName, setup.outputChannels, setup.useDefaultOutputChannels ? defaultNumOuts : -1);
}

Txt AudioDeviceManager::setAudioDeviceSetup (const AudioDeviceSetup& newSetup,
                                                b8 treatAsChosenDevice)
{
    jassert (&newSetup != &currentSetup);    // this will have no effect

    if (newSetup != currentSetup)
        sendChangeMessage();
    else if (currentAudioDevice != nullptr)
        return {};

    stopDevice();

    if (getCurrentDeviceTypeObject() == nullptr
        || (newSetup.inputDeviceName.isEmpty() && newSetup.outputDeviceName.isEmpty()))
    {
        deleteCurrentDevice();

        if (treatAsChosenDevice)
            updateXml();

        return {};
    }

    Txt error;

    const auto needsNewDevice = currentSetup.inputDeviceName  != newSetup.inputDeviceName
                             || currentSetup.outputDeviceName != newSetup.outputDeviceName
                             || currentAudioDevice == nullptr;

    if (needsNewDevice)
    {
        deleteCurrentDevice();
        scanDevicesIfNeeded();

        auto* type = getCurrentDeviceTypeObject();

        for (const auto isInput : { false, true })
        {
            const auto name = getSetupInfo (newSetup, isInput).name;

            if (name.isNotEmpty() && ! deviceListContains (type, isInput, name))
                return "No such device: " + name;
        }

        currentAudioDevice.reset (type->createDevice (newSetup.outputDeviceName, newSetup.inputDeviceName));

        if (currentAudioDevice == nullptr)
            error = "Can't open the audio device!\n\n"
                    "This may be because another application is currently using the same device - "
                    "if so, you should close any other applications and try again!";
        else
            error = currentAudioDevice->getLastError();

        if (error.isNotEmpty())
        {
            deleteCurrentDevice();
            return error;
        }
    }

    currentSetup = newSetup;

    if (! currentSetup.useDefaultInputChannels)    numInputChansNeeded  = currentSetup.inputChannels.countNumberOfSetBits();
    if (! currentSetup.useDefaultOutputChannels)   numOutputChansNeeded = currentSetup.outputChannels.countNumberOfSetBits();

    updateSetupChannels (currentSetup, numInputChansNeeded, numOutputChansNeeded);

    if (currentSetup.inputChannels.isZero() && currentSetup.outputChannels.isZero())
    {
        if (treatAsChosenDevice)
            updateXml();

        return {};
    }

    currentSetup.sampleRate = chooseBestSampleRate (currentSetup.sampleRate);
    currentSetup.bufferSize = chooseBestBufferSize (currentSetup.bufferSize);

    error = currentAudioDevice->open (currentSetup.inputChannels,
                                      currentSetup.outputChannels,
                                      currentSetup.sampleRate,
                                      currentSetup.bufferSize);

    if (error.isEmpty())
    {
        currentDeviceType = currentAudioDevice->getTypeName();

        currentAudioDevice->start (callbackHandler.get());

        error = currentAudioDevice->getLastError();
    }

    if (error.isEmpty())
    {
        updateCurrentSetup();

        for (i32 i = 0; i < availableDeviceTypes.size(); ++i)
            if (availableDeviceTypes.getUnchecked (i)->getTypeName() == currentDeviceType)
                *(lastDeviceTypeConfigs.getUnchecked (i)) = currentSetup;

        if (treatAsChosenDevice)
            updateXml();
    }
    else
    {
        deleteCurrentDevice();
    }

    return error;
}

f64 AudioDeviceManager::chooseBestSampleRate (f64 rate) const
{
    jassert (currentAudioDevice != nullptr);

    auto rates = currentAudioDevice->getAvailableSampleRates();

    if (rate > 0 && rates.contains (rate))
        return rate;

    rate = currentAudioDevice->getCurrentSampleRate();

    if (rate > 0 && rates.contains (rate))
        return rate;

    f64 lowestAbove44 = 0.0;

    for (i32 i = rates.size(); --i >= 0;)
    {
        auto sr = rates[i];

        if (sr >= 44100.0 && (lowestAbove44 < 1.0 || sr < lowestAbove44))
            lowestAbove44 = sr;
    }

    if (lowestAbove44 > 0.0)
        return lowestAbove44;

    return rates[0];
}

i32 AudioDeviceManager::chooseBestBufferSize (i32 bufferSize) const
{
    jassert (currentAudioDevice != nullptr);

    if (bufferSize > 0 && currentAudioDevice->getAvailableBufferSizes().contains (bufferSize))
        return bufferSize;

    return currentAudioDevice->getDefaultBufferSize();
}

z0 AudioDeviceManager::stopDevice()
{
    if (currentAudioDevice != nullptr)
        currentAudioDevice->stop();

    testSound.reset();
}

z0 AudioDeviceManager::closeAudioDevice()
{
    stopDevice();
    currentAudioDevice.reset();
    loadMeasurer.reset();
}

z0 AudioDeviceManager::restartLastAudioDevice()
{
    if (currentAudioDevice == nullptr)
    {
        if (currentSetup.inputDeviceName.isEmpty()
              && currentSetup.outputDeviceName.isEmpty())
        {
            // This method will only reload the last device that was running
            // before closeAudioDevice() was called - you need to actually open
            // one first, with setAudioDeviceSetup().
            jassertfalse;
            return;
        }

        AudioDeviceSetup s (currentSetup);
        setAudioDeviceSetup (s, false);
    }
}

z0 AudioDeviceManager::updateXml()
{
    lastExplicitSettings.reset (new XmlElement ("DEVICESETUP"));

    lastExplicitSettings->setAttribute ("deviceType", currentDeviceType);
    lastExplicitSettings->setAttribute ("audioOutputDeviceName", currentSetup.outputDeviceName);
    lastExplicitSettings->setAttribute ("audioInputDeviceName", currentSetup.inputDeviceName);

    if (currentAudioDevice != nullptr)
    {
        lastExplicitSettings->setAttribute ("audioDeviceRate",       currentAudioDevice->getCurrentSampleRate());
        lastExplicitSettings->setAttribute ("audioDeviceBufferSize", currentAudioDevice->getCurrentBufferSizeSamples());

        if (! currentSetup.useDefaultInputChannels)
            lastExplicitSettings->setAttribute ("audioDeviceInChans", currentSetup.inputChannels.toString (2));

        if (! currentSetup.useDefaultOutputChannels)
            lastExplicitSettings->setAttribute ("audioDeviceOutChans", currentSetup.outputChannels.toString (2));
    }

    for (auto& input : enabledMidiInputs)
    {
        auto* child = lastExplicitSettings->createNewChildElement ("MIDIINPUT");

        child->setAttribute ("name",       input->getName());
        child->setAttribute ("identifier", input->getIdentifier());
    }

    if (midiDeviceInfosFromXml.size() > 0)
    {
        // Add any midi devices that have been enabled before, but which aren't currently
        // open because the device has been disconnected.
        auto availableMidiDevices = MidiInput::getAvailableDevices();

        for (auto& d : midiDeviceInfosFromXml)
        {
            if (! availableMidiDevices.contains (d))
            {
                auto* child = lastExplicitSettings->createNewChildElement ("MIDIINPUT");

                child->setAttribute ("name",       d.name);
                child->setAttribute ("identifier", d.identifier);
            }
        }
    }

    if (defaultMidiOutputDeviceInfo != MidiDeviceInfo())
    {
        lastExplicitSettings->setAttribute ("defaultMidiOutput",       defaultMidiOutputDeviceInfo.name);
        lastExplicitSettings->setAttribute ("defaultMidiOutputDevice", defaultMidiOutputDeviceInfo.identifier);
    }
}

//==============================================================================
z0 AudioDeviceManager::addAudioCallback (AudioIODeviceCallback* newCallback)
{
    {
        const ScopedLock sl (audioCallbackLock);

        if (callbacks.contains (newCallback))
            return;
    }

    if (currentAudioDevice != nullptr && newCallback != nullptr)
        newCallback->audioDeviceAboutToStart (currentAudioDevice.get());

    const ScopedLock sl (audioCallbackLock);
    callbacks.add (newCallback);
}

z0 AudioDeviceManager::removeAudioCallback (AudioIODeviceCallback* callbackToRemove)
{
    if (callbackToRemove != nullptr)
    {
        b8 needsDeinitialising = currentAudioDevice != nullptr;

        {
            const ScopedLock sl (audioCallbackLock);

            needsDeinitialising = needsDeinitialising && callbacks.contains (callbackToRemove);
            callbacks.removeFirstMatchingValue (callbackToRemove);
        }

        if (needsDeinitialising)
            callbackToRemove->audioDeviceStopped();
    }
}

z0 AudioDeviceManager::audioDeviceIOCallbackInt (const f32* const* inputChannelData,
                                                   i32 numInputChannels,
                                                   f32* const* outputChannelData,
                                                   i32 numOutputChannels,
                                                   i32 numSamples,
                                                   const AudioIODeviceCallbackContext& context)
{
    const ScopedLock sl (audioCallbackLock);

    inputLevelGetter->updateLevel (inputChannelData, numInputChannels, numSamples);

    if (callbacks.size() > 0)
    {
        AudioProcessLoadMeasurer::ScopedTimer timer (loadMeasurer, numSamples);

        tempBuffer.setSize (jmax (1, numOutputChannels), jmax (1, numSamples), false, false, true);

        callbacks.getUnchecked (0)->audioDeviceIOCallbackWithContext (inputChannelData,
                                                                      numInputChannels,
                                                                      outputChannelData,
                                                                      numOutputChannels,
                                                                      numSamples,
                                                                      context);

        auto* const* tempChans = tempBuffer.getArrayOfWritePointers();

        for (i32 i = callbacks.size(); --i > 0;)
        {
            callbacks.getUnchecked (i)->audioDeviceIOCallbackWithContext (inputChannelData,
                                                                          numInputChannels,
                                                                          tempChans,
                                                                          numOutputChannels,
                                                                          numSamples,
                                                                          context);

            for (i32 chan = 0; chan < numOutputChannels; ++chan)
            {
                if (auto* src = tempChans [chan])
                    if (auto* dst = outputChannelData [chan])
                        for (i32 j = 0; j < numSamples; ++j)
                            dst[j] += src[j];
            }
        }
    }
    else
    {
        for (i32 i = 0; i < numOutputChannels; ++i)
            zeromem (outputChannelData[i], (size_t) numSamples * sizeof (f32));
    }

    if (testSound != nullptr)
    {
        auto numSamps = jmin (numSamples, testSound->getNumSamples() - testSoundPosition);
        auto* src = testSound->getReadPointer (0, testSoundPosition);

        for (i32 i = 0; i < numOutputChannels; ++i)
            if (auto* dst = outputChannelData [i])
                for (i32 j = 0; j < numSamps; ++j)
                    dst[j] += src[j];

        testSoundPosition += numSamps;

        if (testSoundPosition >= testSound->getNumSamples())
            testSound.reset();
    }

    outputLevelGetter->updateLevel (outputChannelData, numOutputChannels, numSamples);
}

z0 AudioDeviceManager::audioDeviceAboutToStartInt (AudioIODevice* const device)
{
    loadMeasurer.reset (device->getCurrentSampleRate(),
                        device->getCurrentBufferSizeSamples());

    updateCurrentSetup();

    {
        const ScopedLock sl (audioCallbackLock);

        for (i32 i = callbacks.size(); --i >= 0;)
            callbacks.getUnchecked (i)->audioDeviceAboutToStart (device);
    }

    sendChangeMessage();
}

z0 AudioDeviceManager::audioDeviceStoppedInt()
{
    sendChangeMessage();

    const ScopedLock sl (audioCallbackLock);

    loadMeasurer.reset();

    for (i32 i = callbacks.size(); --i >= 0;)
        callbacks.getUnchecked (i)->audioDeviceStopped();
}

z0 AudioDeviceManager::audioDeviceErrorInt (const Txt& message)
{
    const ScopedLock sl (audioCallbackLock);

    for (i32 i = callbacks.size(); --i >= 0;)
        callbacks.getUnchecked (i)->audioDeviceError (message);
}

f64 AudioDeviceManager::getCpuUsage() const
{
    return loadMeasurer.getLoadAsProportion();
}

//==============================================================================
z0 AudioDeviceManager::setMidiInputDeviceEnabled (const Txt& identifier, b8 enabled)
{
    if (enabled != isMidiInputDeviceEnabled (identifier))
    {
        if (enabled)
        {
            if (auto midiIn = MidiInput::openDevice (identifier, callbackHandler.get()))
            {
                enabledMidiInputs.push_back (std::move (midiIn));
                enabledMidiInputs.back()->start();
            }
        }
        else
        {
            auto removePredicate = [identifier] (const std::unique_ptr<MidiInput>& in) { return in->getIdentifier() == identifier; };
            enabledMidiInputs.erase (std::remove_if (std::begin (enabledMidiInputs), std::end (enabledMidiInputs), removePredicate),
                                     std::end (enabledMidiInputs));
        }

        updateXml();
        sendChangeMessage();
    }
}

b8 AudioDeviceManager::isMidiInputDeviceEnabled (const Txt& identifier) const
{
    for (auto& mi : enabledMidiInputs)
        if (mi->getIdentifier() == identifier)
            return true;

    return false;
}

z0 AudioDeviceManager::addMidiInputDeviceCallback (const Txt& identifier, MidiInputCallback* callbackToAdd)
{
    removeMidiInputDeviceCallback (identifier, callbackToAdd);

    if (identifier.isEmpty() || isMidiInputDeviceEnabled (identifier))
    {
        const ScopedLock sl (midiCallbackLock);
        midiCallbacks.add ({ identifier, callbackToAdd });
    }
}

z0 AudioDeviceManager::removeMidiInputDeviceCallback (const Txt& identifier, MidiInputCallback* callbackToRemove)
{
    for (i32 i = midiCallbacks.size(); --i >= 0;)
    {
        auto& mc = midiCallbacks.getReference (i);

        if (mc.callback == callbackToRemove && mc.deviceIdentifier == identifier)
        {
            const ScopedLock sl (midiCallbackLock);
            midiCallbacks.remove (i);
        }
    }
}

z0 AudioDeviceManager::handleIncomingMidiMessageInt (MidiInput* source, const MidiMessage& message)
{
    if (! message.isActiveSense())
    {
        const ScopedLock sl (midiCallbackLock);

        for (auto& mc : midiCallbacks)
            if (mc.deviceIdentifier.isEmpty() || mc.deviceIdentifier == source->getIdentifier())
                mc.callback->handleIncomingMidiMessage (source, message);
    }
}

//==============================================================================
z0 AudioDeviceManager::setDefaultMidiOutputDevice (const Txt& identifier)
{
    if (defaultMidiOutputDeviceInfo.identifier != identifier)
    {
        std::unique_ptr<MidiOutput> oldMidiPort;
        Array<AudioIODeviceCallback*> oldCallbacks;

        {
            const ScopedLock sl (audioCallbackLock);
            oldCallbacks.swapWith (callbacks);
        }

        if (currentAudioDevice != nullptr)
            for (i32 i = oldCallbacks.size(); --i >= 0;)
                oldCallbacks.getUnchecked (i)->audioDeviceStopped();

        std::swap (oldMidiPort, defaultMidiOutput);

        if (identifier.isNotEmpty())
            defaultMidiOutput = MidiOutput::openDevice (identifier);

        if (defaultMidiOutput != nullptr)
            defaultMidiOutputDeviceInfo = defaultMidiOutput->getDeviceInfo();
        else
            defaultMidiOutputDeviceInfo = {};

        if (currentAudioDevice != nullptr)
            for (auto* c : oldCallbacks)
                c->audioDeviceAboutToStart (currentAudioDevice.get());

        {
            const ScopedLock sl (audioCallbackLock);
            oldCallbacks.swapWith (callbacks);
        }

        updateXml();
        sendSynchronousChangeMessage();
    }
}

//==============================================================================
AudioDeviceManager::LevelMeter::LevelMeter() noexcept : level() {}

z0 AudioDeviceManager::LevelMeter::updateLevel (const f32* const* channelData, i32 numChannels, i32 numSamples) noexcept
{
    if (getReferenceCount() <= 1)
        return;

    auto localLevel = level.get();

    if (numChannels > 0)
    {
        for (i32 j = 0; j < numSamples; ++j)
        {
            f32 s = 0;

            for (i32 i = 0; i < numChannels; ++i)
                s += std::abs (channelData[i][j]);

            s /= (f32) numChannels;

            const f32 decayFactor = 0.99992f;

            if (s > localLevel)
                localLevel = s;
            else if (localLevel > 0.001f)
                localLevel *= decayFactor;
            else
                localLevel = 0;
        }
    }
    else
    {
        localLevel = 0;
    }

    level = localLevel;
}

f64 AudioDeviceManager::LevelMeter::getCurrentLevel() const noexcept
{
    jassert (getReferenceCount() > 1);
    return level.get();
}

z0 AudioDeviceManager::playTestSound()
{
    { // cunningly nested to swap, unlock and delete in that order.
        std::unique_ptr<AudioBuffer<f32>> oldSound;

        {
            const ScopedLock sl (audioCallbackLock);
            std::swap (oldSound, testSound);
        }
    }

    testSoundPosition = 0;

    if (currentAudioDevice != nullptr)
    {
        auto sampleRate = currentAudioDevice->getCurrentSampleRate();
        auto soundLength = (i32) sampleRate;

        f64 frequency = 440.0;
        f32 amplitude = 0.5f;

        auto phasePerSample = MathConstants<f64>::twoPi / (sampleRate / frequency);

        std::unique_ptr<AudioBuffer<f32>> newSound (new AudioBuffer<f32> (1, soundLength));

        for (i32 i = 0; i < soundLength; ++i)
            newSound->setSample (0, i, amplitude * (f32) std::sin (i * phasePerSample));

        newSound->applyGainRamp (0, 0, soundLength / 10, 0.0f, 1.0f);
        newSound->applyGainRamp (0, soundLength - soundLength / 4, soundLength / 4, 1.0f, 0.0f);

        {
            const ScopedLock sl (audioCallbackLock);
            std::swap (testSound, newSound);
        }
    }
}

i32 AudioDeviceManager::getXRunCount() const noexcept
{
    auto deviceXRuns = (currentAudioDevice != nullptr ? currentAudioDevice->getXRunCount() : -1);
    return jmax (0, deviceXRuns) + loadMeasurer.getXRunCount();
}

//==============================================================================
// Deprecated
z0 AudioDeviceManager::setMidiInputEnabled (const Txt& name, const b8 enabled)
{
    for (auto& device : MidiInput::getAvailableDevices())
    {
        if (device.name == name)
        {
            setMidiInputDeviceEnabled (device.identifier, enabled);
            return;
        }
    }
}

b8 AudioDeviceManager::isMidiInputEnabled (const Txt& name) const
{
    for (auto& device : MidiInput::getAvailableDevices())
        if (device.name == name)
            return isMidiInputDeviceEnabled (device.identifier);

    return false;
}

z0 AudioDeviceManager::addMidiInputCallback (const Txt& name, MidiInputCallback* callbackToAdd)
{
    if (name.isEmpty())
    {
        addMidiInputDeviceCallback ({}, callbackToAdd);
    }
    else
    {
        for (auto& device : MidiInput::getAvailableDevices())
        {
            if (device.name == name)
            {
                addMidiInputDeviceCallback (device.identifier, callbackToAdd);
                return;
            }
        }
    }
}

z0 AudioDeviceManager::removeMidiInputCallback (const Txt& name, MidiInputCallback* callbackToRemove)
{
    if (name.isEmpty())
    {
        removeMidiInputDeviceCallback ({}, callbackToRemove);
    }
    else
    {
        for (auto& device : MidiInput::getAvailableDevices())
        {
            if (device.name == name)
            {
                removeMidiInputDeviceCallback (device.identifier, callbackToRemove);
                return;
            }
        }
    }
}

z0 AudioDeviceManager::setDefaultMidiOutput (const Txt& name)
{
    for (auto& device : MidiOutput::getAvailableDevices())
    {
        if (device.name == name)
        {
            setDefaultMidiOutputDevice (device.identifier);
            return;
        }
    }
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class AudioDeviceManagerTests final : public UnitTest
{
public:
    AudioDeviceManagerTests() : UnitTest ("AudioDeviceManager", UnitTestCategories::audio) {}

    z0 runTest() override
    {
        ScopedDrxInitialiser_GUI libraryInitialiser;

        beginTest ("When the AudioDeviceSetup has non-empty device names, initialise uses the requested devices");
        {
            AudioDeviceManager manager;
            initialiseManager (manager);

            expectEquals (manager.getAvailableDeviceTypes().size(), 2);

            AudioDeviceManager::AudioDeviceSetup setup;
            setup.outputDeviceName = "z";
            setup.inputDeviceName = "c";

            expect (manager.initialise (2, 2, nullptr, true, Txt{}, &setup).isEmpty());

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);
            expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        }

        beginTest ("When the AudioDeviceSetup has empty device names, initialise picks suitable default devices");
        {
            AudioDeviceManager manager;
            initialiseManager (manager);

            AudioDeviceManager::AudioDeviceSetup setup;

            expect (manager.initialise (2, 2, nullptr, true, Txt{}, &setup).isEmpty());

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, Txt ("x"));
            expectEquals (newSetup.inputDeviceName, Txt ("a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        }

        beginTest ("When the preferred device name matches an input and an output on the same type, that type is used");
        {
            AudioDeviceManager manager;
            initialiseManagerWithDifferentDeviceNames (manager);

            expect (manager.initialise (2, 2, nullptr, true, "bar *").isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), Txt ("bar"));

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, Txt ("bar out a"));
            expectEquals (newSetup.inputDeviceName, Txt ("bar in a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);

            expect (manager.getCurrentAudioDevice() != nullptr);
        }

        beginTest ("When the preferred device name matches either an input and an output, but not both, that type is used");
        {
            AudioDeviceManager manager;
            initialiseManagerWithDifferentDeviceNames (manager);

            expect (manager.initialise (2, 2, nullptr, true, "bar out b").isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), Txt ("bar"));

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, Txt ("bar out b"));
            expectEquals (newSetup.inputDeviceName, Txt ("bar in a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);

            expect (manager.getCurrentAudioDevice() != nullptr);
        }

        beginTest ("When the preferred device name does not match any inputs or outputs, defaults are used");
        {
            AudioDeviceManager manager;
            initialiseManagerWithDifferentDeviceNames (manager);

            expect (manager.initialise (2, 2, nullptr, true, "unmatchable").isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), Txt ("foo"));

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, Txt ("foo out a"));
            expectEquals (newSetup.inputDeviceName, Txt ("foo in a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);

            expect (manager.getCurrentAudioDevice() != nullptr);
        }

        beginTest ("When first device type has no devices, a device type with devices is used instead");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType (manager);

            AudioDeviceManager::AudioDeviceSetup setup;

            expect (manager.initialise (2, 2, nullptr, true, {}, &setup).isEmpty());

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, Txt ("x"));
            expectEquals (newSetup.inputDeviceName, Txt ("a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        }

        beginTest ("If a device type has been explicitly set to a type with devices, "
                   "initialisation should respect this choice");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType (manager);
            manager.setCurrentAudioDeviceType (mockBName, true);

            AudioDeviceManager::AudioDeviceSetup setup;
            expect (manager.initialise (2, 2, nullptr, true, {}, &setup).isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), mockBName);
        }

        beginTest ("If a device type has been explicitly set to a type without devices, "
                   "initialisation should pick a type with devices instead");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType (manager);
            manager.setCurrentAudioDeviceType (emptyName, true);

            AudioDeviceManager::AudioDeviceSetup setup;
            expect (manager.initialise (2, 2, nullptr, true, {}, &setup).isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), mockAName);
        }

        beginTest ("Carry out a i64 sequence of configuration changes");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType    (manager);
            initialiseWithDefaultDevices            (manager);
            disableInputChannelsButLeaveDeviceOpen  (manager);
            selectANewInputDevice                   (manager);
            disableInputDevice                      (manager);
            reenableInputDeviceWithNoChannels       (manager);
            enableInputChannels                     (manager);
            disableInputChannelsButLeaveDeviceOpen  (manager);
            switchDeviceType                        (manager);
            enableInputChannels                     (manager);
            closeDeviceByRequestingEmptyNames       (manager);
        }

        beginTest ("AudioDeviceManager updates its current settings before notifying callbacks when device restarts itself");
        {
            AudioDeviceManager manager;
            auto deviceType = std::make_unique<MockDeviceType> ("foo",
                                                                StringArray { "foo in a", "foo in b" },
                                                                StringArray { "foo out a", "foo out b" });
            auto* ptr = deviceType.get();
            manager.addAudioDeviceType (std::move (deviceType));

            AudioDeviceManager::AudioDeviceSetup setup;
            setup.sampleRate = 48000.0;
            setup.bufferSize = 256;
            setup.inputDeviceName = "foo in a";
            setup.outputDeviceName = "foo out a";
            setup.useDefaultInputChannels = true;
            setup.useDefaultOutputChannels = true;
            manager.setAudioDeviceSetup (setup, true);

            const auto currentSetup = manager.getAudioDeviceSetup();
            expectEquals (currentSetup.sampleRate, setup.sampleRate);
            expectEquals (currentSetup.bufferSize, setup.bufferSize);

            MockCallback callback;
            manager.addAudioCallback (&callback);

            constexpr auto newSr = 10000.0;
            constexpr auto newBs = 1024;
            auto numCalls = 0;

            // Compilers disagree about whether newSr and newBs need to be captured
            callback.aboutToStart = [&]
            {
                ++numCalls;
                const auto current = manager.getAudioDeviceSetup();
                expectEquals (current.sampleRate, newSr);
                expectEquals (current.bufferSize, newBs);
            };

            ptr->restartDevices (newSr, newBs);
            expectEquals (numCalls, 1);
        }
    }

private:
    z0 initialiseWithDefaultDevices (AudioDeviceManager& manager)
    {
        manager.initialiseWithDefaultDevices (2, 2);
        const auto& setup = manager.getAudioDeviceSetup();

        expectEquals (setup.inputChannels.countNumberOfSetBits(), 2);
        expectEquals (setup.outputChannels.countNumberOfSetBits(), 2);

        expect (setup.useDefaultInputChannels);
        expect (setup.useDefaultOutputChannels);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 disableInputChannelsButLeaveDeviceOpen (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputChannels.clear();
        setup.useDefaultInputChannels = false;

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 selectANewInputDevice (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "b";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 disableInputDevice (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 reenableInputDeviceWithNoChannels (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "a";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 enableInputChannels (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = manager.getCurrentDeviceTypeObject()->getDeviceNames (true)[0];
        setup.inputChannels = 3;
        setup.useDefaultInputChannels = false;

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 switchDeviceType (AudioDeviceManager& manager)
    {
        const auto oldSetup = manager.getAudioDeviceSetup();

        expectEquals (manager.getCurrentAudioDeviceType(), Txt (mockAName));

        manager.setCurrentAudioDeviceType (mockBName, true);

        expectEquals (manager.getCurrentAudioDeviceType(), Txt (mockBName));

        const auto newSetup = manager.getAudioDeviceSetup();

        expect (newSetup.outputDeviceName.isNotEmpty());
        // We had no channels enabled, which means we don't need to open a new input device
        expect (newSetup.inputDeviceName.isEmpty());

        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    z0 closeDeviceByRequestingEmptyNames (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "";
        setup.outputDeviceName = "";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (newSetup.inputDeviceName.isEmpty());
        expect (newSetup.outputDeviceName.isEmpty());

        expect (manager.getCurrentAudioDevice() == nullptr);
    }

    const Txt mockAName = "mockA";
    const Txt mockBName = "mockB";
    const Txt emptyName = "empty";

    struct Restartable
    {
        virtual ~Restartable() = default;
        virtual z0 restart (f64 newSr, i32 newBs) = 0;
    };

    class MockDevice final : public AudioIODevice,
                             private Restartable
    {
    public:
        MockDevice (ListenerList<Restartable>& l, Txt typeNameIn, Txt outNameIn, Txt inNameIn)
            : AudioIODevice ("mock", typeNameIn), listeners (l), outName (outNameIn), inName (inNameIn)
        {
            listeners.add (this);
        }

        ~MockDevice() override
        {
            listeners.remove (this);
        }

        StringArray getOutputChannelNames() override { return { "o1", "o2", "o3" }; }
        StringArray getInputChannelNames()  override { return { "i1", "i2", "i3" }; }

        Array<f64> getAvailableSampleRates() override { return { 44100.0, 48000.0 }; }
        Array<i32> getAvailableBufferSizes() override { return { 128, 256 }; }
        i32 getDefaultBufferSize() override { return 128; }

        Txt open (const BigInteger& inputs, const BigInteger& outputs, f64 sr, i32 bs) override
        {
            inChannels = inputs;
            outChannels = outputs;
            sampleRate = sr;
            blockSize = bs;
            on = true;
            return {};
        }

        z0 close() override { on = false; }
        b8 isOpen() override { return on; }

        z0 start (AudioIODeviceCallback* c) override
        {
            callback = c;
            callback->audioDeviceAboutToStart (this);
            playing = true;
        }

        z0 stop() override
        {
            playing = false;
            callback->audioDeviceStopped();
        }

        b8 isPlaying() override { return playing; }

        Txt getLastError() override { return {}; }
        i32 getCurrentBufferSizeSamples() override { return blockSize; }
        f64 getCurrentSampleRate() override { return sampleRate; }
        i32 getCurrentBitDepth() override { return 16; }

        BigInteger getActiveOutputChannels() const override { return outChannels; }
        BigInteger getActiveInputChannels()  const override { return inChannels; }

        i32 getOutputLatencyInSamples() override { return 0; }
        i32 getInputLatencyInSamples() override { return 0; }

    private:
        z0 restart (f64 newSr, i32 newBs) override
        {
            stop();
            close();
            open (inChannels, outChannels, newSr, newBs);
            start (callback);
        }

        ListenerList<Restartable>& listeners;
        AudioIODeviceCallback* callback = nullptr;
        Txt outName, inName;
        BigInteger outChannels, inChannels;
        f64 sampleRate = 0.0;
        i32 blockSize = 0;
        b8 on = false, playing = false;
    };

    class MockDeviceType final : public AudioIODeviceType
    {
    public:
        explicit MockDeviceType (Txt kind)
            : MockDeviceType (std::move (kind), { "a", "b", "c" }, { "x", "y", "z" }) {}

        MockDeviceType (Txt kind, StringArray inputNames, StringArray outputNames)
            : AudioIODeviceType (std::move (kind)),
              inNames (std::move (inputNames)),
              outNames (std::move (outputNames)) {}

        ~MockDeviceType() override
        {
            // A Device outlived its DeviceType!
            jassert (listeners.isEmpty());
        }

        z0 scanForDevices() override {}

        StringArray getDeviceNames (b8 isInput) const override
        {
            return getNames (isInput);
        }

        i32 getDefaultDeviceIndex (b8) const override { return 0; }

        i32 getIndexOfDevice (AudioIODevice* device, b8 isInput) const override
        {
            return getNames (isInput).indexOf (device->getName());
        }

        b8 hasSeparateInputsAndOutputs() const override { return true; }

        AudioIODevice* createDevice (const Txt& outputName, const Txt& inputName) override
        {
            if (inNames.contains (inputName) || outNames.contains (outputName))
                return new MockDevice (listeners, getTypeName(), outputName, inputName);

            return nullptr;
        }

        // Call this to emulate the device restarting itself with new settings.
        // This might happen e.g. when a user changes the ASIO settings.
        z0 restartDevices (f64 newSr, i32 newBs)
        {
            listeners.call ([&] (auto& l) { return l.restart (newSr, newBs); });
        }

    private:
        const StringArray& getNames (b8 isInput) const { return isInput ? inNames : outNames; }

        const StringArray inNames, outNames;
        ListenerList<Restartable> listeners;
    };

    class MockCallback final : public AudioIODeviceCallback
    {
    public:
        std::function<z0()> callback;
        std::function<z0()> aboutToStart;
        std::function<z0()> stopped;
        std::function<z0()> error;

        z0 audioDeviceIOCallbackWithContext (const f32* const*,
                                               i32,
                                               f32* const*,
                                               i32,
                                               i32,
                                               const AudioIODeviceCallbackContext&) override
        {
            NullCheckedInvocation::invoke (callback);
        }

        z0 audioDeviceAboutToStart (AudioIODevice*) override { NullCheckedInvocation::invoke (aboutToStart); }
        z0 audioDeviceStopped()                     override { NullCheckedInvocation::invoke (stopped); }
        z0 audioDeviceError (const Txt&)         override { NullCheckedInvocation::invoke (error); }
    };

    z0 initialiseManager (AudioDeviceManager& manager)
    {
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> (mockAName));
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> (mockBName));
    }

    z0 initialiseManagerWithEmptyDeviceType (AudioDeviceManager& manager)
    {
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> (emptyName, StringArray{}, StringArray{}));
        initialiseManager (manager);
    }

    z0 initialiseManagerWithDifferentDeviceNames (AudioDeviceManager& manager)
    {
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> ("foo",
                                                                      StringArray { "foo in a", "foo in b" },
                                                                      StringArray { "foo out a", "foo out b" }));

        manager.addAudioDeviceType (std::make_unique<MockDeviceType> ("bar",
                                                                      StringArray { "bar in a", "bar in b" },
                                                                      StringArray { "bar out a", "bar out b" }));
    }
};

static AudioDeviceManagerTests audioDeviceManagerTests;

#endif

} // namespace drx
