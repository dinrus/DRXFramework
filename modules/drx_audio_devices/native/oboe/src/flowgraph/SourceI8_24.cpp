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

#include <algorithm>
#include <unistd.h>

#include "FlowGraphNode.h"
#include "SourceI8_24.h"

#if FLOWGRAPH_ANDROID_INTERNAL
#include <audio_utils/primitives.h>
#endif

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

SourceI8_24::SourceI8_24(i32 channelCount)
        : FlowGraphSourceBuffered(channelCount) {
}

i32 SourceI8_24::onProcess(i32 numFrames) {
    f32 *floatData = output.getBuffer();
    const i32 channelCount = output.getSamplesPerFrame();

    const i32 framesLeft = mSizeInFrames - mFrameIndex;
    const i32 framesToProcess = std::min(numFrames, framesLeft);
    const i32 numSamples = framesToProcess * channelCount;

    const i32 *intBase = static_cast<const i32 *>(mData);
    const i32 *intData = &intBase[mFrameIndex * channelCount];

#if FLOWGRAPH_ANDROID_INTERNAL
    memcpy_to_float_from_q8_23(floatData, intData, numSamples);
#else
    for (i32 i = 0; i < numSamples; i++) {
        *floatData++ = *intData++ * kScale;
    }
#endif

    mFrameIndex += framesToProcess;
    return framesToProcess;
}
