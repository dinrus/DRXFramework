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

#include <algorithm>
#include <memory.h>
#include <stdint.h>

#include "oboe/FifoControllerBase.h"
#include "fifo/FifoController.h"
#include "fifo/FifoControllerIndirect.h"
#include "oboe/FifoBuffer.h"

namespace oboe {

FifoBuffer::FifoBuffer(u32 bytesPerFrame, u32 capacityInFrames)
        : mBytesPerFrame(bytesPerFrame)
        , mStorage(nullptr)
        , mFramesReadCount(0)
        , mFramesUnderrunCount(0)
{
    mFifo = std::make_unique<FifoController>(capacityInFrames);
    // allocate buffer
    i32 bytesPerBuffer = bytesPerFrame * capacityInFrames;
    mStorage = new u8[bytesPerBuffer];
    mStorageOwned = true;
}

FifoBuffer::FifoBuffer( u32  bytesPerFrame,
                        u32  capacityInFrames,
                        std::atomic<zu64>  *readCounterAddress,
                        std::atomic<zu64>  *writeCounterAddress,
                        u8  *dataStorageAddress
                        )
        : mBytesPerFrame(bytesPerFrame)
        , mStorage(dataStorageAddress)
        , mFramesReadCount(0)
        , mFramesUnderrunCount(0)
{
    mFifo = std::make_unique<FifoControllerIndirect>(capacityInFrames,
                                       readCounterAddress,
                                       writeCounterAddress);
    mStorage = dataStorageAddress;
    mStorageOwned = false;
}

FifoBuffer::~FifoBuffer() {
    if (mStorageOwned) {
        delete[] mStorage;
    }
}

i32 FifoBuffer::convertFramesToBytes(i32 frames) {
    return frames * mBytesPerFrame;
}

i32 FifoBuffer::read(uk buffer, i32 numFrames) {
    if (numFrames <= 0) {
        return 0;
    }
    // safe because numFrames is guaranteed positive
    u32 framesToRead = static_cast<u32>(numFrames);
    u32 framesAvailable = mFifo->getFullFramesAvailable();
    framesToRead = std::min(framesToRead, framesAvailable);

    u32 readIndex = mFifo->getReadIndex(); // ranges 0 to capacity
    u8 *destination = reinterpret_cast<u8 *>(buffer);
    u8 *source = &mStorage[convertFramesToBytes(readIndex)];
    if ((readIndex + framesToRead) > mFifo->getFrameCapacity()) {
        // read in two parts, first part here is at the end of the mStorage buffer
        i32 frames1 = static_cast<i32>(mFifo->getFrameCapacity() - readIndex);
        i32 numBytes = convertFramesToBytes(frames1);
        if (numBytes < 0) {
            return static_cast<i32>(Result::ErrorOutOfRange);
        }
        memcpy(destination, source, static_cast<size_t>(numBytes));
        destination += numBytes;
        // read second part, which is at the beginning of mStorage
        source = &mStorage[0];
        i32 frames2 = static_cast<u32>(framesToRead - frames1);
        numBytes = convertFramesToBytes(frames2);
        if (numBytes < 0) {
            return static_cast<i32>(Result::ErrorOutOfRange);
        }
        memcpy(destination, source, static_cast<size_t>(numBytes));
    } else {
        // just read in one shot
        i32 numBytes = convertFramesToBytes(framesToRead);
        if (numBytes < 0) {
            return static_cast<i32>(Result::ErrorOutOfRange);
        }
        memcpy(destination, source, static_cast<size_t>(numBytes));
    }
    mFifo->advanceReadIndex(framesToRead);

    return framesToRead;
}

i32 FifoBuffer::write(ukk buffer, i32 numFrames) {
    if (numFrames <= 0) {
        return 0;
    }
    // Guaranteed positive.
    u32 framesToWrite = static_cast<u32>(numFrames);
    u32 framesAvailable = mFifo->getEmptyFramesAvailable();
    framesToWrite = std::min(framesToWrite, framesAvailable);

    u32 writeIndex = mFifo->getWriteIndex();
    i32 byteIndex = convertFramesToBytes(writeIndex);
    u8k *source = reinterpret_cast<u8k *>(buffer);
    u8 *destination = &mStorage[byteIndex];
    if ((writeIndex + framesToWrite) > mFifo->getFrameCapacity()) {
        // write in two parts, first part here
        i32 frames1 = static_cast<u32>(mFifo->getFrameCapacity() - writeIndex);
        i32 numBytes = convertFramesToBytes(frames1);
        if (numBytes < 0) {
            return static_cast<i32>(Result::ErrorOutOfRange);
        }
        memcpy(destination, source, static_cast<size_t>(numBytes));
        // read second part
        source += convertFramesToBytes(frames1);
        destination = &mStorage[0];
        i32 frames2 = static_cast<u32>(framesToWrite - frames1);
        numBytes = convertFramesToBytes(frames2);
        if (numBytes < 0) {
            return static_cast<i32>(Result::ErrorOutOfRange);
        }
        memcpy(destination, source, static_cast<size_t>(numBytes));
    } else {
        // just write in one shot
        i32 numBytes = convertFramesToBytes(framesToWrite);
        if (numBytes < 0) {
            return static_cast<i32>(Result::ErrorOutOfRange);
        }
        memcpy(destination, source, static_cast<size_t>(numBytes));
    }
    mFifo->advanceWriteIndex(framesToWrite);

    return framesToWrite;
}

i32 FifoBuffer::readNow(uk buffer, i32 numFrames) {
    i32 framesRead = read(buffer, numFrames);
    if (framesRead < 0) {
        return framesRead;
    }
    i32 framesLeft = numFrames - framesRead;
    mFramesReadCount += framesRead;
    mFramesUnderrunCount += framesLeft;
    // Zero out any samples we could not set.
    if (framesLeft > 0) {
        u8 *destination = reinterpret_cast<u8 *>(buffer);
        destination += convertFramesToBytes(framesRead); // point to first byte not set
        i32 bytesToZero = convertFramesToBytes(framesLeft);
        memset(destination, 0, static_cast<size_t>(bytesToZero));
    }

    return framesRead;
}


u32 FifoBuffer::getBufferCapacityInFrames() const {
    return mFifo->getFrameCapacity();
}

} // namespace oboe
