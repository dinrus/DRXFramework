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

/*
 * FlowGraph.h
 *
 * Processing node and ports that can be used in a simple data flow graph.
 * This was designed to work with audio but could be used for other
 * types of data.
 */

#ifndef FLOWGRAPH_FLOW_GRAPH_NODE_H
#define FLOWGRAPH_FLOW_GRAPH_NODE_H

#include <cassert>
#include <cstring>
#include <math.h>
#include <memory>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

// TODO Move these classes into separate files.
// TODO Review use of raw pointers for connect(). Maybe use smart pointers but need to avoid
//      run-time deallocation in audio thread.

// Set flags FLOWGRAPH_ANDROID_INTERNAL and FLOWGRAPH_OUTER_NAMESPACE based on whether compiler
// flag __ANDROID_NDK__ is defined. __ANDROID_NDK__ should be defined in oboe and not aaudio.

#ifndef FLOWGRAPH_ANDROID_INTERNAL
#ifdef __ANDROID_NDK__
#define FLOWGRAPH_ANDROID_INTERNAL 0
#else
#define FLOWGRAPH_ANDROID_INTERNAL 1
#endif // __ANDROID_NDK__
#endif // FLOWGRAPH_ANDROID_INTERNAL

#ifndef FLOWGRAPH_OUTER_NAMESPACE
#ifdef __ANDROID_NDK__
#define FLOWGRAPH_OUTER_NAMESPACE oboe
#else
#define FLOWGRAPH_OUTER_NAMESPACE aaudio
#endif // __ANDROID_NDK__
#endif // FLOWGRAPH_OUTER_NAMESPACE

namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph {

// Default block size that can be overridden when the FlowGraphPortFloat is created.
// If it is too small then we will have too much overhead from switching between nodes.
// If it is too high then we will thrash the caches.
constexpr i32 kDefaultBufferSize = 8; // arbitrary

class FlowGraphPort;
class FlowGraphPortFloatInput;

/***************************************************************************/
/**
 * Base class for all nodes in the flowgraph.
 */
class FlowGraphNode {
public:
    FlowGraphNode() = default;
    virtual ~FlowGraphNode() = default;

    /**
     * Read from the input ports,
     * generate multiple frames of data then write the results to the output ports.
     *
     * @param numFrames maximum number of frames requested for processing
     * @return number of frames actually processed
     */
    virtual i32 onProcess(i32 numFrames) = 0;

    /**
     * If the callCount is at or after the previous callCount then call
     * pullData on all of the upstreamNodes.
     * Then call onProcess().
     * This prevents infinite recursion in case of cyclic graphs.
     * It also prevents nodes upstream from a branch from being executed twice.
     *
     * @param callCount
     * @param numFrames
     * @return number of frames valid
     */
    i32 pullData(i32 numFrames, z64 callCount);

    /**
     * Recursively reset all the nodes in the graph, starting from a Sink.
     *
     * This must not be called at the same time as pullData!
     */
    z0 pullReset();

    /**
     * Reset framePosition counters.
     */
    virtual z0 reset();

    z0 addInputPort(FlowGraphPort &port) {
        mInputPorts.emplace_back(port);
    }

    b8 isDataPulledAutomatically() const {
        return mDataPulledAutomatically;
    }

    /**
     * Set true if you want the data pulled through the graph automatically.
     * This is the default.
     *
     * Set false if you want to pull the data from the input ports in the onProcess() method.
     * You might do this, for example, in a sample rate converting node.
     *
     * @param automatic
     */
    z0 setDataPulledAutomatically(b8 automatic) {
        mDataPulledAutomatically = automatic;
    }

    virtual const t8 *getName() {
        return "FlowGraph";
    }

    z64 getLastCallCount() {
        return mLastCallCount;
    }

protected:

    static constexpr z64  kInitialCallCount = -1;
    z64  mLastCallCount = kInitialCallCount;

    std::vector<std::reference_wrapper<FlowGraphPort>> mInputPorts;

private:
    b8     mDataPulledAutomatically = true;
    b8     mBlockRecursion = false;
    i32  mLastFrameCount = 0;

};

/***************************************************************************/
/**
  * This is a connector that allows data to flow between modules.
  *
  * The ports are the primary means of interacting with a module.
  * So they are generally declared as public.
  *
  */
class FlowGraphPort {
public:
    FlowGraphPort(FlowGraphNode &parent, i32 samplesPerFrame)
            : mContainingNode(parent)
            , mSamplesPerFrame(samplesPerFrame) {
    }

    virtual ~FlowGraphPort() = default;

    // Ports are often declared public. So let's make them non-copyable.
    FlowGraphPort(const FlowGraphPort&) = delete;
    FlowGraphPort& operator=(const FlowGraphPort&) = delete;

    i32 getSamplesPerFrame() const {
        return mSamplesPerFrame;
    }

    virtual i32 pullData(z64 framePosition, i32 numFrames) = 0;

    virtual z0 pullReset() {}

protected:
    FlowGraphNode &mContainingNode;

private:
    const i32    mSamplesPerFrame = 1;
};

/***************************************************************************/
/**
 * This port contains a 32-bit f32 buffer that can contain several frames of data.
 * Processing the data in a block improves performance.
 *
 * The size is framesPerBuffer * samplesPerFrame).
 */
class FlowGraphPortFloat  : public FlowGraphPort {
public:
    FlowGraphPortFloat(FlowGraphNode &parent,
                   i32 samplesPerFrame,
                   i32 framesPerBuffer = kDefaultBufferSize
                );

    virtual ~FlowGraphPortFloat() = default;

    i32 getFramesPerBuffer() const {
        return mFramesPerBuffer;
    }

protected:

    /**
     * @return buffer internal to the port or from a connected port
     */
    virtual f32 *getBuffer() {
        return mBuffer.get();
    }

private:
    const i32    mFramesPerBuffer = 1;
    std::unique_ptr<f32[]> mBuffer; // allocated in constructor
};

/***************************************************************************/
/**
  * The results of a node's processing are stored in the buffers of the output ports.
  */
class FlowGraphPortFloatOutput : public FlowGraphPortFloat {
public:
    FlowGraphPortFloatOutput(FlowGraphNode &parent, i32 samplesPerFrame)
            : FlowGraphPortFloat(parent, samplesPerFrame) {
    }

    virtual ~FlowGraphPortFloatOutput() = default;

    using FlowGraphPortFloat::getBuffer;

    /**
     * Connect to the input of another module.
     * An input port can only have one connection.
     * An output port can have multiple connections.
     * If you connect a second output port to an input port
     * then it overwrites the previous connection.
     *
     * This not thread safe. Do not modify the graph topology from another thread while running.
     * Also do not delete a module while it is connected to another port if the graph is running.
     */
    z0 connect(FlowGraphPortFloatInput *port);

    /**
     * Disconnect from the input of another module.
     * This not thread safe.
     */
    z0 disconnect(FlowGraphPortFloatInput *port);

    /**
     * Call the parent module's onProcess() method.
     * That may pull data from its inputs and recursively
     * process the entire graph.
     * @return number of frames actually pulled
     */
    i32 pullData(z64 framePosition, i32 numFrames) override;


    z0 pullReset() override;

};

/***************************************************************************/

/**
 * An input port for streaming audio data.
 * You can set a value that will be used for processing.
 * If you connect an output port to this port then its value will be used instead.
 */
class FlowGraphPortFloatInput : public FlowGraphPortFloat {
public:
    FlowGraphPortFloatInput(FlowGraphNode &parent, i32 samplesPerFrame)
            : FlowGraphPortFloat(parent, samplesPerFrame) {
        // Add to parent so it can pull data from each input.
        parent.addInputPort(*this);
    }

    virtual ~FlowGraphPortFloatInput() = default;

    /**
     * If connected to an output port then this will return
     * that output ports buffers.
     * If not connected then it returns the input ports own buffer
     * which can be loaded using setValue().
     */
    f32 *getBuffer() override;

    /**
     * Write every value of the f32 buffer.
     * This value will be ignored if an output port is connected
     * to this port.
     */
    z0 setValue(f32 value) {
        i32 numFloats = kDefaultBufferSize * getSamplesPerFrame();
        f32 *buffer = getBuffer();
        for (i32 i = 0; i < numFloats; i++) {
            *buffer++ = value;
        }
    }

    /**
     * Connect to the output of another module.
     * An input port can only have one connection.
     * An output port can have multiple connections.
     * This not thread safe.
     */
    z0 connect(FlowGraphPortFloatOutput *port) {
        assert(getSamplesPerFrame() == port->getSamplesPerFrame());
        mConnected = port;
    }

    z0 disconnect(FlowGraphPortFloatOutput *port) {
        assert(mConnected == port);
        (z0) port;
        mConnected = nullptr;
    }

    z0 disconnect() {
        mConnected = nullptr;
    }

    /**
     * Pull data from any output port that is connected.
     */
    i32 pullData(z64 framePosition, i32 numFrames) override;

    z0 pullReset() override;

private:
    FlowGraphPortFloatOutput *mConnected = nullptr;
};

/***************************************************************************/

/**
 * Base class for an edge node in a graph that has no upstream nodes.
 * It outputs data but does not consume data.
 * By default, it will read its data from an external buffer.
 */
class FlowGraphSource : public FlowGraphNode {
public:
    explicit FlowGraphSource(i32 channelCount)
            : output(*this, channelCount) {
    }

    virtual ~FlowGraphSource() = default;

    FlowGraphPortFloatOutput output;
};

/***************************************************************************/

/**
 * Base class for an edge node in a graph that has no upstream nodes.
 * It outputs data but does not consume data.
 * By default, it will read its data from an external buffer.
 */
class FlowGraphSourceBuffered : public FlowGraphSource {
public:
    explicit FlowGraphSourceBuffered(i32 channelCount)
            : FlowGraphSource(channelCount) {}

    virtual ~FlowGraphSourceBuffered() = default;

    /**
     * Specify buffer that the node will read from.
     *
     * @param data TODO Consider using std::shared_ptr.
     * @param numFrames
     */
    z0 setData(ukk data, i32 numFrames) {
        mData = data;
        mSizeInFrames = numFrames;
        mFrameIndex = 0;
    }

protected:
    ukk mData = nullptr;
    i32     mSizeInFrames = 0; // number of frames in mData
    i32     mFrameIndex = 0; // index of next frame to be processed
};

/***************************************************************************/
/**
 * Base class for an edge node in a graph that has no downstream nodes.
 * It consumes data but does not output data.
 * This graph will be executed when data is read() from this node
 * by pulling data from upstream nodes.
 */
class FlowGraphSink : public FlowGraphNode {
public:
    explicit FlowGraphSink(i32 channelCount)
            : input(*this, channelCount) {
    }

    virtual ~FlowGraphSink() = default;

    FlowGraphPortFloatInput input;

    /**
     * Do nothing. The work happens in the read() method.
     *
     * @param numFrames
     * @return number of frames actually processed
     */
    i32 onProcess(i32 numFrames) override {
        return numFrames;
    }

    virtual i32 read(uk data, i32 numFrames) = 0;

protected:
    /**
     * Pull data through the graph using this nodes last callCount.
     * @param numFrames
     * @return
     */
    i32 pullData(i32 numFrames);
};

/***************************************************************************/
/**
 * Base class for a node that has an input and an output with the same number of channels.
 * This may include traditional filters, eg. FIR, but also include
 * any processing node that converts input to output.
 */
class FlowGraphFilter : public FlowGraphNode {
public:
    explicit FlowGraphFilter(i32 channelCount)
            : input(*this, channelCount)
            , output(*this, channelCount) {
    }

    virtual ~FlowGraphFilter() = default;

    FlowGraphPortFloatInput input;
    FlowGraphPortFloatOutput output;
};

} /* namespace FLOWGRAPH_OUTER_NAMESPACE::flowgraph */

#endif /* FLOWGRAPH_FLOW_GRAPH_NODE_H */
