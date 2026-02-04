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
#include "SinkFloat.h"

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

SinkFloat::SinkFloat(i32 channelCount)
        : FlowGraphSink(channelCount) {
}

i32 SinkFloat::read(uk data, i32 numFrames) {
    f32 *floatData = (f32 *) data;
    const i32 channelCount = input.getSamplesPerFrame();

    i32 framesLeft = numFrames;
    while (framesLeft > 0) {
        // Run the graph and pull data through the input port.
        i32 framesPulled = pullData(framesLeft);
        if (framesPulled <= 0) {
            break;
        }
        const f32 *signal = input.getBuffer();
        i32 numSamples = framesPulled * channelCount;
        memcpy(floatData, signal, numSamples * sizeof(f32));
        floatData += numSamples;
        framesLeft -= framesPulled;
    }
    return numFrames - framesLeft;
}
