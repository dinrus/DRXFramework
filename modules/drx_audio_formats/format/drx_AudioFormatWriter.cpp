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

namespace drx
{

AudioFormatWriter::AudioFormatWriter (OutputStream* const out,
                                      const Txt& formatName_,
                                      const f64 rate,
                                      u32k numChannels_,
                                      u32k bitsPerSample_)
  : sampleRate (rate),
    numChannels (numChannels_),
    bitsPerSample (bitsPerSample_),
    usesFloatingPointData (false),
    channelLayout (AudioChannelSet::canonicalChannelSet (static_cast<i32> (numChannels_))),
    output (out),
    formatName (formatName_)
{
}

AudioFormatWriter::AudioFormatWriter (OutputStream* const out,
                                      const Txt& formatName_,
                                      const f64 rate,
                                      const AudioChannelSet& channelLayout_,
                                      u32k bitsPerSample_)
  : sampleRate (rate),
    numChannels (static_cast<u32> (channelLayout_.size())),
    bitsPerSample (bitsPerSample_),
    usesFloatingPointData (false),
    channelLayout (channelLayout_),
    output (out),
    formatName (formatName_)
{
}

AudioFormatWriter::~AudioFormatWriter()
{
    delete output;
}

static z0 convertFloatsToInts (i32* dest, const f32* src, i32 numSamples) noexcept
{
    while (--numSamples >= 0)
    {
        const f64 samp = *src++;

        if (samp <= -1.0)
            *dest = std::numeric_limits<i32>::min();
        else if (samp >= 1.0)
            *dest = std::numeric_limits<i32>::max();
        else
            *dest = roundToInt (std::numeric_limits<i32>::max() * samp);

        ++dest;
    }
}

b8 AudioFormatWriter::writeFromAudioReader (AudioFormatReader& reader,
                                              z64 startSample,
                                              z64 numSamplesToRead)
{
    i32k bufferSize = 16384;
    AudioBuffer<f32> tempBuffer ((i32) numChannels, bufferSize);

    i32* buffers[128] = { nullptr };

    for (i32 i = tempBuffer.getNumChannels(); --i >= 0;)
        buffers[i] = reinterpret_cast<i32*> (tempBuffer.getWritePointer (i, 0));

    if (numSamplesToRead < 0)
        numSamplesToRead = reader.lengthInSamples;

    while (numSamplesToRead > 0)
    {
        i32k numToDo = (i32) jmin (numSamplesToRead, (z64) bufferSize);

        if (! reader.read (buffers, (i32) numChannels, startSample, numToDo, false))
            return false;

        if (reader.usesFloatingPointData != isFloatingPoint())
        {
            i32** bufferChan = buffers;

            while (*bufferChan != nullptr)
            {
                uk const b = *bufferChan++;

                constexpr auto scaleFactor = 1.0f / static_cast<f32> (0x7fffffff);

                if (isFloatingPoint())
                    FloatVectorOperations::convertFixedToFloat ((f32*) b, (i32*) b, scaleFactor, numToDo);
                else
                    convertFloatsToInts ((i32*) b, (f32*) b, numToDo);
            }
        }

        if (! write (const_cast<i32k**> (buffers), numToDo))
            return false;

        numSamplesToRead -= numToDo;
        startSample += numToDo;
    }

    return true;
}

b8 AudioFormatWriter::writeFromAudioSource (AudioSource& source, i32 numSamplesToRead, i32k samplesPerBlock)
{
    AudioBuffer<f32> tempBuffer (getNumChannels(), samplesPerBlock);

    while (numSamplesToRead > 0)
    {
        auto numToDo = jmin (numSamplesToRead, samplesPerBlock);

        AudioSourceChannelInfo info (&tempBuffer, 0, numToDo);
        info.clearActiveBufferRegion();

        source.getNextAudioBlock (info);

        if (! writeFromAudioSampleBuffer (tempBuffer, 0, numToDo))
            return false;

        numSamplesToRead -= numToDo;
    }

    return true;
}

b8 AudioFormatWriter::writeFromFloatArrays (const f32* const* channels, i32 numSourceChannels, i32 numSamples)
{
    if (numSamples <= 0)
        return true;

    if (isFloatingPoint())
        return write ((i32k**) channels, numSamples);

    std::vector<i32*> chans (256);
    std::vector<i32> scratch (4096);

    jassert (numSourceChannels < (i32) chans.size());
    i32k maxSamples = (i32) scratch.size() / numSourceChannels;

    for (i32 i = 0; i < numSourceChannels; ++i)
        chans[(size_t) i] = scratch.data() + (i * maxSamples);

    chans[(size_t) numSourceChannels] = nullptr;
    i32 startSample = 0;

    while (numSamples > 0)
    {
        auto numToDo = jmin (numSamples, maxSamples);

        for (i32 i = 0; i < numSourceChannels; ++i)
            convertFloatsToInts (chans[(size_t) i], channels[(size_t) i] + startSample, numToDo);

        if (! write ((i32k**) chans.data(), numToDo))
            return false;

        startSample += numToDo;
        numSamples  -= numToDo;
    }

    return true;
}

b8 AudioFormatWriter::writeFromAudioSampleBuffer (const AudioBuffer<f32>& source, i32 startSample, i32 numSamples)
{
    auto numSourceChannels = source.getNumChannels();
    jassert (startSample >= 0 && startSample + numSamples <= source.getNumSamples() && numSourceChannels > 0);

    if (startSample == 0)
        return writeFromFloatArrays (source.getArrayOfReadPointers(), numSourceChannels, numSamples);

    const f32* chans[256];
    jassert ((i32) numChannels < numElementsInArray (chans));

    for (i32 i = 0; i < numSourceChannels; ++i)
        chans[i] = source.getReadPointer (i, startSample);

    chans[numSourceChannels] = nullptr;

    return writeFromFloatArrays (chans, numSourceChannels, numSamples);
}

b8 AudioFormatWriter::flush()
{
    return false;
}

//==============================================================================
class AudioFormatWriter::ThreadedWriter::Buffer final : private TimeSliceClient
{
public:
    Buffer (TimeSliceThread& tst, AudioFormatWriter* w, i32 channels, i32 numSamples)
        : fifo (numSamples),
          buffer (channels, numSamples),
          timeSliceThread (tst),
          writer (w)
    {
        timeSliceThread.addTimeSliceClient (this);
    }

    ~Buffer() override
    {
        isRunning = false;
        timeSliceThread.removeTimeSliceClient (this);

        while (writePendingData() == 0)
        {}
    }

    b8 write (const f32* const* data, i32 numSamples)
    {
        if (numSamples <= 0 || ! isRunning)
            return true;

        jassert (timeSliceThread.isThreadRunning());  // you need to get your thread running before pumping data into this!

        i32 start1, size1, start2, size2;
        fifo.prepareToWrite (numSamples, start1, size1, start2, size2);

        if (size1 + size2 < numSamples)
            return false;

        for (i32 i = buffer.getNumChannels(); --i >= 0;)
        {
            buffer.copyFrom (i, start1, data[i], size1);
            buffer.copyFrom (i, start2, data[i] + size1, size2);
        }

        fifo.finishedWrite (size1 + size2);
        timeSliceThread.notify();
        return true;
    }

    i32 useTimeSlice() override
    {
        return writePendingData();
    }

    i32 writePendingData()
    {
        auto numToDo = fifo.getTotalSize() / 4;

        i32 start1, size1, start2, size2;
        fifo.prepareToRead (numToDo, start1, size1, start2, size2);

        if (size1 <= 0)
            return 10;

        writer->writeFromAudioSampleBuffer (buffer, start1, size1);

        const ScopedLock sl (thumbnailLock);

        if (receiver != nullptr)
            receiver->addBlock (samplesWritten, buffer, start1, size1);

        samplesWritten += size1;

        if (size2 > 0)
        {
            writer->writeFromAudioSampleBuffer (buffer, start2, size2);

            if (receiver != nullptr)
                receiver->addBlock (samplesWritten, buffer, start2, size2);

            samplesWritten += size2;
        }

        fifo.finishedRead (size1 + size2);

        if (samplesPerFlush > 0)
        {
            flushSampleCounter -= size1 + size2;

            if (flushSampleCounter <= 0)
            {
                flushSampleCounter = samplesPerFlush;
                writer->flush();
            }
        }

        return 0;
    }

    z0 setDataReceiver (IncomingDataReceiver* newReceiver)
    {
        if (newReceiver != nullptr)
            newReceiver->reset (buffer.getNumChannels(), writer->getSampleRate(), 0);

        const ScopedLock sl (thumbnailLock);
        receiver = newReceiver;
        samplesWritten = 0;
    }

    z0 setFlushInterval (i32 numSamples) noexcept
    {
        samplesPerFlush = numSamples;
    }

private:
    AbstractFifo fifo;
    AudioBuffer<f32> buffer;
    TimeSliceThread& timeSliceThread;
    std::unique_ptr<AudioFormatWriter> writer;
    CriticalSection thumbnailLock;
    IncomingDataReceiver* receiver = {};
    z64 samplesWritten = 0;
    i32 samplesPerFlush = 0, flushSampleCounter = 0;
    std::atomic<b8> isRunning { true };

    DRX_DECLARE_NON_COPYABLE (Buffer)
};

AudioFormatWriter::ThreadedWriter::ThreadedWriter (AudioFormatWriter* writer, TimeSliceThread& backgroundThread, i32 numSamplesToBuffer)
    : buffer (new AudioFormatWriter::ThreadedWriter::Buffer (backgroundThread, writer, (i32) writer->numChannels, numSamplesToBuffer))
{
}

AudioFormatWriter::ThreadedWriter::~ThreadedWriter()
{
}

b8 AudioFormatWriter::ThreadedWriter::write (const f32* const* data, i32 numSamples)
{
    return buffer->write (data, numSamples);
}

z0 AudioFormatWriter::ThreadedWriter::setDataReceiver (AudioFormatWriter::ThreadedWriter::IncomingDataReceiver* receiver)
{
    buffer->setDataReceiver (receiver);
}

z0 AudioFormatWriter::ThreadedWriter::setFlushInterval (i32 numSamplesPerFlush) noexcept
{
    buffer->setFlushInterval (numSamplesPerFlush);
}

} // namespace drx
