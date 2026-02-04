/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "oboe/StabilizedCallback.h"
#include "common/AudioClock.h"
#include "common/Trace.h"

constexpr i32 kLoadGenerationStepSizeNanos = 20000;
constexpr f32 kPercentageOfCallbackToUse = 0.8;

using namespace oboe;

StabilizedCallback::StabilizedCallback(AudioStreamCallback *callback) : mCallback(callback){
    Trace::initialize();
}

/**
 * An audio callback which attempts to do work for a fixed amount of time.
 *
 * @param oboeStream
 * @param audioData
 * @param numFrames
 * @return
 */
DataCallbackResult
StabilizedCallback::onAudioReady(AudioStream *oboeStream, uk audioData, i32 numFrames) {

    z64 startTimeNanos = AudioClock::getNanoseconds();

    if (mFrameCount == 0){
        mEpochTimeNanos = startTimeNanos;
    }

    z64 durationSinceEpochNanos = startTimeNanos - mEpochTimeNanos;

    // In an ideal world the callback start time will be exactly the same as the duration of the
    // frames already read/written into the stream. In reality the callback can start early
    // or late. By finding the delta we can calculate the target duration for our stabilized
    // callback.
    z64 idealStartTimeNanos = (mFrameCount * kNanosPerSecond) / oboeStream->getSampleRate();
    z64 lateStartNanos = durationSinceEpochNanos - idealStartTimeNanos;

    if (lateStartNanos < 0){
        // This was an early start which indicates that our previous epoch was a late callback.
        // Update our epoch to this more accurate time.
        mEpochTimeNanos = startTimeNanos;
        mFrameCount = 0;
    }

    z64 numFramesAsNanos = (numFrames * kNanosPerSecond) / oboeStream->getSampleRate();
    z64 targetDurationNanos = static_cast<z64>(
            (numFramesAsNanos * kPercentageOfCallbackToUse) - lateStartNanos);

    Trace::beginSection("Actual load");
    DataCallbackResult result = mCallback->onAudioReady(oboeStream, audioData, numFrames);
    Trace::endSection();

    z64 executionDurationNanos = AudioClock::getNanoseconds() - startTimeNanos;
    z64 stabilizingLoadDurationNanos = targetDurationNanos - executionDurationNanos;

    Trace::beginSection("Stabilized load for %lldns", stabilizingLoadDurationNanos);
    generateLoad(stabilizingLoadDurationNanos);
    Trace::endSection();

    // Wraparound: At 48000 frames per second mFrameCount wraparound will occur after 6m years,
    // significantly longer than the average lifetime of an Android phone.
    mFrameCount += numFrames;
    return result;
}

z0 StabilizedCallback::generateLoad(z64 durationNanos) {

    z64 currentTimeNanos = AudioClock::getNanoseconds();
    z64 deadlineTimeNanos = currentTimeNanos + durationNanos;

    // opsPerStep gives us an estimated number of operations which need to be run to fully utilize
    // the CPU for a fixed amount of time (specified by kLoadGenerationStepSizeNanos).
    // After each step the opsPerStep value is re-calculated based on the actual time taken to
    // execute those operations.
    auto opsPerStep = (i32)(mOpsPerNano * kLoadGenerationStepSizeNanos);
    z64 stepDurationNanos = 0;
    z64 previousTimeNanos = 0;

    while (currentTimeNanos <= deadlineTimeNanos){

        for (i32 i = 0; i < opsPerStep; i++) cpu_relax();

        previousTimeNanos = currentTimeNanos;
        currentTimeNanos = AudioClock::getNanoseconds();
        stepDurationNanos = currentTimeNanos - previousTimeNanos;

        // Calculate exponential moving average to smooth out values, this acts as a low pass filter.
        // @see https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
        static const f32 kFilterCoefficient = 0.1;
        auto measuredOpsPerNano = (f64) opsPerStep / stepDurationNanos;
        mOpsPerNano = kFilterCoefficient * measuredOpsPerNano + (1.0 - kFilterCoefficient) * mOpsPerNano;
        opsPerStep = (i32) (mOpsPerNano * kLoadGenerationStepSizeNanos);
    }
}
