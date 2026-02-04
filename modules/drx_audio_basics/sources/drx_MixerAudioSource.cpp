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

MixerAudioSource::MixerAudioSource()
   : currentSampleRate (0.0), bufferSizeExpected (0)
{
}

MixerAudioSource::~MixerAudioSource()
{
    removeAllInputs();
}

//==============================================================================
z0 MixerAudioSource::addInputSource (AudioSource* input, const b8 deleteWhenRemoved)
{
    if (input != nullptr && ! inputs.contains (input))
    {
        f64 localRate;
        i32 localBufferSize;

        {
            const ScopedLock sl (lock);
            localRate = currentSampleRate;
            localBufferSize = bufferSizeExpected;
        }

        if (localRate > 0.0)
            input->prepareToPlay (localBufferSize, localRate);

        const ScopedLock sl (lock);

        inputsToDelete.setBit (inputs.size(), deleteWhenRemoved);
        inputs.add (input);
    }
}

z0 MixerAudioSource::removeInputSource (AudioSource* const input)
{
    if (input != nullptr)
    {
        std::unique_ptr<AudioSource> toDelete;

        {
            const ScopedLock sl (lock);
            i32k index = inputs.indexOf (input);

            if (index < 0)
                return;

            if (inputsToDelete [index])
                toDelete.reset (input);

            inputsToDelete.shiftBits (-1, index);
            inputs.remove (index);
        }

        input->releaseResources();
    }
}

z0 MixerAudioSource::removeAllInputs()
{
    OwnedArray<AudioSource> toDelete;

    {
        const ScopedLock sl (lock);

        for (i32 i = inputs.size(); --i >= 0;)
            if (inputsToDelete[i])
                toDelete.add (inputs.getUnchecked (i));

        inputs.clear();
    }

    for (i32 i = toDelete.size(); --i >= 0;)
        toDelete.getUnchecked (i)->releaseResources();
}

z0 MixerAudioSource::prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate)
{
    tempBuffer.setSize (2, samplesPerBlockExpected);

    const ScopedLock sl (lock);

    currentSampleRate = sampleRate;
    bufferSizeExpected = samplesPerBlockExpected;

    for (i32 i = inputs.size(); --i >= 0;)
        inputs.getUnchecked (i)->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

z0 MixerAudioSource::releaseResources()
{
    const ScopedLock sl (lock);

    for (i32 i = inputs.size(); --i >= 0;)
        inputs.getUnchecked (i)->releaseResources();

    tempBuffer.setSize (2, 0);

    currentSampleRate = 0;
    bufferSizeExpected = 0;
}

z0 MixerAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (lock);

    if (inputs.size() > 0)
    {
        inputs.getUnchecked (0)->getNextAudioBlock (info);

        if (inputs.size() > 1)
        {
            tempBuffer.setSize (jmax (1, info.buffer->getNumChannels()),
                                info.buffer->getNumSamples());

            AudioSourceChannelInfo info2 (&tempBuffer, 0, info.numSamples);

            for (i32 i = 1; i < inputs.size(); ++i)
            {
                inputs.getUnchecked (i)->getNextAudioBlock (info2);

                for (i32 chan = 0; chan < info.buffer->getNumChannels(); ++chan)
                    info.buffer->addFrom (chan, info.startSample, tempBuffer, chan, 0, info.numSamples);
            }
        }
    }
    else
    {
        info.clearActiveBufferRegion();
    }
}

} // namespace drx
