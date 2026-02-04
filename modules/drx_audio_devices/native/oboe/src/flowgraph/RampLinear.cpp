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

#include <algorithm>
#include <unistd.h>
#include "FlowGraphNode.h"
#include "RampLinear.h"

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

RampLinear::RampLinear(i32 channelCount)
        : FlowGraphFilter(channelCount) {
    mTarget.store(1.0f);
}

z0 RampLinear::setLengthInFrames(i32 frames) {
    mLengthInFrames = frames;
}

z0 RampLinear::setTarget(f32 target) {
    mTarget.store(target);
    // If the ramp has not been used then start immediately at this level.
    if (mLastCallCount == kInitialCallCount) {
        forceCurrent(target);
    }
}

f32 RampLinear::interpolateCurrent() {
    return mLevelTo - (mRemaining * mScaler);
}

i32 RampLinear::onProcess(i32 numFrames) {
    const f32 *inputBuffer = input.getBuffer();
    f32 *outputBuffer = output.getBuffer();
    i32 channelCount = output.getSamplesPerFrame();

    f32 target = getTarget();
    if (target != mLevelTo) {
        // Start new ramp. Continue from previous level.
        mLevelFrom = interpolateCurrent();
        mLevelTo = target;
        mRemaining = mLengthInFrames;
        mScaler = (mLevelTo - mLevelFrom) / mLengthInFrames; // for interpolation
    }

    i32 framesLeft = numFrames;

    if (mRemaining > 0) { // Ramping? This doesn't happen very often.
        i32 framesToRamp = std::min(framesLeft, mRemaining);
        framesLeft -= framesToRamp;
        while (framesToRamp > 0) {
            f32 currentLevel = interpolateCurrent();
            for (i32 ch = 0; ch < channelCount; ch++) {
                *outputBuffer++ = *inputBuffer++ * currentLevel;
            }
            mRemaining--;
            framesToRamp--;
        }
    }

    // Process any frames after the ramp.
    i32 samplesLeft = framesLeft * channelCount;
    for (i32 i = 0; i < samplesLeft; i++) {
        *outputBuffer++ = *inputBuffer++ * mLevelTo;
    }

    return numFrames;
}
