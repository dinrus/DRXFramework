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
#include <drx_audio_plugin_client/detail/drx_CheckSettingMacros.h>

#if DrxPlugin_Build_AUv3

#ifndef __OBJC2__
 #error AUv3 needs Objective-C 2 support (compile with 64-bit)
#endif

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1

#include <drx_audio_plugin_client/detail/drx_IncludeSystemHeaders.h>
#include <drx_audio_plugin_client/detail/drx_PluginUtilities.h>

#import <CoreAudioKit/CoreAudioKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

#include <drx_graphics/native/drx_CoreGraphicsHelpers_mac.h>
#include <drx_audio_basics/native/drx_CoreAudioLayouts_mac.h>
#include <drx_audio_basics/native/drx_CoreAudioTimeConversions_mac.h>
#include <drx_audio_basics/native/drx_AudioWorkgroup_mac.h>
#include <drx_audio_processors/format_types/drx_LegacyAudioParameter.cpp>
#include <drx_audio_processors/format_types/drx_AU_Shared.h>

#define DRX_VIEWCONTROLLER_OBJC_NAME(x) DRX_JOIN_MACRO (x, FactoryAUv3)

#if DRX_IOS
 #define DRX_IOS_MAC_VIEW  UIView
#else
 #define DRX_IOS_MAC_VIEW  NSView
#endif

#define DRX_AUDIOUNIT_OBJC_NAME(x) DRX_JOIN_MACRO (x, AUv3)

#include <future>

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfour-t8-constants")
inline constexpr auto pluginIsMidiEffect = DrxPlugin_AUMainType == kAudioUnitType_MIDIProcessor;
DRX_END_IGNORE_WARNINGS_GCC_LIKE

inline constexpr auto pluginProducesMidiOutput =
#if DrxPlugin_ProducesMidiOutput
        true;
#else
        pluginIsMidiEffect;
#endif

inline constexpr auto pluginWantsMidiInput =
#if DrxPlugin_WantsMidiInput
        true;
#else
        pluginIsMidiEffect;
#endif

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullability-completeness")

using namespace drx;

struct AudioProcessorHolder final : public ReferenceCountedObject
{
    AudioProcessorHolder() = default;
    explicit AudioProcessorHolder (std::unique_ptr<AudioProcessor> p) : processor (std::move (p)) {}
    AudioProcessor& operator*() noexcept        { return *processor; }
    AudioProcessor* operator->() noexcept       { return processor.get(); }
    AudioProcessor* get() noexcept              { return processor.get(); }

    struct ViewConfig
    {
        f64 width;
        f64 height;
        b8 hostHasMIDIController;
    };

    std::unique_ptr<ViewConfig> viewConfiguration;

    using Ptr = ReferenceCountedObjectPtr<AudioProcessorHolder>;

private:
    std::unique_ptr<AudioProcessor> processor;

    AudioProcessorHolder& operator= (AudioProcessor*) = delete;
    AudioProcessorHolder (AudioProcessorHolder&) = delete;
    AudioProcessorHolder& operator= (AudioProcessorHolder&) = delete;
};

#if ! DRX_AUDIOWORKGROUP_TYPES_AVAILABLE
 struct AudioUnitRenderContext;
 typedef z0 (^AURenderContextObserver) (const AudioUnitRenderContext*);
#endif

//==============================================================================
//=========================== The actual AudioUnit =============================
//==============================================================================
class DrxAudioUnitv3 final : public AudioProcessorListener,
                              public AudioPlayHead,
                              private AudioProcessorParameter::Listener
{
public:
    DrxAudioUnitv3 (const AudioProcessorHolder::Ptr& processor,
                     const AudioComponentDescription& descr,
                     AudioComponentInstantiationOptions options,
                     NSError** error)
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wobjc-method-access")
        : au ([getClass().createInstance() initWithComponentDescription: descr
                                                                options: options
                                                                  error: error
                                                              juceClass: this]),
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
          processorHolder (processor)
    {
        init();
    }

    DrxAudioUnitv3 (AUAudioUnit* audioUnit, AudioComponentDescription, AudioComponentInstantiationOptions, NSError**)
        : au (audioUnit),
          processorHolder (new AudioProcessorHolder (createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnitv3)))
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
        initialiseDrx_GUI();

        init();
    }

    ~DrxAudioUnitv3() override
    {
        auto& processor = getAudioProcessor();
        processor.removeListener (this);

        if (bypassParam != nullptr)
            bypassParam->removeListener (this);

        removeEditor (processor);
    }

    //==============================================================================
    z0 init()
    {
        inParameterChangedCallback = false;

        AudioProcessor& processor = getAudioProcessor();
        const AUAudioFrameCount maxFrames = [au maximumFramesToRender];

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = {DrxPlugin_PreferredChannelConfigurations};
        i32k numConfigs = sizeof (configs) / sizeof (short[2]);

        jassert (numConfigs > 0 && (configs[0][0] > 0 || configs[0][1] > 0));
        processor.setPlayConfigDetails (configs[0][0], configs[0][1], kDefaultSampleRate, static_cast<i32> (maxFrames));

        Array<AUChannelInfo> channelInfos;

        for (i32 i = 0; i < numConfigs; ++i)
        {
            AUChannelInfo channelInfo;

            channelInfo.inChannels  = configs[i][0];
            channelInfo.outChannels = configs[i][1];

            channelInfos.add (channelInfo);
        }
       #else
        Array<AUChannelInfo> channelInfos = AudioUnitHelpers::getAUChannelInfo (processor);
       #endif

        processor.setPlayHead (this);

        totalInChannels  = processor.getTotalNumInputChannels();
        totalOutChannels = processor.getTotalNumOutputChannels();

        {
            channelCapabilities.reset ([[NSMutableArray<NSNumber*> alloc] init]);

            for (i32 i = 0; i < channelInfos.size(); ++i)
            {
                AUChannelInfo& info = channelInfos.getReference (i);

                [channelCapabilities.get() addObject: [NSNumber numberWithInteger: info.inChannels]];
                [channelCapabilities.get() addObject: [NSNumber numberWithInteger: info.outChannels]];
            }
        }

        internalRenderBlock = CreateObjCBlock (this, &DrxAudioUnitv3::renderCallback);

       #if DRX_AUDIOWORKGROUP_TYPES_AVAILABLE
        renderContextObserver = ^(const AudioUnitRenderContext* context)
        {
            getAudioProcessor().audioWorkgroupContextChanged (makeRealAudioWorkgroup (context->workgroup));
        };
       #else
        renderContextObserver = ^(const AudioUnitRenderContext*) {};
       #endif

        processor.setRateAndBufferSizeDetails (kDefaultSampleRate, static_cast<i32> (maxFrames));
        processor.prepareToPlay (kDefaultSampleRate, static_cast<i32> (maxFrames));
        processor.addListener (this);

        addParameters();
        addPresets();

        addAudioUnitBusses (true);
        addAudioUnitBusses (false);
    }

    AudioProcessor& getAudioProcessor() const noexcept        { return **processorHolder; }

    //==============================================================================
    z0 reset()
    {
        midiMessages.clear();
        lastTimeStamp.mSampleTime = std::numeric_limits<Float64>::max();
        lastTimeStamp.mFlags = 0;
    }

    //==============================================================================
    AUAudioUnitPreset* getCurrentPreset() const
    {
        return factoryPresets.getAtIndex (getAudioProcessor().getCurrentProgram());
    }

    z0 setCurrentPreset (AUAudioUnitPreset* preset)
    {
        getAudioProcessor().setCurrentProgram (static_cast<i32> ([preset number]));
    }

    NSArray<AUAudioUnitPreset*>* getFactoryPresets() const
    {
        return factoryPresets.get();
    }

    NSDictionary<NSString*, id>* getFullState() const
    {
        NSMutableDictionary<NSString*, id>* retval = [[NSMutableDictionary<NSString*, id> alloc] init];

        {
            auto* superRetval = ObjCMsgSendSuper<AUAudioUnit, NSDictionary<NSString*, id>*> (au, @selector (fullState));

            if (superRetval != nullptr)
                [retval addEntriesFromDictionary:superRetval];
        }

        drx::MemoryBlock state;

       #if DRX_AU_WRAPPERS_SAVE_PROGRAM_STATES
        getAudioProcessor().getCurrentProgramStateInformation (state);
       #else
        getAudioProcessor().getStateInformation (state);
       #endif

        if (state.getSize() > 0)
        {
            [retval setObject: [[NSData alloc] initWithBytes: state.getData() length: state.getSize()]
                       forKey: @DRX_STATE_DICTIONARY_KEY];
        }

        return [retval autorelease];
    }

    z0 setFullState (NSDictionary<NSString*, id>* state)
    {
        if (state == nullptr)
            return;

        NSObject* obj = [state objectForKey: @DRX_STATE_DICTIONARY_KEY];

        if (obj == nullptr || ! [obj isKindOfClass: [NSData class]])
            return;

        auto* data = reinterpret_cast<NSData*> (obj);
        const auto numBytes = static_cast<i32> ([data length]);

        if (numBytes <= 0)
            return;

        auto* rawBytes = reinterpret_cast<const drx::u8* const> ([data bytes]);

        ScopedKeyChange scope (au, @"allParameterValues");

       #if DRX_AU_WRAPPERS_SAVE_PROGRAM_STATES
        getAudioProcessor().setCurrentProgramStateInformation (rawBytes, numBytes);
       #else
        getAudioProcessor().setStateInformation (rawBytes, numBytes);
       #endif
    }

    AUParameterTree* getParameterTree() const
    {
        return paramTree.get();
    }

    NSArray<NSNumber*>* parametersForOverviewWithCount (i32 count) const
    {
        auto* retval = [[[NSMutableArray<NSNumber*> alloc] init] autorelease];

        for (const auto& address : addressForIndex)
        {
            if (static_cast<size_t> (count) <= [retval count])
                break;

            [retval addObject: [NSNumber numberWithUnsignedLongLong: address]];
        }

        return retval;
    }

    //==============================================================================
    NSTimeInterval getLatency() const
    {
        auto& p = getAudioProcessor();
        return p.getLatencySamples() / p.getSampleRate();
    }

    NSTimeInterval getTailTime() const
    {
        return getAudioProcessor().getTailLengthSeconds();
    }

    //==============================================================================
    AUAudioUnitBusArray* getInputBusses()                const { return inputBusses.get();  }
    AUAudioUnitBusArray* getOutputBusses()               const { return outputBusses.get(); }
    NSArray<NSNumber*>* getChannelCapabilities()         const { return channelCapabilities.get(); }

    b8 shouldChangeToFormat (AVAudioFormat* format, AUAudioUnitBus* auBus)
    {
        if (allocated)
            return false;

        const auto isInput = ([auBus busType] == AUAudioUnitBusTypeInput);
        const auto busIdx = static_cast<i32> ([auBus index]);
        const auto newNumChannels = static_cast<i32> ([format channelCount]);

        AudioProcessor& processor = getAudioProcessor();

        if ([[maybe_unused]] AudioProcessor::Bus* bus = processor.getBus (isInput, busIdx))
        {
          #ifdef DrxPlugin_PreferredChannelConfigurations
            short configs[][2] = {DrxPlugin_PreferredChannelConfigurations};

            if (! AudioUnitHelpers::isLayoutSupported (processor, isInput, busIdx, newNumChannels, configs))
                return false;
          #else
            const AVAudioChannelLayout* layout    = [format channelLayout];
            const AudioChannelLayoutTag layoutTag = (layout != nullptr ? [layout layoutTag] : 0);

            if (layoutTag != 0)
            {
                AudioChannelSet newLayout = CoreAudioLayouts::fromCoreAudio (layoutTag);

                if (newLayout.size() != newNumChannels)
                    return false;

                if (! bus->isLayoutSupported (newLayout))
                    return false;
            }
            else
            {
                if (! bus->isNumberOfChannelsSupported (newNumChannels))
                    return false;
            }
           #endif

            return true;
        }

        return false;
    }

    //==============================================================================
    i32 getVirtualMIDICableCount() const
    {
        return pluginWantsMidiInput;
    }

    b8 getSupportsMPE() const
    {
        return getAudioProcessor().supportsMPE();
    }

    NSArray<NSString*>* getMIDIOutputNames() const
    {
        if constexpr (pluginProducesMidiOutput)
            return @[@"MIDI Out"];

        return @[];
    }

    //==============================================================================
    AUInternalRenderBlock   getInternalRenderBlock()      const { return internalRenderBlock; }
    AURenderContextObserver getInternalContextObserver()  const { return renderContextObserver; }

    b8 getRenderingOffline()                            const { return getAudioProcessor().isNonRealtime(); }
    z0 setRenderingOffline (b8 offline)
    {
        auto& processor = getAudioProcessor();
        auto isCurrentlyNonRealtime = processor.isNonRealtime();

        if (isCurrentlyNonRealtime != offline)
        {
            ScopedLock callbackLock (processor.getCallbackLock());

            processor.setNonRealtime (offline);
            processor.prepareToPlay (processor.getSampleRate(), processor.getBlockSize());
        }
    }

    b8 getShouldBypassEffect() const
    {
        if (bypassParam != nullptr)
            return (bypassParam->getValue() != 0.0f);

        return (ObjCMsgSendSuper<AUAudioUnit, BOOL> (au, @selector (shouldBypassEffect)) == YES);
    }

    z0 setShouldBypassEffect (b8 shouldBypass)
    {
        if (bypassParam != nullptr)
            bypassParam->setValue (shouldBypass ? 1.0f : 0.0f);

        ObjCMsgSendSuper<AUAudioUnit, z0> (au, @selector (setShouldBypassEffect:), shouldBypass ? YES : NO);
    }

    //==============================================================================
    NSString* getContextName() const                  { return juceStringToNS (contextName); }
    z0 setContextName (NSString* str)
    {
        if (str != nullptr)
        {
            AudioProcessor::TrackProperties props;
            props.name = std::make_optional (nsStringToDrx (str));

            getAudioProcessor().updateTrackProperties (props);
        }
    }

    //==============================================================================
    b8 allocateRenderResourcesAndReturnError (NSError **outError)
    {
        allocated = false;
        AudioProcessor& processor = getAudioProcessor();
        const AUAudioFrameCount maxFrames = [au maximumFramesToRender];

        if (ObjCMsgSendSuper<AUAudioUnit, BOOL, NSError**> (au, @selector (allocateRenderResourcesAndReturnError:), outError) == NO)
            return false;

        if (outError != nullptr)
            *outError = nullptr;

        AudioProcessor::BusesLayout layouts;
        for (i32 dir = 0; dir < 2; ++dir)
        {
            const b8 isInput = (dir == 0);
            i32k n = AudioUnitHelpers::getBusCountForWrapper (processor, isInput);
            Array<AudioChannelSet>& channelSets = (isInput ? layouts.inputBuses : layouts.outputBuses);

            AUAudioUnitBusArray* auBuses = (isInput ? [au inputBusses] : [au outputBusses]);
            jassert ([auBuses count] == static_cast<NSUInteger> (n));

            for (i32 busIdx = 0; busIdx < n; ++busIdx)
            {
                if (AudioProcessor::Bus* bus = processor.getBus (isInput, busIdx))
                {
                    AVAudioFormat* format = [[auBuses objectAtIndexedSubscript:static_cast<NSUInteger> (busIdx)] format];

                    AudioChannelSet newLayout;
                    const AVAudioChannelLayout* layout    = [format channelLayout];
                    const AudioChannelLayoutTag layoutTag = (layout != nullptr ? [layout layoutTag] : 0);

                    if (layoutTag != 0)
                        newLayout = CoreAudioLayouts::fromCoreAudio (layoutTag);
                    else
                        newLayout = bus->supportedLayoutWithChannels (static_cast<i32> ([format channelCount]));

                    if (newLayout.isDisabled())
                        return false;

                    channelSets.add (newLayout);
                }
            }
        }

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = {DrxPlugin_PreferredChannelConfigurations};

        if (! AudioProcessor::containsLayout (layouts, configs))
        {
            if (outError != nullptr)
                *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:kAudioUnitErr_FormatNotSupported userInfo:nullptr];

            return false;
        }
       #endif

        if (! AudioUnitHelpers::setBusesLayout (&getAudioProcessor(), layouts))
        {
            if (outError != nullptr)
                *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:kAudioUnitErr_FormatNotSupported userInfo:nullptr];

            return false;
        }

        totalInChannels  = processor.getTotalNumInputChannels();
        totalOutChannels = processor.getTotalNumOutputChannels();

        allocateBusBuffer (true);
        allocateBusBuffer (false);

        mapper.alloc (processor);

        audioBuffer.prepare (AudioUnitHelpers::getBusesLayout (&processor), static_cast<i32> (maxFrames));

        auto sampleRate = [&]
        {
            for (auto* buffer : { inputBusses.get(), outputBusses.get() })
                if ([buffer count] > 0)
                    return [[[buffer objectAtIndexedSubscript: 0] format] sampleRate];

            return 44100.0;
        }();

        processor.setRateAndBufferSizeDetails (sampleRate, static_cast<i32> (maxFrames));
        processor.prepareToPlay (sampleRate, static_cast<i32> (maxFrames));

        midiMessages.ensureSize (2048);
        midiMessages.clear();

        hostMusicalContextCallback = [au musicalContextBlock];
        hostTransportStateCallback = [au transportStateBlock];

       #if DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED
        if (@available (macOS 12, iOS 15, *))
            eventListOutput.setBlock ([au MIDIOutputEventListBlock]);
       #endif

        if (@available (macOS 10.13, *))
            midiOutputEventBlock = [au MIDIOutputEventBlock];

        reset();
        allocated = true;

        return true;
    }

    z0 deallocateRenderResources()
    {
        allocated = false;
        midiOutputEventBlock = nullptr;

        hostMusicalContextCallback = nullptr;
        hostTransportStateCallback = nullptr;

        getAudioProcessor().releaseResources();
        audioBuffer.release();

        inBusBuffers. clear();
        outBusBuffers.clear();

        mapper.release();

        ObjCMsgSendSuper<AUAudioUnit, z0> (au, @selector (deallocateRenderResources));
    }

    //==============================================================================
    struct ScopedKeyChange
    {
        ScopedKeyChange (AUAudioUnit* a, NSString* k)
            : au (a), key (k)
        {
            [au willChangeValueForKey: key];
        }

        ~ScopedKeyChange()
        {
            [au didChangeValueForKey: key];
        }

        AUAudioUnit* au;
        NSString* key;
    };

    //==============================================================================
    z0 audioProcessorChanged ([[maybe_unused]] AudioProcessor* processor, const ChangeDetails& details) override
    {
        if (details.programChanged)
        {
            {
                ScopedKeyChange scope (au, @"allParameterValues");
                addPresets();
            }

            {
                ScopedKeyChange scope (au, @"currentPreset");
            }
        }

        if (details.latencyChanged)
        {
            ScopedKeyChange scope (au, @"latency");
        }

        if (details.parameterInfoChanged)
        {
            ScopedKeyChange scope (au, @"parameterTree");
            auto nodes = createParameterNodes<NodeArrayResult> (processor->getParameterTree());
            installNewParameterTree (std::move (nodes.nodeArray));
        }
    }

    z0 sendParameterEvent (i32 idx, const f32* newValue, AUParameterAutomationEventType type)
    {
        if (inParameterChangedCallback.get())
        {
            inParameterChangedCallback = false;
            return;
        }

        if (auto* juceParam = juceParameters.getParamForIndex (idx))
        {
            if (auto* param = [paramTree.get() parameterWithAddress: getAUParameterAddressForIndex (idx)])
            {
                const auto value = (newValue != nullptr ? *newValue : juceParam->getValue()) * getMaximumParameterValue (*juceParam);

                if (@available (macOS 10.12, *))
                {
                    [param setValue: value
                         originator: editorObserverToken.get()
                         atHostTime: lastTimeStamp.mHostTime
                          eventType: type];
                }
                else if (type == AUParameterAutomationEventTypeValue)
                {
                    [param setValue: value originator: editorObserverToken.get()];
                }
            }
        }
    }

    z0 audioProcessorParameterChanged (AudioProcessor*, i32 idx, f32 newValue) override
    {
        sendParameterEvent (idx, &newValue, AUParameterAutomationEventTypeValue);
    }

    z0 audioProcessorParameterChangeGestureBegin (AudioProcessor*, i32 idx) override
    {
        sendParameterEvent (idx, nullptr, AUParameterAutomationEventTypeTouch);
    }

    z0 audioProcessorParameterChangeGestureEnd (AudioProcessor*, i32 idx) override
    {
        sendParameterEvent (idx, nullptr, AUParameterAutomationEventTypeRelease);
    }

    //==============================================================================
    Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        info.setTimeInSamples ((z64) (lastTimeStamp.mSampleTime + 0.5));
        info.setTimeInSeconds (*info.getTimeInSamples() / getAudioProcessor().getSampleRate());

        info.setFrameRate ([this]
        {
            switch (lastTimeStamp.mSMPTETime.mType)
            {
                case kSMPTETimeType2398:        return FrameRate().withBaseRate (24).withPullDown();
                case kSMPTETimeType24:          return FrameRate().withBaseRate (24);
                case kSMPTETimeType25:          return FrameRate().withBaseRate (25);
                case kSMPTETimeType30Drop:      return FrameRate().withBaseRate (30).withDrop();
                case kSMPTETimeType30:          return FrameRate().withBaseRate (30);
                case kSMPTETimeType2997:        return FrameRate().withBaseRate (30).withPullDown();
                case kSMPTETimeType2997Drop:    return FrameRate().withBaseRate (30).withPullDown().withDrop();
                case kSMPTETimeType60:          return FrameRate().withBaseRate (60);
                case kSMPTETimeType60Drop:      return FrameRate().withBaseRate (60).withDrop();
                case kSMPTETimeType5994:        return FrameRate().withBaseRate (60).withPullDown();
                case kSMPTETimeType5994Drop:    return FrameRate().withBaseRate (60).withPullDown().withDrop();
                case kSMPTETimeType50:          return FrameRate().withBaseRate (50);
                default:                        break;
            }

            return FrameRate();
        }());

        f64 num;
        NSInteger den;
        NSInteger outDeltaSampleOffsetToNextBeat;
        f64 outCurrentMeasureDownBeat, bpm;
        f64 ppqPosition;

        if (hostMusicalContextCallback != nullptr)
        {
            AUHostMusicalContextBlock musicalContextCallback = hostMusicalContextCallback;

            if (musicalContextCallback (&bpm, &num, &den, &ppqPosition, &outDeltaSampleOffsetToNextBeat, &outCurrentMeasureDownBeat))
            {
                info.setTimeSignature (TimeSignature { (i32) num, (i32) den });
                info.setPpqPositionOfLastBarStart (outCurrentMeasureDownBeat);
                info.setBpm (bpm);
                info.setPpqPosition (ppqPosition);
            }
        }

        f64 outCurrentSampleInTimeLine = 0, outCycleStartBeat = 0, outCycleEndBeat = 0;
        AUHostTransportStateFlags flags;

        if (hostTransportStateCallback != nullptr)
        {
            AUHostTransportStateBlock transportStateCallback = hostTransportStateCallback;

            if (transportStateCallback (&flags, &outCurrentSampleInTimeLine, &outCycleStartBeat, &outCycleEndBeat))
            {
                info.setTimeInSamples  ((z64) (outCurrentSampleInTimeLine + 0.5));
                info.setTimeInSeconds  (*info.getTimeInSamples() / getAudioProcessor().getSampleRate());
                info.setIsPlaying      ((flags & AUHostTransportStateMoving) != 0);
                info.setIsLooping      ((flags & AUHostTransportStateCycling) != 0);
                info.setIsRecording    ((flags & AUHostTransportStateRecording) != 0);
                info.setLoopPoints     (LoopPoints { outCycleStartBeat, outCycleEndBeat });
            }
        }

        if ((lastTimeStamp.mFlags & kAudioTimeStampHostTimeValid) != 0)
            info.setHostTimeNs (timeConversions.hostTimeToNanos (lastTimeStamp.mHostTime));

        return info;
    }

    //==============================================================================
    static z0 removeEditor (AudioProcessor& processor)
    {
        ScopedLock editorLock (processor.getCallbackLock());

        if (AudioProcessorEditor* editor = processor.getActiveEditor())
        {
            processor.editorBeingDeleted (editor);
            delete editor;
        }
    }

    AUAudioUnit* getAudioUnit() const { return au; }

private:
    struct Class final : public ObjCClass<AUAudioUnit>
    {
        Class()
            : ObjCClass ("AUAudioUnit_")
        {
            addIvar<DrxAudioUnitv3*> ("cppObject");

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (initWithComponentDescription:options:error:juceClass:), [] (id _self,
                                                                                              SEL,
                                                                                              AudioComponentDescription descr,
                                                                                              AudioComponentInstantiationOptions options,
                                                                                              NSError** error,
                                                                                              DrxAudioUnitv3* juceAU)
            {
                AUAudioUnit* self = _self;

                self = ObjCMsgSendSuper<AUAudioUnit, AUAudioUnit*, AudioComponentDescription,
                                        AudioComponentInstantiationOptions, NSError**> (self, @selector (initWithComponentDescription:options:error:), descr, options, error);

                setThis (self, juceAU);
                return self;
            });

            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            addMethod (@selector (initWithComponentDescription:options:error:), [] (id _self,
                                                                                    SEL,
                                                                                    AudioComponentDescription descr,
                                                                                    AudioComponentInstantiationOptions options,
                                                                                    NSError** error)
            {
                AUAudioUnit* self = _self;

                self = ObjCMsgSendSuper<AUAudioUnit, AUAudioUnit*, AudioComponentDescription,
                                        AudioComponentInstantiationOptions, NSError**> (self, @selector (initWithComponentDescription:options:error:), descr, options, error);

                auto* juceAU = DrxAudioUnitv3::create (self, descr, options, error);

                setThis (self, juceAU);
                return self;
            });

            addMethod (@selector (dealloc), [] (id self, SEL)
            {
                if (! MessageManager::getInstance()->isThisTheMessageThread())
                {
                    WaitableEvent deletionEvent;

                    struct AUDeleter final : public CallbackMessage
                    {
                        AUDeleter (id selfToDelete, WaitableEvent& event)
                            : parentSelf (selfToDelete), parentDeletionEvent (event)
                        {
                        }

                        z0 messageCallback() override
                        {
                            delete _this (parentSelf);
                            parentDeletionEvent.signal();
                        }

                        id parentSelf;
                        WaitableEvent& parentDeletionEvent;
                    };

                    (new AUDeleter (self, deletionEvent))->post();
                    deletionEvent.wait (-1);
                }
                else
                {
                    delete _this (self);
                }
            });

            //==============================================================================
            addMethod (@selector (reset),                                   [] (id self, SEL)                                                   { return _this (self)->reset(); });

            //==============================================================================
            addMethod (@selector (currentPreset),                           [] (id self, SEL)                                                   { return _this (self)->getCurrentPreset(); });
            addMethod (@selector (setCurrentPreset:),                       [] (id self, SEL, AUAudioUnitPreset* preset)                        { return _this (self)->setCurrentPreset (preset); });
            addMethod (@selector (factoryPresets),                          [] (id self, SEL)                                                   { return _this (self)->getFactoryPresets(); });
            addMethod (@selector (fullState),                               [] (id self, SEL)                                                   { return _this (self)->getFullState(); });
            addMethod (@selector (setFullState:),                           [] (id self, SEL, NSDictionary<NSString *, id>* state)              { return _this (self)->setFullState (state); });
            addMethod (@selector (parameterTree),                           [] (id self, SEL)                                                   { return _this (self)->getParameterTree(); });
            addMethod (@selector (parametersForOverviewWithCount:),         [] (id self, SEL, NSInteger count)                                  { return _this (self)->parametersForOverviewWithCount (static_cast<i32> (count)); });

            //==============================================================================
            addMethod (@selector (latency),                                 [] (id self, SEL)                                                   { return _this (self)->getLatency(); });
            addMethod (@selector (tailTime),                                [] (id self, SEL)                                                   { return _this (self)->getTailTime(); });

            //==============================================================================
            addMethod (@selector (inputBusses),                             [] (id self, SEL)                                                   { return _this (self)->getInputBusses(); });
            addMethod (@selector (outputBusses),                            [] (id self, SEL)                                                   { return _this (self)->getOutputBusses(); });
            addMethod (@selector (channelCapabilities),                     [] (id self, SEL)                                                   { return _this (self)->getChannelCapabilities(); });
            addMethod (@selector (shouldChangeToFormat:forBus:),            [] (id self, SEL, AVAudioFormat* format, AUAudioUnitBus* bus)       { return _this (self)->shouldChangeToFormat (format, bus) ? YES : NO; });

            //==============================================================================
            addMethod (@selector (virtualMIDICableCount),                   [] (id self, SEL)                                                   { return _this (self)->getVirtualMIDICableCount(); });

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (supportsMPE),                             [] (id self, SEL)                                                   { return _this (self)->getSupportsMPE() ? YES : NO; });
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            if (@available (macOS 10.13, *))
                addMethod (@selector (MIDIOutputNames),                     [] (id self, SEL)                                                   { return _this (self)->getMIDIOutputNames(); });

            //==============================================================================
            addMethod (@selector (internalRenderBlock),                     [] (id self, SEL)                                                   { return _this (self)->getInternalRenderBlock(); });
            addMethod (@selector (canProcessInPlace),                       [] (id,      SEL)                                                   { return NO; });
            addMethod (@selector (isRenderingOffline),                      [] (id self, SEL)                                                   { return _this (self)->getRenderingOffline() ? YES : NO; });
            addMethod (@selector (setRenderingOffline:),                    [] (id self, SEL, BOOL renderingOffline)                            { return _this (self)->setRenderingOffline (renderingOffline); });
            addMethod (@selector (shouldBypassEffect),                      [] (id self, SEL)                                                   { return _this (self)->getShouldBypassEffect() ? YES : NO; });
            addMethod (@selector (setShouldBypassEffect:),                  [] (id self, SEL, BOOL shouldBypass)                                { return _this (self)->setShouldBypassEffect (shouldBypass); });
            addMethod (@selector (allocateRenderResourcesAndReturnError:),  [] (id self, SEL, NSError** error)                                  { return _this (self)->allocateRenderResourcesAndReturnError (error) ? YES : NO; });
            addMethod (@selector (deallocateRenderResources),               [] (id self, SEL)                                                   { return _this (self)->deallocateRenderResources(); });
            addMethod (@selector (renderResourcesAllocated),                [] (id self, SEL)                                                   { return _this (self)->allocated; });

            //==============================================================================
            addMethod (@selector (contextName),                             [] (id self, SEL)                                                   { return _this (self)->getContextName(); });
            addMethod (@selector (setContextName:),                         [] (id self, SEL, NSString* str)                                    { return _this (self)->setContextName (str); });

           #if DRX_AUDIOWORKGROUP_TYPES_AVAILABLE
            addMethod (@selector (renderContextObserver),                   [] (id self, SEL)                                                   { return _this (self)->getInternalContextObserver(); });
           #endif

            //==============================================================================
            if (@available (macOS 10.13, *))
            {
                addMethod (@selector (supportedViewConfigurations:), [] (id self, SEL, NSArray<AUAudioUnitViewConfiguration*>* configs)
                {
                    auto supportedViewIndices = [[NSMutableIndexSet alloc] init];
                    auto n = [configs count];

                    if (auto* editor = _this (self)->getAudioProcessor().createEditorIfNeeded())
                    {
                        // If you hit this assertion then your plug-in's editor is reporting that it doesn't support
                        // any host MIDI controller configurations!
                        jassert (editor->supportsHostMIDIControllerPresence (true) || editor->supportsHostMIDIControllerPresence (false));

                        for (auto i = 0u; i < n; ++i)
                        {
                            if (auto viewConfiguration = [configs objectAtIndex: i])
                            {
                                if (editor->supportsHostMIDIControllerPresence ([viewConfiguration hostHasController] == YES))
                                {
                                    auto* constrainer = editor->getConstrainer();
                                    auto height = (i32) [viewConfiguration height];
                                    auto width  = (i32) [viewConfiguration width];

                                    const auto maxLimits = std::numeric_limits<i32>::max() / 2;
                                    const Rectangle<i32> requestedBounds { width, height };
                                    auto modifiedBounds = requestedBounds;
                                    constrainer->checkBounds (modifiedBounds, editor->getBounds().withZeroOrigin(), { maxLimits, maxLimits }, false, false, false, false);

                                    if (modifiedBounds == requestedBounds)
                                        [supportedViewIndices addIndex: i];
                                }
                            }
                        }
                    }

                    return [supportedViewIndices autorelease];
                });

                addMethod (@selector (selectViewConfiguration:), [] (id self, SEL, AUAudioUnitViewConfiguration* config)
                {
                    _this (self)->processorHolder->viewConfiguration.reset (new AudioProcessorHolder::ViewConfig { [config width], [config height], [config hostHasController] == YES });
                });
            }

            registerClass();
        }

        //==============================================================================
        static DrxAudioUnitv3* _this (id self)                     { return getIvar<DrxAudioUnitv3*>     (self, "cppObject"); }
        static z0 setThis (id self, DrxAudioUnitv3* cpp)         { object_setInstanceVariable           (self, "cppObject", cpp); }
    };

    static DrxAudioUnitv3* create (AUAudioUnit* audioUnit, AudioComponentDescription descr, AudioComponentInstantiationOptions options, NSError** error)
    {
        return new DrxAudioUnitv3 (audioUnit, descr, options, error);
    }

    //==============================================================================
    static Class& getClass()
    {
        static Class result;
        return result;
    }

    //==============================================================================
    struct BusBuffer
    {
        BusBuffer (AUAudioUnitBus* bus, i32 maxFramesPerBuffer)
            : auBus (bus),
              maxFrames (maxFramesPerBuffer),
              numberOfChannels (static_cast<i32> ([[auBus format] channelCount])),
              isInterleaved ([[auBus format] isInterleaved])
        {
            alloc();
        }

        //==============================================================================
        z0 alloc()
        {
            i32k numBuffers = isInterleaved ? 1 : numberOfChannels;
            i32 bytes = static_cast<i32> (sizeof (AudioBufferList))
                          + ((numBuffers - 1) * static_cast<i32> (sizeof (::AudioBuffer)));
            jassert (bytes > 0);

            bufferListStorage.calloc (static_cast<size_t> (bytes));
            bufferList = reinterpret_cast<AudioBufferList*> (bufferListStorage.getData());

            i32k bufferChannels = isInterleaved ? numberOfChannels : 1;
            scratchBuffer.setSize (numBuffers, bufferChannels * maxFrames);
        }

        z0 dealloc()
        {
            bufferList = nullptr;
            bufferListStorage.free();
            scratchBuffer.setSize (0, 0);
        }

        //==============================================================================
        i32 numChannels() const noexcept                { return numberOfChannels; }
        b8 interleaved() const noexcept               { return isInterleaved; }
        AudioBufferList* get() const noexcept           { return bufferList; }

        //==============================================================================
        z0 prepare (UInt32 nFrames, const AudioBufferList* other = nullptr) noexcept
        {
            i32k numBuffers = isInterleaved ? 1 : numberOfChannels;
            const b8 isCompatible = isCompatibleWith (other);

            bufferList->mNumberBuffers = static_cast<UInt32> (numBuffers);

            for (i32 i = 0; i < numBuffers; ++i)
            {
                const UInt32 bufferChannels = static_cast<UInt32> (isInterleaved ? numberOfChannels : 1);
                bufferList->mBuffers[i].mNumberChannels = bufferChannels;
                bufferList->mBuffers[i].mData = (isCompatible ? other->mBuffers[i].mData
                                                              : scratchBuffer.getWritePointer (i));
                bufferList->mBuffers[i].mDataByteSize = nFrames * bufferChannels * sizeof (f32);
            }
        }

        //==============================================================================
        b8 isCompatibleWith (const AudioBufferList* other) const noexcept
        {
            if (other == nullptr)
                return false;

            if (other->mNumberBuffers > 0)
            {
                const b8 otherInterleaved = AudioUnitHelpers::isAudioBufferInterleaved (*other);
                i32k otherChannels = static_cast<i32> (otherInterleaved ? other->mBuffers[0].mNumberChannels
                                                                             : other->mNumberBuffers);

                return otherInterleaved == isInterleaved
                    && numberOfChannels == otherChannels;
            }

            return numberOfChannels == 0;
        }

    private:
        AUAudioUnitBus* auBus;
        HeapBlock<t8> bufferListStorage;
        AudioBufferList* bufferList = nullptr;
        i32 maxFrames, numberOfChannels;
        b8 isInterleaved;
        drx::AudioBuffer<f32> scratchBuffer;
    };

    class FactoryPresets
    {
    public:
        using Presets = NSUniquePtr<NSMutableArray<AUAudioUnitPreset*>>;

        z0 set (Presets newPresets)
        {
            std::lock_guard<std::mutex> lock (mutex);
            std::swap (presets, newPresets);
        }

        NSArray* get() const
        {
            std::lock_guard<std::mutex> lock (mutex);
            return presets.get();
        }

        AUAudioUnitPreset* getAtIndex (i32 index) const
        {
            std::lock_guard<std::mutex> lock (mutex);

            if (index < (i32) [presets.get() count])
                return [presets.get() objectAtIndex: (u32) index];

            return nullptr;
        }

    private:
        Presets presets;
        mutable std::mutex mutex;
    };

    //==============================================================================
    z0 addAudioUnitBusses (b8 isInput)
    {
        NSUniquePtr<NSMutableArray<AUAudioUnitBus*>> array ([[NSMutableArray<AUAudioUnitBus*> alloc] init]);
        AudioProcessor& processor = getAudioProcessor();
        const auto numWrapperBuses = AudioUnitHelpers::getBusCountForWrapper (processor, isInput);
        const auto numProcessorBuses = AudioUnitHelpers::getBusCount (processor, isInput);

        for (i32 i = 0; i < numWrapperBuses; ++i)
        {
            using AVAudioFormatPtr = NSUniquePtr<AVAudioFormat>;

            const auto audioFormat = [&]
            {
                const auto defaultLayout = i < numProcessorBuses ? processor.getBus (isInput, i)->getLastEnabledLayout()
                                                                 : AudioChannelSet::stereo();
                NSUniquePtr<AVAudioChannelLayout> layout { [[AVAudioChannelLayout alloc] initWithLayoutTag: CoreAudioLayouts::toCoreAudio (defaultLayout)] };

                if (AVAudioFormatPtr format { [[AVAudioFormat alloc] initStandardFormatWithSampleRate: kDefaultSampleRate
                                                                                        channelLayout: layout.get()] })
                    return format;

                // According to the docs, this will fail if the number of channels is greater than 2.
                if (AVAudioFormatPtr format { [[AVAudioFormat alloc] initStandardFormatWithSampleRate: kDefaultSampleRate
                                                                                             channels: static_cast<AVAudioChannelCount> (defaultLayout.size())] })
                    return format;

                jassertfalse;
                return AVAudioFormatPtr{};
            }();

            using AUAudioUnitBusPtr = NSUniquePtr<AUAudioUnitBus>;

            const auto audioUnitBus = [&]
            {
                if (audioFormat == nullptr)
                {
                    jassertfalse;
                    return AUAudioUnitBusPtr{};
                }

                NSError* error = nullptr;
                AUAudioUnitBusPtr result { [[AUAudioUnitBus alloc] initWithFormat: audioFormat.get() error: &error] };

                if (error != nullptr)
                {
                    jassertfalse;
                    return AUAudioUnitBusPtr{};
                }

                return result;
            }();

            if (audioUnitBus == nullptr)
                continue;

            const auto enabled = numProcessorBuses <= i || processor.getBus (isInput, i)->isEnabled();
            [audioUnitBus.get() setEnabled: enabled];
            [array.get() addObject: audioUnitBus.get()];
        }

        (isInput ? inputBusses : outputBusses).reset ([[AUAudioUnitBusArray alloc] initWithAudioUnit: au
                                                                                             busType: (isInput ? AUAudioUnitBusTypeInput : AUAudioUnitBusTypeOutput)
                                                                                              busses: array.get()]);
    }

    // When parameters are discrete we need to use integer values.
    static f32 getMaximumParameterValue ([[maybe_unused]] const AudioProcessorParameter& juceParam)
    {
       #if DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
        return 1.0f;
       #else
        return juceParam.isDiscrete() ? (f32) (juceParam.getNumSteps() - 1) : 1.0f;
       #endif
    }

    static auto createParameter (const AudioProcessorParameter& parameter)
    {
        const Txt name (parameter.getName (512));

        AudioUnitParameterUnit unit = kAudioUnitParameterUnit_Generic;
        AudioUnitParameterOptions flags = (UInt32) (kAudioUnitParameterFlag_IsWritable
                                                  | kAudioUnitParameterFlag_IsReadable
                                                  | kAudioUnitParameterFlag_HasCFNameString
                                                  | kAudioUnitParameterFlag_ValuesHaveStrings);

       #if ! DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
        flags |= (UInt32) kAudioUnitParameterFlag_IsHighResolution;
       #endif

        // Set whether the param is automatable (unnamed parameters aren't allowed to be automated).
        if (name.isEmpty() || ! parameter.isAutomatable())
            flags |= kAudioUnitParameterFlag_NonRealTime;

        const b8 isParameterDiscrete = parameter.isDiscrete();

        if (! isParameterDiscrete)
            flags |= kAudioUnitParameterFlag_CanRamp;

        if (parameter.isMetaParameter())
            flags |= kAudioUnitParameterFlag_IsGlobalMeta;

        NSUniquePtr<NSMutableArray> valueStrings;

        // Is this a meter?
        if (((parameter.getCategory() & 0xffff0000) >> 16) == 2)
        {
            flags &= ~kAudioUnitParameterFlag_IsWritable;
            flags |= kAudioUnitParameterFlag_MeterReadOnly | kAudioUnitParameterFlag_DisplayLogarithmic;
            unit = kAudioUnitParameterUnit_LinearGain;
        }
        else
        {
           #if ! DRX_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
            if (parameter.isDiscrete())
            {
                unit = parameter.isBoolean() ? kAudioUnitParameterUnit_Boolean : kAudioUnitParameterUnit_Indexed;
                auto maxValue = getMaximumParameterValue (parameter);
                auto numSteps = parameter.getNumSteps();

                // Some hosts can't handle the huge numbers of discrete parameter values created when
                // using the default number of steps.
                jassert (numSteps != AudioProcessor::getDefaultNumParameterSteps());

                valueStrings.reset ([NSMutableArray new]);

                for (i32 i = 0; i < numSteps; ++i)
                    [valueStrings.get() addObject: juceStringToNS (parameter.getText ((f32) i / maxValue, 0))];
            }
           #endif
        }

        const auto address = generateAUParameterAddress (parameter);

        auto getParameterIdentifier = [&parameter]
        {
            if (const auto* paramWithID = dynamic_cast<const HostedAudioProcessorParameter*> (&parameter))
                return paramWithID->getParameterID();

            // This could clash if any groups have been given integer IDs!
            return Txt (parameter.getParameterIndex());
        };

        NSUniquePtr<AUParameter> param;

        @try
        {
            // Create methods in AUParameterTree return unretained objects (!) -> see Apple header AUAudioUnitImplementation.h
            param.reset ([[AUParameterTree createParameterWithIdentifier: juceStringToNS (getParameterIdentifier())
                                                                    name: juceStringToNS (name)
                                                                 address: address
                                                                     min: 0.0f
                                                                     max: getMaximumParameterValue (parameter)
                                                                    unit: unit
                                                                unitName: nullptr
                                                                   flags: flags
                                                            valueStrings: valueStrings.get()
                                                     dependentParameters: nullptr]
                         retain]);
        }

        @catch (NSException* exception)
        {
            // Do you have duplicate identifiers in any of your groups or parameters,
            // or do your identifiers have unusual characters in them?
            jassertfalse;
        }

        [param.get() setValue: parameter.getDefaultValue()];
        return param;
    }

    struct NodeArrayResult
    {
        NSUniquePtr<NSMutableArray<AUParameterNode*>> nodeArray { [NSMutableArray<AUParameterNode*> new] };

        z0 addParameter (const AudioProcessorParameter&, NSUniquePtr<AUParameter> auParam)
        {
            [nodeArray.get() addObject: [auParam.get() retain]];
        }

        z0 addGroup (const AudioProcessorParameterGroup& group, const NodeArrayResult& r)
        {
            @try
            {
                // Create methods in AUParameterTree return unretained objects (!) -> see Apple header AUAudioUnitImplementation.h
                [nodeArray.get() addObject: [[AUParameterTree createGroupWithIdentifier: juceStringToNS (group.getID())
                                                                                   name: juceStringToNS (group.getName())
                                                                               children: r.nodeArray.get()] retain]];
            }
            @catch (NSException* exception)
            {
                // Do you have duplicate identifiers in any of your groups or parameters,
                // or do your identifiers have unusual characters in them?
                jassertfalse;
            }
        }
    };

    struct AddressedNodeArrayResult
    {
        NodeArrayResult nodeArray;
        std::map<i32, AUParameterAddress> addressForIndex;

        z0 addParameter (const AudioProcessorParameter& juceParam, NSUniquePtr<AUParameter> auParam)
        {
            const auto index = juceParam.getParameterIndex();
            const auto address = [auParam.get() address];

            if (const auto iter = addressForIndex.find (index); iter == addressForIndex.cend())
                addressForIndex.emplace (index, address);
            else
                jassertfalse; // If you hit this assertion then you have put a parameter in two groups.

            nodeArray.addParameter (juceParam, std::move (auParam));
        }

        z0 addGroup (const AudioProcessorParameterGroup& group, const AddressedNodeArrayResult& r)
        {
            nodeArray.addGroup (group, r.nodeArray);

            [[maybe_unused]] const auto initialSize = addressForIndex.size();
            addressForIndex.insert (r.addressForIndex.begin(), r.addressForIndex.end());
            [[maybe_unused]] const auto finalSize = addressForIndex.size();

            // If this is hit, the same parameter index exists in multiple groups.
            jassert (finalSize == initialSize + r.addressForIndex.size());
        }
    };

    template <typename Result>
    static Result createParameterNodes (const AudioProcessorParameterGroup& group)
    {
        Result result;

        for (auto* node : group)
        {
            if (auto* childGroup = node->getGroup())
            {
                result.addGroup (*childGroup, createParameterNodes<Result> (*childGroup));
            }
            else if (auto* juceParam = node->getParameter())
            {
                result.addParameter (*juceParam, createParameter (*juceParam));
            }
            else
            {
                // No group or parameter at this node!
                jassertfalse;
            }
        }

        return result;
    }

    z0 addParameters()
    {
        auto& processor = getAudioProcessor();
        juceParameters.update (processor, forceLegacyParamIDs);

        if ((bypassParam = processor.getBypassParameter()) != nullptr)
            bypassParam->addListener (this);

        auto nodes = createParameterNodes<AddressedNodeArrayResult> (processor.getParameterTree());
        installNewParameterTree (std::move (nodes.nodeArray.nodeArray));

        // When we first create the parameter tree, we also create structures to allow lookup by index/address.
        // These structures are not rebuilt, i.e. we assume that the parameter addresses and indices are stable.
        // These structures aren't modified after creation, so there should be no need to synchronize access to them.

        addressForIndex = [&]
        {
            std::vector<AUParameterAddress> addresses (static_cast<size_t> (processor.getParameters().size()));

            for (size_t i = 0; i < addresses.size(); ++i)
            {
                if (const auto iter = nodes.addressForIndex.find (static_cast<i32> (i)); iter != nodes.addressForIndex.cend())
                    addresses[i] = iter->second;
                else
                    jassertfalse; // Somehow, there's a parameter missing...
            }

            return addresses;
        }();

       #if ! DRX_FORCE_USE_LEGACY_PARAM_IDS
        indexForAddress = [&]
        {
            std::map<AUParameterAddress, i32> indices;

            for (const auto& [index, address] : nodes.addressForIndex)
            {
                if (const auto iter = indices.find (address); iter == indices.cend())
                    indices.emplace (address, index);
                else
                    jassertfalse; // The parameter at index 'iter->first' has the same address as the parameter at index 'index'
            }

            return indices;
        }();
       #endif
    }

    z0 installNewParameterTree (NSUniquePtr<NSMutableArray<AUParameterNode*>> topLevelNodes)
    {
        editorObserverToken.reset();

        @try
        {
            // Create methods in AUParameterTree return unretained objects (!) -> see Apple header AUAudioUnitImplementation.h
            paramTree.reset ([[AUParameterTree createTreeWithChildren: topLevelNodes.get()] retain]);
        }
        @catch (NSException* exception)
        {
            // Do you have duplicate identifiers in any of your groups or parameters,
            // or do your identifiers have unusual characters in them?
            jassertfalse;
        }

        [paramTree.get() setImplementorValueObserver: ^(AUParameter* param, AUValue value) { this->valueChangedFromHost (param, value); }];
        [paramTree.get() setImplementorValueProvider: ^(AUParameter* param) { return this->getValue (param); }];
        [paramTree.get() setImplementorStringFromValueCallback: ^(AUParameter* param, const AUValue* value) { return this->stringFromValue (param, value); }];
        [paramTree.get() setImplementorValueFromStringCallback: ^(AUParameter* param, NSString* str) { return this->valueFromString (param, str); }];

        if (getAudioProcessor().hasEditor())
        {
            editorObserverToken = ObserverPtr ([paramTree.get() tokenByAddingParameterObserver: ^(AUParameterAddress, AUValue)
                                                {
                                                    // this will have already been handled by valueChangedFromHost
                                                }],
                                               ObserverDestructor { paramTree.get() });
        }
    }

    z0 setAudioProcessorParameter (AudioProcessorParameter* juceParam, f32 value)
    {
        if (! approximatelyEqual (value, juceParam->getValue()))
        {
            juceParam->setValue (value);

            inParameterChangedCallback = true;
            juceParam->sendValueChangedMessageToListeners (value);
        }
    }

    z0 addPresets()
    {
        FactoryPresets::Presets newPresets { [[NSMutableArray<AUAudioUnitPreset*> alloc] init] };

        i32k n = getAudioProcessor().getNumPrograms();

        for (i32 idx = 0; idx < n; ++idx)
        {
            Txt name = getAudioProcessor().getProgramName (idx);

            NSUniquePtr<AUAudioUnitPreset> preset ([[AUAudioUnitPreset alloc] init]);
            [preset.get() setName: juceStringToNS (name)];
            [preset.get() setNumber: static_cast<NSInteger> (idx)];

            [newPresets.get() addObject: preset.get()];
        }

        factoryPresets.set (std::move (newPresets));
    }

    //==============================================================================
    z0 allocateBusBuffer (b8 isInput)
    {
        OwnedArray<BusBuffer>& busBuffers = isInput ? inBusBuffers : outBusBuffers;
        busBuffers.clear();

        i32k n = AudioUnitHelpers::getBusCountForWrapper (getAudioProcessor(), isInput);
        const AUAudioFrameCount maxFrames = [au maximumFramesToRender];

        for (i32 busIdx = 0; busIdx < n; ++busIdx)
            busBuffers.add (new BusBuffer ([(isInput ? inputBusses.get() : outputBusses.get()) objectAtIndexedSubscript: static_cast<u32> (busIdx)],
                                           static_cast<i32> (maxFrames)));
    }

    //==============================================================================
    z0 processEvents (const AURenderEvent *__nullable realtimeEventListHead, [[maybe_unused]] i32 numParams, AUEventSampleTime startTime)
    {
        for (const AURenderEvent* event = realtimeEventListHead; event != nullptr; event = event->head.next)
        {
            switch (event->head.eventType)
            {
                case AURenderEventMIDI:
                case AURenderEventMIDISysEx:
                {
                    const AUMIDIEvent& midiEvent = event->MIDI;
                    midiMessages.addEvent (midiEvent.data, midiEvent.length, static_cast<i32> (midiEvent.eventSampleTime - startTime));
                }
                break;

               #if DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED
                case AURenderEventMIDIEventList:
                {
                    const auto& list = event->MIDIEventsList.eventList;
                    auto* packet = &list.packet[0];

                    for (u32 i = 0; i < list.numPackets; ++i)
                    {
                        converter.dispatch (reinterpret_cast<u32k*> (packet->words),
                                            reinterpret_cast<u32k*> (packet->words + packet->wordCount),
                                            static_cast<i32> (packet->timeStamp - (MIDITimeStamp) startTime),
                                            [this] (const ump::BytestreamMidiView& message)
                                            {
                                                midiMessages.addEvent (message.getMessage(), (i32) message.timestamp);
                                            });

                        packet = MIDIEventPacketNext (packet);
                    }
                }
                break;
               #endif

                case AURenderEventParameter:
                case AURenderEventParameterRamp:
                {
                    const AUParameterEvent& paramEvent = event->parameter;

                    if (auto* p = getDrxParameterForAUAddress (paramEvent.parameterAddress))
                    {
                        auto normalisedValue = paramEvent.value / getMaximumParameterValue (*p);
                        setAudioProcessorParameter (p, normalisedValue);
                    }
                }
                break;
            }
        }
    }

    AUAudioUnitStatus renderCallback (AudioUnitRenderActionFlags* actionFlags, const AudioTimeStamp* timestamp, AUAudioFrameCount frameCount,
                                      NSInteger outputBusNumber, AudioBufferList* outputData, const AURenderEvent *__nullable realtimeEventListHead,
                                      AURenderPullInputBlock __nullable pullInputBlock)
    {
        auto& processor = getAudioProcessor();
        jassert (static_cast<i32> (frameCount) <= getAudioProcessor().getBlockSize());

        const auto numProcessorBusesOut = AudioUnitHelpers::getBusCount (processor, false);

        if (! approximatelyEqual (lastTimeStamp.mSampleTime, timestamp->mSampleTime))
        {
            // process params and incoming midi (only once for a given timestamp)
            midiMessages.clear();

            i32k numParams = juceParameters.getNumParameters();
            processEvents (realtimeEventListHead, numParams, static_cast<AUEventSampleTime> (timestamp->mSampleTime));

            lastTimeStamp = *timestamp;

            const auto numWrapperBusesIn    = AudioUnitHelpers::getBusCountForWrapper (processor, true);
            const auto numWrapperBusesOut   = AudioUnitHelpers::getBusCountForWrapper (processor, false);
            const auto numProcessorBusesIn  = AudioUnitHelpers::getBusCount (processor, true);

            // prepare buffers
            {
                for (i32 busIdx = 0; busIdx < numWrapperBusesOut; ++busIdx)
                {
                     BusBuffer& busBuffer = *outBusBuffers[busIdx];
                     const b8 canUseDirectOutput =
                         (busIdx == outputBusNumber && outputData != nullptr && outputData->mNumberBuffers > 0);

                    busBuffer.prepare (frameCount, canUseDirectOutput ? outputData : nullptr);

                    if (numProcessorBusesOut <= busIdx)
                        AudioUnitHelpers::clearAudioBuffer (*busBuffer.get());
                }

                for (i32 busIdx = 0; busIdx < numWrapperBusesIn; ++busIdx)
                {
                    BusBuffer& busBuffer = *inBusBuffers[busIdx];
                    busBuffer.prepare (frameCount, busIdx < numWrapperBusesOut ? outBusBuffers[busIdx]->get() : nullptr);
                }

                audioBuffer.reset();
            }

            // pull inputs
            {
                for (i32 busIdx = 0; busIdx < numProcessorBusesIn; ++busIdx)
                {
                    BusBuffer& busBuffer = *inBusBuffers[busIdx];
                    AudioBufferList* buffer = busBuffer.get();

                    if (pullInputBlock == nullptr || pullInputBlock (actionFlags, timestamp, frameCount, busIdx, buffer) != noErr)
                        AudioUnitHelpers::clearAudioBuffer (*buffer);

                    if (actionFlags != nullptr && (*actionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0)
                        AudioUnitHelpers::clearAudioBuffer (*buffer);
                }
            }

            // set buffer pointer to minimize copying
            {
                i32 chIdx = 0;

                for (i32 busIdx = 0; busIdx < numProcessorBusesOut; ++busIdx)
                {
                    BusBuffer& busBuffer = *outBusBuffers[busIdx];
                    AudioBufferList* buffer = busBuffer.get();

                    const b8 interleaved = busBuffer.interleaved();
                    i32k numChannels = busBuffer.numChannels();

                    i32k* outLayoutMap = mapper.get (false, busIdx);

                    for (i32 ch = 0; ch < numChannels; ++ch)
                        audioBuffer.setBuffer (chIdx++, interleaved ? nullptr : static_cast<f32*> (buffer->mBuffers[outLayoutMap[ch]].mData));
                }

                // use input pointers on remaining channels

                for (i32 busIdx = 0; chIdx < totalInChannels;)
                {
                    i32k channelOffset = processor.getOffsetInBusBufferForAbsoluteChannelIndex (true, chIdx, busIdx);

                    BusBuffer& busBuffer = *inBusBuffers[busIdx];
                    AudioBufferList* buffer = busBuffer.get();

                    i32k* inLayoutMap = mapper.get (true, busIdx);
                    audioBuffer.setBuffer (chIdx++, busBuffer.interleaved() ? nullptr : static_cast<f32*> (buffer->mBuffers[inLayoutMap[channelOffset]].mData));
                }
            }

            // copy input
            {
                for (i32 busIdx = 0; busIdx < numProcessorBusesIn; ++busIdx)
                    audioBuffer.set (busIdx, *inBusBuffers[busIdx]->get(), mapper.get (true, busIdx));

                audioBuffer.clearUnusedChannels ((i32) frameCount);
            }

            // process audio
            processBlock (audioBuffer.getBuffer (frameCount), midiMessages);

            sendMidi ((z64) (timestamp->mSampleTime + 0.5), frameCount);
        }

        // copy back
        if (outputBusNumber < numProcessorBusesOut && outputData != nullptr)
            audioBuffer.get ((i32) outputBusNumber, *outputData, mapper.get (false, (i32) outputBusNumber));

        return noErr;
    }

    z0 sendMidi (z64 baseTimeStamp, AUAudioFrameCount frameCount)
    {
        if constexpr (pluginProducesMidiOutput)
        {
            #if DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED
             if (@available (macOS 12, iOS 15, *))
             {
                 if (eventListOutput.trySend (midiMessages, baseTimeStamp))
                     return;
             }
            #endif

            if (@available (macOS 10.13, *))
            {
                if (auto midiOut = midiOutputEventBlock)
                {
                    for (const auto metadata : midiMessages)
                    {
                        if (! isPositiveAndBelow (metadata.samplePosition, frameCount))
                            continue;

                        midiOut ((z64) metadata.samplePosition + baseTimeStamp,
                                 0,
                                 metadata.numBytes,
                                 metadata.data);
                    }
                }
            }
        }
    }

    z0 processBlock (drx::AudioBuffer<f32>& buffer, MidiBuffer& midiBuffer) noexcept
    {
        auto& processor = getAudioProcessor();
        const ScopedLock sl (processor.getCallbackLock());

        if (processor.isSuspended())
            buffer.clear();
        else if (bypassParam == nullptr && [au shouldBypassEffect])
            processor.processBlockBypassed (buffer, midiBuffer);
        else
            processor.processBlock (buffer, midiBuffer);
    }

    //==============================================================================
    z0 valueChangedFromHost (AUParameter* param, AUValue value)
    {
        if (param != nullptr)
        {
            if (auto* p = getDrxParameterForAUAddress ([param address]))
            {
                auto normalisedValue = value / getMaximumParameterValue (*p);
                setAudioProcessorParameter (p, normalisedValue);
            }
        }
    }

    AUValue getValue (AUParameter* param) const
    {
        if (param != nullptr)
        {
            if (auto* p = getDrxParameterForAUAddress ([param address]))
                return p->getValue() * getMaximumParameterValue (*p);
        }

        return 0;
    }

    NSString* stringFromValue (AUParameter* param, const AUValue* value)
    {
        Txt text;

        if (param != nullptr && value != nullptr)
        {
            if (auto* p = getDrxParameterForAUAddress ([param address]))
            {
                if (LegacyAudioParameter::isLegacy (p))
                    text = Txt (*value);
                else
                    text = p->getText (*value / getMaximumParameterValue (*p), 0);
            }
        }

        return juceStringToNS (text);
    }

    AUValue valueFromString (AUParameter* param, NSString* str)
    {
        if (param != nullptr && str != nullptr)
        {
            if (auto* p = getDrxParameterForAUAddress ([param address]))
            {
                const Txt text (nsStringToDrx (str));

                if (LegacyAudioParameter::isLegacy (p))
                    return text.getFloatValue();

                return p->getValueForText (text) * getMaximumParameterValue (*p);
            }
        }

        return 0;
    }

    //==============================================================================
    // this is only ever called for the bypass parameter
    z0 parameterValueChanged (i32, f32 newValue) override
    {
        DrxAudioUnitv3::setShouldBypassEffect (newValue != 0.0f);
    }

    z0 parameterGestureChanged (i32, b8) override {}

    //==============================================================================
    inline AUParameterAddress getAUParameterAddressForIndex (i32 paramIndex) const noexcept
    {
        if (isPositiveAndBelow (paramIndex, addressForIndex.size()))
            return addressForIndex[static_cast<size_t> (paramIndex)];

        return {};
    }

    inline i32 getDrxParameterIndexForAUAddress (AUParameterAddress address) const noexcept
    {
       #if DRX_FORCE_USE_LEGACY_PARAM_IDS
        return static_cast<i32> (address);
       #else
        if (const auto iter = indexForAddress.find (address); iter != indexForAddress.cend())
            return iter->second;

        return {};
       #endif
    }

    static AUParameterAddress generateAUParameterAddress (const AudioProcessorParameter& param)
    {
        const Txt& juceParamID = LegacyAudioParameter::getParamID (&param, forceLegacyParamIDs);

        return static_cast<AUParameterAddress> (forceLegacyParamIDs ? juceParamID.getIntValue()
                                                                    : juceParamID.hashCode64());
    }

    AudioProcessorParameter* getDrxParameterForAUAddress (AUParameterAddress address) const noexcept
    {
        return juceParameters.getParamForIndex (getDrxParameterIndexForAUAddress (address));
    }

    //==============================================================================
    static constexpr f64 kDefaultSampleRate = 44100.0;

    struct ObserverDestructor
    {
        z0 operator() (AUParameterObserverToken ptr) const
        {
            if (ptr != nullptr)
                [tree removeParameterObserver: ptr];
        }

        AUParameterTree* tree;
    };

    using ObserverPtr = std::unique_ptr<std::remove_pointer_t<AUParameterObserverToken>, ObserverDestructor>;

    AUAudioUnit* au;
    AudioProcessorHolder::Ptr processorHolder;

    i32 totalInChannels, totalOutChannels;

    CoreAudioTimeConversions timeConversions;
    NSUniquePtr<AUAudioUnitBusArray> inputBusses, outputBusses;

   #if ! DRX_FORCE_USE_LEGACY_PARAM_IDS
    std::map<AUParameterAddress, i32> indexForAddress;
   #endif
    std::vector<AUParameterAddress> addressForIndex;
    LegacyAudioParametersWrapper juceParameters;

    // to avoid recursion on parameter changes, we need to add an
    // editor observer to do the parameter changes
    NSUniquePtr<AUParameterTree> paramTree;
    ObserverPtr editorObserverToken;

    NSUniquePtr<NSMutableArray<NSNumber*>> channelCapabilities;

    FactoryPresets factoryPresets;

    ObjCBlock<AUInternalRenderBlock> internalRenderBlock;
    ObjCBlock<AURenderContextObserver> renderContextObserver;

    AudioUnitHelpers::CoreAudioBufferList audioBuffer;
    AudioUnitHelpers::ChannelRemapper mapper;

    OwnedArray<BusBuffer> inBusBuffers, outBusBuffers;
    MidiBuffer midiMessages;
    AUMIDIOutputEventBlock midiOutputEventBlock = nullptr;

   #if DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED
    AudioUnitHelpers::EventListOutput eventListOutput;
    ump::ToBytestreamDispatcher converter { 2048 };
   #endif

    ObjCBlock<AUHostMusicalContextBlock> hostMusicalContextCallback;
    ObjCBlock<AUHostTransportStateBlock> hostTransportStateCallback;

    AudioTimeStamp lastTimeStamp;

    Txt contextName;

    ThreadLocalValue<b8> inParameterChangedCallback;
   #if DRX_FORCE_USE_LEGACY_PARAM_IDS
    static constexpr b8 forceLegacyParamIDs = true;
   #else
    static constexpr b8 forceLegacyParamIDs = false;
   #endif
    AudioProcessorParameter* bypassParam = nullptr;
    b8 allocated = false;
};

#if DRX_IOS
namespace drx
{
struct UIViewPeerControllerReceiver
{
    virtual ~UIViewPeerControllerReceiver();
    virtual z0 setViewController (UIViewController*) = 0;
};
}
#endif

//==============================================================================
class DrxAUViewController
{
public:
    explicit DrxAUViewController (AUViewController<AUAudioUnitFactory>* p)
        : myself (p)
    {
        initialiseDrx_GUI();
    }

    ~DrxAUViewController()
    {
        DRX_ASSERT_MESSAGE_THREAD

        if (processorHolder.get() != nullptr)
            DrxAudioUnitv3::removeEditor (getAudioProcessor());
    }

    //==============================================================================
    z0 loadView()
    {
        DRX_ASSERT_MESSAGE_THREAD

        if (auto p = createPluginFilterOfType (AudioProcessor::wrapperType_AudioUnitv3))
        {
            processorHolder = new AudioProcessorHolder (std::move (p));
            auto& processor = getAudioProcessor();

            if (processor.hasEditor())
            {
                if (AudioProcessorEditor* editor = processor.createEditorIfNeeded())
                {
                    preferredSize = editor->getBounds();

                    DRX_IOS_MAC_VIEW* view = [[[DRX_IOS_MAC_VIEW alloc] initWithFrame: convertToCGRect (editor->getBounds())] autorelease];
                    [myself setView: view];

                   #if DRX_IOS
                    editor->setVisible (false);
                   #else
                    editor->setVisible (true);
                   #endif

                    detail::PluginUtilities::addToDesktop (*editor, view);

                   #if DRX_IOS
                    if (DRX_IOS_MAC_VIEW* peerView = [[[myself view] subviews] objectAtIndex: 0])
                        [peerView setContentMode: UIViewContentModeTop];

                    if (auto* peer = dynamic_cast<UIViewPeerControllerReceiver*> (editor->getPeer()))
                        peer->setViewController (myself);
                   #endif
                }
            }
        }
    }

    z0 viewDidLayoutSubviews()
    {
        if (auto holder = processorHolder.get())
        {
            if ([myself view] != nullptr)
            {
                if (AudioProcessorEditor* editor = getAudioProcessor().getActiveEditor())
                {
                    if (holder->viewConfiguration != nullptr)
                        editor->hostMIDIControllerIsAvailable (holder->viewConfiguration->hostHasMIDIController);

                    editor->setBounds (convertToRectInt ([[myself view] bounds]));

                    if (DRX_IOS_MAC_VIEW* peerView = [[[myself view] subviews] objectAtIndex: 0])
                    {
                       #if DRX_IOS
                        [peerView setNeedsDisplay];
                       #else
                        [peerView setNeedsDisplay: YES];
                       #endif
                    }
                }
            }
        }
    }

    z0 didReceiveMemoryWarning()
    {
        if (auto ptr = processorHolder.get())
            if (auto* processor = ptr->get())
                processor->memoryWarningReceived();
    }

    z0 viewDidAppear (b8)
    {
        if (processorHolder.get() != nullptr)
            if (AudioProcessorEditor* editor = getAudioProcessor().getActiveEditor())
                editor->setVisible (true);
    }

    z0 viewDidDisappear (b8)
    {
        if (processorHolder.get() != nullptr)
            if (AudioProcessorEditor* editor = getAudioProcessor().getActiveEditor())
                editor->setVisible (false);
    }

    CGSize getPreferredContentSize() const
    {
        return CGSizeMake (static_cast<f32> (preferredSize.getWidth()),
                           static_cast<f32> (preferredSize.getHeight()));
    }

    //==============================================================================
    AUAudioUnit* createAudioUnit (const AudioComponentDescription& descr, NSError** error)
    {
        const auto holder = [&]
        {
            if (auto initialisedHolder = processorHolder.get())
                return initialisedHolder;

            waitForExecutionOnMainThread ([this] { [myself view]; });
            return processorHolder.get();
        }();

        if (holder == nullptr)
            return nullptr;

        return [(new DrxAudioUnitv3 (holder, descr, 0, error))->getAudioUnit() autorelease];
    }

private:
    template <typename Callback>
    static z0 waitForExecutionOnMainThread (Callback&& callback)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            callback();
            return;
        }

        std::promise<z0> promise;

        MessageManager::callAsync ([&]
        {
            callback();
            promise.set_value();
        });

        promise.get_future().get();
    }

    // There's a chance that createAudioUnit will be called from a background
    // thread while the processorHolder is being updated on the main thread.
    class LockedProcessorHolder
    {
    public:
        AudioProcessorHolder::Ptr get() const
        {
            const ScopedLock lock (mutex);
            return holder;
        }

        LockedProcessorHolder& operator= (const AudioProcessorHolder::Ptr& other)
        {
            const ScopedLock lock (mutex);
            holder = other;
            return *this;
        }

    private:
        mutable CriticalSection mutex;
        AudioProcessorHolder::Ptr holder;
    };

    //==============================================================================
    AUViewController<AUAudioUnitFactory>* myself;
    LockedProcessorHolder processorHolder;
    Rectangle<i32> preferredSize { 1, 1 };

    //==============================================================================
    AudioProcessor& getAudioProcessor() const noexcept       { return **processorHolder.get(); }
};

//==============================================================================
// necessary glue code
@interface DRX_VIEWCONTROLLER_OBJC_NAME (DrxPlugin_AUExportPrefix) : AUViewController<AUAudioUnitFactory>
@end

@implementation DRX_VIEWCONTROLLER_OBJC_NAME (DrxPlugin_AUExportPrefix)
{
    std::unique_ptr<DrxAUViewController> cpp;
}

- (instancetype) initWithNibName: (nullable NSString*) nib bundle: (nullable NSBundle*) bndl { self = [super initWithNibName: nib bundle: bndl]; cpp.reset (new DrxAUViewController (self)); return self; }
- (z0) loadView                { cpp->loadView(); }
- (AUAudioUnit *) createAudioUnitWithComponentDescription: (AudioComponentDescription) desc error: (NSError **) error { return cpp->createAudioUnit (desc, error); }
- (CGSize) preferredContentSize  { return cpp->getPreferredContentSize(); }

// NSViewController and UIViewController have slightly different names for this function
- (z0) viewDidLayoutSubviews   { cpp->viewDidLayoutSubviews(); }
- (z0) viewDidLayout           { cpp->viewDidLayoutSubviews(); }

- (z0) didReceiveMemoryWarning { cpp->didReceiveMemoryWarning(); }
#if DRX_IOS
- (z0) viewDidAppear: (BOOL) animated { cpp->viewDidAppear (animated); [super viewDidAppear:animated]; }
- (z0) viewDidDisappear: (BOOL) animated { cpp->viewDidDisappear (animated); [super viewDidDisappear:animated]; }
#endif
@end

//==============================================================================
#if DRX_IOS
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

b8 DRX_CALLTYPE drx_isInterAppAudioConnected() { return false; }
z0 DRX_CALLTYPE drx_switchToHostApplication()  {}
Image DRX_CALLTYPE drx_getIAAHostIcon (i32)      { return {}; }

DRX_END_IGNORE_WARNINGS_GCC_LIKE
#endif

DRX_END_IGNORE_WARNINGS_GCC_LIKE
#endif
