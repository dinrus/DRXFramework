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

#ifndef DOXYGEN

// This macro can be set if you need to override this internal name for some reason..
#ifndef DRX_STATE_DICTIONARY_KEY
 #define DRX_STATE_DICTIONARY_KEY   "jucePluginState"
#endif


#if (DRX_IOS && DRX_IOS_API_VERSION_CAN_BE_BUILT (15, 0)) \
   || (DRX_MAC && DRX_MAC_API_VERSION_CAN_BE_BUILT (12, 0))
 #define DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED 1
#else
 #define DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED 0
#endif

#include <drx_audio_basics/midi/drx_MidiDataConcatenator.h>

#if DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED
 #include <drx_audio_basics/midi/ump/drx_UMP.h>
#endif

namespace drx
{

struct AudioUnitHelpers
{
    class ChannelRemapper
    {
    public:
        z0 alloc (AudioProcessor& processor)
        {
            i32k numInputBuses  = AudioUnitHelpers::getBusCount (processor, true);
            i32k numOutputBuses = AudioUnitHelpers::getBusCount (processor, false);

            initializeChannelMapArray (processor, true, numInputBuses);
            initializeChannelMapArray (processor, false, numOutputBuses);

            for (i32 busIdx = 0; busIdx < numInputBuses; ++busIdx)
                fillLayoutChannelMaps (processor, true, busIdx);

            for (i32 busIdx = 0; busIdx < numOutputBuses; ++busIdx)
                fillLayoutChannelMaps (processor, false, busIdx);
        }

        z0 release()
        {
            inputLayoutMap = outputLayoutMap = nullptr;
            inputLayoutMapPtrStorage.free();
            outputLayoutMapPtrStorage.free();
            inputLayoutMapStorage.free();
            outputLayoutMapStorage.free();
        }

        inline i32k* get (b8 input, i32 bus) const noexcept { return (input ? inputLayoutMap : outputLayoutMap)[bus]; }

    private:
        //==============================================================================
        HeapBlock<i32*> inputLayoutMapPtrStorage, outputLayoutMapPtrStorage;
        HeapBlock<i32>  inputLayoutMapStorage, outputLayoutMapStorage;
        i32** inputLayoutMap  = nullptr;
        i32** outputLayoutMap = nullptr;

        //==============================================================================
        z0 initializeChannelMapArray (AudioProcessor& processor, b8 isInput, i32k numBuses)
        {
            HeapBlock<i32*>& layoutMapPtrStorage = isInput ? inputLayoutMapPtrStorage : outputLayoutMapPtrStorage;
            HeapBlock<i32>& layoutMapStorage = isInput ? inputLayoutMapStorage : outputLayoutMapStorage;
            i32**& layoutMap = isInput ? inputLayoutMap : outputLayoutMap;

            i32k totalInChannels  = processor.getTotalNumInputChannels();
            i32k totalOutChannels = processor.getTotalNumOutputChannels();

            layoutMapPtrStorage.calloc (static_cast<size_t> (numBuses));
            layoutMapStorage.calloc (static_cast<size_t> (isInput ? totalInChannels : totalOutChannels));

            layoutMap = layoutMapPtrStorage.get();

            i32 ch = 0;
            for (i32 busIdx = 0; busIdx < numBuses; ++busIdx)
            {
                layoutMap[busIdx] = layoutMapStorage.get() + ch;
                ch += processor.getChannelCountOfBus (isInput, busIdx);
            }
        }

        z0 fillLayoutChannelMaps (AudioProcessor& processor, b8 isInput, i32 busNr)
        {
            i32* layoutMap = (isInput ? inputLayoutMap : outputLayoutMap)[busNr];
            auto channelFormat = processor.getChannelLayoutOfBus (isInput, busNr);
            AudioChannelLayout coreAudioLayout;

            zerostruct (coreAudioLayout);
            coreAudioLayout.mChannelLayoutTag = CoreAudioLayouts::toCoreAudio (channelFormat);

            i32k numChannels = channelFormat.size();
            auto coreAudioChannels = CoreAudioLayouts::getCoreAudioLayoutChannels (coreAudioLayout);

            for (i32 i = 0; i < numChannels; ++i)
                layoutMap[i] = coreAudioChannels.indexOf (channelFormat.getTypeOfChannel (i));
        }
    };

    //==============================================================================
    class CoreAudioBufferList
    {
    public:
        z0 prepare (const AudioProcessor::BusesLayout& layout, i32 maxFrames)
        {
            const auto getChannelOffsets = [] (const auto& range)
            {
                std::vector<i32> result { 0 };

                for (const auto& bus : range)
                    result.push_back (result.back() + bus.size());

                return result;
            };

            inputBusOffsets  = getChannelOffsets (layout.inputBuses);
            outputBusOffsets = getChannelOffsets (layout.outputBuses);

            const auto numChannels = jmax (inputBusOffsets.back(), outputBusOffsets.back());
            scratch.setSize (numChannels, maxFrames);
            channels.resize (static_cast<size_t> (numChannels), nullptr);

            reset();
        }

        z0 release()
        {
            scratch.setSize (0, 0);
            channels = {};
            inputBusOffsets = outputBusOffsets = std::vector<i32>();
        }

        z0 reset()
        {
            std::fill (channels.begin(), channels.end(), nullptr);
        }

        f32* setBuffer (i32k idx, f32* ptr = nullptr) noexcept
        {
            jassert (idx < scratch.getNumChannels());
            return channels[(size_t) idx] = uniqueBuffer (idx, ptr);
        }

        AudioBuffer<f32>& getBuffer (UInt32 frames) noexcept
        {
            jassert (std::none_of (channels.begin(), channels.end(), [] (auto* x) { return x == nullptr; }));

            const auto channelPtr = channels.empty() ? scratch.getArrayOfWritePointers() : channels.data();
            mutableBuffer.setDataToReferTo (channelPtr, (i32) channels.size(), static_cast<i32> (frames));

            return mutableBuffer;
        }

        z0 set (i32 bus, AudioBufferList& bufferList, i32k* channelMap) noexcept
        {
            if (bufferList.mNumberBuffers <= 0 || ! isPositiveAndBelow (bus, inputBusOffsets.size() - 1))
                return;

            const auto n = (UInt32) (bufferList.mBuffers[0].mDataByteSize / (bufferList.mBuffers[0].mNumberChannels * sizeof (f32)));
            const auto isInterleaved = isAudioBufferInterleaved (bufferList);
            const auto numChannels = (i32) (isInterleaved ? bufferList.mBuffers[0].mNumberChannels
                                                          : bufferList.mNumberBuffers);

            for (i32 ch = 0; ch < numChannels; ++ch)
            {
                f32* data = channels[(size_t) (inputBusOffsets[(size_t) bus] + ch)];

                const auto mappedChannel = channelMap[ch];

                if (isInterleaved || static_cast<f32*> (bufferList.mBuffers[mappedChannel].mData) != data)
                    copyAudioBuffer (bufferList, mappedChannel, n, data);
            }
        }

        z0 get (i32 bus, AudioBufferList& buffer, i32k* channelMap) noexcept
        {
            if (buffer.mNumberBuffers <= 0 || ! isPositiveAndBelow (bus, outputBusOffsets.size() - 1))
                return;

            const auto n = (UInt32) (buffer.mBuffers[0].mDataByteSize / (buffer.mBuffers[0].mNumberChannels * sizeof (f32)));
            const auto isInterleaved = isAudioBufferInterleaved (buffer);
            const auto numChannels = (i32) (isInterleaved ? buffer.mBuffers[0].mNumberChannels
                                                          : buffer.mNumberBuffers);

            for (i32 ch = 0; ch < numChannels; ++ch)
            {
                f32* data = channels[(size_t) (outputBusOffsets[(size_t) bus] + ch)];

                const auto mappedChannel = channelMap[ch];

                if (data == buffer.mBuffers[mappedChannel].mData && ! isInterleaved)
                    continue; // no copying necessary

                if (buffer.mBuffers[mappedChannel].mData == nullptr && ! isInterleaved)
                    buffer.mBuffers[mappedChannel].mData = data;
                else
                    copyAudioBuffer (data, mappedChannel, n, buffer);
            }
        }

        z0 clearInputBus (i32 index, i32 bufferLength)
        {
            if (isPositiveAndBelow (index, inputBusOffsets.size() - 1))
                clearChannels ({ inputBusOffsets[(size_t) index], inputBusOffsets[(size_t) (index + 1)] }, bufferLength);
        }

        z0 clearUnusedChannels (i32 bufferLength)
        {
            jassert (! inputBusOffsets .empty());
            jassert (! outputBusOffsets.empty());

            clearChannels ({ inputBusOffsets.back(), outputBusOffsets.back() }, bufferLength);
        }

    private:
        z0 clearChannels (Range<i32> range, i32 bufferLength)
        {
            jassert (bufferLength <= scratch.getNumSamples());

            if (range.getEnd() <= (i32) channels.size())
            {
                std::for_each (channels.begin() + range.getStart(),
                               channels.begin() + range.getEnd(),
                               [bufferLength] (f32* ptr) { zeromem (ptr, sizeof (f32) * (size_t) bufferLength); });
            }
        }

        f32* uniqueBuffer (i32 idx, f32* buffer) noexcept
        {
            if (buffer == nullptr)
                return scratch.getWritePointer (idx);

            for (i32 ch = 0; ch < idx; ++ch)
                if (buffer == channels[(size_t) ch])
                    return scratch.getWritePointer (idx);

            return buffer;
        }

        //==============================================================================
        AudioBuffer<f32> scratch, mutableBuffer;
        std::vector<f32*> channels;
        std::vector<i32> inputBusOffsets, outputBusOffsets;
    };

    static b8 isAudioBufferInterleaved (const AudioBufferList& audioBuffer) noexcept
    {
        return (audioBuffer.mNumberBuffers == 1 && audioBuffer.mBuffers[0].mNumberChannels > 1);
    }

    static z0 clearAudioBuffer (const AudioBufferList& audioBuffer) noexcept
    {
        for (u32 ch = 0; ch < audioBuffer.mNumberBuffers; ++ch)
            zeromem (audioBuffer.mBuffers[ch].mData, audioBuffer.mBuffers[ch].mDataByteSize);
    }

    static z0 copyAudioBuffer (const AudioBufferList& audioBuffer, i32k channel, const UInt32 size, f32* dst) noexcept
    {
        if (! isAudioBufferInterleaved (audioBuffer))
        {
            jassert (channel < static_cast<i32> (audioBuffer.mNumberBuffers));
            jassert (audioBuffer.mBuffers[channel].mDataByteSize == (size * sizeof (f32)));

            memcpy (dst, audioBuffer.mBuffers[channel].mData, size * sizeof (f32));
        }
        else
        {
            i32k numChannels = static_cast<i32> (audioBuffer.mBuffers[0].mNumberChannels);
            const UInt32 n = static_cast<UInt32> (numChannels) * size;
            const f32* src = static_cast<const f32*> (audioBuffer.mBuffers[0].mData);

            jassert (channel < numChannels);
            jassert (audioBuffer.mBuffers[0].mDataByteSize == (n * sizeof (f32)));

            for (const f32* inData = src; inData < (src + n); inData += numChannels)
                *dst++ = inData[channel];
        }
    }

    static z0 copyAudioBuffer (const f32 *src, i32k channel, const UInt32 size, AudioBufferList& audioBuffer) noexcept
    {
        if (! isAudioBufferInterleaved (audioBuffer))
        {
            jassert (channel < static_cast<i32> (audioBuffer.mNumberBuffers));
            jassert (audioBuffer.mBuffers[channel].mDataByteSize == (size * sizeof (f32)));

            memcpy (audioBuffer.mBuffers[channel].mData, src, size * sizeof (f32));
        }
        else
        {
            i32k numChannels = static_cast<i32> (audioBuffer.mBuffers[0].mNumberChannels);
            const UInt32 n = static_cast<UInt32> (numChannels) * size;
            f32* dst = static_cast<f32*> (audioBuffer.mBuffers[0].mData);

            jassert (channel < numChannels);
            jassert (audioBuffer.mBuffers[0].mDataByteSize == (n * sizeof (f32)));

            for (f32* outData = dst; outData < (dst + n); outData += numChannels)
                outData[channel] = *src++;
        }
    }

    template <size_t numLayouts>
    static b8 isLayoutSupported (const AudioProcessor& processor,
                                   b8 isInput, i32 busIdx,
                                   i32 numChannels,
                                   const short (&channelLayoutList)[numLayouts][2],
                                   b8 hasLayoutMap = true)
    {
        if (const AudioProcessor::Bus* bus = processor.getBus (isInput, busIdx))
        {
            if (! bus->isNumberOfChannelsSupported (numChannels))
                return false;

            if (! hasLayoutMap)
                return true;

            i32k numConfigs = sizeof (channelLayoutList) / sizeof (short[2]);

            for (i32 i = 0; i < numConfigs; ++i)
            {
                if (channelLayoutList[i][isInput ? 0 : 1] == numChannels)
                    return true;
            }
        }

        return false;
    }

    static Array<AUChannelInfo> getAUChannelInfo (const AudioProcessor& processor)
    {
       #ifdef DrxPlugin_AUMainType
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfour-t8-constants")
        if constexpr (DrxPlugin_AUMainType == kAudioUnitType_MIDIProcessor)
        {
            // A MIDI effect requires an output bus in order to determine the sample rate.
            // No audio will be written to the output bus, so it can have any number of channels.
            // No input bus is required.
            return { AUChannelInfo { 0, -1 } };
        }
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
       #endif

        Array<AUChannelInfo> channelInfo;

        auto hasMainInputBus  = (AudioUnitHelpers::getBusCountForWrapper (processor, true)  > 0);
        auto hasMainOutputBus = (AudioUnitHelpers::getBusCountForWrapper (processor, false) > 0);

        auto layout = processor.getBusesLayout();

        // The 'standard' layout with the most channels defined is AudioChannelSet::create9point1point6().
        // This value should be updated if larger standard channel layouts are added in the future.
        constexpr auto maxNumChanToCheckFor = 16;

        auto defaultInputs  = processor.getChannelCountOfBus (true,  0);
        auto defaultOutputs = processor.getChannelCountOfBus (false, 0);

        struct Channels
        {
            SInt16 ins, outs;

            std::pair<SInt16, SInt16> makePair() const noexcept { return std::make_pair (ins, outs); }

            b8 operator<  (const Channels& other) const noexcept { return makePair() <  other.makePair(); }
            b8 operator== (const Channels& other) const noexcept { return makePair() == other.makePair(); }
        };

        SortedSet<Channels> supportedChannels;

        // add the current configuration
        if (defaultInputs != 0 || defaultOutputs != 0)
            supportedChannels.add ({ static_cast<SInt16> (defaultInputs),
                                     static_cast<SInt16> (defaultOutputs) });

        for (auto inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
        {
            auto inLayout = layout;

            if (auto* inBus = processor.getBus (true, 0))
                if (! isNumberOfChannelsSupported (inBus, inChanNum, inLayout))
                    continue;

            for (auto outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
            {
                auto outLayout = inLayout;

                if (auto* outBus = processor.getBus (false, 0))
                    if (! isNumberOfChannelsSupported (outBus, outChanNum, outLayout))
                        continue;

                supportedChannels.add ({ static_cast<SInt16> (hasMainInputBus  ? outLayout.getMainInputChannels()  : 0),
                                         static_cast<SInt16> (hasMainOutputBus ? outLayout.getMainOutputChannels() : 0) });
            }
        }

        auto hasInOutMismatch = false;

        for (const auto& supported : supportedChannels)
        {
            if (supported.ins != supported.outs)
            {
                hasInOutMismatch = true;
                break;
            }
        }

        auto hasUnsupportedInput = ! hasMainInputBus, hasUnsupportedOutput = ! hasMainOutputBus;

        for (auto inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
        {
            Channels channelConfiguration { static_cast<SInt16> (inChanNum),
                                            static_cast<SInt16> (hasInOutMismatch ? defaultOutputs : inChanNum) };

            if (! supportedChannels.contains (channelConfiguration))
            {
                hasUnsupportedInput = true;
                break;
            }
        }

        for (auto outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
        {
            Channels channelConfiguration { static_cast<SInt16> (hasInOutMismatch ? defaultInputs : outChanNum),
                                            static_cast<SInt16> (outChanNum) };

            if (! supportedChannels.contains (channelConfiguration))
            {
                hasUnsupportedOutput = true;
                break;
            }
        }

        for (const auto& supported : supportedChannels)
        {
            AUChannelInfo info;

            // see here: https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnit/TheAudioUnit.html
            info.inChannels  = static_cast<SInt16> (hasMainInputBus  ? (hasUnsupportedInput  ? supported.ins  : (hasInOutMismatch && (! hasUnsupportedOutput) ? -2 : -1)) : 0);
            info.outChannels = static_cast<SInt16> (hasMainOutputBus ? (hasUnsupportedOutput ? supported.outs : (hasInOutMismatch && (! hasUnsupportedInput)  ? -2 : -1)) : 0);

            if (info.inChannels == -2 && info.outChannels == -2)
                info.inChannels = -1;

            i32 j;
            for (j = 0; j < channelInfo.size(); ++j)
                if (info.inChannels == channelInfo.getReference (j).inChannels
                      && info.outChannels == channelInfo.getReference (j).outChannels)
                    break;

            if (j >= channelInfo.size())
                channelInfo.add (info);
        }

        return channelInfo;
    }

    static b8 isNumberOfChannelsSupported (const AudioProcessor::Bus* b, i32 numChannels, AudioProcessor::BusesLayout& inOutCurrentLayout)
    {
        auto potentialSets = AudioChannelSet::channelSetsWithNumberOfChannels (static_cast<i32> (numChannels));

        for (auto set : potentialSets)
        {
            auto copy = inOutCurrentLayout;

            if (b->isLayoutSupported (set, &copy))
            {
                inOutCurrentLayout = copy;
                return true;
            }
        }

        return false;
    }

    //==============================================================================
    static i32 getBusCount (const AudioProcessor& juceFilter, b8 isInput)
    {
        i32 busCount = juceFilter.getBusCount (isInput);

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = {DrxPlugin_PreferredChannelConfigurations};
        i32k numConfigs = sizeof (configs) / sizeof (short[2]);

        b8 hasOnlyZeroChannels = true;

        for (i32 i = 0; i < numConfigs && hasOnlyZeroChannels == true; ++i)
            if (configs[i][isInput ? 0 : 1] != 0)
                hasOnlyZeroChannels = false;

        busCount = jmin (busCount, hasOnlyZeroChannels ? 0 : 1);
       #endif

        return busCount;
    }

    static i32 getBusCountForWrapper (const AudioProcessor& juceFilter, b8 isInput)
    {
       #ifdef DrxPlugin_AUMainType
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfour-t8-constants")
        constexpr auto pluginIsMidiEffect = DrxPlugin_AUMainType == kAudioUnitType_MIDIProcessor;
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
       #else
        constexpr auto pluginIsMidiEffect = false;
       #endif

        const auto numRequiredBuses = (isInput || ! pluginIsMidiEffect) ? 0 : 1;

        return jmax (numRequiredBuses, getBusCount (juceFilter, isInput));
    }

    static b8 setBusesLayout (AudioProcessor* juceFilter, const AudioProcessor::BusesLayout& requestedLayouts)
    {
       #ifdef DrxPlugin_PreferredChannelConfigurations
        AudioProcessor::BusesLayout copy (requestedLayouts);

        for (i32 dir = 0; dir < 2; ++dir)
        {
            const b8 isInput = (dir == 0);

            i32k actualBuses = juceFilter->getBusCount (isInput);
            i32k auNumBuses  = getBusCount (*juceFilter, isInput);
            Array<AudioChannelSet>& buses = (isInput ? copy.inputBuses : copy.outputBuses);

            for (i32 i = auNumBuses; i < actualBuses; ++i)
                buses.add (AudioChannelSet::disabled());
        }

        return juceFilter->setBusesLayout (copy);
       #else
        return juceFilter->setBusesLayout (requestedLayouts);
       #endif
    }

    static AudioProcessor::BusesLayout getBusesLayout (const AudioProcessor* juceFilter)
    {
       #ifdef DrxPlugin_PreferredChannelConfigurations
        AudioProcessor::BusesLayout layout = juceFilter->getBusesLayout();

        for (i32 dir = 0; dir < 2; ++dir)
        {
            const b8 isInput = (dir == 0);

            i32k actualBuses = juceFilter->getBusCount (isInput);
            i32k auNumBuses  = getBusCount (*juceFilter, isInput);
            auto& buses = (isInput ? layout.inputBuses : layout.outputBuses);

            for (i32 i = auNumBuses; i < actualBuses; ++i)
                buses.removeLast();
        }

        return layout;
       #else
        return juceFilter->getBusesLayout();
       #endif
    }

   #if DRX_APPLE_MIDI_EVENT_LIST_SUPPORTED
    class ScopedMIDIEventListBlock
    {
    public:
        ScopedMIDIEventListBlock() = default;

        ScopedMIDIEventListBlock (ScopedMIDIEventListBlock&& other) noexcept
            : midiEventListBlock (std::exchange (other.midiEventListBlock, nil)) {}

        ScopedMIDIEventListBlock& operator= (ScopedMIDIEventListBlock&& other) noexcept
        {
            ScopedMIDIEventListBlock { std::move (other) }.swap (*this);
            return *this;
        }

        ~ScopedMIDIEventListBlock()
        {
            if (midiEventListBlock != nil)
                [midiEventListBlock release];
        }

        static ScopedMIDIEventListBlock copy (AUMIDIEventListBlock b)
        {
            return ScopedMIDIEventListBlock { b };
        }

        explicit operator b8() const { return midiEventListBlock != nil; }

        z0 operator() (AUEventSampleTime eventSampleTime, u8 cable, const struct MIDIEventList * eventList) const
        {
            jassert (midiEventListBlock != nil);
            midiEventListBlock (eventSampleTime, cable, eventList);
        }

    private:
        z0 swap (ScopedMIDIEventListBlock& other) noexcept
        {
            std::swap (other.midiEventListBlock, midiEventListBlock);
        }

        explicit ScopedMIDIEventListBlock (AUMIDIEventListBlock b) : midiEventListBlock ([b copy]) {}

        AUMIDIEventListBlock midiEventListBlock = nil;
    };

    class EventListOutput
    {
    public:
        API_AVAILABLE (macos (12.0), ios (15.0))
        z0 setBlock (ScopedMIDIEventListBlock x)
        {
            block = std::move (x);
        }

        API_AVAILABLE (macos (12.0), ios (15.0))
        z0 setBlock (AUMIDIEventListBlock x)
        {
            setBlock (ScopedMIDIEventListBlock::copy (x));
        }

        b8 trySend (const MidiBuffer& buffer, z64 baseTimeStamp)
        {
            if (! block)
                return false;

            struct MIDIEventList stackList = {};
            MIDIEventPacket* end = nullptr;

            const auto init = [&]
            {
                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")
                end = MIDIEventListInit (&stackList, kMIDIProtocol_1_0);
                DRX_END_IGNORE_WARNINGS_GCC_LIKE
            };

            const auto send = [&]
            {
                block (baseTimeStamp, 0, &stackList);
            };

            const auto add = [&] (const ump::View& view, i32 timeStamp)
            {
                static_assert (sizeof (u32) == sizeof (UInt32)
                               && alignof (u32) == alignof (UInt32),
                               "If this fails, the cast below will be broken too!");
                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")
                using List = struct MIDIEventList;
                end = MIDIEventListAdd (&stackList,
                                        sizeof (List::packet),
                                        end,
                                        (MIDITimeStamp) timeStamp,
                                        view.size(),
                                        reinterpret_cast<const UInt32*> (view.data()));
                DRX_END_IGNORE_WARNINGS_GCC_LIKE
            };

            init();

            for (const auto metadata : buffer)
            {
                toUmp1Converter.convert (ump::BytestreamMidiView (metadata), [&] (const ump::View& view)
                {
                    add (view, metadata.samplePosition);

                    if (end != nullptr)
                        return;

                    send();
                    init();
                    add (view, metadata.samplePosition);
                });
            }

            send();

            return true;
        }

    private:
        ScopedMIDIEventListBlock block;
        ump::ToUMP1Converter toUmp1Converter;
    };
   #endif
};

} // namespace drx

#endif
