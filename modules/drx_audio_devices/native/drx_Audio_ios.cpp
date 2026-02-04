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

class iOSAudioIODevice;

constexpr tukk const iOSAudioDeviceName = "iOS Audio";

#ifndef DRX_IOS_AUDIO_EXPLICIT_SAMPLERATES
 #define DRX_IOS_AUDIO_EXPLICIT_SAMPLERATES
#endif

constexpr std::initializer_list<f64> iOSExplicitSampleRates { DRX_IOS_AUDIO_EXPLICIT_SAMPLERATES };

//==============================================================================
struct AudioSessionHolder
{
    AudioSessionHolder();
    ~AudioSessionHolder();

    z0 handleStatusChange (b8 enabled, tukk reason) const;
    z0 handleRouteChange (AVAudioSessionRouteChangeReason reason);

    Array<iOSAudioIODevice::Pimpl*> activeDevices;
    Array<iOSAudioIODeviceType*> activeDeviceTypes;

    id nativeSession;
};

static tukk getRoutingChangeReason (AVAudioSessionRouteChangeReason reason) noexcept
{
    switch (reason)
    {
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:         return "New device available";
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:       return "Old device unavailable";
        case AVAudioSessionRouteChangeReasonCategoryChange:             return "Category change";
        case AVAudioSessionRouteChangeReasonOverride:                   return "Override";
        case AVAudioSessionRouteChangeReasonWakeFromSleep:              return "Wake from sleep";
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory: return "No suitable route for category";
        case AVAudioSessionRouteChangeReasonRouteConfigurationChange:   return "Route configuration change";
        case AVAudioSessionRouteChangeReasonUnknown:
        default:                                                        return "Unknown";
    }
}

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

b8 getNotificationValueForKey (NSNotification* notification, NSString* key, NSUInteger& value) noexcept
{
    if (notification != nil)
    {
        if (NSDictionary* userInfo = [notification userInfo])
        {
            if (NSNumber* number = [userInfo objectForKey: key])
            {
                value = [number unsignedIntegerValue];
                return true;
            }
        }
    }

    jassertfalse;
    return false;
}

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx

//==============================================================================
@interface iOSAudioSessionNative  : NSObject
{
@private
    drx::AudioSessionHolder* audioSessionHolder;
};

- (id) init: (drx::AudioSessionHolder*) holder;
- (z0) dealloc;

- (z0) audioSessionChangedInterruptionType: (NSNotification*) notification;
- (z0) handleMediaServicesReset;
- (z0) handleMediaServicesLost;
- (z0) handleRouteChange: (NSNotification*) notification;
@end

@implementation iOSAudioSessionNative

- (id) init: (drx::AudioSessionHolder*) holder
{
    self = [super init];

    if (self != nil)
    {
        audioSessionHolder = holder;

        auto session = [AVAudioSession sharedInstance];
        auto centre = [NSNotificationCenter defaultCenter];

        [centre addObserver: self
                   selector: @selector (audioSessionChangedInterruptionType:)
                       name: AVAudioSessionInterruptionNotification
                     object: session];

        [centre addObserver: self
                   selector: @selector (handleMediaServicesLost)
                       name: AVAudioSessionMediaServicesWereLostNotification
                     object: session];

        [centre addObserver: self
                   selector: @selector (handleMediaServicesReset)
                       name: AVAudioSessionMediaServicesWereResetNotification
                     object: session];

        [centre addObserver: self
                   selector: @selector (handleRouteChange:)
                       name: AVAudioSessionRouteChangeNotification
                     object: session];
    }
    else
    {
        jassertfalse;
    }

    return self;
}

- (z0) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    [super dealloc];
}

- (z0) audioSessionChangedInterruptionType: (NSNotification*) notification
{
    NSUInteger value;

    if (drx::getNotificationValueForKey (notification, AVAudioSessionInterruptionTypeKey, value))
    {
        switch ((AVAudioSessionInterruptionType) value)
        {
            case AVAudioSessionInterruptionTypeBegan:
                audioSessionHolder->handleStatusChange (false, "AVAudioSessionInterruptionTypeBegan");
                break;

            case AVAudioSessionInterruptionTypeEnded:
                audioSessionHolder->handleStatusChange (true, "AVAudioSessionInterruptionTypeEnded");
                break;

            // No default so the code doesn't compile if this enum is extended.
        }
    }
}

- (z0) handleMediaServicesReset
{
    audioSessionHolder->handleStatusChange (true, "AVAudioSessionMediaServicesWereResetNotification");
}

- (z0) handleMediaServicesLost
{
    audioSessionHolder->handleStatusChange (false, "AVAudioSessionMediaServicesWereLostNotification");
}

- (z0) handleRouteChange: (NSNotification*) notification
{
    NSUInteger value;

    if (drx::getNotificationValueForKey (notification, AVAudioSessionRouteChangeReasonKey, value))
        audioSessionHolder->handleRouteChange ((AVAudioSessionRouteChangeReason) value);
}

@end

//==============================================================================
namespace drx {

#ifndef DRX_IOS_AUDIO_LOGGING
 #define DRX_IOS_AUDIO_LOGGING 0
#endif

#if DRX_IOS_AUDIO_LOGGING
 #define DRX_IOS_AUDIO_LOG(x)  DBG(x)
#else
 #define DRX_IOS_AUDIO_LOG(x)
#endif

static z0 logNSError (NSError* e)
{
    if (e != nil)
    {
        DRX_IOS_AUDIO_LOG ("iOS Audio error: " << [e.localizedDescription UTF8String]);
        jassertfalse;
    }
}

#define DRX_NSERROR_CHECK(X)     { NSError* error = nil; X; logNSError (error); }

//==============================================================================
class iOSAudioIODeviceType final : public AudioIODeviceType,
                                   public AsyncUpdater
{
public:
    iOSAudioIODeviceType();
    ~iOSAudioIODeviceType() override;

    z0 scanForDevices() override;
    StringArray getDeviceNames (b8) const override;
    i32 getDefaultDeviceIndex (b8) const override;
    i32 getIndexOfDevice (AudioIODevice*, b8) const override;
    b8 hasSeparateInputsAndOutputs() const override;
    AudioIODevice* createDevice (const Txt&, const Txt&) override;

private:
    z0 handleRouteChange (AVAudioSessionRouteChangeReason);

    z0 handleAsyncUpdate() override;

    friend struct AudioSessionHolder;
    friend struct iOSAudioIODevice::Pimpl;

    SharedResourcePointer<AudioSessionHolder> sessionHolder;

    DRX_DECLARE_WEAK_REFERENCEABLE (iOSAudioIODeviceType)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSAudioIODeviceType)
};

class SubstituteAudioUnit
{
public:
    /* Возвращает true, если the audio callback was called. False if a timeout occurred. */
    b8 waitForAudioCallback()
    {
        if (audioUnit != nullptr)
        {
            AudioComponentInstanceDispose (audioUnit);
            audioUnit = nullptr;
        }

        AudioComponentDescription desc;
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_RemoteIO;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        desc.componentFlags = 0;
        desc.componentFlagsMask = 0;

        AudioComponent comp = AudioComponentFindNext (nullptr, &desc);
        AudioComponentInstanceNew (comp, &audioUnit);

        if (audioUnit == nullptr)
            return false;

        {
            AURenderCallbackStruct inputProc;
            inputProc.inputProc = audioUnitCallback;
            inputProc.inputProcRefCon = this;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inputProc, sizeof (inputProc));
        }

        {
            AudioStreamBasicDescription format;
            zerostruct (format);
            format.mSampleRate = [AVAudioSession sharedInstance].sampleRate;
            format.mFormatID = kAudioFormatLinearPCM;
            format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsPacked;
            format.mBitsPerChannel = 8 * sizeof (f32);
            format.mFramesPerPacket = 1;
            format.mChannelsPerFrame = 2;
            format.mBytesPerFrame = format.mBytesPerPacket = sizeof (f32);

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,  0, &format, sizeof (format));
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, sizeof (format));
        }

        AudioUnitInitialize (audioUnit);
        AudioOutputUnitStart (audioUnit);

        const auto result = audioCallbackOccurred.wait (1000.0);

        AudioComponentInstanceDispose (audioUnit);
        audioUnit = nullptr;

        return result;
    }

private:
    static OSStatus audioUnitCallback (uk object,
                                       AudioUnitRenderActionFlags*,
                                       const AudioTimeStamp*,
                                       UInt32,
                                       UInt32,
                                       AudioBufferList*)
    {
        static_cast<SubstituteAudioUnit*> (object)->audioCallbackOccurred.signal();

        return noErr;
    }

    AudioUnit audioUnit{};
    WaitableEvent audioCallbackOccurred;
};

//==============================================================================
struct iOSAudioIODevice::Pimpl final : public AsyncUpdater
{
    Pimpl (iOSAudioIODeviceType* ioDeviceType, iOSAudioIODevice& ioDevice)
        : deviceType (ioDeviceType),
          owner (ioDevice)
    {
        DRX_IOS_AUDIO_LOG ("Creating iOS audio device");

        // We need to activate the audio session here to obtain the available sample rates and buffer sizes,
        // but if we don't set a category first then background audio will always be stopped. This category
        // may be changed later.
        setAudioSessionCategory (AVAudioSessionCategoryPlayAndRecord);

        setAudioSessionActive (true);
        updateHardwareInfo();
        channelData.reconfigure ({}, {});
        setAudioSessionActive (false);

        sessionHolder->activeDevices.add (this);
    }

    ~Pimpl() override
    {
        sessionHolder->activeDevices.removeFirstMatchingValue (this);

        close();
    }

    static z0 setAudioSessionCategory (NSString* category)
    {
        NSUInteger options = 0;

       #if ! DRX_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS
        options |= AVAudioSessionCategoryOptionMixWithOthers; // Alternatively AVAudioSessionCategoryOptionDuckOthers
       #endif

        if (category == AVAudioSessionCategoryPlayAndRecord)
        {
            options |= AVAudioSessionCategoryOptionDefaultToSpeaker
                     | AVAudioSessionCategoryOptionAllowBluetooth
                     | AVAudioSessionCategoryOptionAllowAirPlay
                     | AVAudioSessionCategoryOptionAllowBluetoothA2DP;
        }

        DRX_NSERROR_CHECK ([[AVAudioSession sharedInstance] setCategory: category
                                                             withOptions: options
                                                                   error: &error]);
    }

    static z0 setAudioSessionActive (b8 enabled)
    {
        DRX_NSERROR_CHECK ([[AVAudioSession sharedInstance] setActive: enabled
                                                                 error: &error]);

        if (@available (ios 18, *))
        {
            if (enabled)
            {
                SubstituteAudioUnit au;
                [[maybe_unused]] const auto success = au.waitForAudioCallback();
                jassert (success);
            }
        }
    }

    i32 getBufferSize (const f64 currentSampleRate)
    {
        return roundToInt (currentSampleRate * [AVAudioSession sharedInstance].IOBufferDuration);
    }

    i32 tryBufferSize (const f64 currentSampleRate, i32k newBufferSize)
    {
        if (newBufferSize == getBufferSize (currentSampleRate))
            return newBufferSize;

        const auto extraOffset = std::invoke ([&]
        {
            // Older iOS versions (iOS 12) seem to require that the requested buffer size is a bit
            // larger than the desired buffer size.
            // This breaks on iOS 18, which needs the buffer duration to be as precise as possible.
            if (@available (ios 18, *))
                return 0;

            return 1;
        });

        NSTimeInterval bufferDuration = currentSampleRate > 0 ? (NSTimeInterval) (newBufferSize + extraOffset) / currentSampleRate : 0.0;

        auto session = [AVAudioSession sharedInstance];

        DRX_NSERROR_CHECK ([session setPreferredIOBufferDuration: bufferDuration error: &error]);

        // iOS requires additional effort to observe the actual buffer size
        // change however, it seems the buffer size change will always work
        // so instead we just assume the change will apply eventually
        if (@available (ios 18, *))
            return newBufferSize;

        return getBufferSize (currentSampleRate);
    }

    z0 updateAvailableBufferSizes()
    {
        availableBufferSizes.clear();

        const auto [minBufSize, maxBufSize] = std::invoke ([this]
        {
            constexpr auto suggestedMin = 64;
            constexpr auto suggestedMax = 4096;

            if (@available (ios 18, *))
                return std::tuple (suggestedMin, suggestedMax);

            const auto min = tryBufferSize (sampleRate, suggestedMin);
            const auto max = tryBufferSize (sampleRate, suggestedMax);

            bufferSize = tryBufferSize (sampleRate, bufferSize);

            return std::tuple (min, max);
        });

        jassert (minBufSize > 0);

        for (auto i = minBufSize; i <= maxBufSize; i *= 2)
            availableBufferSizes.add (i);

        // Sometimes the largest supported buffer size is not a power of 2
        availableBufferSizes.addIfNotAlreadyThere (maxBufSize);

       #if DRX_IOS_AUDIO_LOGGING
        {
            Txt info ("Available buffer sizes:");

            for (auto size : availableBufferSizes)
                info << " " << size;

            DRX_IOS_AUDIO_LOG (info);
        }
       #endif

        DRX_IOS_AUDIO_LOG ("Buffer size after detecting available buffer sizes: " << bufferSize);
    }

    API_AVAILABLE (ios (18))
    std::optional<f64> getSampleRateFromAudioQueue() const
    {
        AudioStreamBasicDescription stream{};
        stream.mFormatID = kAudioFormatLinearPCM;
        stream.mChannelsPerFrame = 2;
        stream.mBitsPerChannel = 32;
        stream.mFramesPerPacket = 1;
        stream.mBytesPerFrame = stream.mChannelsPerFrame * stream.mBitsPerChannel / 8;
        stream.mBytesPerPacket = stream.mBytesPerFrame * stream.mFramesPerPacket;
        stream.mFormatFlags = stream.mBitsPerChannel;
        stream.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger
                            | kLinearPCMFormatFlagIsBigEndian
                            | kLinearPCMFormatFlagIsPacked;

        AudioQueueRef audioQueue;

        const auto err = AudioQueueNewOutput (&stream,
                                              [] (auto, auto, auto) {},
                                              nullptr,
                                              nullptr,
                                              kCFRunLoopCommonModes,
                                              0,
                                              &audioQueue);

        if (err != noErr || audioQueue == nullptr)
        {
            jassertfalse;
            return {};
        }

        const ScopeGuard disposeAudioQueueOnReturn { [&]
        {
            AudioQueueDispose (audioQueue, true);
        }};

        f64 result{};

        UInt32 size = sizeof (sampleRate);
        const auto propErr = AudioQueueGetProperty (audioQueue,
                                                    kAudioQueueDeviceProperty_SampleRate,
                                                    &result,
                                                    &size);

        if (propErr != noErr || size != sizeof (result))
        {
            jassertfalse;
            return {};
        }

        return result;
    }

    f64 getSampleRate() const
    {
        const auto session = [AVAudioSession sharedInstance];

        // On iOS 18 the AVAudioSession sample rate is not always accurate but
        // probing the sample rate via an AudioQueue seems to work reliably
        if (@available (ios 18, *))
            return getSampleRateFromAudioQueue().value_or (session.sampleRate);

        return session.sampleRate;
    }

    f64 trySampleRate (f64 rate)
    {
        if (exactlyEqual (rate, getSampleRate()))
            return rate;

        auto session = [AVAudioSession sharedInstance];

        DRX_NSERROR_CHECK ([session setPreferredSampleRate: rate error: &error]);

        return getSampleRate();
    }

    // Important: the supported audio sample rates change on the iPhone 6S
    // depending on whether the headphones are plugged in or not!
    z0 updateAvailableSampleRates()
    {
        if (iOSExplicitSampleRates.size() != 0)
        {
            availableSampleRates = Array<f64> (iOSExplicitSampleRates);
            return;
        }

        const auto deviceId = std::invoke ([]
        {
            const auto route = [AVAudioSession sharedInstance].currentRoute;

            const auto describePorts = [] (auto ports, auto& id)
            {
                Txt description;
                auto count = 0;

                for (AVAudioSessionPortDescription* port in ports)
                    description << nsStringToDrx (port.UID) << id << count++;

                return description;
            };

            return describePorts (route.inputs, "i")
                 + describePorts (route.outputs, "o");
        });

        availableSampleRates = deviceSampleRatesCache.get (deviceId, [&] ([[maybe_unused]] auto key)
        {
            DRX_IOS_AUDIO_LOG ("Finding supported sample rates for: " << key);

            Array<f64> sampleRates;

            AudioUnitRemovePropertyListenerWithUserData (audioUnit,
                                                         kAudioUnitProperty_StreamFormat,
                                                         dispatchAudioUnitPropertyChange,
                                                         this);

            const f64 lowestRate = trySampleRate (4000);
            sampleRates.add (lowestRate);
            const f64 highestRate = trySampleRate (192000);

            DRX_IOS_AUDIO_LOG ("Lowest supported sample rate: "  << lowestRate);
            DRX_IOS_AUDIO_LOG ("Highest supported sample rate: " << highestRate);

            for (f64 rate = lowestRate + 1000; rate < highestRate; rate += 1000)
            {
                const f64 supportedRate = trySampleRate (rate);
                DRX_IOS_AUDIO_LOG ("Trying a sample rate of " << rate << ", got " << supportedRate);
                sampleRates.addIfNotAlreadyThere (supportedRate);
                rate = jmax (rate, supportedRate);
            }

            sampleRates.addIfNotAlreadyThere (highestRate);

            sampleRate = trySampleRate (sampleRate);

            AudioUnitAddPropertyListener (audioUnit,
                                          kAudioUnitProperty_StreamFormat,
                                          dispatchAudioUnitPropertyChange,
                                          this);

            // Check the current stream format in case things have changed whilst we
            // were going through the sample rates
            handleStreamFormatChange();

           #if DRX_IOS_AUDIO_LOGGING
            {
                Txt info ("Available sample rates:");

                for (auto rate : availableSampleRates)
                    info << " " << rate;

                DRX_IOS_AUDIO_LOG (info);
            }
           #endif

            DRX_IOS_AUDIO_LOG ("Sample rate after detecting available sample rates: " << sampleRate);

            return sampleRates;
        });
    }

    z0 updateHardwareInfo (b8 forceUpdate = false)
    {
        if (! forceUpdate && ! hardwareInfoNeedsUpdating.compareAndSetBool (false, true))
            return;

        DRX_IOS_AUDIO_LOG ("Updating hardware info");

        updateAvailableSampleRates();
        updateAvailableBufferSizes();

        if (deviceType != nullptr)
            deviceType->callDeviceChangeListeners();
    }

    z0 setTargetSampleRateAndBufferSize()
    {
        DRX_IOS_AUDIO_LOG ("Setting target sample rate: " << targetSampleRate);
        sampleRate = trySampleRate (targetSampleRate);
        DRX_IOS_AUDIO_LOG ("Actual sample rate: " << sampleRate);

        DRX_IOS_AUDIO_LOG ("Setting target buffer size: " << targetBufferSize);
        bufferSize = tryBufferSize (sampleRate, targetBufferSize);
        DRX_IOS_AUDIO_LOG ("Actual buffer size: " << bufferSize);
    }

    Txt open (const BigInteger& inputChannelsWanted,
                 const BigInteger& outputChannelsWanted,
                 f64 sampleRateWanted, i32 bufferSizeWanted)
    {
        close();

        firstHostTime = true;
        lastNumFrames = 0;
        xrun = 0;
        lastError.clear();

        requestedInputChannels  = inputChannelsWanted;
        requestedOutputChannels = outputChannelsWanted;
        targetSampleRate = sampleRateWanted;
        targetBufferSize = bufferSizeWanted > 0 ? bufferSizeWanted : defaultBufferSize;

        DRX_IOS_AUDIO_LOG ("Opening audio device:"
                            <<  " inputChannelsWanted: "  << requestedInputChannels .toString (2)
                            << ", outputChannelsWanted: " << requestedOutputChannels.toString (2)
                            << ", targetSampleRate: " << targetSampleRate
                            << ", targetBufferSize: " << targetBufferSize);

        setAudioSessionActive (true);
        setAudioSessionCategory (requestedInputChannels > 0 ? AVAudioSessionCategoryPlayAndRecord
                                                            : AVAudioSessionCategoryPlayback);
        channelData.reconfigure (requestedInputChannels, requestedOutputChannels);
        setTargetSampleRateAndBufferSize();
        updateHardwareInfo (true);
        fixAudioRouteIfSetToReceiver();

        isRunning = true;

        if (! createAudioUnit())
        {
            lastError = "Couldn't open the device";
            return lastError;
        }

        const ScopedLock sl (callbackLock);

        AudioOutputUnitStart (audioUnit);

        if (callback != nullptr)
            callback->audioDeviceAboutToStart (&owner);

        return lastError;
    }

    z0 close()
    {
        stop();

        if (isRunning)
        {
            isRunning = false;

            if (audioUnit != nullptr)
            {
                AudioOutputUnitStart (audioUnit);
                AudioComponentInstanceDispose (audioUnit);
                audioUnit = nullptr;
            }

            setAudioSessionActive (false);
        }
    }

    z0 start (AudioIODeviceCallback* newCallback)
    {
        if (isRunning && callback != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (&owner);

            const ScopedLock sl (callbackLock);
            callback = newCallback;
        }
    }

    z0 stop()
    {
        if (isRunning)
        {
            AudioIODeviceCallback* lastCallback;

            {
                const ScopedLock sl (callbackLock);
                lastCallback = callback;
                callback = nullptr;
            }

            if (lastCallback != nullptr)
                lastCallback->audioDeviceStopped();
        }
    }

    b8 setAudioPreprocessingEnabled (b8 enable)
    {
        auto session = [AVAudioSession sharedInstance];

        NSString* mode = (enable ? AVAudioSessionModeDefault
                                 : AVAudioSessionModeMeasurement);

        DRX_NSERROR_CHECK ([session setMode: mode
                                       error: &error]);

        return session.mode == mode;
    }

    //==============================================================================
    class PlayHead final : public AudioPlayHead
    {
    public:
        explicit PlayHead (Pimpl& implIn) : impl (implIn) {}

        b8 canControlTransport() override                    { return canControlTransportImpl(); }

        z0 transportPlay (b8 shouldSartPlaying) override
        {
            if (! canControlTransport())
                return;

            HostCallbackInfo callbackInfo;
            impl.fillHostCallbackInfo (callbackInfo);

            Boolean hostIsPlaying = NO;
            [[maybe_unused]] OSStatus err = callbackInfo.transportStateProc2 (callbackInfo.hostUserData,
                                                                              &hostIsPlaying,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr);

            jassert (err == noErr);

            if (hostIsPlaying != shouldSartPlaying)
                impl.handleAudioTransportEvent (kAudioUnitRemoteControlEvent_TogglePlayPause);
        }

        z0 transportRecord (b8 shouldStartRecording) override
        {
            if (! canControlTransport())
                return;

            HostCallbackInfo callbackInfo;
            impl.fillHostCallbackInfo (callbackInfo);

            Boolean hostIsRecording = NO;
            [[maybe_unused]] OSStatus err = callbackInfo.transportStateProc2 (callbackInfo.hostUserData,
                                                                              nullptr,
                                                                              &hostIsRecording,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr,
                                                                              nullptr);
            jassert (err == noErr);

            if (hostIsRecording != shouldStartRecording)
                impl.handleAudioTransportEvent (kAudioUnitRemoteControlEvent_ToggleRecord);
        }

        z0 transportRewind() override
        {
            if (canControlTransport())
                impl.handleAudioTransportEvent (kAudioUnitRemoteControlEvent_Rewind);
        }

        Optional<PositionInfo> getPosition() const override
        {
            if (! canControlTransportImpl())
                return {};

            HostCallbackInfo callbackInfo;
            impl.fillHostCallbackInfo (callbackInfo);

            if (callbackInfo.hostUserData == nullptr)
                return {};

            Boolean hostIsPlaying               = NO;
            Boolean hostIsRecording             = NO;
            Float64 hostCurrentSampleInTimeLine = 0;
            Boolean hostIsCycling               = NO;
            Float64 hostCycleStartBeat          = 0;
            Float64 hostCycleEndBeat            = 0;
            auto transportErr = callbackInfo.transportStateProc2 (callbackInfo.hostUserData,
                                                                  &hostIsPlaying,
                                                                  &hostIsRecording,
                                                                  nullptr,
                                                                  &hostCurrentSampleInTimeLine,
                                                                  &hostIsCycling,
                                                                  &hostCycleStartBeat,
                                                                  &hostCycleEndBeat);
            if (transportErr == kAUGraphErr_CannotDoInCurrentContext)
                return {};

            jassert (transportErr == noErr);

            PositionInfo result;

            result.setTimeInSamples ((z64) hostCurrentSampleInTimeLine);
            result.setIsPlaying     (hostIsPlaying);
            result.setIsRecording   (hostIsRecording);
            result.setIsLooping     (hostIsCycling);
            result.setLoopPoints    (LoopPoints { hostCycleStartBeat, hostCycleEndBeat });
            result.setTimeInSeconds ((f64) *result.getTimeInSamples() / impl.sampleRate);

            Float64 hostBeat = 0;
            Float64 hostTempo = 0;
            [[maybe_unused]] auto batErr = callbackInfo.beatAndTempoProc (callbackInfo.hostUserData,
                                                                          &hostBeat,
                                                                          &hostTempo);
            jassert (batErr == noErr);

            result.setPpqPosition (hostBeat);
            result.setBpm         (hostTempo);

            Float32 hostTimeSigNumerator = 0;
            UInt32 hostTimeSigDenominator = 0;
            Float64 hostCurrentMeasureDownBeat = 0;
            [[maybe_unused]] auto timeErr = callbackInfo.musicalTimeLocationProc (callbackInfo.hostUserData,
                                                                                  nullptr,
                                                                                  &hostTimeSigNumerator,
                                                                                  &hostTimeSigDenominator,
                                                                                  &hostCurrentMeasureDownBeat);
            jassert (timeErr == noErr);

            result.setPpqPositionOfLastBarStart (hostCurrentMeasureDownBeat);
            result.setTimeSignature (TimeSignature { (i32) hostTimeSigNumerator, (i32) hostTimeSigDenominator });

            result.setFrameRate (AudioPlayHead::fpsUnknown);

            return result;
        }

    private:
        b8 canControlTransportImpl() const { return impl.interAppAudioConnected; }

        Pimpl& impl;
    };

    //==============================================================================
   #if DRX_MODULE_AVAILABLE_drx_graphics
    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    Image getIcon (i32 size)
    {
       #if TARGET_OS_MACCATALYST
        if (@available (macCatalyst 14.0, *))
       #endif
        {
            if (interAppAudioConnected)
            {
                if (UIImage* hostUIImage = AudioOutputUnitGetHostIcon (audioUnit, (f32) size))
                    return drx_createImageFromUIImage (hostUIImage);
            }
        }

        return {};
    }
    DRX_END_IGNORE_DEPRECATION_WARNINGS
   #endif

    z0 switchApplication()
    {
        if (! interAppAudioConnected)
            return;

        CFURLRef hostUrl;
        UInt32 dataSize = sizeof (hostUrl);
        OSStatus err = AudioUnitGetProperty (audioUnit,
                                             kAudioUnitProperty_PeerURL,
                                             kAudioUnitScope_Global,
                                             0,
                                             &hostUrl,
                                             &dataSize);
        if (err == noErr)
        {
            [[UIApplication sharedApplication] openURL: (NSURL*) hostUrl
                                               options: @{}
                                     completionHandler: nil];
        }
    }

    //==============================================================================
    z0 invokeAudioDeviceErrorCallback (const Txt& reason)
    {
        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
            callback->audioDeviceError (reason);
    }

    z0 handleStatusChange (b8 enabled, tukk reason)
    {
        const ScopedLock myScopedLock (callbackLock);

        DRX_IOS_AUDIO_LOG ("handleStatusChange: enabled: " << (i32) enabled << ", reason: " << reason);

        isRunning = enabled;
        setAudioSessionActive (enabled);

        if (enabled)
            AudioOutputUnitStart (audioUnit);
        else
            AudioOutputUnitStop (audioUnit);

        if (! enabled)
            invokeAudioDeviceErrorCallback (reason);
    }

    z0 handleRouteChange (AVAudioSessionRouteChangeReason reason)
    {
        const ScopedLock myScopedLock (callbackLock);

        const Txt reasonString (getRoutingChangeReason (reason));
        DRX_IOS_AUDIO_LOG ("handleRouteChange: " << reasonString);

        if (isRunning)
            invokeAudioDeviceErrorCallback (reasonString);

        switch (reason)
        {
        case AVAudioSessionRouteChangeReasonCategoryChange:
        case AVAudioSessionRouteChangeReasonRouteConfigurationChange:
            break;
        case AVAudioSessionRouteChangeReasonOverride:
        case AVAudioSessionRouteChangeReasonUnknown:
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
        case AVAudioSessionRouteChangeReasonWakeFromSleep:
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory:
        {
            hardwareInfoNeedsUpdating = true;
            triggerAsyncUpdate();
            break;
        }

        // No default so the code doesn't compile if this enum is extended.
        }
    }

    z0 handleAudioUnitPropertyChange (AudioUnit,
                                        AudioUnitPropertyID propertyID,
                                        [[maybe_unused]] AudioUnitScope scope,
                                        [[maybe_unused]] AudioUnitElement element)
    {
        DRX_IOS_AUDIO_LOG ("handleAudioUnitPropertyChange: propertyID: " << Txt (propertyID)
                                                            << " scope: " << Txt (scope)
                                                          << " element: " << Txt (element));

        switch (propertyID)
        {
            case kAudioUnitProperty_IsInterAppConnected:
                handleInterAppAudioConnectionChange();
                return;
            case kAudioUnitProperty_StreamFormat:
                handleStreamFormatChange();
                return;
            default:
                jassertfalse;
        }
    }

    z0 handleInterAppAudioConnectionChange()
    {
        UInt32 connected;
        UInt32 dataSize = sizeof (connected);
        [[maybe_unused]] OSStatus err = AudioUnitGetProperty (audioUnit, kAudioUnitProperty_IsInterAppConnected,
                                                              kAudioUnitScope_Global, 0, &connected, &dataSize);
        jassert (err == noErr);

        DRX_IOS_AUDIO_LOG ("handleInterAppAudioConnectionChange: " << (connected ? "connected"
                                                                                  : "disconnected"));

        if (connected != interAppAudioConnected)
        {
            const ScopedLock myScopedLock (callbackLock);

            interAppAudioConnected = connected;

            UIApplicationState appstate = [UIApplication sharedApplication].applicationState;
            b8 inForeground = (appstate != UIApplicationStateBackground);

            if (interAppAudioConnected || inForeground)
            {
                setAudioSessionActive (true);
                AudioOutputUnitStart (audioUnit);

                if (callback != nullptr)
                    callback->audioDeviceAboutToStart (&owner);
            }
            else if (! inForeground)
            {
                AudioOutputUnitStop (audioUnit);
                setAudioSessionActive (false);

                if (callback != nullptr)
                    callback->audioDeviceStopped();
            }
        }
    }

    //==============================================================================
    OSStatus process (AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
                      const UInt32 numFrames, AudioBufferList* data)
    {
        // If you hit this assertion please contact the DRX team and let us
        // know the iOS version/device and audio device that you're using
        jassert (bufferSize == (i32) numFrames);

        OSStatus err = noErr;

        recordXruns (time, numFrames);

        const b8 useInput = channelData.areInputChannelsAvailable();

        if (useInput)
            err = AudioUnitRender (audioUnit, flags, time, 1, numFrames, data);

        const auto channelDataSize = sizeof (f32) * numFrames;

        const ScopedTryLock stl (callbackLock);

        if (stl.isLocked() && callback != nullptr)
        {
            if ((i32) numFrames > channelData.getFloatBufferSize())
                channelData.setFloatBufferSize ((i32) numFrames);

            f32* const* const inputData = channelData.audioData.getArrayOfWritePointers();
            f32* const* const outputData = inputData + channelData.inputs->numActiveChannels;

            if (useInput)
            {
                for (i32 c = 0; c < channelData.inputs->numActiveChannels; ++c)
                {
                    auto channelIndex = channelData.inputs->activeChannelIndices[c];
                    memcpy (inputData[c], (f32*) data->mBuffers[channelIndex].mData, channelDataSize);
                }
            }
            else
            {
                for (i32 c = 0; c < channelData.inputs->numActiveChannels; ++c)
                    zeromem (inputData[c], channelDataSize);
            }

            const auto nanos = time != nullptr ? timeConversions.hostTimeToNanos (time->mHostTime) : 0;

            callback->audioDeviceIOCallbackWithContext ((const f32**) inputData,
                                                        channelData.inputs ->numActiveChannels,
                                                        outputData,
                                                        channelData.outputs->numActiveChannels,
                                                        (i32) numFrames,
                                                        { (time != nullptr && (time->mFlags & kAudioTimeStampHostTimeValid) != 0) ? &nanos : nullptr });

            for (i32 c = 0; c < channelData.outputs->numActiveChannels; ++c)
            {
                auto channelIndex = channelData.outputs->activeChannelIndices[c];
                memcpy (data->mBuffers[channelIndex].mData, outputData[c], channelDataSize);
            }

            for (auto c : channelData.outputs->inactiveChannelIndices)
                zeromem (data->mBuffers[c].mData, channelDataSize);
        }
        else
        {
            for (u32 c = 0; c < data->mNumberBuffers; ++c)
                zeromem (data->mBuffers[c].mData, channelDataSize);
        }

        return err;
    }

    z0 recordXruns (const AudioTimeStamp* time, UInt32 numFrames)
    {
        if (time != nullptr && (time->mFlags & kAudioTimeStampSampleTimeValid) != 0)
        {
            if (! firstHostTime)
            {
                if (! approximatelyEqual ((time->mSampleTime - lastSampleTime), (f64) lastNumFrames))
                    xrun++;
            }
            else
                firstHostTime = false;

            lastSampleTime = time->mSampleTime;
        }
        else
            firstHostTime = true;

        lastNumFrames = numFrames;
    }

    //==============================================================================
    static OSStatus processStatic (uk client, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
                                   UInt32 /*busNumber*/, UInt32 numFrames, AudioBufferList* data)
    {
        return static_cast<Pimpl*> (client)->process (flags, time, numFrames, data);
    }

    //==============================================================================
    b8 createAudioUnit()
    {
        DRX_IOS_AUDIO_LOG ("Creating the audio unit");

        if (audioUnit != nullptr)
        {
            AudioComponentInstanceDispose (audioUnit);
            audioUnit = nullptr;
        }

        AudioComponentDescription desc;
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_RemoteIO;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        desc.componentFlags = 0;
        desc.componentFlagsMask = 0;

        AudioComponent comp = AudioComponentFindNext (nullptr, &desc);
        AudioComponentInstanceNew (comp, &audioUnit);

        if (audioUnit == nullptr)
            return false;

       #if DrxPlugin_Enable_IAA
        AudioComponentDescription appDesc;
        appDesc.componentType = DrxPlugin_IAAType;
        appDesc.componentSubType = DrxPlugin_IAASubType;
        appDesc.componentManufacturer = DrxPlugin_ManufacturerCode;
        appDesc.componentFlags = 0;
        appDesc.componentFlagsMask = 0;
        OSStatus err = AudioOutputUnitPublish (&appDesc,
                                               CFSTR (DrxPlugin_IAAName),
                                               DrxPlugin_VersionCode,
                                               audioUnit);

        // This assert will be hit if the Inter-App Audio entitlement has not
        // been enabled, or the description being published with
        // AudioOutputUnitPublish is different from any in the AudioComponents
        // array in this application's .plist file.
        jassert (err == noErr);

        err = AudioUnitAddPropertyListener (audioUnit,
                                            kAudioUnitProperty_IsInterAppConnected,
                                            dispatchAudioUnitPropertyChange,
                                            this);
        jassert (err == noErr);

        AudioOutputUnitMIDICallbacks midiCallbacks;
        midiCallbacks.userData = this;
        midiCallbacks.MIDIEventProc = midiEventCallback;
        midiCallbacks.MIDISysExProc = midiSysExCallback;
        err = AudioUnitSetProperty (audioUnit,
                                    kAudioOutputUnitProperty_MIDICallbacks,
                                    kAudioUnitScope_Global,
                                    0,
                                    &midiCallbacks,
                                    sizeof (midiCallbacks));
        jassert (err == noErr);
       #endif

        if (channelData.areInputChannelsAvailable())
        {
            const UInt32 one = 1;
            AudioUnitSetProperty (audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &one, sizeof (one));
        }

        {
            AURenderCallbackStruct inputProc;
            inputProc.inputProc = processStatic;
            inputProc.inputProcRefCon = this;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &inputProc, sizeof (inputProc));
        }

        {
            AudioStreamBasicDescription format;
            zerostruct (format);
            format.mSampleRate = sampleRate;
            format.mFormatID = kAudioFormatLinearPCM;
            format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsPacked;
            format.mBitsPerChannel = 8 * sizeof (f32);
            format.mFramesPerPacket = 1;
            format.mChannelsPerFrame = (UInt32) jmax (channelData.inputs->numHardwareChannels, channelData.outputs->numHardwareChannels);
            format.mBytesPerFrame = format.mBytesPerPacket = sizeof (f32);

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,  0, &format, sizeof (format));
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &format, sizeof (format));
        }

        AudioUnitInitialize (audioUnit);

        {
            // Querying the kAudioUnitProperty_MaximumFramesPerSlice property after calling AudioUnitInitialize
            // seems to be more reliable than calling it before.
            UInt32 framesPerSlice, dataSize = sizeof (framesPerSlice);

            if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_MaximumFramesPerSlice,
                                      kAudioUnitScope_Global, 0, &framesPerSlice, &dataSize) == noErr
                    && dataSize == sizeof (framesPerSlice)
                    && static_cast<i32> (framesPerSlice) != bufferSize)
            {
                DRX_IOS_AUDIO_LOG ("Internal buffer size: " << Txt (framesPerSlice));
                channelData.setFloatBufferSize (static_cast<i32> (framesPerSlice));
            }
        }

       #if DRX_AUDIOWORKGROUP_TYPES_AVAILABLE
        workgroup = [this]
        {
            UInt32 dataSize = sizeof (os_workgroup_t);
            os_workgroup_t wgHandle = nullptr;

            AudioUnitGetProperty (audioUnit, kAudioOutputUnitProperty_OSWorkgroup,
                                  kAudioUnitScope_Global, 0, &wgHandle, &dataSize);

            return makeRealAudioWorkgroup (wgHandle);
        }();
       #endif

        AudioUnitAddPropertyListener (audioUnit, kAudioUnitProperty_StreamFormat, dispatchAudioUnitPropertyChange, this);

        return true;
    }

    z0 fillHostCallbackInfo (HostCallbackInfo& callbackInfo)
    {
        zerostruct (callbackInfo);
        UInt32 dataSize = sizeof (HostCallbackInfo);
        [[maybe_unused]] OSStatus err = AudioUnitGetProperty (audioUnit,
                                                              kAudioUnitProperty_HostCallbacks,
                                                              kAudioUnitScope_Global,
                                                              0,
                                                              &callbackInfo,
                                                              &dataSize);
        jassert (err == noErr);
    }

    z0 handleAudioTransportEvent (AudioUnitRemoteControlEvent event)
    {
        [[maybe_unused]] OSStatus err = AudioUnitSetProperty (audioUnit, kAudioOutputUnitProperty_RemoteControlToHost,
                                                              kAudioUnitScope_Global, 0, &event, sizeof (event));
        jassert (err == noErr);
    }

    // If the routing is set to go through the receiver (i.e. the speaker, but quiet), this re-routes it
    // to make it loud. Needed because by default when using an input + output, the output is kept quiet.
    static z0 fixAudioRouteIfSetToReceiver()
    {
        auto session = [AVAudioSession sharedInstance];
        auto route = session.currentRoute;

        for (AVAudioSessionPortDescription* port in route.outputs)
        {
            if ([port.portName isEqualToString: @"Receiver"])
            {
                DRX_NSERROR_CHECK ([session overrideOutputAudioPort: AVAudioSessionPortOverrideSpeaker
                                                               error: &error]);
                setAudioSessionActive (true);
            }
        }
    }

    z0 restart()
    {
        const ScopedLock sl (callbackLock);

        if (isRunning)
        {
            if (audioUnit != nullptr)
            {
                AudioComponentInstanceDispose (audioUnit);
                audioUnit = nullptr;

                if (callback != nullptr)
                    callback->audioDeviceStopped();
            }

        }

        setTargetSampleRateAndBufferSize();
        updateHardwareInfo();

        if (isRunning)
        {
            channelData.reconfigure (requestedInputChannels, requestedOutputChannels);

            createAudioUnit();

            if (audioUnit != nullptr)
            {
                isRunning = true;

                if (callback != nullptr)
                    callback->audioDeviceAboutToStart (&owner);

                AudioOutputUnitStart (audioUnit);
            }
        }
    }

    z0 handleAsyncUpdate() override
    {
        restart();
    }

    z0 handleStreamFormatChange()
    {
        AudioStreamBasicDescription desc;
        zerostruct (desc);
        UInt32 dataSize = sizeof (desc);
        AudioUnitGetProperty (audioUnit,
                              kAudioUnitProperty_StreamFormat,
                              kAudioUnitScope_Output,
                              0,
                              &desc,
                              &dataSize);

        if (! approximatelyEqual (desc.mSampleRate, 0.0) && ! approximatelyEqual (desc.mSampleRate, sampleRate))
        {
            DRX_IOS_AUDIO_LOG ("Stream format has changed: Sample rate " << desc.mSampleRate);
            triggerAsyncUpdate();
        }
    }

    static z0 dispatchAudioUnitPropertyChange (uk data, AudioUnit unit, AudioUnitPropertyID propertyID,
                                                 AudioUnitScope scope, AudioUnitElement element)
    {
        static_cast<Pimpl*> (data)->handleAudioUnitPropertyChange (unit, propertyID, scope, element);
    }

    static f64 getTimestampForMIDI()
    {
        return Time::getMillisecondCounter() / 1000.0;
    }

    static z0 midiEventCallback (uk client, UInt32 status, UInt32 data1, UInt32 data2, UInt32)
    {
        return static_cast<Pimpl*> (client)->handleMidiMessage (MidiMessage ((i32) status,
                                                                             (i32) data1,
                                                                             (i32) data2,
                                                                             getTimestampForMIDI()));
    }

    static z0 midiSysExCallback (uk client, const UInt8 *data, UInt32 length)
    {
        return static_cast<Pimpl*> (client)->handleMidiMessage (MidiMessage (data, (i32) length, getTimestampForMIDI()));
    }

    z0 handleMidiMessage (MidiMessage msg)
    {
        if (messageCollector != nullptr)
            messageCollector->addMessageToQueue (msg);
    }

    struct IOChannelData
    {
        class IOChannelConfig
        {
        public:
            IOChannelConfig (const b8 isInput, const BigInteger requiredChannels)
                : hardwareChannelNames (getHardwareChannelNames (isInput)),
                  numHardwareChannels (hardwareChannelNames.size()),
                  areChannelsAccessible ((! isInput) || [AVAudioSession sharedInstance].isInputAvailable),
                  activeChannels (limitRequiredChannelsToHardware (numHardwareChannels, requiredChannels)),
                  numActiveChannels (activeChannels.countNumberOfSetBits()),
                  activeChannelIndices (getActiveChannelIndices (activeChannels)),
                  inactiveChannelIndices (getInactiveChannelIndices (activeChannelIndices, numHardwareChannels))
            {
               #if DRX_IOS_AUDIO_LOGGING
                {
                    Txt info;

                    info << "Number of hardware channels: " << numHardwareChannels
                         << ", Hardware channel names:";

                    for (auto& name : hardwareChannelNames)
                        info << " \"" << name << "\"";

                    info << ", Are channels available: " << (areChannelsAccessible ? "yes" : "no")
                         << ", Active channel indices:";

                    for (auto i : activeChannelIndices)
                        info << " " << i;

                    info << ", Inactive channel indices:";

                    for (auto i : inactiveChannelIndices)
                        info << " " << i;

                    DRX_IOS_AUDIO_LOG ((isInput ? "Input" : "Output") << " channel configuration: {" << info << "}");
                }
               #endif
            }

            const StringArray hardwareChannelNames;
            i32k numHardwareChannels;
            const b8 areChannelsAccessible;

            const BigInteger activeChannels;
            i32k numActiveChannels;

            const Array<i32> activeChannelIndices, inactiveChannelIndices;

        private:
            static StringArray getHardwareChannelNames (const b8 isInput)
            {
                StringArray result;

                auto route = [AVAudioSession sharedInstance].currentRoute;

                for (AVAudioSessionPortDescription* port in (isInput ? route.inputs : route.outputs))
                {
                    for (AVAudioSessionChannelDescription* desc in port.channels)
                        result.add (nsStringToDrx (desc.channelName));
                }

                // A fallback for the iOS simulator and older iOS versions
                if (result.isEmpty())
                    return { "Left", "Right" };

                return result;
            }

            static BigInteger limitRequiredChannelsToHardware (i32k numHardwareChannelsAvailable,
                                                               BigInteger requiredChannels)
            {
                requiredChannels.setRange (numHardwareChannelsAvailable,
                                           requiredChannels.getHighestBit() + 1,
                                           false);

                return requiredChannels;
            }

            static Array<i32> getActiveChannelIndices (const BigInteger activeChannelsToIndex)
            {
                Array<i32> result;

                auto index = activeChannelsToIndex.findNextSetBit (0);

                while (index != -1)
                {
                    result.add (index);
                    index = activeChannelsToIndex.findNextSetBit (++index);
                }

                return result;
            }

            static Array<i32> getInactiveChannelIndices (const Array<i32>& activeIndices, i32 numChannels)
            {
                Array<i32> result;

                auto nextActiveChannel = activeIndices.begin();

                for (i32 i = 0; i < numChannels; ++i)
                    if (nextActiveChannel != activeIndices.end() && i == *nextActiveChannel)
                        ++nextActiveChannel;
                    else
                        result.add (i);

                return result;
            }
        };

        z0 reconfigure (const BigInteger requiredInputChannels,
                          const BigInteger requiredOutputChannels)
        {
            inputs .reset (new IOChannelConfig (true,  requiredInputChannels));
            outputs.reset (new IOChannelConfig (false, requiredOutputChannels));

            audioData.setSize (inputs->numActiveChannels + outputs->numActiveChannels,
                               audioData.getNumSamples());
        }

        i32 getFloatBufferSize() const
        {
            return audioData.getNumSamples();
        }

        z0 setFloatBufferSize (i32k newSize)
        {
            audioData.setSize (audioData.getNumChannels(), newSize);
        }

        b8 areInputChannelsAvailable() const
        {
            return inputs->areChannelsAccessible && inputs->numActiveChannels > 0;
        }

        std::unique_ptr<IOChannelConfig> inputs;
        std::unique_ptr<IOChannelConfig> outputs;

        AudioBuffer<f32> audioData { 0, 0 };
    };

    CoreAudioTimeConversions timeConversions;

    IOChannelData channelData;

    BigInteger requestedInputChannels, requestedOutputChannels;

    b8 isRunning = false;

    AudioIODeviceCallback* callback = nullptr;

    Txt lastError;

   #if TARGET_IPHONE_SIMULATOR
    static constexpr i32 defaultBufferSize = 512;
   #else
    static constexpr i32 defaultBufferSize = 256;
   #endif
    i32 targetBufferSize = defaultBufferSize, bufferSize = targetBufferSize;

    f64 targetSampleRate = 44100.0, sampleRate = targetSampleRate;

    Array<f64> availableSampleRates;
    Array<i32> availableBufferSizes;

    static inline LruCache<Txt, Array<f64>> deviceSampleRatesCache;

    b8 interAppAudioConnected = false;

    MidiMessageCollector* messageCollector = nullptr;

    WeakReference<iOSAudioIODeviceType> deviceType;
    iOSAudioIODevice& owner;

    CriticalSection callbackLock;

    Atomic<b8> hardwareInfoNeedsUpdating { true };

    AudioUnit audioUnit {};
    AudioWorkgroup workgroup;

    SharedResourcePointer<AudioSessionHolder> sessionHolder;

    b8 firstHostTime;
    Float64 lastSampleTime;
    u32 lastNumFrames;
    i32 xrun;
    PlayHead playhead { *this };

    DRX_DECLARE_NON_COPYABLE (Pimpl)
};

//==============================================================================
iOSAudioIODevice::iOSAudioIODevice (iOSAudioIODeviceType* ioDeviceType, const Txt&, const Txt&)
    : AudioIODevice (iOSAudioDeviceName, iOSAudioDeviceName),
      pimpl (new Pimpl (ioDeviceType, *this))
{
}

//==============================================================================
Txt iOSAudioIODevice::open (const BigInteger& inChans, const BigInteger& outChans,
                               f64 requestedSampleRate, i32 requestedBufferSize)
{
    return pimpl->open (inChans, outChans, requestedSampleRate, requestedBufferSize);
}

z0 iOSAudioIODevice::close()                                      { pimpl->close(); }

z0 iOSAudioIODevice::start (AudioIODeviceCallback* callbackToUse) { pimpl->start (callbackToUse); }
z0 iOSAudioIODevice::stop()                                       { pimpl->stop(); }

Array<f64> iOSAudioIODevice::getAvailableSampleRates()           { return pimpl->availableSampleRates; }
Array<i32> iOSAudioIODevice::getAvailableBufferSizes()              { return pimpl->availableBufferSizes; }

b8 iOSAudioIODevice::setAudioPreprocessingEnabled (b8 enabled)  { return pimpl->setAudioPreprocessingEnabled (enabled); }

b8 iOSAudioIODevice::isPlaying()                                  { return pimpl->isRunning && pimpl->callback != nullptr; }
b8 iOSAudioIODevice::isOpen()                                     { return pimpl->isRunning; }
Txt iOSAudioIODevice::getLastError()                             { return pimpl->lastError; }

StringArray iOSAudioIODevice::getOutputChannelNames()               { return pimpl->channelData.outputs->hardwareChannelNames; }
StringArray iOSAudioIODevice::getInputChannelNames()                { return pimpl->channelData.inputs->areChannelsAccessible ? pimpl->channelData.inputs->hardwareChannelNames : StringArray(); }

i32 iOSAudioIODevice::getDefaultBufferSize()                        { return pimpl->defaultBufferSize; }
i32 iOSAudioIODevice::getCurrentBufferSizeSamples()                 { return pimpl->bufferSize; }

f64 iOSAudioIODevice::getCurrentSampleRate()                     { return pimpl->sampleRate; }

i32 iOSAudioIODevice::getCurrentBitDepth()                          { return 16; }

BigInteger iOSAudioIODevice::getActiveInputChannels() const         { return pimpl->channelData.inputs->activeChannels; }
BigInteger iOSAudioIODevice::getActiveOutputChannels() const        { return pimpl->channelData.outputs->activeChannels; }

i32 iOSAudioIODevice::getInputLatencyInSamples()                    { return roundToInt (pimpl->sampleRate * [AVAudioSession sharedInstance].inputLatency); }
i32 iOSAudioIODevice::getOutputLatencyInSamples()                   { return roundToInt (pimpl->sampleRate * [AVAudioSession sharedInstance].outputLatency); }
i32 iOSAudioIODevice::getXRunCount() const noexcept                 { return pimpl->xrun; }
AudioWorkgroup iOSAudioIODevice::getWorkgroup() const               { return pimpl->workgroup; }

z0 iOSAudioIODevice::setMidiMessageCollector (MidiMessageCollector* collector) { pimpl->messageCollector = collector; }
AudioPlayHead* iOSAudioIODevice::getAudioPlayHead() const           { return &pimpl->playhead; }

b8 iOSAudioIODevice::isInterAppAudioConnected() const             { return pimpl->interAppAudioConnected; }
#if DRX_MODULE_AVAILABLE_drx_graphics
Image iOSAudioIODevice::getIcon (i32 size)                          { return pimpl->getIcon (size); }
#endif
z0 iOSAudioIODevice::switchApplication()                          { return pimpl->switchApplication(); }

//==============================================================================
iOSAudioIODeviceType::iOSAudioIODeviceType()
    : AudioIODeviceType (iOSAudioDeviceName)
{
    sessionHolder->activeDeviceTypes.add (this);
}

iOSAudioIODeviceType::~iOSAudioIODeviceType()
{
    sessionHolder->activeDeviceTypes.removeFirstMatchingValue (this);
}

// The list of devices is updated automatically
z0 iOSAudioIODeviceType::scanForDevices() {}
StringArray iOSAudioIODeviceType::getDeviceNames (b8) const             { return { iOSAudioDeviceName }; }
i32 iOSAudioIODeviceType::getDefaultDeviceIndex (b8) const              { return 0; }
i32 iOSAudioIODeviceType::getIndexOfDevice (AudioIODevice*, b8) const   { return 0; }
b8 iOSAudioIODeviceType::hasSeparateInputsAndOutputs() const            { return false; }

AudioIODevice* iOSAudioIODeviceType::createDevice (const Txt& outputDeviceName, const Txt& inputDeviceName)
{
    return new iOSAudioIODevice (this, outputDeviceName, inputDeviceName);
}

z0 iOSAudioIODeviceType::handleRouteChange (AVAudioSessionRouteChangeReason)
{
    triggerAsyncUpdate();
}

z0 iOSAudioIODeviceType::handleAsyncUpdate()
{
    callDeviceChangeListeners();
}

//==============================================================================
AudioSessionHolder::AudioSessionHolder()    { nativeSession = [[iOSAudioSessionNative alloc] init: this]; }
AudioSessionHolder::~AudioSessionHolder()   { [nativeSession release]; }

z0 AudioSessionHolder::handleStatusChange (b8 enabled, tukk reason) const
{
    for (auto device: activeDevices)
        device->handleStatusChange (enabled, reason);
}

z0 AudioSessionHolder::handleRouteChange (AVAudioSessionRouteChangeReason reason)
{
    for (auto device: activeDevices)
        device->handleRouteChange (reason);

    for (auto deviceType: activeDeviceTypes)
        deviceType->handleRouteChange (reason);
}

#undef DRX_NSERROR_CHECK

} // namespace drx
