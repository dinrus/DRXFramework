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

struct AudioThumbnail::MinMaxValue
{
    MinMaxValue() noexcept
    {
        values[0] = 0;
        values[1] = 0;
    }

    inline z0 set (const i8 newMin, const i8 newMax) noexcept
    {
        values[0] = newMin;
        values[1] = newMax;
    }

    inline i8 getMinValue() const noexcept        { return values[0]; }
    inline i8 getMaxValue() const noexcept        { return values[1]; }

    inline z0 setFloat (Range<f32> newRange) noexcept
    {
        // Workaround for an ndk armeabi compiler bug which crashes on signed saturation
       #if DRX_ANDROID
        Range<f32> limitedRange (jlimit (-1.0f, 1.0f, newRange.getStart()),
                                   jlimit (-1.0f, 1.0f, newRange.getEnd()));
        values[0] = (i8) (limitedRange.getStart() * 127.0f);
        values[1] = (i8) (limitedRange.getEnd()   * 127.0f);
       #else
        values[0] = (i8) jlimit (-128, 127, roundToInt (newRange.getStart() * 127.0f));
        values[1] = (i8) jlimit (-128, 127, roundToInt (newRange.getEnd()   * 127.0f));
       #endif

        if (values[0] == values[1])
        {
            if (values[1] == 127)
                values[0]--;
            else
                values[1]++;
        }
    }

    inline b8 isNonZero() const noexcept
    {
        return values[1] > values[0];
    }

    inline i32 getPeak() const noexcept
    {
        return jmax (std::abs ((i32) values[0]),
                     std::abs ((i32) values[1]));
    }

    inline z0 read (InputStream& input)      { input.read (values, 2); }
    inline z0 write (OutputStream& output)   { output.write (values, 2); }

private:
    i8 values[2];
};


//==============================================================================
template <typename T>
class AudioBufferReader final : public AudioFormatReader
{
public:
    AudioBufferReader (const AudioBuffer<T>* bufferIn, f64 rate)
        : AudioFormatReader (nullptr, "AudioBuffer"), buffer (bufferIn)
    {
        sampleRate = rate;
        bitsPerSample = 32;
        lengthInSamples = buffer->getNumSamples();
        numChannels = (u32) buffer->getNumChannels();
        usesFloatingPointData = std::is_floating_point_v<T>;
    }

    b8 readSamples (i32* const* destChannels,
                      i32 numDestChannels,
                      i32 startOffsetInDestBuffer,
                      z64 startSampleInFile,
                      i32 numSamples) override
    {
        clearSamplesBeyondAvailableLength (destChannels, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        const auto numAvailableSamples = (i32) ((z64) buffer->getNumSamples() - startSampleInFile);
        const auto numSamplesToCopy = std::clamp (numAvailableSamples, 0, numSamples);

        if (numSamplesToCopy == 0)
            return true;

        for (i32 i = 0; i < numDestChannels; ++i)
        {
            if (uk targetChannel = destChannels[i])
            {
                const auto dest = DestType (targetChannel) + startOffsetInDestBuffer;

                if (i < buffer->getNumChannels())
                    dest.convertSamples (SourceType (buffer->getReadPointer (i) + startSampleInFile), numSamplesToCopy);
                else
                    dest.clearSamples (numSamples);
            }
        }

        return true;
    }

private:
    using SourceNumericalType =
        std::conditional_t<std::is_same_v<T, i32>, AudioData::Int32,
                           std::conditional_t<std::is_same_v<T, f32>, AudioData::Float32, z0>>;

    using DestinationNumericalType = std::conditional_t<std::is_floating_point_v<T>, AudioData::Float32, AudioData::Int32>;

    using DestType   = AudioData::Pointer<DestinationNumericalType, AudioData::LittleEndian, AudioData::NonInterleaved, AudioData::NonConst>;
    using SourceType = AudioData::Pointer<SourceNumericalType,      AudioData::LittleEndian, AudioData::NonInterleaved, AudioData::Const>;

    const AudioBuffer<T>* buffer;
};

//==============================================================================
class AudioThumbnail::LevelDataSource final : public TimeSliceClient
{
public:
    LevelDataSource (AudioThumbnail& thumb, AudioFormatReader* newReader, z64 hash)
        : hashCode (hash), owner (thumb), reader (newReader)
    {
    }

    LevelDataSource (AudioThumbnail& thumb, InputSource* src)
        : hashCode (src->hashCode()), owner (thumb), source (src)
    {
    }

    ~LevelDataSource() override
    {
        owner.cache.getTimeSliceThread().removeTimeSliceClient (this);
    }

    enum { timeBeforeDeletingReader = 3000 };

    z0 initialise (z64 samplesFinished)
    {
        const ScopedLock sl (readerLock);

        numSamplesFinished = samplesFinished;

        createReader();

        if (reader != nullptr)
        {
            lengthInSamples = reader->lengthInSamples;
            numChannels = reader->numChannels;
            sampleRate = reader->sampleRate;

            if (lengthInSamples <= 0 || isFullyLoaded())
                reader.reset();
            else
                owner.cache.getTimeSliceThread().addTimeSliceClient (this);
        }
    }

    z0 getLevels (z64 startSample, i32 numSamples, Array<Range<f32>>& levels)
    {
        const ScopedLock sl (readerLock);

        if (reader == nullptr)
        {
            createReader();

            if (reader != nullptr)
            {
                lastReaderUseTime = Time::getMillisecondCounter();
                owner.cache.getTimeSliceThread().addTimeSliceClient (this);
            }
        }

        if (reader != nullptr)
        {
            if (levels.size() < (i32) reader->numChannels)
                levels.insertMultiple (0, {}, (i32) reader->numChannels - levels.size());

            reader->readMaxLevels (startSample, numSamples, levels.getRawDataPointer(), (i32) reader->numChannels);

            lastReaderUseTime = Time::getMillisecondCounter();
        }
    }

    z0 releaseResources()
    {
        const ScopedLock sl (readerLock);
        reader.reset();
    }

    i32 useTimeSlice() override
    {
        if (isFullyLoaded())
        {
            if (reader != nullptr && source != nullptr)
            {
                if (Time::getMillisecondCounter() > lastReaderUseTime + timeBeforeDeletingReader)
                    releaseResources();
                else
                    return 200;
            }

            return -1;
        }

        b8 justFinished = false;

        {
            const ScopedLock sl (readerLock);
            createReader();

            if (reader != nullptr)
            {
                if (! readNextBlock())
                    return 0;

                justFinished = true;
            }
        }

        if (justFinished)
            owner.cache.storeThumb (owner, hashCode);

        return 200;
    }

    b8 isFullyLoaded() const noexcept
    {
        return numSamplesFinished >= lengthInSamples;
    }

    inline i32 sampleToThumbSample (const z64 originalSample) const noexcept
    {
        return (i32) (originalSample / owner.samplesPerThumbSample);
    }

    z64 lengthInSamples = 0, numSamplesFinished = 0;
    f64 sampleRate = 0;
    u32 numChannels = 0;
    z64 hashCode = 0;

private:
    AudioThumbnail& owner;
    std::unique_ptr<InputSource> source;
    std::unique_ptr<AudioFormatReader> reader;
    CriticalSection readerLock;
    std::atomic<u32> lastReaderUseTime { 0 };

    z0 createReader()
    {
        if (reader == nullptr && source != nullptr)
            if (auto* audioFileStream = source->createInputStream())
                reader.reset (owner.formatManagerToUse.createReaderFor (std::unique_ptr<InputStream> (audioFileStream)));
    }

    b8 readNextBlock()
    {
        jassert (reader != nullptr);

        if (! isFullyLoaded())
        {
            auto numToDo = (i32) jmin (256 * (z64) owner.samplesPerThumbSample, lengthInSamples - numSamplesFinished);

            if (numToDo > 0)
            {
                auto startSample = numSamplesFinished;

                auto firstThumbIndex = sampleToThumbSample (startSample);
                auto lastThumbIndex  = sampleToThumbSample (startSample + numToDo);
                auto numThumbSamps = lastThumbIndex - firstThumbIndex;

                HeapBlock<MinMaxValue> levelData ((u32) numThumbSamps * numChannels);
                HeapBlock<MinMaxValue*> levels (numChannels);

                for (i32 i = 0; i < (i32) numChannels; ++i)
                    levels[i] = levelData + i * numThumbSamps;

                HeapBlock<Range<f32>> levelsRead (numChannels);

                for (i32 i = 0; i < numThumbSamps; ++i)
                {
                    reader->readMaxLevels ((firstThumbIndex + i) * owner.samplesPerThumbSample,
                                           owner.samplesPerThumbSample, levelsRead, (i32) numChannels);

                    for (i32 j = 0; j < (i32) numChannels; ++j)
                        levels[j][i].setFloat (levelsRead[j]);
                }

                {
                    const ScopedUnlock su (readerLock);
                    owner.setLevels (levels, firstThumbIndex, (i32) numChannels, numThumbSamps);
                }

                numSamplesFinished += numToDo;
                lastReaderUseTime = Time::getMillisecondCounter();
            }
        }

        return isFullyLoaded();
    }
};

//==============================================================================
class AudioThumbnail::ThumbData
{
public:
    ThumbData (i32 numThumbSamples)
    {
        ensureSize (numThumbSamples);
    }

    inline MinMaxValue* getData (i32 thumbSampleIndex) noexcept
    {
        jassert (thumbSampleIndex < data.size());
        return data.getRawDataPointer() + thumbSampleIndex;
    }

    i32 getSize() const noexcept
    {
        return data.size();
    }

    z0 getMinMax (i32 startSample, i32 endSample, MinMaxValue& result) const noexcept
    {
        if (startSample >= 0)
        {
            endSample = jmin (endSample, data.size() - 1);

            i8 mx = -128;
            i8 mn = 127;

            while (startSample <= endSample)
            {
                auto& v = data.getReference (startSample);

                if (v.getMinValue() < mn)  mn = v.getMinValue();
                if (v.getMaxValue() > mx)  mx = v.getMaxValue();

                ++startSample;
            }

            if (mn <= mx)
            {
                result.set (mn, mx);
                return;
            }
        }

        result.set (1, 0);
    }

    z0 write (const MinMaxValue* values, i32 startIndex, i32 numValues)
    {
        resetPeak();

        if (startIndex + numValues > data.size())
            ensureSize (startIndex + numValues);

        auto* dest = getData (startIndex);

        for (i32 i = 0; i < numValues; ++i)
            dest[i] = values[i];
    }

    z0 resetPeak() noexcept
    {
        peakLevel = -1;
    }

    i32 getPeak() noexcept
    {
        if (peakLevel < 0)
        {
            for (auto& s : data)
            {
                auto peak = s.getPeak();

                if (peak > peakLevel)
                    peakLevel = peak;
            }
        }

        return peakLevel;
    }

private:
    Array<MinMaxValue> data;
    i32 peakLevel = -1;

    z0 ensureSize (i32 thumbSamples)
    {
        auto extraNeeded = thumbSamples - data.size();

        if (extraNeeded > 0)
            data.insertMultiple (-1, MinMaxValue(), extraNeeded);
    }
};

//==============================================================================
class AudioThumbnail::CachedWindow
{
public:
    CachedWindow() {}

    z0 invalidate()
    {
        cacheNeedsRefilling = true;
    }

    z0 drawChannel (Graphics& g, const Rectangle<i32>& area,
                      const f64 startTime, const f64 endTime,
                      i32k channelNum, const f32 verticalZoomFactor,
                      const f64 rate, i32k numChans, i32k sampsPerThumbSample,
                      LevelDataSource* levelData, const OwnedArray<ThumbData>& chans)
    {
        if (refillCache (area.getWidth(), startTime, endTime, rate,
                         numChans, sampsPerThumbSample, levelData, chans)
             && isPositiveAndBelow (channelNum, numChannelsCached))
        {
            auto clip = g.getClipBounds().getIntersection (area.withWidth (jmin (numSamplesCached, area.getWidth())));

            if (! clip.isEmpty())
            {
                auto topY = (f32) area.getY();
                auto bottomY = (f32) area.getBottom();
                auto midY = (topY + bottomY) * 0.5f;
                auto vscale = verticalZoomFactor * (bottomY - topY) / 256.0f;

                auto* cacheData = getData (channelNum, clip.getX() - area.getX());

                RectangleList<f32> waveform;
                waveform.ensureStorageAllocated (clip.getWidth());

                auto x = (f32) clip.getX();

                for (i32 w = clip.getWidth(); --w >= 0;)
                {
                    if (cacheData->isNonZero())
                    {
                        auto top    = jmax (midY - cacheData->getMaxValue() * vscale - 0.3f, topY);
                        auto bottom = jmin (midY - cacheData->getMinValue() * vscale + 0.3f, bottomY);

                        waveform.addWithoutMerging (Rectangle<f32> (x, top, 1.0f, bottom - top));
                    }

                    x += 1.0f;
                    ++cacheData;
                }

                g.fillRectList (waveform);
            }
        }
    }

private:
    Array<MinMaxValue> data;
    f64 cachedStart = 0, cachedTimePerPixel = 0;
    i32 numChannelsCached = 0, numSamplesCached = 0;
    b8 cacheNeedsRefilling = true;

    b8 refillCache (i32 numSamples, f64 startTime, f64 endTime,
                      f64 rate, i32 numChans, i32 sampsPerThumbSample,
                      LevelDataSource* levelData, const OwnedArray<ThumbData>& chans)
    {
        auto timePerPixel = (endTime - startTime) / numSamples;

        if (numSamples <= 0 || timePerPixel <= 0.0 || rate <= 0)
        {
            invalidate();
            return false;
        }

        if (numSamples == numSamplesCached
             && numChannelsCached == numChans
             && approximatelyEqual (startTime, cachedStart)
             && approximatelyEqual (timePerPixel, cachedTimePerPixel)
             && ! cacheNeedsRefilling)
        {
            return ! cacheNeedsRefilling;
        }

        numSamplesCached = numSamples;
        numChannelsCached = numChans;
        cachedStart = startTime;
        cachedTimePerPixel = timePerPixel;
        cacheNeedsRefilling = false;

        ensureSize (numSamples);

        if (timePerPixel * rate <= sampsPerThumbSample && levelData != nullptr)
        {
            auto sample = roundToInt (startTime * rate);
            Array<Range<f32>> levels;

            i32 i;
            for (i = 0; i < numSamples; ++i)
            {
                auto nextSample = roundToInt ((startTime + timePerPixel) * rate);

                if (sample >= 0)
                {
                    if (sample >= levelData->lengthInSamples)
                    {
                        for (i32 chan = 0; chan < numChannelsCached; ++chan)
                            *getData (chan, i) = MinMaxValue();
                    }
                    else
                    {
                        levelData->getLevels (sample, jmax (1, nextSample - sample), levels);

                        auto totalChans = jmin (levels.size(), numChannelsCached);

                        for (i32 chan = 0; chan < totalChans; ++chan)
                            getData (chan, i)->setFloat (levels.getReference (chan));
                    }
                }

                startTime += timePerPixel;
                sample = nextSample;
            }

            numSamplesCached = i;
        }
        else
        {
            jassert (chans.size() == numChannelsCached);

            for (i32 channelNum = 0; channelNum < numChannelsCached; ++channelNum)
            {
                ThumbData* channelData = chans.getUnchecked (channelNum);
                MinMaxValue* cacheData = getData (channelNum, 0);

                auto timeToThumbSampleFactor = rate / (f64) sampsPerThumbSample;

                startTime = cachedStart;
                auto sample = roundToInt (startTime * timeToThumbSampleFactor);

                for (i32 i = numSamples; --i >= 0;)
                {
                    auto nextSample = roundToInt ((startTime + timePerPixel) * timeToThumbSampleFactor);

                    channelData->getMinMax (sample, nextSample, *cacheData);

                    ++cacheData;
                    startTime += timePerPixel;
                    sample = nextSample;
                }
            }
        }

        return true;
    }

    MinMaxValue* getData (i32k channelNum, i32k cacheIndex) noexcept
    {
        jassert (isPositiveAndBelow (channelNum, numChannelsCached) && isPositiveAndBelow (cacheIndex, data.size()));

        return data.getRawDataPointer() + channelNum * numSamplesCached
                                        + cacheIndex;
    }

    z0 ensureSize (i32k numSamples)
    {
        auto itemsRequired = numSamples * numChannelsCached;

        if (data.size() < itemsRequired)
            data.insertMultiple (-1, MinMaxValue(), itemsRequired - data.size());
    }
};

//==============================================================================
AudioThumbnail::AudioThumbnail (i32k originalSamplesPerThumbnailSample,
                                AudioFormatManager& formatManager,
                                AudioThumbnailCache& cacheToUse)
    : formatManagerToUse (formatManager),
      cache (cacheToUse),
      window (new CachedWindow()),
      samplesPerThumbSample (originalSamplesPerThumbnailSample)
{
}

AudioThumbnail::~AudioThumbnail()
{
    clear();
}

z0 AudioThumbnail::clear()
{
    source.reset();
    const ScopedLock sl (lock);
    clearChannelData();
}

z0 AudioThumbnail::clearChannelData()
{
    window->invalidate();
    channels.clear();
    totalSamples = numSamplesFinished = 0;
    numChannels = 0;
    sampleRate = 0;

    sendChangeMessage();
}

z0 AudioThumbnail::reset (i32 newNumChannels, f64 newSampleRate, z64 totalSamplesInSource)
{
    clear();

    const ScopedLock sl (lock);
    numChannels = newNumChannels;
    sampleRate = newSampleRate;
    totalSamples = totalSamplesInSource;

    createChannels (1 + (i32) (totalSamplesInSource / samplesPerThumbSample));
}

z0 AudioThumbnail::createChannels (i32k length)
{
    while (channels.size() < numChannels)
        channels.add (new ThumbData (length));
}

//==============================================================================
b8 AudioThumbnail::loadFrom (InputStream& rawInput)
{
    BufferedInputStream input (rawInput, 4096);

    if (input.readByte() != 'j' || input.readByte() != 'a' || input.readByte() != 't' || input.readByte() != 'm')
        return false;

    const ScopedLock sl (lock);
    clearChannelData();

    samplesPerThumbSample = input.readInt();
    totalSamples = input.readInt64();             // Total number of source samples.
    numSamplesFinished = input.readInt64();       // Number of valid source samples that have been read into the thumbnail.
    i32 numThumbnailSamples = input.readInt();  // Number of samples in the thumbnail data.
    numChannels = input.readInt();                // Number of audio channels.
    sampleRate = input.readInt();                 // Source sample rate.
    input.skipNextBytes (16);                     // (reserved)

    createChannels (numThumbnailSamples);

    for (i32 i = 0; i < numThumbnailSamples; ++i)
        for (i32 chan = 0; chan < numChannels; ++chan)
            channels.getUnchecked (chan)->getData (i)->read (input);

    return true;
}

z0 AudioThumbnail::saveTo (OutputStream& output) const
{
    const ScopedLock sl (lock);

    i32k numThumbnailSamples = channels.size() == 0 ? 0 : channels.getUnchecked (0)->getSize();

    output.write ("jatm", 4);
    output.writeInt (samplesPerThumbSample);
    output.writeInt64 (totalSamples);
    output.writeInt64 (numSamplesFinished);
    output.writeInt (numThumbnailSamples);
    output.writeInt (numChannels);
    output.writeInt ((i32) sampleRate);
    output.writeInt64 (0);
    output.writeInt64 (0);

    for (i32 i = 0; i < numThumbnailSamples; ++i)
        for (i32 chan = 0; chan < numChannels; ++chan)
            channels.getUnchecked (chan)->getData (i)->write (output);
}

//==============================================================================
b8 AudioThumbnail::setDataSource (LevelDataSource* newSource)
{
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    numSamplesFinished = 0;
    auto wasSuccessful = [&] { return sampleRate > 0 && totalSamples > 0; };

    if (cache.loadThumb (*this, newSource->hashCode) && isFullyLoaded())
    {
        source.reset (newSource); // (make sure this isn't done before loadThumb is called)

        source->lengthInSamples = totalSamples;
        source->sampleRate = sampleRate;
        source->numChannels = (u32) numChannels;
        source->numSamplesFinished = numSamplesFinished;

        return wasSuccessful();
    }

    source.reset (newSource);

    const ScopedLock sl (lock);
    source->initialise (numSamplesFinished);

    totalSamples = source->lengthInSamples;
    sampleRate = source->sampleRate;
    numChannels = (i32) source->numChannels;

    createChannels (1 + (i32) (totalSamples / samplesPerThumbSample));

    return wasSuccessful();
}

b8 AudioThumbnail::setSource (InputSource* const newSource)
{
    clear();

    return newSource != nullptr && setDataSource (new LevelDataSource (*this, newSource));
}

z0 AudioThumbnail::setReader (AudioFormatReader* newReader, z64 hash)
{
    clear();

    if (newReader != nullptr)
        setDataSource (new LevelDataSource (*this, newReader, hash));
}

z0 AudioThumbnail::setSource (const AudioBuffer<f32>* newSource, f64 rate, z64 hash)
{
    setReader (new AudioBufferReader<f32> (newSource, rate), hash);
}

z0 AudioThumbnail::setSource (const AudioBuffer<i32>* newSource, f64 rate, z64 hash)
{
    setReader (new AudioBufferReader<i32> (newSource, rate), hash);
}

z64 AudioThumbnail::getHashCode() const
{
    return source == nullptr ? 0 : source->hashCode;
}

z0 AudioThumbnail::addBlock (z64 startSample, const AudioBuffer<f32>& incoming,
                               i32 startOffsetInBuffer, i32 numSamples)
{
    jassert (startSample >= 0
              && startOffsetInBuffer >= 0
              && startOffsetInBuffer + numSamples <= incoming.getNumSamples());

    auto firstThumbIndex = (i32) (startSample / samplesPerThumbSample);
    auto lastThumbIndex  = (i32) ((startSample + numSamples + (samplesPerThumbSample - 1)) / samplesPerThumbSample);
    auto numToDo = lastThumbIndex - firstThumbIndex;

    if (numToDo > 0)
    {
        auto numChans = jmin (channels.size(), incoming.getNumChannels());

        const HeapBlock<MinMaxValue> thumbData (numToDo * numChans);
        const HeapBlock<MinMaxValue*> thumbChannels (numChans);

        for (i32 chan = 0; chan < numChans; ++chan)
        {
            auto* sourceData = incoming.getReadPointer (chan, startOffsetInBuffer);
            auto* dest = thumbData + numToDo * chan;
            thumbChannels [chan] = dest;

            for (i32 i = 0; i < numToDo; ++i)
            {
                auto start = i * samplesPerThumbSample;
                dest[i].setFloat (FloatVectorOperations::findMinAndMax (sourceData + start, jmin (samplesPerThumbSample, numSamples - start)));
            }
        }

        setLevels (thumbChannels, firstThumbIndex, numChans, numToDo);
    }
}

z0 AudioThumbnail::setLevels (const MinMaxValue* const* values, i32 thumbIndex, i32 numChans, i32 numValues)
{
    const ScopedLock sl (lock);

    for (i32 i = jmin (numChans, channels.size()); --i >= 0;)
        channels.getUnchecked (i)->write (values[i], thumbIndex, numValues);

    auto start = thumbIndex * (z64) samplesPerThumbSample;
    auto end   = (thumbIndex + numValues) * (z64) samplesPerThumbSample;

    if (numSamplesFinished >= start && end > numSamplesFinished)
        numSamplesFinished = end;

    totalSamples = jmax (numSamplesFinished, totalSamples);
    window->invalidate();
    sendChangeMessage();
}

//==============================================================================
i32 AudioThumbnail::getNumChannels() const noexcept
{
    const ScopedLock sl (lock);
    return numChannels;
}

f64 AudioThumbnail::getTotalLength() const noexcept
{
    const ScopedLock sl (lock);
    return sampleRate > 0 ? ((f64) totalSamples / sampleRate) : 0.0;
}

b8 AudioThumbnail::isFullyLoaded() const noexcept
{
    const ScopedLock sl (lock);
    return numSamplesFinished >= totalSamples - samplesPerThumbSample;
}

f64 AudioThumbnail::getProportionComplete() const noexcept
{
    const ScopedLock sl (lock);
    return jlimit (0.0, 1.0, (f64) numSamplesFinished / (f64) jmax ((z64) 1, totalSamples));
}

z64 AudioThumbnail::getNumSamplesFinished() const noexcept
{
    const ScopedLock sl (lock);
    return numSamplesFinished;
}

f32 AudioThumbnail::getApproximatePeak() const
{
    const ScopedLock sl (lock);
    i32 peak = 0;

    for (auto* c : channels)
        peak = jmax (peak, c->getPeak());

    return (f32) jlimit (0, 127, peak) / 127.0f;
}

z0 AudioThumbnail::getApproximateMinMax (f64 startTime, f64 endTime, i32 channelIndex,
                                           f32& minValue, f32& maxValue) const noexcept
{
    const ScopedLock sl (lock);
    MinMaxValue result;
    auto* data = channels [channelIndex];

    if (data != nullptr && sampleRate > 0)
    {
        auto firstThumbIndex = (i32) ((startTime * sampleRate) / samplesPerThumbSample);
        auto lastThumbIndex  = (i32) (((endTime * sampleRate) + samplesPerThumbSample - 1) / samplesPerThumbSample);

        data->getMinMax (jmax (0, firstThumbIndex), lastThumbIndex, result);
    }

    minValue = result.getMinValue() / 128.0f;
    maxValue = result.getMaxValue() / 128.0f;
}

z0 AudioThumbnail::drawChannel (Graphics& g, const Rectangle<i32>& area, f64 startTime,
                                  f64 endTime, i32 channelNum, f32 verticalZoomFactor)
{
    const ScopedLock sl (lock);

    window->drawChannel (g, area, startTime, endTime, channelNum, verticalZoomFactor,
                         sampleRate, numChannels, samplesPerThumbSample, source.get(), channels);
}

z0 AudioThumbnail::drawChannels (Graphics& g, const Rectangle<i32>& area, f64 startTimeSeconds,
                                   f64 endTimeSeconds, f32 verticalZoomFactor)
{
    for (i32 i = 0; i < numChannels; ++i)
    {
        auto y1 = roundToInt ((i * area.getHeight()) / numChannels);
        auto y2 = roundToInt (((i + 1) * area.getHeight()) / numChannels);

        drawChannel (g, { area.getX(), area.getY() + y1, area.getWidth(), y2 - y1 },
                     startTimeSeconds, endTimeSeconds, i, verticalZoomFactor);
    }
}

} // namespace drx
