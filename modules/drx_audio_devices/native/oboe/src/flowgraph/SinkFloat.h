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


#ifndef FLOWGRAPH_SINK_FLOAT_H
#define FLOWGRAPH_SINK_FLOAT_H

#include <unistd.h>
#include <sys/types.h>

#include "FlowGraphNode.h"

namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph {

/**
 * AudioSink that lets you read data as 32-bit floats.
 */
class SinkFloat : public FlowGraphSink {
public:
    explicit SinkFloat(i32 channelCount);
    ~SinkFloat() override = default;

    i32 read(uk data, i32 numFrames) override;

    const t8 *getName() override {
        return "SinkFloat";
    }
};

} /* namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph */

#endif //FLOWGRAPH_SINK_FLOAT_H
