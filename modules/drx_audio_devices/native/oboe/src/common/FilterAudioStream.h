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

#ifndef OBOE_FILTER_AUDIO_STREAM_H
#define OBOE_FILTER_AUDIO_STREAM_H

#include <memory>
#include <oboe/AudioStream.h>
#include "DataConversionFlowGraph.h"

namespace oboe {

/**
 * An AudioStream that wraps another AudioStream and provides audio data conversion.
 * Operations may include channel conversion, data format conversion and/or sample rate conversion.
 */
class FilterAudioStream : public AudioStream, AudioStreamCallback {
public:

    /**
     * Construct an `AudioStream` using the given `AudioStreamBuilder` and a child AudioStream.
     *
     * This should only be called internally by AudioStreamBuilder.
     * Ownership of childStream will be passed to this object.
     *
     * @param builder containing all the stream's attributes
     */
    FilterAudioStream(const AudioStreamBuilder &builder, std::shared_ptr<AudioStream> childStream)
    : AudioStream(builder)
     , mChildStream(childStream) {
        // Intercept the callback if used.
        if (builder.isErrorCallbackSpecified()) {
            mErrorCallback = mChildStream->swapErrorCallback(this);
        }
        if (builder.isDataCallbackSpecified()) {
            mDataCallback = mChildStream->swapDataCallback(this);
        } else {
            i32k size = childStream->getFramesPerBurst() * childStream->getBytesPerFrame();
            mBlockingBuffer = std::make_unique<u8[]>(size);
        }

        // Copy parameters that may not match builder.
        mBufferCapacityInFrames = mChildStream->getBufferCapacityInFrames();
        mPerformanceMode = mChildStream->getPerformanceMode();
        mSharingMode = mChildStream->getSharingMode();
        mInputPreset = mChildStream->getInputPreset();
        mFramesPerBurst = mChildStream->getFramesPerBurst();
        mDeviceId = mChildStream->getDeviceId();
        mHardwareSampleRate = mChildStream->getHardwareSampleRate();
        mHardwareChannelCount = mChildStream->getHardwareChannelCount();
        mHardwareFormat = mChildStream->getHardwareFormat();
    }

    virtual ~FilterAudioStream() = default;

    Result configureFlowGraph();

    // Close child and parent.
    Result close()  override {
        const Result result1 = mChildStream->close();
        const Result result2 = AudioStream::close();
        return (result1 != Result::OK ? result1 : result2);
    }

    /**
     * Start the stream asynchronously. Returns immediately (does not block). Equivalent to calling
     * `start(0)`.
     */
    Result requestStart() override {
        return mChildStream->requestStart();
    }

    /**
     * Pause the stream asynchronously. Returns immediately (does not block). Equivalent to calling
     * `pause(0)`.
     */
    Result requestPause() override {
        return mChildStream->requestPause();
    }

    /**
     * Flush the stream asynchronously. Returns immediately (does not block). Equivalent to calling
     * `flush(0)`.
     */
    Result requestFlush() override {
        return mChildStream->requestFlush();
    }

    /**
     * Stop the stream asynchronously. Returns immediately (does not block). Equivalent to calling
     * `stop(0)`.
     */
    Result requestStop() override {
        return mChildStream->requestStop();
    }

    ResultWithValue<i32> read(uk buffer,
            i32 numFrames,
            z64 timeoutNanoseconds) override;

    ResultWithValue<i32> write(ukk buffer,
            i32 numFrames,
            z64 timeoutNanoseconds) override;

    StreamState getState() override {
        return mChildStream->getState();
    }

    Result waitForStateChange(
            StreamState inputState,
            StreamState *nextState,
            z64 timeoutNanoseconds) override {
        return mChildStream->waitForStateChange(inputState, nextState, timeoutNanoseconds);
    }

    b8 isXRunCountSupported() const override {
        return mChildStream->isXRunCountSupported();
    }

    AudioApi getAudioApi() const override {
        return mChildStream->getAudioApi();
    }

    z0 updateFramesWritten() override {
        // TODO for output, just count local writes?
        mFramesWritten = static_cast<z64>(mChildStream->getFramesWritten() * mRateScaler);
    }

    z0 updateFramesRead() override {
        // TODO for input, just count local reads?
        mFramesRead = static_cast<z64>(mChildStream->getFramesRead() * mRateScaler);
    }

    uk getUnderlyingStream() const  override {
        return mChildStream->getUnderlyingStream();
    }

    ResultWithValue<i32> setBufferSizeInFrames(i32 requestedFrames) override {
        return mChildStream->setBufferSizeInFrames(requestedFrames);
    }

    i32 getBufferSizeInFrames() override {
        mBufferSizeInFrames = mChildStream->getBufferSizeInFrames();
        return mBufferSizeInFrames;
    }

    ResultWithValue<i32> getXRunCount() override {
        return mChildStream->getXRunCount();
    }

    ResultWithValue<f64> calculateLatencyMillis() override {
        // This will automatically include the latency of the flowgraph?
        return mChildStream->calculateLatencyMillis();
    }

    Result getTimestamp(clockid_t clockId,
            z64 *framePosition,
            z64 *timeNanoseconds) override {
        z64 childPosition = 0;
        Result result = mChildStream->getTimestamp(clockId, &childPosition, timeNanoseconds);
        // It is OK if framePosition is null.
        if (framePosition) {
            *framePosition = childPosition * mRateScaler;
        }
        return result;
    }

    DataCallbackResult onAudioReady(AudioStream *oboeStream,
            uk audioData,
            i32 numFrames) override;

    b8 onError(AudioStream * /*audioStream*/, Result error) override {
        if (mErrorCallback != nullptr) {
            return mErrorCallback->onError(this, error);
        }
        return false;
    }

    z0 onErrorBeforeClose(AudioStream * /*oboeStream*/, Result error) override {
        if (mErrorCallback != nullptr) {
            mErrorCallback->onErrorBeforeClose(this, error);
        }
    }

    z0 onErrorAfterClose(AudioStream * /*oboeStream*/, Result error) override {
        // Close this parent stream because the callback will only close the child.
        AudioStream::close();
        if (mErrorCallback != nullptr) {
            mErrorCallback->onErrorAfterClose(this, error);
        }
    }

    /**
     * @return last result passed from an error callback
     */
    oboe::Result getLastErrorCallbackResult() const override {
        return mChildStream->getLastErrorCallbackResult();
    }

private:

    std::shared_ptr<AudioStream>             mChildStream; // this stream wraps the child stream
    std::unique_ptr<DataConversionFlowGraph> mFlowGraph; // for converting data
    std::unique_ptr<u8[]>               mBlockingBuffer; // temp buffer for write()
    f64                                   mRateScaler = 1.0; // ratio parent/child sample rates
};

} // oboe

#endif //OBOE_FILTER_AUDIO_STREAM_H
