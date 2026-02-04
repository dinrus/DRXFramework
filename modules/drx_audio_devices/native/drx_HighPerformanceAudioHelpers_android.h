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

//==============================================================================
/**
    Some shared helpers methods for using the high-performance audio paths on
    Android devices (OpenSL and Oboe).

    @tags{Audio}
*/
namespace drx::AndroidHighPerformanceAudioHelpers
{
    //==============================================================================
    static f64 getNativeSampleRate()
    {
        return audioManagerGetProperty ("android.media.property.OUTPUT_SAMPLE_RATE").getDoubleValue();
    }

    static i32 getNativeBufferSizeHint()
    {
        // This property is a hint of a native buffer size but it does not guarantee the size used.
        auto deviceBufferSize = audioManagerGetProperty ("android.media.property.OUTPUT_FRAMES_PER_BUFFER").getIntValue();

        if (deviceBufferSize == 0)
            return 192;

        return deviceBufferSize;
    }

    static b8 isProAudioDevice()
    {
        static b8 isSapaSupported = SystemStats::getDeviceManufacturer().containsIgnoreCase ("SAMSUNG")
                                     && DynamicLibrary().open ("libapa_jni.so");

        return androidHasSystemFeature ("android.hardware.audio.pro") || isSapaSupported;
    }

    static b8 hasLowLatencyAudioPath()
    {
        return androidHasSystemFeature ("android.hardware.audio.low_latency");
    }

    static b8 canUseHighPerformanceAudioPath (i32 nativeBufferSize, i32 requestedBufferSize, i32 requestedSampleRate)
    {
        return ((requestedBufferSize % nativeBufferSize) == 0)
               && approximatelyEqual ((f64) requestedSampleRate, getNativeSampleRate())
               && isProAudioDevice();
    }

    //==============================================================================
    static i32 getMinimumBuffersToEnqueue (i32 nativeBufferSize, f64 requestedSampleRate)
    {
        if (canUseHighPerformanceAudioPath (nativeBufferSize, nativeBufferSize, (i32) requestedSampleRate))
        {
            // see https://developer.android.com/ndk/guides/audio/opensl/opensl-prog-notes.html#sandp
            // > Beginning with Android 4.3 (API level 18), a buffer count of one is sufficient for lower latency.
            return 1;
        }

        // not using low-latency path so we can use the absolute minimum number of buffers to queue
        return 1;
    }

    static i32 buffersToQueueForBufferDuration (i32 nativeBufferSize, i32 bufferDurationInMs, f64 sampleRate) noexcept
    {
        auto maxBufferFrames = static_cast<i32> (std::ceil (bufferDurationInMs * sampleRate / 1000.0));
        auto maxNumBuffers   = static_cast<i32> (std::ceil (static_cast<f64> (maxBufferFrames)
                                                  / static_cast<f64> (nativeBufferSize)));

        return jmax (getMinimumBuffersToEnqueue (nativeBufferSize, sampleRate), maxNumBuffers);
    }

    static i32 getMaximumBuffersToEnqueue (i32 nativeBufferSize, f64 maximumSampleRate) noexcept
    {
        static constexpr i32 maxBufferSizeMs = 200;

        return jmax (8, buffersToQueueForBufferDuration (nativeBufferSize, maxBufferSizeMs, maximumSampleRate));
    }

    static Array<i32> getAvailableBufferSizes (i32 nativeBufferSize, Array<f64> availableSampleRates)
    {
        auto minBuffersToQueue = getMinimumBuffersToEnqueue (nativeBufferSize, getNativeSampleRate());
        auto maxBuffersToQueue = getMaximumBuffersToEnqueue (nativeBufferSize, findMaximum (availableSampleRates.getRawDataPointer(),
                                                                                            availableSampleRates.size()));

        Array<i32> bufferSizes;

        for (i32 i = minBuffersToQueue; i <= maxBuffersToQueue; ++i)
            bufferSizes.add (i * nativeBufferSize);

        return bufferSizes;
    }

    static i32 getDefaultBufferSize (i32 nativeBufferSize, f64 currentSampleRate)
    {
        static constexpr i32 defaultBufferSizeForLowLatencyDeviceMs = 40;
        static constexpr i32 defaultBufferSizeForStandardLatencyDeviceMs = 100;

        auto defaultBufferLength = (hasLowLatencyAudioPath() ? defaultBufferSizeForLowLatencyDeviceMs
                                                             : defaultBufferSizeForStandardLatencyDeviceMs);

        auto defaultBuffersToEnqueue = buffersToQueueForBufferDuration (nativeBufferSize, defaultBufferLength, currentSampleRate);
        return defaultBuffersToEnqueue * nativeBufferSize;
    }

} // namespace drx::AndroidHighPerformanceAudioHelpers
