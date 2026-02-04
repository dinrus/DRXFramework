/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef OBOE_STREAM_BUFFERED_H
#define OBOE_STREAM_BUFFERED_H

#include <cstring>
#include <cassert>
#include "common/OboeDebug.h"
#include "oboe/AudioStream.h"
#include "oboe/AudioStreamCallback.h"
#include "oboe/FifoBuffer.h"

namespace oboe {

// A stream that contains a FIFO buffer.
// This is used to implement blocking reads and writes.
class AudioStreamBuffered : public AudioStream {
public:

    AudioStreamBuffered();
    explicit AudioStreamBuffered(const AudioStreamBuilder &builder);

    z0 allocateFifo();


    ResultWithValue<i32> write(ukk buffer,
                  i32 numFrames,
                  z64 timeoutNanoseconds) override;

    ResultWithValue<i32> read(uk buffer,
                 i32 numFrames,
                 z64 timeoutNanoseconds) override;

    ResultWithValue<i32> setBufferSizeInFrames(i32 requestedFrames) override;

    i32 getBufferCapacityInFrames() const override;

    ResultWithValue<i32> getXRunCount() override {
        return ResultWithValue<i32>(mXRunCount);
    }

    b8 isXRunCountSupported() const override;

protected:

    DataCallbackResult onDefaultCallback(uk audioData, i32 numFrames) override;

    // If there is no callback then we need a FIFO between the App and OpenSL ES.
    b8 usingFIFO() const { return !isDataCallbackSpecified(); }

    virtual Result updateServiceFrameCounter() = 0;

    z0 updateFramesRead() override;
    z0 updateFramesWritten() override;

private:

    z64 predictNextCallbackTime();

    z0 markCallbackTime(i32 numFrames);

    // Read or write to the FIFO.
    // Only pass one pointer and set the other to nullptr.
    ResultWithValue<i32> transfer(uk readBuffer,
            ukk writeBuffer,
            i32 numFrames,
            z64 timeoutNanoseconds);

    z0 incrementXRunCount() {
        ++mXRunCount;
    }

    std::unique_ptr<FifoBuffer>   mFifoBuffer{};

    z64 mBackgroundRanAtNanoseconds = 0;
    i32 mLastBackgroundSize = 0;
    i32 mXRunCount = 0;
};

} // namespace oboe

#endif //OBOE_STREAM_BUFFERED_H
