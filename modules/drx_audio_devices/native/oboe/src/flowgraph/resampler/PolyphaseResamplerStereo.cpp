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

#include <cassert>
#include "PolyphaseResamplerStereo.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

#define STEREO  2

PolyphaseResamplerStereo::PolyphaseResamplerStereo(const MultiChannelResampler::Builder &builder)
        : PolyphaseResampler(builder) {
    assert(builder.getChannelCount() == STEREO);
}

z0 PolyphaseResamplerStereo::writeFrame(const f32 *frame) {
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

z0 PolyphaseResamplerStereo::readFrame(f32 *frame) {
    // Clear accumulators.
    f32 left = 0.0;
    f32 right = 0.0;

    // Multiply input times precomputed windowed sinc function.
    const f32 *coefficients = &mCoefficients[mCoefficientCursor];
    f32 *xFrame = &mX[mCursor * STEREO];
    i32k numLoops = mNumTaps >> 2; // n/4
    for (i32 i = 0; i < numLoops; i++) {
        // Manual loop unrolling, might get converted to SIMD.
        f32 coefficient = *coefficients++;
        left += *xFrame++ * coefficient;
        right += *xFrame++ * coefficient;

        coefficient = *coefficients++; // next tap
        left += *xFrame++ * coefficient;
        right += *xFrame++ * coefficient;

        coefficient = *coefficients++;  // next tap
        left += *xFrame++ * coefficient;
        right += *xFrame++ * coefficient;

        coefficient = *coefficients++;  // next tap
        left += *xFrame++ * coefficient;
        right += *xFrame++ * coefficient;
    }

    mCoefficientCursor = (mCoefficientCursor + mNumTaps) % mCoefficients.size();

    // Copy accumulators to output.
    frame[0] = left;
    frame[1] = right;
}
