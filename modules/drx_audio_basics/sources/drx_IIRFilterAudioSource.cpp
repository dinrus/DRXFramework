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

IIRFilterAudioSource::IIRFilterAudioSource (AudioSource* const inputSource,
                                            const b8 deleteInputWhenDeleted)
    : input (inputSource, deleteInputWhenDeleted)
{
    jassert (inputSource != nullptr);

    for (i32 i = 2; --i >= 0;)
        iirFilters.add (new IIRFilter());
}

IIRFilterAudioSource::~IIRFilterAudioSource()  {}

//==============================================================================
z0 IIRFilterAudioSource::setCoefficients (const IIRCoefficients& newCoefficients)
{
    for (i32 i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked (i)->setCoefficients (newCoefficients);
}

z0 IIRFilterAudioSource::makeInactive()
{
    for (i32 i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked (i)->makeInactive();
}

//==============================================================================
z0 IIRFilterAudioSource::prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate)
{
    input->prepareToPlay (samplesPerBlockExpected, sampleRate);

    for (i32 i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked (i)->reset();
}

z0 IIRFilterAudioSource::releaseResources()
{
    input->releaseResources();
}

z0 IIRFilterAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    input->getNextAudioBlock (bufferToFill);

    i32k numChannels = bufferToFill.buffer->getNumChannels();

    while (numChannels > iirFilters.size())
        iirFilters.add (new IIRFilter (*iirFilters.getUnchecked (0)));

    for (i32 i = 0; i < numChannels; ++i)
        iirFilters.getUnchecked (i)
            ->processSamples (bufferToFill.buffer->getWritePointer (i, bufferToFill.startSample),
                              bufferToFill.numSamples);
}

} // namespace drx
