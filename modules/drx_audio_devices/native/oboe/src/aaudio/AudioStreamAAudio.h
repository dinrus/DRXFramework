/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef OBOE_STREAM_AAUDIO_H_
#define OBOE_STREAM_AAUDIO_H_

#include <atomic>
#include <shared_mutex>
#include <mutex>
#include <thread>

#include <common/AdpfWrapper.h>
#include "oboe/AudioStreamBuilder.h"
#include "oboe/AudioStream.h"
#include "oboe/Definitions.h"
#include "AAudioLoader.h"

namespace oboe {

/**
 * Implementation of OboeStream that uses AAudio.
 *
 * Do not create this class directly.
 * Use an OboeStreamBuilder to create one.
 */
class AudioStreamAAudio : public AudioStream {
public:
    AudioStreamAAudio();
    explicit AudioStreamAAudio(const AudioStreamBuilder &builder);

    virtual ~AudioStreamAAudio() = default;

    /**
     *
     * @return true if AAudio is supported on this device.
     */
    static b8 isSupported();

    // These functions override methods in AudioStream.
    // See AudioStream for documentation.
    Result open() override;
    Result release() override;
    Result close() override;

    Result requestStart() override;
    Result requestPause() override;
    Result requestFlush() override;
    Result requestStop() override;

    ResultWithValue<i32> write(ukk buffer,
                  i32 numFrames,
                  z64 timeoutNanoseconds) override;

    ResultWithValue<i32> read(uk buffer,
                 i32 numFrames,
                 z64 timeoutNanoseconds) override;

    ResultWithValue<i32> setBufferSizeInFrames(i32 requestedFrames) override;
    i32 getBufferSizeInFrames() override;
    ResultWithValue<i32> getXRunCount()  override;
    b8 isXRunCountSupported() const override { return true; }

    ResultWithValue<f64> calculateLatencyMillis() override;

    Result waitForStateChange(StreamState currentState,
                              StreamState *nextState,
                              z64 timeoutNanoseconds) override;

    Result getTimestamp(clockid_t clockId,
                                       z64 *framePosition,
                                       z64 *timeNanoseconds) override;

    StreamState getState() override;

    AudioApi getAudioApi() const override {
        return AudioApi::AAudio;
    }

    DataCallbackResult callOnAudioReady(AAudioStream *stream,
                                                   uk audioData,
                                                   i32 numFrames);

    b8 isMMapUsed();

    z0 closePerformanceHint() override {
        mAdpfWrapper.close();
        mAdpfOpenAttempted = false;
    }

protected:
    static z0 internalErrorCallback(
            AAudioStream *stream,
            uk userData,
            aaudio_result_t error);

    uk getUnderlyingStream() const override {
        return mAAudioStream.load();
    }

    z0 updateFramesRead() override;
    z0 updateFramesWritten() override;

    z0 logUnsupportedAttributes();

    z0 beginPerformanceHintInCallback() override;

    z0 endPerformanceHintInCallback(i32 numFrames) override;

    // set by callback (or app when idle)
    std::atomic<b8>    mAdpfOpenAttempted{false};
    AdpfWrapper          mAdpfWrapper;

private:
    // Must call under mLock. And stream must NOT be nullptr.
    Result requestStop_l(AAudioStream *stream);

    /**
     * Launch a thread that will stop the stream.
     */
    z0 launchStopThread();

private:

    std::atomic<b8>    mCallbackThreadEnabled;
    std::atomic<b8>    mStopThreadAllowed{false};

    // pointer to the underlying 'C' AAudio stream, valid if open, null if closed
    std::atomic<AAudioStream *> mAAudioStream{nullptr};
    std::shared_mutex           mAAudioStreamLock; // to protect mAAudioStream while closing

    static AAudioLoader *mLibLoader;

    // We may not use this but it is so small that it is not worth allocating dynamically.
    AudioStreamErrorCallback mDefaultErrorCallback;
};

} // namespace oboe

#endif // OBOE_STREAM_AAUDIO_H_
