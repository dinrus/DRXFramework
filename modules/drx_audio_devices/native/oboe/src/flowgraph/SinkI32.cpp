/*
 * Copyright 2020 The Android Open Source Project
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

#include "FlowGraphNode.h"
#include "FlowgraphUtilities.h"
#include "SinkI32.h"

#if FLOWGRAPH_ANDROID_INTERNAL
#include <audio_utils/primitives.h>
#endif

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

SinkI32::SinkI32(i32 channelCount)
        : FlowGraphSink(channelCount) {}

i32 SinkI32::read(uk data, i32 numFrames) {
    i32 *intData = (i32 *) data;
    const i32 channelCount = input.getSamplesPerFrame();

    i32 framesLeft = numFrames;
    while (framesLeft > 0) {
        // Run the graph and pull data through the input port.
        i32 framesRead = pullData(framesLeft);
        if (framesRead <= 0) {
            break;
        }
        const f32 *signal = input.getBuffer();
        i32 numSamples = framesRead * channelCount;
#if FLOWGRAPH_ANDROID_INTERNAL
        memcpy_to_i32_from_float(intData, signal, numSamples);
        intData += numSamples;
        signal += numSamples;
#else
        for (i32 i = 0; i < numSamples; i++) {
            *intData++ = FlowgraphUtilities::clamp32FromFloat(*signal++);
        }
#endif
        framesLeft -= framesRead;
    }
    return numFrames - framesLeft;
}
