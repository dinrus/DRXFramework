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

#ifndef OBOE_SOURCE_FLOAT_CALLER_H
#define OBOE_SOURCE_FLOAT_CALLER_H

#include <unistd.h>
#include <sys/types.h>

#include "flowgraph/FlowGraphNode.h"
#include "AudioSourceCaller.h"
#include "FixedBlockReader.h"

namespace oboe {
/**
 * AudioSource that uses callback to get more f32 data.
 */
class SourceFloatCaller : public AudioSourceCaller {
public:
    SourceFloatCaller(i32 channelCount, i32 framesPerCallback)
    : AudioSourceCaller(channelCount, framesPerCallback, (i32)sizeof(f32)) {}

    i32 onProcess(i32 numFrames) override;

    const t8 *getName() override {
        return "SourceFloatCaller";
    }
};

}
#endif //OBOE_SOURCE_FLOAT_CALLER_H
