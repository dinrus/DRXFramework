/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx::dsp
{

template <typename Element>
class Queue
{
public:
    explicit Queue (i32 size)
        : fifo (size), storage (static_cast<size_t> (size)) {}

    b8 push (Element& element) noexcept
    {
        if (fifo.getFreeSpace() == 0)
            return false;

        const auto writer = fifo.write (1);

        if (writer.blockSize1 != 0)
            storage[static_cast<size_t> (writer.startIndex1)] = std::move (element);
        else if (writer.blockSize2 != 0)
            storage[static_cast<size_t> (writer.startIndex2)] = std::move (element);

        return true;
    }

    template <typename Fn>
    z0 pop (Fn&& fn) { popN (1, std::forward<Fn> (fn)); }

    template <typename Fn>
    z0 popAll (Fn&& fn) { popN (fifo.getNumReady(), std::forward<Fn> (fn)); }

    b8 hasPendingMessages() const noexcept { return fifo.getNumReady() > 0; }

private:
    template <typename Fn>
    z0 popN (i32 n, Fn&& fn)
    {
        fifo.read (n).forEach ([&] (i32 index)
                               {
                                   fn (storage[static_cast<size_t> (index)]);
                               });
    }

    AbstractFifo fifo;
    std::vector<Element> storage;
};

class BackgroundMessageQueue : private Thread
{
public:
    explicit BackgroundMessageQueue (i32 entries)
        : Thread (SystemStats::getDRXVersion() + ": Convolution background loader"), queue (entries)
    {}

    using IncomingCommand = FixedSizeFunction<400, z0()>;

    // Push functions here, and they'll be called later on a background thread.
    // This function is wait-free.
    // This function is only safe to call from a single thread at a time.
    b8 push (IncomingCommand& command) { return queue.push (command); }

    z0 popAll()
    {
        const ScopedLock lock (popMutex);
        queue.popAll ([] (IncomingCommand& command) { command(); command = nullptr; });
    }

    using Thread::startThread;
    using Thread::stopThread;

private:
    z0 run() override
    {
        while (! threadShouldExit())
        {
            const auto tryPop = [&]
            {
                const ScopedLock lock (popMutex);

                if (! queue.hasPendingMessages())
                    return false;

                queue.pop ([] (IncomingCommand& command) { command(); command = nullptr;});
                return true;
            };

            if (! tryPop())
                sleep (10);
        }
    }

    CriticalSection popMutex;
    Queue<IncomingCommand> queue;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BackgroundMessageQueue)
};

struct ConvolutionMessageQueue::Impl  : public BackgroundMessageQueue
{
    using BackgroundMessageQueue::BackgroundMessageQueue;
};

ConvolutionMessageQueue::ConvolutionMessageQueue()
    : ConvolutionMessageQueue (1000)
{}

ConvolutionMessageQueue::ConvolutionMessageQueue (i32 entries)
    : pimpl (std::make_unique<Impl> (entries))
{
    pimpl->startThread();
}

ConvolutionMessageQueue::~ConvolutionMessageQueue() noexcept
{
    pimpl->stopThread (-1);
}

ConvolutionMessageQueue::ConvolutionMessageQueue (ConvolutionMessageQueue&&) noexcept = default;
ConvolutionMessageQueue& ConvolutionMessageQueue::operator= (ConvolutionMessageQueue&&) noexcept = default;

//==============================================================================
struct ConvolutionEngine
{
    ConvolutionEngine (const f32* samples,
                       size_t numSamples,
                       size_t maxBlockSize)
        : blockSize ((size_t) nextPowerOfTwo ((i32) maxBlockSize)),
          fftSize (blockSize > 128 ? 2 * blockSize : 4 * blockSize),
          fftObject (std::make_unique<FFT> (roundToInt (std::log2 (fftSize)))),
          numSegments (numSamples / (fftSize - blockSize) + 1u),
          numInputSegments ((blockSize > 128 ? numSegments : 3 * numSegments)),
          bufferInput      (1, static_cast<i32> (fftSize)),
          bufferOutput     (1, static_cast<i32> (fftSize * 2)),
          bufferTempOutput (1, static_cast<i32> (fftSize * 2)),
          bufferOverlap    (1, static_cast<i32> (fftSize))
    {
        bufferOutput.clear();

        auto updateSegmentsIfNecessary = [this] (size_t numSegmentsToUpdate,
                                                 std::vector<AudioBuffer<f32>>& segments)
        {
            if (numSegmentsToUpdate == 0
                || numSegmentsToUpdate != (size_t) segments.size()
                || (size_t) segments[0].getNumSamples() != fftSize * 2)
            {
                segments.clear();

                for (size_t i = 0; i < numSegmentsToUpdate; ++i)
                    segments.push_back ({ 1, static_cast<i32> (fftSize * 2) });
            }
        };

        updateSegmentsIfNecessary (numInputSegments, buffersInputSegments);
        updateSegmentsIfNecessary (numSegments,      buffersImpulseSegments);

        auto FFTTempObject = std::make_unique<FFT> (roundToInt (std::log2 (fftSize)));
        size_t currentPtr = 0;

        for (auto& buf : buffersImpulseSegments)
        {
            buf.clear();

            auto* impulseResponse = buf.getWritePointer (0);

            if (&buf == &buffersImpulseSegments.front())
                impulseResponse[0] = 1.0f;

            FloatVectorOperations::copy (impulseResponse,
                                         samples + currentPtr,
                                         static_cast<i32> (jmin (fftSize - blockSize, numSamples - currentPtr)));

            FFTTempObject->performRealOnlyForwardTransform (impulseResponse);
            prepareForConvolution (impulseResponse);

            currentPtr += (fftSize - blockSize);
        }

        reset();
    }

    z0 reset()
    {
        bufferInput.clear();
        bufferOverlap.clear();
        bufferTempOutput.clear();
        bufferOutput.clear();

        for (auto& buf : buffersInputSegments)
            buf.clear();

        currentSegment = 0;
        inputDataPos = 0;
    }

    z0 processSamples (const f32* input, f32* output, size_t numSamples)
    {
        // Overlap-add, zero latency convolution algorithm with uniform partitioning
        size_t numSamplesProcessed = 0;

        auto indexStep = numInputSegments / numSegments;

        auto* inputData      = bufferInput.getWritePointer (0);
        auto* outputTempData = bufferTempOutput.getWritePointer (0);
        auto* outputData     = bufferOutput.getWritePointer (0);
        auto* overlapData    = bufferOverlap.getWritePointer (0);

        while (numSamplesProcessed < numSamples)
        {
            const b8 inputDataWasEmpty = (inputDataPos == 0);
            auto numSamplesToProcess = jmin (numSamples - numSamplesProcessed, blockSize - inputDataPos);

            FloatVectorOperations::copy (inputData + inputDataPos, input + numSamplesProcessed, static_cast<i32> (numSamplesToProcess));

            auto* inputSegmentData = buffersInputSegments[currentSegment].getWritePointer (0);
            FloatVectorOperations::copy (inputSegmentData, inputData, static_cast<i32> (fftSize));

            fftObject->performRealOnlyForwardTransform (inputSegmentData);
            prepareForConvolution (inputSegmentData);

            // Complex multiplication
            if (inputDataWasEmpty)
            {
                FloatVectorOperations::fill (outputTempData, 0, static_cast<i32> (fftSize + 1));

                auto index = currentSegment;

                for (size_t i = 1; i < numSegments; ++i)
                {
                    index += indexStep;

                    if (index >= numInputSegments)
                        index -= numInputSegments;

                    convolutionProcessingAndAccumulate (buffersInputSegments[index].getWritePointer (0),
                                                        buffersImpulseSegments[i].getWritePointer (0),
                                                        outputTempData);
                }
            }

            FloatVectorOperations::copy (outputData, outputTempData, static_cast<i32> (fftSize + 1));

            convolutionProcessingAndAccumulate (inputSegmentData,
                                                buffersImpulseSegments.front().getWritePointer (0),
                                                outputData);

            updateSymmetricFrequencyDomainData (outputData);
            fftObject->performRealOnlyInverseTransform (outputData);

            // Add overlap
            FloatVectorOperations::add (&output[numSamplesProcessed], &outputData[inputDataPos], &overlapData[inputDataPos], (i32) numSamplesToProcess);

            // Input buffer full => Next block
            inputDataPos += numSamplesToProcess;

            if (inputDataPos == blockSize)
            {
                // Input buffer is empty again now
                FloatVectorOperations::fill (inputData, 0.0f, static_cast<i32> (fftSize));

                inputDataPos = 0;

                // Extra step for segSize > blockSize
                FloatVectorOperations::add (&(outputData[blockSize]), &(overlapData[blockSize]), static_cast<i32> (fftSize - 2 * blockSize));

                // Save the overlap
                FloatVectorOperations::copy (overlapData, &(outputData[blockSize]), static_cast<i32> (fftSize - blockSize));

                currentSegment = (currentSegment > 0) ? (currentSegment - 1) : (numInputSegments - 1);
            }

            numSamplesProcessed += numSamplesToProcess;
        }
    }

    z0 processSamplesWithAddedLatency (const f32* input, f32* output, size_t numSamples)
    {
        // Overlap-add, zero latency convolution algorithm with uniform partitioning
        size_t numSamplesProcessed = 0;

        auto indexStep = numInputSegments / numSegments;

        auto* inputData      = bufferInput.getWritePointer (0);
        auto* outputTempData = bufferTempOutput.getWritePointer (0);
        auto* outputData     = bufferOutput.getWritePointer (0);
        auto* overlapData    = bufferOverlap.getWritePointer (0);

        while (numSamplesProcessed < numSamples)
        {
            auto numSamplesToProcess = jmin (numSamples - numSamplesProcessed, blockSize - inputDataPos);

            FloatVectorOperations::copy (inputData + inputDataPos, input + numSamplesProcessed, static_cast<i32> (numSamplesToProcess));

            FloatVectorOperations::copy (output + numSamplesProcessed, outputData + inputDataPos, static_cast<i32> (numSamplesToProcess));

            numSamplesProcessed += numSamplesToProcess;
            inputDataPos += numSamplesToProcess;

            // processing itself when needed (with latency)
            if (inputDataPos == blockSize)
            {
                // Copy input data in input segment
                auto* inputSegmentData = buffersInputSegments[currentSegment].getWritePointer (0);
                FloatVectorOperations::copy (inputSegmentData, inputData, static_cast<i32> (fftSize));

                fftObject->performRealOnlyForwardTransform (inputSegmentData);
                prepareForConvolution (inputSegmentData);

                // Complex multiplication
                FloatVectorOperations::fill (outputTempData, 0, static_cast<i32> (fftSize + 1));

                auto index = currentSegment;

                for (size_t i = 1; i < numSegments; ++i)
                {
                    index += indexStep;

                    if (index >= numInputSegments)
                        index -= numInputSegments;

                    convolutionProcessingAndAccumulate (buffersInputSegments[index].getWritePointer (0),
                                                        buffersImpulseSegments[i].getWritePointer (0),
                                                        outputTempData);
                }

                FloatVectorOperations::copy (outputData, outputTempData, static_cast<i32> (fftSize + 1));

                convolutionProcessingAndAccumulate (inputSegmentData,
                                                    buffersImpulseSegments.front().getWritePointer (0),
                                                    outputData);

                updateSymmetricFrequencyDomainData (outputData);
                fftObject->performRealOnlyInverseTransform (outputData);

                // Add overlap
                FloatVectorOperations::add (outputData, overlapData, static_cast<i32> (blockSize));

                // Input buffer is empty again now
                FloatVectorOperations::fill (inputData, 0.0f, static_cast<i32> (fftSize));

                // Extra step for segSize > blockSize
                FloatVectorOperations::add (&(outputData[blockSize]), &(overlapData[blockSize]), static_cast<i32> (fftSize - 2 * blockSize));

                // Save the overlap
                FloatVectorOperations::copy (overlapData, &(outputData[blockSize]), static_cast<i32> (fftSize - blockSize));

                currentSegment = (currentSegment > 0) ? (currentSegment - 1) : (numInputSegments - 1);

                inputDataPos = 0;
            }
        }
    }

    // After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls.
    z0 prepareForConvolution (f32 *samples) noexcept
    {
        auto FFTSizeDiv2 = fftSize / 2;

        for (size_t i = 0; i < FFTSizeDiv2; i++)
            samples[i] = samples[i << 1];

        samples[FFTSizeDiv2] = 0;

        for (size_t i = 1; i < FFTSizeDiv2; i++)
            samples[i + FFTSizeDiv2] = -samples[((fftSize - i) << 1) + 1];
    }

    // Does the convolution operation itself only on half of the frequency domain samples.
    z0 convolutionProcessingAndAccumulate (const f32 *input, const f32 *impulse, f32 *output)
    {
        auto FFTSizeDiv2 = fftSize / 2;

        FloatVectorOperations::addWithMultiply      (output, input, impulse, static_cast<i32> (FFTSizeDiv2));
        FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<i32> (FFTSizeDiv2));

        FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<i32> (FFTSizeDiv2));
        FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<i32> (FFTSizeDiv2));

        output[fftSize] += input[fftSize] * impulse[fftSize];
    }

    // Undoes the re-organization of samples from the function prepareForConvolution.
    // Then takes the conjugate of the frequency domain first half of samples to fill the
    // second half, so that the inverse transform will return real samples in the time domain.
    z0 updateSymmetricFrequencyDomainData (f32* samples) noexcept
    {
        auto FFTSizeDiv2 = fftSize / 2;

        for (size_t i = 1; i < FFTSizeDiv2; i++)
        {
            samples[(fftSize - i) << 1] = samples[i];
            samples[((fftSize - i) << 1) + 1] = -samples[FFTSizeDiv2 + i];
        }

        samples[1] = 0.f;

        for (size_t i = 1; i < FFTSizeDiv2; i++)
        {
            samples[i << 1] = samples[(fftSize - i) << 1];
            samples[(i << 1) + 1] = -samples[((fftSize - i) << 1) + 1];
        }
    }

    //==============================================================================
    const size_t blockSize;
    const size_t fftSize;
    const std::unique_ptr<FFT> fftObject;
    const size_t numSegments;
    const size_t numInputSegments;
    size_t currentSegment = 0, inputDataPos = 0;

    AudioBuffer<f32> bufferInput, bufferOutput, bufferTempOutput, bufferOverlap;
    std::vector<AudioBuffer<f32>> buffersInputSegments, buffersImpulseSegments;
};

//==============================================================================
class MultichannelEngine
{
public:
    MultichannelEngine (const AudioBuffer<f32>& buf,
                        i32 maxBlockSize,
                        i32 maxBufferSize,
                        Convolution::NonUniform headSizeIn,
                        b8 isZeroDelayIn)
        : tailBuffer (1, maxBlockSize),
          latency (isZeroDelayIn ? 0 : maxBufferSize),
          irSize (buf.getNumSamples()),
          blockSize (maxBlockSize),
          isZeroDelay (isZeroDelayIn)
    {
        constexpr auto numChannels = 2;

        const auto makeEngine = [&] (i32 channel, i32 offset, i32 length, u32 thisBlockSize)
        {
            return std::make_unique<ConvolutionEngine> (buf.getReadPointer (jmin (buf.getNumChannels() - 1, channel), offset),
                                                        length,
                                                        static_cast<size_t> (thisBlockSize));
        };

        if (headSizeIn.headSizeInSamples == 0)
        {
            for (i32 i = 0; i < numChannels; ++i)
                head.emplace_back (makeEngine (i, 0, buf.getNumSamples(), static_cast<u32> (maxBufferSize)));
        }
        else
        {
            const auto size = jmin (buf.getNumSamples(), headSizeIn.headSizeInSamples);

            for (i32 i = 0; i < numChannels; ++i)
                head.emplace_back (makeEngine (i, 0, size, static_cast<u32> (maxBufferSize)));

            const auto tailBufferSize = static_cast<u32> (headSizeIn.headSizeInSamples + (isZeroDelay ? 0 : maxBufferSize));

            if (size != buf.getNumSamples())
                for (i32 i = 0; i < numChannels; ++i)
                    tail.emplace_back (makeEngine (i, size, buf.getNumSamples() - size, tailBufferSize));
        }
    }

    z0 reset()
    {
        for (const auto& e : head)
            e->reset();

        for (const auto& e : tail)
            e->reset();
    }

    z0 processSamples (const AudioBlock<const f32>& input, AudioBlock<f32>& output)
    {
        const auto numChannels = jmin (head.size(), input.getNumChannels(), output.getNumChannels());
        const auto numSamples  = jmin (input.getNumSamples(), output.getNumSamples());

        const AudioBlock<f32> fullTailBlock (tailBuffer);
        const auto tailBlock = fullTailBlock.getSubBlock (0, (size_t) numSamples);

        const auto isUniform = tail.empty();

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            if (! isUniform)
                tail[channel]->processSamplesWithAddedLatency (input.getChannelPointer (channel),
                                                               tailBlock.getChannelPointer (0),
                                                               numSamples);

            if (isZeroDelay)
                head[channel]->processSamples (input.getChannelPointer (channel),
                                               output.getChannelPointer (channel),
                                               numSamples);
            else
                head[channel]->processSamplesWithAddedLatency (input.getChannelPointer (channel),
                                                               output.getChannelPointer (channel),
                                                               numSamples);

            if (! isUniform)
                output.getSingleChannelBlock (channel) += tailBlock;
        }

        const auto numOutputChannels = output.getNumChannels();

        for (auto i = numChannels; i < numOutputChannels; ++i)
            output.getSingleChannelBlock (i).copyFrom (output.getSingleChannelBlock (0));
    }

    i32 getIRSize() const noexcept     { return irSize; }
    i32 getLatency() const noexcept    { return latency; }
    i32 getBlockSize() const noexcept  { return blockSize; }

private:
    std::vector<std::unique_ptr<ConvolutionEngine>> head, tail;
    AudioBuffer<f32> tailBuffer;

    i32k latency;
    i32k irSize;
    i32k blockSize;
    const b8 isZeroDelay;
};

static AudioBuffer<f32> fixNumChannels (const AudioBuffer<f32>& buf, Convolution::Stereo stereo)
{
    const auto numChannels = jmin (buf.getNumChannels(), stereo == Convolution::Stereo::yes ? 2 : 1);
    const auto numSamples = buf.getNumSamples();

    AudioBuffer<f32> result (numChannels, buf.getNumSamples());

    for (auto channel = 0; channel != numChannels; ++channel)
        result.copyFrom (channel, 0, buf.getReadPointer (channel), numSamples);

    if (result.getNumSamples() == 0 || result.getNumChannels() == 0)
    {
        result.setSize (1, 1);
        result.setSample (0, 0, 1.0f);
    }

    return result;
}

static AudioBuffer<f32> trimImpulseResponse (const AudioBuffer<f32>& buf)
{
    const auto thresholdTrim = Decibels::decibelsToGain (-80.0f);

    const auto numChannels = buf.getNumChannels();
    const auto numSamples = buf.getNumSamples();

    std::ptrdiff_t offsetBegin = numSamples;
    std::ptrdiff_t offsetEnd   = numSamples;

    for (auto channel = 0; channel < numChannels; ++channel)
    {
        const auto indexAboveThreshold = [&] (auto begin, auto end)
        {
            return std::distance (begin, std::find_if (begin, end, [&] (f32 sample)
            {
                return std::abs (sample) >= thresholdTrim;
            }));
        };

        const auto channelBegin = buf.getReadPointer (channel);
        const auto channelEnd = channelBegin + numSamples;
        const auto itStart = indexAboveThreshold (channelBegin, channelEnd);
        const auto itEnd = indexAboveThreshold (std::make_reverse_iterator (channelEnd),
                                                std::make_reverse_iterator (channelBegin));

        offsetBegin = jmin (offsetBegin, itStart);
        offsetEnd   = jmin (offsetEnd,   itEnd);
    }

    if (offsetBegin == numSamples)
    {
        auto result = AudioBuffer<f32> (numChannels, 1);
        result.clear();
        return result;
    }

    const auto newLength = jmax (1, numSamples - static_cast<i32> (offsetBegin + offsetEnd));

    AudioBuffer<f32> result (numChannels, newLength);

    for (auto channel = 0; channel < numChannels; ++channel)
    {
        result.copyFrom (channel,
                         0,
                         buf.getReadPointer (channel, static_cast<i32> (offsetBegin)),
                         result.getNumSamples());
    }

    return result;
}

static f32 calculateNormalisationFactor (f32 sumSquaredMagnitude)
{
    if (sumSquaredMagnitude < 1e-8f)
        return 1.0f;

    return 0.125f / std::sqrt (sumSquaredMagnitude);
}

static z0 normaliseImpulseResponse (AudioBuffer<f32>& buf)
{
    const auto numChannels = buf.getNumChannels();
    const auto numSamples  = buf.getNumSamples();
    const auto channelPtrs = buf.getArrayOfWritePointers();

    const auto maxSumSquaredMag = std::accumulate (channelPtrs, channelPtrs + numChannels, 0.0f, [numSamples] (auto max, auto* channel)
    {
        return jmax (max, std::accumulate (channel, channel + numSamples, 0.0f, [] (auto sum, auto samp)
        {
            return sum + (samp * samp);
        }));
    });

    const auto normalisationFactor = calculateNormalisationFactor (maxSumSquaredMag);

    std::for_each (channelPtrs, channelPtrs + numChannels, [normalisationFactor, numSamples] (auto* channel)
    {
        FloatVectorOperations::multiply (channel, normalisationFactor, numSamples);
    });
}

static AudioBuffer<f32> resampleImpulseResponse (const AudioBuffer<f32>& buf,
                                                   const f64 srcSampleRate,
                                                   const f64 destSampleRate)
{
    if (approximatelyEqual (srcSampleRate, destSampleRate))
        return buf;

    const auto factorReading = srcSampleRate / destSampleRate;

    AudioBuffer<f32> original = buf;
    MemoryAudioSource memorySource (original, false);
    ResamplingAudioSource resamplingSource (&memorySource, false, buf.getNumChannels());

    const auto finalSize = roundToInt (jmax (1.0, buf.getNumSamples() / factorReading));
    resamplingSource.setResamplingRatio (factorReading);
    resamplingSource.prepareToPlay (finalSize, srcSampleRate);

    AudioBuffer<f32> result (buf.getNumChannels(), finalSize);
    resamplingSource.getNextAudioBlock ({ &result, 0, result.getNumSamples() });

    return result;
}

//==============================================================================
template <typename Element>
class TryLockedPtr
{
public:
    z0 set (std::unique_ptr<Element> p)
    {
        const SpinLock::ScopedLockType lock (mutex);
        ptr = std::move (p);
    }

    std::unique_ptr<MultichannelEngine> get()
    {
        const SpinLock::ScopedTryLockType lock (mutex);
        return lock.isLocked() ? std::move (ptr) : nullptr;
    }

private:
    std::unique_ptr<Element> ptr;
    SpinLock mutex;
};

struct BufferWithSampleRate
{
    BufferWithSampleRate() = default;

    BufferWithSampleRate (AudioBuffer<f32>&& bufferIn, f64 sampleRateIn)
        : buffer (std::move (bufferIn)), sampleRate (sampleRateIn) {}

    AudioBuffer<f32> buffer;
    f64 sampleRate = 0.0;
};

static BufferWithSampleRate loadStreamToBuffer (std::unique_ptr<InputStream> stream, size_t maxLength)
{
    AudioFormatManager manager;
    manager.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> formatReader (manager.createReaderFor (std::move (stream)));

    if (formatReader == nullptr)
        return {};

    const auto fileLength = static_cast<size_t> (formatReader->lengthInSamples);
    const auto lengthToLoad = maxLength == 0 ? fileLength : jmin (maxLength, fileLength);

    BufferWithSampleRate result { { jlimit (1, 2, static_cast<i32> (formatReader->numChannels)),
                                    static_cast<i32> (lengthToLoad) },
                                  formatReader->sampleRate };

    formatReader->read (result.buffer.getArrayOfWritePointers(),
                        result.buffer.getNumChannels(),
                        0,
                        result.buffer.getNumSamples());

    return result;
}

// This class caches the data required to build a new convolution engine
// (in particular, impulse response data and a ProcessSpec).
// Calls to `setProcessSpec` and `setImpulseResponse` construct a
// new engine, which can be retrieved by calling `getEngine`.
class ConvolutionEngineFactory
{
public:
    ConvolutionEngineFactory (Convolution::Latency requiredLatency,
                              Convolution::NonUniform requiredHeadSize)
        : latency  { (requiredLatency.latencyInSamples   <= 0) ? 0 : jmax (64, nextPowerOfTwo (requiredLatency.latencyInSamples)) },
          headSize { (requiredHeadSize.headSizeInSamples <= 0) ? 0 : jmax (64, nextPowerOfTwo (requiredHeadSize.headSizeInSamples)) },
          shouldBeZeroLatency (requiredLatency.latencyInSamples == 0)
    {}

    // It is safe to call this method simultaneously with other public
    // member functions.
    z0 setProcessSpec (const ProcessSpec& spec)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        processSpec = spec;

        engine.set (makeEngine());
    }

    // It is safe to call this method simultaneously with other public
    // member functions.
    z0 setImpulseResponse (BufferWithSampleRate&& buf,
                             Convolution::Stereo stereo,
                             Convolution::Trim trim,
                             Convolution::Normalise normalise)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        wantsNormalise = normalise;
        originalSampleRate = buf.sampleRate;

        impulseResponse = [&]
        {
            auto corrected = fixNumChannels (buf.buffer, stereo);
            return trim == Convolution::Trim::yes ? trimImpulseResponse (corrected) : corrected;
        }();

        engine.set (makeEngine());
    }

    // Returns the most recently-created engine, or nullptr
    // if there is no pending engine, or if the engine is currently
    // being updated by one of the setter methods.
    // It is safe to call this simultaneously with other public
    // member functions.
    std::unique_ptr<MultichannelEngine> getEngine() { return engine.get(); }

private:
    std::unique_ptr<MultichannelEngine> makeEngine()
    {
        auto resampled = resampleImpulseResponse (impulseResponse, originalSampleRate, processSpec.sampleRate);

        if (wantsNormalise == Convolution::Normalise::yes)
            normaliseImpulseResponse (resampled);
        else
            resampled.applyGain ((f32) (originalSampleRate / processSpec.sampleRate));

        const auto currentLatency = jmax (processSpec.maximumBlockSize, (u32) latency.latencyInSamples);
        const auto maxBufferSize = shouldBeZeroLatency ? static_cast<i32> (processSpec.maximumBlockSize)
                                                       : nextPowerOfTwo (static_cast<i32> (currentLatency));

        return std::make_unique<MultichannelEngine> (resampled,
                                                     processSpec.maximumBlockSize,
                                                     maxBufferSize,
                                                     headSize,
                                                     shouldBeZeroLatency);
    }

    static AudioBuffer<f32> makeImpulseBuffer()
    {
        AudioBuffer<f32> result (1, 1);
        result.setSample (0, 0, 1.0f);
        return result;
    }

    ProcessSpec processSpec { 44100.0, 128, 2 };
    AudioBuffer<f32> impulseResponse = makeImpulseBuffer();
    f64 originalSampleRate = processSpec.sampleRate;
    Convolution::Normalise wantsNormalise = Convolution::Normalise::no;
    const Convolution::Latency latency;
    const Convolution::NonUniform headSize;
    const b8 shouldBeZeroLatency;

    TryLockedPtr<MultichannelEngine> engine;

    mutable std::mutex mutex;
};

static z0 setImpulseResponse (ConvolutionEngineFactory& factory,
                                ukk sourceData,
                                size_t sourceDataSize,
                                Convolution::Stereo stereo,
                                Convolution::Trim trim,
                                size_t size,
                                Convolution::Normalise normalise)
{
    factory.setImpulseResponse (loadStreamToBuffer (std::make_unique<MemoryInputStream> (sourceData, sourceDataSize, false), size),
                                stereo, trim, normalise);
}

static z0 setImpulseResponse (ConvolutionEngineFactory& factory,
                                const File& fileImpulseResponse,
                                Convolution::Stereo stereo,
                                Convolution::Trim trim,
                                size_t size,
                                Convolution::Normalise normalise)
{
    factory.setImpulseResponse (loadStreamToBuffer (std::make_unique<FileInputStream> (fileImpulseResponse), size),
                                stereo, trim, normalise);
}

// This class acts as a destination for convolution engines which are loaded on
// a background thread.

// Deriving from `enable_shared_from_this` allows us to capture a reference to
// this object when adding commands to the background message queue.
// That way, we can avoid dangling references in the background thread in the case
// that a Convolution instance is deleted before the background message queue.
class ConvolutionEngineQueue final : public std::enable_shared_from_this<ConvolutionEngineQueue>
{
public:
    ConvolutionEngineQueue (BackgroundMessageQueue& queue,
                            Convolution::Latency latencyIn,
                            Convolution::NonUniform headSizeIn)
        : messageQueue (queue), factory (latencyIn, headSizeIn) {}

    z0 loadImpulseResponse (AudioBuffer<f32>&& buffer,
                              f64 sr,
                              Convolution::Stereo stereo,
                              Convolution::Trim trim,
                              Convolution::Normalise normalise)
    {
        callLater ([b = std::move (buffer), sr, stereo, trim, normalise] (ConvolutionEngineFactory& f) mutable
        {
            f.setImpulseResponse ({ std::move (b), sr }, stereo, trim, normalise);
        });
    }

    z0 loadImpulseResponse (ukk sourceData,
                              size_t sourceDataSize,
                              Convolution::Stereo stereo,
                              Convolution::Trim trim,
                              size_t size,
                              Convolution::Normalise normalise)
    {
        callLater ([sourceData, sourceDataSize, stereo, trim, size, normalise] (ConvolutionEngineFactory& f) mutable
        {
            setImpulseResponse (f, sourceData, sourceDataSize, stereo, trim, size, normalise);
        });
    }

    z0 loadImpulseResponse (const File& fileImpulseResponse,
                              Convolution::Stereo stereo,
                              Convolution::Trim trim,
                              size_t size,
                              Convolution::Normalise normalise)
    {
        callLater ([fileImpulseResponse, stereo, trim, size, normalise] (ConvolutionEngineFactory& f) mutable
        {
            setImpulseResponse (f, fileImpulseResponse, stereo, trim, size, normalise);
        });
    }

    z0 prepare (const ProcessSpec& spec)
    {
        factory.setProcessSpec (spec);
    }

    // Call this regularly to try to resend any pending message.
    // This allows us to always apply the most recently requested
    // state (eventually), even if the message queue fills up.
    z0 postPendingCommand()
    {
        if (pendingCommand == nullptr)
            return;

        if (messageQueue.push (pendingCommand))
            pendingCommand = nullptr;
    }

    std::unique_ptr<MultichannelEngine> getEngine() { return factory.getEngine(); }

private:
    template <typename Fn>
    z0 callLater (Fn&& fn)
    {
        // If there was already a pending command (because the queue was full) we'll end up deleting it here.
        // Not much we can do about that!
        pendingCommand = [weak = weakFromThis(), callback = std::forward<Fn> (fn)]() mutable
        {
            if (auto t = weak.lock())
                callback (t->factory);
        };

        postPendingCommand();
    }

    std::weak_ptr<ConvolutionEngineQueue> weakFromThis() { return shared_from_this(); }

    BackgroundMessageQueue& messageQueue;
    ConvolutionEngineFactory factory;
    BackgroundMessageQueue::IncomingCommand pendingCommand;
};

class CrossoverMixer
{
public:
    z0 reset()
    {
        smoother.setCurrentAndTargetValue (1.0f);
    }

    z0 prepare (const ProcessSpec& spec)
    {
        smoother.reset (spec.sampleRate, 0.05);
        smootherBuffer.setSize (1, static_cast<i32> (spec.maximumBlockSize));
        mixBuffer.setSize (static_cast<i32> (spec.numChannels), static_cast<i32> (spec.maximumBlockSize));
        reset();
    }

    template <typename ProcessCurrent, typename ProcessPrevious, typename NotifyDone>
    z0 processSamples (const AudioBlock<const f32>& input,
                         AudioBlock<f32>& output,
                         ProcessCurrent&& current,
                         ProcessPrevious&& previous,
                         NotifyDone&& notifyDone)
    {
        if (smoother.isSmoothing())
        {
            const auto numSamples = static_cast<i32> (input.getNumSamples());

            for (auto sample = 0; sample != numSamples; ++sample)
                smootherBuffer.setSample (0, sample, smoother.getNextValue());

            AudioBlock<f32> mixBlock (mixBuffer);
            mixBlock.clear();
            previous (input, mixBlock);

            for (size_t channel = 0; channel != output.getNumChannels(); ++channel)
            {
                FloatVectorOperations::multiply (mixBlock.getChannelPointer (channel),
                                                 smootherBuffer.getReadPointer (0),
                                                 numSamples);
            }

            FloatVectorOperations::multiply (smootherBuffer.getWritePointer (0), -1.0f, numSamples);
            FloatVectorOperations::add (smootherBuffer.getWritePointer (0), 1.0f, numSamples);

            current (input, output);

            for (size_t channel = 0; channel != output.getNumChannels(); ++channel)
            {
                FloatVectorOperations::multiply (output.getChannelPointer (channel),
                                                 smootherBuffer.getReadPointer (0),
                                                 numSamples);
                FloatVectorOperations::add (output.getChannelPointer (channel),
                                            mixBlock.getChannelPointer (channel),
                                            numSamples);
            }

            if (! smoother.isSmoothing())
                notifyDone();
        }
        else
        {
            current (input, output);
        }
    }

    z0 beginTransition()
    {
        smoother.setCurrentAndTargetValue (1.0f);
        smoother.setTargetValue (0.0f);
    }

private:
    LinearSmoothedValue<f32> smoother;
    AudioBuffer<f32> smootherBuffer;
    AudioBuffer<f32> mixBuffer;
};

using OptionalQueue = OptionalScopedPointer<ConvolutionMessageQueue>;

class Convolution::Impl
{
public:
    Impl (Latency requiredLatency,
          NonUniform requiredHeadSize,
          OptionalQueue&& queue)
        : messageQueue (std::move (queue)),
          engineQueue (std::make_shared<ConvolutionEngineQueue> (*messageQueue->pimpl,
                                                                 requiredLatency,
                                                                 requiredHeadSize))
    {}

    z0 reset()
    {
        mixer.reset();

        if (currentEngine != nullptr)
            currentEngine->reset();

        destroyPreviousEngine();
    }

    z0 prepare (const ProcessSpec& spec)
    {
        messageQueue->pimpl->popAll();
        mixer.prepare (spec);
        engineQueue->prepare (spec);

        if (auto newEngine = engineQueue->getEngine())
            currentEngine = std::move (newEngine);

        previousEngine = nullptr;
        jassert (currentEngine != nullptr);
    }

    z0 processSamples (const AudioBlock<const f32>& input, AudioBlock<f32>& output)
    {
        engineQueue->postPendingCommand();

        if (previousEngine == nullptr)
            installPendingEngine();

        mixer.processSamples (input,
                              output,
                              [this] (const AudioBlock<const f32>& in, AudioBlock<f32>& out)
                              {
                                  currentEngine->processSamples (in, out);
                              },
                              [this] (const AudioBlock<const f32>& in, AudioBlock<f32>& out)
                              {
                                  if (previousEngine != nullptr)
                                      previousEngine->processSamples (in, out);
                                  else
                                      out.copyFrom (in);
                              },
                              [this] { destroyPreviousEngine(); });
    }

    i32 getCurrentIRSize() const { return currentEngine != nullptr ? currentEngine->getIRSize() : 0; }

    i32 getLatency() const { return currentEngine != nullptr ? currentEngine->getLatency() : 0; }

    z0 loadImpulseResponse (AudioBuffer<f32>&& buffer,
                              f64 originalSampleRate,
                              Stereo stereo,
                              Trim trim,
                              Normalise normalise)
    {
        engineQueue->loadImpulseResponse (std::move (buffer), originalSampleRate, stereo, trim, normalise);
    }

    z0 loadImpulseResponse (ukk sourceData,
                              size_t sourceDataSize,
                              Stereo stereo,
                              Trim trim,
                              size_t size,
                              Normalise normalise)
    {
        engineQueue->loadImpulseResponse (sourceData, sourceDataSize, stereo, trim, size, normalise);
    }

    z0 loadImpulseResponse (const File& fileImpulseResponse,
                              Stereo stereo,
                              Trim trim,
                              size_t size,
                              Normalise normalise)
    {
        engineQueue->loadImpulseResponse (fileImpulseResponse, stereo, trim, size, normalise);
    }

private:
    z0 destroyPreviousEngine()
    {
        // If the queue is full, we'll destroy this straight away
        BackgroundMessageQueue::IncomingCommand command = [p = std::move (previousEngine)]() mutable { p = nullptr; };
        messageQueue->pimpl->push (command);
    }

    z0 installNewEngine (std::unique_ptr<MultichannelEngine> newEngine)
    {
        destroyPreviousEngine();
        previousEngine = std::move (currentEngine);
        currentEngine = std::move (newEngine);
        mixer.beginTransition();
    }

    z0 installPendingEngine()
    {
        if (auto newEngine = engineQueue->getEngine())
            installNewEngine (std::move (newEngine));
    }

    OptionalQueue messageQueue;
    std::shared_ptr<ConvolutionEngineQueue> engineQueue;
    std::unique_ptr<MultichannelEngine> previousEngine, currentEngine;
    CrossoverMixer mixer;
};

//==============================================================================
z0 Convolution::Mixer::prepare (const ProcessSpec& spec)
{
    for (auto& dry : volumeDry)
        dry.reset (spec.sampleRate, 0.05);

    for (auto& wet : volumeWet)
        wet.reset (spec.sampleRate, 0.05);

    sampleRate = spec.sampleRate;

    dryBlock = AudioBlock<f32> (dryBlockStorage,
                                  jmin (spec.numChannels, 2u),
                                  spec.maximumBlockSize);

}

template <typename ProcessWet>
z0 Convolution::Mixer::processSamples (const AudioBlock<const f32>& input,
                                         AudioBlock<f32>& output,
                                         b8 isBypassed,
                                         ProcessWet&& processWet) noexcept
{
    const auto numChannels = jmin (input.getNumChannels(), volumeDry.size());
    const auto numSamples  = jmin (input.getNumSamples(), output.getNumSamples());

    auto dry = dryBlock.getSubsetChannelBlock (0, numChannels);

    if (volumeDry[0].isSmoothing())
    {
        dry.copyFrom (input);

        for (size_t channel = 0; channel < numChannels; ++channel)
            volumeDry[channel].applyGain (dry.getChannelPointer (channel), (i32) numSamples);

        processWet (input, output);

        for (size_t channel = 0; channel < numChannels; ++channel)
            volumeWet[channel].applyGain (output.getChannelPointer (channel), (i32) numSamples);

        output += dry;
    }
    else
    {
        if (! currentIsBypassed)
            processWet (input, output);

        if (isBypassed != currentIsBypassed)
        {
            currentIsBypassed = isBypassed;

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                volumeDry[channel].setTargetValue (isBypassed ? 0.0f : 1.0f);
                volumeDry[channel].reset (sampleRate, 0.05);
                volumeDry[channel].setTargetValue (isBypassed ? 1.0f : 0.0f);

                volumeWet[channel].setTargetValue (isBypassed ? 1.0f : 0.0f);
                volumeWet[channel].reset (sampleRate, 0.05);
                volumeWet[channel].setTargetValue (isBypassed ? 0.0f : 1.0f);
            }
        }
    }
}

z0 Convolution::Mixer::reset() { dryBlock.clear(); }

//==============================================================================
Convolution::Convolution()
    : Convolution (Latency { 0 })
{}

Convolution::Convolution (ConvolutionMessageQueue& queue)
    : Convolution (Latency { 0 }, queue)
{}

Convolution::Convolution (const Latency& requiredLatency)
    : Convolution (requiredLatency,
                   {},
                   OptionalQueue { std::make_unique<ConvolutionMessageQueue>() })
{}

Convolution::Convolution (const NonUniform& nonUniform)
    : Convolution ({},
                   nonUniform,
                   OptionalQueue { std::make_unique<ConvolutionMessageQueue>() })
{}

Convolution::Convolution (const Latency& requiredLatency, ConvolutionMessageQueue& queue)
    : Convolution (requiredLatency, {}, OptionalQueue { queue })
{}

Convolution::Convolution (const NonUniform& nonUniform, ConvolutionMessageQueue& queue)
    : Convolution ({}, nonUniform, OptionalQueue { queue })
{}

Convolution::Convolution (const Latency& latency,
                          const NonUniform& nonUniform,
                          OptionalQueue&& queue)
    : pimpl (std::make_unique<Impl> (latency, nonUniform, std::move (queue)))
{}

Convolution::~Convolution() noexcept = default;

z0 Convolution::loadImpulseResponse (ukk sourceData,
                                       size_t sourceDataSize,
                                       Stereo stereo,
                                       Trim trim,
                                       size_t size,
                                       Normalise normalise)
{
    pimpl->loadImpulseResponse (sourceData, sourceDataSize, stereo, trim, size, normalise);
}

z0 Convolution::loadImpulseResponse (const File& fileImpulseResponse,
                                       Stereo stereo,
                                       Trim trim,
                                       size_t size,
                                       Normalise normalise)
{
    pimpl->loadImpulseResponse (fileImpulseResponse, stereo, trim, size, normalise);
}

z0 Convolution::loadImpulseResponse (AudioBuffer<f32>&& buffer,
                                       f64 originalSampleRate,
                                       Stereo stereo,
                                       Trim trim,
                                       Normalise normalise)
{
    pimpl->loadImpulseResponse (std::move (buffer), originalSampleRate, stereo, trim, normalise);
}

z0 Convolution::prepare (const ProcessSpec& spec)
{
    mixer.prepare (spec);
    pimpl->prepare (spec);
    isActive = true;
}

z0 Convolution::reset() noexcept
{
    mixer.reset();
    pimpl->reset();
}

z0 Convolution::processSamples (const AudioBlock<const f32>& input,
                                  AudioBlock<f32>& output,
                                  b8 isBypassed) noexcept
{
    if (! isActive)
        return;

    jassert (input.getNumChannels() == output.getNumChannels());
    jassert (isPositiveAndBelow (input.getNumChannels(), static_cast<size_t> (3))); // only mono and stereo is supported

    mixer.processSamples (input, output, isBypassed, [this] (const auto& in, auto& out)
    {
        pimpl->processSamples (in, out);
    });
}

i32 Convolution::getCurrentIRSize() const { return pimpl->getCurrentIRSize(); }

i32 Convolution::getLatency() const { return pimpl->getLatency(); }

} // namespace drx::dsp
