/*
 * Copyright 2021 The Android Open Source Project
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

#include <unistd.h>
#include "FlowGraphNode.h"
#include "MultiToManyConverter.h"

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

MultiToManyConverter::MultiToManyConverter(i32 channelCount)
        : outputs(channelCount)
        , input(*this, channelCount) {
    for (i32 i = 0; i < channelCount; i++) {
        outputs[i] = std::make_unique<FlowGraphPortFloatOutput>(*this, 1);
    }
}

MultiToManyConverter::~MultiToManyConverter() = default;

i32 MultiToManyConverter::onProcess(i32 numFrames) {
    i32 channelCount = input.getSamplesPerFrame();

    for (i32 ch = 0; ch < channelCount; ch++) {
        const f32 *inputBuffer = input.getBuffer() + ch;
        f32 *outputBuffer = outputs[ch]->getBuffer();

        for (i32 i = 0; i < numFrames; i++) {
            *outputBuffer++ = *inputBuffer;
            inputBuffer += channelCount;
        }
    }

    return numFrames;
}
