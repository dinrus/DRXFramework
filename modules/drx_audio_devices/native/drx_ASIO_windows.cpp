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

#undef WINDOWS

/* The ASIO SDK *should* declare its callback functions as being __cdecl, but different versions seem
   to be pretty random about whether or not they do this. If you hit an error using these functions
   it'll be because you're trying to build using __stdcall, in which case you'd need to either get hold of
   an ASIO SDK which correctly specifies __cdecl, or add the __cdecl keyword to its functions yourself.
*/
#define DRX_ASIOCALLBACK __cdecl

//==============================================================================
namespace ASIODebugging
{
   #if DRX_ASIO_DEBUGGING
    #define DRX_ASIO_LOG(msg)               ASIODebugging::logMessage (msg)
    #define DRX_ASIO_LOG_ERROR(msg, errNum) ASIODebugging::logError ((msg), (errNum))

    static z0 logMessage (Txt message)
    {
        message = "ASIO: " + message;
        DBG (message);

        if (Logger::getCurrentLogger() != nullptr)
            Logger::writeToLog (message);
    }

    static z0 logError (const Txt& context, i64 error)
    {
        tukk err = "Unknown error";

        switch (error)
        {
            case ASE_OK:               return;
            case ASE_NotPresent:       err = "Not Present"; break;
            case ASE_HWMalfunction:    err = "Hardware Malfunction"; break;
            case ASE_InvalidParameter: err = "Invalid Parameter"; break;
            case ASE_InvalidMode:      err = "Invalid Mode"; break;
            case ASE_SPNotAdvancing:   err = "Sample position not advancing"; break;
            case ASE_NoClock:          err = "No Clock"; break;
            case ASE_NoMemory:         err = "Out of memory"; break;
            default:                   break;
        }

        logMessage ("error: " + context + " - " + err);
    }
   #else
    static z0 dummyLog() {}
    #define DRX_ASIO_LOG(msg)               ASIODebugging::dummyLog()
    #define DRX_ASIO_LOG_ERROR(msg, errNum) ignoreUnused (errNum); ASIODebugging::dummyLog()
   #endif
}

//==============================================================================
struct ASIOSampleFormat
{
    ASIOSampleFormat() noexcept {}

    ASIOSampleFormat (i64 type) noexcept
    {
        switch (type)
        {
            case ASIOSTInt16MSB:    byteStride = 2; littleEndian = false; bitDepth = 16; break;
            case ASIOSTInt24MSB:    byteStride = 3; littleEndian = false; break;
            case ASIOSTInt32MSB:    bitDepth = 32; littleEndian = false; break;
            case ASIOSTFloat32MSB:  bitDepth = 32; littleEndian = false; formatIsFloat = true; break;
            case ASIOSTFloat64MSB:  bitDepth = 64; byteStride = 8; littleEndian = false; break;
            case ASIOSTInt32MSB16:  bitDepth = 16; littleEndian = false; break;
            case ASIOSTInt32MSB18:  littleEndian = false; break;
            case ASIOSTInt32MSB20:  littleEndian = false; break;
            case ASIOSTInt32MSB24:  littleEndian = false; break;
            case ASIOSTInt16LSB:    byteStride = 2; bitDepth = 16; break;
            case ASIOSTInt24LSB:    byteStride = 3; break;
            case ASIOSTInt32LSB:    bitDepth = 32; break;
            case ASIOSTFloat32LSB:  bitDepth = 32; formatIsFloat = true; break;
            case ASIOSTFloat64LSB:  bitDepth = 64; byteStride = 8; break;
            case ASIOSTInt32LSB16:  bitDepth = 16; break;
            case ASIOSTInt32LSB18:  break; // (unhandled)
            case ASIOSTInt32LSB20:  break; // (unhandled)
            case ASIOSTInt32LSB24:  break;

            case ASIOSTDSDInt8LSB1: break; // (unhandled)
            case ASIOSTDSDInt8MSB1: break; // (unhandled)
            case ASIOSTDSDInt8NER8: break; // (unhandled)

            default:
                jassertfalse;  // (not a valid format code..)
                break;
        }
    }

    z0 convertToFloat (ukk src, f32* dst, i32 samps) const noexcept
    {
        if (formatIsFloat)
        {
            memcpy (dst, src, samps * sizeof (f32));
        }
        else
        {
            switch (bitDepth)
            {
                case 16: convertInt16ToFloat (static_cast<tukk> (src), dst, byteStride, samps, littleEndian); break;
                case 24: convertInt24ToFloat (static_cast<tukk> (src), dst, byteStride, samps, littleEndian); break;
                case 32: convertInt32ToFloat (static_cast<tukk> (src), dst, byteStride, samps, littleEndian); break;
                default: jassertfalse; break;
            }
        }
    }

    z0 convertFromFloat (const f32* src, uk dst, i32 samps) const noexcept
    {
        if (formatIsFloat)
        {
            memcpy (dst, src, samps * sizeof (f32));
        }
        else
        {
            switch (bitDepth)
            {
                case 16: convertFloatToInt16 (src, static_cast<tuk> (dst), byteStride, samps, littleEndian); break;
                case 24: convertFloatToInt24 (src, static_cast<tuk> (dst), byteStride, samps, littleEndian); break;
                case 32: convertFloatToInt32 (src, static_cast<tuk> (dst), byteStride, samps, littleEndian); break;
                default: jassertfalse; break;
            }
        }
    }

    z0 clear (uk dst, i32 numSamps) noexcept
    {
        if (dst != nullptr)
            zeromem (dst, numSamps * byteStride);
    }

    i32 bitDepth = 24, byteStride = 4;
    b8 formatIsFloat = false, littleEndian = true;

private:
    static z0 convertInt16ToFloat (tukk src, f32* dest, i32 srcStrideBytes,
                                     i32 numSamples, b8 littleEndian) noexcept
    {
        const f64 g = 1.0 / 32768.0;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *dest++ = (f32) (g * (short) ByteOrder::littleEndianShort (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (f32) (g * (short) ByteOrder::bigEndianShort (src));
                src += srcStrideBytes;
            }
        }
    }

    static z0 convertFloatToInt16 (const f32* src, tuk dest, i32 dstStrideBytes,
                                     i32 numSamples, b8 littleEndian) noexcept
    {
        const f64 maxVal = (f64) 0x7fff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *(u16*) dest = ByteOrder::swapIfBigEndian ((u16) (short) roundToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *(u16*) dest = ByteOrder::swapIfLittleEndian ((u16) (short) roundToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
    }

    static z0 convertInt24ToFloat (tukk src, f32* dest, i32 srcStrideBytes,
                                     i32 numSamples, b8 littleEndian) noexcept
    {
        const f64 g = 1.0 / 0x7fffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *dest++ = (f32) (g * ByteOrder::littleEndian24Bit (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (f32) (g * ByteOrder::bigEndian24Bit (src));
                src += srcStrideBytes;
            }
        }
    }

    static z0 convertFloatToInt24 (const f32* src, tuk dest, i32 dstStrideBytes,
                                     i32 numSamples, b8 littleEndian) noexcept
    {
        const f64 maxVal = (f64) 0x7fffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                ByteOrder::littleEndian24BitToChars ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * *src++)), dest);
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                ByteOrder::bigEndian24BitToChars ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * *src++)), dest);
                dest += dstStrideBytes;
            }
        }
    }

    static z0 convertInt32ToFloat (tukk src, f32* dest, i32 srcStrideBytes,
                                     i32 numSamples, b8 littleEndian) noexcept
    {
        const f64 g = 1.0 / 0x7fffffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *dest++ = (f32) (g * (i32) ByteOrder::littleEndianInt (src));
                src += srcStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *dest++ = (f32) (g * (i32) ByteOrder::bigEndianInt (src));
                src += srcStrideBytes;
            }
        }
    }

    static z0 convertFloatToInt32 (const f32* src, tuk dest, i32 dstStrideBytes,
                                     i32 numSamples, b8 littleEndian) noexcept
    {
        const f64 maxVal = (f64) 0x7fffffff;

        if (littleEndian)
        {
            while (--numSamples >= 0)
            {
                *(u32*) dest = ByteOrder::swapIfBigEndian ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                *(u32*) dest = ByteOrder::swapIfLittleEndian ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * *src++)));
                dest += dstStrideBytes;
            }
        }
    }
};

//==============================================================================
constexpr i32 maxNumASIODevices = 16;
class ASIOAudioIODevice;
static ASIOAudioIODevice* currentASIODev[maxNumASIODevices] = {};

extern HWND drx_messageWindowHandle;

class ASIOAudioIODeviceType;
static z0 sendASIODeviceChangeToListeners (ASIOAudioIODeviceType*);

//==============================================================================
class ASIOAudioIODevice final : public AudioIODevice,
                                private Timer
{
public:
    ASIOAudioIODevice (ASIOAudioIODeviceType* ownerType, const Txt& devName,
                       CLSID clsID, i32 slotNumber)
       : AudioIODevice (devName, "ASIO"),
         owner (ownerType),
         classId (clsID)
    {
        ::CoInitialize (nullptr);

        name = devName;
        inBuffers.calloc (4);
        outBuffers.calloc (4);

        jassert (currentASIODev[slotNumber] == nullptr);
        currentASIODev[slotNumber] = this;

        openDevice();
    }

    ~ASIOAudioIODevice() override
    {
        for (i32 i = 0; i < maxNumASIODevices; ++i)
            if (currentASIODev[i] == this)
                currentASIODev[i] = nullptr;

        close();
        DRX_ASIO_LOG ("closed");

        if (! removeCurrentDriver())
            DRX_ASIO_LOG ("** Driver crashed while being closed");
    }

    z0 updateSampleRates()
    {
        // find a list of sample rates..
        Array<f64> newRates;

        if (asioObject != nullptr)
            for (const auto rate : SampleRateHelpers::getAllSampleRates())
                if (asioObject->canSampleRate (rate) == 0)
                    newRates.add (rate);

        if (newRates.isEmpty())
        {
            auto cr = getSampleRate();
            DRX_ASIO_LOG ("No sample rates supported - current rate: " + Txt ((i32) cr));

            if (cr > 0)
                newRates.add ((i32) cr);
        }

        if (sampleRates != newRates)
        {
            sampleRates.swapWith (newRates);

           #if DRX_ASIO_DEBUGGING
            StringArray s;

            for (auto r : sampleRates)
                s.add (Txt (r));

            DRX_ASIO_LOG ("Rates: " + s.joinIntoString (" "));
           #endif
        }
    }

    StringArray getOutputChannelNames() override        { return outputChannelNames; }
    StringArray getInputChannelNames() override         { return inputChannelNames; }

    Array<f64> getAvailableSampleRates() override    { return sampleRates; }
    Array<i32> getAvailableBufferSizes() override       { return bufferSizes; }
    i32 getDefaultBufferSize() override                 { return preferredBufferSize; }

    i32 getXRunCount() const noexcept override          { return xruns; }

    Txt open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 f64 sr, i32 bufferSizeSamples) override
    {
        if (isOpen())
            close();

        jassert (currentCallback == nullptr);

        if (bufferSizeSamples < 8 || bufferSizeSamples > 32768)
            shouldUsePreferredSize = true;

        if (asioObject == nullptr)
        {
            auto openingError = openDevice();

            if (asioObject == nullptr)
                return openingError;
        }

        isStarted = false;

        auto err = asioObject->getChannels (&totalNumInputChans, &totalNumOutputChans);
        jassert (err == ASE_OK);

        auto sampleRate = sr;
        currentSampleRate = sampleRate;
        currentChansOut.clear();
        currentChansIn.clear();

        updateSampleRates();

        if (sampleRate == 0 || (sampleRates.size() > 0 && ! sampleRates.contains (sampleRate)))
            sampleRate = sampleRates[0];

        if (sampleRate == 0)
        {
            jassertfalse;
            sampleRate = 44100.0;
        }

        updateClockSources();
        currentSampleRate = getSampleRate();

        error.clear();
        buffersCreated = false;

        setSampleRate (sampleRate);
        currentBlockSizeSamples = bufferSizeSamples = readBufferSizes (bufferSizeSamples);

        // (need to get this again in case a sample rate change affected the channel count)
        err = asioObject->getChannels (&totalNumInputChans, &totalNumOutputChans);
        jassert (err == ASE_OK);

        if (asioObject->future (kAsioCanReportOverload, nullptr) != ASE_OK)
            xruns = -1;

        inBuffers.calloc (totalNumInputChans + 8);
        outBuffers.calloc (totalNumOutputChans + 8);

        if (needToReset)
        {
            DRX_ASIO_LOG (" Resetting");

            if (! removeCurrentDriver())
                DRX_ASIO_LOG ("** Driver crashed while being closed");

            loadDriver();
            Txt initError = initDriver();

            if (initError.isNotEmpty())
                DRX_ASIO_LOG ("ASIOInit: " + initError);

            setSampleRate (getSampleRate());

            needToReset = false;
        }

        auto totalBuffers = resetBuffers (inputChannels, outputChannels);

        setCallbackFunctions();

        DRX_ASIO_LOG ("disposing buffers");
        err = asioObject->disposeBuffers();

        DRX_ASIO_LOG ("creating buffers: " + Txt (totalBuffers) + ", " + Txt (currentBlockSizeSamples));
        err = asioObject->createBuffers (bufferInfos, totalBuffers, currentBlockSizeSamples, &callbacks);

        if (err != ASE_OK)
        {
            currentBlockSizeSamples = preferredBufferSize;
            DRX_ASIO_LOG_ERROR ("create buffers 2", err);

            asioObject->disposeBuffers();
            err = asioObject->createBuffers (bufferInfos, totalBuffers, currentBlockSizeSamples, &callbacks);
        }

        if (err == ASE_OK)
        {
            buffersCreated = true;
            ioBufferSpace.calloc (totalBuffers * currentBlockSizeSamples + 32);

            i32 n = 0;
            Array<i32> types;
            currentBitDepth = 16;

            for (i32 i = 0; i < (i32) totalNumInputChans; ++i)
            {
                if (inputChannels[i])
                {
                    inBuffers[n] = ioBufferSpace + (currentBlockSizeSamples * n);

                    ASIOChannelInfo channelInfo = {};
                    channelInfo.channel = i;
                    channelInfo.isInput = 1;
                    asioObject->getChannelInfo (&channelInfo);

                    types.addIfNotAlreadyThere (channelInfo.type);
                    inputFormat[n] = ASIOSampleFormat (channelInfo.type);

                    currentBitDepth = jmax (currentBitDepth, inputFormat[n].bitDepth);
                    ++n;
                }
            }

            jassert (numActiveInputChans == n);
            n = 0;

            for (i32 i = 0; i < (i32) totalNumOutputChans; ++i)
            {
                if (outputChannels[i])
                {
                    outBuffers[n] = ioBufferSpace + (currentBlockSizeSamples * (numActiveInputChans + n));

                    ASIOChannelInfo channelInfo = {};
                    channelInfo.channel = i;
                    channelInfo.isInput = 0;
                    asioObject->getChannelInfo (&channelInfo);

                    types.addIfNotAlreadyThere (channelInfo.type);
                    outputFormat[n] = ASIOSampleFormat (channelInfo.type);

                    currentBitDepth = jmax (currentBitDepth, outputFormat[n].bitDepth);
                    ++n;
                }
            }

            jassert (numActiveOutputChans == n);

            for (i32 i = types.size(); --i >= 0;)
                DRX_ASIO_LOG ("channel format: " + Txt (types[i]));

            jassert (n <= totalBuffers);

            for (i32 i = 0; i < numActiveOutputChans; ++i)
            {
                outputFormat[i].clear (bufferInfos[numActiveInputChans + i].buffers[0], currentBlockSizeSamples);
                outputFormat[i].clear (bufferInfos[numActiveInputChans + i].buffers[1], currentBlockSizeSamples);
            }

            readLatencies();
            refreshBufferSizes();
            deviceIsOpen = true;

            DRX_ASIO_LOG ("starting");
            calledback = false;
            err = asioObject->start();

            if (err != 0)
            {
                deviceIsOpen = false;
                DRX_ASIO_LOG ("stop on failure");
                Thread::sleep (10);
                asioObject->stop();
                error = "Can't start device";
                Thread::sleep (10);
            }
            else
            {
                i32 count = 300;
                while (--count > 0 && ! calledback)
                    Thread::sleep (10);

                isStarted = true;

                if (! calledback)
                {
                    error = "Device didn't start correctly";
                    DRX_ASIO_LOG ("no callbacks - stopping..");
                    asioObject->stop();
                }
            }
        }
        else
        {
            error = "Can't create i/o buffers";
        }

        if (error.isNotEmpty())
        {
            DRX_ASIO_LOG_ERROR (error, err);
            disposeBuffers();

            Thread::sleep (20);
            isStarted = false;
            deviceIsOpen = false;

            auto errorCopy = error;
            close(); // (this resets the error string)
            error = errorCopy;
        }

        needToReset = false;
        return error;
    }

    z0 close() override
    {
        error.clear();
        stopTimer();
        stop();

        if (asioObject != nullptr && deviceIsOpen)
        {
            const ScopedLock sl (callbackLock);

            deviceIsOpen = false;
            isStarted = false;
            needToReset = false;

            DRX_ASIO_LOG ("stopping");

            if (asioObject != nullptr)
            {
                Thread::sleep (20);
                asioObject->stop();
                Thread::sleep (10);
                disposeBuffers();
            }

            Thread::sleep (10);
        }
    }

    b8 isOpen() override                       { return deviceIsOpen || insideControlPanelModalLoop; }
    b8 isPlaying() override                    { return asioObject != nullptr && currentCallback != nullptr; }

    i32 getCurrentBufferSizeSamples() override   { return currentBlockSizeSamples; }
    f64 getCurrentSampleRate() override       { return currentSampleRate; }
    i32 getCurrentBitDepth() override            { return currentBitDepth; }

    BigInteger getActiveOutputChannels() const override    { return currentChansOut; }
    BigInteger getActiveInputChannels() const override     { return currentChansIn; }

    i32 getOutputLatencyInSamples() override     { return outputLatency; }
    i32 getInputLatencyInSamples() override      { return inputLatency; }

    z0 start (AudioIODeviceCallback* callback) override
    {
        if (callback != nullptr)
        {
            callback->audioDeviceAboutToStart (this);

            const ScopedLock sl (callbackLock);
            currentCallback = callback;
        }
    }

    z0 stop() override
    {
        auto* lastCallback = currentCallback;

        {
            const ScopedLock sl (callbackLock);
            currentCallback = nullptr;
        }

        if (lastCallback != nullptr)
            lastCallback->audioDeviceStopped();
    }

    Txt getLastError() override           { return error; }
    b8 hasControlPanel() const override    { return true; }

    b8 showControlPanel() override
    {
        DRX_ASIO_LOG ("showing control panel");

        b8 done = false;
        insideControlPanelModalLoop = true;
        auto started = Time::getMillisecondCounter();

        if (asioObject != nullptr)
        {
            asioObject->controlPanel();

            auto spent = (i32) (Time::getMillisecondCounter() - started);
            DRX_ASIO_LOG ("spent: " + Txt (spent));

            if (spent > 300)
            {
                shouldUsePreferredSize = true;
                done = true;
            }
        }

        insideControlPanelModalLoop = false;
        return done;
    }

    z0 resetRequest() noexcept
    {
        startTimer (500);
    }

    z0 timerCallback() override
    {
        if (! insideControlPanelModalLoop)
        {
            stopTimer();
            DRX_ASIO_LOG ("restart request!");

            auto* oldCallback = currentCallback;
            close();
            needToReset = true;
            open (BigInteger (currentChansIn), BigInteger (currentChansOut),
                  currentSampleRate, currentBlockSizeSamples);

            reloadChannelNames();

            if (oldCallback != nullptr)
                start (oldCallback);

            sendASIODeviceChangeToListeners (owner);
        }
        else
        {
            startTimer (100);
        }
    }

private:
    //==============================================================================
    WeakReference<ASIOAudioIODeviceType> owner;
    IASIO* asioObject = {};
    ASIOCallbacks callbacks;

    CLSID classId;
    Txt error;

    i64 totalNumInputChans = 0, totalNumOutputChans = 0;
    StringArray inputChannelNames, outputChannelNames;

    Array<f64> sampleRates;
    Array<i32> bufferSizes;
    i64 inputLatency = 0, outputLatency = 0;
    i64 minBufferSize = 0, maxBufferSize = 0, preferredBufferSize = 0, bufferGranularity = 0;
    ASIOClockSource clocks[32] = {};
    i32 numClockSources = 0;

    i32 currentBlockSizeSamples = 0;
    i32 currentBitDepth = 16;
    f64 currentSampleRate = 0;
    BigInteger currentChansOut, currentChansIn;
    AudioIODeviceCallback* currentCallback = {};
    CriticalSection callbackLock;

    HeapBlock<ASIOBufferInfo> bufferInfos;
    HeapBlock<f32*> inBuffers, outBuffers;
    HeapBlock<f32> ioBufferSpace;
    HeapBlock<ASIOSampleFormat> inputFormat, outputFormat;
    i32 numActiveInputChans = 0, numActiveOutputChans = 0;

    b8 deviceIsOpen = false, isStarted = false, buffersCreated = false;
    std::atomic<b8> calledback { false };
    b8 postOutput = true, needToReset = false;
    b8 insideControlPanelModalLoop = false;
    b8 shouldUsePreferredSize = false;
    i32 xruns = 0;

    //==============================================================================
    static Txt convertASIOString (tuk text, i32 length)
    {
        if (CharPointer_UTF8::isValidString (text, length))
            return Txt::fromUTF8 (text, length);

        WCHAR wideVersion[512] = {};
        MultiByteToWideChar (CP_ACP, 0, text, length, wideVersion, numElementsInArray (wideVersion));
        return wideVersion;
    }

    Txt getChannelName (i32 index, b8 isInput) const
    {
        ASIOChannelInfo channelInfo = {};
        channelInfo.channel = index;
        channelInfo.isInput = isInput ? 1 : 0;
        asioObject->getChannelInfo (&channelInfo);

        return convertASIOString (channelInfo.name, sizeof (channelInfo.name));
    }

    z0 reloadChannelNames()
    {
        i64 totalInChannels = 0, totalOutChannels = 0;

        if (asioObject != nullptr
             && asioObject->getChannels (&totalInChannels, &totalOutChannels) == ASE_OK)
        {
            totalNumInputChans  = totalInChannels;
            totalNumOutputChans = totalOutChannels;

            inputChannelNames.clear();
            outputChannelNames.clear();

            for (i32 i = 0; i < totalNumInputChans; ++i)
                inputChannelNames.add (getChannelName (i, true));

            for (i32 i = 0; i < totalNumOutputChans; ++i)
                outputChannelNames.add (getChannelName (i, false));

            outputChannelNames.trim();
            inputChannelNames.trim();
            outputChannelNames.appendNumbersToDuplicates (false, true);
            inputChannelNames.appendNumbersToDuplicates (false, true);
        }
    }

    i64 refreshBufferSizes()
    {
        const auto err = asioObject->getBufferSize (&minBufferSize, &maxBufferSize, &preferredBufferSize, &bufferGranularity);

        if (err == ASE_OK)
        {
            bufferSizes.clear();
            addBufferSizes (minBufferSize, maxBufferSize, preferredBufferSize, bufferGranularity);
        }

        return err;
    }

    i32 readBufferSizes (i32 bufferSizeSamples)
    {
        minBufferSize = 0;
        maxBufferSize = 0;
        bufferGranularity = 0;
        i64 newPreferredSize = 0;

        if (asioObject->getBufferSize (&minBufferSize, &maxBufferSize, &newPreferredSize, &bufferGranularity) == ASE_OK)
        {
            if (preferredBufferSize != 0 && newPreferredSize != 0 && newPreferredSize != preferredBufferSize)
                shouldUsePreferredSize = true;

            if (bufferSizeSamples < minBufferSize || bufferSizeSamples > maxBufferSize)
                shouldUsePreferredSize = true;

            preferredBufferSize = newPreferredSize;
        }

        // unfortunate workaround for certain drivers which crash if you make
        // dynamic changes to the buffer size...
        shouldUsePreferredSize = shouldUsePreferredSize || getName().containsIgnoreCase ("Digidesign");

        if (shouldUsePreferredSize)
        {
            DRX_ASIO_LOG ("Using preferred size for buffer..");
            auto err = refreshBufferSizes();

            if (err == ASE_OK)
            {
                bufferSizeSamples = (i32) preferredBufferSize;
            }
            else
            {
                bufferSizeSamples = 1024;
                DRX_ASIO_LOG_ERROR ("getBufferSize1", err);
            }

            shouldUsePreferredSize = false;
        }

        return bufferSizeSamples;
    }

    i32 resetBuffers (const BigInteger& inputChannels,
                      const BigInteger& outputChannels)
    {
        numActiveInputChans = 0;
        numActiveOutputChans = 0;
        auto* info = bufferInfos.get();

        for (i32 i = 0; i < totalNumInputChans; ++i)
        {
            if (inputChannels[i])
            {
                currentChansIn.setBit (i);
                info->isInput = 1;
                info->channelNum = i;
                info->buffers[0] = info->buffers[1] = nullptr;
                ++info;
                ++numActiveInputChans;
            }
        }

        for (i32 i = 0; i < totalNumOutputChans; ++i)
        {
            if (outputChannels[i])
            {
                currentChansOut.setBit (i);
                info->isInput = 0;
                info->channelNum = i;
                info->buffers[0] = info->buffers[1] = nullptr;
                ++info;
                ++numActiveOutputChans;
            }
        }

        return numActiveInputChans + numActiveOutputChans;
    }

    z0 addBufferSizes (i64 minSize, i64 maxSize, i64 preferredSize, i64 granularity)
    {
        // find a list of buffer sizes..
        DRX_ASIO_LOG (Txt ((i32) minSize) + "->" + Txt ((i32) maxSize) + ", "
                        + Txt ((i32) preferredSize) + ", " + Txt ((i32) granularity));

        if (granularity >= 0)
        {
            granularity = jmax (16, (i32) granularity);

            for (i32 i = jmax ((i32) (minSize + 15) & ~15, (i32) granularity); i <= jmin (6400, (i32) maxSize); i += granularity)
                bufferSizes.addIfNotAlreadyThere (granularity * (i / granularity));
        }
        else if (granularity < 0)
        {
            for (i32 i = 0; i < 18; ++i)
            {
                i32k s = (1 << i);

                if (s >= minSize && s <= maxSize)
                    bufferSizes.add (s);
            }
        }

        bufferSizes.addIfNotAlreadyThere (preferredSize);
        bufferSizes.sort();
    }

    f64 getSampleRate() const
    {
        f64 cr = 0;
        auto err = asioObject->getSampleRate (&cr);
        DRX_ASIO_LOG_ERROR ("getSampleRate", err);
        return cr;
    }

    z0 setSampleRate (f64 newRate)
    {
        if (currentSampleRate != newRate)
        {
            DRX_ASIO_LOG ("rate change: " + Txt (currentSampleRate) + " to " + Txt (newRate));
            auto err = asioObject->setSampleRate (newRate);
            DRX_ASIO_LOG_ERROR ("setSampleRate", err);
            Thread::sleep (10);

            if (err == ASE_NoClock && numClockSources > 0)
            {
                DRX_ASIO_LOG ("trying to set a clock source..");
                err = asioObject->setClockSource (clocks[0].index);
                DRX_ASIO_LOG_ERROR ("setClockSource2", err);
                Thread::sleep (10);
                err = asioObject->setSampleRate (newRate);
                DRX_ASIO_LOG_ERROR ("setSampleRate", err);
                Thread::sleep (10);
            }

            if (err == 0)
                currentSampleRate = newRate;

            // on fail, ignore the attempt to change rate, and run with the current one..
        }
    }

    z0 updateClockSources()
    {
        zeromem (clocks, sizeof (clocks));
        i64 numSources = numElementsInArray (clocks);
        asioObject->getClockSources (clocks, &numSources);
        numClockSources = (i32) numSources;

        b8 isSourceSet = false;

        // careful not to remove this loop because it does more than just logging!
        for (i32 i = 0; i < numClockSources; ++i)
        {
            Txt s ("clock: ");
            s += clocks[i].name;

            if (clocks[i].isCurrentSource)
            {
                isSourceSet = true;
                s << " (cur)";
            }

            DRX_ASIO_LOG (s);
        }

        if (numClockSources > 1 && ! isSourceSet)
        {
            DRX_ASIO_LOG ("setting clock source");
            auto err = asioObject->setClockSource (clocks[0].index);
            DRX_ASIO_LOG_ERROR ("setClockSource1", err);
            Thread::sleep (20);
        }
        else
        {
            if (numClockSources == 0)
                DRX_ASIO_LOG ("no clock sources!");
        }
    }

    z0 readLatencies()
    {
        inputLatency = outputLatency = 0;

        if (asioObject->getLatencies (&inputLatency, &outputLatency) != 0)
            DRX_ASIO_LOG ("getLatencies() failed");
        else
            DRX_ASIO_LOG ("Latencies: in = " + Txt ((i32) inputLatency) + ", out = " + Txt ((i32) outputLatency));
    }

    z0 createDummyBuffers (i64 preferredSize)
    {
        numActiveInputChans = 0;
        numActiveOutputChans = 0;

        auto* info = bufferInfos.get();
        i32 numChans = 0;

        for (i32 i = 0; i < jmin (2, (i32) totalNumInputChans); ++i)
        {
            info->isInput = 1;
            info->channelNum = i;
            info->buffers[0] = info->buffers[1] = nullptr;
            ++info;
            ++numChans;
        }

        i32k outputBufferIndex = numChans;

        for (i32 i = 0; i < jmin (2, (i32) totalNumOutputChans); ++i)
        {
            info->isInput = 0;
            info->channelNum = i;
            info->buffers[0] = info->buffers[1] = nullptr;
            ++info;
            ++numChans;
        }

        setCallbackFunctions();

        DRX_ASIO_LOG ("creating buffers (dummy): " + Txt (numChans) + ", " + Txt ((i32) preferredSize));

        if (preferredSize > 0)
        {
            auto err = asioObject->createBuffers (bufferInfos, numChans, preferredSize, &callbacks);
            DRX_ASIO_LOG_ERROR ("dummy buffers", err);
        }

        i64 newInps = 0, newOuts = 0;
        asioObject->getChannels (&newInps, &newOuts);

        if (totalNumInputChans != newInps || totalNumOutputChans != newOuts)
        {
            totalNumInputChans = newInps;
            totalNumOutputChans = newOuts;

            DRX_ASIO_LOG (Txt ((i32) totalNumInputChans) + " in; " + Txt ((i32) totalNumOutputChans) + " out");
        }

        updateSampleRates();
        reloadChannelNames();

        for (i32 i = 0; i < totalNumOutputChans; ++i)
        {
            ASIOChannelInfo channelInfo = {};
            channelInfo.channel = i;
            channelInfo.isInput = 0;
            asioObject->getChannelInfo (&channelInfo);

            outputFormat[i] = ASIOSampleFormat (channelInfo.type);

            if (i < 2)
            {
                // clear the channels that are used with the dummy stuff
                outputFormat[i].clear (bufferInfos[outputBufferIndex + i].buffers[0], preferredBufferSize);
                outputFormat[i].clear (bufferInfos[outputBufferIndex + i].buffers[1], preferredBufferSize);
            }
        }
    }

    b8 removeCurrentDriver()
    {
        b8 releasedOK = true;

        if (asioObject != nullptr)
        {
            __try
            {
                asioObject->Release();
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { releasedOK = false; }

            asioObject = nullptr;
        }

        return releasedOK;
    }

    b8 loadDriver()
    {
        if (! removeCurrentDriver())
            DRX_ASIO_LOG ("** Driver crashed while being closed");

        b8 crashed = false;
        b8 ok = tryCreatingDriver (crashed);

        if (crashed)
            DRX_ASIO_LOG ("** Driver crashed while being opened");

        return ok;
    }

    b8 tryCreatingDriver (b8& crashed)
    {
        __try
        {
            return CoCreateInstance (classId, 0, CLSCTX_INPROC_SERVER,
                                     classId, (uk*) &asioObject) == S_OK;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { crashed = true; }
        return false;
    }

    Txt getLastDriverError() const
    {
        jassert (asioObject != nullptr);

        t8 buffer[512] = {};
        asioObject->getErrorMessage (buffer);

        return convertASIOString (buffer, sizeof (buffer));
    }

    Txt initDriver()
    {
        if (asioObject == nullptr)
            return "No Driver";

        auto initOk = (asioObject->init (drx_messageWindowHandle) > 0);
        Txt driverError;

        // Get error message if init() failed, or if it's a buggy Denon driver,
        // which returns true from init() even when it fails.
        if ((! initOk) || getName().containsIgnoreCase ("denon dj asio"))
            driverError = getLastDriverError();

        if ((! initOk) && driverError.isEmpty())
            driverError = "Driver failed to initialise";

        if (driverError.isEmpty())
        {
            t8 buffer[512] = {};
            asioObject->getDriverName (buffer); // just in case any flimsy drivers expect this to be called..
        }

        return driverError;
    }

    Txt openDevice()
    {
        // open the device and get its info..
        DRX_ASIO_LOG ("opening device: " + getName());

        needToReset = false;
        outputChannelNames.clear();
        inputChannelNames.clear();
        bufferSizes.clear();
        sampleRates.clear();
        deviceIsOpen = false;
        totalNumInputChans = 0;
        totalNumOutputChans = 0;
        numActiveInputChans = 0;
        numActiveOutputChans = 0;
        xruns = 0;
        currentCallback = nullptr;
        error.clear();

        if (getName().isEmpty())
            return error;

        i64 err = 0;

        if (loadDriver())
        {
            if ((error = initDriver()).isEmpty())
            {
                numActiveInputChans = 0;
                numActiveOutputChans = 0;
                totalNumInputChans = 0;
                totalNumOutputChans = 0;

                if (asioObject != nullptr
                     && (err = asioObject->getChannels (&totalNumInputChans, &totalNumOutputChans)) == 0)
                {
                    DRX_ASIO_LOG (Txt ((i32) totalNumInputChans) + " in, " + Txt ((i32) totalNumOutputChans) + " out");

                    i32k chansToAllocate = totalNumInputChans + totalNumOutputChans + 4;
                    bufferInfos.calloc (chansToAllocate);
                    inBuffers.calloc (chansToAllocate);
                    outBuffers.calloc (chansToAllocate);
                    inputFormat.calloc (chansToAllocate);
                    outputFormat.calloc (chansToAllocate);

                    if ((err = refreshBufferSizes()) == 0)
                    {
                        auto currentRate = getSampleRate();

                        if (currentRate < 1.0 || currentRate > 192001.0)
                        {
                            DRX_ASIO_LOG ("setting default sample rate");
                            err = asioObject->setSampleRate (44100.0);
                            DRX_ASIO_LOG_ERROR ("setting sample rate", err);

                            currentRate = getSampleRate();
                        }

                        currentSampleRate = currentRate;
                        postOutput = (asioObject->outputReady() == 0);

                        if (postOutput)
                            DRX_ASIO_LOG ("outputReady true");

                        updateSampleRates();
                        readLatencies();                          // ..doing these steps because cubase does so at this stage
                        createDummyBuffers (preferredBufferSize); // in initialisation, and some devices fail if we don't.
                        readLatencies();

                        // start and stop because cubase does it..
                        err = asioObject->start();
                        // ignore an error here, as it might start later after setting other stuff up
                        DRX_ASIO_LOG_ERROR ("start", err);

                        Thread::sleep (80);
                        asioObject->stop();
                    }
                    else
                    {
                        error = "Can't detect buffer sizes";
                    }
                }
                else
                {
                    error = "Can't detect asio channels";
                }
            }
        }
        else
        {
            error = "No such device";
        }

        if (error.isNotEmpty())
        {
            DRX_ASIO_LOG_ERROR (error, err);
            disposeBuffers();

            if (! removeCurrentDriver())
                DRX_ASIO_LOG ("** Driver crashed while being closed");
        }
        else
        {
            DRX_ASIO_LOG ("device open");
        }

        deviceIsOpen = false;
        needToReset = false;
        stopTimer();
        return error;
    }

    z0 disposeBuffers()
    {
        if (asioObject != nullptr && buffersCreated)
        {
            buffersCreated = false;
            asioObject->disposeBuffers();
        }
    }

    //==============================================================================
    z0 DRX_ASIOCALLBACK callback (i64 index)
    {
        if (isStarted)
        {
            processBuffer (index);
        }
        else
        {
            if (postOutput && (asioObject != nullptr))
                asioObject->outputReady();
        }

        calledback = true;
    }

    z0 processBuffer (i64 bufferIndex)
    {
        const ScopedLock sl (callbackLock);

        if (bufferIndex >= 0)
        {
            auto* infos = bufferInfos.get();
            auto samps = currentBlockSizeSamples;

            if (currentCallback != nullptr)
            {
                for (i32 i = 0; i < numActiveInputChans; ++i)
                {
                    jassert (inBuffers[i] != nullptr);
                    inputFormat[i].convertToFloat (infos[i].buffers[bufferIndex], inBuffers[i], samps);
                }

                currentCallback->audioDeviceIOCallbackWithContext (inBuffers.getData(),
                                                                   numActiveInputChans,
                                                                   outBuffers,
                                                                   numActiveOutputChans,
                                                                   samps,
                                                                   {});

                for (i32 i = 0; i < numActiveOutputChans; ++i)
                {
                    jassert (outBuffers[i] != nullptr);
                    outputFormat[i].convertFromFloat (outBuffers[i], infos[numActiveInputChans + i].buffers[bufferIndex], samps);
                }
            }
            else
            {
                for (i32 i = 0; i < numActiveOutputChans; ++i)
                     outputFormat[i].clear (infos[numActiveInputChans + i].buffers[bufferIndex], samps);
            }
        }

        if (postOutput)
            asioObject->outputReady();
    }

    i64 asioMessagesCallback (i64 selector, i64 value)
    {
        switch (selector)
        {
            case kAsioSelectorSupported:
                if (value == kAsioResetRequest || value == kAsioEngineVersion || value == kAsioResyncRequest
                     || value == kAsioLatenciesChanged || value == kAsioSupportsInputMonitor || value == kAsioOverload)
                    return 1;
                break;

            case kAsioBufferSizeChange:  DRX_ASIO_LOG ("kAsioBufferSizeChange"); resetRequest(); return 1;
            case kAsioResetRequest:      DRX_ASIO_LOG ("kAsioResetRequest");     resetRequest(); return 1;
            case kAsioResyncRequest:     DRX_ASIO_LOG ("kAsioResyncRequest");    resetRequest(); return 1;
            case kAsioLatenciesChanged:  DRX_ASIO_LOG ("kAsioLatenciesChanged"); return 1;
            case kAsioEngineVersion:     return 2;

            case kAsioSupportsTimeInfo:
            case kAsioSupportsTimeCode:  return 0;
            case kAsioOverload:          ++xruns; return 1;
        }

        return 0;
    }

    //==============================================================================
    template <i32 deviceIndex>
    struct ASIOCallbackFunctions
    {
        static ASIOTime* DRX_ASIOCALLBACK bufferSwitchTimeInfoCallback (ASIOTime*, i64 index, i64)
        {
            if (auto* d = currentASIODev[deviceIndex])
                d->callback (index);

            return {};
        }

        static z0 DRX_ASIOCALLBACK bufferSwitchCallback (i64 index, i64)
        {
            if (auto* d = currentASIODev[deviceIndex])
                d->callback (index);
        }

        static i64 DRX_ASIOCALLBACK asioMessagesCallback (i64 selector, i64 value, uk, f64*)
        {
            if (auto* d = currentASIODev[deviceIndex])
                return d->asioMessagesCallback (selector, value);

            return {};
        }

        static z0 DRX_ASIOCALLBACK sampleRateChangedCallback (ASIOSampleRate)
        {
            if (auto* d = currentASIODev[deviceIndex])
                d->resetRequest();
        }

        static z0 setCallbacks (ASIOCallbacks& callbacks) noexcept
        {
            callbacks.bufferSwitch          = &bufferSwitchCallback;
            callbacks.asioMessage           = &asioMessagesCallback;
            callbacks.bufferSwitchTimeInfo  = &bufferSwitchTimeInfoCallback;
            callbacks.sampleRateDidChange   = &sampleRateChangedCallback;
        }

        static z0 setCallbacksForDevice (ASIOCallbacks& callbacks, ASIOAudioIODevice* device) noexcept
        {
            if (currentASIODev[deviceIndex] == device)
                setCallbacks (callbacks);
            else
                ASIOCallbackFunctions<deviceIndex + 1>::setCallbacksForDevice (callbacks, device);
        }
    };

    z0 setCallbackFunctions() noexcept
    {
        ASIOCallbackFunctions<0>::setCallbacksForDevice (callbacks, this);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASIOAudioIODevice)
};

template <>
struct ASIOAudioIODevice::ASIOCallbackFunctions<maxNumASIODevices>
{
    static z0 setCallbacksForDevice (ASIOCallbacks&, ASIOAudioIODevice*) noexcept {}
};

//==============================================================================
class ASIOAudioIODeviceType final : public AudioIODeviceType
{
public:
    ASIOAudioIODeviceType() : AudioIODeviceType ("ASIO") {}

    //==============================================================================
    z0 scanForDevices() override
    {
        hasScanned = true;
        deviceNames.clear();
        classIds.clear();

        HKEY hk = 0;
        i32 index = 0;

        if (RegOpenKey (HKEY_LOCAL_MACHINE, _T ("software\\asio"), &hk) == ERROR_SUCCESS)
        {
            TCHAR name[256] = {};

            while (RegEnumKey (hk, index++, name, numElementsInArray (name)) == ERROR_SUCCESS)
            {
                if (isBlacklistedDriver (name))
                    continue;

                addDriverInfo (name, hk);
            }

            RegCloseKey (hk);
        }
    }

    StringArray getDeviceNames (b8 /*wantInputNames*/) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return deviceNames;
    }

    i32 getDefaultDeviceIndex (b8) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        for (i32 i = deviceNames.size(); --i >= 0;)
            if (deviceNames[i].containsIgnoreCase ("asio4all"))
                return i; // asio4all is a safe choice for a default..

       #if DRX_DEBUG
        if (deviceNames.size() > 1 && deviceNames[0].containsIgnoreCase ("digidesign"))
            return 1; // (the digi m-box driver crashes the app when you run
                      // it in the debugger, which can be a bit annoying)
       #endif

        return 0;
    }

    static i32 findFreeSlot()
    {
        for (i32 i = 0; i < maxNumASIODevices; ++i)
            if (currentASIODev[i] == nullptr)
                return i;

        jassertfalse;  // unfortunately you can only have a finite number
                       // of ASIO devices open at the same time..
        return -1;
    }

    i32 getIndexOfDevice (AudioIODevice* d, b8 /*asInput*/) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return d == nullptr ? -1 : deviceNames.indexOf (d->getName());
    }

    b8 hasSeparateInputsAndOutputs() const override    { return false; }

    AudioIODevice* createDevice (const Txt& outputDeviceName,
                                 const Txt& inputDeviceName) override
    {
        // ASIO can't open two different devices for input and output - they must be the same one.
        jassert (inputDeviceName == outputDeviceName || outputDeviceName.isEmpty() || inputDeviceName.isEmpty());
        jassert (hasScanned); // need to call scanForDevices() before doing this

        auto deviceName = outputDeviceName.isNotEmpty() ? outputDeviceName
                                                        : inputDeviceName;
        auto index = deviceNames.indexOf (deviceName);

        if (index >= 0)
        {
            auto freeSlot = findFreeSlot();

            if (freeSlot >= 0)
                return new ASIOAudioIODevice (this, deviceName,
                                              classIds.getReference (index), freeSlot);
        }

        return nullptr;
    }

    z0 sendDeviceChangeToListeners()
    {
        callDeviceChangeListeners();
    }

    DRX_DECLARE_WEAK_REFERENCEABLE (ASIOAudioIODeviceType)

private:
    StringArray deviceNames;
    Array<CLSID> classIds;

    b8 hasScanned = false;

    //==============================================================================
    static b8 checkClassIsOk (const Txt& classId)
    {
        HKEY hk = 0;
        b8 ok = false;

        if (RegOpenKey (HKEY_CLASSES_ROOT, _T ("clsid"), &hk) == ERROR_SUCCESS)
        {
            i32 index = 0;
            TCHAR name[512] = {};

            while (RegEnumKey (hk, index++, name, numElementsInArray (name)) == ERROR_SUCCESS)
            {
                if (classId.equalsIgnoreCase (name))
                {
                    HKEY subKey, pathKey;

                    if (RegOpenKeyEx (hk, name, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
                    {
                        if (RegOpenKeyEx (subKey, _T ("InprocServer32"), 0, KEY_READ, &pathKey) == ERROR_SUCCESS)
                        {
                            TCHAR pathName[1024] = {};
                            DWORD dtype = REG_SZ;
                            DWORD dsize = sizeof (pathName);

                            if (RegQueryValueEx (pathKey, 0, 0, &dtype, (LPBYTE) pathName, &dsize) == ERROR_SUCCESS)
                                // In older code, this used to check for the existence of the file, but there are situations
                                // where our process doesn't have access to it, but where the driver still loads ok..
                                ok = (pathName[0] != 0);

                            RegCloseKey (pathKey);
                        }

                        RegCloseKey (subKey);
                    }

                    break;
                }
            }

            RegCloseKey (hk);
        }

        return ok;
    }

    //==============================================================================
    static b8 isBlacklistedDriver (const Txt& driverName)
    {
        return driverName.startsWith ("ASIO DirectX Full Duplex") || driverName == "ASIO Multimedia Driver";
    }

    z0 addDriverInfo (const Txt& keyName, HKEY hk)
    {
        HKEY subKey;

        if (RegOpenKeyEx (hk, keyName.toWideCharPointer(), 0, KEY_READ, &subKey) == ERROR_SUCCESS)
        {
            TCHAR buf[256] = {};
            DWORD dtype = REG_SZ;
            DWORD dsize = sizeof (buf);

            if (RegQueryValueEx (subKey, _T ("clsid"), 0, &dtype, (LPBYTE) buf, &dsize) == ERROR_SUCCESS)
            {
                if (dsize > 0 && checkClassIsOk (buf))
                {
                    CLSID classId;

                    if (CLSIDFromString ((LPOLESTR) buf, &classId) == S_OK)
                    {
                        dtype = REG_SZ;
                        dsize = sizeof (buf);
                        Txt deviceName;

                        if (RegQueryValueEx (subKey, _T ("description"), 0, &dtype, (LPBYTE) buf, &dsize) == ERROR_SUCCESS)
                            deviceName = buf;
                        else
                            deviceName = keyName;

                        DRX_ASIO_LOG ("found " + deviceName);
                        deviceNames.add (deviceName);
                        classIds.add (classId);
                    }
                }

                RegCloseKey (subKey);
            }
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ASIOAudioIODeviceType)
};

z0 sendASIODeviceChangeToListeners (ASIOAudioIODeviceType* type)
{
    if (type != nullptr)
        type->sendDeviceChangeToListeners();
}

} // namespace drx
