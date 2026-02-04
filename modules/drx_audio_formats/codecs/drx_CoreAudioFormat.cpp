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

#if DRX_MAC || DRX_IOS

#include <drx_audio_basics/native/drx_CoreAudioLayouts_mac.h>
#include <drx_core/native/drx_CFHelpers_mac.h>

namespace drx
{

//==============================================================================
namespace
{
    tukk const coreAudioFormatName = "CoreAudio supported file";

    StringArray getStringInfo (AudioFilePropertyID property, UInt32 size, uk data)
    {
        CFObjectHolder<CFArrayRef> extensions;
        UInt32 sizeOfArray = sizeof (extensions.object);

        const auto err = AudioFileGetGlobalInfo (property,
                                                 size,
                                                 data,
                                                 &sizeOfArray,
                                                 &extensions.object);

        if (err != noErr)
            return {};

        const auto numValues = CFArrayGetCount (extensions.object);

        StringArray extensionsArray;

        for (CFIndex i = 0; i < numValues; ++i)
            extensionsArray.add ("." + Txt::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (extensions.object, i)));

        return extensionsArray;
    }

    StringArray findFileExtensionsForCoreAudioCodec (AudioFileTypeID type)
    {
        return getStringInfo (kAudioFileGlobalInfo_ExtensionsForType, sizeof (AudioFileTypeID), &type);
    }

    StringArray findFileExtensionsForCoreAudioCodecs()
    {
        return getStringInfo (kAudioFileGlobalInfo_AllExtensions, 0, nullptr);
    }

    static AudioFileTypeID toAudioFileTypeID (CoreAudioFormat::StreamKind kind)
    {
        using StreamKind = CoreAudioFormat::StreamKind;

        switch (kind)
        {
            case StreamKind::kAiff:                 return kAudioFileAIFFType;
            case StreamKind::kAifc:                 return kAudioFileAIFCType;
            case StreamKind::kWave:                 return kAudioFileWAVEType;
            case StreamKind::kSoundDesigner2:       return kAudioFileSoundDesigner2Type;
            case StreamKind::kNext:                 return kAudioFileNextType;
            case StreamKind::kMp3:                  return kAudioFileMP3Type;
            case StreamKind::kMp2:                  return kAudioFileMP2Type;
            case StreamKind::kMp1:                  return kAudioFileMP1Type;
            case StreamKind::kAc3:                  return kAudioFileAC3Type;
            case StreamKind::kAacAdts:              return kAudioFileAAC_ADTSType;
            case StreamKind::kMpeg4:                return kAudioFileMPEG4Type;
            case StreamKind::kM4a:                  return kAudioFileM4AType;
            case StreamKind::kM4b:                  return kAudioFileM4BType;
            case StreamKind::kCaf:                  return kAudioFileCAFType;
            case StreamKind::k3gp:                  return kAudioFile3GPType;
            case StreamKind::k3gp2:                 return kAudioFile3GP2Type;
            case StreamKind::kAmr:                  return kAudioFileAMRType;

            case StreamKind::kNone:                 break;
        }

        return {};
    }
}

//==============================================================================
tukk const CoreAudioFormat::midiDataBase64   = "midiDataBase64";
tukk const CoreAudioFormat::tempo            = "tempo";
tukk const CoreAudioFormat::timeSig          = "time signature";
tukk const CoreAudioFormat::keySig           = "key signature";

//==============================================================================
struct CoreAudioFormatMetatdata
{
    static u32 chunkName (tukk const name) noexcept   { return ByteOrder::bigEndianInt (name); }

    //==============================================================================
    struct FileHeader
    {
        FileHeader (InputStream& input)
        {
            fileType    = (u32) input.readIntBigEndian();
            fileVersion = (u16) input.readShortBigEndian();
            fileFlags   = (u16) input.readShortBigEndian();
        }

        u32 fileType;
        u16 fileVersion;
        u16 fileFlags;
    };

    //==============================================================================
    struct ChunkHeader
    {
        ChunkHeader (InputStream& input)
        {
            chunkType = (u32) input.readIntBigEndian();
            chunkSize = (z64)  input.readInt64BigEndian();
        }

        u32 chunkType;
        z64 chunkSize;
    };

    //==============================================================================
    struct AudioDescriptionChunk
    {
        AudioDescriptionChunk (InputStream& input)
        {
            sampleRate          = input.readDoubleBigEndian();
            formatID            = (u32) input.readIntBigEndian();
            formatFlags         = (u32) input.readIntBigEndian();
            bytesPerPacket      = (u32) input.readIntBigEndian();
            framesPerPacket     = (u32) input.readIntBigEndian();
            channelsPerFrame    = (u32) input.readIntBigEndian();
            bitsPerChannel      = (u32) input.readIntBigEndian();
        }

        f64 sampleRate;
        u32 formatID;
        u32 formatFlags;
        u32 bytesPerPacket;
        u32 framesPerPacket;
        u32 channelsPerFrame;
        u32 bitsPerChannel;
    };

    //==============================================================================
    static StringPairArray parseUserDefinedChunk (InputStream& input, z64 size)
    {
        StringPairArray infoStrings;
        auto originalPosition = input.getPosition();

        u8 uuid[16];
        input.read (uuid, sizeof (uuid));

        if (memcmp (uuid, "\x29\x81\x92\x73\xB5\xBF\x4A\xEF\xB7\x8D\x62\xD1\xEF\x90\xBB\x2C", 16) == 0)
        {
            auto numEntries = (u32) input.readIntBigEndian();

            for (u32 i = 0; i < numEntries && input.getPosition() < originalPosition + size; ++i)
            {
                Txt keyName = input.readString();
                infoStrings.set (keyName, input.readString());
            }
        }

        input.setPosition (originalPosition + size);
        return infoStrings;
    }

    //==============================================================================
    static StringPairArray parseMidiChunk (InputStream& input, z64 size)
    {
        auto originalPosition = input.getPosition();

        MemoryBlock midiBlock;
        input.readIntoMemoryBlock (midiBlock, (ssize_t) size);
        MemoryInputStream midiInputStream (midiBlock, false);

        StringPairArray midiMetadata;
        MidiFile midiFile;

        if (midiFile.readFrom (midiInputStream))
        {
            midiMetadata.set (CoreAudioFormat::midiDataBase64, midiBlock.toBase64Encoding());

            findTempoEvents (midiFile, midiMetadata);
            findTimeSigEvents (midiFile, midiMetadata);
            findKeySigEvents (midiFile, midiMetadata);
        }

        input.setPosition (originalPosition + size);
        return midiMetadata;
    }

    static z0 findTempoEvents (MidiFile& midiFile, StringPairArray& midiMetadata)
    {
        MidiMessageSequence tempoEvents;
        midiFile.findAllTempoEvents (tempoEvents);

        auto numTempoEvents = tempoEvents.getNumEvents();
        MemoryOutputStream tempoSequence;

        for (i32 i = 0; i < numTempoEvents; ++i)
        {
            auto tempo = getTempoFromTempoMetaEvent (tempoEvents.getEventPointer (i));

            if (tempo > 0.0)
            {
                if (i == 0)
                    midiMetadata.set (CoreAudioFormat::tempo, Txt (tempo));

                if (numTempoEvents > 1)
                    tempoSequence << Txt (tempo) << ',' << tempoEvents.getEventTime (i) << ';';
            }
        }

        if (tempoSequence.getDataSize() > 0)
            midiMetadata.set ("tempo sequence", tempoSequence.toUTF8());
    }

    static f64 getTempoFromTempoMetaEvent (MidiMessageSequence::MidiEventHolder* holder)
    {
        if (holder != nullptr)
        {
            auto& midiMessage = holder->message;

            if (midiMessage.isTempoMetaEvent())
            {
                auto tempoSecondsPerQuarterNote = midiMessage.getTempoSecondsPerQuarterNote();

                if (tempoSecondsPerQuarterNote > 0.0)
                    return 60.0 / tempoSecondsPerQuarterNote;
            }
        }

        return 0.0;
    }

    static z0 findTimeSigEvents (MidiFile& midiFile, StringPairArray& midiMetadata)
    {
        MidiMessageSequence timeSigEvents;
        midiFile.findAllTimeSigEvents (timeSigEvents);
        auto numTimeSigEvents = timeSigEvents.getNumEvents();

        MemoryOutputStream timeSigSequence;

        for (i32 i = 0; i < numTimeSigEvents; ++i)
        {
            i32 numerator, denominator;
            timeSigEvents.getEventPointer (i)->message.getTimeSignatureInfo (numerator, denominator);

            Txt timeSigString;
            timeSigString << numerator << '/' << denominator;

            if (i == 0)
                midiMetadata.set (CoreAudioFormat::timeSig, timeSigString);

            if (numTimeSigEvents > 1)
                timeSigSequence << timeSigString << ',' << timeSigEvents.getEventTime (i) << ';';
        }

        if (timeSigSequence.getDataSize() > 0)
            midiMetadata.set ("time signature sequence", timeSigSequence.toUTF8());
    }

    static z0 findKeySigEvents (MidiFile& midiFile, StringPairArray& midiMetadata)
    {
        MidiMessageSequence keySigEvents;
        midiFile.findAllKeySigEvents (keySigEvents);
        auto numKeySigEvents = keySigEvents.getNumEvents();

        MemoryOutputStream keySigSequence;

        for (i32 i = 0; i < numKeySigEvents; ++i)
        {
            auto& message (keySigEvents.getEventPointer (i)->message);
            auto key = jlimit (0, 14, message.getKeySignatureNumberOfSharpsOrFlats() + 7);
            b8 isMajor = message.isKeySignatureMajorKey();

            static tukk majorKeys[] = { "Cb", "Gb", "Db", "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#", "C#" };
            static tukk minorKeys[] = { "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#" };

            Txt keySigString (isMajor ? majorKeys[key]
                                         : minorKeys[key]);

            if (! isMajor)
                keySigString << 'm';

            if (i == 0)
                midiMetadata.set (CoreAudioFormat::keySig, keySigString);

            if (numKeySigEvents > 1)
                keySigSequence << keySigString << ',' << keySigEvents.getEventTime (i) << ';';
        }

        if (keySigSequence.getDataSize() > 0)
            midiMetadata.set ("key signature sequence", keySigSequence.toUTF8());
    }

    //==============================================================================
    static StringPairArray parseInformationChunk (InputStream& input)
    {
        StringPairArray infoStrings;
        auto numEntries = (u32) input.readIntBigEndian();

        for (u32 i = 0; i < numEntries; ++i)
            infoStrings.set (input.readString(), input.readString());

        return infoStrings;
    }

    //==============================================================================
    static b8 read (InputStream& input, StringPairArray& metadataValues)
    {
        auto originalPos = input.getPosition();

        const FileHeader cafFileHeader (input);
        const b8 isCafFile = cafFileHeader.fileType == chunkName ("caff");

        if (isCafFile)
        {
            while (! input.isExhausted())
            {
                const ChunkHeader chunkHeader (input);

                if (chunkHeader.chunkType == chunkName ("desc"))
                {
                    AudioDescriptionChunk audioDescriptionChunk (input);
                }
                else if (chunkHeader.chunkType == chunkName ("uuid"))
                {
                    metadataValues.addArray (parseUserDefinedChunk (input, chunkHeader.chunkSize));
                }
                else if (chunkHeader.chunkType == chunkName ("data"))
                {
                    // -1 signifies an unknown data size so the data has to be at the
                    // end of the file so we must have finished the header

                    if (chunkHeader.chunkSize == -1)
                        break;

                    input.setPosition (input.getPosition() + chunkHeader.chunkSize);
                }
                else if (chunkHeader.chunkType == chunkName ("midi"))
                {
                    metadataValues.addArray (parseMidiChunk (input, chunkHeader.chunkSize));
                }
                else if (chunkHeader.chunkType == chunkName ("info"))
                {
                    metadataValues.addArray (parseInformationChunk (input));
                }
                else
                {
                    // we aren't decoding this chunk yet so just skip over it
                    input.setPosition (input.getPosition() + chunkHeader.chunkSize);
                }
            }
        }

        input.setPosition (originalPos);

        return isCafFile;
    }
};

//==============================================================================
class CoreAudioReader final : public AudioFormatReader
{
public:
    using StreamKind = CoreAudioFormat::StreamKind;

    CoreAudioReader (InputStream* inp, StreamKind streamKind)
        : AudioFormatReader (inp, coreAudioFormatName)
    {
        usesFloatingPointData = true;
        bitsPerSample = 32;

        if (input != nullptr)
            CoreAudioFormatMetatdata::read (*input, metadataValues);

        auto status = AudioFileOpenWithCallbacks (this,
                                                  &readCallback,
                                                  nullptr,  // write needs to be null to avoid permissions errors
                                                  &getSizeCallback,
                                                  nullptr,  // setSize needs to be null to avoid permissions errors
                                                  toAudioFileTypeID (streamKind),
                                                  &audioFileID);
        if (status == noErr)
        {
            status = ExtAudioFileWrapAudioFileID (audioFileID, false, &audioFileRef);

            if (status == noErr)
            {
                AudioStreamBasicDescription sourceAudioFormat;
                UInt32 audioStreamBasicDescriptionSize = sizeof (AudioStreamBasicDescription);
                ExtAudioFileGetProperty (audioFileRef,
                                         kExtAudioFileProperty_FileDataFormat,
                                         &audioStreamBasicDescriptionSize,
                                         &sourceAudioFormat);

                numChannels = sourceAudioFormat.mChannelsPerFrame;
                sampleRate  = sourceAudioFormat.mSampleRate;

                UInt32 sizeOfLengthProperty = sizeof (z64);
                ExtAudioFileGetProperty (audioFileRef,
                                         kExtAudioFileProperty_FileLengthFrames,
                                         &sizeOfLengthProperty,
                                         &lengthInSamples);

                HeapBlock<AudioChannelLayout> caLayout;
                b8 hasLayout = false;
                UInt32 sizeOfLayout = 0, isWritable = 0;

                status = AudioFileGetPropertyInfo (audioFileID, kAudioFilePropertyChannelLayout, &sizeOfLayout, &isWritable);

                if (status == noErr && sizeOfLayout >= (sizeof (AudioChannelLayout) - sizeof (AudioChannelDescription)))
                {
                    caLayout.malloc (1, static_cast<size_t> (sizeOfLayout));

                    status = AudioFileGetProperty (audioFileID, kAudioFilePropertyChannelLayout,
                                                   &sizeOfLayout, caLayout.get());

                    if (status == noErr)
                    {
                        auto fileLayout = CoreAudioLayouts::fromCoreAudio (*caLayout.get());

                        if (fileLayout.size() == static_cast<i32> (numChannels))
                        {
                            hasLayout = true;
                            channelSet = fileLayout;
                        }
                    }
                }

                destinationAudioFormat.mSampleRate       = sampleRate;
                destinationAudioFormat.mFormatID         = kAudioFormatLinearPCM;
                destinationAudioFormat.mFormatFlags      = kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsNonInterleaved | kAudioFormatFlagsNativeEndian;
                destinationAudioFormat.mBitsPerChannel   = sizeof (f32) * 8;
                destinationAudioFormat.mChannelsPerFrame = numChannels;
                destinationAudioFormat.mBytesPerFrame    = sizeof (f32);
                destinationAudioFormat.mFramesPerPacket  = 1;
                destinationAudioFormat.mBytesPerPacket   = destinationAudioFormat.mFramesPerPacket * destinationAudioFormat.mBytesPerFrame;

                status = ExtAudioFileSetProperty (audioFileRef,
                                                  kExtAudioFileProperty_ClientDataFormat,
                                                  sizeof (AudioStreamBasicDescription),
                                                  &destinationAudioFormat);
                if (status == noErr)
                {
                    bufferList.malloc (1, sizeof (AudioBufferList) + numChannels * sizeof (::AudioBuffer));
                    bufferList->mNumberBuffers = numChannels;
                    channelMap.malloc (numChannels);

                    if (hasLayout && caLayout != nullptr)
                    {
                        auto caOrder = CoreAudioLayouts::getCoreAudioLayoutChannels (*caLayout);

                        for (i32 i = 0; i < static_cast<i32> (numChannels); ++i)
                        {
                            auto idx = channelSet.getChannelIndexForType (caOrder.getReference (i));
                            jassert (isPositiveAndBelow (idx, static_cast<i32> (numChannels)));

                            channelMap[i] = idx;
                        }
                    }
                    else
                    {
                        for (i32 i = 0; i < static_cast<i32> (numChannels); ++i)
                            channelMap[i] = i;
                    }

                    ok = true;
                }
            }
        }
    }

    ~CoreAudioReader() override
    {
        ExtAudioFileDispose (audioFileRef);
        AudioFileClose (audioFileID);
    }

    //==============================================================================
    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (numSamples <= 0)
            return true;

        if (lastReadPosition != startSampleInFile)
        {
            OSStatus status = ExtAudioFileSeek (audioFileRef, startSampleInFile);
            if (status != noErr)
                return false;

            lastReadPosition = startSampleInFile;
        }

        while (numSamples > 0)
        {
            auto numThisTime = jmin (8192, numSamples);
            auto numBytes = (size_t) numThisTime * sizeof (f32);

            audioDataBlock.ensureSize (numBytes * numChannels, false);
            auto* data = static_cast<f32*> (audioDataBlock.getData());

            for (i32 j = (i32) numChannels; --j >= 0;)
            {
                bufferList->mBuffers[j].mNumberChannels = 1;
                bufferList->mBuffers[j].mDataByteSize = (UInt32) numBytes;
                bufferList->mBuffers[j].mData = data;
                data += numThisTime;
            }

            auto numFramesToRead = (UInt32) numThisTime;
            auto status = ExtAudioFileRead (audioFileRef, &numFramesToRead, bufferList);

            if (status != noErr)
                return false;

            if (numFramesToRead == 0)
                break;

            if ((i32) numFramesToRead < numThisTime)
            {
                numThisTime = (i32) numFramesToRead;
                numBytes    = (size_t) numThisTime * sizeof (f32);
            }

            for (i32 i = numDestChannels; --i >= 0;)
            {
                auto* dest = destSamples[(i < (i32) numChannels ? channelMap[i] : i)];

                if (dest != nullptr)
                {
                    if (i < (i32) numChannels)
                        memcpy (dest + startOffsetInDestBuffer, bufferList->mBuffers[i].mData, numBytes);
                    else
                        zeromem (dest + startOffsetInDestBuffer, numBytes);
                }
            }

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
            lastReadPosition += numThisTime;
        }

        return true;
    }

    AudioChannelSet getChannelLayout() override
    {
        if (channelSet.size() == static_cast<i32> (numChannels))
            return channelSet;

        return AudioFormatReader::getChannelLayout();
    }

    b8 ok = false;

private:
    AudioFileID audioFileID;
    ExtAudioFileRef audioFileRef;
    AudioChannelSet channelSet;
    AudioStreamBasicDescription destinationAudioFormat;
    MemoryBlock audioDataBlock;
    HeapBlock<AudioBufferList> bufferList;
    z64 lastReadPosition = 0;
    HeapBlock<i32> channelMap;

    static SInt64 getSizeCallback (uk inClientData)
    {
        return static_cast<CoreAudioReader*> (inClientData)->input->getTotalLength();
    }

    static OSStatus readCallback (uk inClientData, SInt64 inPosition, UInt32 requestCount,
                                  uk buffer, UInt32* actualCount)
    {
        auto* reader = static_cast<CoreAudioReader*> (inClientData);
        reader->input->setPosition (inPosition);
        *actualCount = (UInt32) reader->input->read (buffer, (i32) requestCount);
        return noErr;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioReader)
};

//==============================================================================
CoreAudioFormat::CoreAudioFormat()
    : AudioFormat (coreAudioFormatName, findFileExtensionsForCoreAudioCodecs()),
      streamKind (StreamKind::kNone)
{
}

CoreAudioFormat::CoreAudioFormat (StreamKind kind)
    : AudioFormat (coreAudioFormatName, findFileExtensionsForCoreAudioCodec (toAudioFileTypeID (kind))),
      streamKind (kind)
{
}

CoreAudioFormat::~CoreAudioFormat() = default;

Array<i32> CoreAudioFormat::getPossibleSampleRates()    { return {}; }
Array<i32> CoreAudioFormat::getPossibleBitDepths()      { return {}; }

b8 CoreAudioFormat::canDoStereo()     { return true; }
b8 CoreAudioFormat::canDoMono()       { return true; }

//==============================================================================
AudioFormatReader* CoreAudioFormat::createReaderFor (InputStream* sourceStream,
                                                     b8 deleteStreamIfOpeningFails)
{
    std::unique_ptr<CoreAudioReader> r (new CoreAudioReader (sourceStream, streamKind));

    if (r->ok)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

AudioFormatWriter* CoreAudioFormat::createWriterFor (OutputStream*,
                                                     f64 /*sampleRateToUse*/,
                                                     u32 /*numberOfChannels*/,
                                                     i32 /*bitsPerSample*/,
                                                     const StringPairArray& /*metadataValues*/,
                                                     i32 /*qualityOptionIndex*/)
{
    jassertfalse; // not yet implemented!
    return nullptr;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

#define DEFINE_CHANNEL_LAYOUT_DFL_ENTRY(x) CoreAudioChannelLayoutTag { x, #x, AudioChannelSet() }
#define DEFINE_CHANNEL_LAYOUT_TAG_ENTRY(x, y) CoreAudioChannelLayoutTag { x, #x, y }

class CoreAudioLayoutsUnitTest final : public UnitTest
{
public:
    CoreAudioLayoutsUnitTest()
        : UnitTest ("Core Audio Layout <-> DRX channel layout conversion", UnitTestCategories::audio)
    {}

    // some ambisonic tags which are not explicitly defined
    enum
    {
        kAudioChannelLayoutTag_HOA_ACN_SN3D_0Order = (190U<<16) | 1,
        kAudioChannelLayoutTag_HOA_ACN_SN3D_1Order = (190U<<16) | 4,
        kAudioChannelLayoutTag_HOA_ACN_SN3D_2Order = (190U<<16) | 9,
        kAudioChannelLayoutTag_HOA_ACN_SN3D_3Order = (190U<<16) | 16,
        kAudioChannelLayoutTag_HOA_ACN_SN3D_4Order = (190U<<16) | 25,
        kAudioChannelLayoutTag_HOA_ACN_SN3D_5Order = (190U<<16) | 36
    };

    z0 runTest() override
    {
        auto& knownTags = getAllKnownLayoutTags();

        {
            // Check that all known tags defined in CoreAudio SDK version 10.12.4 are known to DRX
            // Include all defined tags even if there are duplicates as Apple will sometimes change
            // definitions
            beginTest ("All CA tags handled");

            for (auto tagEntry : knownTags)
            {
                auto labels = CoreAudioLayouts::fromCoreAudio (tagEntry.tag);

                expect (! labels.isDiscreteLayout(), "Tag \"" + Txt (tagEntry.name) + "\" is not handled by DRX");
            }
        }

        {
            beginTest ("Number of speakers");

            for (auto tagEntry : knownTags)
            {
                auto labels = CoreAudioLayouts::getSpeakerLayoutForCoreAudioTag (tagEntry.tag);

                expect (labels.size() == (tagEntry.tag & 0xffff), "Tag \"" + Txt (tagEntry.name) + "\" has incorrect channel count");
            }
        }

        {
            beginTest ("No duplicate speaker");

            for (auto tagEntry : knownTags)
            {
                auto labels = CoreAudioLayouts::getSpeakerLayoutForCoreAudioTag (tagEntry.tag);
                labels.sort();

                for (i32 i = 0; i < (labels.size() - 1); ++i)
                    expect (labels.getReference (i) != labels.getReference (i + 1),
                            "Tag \"" + Txt (tagEntry.name) + "\" has the same speaker twice");
            }
        }

        {
            beginTest ("CA speaker list and drx layouts are consistent");

            for (auto tagEntry : knownTags)
                expect (AudioChannelSet::channelSetWithChannels (CoreAudioLayouts::getSpeakerLayoutForCoreAudioTag (tagEntry.tag))
                            == CoreAudioLayouts::fromCoreAudio (tagEntry.tag),
                        "Tag \"" + Txt (tagEntry.name) + "\" is not converted consistently by DRX");
        }

        {
            beginTest ("AudioChannelSet documentation is correct");

            for (auto tagEntry : knownTags)
            {
                if (tagEntry.equivalentChannelSet.isDisabled())
                    continue;

                expect (CoreAudioLayouts::fromCoreAudio (tagEntry.tag) == tagEntry.equivalentChannelSet,
                        "Documentation for tag \"" + Txt (tagEntry.name) + "\" is incorrect");
            }
        }

        {
            beginTest ("CA tag reverse conversion");

            for (auto tagEntry : knownTags)
            {
                if (tagEntry.equivalentChannelSet.isDisabled())
                    continue;

                expect (CoreAudioLayouts::toCoreAudio (tagEntry.equivalentChannelSet) == tagEntry.tag,
                        "Incorrect reverse conversion for tag \"" + Txt (tagEntry.name) + "\"");
            }
        }
    }

private:
    struct CoreAudioChannelLayoutTag
    {
        AudioChannelLayoutTag tag;
        tukk name;
        AudioChannelSet equivalentChannelSet; /* referred to this in the AudioChannelSet documentation */
    };

    //==============================================================================
    const Array<CoreAudioChannelLayoutTag>& getAllKnownLayoutTags() const
    {
        static CoreAudioChannelLayoutTag tags[] = {
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_Mono,   AudioChannelSet::mono()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_Stereo, AudioChannelSet::stereo()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_StereoHeadphones),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MatrixStereo),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MidSide),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_XY),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_Binaural),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_Ambisonic_B_Format),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_Quadraphonic, AudioChannelSet::quadraphonic()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_Pentagonal, AudioChannelSet::pentagonal()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_Hexagonal, AudioChannelSet::hexagonal()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_Octagonal, AudioChannelSet::octagonal()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_Cube),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_1_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_2_0),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_3_0_A, AudioChannelSet::createLCR()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_3_0_B),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_4_0_A, AudioChannelSet::createLCRS()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_4_0_B),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_5_0_A, AudioChannelSet::create5point0()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_5_0_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_5_0_C),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_5_0_D),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_5_1_A, AudioChannelSet::create5point1()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_5_1_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_5_1_C),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_5_1_D),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_6_1_A, AudioChannelSet::create6point1()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_7_1_A, AudioChannelSet::create7point1SDDS()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_MPEG_7_1_B),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_MPEG_7_1_C, AudioChannelSet::create7point1()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_Emagic_Default_7_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_SMPTE_DTV),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_1_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_2_0),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_ITU_2_1, AudioChannelSet::createLRS()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_2_2),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_3_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_3_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_3_2),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_3_2_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_ITU_3_4_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_2),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_3),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_4),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_5),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_6),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_7),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_8),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_9),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_10),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_11),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_12),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_13),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_14),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_15),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_16),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_17),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_18),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_19),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DVD_20),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_4),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_5),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_6),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_8),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_5_0),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_AudioUnit_6_0, AudioChannelSet::create6point0()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_AudioUnit_7_0, AudioChannelSet::create7point0()),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_AudioUnit_7_0_Front, AudioChannelSet::create7point0SDDS()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_5_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_6_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_7_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AudioUnit_7_1_Front),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_3_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_Quadraphonic),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_4_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_5_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_5_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_6_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_6_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_7_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_7_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_7_1_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_7_1_C),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AAC_Octagonal),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_TMH_10_2_std),
            // DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_TMH_10_2_full), no indication on how to handle this tag
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AC3_1_0_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AC3_3_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AC3_3_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AC3_3_0_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AC3_2_1_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_AC3_3_1_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC_6_0_A),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC_7_0_A),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_6_1_A),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_6_1_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_6_1_C),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_A),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_C),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_D),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_E),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_F),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_G),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_EAC3_7_1_H),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_3_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_4_1),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_DTS_6_0_A, AudioChannelSet::create6point0Music()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_6_0_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_6_0_C),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_DTS_6_1_A, AudioChannelSet::create6point1Music()),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_6_1_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_6_1_C),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_7_0),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_7_1),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_8_0_A),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_8_0_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_8_1_A),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_8_1_B),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_6_1_D),
            DEFINE_CHANNEL_LAYOUT_DFL_ENTRY (kAudioChannelLayoutTag_DTS_6_1_D),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_HOA_ACN_SN3D_0Order,  AudioChannelSet::ambisonic (0)),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_HOA_ACN_SN3D_1Order,  AudioChannelSet::ambisonic (1)),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_HOA_ACN_SN3D_2Order,  AudioChannelSet::ambisonic (2)),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_HOA_ACN_SN3D_3Order, AudioChannelSet::ambisonic (3)),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_HOA_ACN_SN3D_4Order, AudioChannelSet::ambisonic (4)),
            DEFINE_CHANNEL_LAYOUT_TAG_ENTRY (kAudioChannelLayoutTag_HOA_ACN_SN3D_5Order, AudioChannelSet::ambisonic (5))
        };
        static Array<CoreAudioChannelLayoutTag> knownTags (tags, sizeof (tags) / sizeof (CoreAudioChannelLayoutTag));

        return knownTags;
    }
};

static CoreAudioLayoutsUnitTest coreAudioLayoutsUnitTest;

#endif

} // namespace drx

#endif
