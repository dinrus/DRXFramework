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
#include "IntegerRatio.h"
#include "PolyphaseResampler.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

PolyphaseResampler::PolyphaseResampler(const MultiChannelResampler::Builder &builder)
        : MultiChannelResampler(builder)
        {
    assert((getNumTaps() % 4) == 0); // Required for loop unrolling.

    i32 inputRate = builder.getInputRate();
    i32 outputRate = builder.getOutputRate();

    i32 numRows = mDenominator;
    f64 phaseIncrement = (f64) inputRate / (f64) outputRate;
    generateCoefficients(inputRate, outputRate,
                         numRows, phaseIncrement,
                         builder.getNormalizedCutoff());
}

z0 PolyphaseResampler::readFrame(f32 *frame) {
    // Clear accumulator for mixing.
    std::fill(mSingleFrame.begin(), mSingleFrame.end(), 0.0);

    // Multiply input times windowed sinc function.
    f32 *coefficients = &mCoefficients[mCoefficientCursor];
    f32 *xFrame = &mX[static_cast<size_t>(mCursor) * static_cast<size_t>(getChannelCount())];
    for (i32 i = 0; i < mNumTaps; i++) {
        f32 coefficient = *coefficients++;
        for (i32 channel = 0; channel < getChannelCount(); channel++) {
            mSingleFrame[channel] += *xFrame++ * coefficient;
        }
    }

    // Advance and wrap through coefficients.
    mCoefficientCursor = (mCoefficientCursor + mNumTaps) % mCoefficients.size();

    // Copy accumulator to output.
    for (i32 channel = 0; channel < getChannelCount(); channel++) {
        frame[channel] = mSingleFrame[channel];
    }
}
