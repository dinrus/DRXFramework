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

#ifndef OBOE_SOURCE_I32_CALLER_H
#define OBOE_SOURCE_I32_CALLER_H

#include <memory.h>
#include <unistd.h>
#include <sys/types.h>

#include "flowgraph/FlowGraphNode.h"
#include "AudioSourceCaller.h"
#include "FixedBlockReader.h"

namespace oboe {

/**
 * AudioSource that uses callback to get more data.
 */
class SourceI32Caller : public AudioSourceCaller {
public:
    SourceI32Caller(i32 channelCount, i32 framesPerCallback)
    : AudioSourceCaller(channelCount, framesPerCallback, sizeof(i32)) {
        mConversionBuffer = std::make_unique<i32[]>(static_cast<size_t>(channelCount)
                * static_cast<size_t>(output.getFramesPerBuffer()));
    }

    i32 onProcess(i32 numFrames) override;

    const t8 *getName() override {
        return "SourceI32Caller";
    }

private:
    std::unique_ptr<i32[]>  mConversionBuffer;
    static constexpr f32 kScale = 1.0 / (1UL << 31);
};

}
#endif //OBOE_SOURCE_I32_CALLER_H
