/*
 * Copyright 2018 The Android Open Source Project
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
#include <unistd.h>

#include "FlowGraphNode.h"
#include "SourceI24.h"

#if FLOWGRAPH_ANDROID_INTERNAL
#include <audio_utils/primitives.h>
#endif

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

constexpr i32 kBytesPerI24Packed = 3;

SourceI24::SourceI24(i32 channelCount)
        : FlowGraphSourceBuffered(channelCount) {
}

i32 SourceI24::onProcess(i32 numFrames) {
    f32 *floatData = output.getBuffer();
    i32 channelCount = output.getSamplesPerFrame();

    i32 framesLeft = mSizeInFrames - mFrameIndex;
    i32 framesToProcess = std::min(numFrames, framesLeft);
    i32 numSamples = framesToProcess * channelCount;

    u8k *byteBase = (u8 *) mData;
    u8k *byteData = &byteBase[mFrameIndex * channelCount * kBytesPerI24Packed];

#if FLOWGRAPH_ANDROID_INTERNAL
    memcpy_to_float_from_p24(floatData, byteData, numSamples);
#else
    static const f32 scale = 1. / (f32)(1UL << 31);
    for (i32 i = 0; i < numSamples; i++) {
        // Assemble the data assuming Little Endian format.
        i32 pad = byteData[2];
        pad <<= 8;
        pad |= byteData[1];
        pad <<= 8;
        pad |= byteData[0];
        pad <<= 8; // Shift to 32 bit data so the sign is correct.
        byteData += kBytesPerI24Packed;
        *floatData++ = pad * scale; // scale to range -1.0 to 1.0
    }
#endif

    mFrameIndex += framesToProcess;
    return framesToProcess;
}
