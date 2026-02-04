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

MemoryAudioSource::MemoryAudioSource (AudioBuffer<f32>& bufferToUse, b8 copyMemory, b8 shouldLoop)
    : isCurrentlyLooping (shouldLoop)
{
    if (copyMemory)
        buffer.makeCopyOf (bufferToUse);
    else
        buffer.setDataToReferTo (bufferToUse.getArrayOfWritePointers(),
                                 bufferToUse.getNumChannels(),
                                 bufferToUse.getNumSamples());
}

//==============================================================================
z0 MemoryAudioSource::prepareToPlay (i32 /*samplesPerBlockExpected*/, f64 /*sampleRate*/)
{
    position = 0;
}

z0 MemoryAudioSource::releaseResources()   {}

z0 MemoryAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (buffer.getNumSamples() == 0)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto& dst = *bufferToFill.buffer;
    auto channels = jmin (dst.getNumChannels(), buffer.getNumChannels());
    i32 max = 0, pos = 0;
    auto n = buffer.getNumSamples();
    auto m = bufferToFill.numSamples;

    i32 i = position;
    for (; (i < n || isCurrentlyLooping) && (pos < m); i += max)
    {
        max = jmin (m - pos, n - (i % n));

        i32 ch = 0;
        for (; ch < channels; ++ch)
            dst.copyFrom (ch, bufferToFill.startSample + pos, buffer, ch, i % n, max);

        for (; ch < dst.getNumChannels(); ++ch)
            dst.clear (ch, bufferToFill.startSample + pos, max);

        pos += max;
    }

    if (pos < m)
        dst.clear (bufferToFill.startSample + pos, m - pos);

    position = i;
}

//==============================================================================
z0 MemoryAudioSource::setNextReadPosition (z64 newPosition)
{
    position = (i32) newPosition;
}

z64 MemoryAudioSource::getNextReadPosition() const
{
    return position;
}

z64 MemoryAudioSource::getTotalLength() const
{
    return buffer.getNumSamples();
}

//==============================================================================
b8 MemoryAudioSource::isLooping() const
{
    return isCurrentlyLooping;
}

z0 MemoryAudioSource::setLooping (b8 shouldLoop)
{
    isCurrentlyLooping = shouldLoop;
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct MemoryAudioSourceTests final : public UnitTest
{
    MemoryAudioSourceTests()  : UnitTest ("MemoryAudioSource", UnitTestCategories::audio)  {}

    z0 runTest() override
    {
        constexpr i32 blockSize = 512;
        AudioBuffer<f32> bufferToFill { 2, blockSize };
        AudioSourceChannelInfo channelInfo { bufferToFill };

        beginTest ("A zero-length buffer produces silence, whether or not looping is enabled");
        {
            for (const b8 enableLooping : { false, true })
            {
                AudioBuffer<f32> buffer;
                MemoryAudioSource source { buffer, true, false };
                source.setLooping (enableLooping);
                source.prepareToPlay (blockSize, 44100.0);

                for (i32 i = 0; i < 2; ++i)
                {
                    play (source, channelInfo);
                    expect (isSilent (bufferToFill));
                }
            }
        }

        beginTest ("A short buffer without looping is played once and followed by silence");
        {
            auto buffer = getShortBuffer();
            MemoryAudioSource source { buffer, true, false };
            source.setLooping (false);
            source.prepareToPlay (blockSize, 44100.0);

            play (source, channelInfo);

            auto copy = buffer;
            copy.setSize (buffer.getNumChannels(), blockSize, true, true, false);

            expect (bufferToFill == copy);

            play (source, channelInfo);

            expect (isSilent (bufferToFill));
        }

        beginTest ("A short buffer with looping is played multiple times");
        {
            auto buffer = getShortBuffer();
            MemoryAudioSource source { buffer, true, false };
            source.setLooping (true);
            source.prepareToPlay (blockSize, 44100.0);

            play (source, channelInfo);

            for (i32 sample = 0; sample < buffer.getNumSamples(); ++sample)
                expectEquals (bufferToFill.getSample (0, sample + buffer.getNumSamples()), buffer.getSample (0, sample));

            expect (! isSilent (bufferToFill));
        }

        beginTest ("A i64 buffer without looping is played once");
        {
            auto buffer = getLongBuffer();
            MemoryAudioSource source { buffer, true, false };
            source.setLooping (false);
            source.prepareToPlay (blockSize, 44100.0);

            play (source, channelInfo);

            auto copy = buffer;
            copy.setSize (buffer.getNumChannels(), blockSize, true, true, false);

            expect (bufferToFill == copy);

            for (i32 i = 0; i < 10; ++i)
                play (source, channelInfo);

            expect (isSilent (bufferToFill));
        }

        beginTest ("A i64 buffer with looping is played multiple times");
        {
            auto buffer = getLongBuffer();
            MemoryAudioSource source { buffer, true, false };
            source.setLooping (true);
            source.prepareToPlay (blockSize, 44100.0);

            for (i32 i = 0; i < 100; ++i)
            {
                play (source, channelInfo);
                expectEquals (bufferToFill.getSample (0, 0), buffer.getSample (0, (i * blockSize) % buffer.getNumSamples()));
            }
        }
    }

    static AudioBuffer<f32> getTestBuffer (i32 length)
    {
        AudioBuffer<f32> buffer { 2, length };

        for (i32 channel = 0; channel < buffer.getNumChannels(); ++channel)
            for (i32 sample = 0; sample < buffer.getNumSamples(); ++sample)
                buffer.setSample (channel, sample, jmap ((f32) sample, 0.0f, (f32) length, -1.0f, 1.0f));

        return buffer;
    }

    static AudioBuffer<f32> getShortBuffer()  { return getTestBuffer (5); }
    static AudioBuffer<f32> getLongBuffer()   { return getTestBuffer (1000); }

    static z0 play (MemoryAudioSource& source, AudioSourceChannelInfo& info)
    {
        info.clearActiveBufferRegion();
        source.getNextAudioBlock (info);
    }

    static b8 isSilent (const AudioBuffer<f32>& b)
    {
        for (i32 channel = 0; channel < b.getNumChannels(); ++channel)
            if (b.findMinMax (channel, 0, b.getNumSamples()) != Range<f32>{})
                return false;

        return true;
    }
};

static MemoryAudioSourceTests memoryAudioSourceTests;

#endif

} // namespace drx
