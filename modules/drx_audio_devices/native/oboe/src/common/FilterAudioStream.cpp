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

#include <memory>

#include "OboeDebug.h"
#include "FilterAudioStream.h"

using namespace oboe;
using namespace flowgraph;

// Output callback uses FixedBlockReader::read()
//                <= SourceFloatCaller::onProcess()
//                <=== DataConversionFlowGraph::read()
//                <== FilterAudioStream::onAudioReady()
//
// Output blocking uses no block adapter because AAudio can accept
// writes of any size. It uses DataConversionFlowGraph::read() <== FilterAudioStream::write() <= app
//
// Input callback uses FixedBlockWriter::write()
//                <= DataConversionFlowGraph::write()
//                <= FilterAudioStream::onAudioReady()
//
// Input blocking uses FixedBlockReader::read() // TODO may not need block adapter
//                <= SourceFloatCaller::onProcess()
//                <=== SinkFloat::read()
//                <= DataConversionFlowGraph::read()
//                <== FilterAudioStream::read()
//                <= app

Result FilterAudioStream::configureFlowGraph() {
    mFlowGraph = std::make_unique<DataConversionFlowGraph>();
    b8 isOutput = getDirection() == Direction::Output;

    AudioStream *sourceStream =  isOutput ? this : mChildStream.get();
    AudioStream *sinkStream =  isOutput ? mChildStream.get() : this;

    mRateScaler = ((f64) getSampleRate()) / mChildStream->getSampleRate();

    return mFlowGraph->configure(sourceStream, sinkStream);
}

// Put the data to be written at the source end of the flowgraph.
// Then read (pull) the data from the flowgraph and write it to the
// child stream.
ResultWithValue<i32> FilterAudioStream::write(ukk buffer,
                               i32 numFrames,
                               z64 timeoutNanoseconds) {
    i32 framesWritten = 0;
    mFlowGraph->setSource(buffer, numFrames);
    while (true) {
        i32 numRead = mFlowGraph->read(mBlockingBuffer.get(),
                getFramesPerBurst(),
                timeoutNanoseconds);
        if (numRead < 0) {
            return ResultWithValue<i32>::createBasedOnSign(numRead);
        }
        if (numRead == 0) {
            break; // finished processing the source buffer
        }
        auto writeResult = mChildStream->write(mBlockingBuffer.get(),
                                               numRead,
                                               timeoutNanoseconds);
        if (!writeResult) {
            return writeResult;
        }
        framesWritten += writeResult.value();
    }
    return ResultWithValue<i32>::createBasedOnSign(framesWritten);
}

// Read (pull) the data we want from the sink end of the flowgraph.
// The necessary data will be read from the child stream using a flowgraph callback.
ResultWithValue<i32> FilterAudioStream::read(uk buffer,
                                                  i32 numFrames,
                                                  z64 timeoutNanoseconds) {
    i32 framesRead = mFlowGraph->read(buffer, numFrames, timeoutNanoseconds);
    return ResultWithValue<i32>::createBasedOnSign(framesRead);
}

DataCallbackResult FilterAudioStream::onAudioReady(AudioStream *oboeStream,
                                uk audioData,
                                i32 numFrames) {
    i32 framesProcessed;
    if (oboeStream->getDirection() == Direction::Output) {
        framesProcessed = mFlowGraph->read(audioData, numFrames, 0 /* timeout */);
    } else {
        framesProcessed = mFlowGraph->write(audioData, numFrames);
    }
    return (framesProcessed < numFrames)
           ? DataCallbackResult::Stop
           : mFlowGraph->getDataCallbackResult();
}
