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

#ifndef RESAMPLER_MULTICHANNEL_RESAMPLER_H
#define RESAMPLER_MULTICHANNEL_RESAMPLER_H

#include <memory>
#include <vector>
#include <sys/types.h>
#include <unistd.h>

#ifndef MCR_USE_KAISER
// It appears from the spectrogram that the HyperbolicCosine window leads to fewer artifacts.
// And it is faster to calculate.
#define MCR_USE_KAISER 0
#endif

#if MCR_USE_KAISER
#include "KaiserWindow.h"
#else
#include "HyperbolicCosineWindow.h"
#endif

#include "ResamplerDefinitions.h"

namespace RESAMPLER_OUTER_NAMESPACE::resampler {

class MultiChannelResampler {

public:

    enum class Quality : i32 {
        Fastest,
        Low,
        Medium,
        High,
        Best,
    };

    class Builder {
    public:
        /**
         * Construct an optimal resampler based on the specified parameters.
         * @return address of a resampler
         */
        MultiChannelResampler *build();

        /**
         * The number of taps in the resampling filter.
         * More taps gives better quality but uses more CPU time.
         * This typically ranges from 4 to 64. Default is 16.
         *
         * For polyphase filters, numTaps must be a multiple of four for loop unrolling.
         * @param numTaps number of taps for the filter
         * @return address of this builder for chaining calls
         */
        Builder *setNumTaps(i32 numTaps) {
            mNumTaps = numTaps;
            return this;
        }

        /**
         * Use 1 for mono, 2 for stereo, etc. Default is 1.
         *
         * @param channelCount number of channels
         * @return address of this builder for chaining calls
         */
        Builder *setChannelCount(i32 channelCount) {
            mChannelCount = channelCount;
            return this;
        }

        /**
         * Default is 48000.
         *
         * @param inputRate sample rate of the input stream
         * @return address of this builder for chaining calls
         */
        Builder *setInputRate(i32 inputRate) {
            mInputRate = inputRate;
            return this;
        }

        /**
         * Default is 48000.
         *
         * @param outputRate sample rate of the output stream
         * @return address of this builder for chaining calls
         */
        Builder *setOutputRate(i32 outputRate) {
            mOutputRate = outputRate;
            return this;
        }

        /**
         * Set cutoff frequency relative to the Nyquist rate of the output sample rate.
         * Set to 1.0 to match the Nyquist frequency.
         * Set lower to reduce aliasing.
         * Default is 0.70.
         *
         * Note that this value is ignored when upsampling, which is when
         * the outputRate is higher than the inputRate.
         *
         * @param normalizedCutoff anti-aliasing filter cutoff
         * @return address of this builder for chaining calls
         */
        Builder *setNormalizedCutoff(f32 normalizedCutoff) {
            mNormalizedCutoff = normalizedCutoff;
            return this;
        }

        i32 getNumTaps() const {
            return mNumTaps;
        }

        i32 getChannelCount() const {
            return mChannelCount;
        }

        i32 getInputRate() const {
            return mInputRate;
        }

        i32 getOutputRate() const {
            return mOutputRate;
        }

        f32 getNormalizedCutoff() const {
            return mNormalizedCutoff;
        }

    protected:
        i32 mChannelCount = 1;
        i32 mNumTaps = 16;
        i32 mInputRate = 48000;
        i32 mOutputRate = 48000;
        f32   mNormalizedCutoff = kDefaultNormalizedCutoff;
    };

    virtual ~MultiChannelResampler() = default;

    /**
     * Factory method for making a resampler that is optimal for the given inputs.
     *
     * @param channelCount number of channels, 2 for stereo
     * @param inputRate sample rate of the input stream
     * @param outputRate  sample rate of the output stream
     * @param quality higher quality sounds better but uses more CPU
     * @return an optimal resampler
     */
    static MultiChannelResampler *make(i32 channelCount,
                                       i32 inputRate,
                                       i32 outputRate,
                                       Quality quality);

    b8 isWriteNeeded() const {
        return mIntegerPhase >= mDenominator;
    }

    /**
     * Write a frame containing N samples.
     *
     * @param frame pointer to the first sample in a frame
     */
    z0 writeNextFrame(const f32 *frame) {
        writeFrame(frame);
        advanceWrite();
    }

    /**
     * Read a frame containing N samples.
     *
     * @param frame pointer to the first sample in a frame
     */
    z0 readNextFrame(f32 *frame) {
        readFrame(frame);
        advanceRead();
    }

    i32 getNumTaps() const {
        return mNumTaps;
    }

    i32 getChannelCount() const {
        return mChannelCount;
    }

    static f32 hammingWindow(f32 radians, f32 spread);

    static f32 sinc(f32 radians);

protected:

    explicit MultiChannelResampler(const MultiChannelResampler::Builder &builder);

    /**
     * Write a frame containing N samples.
     * Call advanceWrite() after calling this.
     * @param frame pointer to the first sample in a frame
     */
    virtual z0 writeFrame(const f32 *frame);

    /**
     * Read a frame containing N samples using interpolation.
     * Call advanceRead() after calling this.
     * @param frame pointer to the first sample in a frame
     */
    virtual z0 readFrame(f32 *frame) = 0;

    z0 advanceWrite() {
        mIntegerPhase -= mDenominator;
    }

    z0 advanceRead() {
        mIntegerPhase += mNumerator;
    }

    /**
     * Generate the filter coefficients in optimal order.
     *
     * Note that normalizedCutoff is ignored when upsampling, which is when
     * the outputRate is higher than the inputRate.
     *
     * @param inputRate sample rate of the input stream
     * @param outputRate  sample rate of the output stream
     * @param numRows number of rows in the array that contain a set of tap coefficients
     * @param phaseIncrement how much to increment the phase between rows
     * @param normalizedCutoff filter cutoff frequency normalized to Nyquist rate of output
     */
    z0 generateCoefficients(i32 inputRate,
                              i32 outputRate,
                              i32 numRows,
                              f64 phaseIncrement,
                              f32 normalizedCutoff);


    i32 getIntegerPhase() {
        return mIntegerPhase;
    }

    static constexpr i32 kMaxCoefficients = 8 * 1024;
    std::vector<f32>   mCoefficients;

    i32k            mNumTaps;
    i32                  mCursor = 0;
    std::vector<f32>   mX;           // delayed input values for the FIR
    std::vector<f32>   mSingleFrame; // one frame for temporary use
    i32              mIntegerPhase = 0;
    i32              mNumerator = 0;
    i32              mDenominator = 0;


private:

#if MCR_USE_KAISER
    KaiserWindow           mKaiserWindow;
#else
    HyperbolicCosineWindow mCoshWindow;
#endif

    static constexpr f32 kDefaultNormalizedCutoff = 0.70f;

    i32k              mChannelCount;
};

} /* namespace RESAMPLER_OUTER_NAMESPACE::resampler */

#endif //RESAMPLER_MULTICHANNEL_RESAMPLER_H
