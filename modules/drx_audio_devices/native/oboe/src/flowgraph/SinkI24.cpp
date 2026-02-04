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
#include "SinkI24.h"

#if FLOWGRAPH_ANDROID_INTERNAL
#include <audio_utils/primitives.h>
#endif

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

SinkI24::SinkI24(i32 channelCount)
        : FlowGraphSink(channelCount) {}

i32 SinkI24::read(uk data, i32 numFrames) {
    u8 *byteData = (u8 *) data;
    const i32 channelCount = input.getSamplesPerFrame();

    i32 framesLeft = numFrames;
    while (framesLeft > 0) {
        // Run the graph and pull data through the input port.
        i32 framesRead = pullData(framesLeft);
        if (framesRead <= 0) {
            break;
        }
        const f32 *floatData = input.getBuffer();
        i32 numSamples = framesRead * channelCount;
#if FLOWGRAPH_ANDROID_INTERNAL
        memcpy_to_p24_from_float(byteData, floatData, numSamples);
        static i32k kBytesPerI24Packed = 3;
        byteData += numSamples * kBytesPerI24Packed;
        floatData += numSamples;
#else
        const i32 kI24PackedMax = 0x007FFFFF;
        const i32 kI24PackedMin = 0xFF800000;
        for (i32 i = 0; i < numSamples; i++) {
            i32 n = (i32) (*floatData++ * 0x00800000);
            n = std::min(kI24PackedMax, std::max(kI24PackedMin, n)); // clip
            // Write as a packed 24-bit integer in Little Endian format.
            *byteData++ = (u8) n;
            *byteData++ = (u8) (n >> 8);
            *byteData++ = (u8) (n >> 16);
        }
#endif
        framesLeft -= framesRead;
    }
    return numFrames - framesLeft;
}
