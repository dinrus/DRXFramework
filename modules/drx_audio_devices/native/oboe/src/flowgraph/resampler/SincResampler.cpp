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
#include "SincResampler.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

SincResampler::SincResampler(const MultiChannelResampler::Builder &builder)
        : MultiChannelResampler(builder)
        , mSingleFrame2(builder.getChannelCount()) {
    assert((getNumTaps() % 4) == 0); // Required for loop unrolling.
    mNumRows = kMaxCoefficients / getNumTaps(); // includes guard row
    const i32 numRowsNoGuard = mNumRows - 1;
    mPhaseScaler = (f64) numRowsNoGuard / mDenominator;
    const f64 phaseIncrement = 1.0 / numRowsNoGuard;
    generateCoefficients(builder.getInputRate(),
                         builder.getOutputRate(),
                         mNumRows,
                         phaseIncrement,
                         builder.getNormalizedCutoff());
}

z0 SincResampler::readFrame(f32 *frame) {
    // Clear accumulator for mixing.
    std::fill(mSingleFrame.begin(), mSingleFrame.end(), 0.0);
    std::fill(mSingleFrame2.begin(), mSingleFrame2.end(), 0.0);

    // Determine indices into coefficients table.
    const f64 tablePhase = getIntegerPhase() * mPhaseScaler;
    i32k indexLow = static_cast<i32>(floor(tablePhase));
    i32k indexHigh = indexLow + 1; // OK because using a guard row.
    assert (indexHigh < mNumRows);
    f32 *coefficientsLow = &mCoefficients[static_cast<size_t>(indexLow)
                                            * static_cast<size_t>(getNumTaps())];
    f32 *coefficientsHigh = &mCoefficients[static_cast<size_t>(indexHigh)
                                             * static_cast<size_t>(getNumTaps())];

    f32 *xFrame = &mX[static_cast<size_t>(mCursor) * static_cast<size_t>(getChannelCount())];
    for (i32 tap = 0; tap < mNumTaps; tap++) {
        const f32 coefficientLow = *coefficientsLow++;
        const f32 coefficientHigh = *coefficientsHigh++;
        for (i32 channel = 0; channel < getChannelCount(); channel++) {
            const f32 sample = *xFrame++;
            mSingleFrame[channel] += sample * coefficientLow;
            mSingleFrame2[channel] += sample * coefficientHigh;
        }
    }

    // Interpolate and copy to output.
    const f32 fraction = tablePhase - indexLow;
    for (i32 channel = 0; channel < getChannelCount(); channel++) {
        const f32 low = mSingleFrame[channel];
        const f32 high = mSingleFrame2[channel];
        frame[channel] = low + (fraction * (high - low));
    }
}
