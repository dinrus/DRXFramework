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

ReverbAudioSource::ReverbAudioSource (AudioSource* const inputSource, const b8 deleteInputWhenDeleted)
   : input (inputSource, deleteInputWhenDeleted),
     bypass (false)
{
    jassert (inputSource != nullptr);
}

ReverbAudioSource::~ReverbAudioSource() {}

z0 ReverbAudioSource::prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate)
{
    const ScopedLock sl (lock);
    input->prepareToPlay (samplesPerBlockExpected, sampleRate);
    reverb.setSampleRate (sampleRate);
}

z0 ReverbAudioSource::releaseResources() {}

z0 ReverbAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    const ScopedLock sl (lock);

    input->getNextAudioBlock (bufferToFill);

    if (! bypass)
    {
        f32* const firstChannel = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);

        if (bufferToFill.buffer->getNumChannels() > 1)
        {
            reverb.processStereo (firstChannel,
                                  bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample),
                                  bufferToFill.numSamples);
        }
        else
        {
            reverb.processMono (firstChannel, bufferToFill.numSamples);
        }
    }
}

z0 ReverbAudioSource::setParameters (const Reverb::Parameters& newParams)
{
    const ScopedLock sl (lock);
    reverb.setParameters (newParams);
}

z0 ReverbAudioSource::setBypassed (b8 b) noexcept
{
    if (bypass != b)
    {
        const ScopedLock sl (lock);
        bypass = b;
        reverb.reset();
    }
}

} // namespace drx
