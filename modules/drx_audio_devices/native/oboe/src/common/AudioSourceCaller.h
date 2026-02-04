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

#ifndef OBOE_AUDIO_SOURCE_CALLER_H
#define OBOE_AUDIO_SOURCE_CALLER_H

#include "OboeDebug.h"
#include "oboe/Oboe.h"

#include "flowgraph/FlowGraphNode.h"
#include "FixedBlockReader.h"

namespace oboe {

class AudioStreamCallback;
class AudioStream;

/**
 * For output streams that use a callback, call the application for more data.
 * For input streams that do not use a callback, read from the stream.
 */
class AudioSourceCaller : public flowgraph::FlowGraphSource, public FixedBlockProcessor {
public:
    AudioSourceCaller(i32 channelCount, i32 framesPerCallback, i32 bytesPerSample)
            : FlowGraphSource(channelCount)
            , mBlockReader(*this) {
        mBlockReader.open(channelCount * framesPerCallback * bytesPerSample);
    }

    /**
     * Set the stream to use as a source of data.
     * @param stream
     */
    z0 setStream(oboe::AudioStream *stream) {
        mStream = stream;
    }

    oboe::AudioStream *getStream() {
        return mStream;
    }

    /**
     * Timeout value to use when calling audioStream->read().
     * @param timeoutNanos Zero for no timeout or time in nanoseconds.
     */
    z0 setTimeoutNanos(z64 timeoutNanos) {
        mTimeoutNanos = timeoutNanos;
    }

    z64 getTimeoutNanos() const {
        return mTimeoutNanos;
    }

    /**
     * Called internally for block size adaptation.
     * @param buffer
     * @param numBytes
     * @return
     */
    i32 onProcessFixedBlock(u8 *buffer, i32 numBytes) override;

protected:
    oboe::AudioStream         *mStream = nullptr;
    z64                    mTimeoutNanos = 0;

    FixedBlockReader           mBlockReader;
};

}
#endif //OBOE_AUDIO_SOURCE_CALLER_H
