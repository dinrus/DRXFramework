/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef OBOE_AUDIO_STREAM_OPENSL_ES_H_
#define OBOE_AUDIO_STREAM_OPENSL_ES_H_

#include <memory>

#include "oboe/Oboe.h"
#include "common/MonotonicCounter.h"
#include "opensles/AudioStreamBuffered.h"
#include "opensles/EngineOpenSLES.h"

namespace oboe {

constexpr i32 kBitsPerByte = 8;
constexpr i32 kBufferQueueLengthDefault = 2; // f64 buffered for callbacks
constexpr i32 kBufferQueueLengthMax = 8; // AudioFlinger won't use more than 8

/**
 * INTERNAL USE ONLY
 *
 * A stream that wraps OpenSL ES.
 *
 * Do not instantiate this class directly.
 * Use an OboeStreamBuilder to create one.
 */

class AudioStreamOpenSLES : public AudioStreamBuffered {
public:

    AudioStreamOpenSLES();
    explicit AudioStreamOpenSLES(const AudioStreamBuilder &builder);

    virtual ~AudioStreamOpenSLES() = default;

    virtual Result open() override;

    /**
     * Query the current state, eg. OBOE_STREAM_STATE_PAUSING
     *
     * @return state or a negative error.
     */
    StreamState getState() override { return mState.load(); }

    AudioApi getAudioApi() const override {
        return AudioApi::OpenSLES;
    }

    /**
     * Process next OpenSL ES buffer.
     * Called by by OpenSL ES framework.
     *
     * This is public, but don't call it directly.
     *
     * @return whether the current stream should be stopped.
     */
    b8 processBufferCallback(SLAndroidSimpleBufferQueueItf bq);

    Result waitForStateChange(StreamState currentState,
                              StreamState *nextState,
                              z64 timeoutNanoseconds) override;

protected:

    /**
     * Finish setting up the stream. Common for INPUT and OUTPUT.
     *
     * @param configItf
     * @return SL_RESULT_SUCCESS if OK.
     */
    SLresult finishCommonOpen(SLAndroidConfigurationItf configItf);

    // This must be called under mLock.
    Result close_l();

    SLuint32 channelCountToChannelMaskDefault(i32 channelCount) const;

    virtual Result onBeforeDestroy() { return Result::OK; }
    virtual Result onAfterDestroy() { return Result::OK; }

    static SLuint32 getDefaultByteOrder();

    i32 getBufferDepth(SLAndroidSimpleBufferQueueItf bq);

    i32 calculateOptimalBufferQueueLength();
    i32 estimateNativeFramesPerBurst();

    SLresult enqueueCallbackBuffer(SLAndroidSimpleBufferQueueItf bq);

    SLresult configurePerformanceMode(SLAndroidConfigurationItf configItf);

    PerformanceMode convertPerformanceMode(SLuint32 openslMode) const;
    SLuint32 convertPerformanceMode(PerformanceMode oboeMode) const;

    z0 logUnsupportedAttributes();

    /**
     * Internal use only.
     * Use this instead of directly setting the internal state variable.
     */
    z0 setState(StreamState state) {
        mState.store(state);
    }

    z64 getFramesProcessedByServer();

    // OpenSLES stuff
    SLObjectItf                   mObjectInterface = nullptr;
    SLAndroidSimpleBufferQueueItf mSimpleBufferQueueInterface = nullptr;
    i32                           mBufferQueueLength = 0;

    i32                       mBytesPerCallback = oboe::kUnspecified;
    MonotonicCounter              mPositionMillis; // for tracking OpenSL ES service position

private:

    constexpr static i32 kDoubleBufferCount = 2;

    SLresult registerBufferQueueCallback();
    SLresult updateStreamParameters(SLAndroidConfigurationItf configItf);
    Result configureBufferSizes(i32 sampleRate);

    std::unique_ptr<u8[]>    mCallbackBuffer[kBufferQueueLengthMax];
    i32                           mCallbackBufferIndex = 0;
    std::atomic<StreamState>      mState{StreamState::Uninitialized};

};

} // namespace oboe

#endif // OBOE_AUDIO_STREAM_OPENSL_ES_H_
