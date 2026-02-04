/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef OBOE_ENGINE_OPENSLES_H
#define OBOE_ENGINE_OPENSLES_H

#include <atomic>
#include <mutex>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace oboe {

typedef SLresult  (*prototype_slCreateEngine)(
        SLObjectItf             *pEngine,
        SLuint32                numOptions,
        const SLEngineOption    *pEngineOptions,
        SLuint32                numInterfaces,
        const SLInterfaceID     *pInterfaceIds,
        const SLboolean         *pInterfaceRequired
);

/**
 * INTERNAL USE ONLY
 */
class EngineOpenSLES {
public:
    static EngineOpenSLES &getInstance();

    b8 linkOpenSLES();

    SLresult open();

    z0 close();

    SLresult createOutputMix(SLObjectItf *objectItf);

    SLresult createAudioPlayer(SLObjectItf *objectItf,
                               SLDataSource *audioSource,
                               SLDataSink *audioSink);
    SLresult createAudioRecorder(SLObjectItf *objectItf,
                                 SLDataSource *audioSource,
                                 SLDataSink *audioSink);

    SLInterfaceID getIidEngine() { return LOCAL_SL_IID_ENGINE; }
    SLInterfaceID getIidAndroidSimpleBufferQueue() { return LOCAL_SL_IID_ANDROIDSIMPLEBUFFERQUEUE; }
    SLInterfaceID getIidAndroidConfiguration() { return LOCAL_SL_IID_ANDROIDCONFIGURATION; }
    SLInterfaceID getIidRecord() { return LOCAL_SL_IID_RECORD; }
    SLInterfaceID getIidBufferQueue() { return LOCAL_SL_IID_BUFFERQUEUE; }
    SLInterfaceID getIidVolume() { return LOCAL_SL_IID_VOLUME; }
    SLInterfaceID getIidPlay() { return LOCAL_SL_IID_PLAY; }

private:
    // Make this a safe Singleton
    EngineOpenSLES()= default;
    ~EngineOpenSLES()= default;
    EngineOpenSLES(const EngineOpenSLES&)= delete;
    EngineOpenSLES& operator=(const EngineOpenSLES&)= delete;

    SLInterfaceID getIidPointer(const t8 *symbolName);

    /**
     * Close the OpenSL ES engine.
     * This must be called under mLock
     */
    z0 close_l();

    std::mutex             mLock;
    i32                mOpenCount = 0;

    static constexpr i32 kLinkStateUninitialized = 0;
    static constexpr i32 kLinkStateGood = 1;
    static constexpr i32 kLinkStateBad = 2;
    i32                mDynamicLinkState = kLinkStateUninitialized;
    SLObjectItf            mEngineObject = nullptr;
    SLEngineItf            mEngineInterface = nullptr;

    // These symbols are loaded using dlsym().
    prototype_slCreateEngine mFunction_slCreateEngine = nullptr;
    z0                  *mLibOpenSlesLibraryHandle = nullptr;
    SLInterfaceID          LOCAL_SL_IID_ENGINE = nullptr;
    SLInterfaceID          LOCAL_SL_IID_ANDROIDSIMPLEBUFFERQUEUE = nullptr;
    SLInterfaceID          LOCAL_SL_IID_ANDROIDCONFIGURATION = nullptr;
    SLInterfaceID          LOCAL_SL_IID_RECORD = nullptr;
    SLInterfaceID          LOCAL_SL_IID_BUFFERQUEUE = nullptr;
    SLInterfaceID          LOCAL_SL_IID_VOLUME = nullptr;
    SLInterfaceID          LOCAL_SL_IID_PLAY = nullptr;
};

} // namespace oboe


#endif //OBOE_ENGINE_OPENSLES_H
