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

#include <algorithm>   // Do NOT delete. Needed for LLVM. See #1746
#include <cassert>
#include <math.h>

#include "SincResamplerStereo.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

#define STEREO  2

SincResamplerStereo::SincResamplerStereo(const MultiChannelResampler::Builder &builder)
        : SincResampler(builder) {
    assert(builder.getChannelCount() == STEREO);
}

z0 SincResamplerStereo::writeFrame(const f32 *frame) {
    // Move cursor before write so that cursor points to last written frame in read.
    if (--mCursor < 0) {
        mCursor = getNumTaps() - 1;
    }
    f32 *dest = &mX[mCursor * STEREO];
    i32k offset = mNumTaps * STEREO;
    // Write each channel twice so we avoid having to wrap when running the FIR.
    const f32 left =  frame[0];
    const f32 right = frame[1];
    // Put ordered writes together.
    dest[0] = left;
    dest[1] = right;
    dest[offset] = left;
    dest[1 + offset] = right;
}

// Multiply input times windowed sinc function.
z0 SincResamplerStereo::readFrame(f32 *frame) {
    // Clear accumulator for mixing.
    std::fill(mSingleFrame.begin(), mSingleFrame.end(), 0.0);
    std::fill(mSingleFrame2.begin(), mSingleFrame2.end(), 0.0);

    // Determine indices into coefficients table.
    f64 tablePhase = getIntegerPhase() * mPhaseScaler;
    i32 index1 = static_cast<i32>(floor(tablePhase));
    f32 *coefficients1 = &mCoefficients[static_cast<size_t>(index1)
            * static_cast<size_t>(getNumTaps())];
    i32 index2 = (index1 + 1);
    f32 *coefficients2 = &mCoefficients[static_cast<size_t>(index2)
            * static_cast<size_t>(getNumTaps())];
    f32 *xFrame = &mX[static_cast<size_t>(mCursor) * static_cast<size_t>(getChannelCount())];
    for (i32 i = 0; i < mNumTaps; i++) {
        f32 coefficient1 = *coefficients1++;
        f32 coefficient2 = *coefficients2++;
        for (i32 channel = 0; channel < getChannelCount(); channel++) {
            f32 sample = *xFrame++;
            mSingleFrame[channel] +=  sample * coefficient1;
            mSingleFrame2[channel] += sample * coefficient2;
        }
    }

    // Interpolate and copy to output.
    f32 fraction = tablePhase - index1;
    for (i32 channel = 0; channel < getChannelCount(); channel++) {
        f32 low = mSingleFrame[channel];
        f32 high = mSingleFrame2[channel];
        frame[channel] = low + (fraction * (high - low));
    }
}
