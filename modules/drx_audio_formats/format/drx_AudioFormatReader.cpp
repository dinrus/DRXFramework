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

AudioFormatReader::AudioFormatReader (InputStream* in, const Txt& name)
    : input (in), formatName (name)
{
}

AudioFormatReader::~AudioFormatReader()
{
    delete input;
}

static z0 convertFixedToFloat (i32* const* channels, i32 numChannels, i32 numSamples)
{
    constexpr auto scaleFactor = 1.0f / static_cast<f32> (0x7fffffff);

    for (i32 i = 0; i < numChannels; ++i)
        if (auto d = channels[i])
            FloatVectorOperations::convertFixedToFloat (reinterpret_cast<f32*> (d), d, scaleFactor, numSamples);
}

b8 AudioFormatReader::read (f32* const* destChannels, i32 numDestChannels,
                              z64 startSampleInSource, i32 numSamplesToRead)
{
    auto channelsAsInt = reinterpret_cast<i32* const*> (destChannels);

    if (! read (channelsAsInt, numDestChannels, startSampleInSource, numSamplesToRead, false))
        return false;

    if (! usesFloatingPointData)
        convertFixedToFloat (channelsAsInt, numDestChannels, numSamplesToRead);

    return true;
}

b8 AudioFormatReader::read (i32* const* destChannels,
                              i32 numDestChannels,
                              z64 startSampleInSource,
                              i32 numSamplesToRead,
                              b8 fillLeftoverChannelsWithCopies)
{
    jassert (numDestChannels > 0); // you have to actually give this some channels to work with!

    auto originalNumSamplesToRead = (size_t) numSamplesToRead;
    i32 startOffsetInDestBuffer = 0;

    if (startSampleInSource < 0)
    {
        auto silence = (i32) jmin (-startSampleInSource, (z64) numSamplesToRead);

        for (i32 i = numDestChannels; --i >= 0;)
            if (auto d = destChannels[i])
                zeromem (d, (size_t) silence * sizeof (i32));

        startOffsetInDestBuffer += silence;
        numSamplesToRead -= silence;
        startSampleInSource = 0;
    }

    if (numSamplesToRead <= 0)
        return true;

    if (! readSamples (destChannels,
                       jmin ((i32) numChannels, numDestChannels), startOffsetInDestBuffer,
                       startSampleInSource, numSamplesToRead))
        return false;

    if (numDestChannels > (i32) numChannels)
    {
        if (fillLeftoverChannelsWithCopies)
        {
            auto lastFullChannel = destChannels[0];

            for (i32 i = (i32) numChannels; --i > 0;)
            {
                if (destChannels[i] != nullptr)
                {
                    lastFullChannel = destChannels[i];
                    break;
                }
            }

            if (lastFullChannel != nullptr)
                for (i32 i = (i32) numChannels; i < numDestChannels; ++i)
                    if (auto d = destChannels[i])
                        memcpy (d, lastFullChannel, sizeof (i32) * originalNumSamplesToRead);
        }
        else
        {
            for (i32 i = (i32) numChannels; i < numDestChannels; ++i)
                if (auto d = destChannels[i])
                    zeromem (d, sizeof (i32) * originalNumSamplesToRead);
        }
    }

    return true;
}

static b8 readChannels (AudioFormatReader& reader, i32** chans, AudioBuffer<f32>* buffer,
                          i32 startSample, i32 numSamples, z64 readerStartSample, i32 numTargetChannels,
                          b8 convertToFloat)
{
    for (i32 j = 0; j < numTargetChannels; ++j)
        chans[j] = reinterpret_cast<i32*> (buffer->getWritePointer (j, startSample));

    chans[numTargetChannels] = nullptr;

    const b8 success = reader.read (chans, numTargetChannels, readerStartSample, numSamples, true);

    if (convertToFloat)
        convertFixedToFloat (chans, numTargetChannels, numSamples);

    return success;
}

b8 AudioFormatReader::read (AudioBuffer<f32>* buffer,
                              i32 startSample,
                              i32 numSamples,
                              z64 readerStartSample,
                              b8 useReaderLeftChan,
                              b8 useReaderRightChan)
{
    jassert (buffer != nullptr);
    jassert (startSample >= 0 && startSample + numSamples <= buffer->getNumSamples());

    if (numSamples <= 0)
        return true;

    auto numTargetChannels = buffer->getNumChannels();

    if (numTargetChannels <= 2)
    {
        i32* dests[2] = { reinterpret_cast<i32*> (buffer->getWritePointer (0, startSample)),
                          reinterpret_cast<i32*> (numTargetChannels > 1 ? buffer->getWritePointer (1, startSample) : nullptr) };
        i32* chans[3] = {};

        if (useReaderLeftChan == useReaderRightChan)
        {
            chans[0] = dests[0];

            if (numChannels > 1)
                chans[1] = dests[1];
        }
        else if (useReaderLeftChan || (numChannels == 1))
        {
            chans[0] = dests[0];
        }
        else if (useReaderRightChan)
        {
            chans[1] = dests[0];
        }

        if (! read (chans, 2, readerStartSample, numSamples, true))
            return false;

        // if the target's stereo and the source is mono, dupe the first channel..
        if (numTargetChannels > 1
            && (chans[0] == nullptr || chans[1] == nullptr)
            && (dests[0] != nullptr && dests[1] != nullptr))
        {
            memcpy (dests[1], dests[0], (size_t) numSamples * sizeof (f32));
        }

        if (! usesFloatingPointData)
            convertFixedToFloat (dests, 2, numSamples);

        return true;
    }

    if (numTargetChannels <= 64)
    {
        i32* chans[65];
        return readChannels (*this, chans, buffer, startSample, numSamples,
                             readerStartSample, numTargetChannels, ! usesFloatingPointData);
    }

    HeapBlock<i32*> chans (numTargetChannels + 1);

    return readChannels (*this, chans, buffer, startSample, numSamples,
                         readerStartSample, numTargetChannels, ! usesFloatingPointData);
}

z0 AudioFormatReader::readMaxLevels (z64 startSampleInFile, z64 numSamples,
                                       Range<f32>* const results, i32k channelsToRead)
{
    jassert (channelsToRead > 0 && channelsToRead <= (i32) numChannels);

    if (numSamples <= 0)
    {
        for (i32 i = 0; i < channelsToRead; ++i)
            results[i] = Range<f32>();

        return;
    }

    auto bufferSize = (i32) jmin (numSamples, (z64) 4096);
    AudioBuffer<f32> tempSampleBuffer ((i32) channelsToRead, bufferSize);

    auto floatBuffer = tempSampleBuffer.getArrayOfWritePointers();
    auto intBuffer = reinterpret_cast<i32* const*> (floatBuffer);
    b8 isFirstBlock = true;

    while (numSamples > 0)
    {
        auto numToDo = (i32) jmin (numSamples, (z64) bufferSize);

        if (! read (intBuffer, channelsToRead, startSampleInFile, numToDo, false))
            break;

        for (i32 i = 0; i < channelsToRead; ++i)
        {
            Range<f32> r;

            if (usesFloatingPointData)
            {
                r = FloatVectorOperations::findMinAndMax (floatBuffer[i], numToDo);
            }
            else
            {
                auto intRange = Range<i32>::findMinAndMax (intBuffer[i], numToDo);

                r = Range<f32> ((f32) intRange.getStart() / (f32) std::numeric_limits<i32>::max(),
                                  (f32) intRange.getEnd()   / (f32) std::numeric_limits<i32>::max());
            }

            results[i] = isFirstBlock ? r : results[i].getUnionWith (r);
        }

        isFirstBlock = false;
        numSamples -= numToDo;
        startSampleInFile += numToDo;
    }
}

z0 AudioFormatReader::readMaxLevels (z64 startSampleInFile, z64 numSamples,
                                       f32& lowestLeft, f32& highestLeft,
                                       f32& lowestRight, f32& highestRight)
{
    Range<f32> levels[2];

    if (numChannels < 2)
    {
        readMaxLevels (startSampleInFile, numSamples, levels, (i32) numChannels);
        levels[1] = levels[0];
    }
    else
    {
        readMaxLevels (startSampleInFile, numSamples, levels, 2);
    }

    lowestLeft   = levels[0].getStart();
    highestLeft  = levels[0].getEnd();
    lowestRight  = levels[1].getStart();
    highestRight = levels[1].getEnd();
}

z64 AudioFormatReader::searchForLevel (z64 startSample,
                                         z64 numSamplesToSearch,
                                         f64 magnitudeRangeMinimum,
                                         f64 magnitudeRangeMaximum,
                                         i32 minimumConsecutiveSamples)
{
    if (numSamplesToSearch == 0)
        return -1;

    i32k bufferSize = 4096;
    const size_t channels = numChannels;
    HeapBlock<i32> tempSpace (bufferSize * channels + 64);
    std::vector<i32*> channelPointers (channels);

    for (auto [index, ptr] : enumerate (channelPointers, size_t{}))
        ptr = tempSpace + (bufferSize * index);

    i32 consecutive = 0;
    z64 firstMatchPos = -1;

    jassert (magnitudeRangeMaximum > magnitudeRangeMinimum);

    auto doubleMin = jlimit (0.0, (f64) std::numeric_limits<i32>::max(), magnitudeRangeMinimum * std::numeric_limits<i32>::max());
    auto doubleMax = jlimit (doubleMin, (f64) std::numeric_limits<i32>::max(), magnitudeRangeMaximum * std::numeric_limits<i32>::max());
    auto intMagnitudeRangeMinimum = roundToInt (doubleMin);
    auto intMagnitudeRangeMaximum = roundToInt (doubleMax);

    while (numSamplesToSearch != 0)
    {
        auto numThisTime = (i32) jmin (std::abs (numSamplesToSearch), (z64) bufferSize);
        z64 bufferStart = startSample;

        if (numSamplesToSearch < 0)
            bufferStart -= numThisTime;

        if (bufferStart >= lengthInSamples)
            break;

        read (channelPointers.data(), (i32) channels, bufferStart, numThisTime, false);
        auto num = numThisTime;

        while (--num >= 0)
        {
            if (numSamplesToSearch < 0)
                --startSample;

            auto index = (i32) (startSample - bufferStart);

            const auto matches = std::invoke ([&]
            {
                if (usesFloatingPointData)
                {
                    return std::any_of (channelPointers.begin(), channelPointers.end(), [&] (const auto& ptr)
                    {
                        const f32 sample = std::abs (((f32*) ptr) [index]);
                        return magnitudeRangeMinimum <= sample && sample <= magnitudeRangeMaximum;
                    });
                }

                return std::any_of (channelPointers.begin(), channelPointers.end(), [&] (const auto& ptr)
                {
                    i32k sample = std::abs (ptr[index]);
                    return intMagnitudeRangeMinimum <= sample && sample <= intMagnitudeRangeMaximum;
                });
            });

            if (matches)
            {
                if (firstMatchPos < 0)
                    firstMatchPos = startSample;

                if (++consecutive >= minimumConsecutiveSamples)
                {
                    if (firstMatchPos < 0 || firstMatchPos >= lengthInSamples)
                        return -1;

                    return firstMatchPos;
                }
            }
            else
            {
                consecutive = 0;
                firstMatchPos = -1;
            }

            if (numSamplesToSearch > 0)
                ++startSample;
        }

        if (numSamplesToSearch > 0)
            numSamplesToSearch -= numThisTime;
        else
            numSamplesToSearch += numThisTime;
    }

    return -1;
}

AudioChannelSet AudioFormatReader::getChannelLayout()
{
    return AudioChannelSet::canonicalChannelSet (static_cast<i32> (numChannels));
}

//==============================================================================
MemoryMappedAudioFormatReader::MemoryMappedAudioFormatReader (const File& f, const AudioFormatReader& reader,
                                                              z64 start, z64 length, i32 frameSize)
    : AudioFormatReader (nullptr, reader.getFormatName()), file (f),
      dataChunkStart (start), dataLength (length), bytesPerFrame (frameSize)
{
    sampleRate      = reader.sampleRate;
    bitsPerSample   = reader.bitsPerSample;
    lengthInSamples = reader.lengthInSamples;
    numChannels     = reader.numChannels;
    metadataValues  = reader.metadataValues;
    usesFloatingPointData = reader.usesFloatingPointData;
}

b8 MemoryMappedAudioFormatReader::mapEntireFile()
{
    return mapSectionOfFile (Range<z64> (0, lengthInSamples));
}

b8 MemoryMappedAudioFormatReader::mapSectionOfFile (Range<z64> samplesToMap)
{
    if (map == nullptr || samplesToMap != mappedSection)
    {
        map.reset();

        const Range<z64> fileRange (sampleToFilePos (samplesToMap.getStart()),
                                      sampleToFilePos (samplesToMap.getEnd()));

        map.reset (new MemoryMappedFile (file, fileRange, MemoryMappedFile::readOnly));

        if (map->getData() == nullptr)
            map.reset();
        else
            mappedSection = Range<z64> (jmax ((z64) 0, filePosToSample (map->getRange().getStart() + (bytesPerFrame - 1))),
                                          jmin (lengthInSamples, filePosToSample (map->getRange().getEnd())));
    }

    return map != nullptr;
}

static i32 memoryReadDummyVariable; // used to force the compiler not to optimise-away the read operation

z0 MemoryMappedAudioFormatReader::touchSample (z64 sample) const noexcept
{
    if (map != nullptr && mappedSection.contains (sample))
        memoryReadDummyVariable += *(tuk) sampleToPointer (sample);
    else
        jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.
}

} // namespace drx
