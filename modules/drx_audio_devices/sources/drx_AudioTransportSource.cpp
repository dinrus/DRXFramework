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

AudioTransportSource::AudioTransportSource()
{
}

AudioTransportSource::~AudioTransportSource()
{
    setSource (nullptr);
    releaseMasterResources();
}

z0 AudioTransportSource::setSource (PositionableAudioSource* const newSource,
                                      i32 readAheadSize, TimeSliceThread* readAheadThread,
                                      f64 sourceSampleRateToCorrectFor, i32 maxNumChannels)
{
    if (source == newSource)
    {
        if (source == nullptr)
            return;

        setSource (nullptr, 0, nullptr); // deselect and reselect to avoid releasing resources wrongly
    }

    ResamplingAudioSource* newResamplerSource = nullptr;
    BufferingAudioSource* newBufferingSource = nullptr;
    PositionableAudioSource* newPositionableSource = nullptr;
    AudioSource* newMasterSource = nullptr;

    std::unique_ptr<ResamplingAudioSource> oldResamplerSource (resamplerSource);
    std::unique_ptr<BufferingAudioSource> oldBufferingSource (bufferingSource);
    AudioSource* oldMasterSource = masterSource;

    if (newSource != nullptr)
    {
        newPositionableSource = newSource;

        if (readAheadSize > 0)
        {
            // If you want to use a read-ahead buffer, you must also provide a TimeSliceThread
            // for it to use!
            jassert (readAheadThread != nullptr);

            newPositionableSource = newBufferingSource
                = new BufferingAudioSource (newPositionableSource, *readAheadThread,
                                            false, readAheadSize, maxNumChannels);
        }

        newPositionableSource->setNextReadPosition (0);

        if (sourceSampleRateToCorrectFor > 0)
            newMasterSource = newResamplerSource
                = new ResamplingAudioSource (newPositionableSource, false, maxNumChannels);
        else
            newMasterSource = newPositionableSource;

        if (isPrepared)
        {
            if (newResamplerSource != nullptr && sourceSampleRateToCorrectFor > 0 && sampleRate > 0)
                newResamplerSource->setResamplingRatio (sourceSampleRateToCorrectFor / sampleRate);

            newMasterSource->prepareToPlay (blockSize, sampleRate);
        }
    }

    {
        const ScopedLock sl (callbackLock);

        source = newSource;
        resamplerSource = newResamplerSource;
        bufferingSource = newBufferingSource;
        masterSource = newMasterSource;
        positionableSource = newPositionableSource;
        readAheadBufferSize = readAheadSize;
        sourceSampleRate = sourceSampleRateToCorrectFor;

        playing = false;
    }

    if (oldMasterSource != nullptr)
        oldMasterSource->releaseResources();
}

z0 AudioTransportSource::start()
{
    if ((! playing) && masterSource != nullptr)
    {
        {
            const ScopedLock sl (callbackLock);
            playing = true;
            stopped = false;
        }

        sendChangeMessage();
    }
}

z0 AudioTransportSource::stop()
{
    if (playing)
    {
        playing = false;

        i32 n = 500;
        while (--n >= 0 && ! stopped)
            Thread::sleep (2);

        sendChangeMessage();
    }
}

z0 AudioTransportSource::setPosition (f64 newPosition)
{
    if (sampleRate > 0.0)
        setNextReadPosition ((z64) (newPosition * sampleRate));
}

f64 AudioTransportSource::getCurrentPosition() const
{
    if (sampleRate > 0.0)
        return (f64) getNextReadPosition() / sampleRate;

    return 0.0;
}

f64 AudioTransportSource::getLengthInSeconds() const
{
    if (sampleRate > 0.0)
        return (f64) getTotalLength() / sampleRate;

    return 0.0;
}

b8 AudioTransportSource::hasStreamFinished() const noexcept
{
    if (positionableSource == nullptr)
        return true;

    if (positionableSource->isLooping())
        return false;

    return positionableSource->getNextReadPosition() >= positionableSource->getTotalLength();
}

z0 AudioTransportSource::setNextReadPosition (z64 newPosition)
{
    if (positionableSource != nullptr)
    {
        if (sampleRate > 0 && sourceSampleRate > 0)
            newPosition = (z64) ((f64) newPosition * sourceSampleRate / sampleRate);

        positionableSource->setNextReadPosition (newPosition);

        if (resamplerSource != nullptr)
            resamplerSource->flushBuffers();
    }
}

z64 AudioTransportSource::getNextReadPosition() const
{
    const ScopedLock sl (callbackLock);

    if (positionableSource != nullptr)
    {
        const f64 ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;
        return (z64) ((f64) positionableSource->getNextReadPosition() * ratio);
    }

    return 0;
}

z64 AudioTransportSource::getTotalLength() const
{
    const ScopedLock sl (callbackLock);

    if (positionableSource != nullptr)
    {
        const f64 ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;
        return (z64) ((f64) positionableSource->getTotalLength() * ratio);
    }

    return 0;
}

b8 AudioTransportSource::isLooping() const
{
    const ScopedLock sl (callbackLock);
    return positionableSource != nullptr && positionableSource->isLooping();
}

z0 AudioTransportSource::setGain (const f32 newGain) noexcept
{
    gain = newGain;
}

z0 AudioTransportSource::prepareToPlay (i32 samplesPerBlockExpected, f64 newSampleRate)
{
    const ScopedLock sl (callbackLock);

    sampleRate = newSampleRate;
    blockSize = samplesPerBlockExpected;

    if (masterSource != nullptr)
        masterSource->prepareToPlay (samplesPerBlockExpected, sampleRate);

    if (resamplerSource != nullptr && sourceSampleRate > 0)
        resamplerSource->setResamplingRatio (sourceSampleRate / sampleRate);

    isPrepared = true;
}

z0 AudioTransportSource::releaseMasterResources()
{
    const ScopedLock sl (callbackLock);

    if (masterSource != nullptr)
        masterSource->releaseResources();

    isPrepared = false;
}

z0 AudioTransportSource::releaseResources()
{
    releaseMasterResources();
}

z0 AudioTransportSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (callbackLock);

    if (masterSource != nullptr && ! stopped)
    {
        masterSource->getNextAudioBlock (info);

        if (! playing)
        {
            // just stopped playing, so fade out the last block..
            for (i32 i = info.buffer->getNumChannels(); --i >= 0;)
                info.buffer->applyGainRamp (i, info.startSample, jmin (256, info.numSamples), 1.0f, 0.0f);

            if (info.numSamples > 256)
                info.buffer->clear (info.startSample + 256, info.numSamples - 256);
        }

        if (hasStreamFinished())
        {
            playing = false;
            sendChangeMessage();
        }

        stopped = ! playing;

        for (i32 i = info.buffer->getNumChannels(); --i >= 0;)
            info.buffer->applyGainRamp (i, info.startSample, info.numSamples, lastGain, gain);
    }
    else
    {
        info.clearActiveBufferRegion();
        stopped = true;
    }

    lastGain = gain;
}

} // namespace drx
