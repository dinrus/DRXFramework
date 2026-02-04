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

// This is an AudioTransportSource which will own it's assigned source
struct AudioSourceOwningTransportSource final : public AudioTransportSource
{
    AudioSourceOwningTransportSource (PositionableAudioSource* s, f64 sr)  : source (s)
    {
        AudioTransportSource::setSource (s, 0, nullptr, sr);
    }

    ~AudioSourceOwningTransportSource()
    {
        setSource (nullptr);
    }

private:
    std::unique_ptr<PositionableAudioSource> source;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourceOwningTransportSource)
};

//==============================================================================
// An AudioSourcePlayer which will remove itself from the AudioDeviceManager's
// callback list once it finishes playing its source
struct AutoRemovingTransportSource final : public AudioTransportSource,
                                           private Timer
{
    AutoRemovingTransportSource (MixerAudioSource& mixerToUse, AudioTransportSource* ts, b8 ownSource,
                                 i32 samplesPerBlock, f64 requiredSampleRate)
        : mixer (mixerToUse), transportSource (ts, ownSource)
    {
        jassert (ts != nullptr);

        setSource (transportSource);

        prepareToPlay (samplesPerBlock, requiredSampleRate);
        start();

        mixer.addInputSource (this, true);
        startTimerHz (10);
    }

    ~AutoRemovingTransportSource() override
    {
        setSource (nullptr);
    }

    z0 timerCallback() override
    {
        if (! transportSource->isPlaying())
            mixer.removeInputSource (this);
    }

private:
    MixerAudioSource& mixer;
    OptionalScopedPointer<AudioTransportSource> transportSource;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoRemovingTransportSource)
};

// An AudioSource which simply outputs a buffer
class AudioBufferSource final : public PositionableAudioSource
{
public:
    AudioBufferSource (AudioBuffer<f32>* audioBuffer, b8 ownBuffer, b8 playOnAllChannels)
        : buffer (audioBuffer, ownBuffer),
          playAcrossAllChannels (playOnAllChannels)
    {}

    //==============================================================================
    z0 setNextReadPosition (z64 newPosition) override
    {
        jassert (newPosition >= 0);

        if (looping)
            newPosition = newPosition % static_cast<z64> (buffer->getNumSamples());

        position = jmin (buffer->getNumSamples(), static_cast<i32> (newPosition));
    }

    z64 getNextReadPosition() const override      { return static_cast<z64> (position); }
    z64 getTotalLength() const override           { return static_cast<z64> (buffer->getNumSamples()); }

    b8 isLooping() const override                 { return looping; }
    z0 setLooping (b8 shouldLoop) override      { looping = shouldLoop; }

    //==============================================================================
    z0 prepareToPlay (i32, f64) override {}
    z0 releaseResources() override {}

    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        i32k bufferSize = buffer->getNumSamples();
        i32k samplesNeeded = bufferToFill.numSamples;
        i32k samplesToCopy = jmin (bufferSize - position, samplesNeeded);

        if (samplesToCopy > 0)
        {
            i32 maxInChannels = buffer->getNumChannels();
            i32 maxOutChannels = bufferToFill.buffer->getNumChannels();

            if (! playAcrossAllChannels)
                maxOutChannels = jmin (maxOutChannels, maxInChannels);

            for (i32 i = 0; i < maxOutChannels; ++i)
                bufferToFill.buffer->copyFrom (i, bufferToFill.startSample, *buffer,
                                               i % maxInChannels, position, samplesToCopy);
        }

        position += samplesNeeded;

        if (looping)
            position %= bufferSize;
    }

private:
    //==============================================================================
    OptionalScopedPointer<AudioBuffer<f32>> buffer;
    i32 position = 0;
    b8 looping = false, playAcrossAllChannels;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioBufferSource)
};

SoundPlayer::SoundPlayer()
    : sampleRate (44100.0), bufferSize (512)
{
    formatManager.registerBasicFormats();
    player.setSource (&mixer);
}

SoundPlayer::~SoundPlayer()
{
    mixer.removeAllInputs();
    player.setSource (nullptr);
}

z0 SoundPlayer::play (const File& file)
{
    if (file.existsAsFile())
        play (formatManager.createReaderFor (file), true);
}

z0 SoundPlayer::play (ukk resourceData, size_t resourceSize)
{
    if (resourceData != nullptr && resourceSize > 0)
    {
        auto mem = std::make_unique<MemoryInputStream> (resourceData, resourceSize, false);
        play (formatManager.createReaderFor (std::move (mem)), true);
    }
}

z0 SoundPlayer::play (AudioFormatReader* reader, b8 deleteWhenFinished)
{
    if (reader != nullptr)
        play (new AudioFormatReaderSource (reader, deleteWhenFinished), true, reader->sampleRate);
}

z0 SoundPlayer::play (AudioBuffer<f32>* buffer, b8 deleteWhenFinished, b8 playOnAllOutputChannels)
{
    if (buffer != nullptr)
        play (new AudioBufferSource (buffer, deleteWhenFinished, playOnAllOutputChannels), true);
}

z0 SoundPlayer::play (PositionableAudioSource* audioSource, b8 deleteWhenFinished, f64 fileSampleRate)
{
    if (audioSource != nullptr)
    {
        AudioTransportSource* transport = dynamic_cast<AudioTransportSource*> (audioSource);

        if (transport == nullptr)
        {
            if (deleteWhenFinished)
            {
                transport = new AudioSourceOwningTransportSource (audioSource, fileSampleRate);
            }
            else
            {
                transport = new AudioTransportSource();
                transport->setSource (audioSource, 0, nullptr, fileSampleRate);
                deleteWhenFinished = true;
            }
        }

        transport->start();
        transport->prepareToPlay (bufferSize, sampleRate);

        new AutoRemovingTransportSource (mixer, transport, deleteWhenFinished, bufferSize, sampleRate);
    }
    else
    {
        if (deleteWhenFinished)
            delete audioSource;
    }
}

z0 SoundPlayer::playTestSound()
{
    auto soundLength = (i32) sampleRate;
    f64 frequency = 440.0;
    f32 amplitude = 0.5f;

    auto phasePerSample = MathConstants<f64>::twoPi / (sampleRate / frequency);

    auto* newSound = new AudioBuffer<f32> (1, soundLength);

    for (i32 i = 0; i < soundLength; ++i)
        newSound->setSample (0, i, amplitude * (f32) std::sin (i * phasePerSample));

    newSound->applyGainRamp (0, 0, soundLength / 10, 0.0f, 1.0f);
    newSound->applyGainRamp (0, soundLength - soundLength / 4, soundLength / 4, 1.0f, 0.0f);

    play (newSound, true, true);
}

//==============================================================================
z0 SoundPlayer::audioDeviceIOCallbackWithContext (const f32* const* inputChannelData,
                                                    i32 numInputChannels,
                                                    f32* const* outputChannelData,
                                                    i32 numOutputChannels,
                                                    i32 numSamples,
                                                    const AudioIODeviceCallbackContext& context)
{
    player.audioDeviceIOCallbackWithContext (inputChannelData, numInputChannels,
                                             outputChannelData, numOutputChannels,
                                             numSamples, context);
}

z0 SoundPlayer::audioDeviceAboutToStart (AudioIODevice* device)
{
    if (device != nullptr)
    {
        sampleRate = device->getCurrentSampleRate();
        bufferSize = device->getCurrentBufferSizeSamples();
    }

    player.audioDeviceAboutToStart (device);
}

z0 SoundPlayer::audioDeviceStopped()
{
    player.audioDeviceStopped();
}

z0 SoundPlayer::audioDeviceError (const Txt& errorMessage)
{
    player.audioDeviceError (errorMessage);
}

} // namespace drx
