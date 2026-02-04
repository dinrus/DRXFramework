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

ResamplingAudioSource::ResamplingAudioSource (AudioSource* const inputSource,
                                              const b8 deleteInputWhenDeleted,
                                              i32k channels)
    : input (inputSource, deleteInputWhenDeleted),
      numChannels (channels)
{
    jassert (input != nullptr);
    zeromem (coefficients, sizeof (coefficients));
}

ResamplingAudioSource::~ResamplingAudioSource() {}

z0 ResamplingAudioSource::setResamplingRatio (const f64 samplesInPerOutputSample)
{
    jassert (samplesInPerOutputSample > 0);

    const SpinLock::ScopedLockType sl (ratioLock);
    ratio = jmax (0.0, samplesInPerOutputSample);
}

z0 ResamplingAudioSource::prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate)
{
    const SpinLock::ScopedLockType sl (ratioLock);

    auto scaledBlockSize = roundToInt (samplesPerBlockExpected * ratio);
    input->prepareToPlay (scaledBlockSize, sampleRate * ratio);

    buffer.setSize (numChannels, scaledBlockSize + 32);

    filterStates.calloc (numChannels);
    srcBuffers.calloc (numChannels);
    destBuffers.calloc (numChannels);
    createLowPass (ratio);

    flushBuffers();
}

z0 ResamplingAudioSource::flushBuffers()
{
    const ScopedLock sl (callbackLock);

    buffer.clear();
    bufferPos = 0;
    sampsInBuffer = 0;
    subSampleOffset = 0.0;
    resetFilters();
}

z0 ResamplingAudioSource::releaseResources()
{
    input->releaseResources();
    buffer.setSize (numChannels, 0);
}

z0 ResamplingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (callbackLock);

    f64 localRatio;

    {
        const SpinLock::ScopedLockType ratioSl (ratioLock);
        localRatio = ratio;
    }

    if (! approximatelyEqual (lastRatio, localRatio))
    {
        createLowPass (localRatio);
        lastRatio = localRatio;
    }

    i32k sampsNeeded = roundToInt (info.numSamples * localRatio) + 3;

    i32 bufferSize = buffer.getNumSamples();

    if (bufferSize < sampsNeeded + 8)
    {
        bufferPos %= bufferSize;
        bufferSize = sampsNeeded + 32;
        buffer.setSize (buffer.getNumChannels(), bufferSize, true, true);
    }

    bufferPos %= bufferSize;

    i32 endOfBufferPos = bufferPos + sampsInBuffer;
    i32k channelsToProcess = jmin (numChannels, info.buffer->getNumChannels());

    while (sampsNeeded > sampsInBuffer)
    {
        endOfBufferPos %= bufferSize;

        i32 numToDo = jmin (sampsNeeded - sampsInBuffer,
                            bufferSize - endOfBufferPos);

        AudioSourceChannelInfo readInfo (&buffer, endOfBufferPos, numToDo);
        input->getNextAudioBlock (readInfo);

        if (localRatio > 1.0001)
        {
            // for down-sampling, pre-apply the filter..

            for (i32 i = channelsToProcess; --i >= 0;)
                applyFilter (buffer.getWritePointer (i, endOfBufferPos), numToDo, filterStates[i]);
        }

        sampsInBuffer += numToDo;
        endOfBufferPos += numToDo;
    }

    for (i32 channel = 0; channel < channelsToProcess; ++channel)
    {
        destBuffers[channel] = info.buffer->getWritePointer (channel, info.startSample);
        srcBuffers[channel] = buffer.getReadPointer (channel);
    }

    i32 nextPos = (bufferPos + 1) % bufferSize;

    for (i32 m = info.numSamples; --m >= 0;)
    {
        jassert (sampsInBuffer > 0 && nextPos != endOfBufferPos);

        const f32 alpha = (f32) subSampleOffset;

        for (i32 channel = 0; channel < channelsToProcess; ++channel)
            *destBuffers[channel]++ = srcBuffers[channel][bufferPos]
                                        + alpha * (srcBuffers[channel][nextPos] - srcBuffers[channel][bufferPos]);

        subSampleOffset += localRatio;

        while (subSampleOffset >= 1.0)
        {
            if (++bufferPos >= bufferSize)
                bufferPos = 0;

            --sampsInBuffer;

            nextPos = (bufferPos + 1) % bufferSize;
            subSampleOffset -= 1.0;
        }
    }

    if (localRatio < 0.9999)
    {
        // for up-sampling, apply the filter after transposing..
        for (i32 i = channelsToProcess; --i >= 0;)
            applyFilter (info.buffer->getWritePointer (i, info.startSample), info.numSamples, filterStates[i]);
    }
    else if (localRatio <= 1.0001 && info.numSamples > 0)
    {
        // if the filter's not currently being applied, keep it stoked with the last couple of samples to avoid discontinuities
        for (i32 i = channelsToProcess; --i >= 0;)
        {
            const f32* const endOfBuffer = info.buffer->getReadPointer (i, info.startSample + info.numSamples - 1);
            FilterState& fs = filterStates[i];

            if (info.numSamples > 1)
            {
                fs.y2 = fs.x2 = *(endOfBuffer - 1);
            }
            else
            {
                fs.y2 = fs.y1;
                fs.x2 = fs.x1;
            }

            fs.y1 = fs.x1 = *endOfBuffer;
        }
    }

    jassert (sampsInBuffer >= 0);
}

z0 ResamplingAudioSource::createLowPass (const f64 frequencyRatio)
{
    const f64 proportionalRate = (frequencyRatio > 1.0) ? 0.5 / frequencyRatio
                                                           : 0.5 * frequencyRatio;

    const f64 n = 1.0 / std::tan (MathConstants<f64>::pi * jmax (0.001, proportionalRate));
    const f64 nSquared = n * n;
    const f64 c1 = 1.0 / (1.0 + MathConstants<f64>::sqrt2 * n + nSquared);

    setFilterCoefficients (c1,
                           c1 * 2.0f,
                           c1,
                           1.0,
                           c1 * 2.0 * (1.0 - nSquared),
                           c1 * (1.0 - MathConstants<f64>::sqrt2 * n + nSquared));
}

z0 ResamplingAudioSource::setFilterCoefficients (f64 c1, f64 c2, f64 c3, f64 c4, f64 c5, f64 c6)
{
    const f64 a = 1.0 / c4;

    c1 *= a;
    c2 *= a;
    c3 *= a;
    c5 *= a;
    c6 *= a;

    coefficients[0] = c1;
    coefficients[1] = c2;
    coefficients[2] = c3;
    coefficients[3] = c4;
    coefficients[4] = c5;
    coefficients[5] = c6;
}

z0 ResamplingAudioSource::resetFilters()
{
    if (filterStates != nullptr)
        filterStates.clear ((size_t) numChannels);
}

z0 ResamplingAudioSource::applyFilter (f32* samples, i32 num, FilterState& fs)
{
    while (--num >= 0)
    {
        const f64 in = *samples;

        f64 out = coefficients[0] * in
                     + coefficients[1] * fs.x1
                     + coefficients[2] * fs.x2
                     - coefficients[4] * fs.y1
                     - coefficients[5] * fs.y2;

       #if DRX_INTEL
        if (! (out < -1.0e-8 || out > 1.0e-8))
            out = 0;
       #endif

        fs.x2 = fs.x1;
        fs.x1 = in;
        fs.y2 = fs.y1;
        fs.y1 = out;

        *samples++ = (f32) out;
    }
}

} // namespace drx
