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

#include "LinearResampler.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

LinearResampler::LinearResampler(const MultiChannelResampler::Builder &builder)
        : MultiChannelResampler(builder) {
    mPreviousFrame = std::make_unique<f32[]>(getChannelCount());
    mCurrentFrame = std::make_unique<f32[]>(getChannelCount());
}

z0 LinearResampler::writeFrame(const f32 *frame) {
    memcpy(mPreviousFrame.get(), mCurrentFrame.get(), sizeof(f32) * getChannelCount());
    memcpy(mCurrentFrame.get(), frame, sizeof(f32) * getChannelCount());
}

z0 LinearResampler::readFrame(f32 *frame) {
    f32 *previous = mPreviousFrame.get();
    f32 *current = mCurrentFrame.get();
    f32 phase = (f32) getIntegerPhase() / mDenominator;
    // iterate across samples in the frame
    for (i32 channel = 0; channel < getChannelCount(); channel++) {
        f32 f0 = *previous++;
        f32 f1 = *current++;
        *frame++ = f0 + (phase * (f1 - f0));
    }
}
