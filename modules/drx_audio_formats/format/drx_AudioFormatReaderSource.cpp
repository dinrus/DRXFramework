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

AudioFormatReaderSource::AudioFormatReaderSource (AudioFormatReader* const r,
                                                  const b8 deleteReaderWhenThisIsDeleted)
    : reader (r, deleteReaderWhenThisIsDeleted),
      nextPlayPos (0),
      looping (false)
{
    jassert (reader != nullptr);
}

AudioFormatReaderSource::~AudioFormatReaderSource() {}

z64 AudioFormatReaderSource::getTotalLength() const                   { return reader->lengthInSamples; }
z0 AudioFormatReaderSource::setNextReadPosition (z64 newPosition)   { nextPlayPos = newPosition; }
z0 AudioFormatReaderSource::setLooping (b8 shouldLoop)              { looping = shouldLoop; }

z64 AudioFormatReaderSource::getNextReadPosition() const
{
    return looping ? nextPlayPos % reader->lengthInSamples
                   : nextPlayPos;
}

z0 AudioFormatReaderSource::prepareToPlay (i32 /*samplesPerBlockExpected*/, f64 /*sampleRate*/) {}
z0 AudioFormatReaderSource::releaseResources() {}

z0 AudioFormatReaderSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (info.numSamples <= 0)
        return;

    for (auto destOffset = 0; destOffset < info.numSamples;)
    {
        const auto readFrom = looping ? nextPlayPos % reader->lengthInSamples : nextPlayPos;

        const auto numSamplesToRead = jlimit ((z64) 0,
                                              (z64) (info.numSamples - destOffset),
                                              reader->lengthInSamples - readFrom);

        reader->read (info.buffer, info.startSample + destOffset,
                      (i32) numSamplesToRead, readFrom, true, true);

        destOffset += (i32) numSamplesToRead;
        nextPlayPos += numSamplesToRead;

        if (! looping)
        {
            const auto numSamplesToClear = info.numSamples - destOffset;
            info.buffer->clear (info.startSample + destOffset, numSamplesToClear);

            destOffset += numSamplesToClear;
            nextPlayPos += numSamplesToClear;
        }
    }
}

#if DRX_UNIT_TESTS

struct AudioFormatReaderSourceTests : public UnitTest
{
    AudioFormatReaderSourceTests()
        : UnitTest ("AudioFormatReaderSource", UnitTestCategories::audio)
    {}

    //==============================================================================
    struct GetNextAudioBlockTestParams
    {
        i32 audioFormatReaderLength;
        i32 readFrom;
        i32 numSamplesToRead;
        b8 enableLooping;
    };

    static z0 mockReadSamples (f32* dest, z64 audioFormatReaderLength, z64 readFrom, i32 numSamplesToRead)
    {
        for (auto i = readFrom; i < readFrom + numSamplesToRead; ++i)
        {
            *dest = i < audioFormatReaderLength ? 0.001f * (f32) i : 0.0f;
            ++dest;
        }
    }

    static z0 createGetNextAudioBlockExpectedOutput (const GetNextAudioBlockTestParams& params,
                                                       std::vector<f32>& expected)
    {
        for (auto i = params.readFrom, end = i + params.numSamplesToRead; i < end; ++i)
        {
            const auto expectedResult = params.enableLooping || i < params.audioFormatReaderLength
                                      ? 0.001f * (f32) (i % params.audioFormatReaderLength)
                                      : 0.0f;

            expected.push_back (expectedResult);
        }
    }

    //==============================================================================
    struct TestAudioFormatReader : public AudioFormatReader
    {
        explicit TestAudioFormatReader (i32 audioFormatReaderLength)
            : AudioFormatReader { nullptr, "test_format" }
        {
            jassert (audioFormatReaderLength < 1000);

            lengthInSamples = (z64) audioFormatReaderLength;
            numChannels = 1;
            usesFloatingPointData = true;
            bitsPerSample = 32;
        }

        z0 readMaxLevels (z64, z64, Range<f32>*, i32) override { jassertfalse; }
        z0 readMaxLevels (z64, z64, f32&, f32&, f32&, f32&) override { jassertfalse; }

        AudioChannelSet getChannelLayout() override
        {
            return AudioChannelSet::mono();
        }

        b8 readSamples (i32* const* destChannels,
                          [[maybe_unused]] i32 numDestChannels,
                          i32 startOffsetInDestBuffer,
                          z64 startSampleInFile,
                          i32 numSamples) override
        {
            jassert (numDestChannels == 1);
            mockReadSamples (reinterpret_cast<f32*> (*destChannels + startOffsetInDestBuffer),
                             lengthInSamples,
                             startSampleInFile,
                             numSamples);
            return true;
        }
    };

    static auto createTestAudioFormatReaderSource (const GetNextAudioBlockTestParams& params)
    {
        return AudioFormatReaderSource { new TestAudioFormatReader (params.audioFormatReaderLength), true };
    }

    static z0 getNextAudioBlock (AudioFormatReaderSource& source,
                                   const GetNextAudioBlockTestParams& params,
                                   std::vector<f32>& result)
    {
        source.setLooping (params.enableLooping);
        source.setNextReadPosition (params.readFrom);

        AudioBuffer<f32> buffer { 1, params.numSamplesToRead };
        AudioSourceChannelInfo info { &buffer, 0, buffer.getNumSamples() };

        source.getNextAudioBlock (info);

        result.insert (result.end(),
                       buffer.getReadPointer (0),
                       buffer.getReadPointer (0) + buffer.getNumSamples());
    }

    static auto createFailureMessage (const GetNextAudioBlockTestParams& params)
    {
        return Txt { "AudioFormatReaderSource::getNextAudioBlock() failed for "
                        "audioFormatReaderLength=%audioFormatReaderLength%, "
                        "readFrom=%readFrom%, "
                        "numSamplesToRead=%numSamplesToRead%, "
                        "enableLooping=%enableLooping%" }
                            .replace ("%audioFormatReaderLength%", Txt { params.audioFormatReaderLength })
                            .replace ("%readFrom%", Txt { params.readFrom })
                            .replace ("%numSamplesToRead%", Txt { params.numSamplesToRead })
                            .replace ("%enableLooping%", params.enableLooping ? "true" : "false");
    }

    z0 runTest() override
    {
        const auto predicate = [] (auto a, auto b) { return exactlyEqual (a, b); };

        const auto testGetNextAudioBlock = [this, &predicate] (const GetNextAudioBlockTestParams& params)
        {
            auto uut = createTestAudioFormatReaderSource (params);
            std::vector<f32> actual;
            getNextAudioBlock (uut, params, actual);

            std::vector<f32> expected;
            createGetNextAudioBlockExpectedOutput (params, expected);

            expect (std::equal (expected.begin(), expected.end(), actual.begin(), actual.end(), predicate),
                    createFailureMessage (params));
        };

        beginTest ("A buffer without looping is played once and followed by silence");
        {
            GetNextAudioBlockTestParams testParams { 32, 0, 48, false };
            testGetNextAudioBlock (testParams);
        }

        beginTest ("A buffer with looping is played multiple times");
        {
            GetNextAudioBlockTestParams params { 32, 0, 24, true };

            auto uut = createTestAudioFormatReaderSource (params);
            std::vector<f32> actual;
            std::vector<f32> expected;
            const auto numReads = 4;

            for (auto i = 0; i < numReads; ++i)
            {
                getNextAudioBlock (uut, params, actual);
                createGetNextAudioBlockExpectedOutput (params, expected);
                params.readFrom += params.numSamplesToRead;
            }

            expect (std::equal (expected.begin(), expected.end(), actual.begin(), actual.end(), predicate),
                    createFailureMessage (params) + " numReads=" + Txt { numReads });
        }

        beginTest ("A buffer with looping, loops even if the blockSize is greater than the internal buffer");
        {
            GetNextAudioBlockTestParams testParams { 32, 16, 128, true };
            testGetNextAudioBlock (testParams);
        }

        beginTest ("Behavioural invariants hold even if we turn on looping after prior reads");
        {
            GetNextAudioBlockTestParams params { 32, 0, 24, false };

            auto uut = createTestAudioFormatReaderSource (params);
            std::vector<f32> actual;
            std::vector<f32> expected;

            for (auto i = 0; i < 4; ++i)
            {
                getNextAudioBlock (uut, params, actual);
                createGetNextAudioBlockExpectedOutput (params, expected);
                params.readFrom += params.numSamplesToRead;
            }

            params.enableLooping = true;

            for (auto i = 0; i < 4; ++i)
            {
                getNextAudioBlock (uut, params, actual);
                createGetNextAudioBlockExpectedOutput (params, expected);
                params.readFrom += params.numSamplesToRead;
            }

            expect (std::equal (expected.begin(), expected.end(), actual.begin(), actual.end(), predicate),
                    createFailureMessage (params));
        }

        beginTest ("Fuzzing: getNextAudioBlock() should return correct results for all possible inputs");
        {
            for (auto params : { GetNextAudioBlockTestParams { 32, 0,  32,  false },
                                 GetNextAudioBlockTestParams { 32, 16, 32,  false },
                                 GetNextAudioBlockTestParams { 32, 16, 32,  true  },
                                 GetNextAudioBlockTestParams { 32, 16, 48,  false },
                                 GetNextAudioBlockTestParams { 32, 16, 128, false },
                                 GetNextAudioBlockTestParams { 32, 16, 48,  true  },
                                 GetNextAudioBlockTestParams { 32, 16, 128, true  } })
            {
                testGetNextAudioBlock (params);
            }

            const Range<i32> audioFormatReaderLengthRange { 16, 128 };
            const Range<i32> startFromRange { 0, 128 };
            const Range<i32> numSamplesRange { 0, 128 };

            auto r = getRandom();

            for (i32 i = 0; i < 100; ++i)
            {
                GetNextAudioBlockTestParams params { r.nextInt (audioFormatReaderLengthRange),
                                                     r.nextInt (startFromRange),
                                                     r.nextInt (numSamplesRange),
                                                     r.nextBool() };

                testGetNextAudioBlock (params);
            }
        }
    }
};

static AudioFormatReaderSourceTests audioFormatReaderSourceTests;

#endif

} // namespace drx
