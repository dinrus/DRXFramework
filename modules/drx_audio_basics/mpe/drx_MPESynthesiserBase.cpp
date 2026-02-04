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

MPESynthesiserBase::MPESynthesiserBase()
    : instrument (defaultInstrument)
{
    instrument.addListener (this);
}

MPESynthesiserBase::MPESynthesiserBase (MPEInstrument& inst)
    : instrument (inst)
{
    instrument.addListener (this);
}

//==============================================================================
MPEZoneLayout MPESynthesiserBase::getZoneLayout() const noexcept
{
    return instrument.getZoneLayout();
}

z0 MPESynthesiserBase::setZoneLayout (MPEZoneLayout newLayout)
{
    instrument.setZoneLayout (newLayout);
}

//==============================================================================
z0 MPESynthesiserBase::enableLegacyMode (i32 pitchbendRange, Range<i32> channelRange)
{
    instrument.enableLegacyMode (pitchbendRange, channelRange);
}

b8 MPESynthesiserBase::isLegacyModeEnabled() const noexcept
{
    return instrument.isLegacyModeEnabled();
}

Range<i32> MPESynthesiserBase::getLegacyModeChannelRange() const noexcept
{
    return instrument.getLegacyModeChannelRange();
}

z0 MPESynthesiserBase::setLegacyModeChannelRange (Range<i32> channelRange)
{
    instrument.setLegacyModeChannelRange (channelRange);
}

i32 MPESynthesiserBase::getLegacyModePitchbendRange() const noexcept
{
    return instrument.getLegacyModePitchbendRange();
}

z0 MPESynthesiserBase::setLegacyModePitchbendRange (i32 pitchbendRange)
{
    instrument.setLegacyModePitchbendRange (pitchbendRange);
}

//==============================================================================
z0 MPESynthesiserBase::setPressureTrackingMode (TrackingMode modeToUse)
{
    instrument.setPressureTrackingMode (modeToUse);
}

z0 MPESynthesiserBase::setPitchbendTrackingMode (TrackingMode modeToUse)
{
    instrument.setPitchbendTrackingMode (modeToUse);
}

z0 MPESynthesiserBase::setTimbreTrackingMode (TrackingMode modeToUse)
{
    instrument.setTimbreTrackingMode (modeToUse);
}

//==============================================================================
z0 MPESynthesiserBase::handleMidiEvent (const MidiMessage& m)
{
    instrument.processNextMidiEvent (m);
}

//==============================================================================
template <typename floatType>
z0 MPESynthesiserBase::renderNextBlock (AudioBuffer<floatType>& outputAudio,
                                          const MidiBuffer& inputMidi,
                                          i32 startSample,
                                          i32 numSamples)
{
    // you must set the sample rate before using this!
    jassert (! approximatelyEqual (sampleRate, 0.0));

    const ScopedLock sl (noteStateLock);

    auto prevSample = startSample;
    const auto endSample = startSample + numSamples;

    for (auto it = inputMidi.findNextSamplePosition (startSample); it != inputMidi.cend(); ++it)
    {
        const auto metadata = *it;

        if (metadata.samplePosition >= endSample)
            break;

        const auto smallBlockAllowed = (prevSample == startSample && ! subBlockSubdivisionIsStrict);
        const auto thisBlockSize = smallBlockAllowed ? 1 : minimumSubBlockSize;

        if (metadata.samplePosition >= prevSample + thisBlockSize)
        {
            renderNextSubBlock (outputAudio, prevSample, metadata.samplePosition - prevSample);
            prevSample = metadata.samplePosition;
        }

        handleMidiEvent (metadata.getMessage());
    }

    if (prevSample < endSample)
        renderNextSubBlock (outputAudio, prevSample, endSample - prevSample);
}

// explicit instantiation for supported f32 types:
template z0 MPESynthesiserBase::renderNextBlock<f32> (AudioBuffer<f32>&, const MidiBuffer&, i32, i32);
template z0 MPESynthesiserBase::renderNextBlock<f64> (AudioBuffer<f64>&, const MidiBuffer&, i32, i32);

//==============================================================================
z0 MPESynthesiserBase::setCurrentPlaybackSampleRate (const f64 newRate)
{
    if (! approximatelyEqual (sampleRate, newRate))
    {
        const ScopedLock sl (noteStateLock);
        instrument.releaseAllNotes();
        sampleRate = newRate;
    }
}

//==============================================================================
z0 MPESynthesiserBase::setMinimumRenderingSubdivisionSize (i32 numSamples, b8 shouldBeStrict) noexcept
{
    jassert (numSamples > 0); // it wouldn't make much sense for this to be less than 1
    minimumSubBlockSize = numSamples;
    subBlockSubdivisionIsStrict = shouldBeStrict;
}

#if DRX_UNIT_TESTS

namespace
{
    class MpeSynthesiserBaseTests final : public UnitTest
    {
        enum class CallbackKind { process, midi };

        struct StartAndLength
        {
            StartAndLength (i32 s, i32 l) : start (s), length (l) {}

            i32 start  = 0;
            i32 length = 0;

            std::tuple<i32k&, i32k&> tie() const noexcept { return std::tie (start, length); }

            b8 operator== (const StartAndLength& other) const noexcept { return tie() == other.tie(); }
            b8 operator!= (const StartAndLength& other) const noexcept { return tie() != other.tie(); }

            b8 operator< (const StartAndLength& other) const noexcept { return tie() < other.tie(); }
        };

        struct Events
        {
            std::vector<StartAndLength> blocks;
            std::vector<MidiMessage> messages;
            std::vector<CallbackKind> order;
        };

        class MockSynthesiser final : public MPESynthesiserBase
        {
        public:
            Events events;

            z0 handleMidiEvent (const MidiMessage& m) override
            {
                events.messages.emplace_back (m);
                events.order.emplace_back (CallbackKind::midi);
            }

        private:
            using MPESynthesiserBase::renderNextSubBlock;

            z0 renderNextSubBlock (AudioBuffer<f32>&,
                                     i32 startSample,
                                     i32 numSamples) override
            {
                events.blocks.push_back ({ startSample, numSamples });
                events.order.emplace_back (CallbackKind::process);
            }
        };

        static MidiBuffer makeTestBuffer (i32k bufferLength)
        {
            MidiBuffer result;

            for (i32 i = 0; i != bufferLength; ++i)
                result.addEvent ({}, i);

            return result;
        }

    public:
        MpeSynthesiserBaseTests()
            : UnitTest ("MPE Synthesiser Base", UnitTestCategories::midi) {}

        z0 runTest() override
        {
            const auto sumBlockLengths = [] (const std::vector<StartAndLength>& b)
            {
                const auto addBlock = [] (i32 acc, const StartAndLength& info) { return acc + info.length; };
                return std::accumulate (b.begin(), b.end(), 0, addBlock);
            };

            beginTest ("Rendering sparse subblocks works");
            {
                i32k blockSize = 512;
                const auto midi = [&] { MidiBuffer b; b.addEvent ({}, blockSize / 2); return b; }();
                AudioBuffer<f32> audio (1, blockSize);

                const auto processEvents = [&] (i32 start, i32 length)
                {
                    MockSynthesiser synth;
                    synth.setMinimumRenderingSubdivisionSize (1, false);
                    synth.setCurrentPlaybackSampleRate (44100);
                    synth.renderNextBlock (audio, midi, start, length);
                    return synth.events;
                };

                {
                    const auto e = processEvents (0, blockSize);
                    expect (e.blocks.size() == 2);
                    expect (e.messages.size() == 1);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == blockSize);
                    expect (e.order == std::vector<CallbackKind> { CallbackKind::process,
                                                                   CallbackKind::midi,
                                                                   CallbackKind::process });
                }
            }

            beginTest ("Rendering subblocks processes only contained midi events");
            {
                i32k blockSize = 512;
                const auto midi = makeTestBuffer (blockSize);
                AudioBuffer<f32> audio (1, blockSize);

                const auto processEvents = [&] (i32 start, i32 length)
                {
                    MockSynthesiser synth;
                    synth.setMinimumRenderingSubdivisionSize (1, false);
                    synth.setCurrentPlaybackSampleRate (44100);
                    synth.renderNextBlock (audio, midi, start, length);
                    return synth.events;
                };

                {
                    i32k subBlockLength = 0;
                    const auto e = processEvents (0, subBlockLength);
                    expect (e.blocks.size() == 0);
                    expect (e.messages.size() == 0);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == subBlockLength);
                }

                {
                    i32k subBlockLength = 0;
                    const auto e = processEvents (1, subBlockLength);
                    expect (e.blocks.size() == 0);
                    expect (e.messages.size() == 0);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == subBlockLength);
                }

                {
                    i32k subBlockLength = 1;
                    const auto e = processEvents (1, subBlockLength);
                    expect (e.blocks.size() == 1);
                    expect (e.messages.size() == 1);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == subBlockLength);
                    expect (e.order == std::vector<CallbackKind> { CallbackKind::midi,
                                                                   CallbackKind::process });
                }

                {
                    const auto e = processEvents (0, blockSize);
                    expect (e.blocks.size() == blockSize);
                    expect (e.messages.size() == blockSize);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == blockSize);
                    expect (e.order.front() == CallbackKind::midi);
                }
            }

            beginTest ("Subblocks respect their minimum size");
            {
                i32k blockSize = 512;
                const auto midi = makeTestBuffer (blockSize);
                AudioBuffer<f32> audio (1, blockSize);

                const auto blockLengthsAreValid = [] (const std::vector<StartAndLength>& info, i32 minLength, b8 strict)
                {
                    if (info.size() <= 1)
                        return true;

                    const auto lengthIsValid = [&] (const StartAndLength& s) { return minLength <= s.length; };
                    const auto begin = strict ? info.begin() : std::next (info.begin());
                    // The final block is allowed to be shorter than the minLength
                    return std::all_of (begin, std::prev (info.end()), lengthIsValid);
                };

                for (auto strict : { false, true })
                {
                    for (auto subblockSize : { 1, 16, 32, 64, 1024 })
                    {
                        MockSynthesiser synth;
                        synth.setMinimumRenderingSubdivisionSize (subblockSize, strict);
                        synth.setCurrentPlaybackSampleRate (44100);
                        synth.renderNextBlock (audio, midi, 0, blockSize);

                        const auto& e = synth.events;
                        expectWithinAbsoluteError (f32 (e.blocks.size()),
                                                   std::ceil ((f32) blockSize / (f32) subblockSize),
                                                   1.0f);
                        expect (e.messages.size() == blockSize);
                        expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                        expect (sumBlockLengths (e.blocks) == blockSize);
                        expect (blockLengthsAreValid (e.blocks, subblockSize, strict));
                    }
                }

                {
                    MockSynthesiser synth;
                    synth.setMinimumRenderingSubdivisionSize (32, true);
                    synth.setCurrentPlaybackSampleRate (44100);
                    synth.renderNextBlock (audio, MidiBuffer{}, 0, 16);

                    expect (synth.events.blocks == std::vector<StartAndLength> { { 0, 16 } });
                    expect (synth.events.order == std::vector<CallbackKind> { CallbackKind::process });
                    expect (synth.events.messages.empty());
                }
            }
        }
    };

    MpeSynthesiserBaseTests mpeSynthesiserBaseTests;
}

#endif

} // namespace drx
