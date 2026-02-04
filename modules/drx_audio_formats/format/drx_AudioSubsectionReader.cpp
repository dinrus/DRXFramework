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

AudioSubsectionReader::AudioSubsectionReader (AudioFormatReader* sourceToUse,
                                              z64 startSampleToUse, z64 lengthToUse,
                                              b8 deleteSource)
   : AudioFormatReader (nullptr, sourceToUse->getFormatName()),
     source (sourceToUse),
     startSample (startSampleToUse),
     deleteSourceWhenDeleted (deleteSource)
{
    length = jmin (jmax ((z64) 0, source->lengthInSamples - startSample), lengthToUse);

    sampleRate = source->sampleRate;
    bitsPerSample = source->bitsPerSample;
    lengthInSamples = length;
    numChannels = source->numChannels;
    usesFloatingPointData = source->usesFloatingPointData;
}

AudioSubsectionReader::~AudioSubsectionReader()
{
    if (deleteSourceWhenDeleted)
        delete source;
}

//==============================================================================
b8 AudioSubsectionReader::readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                                         z64 startSampleInFile, i32 numSamples)
{
    clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                       startSampleInFile, numSamples, length);

    if (numSamples <= 0)
        return true;

    return source->readSamples (destSamples, numDestChannels, startOffsetInDestBuffer,
                                startSampleInFile + startSample, numSamples);
}

z0 AudioSubsectionReader::readMaxLevels (z64 startSampleInFile, z64 numSamples, Range<f32>* results, i32 numChannelsToRead)
{
    startSampleInFile = jmax ((z64) 0, startSampleInFile);
    numSamples = jmax ((z64) 0, jmin (numSamples, length - startSampleInFile));

    source->readMaxLevels (startSampleInFile + startSample, numSamples, results, numChannelsToRead);
}

} // namespace drx
