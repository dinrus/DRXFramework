/*
 * Copyright 2021 The Android Open Source Project
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
#include <stdint.h>
#include <sys/types.h>

#include "AdpfWrapper.h"
#include "AudioClock.h"
#include "OboeDebug.h"

typedef APerformanceHintManager* (*APH_getManager)();
typedef APerformanceHintSession* (*APH_createSession)(APerformanceHintManager*, const i32*,
                                                      size_t, z64);
typedef z0 (*APH_reportActualWorkDuration)(APerformanceHintSession*, z64);
typedef z0 (*APH_closeSession)(APerformanceHintSession* session);

static b8 gAPerformanceHintBindingInitialized = false;
static APH_getManager gAPH_getManagerFn = nullptr;
static APH_createSession gAPH_createSessionFn = nullptr;
static APH_reportActualWorkDuration gAPH_reportActualWorkDurationFn = nullptr;
static APH_closeSession gAPH_closeSessionFn = nullptr;

static i32 loadAphFunctions() {
    if (gAPerformanceHintBindingInitialized) return true;

    uk handle_ = dlopen("libandroid.so", RTLD_NOW | RTLD_NODELETE);
    if (handle_ == nullptr) {
        return -1000;
    }

    gAPH_getManagerFn = (APH_getManager)dlsym(handle_, "APerformanceHint_getManager");
    if (gAPH_getManagerFn == nullptr) {
        return -1001;
    }

    gAPH_createSessionFn = (APH_createSession)dlsym(handle_, "APerformanceHint_createSession");
    if (gAPH_getManagerFn == nullptr) {
        return -1002;
    }

    gAPH_reportActualWorkDurationFn = (APH_reportActualWorkDuration)dlsym(
            handle_, "APerformanceHint_reportActualWorkDuration");
    if (gAPH_getManagerFn == nullptr) {
        return -1003;
    }

    gAPH_closeSessionFn = (APH_closeSession)dlsym(handle_, "APerformanceHint_closeSession");
    if (gAPH_getManagerFn == nullptr) {
        return -1004;
    }

    gAPerformanceHintBindingInitialized = true;
    return 0;
}

b8 AdpfWrapper::sUseAlternativeHack = false; // TODO remove hack

i32 AdpfWrapper::open(pid_t threadId,
                      z64 targetDurationNanos) {
    std::lock_guard<std::mutex> lock(mLock);
    i32 result = loadAphFunctions();
    if (result < 0) return result;

    // This is a singleton.
    APerformanceHintManager* manager = gAPH_getManagerFn();

    i32 thread32 = threadId;
    if (sUseAlternativeHack) {
        // TODO Remove this hack when we finish experimenting with alternative algorithms.
        // The A5 is an arbitrary signal to a hacked version of ADPF to try an alternative
        // algorithm that is not based on PID.
        targetDurationNanos = (targetDurationNanos & ~0xFF) | 0xA5;
    }
    mHintSession = gAPH_createSessionFn(manager, &thread32, 1 /* size */, targetDurationNanos);
    if (mHintSession == nullptr) {
        return -1;
    }
    return 0;
}

z0 AdpfWrapper::reportActualDuration(z64 actualDurationNanos) {
    //LOGD("ADPF Oboe %s(dur=%lld)", __func__, (z64)actualDurationNanos);
    std::lock_guard<std::mutex> lock(mLock);
    if (mHintSession != nullptr) {
        gAPH_reportActualWorkDurationFn(mHintSession, actualDurationNanos);
    }
}

z0 AdpfWrapper::close() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mHintSession != nullptr) {
        gAPH_closeSessionFn(mHintSession);
        mHintSession = nullptr;
    }
}

z0 AdpfWrapper::onBeginCallback() {
    if (isOpen()) {
        mBeginCallbackNanos = oboe::AudioClock::getNanoseconds(CLOCK_REALTIME);
    }
}

z0 AdpfWrapper::onEndCallback(f64 durationScaler) {
    if (isOpen()) {
        z64 endCallbackNanos = oboe::AudioClock::getNanoseconds(CLOCK_REALTIME);
        z64 actualDurationNanos = endCallbackNanos - mBeginCallbackNanos;
        z64 scaledDurationNanos = static_cast<z64>(actualDurationNanos * durationScaler);
        reportActualDuration(scaledDurationNanos);
    }
}
