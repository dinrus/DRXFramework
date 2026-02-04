/*
 * Copyright 2023 The Android Open Source Project
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

#ifndef OBOE_FULL_DUPLEX_STREAM_
#define OBOE_FULL_DUPLEX_STREAM_

#include <cstdint>
#include "oboe/Definitions.h"
#include "oboe/AudioStream.h"
#include "oboe/AudioStreamCallback.h"

namespace oboe {

/**
 * FullDuplexStream can be used to synchronize an input and output stream.
 *
 * For the builder of the output stream, call setDataCallback() with this object.
 *
 * When both streams are ready, onAudioReady() of the output stream will call onBothStreamsReady().
 * Callers must override onBothStreamsReady().
 *
 * To ensure best results, open an output stream before the input stream.
 * Call inputBuilder.setBufferCapacityInFrames(mOutputStream->getBufferCapacityInFrames() * 2).
 * Also, call inputBuilder.setSampleRate(mOutputStream->getSampleRate()).
 *
 * Callers must call setInputStream() and setOutputStream().
 * Call start() to start both streams and stop() to stop both streams.
 * Caller is responsible for closing both streams.
 *
 * Callers should handle error callbacks with setErrorCallback() for the output stream.
 * When an error callback occurs for the output stream, Oboe will stop and close the output stream.
 * The caller is responsible for stopping and closing the input stream.
 * The caller should also reopen and restart both streams when the error callback is ErrorDisconnected.
 * See the LiveEffect sample as an example of this. 
 *
 */
class FullDuplexStream : public AudioStreamDataCallback {
public:
    FullDuplexStream() {}
    virtual ~FullDuplexStream() = default;

    /**
     * Sets the input stream. Calling this is mandatory.
     *
     * @param stream the output stream
     */
    z0 setInputStream(AudioStream *stream) {
        mInputStream = stream;
    }

    /**
     * Gets the input stream
     *
     * @return the input stream
     */
    AudioStream *getInputStream() {
        return mInputStream;
    }

    /**
     * Sets the output stream. Calling this is mandatory.
     *
     * @param stream the output stream
     */
    z0 setOutputStream(AudioStream *stream) {
        mOutputStream = stream;
    }

    /**
     * Gets the output stream
     *
     * @return the output stream
     */
    AudioStream *getOutputStream() {
        return mOutputStream;
    }

    /**
     * Attempts to start both streams. Please call setInputStream() and setOutputStream() before
     * calling this function.
     *
     * @return result of the operation
     */
    virtual Result start() {
        mCountCallbacksToDrain = kNumCallbacksToDrain;
        mCountInputBurstsCushion = mNumInputBurstsCushion;
        mCountCallbacksToDiscard = kNumCallbacksToDiscard;

        // Determine maximum size that could possibly be called.
        i32 bufferSize = getOutputStream()->getBufferCapacityInFrames()
                             * getOutputStream()->getChannelCount();
        if (bufferSize > mBufferSize) {
            mInputBuffer = std::make_unique<f32[]>(bufferSize);
            mBufferSize = bufferSize;
        }

        oboe::Result result = getInputStream()->requestStart();
        if (result != oboe::Result::OK) {
            return result;
        }
        return getOutputStream()->requestStart();
    }

    /**
     * Stops both streams. Returns Result::OK if neither stream had an error during close.
     *
     * @return result of the operation
     */
    virtual Result stop() {
        Result outputResult = Result::OK;
        Result inputResult = Result::OK;
        if (getOutputStream()) {
            outputResult = mOutputStream->requestStop();
        }
        if (getInputStream()) {
            inputResult = mInputStream->requestStop();
        }
        if (outputResult != Result::OK) {
            return outputResult;
        } else {
            return inputResult;
        }
    }

    /**
     * Reads input from the input stream. Callers should not call this directly as this is called
     * in onAudioReady().
     *
     * @param numFrames
     * @return result of the operation
     */
    virtual ResultWithValue<i32> readInput(i32 numFrames) {
        return getInputStream()->read(mInputBuffer.get(), numFrames, 0 /* timeout */);
    }

    /**
     * Called when data is available on both streams.
     * Caller should override this method.
     * numInputFrames and numOutputFrames may be zero.
     *
     * @param inputData buffer containing input data
     * @param numInputFrames number of input frames
     * @param outputData a place to put output data
     * @param numOutputFrames number of output frames
     * @return DataCallbackResult::Continue or DataCallbackResult::Stop
     */
    virtual DataCallbackResult onBothStreamsReady(
            ukk inputData,
            i32   numInputFrames,
            uk outputData,
            i32   numOutputFrames
            ) = 0;

    /**
     * Called when the output stream is ready to process audio.
     * This in return calls onBothStreamsReady() when data is available on both streams.
     * Callers should call this function when the output stream is ready.
     * Callers must override onBothStreamsReady().
     *
     * @param audioStream pointer to the associated stream
     * @param audioData a place to put output data
     * @param numFrames number of frames to be processed
     * @return DataCallbackResult::Continue or DataCallbackResult::Stop
     *
     */
    DataCallbackResult onAudioReady(
            AudioStream * /*audioStream*/,
            uk audioData,
            i32 numFrames) {
        DataCallbackResult callbackResult = DataCallbackResult::Continue;
        i32 actualFramesRead = 0;

        // Silence the output.
        i32 numBytes = numFrames * getOutputStream()->getBytesPerFrame();
        memset(audioData, 0 /* value */, numBytes);

        if (mCountCallbacksToDrain > 0) {
            // Drain the input.
            i32 totalFramesRead = 0;
            do {
                ResultWithValue<i32> result = readInput(numFrames);
                if (!result) {
                    // Ignore errors because input stream may not be started yet.
                    break;
                }
                actualFramesRead = result.value();
                totalFramesRead += actualFramesRead;
            } while (actualFramesRead > 0);
            // Only counts if we actually got some data.
            if (totalFramesRead > 0) {
                mCountCallbacksToDrain--;
            }

        } else if (mCountInputBurstsCushion > 0) {
            // Let the input fill up a bit so we are not so close to the write pointer.
            mCountInputBurstsCushion--;

        } else if (mCountCallbacksToDiscard > 0) {
            mCountCallbacksToDiscard--;
            // Ignore. Allow the input to reach to equilibrium with the output.
            ResultWithValue<i32> resultAvailable = getInputStream()->getAvailableFrames();
            if (!resultAvailable) {
                callbackResult = DataCallbackResult::Stop;
            } else {
                i32 framesAvailable = resultAvailable.value();
                if (framesAvailable >= mMinimumFramesBeforeRead) {
                    ResultWithValue<i32> resultRead = readInput(numFrames);
                    if (!resultRead) {
                        callbackResult = DataCallbackResult::Stop;
                    }
                }
            }
        } else {
            i32 framesRead = 0;
            ResultWithValue<i32> resultAvailable = getInputStream()->getAvailableFrames();
            if (!resultAvailable) {
                callbackResult = DataCallbackResult::Stop;
            } else {
                i32 framesAvailable = resultAvailable.value();
                if (framesAvailable >= mMinimumFramesBeforeRead) {
                    // Read data into input buffer.
                    ResultWithValue<i32> resultRead = readInput(numFrames);
                    if (!resultRead) {
                        callbackResult = DataCallbackResult::Stop;
                    } else {
                        framesRead = resultRead.value();
                    }
                }
            }

            if (callbackResult == DataCallbackResult::Continue) {
                callbackResult = onBothStreamsReady(mInputBuffer.get(), framesRead,
                                                    audioData, numFrames);
            }
        }

        if (callbackResult == DataCallbackResult::Stop) {
            getInputStream()->requestStop();
        }

        return callbackResult;
    }

    /**
     *
     * This is a cushion between the DSP and the application processor cursors to prevent collisions.
     * Typically 0 for latency measurements or 1 for glitch tests.
     *
     * @param numBursts number of bursts to leave in the input buffer as a cushion
     */
    z0 setNumInputBurstsCushion(i32 numBursts) {
        mNumInputBurstsCushion = numBursts;
    }

    /**
     * Get the number of bursts left in the input buffer as a cushion.
     *
     * @return number of bursts in the input buffer as a cushion
     */
    i32 getNumInputBurstsCushion() const {
        return mNumInputBurstsCushion;
    }

    /**
     * Minimum number of frames in the input stream buffer before calling readInput().
     *
     * @param numFrames number of bursts in the input buffer as a cushion
     */
    z0 setMinimumFramesBeforeRead(i32 numFrames) {
        mMinimumFramesBeforeRead = numFrames;
    }

    /**
     * Gets the minimum number of frames in the input stream buffer before calling readInput().
     *
     * @return minimum number of frames before reading
     */
    i32 getMinimumFramesBeforeRead() const {
        return mMinimumFramesBeforeRead;
    }

private:

    // TODO add getters and setters
    static constexpr i32 kNumCallbacksToDrain   = 20;
    static constexpr i32 kNumCallbacksToDiscard = 30;

    // let input fill back up, usually 0 or 1
    i32 mNumInputBurstsCushion =  0;
    i32 mMinimumFramesBeforeRead = 0;

    // We want to reach a state where the input buffer is empty and
    // the output buffer is full.
    // These are used in order.
    // Drain several callback so that input is empty.
    i32              mCountCallbacksToDrain = kNumCallbacksToDrain;
    // Let the input fill back up slightly so we don't run dry.
    i32              mCountInputBurstsCushion = mNumInputBurstsCushion;
    // Discard some callbacks so the input and output reach equilibrium.
    i32              mCountCallbacksToDiscard = kNumCallbacksToDiscard;

    AudioStream   *mInputStream = nullptr;
    AudioStream   *mOutputStream = nullptr;

    i32              mBufferSize = 0;
    std::unique_ptr<f32[]> mInputBuffer;
};

} // namespace oboe

#endif //OBOE_FULL_DUPLEX_STREAM_
