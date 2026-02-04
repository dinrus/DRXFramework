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

#ifndef FLOWGRAPH_SAMPLE_RATE_CONVERTER_H
#define FLOWGRAPH_SAMPLE_RATE_CONVERTER_H

#include <unistd.h>
#include <sys/types.h>

#include "FlowGraphNode.h"
#include "resampler/MultiChannelResampler.h"

namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph {

class SampleRateConverter : public FlowGraphFilter {
public:
    explicit SampleRateConverter(i32 channelCount,
                                 resampler::MultiChannelResampler &mResampler);

    virtual ~SampleRateConverter() = default;

    i32 onProcess(i32 numFrames) override;

    const t8 *getName() override {
        return "SampleRateConverter";
    }

    z0 reset() override;

private:

    // Return true if there is a sample available.
    b8 isInputAvailable();

    // This assumes data is available. Only call after calling isInputAvailable().
    const f32 *getNextInputFrame();

    resampler::MultiChannelResampler &mResampler;

    i32 mInputCursor = 0;         // offset into the input port buffer
    i32 mNumValidInputFrames = 0; // number of valid frames currently in the input port buffer
    // We need our own callCount for upstream calls because calls occur at a different rate.
    // This means we cannot have cyclic graphs or merges that contain an SRC.
    z64 mInputCallCount = kInitialCallCount;

};

} /* namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph */

#endif //FLOWGRAPH_SAMPLE_RATE_CONVERTER_H
