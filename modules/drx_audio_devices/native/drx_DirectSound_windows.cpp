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

extern "C"
{
    // Declare just the minimum number of interfaces for the DSound objects that we need..
    struct DSBUFFERDESC
    {
        DWORD dwSize;
        DWORD dwFlags;
        DWORD dwBufferBytes;
        DWORD dwReserved;
        LPWAVEFORMATEX lpwfxFormat;
        GUID guid3DAlgorithm;
    };

    struct IDirectSoundBuffer;

    #undef INTERFACE
    #define INTERFACE IDirectSound
    DECLARE_INTERFACE_ (IDirectSound, IUnknown)
    {
        STDMETHOD  (QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
        STDMETHOD_ (ULONG,AddRef)         (THIS) PURE;
        STDMETHOD_ (ULONG,Release)        (THIS) PURE;
        STDMETHOD  (CreateSoundBuffer)    (THIS_ DSBUFFERDESC*, IDirectSoundBuffer**, LPUNKNOWN) PURE;
        STDMETHOD  (GetCaps)              (THIS_ uk) PURE;
        STDMETHOD  (DuplicateSoundBuffer) (THIS_ IDirectSoundBuffer*, IDirectSoundBuffer**) PURE;
        STDMETHOD  (SetCooperativeLevel)  (THIS_ HWND, DWORD) PURE;
        STDMETHOD  (Compact)              (THIS) PURE;
        STDMETHOD  (GetSpeakerConfig)     (THIS_ LPDWORD) PURE;
        STDMETHOD  (SetSpeakerConfig)     (THIS_ DWORD) PURE;
        STDMETHOD  (Initialize)           (THIS_ const GUID*) PURE;
    };

    #undef INTERFACE
    #define INTERFACE IDirectSoundBuffer
    DECLARE_INTERFACE_ (IDirectSoundBuffer, IUnknown)
    {
        STDMETHOD  (QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
        STDMETHOD_ (ULONG,AddRef)         (THIS) PURE;
        STDMETHOD_ (ULONG,Release)        (THIS) PURE;
        STDMETHOD  (GetCaps)              (THIS_ uk) PURE;
        STDMETHOD  (GetCurrentPosition)   (THIS_ LPDWORD, LPDWORD) PURE;
        STDMETHOD  (GetFormat)            (THIS_ LPWAVEFORMATEX, DWORD, LPDWORD) PURE;
        STDMETHOD  (GetVolume)            (THIS_ LPLONG) PURE;
        STDMETHOD  (GetPan)               (THIS_ LPLONG) PURE;
        STDMETHOD  (GetFrequency)         (THIS_ LPDWORD) PURE;
        STDMETHOD  (GetStatus)            (THIS_ LPDWORD) PURE;
        STDMETHOD  (Initialize)           (THIS_ IDirectSound*, DSBUFFERDESC*) PURE;
        STDMETHOD  (Lock)                 (THIS_ DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD, DWORD) PURE;
        STDMETHOD  (Play)                 (THIS_ DWORD, DWORD, DWORD) PURE;
        STDMETHOD  (SetCurrentPosition)   (THIS_ DWORD) PURE;
        STDMETHOD  (SetFormat)            (THIS_ const WAVEFORMATEX*) PURE;
        STDMETHOD  (SetVolume)            (THIS_ LONG) PURE;
        STDMETHOD  (SetPan)               (THIS_ LONG) PURE;
        STDMETHOD  (SetFrequency)         (THIS_ DWORD) PURE;
        STDMETHOD  (Stop)                 (THIS) PURE;
        STDMETHOD  (Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD) PURE;
        STDMETHOD  (Restore)              (THIS) PURE;
    };

    //==============================================================================
    struct DSCBUFFERDESC
    {
        DWORD dwSize;
        DWORD dwFlags;
        DWORD dwBufferBytes;
        DWORD dwReserved;
        LPWAVEFORMATEX lpwfxFormat;
    };

    struct IDirectSoundCaptureBuffer;

    #undef INTERFACE
    #define INTERFACE IDirectSoundCapture
    DECLARE_INTERFACE_ (IDirectSoundCapture, IUnknown)
    {
        STDMETHOD  (QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
        STDMETHOD_ (ULONG,AddRef)         (THIS) PURE;
        STDMETHOD_ (ULONG,Release)        (THIS) PURE;
        STDMETHOD  (CreateCaptureBuffer)  (THIS_ DSCBUFFERDESC*, IDirectSoundCaptureBuffer**, LPUNKNOWN) PURE;
        STDMETHOD  (GetCaps)              (THIS_ uk) PURE;
        STDMETHOD  (Initialize)           (THIS_ const GUID*) PURE;
    };

    #undef INTERFACE
    #define INTERFACE IDirectSoundCaptureBuffer
    DECLARE_INTERFACE_ (IDirectSoundCaptureBuffer, IUnknown)
    {
        STDMETHOD  (QueryInterface)       (THIS_ REFIID, LPVOID*) PURE;
        STDMETHOD_ (ULONG,AddRef)         (THIS) PURE;
        STDMETHOD_ (ULONG,Release)        (THIS) PURE;
        STDMETHOD  (GetCaps)              (THIS_ uk) PURE;
        STDMETHOD  (GetCurrentPosition)   (THIS_ LPDWORD, LPDWORD) PURE;
        STDMETHOD  (GetFormat)            (THIS_ LPWAVEFORMATEX, DWORD, LPDWORD) PURE;
        STDMETHOD  (GetStatus)            (THIS_ LPDWORD) PURE;
        STDMETHOD  (Initialize)           (THIS_ IDirectSoundCapture*, DSCBUFFERDESC*) PURE;
        STDMETHOD  (Lock)                 (THIS_ DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD, DWORD) PURE;
        STDMETHOD  (Start)                (THIS_ DWORD) PURE;
        STDMETHOD  (Stop)                 (THIS) PURE;
        STDMETHOD  (Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD) PURE;
    };

    #undef INTERFACE
}

namespace drx
{

//==============================================================================
namespace DSoundLogging
{
    static Txt getErrorMessage (HRESULT hr)
    {
        tukk result = nullptr;

        switch (hr)
        {
            case MAKE_HRESULT (1, 0x878, 10):    result = "Device already allocated"; break;
            case MAKE_HRESULT (1, 0x878, 30):    result = "Control unavailable"; break;
            case E_INVALIDARG:                   result = "Invalid parameter"; break;
            case MAKE_HRESULT (1, 0x878, 50):    result = "Invalid call"; break;
            case E_FAIL:                         result = "Generic error"; break;
            case MAKE_HRESULT (1, 0x878, 70):    result = "Priority level error"; break;
            case E_OUTOFMEMORY:                  result = "Out of memory"; break;
            case MAKE_HRESULT (1, 0x878, 100):   result = "Bad format"; break;
            case E_NOTIMPL:                      result = "Unsupported function"; break;
            case MAKE_HRESULT (1, 0x878, 120):   result = "No driver"; break;
            case MAKE_HRESULT (1, 0x878, 130):   result = "Already initialised"; break;
            case CLASS_E_NOAGGREGATION:          result = "No aggregation"; break;
            case MAKE_HRESULT (1, 0x878, 150):   result = "Buffer lost"; break;
            case MAKE_HRESULT (1, 0x878, 160):   result = "Another app has priority"; break;
            case MAKE_HRESULT (1, 0x878, 170):   result = "Uninitialised"; break;
            case E_NOINTERFACE:                  result = "No interface"; break;
            case S_OK:                           result = "No error"; break;
            default:                             return "Unknown error: " + Txt ((i32) hr);
        }

        return result;
    }

    //==============================================================================
   #if DRX_DIRECTSOUND_LOGGING
    static z0 logMessage (Txt message)
    {
        message = "DSOUND: " + message;
        DBG (message);
        Logger::writeToLog (message);
    }

    static z0 logError (HRESULT hr, i32 lineNum)
    {
        if (FAILED (hr))
        {
            Txt error ("Error at line ");
            error << lineNum << ": " << getErrorMessage (hr);
            logMessage (error);
        }
    }

    #define DRX_DS_LOG(a)        DSoundLogging::logMessage(a);
    #define DRX_DS_LOG_ERROR(a)  DSoundLogging::logError(a, __LINE__);
   #else
    #define DRX_DS_LOG(a)
    #define DRX_DS_LOG_ERROR(a)
   #endif
}

//==============================================================================
namespace
{
    #define DSOUND_FUNCTION(functionName, params) \
        typedef HRESULT (WINAPI *type##functionName) params; \
        static type##functionName ds##functionName = nullptr;

    #define DSOUND_FUNCTION_LOAD(functionName) \
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type") \
        ds##functionName = (type##functionName) GetProcAddress (h, #functionName); \
        DRX_END_IGNORE_WARNINGS_GCC_LIKE \
        jassert (ds##functionName != nullptr);

    typedef BOOL (CALLBACK *LPDSENUMCALLBACKW) (LPGUID, LPCWSTR, LPCWSTR, LPVOID);
    typedef BOOL (CALLBACK *LPDSENUMCALLBACKA) (LPGUID, LPCSTR, LPCSTR, LPVOID);

    DSOUND_FUNCTION (DirectSoundCreate, (const GUID*, IDirectSound**, LPUNKNOWN))
    DSOUND_FUNCTION (DirectSoundCaptureCreate, (const GUID*, IDirectSoundCapture**, LPUNKNOWN))
    DSOUND_FUNCTION (DirectSoundEnumerateW, (LPDSENUMCALLBACKW, LPVOID))
    DSOUND_FUNCTION (DirectSoundCaptureEnumerateW, (LPDSENUMCALLBACKW, LPVOID))

    z0 initialiseDSoundFunctions()
    {
        if (dsDirectSoundCreate == nullptr)
        {
            if (auto* h = LoadLibraryA ("dsound.dll"))
            {
                DSOUND_FUNCTION_LOAD (DirectSoundCreate)
                DSOUND_FUNCTION_LOAD (DirectSoundCaptureCreate)
                DSOUND_FUNCTION_LOAD (DirectSoundEnumerateW)
                DSOUND_FUNCTION_LOAD (DirectSoundCaptureEnumerateW)

                return;
            }

            jassertfalse;
        }
    }

    // the overall size of buffer used is this value x the block size
    enum { blocksPerOverallBuffer = 16 };
}

//==============================================================================
class DSoundInternalOutChannel
{
public:
    DSoundInternalOutChannel (const Txt& name_, const GUID& guid_, i32 rate,
                              i32 bufferSize, f32* left, f32* right)
        : bitDepth (16), name (name_), guid (guid_), sampleRate (rate),
          bufferSizeSamples (bufferSize), leftBuffer (left), rightBuffer (right),
          pDirectSound (nullptr), pOutputBuffer (nullptr)
    {
    }

    ~DSoundInternalOutChannel()
    {
        close();
    }

    z0 close()
    {
        if (pOutputBuffer != nullptr)
        {
            DRX_DS_LOG ("closing output: " + name);
            [[maybe_unused]] HRESULT hr = pOutputBuffer->Stop();
            DRX_DS_LOG_ERROR (hr);

            pOutputBuffer->Release();
            pOutputBuffer = nullptr;
        }

        if (pDirectSound != nullptr)
        {
            pDirectSound->Release();
            pDirectSound = nullptr;
        }
    }

    Txt open()
    {
        DRX_DS_LOG ("opening output: " + name + "  rate=" + Txt (sampleRate)
                       + " bits=" + Txt (bitDepth) + " buf=" + Txt (bufferSizeSamples));

        pDirectSound = nullptr;
        pOutputBuffer = nullptr;
        writeOffset = 0;
        xruns = 0;

        firstPlayTime = true;
        lastPlayTime = 0;

        Txt error;
        HRESULT hr = E_NOINTERFACE;

        if (dsDirectSoundCreate != nullptr)
            hr = dsDirectSoundCreate (&guid, &pDirectSound, nullptr);

        if (SUCCEEDED (hr))
        {
            bytesPerBuffer = (bufferSizeSamples * (bitDepth >> 2)) & ~15;
            ticksPerBuffer = bytesPerBuffer * Time::getHighResolutionTicksPerSecond() / (sampleRate * (bitDepth >> 2));
            totalBytesPerBuffer = (blocksPerOverallBuffer * bytesPerBuffer) & ~15;
            i32k numChannels = 2;

            hr = pDirectSound->SetCooperativeLevel (GetDesktopWindow(), 2 /* DSSCL_PRIORITY */);
            DRX_DS_LOG_ERROR (hr);

            if (SUCCEEDED (hr))
            {
                IDirectSoundBuffer* pPrimaryBuffer;

                DSBUFFERDESC primaryDesc = {};
                primaryDesc.dwSize = sizeof (DSBUFFERDESC);
                primaryDesc.dwFlags = 1 /* DSBCAPS_PRIMARYBUFFER */;
                primaryDesc.dwBufferBytes = 0;
                primaryDesc.lpwfxFormat = nullptr;

                DRX_DS_LOG ("co-op level set");
                hr = pDirectSound->CreateSoundBuffer (&primaryDesc, &pPrimaryBuffer, nullptr);
                DRX_DS_LOG_ERROR (hr);

                if (SUCCEEDED (hr))
                {
                    WAVEFORMATEX wfFormat;
                    wfFormat.wFormatTag       = WAVE_FORMAT_PCM;
                    wfFormat.nChannels        = (u16) numChannels;
                    wfFormat.nSamplesPerSec   = (DWORD) sampleRate;
                    wfFormat.wBitsPerSample   = (u16) bitDepth;
                    wfFormat.nBlockAlign      = (u16) (wfFormat.nChannels * wfFormat.wBitsPerSample / 8);
                    wfFormat.nAvgBytesPerSec  = wfFormat.nSamplesPerSec * wfFormat.nBlockAlign;
                    wfFormat.cbSize = 0;

                    hr = pPrimaryBuffer->SetFormat (&wfFormat);
                    DRX_DS_LOG_ERROR (hr);

                    if (SUCCEEDED (hr))
                    {
                        DSBUFFERDESC secondaryDesc = {};
                        secondaryDesc.dwSize = sizeof (DSBUFFERDESC);
                        secondaryDesc.dwFlags =  0x8000 /* DSBCAPS_GLOBALFOCUS */
                                                  | 0x10000 /* DSBCAPS_GETCURRENTPOSITION2 */;
                        secondaryDesc.dwBufferBytes = (DWORD) totalBytesPerBuffer;
                        secondaryDesc.lpwfxFormat = &wfFormat;

                        hr = pDirectSound->CreateSoundBuffer (&secondaryDesc, &pOutputBuffer, nullptr);
                        DRX_DS_LOG_ERROR (hr);

                        if (SUCCEEDED (hr))
                        {
                            DRX_DS_LOG ("buffer created");

                            DWORD dwDataLen;
                            u8* pDSBuffData;

                            hr = pOutputBuffer->Lock (0, (DWORD) totalBytesPerBuffer,
                                                      (LPVOID*) &pDSBuffData, &dwDataLen, nullptr, nullptr, 0);
                            DRX_DS_LOG_ERROR (hr);

                            if (SUCCEEDED (hr))
                            {
                                zeromem (pDSBuffData, dwDataLen);

                                hr = pOutputBuffer->Unlock (pDSBuffData, dwDataLen, nullptr, 0);

                                if (SUCCEEDED (hr))
                                {
                                    hr = pOutputBuffer->SetCurrentPosition (0);

                                    if (SUCCEEDED (hr))
                                    {
                                        hr = pOutputBuffer->Play (0, 0, 1 /* DSBPLAY_LOOPING */);

                                        if (SUCCEEDED (hr))
                                            return {};
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        error = DSoundLogging::getErrorMessage (hr);
        close();
        return error;
    }

    z0 synchronisePosition()
    {
        if (pOutputBuffer != nullptr)
        {
            DWORD playCursor;
            pOutputBuffer->GetCurrentPosition (&playCursor, &writeOffset);
        }
    }

    b8 service()
    {
        if (pOutputBuffer == nullptr)
            return true;

        DWORD playCursor, writeCursor;

        for (;;)
        {
            HRESULT hr = pOutputBuffer->GetCurrentPosition (&playCursor, &writeCursor);

            if (hr == MAKE_HRESULT (1, 0x878, 150)) // DSERR_BUFFERLOST
            {
                pOutputBuffer->Restore();
                continue;
            }

            if (SUCCEEDED (hr))
                break;

            DRX_DS_LOG_ERROR (hr);
            jassertfalse;
            return true;
        }

        auto currentPlayTime = Time::getHighResolutionTicks();
        if (! firstPlayTime)
        {
            auto expectedBuffers = (currentPlayTime - lastPlayTime) / ticksPerBuffer;

            playCursor += static_cast<DWORD> (expectedBuffers * bytesPerBuffer);
        }
        else
            firstPlayTime = false;

        lastPlayTime = currentPlayTime;

        i32 playWriteGap = (i32) (writeCursor - playCursor);
        if (playWriteGap < 0)
            playWriteGap += totalBytesPerBuffer;

        i32 bytesEmpty = (i32) (playCursor - writeOffset);
        if (bytesEmpty < 0)
            bytesEmpty += totalBytesPerBuffer;

        if (bytesEmpty > (totalBytesPerBuffer - playWriteGap))
        {
            writeOffset = writeCursor;
            bytesEmpty = totalBytesPerBuffer - playWriteGap;

            // buffer underflow
            xruns++;
        }

        if (bytesEmpty >= bytesPerBuffer)
        {
            i32* buf1 = nullptr;
            i32* buf2 = nullptr;
            DWORD dwSize1 = 0;
            DWORD dwSize2 = 0;

            HRESULT hr = pOutputBuffer->Lock (writeOffset, (DWORD) bytesPerBuffer,
                                              (uk*) &buf1, &dwSize1,
                                              (uk*) &buf2, &dwSize2, 0);

            if (hr == MAKE_HRESULT (1, 0x878, 150)) // DSERR_BUFFERLOST
            {
                pOutputBuffer->Restore();

                hr = pOutputBuffer->Lock (writeOffset, (DWORD) bytesPerBuffer,
                                          (uk*) &buf1, &dwSize1,
                                          (uk*) &buf2, &dwSize2, 0);
            }

            if (SUCCEEDED (hr))
            {
                if (bitDepth == 16)
                {
                    const f32* left = leftBuffer;
                    const f32* right = rightBuffer;
                    i32 samples1 = (i32) (dwSize1 >> 2);
                    i32 samples2 = (i32) (dwSize2 >> 2);

                    if (left == nullptr)
                    {
                        for (i32* dest = buf1; --samples1 >= 0;)  *dest++ = convertInputValues (0, *right++);
                        for (i32* dest = buf2; --samples2 >= 0;)  *dest++ = convertInputValues (0, *right++);
                    }
                    else if (right == nullptr)
                    {
                        for (i32* dest = buf1; --samples1 >= 0;)  *dest++ = convertInputValues (*left++, 0);
                        for (i32* dest = buf2; --samples2 >= 0;)  *dest++ = convertInputValues (*left++, 0);
                    }
                    else
                    {
                        for (i32* dest = buf1; --samples1 >= 0;)  *dest++ = convertInputValues (*left++, *right++);
                        for (i32* dest = buf2; --samples2 >= 0;)  *dest++ = convertInputValues (*left++, *right++);
                    }
                }
                else
                {
                    jassertfalse;
                }

                writeOffset = (writeOffset + dwSize1 + dwSize2) % (DWORD) totalBytesPerBuffer;

                pOutputBuffer->Unlock (buf1, dwSize1, buf2, dwSize2);
            }
            else
            {
                jassertfalse;
                DRX_DS_LOG_ERROR (hr);
            }

            bytesEmpty -= bytesPerBuffer;
            return true;
        }
        else
        {
            return false;
        }
    }

    i32 bitDepth, xruns;
    b8 doneFlag;

private:
    Txt name;
    GUID guid;
    i32 sampleRate, bufferSizeSamples;
    f32* leftBuffer;
    f32* rightBuffer;

    IDirectSound* pDirectSound;
    IDirectSoundBuffer* pOutputBuffer;
    DWORD writeOffset;
    i32 totalBytesPerBuffer, bytesPerBuffer;

    b8 firstPlayTime;
    z64 lastPlayTime, ticksPerBuffer;

    static i32 convertInputValues (const f32 l, const f32 r) noexcept
    {
        return jlimit (-32768, 32767, roundToInt (32767.0f * r)) << 16
                | (0xffff & jlimit (-32768, 32767, roundToInt (32767.0f * l)));
    }

    DRX_DECLARE_NON_COPYABLE (DSoundInternalOutChannel)
};

//==============================================================================
struct DSoundInternalInChannel
{
public:
    DSoundInternalInChannel (const Txt& name_, const GUID& guid_, i32 rate,
                             i32 bufferSize, f32* left, f32* right)
        : name (name_), guid (guid_), sampleRate (rate),
          bufferSizeSamples (bufferSize), leftBuffer (left), rightBuffer (right)
    {
    }

    ~DSoundInternalInChannel()
    {
        close();
    }

    z0 close()
    {
        if (pInputBuffer != nullptr)
        {
            DRX_DS_LOG ("closing input: " + name);
            [[maybe_unused]] HRESULT hr = pInputBuffer->Stop();
            DRX_DS_LOG_ERROR (hr);

            pInputBuffer->Release();
            pInputBuffer = nullptr;
        }

        if (pDirectSoundCapture != nullptr)
        {
            pDirectSoundCapture->Release();
            pDirectSoundCapture = nullptr;
        }

        if (pDirectSound != nullptr)
        {
            pDirectSound->Release();
            pDirectSound = nullptr;
        }
    }

    Txt open()
    {
        DRX_DS_LOG ("opening input: " + name
                       + "  rate=" + Txt (sampleRate) + " bits=" + Txt (bitDepth) + " buf=" + Txt (bufferSizeSamples));

        pDirectSound = nullptr;
        pDirectSoundCapture = nullptr;
        pInputBuffer = nullptr;
        readOffset = 0;
        totalBytesPerBuffer = 0;

        HRESULT hr = dsDirectSoundCaptureCreate != nullptr
                        ? dsDirectSoundCaptureCreate (&guid, &pDirectSoundCapture, nullptr)
                        : E_NOINTERFACE;

        if (SUCCEEDED (hr))
        {
            i32k numChannels = 2;
            bytesPerBuffer = (bufferSizeSamples * (bitDepth >> 2)) & ~15;
            totalBytesPerBuffer = (blocksPerOverallBuffer * bytesPerBuffer) & ~15;

            WAVEFORMATEX wfFormat;
            wfFormat.wFormatTag       = WAVE_FORMAT_PCM;
            wfFormat.nChannels        = (u16)numChannels;
            wfFormat.nSamplesPerSec   = (DWORD) sampleRate;
            wfFormat.wBitsPerSample   = (u16) bitDepth;
            wfFormat.nBlockAlign      = (u16) (wfFormat.nChannels * (wfFormat.wBitsPerSample / 8));
            wfFormat.nAvgBytesPerSec  = wfFormat.nSamplesPerSec * wfFormat.nBlockAlign;
            wfFormat.cbSize = 0;

            DSCBUFFERDESC captureDesc = {};
            captureDesc.dwSize = sizeof (DSCBUFFERDESC);
            captureDesc.dwFlags = 0;
            captureDesc.dwBufferBytes = (DWORD) totalBytesPerBuffer;
            captureDesc.lpwfxFormat = &wfFormat;

            DRX_DS_LOG ("object created");
            hr = pDirectSoundCapture->CreateCaptureBuffer (&captureDesc, &pInputBuffer, nullptr);

            if (SUCCEEDED (hr))
            {
                hr = pInputBuffer->Start (1 /* DSCBSTART_LOOPING */);

                if (SUCCEEDED (hr))
                    return {};
            }
        }

        DRX_DS_LOG_ERROR (hr);
        const Txt error (DSoundLogging::getErrorMessage (hr));
        close();

        return error;
    }

    z0 synchronisePosition()
    {
        if (pInputBuffer != nullptr)
        {
            DWORD capturePos;
            pInputBuffer->GetCurrentPosition (&capturePos, (DWORD*) &readOffset);
        }
    }

    b8 service()
    {
        if (pInputBuffer == nullptr)
            return true;

        DWORD capturePos, readPos;
        HRESULT hr = pInputBuffer->GetCurrentPosition (&capturePos, &readPos);
        DRX_DS_LOG_ERROR (hr);

        if (FAILED (hr))
            return true;

        i32 bytesFilled = (i32) (readPos - readOffset);

        if (bytesFilled < 0)
            bytesFilled += totalBytesPerBuffer;

        if (bytesFilled >= bytesPerBuffer)
        {
            short* buf1 = nullptr;
            short* buf2 = nullptr;
            DWORD dwsize1 = 0;
            DWORD dwsize2 = 0;

            hr = pInputBuffer->Lock ((DWORD) readOffset, (DWORD) bytesPerBuffer,
                                             (uk*) &buf1, &dwsize1,
                                             (uk*) &buf2, &dwsize2, 0);

            if (SUCCEEDED (hr))
            {
                if (bitDepth == 16)
                {
                    const f32 g = 1.0f / 32768.0f;

                    f32* destL = leftBuffer;
                    f32* destR = rightBuffer;
                    i32 samples1 = (i32) (dwsize1 >> 2);
                    i32 samples2 = (i32) (dwsize2 >> 2);

                    if (destL == nullptr)
                    {
                        for (const short* src = buf1; --samples1 >= 0;) { ++src; *destR++ = *src++ * g; }
                        for (const short* src = buf2; --samples2 >= 0;) { ++src; *destR++ = *src++ * g; }
                    }
                    else if (destR == nullptr)
                    {
                        for (const short* src = buf1; --samples1 >= 0;) { *destL++ = *src++ * g; ++src; }
                        for (const short* src = buf2; --samples2 >= 0;) { *destL++ = *src++ * g; ++src; }
                    }
                    else
                    {
                        for (const short* src = buf1; --samples1 >= 0;) { *destL++ = *src++ * g; *destR++ = *src++ * g; }
                        for (const short* src = buf2; --samples2 >= 0;) { *destL++ = *src++ * g; *destR++ = *src++ * g; }
                    }
                }
                else
                {
                    jassertfalse;
                }

                readOffset = (readOffset + dwsize1 + dwsize2) % (DWORD) totalBytesPerBuffer;

                pInputBuffer->Unlock (buf1, dwsize1, buf2, dwsize2);
            }
            else
            {
                DRX_DS_LOG_ERROR (hr);
                jassertfalse;
            }

            bytesFilled -= bytesPerBuffer;

            return true;
        }
        else
        {
            return false;
        }
    }

    u32 readOffset;
    i32 bytesPerBuffer, totalBytesPerBuffer;
    i32 bitDepth = 16;
    b8 doneFlag;

private:
    Txt name;
    GUID guid;
    i32 sampleRate, bufferSizeSamples;
    f32* leftBuffer;
    f32* rightBuffer;

    IDirectSound* pDirectSound = nullptr;
    IDirectSoundCapture* pDirectSoundCapture = nullptr;
    IDirectSoundCaptureBuffer* pInputBuffer = nullptr;

    DRX_DECLARE_NON_COPYABLE (DSoundInternalInChannel)
};

//==============================================================================
class DSoundAudioIODevice final : public AudioIODevice,
                                  public Thread
{
public:
    DSoundAudioIODevice (const Txt& deviceName,
                         i32k outputDeviceIndex_,
                         i32k inputDeviceIndex_)
        : AudioIODevice (deviceName, "DirectSound"),
          Thread (SystemStats::getDRXVersion() + ": DSound"),
          outputDeviceIndex (outputDeviceIndex_),
          inputDeviceIndex (inputDeviceIndex_)
    {
        if (outputDeviceIndex_ >= 0)
        {
            outChannels.add (TRANS ("Left"));
            outChannels.add (TRANS ("Right"));
        }

        if (inputDeviceIndex_ >= 0)
        {
            inChannels.add (TRANS ("Left"));
            inChannels.add (TRANS ("Right"));
        }
    }

    ~DSoundAudioIODevice() override
    {
        close();
    }

    Txt open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 f64 newSampleRate, i32 newBufferSize) override
    {
        lastError = openDevice (inputChannels, outputChannels, newSampleRate, newBufferSize);
        isOpen_ = lastError.isEmpty();

        return lastError;
    }

    z0 close() override
    {
        stop();

        if (isOpen_)
        {
            closeDevice();
            isOpen_ = false;
        }
    }

    b8 isOpen() override                              { return isOpen_ && isThreadRunning(); }
    i32 getCurrentBufferSizeSamples() override          { return bufferSizeSamples; }
    f64 getCurrentSampleRate() override              { return sampleRate; }
    BigInteger getActiveOutputChannels() const override { return enabledOutputs; }
    BigInteger getActiveInputChannels() const override  { return enabledInputs; }
    i32 getOutputLatencyInSamples() override            { return (i32) (getCurrentBufferSizeSamples() * 1.5); }
    i32 getInputLatencyInSamples() override             { return getOutputLatencyInSamples(); }
    StringArray getOutputChannelNames() override        { return outChannels; }
    StringArray getInputChannelNames() override         { return inChannels; }

    Array<f64> getAvailableSampleRates() override
    {
        return { 44100.0, 48000.0, 88200.0, 96000.0 };
    }

    Array<i32> getAvailableBufferSizes() override
    {
        Array<i32> r;
        i32 n = 64;

        for (i32 i = 0; i < 50; ++i)
        {
            r.add (n);
            n += (n < 512) ? 32
                           : ((n < 1024) ? 64
                                         : ((n < 2048) ? 128 : 256));
        }

        return r;
    }

    i32 getDefaultBufferSize() override                 { return 2560; }

    i32 getCurrentBitDepth() override
    {
        i32 bits = 256;

        for (i32 i = inChans.size(); --i >= 0;)
            bits = jmin (bits, inChans[i]->bitDepth);

        for (i32 i = outChans.size(); --i >= 0;)
            bits = jmin (bits, outChans[i]->bitDepth);

        if (bits > 32)
            bits = 16;

        return bits;
    }

    z0 start (AudioIODeviceCallback* call) override
    {
        if (isOpen_ && call != nullptr && ! isStarted)
        {
            if (! isThreadRunning())
            {
                // something gone wrong and the thread's stopped..
                isOpen_ = false;
                return;
            }

            call->audioDeviceAboutToStart (this);

            const ScopedLock sl (startStopLock);
            callback = call;
            isStarted = true;
        }
    }

    z0 stop() override
    {
        if (isStarted)
        {
            auto* callbackLocal = callback;

            {
                const ScopedLock sl (startStopLock);
                isStarted = false;
            }

            if (callbackLocal != nullptr)
                callbackLocal->audioDeviceStopped();
        }
    }

    b8 isPlaying() override            { return isStarted && isOpen_ && isThreadRunning(); }
    Txt getLastError() override       { return lastError; }

    i32 getXRunCount() const noexcept override
    {
        return outChans[0] != nullptr ? outChans[0]->xruns : -1;
    }

    //==============================================================================
    StringArray inChannels, outChannels;
    i32 outputDeviceIndex, inputDeviceIndex;

private:
    b8 isOpen_ = false;
    b8 isStarted = false;
    Txt lastError;

    OwnedArray<DSoundInternalInChannel> inChans;
    OwnedArray<DSoundInternalOutChannel> outChans;
    WaitableEvent startEvent;

    i32 bufferSizeSamples = 0;
    f64 sampleRate = 0;
    BigInteger enabledInputs, enabledOutputs;
    AudioBuffer<f32> inputBuffers, outputBuffers;

    AudioIODeviceCallback* callback = nullptr;
    CriticalSection startStopLock;

    Txt openDevice (const BigInteger& inputChannels,
                       const BigInteger& outputChannels,
                       f64 sampleRate_, i32 bufferSizeSamples_);

    z0 closeDevice()
    {
        isStarted = false;
        stopThread (5000);

        inChans.clear();
        outChans.clear();
    }

    z0 resync()
    {
        if (! threadShouldExit())
        {
            sleep (5);

            for (i32 i = 0; i < outChans.size(); ++i)
                outChans.getUnchecked (i)->synchronisePosition();

            for (i32 i = 0; i < inChans.size(); ++i)
                inChans.getUnchecked (i)->synchronisePosition();
        }
    }

public:
    z0 run() override
    {
        while (! threadShouldExit())
        {
            if (wait (100))
                break;
        }

        const auto latencyMs = (u32) (bufferSizeSamples * 1000.0 / sampleRate);
        const auto maxTimeMS = jmax ((u32) 5, 3 * latencyMs);

        while (! threadShouldExit())
        {
            i32 numToDo = 0;
            u32 startTime = Time::getMillisecondCounter();

            for (i32 i = inChans.size(); --i >= 0;)
            {
                inChans.getUnchecked (i)->doneFlag = false;
                ++numToDo;
            }

            for (i32 i = outChans.size(); --i >= 0;)
            {
                outChans.getUnchecked (i)->doneFlag = false;
                ++numToDo;
            }

            if (numToDo > 0)
            {
                i32k maxCount = 3;
                i32 count = maxCount;

                for (;;)
                {
                    for (i32 i = inChans.size(); --i >= 0;)
                    {
                        DSoundInternalInChannel* const in = inChans.getUnchecked (i);

                        if ((! in->doneFlag) && in->service())
                        {
                            in->doneFlag = true;
                            --numToDo;
                        }
                    }

                    for (i32 i = outChans.size(); --i >= 0;)
                    {
                        DSoundInternalOutChannel* const out = outChans.getUnchecked (i);

                        if ((! out->doneFlag) && out->service())
                        {
                            out->doneFlag = true;
                            --numToDo;
                        }
                    }

                    if (numToDo <= 0)
                        break;

                    if (Time::getMillisecondCounter() > startTime + maxTimeMS)
                    {
                        resync();
                        break;
                    }

                    if (--count <= 0)
                    {
                        Sleep (1);
                        count = maxCount;
                    }

                    if (threadShouldExit())
                        return;
                }
            }
            else
            {
                sleep (1);
            }

            const ScopedLock sl (startStopLock);

            if (isStarted)
            {
                callback->audioDeviceIOCallbackWithContext (inputBuffers.getArrayOfReadPointers(),
                                                            inputBuffers.getNumChannels(),
                                                            outputBuffers.getArrayOfWritePointers(),
                                                            outputBuffers.getNumChannels(),
                                                            bufferSizeSamples,
                                                            {});
            }
            else
            {
                outputBuffers.clear();
                sleep (1);
            }
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSoundAudioIODevice)
};

//==============================================================================
class DSoundDeviceList
{
    auto tie() const
    {
        return std::tie (outputDeviceNames, inputDeviceNames, outputGuids, inputGuids);
    }

public:
    StringArray outputDeviceNames, inputDeviceNames;
    Array<GUID> outputGuids, inputGuids;

    z0 scan()
    {
        outputDeviceNames.clear();
        inputDeviceNames.clear();
        outputGuids.clear();
        inputGuids.clear();

        if (dsDirectSoundEnumerateW != nullptr)
        {
            dsDirectSoundEnumerateW (outputEnumProcW, this);
            dsDirectSoundCaptureEnumerateW (inputEnumProcW, this);
        }
    }

    b8 operator== (const DSoundDeviceList& other) const noexcept { return tie() == other.tie(); }
    b8 operator!= (const DSoundDeviceList& other) const noexcept { return tie() != other.tie(); }

private:
    static BOOL enumProc (LPGUID lpGUID, Txt desc, StringArray& names, Array<GUID>& guids)
    {
        desc = desc.trim();

        if (desc.isNotEmpty())
        {
            const Txt origDesc (desc);

            i32 n = 2;
            while (names.contains (desc))
                desc = origDesc + " (" + Txt (n++) + ")";

            names.add (desc);
            guids.add (lpGUID != nullptr ? *lpGUID : GUID());
        }

        return TRUE;
    }

    BOOL outputEnumProc (LPGUID guid, LPCWSTR desc)  { return enumProc (guid, desc, outputDeviceNames, outputGuids); }
    BOOL inputEnumProc  (LPGUID guid, LPCWSTR desc)  { return enumProc (guid, desc, inputDeviceNames,  inputGuids); }

    static BOOL CALLBACK outputEnumProcW (LPGUID lpGUID, LPCWSTR description, LPCWSTR, LPVOID object)
    {
        return static_cast<DSoundDeviceList*> (object)->outputEnumProc (lpGUID, description);
    }

    static BOOL CALLBACK inputEnumProcW (LPGUID lpGUID, LPCWSTR description, LPCWSTR, LPVOID object)
    {
        return static_cast<DSoundDeviceList*> (object)->inputEnumProc (lpGUID, description);
    }
};

//==============================================================================
Txt DSoundAudioIODevice::openDevice (const BigInteger& inputChannels,
                                        const BigInteger& outputChannels,
                                        f64 sampleRate_, i32 bufferSizeSamples_)
{
    closeDevice();

    sampleRate = sampleRate_ > 0.0 ? sampleRate_ : 44100.0;

    if (bufferSizeSamples_ <= 0)
        bufferSizeSamples_ = 960; // use as a default size if none is set.

    bufferSizeSamples = bufferSizeSamples_ & ~7;

    DSoundDeviceList dlh;
    dlh.scan();

    enabledInputs = inputChannels;
    enabledInputs.setRange (inChannels.size(),
                            enabledInputs.getHighestBit() + 1 - inChannels.size(),
                            false);

    inputBuffers.setSize (enabledInputs.countNumberOfSetBits(), bufferSizeSamples);
    inputBuffers.clear();
    i32 numIns = 0;

    for (i32 i = 0; i <= enabledInputs.getHighestBit(); i += 2)
    {
        f32* left  = enabledInputs[i]     ? inputBuffers.getWritePointer (numIns++) : nullptr;
        f32* right = enabledInputs[i + 1] ? inputBuffers.getWritePointer (numIns++) : nullptr;

        if (left != nullptr || right != nullptr)
            inChans.add (new DSoundInternalInChannel (dlh.inputDeviceNames [inputDeviceIndex],
                                                      dlh.inputGuids [inputDeviceIndex],
                                                      (i32) sampleRate, bufferSizeSamples,
                                                      left, right));
    }

    enabledOutputs = outputChannels;
    enabledOutputs.setRange (outChannels.size(),
                             enabledOutputs.getHighestBit() + 1 - outChannels.size(),
                             false);

    outputBuffers.setSize (enabledOutputs.countNumberOfSetBits(), bufferSizeSamples);
    outputBuffers.clear();
    i32 numOuts = 0;

    for (i32 i = 0; i <= enabledOutputs.getHighestBit(); i += 2)
    {
        f32* left  = enabledOutputs[i]     ? outputBuffers.getWritePointer (numOuts++) : nullptr;
        f32* right = enabledOutputs[i + 1] ? outputBuffers.getWritePointer (numOuts++) : nullptr;

        if (left != nullptr || right != nullptr)
            outChans.add (new DSoundInternalOutChannel (dlh.outputDeviceNames[outputDeviceIndex],
                                                        dlh.outputGuids [outputDeviceIndex],
                                                        (i32) sampleRate, bufferSizeSamples,
                                                        left, right));
    }

    Txt error;

    // boost our priority while opening the devices to try to get better sync between them
    i32k oldThreadPri = GetThreadPriority (GetCurrentThread());
    const DWORD oldProcPri = GetPriorityClass (GetCurrentProcess());
    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    SetPriorityClass (GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    for (i32 i = 0; i < outChans.size(); ++i)
    {
        error = outChans[i]->open();

        if (error.isNotEmpty())
        {
            error = "Error opening " + dlh.outputDeviceNames[i] + ": \"" + error + "\"";
            break;
        }
    }

    if (error.isEmpty())
    {
        for (i32 i = 0; i < inChans.size(); ++i)
        {
            error = inChans[i]->open();

            if (error.isNotEmpty())
            {
                error = "Error opening " + dlh.inputDeviceNames[i] + ": \"" + error + "\"";
                break;
            }
        }
    }

    if (error.isEmpty())
    {
        for (i32 i = 0; i < outChans.size(); ++i)
            outChans.getUnchecked (i)->synchronisePosition();

        for (i32 i = 0; i < inChans.size(); ++i)
            inChans.getUnchecked (i)->synchronisePosition();

        startThread (Priority::highest);
        sleep (10);

        notify();
    }
    else
    {
        DRX_DS_LOG ("Opening failed: " + error);
    }

    SetThreadPriority (GetCurrentThread(), oldThreadPri);
    SetPriorityClass (GetCurrentProcess(), oldProcPri);

    return error;
}

//==============================================================================
class DSoundAudioIODeviceType final : public AudioIODeviceType
{
public:
    DSoundAudioIODeviceType()
        : AudioIODeviceType ("DirectSound")
    {
        initialiseDSoundFunctions();
    }

    z0 scanForDevices() override
    {
        hasScanned = true;
        deviceList.scan();
    }

    StringArray getDeviceNames (b8 wantInputNames) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? deviceList.inputDeviceNames
                              : deviceList.outputDeviceNames;
    }

    i32 getDefaultDeviceIndex (b8 /*forInput*/) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    i32 getIndexOfDevice (AudioIODevice* device, b8 asInput) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (DSoundAudioIODevice* const d = dynamic_cast<DSoundAudioIODevice*> (device))
            return asInput ? d->inputDeviceIndex
                           : d->outputDeviceIndex;

        return -1;
    }

    b8 hasSeparateInputsAndOutputs() const override   { return true; }

    AudioIODevice* createDevice (const Txt& outputDeviceName,
                                 const Txt& inputDeviceName) override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        i32k outputIndex = deviceList.outputDeviceNames.indexOf (outputDeviceName);
        i32k inputIndex = deviceList.inputDeviceNames.indexOf (inputDeviceName);

        if (outputIndex >= 0 || inputIndex >= 0)
            return new DSoundAudioIODevice (outputDeviceName.isNotEmpty() ? outputDeviceName
                                                                          : inputDeviceName,
                                            outputIndex, inputIndex);

        return nullptr;
    }

private:
    DeviceChangeDetector detector { L"DirectSound", [this] { systemDeviceChanged(); } };
    DSoundDeviceList deviceList;
    b8 hasScanned = false;

    z0 systemDeviceChanged()
    {
        DSoundDeviceList newList;
        newList.scan();

        if (std::exchange (deviceList, newList) != newList)
            callDeviceChangeListeners();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSoundAudioIODeviceType)
};

} // namespace drx
