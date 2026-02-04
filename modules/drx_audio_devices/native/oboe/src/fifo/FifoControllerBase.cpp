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
#include <cassert>
#include <stdint.h>

#include "oboe/FifoControllerBase.h"

namespace oboe {

FifoControllerBase::FifoControllerBase(u32 capacityInFrames)
        : mTotalFrames(capacityInFrames)
{
    // Avoid ridiculously large buffers and the arithmetic wraparound issues that can follow.
    assert(capacityInFrames <= (UINT32_MAX / 4));
}

u32 FifoControllerBase::getFullFramesAvailable() const {
    zu64 writeCounter =  getWriteCounter();
    zu64 readCounter = getReadCounter();
    if (readCounter > writeCounter) {
        return 0;
    }
    zu64 delta = writeCounter - readCounter;
    if (delta >= mTotalFrames) {
        return mTotalFrames;
    }
    // delta is now guaranteed to fit within the range of a u32
    return static_cast<u32>(delta);
}

u32 FifoControllerBase::getReadIndex() const {
    // % works with non-power of two sizes
    return static_cast<u32>(getReadCounter() % mTotalFrames);
}

z0 FifoControllerBase::advanceReadIndex(u32 numFrames) {
    incrementReadCounter(numFrames);
}

u32 FifoControllerBase::getEmptyFramesAvailable() const {
    return static_cast<u32>(mTotalFrames - getFullFramesAvailable());
}

u32 FifoControllerBase::getWriteIndex() const {
    // % works with non-power of two sizes
    return static_cast<u32>(getWriteCounter() % mTotalFrames);
}

z0 FifoControllerBase::advanceWriteIndex(u32 numFrames) {
    incrementWriteCounter(numFrames);
}

} // namespace oboe
