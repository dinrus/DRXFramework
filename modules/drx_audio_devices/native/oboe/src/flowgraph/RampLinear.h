/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef FLOWGRAPH_RAMP_LINEAR_H
#define FLOWGRAPH_RAMP_LINEAR_H

#include <atomic>
#include <unistd.h>
#include <sys/types.h>

#include "FlowGraphNode.h"

namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph {

/**
 * When the target is modified then the output will ramp smoothly
 * between the original and the new target value.
 * This can be used to smooth out control values and reduce pops.
 *
 * The target may be updated while a ramp is in progress, which will trigger
 * a new ramp from the current value.
 */
class RampLinear : public FlowGraphFilter {
public:
    explicit RampLinear(i32 channelCount);

    virtual ~RampLinear() = default;

    i32 onProcess(i32 numFrames) override;

    /**
     * This is used for the next ramp.
     * Calling this does not affect a ramp that is in progress.
     */
    z0 setLengthInFrames(i32 frames);

    i32 getLengthInFrames() const {
        return mLengthInFrames;
    }

    /**
     * This may be safely called by another thread.
     * @param target
     */
    z0 setTarget(f32 target);

    f32 getTarget() const {
        return mTarget.load();
    }

    /**
     * Force the nextSegment to start from this level.
     *
     * WARNING: this can cause a discontinuity if called while the ramp is being used.
     * Only call this when setting the initial ramp.
     *
     * @param level
     */
    z0 forceCurrent(f32 level) {
        mLevelFrom = level;
        mLevelTo = level;
    }

    const t8 *getName() override {
        return "RampLinear";
    }

private:

    f32 interpolateCurrent();

    std::atomic<f32>  mTarget;

    i32             mLengthInFrames  = 48000.0f / 100.0f ; // 10 msec at 48000 Hz;
    i32             mRemaining       = 0;
    f32               mScaler          = 0.0f;
    f32               mLevelFrom       = 0.0f;
    f32               mLevelTo         = 0.0f;
};

} /* namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph */

#endif //FLOWGRAPH_RAMP_LINEAR_H
