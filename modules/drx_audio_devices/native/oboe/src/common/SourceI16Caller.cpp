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
#include "SourceI16Caller.h"

#if FLOWGRAPH_ANDROID_INTERNAL
#include <audio_utils/primitives.h>
#endif

using namespace oboe;
using namespace flowgraph;

i32 SourceI16Caller::onProcess(i32 numFrames) {
    i32 numBytes = mStream->getBytesPerFrame() * numFrames;
    i32 bytesRead = mBlockReader.read((u8 *) mConversionBuffer.get(), numBytes);
    i32 framesRead = bytesRead / mStream->getBytesPerFrame();

    f32 *floatData = output.getBuffer();
    i16k *shortData = mConversionBuffer.get();
    i32 numSamples = framesRead * output.getSamplesPerFrame();

#if FLOWGRAPH_ANDROID_INTERNAL
    memcpy_to_float_from_i16(floatData, shortData, numSamples);
#else
    for (i32 i = 0; i < numSamples; i++) {
        *floatData++ = *shortData++ * (1.0f / 32768);
    }
#endif

    return framesRead;
}
