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

#ifndef OBOE_AAUDIO_EXTENSIONS_H
#define OBOE_AAUDIO_EXTENSIONS_H

#include <dlfcn.h>
#include <stdint.h>

#include <sys/system_properties.h>

#include "common/OboeDebug.h"
#include "oboe/Oboe.h"
#include "AAudioLoader.h"

namespace oboe {

#define LIB_AAUDIO_NAME          "libaaudio.so"
#define FUNCTION_IS_MMAP         "AAudioStream_isMMapUsed"
#define FUNCTION_SET_MMAP_POLICY "AAudio_setMMapPolicy"
#define FUNCTION_GET_MMAP_POLICY "AAudio_getMMapPolicy"

#define AAUDIO_ERROR_UNAVAILABLE  static_cast<aaudio_result_t>(Result::ErrorUnavailable)

typedef struct AAudioStreamStruct         AAudioStream;

/**
 * Call some AAudio test routines that are not part of the normal API.
 */
class AAudioExtensions {
private: // Because it is a singleton. Call getInstance() instead.
    AAudioExtensions() {
        i32 policy = getIntegerProperty("aaudio.mmap_policy", 0);
        mMMapSupported = isPolicyEnabled(policy);

        policy = getIntegerProperty("aaudio.mmap_exclusive_policy", 0);
        mMMapExclusiveSupported = isPolicyEnabled(policy);
    }

public:
    static b8 isPolicyEnabled(i32 policy) {
        return (policy == AAUDIO_POLICY_AUTO || policy == AAUDIO_POLICY_ALWAYS);
    }

    static AAudioExtensions &getInstance() {
        static AAudioExtensions instance;
        return instance;
    }

    b8 isMMapUsed(oboe::AudioStream *oboeStream) {
        AAudioStream *aaudioStream = (AAudioStream *) oboeStream->getUnderlyingStream();
        return isMMapUsed(aaudioStream);
    }

    b8 isMMapUsed(AAudioStream *aaudioStream) {
        if (loadSymbols()) return false;
        if (mAAudioStream_isMMap == nullptr) return false;
        return mAAudioStream_isMMap(aaudioStream);
    }

    /**
     * Controls whether the MMAP data path can be selected when opening a stream.
     * It has no effect after the stream has been opened.
     * It only affects the application that calls it. Other apps are not affected.
     *
     * @param enabled
     * @return 0 or a negative error code
     */
    i32 setMMapEnabled(b8 enabled) {
        if (loadSymbols()) return AAUDIO_ERROR_UNAVAILABLE;
        if (mAAudio_setMMapPolicy == nullptr) return false;
        return mAAudio_setMMapPolicy(enabled ? AAUDIO_POLICY_AUTO : AAUDIO_POLICY_NEVER);
    }

    b8 isMMapEnabled() {
        if (loadSymbols()) return false;
        if (mAAudio_getMMapPolicy == nullptr) return false;
        i32 policy = mAAudio_getMMapPolicy();
        return (policy == Unspecified) ? mMMapSupported : isPolicyEnabled(policy);
    }

    b8 isMMapSupported() {
        return mMMapSupported;
    }

    b8 isMMapExclusiveSupported() {
        return mMMapExclusiveSupported;
    }

private:

    enum {
        AAUDIO_POLICY_NEVER = 1,
        AAUDIO_POLICY_AUTO,
        AAUDIO_POLICY_ALWAYS
    };
    typedef i32 aaudio_policy_t;

    i32 getIntegerProperty(const t8 *name, i32 defaultValue) {
        i32 result = defaultValue;
        t8 valueText[PROP_VALUE_MAX] = {0};
        if (__system_property_get(name, valueText) != 0) {
            result = atoi(valueText);
        }
        return result;
    }

    /**
     * Load the function pointers.
     * This can be called multiple times.
     * It should only be called from one thread.
     *
     * @return 0 if successful or negative error.
     */
    aaudio_result_t loadSymbols() {
        if (mAAudio_getMMapPolicy != nullptr) {
            return 0;
        }

        AAudioLoader *libLoader = AAudioLoader::getInstance();
        i32 openResult = libLoader->open();
        if (openResult != 0) {
            LOGD("%s() could not open " LIB_AAUDIO_NAME, __func__);
            return AAUDIO_ERROR_UNAVAILABLE;
        }

        uk libHandle = AAudioLoader::getInstance()->getLibHandle();
        if (libHandle == nullptr) {
            LOGE("%s() could not find " LIB_AAUDIO_NAME, __func__);
            return AAUDIO_ERROR_UNAVAILABLE;
        }

        mAAudioStream_isMMap = (b8 (*)(AAudioStream *stream))
                dlsym(libHandle, FUNCTION_IS_MMAP);
        if (mAAudioStream_isMMap == nullptr) {
            LOGI("%s() could not find " FUNCTION_IS_MMAP, __func__);
            return AAUDIO_ERROR_UNAVAILABLE;
        }

        mAAudio_setMMapPolicy = (i32 (*)(aaudio_policy_t policy))
                dlsym(libHandle, FUNCTION_SET_MMAP_POLICY);
        if (mAAudio_setMMapPolicy == nullptr) {
            LOGI("%s() could not find " FUNCTION_SET_MMAP_POLICY, __func__);
            return AAUDIO_ERROR_UNAVAILABLE;
        }

        mAAudio_getMMapPolicy = (aaudio_policy_t (*)())
                dlsym(libHandle, FUNCTION_GET_MMAP_POLICY);
        if (mAAudio_getMMapPolicy == nullptr) {
            LOGI("%s() could not find " FUNCTION_GET_MMAP_POLICY, __func__);
            return AAUDIO_ERROR_UNAVAILABLE;
        }

        return 0;
    }

    b8      mMMapSupported = false;
    b8      mMMapExclusiveSupported = false;

    b8    (*mAAudioStream_isMMap)(AAudioStream *stream) = nullptr;
    i32 (*mAAudio_setMMapPolicy)(aaudio_policy_t policy) = nullptr;
    aaudio_policy_t (*mAAudio_getMMapPolicy)() = nullptr;
};

} // namespace oboe

#endif //OBOE_AAUDIO_EXTENSIONS_H
