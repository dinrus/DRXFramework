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

namespace
{

#ifndef DRX_ALSA_LOGGING
 #define DRX_ALSA_LOGGING 0
#endif

#if DRX_ALSA_LOGGING
 #define DRX_ALSA_LOG(dbgtext)   { drx::Txt tempDbgBuf ("ALSA: "); tempDbgBuf << dbgtext; Logger::writeToLog (tempDbgBuf); DBG (tempDbgBuf); }
 #define DRX_CHECKED_RESULT(x)   (logErrorMessage (x, __LINE__))

 static i32 logErrorMessage (i32 err, i32 lineNum)
 {
    if (err < 0)
        DRX_ALSA_LOG ("Error: line " << lineNum << ": code " << err << " (" << snd_strerror (err) << ")");

    return err;
 }
#else
 #define DRX_ALSA_LOG(x)         {}
 #define DRX_CHECKED_RESULT(x)   (x)
#endif

#define DRX_ALSA_FAILED(x)  failed (x)

static z0 getDeviceSampleRates (snd_pcm_t* handle, Array<f64>& rates)
{
    snd_pcm_hw_params_t* hwParams;
    snd_pcm_hw_params_alloca (&hwParams);

    for (const auto rateToTry : SampleRateHelpers::getAllSampleRates())
    {
        if (snd_pcm_hw_params_any (handle, hwParams) >= 0
             && snd_pcm_hw_params_test_rate (handle, hwParams, (u32) rateToTry, 0) == 0)
        {
            rates.addIfNotAlreadyThere (rateToTry);
        }
    }
}

static z0 getDeviceNumChannels (snd_pcm_t* handle, u32* minChans, u32* maxChans)
{
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca (&params);

    if (snd_pcm_hw_params_any (handle, params) >= 0)
    {
        snd_pcm_hw_params_get_channels_min (params, minChans);
        snd_pcm_hw_params_get_channels_max (params, maxChans);

        DRX_ALSA_LOG ("getDeviceNumChannels: " << (i32) *minChans << " " << (i32) *maxChans);

        // some virtual devices (dmix for example) report 10000 channels , we have to clamp these values
        *maxChans = jmin (*maxChans, 256u);
        *minChans = jmin (*minChans, *maxChans);
    }
    else
    {
        DRX_ALSA_LOG ("getDeviceNumChannels failed");
    }
}

static z0 getDeviceProperties (const Txt& deviceID,
                                 u32& minChansOut,
                                 u32& maxChansOut,
                                 u32& minChansIn,
                                 u32& maxChansIn,
                                 Array<f64>& rates,
                                 b8 testOutput,
                                 b8 testInput)
{
    minChansOut = maxChansOut = minChansIn = maxChansIn = 0;

    if (deviceID.isEmpty())
        return;

    DRX_ALSA_LOG ("getDeviceProperties(" << deviceID.toUTF8().getAddress() << ")");

    snd_pcm_info_t* info;
    snd_pcm_info_alloca (&info);

    if (testOutput)
    {
        snd_pcm_t* pcmHandle;

        if (DRX_CHECKED_RESULT (snd_pcm_open (&pcmHandle, deviceID.toUTF8().getAddress(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) >= 0)
        {
            getDeviceNumChannels (pcmHandle, &minChansOut, &maxChansOut);
            getDeviceSampleRates (pcmHandle, rates);

            snd_pcm_close (pcmHandle);
        }
    }

    if (testInput)
    {
        snd_pcm_t* pcmHandle;

        if (DRX_CHECKED_RESULT (snd_pcm_open (&pcmHandle, deviceID.toUTF8(), SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK) >= 0))
        {
            getDeviceNumChannels (pcmHandle, &minChansIn, &maxChansIn);

            if (rates.size() == 0)
                getDeviceSampleRates (pcmHandle, rates);

            snd_pcm_close (pcmHandle);
        }
    }
}

static z0 ensureMinimumNumBitsSet (BigInteger& chans, i32 minNumChans)
{
    i32 i = 0;

    while (chans.countNumberOfSetBits() < minNumChans)
        chans.setBit (i++);
}

static z0 silentErrorHandler (tukk, i32, tukk, i32, tukk,...) {}

//==============================================================================
class ALSADevice
{
public:
    ALSADevice (const Txt& devID, b8 forInput)
        : handle (nullptr),
          bitDepth (16),
          numChannelsRunning (0),
          latency (0),
          deviceID (devID),
          isInput (forInput),
          isInterleaved (true)
    {
        DRX_ALSA_LOG ("snd_pcm_open (" << deviceID.toUTF8().getAddress() << ", forInput=" << (i32) forInput << ")");

        i32 err = snd_pcm_open (&handle, deviceID.toUTF8(),
                                forInput ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK,
                                SND_PCM_ASYNC);
        if (err < 0)
        {
            if (-err == EBUSY)
                error << "The device \"" << deviceID << "\" is busy (another application is using it).";
            else if (-err == ENOENT)
                error << "The device \"" << deviceID << "\" is not available.";
            else
                error << "Could not open " << (forInput ? "input" : "output") << " device \"" << deviceID
                      << "\": " << snd_strerror (err) << " (" << err << ")";

            DRX_ALSA_LOG ("snd_pcm_open failed; " << error);
        }
    }

    ~ALSADevice()
    {
        closeNow();
    }

    z0 closeNow()
    {
        if (handle != nullptr)
        {
            snd_pcm_close (handle);
            handle = nullptr;
        }
    }

    b8 setParameters (u32 sampleRate, i32 numChannels, i32 bufferSize)
    {
        if (handle == nullptr)
            return false;

        DRX_ALSA_LOG ("ALSADevice::setParameters(" << deviceID << ", "
                         << (i32) sampleRate << ", " << numChannels << ", " << bufferSize << ")");

        snd_pcm_hw_params_t* hwParams;
        snd_pcm_hw_params_alloca (&hwParams);

        if (snd_pcm_hw_params_any (handle, hwParams) < 0)
        {
            // this is the error message that aplay returns when an error happens here,
            // it is a bit more explicit that "Invalid parameter"
            error = "Broken configuration for this PCM: no configurations available";
            return false;
        }

        if (snd_pcm_hw_params_set_access (handle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED) >= 0) // works better for plughw..
            isInterleaved = true;
        else if (snd_pcm_hw_params_set_access (handle, hwParams, SND_PCM_ACCESS_RW_NONINTERLEAVED) >= 0)
            isInterleaved = false;
        else
        {
            jassertfalse;
            return false;
        }

        enum { isFloatBit = 1 << 16, isLittleEndianBit = 1 << 17, onlyUseLower24Bits = 1 << 18 };

        i32k formatsToTry[] = { SND_PCM_FORMAT_FLOAT_LE,   32 | isFloatBit | isLittleEndianBit,
                                     SND_PCM_FORMAT_FLOAT_BE,   32 | isFloatBit,
                                     SND_PCM_FORMAT_S32_LE,     32 | isLittleEndianBit,
                                     SND_PCM_FORMAT_S32_BE,     32,
                                     SND_PCM_FORMAT_S24_3LE,    24 | isLittleEndianBit,
                                     SND_PCM_FORMAT_S24_3BE,    24,
                                     SND_PCM_FORMAT_S24_LE,     32 | isLittleEndianBit | onlyUseLower24Bits,
                                     SND_PCM_FORMAT_S16_LE,     16 | isLittleEndianBit,
                                     SND_PCM_FORMAT_S16_BE,     16 };
        bitDepth = 0;

        for (i32 i = 0; i < numElementsInArray (formatsToTry); i += 2)
        {
            if (snd_pcm_hw_params_set_format (handle, hwParams, (_snd_pcm_format) formatsToTry [i]) >= 0)
            {
                i32k type = formatsToTry [i + 1];
                bitDepth = type & 255;

                converter.reset (createConverter (isInput, bitDepth,
                                                  (type & isFloatBit) != 0,
                                                  (type & isLittleEndianBit) != 0,
                                                  (type & onlyUseLower24Bits) != 0,
                                                  numChannels,
                                                  isInterleaved));
                break;
            }
        }

        if (bitDepth == 0)
        {
            error = "device doesn't support a compatible PCM format";
            DRX_ALSA_LOG ("Error: " + error);
            return false;
        }

        i32 dir = 0;
        u32 periods = 4;
        snd_pcm_uframes_t samplesPerPeriod = (snd_pcm_uframes_t) bufferSize;

        if (DRX_ALSA_FAILED (snd_pcm_hw_params_set_rate_near (handle, hwParams, &sampleRate, nullptr))
            || DRX_ALSA_FAILED (snd_pcm_hw_params_set_channels (handle, hwParams, (u32 ) numChannels))
            || DRX_ALSA_FAILED (snd_pcm_hw_params_set_periods_near (handle, hwParams, &periods, &dir))
            || DRX_ALSA_FAILED (snd_pcm_hw_params_set_period_size_near (handle, hwParams, &samplesPerPeriod, &dir))
            || DRX_ALSA_FAILED (snd_pcm_hw_params (handle, hwParams)))
        {
            return false;
        }

        snd_pcm_uframes_t frames = 0;

        if (DRX_ALSA_FAILED (snd_pcm_hw_params_get_period_size (hwParams, &frames, &dir))
             || DRX_ALSA_FAILED (snd_pcm_hw_params_get_periods (hwParams, &periods, &dir)))
            latency = 0;
        else
            latency = (i32) frames * ((i32) periods - 1); // (this is the method JACK uses to guess the latency..)

        DRX_ALSA_LOG ("frames: " << (i32) frames << ", periods: " << (i32) periods
                          << ", samplesPerPeriod: " << (i32) samplesPerPeriod);

        snd_pcm_sw_params_t* swParams;
        snd_pcm_sw_params_alloca (&swParams);
        snd_pcm_uframes_t boundary;

        if (DRX_ALSA_FAILED (snd_pcm_sw_params_current (handle, swParams))
            || DRX_ALSA_FAILED (snd_pcm_sw_params_get_boundary (swParams, &boundary))
            || DRX_ALSA_FAILED (snd_pcm_sw_params_set_silence_threshold (handle, swParams, 0))
            || DRX_ALSA_FAILED (snd_pcm_sw_params_set_silence_size (handle, swParams, boundary))
            || DRX_ALSA_FAILED (snd_pcm_sw_params_set_start_threshold (handle, swParams, samplesPerPeriod))
            || DRX_ALSA_FAILED (snd_pcm_sw_params_set_stop_threshold (handle, swParams, boundary))
            || DRX_ALSA_FAILED (snd_pcm_sw_params (handle, swParams)))
        {
            return false;
        }

       #if DRX_ALSA_LOGGING
        // enable this to dump the config of the devices that get opened
        snd_output_t* out;
        snd_output_stdio_attach (&out, stderr, 0);
        snd_pcm_hw_params_dump (hwParams, out);
        snd_pcm_sw_params_dump (swParams, out);
       #endif

        numChannelsRunning = numChannels;

        return true;
    }

    //==============================================================================
    b8 writeToOutputDevice (AudioBuffer<f32>& outputChannelBuffer, i32k numSamples)
    {
        jassert (numChannelsRunning <= outputChannelBuffer.getNumChannels());
        f32* const* const data = outputChannelBuffer.getArrayOfWritePointers();
        snd_pcm_sframes_t numDone = 0;

        if (isInterleaved)
        {
            scratch.ensureSize ((size_t) ((i32) sizeof (f32) * numSamples * numChannelsRunning), false);

            for (i32 i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (scratch.getData(), i, data[i], 0, numSamples);

            numDone = snd_pcm_writei (handle, scratch.getData(), (snd_pcm_uframes_t) numSamples);
        }
        else
        {
            for (i32 i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (data[i], data[i], numSamples);

            numDone = snd_pcm_writen (handle, (uk*) data, (snd_pcm_uframes_t) numSamples);
        }

        if (numDone < 0)
        {
            if (numDone == -(EPIPE))
                underrunCount++;

            if (DRX_ALSA_FAILED (snd_pcm_recover (handle, (i32) numDone, 1 /* silent */)))
                return false;
        }

        if (numDone < numSamples)
            DRX_ALSA_LOG ("Did not write all samples: numDone: " << numDone << ", numSamples: " << numSamples);

        return true;
    }

    b8 readFromInputDevice (AudioBuffer<f32>& inputChannelBuffer, i32k numSamples)
    {
        jassert (numChannelsRunning <= inputChannelBuffer.getNumChannels());
        f32* const* const data = inputChannelBuffer.getArrayOfWritePointers();

        if (isInterleaved)
        {
            scratch.ensureSize ((size_t) ((i32) sizeof (f32) * numSamples * numChannelsRunning), false);
            scratch.fillWith (0); // (not clearing this data causes warnings in valgrind)

            auto num = snd_pcm_readi (handle, scratch.getData(), (snd_pcm_uframes_t) numSamples);

            if (num < 0)
            {
                if (num == -(EPIPE))
                    overrunCount++;

                if (DRX_ALSA_FAILED (snd_pcm_recover (handle, (i32) num, 1 /* silent */)))
                    return false;
            }


            if (num < numSamples)
                DRX_ALSA_LOG ("Did not read all samples: num: " << num << ", numSamples: " << numSamples);

            for (i32 i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (data[i], 0, scratch.getData(), i, numSamples);
        }
        else
        {
            auto num = snd_pcm_readn (handle, (uk*) data, (snd_pcm_uframes_t) numSamples);

            if (num < 0)
            {
                if (num == -(EPIPE))
                    overrunCount++;

                if (DRX_ALSA_FAILED (snd_pcm_recover (handle, (i32) num, 1 /* silent */)))
                    return false;
            }

            if (num < numSamples)
                DRX_ALSA_LOG ("Did not read all samples: num: " << num << ", numSamples: " << numSamples);

            for (i32 i = 0; i < numChannelsRunning; ++i)
                converter->convertSamples (data[i], data[i], numSamples);
        }

        return true;
    }

    //==============================================================================
    snd_pcm_t* handle;
    Txt error;
    i32 bitDepth, numChannelsRunning, latency;
    i32 underrunCount = 0, overrunCount = 0;

private:
    //==============================================================================
    Txt deviceID;
    const b8 isInput;
    b8 isInterleaved;
    MemoryBlock scratch;
    std::unique_ptr<AudioData::Converter> converter;

    //==============================================================================
    template <class SampleType>
    struct ConverterHelper
    {
        static AudioData::Converter* createConverter (const b8 forInput, const b8 isLittleEndian, i32k numInterleavedChannels, b8 interleaved)
        {
            if (interleaved)
                return create<AudioData::Interleaved> (forInput, isLittleEndian, numInterleavedChannels);

            return create<AudioData::NonInterleaved> (forInput, isLittleEndian, numInterleavedChannels);
        }

    private:
        template <class InterleavedType>
        static AudioData::Converter* create (const b8 forInput, const b8 isLittleEndian, i32k numInterleavedChannels)
        {
            if (forInput)
            {
                using DestType = AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst>;

                if (isLittleEndian)
                    return new AudioData::ConverterInstance <AudioData::Pointer <SampleType, AudioData::LittleEndian, InterleavedType, AudioData::Const>, DestType> (numInterleavedChannels, 1);

                return new AudioData::ConverterInstance <AudioData::Pointer <SampleType, AudioData::BigEndian, InterleavedType, AudioData::Const>, DestType> (numInterleavedChannels, 1);
            }

            using SourceType = AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const>;

            if (isLittleEndian)
                return new AudioData::ConverterInstance <SourceType, AudioData::Pointer <SampleType, AudioData::LittleEndian, InterleavedType, AudioData::NonConst>> (1, numInterleavedChannels);

            return new AudioData::ConverterInstance <SourceType, AudioData::Pointer <SampleType, AudioData::BigEndian, InterleavedType, AudioData::NonConst>> (1, numInterleavedChannels);
        }
    };

    static AudioData::Converter* createConverter (b8 forInput, i32 bitDepth,
                                                  b8 isFloat, b8 isLittleEndian, b8 useOnlyLower24Bits,
                                                  i32 numInterleavedChannels,
                                                  b8 interleaved)
    {
        DRX_ALSA_LOG ("format: bitDepth=" << bitDepth << ", isFloat=" << (i32) isFloat
                        << ", isLittleEndian=" << (i32) isLittleEndian << ", numChannels=" << numInterleavedChannels);

        if (isFloat)         return ConverterHelper <AudioData::Float32>::createConverter (forInput, isLittleEndian, numInterleavedChannels, interleaved);
        if (bitDepth == 16)  return ConverterHelper <AudioData::Int16>  ::createConverter (forInput, isLittleEndian, numInterleavedChannels, interleaved);
        if (bitDepth == 24)  return ConverterHelper <AudioData::Int24>  ::createConverter (forInput, isLittleEndian, numInterleavedChannels, interleaved);

        jassert (bitDepth == 32);

        if (useOnlyLower24Bits)
            return ConverterHelper <AudioData::Int24in32>::createConverter (forInput, isLittleEndian, numInterleavedChannels, interleaved);

        return ConverterHelper <AudioData::Int32>::createConverter (forInput, isLittleEndian, numInterleavedChannels, interleaved);
    }

    //==============================================================================
    b8 failed (i32k errorNum)
    {
        if (errorNum >= 0)
            return false;

        error = snd_strerror (errorNum);
        DRX_ALSA_LOG ("ALSA error: " << error);
        return true;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALSADevice)
};

//==============================================================================
class ALSAThread final : public Thread
{
public:
    ALSAThread (const Txt& inputDeviceID, const Txt& outputDeviceID)
        : Thread (SystemStats::getDRXVersion() + ": ALSA"),
          inputId (inputDeviceID),
          outputId (outputDeviceID)
    {
        initialiseRatesAndChannels();
    }

    ~ALSAThread() override
    {
        close();
    }

    z0 open (BigInteger inputChannels,
               BigInteger outputChannels,
               f64 newSampleRate,
               i32 newBufferSize)
    {
        close();

        error.clear();
        sampleRate = newSampleRate;
        bufferSize = newBufferSize;

        i32 maxInputsRequested = inputChannels.getHighestBit() + 1;
        maxInputsRequested = jmax ((i32) minChansIn, jmin ((i32) maxChansIn, maxInputsRequested));

        inputChannelBuffer.setSize (maxInputsRequested, bufferSize);
        inputChannelBuffer.clear();
        inputChannelDataForCallback.clear();
        currentInputChans.clear();

        if (inputChannels.getHighestBit() >= 0)
        {
            for (i32 i = 0; i < maxInputsRequested; ++i)
            {
                if (inputChannels[i])
                {
                    inputChannelDataForCallback.add (inputChannelBuffer.getReadPointer (i));
                    currentInputChans.setBit (i);
                }
            }
        }

        ensureMinimumNumBitsSet (outputChannels, (i32) minChansOut);

        i32 maxOutputsRequested = outputChannels.getHighestBit() + 1;
        maxOutputsRequested = jmax ((i32) minChansOut, jmin ((i32) maxChansOut, maxOutputsRequested));

        outputChannelBuffer.setSize (maxOutputsRequested, bufferSize);
        outputChannelBuffer.clear();
        outputChannelDataForCallback.clear();
        currentOutputChans.clear();

        // Note that the input device is opened before an output, because we've heard
        // of drivers where doing it in the reverse order mysteriously fails.. If this
        // order also causes problems, let us know and we'll see if we can find a compromise!

        if (inputChannelDataForCallback.size() > 0 && inputId.isNotEmpty())
        {
            inputDevice.reset (new ALSADevice (inputId, true));

            if (inputDevice->error.isNotEmpty())
            {
                error = inputDevice->error;
                inputDevice.reset();
                return;
            }

            ensureMinimumNumBitsSet (currentInputChans, (i32) minChansIn);

            if (! inputDevice->setParameters ((u32) sampleRate,
                                              jlimit ((i32) minChansIn, (i32) maxChansIn, currentInputChans.getHighestBit() + 1),
                                              bufferSize))
            {
                error = inputDevice->error;
                inputDevice.reset();
                return;
            }

            inputLatency = inputDevice->latency;
        }

        if (outputChannels.getHighestBit() >= 0)
        {
            for (i32 i = 0; i < maxOutputsRequested; ++i)
            {
                if (outputChannels[i])
                {
                    outputChannelDataForCallback.add (outputChannelBuffer.getWritePointer (i));
                    currentOutputChans.setBit (i);
                }
            }
        }

        if (outputChannelDataForCallback.size() > 0 && outputId.isNotEmpty())
        {
            outputDevice.reset (new ALSADevice (outputId, false));

            if (outputDevice->error.isNotEmpty())
            {
                error = outputDevice->error;
                outputDevice.reset();
                return;
            }

            if (! outputDevice->setParameters ((u32) sampleRate,
                                               jlimit ((i32) minChansOut, (i32) maxChansOut,
                                                       currentOutputChans.getHighestBit() + 1),
                                               bufferSize))
            {
                error = outputDevice->error;
                outputDevice.reset();
                return;
            }

            outputLatency = outputDevice->latency;
        }

        if (outputDevice == nullptr && inputDevice == nullptr)
        {
            error = "no channels";
            return;
        }

        if (outputDevice != nullptr && inputDevice != nullptr)
            snd_pcm_link (outputDevice->handle, inputDevice->handle);

        if (inputDevice != nullptr && DRX_ALSA_FAILED (snd_pcm_prepare (inputDevice->handle)))
            return;

        if (outputDevice != nullptr && DRX_ALSA_FAILED (snd_pcm_prepare (outputDevice->handle)))
            return;

        startThread (Priority::high);

        i32 count = 1000;

        while (numCallbacks == 0)
        {
            sleep (5);

            if (--count < 0 || ! isThreadRunning())
            {
                error = "device didn't start";
                break;
            }
        }
    }

    z0 close()
    {
        if (isThreadRunning())
        {
            // problem: when pulseaudio is suspended (with pasuspend) , the ALSAThread::run is just stuck in
            // snd_pcm_writei -- no error, no nothing it just stays stuck. So the only way I found to exit "nicely"
            // (that is without the "killing thread by force" of stopThread) , is to just call snd_pcm_close from
            // here which will cause the thread to resume, and exit
            signalThreadShouldExit();

            i32k callbacksToStop = numCallbacks;

            if ((! waitForThreadToExit (400)) && audioIoInProgress && numCallbacks == callbacksToStop)
            {
                DRX_ALSA_LOG ("Thread is stuck in i/o.. Is pulseaudio suspended?");

                if (outputDevice != nullptr) outputDevice->closeNow();
                if (inputDevice != nullptr) inputDevice->closeNow();
            }
        }

        stopThread (6000);

        inputDevice.reset();
        outputDevice.reset();

        inputChannelBuffer.setSize (1, 1);
        outputChannelBuffer.setSize (1, 1);

        numCallbacks = 0;
    }

    z0 setCallback (AudioIODeviceCallback* const newCallback) noexcept
    {
        const ScopedLock sl (callbackLock);
        callback = newCallback;
    }

    z0 run() override
    {
        while (! threadShouldExit())
        {
            if (inputDevice != nullptr && inputDevice->handle != nullptr)
            {
                if (outputDevice == nullptr || outputDevice->handle == nullptr)
                {
                    DRX_ALSA_FAILED (snd_pcm_wait (inputDevice->handle, 2000));

                    if (threadShouldExit())
                        break;

                    auto avail = snd_pcm_avail_update (inputDevice->handle);

                    if (avail < 0)
                        DRX_ALSA_FAILED (snd_pcm_recover (inputDevice->handle, (i32) avail, 0));
                }

                audioIoInProgress = true;

                if (! inputDevice->readFromInputDevice (inputChannelBuffer, bufferSize))
                {
                    DRX_ALSA_LOG ("Read failure");
                    break;
                }

                audioIoInProgress = false;
            }

            if (threadShouldExit())
                break;

            {
                const ScopedLock sl (callbackLock);
                ++numCallbacks;

                if (callback != nullptr)
                {
                    callback->audioDeviceIOCallbackWithContext (inputChannelDataForCallback.getRawDataPointer(),
                                                                inputChannelDataForCallback.size(),
                                                                outputChannelDataForCallback.getRawDataPointer(),
                                                                outputChannelDataForCallback.size(),
                                                                bufferSize,
                                                                {});
                }
                else
                {
                    for (i32 i = 0; i < outputChannelDataForCallback.size(); ++i)
                        zeromem (outputChannelDataForCallback[i], (size_t) bufferSize * sizeof (f32));
                }
            }

            if (outputDevice != nullptr && outputDevice->handle != nullptr)
            {
                DRX_ALSA_FAILED (snd_pcm_wait (outputDevice->handle, 2000));

                if (threadShouldExit())
                    break;

                auto avail = snd_pcm_avail_update (outputDevice->handle);

                if (avail < 0)
                    DRX_ALSA_FAILED (snd_pcm_recover (outputDevice->handle, (i32) avail, 0));

                audioIoInProgress = true;

                if (! outputDevice->writeToOutputDevice (outputChannelBuffer, bufferSize))
                {
                    DRX_ALSA_LOG ("write failure");
                    break;
                }

                audioIoInProgress = false;
            }
        }

        audioIoInProgress = false;
    }

    i32 getBitDepth() const noexcept
    {
        if (outputDevice != nullptr)
            return outputDevice->bitDepth;

        if (inputDevice != nullptr)
            return inputDevice->bitDepth;

        return 16;
    }

    i32 getXRunCount() const noexcept
    {
        i32 result = 0;

        if (outputDevice != nullptr)
            result += outputDevice->underrunCount;

        if (inputDevice != nullptr)
            result += inputDevice->overrunCount;

        return result;
    }

    //==============================================================================
    Txt error;
    f64 sampleRate = 0;
    i32 bufferSize = 0, outputLatency = 0, inputLatency = 0;
    BigInteger currentInputChans, currentOutputChans;

    Array<f64> sampleRates;
    StringArray channelNamesOut, channelNamesIn;
    AudioIODeviceCallback* callback = nullptr;

private:
    //==============================================================================
    const Txt inputId, outputId;
    std::unique_ptr<ALSADevice> outputDevice, inputDevice;
    std::atomic<i32> numCallbacks { 0 };
    std::atomic<b8> audioIoInProgress { false };

    CriticalSection callbackLock;

    AudioBuffer<f32> inputChannelBuffer, outputChannelBuffer;
    Array<const f32*> inputChannelDataForCallback;
    Array<f32*> outputChannelDataForCallback;

    u32 minChansOut = 0, maxChansOut = 0;
    u32 minChansIn = 0, maxChansIn = 0;

    b8 failed (i32k errorNum)
    {
        if (errorNum >= 0)
            return false;

        error = snd_strerror (errorNum);
        DRX_ALSA_LOG ("ALSA error: " << error);
        return true;
    }

    z0 initialiseRatesAndChannels()
    {
        sampleRates.clear();
        channelNamesOut.clear();
        channelNamesIn.clear();
        minChansOut = 0;
        maxChansOut = 0;
        minChansIn = 0;
        maxChansIn = 0;
        u32 dummy = 0;

        getDeviceProperties (inputId, dummy, dummy, minChansIn, maxChansIn, sampleRates, false, true);
        getDeviceProperties (outputId, minChansOut, maxChansOut, dummy, dummy, sampleRates, true, false);

        for (u32 i = 0; i < maxChansOut; ++i)
            channelNamesOut.add ("channel " + Txt ((i32) i + 1));

        for (u32 i = 0; i < maxChansIn; ++i)
            channelNamesIn.add ("channel " + Txt ((i32) i + 1));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALSAThread)
};


//==============================================================================
class ALSAAudioIODevice final : public AudioIODevice
{
public:
    ALSAAudioIODevice (const Txt& deviceName,
                       const Txt& deviceTypeName,
                       const Txt& inputDeviceID,
                       const Txt& outputDeviceID)
        : AudioIODevice (deviceName, deviceTypeName),
          inputId (inputDeviceID),
          outputId (outputDeviceID),
          internal (inputDeviceID, outputDeviceID)
    {
    }

    ~ALSAAudioIODevice() override
    {
        close();
    }

    StringArray getOutputChannelNames() override            { return internal.channelNamesOut; }
    StringArray getInputChannelNames() override             { return internal.channelNamesIn; }

    Array<f64> getAvailableSampleRates() override        { return internal.sampleRates; }

    Array<i32> getAvailableBufferSizes() override
    {
        Array<i32> r;
        i32 n = 16;

        for (i32 i = 0; i < 50; ++i)
        {
            r.add (n);
            n += n < 64 ? 16
                        : (n < 512 ? 32
                                   : (n < 1024 ? 64
                                               : (n < 2048 ? 128 : 256)));
        }

        return r;
    }

    i32 getDefaultBufferSize() override                      { return 512; }

    Txt open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 f64 sampleRate,
                 i32 bufferSizeSamples) override
    {
        close();

        if (bufferSizeSamples <= 0)
            bufferSizeSamples = getDefaultBufferSize();

        if (sampleRate <= 0)
        {
            for (i32 i = 0; i < internal.sampleRates.size(); ++i)
            {
                f64 rate = internal.sampleRates[i];

                if (rate >= 44100)
                {
                    sampleRate = rate;
                    break;
                }
            }
        }

        internal.open (inputChannels, outputChannels,
                       sampleRate, bufferSizeSamples);

        isOpen_ = internal.error.isEmpty();
        return internal.error;
    }

    z0 close() override
    {
        stop();
        internal.close();
        isOpen_ = false;
    }

    b8 isOpen() override                           { return isOpen_; }
    b8 isPlaying() override                        { return isStarted && internal.error.isEmpty(); }
    Txt getLastError() override                   { return internal.error; }

    i32 getCurrentBufferSizeSamples() override       { return internal.bufferSize; }
    f64 getCurrentSampleRate() override           { return internal.sampleRate; }
    i32 getCurrentBitDepth() override                { return internal.getBitDepth(); }

    BigInteger getActiveOutputChannels() const override    { return internal.currentOutputChans; }
    BigInteger getActiveInputChannels() const override     { return internal.currentInputChans; }

    i32 getOutputLatencyInSamples() override         { return internal.outputLatency; }
    i32 getInputLatencyInSamples() override          { return internal.inputLatency; }

    i32 getXRunCount() const noexcept override       { return internal.getXRunCount(); }

    z0 start (AudioIODeviceCallback* callback) override
    {
        if (! isOpen_)
            callback = nullptr;

        if (callback != nullptr)
            callback->audioDeviceAboutToStart (this);

        internal.setCallback (callback);

        isStarted = (callback != nullptr);
    }

    z0 stop() override
    {
        auto oldCallback = internal.callback;

        start (nullptr);

        if (oldCallback != nullptr)
            oldCallback->audioDeviceStopped();
    }

    Txt inputId, outputId;

private:
    b8 isOpen_ = false, isStarted = false;
    ALSAThread internal;
};


//==============================================================================
class ALSAAudioIODeviceType final : public AudioIODeviceType
{
public:
    ALSAAudioIODeviceType (b8 onlySoundcards, const Txt& deviceTypeName)
        : AudioIODeviceType (deviceTypeName),
          listOnlySoundcards (onlySoundcards)
    {
       #if ! DRX_ALSA_LOGGING
        snd_lib_error_set_handler (&silentErrorHandler);
       #endif
    }

    ~ALSAAudioIODeviceType() override
    {
       #if ! DRX_ALSA_LOGGING
        snd_lib_error_set_handler (nullptr);
       #endif

        snd_config_update_free_global(); // prevent valgrind from screaming about alsa leaks
    }

    //==============================================================================
    z0 scanForDevices() override
    {
        if (hasScanned)
            return;

        hasScanned = true;
        inputNames.clear();
        inputIds.clear();
        outputNames.clear();
        outputIds.clear();

        DRX_ALSA_LOG ("scanForDevices()");

        if (listOnlySoundcards)
            enumerateAlsaSoundcards();
        else
            enumerateAlsaPCMDevices();

        inputNames.appendNumbersToDuplicates (false, true);
        outputNames.appendNumbersToDuplicates (false, true);
    }

    StringArray getDeviceNames (b8 wantInputNames) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputNames : outputNames;
    }

    i32 getDefaultDeviceIndex (b8 forInput) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        auto idx = (forInput ? inputIds : outputIds).indexOf ("default");
        return idx >= 0 ? idx : 0;
    }

    b8 hasSeparateInputsAndOutputs() const override { return true; }

    i32 getIndexOfDevice (AudioIODevice* device, b8 asInput) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (auto* d = dynamic_cast<ALSAAudioIODevice*> (device))
            return asInput ? inputIds.indexOf (d->inputId)
                           : outputIds.indexOf (d->outputId);

        return -1;
    }

    AudioIODevice* createDevice (const Txt& outputDeviceName,
                                 const Txt& inputDeviceName) override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        auto inputIndex = inputNames.indexOf (inputDeviceName);
        auto outputIndex = outputNames.indexOf (outputDeviceName);

        Txt deviceName (outputIndex >= 0 ? outputDeviceName
                                            : inputDeviceName);

        if (inputIndex >= 0 || outputIndex >= 0)
            return new ALSAAudioIODevice (deviceName, getTypeName(),
                                          inputIds [inputIndex],
                                          outputIds [outputIndex]);

        return nullptr;
    }

private:
    //==============================================================================
    StringArray inputNames, outputNames, inputIds, outputIds;
    b8 hasScanned = false;
    const b8 listOnlySoundcards;

    b8 testDevice (const Txt& id, const Txt& outputName, const Txt& inputName)
    {
        u32 minChansOut = 0, maxChansOut = 0;
        u32 minChansIn = 0, maxChansIn = 0;
        Array<f64> rates;

        b8 isInput = inputName.isNotEmpty(), isOutput = outputName.isNotEmpty();
        getDeviceProperties (id, minChansOut, maxChansOut, minChansIn, maxChansIn, rates, isOutput, isInput);

        isInput  = maxChansIn > 0;
        isOutput = maxChansOut > 0;

        if ((isInput || isOutput) && rates.size() > 0)
        {
            DRX_ALSA_LOG ("testDevice: '" << id.toUTF8().getAddress() << "' -> isInput: "
                            << (i32) isInput << ", isOutput: " << (i32) isOutput);

            if (isInput)
            {
                inputNames.add (inputName);
                inputIds.add (id);
            }

            if (isOutput)
            {
                outputNames.add (outputName);
                outputIds.add (id);
            }

            return isInput || isOutput;
        }

        return false;
    }

    z0 enumerateAlsaSoundcards()
    {
        snd_ctl_t* handle = nullptr;
        snd_ctl_card_info_t* info = nullptr;
        snd_ctl_card_info_alloca (&info);

        i32 cardNum = -1;

        while (outputIds.size() + inputIds.size() <= 64)
        {
            snd_card_next (&cardNum);

            if (cardNum < 0)
                break;

            if (DRX_CHECKED_RESULT (snd_ctl_open (&handle, ("hw:" + Txt (cardNum)).toRawUTF8(), SND_CTL_NONBLOCK)) >= 0)
            {
                if (DRX_CHECKED_RESULT (snd_ctl_card_info (handle, info)) >= 0)
                {
                    Txt cardId (snd_ctl_card_info_get_id (info));

                    if (cardId.removeCharacters ("0123456789").isEmpty())
                        cardId = Txt (cardNum);

                    Txt cardName = snd_ctl_card_info_get_name (info);

                    if (cardName.isEmpty())
                        cardName = cardId;

                    i32 device = -1;

                    snd_pcm_info_t* pcmInfo;
                    snd_pcm_info_alloca (&pcmInfo);

                    for (;;)
                    {
                        if (snd_ctl_pcm_next_device (handle, &device) < 0 || device < 0)
                            break;

                        snd_pcm_info_set_device (pcmInfo, (u32) device);

                        for (u32 subDevice = 0, nbSubDevice = 1; subDevice < nbSubDevice; ++subDevice)
                        {
                            snd_pcm_info_set_subdevice (pcmInfo, subDevice);
                            snd_pcm_info_set_stream (pcmInfo, SND_PCM_STREAM_CAPTURE);
                            const b8 isInput = (snd_ctl_pcm_info (handle, pcmInfo) >= 0);

                            snd_pcm_info_set_stream (pcmInfo, SND_PCM_STREAM_PLAYBACK);
                            const b8 isOutput = (snd_ctl_pcm_info (handle, pcmInfo) >= 0);

                            if (! (isInput || isOutput))
                                continue;

                            if (nbSubDevice == 1)
                                nbSubDevice = snd_pcm_info_get_subdevices_count (pcmInfo);

                            Txt id, name;

                            if (nbSubDevice == 1)
                            {
                                id << "hw:" << cardId << "," << device;
                                name << cardName << ", " << snd_pcm_info_get_name (pcmInfo);
                            }
                            else
                            {
                                id << "hw:" << cardId << "," << device << "," << (i32) subDevice;
                                name << cardName << ", " << snd_pcm_info_get_name (pcmInfo)
                                     << " {" <<  snd_pcm_info_get_subdevice_name (pcmInfo) << "}";
                            }

                            DRX_ALSA_LOG ("Soundcard ID: " << id << ", name: '" << name
                                            << ", isInput:"  << (i32) isInput
                                            << ", isOutput:" << (i32) isOutput << "\n");

                            if (isInput)
                            {
                                inputNames.add (name);
                                inputIds.add (id);
                            }

                            if (isOutput)
                            {
                                outputNames.add (name);
                                outputIds.add (id);
                            }
                        }
                    }
                }

                DRX_CHECKED_RESULT (snd_ctl_close (handle));
            }
        }
    }

    /* Enumerates all ALSA output devices (as output by the command aplay -L)
       Does not try to open the devices (with "testDevice" for example),
       so that it also finds devices that are busy and not yet available.
    */
    z0 enumerateAlsaPCMDevices()
    {
        uk* hints = nullptr;

        if (DRX_CHECKED_RESULT (snd_device_name_hint (-1, "pcm", &hints)) == 0)
        {
            for (tuk* h = (tuk*) hints; *h; ++h)
            {
                const Txt id (hintToString (*h, "NAME"));
                const Txt description (hintToString (*h, "DESC"));
                const Txt ioid (hintToString (*h, "IOID"));

                DRX_ALSA_LOG ("ID: " << id << "; desc: " << description << "; ioid: " << ioid);

                Txt ss = id.fromFirstOccurrenceOf ("=", false, false)
                              .upToFirstOccurrenceOf (",", false, false);

                if (id.isEmpty()
                     || id.startsWith ("default:") || id.startsWith ("sysdefault:")
                     || id.startsWith ("plughw:") || id == "null")
                    continue;

                Txt name (description.replace ("\n", "; "));

                if (name.isEmpty())
                    name = id;

                b8 isOutput = (ioid != "Input");
                b8 isInput  = (ioid != "Output");

                // alsa is stupid here, it advertises dmix and dsnoop as input/output devices, but
                // opening dmix as input, or dsnoop as output will trigger errors..
                isInput  = isInput  && ! id.startsWith ("dmix");
                isOutput = isOutput && ! id.startsWith ("dsnoop");

                if (isInput)
                {
                    inputNames.add (name);
                    inputIds.add (id);
                }

                if (isOutput)
                {
                    outputNames.add (name);
                    outputIds.add (id);
                }
            }

            snd_device_name_free_hint (hints);
        }

        // sometimes the "default" device is not listed, but it is nice to see it explicitly in the list
        if (! outputIds.contains ("default"))
            testDevice ("default", "Default ALSA Output", "Default ALSA Input");

        // same for the pulseaudio plugin
        if (! outputIds.contains ("pulse"))
            testDevice ("pulse", "Pulseaudio output", "Pulseaudio input");

        // make sure the default device is listed first, and followed by the pulse device (if present)
        auto idx = outputIds.indexOf ("pulse");
        outputIds.move (idx, 0);
        outputNames.move (idx, 0);

        idx = inputIds.indexOf ("pulse");
        inputIds.move (idx, 0);
        inputNames.move (idx, 0);

        idx = outputIds.indexOf ("default");
        outputIds.move (idx, 0);
        outputNames.move (idx, 0);

        idx = inputIds.indexOf ("default");
        inputIds.move (idx, 0);
        inputNames.move (idx, 0);
    }

    static Txt hintToString (ukk hints, tukk type)
    {
        tuk hint = snd_device_name_get_hint (hints, type);
        auto s = Txt::fromUTF8 (hint);
        ::free (hint);
        return s;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALSAAudioIODeviceType)
};

}

//==============================================================================
static inline AudioIODeviceType* createAudioIODeviceType_ALSA_Soundcards()
{
    return new ALSAAudioIODeviceType (true, "ALSA HW");
}

static inline AudioIODeviceType* createAudioIODeviceType_ALSA_PCMDevices()
{
    return new ALSAAudioIODeviceType (false, "ALSA");
}

} // namespace drx
