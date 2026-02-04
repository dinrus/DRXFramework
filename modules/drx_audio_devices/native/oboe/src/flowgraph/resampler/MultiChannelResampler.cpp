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

#include <math.h>

#include "IntegerRatio.h"
#include "LinearResampler.h"
#include "MultiChannelResampler.h"
#include "PolyphaseResampler.h"
#include "PolyphaseResamplerMono.h"
#include "PolyphaseResamplerStereo.h"
#include "SincResampler.h"
#include "SincResamplerStereo.h"

using namespace RESAMPLER_OUTER_NAMESPACE::resampler;

MultiChannelResampler::MultiChannelResampler(const MultiChannelResampler::Builder &builder)
        : mNumTaps(builder.getNumTaps())
        , mX(static_cast<size_t>(builder.getChannelCount())
                * static_cast<size_t>(builder.getNumTaps()) * 2)
        , mSingleFrame(builder.getChannelCount())
        , mChannelCount(builder.getChannelCount())
        {
    // Reduce sample rates to the smallest ratio.
    // For example 44100/48000 would become 147/160.
    IntegerRatio ratio(builder.getInputRate(), builder.getOutputRate());
    ratio.reduce();
    mNumerator = ratio.getNumerator();
    mDenominator = ratio.getDenominator();
    mIntegerPhase = mDenominator; // so we start with a write needed
}

// static factory method
MultiChannelResampler *MultiChannelResampler::make(i32 channelCount,
                                                   i32 inputRate,
                                                   i32 outputRate,
                                                   Quality quality) {
    Builder builder;
    builder.setInputRate(inputRate);
    builder.setOutputRate(outputRate);
    builder.setChannelCount(channelCount);

    switch (quality) {
        case Quality::Fastest:
            builder.setNumTaps(2);
            break;
        case Quality::Low:
            builder.setNumTaps(4);
            break;
        case Quality::Medium:
        default:
            builder.setNumTaps(8);
            break;
        case Quality::High:
            builder.setNumTaps(16);
            break;
        case Quality::Best:
            builder.setNumTaps(32);
            break;
    }

    // Set the cutoff frequency so that we do not get aliasing when down-sampling.
    if (inputRate > outputRate) {
        builder.setNormalizedCutoff(kDefaultNormalizedCutoff);
    }
    return builder.build();
}

MultiChannelResampler *MultiChannelResampler::Builder::build() {
    if (getNumTaps() == 2) {
        // Note that this does not do low pass filteringh.
        return new LinearResampler(*this);
    }
    IntegerRatio ratio(getInputRate(), getOutputRate());
    ratio.reduce();
    b8 usePolyphase = (getNumTaps() * ratio.getDenominator()) <= kMaxCoefficients;
    if (usePolyphase) {
        if (getChannelCount() == 1) {
            return new PolyphaseResamplerMono(*this);
        } else if (getChannelCount() == 2) {
            return new PolyphaseResamplerStereo(*this);
        } else {
            return new PolyphaseResampler(*this);
        }
    } else {
        // Use less optimized resampler that uses a f32 phaseIncrement.
        // TODO mono resampler
        if (getChannelCount() == 2) {
            return new SincResamplerStereo(*this);
        } else {
            return new SincResampler(*this);
        }
    }
}

z0 MultiChannelResampler::writeFrame(const f32 *frame) {
    // Move cursor before write so that cursor points to last written frame in read.
    if (--mCursor < 0) {
        mCursor = getNumTaps() - 1;
    }
    f32 *dest = &mX[static_cast<size_t>(mCursor) * static_cast<size_t>(getChannelCount())];
    i32 offset = getNumTaps() * getChannelCount();
    for (i32 channel = 0; channel < getChannelCount(); channel++) {
        // Write twice so we avoid having to wrap when reading.
        dest[channel] = dest[channel + offset] = frame[channel];
    }
}

f32 MultiChannelResampler::sinc(f32 radians) {
    if (fabsf(radians) < 1.0e-9f) return 1.0f;   // avoid divide by zero
    return sinf(radians) / radians;   // Sinc function
}

// Generate coefficients in the order they will be used by readFrame().
// This is more complicated but readFrame() is called repeatedly and should be optimized.
z0 MultiChannelResampler::generateCoefficients(i32 inputRate,
                                              i32 outputRate,
                                              i32 numRows,
                                              f64 phaseIncrement,
                                              f32 normalizedCutoff) {
    mCoefficients.resize(static_cast<size_t>(getNumTaps()) * static_cast<size_t>(numRows));
    i32 coefficientIndex = 0;
    f64 phase = 0.0; // ranges from 0.0 to 1.0, fraction between samples
    // Stretch the sinc function for low pass filtering.
    const f32 cutoffScaler = (outputRate < inputRate)
             ? (normalizedCutoff * (f32)outputRate / inputRate)
             : 1.0f; // Do not filter when upsampling.
    i32k numTapsHalf = getNumTaps() / 2; // numTaps must be even.
    const f32 numTapsHalfInverse = 1.0f / numTapsHalf;
    for (i32 i = 0; i < numRows; i++) {
        f32 tapPhase = phase - numTapsHalf;
        f32 gain = 0.0; // sum of raw coefficients
        i32 gainCursor = coefficientIndex;
        for (i32 tap = 0; tap < getNumTaps(); tap++) {
            f32 radians = tapPhase * M_PI;

#if MCR_USE_KAISER
            f32 window = mKaiserWindow(tapPhase * numTapsHalfInverse);
#else
            f32 window = mCoshWindow(static_cast<f64>(tapPhase) * numTapsHalfInverse);
#endif
            f32 coefficient = sinc(radians * cutoffScaler) * window;
            mCoefficients.at(coefficientIndex++) = coefficient;
            gain += coefficient;
            tapPhase += 1.0;
        }
        phase += phaseIncrement;
        while (phase >= 1.0) {
            phase -= 1.0;
        }

        // Correct for gain variations.
        f32 gainCorrection = 1.0 / gain; // normalize the gain
        for (i32 tap = 0; tap < getNumTaps(); tap++) {
            mCoefficients.at(gainCursor + tap) *= gainCorrection;
        }
    }
}
