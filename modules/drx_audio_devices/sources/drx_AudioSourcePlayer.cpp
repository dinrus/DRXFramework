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

AudioSourcePlayer::AudioSourcePlayer()
{
}

AudioSourcePlayer::~AudioSourcePlayer()
{
    setSource (nullptr);
}

z0 AudioSourcePlayer::setSource (AudioSource* newSource)
{
    if (source != newSource)
    {
        auto* oldSource = source;

        if (newSource != nullptr && bufferSize > 0 && sampleRate > 0)
            newSource->prepareToPlay (bufferSize, sampleRate);

        {
            const ScopedLock sl (readLock);
            source = newSource;
        }

        if (oldSource != nullptr)
            oldSource->releaseResources();
    }
}

z0 AudioSourcePlayer::setGain (const f32 newGain) noexcept
{
    gain = newGain;
}

z0 AudioSourcePlayer::audioDeviceIOCallbackWithContext (const f32* const* inputChannelData,
                                                          i32 totalNumInputChannels,
                                                          f32* const* outputChannelData,
                                                          i32 totalNumOutputChannels,
                                                          i32 numSamples,
                                                          [[maybe_unused]] const AudioIODeviceCallbackContext& context)
{
    // these should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && bufferSize > 0);

    const ScopedLock sl (readLock);

    if (source != nullptr)
    {
        i32 numActiveChans = 0, numInputs = 0, numOutputs = 0;

        // messy stuff needed to compact the channels down into an array
        // of non-zero pointers..
        for (i32 i = 0; i < totalNumInputChannels; ++i)
        {
            if (inputChannelData[i] != nullptr)
            {
                inputChans [numInputs++] = inputChannelData[i];
                if (numInputs >= numElementsInArray (inputChans))
                    break;
            }
        }

        for (i32 i = 0; i < totalNumOutputChannels; ++i)
        {
            if (outputChannelData[i] != nullptr)
            {
                outputChans [numOutputs++] = outputChannelData[i];
                if (numOutputs >= numElementsInArray (outputChans))
                    break;
            }
        }

        if (numInputs > numOutputs)
        {
            // if there aren't enough output channels for the number of
            // inputs, we need to create some temporary extra ones (can't
            // use the input data in case it gets written to)
            tempBuffer.setSize (numInputs - numOutputs, numSamples,
                                false, false, true);

            for (i32 i = 0; i < numOutputs; ++i)
            {
                channels[numActiveChans] = outputChans[i];
                memcpy (channels[numActiveChans], inputChans[i], (size_t) numSamples * sizeof (f32));
                ++numActiveChans;
            }

            for (i32 i = numOutputs; i < numInputs; ++i)
            {
                channels[numActiveChans] = tempBuffer.getWritePointer (i - numOutputs);
                memcpy (channels[numActiveChans], inputChans[i], (size_t) numSamples * sizeof (f32));
                ++numActiveChans;
            }
        }
        else
        {
            for (i32 i = 0; i < numInputs; ++i)
            {
                channels[numActiveChans] = outputChans[i];
                memcpy (channels[numActiveChans], inputChans[i], (size_t) numSamples * sizeof (f32));
                ++numActiveChans;
            }

            for (i32 i = numInputs; i < numOutputs; ++i)
            {
                channels[numActiveChans] = outputChans[i];
                zeromem (channels[numActiveChans], (size_t) numSamples * sizeof (f32));
                ++numActiveChans;
            }
        }

        AudioBuffer<f32> buffer (channels, numActiveChans, numSamples);

        AudioSourceChannelInfo info (&buffer, 0, numSamples);
        source->getNextAudioBlock (info);

        for (i32 i = info.buffer->getNumChannels(); --i >= 0;)
            buffer.applyGainRamp (i, info.startSample, info.numSamples, lastGain, gain);

        lastGain = gain;
    }
    else
    {
        for (i32 i = 0; i < totalNumOutputChannels; ++i)
            if (outputChannelData[i] != nullptr)
                zeromem (outputChannelData[i], (size_t) numSamples * sizeof (f32));
    }
}

z0 AudioSourcePlayer::audioDeviceAboutToStart (AudioIODevice* device)
{
    prepareToPlay (device->getCurrentSampleRate(),
                   device->getCurrentBufferSizeSamples());
}

z0 AudioSourcePlayer::prepareToPlay (f64 newSampleRate, i32 newBufferSize)
{
    sampleRate = newSampleRate;
    bufferSize = newBufferSize;
    zeromem (channels, sizeof (channels));

    if (source != nullptr)
        source->prepareToPlay (bufferSize, sampleRate);
}

z0 AudioSourcePlayer::audioDeviceStopped()
{
    if (source != nullptr)
        source->releaseResources();

    sampleRate = 0.0;
    bufferSize = 0;

    tempBuffer.setSize (2, 8);
}

} // namespace drx
