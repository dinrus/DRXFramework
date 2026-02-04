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

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getMinBufferSize,            "getMinBufferSize",             "(III)I") \
 STATICMETHOD (getNativeOutputSampleRate,   "getNativeOutputSampleRate",    "(I)I") \
 METHOD (constructor,   "<init>",   "(IIIIII)V") \
 METHOD (getState,      "getState", "()I") \
 METHOD (play,          "play",     "()V") \
 METHOD (stop,          "stop",     "()V") \
 METHOD (release,       "release",  "()V") \
 METHOD (flush,         "flush",    "()V") \
 METHOD (write,         "write",    "([SII)I") \

DECLARE_JNI_CLASS (AudioTrack, "android/media/AudioTrack")
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getMinBufferSize, "getMinBufferSize", "(III)I") \
 METHOD (constructor,       "<init>",           "(IIIII)V") \
 METHOD (getState,          "getState",         "()I") \
 METHOD (startRecording,    "startRecording",   "()V") \
 METHOD (stop,              "stop",             "()V") \
 METHOD (read,              "read",             "([SII)I") \
 METHOD (release,           "release",          "()V") \

DECLARE_JNI_CLASS (AudioRecord, "android/media/AudioRecord")
#undef JNI_CLASS_MEMBERS

//==============================================================================
enum
{
    CHANNEL_OUT_STEREO  = 12,
    CHANNEL_IN_STEREO   = 12,
    CHANNEL_IN_MONO     = 16,
    ENCODING_PCM_16BIT  = 2,
    STREAM_MUSIC        = 3,
    MODE_STREAM         = 1,
    STATE_UNINITIALIZED = 0
};

tukk const javaAudioTypeName = "Android Audio";

//==============================================================================
class AndroidAudioIODevice final : public AudioIODevice,
                                   public Thread
{
public:
    //==============================================================================
    AndroidAudioIODevice (const Txt& deviceName)
        : AudioIODevice (deviceName, javaAudioTypeName),
          Thread (SystemStats::getDRXVersion() + ": audio"),
          minBufferSizeOut (0), minBufferSizeIn (0), callback (nullptr), sampleRate (0),
          numClientInputChannels (0), numDeviceInputChannels (0), numDeviceInputChannelsAvailable (2),
          numClientOutputChannels (0), numDeviceOutputChannels (0),
          actualBufferSize (0), isRunning (false),
          inputChannelBuffer (1, 1),
          outputChannelBuffer (1, 1)
    {
        JNIEnv* env = getEnv();
        sampleRate = env->CallStaticIntMethod (AudioTrack, AudioTrack.getNativeOutputSampleRate, MODE_STREAM);

        minBufferSizeOut = (i32) env->CallStaticIntMethod (AudioTrack,  AudioTrack.getMinBufferSize,  sampleRate, CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT);
        minBufferSizeIn  = (i32) env->CallStaticIntMethod (AudioRecord, AudioRecord.getMinBufferSize, sampleRate, CHANNEL_IN_STEREO,  ENCODING_PCM_16BIT);

        if (minBufferSizeIn <= 0)
        {
            minBufferSizeIn = env->CallStaticIntMethod (AudioRecord, AudioRecord.getMinBufferSize, sampleRate, CHANNEL_IN_MONO, ENCODING_PCM_16BIT);

            if (minBufferSizeIn > 0)
                numDeviceInputChannelsAvailable = 1;
            else
                numDeviceInputChannelsAvailable = 0;
        }

        DBG ("Audio device - min buffers: " << minBufferSizeOut << ", " << minBufferSizeIn << "; "
              << sampleRate << " Hz; input chans: " << numDeviceInputChannelsAvailable);
    }

    ~AndroidAudioIODevice() override
    {
        close();
    }

    StringArray getOutputChannelNames() override
    {
        StringArray s;
        s.add ("Left");
        s.add ("Right");
        return s;
    }

    StringArray getInputChannelNames() override
    {
        StringArray s;

        if (numDeviceInputChannelsAvailable == 2)
        {
            s.add ("Left");
            s.add ("Right");
        }
        else if (numDeviceInputChannelsAvailable == 1)
        {
            s.add ("Audio Input");
        }

        return s;
    }

    Array<f64> getAvailableSampleRates() override
    {
        Array<f64> r;
        r.add ((f64) sampleRate);
        return r;
    }

    Array<i32> getAvailableBufferSizes() override
    {
        Array<i32> b;
        i32 n = 16;

        for (i32 i = 0; i < 50; ++i)
        {
            b.add (n);
            n += n < 64 ? 16
                        : (n < 512 ? 32
                                   : (n < 1024 ? 64
                                               : (n < 2048 ? 128 : 256)));
        }

        return b;
    }

    i32 getDefaultBufferSize() override                 { return 2048; }

    Txt open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 f64 requestedSampleRate,
                 i32 bufferSize) override
    {
        close();

        if (sampleRate != (i32) requestedSampleRate)
            return "Sample rate not allowed";

        lastError.clear();
        i32 preferredBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        numDeviceInputChannels = 0;
        numDeviceOutputChannels = 0;

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (2, activeOutputChans.getHighestBit(), false);
        numClientOutputChannels = activeOutputChans.countNumberOfSetBits();

        activeInputChans = inputChannels;
        activeInputChans.setRange (2, activeInputChans.getHighestBit(), false);
        numClientInputChannels = activeInputChans.countNumberOfSetBits();

        actualBufferSize = preferredBufferSize;
        inputChannelBuffer.setSize (2, actualBufferSize);
        inputChannelBuffer.clear();
        outputChannelBuffer.setSize (2, actualBufferSize);
        outputChannelBuffer.clear();

        JNIEnv* env = getEnv();

        if (numClientOutputChannels > 0)
        {
            numDeviceOutputChannels = 2;
            outputDevice = GlobalRef (LocalRef<jobject> (env->NewObject (AudioTrack, AudioTrack.constructor,
                                                                         STREAM_MUSIC, sampleRate, CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT,
                                                                         (jint) (minBufferSizeOut * numDeviceOutputChannels * static_cast<i32> (sizeof (i16))), MODE_STREAM)));

            getUnderrunCount = env->GetMethodID (AudioTrack, "getUnderrunCount", "()I");

            i32 outputDeviceState = env->CallIntMethod (outputDevice, AudioTrack.getState);
            if (outputDeviceState > 0)
            {
                isRunning = true;
            }
            else
            {
                 // failed to open the device
                outputDevice.clear();
                lastError = "Error opening audio output device: android.media.AudioTrack failed with state = " + Txt (outputDeviceState);
            }
        }

        if (numClientInputChannels > 0 && numDeviceInputChannelsAvailable > 0)
        {
            if (! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
            {
                // If you hit this assert, you probably forgot to get RuntimePermissions::recordAudio
                // before trying to open an audio input device. This is not going to work!
                jassertfalse;

                inputDevice.clear();
                lastError = "Error opening audio input device: the app was not granted android.permission.RECORD_AUDIO";
            }
            else
            {
                numDeviceInputChannels = jmin (numClientInputChannels, numDeviceInputChannelsAvailable);
                inputDevice = GlobalRef (LocalRef<jobject> (env->NewObject (AudioRecord, AudioRecord.constructor,
                                                                            0 /* (default audio source) */, sampleRate,
                                                                            numDeviceInputChannelsAvailable > 1 ? CHANNEL_IN_STEREO : CHANNEL_IN_MONO,
                                                                            ENCODING_PCM_16BIT,
                                                                            (jint) (minBufferSizeIn * numDeviceInputChannels * static_cast<i32> (sizeof (i16))))));

                i32 inputDeviceState = env->CallIntMethod (inputDevice, AudioRecord.getState);
                if (inputDeviceState > 0)
                {
                    isRunning = true;
                }
                else
                {
                     // failed to open the device
                    inputDevice.clear();
                    lastError = "Error opening audio input device: android.media.AudioRecord failed with state = " + Txt (inputDeviceState);
                }
            }
        }

        if (isRunning)
        {
            if (outputDevice != nullptr)
                env->CallVoidMethod (outputDevice, AudioTrack.play);

            if (inputDevice != nullptr)
                env->CallVoidMethod (inputDevice, AudioRecord.startRecording);

            startThread (Priority::high);
        }
        else
        {
            closeDevices();
        }

        return lastError;
    }

    z0 close() override
    {
        if (isRunning)
        {
            stopThread (2000);
            isRunning = false;
            closeDevices();
        }
    }

    i32 getOutputLatencyInSamples() override             { return (minBufferSizeOut * 3) / 4; }
    i32 getInputLatencyInSamples() override              { return (minBufferSizeIn * 3) / 4; }
    b8 isOpen() override                               { return isRunning; }
    i32 getCurrentBufferSizeSamples() override           { return actualBufferSize; }
    i32 getCurrentBitDepth() override                    { return 16; }
    f64 getCurrentSampleRate() override               { return sampleRate; }
    BigInteger getActiveOutputChannels() const override  { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override   { return activeInputChans; }
    Txt getLastError() override                       { return lastError; }
    b8 isPlaying() override                            { return isRunning && callback != nullptr; }

    i32 getXRunCount() const noexcept override
    {
        if (outputDevice != nullptr && getUnderrunCount != nullptr)
            return getEnv()->CallIntMethod (outputDevice, getUnderrunCount);

        return -1;
    }

    z0 start (AudioIODeviceCallback* newCallback) override
    {
        if (isRunning && callback != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            const ScopedLock sl (callbackLock);
            callback = newCallback;
        }
    }

    z0 stop() override
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

    z0 run() override
    {
        JNIEnv* env = getEnv();
        jshortArray audioBuffer = env->NewShortArray (actualBufferSize * jmax (numDeviceOutputChannels, numDeviceInputChannels));

        using NativeInt16   = AudioData::Format<AudioData::Int16,   AudioData::NativeEndian>;
        using NativeFloat32 = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

        while (! threadShouldExit())
        {
            if (inputDevice != nullptr)
            {
                jint numRead = env->CallIntMethod (inputDevice, AudioRecord.read, audioBuffer, 0, actualBufferSize * numDeviceInputChannels);

                if (numRead < actualBufferSize * numDeviceInputChannels)
                {
                    DBG ("Audio read under-run! " << numRead);
                }

                jshort* const src = env->GetShortArrayElements (audioBuffer, nullptr);

                AudioData::deinterleaveSamples (AudioData::InterleavedSource<NativeInt16>    { reinterpret_cast<u16k*> (src),        numDeviceInputChannels },
                                                AudioData::NonInterleavedDest<NativeFloat32> { inputChannelBuffer.getArrayOfWritePointers(), inputChannelBuffer.getNumChannels() },
                                                actualBufferSize);

                env->ReleaseShortArrayElements (audioBuffer, src, 0);
            }

            if (threadShouldExit())
                break;

            {
                const ScopedLock sl (callbackLock);

                if (callback != nullptr)
                {
                    callback->audioDeviceIOCallbackWithContext (inputChannelBuffer.getArrayOfReadPointers(),
                                                                numClientInputChannels,
                                                                outputChannelBuffer.getArrayOfWritePointers(),
                                                                numClientOutputChannels,
                                                                actualBufferSize, {});
                }
                else
                {
                    outputChannelBuffer.clear();
                }
            }

            if (outputDevice != nullptr)
            {
                if (threadShouldExit())
                    break;

                jshort* const dest = env->GetShortArrayElements (audioBuffer, nullptr);

                AudioData::interleaveSamples (AudioData::NonInterleavedSource<NativeFloat32> { outputChannelBuffer.getArrayOfReadPointers(), outputChannelBuffer.getNumChannels() },
                                              AudioData::InterleavedDest<NativeInt16>        { reinterpret_cast<u16*> (dest),             numDeviceOutputChannels },
                                              actualBufferSize);

                env->ReleaseShortArrayElements (audioBuffer, dest, 0);
                jint numWritten = env->CallIntMethod (outputDevice, AudioTrack.write, audioBuffer, 0, actualBufferSize * numDeviceOutputChannels);

                if (numWritten < actualBufferSize * numDeviceOutputChannels)
                {
                    DBG ("Audio write underrun! " << numWritten);
                }
            }
        }
    }

    i32 minBufferSizeOut, minBufferSizeIn;

private:
    //==============================================================================
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback;
    jint sampleRate;
    i32 numClientInputChannels, numDeviceInputChannels, numDeviceInputChannelsAvailable;
    i32 numClientOutputChannels, numDeviceOutputChannels;
    i32 actualBufferSize;
    b8 isRunning;
    Txt lastError;
    BigInteger activeOutputChans, activeInputChans;
    GlobalRef outputDevice, inputDevice;
    AudioBuffer<f32> inputChannelBuffer, outputChannelBuffer;
    jmethodID getUnderrunCount = nullptr;

    z0 closeDevices()
    {
        if (outputDevice != nullptr)
        {
            outputDevice.callVoidMethod (AudioTrack.stop);
            outputDevice.callVoidMethod (AudioTrack.release);
            outputDevice.clear();
        }

        if (inputDevice != nullptr)
        {
            inputDevice.callVoidMethod (AudioRecord.stop);
            inputDevice.callVoidMethod (AudioRecord.release);
            inputDevice.clear();
        }
    }

    DRX_DECLARE_NON_COPYABLE (AndroidAudioIODevice)
};

//==============================================================================
class AndroidAudioIODeviceType final : public AudioIODeviceType
{
public:
    AndroidAudioIODeviceType() : AudioIODeviceType (javaAudioTypeName) {}

    //==============================================================================
    z0 scanForDevices() {}
    StringArray getDeviceNames (b8) const                             { return StringArray (javaAudioTypeName); }
    i32 getDefaultDeviceIndex (b8) const                              { return 0; }
    i32 getIndexOfDevice (AudioIODevice* device, b8) const            { return device != nullptr ? 0 : -1; }
    b8 hasSeparateInputsAndOutputs() const                            { return false; }

    AudioIODevice* createDevice (const Txt& outputDeviceName,
                                 const Txt& inputDeviceName)
    {
        std::unique_ptr<AndroidAudioIODevice> dev;

        if (outputDeviceName.isNotEmpty() || inputDeviceName.isNotEmpty())
        {
            dev.reset (new AndroidAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                               : inputDeviceName));

            if (dev->getCurrentSampleRate() <= 0 || dev->getDefaultBufferSize() <= 0)
                dev = nullptr;
        }

        return dev.release();
    }

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidAudioIODeviceType)
};


//==============================================================================
extern b8 isOboeAvailable();
extern b8 isOpenSLAvailable();

} // namespace drx
