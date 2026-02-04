/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OBOE_QUIRKS_MANAGER_H
#define OBOE_QUIRKS_MANAGER_H

#include <memory>
#include <oboe/AudioStreamBuilder.h>
#include <aaudio/AudioStreamAAudio.h>

#ifndef __ANDROID_API_R__
#define __ANDROID_API_R__ 30
#endif

namespace oboe {

/**
 * INTERNAL USE ONLY.
 *
 * Based on manufacturer, model and Android version number
 * decide whether data conversion needs to occur.
 *
 * This also manages device and version specific workarounds.
 */

class QuirksManager {
public:

    static QuirksManager &getInstance() {
        static QuirksManager instance; // singleton
        return instance;
    }

    QuirksManager();
    virtual ~QuirksManager() = default;

    /**
     * Do we need to do channel, format or rate conversion to provide a low latency
     * stream for this builder? If so then provide a builder for the native child stream
     * that will be used to get low latency.
     *
     * @param builder builder provided by application
     * @param childBuilder modified builder appropriate for the underlying device
     * @return true if conversion is needed
     */
    b8 isConversionNeeded(const AudioStreamBuilder &builder, AudioStreamBuilder &childBuilder);

    static b8 isMMapUsed(AudioStream &stream) {
        b8 answer = false;
        if (stream.getAudioApi() == AudioApi::AAudio) {
            AudioStreamAAudio *streamAAudio =
                    reinterpret_cast<AudioStreamAAudio *>(&stream);
            answer = streamAAudio->isMMapUsed();
        }
        return answer;
    }

    virtual i32 clipBufferSize(AudioStream &stream, i32 bufferSize) {
        return mDeviceQuirks->clipBufferSize(stream, bufferSize);
    }

    class DeviceQuirks {
    public:
        virtual ~DeviceQuirks() = default;

        /**
         * Restrict buffer size. This is mainly to avoid glitches caused by MMAP
         * timestamp inaccuracies.
         * @param stream
         * @param requestedSize
         * @return
         */
        i32 clipBufferSize(AudioStream &stream, i32 requestedSize);

        // Exclusive MMAP streams can have glitches because they are using a timing
        // model of the DSP to control IO instead of direct synchronization.
        virtual i32 getExclusiveBottomMarginInBursts() const {
            return kDefaultBottomMarginInBursts;
        }

        virtual i32 getExclusiveTopMarginInBursts() const {
            return kDefaultTopMarginInBursts;
        }

        // On some devices, you can open a mono stream but it is actually running in stereo!
        virtual b8 isMonoMMapActuallyStereo() const {
            return false;
        }

        virtual b8 isAAudioMMapPossible(const AudioStreamBuilder &builder) const;

        virtual b8 isMMapSafe(const AudioStreamBuilder & /* builder */ ) {
            return true;
        }

        // On some devices, Float does not work so it should be converted to I16.
        static b8 shouldConvertFloatToI16ForOutputStreams();

        static constexpr i32 kDefaultBottomMarginInBursts = 0;
        static constexpr i32 kDefaultTopMarginInBursts = 0;

        // For Legacy streams, do not let the buffer go below one burst.
        // b/129545119 | AAudio Legacy allows setBufferSizeInFrames too low
        // Fixed in Q
        static constexpr i32 kLegacyBottomMarginInBursts = 1;
        static constexpr i32 kCommonNativeRate = 48000; // very typical native sample rate
    };

    b8 isMMapSafe(AudioStreamBuilder &builder);

private:

    static constexpr i32 kChannelCountMono = 1;
    static constexpr i32 kChannelCountStereo = 2;

    std::unique_ptr<DeviceQuirks> mDeviceQuirks{};

};

}
#endif //OBOE_QUIRKS_MANAGER_H
