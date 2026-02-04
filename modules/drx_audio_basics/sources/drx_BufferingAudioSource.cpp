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

BufferingAudioSource::BufferingAudioSource (PositionableAudioSource* s,
                                            TimeSliceThread& thread,
                                            b8 deleteSourceWhenDeleted,
                                            i32 bufferSizeSamples,
                                            i32 numChannels,
                                            b8 prefillBufferOnPrepareToPlay)
    : source (s, deleteSourceWhenDeleted),
      backgroundThread (thread),
      numberOfSamplesToBuffer (jmax (1024, bufferSizeSamples)),
      numberOfChannels (numChannels),
      prefillBuffer (prefillBufferOnPrepareToPlay)
{
    jassert (source != nullptr);

    jassert (numberOfSamplesToBuffer > 1024); // not much point using this class if you're
                                              //  not using a larger buffer..
}

BufferingAudioSource::~BufferingAudioSource()
{
    releaseResources();
}

//==============================================================================
z0 BufferingAudioSource::prepareToPlay (i32 samplesPerBlockExpected, f64 newSampleRate)
{
    auto bufferSizeNeeded = jmax (samplesPerBlockExpected * 2, numberOfSamplesToBuffer);

    if (! approximatelyEqual (newSampleRate, sampleRate)
         || bufferSizeNeeded != buffer.getNumSamples()
         || ! isPrepared)
    {
        backgroundThread.removeTimeSliceClient (this);

        isPrepared = true;
        sampleRate = newSampleRate;

        source->prepareToPlay (samplesPerBlockExpected, newSampleRate);

        buffer.setSize (numberOfChannels, bufferSizeNeeded);
        buffer.clear();

        const ScopedLock sl (bufferRangeLock);

        bufferValidStart = 0;
        bufferValidEnd = 0;

        backgroundThread.addTimeSliceClient (this);

        do
        {
            const ScopedUnlock ul (bufferRangeLock);

            backgroundThread.moveToFrontOfQueue (this);
            Thread::sleep (5);
        }
        while (prefillBuffer
         && (bufferValidEnd - bufferValidStart < jmin (((i32) newSampleRate) / 4, buffer.getNumSamples() / 2)));
    }
}

z0 BufferingAudioSource::releaseResources()
{
    isPrepared = false;
    backgroundThread.removeTimeSliceClient (this);

    buffer.setSize (numberOfChannels, 0);

    // MSVC2017 seems to need this if statement to not generate a warning during linking.
    // As source is set in the constructor, there is no way that source could
    // ever equal this, but it seems to make MSVC2017 happy.
    if (source != this)
        source->releaseResources();
}

z0 BufferingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const auto bufferRange = getValidBufferRange (info.numSamples);

    if (bufferRange.isEmpty())
    {
        // total cache miss
        info.clearActiveBufferRegion();
        return;
    }

    const auto validStart = bufferRange.getStart();
    const auto validEnd = bufferRange.getEnd();

    const ScopedLock sl (callbackLock);

    if (validStart > 0)
        info.buffer->clear (info.startSample, validStart);  // partial cache miss at start

    if (validEnd < info.numSamples)
        info.buffer->clear (info.startSample + validEnd,
                            info.numSamples - validEnd);    // partial cache miss at end

    if (validStart < validEnd)
    {
        for (i32 chan = jmin (numberOfChannels, info.buffer->getNumChannels()); --chan >= 0;)
        {
            jassert (buffer.getNumSamples() > 0);

            const auto startBufferIndex = (i32) ((validStart + nextPlayPos) % buffer.getNumSamples());
            const auto endBufferIndex   = (i32) ((validEnd + nextPlayPos)   % buffer.getNumSamples());

            if (startBufferIndex < endBufferIndex)
            {
                info.buffer->copyFrom (chan, info.startSample + validStart,
                                       buffer,
                                       chan, startBufferIndex,
                                       validEnd - validStart);
            }
            else
            {
                const auto initialSize = buffer.getNumSamples() - startBufferIndex;

                info.buffer->copyFrom (chan, info.startSample + validStart,
                                       buffer,
                                       chan, startBufferIndex,
                                       initialSize);

                info.buffer->copyFrom (chan, info.startSample + validStart + initialSize,
                                       buffer,
                                       chan, 0,
                                       (validEnd - validStart) - initialSize);
            }
        }
    }

    nextPlayPos += info.numSamples;
}

b8 BufferingAudioSource::waitForNextAudioBlockReady (const AudioSourceChannelInfo& info, u32 timeout)
{
    if (source == nullptr || source->getTotalLength() <= 0)
        return false;

    if ((nextPlayPos + info.numSamples < 0)
        || (! isLooping() && nextPlayPos > getTotalLength()))
        return true;

    const auto startTime = Time::getMillisecondCounter();
    auto now = startTime;

    auto elapsed = (now >= startTime ? now - startTime
                                     : (std::numeric_limits<u32>::max() - startTime) + now);

    while (elapsed <= timeout)
    {
        const auto bufferRange = getValidBufferRange (info.numSamples);

        const auto validStart = bufferRange.getStart();
        const auto validEnd = bufferRange.getEnd();

        if (validStart <= 0
            && validStart < validEnd
            && validEnd >= info.numSamples)
        {
            return true;
        }

        if (elapsed < timeout
            && ! bufferReadyEvent.wait (static_cast<i32> (timeout - elapsed)))
        {
            return false;
        }

        now = Time::getMillisecondCounter();
        elapsed = (now >= startTime ? now - startTime
                                    : (std::numeric_limits<u32>::max() - startTime) + now);
    }

    return false;
}

z64 BufferingAudioSource::getNextReadPosition() const
{
    jassert (source->getTotalLength() > 0);
    const auto pos = nextPlayPos.load();

    return (source->isLooping() && nextPlayPos > 0)
                    ? pos % source->getTotalLength()
                    : pos;
}

z0 BufferingAudioSource::setNextReadPosition (z64 newPosition)
{
    const ScopedLock sl (bufferRangeLock);

    nextPlayPos = newPosition;
    backgroundThread.moveToFrontOfQueue (this);
}

Range<i32> BufferingAudioSource::getValidBufferRange (i32 numSamples) const
{
    const ScopedLock sl (bufferRangeLock);

    const auto pos = nextPlayPos.load();

    return { (i32) (jlimit (bufferValidStart, bufferValidEnd, pos) - pos),
             (i32) (jlimit (bufferValidStart, bufferValidEnd, pos + numSamples) - pos) };
}

b8 BufferingAudioSource::readNextBufferChunk()
{
    z64 newBVS, newBVE, sectionToReadStart, sectionToReadEnd;

    {
        const ScopedLock sl (bufferRangeLock);

        if (wasSourceLooping != isLooping())
        {
            wasSourceLooping = isLooping();
            bufferValidStart = 0;
            bufferValidEnd = 0;
        }

        newBVS = jmax ((z64) 0, nextPlayPos.load());
        newBVE = newBVS + buffer.getNumSamples() - 4;
        sectionToReadStart = 0;
        sectionToReadEnd = 0;

        constexpr i32 maxChunkSize = 2048;

        if (newBVS < bufferValidStart || newBVS >= bufferValidEnd)
        {
            newBVE = jmin (newBVE, newBVS + maxChunkSize);

            sectionToReadStart = newBVS;
            sectionToReadEnd = newBVE;

            bufferValidStart = 0;
            bufferValidEnd = 0;
        }
        else if (std::abs ((i32) (newBVS - bufferValidStart)) > 512
                  || std::abs ((i32) (newBVE - bufferValidEnd)) > 512)
        {
            newBVE = jmin (newBVE, bufferValidEnd + maxChunkSize);

            sectionToReadStart = bufferValidEnd;
            sectionToReadEnd = newBVE;

            bufferValidStart = newBVS;
            bufferValidEnd = jmin (bufferValidEnd, newBVE);
        }
    }

    if (sectionToReadStart == sectionToReadEnd)
        return false;

    jassert (buffer.getNumSamples() > 0);

    const auto bufferIndexStart = (i32) (sectionToReadStart % buffer.getNumSamples());
    const auto bufferIndexEnd   = (i32) (sectionToReadEnd   % buffer.getNumSamples());

    if (bufferIndexStart < bufferIndexEnd)
    {
        readBufferSection (sectionToReadStart,
                           (i32) (sectionToReadEnd - sectionToReadStart),
                           bufferIndexStart);
    }
    else
    {
        const auto initialSize = buffer.getNumSamples() - bufferIndexStart;

        readBufferSection (sectionToReadStart,
                           initialSize,
                           bufferIndexStart);

        readBufferSection (sectionToReadStart + initialSize,
                           (i32) (sectionToReadEnd - sectionToReadStart) - initialSize,
                           0);
    }

    {
        const ScopedLock sl2 (bufferRangeLock);

        bufferValidStart = newBVS;
        bufferValidEnd = newBVE;
    }

    bufferReadyEvent.signal();
    return true;
}

z0 BufferingAudioSource::readBufferSection (z64 start, i32 length, i32 bufferOffset)
{
    if (source->getNextReadPosition() != start)
        source->setNextReadPosition (start);

    AudioSourceChannelInfo info (&buffer, bufferOffset, length);

    const ScopedLock sl (callbackLock);
    source->getNextAudioBlock (info);
}

i32 BufferingAudioSource::useTimeSlice()
{
    return readNextBufferChunk() ? 1 : 100;
}

} // namespace drx
