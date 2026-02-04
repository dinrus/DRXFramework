/* Copyright 2015 The Android Open Source Project
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
#include <sys/types.h>
#include <cassert>
#include <android/log.h>

#include <oboe/AudioStream.h>
#include <common/AudioClock.h>

#include "common/OboeDebug.h"
#include "oboe/AudioStreamBuilder.h"
#include "EngineOpenSLES.h"
#include "AudioStreamOpenSLES.h"
#include "OpenSLESUtilities.h"

using namespace oboe;

AudioStreamOpenSLES::AudioStreamOpenSLES(const AudioStreamBuilder &builder)
    : AudioStreamBuffered(builder) {
    // OpenSL ES does not support device IDs. So overwrite value from builder.
    mDeviceId = kUnspecified;
    // OpenSL ES does not support session IDs. So overwrite value from builder.
    mSessionId = SessionId::None;
}

static constexpr i32   kHighLatencyBufferSizeMillis = 20; // typical Android period
static constexpr SLuint32  kAudioChannelCountMax = 30; // TODO Why 30?
static constexpr SLuint32  SL_ANDROID_UNKNOWN_CHANNELMASK  = 0; // Matches name used internally.

SLuint32 AudioStreamOpenSLES::channelCountToChannelMaskDefault(i32 channelCount) const {
    if (channelCount > kAudioChannelCountMax) {
        return SL_ANDROID_UNKNOWN_CHANNELMASK;
    }

    SLuint32 bitfield = (1 << channelCount) - 1;

    // Check for OS at run-time.
    if(getSdkVersion() >= __ANDROID_API_N__) {
        return SL_ANDROID_MAKE_INDEXED_CHANNEL_MASK(bitfield);
    }

    // Indexed channels masks were added in N.
    // For before N, the best we can do is use a positional channel mask.
    return bitfield;
}

static b8 s_isLittleEndian() {
    static u32 value = 1;
    return (*reinterpret_cast<u8 *>(&value) == 1);  // Does address point to LSB?
}

SLuint32 AudioStreamOpenSLES::getDefaultByteOrder() {
    return s_isLittleEndian() ? SL_BYTEORDER_LITTLEENDIAN : SL_BYTEORDER_BIGENDIAN;
}

Result AudioStreamOpenSLES::open() {

    LOGI("AudioStreamOpenSLES::open() chans=%d, rate=%d", mChannelCount, mSampleRate);

    // OpenSL ES only supports I16 and Float
    if (mFormat != AudioFormat::I16 && mFormat != AudioFormat::Float) {
        LOGW("%s() Android's OpenSL ES implementation only supports I16 and Float. Format: %s",
             __func__, oboe::convertToText(mFormat));
        return Result::ErrorInvalidFormat;
    }

    SLresult result = EngineOpenSLES::getInstance().open();
    if (SL_RESULT_SUCCESS != result) {
        return Result::ErrorInternal;
    }

    Result oboeResult = AudioStreamBuffered::open();
    if (oboeResult != Result::OK) {
        EngineOpenSLES::getInstance().close();
        return oboeResult;
    }
    // Convert to defaults if UNSPECIFIED
    if (mSampleRate == kUnspecified) {
        mSampleRate = DefaultStreamValues::SampleRate;
    }
    if (mChannelCount == kUnspecified) {
        mChannelCount = DefaultStreamValues::ChannelCount;
    }
    if (mContentType == kUnspecified) {
        mContentType = ContentType::Music;
    }
    if (static_cast<const i32>(mUsage) == kUnspecified) {
        mUsage = Usage::Media;
    }

    mSharingMode = SharingMode::Shared;

    return Result::OK;
}


SLresult AudioStreamOpenSLES::finishCommonOpen(SLAndroidConfigurationItf configItf) {
    // Setting privacy sensitive mode and allowed capture policy are not supported for OpenSL ES.
    mPrivacySensitiveMode = PrivacySensitiveMode::Unspecified;
    mAllowedCapturePolicy = AllowedCapturePolicy::Unspecified;

    // Spatialization Behavior не поддерживается for OpenSL ES.
    mSpatializationBehavior = SpatializationBehavior::Never;

    SLresult result = registerBufferQueueCallback();
    if (SL_RESULT_SUCCESS != result) {
        return result;
    }

    result = updateStreamParameters(configItf);
    if (SL_RESULT_SUCCESS != result) {
        return result;
    }

    Result oboeResult = configureBufferSizes(mSampleRate);
    if (Result::OK != oboeResult) {
        return (SLresult) oboeResult;
    }

    allocateFifo();

    calculateDefaultDelayBeforeCloseMillis();

    return SL_RESULT_SUCCESS;
}

static i32 roundUpDivideByN(i32 x, i32 n) {
    return (x + n - 1) / n;
}

i32 AudioStreamOpenSLES::calculateOptimalBufferQueueLength() {
    i32 queueLength = kBufferQueueLengthDefault;
    i32 likelyFramesPerBurst = estimateNativeFramesPerBurst();
    i32 minCapacity = mBufferCapacityInFrames; // specified by app or zero
    // The buffer capacity needs to be at least twice the size of the requested callbackSize
    // so that we can have f64 buffering.
    minCapacity = std::max(minCapacity, kDoubleBufferCount * mFramesPerCallback);
    if (minCapacity > 0) {
        i32 queueLengthFromCapacity = roundUpDivideByN(minCapacity, likelyFramesPerBurst);
        queueLength = std::max(queueLength, queueLengthFromCapacity);
    }
    queueLength = std::min(queueLength, kBufferQueueLengthMax); // clip to max
    // TODO Investigate the effect of queueLength on latency for normal streams. (not low latency)
    return queueLength;
}

/**
 * The best information we have is if DefaultStreamValues::FramesPerBurst
 * was set by the app based on AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER.
 * Without that we just have to guess.
 * @return
 */
i32 AudioStreamOpenSLES::estimateNativeFramesPerBurst() {
    i32 framesPerBurst = DefaultStreamValues::FramesPerBurst;
    LOGD("AudioStreamOpenSLES:%s() DefaultStreamValues::FramesPerBurst = %d",
            __func__, DefaultStreamValues::FramesPerBurst);
    framesPerBurst = std::max(framesPerBurst, 16);
    // Calculate the size of a fixed duration high latency buffer based on sample rate.
    // Estimate sample based on default options in order of priority.
    i32 sampleRate = 48000;
    sampleRate = (DefaultStreamValues::SampleRate > 0)
            ? DefaultStreamValues::SampleRate : sampleRate;
    sampleRate = (mSampleRate > 0) ? mSampleRate : sampleRate;
    i32 framesPerHighLatencyBuffer =
            (kHighLatencyBufferSizeMillis * sampleRate) / kMillisPerSecond;
    // For high latency streams, use a larger buffer size.
    // Performance Mode support was added in N_MR1 (7.1)
    if (getSdkVersion() >= __ANDROID_API_N_MR1__
            && mPerformanceMode != PerformanceMode::LowLatency
            && framesPerBurst < framesPerHighLatencyBuffer) {
        // Find a multiple of framesPerBurst >= framesPerHighLatencyBuffer.
        i32 numBursts = roundUpDivideByN(framesPerHighLatencyBuffer, framesPerBurst);
        framesPerBurst *= numBursts;
        LOGD("AudioStreamOpenSLES:%s() NOT low latency, numBursts = %d, mSampleRate = %d, set framesPerBurst = %d",
             __func__, numBursts, mSampleRate, framesPerBurst);
    }
    return framesPerBurst;
}

Result AudioStreamOpenSLES::configureBufferSizes(i32 sampleRate) {
    LOGD("AudioStreamOpenSLES:%s(%d) initial mFramesPerBurst = %d, mFramesPerCallback = %d",
            __func__, mSampleRate, mFramesPerBurst, mFramesPerCallback);
    mFramesPerBurst = estimateNativeFramesPerBurst();
    mFramesPerCallback = (mFramesPerCallback > 0) ? mFramesPerCallback : mFramesPerBurst;
    LOGD("AudioStreamOpenSLES:%s(%d) final mFramesPerBurst = %d, mFramesPerCallback = %d",
         __func__, mSampleRate, mFramesPerBurst, mFramesPerCallback);

    mBytesPerCallback = mFramesPerCallback * getBytesPerFrame();
    if (mBytesPerCallback <= 0) {
        LOGE("AudioStreamOpenSLES::open() bytesPerCallback < 0 = %d, bad format?",
             mBytesPerCallback);
        return Result::ErrorInvalidFormat; // causing bytesPerFrame == 0
    }

    for (i32 i = 0; i < mBufferQueueLength; ++i) {
        mCallbackBuffer[i] = std::make_unique<u8[]>(mBytesPerCallback);
    }

    if (!usingFIFO()) {
        mBufferCapacityInFrames = mFramesPerBurst * mBufferQueueLength;
        // Check for overflow.
        if (mBufferCapacityInFrames <= 0) {
            mBufferCapacityInFrames = 0;
            LOGE("AudioStreamOpenSLES::open() numeric overflow because mFramesPerBurst = %d",
                 mFramesPerBurst);
            return Result::ErrorOutOfRange;
        }
        mBufferSizeInFrames = mBufferCapacityInFrames;
    }

    return Result::OK;
}

SLuint32 AudioStreamOpenSLES::convertPerformanceMode(PerformanceMode oboeMode) const {
    SLuint32 openslMode = SL_ANDROID_PERFORMANCE_NONE;
    switch(oboeMode) {
        case PerformanceMode::None:
            openslMode =  SL_ANDROID_PERFORMANCE_NONE;
            break;
        case PerformanceMode::LowLatency:
            openslMode =  (getSessionId() == SessionId::None) ?  SL_ANDROID_PERFORMANCE_LATENCY : SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS;
            break;
        case PerformanceMode::PowerSaving:
            openslMode =  SL_ANDROID_PERFORMANCE_POWER_SAVING;
            break;
        default:
            break;
    }
    return openslMode;
}

PerformanceMode AudioStreamOpenSLES::convertPerformanceMode(SLuint32 openslMode) const {
    PerformanceMode oboeMode = PerformanceMode::None;
    switch(openslMode) {
        case SL_ANDROID_PERFORMANCE_NONE:
            oboeMode =  PerformanceMode::None;
            break;
        case SL_ANDROID_PERFORMANCE_LATENCY:
        case SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS:
            oboeMode =  PerformanceMode::LowLatency;
            break;
        case SL_ANDROID_PERFORMANCE_POWER_SAVING:
            oboeMode =  PerformanceMode::PowerSaving;
            break;
        default:
            break;
    }
    return oboeMode;
}

z0 AudioStreamOpenSLES::logUnsupportedAttributes() {
    // Log unsupported attributes
    // only report if changed from the default

    // Device ID
    if (mDeviceId != kUnspecified) {
        LOGW("Device ID [AudioStreamBuilder::setDeviceId()] "
             "не поддерживается on OpenSLES streams.");
    }
    // Sharing Mode
    if (mSharingMode != SharingMode::Shared) {
        LOGW("SharingMode [AudioStreamBuilder::setSharingMode()] "
             "не поддерживается on OpenSLES streams.");
    }
    // Performance Mode
    i32 sdkVersion = getSdkVersion();
    if (mPerformanceMode != PerformanceMode::None && sdkVersion < __ANDROID_API_N_MR1__) {
        LOGW("PerformanceMode [AudioStreamBuilder::setPerformanceMode()] "
             "не поддерживается on OpenSLES streams running on pre-Android N-MR1 versions.");
    }
    // Content Type
    if (mContentType != ContentType::Music) {
        LOGW("ContentType [AudioStreamBuilder::setContentType()] "
             "не поддерживается on OpenSLES streams.");
    }

    // Session Id
    if (mSessionId != SessionId::None) {
        LOGW("SessionId [AudioStreamBuilder::setSessionId()] "
             "не поддерживается on OpenSLES streams.");
    }

    // Privacy Sensitive Mode
    if (mPrivacySensitiveMode != PrivacySensitiveMode::Unspecified) {
        LOGW("PrivacySensitiveMode [AudioStreamBuilder::setPrivacySensitiveMode()] "
             "не поддерживается on OpenSLES streams.");
    }

    // Spatialization Behavior
    if (mSpatializationBehavior != SpatializationBehavior::Unspecified) {
        LOGW("SpatializationBehavior [AudioStreamBuilder::setSpatializationBehavior()] "
             "не поддерживается on OpenSLES streams.");
    }

    // Allowed Capture Policy
    if (mAllowedCapturePolicy != AllowedCapturePolicy::Unspecified) {
        LOGW("AllowedCapturePolicy [AudioStreamBuilder::setAllowedCapturePolicy()] "
             "не поддерживается on OpenSLES streams.");
    }
}

SLresult AudioStreamOpenSLES::configurePerformanceMode(SLAndroidConfigurationItf configItf) {

    if (configItf == nullptr) {
        LOGW("%s() called with NULL configuration", __func__);
        mPerformanceMode = PerformanceMode::None;
        return SL_RESULT_INTERNAL_ERROR;
    }
    if (getSdkVersion() < __ANDROID_API_N_MR1__) {
        LOGW("%s() not supported until N_MR1", __func__);
        mPerformanceMode = PerformanceMode::None;
        return SL_RESULT_SUCCESS;
    }

    SLresult result = SL_RESULT_SUCCESS;
    SLuint32 performanceMode = convertPerformanceMode(getPerformanceMode());
    result = (*configItf)->SetConfiguration(configItf, SL_ANDROID_KEY_PERFORMANCE_MODE,
                                                     &performanceMode, sizeof(performanceMode));
    if (SL_RESULT_SUCCESS != result) {
        LOGW("SetConfiguration(PERFORMANCE_MODE, SL %u) returned %s",
             performanceMode, getSLErrStr(result));
        mPerformanceMode = PerformanceMode::None;
    }

    return result;
}

SLresult AudioStreamOpenSLES::updateStreamParameters(SLAndroidConfigurationItf configItf) {
    SLresult result = SL_RESULT_SUCCESS;
    if(getSdkVersion() >= __ANDROID_API_N_MR1__ && configItf != nullptr) {
        SLuint32 performanceMode = 0;
        SLuint32 performanceModeSize = sizeof(performanceMode);
        result = (*configItf)->GetConfiguration(configItf, SL_ANDROID_KEY_PERFORMANCE_MODE,
                                                &performanceModeSize, &performanceMode);
        // A bug in GetConfiguration() before P caused a wrong result code to be returned.
        if (getSdkVersion() <= __ANDROID_API_O_MR1__) {
            result = SL_RESULT_SUCCESS; // Ignore actual result before P.
        }

        if (SL_RESULT_SUCCESS != result) {
            LOGW("GetConfiguration(SL_ANDROID_KEY_PERFORMANCE_MODE) returned %d", result);
            mPerformanceMode = PerformanceMode::None; // If we can't query it then assume None.
        } else {
            mPerformanceMode = convertPerformanceMode(performanceMode); // convert SL to Oboe mode
        }
    } else {
        mPerformanceMode = PerformanceMode::None; // If we can't query it then assume None.
    }
    return result;
}

// This is called under mLock.
Result AudioStreamOpenSLES::close_l() {
    if (mState == StreamState::Closed) {
        return Result::ErrorClosed;
    }

    AudioStreamBuffered::close();

    onBeforeDestroy();

    if (mObjectInterface != nullptr) {
        (*mObjectInterface)->Destroy(mObjectInterface);
        mObjectInterface = nullptr;
    }

    onAfterDestroy();

    mSimpleBufferQueueInterface = nullptr;
    EngineOpenSLES::getInstance().close();

    setState(StreamState::Closed);

    return Result::OK;
}

SLresult AudioStreamOpenSLES::enqueueCallbackBuffer(SLAndroidSimpleBufferQueueItf bq) {
    SLresult result = (*bq)->Enqueue(
            bq, mCallbackBuffer[mCallbackBufferIndex].get(), mBytesPerCallback);
    mCallbackBufferIndex = (mCallbackBufferIndex + 1) % mBufferQueueLength;
    return result;
}

i32 AudioStreamOpenSLES::getBufferDepth(SLAndroidSimpleBufferQueueItf bq) {
    SLAndroidSimpleBufferQueueState queueState;
    SLresult result = (*bq)->GetState(bq, &queueState);
    return (result == SL_RESULT_SUCCESS) ? queueState.count : -1;
}

b8 AudioStreamOpenSLES::processBufferCallback(SLAndroidSimpleBufferQueueItf bq) {
    b8 shouldStopStream = false;
    // Ask the app callback to process the buffer.
    DataCallbackResult result =
            fireDataCallback(mCallbackBuffer[mCallbackBufferIndex].get(), mFramesPerCallback);
    if (result == DataCallbackResult::Continue) {
        // Pass the buffer to OpenSLES.
        SLresult enqueueResult = enqueueCallbackBuffer(bq);
        if (enqueueResult != SL_RESULT_SUCCESS) {
            LOGE("%s() returned %d", __func__, enqueueResult);
            shouldStopStream = true;
        }
        // Update Oboe client position with frames handled by the callback.
        if (getDirection() == Direction::Input) {
            mFramesRead += mFramesPerCallback;
        } else {
            mFramesWritten += mFramesPerCallback;
        }
    } else if (result == DataCallbackResult::Stop) {
        LOGD("Oboe callback returned Stop");
        shouldStopStream = true;
    } else {
        LOGW("Oboe callback returned unexpected value = %d", static_cast<i32>(result));
        shouldStopStream = true;
    }
    if (shouldStopStream) {
        mCallbackBufferIndex = 0;
    }
    return shouldStopStream;
}

// This callback handler is called every time a buffer has been processed by OpenSL ES.
static z0 bqCallbackGlue(SLAndroidSimpleBufferQueueItf bq, uk context) {
    b8 shouldStopStream = (reinterpret_cast<AudioStreamOpenSLES *>(context))
            ->processBufferCallback(bq);
    if (shouldStopStream) {
        (reinterpret_cast<AudioStreamOpenSLES *>(context))->requestStop();
    }
}

SLresult AudioStreamOpenSLES::registerBufferQueueCallback() {
    // The BufferQueue
    SLresult result = (*mObjectInterface)->GetInterface(mObjectInterface,
            EngineOpenSLES::getInstance().getIidAndroidSimpleBufferQueue(),
            &mSimpleBufferQueueInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("get buffer queue interface:%p result:%s",
             mSimpleBufferQueueInterface,
             getSLErrStr(result));
    } else {
        // Register the BufferQueue callback
        result = (*mSimpleBufferQueueInterface)->RegisterCallback(mSimpleBufferQueueInterface,
                                                                  bqCallbackGlue, this);
        if (SL_RESULT_SUCCESS != result) {
            LOGE("RegisterCallback result:%s", getSLErrStr(result));
        }
    }
    return result;
}

z64 AudioStreamOpenSLES::getFramesProcessedByServer() {
    updateServiceFrameCounter();
    z64 millis64 = mPositionMillis.get();
    z64 framesProcessed = millis64 * getSampleRate() / kMillisPerSecond;
    return framesProcessed;
}

Result AudioStreamOpenSLES::waitForStateChange(StreamState currentState,
                                                     StreamState *nextState,
                                                     z64 timeoutNanoseconds) {
    Result oboeResult = Result::ErrorTimeout;
    z64 sleepTimeNanos = 20 * kNanosPerMillisecond; // arbitrary
    z64 timeLeftNanos = timeoutNanoseconds;

    while (true) {
        const StreamState state = getState(); // this does not require a lock
        if (nextState != nullptr) {
            *nextState = state;
        }
        if (currentState != state) { // state changed?
            oboeResult = Result::OK;
            break;
        }

        // Did we timeout or did user ask for non-blocking?
        if (timeLeftNanos <= 0) {
            break;
        }

        if (sleepTimeNanos > timeLeftNanos){
            sleepTimeNanos = timeLeftNanos;
        }
        AudioClock::sleepForNanos(sleepTimeNanos);
        timeLeftNanos -= sleepTimeNanos;
    }

    return oboeResult;
}
