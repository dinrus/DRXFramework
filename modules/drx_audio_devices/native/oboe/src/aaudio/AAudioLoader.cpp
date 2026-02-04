/*
 * Copyright 2016 The Android Open Source Project
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

#include <dlfcn.h>
#include <oboe/Utilities.h>
#include "common/OboeDebug.h"
#include "AAudioLoader.h"

#define LIB_AAUDIO_NAME "libaaudio.so"

namespace oboe {

AAudioLoader::~AAudioLoader() {
    // Issue 360: thread_local variables with non-trivial destructors
    // will cause segfaults if the containing library is dlclose()ed on
    // devices running M or newer, or devices before M when using a static STL.
    // The simple workaround is to not call dlclose.
    // https://github.com/android/ndk/wiki/Changelog-r22#known-issues
    //
    // The libaaudio and libaaudioclient do not use thread_local.
    // But, to be safe, we should avoid dlclose() if possible.
    // Because AAudioLoader is a static Singleton, we can safely skip
    // calling dlclose() without causing a resource leak.
    LOGI("%s() dlclose(%s) not called, OK", __func__, LIB_AAUDIO_NAME);
}

AAudioLoader* AAudioLoader::getInstance() {
    static AAudioLoader instance;
    return &instance;
}

i32 AAudioLoader::open() {
    if (mLibHandle != nullptr) {
        return 0;
    }

    // Use RTLD_NOW to avoid the unpredictable behavior that RTLD_LAZY can cause.
    // Also resolving all the links now will prevent a run-time penalty later.
    mLibHandle = dlopen(LIB_AAUDIO_NAME, RTLD_NOW);
    if (mLibHandle == nullptr) {
        LOGI("AAudioLoader::open() could not find " LIB_AAUDIO_NAME);
        return -1; // TODO review return code
    } else {
        LOGD("AAudioLoader():  dlopen(%s) returned %p", LIB_AAUDIO_NAME, mLibHandle);
    }

    // Load all the function pointers.
    createStreamBuilder        = load_I_PPB("AAudio_createStreamBuilder");
    builder_openStream         = load_I_PBPPS("AAudioStreamBuilder_openStream");

    builder_setChannelCount    = load_V_PBI("AAudioStreamBuilder_setChannelCount");
    if (builder_setChannelCount == nullptr) {
        // Use old deprecated alias if needed.
        builder_setChannelCount = load_V_PBI("AAudioStreamBuilder_setSamplesPerFrame");
    }

    builder_setBufferCapacityInFrames = load_V_PBI("AAudioStreamBuilder_setBufferCapacityInFrames");
    builder_setDeviceId        = load_V_PBI("AAudioStreamBuilder_setDeviceId");
    builder_setDirection       = load_V_PBI("AAudioStreamBuilder_setDirection");
    builder_setFormat          = load_V_PBI("AAudioStreamBuilder_setFormat");
    builder_setFramesPerDataCallback = load_V_PBI("AAudioStreamBuilder_setFramesPerDataCallback");
    builder_setSharingMode     = load_V_PBI("AAudioStreamBuilder_setSharingMode");
    builder_setPerformanceMode = load_V_PBI("AAudioStreamBuilder_setPerformanceMode");
    builder_setSampleRate      = load_V_PBI("AAudioStreamBuilder_setSampleRate");

    if (getSdkVersion() >= __ANDROID_API_P__){
        builder_setUsage       = load_V_PBI("AAudioStreamBuilder_setUsage");
        builder_setContentType = load_V_PBI("AAudioStreamBuilder_setContentType");
        builder_setInputPreset = load_V_PBI("AAudioStreamBuilder_setInputPreset");
        builder_setSessionId   = load_V_PBI("AAudioStreamBuilder_setSessionId");
    }

    if (getSdkVersion() >= __ANDROID_API_Q__){
        builder_setAllowedCapturePolicy = load_V_PBI("AAudioStreamBuilder_setAllowedCapturePolicy");
    }

    if (getSdkVersion() >= __ANDROID_API_R__){
        builder_setPrivacySensitive  = load_V_PBO("AAudioStreamBuilder_setPrivacySensitive");
    }

    if (getSdkVersion() >= __ANDROID_API_S__){
        builder_setPackageName       = load_V_PBCPH("AAudioStreamBuilder_setPackageName");
        builder_setAttributionTag    = load_V_PBCPH("AAudioStreamBuilder_setAttributionTag");
    }

    if (getSdkVersion() >= __ANDROID_API_S_V2__) {
        builder_setChannelMask = load_V_PBU("AAudioStreamBuilder_setChannelMask");
        builder_setIsContentSpatialized = load_V_PBO("AAudioStreamBuilder_setIsContentSpatialized");
        builder_setSpatializationBehavior = load_V_PBI("AAudioStreamBuilder_setSpatializationBehavior");
    }

    builder_delete             = load_I_PB("AAudioStreamBuilder_delete");


    builder_setDataCallback    = load_V_PBPDPV("AAudioStreamBuilder_setDataCallback");
    builder_setErrorCallback   = load_V_PBPEPV("AAudioStreamBuilder_setErrorCallback");

    stream_read                = load_I_PSPVIL("AAudioStream_read");

    stream_write               = load_I_PSCPVIL("AAudioStream_write");

    stream_waitForStateChange  = load_I_PSTPTL("AAudioStream_waitForStateChange");

    stream_getTimestamp        = load_I_PSKPLPL("AAudioStream_getTimestamp");

    stream_getChannelCount     = load_I_PS("AAudioStream_getChannelCount");
    if (stream_getChannelCount == nullptr) {
        // Use old alias if needed.
        stream_getChannelCount    = load_I_PS("AAudioStream_getSamplesPerFrame");
    }

    if (getSdkVersion() >= __ANDROID_API_R__) {
        stream_release         = load_I_PS("AAudioStream_release");
    }

    stream_close               = load_I_PS("AAudioStream_close");

    stream_getBufferSize       = load_I_PS("AAudioStream_getBufferSizeInFrames");
    stream_getDeviceId         = load_I_PS("AAudioStream_getDeviceId");
    stream_getBufferCapacity   = load_I_PS("AAudioStream_getBufferCapacityInFrames");
    stream_getFormat           = load_F_PS("AAudioStream_getFormat");
    stream_getFramesPerBurst   = load_I_PS("AAudioStream_getFramesPerBurst");
    stream_getFramesRead       = load_L_PS("AAudioStream_getFramesRead");
    stream_getFramesWritten    = load_L_PS("AAudioStream_getFramesWritten");
    stream_getPerformanceMode  = load_I_PS("AAudioStream_getPerformanceMode");
    stream_getSampleRate       = load_I_PS("AAudioStream_getSampleRate");
    stream_getSharingMode      = load_I_PS("AAudioStream_getSharingMode");
    stream_getState            = load_I_PS("AAudioStream_getState");
    stream_getXRunCount        = load_I_PS("AAudioStream_getXRunCount");

    stream_requestStart        = load_I_PS("AAudioStream_requestStart");
    stream_requestPause        = load_I_PS("AAudioStream_requestPause");
    stream_requestFlush        = load_I_PS("AAudioStream_requestFlush");
    stream_requestStop         = load_I_PS("AAudioStream_requestStop");

    stream_setBufferSize       = load_I_PSI("AAudioStream_setBufferSizeInFrames");

    convertResultToText        = load_CPH_I("AAudio_convertResultToText");

    if (getSdkVersion() >= __ANDROID_API_P__){
        stream_getUsage        = load_I_PS("AAudioStream_getUsage");
        stream_getContentType  = load_I_PS("AAudioStream_getContentType");
        stream_getInputPreset  = load_I_PS("AAudioStream_getInputPreset");
        stream_getSessionId    = load_I_PS("AAudioStream_getSessionId");
    }

    if (getSdkVersion() >= __ANDROID_API_Q__){
        stream_getAllowedCapturePolicy    = load_I_PS("AAudioStream_getAllowedCapturePolicy");
    }

    if (getSdkVersion() >= __ANDROID_API_R__){
        stream_isPrivacySensitive  = load_O_PS("AAudioStream_isPrivacySensitive");
    }

    if (getSdkVersion() >= __ANDROID_API_S_V2__) {
        stream_getChannelMask = load_U_PS("AAudioStream_getChannelMask");
        stream_isContentSpatialized = load_O_PS("AAudioStream_isContentSpatialized");
        stream_getSpatializationBehavior = load_I_PS("AAudioStream_getSpatializationBehavior");
    }

    if (getSdkVersion() >= __ANDROID_API_U__) {
        stream_getHardwareChannelCount = load_I_PS("AAudioStream_getHardwareChannelCount");
        stream_getHardwareSampleRate = load_I_PS("AAudioStream_getHardwareSampleRate");
        stream_getHardwareFormat = load_F_PS("AAudioStream_getHardwareFormat");
    }

    return 0;
}

static z0 AAudioLoader_check(uk proc, const t8 *functionName) {
    if (proc == nullptr) {
        LOGW("AAudioLoader could not find %s", functionName);
    }
}

AAudioLoader::signature_I_PPB AAudioLoader::load_I_PPB(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PPB>(proc);
}

AAudioLoader::signature_CPH_I AAudioLoader::load_CPH_I(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_CPH_I>(proc);
}

AAudioLoader::signature_V_PBI AAudioLoader::load_V_PBI(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_V_PBI>(proc);
}

AAudioLoader::signature_V_PBCPH AAudioLoader::load_V_PBCPH(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_V_PBCPH>(proc);
}

AAudioLoader::signature_V_PBPDPV AAudioLoader::load_V_PBPDPV(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_V_PBPDPV>(proc);
}

AAudioLoader::signature_V_PBPEPV AAudioLoader::load_V_PBPEPV(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_V_PBPEPV>(proc);
}

AAudioLoader::signature_I_PSI AAudioLoader::load_I_PSI(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PSI>(proc);
}

AAudioLoader::signature_I_PS AAudioLoader::load_I_PS(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PS>(proc);
}

AAudioLoader::signature_L_PS AAudioLoader::load_L_PS(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_L_PS>(proc);
}

AAudioLoader::signature_F_PS AAudioLoader::load_F_PS(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_F_PS>(proc);
}

AAudioLoader::signature_O_PS AAudioLoader::load_O_PS(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_O_PS>(proc);
}

AAudioLoader::signature_I_PB AAudioLoader::load_I_PB(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PB>(proc);
}

AAudioLoader::signature_I_PBPPS AAudioLoader::load_I_PBPPS(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PBPPS>(proc);
}

AAudioLoader::signature_I_PSCPVIL AAudioLoader::load_I_PSCPVIL(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PSCPVIL>(proc);
}

AAudioLoader::signature_I_PSPVIL AAudioLoader::load_I_PSPVIL(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PSPVIL>(proc);
}

AAudioLoader::signature_I_PSTPTL AAudioLoader::load_I_PSTPTL(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PSTPTL>(proc);
}

AAudioLoader::signature_I_PSKPLPL AAudioLoader::load_I_PSKPLPL(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_I_PSKPLPL>(proc);
}

AAudioLoader::signature_V_PBU AAudioLoader::load_V_PBU(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_V_PBU>(proc);
}

AAudioLoader::signature_U_PS AAudioLoader::load_U_PS(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_U_PS>(proc);
}

AAudioLoader::signature_V_PBO AAudioLoader::load_V_PBO(const t8 *functionName) {
    uk proc = dlsym(mLibHandle, functionName);
    AAudioLoader_check(proc, functionName);
    return reinterpret_cast<signature_V_PBO>(proc);
}

// Ensure that all AAudio primitive data types are i32
#define ASSERT_INT32(type) static_assert(std::is_same<i32, type>::value, \
#type" must be i32")

// Ensure that all AAudio primitive data types are u32
#define ASSERT_UINT32(type) static_assert(std::is_same<u32, type>::value, \
#type" must be u32")

#define ERRMSG "Oboe constants must match AAudio constants."

// These asserts help verify that the Oboe definitions match the equivalent AAudio definitions.
// This code is in this .cpp file so it only gets tested once.
#ifdef AAUDIO_AAUDIO_H

    ASSERT_INT32(aaudio_stream_state_t);
    ASSERT_INT32(aaudio_direction_t);
    ASSERT_INT32(aaudio_format_t);
    ASSERT_INT32(aaudio_data_callback_result_t);
    ASSERT_INT32(aaudio_result_t);
    ASSERT_INT32(aaudio_sharing_mode_t);
    ASSERT_INT32(aaudio_performance_mode_t);

    static_assert((i32)StreamState::Uninitialized == AAUDIO_STREAM_STATE_UNINITIALIZED, ERRMSG);
    static_assert((i32)StreamState::Unknown == AAUDIO_STREAM_STATE_UNKNOWN, ERRMSG);
    static_assert((i32)StreamState::Open == AAUDIO_STREAM_STATE_OPEN, ERRMSG);
    static_assert((i32)StreamState::Starting == AAUDIO_STREAM_STATE_STARTING, ERRMSG);
    static_assert((i32)StreamState::Started == AAUDIO_STREAM_STATE_STARTED, ERRMSG);
    static_assert((i32)StreamState::Pausing == AAUDIO_STREAM_STATE_PAUSING, ERRMSG);
    static_assert((i32)StreamState::Paused == AAUDIO_STREAM_STATE_PAUSED, ERRMSG);
    static_assert((i32)StreamState::Flushing == AAUDIO_STREAM_STATE_FLUSHING, ERRMSG);
    static_assert((i32)StreamState::Flushed == AAUDIO_STREAM_STATE_FLUSHED, ERRMSG);
    static_assert((i32)StreamState::Stopping == AAUDIO_STREAM_STATE_STOPPING, ERRMSG);
    static_assert((i32)StreamState::Stopped == AAUDIO_STREAM_STATE_STOPPED, ERRMSG);
    static_assert((i32)StreamState::Closing == AAUDIO_STREAM_STATE_CLOSING, ERRMSG);
    static_assert((i32)StreamState::Closed == AAUDIO_STREAM_STATE_CLOSED, ERRMSG);
    static_assert((i32)StreamState::Disconnected == AAUDIO_STREAM_STATE_DISCONNECTED, ERRMSG);

    static_assert((i32)Direction::Output == AAUDIO_DIRECTION_OUTPUT, ERRMSG);
    static_assert((i32)Direction::Input == AAUDIO_DIRECTION_INPUT, ERRMSG);

    static_assert((i32)AudioFormat::Invalid == AAUDIO_FORMAT_INVALID, ERRMSG);
    static_assert((i32)AudioFormat::Unspecified == AAUDIO_FORMAT_UNSPECIFIED, ERRMSG);
    static_assert((i32)AudioFormat::I16 == AAUDIO_FORMAT_PCM_I16, ERRMSG);
    static_assert((i32)AudioFormat::Float == AAUDIO_FORMAT_PCM_FLOAT, ERRMSG);

    static_assert((i32)DataCallbackResult::Continue == AAUDIO_CALLBACK_RESULT_CONTINUE, ERRMSG);
    static_assert((i32)DataCallbackResult::Stop == AAUDIO_CALLBACK_RESULT_STOP, ERRMSG);

    static_assert((i32)Result::OK == AAUDIO_OK, ERRMSG);
    static_assert((i32)Result::ErrorBase == AAUDIO_ERROR_BASE, ERRMSG);
    static_assert((i32)Result::ErrorDisconnected == AAUDIO_ERROR_DISCONNECTED, ERRMSG);
    static_assert((i32)Result::ErrorIllegalArgument == AAUDIO_ERROR_ILLEGAL_ARGUMENT, ERRMSG);
    static_assert((i32)Result::ErrorInternal == AAUDIO_ERROR_INTERNAL, ERRMSG);
    static_assert((i32)Result::ErrorInvalidState == AAUDIO_ERROR_INVALID_STATE, ERRMSG);
    static_assert((i32)Result::ErrorInvalidHandle == AAUDIO_ERROR_INVALID_HANDLE, ERRMSG);
    static_assert((i32)Result::ErrorUnimplemented == AAUDIO_ERROR_UNIMPLEMENTED, ERRMSG);
    static_assert((i32)Result::ErrorUnavailable == AAUDIO_ERROR_UNAVAILABLE, ERRMSG);
    static_assert((i32)Result::ErrorNoFreeHandles == AAUDIO_ERROR_NO_FREE_HANDLES, ERRMSG);
    static_assert((i32)Result::ErrorNoMemory == AAUDIO_ERROR_NO_MEMORY, ERRMSG);
    static_assert((i32)Result::ErrorNull == AAUDIO_ERROR_NULL, ERRMSG);
    static_assert((i32)Result::ErrorTimeout == AAUDIO_ERROR_TIMEOUT, ERRMSG);
    static_assert((i32)Result::ErrorWouldBlock == AAUDIO_ERROR_WOULD_BLOCK, ERRMSG);
    static_assert((i32)Result::ErrorInvalidFormat == AAUDIO_ERROR_INVALID_FORMAT, ERRMSG);
    static_assert((i32)Result::ErrorOutOfRange == AAUDIO_ERROR_OUT_OF_RANGE, ERRMSG);
    static_assert((i32)Result::ErrorNoService == AAUDIO_ERROR_NO_SERVICE, ERRMSG);
    static_assert((i32)Result::ErrorInvalidRate == AAUDIO_ERROR_INVALID_RATE, ERRMSG);

    static_assert((i32)SharingMode::Exclusive == AAUDIO_SHARING_MODE_EXCLUSIVE, ERRMSG);
    static_assert((i32)SharingMode::Shared == AAUDIO_SHARING_MODE_SHARED, ERRMSG);

    static_assert((i32)PerformanceMode::None == AAUDIO_PERFORMANCE_MODE_NONE, ERRMSG);
    static_assert((i32)PerformanceMode::PowerSaving
            == AAUDIO_PERFORMANCE_MODE_POWER_SAVING, ERRMSG);
    static_assert((i32)PerformanceMode::LowLatency
            == AAUDIO_PERFORMANCE_MODE_LOW_LATENCY, ERRMSG);

// The aaudio_ usage, content and input_preset types were added in NDK 17,
// which is the first version to support Android Pie (API 28).
#if __NDK_MAJOR__ >= 17

    ASSERT_INT32(aaudio_usage_t);
    ASSERT_INT32(aaudio_content_type_t);
    ASSERT_INT32(aaudio_input_preset_t);

    static_assert((i32)Usage::Media == AAUDIO_USAGE_MEDIA, ERRMSG);
    static_assert((i32)Usage::VoiceCommunication == AAUDIO_USAGE_VOICE_COMMUNICATION, ERRMSG);
    static_assert((i32)Usage::VoiceCommunicationSignalling
            == AAUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING, ERRMSG);
    static_assert((i32)Usage::Alarm == AAUDIO_USAGE_ALARM, ERRMSG);
    static_assert((i32)Usage::Notification == AAUDIO_USAGE_NOTIFICATION, ERRMSG);
    static_assert((i32)Usage::NotificationRingtone == AAUDIO_USAGE_NOTIFICATION_RINGTONE, ERRMSG);
    static_assert((i32)Usage::NotificationEvent == AAUDIO_USAGE_NOTIFICATION_EVENT, ERRMSG);
    static_assert((i32)Usage::AssistanceAccessibility == AAUDIO_USAGE_ASSISTANCE_ACCESSIBILITY, ERRMSG);
    static_assert((i32)Usage::AssistanceNavigationGuidance
            == AAUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE, ERRMSG);
    static_assert((i32)Usage::AssistanceSonification == AAUDIO_USAGE_ASSISTANCE_SONIFICATION, ERRMSG);
    static_assert((i32)Usage::Game == AAUDIO_USAGE_GAME, ERRMSG);
    static_assert((i32)Usage::Assistant == AAUDIO_USAGE_ASSISTANT, ERRMSG);

    static_assert((i32)ContentType::Speech == AAUDIO_CONTENT_TYPE_SPEECH, ERRMSG);
    static_assert((i32)ContentType::Music == AAUDIO_CONTENT_TYPE_MUSIC, ERRMSG);
    static_assert((i32)ContentType::Movie == AAUDIO_CONTENT_TYPE_MOVIE, ERRMSG);
    static_assert((i32)ContentType::Sonification == AAUDIO_CONTENT_TYPE_SONIFICATION, ERRMSG);

    static_assert((i32)InputPreset::Generic == AAUDIO_INPUT_PRESET_GENERIC, ERRMSG);
    static_assert((i32)InputPreset::Camcorder == AAUDIO_INPUT_PRESET_CAMCORDER, ERRMSG);
    static_assert((i32)InputPreset::VoiceRecognition == AAUDIO_INPUT_PRESET_VOICE_RECOGNITION, ERRMSG);
    static_assert((i32)InputPreset::VoiceCommunication
            == AAUDIO_INPUT_PRESET_VOICE_COMMUNICATION, ERRMSG);
    static_assert((i32)InputPreset::Unprocessed == AAUDIO_INPUT_PRESET_UNPROCESSED, ERRMSG);

    static_assert((i32)SessionId::None == AAUDIO_SESSION_ID_NONE, ERRMSG);
    static_assert((i32)SessionId::Allocate == AAUDIO_SESSION_ID_ALLOCATE, ERRMSG);

#endif // __NDK_MAJOR__ >= 17

// aaudio_allowed_capture_policy_t was added in NDK 20,
// which is the first version to support Android Q (API 29).
#if __NDK_MAJOR__ >= 20

    ASSERT_INT32(aaudio_allowed_capture_policy_t);

    static_assert((i32)AllowedCapturePolicy::Unspecified == AAUDIO_UNSPECIFIED, ERRMSG);
    static_assert((i32)AllowedCapturePolicy::All == AAUDIO_ALLOW_CAPTURE_BY_ALL, ERRMSG);
    static_assert((i32)AllowedCapturePolicy::System == AAUDIO_ALLOW_CAPTURE_BY_SYSTEM, ERRMSG);
    static_assert((i32)AllowedCapturePolicy::None == AAUDIO_ALLOW_CAPTURE_BY_NONE, ERRMSG);

#endif // __NDK_MAJOR__ >= 20

// The aaudio channel masks and spatialization behavior were added in NDK 24,
// which is the first version to support Android SC_V2 (API 32).
#if __NDK_MAJOR__ >= 24

    ASSERT_UINT32(aaudio_channel_mask_t);

    static_assert((u32)ChannelMask::FrontLeft == AAUDIO_CHANNEL_FRONT_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::FrontRight == AAUDIO_CHANNEL_FRONT_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::FrontCenter == AAUDIO_CHANNEL_FRONT_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::LowFrequency == AAUDIO_CHANNEL_LOW_FREQUENCY, ERRMSG);
    static_assert((u32)ChannelMask::BackLeft == AAUDIO_CHANNEL_BACK_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::BackRight == AAUDIO_CHANNEL_BACK_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::FrontLeftOfCenter == AAUDIO_CHANNEL_FRONT_LEFT_OF_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::FrontRightOfCenter == AAUDIO_CHANNEL_FRONT_RIGHT_OF_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::BackCenter == AAUDIO_CHANNEL_BACK_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::SideLeft == AAUDIO_CHANNEL_SIDE_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::SideRight == AAUDIO_CHANNEL_SIDE_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::TopCenter == AAUDIO_CHANNEL_TOP_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::TopFrontLeft == AAUDIO_CHANNEL_TOP_FRONT_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::TopFrontCenter == AAUDIO_CHANNEL_TOP_FRONT_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::TopFrontRight == AAUDIO_CHANNEL_TOP_FRONT_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::TopBackLeft == AAUDIO_CHANNEL_TOP_BACK_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::TopBackCenter == AAUDIO_CHANNEL_TOP_BACK_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::TopBackRight == AAUDIO_CHANNEL_TOP_BACK_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::TopSideLeft == AAUDIO_CHANNEL_TOP_SIDE_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::TopSideRight == AAUDIO_CHANNEL_TOP_SIDE_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::BottomFrontLeft == AAUDIO_CHANNEL_BOTTOM_FRONT_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::BottomFrontCenter == AAUDIO_CHANNEL_BOTTOM_FRONT_CENTER, ERRMSG);
    static_assert((u32)ChannelMask::BottomFrontRight == AAUDIO_CHANNEL_BOTTOM_FRONT_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::LowFrequency2 == AAUDIO_CHANNEL_LOW_FREQUENCY_2, ERRMSG);
    static_assert((u32)ChannelMask::FrontWideLeft == AAUDIO_CHANNEL_FRONT_WIDE_LEFT, ERRMSG);
    static_assert((u32)ChannelMask::FrontWideRight == AAUDIO_CHANNEL_FRONT_WIDE_RIGHT, ERRMSG);
    static_assert((u32)ChannelMask::Mono == AAUDIO_CHANNEL_MONO, ERRMSG);
    static_assert((u32)ChannelMask::Stereo == AAUDIO_CHANNEL_STEREO, ERRMSG);
    static_assert((u32)ChannelMask::CM2Point1 == AAUDIO_CHANNEL_2POINT1, ERRMSG);
    static_assert((u32)ChannelMask::Tri == AAUDIO_CHANNEL_TRI, ERRMSG);
    static_assert((u32)ChannelMask::TriBack == AAUDIO_CHANNEL_TRI_BACK, ERRMSG);
    static_assert((u32)ChannelMask::CM3Point1 == AAUDIO_CHANNEL_3POINT1, ERRMSG);
    static_assert((u32)ChannelMask::CM2Point0Point2 == AAUDIO_CHANNEL_2POINT0POINT2, ERRMSG);
    static_assert((u32)ChannelMask::CM2Point1Point2 == AAUDIO_CHANNEL_2POINT1POINT2, ERRMSG);
    static_assert((u32)ChannelMask::CM3Point0Point2 == AAUDIO_CHANNEL_3POINT0POINT2, ERRMSG);
    static_assert((u32)ChannelMask::CM3Point1Point2 == AAUDIO_CHANNEL_3POINT1POINT2, ERRMSG);
    static_assert((u32)ChannelMask::Quad == AAUDIO_CHANNEL_QUAD, ERRMSG);
    static_assert((u32)ChannelMask::QuadSide == AAUDIO_CHANNEL_QUAD_SIDE, ERRMSG);
    static_assert((u32)ChannelMask::Surround == AAUDIO_CHANNEL_SURROUND, ERRMSG);
    static_assert((u32)ChannelMask::Penta == AAUDIO_CHANNEL_PENTA, ERRMSG);
    static_assert((u32)ChannelMask::CM5Point1 == AAUDIO_CHANNEL_5POINT1, ERRMSG);
    static_assert((u32)ChannelMask::CM5Point1Side == AAUDIO_CHANNEL_5POINT1_SIDE, ERRMSG);
    static_assert((u32)ChannelMask::CM6Point1 == AAUDIO_CHANNEL_6POINT1, ERRMSG);
    static_assert((u32)ChannelMask::CM7Point1 == AAUDIO_CHANNEL_7POINT1, ERRMSG);
    static_assert((u32)ChannelMask::CM5Point1Point2 == AAUDIO_CHANNEL_5POINT1POINT2, ERRMSG);
    static_assert((u32)ChannelMask::CM5Point1Point4 == AAUDIO_CHANNEL_5POINT1POINT4, ERRMSG);
    static_assert((u32)ChannelMask::CM7Point1Point2 == AAUDIO_CHANNEL_7POINT1POINT2, ERRMSG);
    static_assert((u32)ChannelMask::CM7Point1Point4 == AAUDIO_CHANNEL_7POINT1POINT4, ERRMSG);
    static_assert((u32)ChannelMask::CM9Point1Point4 == AAUDIO_CHANNEL_9POINT1POINT4, ERRMSG);
    static_assert((u32)ChannelMask::CM9Point1Point6 == AAUDIO_CHANNEL_9POINT1POINT6, ERRMSG);
    static_assert((u32)ChannelMask::FrontBack == AAUDIO_CHANNEL_FRONT_BACK, ERRMSG);

    ASSERT_INT32(aaudio_spatialization_behavior_t);

    static_assert((i32)SpatializationBehavior::Unspecified == AAUDIO_UNSPECIFIED, ERRMSG);
    static_assert((i32)SpatializationBehavior::Auto == AAUDIO_SPATIALIZATION_BEHAVIOR_AUTO, ERRMSG);
    static_assert((i32)SpatializationBehavior::Never == AAUDIO_SPATIALIZATION_BEHAVIOR_NEVER, ERRMSG);

#endif

#endif // AAUDIO_AAUDIO_H

} // namespace oboe
