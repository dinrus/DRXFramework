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

#ifndef DRX_OBOE_LOG_ENABLED
 #define DRX_OBOE_LOG_ENABLED 1
#endif

#if DRX_OBOE_LOG_ENABLED
 #define DRX_OBOE_LOG(x) DBG(x)
#else
 #define DRX_OBOE_LOG(x) {}
#endif

namespace drx
{

template <typename OboeDataFormat>  struct OboeAudioIODeviceBufferHelpers {};

template <>
struct OboeAudioIODeviceBufferHelpers<i16>
{
    static oboe::AudioFormat oboeAudioFormat() { return oboe::AudioFormat::I16; }

    static constexpr i32 bitDepth() { return 16; }

    static b8 referAudioBufferDirectlyToOboeIfPossible (i16*, AudioBuffer<f32>&, i32)  { return false; }

    using NativeInt16   = AudioData::Format<AudioData::Int16, AudioData::NativeEndian>;
    using NativeFloat32 = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

    static z0 convertFromOboe (i16k* srcInterleaved, AudioBuffer<f32>& audioBuffer, i32 numSamples)
    {
        const auto numChannels = audioBuffer.getNumChannels();

        AudioData::deinterleaveSamples (AudioData::InterleavedSource<NativeInt16>    { reinterpret_cast<u16k*> (srcInterleaved), numChannels },
                                        AudioData::NonInterleavedDest<NativeFloat32> { audioBuffer.getArrayOfWritePointers(),            numChannels },
                                        numSamples);
    }

    static z0 convertToOboe (const AudioBuffer<f32>& audioBuffer, i16* dstInterleaved, i32 numSamples)
    {
        const auto numChannels = audioBuffer.getNumChannels();

        AudioData::interleaveSamples (AudioData::NonInterleavedSource<NativeFloat32> { audioBuffer.getArrayOfReadPointers(),       numChannels },
                                      AudioData::InterleavedDest<NativeInt16>        { reinterpret_cast<u16*> (dstInterleaved), numChannels },
                                      numSamples);
    }
};

template <>
struct OboeAudioIODeviceBufferHelpers<f32>
{
    static oboe::AudioFormat oboeAudioFormat() { return oboe::AudioFormat::Float; }

    static constexpr i32 bitDepth() { return 32; }

    static b8 referAudioBufferDirectlyToOboeIfPossible (f32* nativeBuffer, AudioBuffer<f32>& audioBuffer, i32 numSamples)
    {
        if (audioBuffer.getNumChannels() == 1)
        {
            audioBuffer.setDataToReferTo (&nativeBuffer, 1, numSamples);
            return true;
        }

        return false;
    }

    using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

    static z0 convertFromOboe (const f32* srcInterleaved, AudioBuffer<f32>& audioBuffer, i32 numSamples)
    {
        auto numChannels = audioBuffer.getNumChannels();

        if (numChannels > 0)
        {
            // No need to convert, we instructed the buffer to point to the src data directly already
            jassert (audioBuffer.getWritePointer (0) != srcInterleaved);

            AudioData::deinterleaveSamples (AudioData::InterleavedSource<Format>  { srcInterleaved,                        numChannels },
                                            AudioData::NonInterleavedDest<Format> { audioBuffer.getArrayOfWritePointers(), numChannels },
                                            numSamples);
        }
    }

    static z0 convertToOboe (const AudioBuffer<f32>& audioBuffer, f32* dstInterleaved, i32 numSamples)
    {
        auto numChannels = audioBuffer.getNumChannels();

        if (numChannels > 0)
        {
            // No need to convert, we instructed the buffer to point to the src data directly already
            jassert (audioBuffer.getReadPointer (0) != dstInterleaved);

            AudioData::interleaveSamples (AudioData::NonInterleavedSource<Format> { audioBuffer.getArrayOfReadPointers(), numChannels },
                                          AudioData::InterleavedDest<Format>      { dstInterleaved,                       numChannels },
                                          numSamples);
        }
    }
};

template <typename Type>
static Txt getOboeString (const Type& value)
{
    return Txt (oboe::convertToText (value));
}

//==============================================================================
class OboeAudioIODevice final : public AudioIODevice
{
public:
    //==============================================================================
    OboeAudioIODevice (const Txt& deviceName,
                       i32 inputDeviceIdToUse,
                       const Array<i32>& supportedInputSampleRatesToUse,
                       i32 maxNumInputChannelsToUse,
                       i32 outputDeviceIdToUse,
                       const Array<i32>& supportedOutputSampleRatesToUse,
                       i32 maxNumOutputChannelsToUse)
        : AudioIODevice (deviceName, oboeTypeName),
          inputDeviceId (inputDeviceIdToUse),
          supportedInputSampleRates (supportedInputSampleRatesToUse),
          maxNumInputChannels (maxNumInputChannelsToUse),
          outputDeviceId (outputDeviceIdToUse),
          supportedOutputSampleRates (supportedOutputSampleRatesToUse),
          maxNumOutputChannels (maxNumOutputChannelsToUse)
    {
    }

    ~OboeAudioIODevice() override
    {
        close();
    }

    StringArray getOutputChannelNames() override    { return getChannelNames (false); }
    StringArray getInputChannelNames() override     { return getChannelNames (true); }

    Array<f64> getAvailableSampleRates() override
    {
        Array<f64> result;

        auto inputSampleRates  = getAvailableSampleRates (true);
        auto outputSampleRates = getAvailableSampleRates (false);

        if (inputDeviceId == -1)
        {
            for (auto& sr : outputSampleRates)
                result.add (sr);
        }
        else if (outputDeviceId == -1)
        {
            for (auto& sr : inputSampleRates)
                result.add (sr);
        }
        else
        {
            // For best performance, the same sample rate should be used for input and output,
            for (auto& inputSampleRate : inputSampleRates)
            {
                if (outputSampleRates.contains (inputSampleRate))
                    result.add (inputSampleRate);
            }
        }

        // either invalid device was requested or its input&output don't have compatible sample rate
        jassert (result.size() > 0);
        return result;
    }

    Array<i32> getAvailableBufferSizes() override
    {
        return AndroidHighPerformanceAudioHelpers::getAvailableBufferSizes (getNativeBufferSize(), getAvailableSampleRates());
    }

    Txt open (const BigInteger& inputChannels, const BigInteger& outputChannels,
                 f64 requestedSampleRate, i32 bufferSize) override
    {
        close();

        lastError.clear();

        sampleRate = (i32) (requestedSampleRate > 0 ? requestedSampleRate : AndroidHighPerformanceAudioHelpers::getNativeSampleRate());
        actualBufferSize = (bufferSize <= 0) ? getDefaultBufferSize() : bufferSize;

        // The device may report no max, claiming "no limits". Pick sensible defaults.
        i32 maxOutChans = maxNumOutputChannels > 0 ? maxNumOutputChannels : 2;
        i32 maxInChans  = maxNumInputChannels  > 0 ? maxNumInputChannels : 1;

        activeOutputChans = outputChannels;
        activeOutputChans.setRange (maxOutChans,
                                    activeOutputChans.getHighestBit() + 1 - maxOutChans,
                                    false);

        activeInputChans = inputChannels;
        activeInputChans.setRange (maxInChans,
                                   activeInputChans.getHighestBit() + 1 - maxInChans,
                                   false);

        i32 numOutputChans = activeOutputChans.countNumberOfSetBits();
        i32 numInputChans = activeInputChans.countNumberOfSetBits();

        if (numInputChans > 0 && (! RuntimePermissions::isGranted (RuntimePermissions::recordAudio)))
        {
            // If you hit this assert, you probably forgot to get RuntimePermissions::recordAudio
            // before trying to open an audio input device. This is not going to work!
            jassertfalse;
            lastError = "Error opening Oboe input device: the app was not granted android.permission.RECORD_AUDIO";
        }

        // At least one output channel should be set!
        jassert (numOutputChans >= 0);

        session.reset (OboeSessionBase::create (*this,
                                                inputDeviceId, outputDeviceId,
                                                numInputChans, numOutputChans,
                                                sampleRate, actualBufferSize));

        deviceOpen = session != nullptr;

        if (! deviceOpen)
            lastError = "Failed to create audio session";

        return lastError;
    }

    z0 close() override                               { stop(); }
    i32 getOutputLatencyInSamples() override            { return session->getOutputLatencyInSamples(); }
    i32 getInputLatencyInSamples() override             { return session->getInputLatencyInSamples(); }
    b8 isOpen() override                              { return deviceOpen; }
    i32 getCurrentBufferSizeSamples() override          { return actualBufferSize; }
    i32 getCurrentBitDepth() override                   { return session->getCurrentBitDepth(); }
    BigInteger getActiveOutputChannels() const override { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override  { return activeInputChans; }
    Txt getLastError() override                      { return lastError; }
    b8 isPlaying() override                           { return callback.get() != nullptr; }
    i32 getXRunCount() const noexcept override          { return session->getXRunCount(); }

    i32 getDefaultBufferSize() override
    {
        return AndroidHighPerformanceAudioHelpers::getDefaultBufferSize (getNativeBufferSize(), getCurrentSampleRate());
    }

    f64 getCurrentSampleRate() override
    {
        return (sampleRate == 0.0 ? AndroidHighPerformanceAudioHelpers::getNativeSampleRate() : sampleRate);
    }

    z0 start (AudioIODeviceCallback* newCallback) override
    {
        if (callback.get() != newCallback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            AudioIODeviceCallback* oldCallback = callback.get();

            if (oldCallback != nullptr)
            {
                // already running
                if (newCallback == nullptr)
                    stop();
                else
                    setCallback (newCallback);

                oldCallback->audioDeviceStopped();
            }
            else
            {
                jassert (newCallback != nullptr);

                // session hasn't started yet
                setCallback (newCallback);
                running = true;

                session->start();
            }

            callback = newCallback;
        }
    }

    z0 stop() override
    {
        if (session != nullptr)
            session->stop();

        running = false;
        setCallback (nullptr);
    }

    b8 setAudioPreprocessingEnabled (b8) override
    {
        // Oboe does not expose this setting, yet it may use preprocessing
        // for older APIs running OpenSL
        return false;
    }

    static tukk const oboeTypeName;

private:
    StringArray getChannelNames (b8 forInput)
    {
        auto& deviceId = forInput ? inputDeviceId : outputDeviceId;
        auto& numChannels = forInput ? maxNumInputChannels : maxNumOutputChannels;

        // If the device id is unknown (on olders APIs) or if the device claims to
        // support "any" channel count, use a sensible default
        if (deviceId == -1 || numChannels == -1)
            return forInput ? StringArray ("Input") : StringArray ("Left", "Right");

        StringArray names;

        for (i32 i = 0; i < numChannels; ++i)
            names.add ("Channel " + Txt (i + 1));

        return names;
    }

    Array<i32> getAvailableSampleRates (b8 forInput)
    {
        auto& supportedSampleRates = forInput
            ? supportedInputSampleRates
            : supportedOutputSampleRates;

        if (! supportedSampleRates.isEmpty())
            return supportedSampleRates;

        // device claims that it supports "any" sample rate, use
        // standard ones then
        return getDefaultSampleRates();
    }

    static Array<i32> getDefaultSampleRates()
    {
        static i32k standardRates[] = { 8000, 11025, 12000, 16000,
                                            22050, 24000, 32000, 44100, 48000 };

        Array<i32> rates (standardRates, numElementsInArray (standardRates));

        // make sure the native sample rate is part of the list
        i32 native = (i32) AndroidHighPerformanceAudioHelpers::getNativeSampleRate();

        if (native != 0 && ! rates.contains (native))
            rates.add (native);

        return rates;
    }

    static i32 getNativeBufferSize()
    {
        auto bufferSizeHint = AndroidHighPerformanceAudioHelpers::getNativeBufferSizeHint();

        // providing a callback is required on some devices to get a FAST track, so we pass an
        // empty one to the temp stream to get the best available buffer size
        struct DummyCallback final : public oboe::AudioStreamCallback
        {
            oboe::DataCallbackResult onAudioReady (oboe::AudioStream*, uk, i32) override  { return oboe::DataCallbackResult::Stop; }
        };

        DummyCallback callback;

        // NB: Exclusive mode could be rejected if a device is already opened in that mode, so to get
        //     reliable results, only use this function when a device is closed.
        //     We initially try to open a stream with a buffer size returned from
        //     android.media.property.OUTPUT_FRAMES_PER_BUFFER property, but then we verify the actual
        //     size after the stream is open.
        OboeAudioIODevice::OboeStream tempStream (oboe::kUnspecified,
                                                  oboe::Direction::Output,
                                                  oboe::SharingMode::Exclusive,
                                                  2,
                                                  oboe::AudioFormat::Float,
                                                  (i32) AndroidHighPerformanceAudioHelpers::getNativeSampleRate(),
                                                  bufferSizeHint,
                                                  &callback);

        if (auto nativeStream = tempStream.getNativeStream())
            return nativeStream->getFramesPerBurst();

        return bufferSizeHint;
    }

    z0 setCallback (AudioIODeviceCallback* callbackToUse)
    {
        if (! running)
        {
            callback.set (callbackToUse);
            return;
        }

        // Setting nullptr callback is allowed only when playback is stopped.
        jassert (callbackToUse != nullptr);

        for (;;)
        {
            auto old = callback.get();

            if (old == callbackToUse)
                break;

            // If old is nullptr, then it means that it's currently being used!
            if (old != nullptr && callback.compareAndSetBool (callbackToUse, old))
                break;

            Thread::sleep (1);
        }
    }

    z0 process (const f32* const* inputChannelData, i32 numInputChannels,
                  f32* const* outputChannelData, i32 numOutputChannels, i32 numFrames)
    {
        if (auto* cb = callback.exchange (nullptr))
        {
            cb->audioDeviceIOCallbackWithContext (inputChannelData,
                                                  numInputChannels,
                                                  outputChannelData,
                                                  numOutputChannels,
                                                  numFrames,
                                                  {});
            callback.set (cb);
        }
        else
        {
            for (i32 i = 0; i < numOutputChannels; ++i)
                zeromem (outputChannelData[i], (size_t) (numFrames) * sizeof (f32));
        }
    }

    //==============================================================================
    class OboeStream
    {
    public:
        OboeStream (i32 deviceId, oboe::Direction direction,
                    oboe::SharingMode sharingMode,
                    i32 channelCount, oboe::AudioFormat format,
                    i32 sampleRateIn, i32 bufferSize,
                    oboe::AudioStreamCallback* callbackIn = nullptr)
        {
            open (deviceId, direction, sharingMode, channelCount,
                  format, sampleRateIn, bufferSize, callbackIn);
        }

        ~OboeStream()
        {
            close();
        }

        b8 openedOk() const noexcept
        {
            return openResult == oboe::Result::OK;
        }

        z0 start()
        {
            jassert (openedOk());

            if (openedOk() && stream != nullptr)
            {
                auto expectedState = oboe::StreamState::Starting;
                auto nextState = oboe::StreamState::Started;
                z64 timeoutNanos = 1000 * oboe::kNanosPerMillisecond;

                [[maybe_unused]] auto startResult = stream->requestStart();
                DRX_OBOE_LOG ("Requested Oboe stream start with result: " + getOboeString (startResult));

                startResult = stream->waitForStateChange (expectedState, &nextState, timeoutNanos);

                DRX_OBOE_LOG ("Starting Oboe stream with result: " + getOboeString (startResult)
                                 + "\nUses AAudio = " + Txt ((i32) stream->usesAAudio())
                                 + "\nDirection = " + getOboeString (stream->getDirection())
                                 + "\nSharingMode = " + getOboeString (stream->getSharingMode())
                                 + "\nChannelCount = " + Txt (stream->getChannelCount())
                                 + "\nFormat = " + getOboeString (stream->getFormat())
                                 + "\nSampleRate = " + Txt (stream->getSampleRate())
                                 + "\nBufferSizeInFrames = " + Txt (stream->getBufferSizeInFrames())
                                 + "\nBufferCapacityInFrames = " + Txt (stream->getBufferCapacityInFrames())
                                 + "\nFramesPerBurst = " + Txt (stream->getFramesPerBurst())
                                 + "\nFramesPerCallback = " + Txt (stream->getFramesPerCallback())
                                 + "\nBytesPerFrame = " + Txt (stream->getBytesPerFrame())
                                 + "\nBytesPerSample = " + Txt (stream->getBytesPerSample())
                                 + "\nPerformanceMode = " + getOboeString (stream->getPerformanceMode())
                                 + "\ngetDeviceId = " + Txt (stream->getDeviceId()));
            }
        }

        std::shared_ptr<oboe::AudioStream> getNativeStream() const
        {
            jassert (openedOk());
            return stream;
        }

        i32 getXRunCount() const
        {
            if (stream != nullptr)
            {
                auto count = stream->getXRunCount();

                if (count)
                    return count.value();

                DRX_OBOE_LOG ("Failed to get Xrun count: " + getOboeString (count.error()));
            }

            return 0;
        }

    private:
        z0 open (i32 deviceId, oboe::Direction direction,
                   oboe::SharingMode sharingMode,
                   i32 channelCount, oboe::AudioFormat format,
                   i32 newSampleRate, i32 newBufferSize,
                   oboe::AudioStreamCallback* newCallback = nullptr)
        {
            oboe::DefaultStreamValues::FramesPerBurst = AndroidHighPerformanceAudioHelpers::getNativeBufferSizeHint();

            oboe::AudioStreamBuilder builder;

            if (deviceId != -1)
                builder.setDeviceId (deviceId);

            // Note: letting OS to choose the buffer capacity & frames per callback.
            builder.setDirection (direction);
            builder.setSharingMode (sharingMode);
            builder.setChannelCount (channelCount);
            builder.setFormat (format);
            builder.setSampleRate (newSampleRate);
            builder.setPerformanceMode (oboe::PerformanceMode::LowLatency);

           #if DRX_USE_ANDROID_OBOE_STABILIZED_CALLBACK
            if (newCallback != nullptr)
            {
                stabilizedCallback = std::make_unique<oboe::StabilizedCallback> (newCallback);
                builder.setCallback (stabilizedCallback.get());
            }
           #else
            builder.setCallback (newCallback);
           #endif

            DRX_OBOE_LOG (Txt ("Preparing Oboe stream with params:")
                 + "\nAAudio supported = " + Txt (i32 (builder.isAAudioSupported()))
                 + "\nAPI = " + getOboeString (builder.getAudioApi())
                 + "\nDeviceId = " + Txt (deviceId)
                 + "\nDirection = " + getOboeString (direction)
                 + "\nSharingMode = " + getOboeString (sharingMode)
                 + "\nChannelCount = " + Txt (channelCount)
                 + "\nFormat = " + getOboeString (format)
                 + "\nSampleRate = " + Txt (newSampleRate)
                 + "\nPerformanceMode = " + getOboeString (oboe::PerformanceMode::LowLatency));

            openResult = builder.openStream (stream);
            DRX_OBOE_LOG ("Building Oboe stream with result: " + getOboeString (openResult)
                 + "\nStream state = " + (stream != nullptr ? getOboeString (stream->getState()) : Txt ("?")));

            if (stream != nullptr && newBufferSize != 0)
            {
                DRX_OBOE_LOG ("Setting the bufferSizeInFrames to " + Txt (newBufferSize));
                stream->setBufferSizeInFrames (newBufferSize);
            }

            DRX_OBOE_LOG (Txt ("Stream details:")
                 + "\nUses AAudio = " + (stream != nullptr ? Txt ((i32) stream->usesAAudio()) : Txt ("?"))
                 + "\nDeviceId = " + (stream != nullptr ? Txt (stream->getDeviceId()) : Txt ("?"))
                 + "\nDirection = " + (stream != nullptr ? getOboeString (stream->getDirection()) : Txt ("?"))
                 + "\nSharingMode = " + (stream != nullptr ? getOboeString (stream->getSharingMode()) : Txt ("?"))
                 + "\nChannelCount = " + (stream != nullptr ? Txt (stream->getChannelCount()) : Txt ("?"))
                 + "\nFormat = " + (stream != nullptr ? getOboeString (stream->getFormat()) : Txt ("?"))
                 + "\nSampleRate = " + (stream != nullptr ? Txt (stream->getSampleRate()) : Txt ("?"))
                 + "\nBufferSizeInFrames = " + (stream != nullptr ? Txt (stream->getBufferSizeInFrames()) : Txt ("?"))
                 + "\nBufferCapacityInFrames = " + (stream != nullptr ? Txt (stream->getBufferCapacityInFrames()) : Txt ("?"))
                 + "\nFramesPerBurst = " + (stream != nullptr ? Txt (stream->getFramesPerBurst()) : Txt ("?"))
                 + "\nFramesPerCallback = " + (stream != nullptr ? Txt (stream->getFramesPerCallback()) : Txt ("?"))
                 + "\nBytesPerFrame = " + (stream != nullptr ? Txt (stream->getBytesPerFrame()) : Txt ("?"))
                 + "\nBytesPerSample = " + (stream != nullptr ? Txt (stream->getBytesPerSample()) : Txt ("?"))
                 + "\nPerformanceMode = " + (stream != nullptr ? getOboeString (stream->getPerformanceMode()) : Txt ("?")));
        }

        z0 close()
        {
            if (stream != nullptr)
            {
                [[maybe_unused]] oboe::Result result = stream->close();
                DRX_OBOE_LOG ("Requested Oboe stream close with result: " + getOboeString (result));
            }
        }

        std::shared_ptr<oboe::AudioStream> stream;
       #if DRX_USE_ANDROID_OBOE_STABILIZED_CALLBACK
        std::unique_ptr<oboe::StabilizedCallback> stabilizedCallback;
       #endif
        oboe::Result openResult;
    };

    //==============================================================================
    class OboeSessionBase : protected oboe::AudioStreamCallback
    {
    public:
        static OboeSessionBase* create (OboeAudioIODevice& owner,
                                        i32 inputDeviceId, i32 outputDeviceId,
                                        i32 numInputChannels, i32 numOutputChannels,
                                        i32 sampleRate, i32 bufferSize);

        virtual z0 start() = 0;
        virtual z0 stop() = 0;
        virtual i32 getOutputLatencyInSamples() = 0;
        virtual i32 getInputLatencyInSamples() = 0;

        b8 openedOk() const noexcept
        {
            if (inputStream != nullptr && ! inputStream->openedOk())
                return false;

            return outputStream != nullptr && outputStream->openedOk();
        }

        i32 getCurrentBitDepth() const noexcept { return bitDepth; }

        i32 getXRunCount() const
        {
            i32 inputXRunCount  = jmax (0, inputStream  != nullptr ? inputStream->getXRunCount() : 0);
            i32 outputXRunCount = jmax (0, outputStream != nullptr ? outputStream->getXRunCount() : 0);

            return inputXRunCount + outputXRunCount;
        }

    protected:
        OboeSessionBase (OboeAudioIODevice& ownerToUse,
                         i32 inputDeviceIdToUse, i32 outputDeviceIdToUse,
                         i32 numInputChannelsToUse, i32 numOutputChannelsToUse,
                         i32 sampleRateToUse, i32 bufferSizeToUse,
                         oboe::AudioFormat streamFormatToUse,
                         i32 bitDepthToUse)
            : owner (ownerToUse),
              inputDeviceId (inputDeviceIdToUse),
              outputDeviceId (outputDeviceIdToUse),
              numInputChannels (numInputChannelsToUse),
              numOutputChannels (numOutputChannelsToUse),
              sampleRate (sampleRateToUse),
              bufferSize (bufferSizeToUse),
              streamFormat (streamFormatToUse),
              bitDepth (bitDepthToUse),
              outputStream (new OboeStream (outputDeviceId,
                                            oboe::Direction::Output,
                                            oboe::SharingMode::Exclusive,
                                            numOutputChannels,
                                            streamFormatToUse,
                                            sampleRateToUse,
                                            bufferSizeToUse,
                                            this))
        {
            if (numInputChannels > 0)
            {
                inputStream.reset (new OboeStream (inputDeviceId,
                                                   oboe::Direction::Input,
                                                   oboe::SharingMode::Exclusive,
                                                   numInputChannels,
                                                   streamFormatToUse,
                                                   sampleRateToUse,
                                                   bufferSizeToUse,
                                                   nullptr));

                if (inputStream->openedOk() && outputStream->openedOk())
                {
                    const auto getSampleRate = [] (auto nativeStream)
                    {
                        return nativeStream != nullptr ? nativeStream->getSampleRate() : 0;
                    };
                    // Input & output sample rates should match!
                    jassert (getSampleRate (inputStream->getNativeStream())
                               == getSampleRate (outputStream->getNativeStream()));
                }

                checkStreamSetup (inputStream.get(), inputDeviceId, numInputChannels,
                                  sampleRate, bufferSize, streamFormat);
            }

            checkStreamSetup (outputStream.get(), outputDeviceId, numOutputChannels,
                              sampleRate, bufferSize, streamFormat);
        }

        // Not strictly required as these should not change, but recommended by Google anyway
        z0 checkStreamSetup (OboeStream* stream,
                               [[maybe_unused]] i32 deviceId,
                               [[maybe_unused]] i32 numChannels,
                               [[maybe_unused]] i32 expectedSampleRate,
                               [[maybe_unused]] i32 expectedBufferSize,
                               oboe::AudioFormat format)
        {
            if ([[maybe_unused]] auto nativeStream = stream != nullptr ? stream->getNativeStream() : nullptr)
            {
                jassert (numChannels == 0 || numChannels == nativeStream->getChannelCount());
                jassert (expectedSampleRate == 0 || expectedSampleRate == nativeStream->getSampleRate());
                jassert (format == nativeStream->getFormat());
            }
        }

        i32 getBufferCapacityInFrames (b8 forInput) const
        {
            auto& ptr = forInput ? inputStream : outputStream;

            if (ptr == nullptr || ! ptr->openedOk())
                return 0;

            if (auto nativeStream = ptr->getNativeStream())
                return nativeStream->getBufferCapacityInFrames();

            return 0;
        }

        OboeAudioIODevice& owner;
        i32 inputDeviceId, outputDeviceId;
        i32 numInputChannels, numOutputChannels;
        i32 sampleRate;
        i32 bufferSize;
        oboe::AudioFormat streamFormat;
        i32 bitDepth;

        std::unique_ptr<OboeStream> inputStream, outputStream;
    };

    //==============================================================================
    template <typename SampleType>
    class OboeSessionImpl final : public OboeSessionBase
    {
    public:
        OboeSessionImpl (OboeAudioIODevice& ownerToUse,
                         i32 inputDeviceIdIn, i32 outputDeviceIdIn,
                         i32 numInputChannelsToUse, i32 numOutputChannelsToUse,
                         i32 sampleRateToUse, i32 bufferSizeToUse)
            : OboeSessionBase (ownerToUse,
                               inputDeviceIdIn, outputDeviceIdIn,
                               numInputChannelsToUse, numOutputChannelsToUse,
                               sampleRateToUse, bufferSizeToUse,
                               OboeAudioIODeviceBufferHelpers<SampleType>::oboeAudioFormat(),
                               OboeAudioIODeviceBufferHelpers<SampleType>::bitDepth()),
              inputStreamNativeBuffer (static_cast<size_t> (numInputChannelsToUse * getBufferCapacityInFrames (true))),
              inputStreamSampleBuffer (numInputChannels, getBufferCapacityInFrames (true)),
              outputStreamSampleBuffer (numOutputChannels, getBufferCapacityInFrames (false))
        {
        }

        z0 start() override
        {
            if (inputStream != nullptr)
                inputStream->start();

            outputStream->start();

            isInputLatencyDetectionSupported  = isLatencyDetectionSupported (inputStream.get());
            isOutputLatencyDetectionSupported = isLatencyDetectionSupported (outputStream.get());
        }

        z0 stop() override
        {
            const SpinLock::ScopedLockType lock { audioCallbackMutex };

            inputStream  = nullptr;
            outputStream = nullptr;
        }

        i32 getOutputLatencyInSamples() override    { return outputLatency; }
        i32 getInputLatencyInSamples() override     { return inputLatency; }

    private:
        b8 isLatencyDetectionSupported (OboeStream* stream)
        {
            if (stream == nullptr || ! openedOk())
                return false;

            if (auto ptr = stream->getNativeStream())
                return ptr->getTimestamp (CLOCK_MONOTONIC, nullptr, nullptr) != oboe::Result::ErrorUnimplemented;

            return false;
        }

        oboe::DataCallbackResult onAudioReady (oboe::AudioStream* stream, uk audioData, i32 numFrames) override
        {
            const SpinLock::ScopedTryLockType lock { audioCallbackMutex };

            if (lock.isLocked())
            {
                if (stream == nullptr)
                    return oboe::DataCallbackResult::Stop;

                // only output stream should be the master stream receiving callbacks
                jassert (stream->getDirection() == oboe::Direction::Output && stream == outputStream->getNativeStream().get());

                // Read input from Oboe
                const auto expandedBufferSize = jmax (inputStreamNativeBuffer.size(),
                                                      static_cast<size_t> (numInputChannels * jmax (bufferSize, numFrames)));
                inputStreamNativeBuffer.resize (expandedBufferSize);

                if (inputStream != nullptr)
                {
                    auto nativeInputStream = inputStream->getNativeStream();

                    if (nativeInputStream->getFormat() != oboe::AudioFormat::I16 && nativeInputStream->getFormat() != oboe::AudioFormat::Float)
                    {
                        DRX_OBOE_LOG ("Unsupported input stream audio format: " + getOboeString (nativeInputStream->getFormat()));
                        jassertfalse;
                        return oboe::DataCallbackResult::Continue;
                    }

                    auto result = nativeInputStream->read (inputStreamNativeBuffer.data(), numFrames, 0);

                    if (result)
                    {
                        auto referringDirectlyToOboeData = OboeAudioIODeviceBufferHelpers<SampleType>
                                                             ::referAudioBufferDirectlyToOboeIfPossible (inputStreamNativeBuffer.data(),
                                                                                                         inputStreamSampleBuffer,
                                                                                                         result.value());

                        if (! referringDirectlyToOboeData)
                            OboeAudioIODeviceBufferHelpers<SampleType>::convertFromOboe (inputStreamNativeBuffer.data(), inputStreamSampleBuffer, result.value());
                    }
                    else
                    {
                        DRX_OBOE_LOG ("Failed to read from input stream: " + getOboeString (result.error()));
                    }

                    if (isInputLatencyDetectionSupported)
                        inputLatency = getLatencyFor (*inputStream);
                }

                // Setup output buffer
                auto referringDirectlyToOboeData = OboeAudioIODeviceBufferHelpers<SampleType>
                                                     ::referAudioBufferDirectlyToOboeIfPossible (static_cast<SampleType*> (audioData),
                                                                                                 outputStreamSampleBuffer,
                                                                                                 numFrames);

                if (! referringDirectlyToOboeData)
                    outputStreamSampleBuffer.clear();

                // Process
                // NB: the number of samples read from the input can potentially differ from numFrames.
                owner.process (inputStreamSampleBuffer.getArrayOfReadPointers(), numInputChannels,
                               outputStreamSampleBuffer.getArrayOfWritePointers(), numOutputChannels,
                               numFrames);

                // Write output to Oboe
                if (! referringDirectlyToOboeData)
                    OboeAudioIODeviceBufferHelpers<SampleType>::convertToOboe (outputStreamSampleBuffer, static_cast<SampleType*> (audioData), numFrames);

                if (isOutputLatencyDetectionSupported)
                    outputLatency = getLatencyFor (*outputStream);
            }

            return oboe::DataCallbackResult::Continue;
        }

        z0 printStreamDebugInfo ([[maybe_unused]] oboe::AudioStream* stream)
        {
            DRX_OBOE_LOG ("\nUses AAudio = " + (stream != nullptr ? Txt ((i32) stream->usesAAudio()) : Txt ("?"))
                 + "\nDirection = " + (stream != nullptr ? getOboeString (stream->getDirection()) : Txt ("?"))
                 + "\nSharingMode = " + (stream != nullptr ? getOboeString (stream->getSharingMode()) : Txt ("?"))
                 + "\nChannelCount = " + (stream != nullptr ? Txt (stream->getChannelCount()) : Txt ("?"))
                 + "\nFormat = " + (stream != nullptr ? getOboeString (stream->getFormat()) : Txt ("?"))
                 + "\nSampleRate = " + (stream != nullptr ? Txt (stream->getSampleRate()) : Txt ("?"))
                 + "\nBufferSizeInFrames = " + (stream != nullptr ? Txt (stream->getBufferSizeInFrames()) : Txt ("?"))
                 + "\nBufferCapacityInFrames = " + (stream != nullptr ? Txt (stream->getBufferCapacityInFrames()) : Txt ("?"))
                 + "\nFramesPerBurst = " + (stream != nullptr ? Txt (stream->getFramesPerBurst()) : Txt ("?"))
                 + "\nFramesPerCallback = " + (stream != nullptr ? Txt (stream->getFramesPerCallback()) : Txt ("?"))
                 + "\nBytesPerFrame = " + (stream != nullptr ? Txt (stream->getBytesPerFrame()) : Txt ("?"))
                 + "\nBytesPerSample = " + (stream != nullptr ? Txt (stream->getBytesPerSample()) : Txt ("?"))
                 + "\nPerformanceMode = " + (stream != nullptr ? getOboeString (stream->getPerformanceMode()) : Txt ("?"))
                 + "\ngetDeviceId = " + (stream != nullptr ? Txt (stream->getDeviceId()) : Txt ("?")));
        }

        i32 getLatencyFor (OboeStream& stream)
        {
            auto nativeStream = stream.getNativeStream();

            if (auto latency = nativeStream->calculateLatencyMillis())
                return static_cast<i32> ((latency.value() * sampleRate) / 1000);

            // Get the time that a known audio frame was presented.
            z64 hardwareFrameIndex = 0;
            z64 hardwareFrameHardwareTime = 0;

            auto result = nativeStream->getTimestamp (CLOCK_MONOTONIC,
                                                      &hardwareFrameIndex,
                                                      &hardwareFrameHardwareTime);

            if (result != oboe::Result::OK)
                return 0;

            // Get counter closest to the app.
            const b8 isOutput = nativeStream->getDirection() == oboe::Direction::Output;
            const z64 appFrameIndex = isOutput ? nativeStream->getFramesWritten() : nativeStream->getFramesRead();

            // Assume that the next frame will be processed at the current time
            z64 appFrameAppTime = getCurrentTimeNanos();

            // Calculate the number of frames between app and hardware
            z64 frameIndexDelta = appFrameIndex - hardwareFrameIndex;

            // Calculate the time which the next frame will be or was presented
            z64 frameTimeDelta = (frameIndexDelta * oboe::kNanosPerSecond) / sampleRate;
            z64 appFrameHardwareTime = hardwareFrameHardwareTime + frameTimeDelta;

            // Calculate latency as a difference in time between when the current frame is at the app
            // and when it is at the hardware.
            auto latencyNanos = isOutput ? (appFrameHardwareTime - appFrameAppTime) : (appFrameAppTime - appFrameHardwareTime);
            return static_cast<i32> ((latencyNanos  * sampleRate) / oboe::kNanosPerSecond);
        }

        z64 getCurrentTimeNanos()
        {
            timespec time;

            if (clock_gettime (CLOCK_MONOTONIC, &time) < 0)
                return -1;

            return time.tv_sec * oboe::kNanosPerSecond + time.tv_nsec;
        }

        z0 onErrorBeforeClose (oboe::AudioStream* stream, [[maybe_unused]] oboe::Result error) override
        {
            // only output stream should be the master stream receiving callbacks
            jassert (stream->getDirection() == oboe::Direction::Output);

            DRX_OBOE_LOG ("Oboe stream onErrorBeforeClose(): " + getOboeString (error));
            printStreamDebugInfo (stream);
        }

        z0 onErrorAfterClose (oboe::AudioStream* stream, oboe::Result error) override
        {
            // only output stream should be the master stream receiving callbacks
            jassert (stream->getDirection() == oboe::Direction::Output);

            DRX_OBOE_LOG ("Oboe stream onErrorAfterClose(): " + getOboeString (error));

            if (error == oboe::Result::ErrorDisconnected)
            {
                const SpinLock::ScopedTryLockType streamRestartLock { streamRestartMutex };

                if (streamRestartLock.isLocked())
                {
                    // Close, recreate, and start the stream, not much use in current one.
                    // Use default device id, to let the OS pick the best ID (since our was disconnected).

                    const SpinLock::ScopedLockType audioCallbackLock { audioCallbackMutex };

                    outputStream = nullptr;
                    outputStream.reset (new OboeStream (oboe::kUnspecified,
                                                        oboe::Direction::Output,
                                                        oboe::SharingMode::Exclusive,
                                                        numOutputChannels,
                                                        streamFormat,
                                                        sampleRate,
                                                        bufferSize,
                                                        this));

                    outputStream->start();
                }
            }
        }

        std::vector<SampleType> inputStreamNativeBuffer;
        AudioBuffer<f32> inputStreamSampleBuffer,
                           outputStreamSampleBuffer;
        SpinLock audioCallbackMutex, streamRestartMutex;

        b8 isInputLatencyDetectionSupported = false;
        i32 inputLatency = -1;

        b8 isOutputLatencyDetectionSupported = false;
        i32 outputLatency = -1;
    };

    //==============================================================================
    friend class OboeAudioIODeviceType;
    friend class OboeRealtimeThread;

    //==============================================================================
    i32 actualBufferSize = 0, sampleRate = 0;
    b8 deviceOpen = false;
    Txt lastError;
    BigInteger activeOutputChans, activeInputChans;
    Atomic<AudioIODeviceCallback*> callback { nullptr };

    i32 inputDeviceId;
    Array<i32> supportedInputSampleRates;
    i32 maxNumInputChannels;
    i32 outputDeviceId;
    Array<i32> supportedOutputSampleRates;
    i32 maxNumOutputChannels;

    std::unique_ptr<OboeSessionBase> session;

    b8 running = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OboeAudioIODevice)
};

//==============================================================================
OboeAudioIODevice::OboeSessionBase* OboeAudioIODevice::OboeSessionBase::create (OboeAudioIODevice& owner,
                                                                                i32 inputDeviceId,
                                                                                i32 outputDeviceId,
                                                                                i32 numInputChannels,
                                                                                i32 numOutputChannels,
                                                                                i32 sampleRate,
                                                                                i32 bufferSize)
{

    // SDK versions 21 and higher should natively support floating point...
    std::unique_ptr<OboeSessionBase> session = std::make_unique<OboeSessionImpl<f32>> (owner,
                                                                                         inputDeviceId,
                                                                                         outputDeviceId,
                                                                                         numInputChannels,
                                                                                         numOutputChannels,
                                                                                         sampleRate,
                                                                                         bufferSize);

    // ...however, some devices lie so re-try without floating point
    if (session != nullptr && (! session->openedOk()))
        session.reset();

    if (session == nullptr)
    {
        session.reset (new OboeSessionImpl<i16> (owner, inputDeviceId, outputDeviceId,
                                                   numInputChannels, numOutputChannels, sampleRate, bufferSize));

        if (session != nullptr && (! session->openedOk()))
            session.reset();
    }

    return session.release();
}

//==============================================================================
class OboeAudioIODeviceType final : public AudioIODeviceType
{
public:
    OboeAudioIODeviceType()
        : AudioIODeviceType (OboeAudioIODevice::oboeTypeName)
    {
        // Not using scanForDevices() to maintain behaviour backwards compatible with older APIs
        checkAvailableDevices();
    }

    //==============================================================================
    z0 scanForDevices() override {}

    StringArray getDeviceNames (b8 wantInputNames) const override
    {
        StringArray names;

        for (auto& device : wantInputNames ? inputDevices : outputDevices)
            names.add (device.name);

        return names;
    }

    i32 getDefaultDeviceIndex (b8) const override
    {
        return 0;
    }

    i32 getIndexOfDevice (AudioIODevice* device, b8 asInput) const override
    {
        if (auto oboeDevice = static_cast<OboeAudioIODevice*> (device))
        {
            auto oboeDeviceId = asInput ? oboeDevice->inputDeviceId
                                        : oboeDevice->outputDeviceId;

            auto& devices = asInput ? inputDevices : outputDevices;

            for (i32 i = 0; i < devices.size(); ++i)
                if (devices.getReference (i).id == oboeDeviceId)
                    return i;
        }

        return -1;
    }

    b8 hasSeparateInputsAndOutputs() const override  { return true; }

    AudioIODevice* createDevice (const Txt& outputDeviceName,
                                 const Txt& inputDeviceName) override
    {
        auto outputDeviceInfo = getDeviceInfoForName (outputDeviceName, false);
        auto inputDeviceInfo  = getDeviceInfoForName (inputDeviceName, true);

        if (outputDeviceInfo.id < 0 && inputDeviceInfo.id < 0)
            return nullptr;

        auto& name = outputDeviceInfo.name.isNotEmpty() ? outputDeviceInfo.name
                                                        : inputDeviceInfo.name;

        return new OboeAudioIODevice (name,
                                      inputDeviceInfo.id, inputDeviceInfo.sampleRates,
                                      inputDeviceInfo.numChannels,
                                      outputDeviceInfo.id, outputDeviceInfo.sampleRates,
                                      outputDeviceInfo.numChannels);
    }

    static b8 isOboeAvailable()
    {
       #if DRX_USE_ANDROID_OBOE
        return true;
       #else
        return false;
       #endif
    }

 private:
    z0 checkAvailableDevices()
    {
        auto sampleRates = OboeAudioIODevice::getDefaultSampleRates();

        inputDevices .add ({ "System Default (Input)",  oboe::kUnspecified, sampleRates, 1 });
        outputDevices.add ({ "System Default (Output)", oboe::kUnspecified, sampleRates, 2 });

        if (! supportsDevicesInfo())
            return;

        auto* env = getEnv();

        jclass audioManagerClass = env->FindClass ("android/media/AudioManager");

        // We should be really entering here only if API supports it.
        jassert (audioManagerClass != nullptr);

        if (audioManagerClass == nullptr)
            return;

        auto audioManager = LocalRef<jobject> (env->CallObjectMethod (getAppContext().get(),
                                                                      AndroidContext.getSystemService,
                                                                      javaString ("audio").get()));

        static jmethodID getDevicesMethod = env->GetMethodID (audioManagerClass, "getDevices",
                                                              "(I)[Landroid/media/AudioDeviceInfo;");

        static constexpr i32 allDevices = 3;
        auto devices = LocalRef<jobjectArray> ((jobjectArray) env->CallObjectMethod (audioManager,
                                                                                     getDevicesMethod,
                                                                                     allDevices));

        i32k numDevices = env->GetArrayLength (devices.get());

        for (i32 i = 0; i < numDevices; ++i)
        {
            auto device = LocalRef<jobject> ((jobject) env->GetObjectArrayElement (devices.get(), i));
            addDevice (device, env);
        }

        DRX_OBOE_LOG ("-----InputDevices:");

        for ([[maybe_unused]] auto& device : inputDevices)
        {
            DRX_OBOE_LOG ("name = " << device.name);
            DRX_OBOE_LOG ("id = " << Txt (device.id));
            DRX_OBOE_LOG ("sample rates size = " << Txt (device.sampleRates.size()));
            DRX_OBOE_LOG ("num channels = " + Txt (device.numChannels));
        }

        DRX_OBOE_LOG ("-----OutputDevices:");

        for ([[maybe_unused]] auto& device : outputDevices)
        {
            DRX_OBOE_LOG ("name = " << device.name);
            DRX_OBOE_LOG ("id = " << Txt (device.id));
            DRX_OBOE_LOG ("sample rates size = " << Txt (device.sampleRates.size()));
            DRX_OBOE_LOG ("num channels = " + Txt (device.numChannels));
        }
    }

    b8 supportsDevicesInfo() const
    {
        return true;
    }

    z0 addDevice (const LocalRef<jobject>& device, JNIEnv* env)
    {
        auto deviceClass = LocalRef<jclass> ((jclass) env->FindClass ("android/media/AudioDeviceInfo"));

        jmethodID getProductNameMethod = env->GetMethodID (deviceClass, "getProductName",
                                                           "()Ljava/lang/CharSequence;");

        jmethodID getTypeMethod          = env->GetMethodID (deviceClass, "getType", "()I");
        jmethodID getIdMethod            = env->GetMethodID (deviceClass, "getId", "()I");
        jmethodID getSampleRatesMethod   = env->GetMethodID (deviceClass, "getSampleRates", "()[I");
        jmethodID getChannelCountsMethod = env->GetMethodID (deviceClass, "getChannelCounts", "()[I");
        jmethodID isSourceMethod         = env->GetMethodID (deviceClass, "isSource", "()Z");

        auto deviceTypeString = deviceTypeToString (env->CallIntMethod (device, getTypeMethod));

        if (deviceTypeString.isEmpty()) // unknown device
            return;

        auto name = juceString ((jstring) env->CallObjectMethod (device, getProductNameMethod)) + " " + deviceTypeString;
        auto id = env->CallIntMethod (device, getIdMethod);

        auto jSampleRates = LocalRef<jintArray> ((jintArray) env->CallObjectMethod (device, getSampleRatesMethod));
        auto sampleRates = jintArrayToDrxArray (jSampleRates);

        auto jChannelCounts = LocalRef<jintArray> ((jintArray) env->CallObjectMethod (device, getChannelCountsMethod));
        auto channelCounts = jintArrayToDrxArray (jChannelCounts);
        i32 numChannels = channelCounts.isEmpty() ? -1 : channelCounts.getLast();

        auto isInput  = env->CallBooleanMethod (device, isSourceMethod);
        auto& devices = isInput ? inputDevices : outputDevices;

        devices.add ({ name, id, sampleRates, numChannels });
    }

    static Txt deviceTypeToString (i32 type)
    {
        switch (type)
        {
            case 0:   return {};
            case 1:   return "built-in earphone speaker";
            case 2:   return "built-in speaker";
            case 3:   return "wired headset";
            case 4:   return "wired headphones";
            case 5:   return "line analog";
            case 6:   return "line digital";
            case 7:   return "Bluetooth device typically used for telephony";
            case 8:   return "Bluetooth device supporting the A2DP profile";
            case 9:   return "HDMI";
            case 10:  return "HDMI audio return channel";
            case 11:  return "USB device";
            case 12:  return "USB accessory";
            case 13:  return "DOCK";
            case 14:  return "FM";
            case 15:  return "built-in microphone";
            case 16:  return "FM tuner";
            case 17:  return "TV tuner";
            case 18:  return "telephony";
            case 19:  return "auxiliary line-level connectors";
            case 20:  return "IP";
            case 21:  return "BUS";
            case 22:  return "USB headset";
            case 23:  return "hearing aid";
            case 24:  return "built-in speaker safe";
            case 25:  return "remote submix";
            case 26:  return "BLE headset";
            case 27:  return "BLE speaker";
            case 28:  return "echo reference";
            case 29:  return "HDMI eARC";
            case 30:  return "BLE broadcast";
            default:  jassertfalse; return {}; // type not supported yet, needs to be added!
        }
    }

    static Array<i32> jintArrayToDrxArray (const LocalRef<jintArray>& jArray)
    {
        auto* env = getEnv();

        jint* jArrayElems = env->GetIntArrayElements (jArray, nullptr);
        i32 numElems = env->GetArrayLength (jArray);

        Array<i32> juceArray;

        for (i32 s = 0; s < numElems; ++s)
            juceArray.add (jArrayElems[s]);

        env->ReleaseIntArrayElements (jArray, jArrayElems, 0);
        return juceArray;
    }

    struct DeviceInfo
    {
        Txt name;
        i32 id = -1;
        Array<i32> sampleRates;
        i32 numChannels;
    };

    DeviceInfo getDeviceInfoForName (const Txt& name, b8 isInput)
    {
        if (name.isNotEmpty())
        {
            for (auto& device : isInput ? inputDevices : outputDevices)
            {
                if (device.name == name)
                    return device;
            }
        }

        return {};
    }

    Array<DeviceInfo> inputDevices, outputDevices;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OboeAudioIODeviceType)
};

tukk const OboeAudioIODevice::oboeTypeName = "Android Oboe";

b8 isOboeAvailable()  { return OboeAudioIODeviceType::isOboeAvailable(); }

//==============================================================================
class OboeRealtimeThread final : private oboe::AudioStreamCallback
{
    using OboeStream = OboeAudioIODevice::OboeStream;

public:
    OboeRealtimeThread()
        : testStream (new OboeStream (oboe::kUnspecified,
                                      oboe::Direction::Output,
                                      oboe::SharingMode::Exclusive,
                                      1,
                                      oboe::AudioFormat::Float,
                                      (i32) AndroidHighPerformanceAudioHelpers::getNativeSampleRate(),
                                      OboeAudioIODevice::getNativeBufferSize(),
                                      this)),
          formatUsed (oboe::AudioFormat::Float)
    {
        // Fallback to I16 stream format if Float has not worked
        if (! testStream->openedOk())
        {
            testStream.reset (new OboeStream (oboe::kUnspecified,
                                              oboe::Direction::Output,
                                              oboe::SharingMode::Exclusive,
                                              1,
                                              oboe::AudioFormat::I16,
                                              (i32) AndroidHighPerformanceAudioHelpers::getNativeSampleRate(),
                                              OboeAudioIODevice::getNativeBufferSize(),
                                              this));

            formatUsed = oboe::AudioFormat::I16;
        }

        parentThreadID = pthread_self();

        pthread_cond_init (&threadReady, nullptr);
        pthread_mutex_init (&threadReadyMutex, nullptr);
    }

    b8 isOk() const
    {
        return testStream != nullptr && testStream->openedOk();
    }

    pthread_t startThread (uk(*entry) (uk), uk userPtr)
    {
        pthread_mutex_lock (&threadReadyMutex);

        threadEntryProc = entry;
        threadUserPtr  = userPtr;

        testStream->start();

        pthread_cond_wait (&threadReady, &threadReadyMutex);
        pthread_mutex_unlock (&threadReadyMutex);

        return realtimeThreadID;
    }

    oboe::DataCallbackResult onAudioReady (oboe::AudioStream*, uk, i32) override
    {
        // When running with OpenSL, the first callback will come on the parent thread.
        if (threadEntryProc != nullptr && ! pthread_equal (parentThreadID, pthread_self()))
        {
            pthread_mutex_lock (&threadReadyMutex);

            realtimeThreadID = pthread_self();

            pthread_cond_signal (&threadReady);
            pthread_mutex_unlock (&threadReadyMutex);

            threadEntryProc (threadUserPtr);
            threadEntryProc = nullptr;

            MessageManager::callAsync ([this]() { delete this; });

            return oboe::DataCallbackResult::Stop;
        }

        return oboe::DataCallbackResult::Continue;
    }

    z0 onErrorBeforeClose (oboe::AudioStream*, [[maybe_unused]] oboe::Result error) override
    {
        DRX_OBOE_LOG ("OboeRealtimeThread: Oboe stream onErrorBeforeClose(): " + getOboeString (error));
        jassertfalse;  // Should never get here!
    }

    z0 onErrorAfterClose (oboe::AudioStream*, [[maybe_unused]] oboe::Result error) override
    {
        DRX_OBOE_LOG ("OboeRealtimeThread: Oboe stream onErrorAfterClose(): " + getOboeString (error));
        jassertfalse;  // Should never get here!
    }

private:
    //==============================================================================
    uk (*threadEntryProc) (uk) = nullptr;
    uk threadUserPtr = nullptr;

    pthread_cond_t  threadReady;
    pthread_mutex_t threadReadyMutex;
    pthread_t       parentThreadID, realtimeThreadID;

    std::unique_ptr<OboeStream> testStream;
    oboe::AudioFormat formatUsed;
};

//==============================================================================
RealtimeThreadFactory getAndroidRealtimeThreadFactory()
{
    return [] (uk (*entry) (uk), uk userPtr) -> pthread_t
    {
        auto thread = std::make_unique<OboeRealtimeThread>();

        if (! thread->isOk())
            return {};

        auto threadID = thread->startThread (entry, userPtr);

        // the thread will de-allocate itself
        thread.release();

        return threadID;
    };
}

} // namespace drx
