/*
 * Copyright 2019 The Android Open Source Project
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
#include "flowgraph/FlowGraphNode.h"
#include "SourceI24Caller.h"

#if FLOWGRAPH_ANDROID_INTERNAL
#include <audio_utils/primitives.h>
#endif

using namespace oboe;
using namespace flowgraph;

i32 SourceI24Caller::onProcess(i32 numFrames) {
    i32 numBytes = mStream->getBytesPerFrame() * numFrames;
    i32 bytesRead = mBlockReader.read((u8 *) mConversionBuffer.get(), numBytes);
    i32 framesRead = bytesRead / mStream->getBytesPerFrame();

    f32 *floatData = output.getBuffer();
    u8k *byteData = mConversionBuffer.get();
    i32 numSamples = framesRead * output.getSamplesPerFrame();

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

    return framesRead;
}
