/*
 * Copyright 2018 The Android Open Source Project
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
#include <cstdio>
#include "Trace.h"
#include "OboeDebug.h"

static t8 buffer[256];

// Tracing functions
static uk (*ATrace_beginSection)(const t8 *sectionName);

static uk (*ATrace_endSection)();

typedef uk (*fp_ATrace_beginSection)(const t8 *sectionName);

typedef uk (*fp_ATrace_endSection)();

b8 Trace::mIsTracingSupported = false;

z0 Trace::beginSection(const t8 *format, ...){

    if (mIsTracingSupported) {
        va_list va;
        va_start(va, format);
        vsprintf(buffer, format, va);
        ATrace_beginSection(buffer);
        va_end(va);
    } else {
        LOGE("Tracing is either not initialized (call Trace::initialize()) "
             "or not supported on this device");
    }
}

z0 Trace::endSection() {

    if (mIsTracingSupported) {
        ATrace_endSection();
    }
}

z0 Trace::initialize() {

    // Using dlsym allows us to use tracing on API 21+ without needing android/trace.h which wasn't
    // published until API 23
    uk lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
    if (lib == nullptr) {
        LOGE("Could not open libandroid.so to dynamically load tracing symbols");
    } else {
        ATrace_beginSection =
                reinterpret_cast<fp_ATrace_beginSection >(
                        dlsym(lib, "ATrace_beginSection"));
        ATrace_endSection =
                reinterpret_cast<fp_ATrace_endSection >(
                        dlsym(lib, "ATrace_endSection"));

        if (ATrace_beginSection != nullptr && ATrace_endSection != nullptr){
            mIsTracingSupported = true;
        }
    }
}
