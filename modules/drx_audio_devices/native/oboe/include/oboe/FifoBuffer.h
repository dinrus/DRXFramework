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

#ifndef OBOE_FIFOPROCESSOR_H
#define OBOE_FIFOPROCESSOR_H

#include <memory>
#include <stdint.h>

#include "oboe/Definitions.h"

#include "oboe/FifoControllerBase.h"

namespace oboe {

class FifoBuffer {
public:
	/**
	 * Construct a `FifoBuffer`.
	 *
	 * @param bytesPerFrame amount of bytes for one frame
	 * @param capacityInFrames the capacity of frames in fifo
	 */
    FifoBuffer(u32 bytesPerFrame, u32 capacityInFrames);

	/**
	 * Construct a `FifoBuffer`.
	 * To be used if the storage allocation is done outside of FifoBuffer.
	 *
	 * @param bytesPerFrame amount of bytes for one frame
	 * @param capacityInFrames capacity of frames in fifo
	 * @param readCounterAddress address of read counter
	 * @param writeCounterAddress address of write counter
	 * @param dataStorageAddress address of storage
	 */
    FifoBuffer(u32   bytesPerFrame,
               u32   capacityInFrames,
               std::atomic<zu64>   *readCounterAddress,
               std::atomic<zu64>   *writeCounterAddress,
               u8   *dataStorageAddress);

    ~FifoBuffer();

	/**
	 * Convert a number of frames in bytes.
	 *
	 * @return number of bytes
	 */
    i32 convertFramesToBytes(i32 frames);

    /**
     * Read framesToRead or, if not enough, then read as many as are available.
     *
     * @param destination
     * @param framesToRead number of frames requested
     * @return number of frames actually read
     */
    i32 read(uk destination, i32 framesToRead);

	/**
	 * Write framesToWrite or, if too enough, then write as many as the fifo are not empty.
	 *
	 * @param destination
	 * @param framesToWrite number of frames requested
	 * @return number of frames actually write
	 */
    i32 write(ukk source, i32 framesToWrite);

	/**
	 * Get the buffer capacity in frames.
	 *
	 * @return number of frames
	 */
    u32 getBufferCapacityInFrames() const;

    /**
     * Calls read(). If all of the frames cannot be read then the remainder of the buffer
     * is set to zero.
     *
     * @param destination
     * @param framesToRead number of frames requested
     * @return number of frames actually read
     */
    i32 readNow(uk destination, i32 numFrames);

	/**
	 * Get the number of frames in the fifo.
	 *
	 * @return number of frames actually in the buffer
	 */
    u32 getFullFramesAvailable() {
        return mFifo->getFullFramesAvailable();
    }

	/**
	 * Get the amount of bytes per frame.
	 *
	 * @return number of bytes per frame
	 */
    u32 getBytesPerFrame() const {
        return mBytesPerFrame;
    }

	/**
	 * Get the position of read counter.
	 *
	 * @return position of read counter
	 */
    zu64 getReadCounter() const {
        return mFifo->getReadCounter();
    }

	/**
	 * Set the position of read counter.
	 *
	 * @param n position of read counter
	 */
    z0 setReadCounter(zu64 n) {
        mFifo->setReadCounter(n);
    }

	/**
	 * Get the position of write counter.
	 *
	 * @return position of write counter
	 */
    zu64 getWriteCounter() {
        return mFifo->getWriteCounter();
    }

    /**
	 * Set the position of write counter.
	 *
	 * @param n position of write counter
	 */
    z0 setWriteCounter(zu64 n) {
        mFifo->setWriteCounter(n);
    }

private:
    u32 mBytesPerFrame;
    u8* mStorage;
    b8     mStorageOwned; // did this object allocate the storage?
    std::unique_ptr<FifoControllerBase> mFifo;
    zu64 mFramesReadCount;
    zu64 mFramesUnderrunCount;
};

} // namespace oboe

#endif //OBOE_FIFOPROCESSOR_H
