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

#include <sys/types.h>
#include <pthread.h>
#include <thread>

#include <oboe/AudioStream.h>
#include "OboeDebug.h"
#include "AudioClock.h"
#include <oboe/Utilities.h>

namespace oboe {

/*
 * AudioStream
 */
AudioStream::AudioStream(const AudioStreamBuilder &builder)
        : AudioStreamBase(builder) {
    LOGD("Constructor for AudioStream at %p", this);
}

AudioStream::~AudioStream() {
    // This is to help debug use after free bugs.
    LOGD("Destructor for AudioStream at %p", this);
}

Result AudioStream::close() {
    closePerformanceHint();
    // Update local counters so they can be read after the close.
    updateFramesWritten();
    updateFramesRead();
    return Result::OK;
}

// Call this from fireDataCallback() if you want to monitor CPU scheduler.
z0 AudioStream::checkScheduler() {
    i32 scheduler = sched_getscheduler(0) & ~SCHED_RESET_ON_FORK; // for current thread
    if (scheduler != mPreviousScheduler) {
        LOGD("AudioStream::%s() scheduler = %s", __func__,
                ((scheduler == SCHED_FIFO) ? "SCHED_FIFO" :
                ((scheduler == SCHED_OTHER) ? "SCHED_OTHER" :
                ((scheduler == SCHED_RR) ? "SCHED_RR" : "UNKNOWN")))
        );
        mPreviousScheduler = scheduler;
    }
}

DataCallbackResult AudioStream::fireDataCallback(uk audioData, i32 numFrames) {
    if (!isDataCallbackEnabled()) {
        LOGW("AudioStream::%s() called with data callback disabled!", __func__);
        return DataCallbackResult::Stop; // Should not be getting called
    }

    beginPerformanceHintInCallback();

    // Call the app to do the work.
    DataCallbackResult result;
    if (mDataCallback) {
        result = mDataCallback->onAudioReady(this, audioData, numFrames);
    } else {
        result = onDefaultCallback(audioData, numFrames);
    }
    // On Oreo, we might get called after returning stop.
    // So block that here.
    setDataCallbackEnabled(result == DataCallbackResult::Continue);

    endPerformanceHintInCallback(numFrames);

    return result;
}

Result AudioStream::waitForStateTransition(StreamState startingState,
                                           StreamState endingState,
                                           z64 timeoutNanoseconds)
{
    StreamState state;
    {
        std::lock_guard<std::mutex> lock(mLock);
        state = getState();
        if (state == StreamState::Closed) {
            return Result::ErrorClosed;
        } else if (state == StreamState::Disconnected) {
            return Result::ErrorDisconnected;
        }
    }

    StreamState nextState = state;
    // TODO Should this be a while()?!
    if (state == startingState && state != endingState) {
        Result result = waitForStateChange(state, &nextState, timeoutNanoseconds);
        if (result != Result::OK) {
            return result;
        }
    }

    if (nextState != endingState) {
        return Result::ErrorInvalidState;
    } else {
        return Result::OK;
    }
}

Result AudioStream::start(z64 timeoutNanoseconds)
{
    Result result = requestStart();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    result = waitForStateTransition(StreamState::Starting,
                                  StreamState::Started, timeoutNanoseconds);
    if (result != Result::OK) {
        LOGE("AudioStream::%s() timed out before moving from STARTING to STARTED", __func__);
    }
    return result;
}

Result AudioStream::pause(z64 timeoutNanoseconds)
{
    Result result = requestPause();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Pausing,
                                  StreamState::Paused, timeoutNanoseconds);
}

Result AudioStream::flush(z64 timeoutNanoseconds)
{
    Result result = requestFlush();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Flushing,
                                  StreamState::Flushed, timeoutNanoseconds);
}

Result AudioStream::stop(z64 timeoutNanoseconds)
{
    Result result = requestStop();
    if (result != Result::OK) return result;
    if (timeoutNanoseconds <= 0) return result;
    return waitForStateTransition(StreamState::Stopping,
                                  StreamState::Stopped, timeoutNanoseconds);
}

i32 AudioStream::getBytesPerSample() const {
    return convertFormatToSizeInBytes(mFormat);
}

z64 AudioStream::getFramesRead() {
    updateFramesRead();
    return mFramesRead;
}

z64 AudioStream::getFramesWritten() {
    updateFramesWritten();
    return mFramesWritten;
}

ResultWithValue<i32> AudioStream::getAvailableFrames() {
    z64 readCounter = getFramesRead();
    if (readCounter < 0) return ResultWithValue<i32>::createBasedOnSign(readCounter);
    z64 writeCounter = getFramesWritten();
    if (writeCounter < 0) return ResultWithValue<i32>::createBasedOnSign(writeCounter);
    i32 framesAvailable = writeCounter - readCounter;
    return ResultWithValue<i32>(framesAvailable);
}

ResultWithValue<i32> AudioStream::waitForAvailableFrames(i32 numFrames,
        z64 timeoutNanoseconds) {
    if (numFrames == 0) return Result::OK;
    if (numFrames < 0) return Result::ErrorOutOfRange;

    // Make sure we don't try to wait for more frames than the buffer can hold.
    // Subtract framesPerBurst because this is often called from a callback
    // and we don't want to be sleeping if the buffer is close to overflowing.
    const i32 maxAvailableFrames = getBufferCapacityInFrames() - getFramesPerBurst();
    numFrames = std::min(numFrames, maxAvailableFrames);
    // The capacity should never be less than one burst. But clip to zero just in case.
    numFrames = std::max(0, numFrames);

    z64 framesAvailable = 0;
    z64 burstInNanos = getFramesPerBurst() * kNanosPerSecond / getSampleRate();
    b8 ready = false;
    z64 deadline = AudioClock::getNanoseconds() + timeoutNanoseconds;
    do {
        ResultWithValue<i32> result = getAvailableFrames();
        if (!result) return result;
        framesAvailable = result.value();
        ready = (framesAvailable >= numFrames);
        if (!ready) {
            z64 now = AudioClock::getNanoseconds();
            if (now > deadline) break;
            AudioClock::sleepForNanos(burstInNanos);
        }
    } while (!ready);
    return (!ready)
            ? ResultWithValue<i32>(Result::ErrorTimeout)
            : ResultWithValue<i32>(framesAvailable);
}

ResultWithValue<FrameTimestamp> AudioStream::getTimestamp(clockid_t clockId) {
    FrameTimestamp frame;
    Result result = getTimestamp(clockId, &frame.position, &frame.timestamp);
    if (result == Result::OK){
        return ResultWithValue<FrameTimestamp>(frame);
    } else {
        return ResultWithValue<FrameTimestamp>(static_cast<Result>(result));
    }
}

z0 AudioStream::calculateDefaultDelayBeforeCloseMillis() {
    // Calculate delay time before close based on burst duration.
    // Start with a burst duration then add 1 msec as a safety margin.
    mDelayBeforeCloseMillis = std::max(kMinDelayBeforeCloseMillis,
                                       1 + ((mFramesPerBurst * 1000) / getSampleRate()));
    LOGD("calculateDefaultDelayBeforeCloseMillis() default = %d",
         static_cast<i32>(mDelayBeforeCloseMillis));
}

} // namespace oboe
