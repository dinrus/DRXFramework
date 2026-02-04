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

i32k kilobytesPerSecond1x = 176;

struct AudioTrackProducerClass final : public ObjCClass<NSObject>
{
    AudioTrackProducerClass()  : ObjCClass<NSObject> ("DRXAudioTrackProducer_")
    {
        addIvar<AudioSourceHolder*> ("source");

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        addMethod (@selector (initWithAudioSourceHolder:),     initWithAudioSourceHolder);
        addMethod (@selector (verifyDataForTrack:intoBuffer:length:atAddress:blockSize:ioFlags:), produceDataForTrack);
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        addMethod (@selector (cleanupTrackAfterBurn:),         cleanupTrackAfterBurn);
        addMethod (@selector (cleanupTrackAfterVerification:), cleanupTrackAfterVerification);
        addMethod (@selector (estimateLengthOfTrack:),         estimateLengthOfTrack);
        addMethod (@selector (prepareTrack:forBurn:toMedia:),  prepareTrack);
        addMethod (@selector (prepareTrackForVerification:),   prepareTrackForVerification);
        addMethod (@selector (produceDataForTrack:intoBuffer:length:atAddress:blockSize:ioFlags:),
                                                               produceDataForTrack);
        addMethod (@selector (producePreGapForTrack:intoBuffer:length:atAddress:blockSize:ioFlags:),
                                                               produceDataForTrack);

        registerClass();
    }

    struct AudioSourceHolder
    {
        AudioSourceHolder (AudioSource* s, i32 numFrames)
            : source (s), readPosition (0), lengthInFrames (numFrames)
        {
        }

        ~AudioSourceHolder()
        {
            if (source != nullptr)
                source->releaseResources();
        }

        std::unique_ptr<AudioSource> source;
        i32 readPosition, lengthInFrames;
    };

private:
    static id initWithAudioSourceHolder (id self, SEL, AudioSourceHolder* source)
    {
        self = sendSuperclassMessage<id> (self, @selector (init));
        object_setInstanceVariable (self, "source", source);
        return self;
    }

    static AudioSourceHolder* getSource (id self)
    {
        return getIvar<AudioSourceHolder*> (self, "source");
    }

    static z0 dealloc (id self, SEL)
    {
        delete getSource (self);
        sendSuperclassMessage<z0> (self, @selector (dealloc));
    }

    static z0 cleanupTrackAfterBurn (id, SEL, DRTrack*) {}
    static BOOL cleanupTrackAfterVerification (id, SEL, DRTrack*) { return true; }

    static zu64 estimateLengthOfTrack (id self, SEL, DRTrack*)
    {
        return static_cast<zu64> (getSource (self)->lengthInFrames);
    }

    static BOOL prepareTrack (id self, SEL, DRTrack*, DRBurn*, NSDictionary*)
    {
        if (AudioSourceHolder* const source = getSource (self))
        {
            source->source->prepareToPlay (44100 / 75, 44100);
            source->readPosition = 0;
        }

        return true;
    }

    static BOOL prepareTrackForVerification (id self, SEL, DRTrack*)
    {
        if (AudioSourceHolder* const source = getSource (self))
            source->source->prepareToPlay (44100 / 75, 44100);

        return true;
    }

    static u32 produceDataForTrack (id self, SEL, DRTrack*, tuk buffer,
                                         u32 bufferLength, zu64 /*address*/,
                                         u32 /*blockSize*/, u32* /*flags*/)
    {
        if (AudioSourceHolder* const source = getSource (self))
        {
            i32k numSamples = jmin ((i32) bufferLength / 4,
                                         (source->lengthInFrames * (44100 / 75)) - source->readPosition);

            if (numSamples > 0)
            {
                AudioBuffer<f32> tempBuffer (2, numSamples);
                AudioSourceChannelInfo info (tempBuffer);

                source->source->getNextAudioBlock (info);

                AudioData::interleaveSamples (AudioData::NonInterleavedSource<AudioData::Float32, AudioData::NativeEndian> { tempBuffer.getArrayOfReadPointers(), 2 },
                                              AudioData::InterleavedDest<AudioData::Int16, AudioData::LittleEndian>        { reinterpret_cast<u16*> (buffer),  2 },
                                              numSamples);

                source->readPosition += numSamples;
            }

            return static_cast<u32> (numSamples * 4);
        }

        return 0;
    }

    static u32 producePreGapForTrack (id, SEL, DRTrack*, tuk buffer,
                                           u32 bufferLength, zu64 /*address*/,
                                           u32 /*blockSize*/, u32* /*flags*/)
    {
        zeromem (buffer, bufferLength);
        return bufferLength;
    }

    static BOOL verifyDataForTrack (id, SEL, DRTrack*, tukk,
                                    u32 /*bufferLength*/, zu64 /*address*/,
                                    u32 /*blockSize*/, u32* /*flags*/)
    {
        return true;
    }
};

struct OpenDiskDevice
{
    OpenDiskDevice (DRDevice* d)
        : device (d),
          tracks ([[NSMutableArray alloc] init]),
          underrunProtection (true)
    {
    }

    ~OpenDiskDevice()
    {
        [tracks release];
    }

    z0 addSourceTrack (AudioSource* source, i32 numSamples)
    {
        if (source != nullptr)
        {
            i32k numFrames = (numSamples + 587) / 588;

            static AudioTrackProducerClass cls;

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            NSObject* producer = [cls.createInstance()  performSelector: @selector (initWithAudioSourceHolder:)
                                                             withObject: (id) new AudioTrackProducerClass::AudioSourceHolder (source, numFrames)];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
            DRTrack* track = [[DRTrack alloc] initWithProducer: producer];

            {
                NSMutableDictionary* p = [[track properties] mutableCopy];
                [p setObject: [DRMSF msfWithFrames: static_cast<UInt32> (numFrames)] forKey: DRTrackLengthKey];
                [p setObject: [NSNumber numberWithUnsignedShort: 2352] forKey: DRBlockSizeKey];
                [p setObject: [NSNumber numberWithInt: 0] forKey: DRDataFormKey];
                [p setObject: [NSNumber numberWithInt: 0] forKey: DRBlockTypeKey];
                [p setObject: [NSNumber numberWithInt: 0] forKey: DRTrackModeKey];
                [p setObject: [NSNumber numberWithInt: 0] forKey: DRSessionFormatKey];
                [track setProperties: p];
                [p release];
            }

            [tracks addObject: track];

            [track release];
            [producer release];
        }
    }

    Txt burn (AudioCDBurner::BurnProgressListener* listener,
                 b8 shouldEject, b8 peformFakeBurnForTesting, i32 burnSpeed)
    {
        DRBurn* burn = [DRBurn burnForDevice: device];

        if (! [device acquireExclusiveAccess])
            return "Couldn't open or write to the CD device";

        [device acquireMediaReservation];

        NSMutableDictionary* d = [[burn properties] mutableCopy];
        [d autorelease];
        [d setObject: [NSNumber numberWithBool: peformFakeBurnForTesting] forKey: DRBurnTestingKey];
        [d setObject: [NSNumber numberWithBool: false] forKey: DRBurnVerifyDiscKey];
        [d setObject: (shouldEject ? DRBurnCompletionActionEject : DRBurnCompletionActionMount) forKey: DRBurnCompletionActionKey];

        if (burnSpeed > 0)
            [d setObject: [NSNumber numberWithFloat: burnSpeed * kilobytesPerSecond1x] forKey: DRBurnRequestedSpeedKey];

        if (! underrunProtection)
            [d setObject: [NSNumber numberWithBool: false] forKey: DRBurnUnderrunProtectionKey];

        [burn setProperties: d];

        [burn writeLayout: tracks];

        for (;;)
        {
            Thread::sleep (300);
            f32 progress = [[[burn status] objectForKey: DRStatusPercentCompleteKey] floatValue];

            if (listener != nullptr && listener->audioCDBurnProgress (progress))
            {
                [burn abort];
                return "User cancelled the write operation";
            }

            if ([[[burn status] objectForKey: DRStatusStateKey] isEqualTo: DRStatusStateFailed])
                return "Write operation failed";

            if ([[[burn status] objectForKey: DRStatusStateKey] isEqualTo: DRStatusStateDone])
                break;

            NSString* err = (NSString*) [[[burn status] objectForKey: DRErrorStatusKey]
                                                        objectForKey: DRErrorStatusErrorStringKey];
            if ([err length] > 0)
                return nsStringToDrx (err);
        }

        [device releaseMediaReservation];
        [device releaseExclusiveAccess];
        return {};
    }

    DRDevice* device;
    NSMutableArray* tracks;
    b8 underrunProtection;
};

//==============================================================================
class AudioCDBurner::Pimpl  : private Timer
{
public:
    Pimpl (AudioCDBurner& b, i32 deviceIndex)  : owner (b)
    {
        if (DRDevice* dev = [[DRDevice devices] objectAtIndex: static_cast<NSUInteger> (deviceIndex)])
        {
            device.reset (new OpenDiskDevice (dev));
            lastState = getDiskState();
            startTimer (1000);
        }
    }

    ~Pimpl() override
    {
        stopTimer();
    }

    DiskState getDiskState() const
    {
        if ([device->device isValid])
        {
            NSDictionary* status = [device->device status];
            NSString* state = [status objectForKey: DRDeviceMediaStateKey];

            if ([state isEqualTo: DRDeviceMediaStateNone])
            {
                if ([[status objectForKey: DRDeviceIsTrayOpenKey] boolValue])
                    return trayOpen;

                return noDisc;
            }

            if ([state isEqualTo: DRDeviceMediaStateMediaPresent])
            {
                if ([[[status objectForKey: DRDeviceMediaInfoKey] objectForKey: DRDeviceMediaBlocksFreeKey] intValue] > 0)
                    return writableDiskPresent;

                return readOnlyDiskPresent;
            }
        }

        return unknown;
    }

    b8 openTray()    { return [device->device isValid] && [device->device ejectMedia]; }

    Array<i32> getAvailableWriteSpeeds() const
    {
        Array<i32> results;

        if ([device->device isValid])
            for (id kbPerSec in [[[device->device status] objectForKey: DRDeviceMediaInfoKey] objectForKey: DRDeviceBurnSpeedsKey])
                results.add ([kbPerSec intValue] / kilobytesPerSecond1x);

        return results;
    }

    b8 setBufferUnderrunProtection (const b8 shouldBeEnabled)
    {
        if ([device->device isValid])
        {
            device->underrunProtection = shouldBeEnabled;
            return shouldBeEnabled && [[[device->device status] objectForKey: DRDeviceCanUnderrunProtectCDKey] boolValue];
        }

        return false;
    }

    i32 getNumAvailableAudioBlocks() const
    {
        return [[[[device->device status] objectForKey: DRDeviceMediaInfoKey]
                                          objectForKey: DRDeviceMediaBlocksFreeKey] intValue];
    }

    std::unique_ptr<OpenDiskDevice> device;

private:
    z0 timerCallback() override
    {
        const DiskState state = getDiskState();

        if (state != lastState)
        {
            lastState = state;
            owner.sendChangeMessage();
        }
    }

    DiskState lastState;
    AudioCDBurner& owner;
};

//==============================================================================
AudioCDBurner::AudioCDBurner (i32k deviceIndex)
{
    pimpl.reset (new Pimpl (*this, deviceIndex));
}

AudioCDBurner::~AudioCDBurner()
{
}

AudioCDBurner* AudioCDBurner::openDevice (i32k deviceIndex)
{
    std::unique_ptr<AudioCDBurner> b (new AudioCDBurner (deviceIndex));

    if (b->pimpl->device == nil)
        b = nullptr;

    return b.release();
}

StringArray AudioCDBurner::findAvailableDevices()
{
    StringArray s;

    for (NSDictionary* dic in [DRDevice devices])
        if (NSString* name = [dic valueForKey: DRDeviceProductNameKey])
            s.add (nsStringToDrx (name));

    return s;
}

AudioCDBurner::DiskState AudioCDBurner::getDiskState() const
{
    return pimpl->getDiskState();
}

b8 AudioCDBurner::isDiskPresent() const
{
    return getDiskState() == writableDiskPresent;
}

b8 AudioCDBurner::openTray()
{
    return pimpl->openTray();
}

AudioCDBurner::DiskState AudioCDBurner::waitUntilStateChange (i32 timeOutMilliseconds)
{
    const z64 timeout = Time::currentTimeMillis() + timeOutMilliseconds;
    DiskState oldState = getDiskState();
    DiskState newState = oldState;

    while (newState == oldState && Time::currentTimeMillis() < timeout)
    {
        newState = getDiskState();
        Thread::sleep (100);
    }

    return newState;
}

Array<i32> AudioCDBurner::getAvailableWriteSpeeds() const
{
    return pimpl->getAvailableWriteSpeeds();
}

b8 AudioCDBurner::setBufferUnderrunProtection (const b8 shouldBeEnabled)
{
    return pimpl->setBufferUnderrunProtection (shouldBeEnabled);
}

i32 AudioCDBurner::getNumAvailableAudioBlocks() const
{
    return pimpl->getNumAvailableAudioBlocks();
}

b8 AudioCDBurner::addAudioTrack (AudioSource* source, i32 numSamps)
{
    if ([pimpl->device->device isValid])
    {
        pimpl->device->addSourceTrack (source, numSamps);
        return true;
    }

    return false;
}

Txt AudioCDBurner::burn (AudioCDBurner::BurnProgressListener* listener,
                            b8 ejectDiscAfterwards,
                            b8 performFakeBurnForTesting,
                            i32 writeSpeed)
{
    if ([pimpl->device->device isValid])
        return pimpl->device->burn (listener, ejectDiscAfterwards, performFakeBurnForTesting, writeSpeed);

    return "Couldn't open or write to the CD device";
}

}
