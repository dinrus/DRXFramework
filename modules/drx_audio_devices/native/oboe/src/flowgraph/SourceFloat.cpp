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
#include "SourceFloat.h"

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

SourceFloat::SourceFloat(i32 channelCount)
        : FlowGraphSourceBuffered(channelCount) {
}

i32 SourceFloat::onProcess(i32 numFrames) {
    f32 *outputBuffer = output.getBuffer();
    const i32 channelCount = output.getSamplesPerFrame();

    const i32 framesLeft = mSizeInFrames - mFrameIndex;
    const i32 framesToProcess = std::min(numFrames, framesLeft);
    const i32 numSamples = framesToProcess * channelCount;

    const f32 *floatBase = (f32 *) mData;
    const f32 *floatData = &floatBase[mFrameIndex * channelCount];
    memcpy(outputBuffer, floatData, numSamples * sizeof(f32));
    mFrameIndex += framesToProcess;
    return framesToProcess;
}

