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

#pragma once

#ifndef DOXYGEN
 #include <drx_audio_plugin_client/detail/drx_CreatePluginFilter.h>
#endif

namespace drx
{

//==============================================================================
/**
    An object that creates and plays a standalone instance of an AudioProcessor.

    The object will create your processor using the same createPluginFilter()
    function that the other plugin wrappers use, and will run it through the
    computer's audio/MIDI devices using AudioDeviceManager and AudioProcessorPlayer.

    @tags{Audio}
*/
class StandalonePluginHolder    : private AudioIODeviceCallback,
                                  private Timer,
                                  private Value::Listener
{
public:
    //==============================================================================
    /** Structure used for the number of inputs and outputs. */
    struct PluginInOuts   { short numIns, numOuts; };

    //==============================================================================
    /** Creates an instance of the default plugin.

        The settings object can be a PropertySet that the class should use to store its
        settings - the takeOwnershipOfSettings indicates whether this object will delete
        the settings automatically when no longer needed. The settings can also be nullptr.

        A default device name can be passed in.

        Preferably a complete setup options object can be used, which takes precedence over
        the preferredDefaultDeviceName and allows you to select the input & output device names,
        sample rate, buffer size etc.

        In all instances, the settingsToUse will take precedence over the "preferred" options if not null.
    */
    StandalonePluginHolder (PropertySet* settingsToUse,
                            b8 takeOwnershipOfSettings = true,
                            const Txt& preferredDefaultDeviceName = Txt(),
                            const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions = nullptr,
                            const Array<PluginInOuts>& channels = Array<PluginInOuts>(),
                           #if DRX_ANDROID || DRX_IOS
                            b8 shouldAutoOpenMidiDevices = true
                           #else
                            b8 shouldAutoOpenMidiDevices = false
                           #endif
                            )

        : settings (settingsToUse, takeOwnershipOfSettings),
          channelConfiguration (channels),
          autoOpenMidiDevices (shouldAutoOpenMidiDevices)
    {
        // Only one StandalonePluginHolder may be created at a time
        jassert (currentInstance == nullptr);
        currentInstance = this;

        shouldMuteInput.addListener (this);
        shouldMuteInput = ! isInterAppAudioConnected();

        handleCreatePlugin();

        auto inChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numIns
                                                           : processor->getMainBusNumInputChannels());

        if (preferredSetupOptions != nullptr)
            options.reset (new AudioDeviceManager::AudioDeviceSetup (*preferredSetupOptions));

        auto audioInputRequired = (inChannels > 0);

        if (audioInputRequired && RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
            && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
            RuntimePermissions::request (RuntimePermissions::recordAudio,
                                         [this, preferredDefaultDeviceName] (b8 granted) { init (granted, preferredDefaultDeviceName); });
        else
            init (audioInputRequired, preferredDefaultDeviceName);
    }

    z0 init (b8 enableAudioInput, const Txt& preferredDefaultDeviceName)
    {
        setupAudioDevices (enableAudioInput, preferredDefaultDeviceName, options.get());
        reloadPluginState();
        startPlaying();

       if (autoOpenMidiDevices)
           startTimer (500);
    }

    ~StandalonePluginHolder() override
    {
        stopTimer();

        handleDeletePlugin();
        shutDownAudioDevices();

        currentInstance = nullptr;
    }

    //==============================================================================
    virtual z0 createPlugin()
    {
        handleCreatePlugin();
    }

    virtual z0 deletePlugin()
    {
        handleDeletePlugin();
    }

    i32 getNumInputChannels() const
    {
        if (processor == nullptr)
            return 0;

        return (channelConfiguration.size() > 0 ? channelConfiguration[0].numIns
                                                : processor->getMainBusNumInputChannels());
    }

    i32 getNumOutputChannels() const
    {
        if (processor == nullptr)
            return 0;

        return (channelConfiguration.size() > 0 ? channelConfiguration[0].numOuts
                                                : processor->getMainBusNumOutputChannels());
    }

    static Txt getFilePatterns (const Txt& fileSuffix)
    {
        if (fileSuffix.isEmpty())
            return {};

        return (fileSuffix.startsWithChar ('.') ? "*" : "*.") + fileSuffix;
    }

    //==============================================================================
    Value& getMuteInputValue()                           { return shouldMuteInput; }
    b8 getProcessorHasPotentialFeedbackLoop() const    { return processorHasPotentialFeedbackLoop; }
    z0 valueChanged (Value& value) override            { muteInput = (b8) value.getValue(); }

    //==============================================================================
    File getLastFile() const
    {
        File f;

        if (settings != nullptr)
            f = File (settings->getValue ("lastStateFile"));

        if (f == File())
            f = File::getSpecialLocation (File::userDocumentsDirectory);

        return f;
    }

    z0 setLastFile (const FileChooser& fc)
    {
        if (settings != nullptr)
            settings->setValue ("lastStateFile", fc.getResult().getFullPathName());
    }

    /** Pops up a dialog letting the user save the processor's state to a file. */
    z0 askUserToSaveState (const Txt& fileSuffix = Txt())
    {
        stateFileChooser = std::make_unique<FileChooser> (TRANS ("Save current state"),
                                                          getLastFile(),
                                                          getFilePatterns (fileSuffix));
        auto flags = FileBrowserComponent::saveMode
                   | FileBrowserComponent::canSelectFiles
                   | FileBrowserComponent::warnAboutOverwriting;

        stateFileChooser->launchAsync (flags, [this] (const FileChooser& fc)
        {
            if (fc.getResult() == File{})
                return;

            setLastFile (fc);

            MemoryBlock data;
            processor->getStateInformation (data);

            if (! fc.getResult().replaceWithData (data.getData(), data.getSize()))
            {
                auto opts = MessageBoxOptions::makeOptionsOk (AlertWindow::WarningIcon,
                                                              TRANS ("Error whilst saving"),
                                                              TRANS ("Couldn't write to the specified file!"));
                messageBox = AlertWindow::showScopedAsync (opts, nullptr);
            }
        });
    }

    /** Pops up a dialog letting the user re-load the processor's state from a file. */
    z0 askUserToLoadState (const Txt& fileSuffix = Txt())
    {
        stateFileChooser = std::make_unique<FileChooser> (TRANS ("Load a saved state"),
                                                          getLastFile(),
                                                          getFilePatterns (fileSuffix));
        auto flags = FileBrowserComponent::openMode
                   | FileBrowserComponent::canSelectFiles;

        stateFileChooser->launchAsync (flags, [this] (const FileChooser& fc)
        {
            if (fc.getResult() == File{})
                return;

            setLastFile (fc);

            MemoryBlock data;

            if (fc.getResult().loadFileAsData (data))
            {
                processor->setStateInformation (data.getData(), (i32) data.getSize());
            }
            else
            {
                auto opts = MessageBoxOptions::makeOptionsOk (AlertWindow::WarningIcon,
                                                              TRANS ("Error whilst loading"),
                                                              TRANS ("Couldn't read from the specified file!"));
                messageBox = AlertWindow::showScopedAsync (opts, nullptr);
            }
        });
    }

    //==============================================================================
    z0 startPlaying()
    {
        player.setProcessor (processor.get());

       #if DrxPlugin_Enable_IAA && DRX_IOS
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
        {
            processor->setPlayHead (device->getAudioPlayHead());
            device->setMidiMessageCollector (&player.getMidiMessageCollector());
        }
       #endif
    }

    z0 stopPlaying()
    {
        player.setProcessor (nullptr);
    }

    //==============================================================================
    /** Shows an audio properties dialog box modally. */
    z0 showAudioSettingsDialog()
    {
        DialogWindow::LaunchOptions o;

        i32 maxNumInputs = 0, maxNumOutputs = 0;

        if (channelConfiguration.size() > 0)
        {
            auto& defaultConfig = channelConfiguration.getReference (0);

            maxNumInputs  = jmax (0, (i32) defaultConfig.numIns);
            maxNumOutputs = jmax (0, (i32) defaultConfig.numOuts);
        }

        if (auto* bus = processor->getBus (true, 0))
            maxNumInputs = jmax (0, bus->getDefaultLayout().size());

        if (auto* bus = processor->getBus (false, 0))
            maxNumOutputs = jmax (0, bus->getDefaultLayout().size());

        auto content = std::make_unique<SettingsComponent> (*this, deviceManager, maxNumInputs, maxNumOutputs);
        content->setSize (500, 550);
        content->setToRecommendedSize();

        o.content.setOwned (content.release());

        o.dialogTitle                   = TRANS ("Audio/MIDI Settings");
        o.dialogBackgroundColor        = o.content->getLookAndFeel().findColor (ResizableWindow::backgroundColorId);
        o.escapeKeyTriggersCloseButton  = true;
        o.useNativeTitleBar             = true;
        o.resizable                     = false;

        o.launchAsync();
    }

    z0 saveAudioDeviceState()
    {
        if (settings != nullptr)
        {
            auto xml = deviceManager.createStateXml();

            settings->setValue ("audioSetup", xml.get());

           #if ! (DRX_IOS || DRX_ANDROID)
            settings->setValue ("shouldMuteInput", (b8) shouldMuteInput.getValue());
           #endif
        }
    }

    z0 reloadAudioDeviceState (b8 enableAudioInput,
                                 const Txt& preferredDefaultDeviceName,
                                 const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions)
    {
        std::unique_ptr<XmlElement> savedState;

        if (settings != nullptr)
        {
            savedState = settings->getXmlValue ("audioSetup");

           #if ! (DRX_IOS || DRX_ANDROID)
            shouldMuteInput.setValue (settings->getBoolValue ("shouldMuteInput", true));
           #endif
        }

        auto inputChannels  = getNumInputChannels();
        auto outputChannels = getNumOutputChannels();

        if (inputChannels == 0 && outputChannels == 0 && processor->isMidiEffect())
        {
            // add a dummy output channel for MIDI effect plug-ins so they can receive audio callbacks
            outputChannels = 1;
        }

        deviceManager.initialise (enableAudioInput ? inputChannels : 0,
                                  outputChannels,
                                  savedState.get(),
                                  true,
                                  preferredDefaultDeviceName,
                                  preferredSetupOptions);
    }

    //==============================================================================
    z0 savePluginState()
    {
        if (settings != nullptr && processor != nullptr)
        {
            MemoryBlock data;
            processor->getStateInformation (data);

            settings->setValue ("filterState", data.toBase64Encoding());
        }
    }

    z0 reloadPluginState()
    {
        if (settings != nullptr)
        {
            MemoryBlock data;

            if (data.fromBase64Encoding (settings->getValue ("filterState")) && data.getSize() > 0)
                processor->setStateInformation (data.getData(), (i32) data.getSize());
        }
    }

    //==============================================================================
    z0 switchToHostApplication()
    {
       #if DRX_IOS
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
            device->switchApplication();
       #endif
    }

    b8 isInterAppAudioConnected()
    {
       #if DRX_IOS
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
            return device->isInterAppAudioConnected();
       #endif

        return false;
    }

    Image getIAAHostIcon ([[maybe_unused]] i32 size)
    {
       #if DRX_IOS && DrxPlugin_Enable_IAA
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
            return device->getIcon (size);
       #else
       #endif

        return {};
    }

    static StandalonePluginHolder* getInstance()
    {
        return currentInstance;
    }

    //==============================================================================
    OptionalScopedPointer<PropertySet> settings;
    std::unique_ptr<AudioProcessor> processor;
    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;
    Array<PluginInOuts> channelConfiguration;

    // avoid feedback loop by default
    b8 processorHasPotentialFeedbackLoop = true;
    std::atomic<b8> muteInput { true };
    Value shouldMuteInput;
    AudioBuffer<f32> emptyBuffer;
    b8 autoOpenMidiDevices;

    std::unique_ptr<AudioDeviceManager::AudioDeviceSetup> options;
    Array<MidiDeviceInfo> lastMidiDevices;

    std::unique_ptr<FileChooser> stateFileChooser;
    ScopedMessageBox messageBox;

private:
    inline static StandalonePluginHolder* currentInstance = nullptr;

    //==============================================================================
    z0 handleCreatePlugin()
    {
        processor = createPluginFilterOfType (AudioProcessor::wrapperType_Standalone);
        processor->disableNonMainBuses();
        processor->setRateAndBufferSizeDetails (44100, 512);

        processorHasPotentialFeedbackLoop = (getNumInputChannels() > 0 && getNumOutputChannels() > 0);
    }

    z0 handleDeletePlugin()
    {
        stopPlaying();
        processor = nullptr;
    }

    //==============================================================================
    /*  This class can be used to ensure that audio callbacks use buffers with a
        predictable maximum size.

        On some platforms (such as iOS 10), the expected buffer size reported in
        audioDeviceAboutToStart may be smaller than the blocks passed to
        audioDeviceIOCallbackWithContext. This can lead to out-of-bounds reads if the render
        callback depends on additional buffers which were initialised using the
        smaller size.

        As a workaround, this class will ensure that the render callback will
        only ever be called with a block with a length less than or equal to the
        expected block size.
    */
    class CallbackMaxSizeEnforcer  : public AudioIODeviceCallback
    {
    public:
        explicit CallbackMaxSizeEnforcer (AudioIODeviceCallback& callbackIn)
            : inner (callbackIn) {}

        z0 audioDeviceAboutToStart (AudioIODevice* device) override
        {
            maximumSize = device->getCurrentBufferSizeSamples();
            storedInputChannels .resize ((size_t) device->getActiveInputChannels() .countNumberOfSetBits());
            storedOutputChannels.resize ((size_t) device->getActiveOutputChannels().countNumberOfSetBits());

            inner.audioDeviceAboutToStart (device);
        }

        z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData,
                                               [[maybe_unused]] i32 numInputChannels,
                                               f32* const* outputChannelData,
                                               [[maybe_unused]] i32 numOutputChannels,
                                               i32 numSamples,
                                               const AudioIODeviceCallbackContext& context) override
        {
            jassert ((i32) storedInputChannels.size()  == numInputChannels);
            jassert ((i32) storedOutputChannels.size() == numOutputChannels);

            i32 position = 0;

            while (position < numSamples)
            {
                const auto blockLength = jmin (maximumSize, numSamples - position);

                initChannelPointers (inputChannelData,  storedInputChannels,  position);
                initChannelPointers (outputChannelData, storedOutputChannels, position);

                inner.audioDeviceIOCallbackWithContext (storedInputChannels.data(),
                                                        (i32) storedInputChannels.size(),
                                                        storedOutputChannels.data(),
                                                        (i32) storedOutputChannels.size(),
                                                        blockLength,
                                                        context);

                position += blockLength;
            }
        }

        z0 audioDeviceStopped() override
        {
            inner.audioDeviceStopped();
        }

    private:
        struct GetChannelWithOffset
        {
            i32 offset;

            template <typename Ptr>
            auto operator() (Ptr ptr) const noexcept -> Ptr { return ptr + offset; }
        };

        template <typename Ptr, typename Vector>
        z0 initChannelPointers (Ptr&& source, Vector&& target, i32 offset)
        {
            std::transform (source, source + target.size(), target.begin(), GetChannelWithOffset { offset });
        }

        AudioIODeviceCallback& inner;
        i32 maximumSize = 0;
        std::vector<const f32*> storedInputChannels;
        std::vector<f32*> storedOutputChannels;
    };

    CallbackMaxSizeEnforcer maxSizeEnforcer { *this };

    //==============================================================================
    class SettingsComponent : public Component
    {
    public:
        SettingsComponent (StandalonePluginHolder& pluginHolder,
                           AudioDeviceManager& deviceManagerToUse,
                           i32 maxAudioInputChannels,
                           i32 maxAudioOutputChannels)
            : owner (pluginHolder),
              deviceSelector (deviceManagerToUse,
                              0, maxAudioInputChannels,
                              0, maxAudioOutputChannels,
                              true,
                              (pluginHolder.processor.get() != nullptr && pluginHolder.processor->producesMidi()),
                              true, false),
              shouldMuteLabel  ("Feedback Loop:", "Feedback Loop:"),
              shouldMuteButton ("Mute audio input")
        {
            setOpaque (true);

            shouldMuteButton.setClickingTogglesState (true);
            shouldMuteButton.getToggleStateValue().referTo (owner.shouldMuteInput);

            addAndMakeVisible (deviceSelector);

            if (owner.getProcessorHasPotentialFeedbackLoop())
            {
                addAndMakeVisible (shouldMuteButton);
                addAndMakeVisible (shouldMuteLabel);

                shouldMuteLabel.attachToComponent (&shouldMuteButton, true);
            }
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
        }

        z0 resized() override
        {
            const ScopedValueSetter<b8> scope (isResizing, true);

            auto r = getLocalBounds();

            if (owner.getProcessorHasPotentialFeedbackLoop())
            {
                auto itemHeight = deviceSelector.getItemHeight();
                auto extra = r.removeFromTop (itemHeight);

                auto seperatorHeight = (itemHeight >> 1);
                shouldMuteButton.setBounds (Rectangle<i32> (extra.proportionOfWidth (0.35f), seperatorHeight,
                                                            extra.proportionOfWidth (0.60f), deviceSelector.getItemHeight()));

                r.removeFromTop (seperatorHeight);
            }

            deviceSelector.setBounds (r);
        }

        z0 childBoundsChanged (Component* childComp) override
        {
            if (! isResizing && childComp == &deviceSelector)
                setToRecommendedSize();
        }

        z0 setToRecommendedSize()
        {
            const auto extraHeight = [&]
            {
                if (! owner.getProcessorHasPotentialFeedbackLoop())
                    return 0;

                const auto itemHeight = deviceSelector.getItemHeight();
                const auto separatorHeight = (itemHeight >> 1);
                return itemHeight + separatorHeight;
            }();

            setSize (getWidth(), deviceSelector.getHeight() + extraHeight);
        }

    private:
        //==============================================================================
        StandalonePluginHolder& owner;
        AudioDeviceSelectorComponent deviceSelector;
        Label shouldMuteLabel;
        ToggleButton shouldMuteButton;
        b8 isResizing = false;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
    };

    //==============================================================================
    z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData,
                                           i32 numInputChannels,
                                           f32* const* outputChannelData,
                                           i32 numOutputChannels,
                                           i32 numSamples,
                                           const AudioIODeviceCallbackContext& context) override
    {
        if (muteInput)
        {
            emptyBuffer.clear();
            inputChannelData = emptyBuffer.getArrayOfReadPointers();
        }

        player.audioDeviceIOCallbackWithContext (inputChannelData,
                                                 numInputChannels,
                                                 outputChannelData,
                                                 numOutputChannels,
                                                 numSamples,
                                                 context);
    }

    z0 audioDeviceAboutToStart (AudioIODevice* device) override
    {
        emptyBuffer.setSize (device->getActiveInputChannels().countNumberOfSetBits(), device->getCurrentBufferSizeSamples());
        emptyBuffer.clear();

        player.audioDeviceAboutToStart (device);
        player.setMidiOutput (deviceManager.getDefaultMidiOutput());
    }

    z0 audioDeviceStopped() override
    {
        player.setMidiOutput (nullptr);
        player.audioDeviceStopped();
        emptyBuffer.setSize (0, 0);
    }

    //==============================================================================
    z0 setupAudioDevices (b8 enableAudioInput,
                            const Txt& preferredDefaultDeviceName,
                            const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions)
    {
        deviceManager.addAudioCallback (&maxSizeEnforcer);
        deviceManager.addMidiInputDeviceCallback ({}, &player);

        reloadAudioDeviceState (enableAudioInput, preferredDefaultDeviceName, preferredSetupOptions);
    }

    z0 shutDownAudioDevices()
    {
        saveAudioDeviceState();

        deviceManager.removeMidiInputDeviceCallback ({}, &player);
        deviceManager.removeAudioCallback (&maxSizeEnforcer);
    }

    z0 timerCallback() override
    {
        auto newMidiDevices = MidiInput::getAvailableDevices();

        if (newMidiDevices != lastMidiDevices)
        {
            for (auto& oldDevice : lastMidiDevices)
                if (! newMidiDevices.contains (oldDevice))
                    deviceManager.setMidiInputDeviceEnabled (oldDevice.identifier, false);

            for (auto& newDevice : newMidiDevices)
                if (! lastMidiDevices.contains (newDevice))
                    deviceManager.setMidiInputDeviceEnabled (newDevice.identifier, true);

            lastMidiDevices = newMidiDevices;
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandalonePluginHolder)
};

//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your DRXApplicationBase::initialise() method, and
    let it do its work. It will create your filter object using the same createPluginFilter() function
    that the other plugin wrappers use.

    @tags{Audio}
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  private Button::Listener
{
public:
    //==============================================================================
    typedef StandalonePluginHolder::PluginInOuts PluginInOuts;

    StandaloneFilterWindow (const Txt& title,
                            Color backgroundColor,
                            std::unique_ptr<StandalonePluginHolder> pluginHolderIn)
        : DocumentWindow (title, backgroundColor, DocumentWindow::minimiseButton | DocumentWindow::closeButton),
          pluginHolder (std::move (pluginHolderIn)),
          optionsButton ("Options")
    {
        setConstrainer (&decoratorConstrainer);

       #if DRX_IOS || DRX_ANDROID
        setTitleBarHeight (0);
       #else
        setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);

        Component::addAndMakeVisible (optionsButton);
        optionsButton.addListener (this);
        optionsButton.setTriggeredOnMouseDown (true);
       #endif

       #if DRX_IOS || DRX_ANDROID
        setFullScreen (true);
        updateContent();
       #else
        updateContent();

        const auto windowScreenBounds = [this]() -> Rectangle<i32>
        {
            const auto width = getWidth();
            const auto height = getHeight();

            const auto& displays = Desktop::getInstance().getDisplays();

            if (displays.displays.isEmpty())
                return { width, height };

            if (auto* props = pluginHolder->settings.get())
            {
                constexpr i32 defaultValue = -100;

                const auto x = props->getIntValue ("windowX", defaultValue);
                const auto y = props->getIntValue ("windowY", defaultValue);

                if (x != defaultValue && y != defaultValue)
                {
                    const auto screenLimits = displays.getDisplayForRect ({ x, y, width, height })->userArea;

                    return { jlimit (screenLimits.getX(), jmax (screenLimits.getX(), screenLimits.getRight()  - width),  x),
                             jlimit (screenLimits.getY(), jmax (screenLimits.getY(), screenLimits.getBottom() - height), y),
                             width, height };
                }
            }

            const auto displayArea = displays.getPrimaryDisplay()->userArea;

            return { displayArea.getCentreX() - width / 2,
                     displayArea.getCentreY() - height / 2,
                     width, height };
        }();

        setBoundsConstrained (windowScreenBounds);

        if (auto* processor = getAudioProcessor())
            if (auto* editor = processor->getActiveEditor())
                setResizable (editor->isResizable(), false);
       #endif
    }

    //==============================================================================
    /** Creates a window with a given title and colour.
        The settings object can be a PropertySet that the class should use to
        store its settings (it can also be null). If takeOwnershipOfSettings is
        true, then the settings object will be owned and deleted by this object.
    */
    StandaloneFilterWindow (const Txt& title,
                            Color backgroundColor,
                            PropertySet* settingsToUse,
                            b8 takeOwnershipOfSettings,
                            const Txt& preferredDefaultDeviceName = Txt(),
                            const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions = nullptr,
                            const Array<PluginInOuts>& constrainToConfiguration = {},
                           #if DRX_ANDROID || DRX_IOS
                            b8 autoOpenMidiDevices = true
                           #else
                            b8 autoOpenMidiDevices = false
                           #endif
                            )
        : StandaloneFilterWindow (title,
                                  backgroundColor,
                                  std::make_unique<StandalonePluginHolder> (settingsToUse,
                                                                            takeOwnershipOfSettings,
                                                                            preferredDefaultDeviceName,
                                                                            preferredSetupOptions,
                                                                            constrainToConfiguration,
                                                                            autoOpenMidiDevices))
    {
    }

    ~StandaloneFilterWindow() override
    {
       #if (! DRX_IOS) && (! DRX_ANDROID)
        if (auto* props = pluginHolder->settings.get())
        {
            props->setValue ("windowX", getX());
            props->setValue ("windowY", getY());
        }
       #endif

        pluginHolder->stopPlaying();
        clearContentComponent();
        pluginHolder = nullptr;
    }

    //==============================================================================
    AudioProcessor* getAudioProcessor() const noexcept      { return pluginHolder->processor.get(); }
    AudioDeviceManager& getDeviceManager() const noexcept   { return pluginHolder->deviceManager; }

    /** Deletes and re-creates the plugin, resetting it to its default state. */
    z0 resetToDefaultState()
    {
        pluginHolder->stopPlaying();
        clearContentComponent();
        pluginHolder->deletePlugin();

        if (auto* props = pluginHolder->settings.get())
            props->removeValue ("filterState");

        pluginHolder->createPlugin();
        updateContent();
        pluginHolder->startPlaying();
    }

    //==============================================================================
    z0 closeButtonPressed() override
    {
        pluginHolder->savePluginState();

        DRXApplicationBase::quit();
    }

    z0 handleMenuResult (i32 result)
    {
        switch (result)
        {
            case 1:  pluginHolder->showAudioSettingsDialog(); break;
            case 2:  pluginHolder->askUserToSaveState(); break;
            case 3:  pluginHolder->askUserToLoadState(); break;
            case 4:  resetToDefaultState(); break;
            default: break;
        }
    }

    static z0 menuCallback (i32 result, StandaloneFilterWindow* button)
    {
        if (button != nullptr && result != 0)
            button->handleMenuResult (result);
    }

    z0 resized() override
    {
        DocumentWindow::resized();
        optionsButton.setBounds (8, 6, 60, getTitleBarHeight() - 8);
    }

    virtual StandalonePluginHolder* getPluginHolder()    { return pluginHolder.get(); }

    std::unique_ptr<StandalonePluginHolder> pluginHolder;

private:
    z0 updateContent()
    {
        auto* content = new MainContentComponent (*this);
        decoratorConstrainer.setMainContentComponent (content);

       #if DRX_IOS || DRX_ANDROID
        constexpr auto resizeAutomatically = false;
       #else
        constexpr auto resizeAutomatically = true;
       #endif

        setContentOwned (content, resizeAutomatically);
    }

    z0 buttonClicked (Button*) override
    {
        PopupMenu m;
        m.addItem (1, TRANS ("Audio/MIDI Settings..."));
        m.addSeparator();
        m.addItem (2, TRANS ("Save current state..."));
        m.addItem (3, TRANS ("Load a saved state..."));
        m.addSeparator();
        m.addItem (4, TRANS ("Reset to default state"));

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (menuCallback, this));
    }

    //==============================================================================
    class MainContentComponent  : public Component,
                                  private Value::Listener,
                                  private Button::Listener,
                                  private ComponentListener
    {
    public:
        MainContentComponent (StandaloneFilterWindow& filterWindow)
            : owner (filterWindow), notification (this),
              editor (owner.getAudioProcessor()->hasEditor() ? owner.getAudioProcessor()->createEditorIfNeeded()
                                                             : new GenericAudioProcessorEditor (*owner.getAudioProcessor()))
        {
            inputMutedValue.referTo (owner.pluginHolder->getMuteInputValue());

            if (editor != nullptr)
            {
                editor->addComponentListener (this);
                handleMovedOrResized();

                addAndMakeVisible (editor.get());
            }

            addChildComponent (notification);

            if (owner.pluginHolder->getProcessorHasPotentialFeedbackLoop())
            {
                inputMutedValue.addListener (this);
                shouldShowNotification = inputMutedValue.getValue();
            }

            inputMutedChanged (shouldShowNotification);
        }

        ~MainContentComponent() override
        {
            if (editor != nullptr)
            {
                editor->removeComponentListener (this);
                owner.pluginHolder->processor->editorBeingDeleted (editor.get());
                editor = nullptr;
            }
        }

        z0 resized() override
        {
            handleResized();
        }

        ComponentBoundsConstrainer* getEditorConstrainer() const
        {
            if (auto* e = editor.get())
                return e->getConstrainer();

            return nullptr;
        }

        BorderSize<i32> computeBorder() const
        {
            const auto nativeFrame = [&]() -> BorderSize<i32>
            {
                if (auto* peer = owner.getPeer())
                    if (const auto frameSize = peer->getFrameSizeIfPresent())
                        return *frameSize;

                return {};
            }();

            return nativeFrame.addedTo (owner.getContentComponentBorder())
                              .addedTo (BorderSize<i32> { shouldShowNotification ? NotificationArea::height : 0, 0, 0, 0 });
        }

    private:
        //==============================================================================
        class NotificationArea : public Component
        {
        public:
            enum { height = 30 };

            NotificationArea (Button::Listener* settingsButtonListener)
                : notification ("notification", "Audio input is muted to avoid feedback loop"),
                 #if DRX_IOS || DRX_ANDROID
                  settingsButton ("Unmute Input")
                 #else
                  settingsButton ("Settings...")
                 #endif
            {
                setOpaque (true);

                notification.setColor (Label::textColorId, Colors::black);

                settingsButton.addListener (settingsButtonListener);

                addAndMakeVisible (notification);
                addAndMakeVisible (settingsButton);
            }

            z0 paint (Graphics& g) override
            {
                auto r = getLocalBounds();

                g.setColor (Colors::darkgoldenrod);
                g.fillRect (r.removeFromBottom (1));

                g.setColor (Colors::lightgoldenrodyellow);
                g.fillRect (r);
            }

            z0 resized() override
            {
                auto r = getLocalBounds().reduced (5);

                settingsButton.setBounds (r.removeFromRight (70));
                notification.setBounds (r);
            }
        private:
            Label notification;
            TextButton settingsButton;
        };

        //==============================================================================
        z0 inputMutedChanged (b8 newInputMutedValue)
        {
            shouldShowNotification = newInputMutedValue;
            notification.setVisible (shouldShowNotification);

           #if DRX_IOS || DRX_ANDROID
            handleResized();
           #else
            if (editor != nullptr)
            {
                i32k extraHeight = shouldShowNotification ? NotificationArea::height : 0;
                const auto rect = getSizeToContainEditor();
                setSize (rect.getWidth(), rect.getHeight() + extraHeight);
            }
           #endif
        }

        z0 valueChanged (Value& value) override     { inputMutedChanged (value.getValue()); }
        z0 buttonClicked (Button*) override
        {
           #if DRX_IOS || DRX_ANDROID
            owner.pluginHolder->getMuteInputValue().setValue (false);
           #else
            owner.pluginHolder->showAudioSettingsDialog();
           #endif
        }

        //==============================================================================
        z0 handleResized()
        {
            auto r = getLocalBounds();

            if (shouldShowNotification)
                notification.setBounds (r.removeFromTop (NotificationArea::height));

            if (editor != nullptr)
            {
                const auto newPos = r.getTopLeft().toFloat().transformedBy (editor->getTransform().inverted());

                if (preventResizingEditor)
                    editor->setTopLeftPosition (newPos.roundToInt());
                else
                    editor->setBoundsConstrained (editor->getLocalArea (this, r.toFloat()).withPosition (newPos).toNearestInt());
            }
        }

        z0 handleMovedOrResized()
        {
            const ScopedValueSetter<b8> scope (preventResizingEditor, true);

            if (editor != nullptr)
            {
                auto rect = getSizeToContainEditor();

                setSize (rect.getWidth(),
                         rect.getHeight() + (shouldShowNotification ? NotificationArea::height : 0));
            }
        }

        z0 componentMovedOrResized (Component&, b8, b8) override
        {
            handleMovedOrResized();
        }

        Rectangle<i32> getSizeToContainEditor() const
        {
            if (editor != nullptr)
                return getLocalArea (editor.get(), editor->getLocalBounds());

            return {};
        }

        //==============================================================================
        StandaloneFilterWindow& owner;
        NotificationArea notification;
        std::unique_ptr<AudioProcessorEditor> editor;
        Value inputMutedValue;
        b8 shouldShowNotification = false;
        b8 preventResizingEditor = false;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
    };

    /*  This custom constrainer checks with the AudioProcessorEditor (which might itself be
        constrained) to ensure that any size we choose for the standalone window will be suitable
        for the editor too.

        Without this constrainer, attempting to resize the standalone window may set bounds on the
        peer that are unsupported by the inner editor. In this scenario, the peer will be set to a
        'bad' size, then the inner editor will be resized. The editor will check the new bounds with
        its own constrainer, and may set itself to a more suitable size. After that, the resizable
        window will see that its content component has changed size, and set the bounds of the peer
        accordingly. The end result is that the peer is resized twice in a row to different sizes,
        which can appear glitchy/flickery to the user.
    */
    class DecoratorConstrainer : public BorderedComponentBoundsConstrainer
    {
    public:
        ComponentBoundsConstrainer* getWrappedConstrainer() const override
        {
            return contentComponent != nullptr ? contentComponent->getEditorConstrainer() : nullptr;
        }

        BorderSize<i32> getAdditionalBorder() const override
        {
            return contentComponent != nullptr ? contentComponent->computeBorder() : BorderSize<i32>{};
        }

        z0 setMainContentComponent (MainContentComponent* in) { contentComponent = in; }

    private:
        MainContentComponent* contentComponent = nullptr;
    };

    //==============================================================================
    TextButton optionsButton;
    DecoratorConstrainer decoratorConstrainer;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow)
};

} // namespace drx
