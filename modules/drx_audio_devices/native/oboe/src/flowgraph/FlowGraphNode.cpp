/*
 * Copyright 2015 The Android Open Source Project
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

#include "stdio.h"
#include <algorithm>
#include <sys/types.h>
#include "FlowGraphNode.h"

using namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph;

/***************************************************************************/
i32 FlowGraphNode::pullData(i32 numFrames, z64 callCount) {
    i32 frameCount = numFrames;
    // Prevent recursion and multiple execution of nodes.
    if (callCount > mLastCallCount) {
        mLastCallCount = callCount;
        if (mDataPulledAutomatically) {
            // Pull from all the upstream nodes.
            for (auto &port : mInputPorts) {
                // TODO fix bug of leaving unused data in some ports if using multiple AudioSource
                frameCount = port.get().pullData(callCount, frameCount);
            }
        }
        if (frameCount > 0) {
            frameCount = onProcess(frameCount);
        }
        mLastFrameCount = frameCount;
    } else {
        frameCount = mLastFrameCount;
    }
    return frameCount;
}

z0 FlowGraphNode::pullReset() {
    if (!mBlockRecursion) {
        mBlockRecursion = true; // for cyclic graphs
        // Pull reset from all the upstream nodes.
        for (auto &port : mInputPorts) {
            port.get().pullReset();
        }
        mBlockRecursion = false;
        reset();
    }
}

z0 FlowGraphNode::reset() {
    mLastFrameCount = 0;
    mLastCallCount = kInitialCallCount;
}

/***************************************************************************/
FlowGraphPortFloat::FlowGraphPortFloat(FlowGraphNode &parent,
                               i32 samplesPerFrame,
                               i32 framesPerBuffer)
        : FlowGraphPort(parent, samplesPerFrame)
        , mFramesPerBuffer(framesPerBuffer)
        , mBuffer(nullptr) {
    size_t numFloats = static_cast<size_t>(framesPerBuffer) * getSamplesPerFrame();
    mBuffer = std::make_unique<f32[]>(numFloats);
}

/***************************************************************************/
i32 FlowGraphPortFloatOutput::pullData(z64 callCount, i32 numFrames) {
    numFrames = std::min(getFramesPerBuffer(), numFrames);
    return mContainingNode.pullData(numFrames, callCount);
}

z0 FlowGraphPortFloatOutput::pullReset() {
    mContainingNode.pullReset();
}

// These need to be in the .cpp file because of forward cross references.
z0 FlowGraphPortFloatOutput::connect(FlowGraphPortFloatInput *port) {
    port->connect(this);
}

z0 FlowGraphPortFloatOutput::disconnect(FlowGraphPortFloatInput *port) {
    port->disconnect(this);
}

/***************************************************************************/
i32 FlowGraphPortFloatInput::pullData(z64 callCount, i32 numFrames) {
    return (mConnected == nullptr)
            ? std::min(getFramesPerBuffer(), numFrames)
            : mConnected->pullData(callCount, numFrames);
}
z0 FlowGraphPortFloatInput::pullReset() {
    if (mConnected != nullptr) mConnected->pullReset();
}

f32 *FlowGraphPortFloatInput::getBuffer() {
    if (mConnected == nullptr) {
        return FlowGraphPortFloat::getBuffer(); // loaded using setValue()
    } else {
        return mConnected->getBuffer();
    }
}

i32 FlowGraphSink::pullData(i32 numFrames) {
    return FlowGraphNode::pullData(numFrames, getLastCallCount() + 1);
}
