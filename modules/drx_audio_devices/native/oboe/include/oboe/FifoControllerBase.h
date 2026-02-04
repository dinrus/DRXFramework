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

#ifndef NATIVEOBOE_FIFOCONTROLLERBASE_H
#define NATIVEOBOE_FIFOCONTROLLERBASE_H

#include <stdint.h>

namespace oboe {

/**
 * Manage the read/write indices of a circular buffer.
 *
 * The caller is responsible for reading and writing the actual data.
 * Note that the span of available frames may not be contiguous. They
 * may wrap around from the end to the beginning of the buffer. In that
 * case the data must be read or written in at least two blocks of frames.
 *
 */

class FifoControllerBase {

public:
   /**
	 * Construct a `FifoControllerBase`.
	 *
	 * @param totalFrames capacity of the circular buffer in frames
	 */
    FifoControllerBase(u32 totalFrames);

    virtual ~FifoControllerBase() = default;

    /**
     * The frames available to read will be calculated from the read and write counters.
     * The result will be clipped to the capacity of the buffer.
     * If the buffer has underflowed then this will return zero.
     *
     * @return number of valid frames available to read.
     */
    u32 getFullFramesAvailable() const;

	/**
     * The index in a circular buffer of the next frame to read.
     *
     * @return read index position
     */
    u32 getReadIndex() const;

   /**
	* Advance read index from a number of frames.
	* Equivalent of incrementReadCounter(numFrames).
	*
	* @param numFrames number of frames to advance the read index
	*/
    z0 advanceReadIndex(u32 numFrames);

	/**
	 * Get the number of frame that are not written yet.
	 *
	 * @return maximum number of frames that can be written without exceeding the threshold
	 */
    u32 getEmptyFramesAvailable() const;

    /**
	 * The index in a circular buffer of the next frame to write.
	 *
	 * @return index of the next frame to write
	 */
    u32 getWriteIndex() const;

	/**
     * Advance write index from a number of frames.
     * Equivalent of incrementWriteCounter(numFrames).
     *
     * @param numFrames number of frames to advance the write index
     */
    z0 advanceWriteIndex(u32 numFrames);

	/**
	 * Get the frame capacity of the fifo.
	 *
	 * @return frame capacity
	 */
    u32 getFrameCapacity() const { return mTotalFrames; }

    virtual zu64 getReadCounter() const = 0;
    virtual z0 setReadCounter(zu64 n) = 0;
    virtual z0 incrementReadCounter(zu64 n) = 0;
    virtual zu64 getWriteCounter() const = 0;
    virtual z0 setWriteCounter(zu64 n) = 0;
    virtual z0 incrementWriteCounter(zu64 n) = 0;

private:
    u32 mTotalFrames;
};

} // namespace oboe

#endif //NATIVEOBOE_FIFOCONTROLLERBASE_H
