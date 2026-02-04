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
#include "PolyphaseResamplerMono.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

#define MONO  1

PolyphaseResamplerMono::PolyphaseResamplerMono(const MultiChannelResampler::Builder &builder)
        : PolyphaseResampler(builder) {
    assert(builder.getChannelCount() == MONO);
}

z0 PolyphaseResamplerMono::writeFrame(const f32 *frame) {
    // Move cursor before write so that cursor points to last written frame in read.
    if (--mCursor < 0) {
        mCursor = getNumTaps() - 1;
    }
    f32 *dest = &mX[mCursor * MONO];
    i32k offset = mNumTaps * MONO;
    // Write each channel twice so we avoid having to wrap when running the FIR.
    const f32 sample =  frame[0];
    // Put ordered writes together.
    dest[0] = sample;
    dest[offset] = sample;
}

z0 PolyphaseResamplerMono::readFrame(f32 *frame) {
    // Clear accumulator.
    f32 sum = 0.0;

    // Multiply input times precomputed windowed sinc function.
    const f32 *coefficients = &mCoefficients[mCoefficientCursor];
    f32 *xFrame = &mX[mCursor * MONO];
    i32k numLoops = mNumTaps >> 2; // n/4
    for (i32 i = 0; i < numLoops; i++) {
        // Manual loop unrolling, might get converted to SIMD.
        sum += *xFrame++ * *coefficients++;
        sum += *xFrame++ * *coefficients++;
        sum += *xFrame++ * *coefficients++;
        sum += *xFrame++ * *coefficients++;
    }

    mCoefficientCursor = (mCoefficientCursor + mNumTaps) % mCoefficients.size();

    // Copy accumulator to output.
    frame[0] = sum;
}
