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

ChannelRemappingAudioSource::ChannelRemappingAudioSource (AudioSource* const source_,
                                                          const b8 deleteSourceWhenDeleted)
   : source (source_, deleteSourceWhenDeleted),
     requiredNumberOfChannels (2)
{
    remappedInfo.buffer = &buffer;
    remappedInfo.startSample = 0;
}

ChannelRemappingAudioSource::~ChannelRemappingAudioSource() {}

//==============================================================================
z0 ChannelRemappingAudioSource::setNumberOfChannelsToProduce (i32k requiredNumberOfChannels_)
{
    const ScopedLock sl (lock);
    requiredNumberOfChannels = requiredNumberOfChannels_;
}

z0 ChannelRemappingAudioSource::clearAllMappings()
{
    const ScopedLock sl (lock);

    remappedInputs.clear();
    remappedOutputs.clear();
}

z0 ChannelRemappingAudioSource::setInputChannelMapping (i32k destIndex, i32k sourceIndex)
{
    const ScopedLock sl (lock);

    while (remappedInputs.size() < destIndex)
        remappedInputs.add (-1);

    remappedInputs.set (destIndex, sourceIndex);
}

z0 ChannelRemappingAudioSource::setOutputChannelMapping (i32k sourceIndex, i32k destIndex)
{
    const ScopedLock sl (lock);

    while (remappedOutputs.size() < sourceIndex)
        remappedOutputs.add (-1);

    remappedOutputs.set (sourceIndex, destIndex);
}

i32 ChannelRemappingAudioSource::getRemappedInputChannel (i32k inputChannelIndex) const
{
    const ScopedLock sl (lock);

    if (inputChannelIndex >= 0 && inputChannelIndex < remappedInputs.size())
        return remappedInputs.getUnchecked (inputChannelIndex);

    return -1;
}

i32 ChannelRemappingAudioSource::getRemappedOutputChannel (i32k outputChannelIndex) const
{
    const ScopedLock sl (lock);

    if (outputChannelIndex >= 0 && outputChannelIndex < remappedOutputs.size())
        return remappedOutputs .getUnchecked (outputChannelIndex);

    return -1;
}

//==============================================================================
z0 ChannelRemappingAudioSource::prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate)
{
    source->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

z0 ChannelRemappingAudioSource::releaseResources()
{
    source->releaseResources();
}

z0 ChannelRemappingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    const ScopedLock sl (lock);

    buffer.setSize (requiredNumberOfChannels, bufferToFill.numSamples, false, false, true);

    i32k numChans = bufferToFill.buffer->getNumChannels();

    for (i32 i = 0; i < buffer.getNumChannels(); ++i)
    {
        i32k remappedChan = getRemappedInputChannel (i);

        if (remappedChan >= 0 && remappedChan < numChans)
        {
            buffer.copyFrom (i, 0, *bufferToFill.buffer,
                             remappedChan,
                             bufferToFill.startSample,
                             bufferToFill.numSamples);
        }
        else
        {
            buffer.clear (i, 0, bufferToFill.numSamples);
        }
    }

    remappedInfo.numSamples = bufferToFill.numSamples;

    source->getNextAudioBlock (remappedInfo);

    bufferToFill.clearActiveBufferRegion();

    for (i32 i = 0; i < requiredNumberOfChannels; ++i)
    {
        i32k remappedChan = getRemappedOutputChannel (i);

        if (remappedChan >= 0 && remappedChan < numChans)
        {
            bufferToFill.buffer->addFrom (remappedChan, bufferToFill.startSample,
                                          buffer, i, 0, bufferToFill.numSamples);

        }
    }
}

//==============================================================================
std::unique_ptr<XmlElement> ChannelRemappingAudioSource::createXml() const
{
    auto e = std::make_unique<XmlElement> ("MAPPINGS");
    Txt ins, outs;

    const ScopedLock sl (lock);

    for (i32 i = 0; i < remappedInputs.size(); ++i)
        ins << remappedInputs.getUnchecked (i) << ' ';

    for (i32 i = 0; i < remappedOutputs.size(); ++i)
        outs << remappedOutputs.getUnchecked (i) << ' ';

    e->setAttribute ("inputs", ins.trimEnd());
    e->setAttribute ("outputs", outs.trimEnd());

    return e;
}

z0 ChannelRemappingAudioSource::restoreFromXml (const XmlElement& e)
{
    if (e.hasTagName ("MAPPINGS"))
    {
        const ScopedLock sl (lock);

        clearAllMappings();

        StringArray ins, outs;
        ins.addTokens (e.getStringAttribute ("inputs"), false);
        outs.addTokens (e.getStringAttribute ("outputs"), false);

        for (i32 i = 0; i < ins.size(); ++i)
            remappedInputs.add (ins[i].getIntValue());

        for (i32 i = 0; i < outs.size(); ++i)
            remappedOutputs.add (outs[i].getIntValue());
    }
}

} // namespace drx
