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

using StringMap = std::unordered_map<Txt, Txt>;

static auto toMap (const StringPairArray& array)
{
    StringMap result;

    for (auto i = 0; i < array.size(); ++i)
        result[array.getAllKeys()[i]] = array.getAllValues()[i];

    return result;
}

static auto getValueWithDefault (const StringMap& m, const Txt& key, const Txt& fallback = {})
{
    const auto iter = m.find (key);
    return iter != m.cend() ? iter->second : fallback;
}

static tukk const wavFormatName = "WAV file";

//==============================================================================
tukk const WavAudioFormat::bwavDescription      = "bwav description";
tukk const WavAudioFormat::bwavOriginator       = "bwav originator";
tukk const WavAudioFormat::bwavOriginatorRef    = "bwav originator ref";
tukk const WavAudioFormat::bwavOriginationDate  = "bwav origination date";
tukk const WavAudioFormat::bwavOriginationTime  = "bwav origination time";
tukk const WavAudioFormat::bwavTimeReference    = "bwav time reference";
tukk const WavAudioFormat::bwavCodingHistory    = "bwav coding history";

StringPairArray WavAudioFormat::createBWAVMetadata (const Txt& description,
                                                    const Txt& originator,
                                                    const Txt& originatorRef,
                                                    Time date,
                                                    z64 timeReferenceSamples,
                                                    const Txt& codingHistory)
{
    StringPairArray m;

    m.set (bwavDescription, description);
    m.set (bwavOriginator, originator);
    m.set (bwavOriginatorRef, originatorRef);
    m.set (bwavOriginationDate, date.formatted ("%Y-%m-%d"));
    m.set (bwavOriginationTime, date.formatted ("%H:%M:%S"));
    m.set (bwavTimeReference, Txt (timeReferenceSamples));
    m.set (bwavCodingHistory, codingHistory);

    return m;
}

tukk const WavAudioFormat::acidOneShot          = "acid one shot";
tukk const WavAudioFormat::acidRootSet          = "acid root set";
tukk const WavAudioFormat::acidStretch          = "acid stretch";
tukk const WavAudioFormat::acidDiskBased        = "acid disk based";
tukk const WavAudioFormat::acidizerFlag         = "acidizer flag";
tukk const WavAudioFormat::acidRootNote         = "acid root note";
tukk const WavAudioFormat::acidBeats            = "acid beats";
tukk const WavAudioFormat::acidDenominator      = "acid denominator";
tukk const WavAudioFormat::acidNumerator        = "acid numerator";
tukk const WavAudioFormat::acidTempo            = "acid tempo";

tukk const WavAudioFormat::riffInfoArchivalLocation      = "IARL";
tukk const WavAudioFormat::riffInfoArtist                = "IART";
tukk const WavAudioFormat::riffInfoBaseURL               = "IBSU";
tukk const WavAudioFormat::riffInfoCinematographer       = "ICNM";
tukk const WavAudioFormat::riffInfoComment               = "CMNT";
tukk const WavAudioFormat::riffInfoComment2              = "ICMT";
tukk const WavAudioFormat::riffInfoComments              = "COMM";
tukk const WavAudioFormat::riffInfoCommissioned          = "ICMS";
tukk const WavAudioFormat::riffInfoCopyright             = "ICOP";
tukk const WavAudioFormat::riffInfoCostumeDesigner       = "ICDS";
tukk const WavAudioFormat::riffInfoCountry               = "ICNT";
tukk const WavAudioFormat::riffInfoCropped               = "ICRP";
tukk const WavAudioFormat::riffInfoDateCreated           = "ICRD";
tukk const WavAudioFormat::riffInfoDateTimeOriginal      = "IDIT";
tukk const WavAudioFormat::riffInfoDefaultAudioStream    = "ICAS";
tukk const WavAudioFormat::riffInfoDimension             = "IDIM";
tukk const WavAudioFormat::riffInfoDirectory             = "DIRC";
tukk const WavAudioFormat::riffInfoDistributedBy         = "IDST";
tukk const WavAudioFormat::riffInfoDotsPerInch           = "IDPI";
tukk const WavAudioFormat::riffInfoEditedBy              = "IEDT";
tukk const WavAudioFormat::riffInfoEighthLanguage        = "IAS8";
tukk const WavAudioFormat::riffInfoEncodedBy             = "CODE";
tukk const WavAudioFormat::riffInfoEndTimecode           = "TCDO";
tukk const WavAudioFormat::riffInfoEngineer              = "IENG";
tukk const WavAudioFormat::riffInfoFifthLanguage         = "IAS5";
tukk const WavAudioFormat::riffInfoFirstLanguage         = "IAS1";
tukk const WavAudioFormat::riffInfoFourthLanguage        = "IAS4";
tukk const WavAudioFormat::riffInfoGenre                 = "GENR";
tukk const WavAudioFormat::riffInfoKeywords              = "IKEY";
tukk const WavAudioFormat::riffInfoLanguage              = "LANG";
tukk const WavAudioFormat::riffInfoLength                = "TLEN";
tukk const WavAudioFormat::riffInfoLightness             = "ILGT";
tukk const WavAudioFormat::riffInfoLocation              = "LOCA";
tukk const WavAudioFormat::riffInfoLogoIconURL           = "ILIU";
tukk const WavAudioFormat::riffInfoLogoURL               = "ILGU";
tukk const WavAudioFormat::riffInfoMedium                = "IMED";
tukk const WavAudioFormat::riffInfoMoreInfoBannerImage   = "IMBI";
tukk const WavAudioFormat::riffInfoMoreInfoBannerURL     = "IMBU";
tukk const WavAudioFormat::riffInfoMoreInfoText          = "IMIT";
tukk const WavAudioFormat::riffInfoMoreInfoURL           = "IMIU";
tukk const WavAudioFormat::riffInfoMusicBy               = "IMUS";
tukk const WavAudioFormat::riffInfoNinthLanguage         = "IAS9";
tukk const WavAudioFormat::riffInfoNumberOfParts         = "PRT2";
tukk const WavAudioFormat::riffInfoOrganisation          = "TORG";
tukk const WavAudioFormat::riffInfoPart                  = "PRT1";
tukk const WavAudioFormat::riffInfoProducedBy            = "IPRO";
tukk const WavAudioFormat::riffInfoProductName           = "IPRD";
tukk const WavAudioFormat::riffInfoProductionDesigner    = "IPDS";
tukk const WavAudioFormat::riffInfoProductionStudio      = "ISDT";
tukk const WavAudioFormat::riffInfoRate                  = "RATE";
tukk const WavAudioFormat::riffInfoRated                 = "AGES";
tukk const WavAudioFormat::riffInfoRating                = "IRTD";
tukk const WavAudioFormat::riffInfoRippedBy              = "IRIP";
tukk const WavAudioFormat::riffInfoSecondaryGenre        = "ISGN";
tukk const WavAudioFormat::riffInfoSecondLanguage        = "IAS2";
tukk const WavAudioFormat::riffInfoSeventhLanguage       = "IAS7";
tukk const WavAudioFormat::riffInfoSharpness             = "ISHP";
tukk const WavAudioFormat::riffInfoSixthLanguage         = "IAS6";
tukk const WavAudioFormat::riffInfoSoftware              = "ISFT";
tukk const WavAudioFormat::riffInfoSoundSchemeTitle      = "DISP";
tukk const WavAudioFormat::riffInfoSource                = "ISRC";
tukk const WavAudioFormat::riffInfoSourceFrom            = "ISRF";
tukk const WavAudioFormat::riffInfoStarring_ISTR         = "ISTR";
tukk const WavAudioFormat::riffInfoStarring_STAR         = "STAR";
tukk const WavAudioFormat::riffInfoStartTimecode         = "TCOD";
tukk const WavAudioFormat::riffInfoStatistics            = "STAT";
tukk const WavAudioFormat::riffInfoSubject               = "ISBJ";
tukk const WavAudioFormat::riffInfoTapeName              = "TAPE";
tukk const WavAudioFormat::riffInfoTechnician            = "ITCH";
tukk const WavAudioFormat::riffInfoThirdLanguage         = "IAS3";
tukk const WavAudioFormat::riffInfoTimeCode              = "ISMP";
tukk const WavAudioFormat::riffInfoTitle                 = "INAM";
tukk const WavAudioFormat::riffInfoTrackNo               = "IPRT";
tukk const WavAudioFormat::riffInfoTrackNumber           = "TRCK";
tukk const WavAudioFormat::riffInfoURL                   = "TURL";
tukk const WavAudioFormat::riffInfoVegasVersionMajor     = "VMAJ";
tukk const WavAudioFormat::riffInfoVegasVersionMinor     = "VMIN";
tukk const WavAudioFormat::riffInfoVersion               = "TVER";
tukk const WavAudioFormat::riffInfoWatermarkURL          = "IWMU";
tukk const WavAudioFormat::riffInfoWrittenBy             = "IWRI";
tukk const WavAudioFormat::riffInfoYear                  = "YEAR";

tukk const WavAudioFormat::aswgContentType               = "contentType";
tukk const WavAudioFormat::aswgProject                   = "project";
tukk const WavAudioFormat::aswgOriginator                = "originator";
tukk const WavAudioFormat::aswgOriginatorStudio          = "originatorStudio";
tukk const WavAudioFormat::aswgNotes                     = "notes";
tukk const WavAudioFormat::aswgSession                   = "session";
tukk const WavAudioFormat::aswgState                     = "state";
tukk const WavAudioFormat::aswgEditor                    = "editor";
tukk const WavAudioFormat::aswgMixer                     = "mixer";
tukk const WavAudioFormat::aswgFxChainName               = "fxChainName";
tukk const WavAudioFormat::aswgChannelConfig             = "channelConfig";
tukk const WavAudioFormat::aswgAmbisonicFormat           = "ambisonicFormat";
tukk const WavAudioFormat::aswgAmbisonicChnOrder         = "ambisonicChnOrder";
tukk const WavAudioFormat::aswgAmbisonicNorm             = "ambisonicNorm";
tukk const WavAudioFormat::aswgMicType                   = "micType";
tukk const WavAudioFormat::aswgMicConfig                 = "micConfig";
tukk const WavAudioFormat::aswgMicDistance               = "micDistance";
tukk const WavAudioFormat::aswgRecordingLoc              = "recordingLoc";
tukk const WavAudioFormat::aswgIsDesigned                = "isDesigned";
tukk const WavAudioFormat::aswgRecEngineer               = "recEngineer";
tukk const WavAudioFormat::aswgRecStudio                 = "recStudio";
tukk const WavAudioFormat::aswgImpulseLocation           = "impulseLocation";
tukk const WavAudioFormat::aswgCategory                  = "category";
tukk const WavAudioFormat::aswgSubCategory               = "subCategory";
tukk const WavAudioFormat::aswgCatId                     = "catId";
tukk const WavAudioFormat::aswgUserCategory              = "userCategory";
tukk const WavAudioFormat::aswgUserData                  = "userData";
tukk const WavAudioFormat::aswgVendorCategory            = "vendorCategory";
tukk const WavAudioFormat::aswgFxName                    = "fxName";
tukk const WavAudioFormat::aswgLibrary                   = "library";
tukk const WavAudioFormat::aswgCreatorId                 = "creatorId";
tukk const WavAudioFormat::aswgSourceId                  = "sourceId";
tukk const WavAudioFormat::aswgRmsPower                  = "rmsPower";
tukk const WavAudioFormat::aswgLoudness                  = "loudness";
tukk const WavAudioFormat::aswgLoudnessRange             = "loudnessRange";
tukk const WavAudioFormat::aswgMaxPeak                   = "maxPeak";
tukk const WavAudioFormat::aswgSpecDensity               = "specDensity";
tukk const WavAudioFormat::aswgZeroCrossRate             = "zeroCrossRate";
tukk const WavAudioFormat::aswgPapr                      = "papr";
tukk const WavAudioFormat::aswgText                      = "text";
tukk const WavAudioFormat::aswgEfforts                   = "efforts";
tukk const WavAudioFormat::aswgEffortType                = "effortType";
tukk const WavAudioFormat::aswgProjection                = "projection";
tukk const WavAudioFormat::aswgLanguage                  = "language";
tukk const WavAudioFormat::aswgTimingRestriction         = "timingRestriction";
tukk const WavAudioFormat::aswgCharacterName             = "characterName";
tukk const WavAudioFormat::aswgCharacterGender           = "characterGender";
tukk const WavAudioFormat::aswgCharacterAge              = "characterAge";
tukk const WavAudioFormat::aswgCharacterRole             = "characterRole";
tukk const WavAudioFormat::aswgActorName                 = "actorName";
tukk const WavAudioFormat::aswgActorGender               = "actorGender";
tukk const WavAudioFormat::aswgDirector                  = "director";
tukk const WavAudioFormat::aswgDirection                 = "direction";
tukk const WavAudioFormat::aswgFxUsed                    = "fxUsed";
tukk const WavAudioFormat::aswgUsageRights               = "usageRights";
tukk const WavAudioFormat::aswgIsUnion                   = "isUnion";
tukk const WavAudioFormat::aswgAccent                    = "accent";
tukk const WavAudioFormat::aswgEmotion                   = "emotion";
tukk const WavAudioFormat::aswgComposor                  = "composor";
tukk const WavAudioFormat::aswgArtist                    = "artist";
tukk const WavAudioFormat::aswgSongTitle                 = "songTitle";
tukk const WavAudioFormat::aswgGenre                     = "genre";
tukk const WavAudioFormat::aswgSubGenre                  = "subGenre";
tukk const WavAudioFormat::aswgProducer                  = "producer";
tukk const WavAudioFormat::aswgMusicSup                  = "musicSup";
tukk const WavAudioFormat::aswgInstrument                = "instrument";
tukk const WavAudioFormat::aswgMusicPublisher            = "musicPublisher";
tukk const WavAudioFormat::aswgRightsOwner               = "rightsOwner";
tukk const WavAudioFormat::aswgIsSource                  = "isSource";
tukk const WavAudioFormat::aswgIsLoop                    = "isLoop";
tukk const WavAudioFormat::aswgIntensity                 = "intensity";
tukk const WavAudioFormat::aswgIsFinal                   = "isFinal";
tukk const WavAudioFormat::aswgOrderRef                  = "orderRef";
tukk const WavAudioFormat::aswgIsOst                     = "isOst";
tukk const WavAudioFormat::aswgIsCinematic               = "isCinematic";
tukk const WavAudioFormat::aswgIsLicensed                = "isLicensed";
tukk const WavAudioFormat::aswgIsDiegetic                = "isDiegetic";
tukk const WavAudioFormat::aswgMusicVersion              = "musicVersion";
tukk const WavAudioFormat::aswgIsrcId                    = "isrcId";
tukk const WavAudioFormat::aswgTempo                     = "tempo";
tukk const WavAudioFormat::aswgTimeSig                   = "timeSig";
tukk const WavAudioFormat::aswgInKey                     = "inKey";
tukk const WavAudioFormat::aswgBillingCode               = "billingCode";
tukk const WavAudioFormat::aswgVersion                   = "IXML_VERSION";

tukk const WavAudioFormat::ISRC                                  = "ISRC";
tukk const WavAudioFormat::internationalStandardRecordingCode    = "international standard recording code";
tukk const WavAudioFormat::tracktionLoopInfo                     = "tracktion loop info";

//==============================================================================
namespace WavFileHelpers
{
    constexpr inline i32 chunkName (tukk name) noexcept         { return (i32) ByteOrder::littleEndianInt (name); }
    constexpr inline size_t roundUpSize (size_t sz) noexcept           { return (sz + 3) & ~3u; }

    #if DRX_MSVC
     #pragma pack (push, 1)
    #endif

    struct BWAVChunk
    {
        t8 description[256];
        t8 originator[32];
        t8 originatorRef[32];
        t8 originationDate[10];
        t8 originationTime[8];
        u32 timeRefLow;
        u32 timeRefHigh;
        u16 version;
        u8 umid[64];
        u8 reserved[190];
        t8 codingHistory[1];

        z0 copyTo (StringMap& values, i32k totalSize) const
        {
            values[WavAudioFormat::bwavDescription]     = Txt::fromUTF8 (description,     sizeof (description));
            values[WavAudioFormat::bwavOriginator]      = Txt::fromUTF8 (originator,      sizeof (originator));
            values[WavAudioFormat::bwavOriginatorRef]   = Txt::fromUTF8 (originatorRef,   sizeof (originatorRef));
            values[WavAudioFormat::bwavOriginationDate] = Txt::fromUTF8 (originationDate, sizeof (originationDate));
            values[WavAudioFormat::bwavOriginationTime] = Txt::fromUTF8 (originationTime, sizeof (originationTime));

            auto timeLow  = ByteOrder::swapIfBigEndian (timeRefLow);
            auto timeHigh = ByteOrder::swapIfBigEndian (timeRefHigh);
            auto time = (((z64) timeHigh) << 32) + timeLow;

            values[WavAudioFormat::bwavTimeReference] = Txt (time);
            values[WavAudioFormat::bwavCodingHistory] = Txt::fromUTF8 (codingHistory, totalSize - (i32) offsetof (BWAVChunk, codingHistory));
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            MemoryBlock data (roundUpSize (sizeof (BWAVChunk) + getValueWithDefault (values, WavAudioFormat::bwavCodingHistory).getNumBytesAsUTF8()));
            data.fillWith (0);

            auto* b = (BWAVChunk*) data.getData();

            // Allow these calls to overwrite an extra byte at the end, which is fine as i64
            // as they get called in the right order.
            getValueWithDefault (values, WavAudioFormat::bwavDescription)    .copyToUTF8 (b->description, 257);
            getValueWithDefault (values, WavAudioFormat::bwavOriginator)     .copyToUTF8 (b->originator, 33);
            getValueWithDefault (values, WavAudioFormat::bwavOriginatorRef)  .copyToUTF8 (b->originatorRef, 33);
            getValueWithDefault (values, WavAudioFormat::bwavOriginationDate).copyToUTF8 (b->originationDate, 11);
            getValueWithDefault (values, WavAudioFormat::bwavOriginationTime).copyToUTF8 (b->originationTime, 9);

            auto time = getValueWithDefault (values, WavAudioFormat::bwavTimeReference).getLargeIntValue();
            b->timeRefLow = ByteOrder::swapIfBigEndian ((u32) (time & 0xffffffff));
            b->timeRefHigh = ByteOrder::swapIfBigEndian ((u32) (time >> 32));

            getValueWithDefault (values, WavAudioFormat::bwavCodingHistory).copyToUTF8 (b->codingHistory, 0x7fffffff);

            if (b->description[0] != 0
                || b->originator[0] != 0
                || b->originationDate[0] != 0
                || b->originationTime[0] != 0
                || b->codingHistory[0] != 0
                || time != 0)
            {
                return data;
            }

            return {};
        }

    } DRX_PACKED;

    //==============================================================================
    inline AudioChannelSet canonicalWavChannelSet (i32 numChannels)
    {
        if (numChannels == 1)  return AudioChannelSet::mono();
        if (numChannels == 2)  return AudioChannelSet::stereo();
        if (numChannels == 3)  return AudioChannelSet::createLCR();
        if (numChannels == 4)  return AudioChannelSet::quadraphonic();
        if (numChannels == 5)  return AudioChannelSet::create5point0();
        if (numChannels == 6)  return AudioChannelSet::create5point1();
        if (numChannels == 7)  return AudioChannelSet::create7point0SDDS();
        if (numChannels == 8)  return AudioChannelSet::create7point1SDDS();

        return AudioChannelSet::discreteChannels (numChannels);
    }

    //==============================================================================
    struct SMPLChunk
    {
        struct SampleLoop
        {
            u32 identifier;
            u32 type; // these are different in AIFF and WAV
            u32 start;
            u32 end;
            u32 fraction;
            u32 playCount;
        } DRX_PACKED;

        u32 manufacturer;
        u32 product;
        u32 samplePeriod;
        u32 midiUnityNote;
        u32 midiPitchFraction;
        u32 smpteFormat;
        u32 smpteOffset;
        u32 numSampleLoops;
        u32 samplerData;
        SampleLoop loops[1];

        template <typename NameType>
        static z0 setValue (StringMap& values, NameType name, u32 val)
        {
            values[name] = Txt (ByteOrder::swapIfBigEndian (val));
        }

        static z0 setValue (StringMap& values, i32 prefix, tukk name, u32 val)
        {
            setValue (values, "Loop" + Txt (prefix) + name, val);
        }

        z0 copyTo (StringMap& values, i32k totalSize) const
        {
            setValue (values, "Manufacturer",      manufacturer);
            setValue (values, "Product",           product);
            setValue (values, "SamplePeriod",      samplePeriod);
            setValue (values, "MidiUnityNote",     midiUnityNote);
            setValue (values, "MidiPitchFraction", midiPitchFraction);
            setValue (values, "SmpteFormat",       smpteFormat);
            setValue (values, "SmpteOffset",       smpteOffset);
            setValue (values, "NumSampleLoops",    numSampleLoops);
            setValue (values, "SamplerData",       samplerData);

            for (i32 i = 0; i < (i32) numSampleLoops; ++i)
            {
                if ((u8*) (loops + (i + 1)) > ((u8*) this) + totalSize)
                    break;

                setValue (values, i, "Identifier", loops[i].identifier);
                setValue (values, i, "Type",       loops[i].type);
                setValue (values, i, "Start",      loops[i].start);
                setValue (values, i, "End",        loops[i].end);
                setValue (values, i, "Fraction",   loops[i].fraction);
                setValue (values, i, "PlayCount",  loops[i].playCount);
            }
        }

        template <typename NameType>
        static u32 getValue (const StringMap& values, NameType name, tukk def)
        {
            return ByteOrder::swapIfBigEndian ((u32) getValueWithDefault (values, name, def).getIntValue());
        }

        static u32 getValue (const StringMap& values, i32 prefix, tukk name, tukk def)
        {
            return getValue (values, "Loop" + Txt (prefix) + name, def);
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            MemoryBlock data;
            auto numLoops = jmin (64, getValueWithDefault (values, "NumSampleLoops", "0").getIntValue());

            data.setSize (roundUpSize (sizeof (SMPLChunk) + (size_t) (jmax (0, numLoops - 1)) * sizeof (SampleLoop)), true);

            auto s = static_cast<SMPLChunk*> (data.getData());

            s->manufacturer      = getValue (values, "Manufacturer", "0");
            s->product           = getValue (values, "Product", "0");
            s->samplePeriod      = getValue (values, "SamplePeriod", "0");
            s->midiUnityNote     = getValue (values, "MidiUnityNote", "60");
            s->midiPitchFraction = getValue (values, "MidiPitchFraction", "0");
            s->smpteFormat       = getValue (values, "SmpteFormat", "0");
            s->smpteOffset       = getValue (values, "SmpteOffset", "0");
            s->numSampleLoops    = ByteOrder::swapIfBigEndian ((u32) numLoops);
            s->samplerData       = getValue (values, "SamplerData", "0");

            for (i32 i = 0; i < numLoops; ++i)
            {
                auto& loop = s->loops[i];
                loop.identifier = getValue (values, i, "Identifier", "0");
                loop.type       = getValue (values, i, "Type", "0");
                loop.start      = getValue (values, i, "Start", "0");
                loop.end        = getValue (values, i, "End", "0");
                loop.fraction   = getValue (values, i, "Fraction", "0");
                loop.playCount  = getValue (values, i, "PlayCount", "0");
            }

            return data;
        }
    } DRX_PACKED;

    //==============================================================================
    struct InstChunk
    {
        i8 baseNote;
        i8 detune;
        i8 gain;
        i8 lowNote;
        i8 highNote;
        i8 lowVelocity;
        i8 highVelocity;

        static z0 setValue (StringMap& values, tukk name, i32 val)
        {
            values[name] = Txt (val);
        }

        z0 copyTo (StringMap& values) const
        {
            setValue (values, "MidiUnityNote",  baseNote);
            setValue (values, "Detune",         detune);
            setValue (values, "Gain",           gain);
            setValue (values, "LowNote",        lowNote);
            setValue (values, "HighNote",       highNote);
            setValue (values, "LowVelocity",    lowVelocity);
            setValue (values, "HighVelocity",   highVelocity);
        }

        static i8 getValue (const StringMap& values, tukk name, tukk def)
        {
            return (i8) getValueWithDefault (values, name, def).getIntValue();
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            MemoryBlock data;

            if (   values.find ("LowNote")  != values.cend()
                && values.find ("HighNote") != values.cend())
            {
                data.setSize (8, true);
                auto* inst = static_cast<InstChunk*> (data.getData());

                inst->baseNote      = getValue (values, "MidiUnityNote", "60");
                inst->detune        = getValue (values, "Detune", "0");
                inst->gain          = getValue (values, "Gain", "0");
                inst->lowNote       = getValue (values, "LowNote", "0");
                inst->highNote      = getValue (values, "HighNote", "127");
                inst->lowVelocity   = getValue (values, "LowVelocity", "1");
                inst->highVelocity  = getValue (values, "HighVelocity", "127");
            }

            return data;
        }
    } DRX_PACKED;

    //==============================================================================
    struct CueChunk
    {
        struct Cue
        {
            u32 identifier;
            u32 order;
            u32 chunkID;
            u32 chunkStart;
            u32 blockStart;
            u32 offset;
        } DRX_PACKED;

        u32 numCues;
        Cue cues[1];

        static z0 setValue (StringMap& values, i32 prefix, tukk name, u32 val)
        {
            values["Cue" + Txt (prefix) + name] = Txt (ByteOrder::swapIfBigEndian (val));
        }

        z0 copyTo (StringMap& values, i32k totalSize) const
        {
            values["NumCuePoints"] = Txt (ByteOrder::swapIfBigEndian (numCues));

            for (i32 i = 0; i < (i32) numCues; ++i)
            {
                if ((u8*) (cues + (i + 1)) > ((u8*) this) + totalSize)
                    break;

                setValue (values, i, "Identifier",  cues[i].identifier);
                setValue (values, i, "Order",       cues[i].order);
                setValue (values, i, "ChunkID",     cues[i].chunkID);
                setValue (values, i, "ChunkStart",  cues[i].chunkStart);
                setValue (values, i, "BlockStart",  cues[i].blockStart);
                setValue (values, i, "Offset",      cues[i].offset);
            }
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            MemoryBlock data;
            i32k numCues = getValueWithDefault (values, "NumCuePoints", "0").getIntValue();

            if (numCues > 0)
            {
                data.setSize (roundUpSize (sizeof (CueChunk) + (size_t) (numCues - 1) * sizeof (Cue)), true);

                auto c = static_cast<CueChunk*> (data.getData());

                c->numCues = ByteOrder::swapIfBigEndian ((u32) numCues);

                const Txt dataChunkID (chunkName ("data"));
                i32 nextOrder = 0;

               #if DRX_DEBUG
                Array<u32> identifiers;
               #endif

                for (i32 i = 0; i < numCues; ++i)
                {
                    auto prefix = "Cue" + Txt (i);
                    auto identifier = (u32) getValueWithDefault (values, prefix + "Identifier", "0").getIntValue();

                   #if DRX_DEBUG
                    jassert (! identifiers.contains (identifier));
                    identifiers.add (identifier);
                   #endif

                    auto order = getValueWithDefault (values, prefix + "Order", Txt (nextOrder)).getIntValue();
                    nextOrder = jmax (nextOrder, order) + 1;

                    auto& cue = c->cues[i];
                    cue.identifier   = ByteOrder::swapIfBigEndian ((u32) identifier);
                    cue.order        = ByteOrder::swapIfBigEndian ((u32) order);
                    cue.chunkID      = ByteOrder::swapIfBigEndian ((u32) getValueWithDefault (values, prefix + "ChunkID", dataChunkID).getIntValue());
                    cue.chunkStart   = ByteOrder::swapIfBigEndian ((u32) getValueWithDefault (values, prefix + "ChunkStart", "0").getIntValue());
                    cue.blockStart   = ByteOrder::swapIfBigEndian ((u32) getValueWithDefault (values, prefix + "BlockStart", "0").getIntValue());
                    cue.offset       = ByteOrder::swapIfBigEndian ((u32) getValueWithDefault (values, prefix + "Offset", "0").getIntValue());
                }
            }

            return data;
        }

    } DRX_PACKED;

    //==============================================================================
    namespace ListChunk
    {
        static i32 getValue (const StringMap& values, const Txt& name)
        {
            return getValueWithDefault (values, name, "0").getIntValue();
        }

        static i32 getValue (const StringMap& values, const Txt& prefix, tukk name)
        {
            return getValue (values, prefix + name);
        }

        static z0 appendLabelOrNoteChunk (const StringMap& values, const Txt& prefix,
                                            i32k chunkType, MemoryOutputStream& out)
        {
            auto label = getValueWithDefault (values, prefix + "Text", prefix);
            auto labelLength = (i32) label.getNumBytesAsUTF8() + 1;
            auto chunkLength = 4 + labelLength + (labelLength & 1);

            out.writeInt (chunkType);
            out.writeInt (chunkLength);
            out.writeInt (getValue (values, prefix, "Identifier"));
            out.write (label.toUTF8(), (size_t) labelLength);

            if ((out.getDataSize() & 1) != 0)
                out.writeByte (0);
        }

        static z0 appendExtraChunk (const StringMap& values, const Txt& prefix, MemoryOutputStream& out)
        {
            auto text = getValueWithDefault (values, prefix + "Text", prefix);

            auto textLength = (i32) text.getNumBytesAsUTF8() + 1; // include null terminator
            auto chunkLength = textLength + 20 + (textLength & 1);

            out.writeInt (chunkName ("ltxt"));
            out.writeInt (chunkLength);
            out.writeInt (getValue (values, prefix, "Identifier"));
            out.writeInt (getValue (values, prefix, "SampleLength"));
            out.writeInt (getValue (values, prefix, "Purpose"));
            out.writeShort ((short) getValue (values, prefix, "Country"));
            out.writeShort ((short) getValue (values, prefix, "Language"));
            out.writeShort ((short) getValue (values, prefix, "Dialect"));
            out.writeShort ((short) getValue (values, prefix, "CodePage"));
            out.write (text.toUTF8(), (size_t) textLength);

            if ((out.getDataSize() & 1) != 0)
                out.writeByte (0);
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            auto numCueLabels  = getValue (values, "NumCueLabels");
            auto numCueNotes   = getValue (values, "NumCueNotes");
            auto numCueRegions = getValue (values, "NumCueRegions");

            MemoryOutputStream out;

            if (numCueLabels + numCueNotes + numCueRegions > 0)
            {
                out.writeInt (chunkName ("adtl"));

                for (i32 i = 0; i < numCueLabels; ++i)
                    appendLabelOrNoteChunk (values, "CueLabel" + Txt (i), chunkName ("labl"), out);

                for (i32 i = 0; i < numCueNotes; ++i)
                    appendLabelOrNoteChunk (values, "CueNote" + Txt (i), chunkName ("note"), out);

                for (i32 i = 0; i < numCueRegions; ++i)
                    appendExtraChunk (values, "CueRegion" + Txt (i), out);
            }

            return out.getMemoryBlock();
        }
    }

    //==============================================================================
    /** Reads a RIFF List Info chunk from a stream positioned just after the size byte. */
    namespace ListInfoChunk
    {
        static tukk const types[] =
        {
            WavAudioFormat::riffInfoArchivalLocation,
            WavAudioFormat::riffInfoArtist,
            WavAudioFormat::riffInfoBaseURL,
            WavAudioFormat::riffInfoCinematographer,
            WavAudioFormat::riffInfoComment,
            WavAudioFormat::riffInfoComments,
            WavAudioFormat::riffInfoComment2,
            WavAudioFormat::riffInfoCommissioned,
            WavAudioFormat::riffInfoCopyright,
            WavAudioFormat::riffInfoCostumeDesigner,
            WavAudioFormat::riffInfoCountry,
            WavAudioFormat::riffInfoCropped,
            WavAudioFormat::riffInfoDateCreated,
            WavAudioFormat::riffInfoDateTimeOriginal,
            WavAudioFormat::riffInfoDefaultAudioStream,
            WavAudioFormat::riffInfoDimension,
            WavAudioFormat::riffInfoDirectory,
            WavAudioFormat::riffInfoDistributedBy,
            WavAudioFormat::riffInfoDotsPerInch,
            WavAudioFormat::riffInfoEditedBy,
            WavAudioFormat::riffInfoEighthLanguage,
            WavAudioFormat::riffInfoEncodedBy,
            WavAudioFormat::riffInfoEndTimecode,
            WavAudioFormat::riffInfoEngineer,
            WavAudioFormat::riffInfoFifthLanguage,
            WavAudioFormat::riffInfoFirstLanguage,
            WavAudioFormat::riffInfoFourthLanguage,
            WavAudioFormat::riffInfoGenre,
            WavAudioFormat::riffInfoKeywords,
            WavAudioFormat::riffInfoLanguage,
            WavAudioFormat::riffInfoLength,
            WavAudioFormat::riffInfoLightness,
            WavAudioFormat::riffInfoLocation,
            WavAudioFormat::riffInfoLogoIconURL,
            WavAudioFormat::riffInfoLogoURL,
            WavAudioFormat::riffInfoMedium,
            WavAudioFormat::riffInfoMoreInfoBannerImage,
            WavAudioFormat::riffInfoMoreInfoBannerURL,
            WavAudioFormat::riffInfoMoreInfoText,
            WavAudioFormat::riffInfoMoreInfoURL,
            WavAudioFormat::riffInfoMusicBy,
            WavAudioFormat::riffInfoNinthLanguage,
            WavAudioFormat::riffInfoNumberOfParts,
            WavAudioFormat::riffInfoOrganisation,
            WavAudioFormat::riffInfoPart,
            WavAudioFormat::riffInfoProducedBy,
            WavAudioFormat::riffInfoProductName,
            WavAudioFormat::riffInfoProductionDesigner,
            WavAudioFormat::riffInfoProductionStudio,
            WavAudioFormat::riffInfoRate,
            WavAudioFormat::riffInfoRated,
            WavAudioFormat::riffInfoRating,
            WavAudioFormat::riffInfoRippedBy,
            WavAudioFormat::riffInfoSecondaryGenre,
            WavAudioFormat::riffInfoSecondLanguage,
            WavAudioFormat::riffInfoSeventhLanguage,
            WavAudioFormat::riffInfoSharpness,
            WavAudioFormat::riffInfoSixthLanguage,
            WavAudioFormat::riffInfoSoftware,
            WavAudioFormat::riffInfoSoundSchemeTitle,
            WavAudioFormat::riffInfoSource,
            WavAudioFormat::riffInfoSourceFrom,
            WavAudioFormat::riffInfoStarring_ISTR,
            WavAudioFormat::riffInfoStarring_STAR,
            WavAudioFormat::riffInfoStartTimecode,
            WavAudioFormat::riffInfoStatistics,
            WavAudioFormat::riffInfoSubject,
            WavAudioFormat::riffInfoTapeName,
            WavAudioFormat::riffInfoTechnician,
            WavAudioFormat::riffInfoThirdLanguage,
            WavAudioFormat::riffInfoTimeCode,
            WavAudioFormat::riffInfoTitle,
            WavAudioFormat::riffInfoTrackNo,
            WavAudioFormat::riffInfoTrackNumber,
            WavAudioFormat::riffInfoURL,
            WavAudioFormat::riffInfoVegasVersionMajor,
            WavAudioFormat::riffInfoVegasVersionMinor,
            WavAudioFormat::riffInfoVersion,
            WavAudioFormat::riffInfoWatermarkURL,
            WavAudioFormat::riffInfoWrittenBy,
            WavAudioFormat::riffInfoYear
        };

        static b8 isMatchingTypeIgnoringCase (i32k value, tukk const name) noexcept
        {
            for (i32 i = 0; i < 4; ++i)
                if ((t32) name[i] != CharacterFunctions::toUpperCase ((t32) ((value >> (i * 8)) & 0xff)))
                    return false;

            return true;
        }

        static z0 addToMetadata (StringMap& values, InputStream& input, z64 chunkEnd)
        {
            while (input.getPosition() < chunkEnd)
            {
                auto infoType = input.readInt();
                auto infoLength = chunkEnd - input.getPosition();

                if (infoLength > 0)
                {
                    infoLength = jmin (infoLength, (z64) input.readInt());

                    if (infoLength <= 0)
                        return;

                    for (auto& type : types)
                    {
                        if (isMatchingTypeIgnoringCase (infoType, type))
                        {
                            MemoryBlock mb;
                            input.readIntoMemoryBlock (mb, (ssize_t) infoLength);
                            values[type] = Txt::createStringFromData ((tukk) mb.getData(),
                                                                         (i32) mb.getSize());
                            break;
                        }
                    }
                }
            }
        }

        static b8 writeValue (const StringMap& values, MemoryOutputStream& out, tukk paramName)
        {
            auto value = getValueWithDefault (values, paramName, {});

            if (value.isEmpty())
                return false;

            auto valueLength = (i32) value.getNumBytesAsUTF8() + 1;
            auto chunkLength = valueLength + (valueLength & 1);

            out.writeInt (chunkName (paramName));
            out.writeInt (chunkLength);
            out.write (value.toUTF8(), (size_t) valueLength);

            if ((out.getDataSize() & 1) != 0)
                out.writeByte (0);

            return true;
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            MemoryOutputStream out;
            out.writeInt (chunkName ("INFO"));
            b8 anyParamsDefined = false;

            for (auto& type : types)
                if (writeValue (values, out, type))
                    anyParamsDefined = true;

            return anyParamsDefined ? out.getMemoryBlock() : MemoryBlock();
        }
    }

    //==============================================================================
    struct AcidChunk
    {
        /** Reads an acid RIFF chunk from a stream positioned just after the size byte. */
        AcidChunk (InputStream& input, size_t length)
        {
            zerostruct (*this);
            input.read (this, (i32) jmin (sizeof (*this), length));
        }

        AcidChunk (const StringMap& values)
        {
            zerostruct (*this);

            flags = getFlagIfPresent (values, WavAudioFormat::acidOneShot,   0x01)
                  | getFlagIfPresent (values, WavAudioFormat::acidRootSet,   0x02)
                  | getFlagIfPresent (values, WavAudioFormat::acidStretch,   0x04)
                  | getFlagIfPresent (values, WavAudioFormat::acidDiskBased, 0x08)
                  | getFlagIfPresent (values, WavAudioFormat::acidizerFlag,  0x10);

            if (getValueWithDefault (values, WavAudioFormat::acidRootSet).getIntValue() != 0)
                rootNote = ByteOrder::swapIfBigEndian ((u16) getValueWithDefault (values, WavAudioFormat::acidRootNote).getIntValue());

            numBeats          = ByteOrder::swapIfBigEndian ((u32) getValueWithDefault (values, WavAudioFormat::acidBeats).getIntValue());
            meterDenominator  = ByteOrder::swapIfBigEndian ((u16) getValueWithDefault (values, WavAudioFormat::acidDenominator).getIntValue());
            meterNumerator    = ByteOrder::swapIfBigEndian ((u16) getValueWithDefault (values, WavAudioFormat::acidNumerator).getIntValue());

            const auto iter = values.find (WavAudioFormat::acidTempo);

            if (iter != values.cend())
                tempo = swapFloatByteOrder (iter->second.getFloatValue());
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            return AcidChunk (values).toMemoryBlock();
        }

        MemoryBlock toMemoryBlock() const
        {
            return (flags != 0 || rootNote != 0 || numBeats != 0 || meterDenominator != 0 || meterNumerator != 0)
                      ? MemoryBlock (this, sizeof (*this)) : MemoryBlock();
        }

        z0 addToMetadata (StringMap& values) const
        {
            setBoolFlag (values, WavAudioFormat::acidOneShot,   0x01);
            setBoolFlag (values, WavAudioFormat::acidRootSet,   0x02);
            setBoolFlag (values, WavAudioFormat::acidStretch,   0x04);
            setBoolFlag (values, WavAudioFormat::acidDiskBased, 0x08);
            setBoolFlag (values, WavAudioFormat::acidizerFlag,  0x10);

            if (flags & 0x02) // root note set
                values[WavAudioFormat::acidRootNote] = Txt (ByteOrder::swapIfBigEndian (rootNote));

            values[WavAudioFormat::acidBeats]       = Txt (ByteOrder::swapIfBigEndian (numBeats));
            values[WavAudioFormat::acidDenominator] = Txt (ByteOrder::swapIfBigEndian (meterDenominator));
            values[WavAudioFormat::acidNumerator]   = Txt (ByteOrder::swapIfBigEndian (meterNumerator));
            values[WavAudioFormat::acidTempo]       = Txt (swapFloatByteOrder (tempo));
        }

        z0 setBoolFlag (StringMap& values, tukk name, u32 mask) const
        {
            values[name] = (flags & ByteOrder::swapIfBigEndian (mask)) ? "1" : "0";
        }

        static u32 getFlagIfPresent (const StringMap& values, tukk name, u32 flag)
        {
            return getValueWithDefault (values, name).getIntValue() != 0 ? ByteOrder::swapIfBigEndian (flag) : 0;
        }

        static f32 swapFloatByteOrder (const f32 x) noexcept
        {
           #ifdef DRX_BIG_ENDIAN
            union { u32 asInt; f32 asFloat; } n;
            n.asFloat = x;
            n.asInt = ByteOrder::swap (n.asInt);
            return n.asFloat;
           #else
            return x;
           #endif
        }

        u32 flags;
        u16 rootNote;
        u16 reserved1;
        f32 reserved2;
        u32 numBeats;
        u16 meterDenominator;
        u16 meterNumerator;
        f32 tempo;

    } DRX_PACKED;

    //==============================================================================
    struct TracktionChunk
    {
        static MemoryBlock createFrom (const StringMap& values)
        {
            MemoryOutputStream out;
            auto s = getValueWithDefault (values, WavAudioFormat::tracktionLoopInfo);

            if (s.isNotEmpty())
            {
                out.writeString (s);

                if ((out.getDataSize() & 1) != 0)
                    out.writeByte (0);
            }

            return out.getMemoryBlock();
        }
    };

    //==============================================================================
    namespace IXMLChunk
    {
        static const std::unordered_set<Txt> aswgMetadataKeys
        {
            WavAudioFormat::aswgContentType,
            WavAudioFormat::aswgProject,
            WavAudioFormat::aswgOriginator,
            WavAudioFormat::aswgOriginatorStudio,
            WavAudioFormat::aswgNotes,
            WavAudioFormat::aswgSession,
            WavAudioFormat::aswgState,
            WavAudioFormat::aswgEditor,
            WavAudioFormat::aswgMixer,
            WavAudioFormat::aswgFxChainName,
            WavAudioFormat::aswgChannelConfig,
            WavAudioFormat::aswgAmbisonicFormat,
            WavAudioFormat::aswgAmbisonicChnOrder,
            WavAudioFormat::aswgAmbisonicNorm,
            WavAudioFormat::aswgMicType,
            WavAudioFormat::aswgMicConfig,
            WavAudioFormat::aswgMicDistance,
            WavAudioFormat::aswgRecordingLoc,
            WavAudioFormat::aswgIsDesigned,
            WavAudioFormat::aswgRecEngineer,
            WavAudioFormat::aswgRecStudio,
            WavAudioFormat::aswgImpulseLocation,
            WavAudioFormat::aswgCategory,
            WavAudioFormat::aswgSubCategory,
            WavAudioFormat::aswgCatId,
            WavAudioFormat::aswgUserCategory,
            WavAudioFormat::aswgUserData,
            WavAudioFormat::aswgVendorCategory,
            WavAudioFormat::aswgFxName,
            WavAudioFormat::aswgLibrary,
            WavAudioFormat::aswgCreatorId,
            WavAudioFormat::aswgSourceId,
            WavAudioFormat::aswgRmsPower,
            WavAudioFormat::aswgLoudness,
            WavAudioFormat::aswgLoudnessRange,
            WavAudioFormat::aswgMaxPeak,
            WavAudioFormat::aswgSpecDensity,
            WavAudioFormat::aswgZeroCrossRate,
            WavAudioFormat::aswgPapr,
            WavAudioFormat::aswgText,
            WavAudioFormat::aswgEfforts,
            WavAudioFormat::aswgEffortType,
            WavAudioFormat::aswgProjection,
            WavAudioFormat::aswgLanguage,
            WavAudioFormat::aswgTimingRestriction,
            WavAudioFormat::aswgCharacterName,
            WavAudioFormat::aswgCharacterGender,
            WavAudioFormat::aswgCharacterAge,
            WavAudioFormat::aswgCharacterRole,
            WavAudioFormat::aswgActorName,
            WavAudioFormat::aswgActorGender,
            WavAudioFormat::aswgDirector,
            WavAudioFormat::aswgDirection,
            WavAudioFormat::aswgFxUsed,
            WavAudioFormat::aswgUsageRights,
            WavAudioFormat::aswgIsUnion,
            WavAudioFormat::aswgAccent,
            WavAudioFormat::aswgEmotion,
            WavAudioFormat::aswgComposor,
            WavAudioFormat::aswgArtist,
            WavAudioFormat::aswgSongTitle,
            WavAudioFormat::aswgGenre,
            WavAudioFormat::aswgSubGenre,
            WavAudioFormat::aswgProducer,
            WavAudioFormat::aswgMusicSup,
            WavAudioFormat::aswgInstrument,
            WavAudioFormat::aswgMusicPublisher,
            WavAudioFormat::aswgRightsOwner,
            WavAudioFormat::aswgIsSource,
            WavAudioFormat::aswgIsLoop,
            WavAudioFormat::aswgIntensity,
            WavAudioFormat::aswgIsFinal,
            WavAudioFormat::aswgOrderRef,
            WavAudioFormat::aswgIsOst,
            WavAudioFormat::aswgIsCinematic,
            WavAudioFormat::aswgIsLicensed,
            WavAudioFormat::aswgIsDiegetic,
            WavAudioFormat::aswgMusicVersion,
            WavAudioFormat::aswgIsrcId,
            WavAudioFormat::aswgTempo,
            WavAudioFormat::aswgTimeSig,
            WavAudioFormat::aswgInKey,
            WavAudioFormat::aswgBillingCode
        };

        static z0 addToMetadata (StringMap& destValues, const Txt& source)
        {
            if (auto xml = parseXML (source))
            {
                if (xml->hasTagName ("BWFXML"))
                {
                    if (const auto* entry = xml->getChildByName (WavAudioFormat::aswgVersion))
                        destValues[WavAudioFormat::aswgVersion] = entry->getAllSubText();

                    if (const auto* aswgElement = xml->getChildByName ("ASWG"))
                    {
                        for (const auto* entry : aswgElement->getChildIterator())
                        {
                            const auto& tag = entry->getTagName();

                            if (aswgMetadataKeys.find (tag) != aswgMetadataKeys.end())
                                destValues[tag] = entry->getAllSubText();
                        }
                    }
                }
            }
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            auto createTextElement = [] (const StringRef& key, const StringRef& value)
            {
                auto* elem = new XmlElement (key);
                elem->addTextElement (value);
                return elem;
            };

            std::unique_ptr<XmlElement> aswgElement;

            for (const auto& pair : values)
            {
                if (aswgMetadataKeys.find (pair.first) != aswgMetadataKeys.end())
                {
                    if (aswgElement == nullptr)
                        aswgElement = std::make_unique<XmlElement> ("ASWG");

                    aswgElement->addChildElement (createTextElement (pair.first, pair.second));
                }
            }

            MemoryOutputStream outputStream;

            if (aswgElement != nullptr)
            {
                XmlElement xml ("BWFXML");
                auto aswgVersion = getValueWithDefault (values, WavAudioFormat::aswgVersion, "3.01");
                xml.addChildElement (createTextElement (WavAudioFormat::aswgVersion, aswgVersion));
                xml.addChildElement (aswgElement.release());
                xml.writeTo (outputStream);
                outputStream.writeRepeatedByte (0, outputStream.getDataSize());
            }

            return outputStream.getMemoryBlock();
        }
    }

    //==============================================================================
    namespace AXMLChunk
    {
        static z0 addToMetadata (StringMap& destValues, const Txt& source)
        {
            if (auto xml = parseXML (source))
            {
                if (xml->hasTagName ("ebucore:ebuCoreMain"))
                {
                    if (auto xml2 = xml->getChildByName ("ebucore:coreMetadata"))
                    {
                        if (auto xml3 = xml2->getChildByName ("ebucore:identifier"))
                        {
                            if (auto xml4 = xml3->getChildByName ("dc:identifier"))
                            {
                                auto ISRCCode = xml4->getAllSubText().fromFirstOccurrenceOf ("ISRC:", false, true);

                                if (ISRCCode.isNotEmpty())
                                {
                                    // We set ISRC here for backwards compatibility.
                                    // If the INFO 'source' field is set in the info chunk, then the
                                    // value for this key will be overwritten later.
                                    destValues[WavAudioFormat::riffInfoSource] = destValues[WavAudioFormat::internationalStandardRecordingCode] = ISRCCode;
                                }
                            }
                        }
                    }
                }
            }
        }

        static MemoryBlock createFrom (const StringMap& values)
        {
            // Use the new ISRC key if it is present, but fall back to the
            // INFO 'source' value for backwards compatibility.
            auto ISRC = getValueWithDefault (values,
                                             WavAudioFormat::internationalStandardRecordingCode,
                                             getValueWithDefault (values, WavAudioFormat::riffInfoSource));

            MemoryOutputStream xml;

            if (ISRC.isNotEmpty())
            {
                // If you are trying to set the ISRC, make sure that you are using
                // WavAudioFormat::internationalStandardRecordingCode as the metadata key,
                // and that the value is 12 characters i64. If you are trying to set the
                // 'source' field in the INFO chunk, set the
                // WavAudioFormat::internationalStandardRecordingCode metadata field to the
                // empty string to silence this assertion.
                jassert (ISRC.length() == 12);

                xml << "<ebucore:ebuCoreMain xmlns:dc=\" http://purl.org/dc/elements/1.1/\" "
                                            "xmlns:ebucore=\"urn:ebu:metadata-schema:ebuCore_2012\">"
                         "<ebucore:coreMetadata>"
                           "<ebucore:identifier typeLabel=\"GUID\" "
                                               "typeDefinition=\"Globally Unique Identifier\" "
                                               "formatLabel=\"ISRC\" "
                                               "formatDefinition=\"International Standard Recording Code\" "
                                               "formatLink=\"http://www.ebu.ch/metadata/cs/ebu_IdentifierTypeCodeCS.xml#3.7\">"
                             "<dc:identifier>ISRC:" << ISRC << "</dc:identifier>"
                           "</ebucore:identifier>"
                         "</ebucore:coreMetadata>"
                       "</ebucore:ebuCoreMain>";

                xml.writeRepeatedByte (0, xml.getDataSize());  // ensures even size, null termination and room for future growing
            }

            return xml.getMemoryBlock();
        }
    }

    //==============================================================================
    struct ExtensibleWavSubFormat
    {
        u32 data1;
        u16 data2;
        u16 data3;
        u8  data4[8];

        b8 operator== (const ExtensibleWavSubFormat& other) const noexcept   { return memcmp (this, &other, sizeof (*this)) == 0; }
        b8 operator!= (const ExtensibleWavSubFormat& other) const noexcept   { return ! operator== (other); }

    } DRX_PACKED;

    static const ExtensibleWavSubFormat pcmFormat       = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
    static const ExtensibleWavSubFormat IEEEFloatFormat = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
    static const ExtensibleWavSubFormat ambisonicFormat = { 0x00000001, 0x0721, 0x11d3, { 0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00 } };

    struct DataSize64Chunk   // chunk ID = 'ds64' if data size > 0xffffffff, 'JUNK' otherwise
    {
        u32 riffSizeLow;     // low 4 byte size of RF64 block
        u32 riffSizeHigh;    // high 4 byte size of RF64 block
        u32 dataSizeLow;     // low 4 byte size of data chunk
        u32 dataSizeHigh;    // high 4 byte size of data chunk
        u32 sampleCountLow;  // low 4 byte sample count of fact chunk
        u32 sampleCountHigh; // high 4 byte sample count of fact chunk
        u32 tableLength;     // number of valid entries in array 'table'
    } DRX_PACKED;

    #if DRX_MSVC
     #pragma pack (pop)
    #endif
}

//==============================================================================
class WavAudioFormatReader final : public AudioFormatReader
{
public:
    WavAudioFormatReader (InputStream* in)  : AudioFormatReader (in, wavFormatName)
    {
        using namespace WavFileHelpers;
        zu64 len = 0, end = 0;
        i32 cueNoteIndex = 0;
        i32 cueLabelIndex = 0;
        i32 cueRegionIndex = 0;

        StringMap dict;

        auto streamStartPos = input->getPosition();
        auto firstChunkType = input->readInt();

        if (firstChunkType == chunkName ("RF64"))
        {
            input->skipNextBytes (4); // size is -1 for RF64
            isRF64 = true;
        }
        else if (firstChunkType == chunkName ("RIFF"))
        {
            len = (zu64) (u32) input->readInt();
            end = len + (zu64) input->getPosition();
        }
        else
        {
            return;
        }

        auto startOfRIFFChunk = input->getPosition();

        if (input->readInt() == chunkName ("WAVE"))
        {
            if (isRF64 && input->readInt() == chunkName ("ds64"))
            {
                auto length = (u32) input->readInt();

                if (length < 28)
                    return;

                auto chunkEnd = input->getPosition() + length + (length & 1);
                len = (zu64) input->readInt64();
                end = len + (zu64) startOfRIFFChunk;
                dataLength = input->readInt64();
                input->setPosition (chunkEnd);
            }

            while ((zu64) input->getPosition() < end && ! input->isExhausted())
            {
                auto chunkType = input->readInt();
                auto length = (u32) input->readInt();
                auto chunkEnd = input->getPosition() + length + (length & 1);

                if (chunkType == chunkName ("fmt "))
                {
                    // read the format chunk
                    auto format = (u16) input->readShort();
                    numChannels = (u32) input->readShort();
                    sampleRate = input->readInt();
                    auto bytesPerSec = input->readInt();
                    input->skipNextBytes (2);
                    bitsPerSample = (u32) (i32) input->readShort();

                    if (bitsPerSample > 64 && (i32) sampleRate != 0)
                    {
                        bytesPerFrame = bytesPerSec / (i32) sampleRate;

                        if (numChannels != 0)
                            bitsPerSample = 8 * (u32) bytesPerFrame / numChannels;
                    }
                    else
                    {
                        bytesPerFrame = (i32) (numChannels * bitsPerSample / 8);
                    }

                    if (format == 3)
                    {
                        usesFloatingPointData = true;
                    }
                    else if (format == 0xfffe) // WAVE_FORMAT_EXTENSIBLE
                    {
                        if (length < 40) // too short
                        {
                            bytesPerFrame = 0;
                        }
                        else
                        {
                            input->skipNextBytes (4); // skip over size and bitsPerSample
                            auto channelMask = input->readInt();
                            dict["ChannelMask"] = Txt (channelMask);
                            channelLayout = getChannelLayoutFromMask (channelMask, numChannels);

                            ExtensibleWavSubFormat subFormat;
                            subFormat.data1 = (u32) input->readInt();
                            subFormat.data2 = (u16) input->readShort();
                            subFormat.data3 = (u16) input->readShort();
                            input->read (subFormat.data4, sizeof (subFormat.data4));

                            if (subFormat == IEEEFloatFormat)
                                usesFloatingPointData = true;
                            else if (subFormat != pcmFormat && subFormat != ambisonicFormat)
                                bytesPerFrame = 0;
                        }
                    }
                    else if (format == 0x674f  // WAVE_FORMAT_OGG_VORBIS_MODE_1
                          || format == 0x6750  // WAVE_FORMAT_OGG_VORBIS_MODE_2
                          || format == 0x6751  // WAVE_FORMAT_OGG_VORBIS_MODE_3
                          || format == 0x676f  // WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS
                          || format == 0x6770  // WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS
                          || format == 0x6771) // WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS
                    {
                        isSubformatOggVorbis = true;
                        sampleRate = 0; // to mark the wav reader as failed
                        input->setPosition (streamStartPos);
                        return;
                    }
                    else if (format != 1)
                    {
                        bytesPerFrame = 0;
                    }
                }
                else if (chunkType == chunkName ("data"))
                {
                    if (isRF64)
                    {
                        if (dataLength > 0)
                            chunkEnd = input->getPosition() + dataLength + (dataLength & 1);
                    }
                    else
                    {
                        dataLength = length;
                    }

                    dataChunkStart = input->getPosition();
                    lengthInSamples = (bytesPerFrame > 0) ? (dataLength / bytesPerFrame) : 0;
                }
                else if (chunkType == chunkName ("bext"))
                {
                    bwavChunkStart = input->getPosition();
                    bwavSize = length;

                    HeapBlock<BWAVChunk> bwav;
                    bwav.calloc (jmax ((size_t) length + 1, sizeof (BWAVChunk)), 1);
                    input->read (bwav, (i32) length);
                    bwav->copyTo (dict, (i32) length);
                }
                else if (chunkType == chunkName ("smpl"))
                {
                    HeapBlock<SMPLChunk> smpl;
                    smpl.calloc (jmax ((size_t) length + 1, sizeof (SMPLChunk)), 1);
                    input->read (smpl, (i32) length);
                    smpl->copyTo (dict, (i32) length);
                }
                else if (chunkType == chunkName ("inst") || chunkType == chunkName ("INST")) // need to check which...
                {
                    HeapBlock<InstChunk> inst;
                    inst.calloc (jmax ((size_t) length + 1, sizeof (InstChunk)), 1);
                    input->read (inst, (i32) length);
                    inst->copyTo (dict);
                }
                else if (chunkType == chunkName ("cue "))
                {
                    HeapBlock<CueChunk> cue;
                    cue.calloc (jmax ((size_t) length + 1, sizeof (CueChunk)), 1);
                    input->read (cue, (i32) length);
                    cue->copyTo (dict, (i32) length);
                }
                else if (chunkType == chunkName ("axml"))
                {
                    MemoryBlock axml;
                    input->readIntoMemoryBlock (axml, (ssize_t) length);
                    AXMLChunk::addToMetadata (dict, axml.toString());
                }
                else if (chunkType == chunkName ("iXML"))
                {
                    MemoryBlock ixml;
                    input->readIntoMemoryBlock (ixml, (ssize_t) length);
                    IXMLChunk::addToMetadata (dict, ixml.toString());
                }
                else if (chunkType == chunkName ("LIST"))
                {
                    auto subChunkType = input->readInt();

                    if (subChunkType == chunkName ("info") || subChunkType == chunkName ("INFO"))
                    {
                        ListInfoChunk::addToMetadata (dict, *input, chunkEnd);
                    }
                    else if (subChunkType == chunkName ("adtl"))
                    {
                        while (input->getPosition() < chunkEnd)
                        {
                            auto adtlChunkType = input->readInt();
                            auto adtlLength = (u32) input->readInt();
                            auto adtlChunkEnd = input->getPosition() + (adtlLength + (adtlLength & 1));

                            if (adtlChunkType == chunkName ("labl") || adtlChunkType == chunkName ("note"))
                            {
                                Txt prefix;

                                if (adtlChunkType == chunkName ("labl"))
                                    prefix << "CueLabel" << cueLabelIndex++;
                                else if (adtlChunkType == chunkName ("note"))
                                    prefix << "CueNote" << cueNoteIndex++;

                                auto identifier = (u32) input->readInt();
                                auto stringLength = (i32) adtlLength - 4;

                                MemoryBlock textBlock;
                                input->readIntoMemoryBlock (textBlock, stringLength);

                                dict[prefix + "Identifier"] = Txt (identifier);
                                dict[prefix + "Text"] = textBlock.toString();
                            }
                            else if (adtlChunkType == chunkName ("ltxt"))
                            {
                                auto prefix = "CueRegion" + Txt (cueRegionIndex++);
                                auto identifier     = (u32) input->readInt();
                                auto sampleLength   = (u32) input->readInt();
                                auto purpose        = (u32) input->readInt();
                                auto country        = (u16) input->readShort();
                                auto language       = (u16) input->readShort();
                                auto dialect        = (u16) input->readShort();
                                auto codePage       = (u16) input->readShort();
                                auto stringLength   = adtlLength - 20;

                                MemoryBlock textBlock;
                                input->readIntoMemoryBlock (textBlock, (i32) stringLength);

                                dict[prefix + "Identifier"]   = Txt (identifier);
                                dict[prefix + "SampleLength"] = Txt (sampleLength);
                                dict[prefix + "Purpose"]      = Txt (purpose);
                                dict[prefix + "Country"]      = Txt (country);
                                dict[prefix + "Language"]     = Txt (language);
                                dict[prefix + "Dialect"]      = Txt (dialect);
                                dict[prefix + "CodePage"]     = Txt (codePage);
                                dict[prefix + "Text"]         = textBlock.toString();
                            }

                            input->setPosition (adtlChunkEnd);
                        }
                    }
                }
                else if (chunkType == chunkName ("acid"))
                {
                    AcidChunk (*input, length).addToMetadata (dict);
                }
                else if (chunkType == chunkName ("Trkn"))
                {
                    MemoryBlock tracktion;
                    input->readIntoMemoryBlock (tracktion, (ssize_t) length);
                    dict[WavAudioFormat::tracktionLoopInfo] = tracktion.toString();
                }
                else if (chunkEnd <= input->getPosition())
                {
                    break;
                }

                input->setPosition (chunkEnd);
            }
        }

        if (cueLabelIndex > 0)          dict["NumCueLabels"]    = Txt (cueLabelIndex);
        if (cueNoteIndex > 0)           dict["NumCueNotes"]     = Txt (cueNoteIndex);
        if (cueRegionIndex > 0)         dict["NumCueRegions"]   = Txt (cueRegionIndex);
        if (dict.size() > 0)            dict["MetaDataSource"]  = "WAV";

        metadataValues.addUnorderedMap (dict);
    }

    //==============================================================================
    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (numSamples <= 0)
            return true;

        input->setPosition (dataChunkStart + startSampleInFile * bytesPerFrame);

        while (numSamples > 0)
        {
            i32k tempBufSize = 480 * 3 * 4; // (keep this a multiple of 3)
            t8 tempBuffer[tempBufSize];

            auto numThisTime = jmin (tempBufSize / bytesPerFrame, numSamples);
            auto bytesRead = input->read (tempBuffer, numThisTime * bytesPerFrame);

            if (bytesRead < numThisTime * bytesPerFrame)
            {
                jassert (bytesRead >= 0);
                zeromem (tempBuffer + bytesRead, (size_t) (numThisTime * bytesPerFrame - bytesRead));
            }

            copySampleData (bitsPerSample, usesFloatingPointData,
                            destSamples, startOffsetInDestBuffer, numDestChannels,
                            tempBuffer, (i32) numChannels, numThisTime);

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
        }

        return true;
    }

    static z0 copySampleData (u32 numBitsPerSample, const b8 floatingPointData,
                                i32* const* destSamples, i32 startOffsetInDestBuffer, i32 numDestChannels,
                                ukk sourceData, i32 numberOfChannels, i32 numSamples) noexcept
    {
        switch (numBitsPerSample)
        {
            case 8:     ReadHelper<AudioData::Int32, AudioData::UInt8, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numberOfChannels, numSamples); break;
            case 16:    ReadHelper<AudioData::Int32, AudioData::Int16, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numberOfChannels, numSamples); break;
            case 24:    ReadHelper<AudioData::Int32, AudioData::Int24, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numberOfChannels, numSamples); break;
            case 32:    if (floatingPointData) ReadHelper<AudioData::Float32, AudioData::Float32, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numberOfChannels, numSamples);
                        else                   ReadHelper<AudioData::Int32,   AudioData::Int32,   AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numberOfChannels, numSamples);
                        break;
            default:    jassertfalse; break;
        }
    }

    //==============================================================================
    AudioChannelSet getChannelLayout() override
    {
        if (channelLayout.size() == static_cast<i32> (numChannels))
            return channelLayout;

        return WavFileHelpers::canonicalWavChannelSet (static_cast<i32> (numChannels));
    }

    static AudioChannelSet getChannelLayoutFromMask (i32 dwChannelMask, size_t totalNumChannels)
    {
        AudioChannelSet wavFileChannelLayout;

        // AudioChannelSet and wav's dwChannelMask are compatible
        BigInteger channelBits (dwChannelMask);

        for (auto bit = channelBits.findNextSetBit (0); bit >= 0; bit = channelBits.findNextSetBit (bit + 1))
            wavFileChannelLayout.addChannel (static_cast<AudioChannelSet::ChannelType> (bit + 1));

        // channel layout and number of channels do not match
        if (wavFileChannelLayout.size() != static_cast<i32> (totalNumChannels))
        {
            // for backward compatibility with old wav files, assume 1 or 2
            // channel wav files are mono/stereo respectively
            if (totalNumChannels <= 2 && dwChannelMask == 0)
                wavFileChannelLayout = AudioChannelSet::canonicalChannelSet (static_cast<i32> (totalNumChannels));
            else
            {
                auto discreteSpeaker = static_cast<i32> (AudioChannelSet::discreteChannel0);

                while (wavFileChannelLayout.size() < static_cast<i32> (totalNumChannels))
                    wavFileChannelLayout.addChannel (static_cast<AudioChannelSet::ChannelType> (discreteSpeaker++));
            }
        }

        return wavFileChannelLayout;
    }

    z64 bwavChunkStart = 0, bwavSize = 0;
    z64 dataChunkStart = 0, dataLength = 0;
    i32 bytesPerFrame = 0;
    b8 isRF64 = false;
    b8 isSubformatOggVorbis = false;

    AudioChannelSet channelLayout;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormatReader)
};

//==============================================================================
class WavAudioFormatWriter final : public AudioFormatWriter
{
public:
    WavAudioFormatWriter (OutputStream* const out, const f64 rate,
                          const AudioChannelSet& channelLayoutToUse, u32k bits,
                          const StringPairArray& metadataValues)
        : AudioFormatWriter (out, wavFormatName, rate, channelLayoutToUse, bits)
    {
        using namespace WavFileHelpers;

        if (metadataValues.size() > 0)
        {
            // The meta data should have been sanitised for the WAV format.
            // If it was originally sourced from an AIFF file the MetaDataSource
            // key should be removed (or set to "WAV") once this has been done
            jassert (metadataValues.getValue ("MetaDataSource", "None") != "AIFF");

            const auto map = toMap (metadataValues);

            bwavChunk     = BWAVChunk::createFrom (map);
            ixmlChunk     = IXMLChunk::createFrom (map);
            axmlChunk     = AXMLChunk::createFrom (map);
            smplChunk     = SMPLChunk::createFrom (map);
            instChunk     = InstChunk::createFrom (map);
            cueChunk      = CueChunk ::createFrom (map);
            listChunk     = ListChunk::createFrom (map);
            listInfoChunk = ListInfoChunk::createFrom (map);
            acidChunk     = AcidChunk::createFrom (map);
            trckChunk     = TracktionChunk::createFrom (map);
        }

        headerPosition = out->getPosition();
        writeHeader();
    }

    ~WavAudioFormatWriter() override
    {
        writeHeader();
    }

    //==============================================================================
    b8 write (i32k** data, i32 numSamples) override
    {
        jassert (numSamples >= 0);
        jassert (data != nullptr && *data != nullptr); // the input must contain at least one channel!

        if (writeFailed)
            return false;

        auto bytes = numChannels * (size_t) numSamples * bitsPerSample / 8;
        tempBlock.ensureSize (bytes, false);

        switch (bitsPerSample)
        {
            case 8:     WriteHelper<AudioData::UInt8, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (i32) numChannels, data, numSamples); break;
            case 16:    WriteHelper<AudioData::Int16, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (i32) numChannels, data, numSamples); break;
            case 24:    WriteHelper<AudioData::Int24, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (i32) numChannels, data, numSamples); break;
            case 32:    WriteHelper<AudioData::Int32, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (i32) numChannels, data, numSamples); break;
            default:    jassertfalse; break;
        }

        if (! output->write (tempBlock.getData(), bytes))
        {
            // failed to write to disk, so let's try writing the header.
            // If it's just run out of disk space, then if it does manage
            // to write the header, we'll still have a usable file..
            writeHeader();
            writeFailed = true;
            return false;
        }

        bytesWritten += bytes;
        lengthInSamples += (zu64) numSamples;
        return true;
    }

    b8 flush() override
    {
        auto lastWritePos = output->getPosition();
        writeHeader();

        if (output->setPosition (lastWritePos))
            return true;

        // if this fails, you've given it an output stream that can't seek! It needs
        // to be able to seek back to write the header
        jassertfalse;
        return false;
    }

private:
    MemoryBlock tempBlock, bwavChunk, ixmlChunk, axmlChunk, smplChunk, instChunk, cueChunk, listChunk, listInfoChunk, acidChunk, trckChunk;
    zu64 lengthInSamples = 0, bytesWritten = 0;
    z64 headerPosition = 0;
    b8 writeFailed = false;

    z0 writeHeader()
    {
        if ((bytesWritten & 1) != 0) // pad to an even length
            output->writeByte (0);

        using namespace WavFileHelpers;

        if (headerPosition != output->getPosition() && ! output->setPosition (headerPosition))
        {
            // if this fails, you've given it an output stream that can't seek! It needs to be
            // able to seek back to go back and write the header after the data has been written.
            jassertfalse;
            return;
        }

        const size_t bytesPerFrame = numChannels * bitsPerSample / 8;
        zu64 audioDataSize = bytesPerFrame * lengthInSamples;
        auto channelMask = getChannelMaskFromChannelLayout (channelLayout);

        const b8 isRF64 = (bytesWritten >= 0x100000000LL);
        const b8 isWaveFmtEx = isRF64 || (channelMask != 0);

        z64 riffChunkSize = (z64) (4 /* 'RIFF' */ + 8 + 40 /* WAVEFORMATEX */
                                       + 8 + audioDataSize + (audioDataSize & 1)
                                       + chunkSize (bwavChunk)
                                       + chunkSize (ixmlChunk)
                                       + chunkSize (axmlChunk)
                                       + chunkSize (smplChunk)
                                       + chunkSize (instChunk)
                                       + chunkSize (cueChunk)
                                       + chunkSize (listChunk)
                                       + chunkSize (listInfoChunk)
                                       + chunkSize (acidChunk)
                                       + chunkSize (trckChunk)
                                       + (8 + 28)); // (ds64 chunk)

        riffChunkSize += (riffChunkSize & 1);

        if (isRF64)
            writeChunkHeader (chunkName ("RF64"), -1);
        else
            writeChunkHeader (chunkName ("RIFF"), (i32) riffChunkSize);

        output->writeInt (chunkName ("WAVE"));

        if (! isRF64)
        {
           #if ! DRX_WAV_DO_NOT_PAD_HEADER_SIZE
            /* NB: This junk chunk is added for padding, so that the header is a fixed size
               regardless of whether it's RF64 or not. That way, we can begin recording a file,
               and when it's finished, can go back and write either a RIFF or RF64 header,
               depending on whether more than 2^32 samples were written.

               The DRX_WAV_DO_NOT_PAD_HEADER_SIZE macro allows you to disable this feature in case
               you need to create files for crappy WAV players with bugs that stop them skipping chunks
               which they don't recognise. But DO NOT USE THIS option unless you really have no choice,
               because it means that if you write more than 2^32 samples to the file, you'll corrupt it.
            */
            writeChunkHeader (chunkName ("JUNK"), 28 + (isWaveFmtEx? 0 : 24));
            output->writeRepeatedByte (0, 28 /* ds64 */ + (isWaveFmtEx? 0 : 24));
           #endif
        }
        else
        {
           #if DRX_WAV_DO_NOT_PAD_HEADER_SIZE
            // If you disable padding, then you MUST NOT write more than 2^32 samples to a file.
            jassertfalse;
           #endif

            writeChunkHeader (chunkName ("ds64"), 28);  // chunk size for uncompressed data (no table)
            output->writeInt64 (riffChunkSize);
            output->writeInt64 ((z64) audioDataSize);
            output->writeRepeatedByte (0, 12);
        }

        if (isWaveFmtEx)
        {
            writeChunkHeader (chunkName ("fmt "), 40);
            output->writeShort ((short) (u16) 0xfffe); // WAVE_FORMAT_EXTENSIBLE
        }
        else
        {
            writeChunkHeader (chunkName ("fmt "), 16);
            output->writeShort (bitsPerSample < 32 ? (short) 1 /*WAVE_FORMAT_PCM*/
                                                   : (short) 3 /*WAVE_FORMAT_IEEE_FLOAT*/);
        }

        output->writeShort ((short) numChannels);
        output->writeInt ((i32) sampleRate);
        output->writeInt ((i32) ((f64) bytesPerFrame * sampleRate)); // nAvgBytesPerSec
        output->writeShort ((short) bytesPerFrame); // nBlockAlign
        output->writeShort ((short) bitsPerSample); // wBitsPerSample

        if (isWaveFmtEx)
        {
            output->writeShort (22); // cbSize (size of the extension)
            output->writeShort ((short) bitsPerSample); // wValidBitsPerSample
            output->writeInt (channelMask);

            const ExtensibleWavSubFormat& subFormat = bitsPerSample < 32 ? pcmFormat : IEEEFloatFormat;

            output->writeInt ((i32) subFormat.data1);
            output->writeShort ((short) subFormat.data2);
            output->writeShort ((short) subFormat.data3);
            output->write (subFormat.data4, sizeof (subFormat.data4));
        }

        writeChunk (bwavChunk,     chunkName ("bext"));
        writeChunk (ixmlChunk,     chunkName ("iXML"));
        writeChunk (axmlChunk,     chunkName ("axml"));
        writeChunk (smplChunk,     chunkName ("smpl"));
        writeChunk (instChunk,     chunkName ("inst"), 7);
        writeChunk (cueChunk,      chunkName ("cue "));
        writeChunk (listChunk,     chunkName ("LIST"));
        writeChunk (listInfoChunk, chunkName ("LIST"));
        writeChunk (acidChunk,     chunkName ("acid"));
        writeChunk (trckChunk,     chunkName ("Trkn"));

        writeChunkHeader (chunkName ("data"), isRF64 ? -1 : (i32) (lengthInSamples * bytesPerFrame));

        usesFloatingPointData = (bitsPerSample == 32);
    }

    static size_t chunkSize (const MemoryBlock& data) noexcept     { return data.isEmpty() ? 0 : (8 + data.getSize()); }

    z0 writeChunkHeader (i32 chunkType, i32 size) const
    {
        output->writeInt (chunkType);
        output->writeInt (size);
    }

    z0 writeChunk (const MemoryBlock& data, i32 chunkType, i32 size = 0) const
    {
        if (! data.isEmpty())
        {
            writeChunkHeader (chunkType, size != 0 ? size : (i32) data.getSize());
            *output << data;
        }
    }

    static i32 getChannelMaskFromChannelLayout (const AudioChannelSet& layout)
    {
        if (layout.isDiscreteLayout())
            return 0;

        // Don't add an extended format chunk for mono and stereo. Basically, all wav players
        // interpret a wav file with only one or two channels to be mono or stereo anyway.
        if (layout == AudioChannelSet::mono() || layout == AudioChannelSet::stereo())
            return 0;

        auto channels = layout.getChannelTypes();
        auto wavChannelMask = 0;

        for (auto channel : channels)
        {
            i32 wavChannelBit = static_cast<i32> (channel) - 1;
            jassert (wavChannelBit >= 0 && wavChannelBit <= 31);

            wavChannelMask |= (1 << wavChannelBit);
        }

        return wavChannelMask;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormatWriter)
};

//==============================================================================
class MemoryMappedWavReader final : public MemoryMappedAudioFormatReader
{
public:
    MemoryMappedWavReader (const File& wavFile, const WavAudioFormatReader& reader)
        : MemoryMappedAudioFormatReader (wavFile, reader, reader.dataChunkStart,
                                         reader.dataLength, reader.bytesPerFrame)
    {
    }

    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (numSamples <= 0)
            return true;

        if (map == nullptr || ! mappedSection.contains (Range<z64> (startSampleInFile, startSampleInFile + numSamples)))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.
            return false;
        }

        WavAudioFormatReader::copySampleData (bitsPerSample, usesFloatingPointData,
                                              destSamples, startOffsetInDestBuffer, numDestChannels,
                                              sampleToPointer (startSampleInFile), (i32) numChannels, numSamples);
        return true;
    }

    z0 getSample (z64 sample, f32* result) const noexcept override
    {
        auto num = (i32) numChannels;

        if (map == nullptr || ! mappedSection.contains (sample))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.

            zeromem (result, (size_t) num * sizeof (f32));
            return;
        }

        auto dest = &result;
        auto source = sampleToPointer (sample);

        switch (bitsPerSample)
        {
            case 8:     ReadHelper<AudioData::Float32, AudioData::UInt8, AudioData::LittleEndian>::read (dest, 0, 1, source, 1, num); break;
            case 16:    ReadHelper<AudioData::Float32, AudioData::Int16, AudioData::LittleEndian>::read (dest, 0, 1, source, 1, num); break;
            case 24:    ReadHelper<AudioData::Float32, AudioData::Int24, AudioData::LittleEndian>::read (dest, 0, 1, source, 1, num); break;
            case 32:    if (usesFloatingPointData) ReadHelper<AudioData::Float32, AudioData::Float32, AudioData::LittleEndian>::read (dest, 0, 1, source, 1, num);
                        else                       ReadHelper<AudioData::Float32, AudioData::Int32,   AudioData::LittleEndian>::read (dest, 0, 1, source, 1, num);
                        break;
            default:    jassertfalse; break;
        }
    }

    z0 readMaxLevels (z64 startSampleInFile, z64 numSamples, Range<f32>* results, i32 numChannelsToRead) override
    {
        numSamples = jmin (numSamples, lengthInSamples - startSampleInFile);

        if (map == nullptr || numSamples <= 0 || ! mappedSection.contains (Range<z64> (startSampleInFile, startSampleInFile + numSamples)))
        {
            jassert (numSamples <= 0); // you must make sure that the window contains all the samples you're going to attempt to read.

            for (i32 i = 0; i < numChannelsToRead; ++i)
                results[i] = {};

            return;
        }

        switch (bitsPerSample)
        {
            case 8:     scanMinAndMax<AudioData::UInt8> (startSampleInFile, numSamples, results, numChannelsToRead); break;
            case 16:    scanMinAndMax<AudioData::Int16> (startSampleInFile, numSamples, results, numChannelsToRead); break;
            case 24:    scanMinAndMax<AudioData::Int24> (startSampleInFile, numSamples, results, numChannelsToRead); break;
            case 32:    if (usesFloatingPointData) scanMinAndMax<AudioData::Float32> (startSampleInFile, numSamples, results, numChannelsToRead);
                        else                       scanMinAndMax<AudioData::Int32>   (startSampleInFile, numSamples, results, numChannelsToRead);
                        break;
            default:    jassertfalse; break;
        }
    }

    using AudioFormatReader::readMaxLevels;

private:
    template <typename SampleType>
    z0 scanMinAndMax (z64 startSampleInFile, z64 numSamples, Range<f32>* results, i32 numChannelsToRead) const noexcept
    {
        for (i32 i = 0; i < numChannelsToRead; ++i)
            results[i] = scanMinAndMaxInterleaved<SampleType, AudioData::LittleEndian> (i, startSampleInFile, numSamples);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedWavReader)
};

//==============================================================================
WavAudioFormat::WavAudioFormat()  : AudioFormat (wavFormatName, ".wav .bwf") {}
WavAudioFormat::~WavAudioFormat() {}

Array<i32> WavAudioFormat::getPossibleSampleRates()
{
    return { 8000,  11025, 12000, 16000,  22050,  32000,  44100,
             48000, 88200, 96000, 176400, 192000, 352800, 384000 };
}

Array<i32> WavAudioFormat::getPossibleBitDepths()
{
    return { 8, 16, 24, 32 };
}

b8 WavAudioFormat::canDoStereo()  { return true; }
b8 WavAudioFormat::canDoMono()    { return true; }

b8 WavAudioFormat::isChannelLayoutSupported (const AudioChannelSet& channelSet)
{
    auto channelTypes = channelSet.getChannelTypes();

    // When
    if (channelSet.isDiscreteLayout())
        return true;

    // WAV supports all channel types from left ... topRearRight
    for (auto channel : channelTypes)
        if (channel < AudioChannelSet::left || channel > AudioChannelSet::topRearRight)
            return false;

    return true;
}

AudioFormatReader* WavAudioFormat::createReaderFor (InputStream* sourceStream, b8 deleteStreamIfOpeningFails)
{
    std::unique_ptr<WavAudioFormatReader> r (new WavAudioFormatReader (sourceStream));

   #if DRX_USE_OGGVORBIS
    if (r->isSubformatOggVorbis)
    {
        r->input = nullptr;
        return OggVorbisAudioFormat().createReaderFor (sourceStream, deleteStreamIfOpeningFails);
    }
   #endif

    if (r->sampleRate > 0 && r->numChannels > 0 && r->bytesPerFrame > 0 && r->bitsPerSample <= 32)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

MemoryMappedAudioFormatReader* WavAudioFormat::createMemoryMappedReader (const File& file)
{
    return createMemoryMappedReader (file.createInputStream().release());
}

MemoryMappedAudioFormatReader* WavAudioFormat::createMemoryMappedReader (FileInputStream* fin)
{
    if (fin != nullptr)
    {
        WavAudioFormatReader reader (fin);

        if (reader.lengthInSamples > 0)
            return new MemoryMappedWavReader (fin->getFile(), reader);
    }

    return nullptr;
}

AudioFormatWriter* WavAudioFormat::createWriterFor (OutputStream* out, f64 sampleRate,
                                                    u32 numChannels, i32 bitsPerSample,
                                                    const StringPairArray& metadataValues, i32 qualityOptionIndex)
{
    return createWriterFor (out, sampleRate, WavFileHelpers::canonicalWavChannelSet (static_cast<i32> (numChannels)),
                            bitsPerSample, metadataValues, qualityOptionIndex);
}

AudioFormatWriter* WavAudioFormat::createWriterFor (OutputStream* out,
                                                    f64 sampleRate,
                                                    const AudioChannelSet& channelLayout,
                                                    i32 bitsPerSample,
                                                    const StringPairArray& metadataValues,
                                                    i32 /*qualityOptionIndex*/)
{
    if (out != nullptr && getPossibleBitDepths().contains (bitsPerSample) && isChannelLayoutSupported (channelLayout))
        return new WavAudioFormatWriter (out, sampleRate, channelLayout,
                                         (u32) bitsPerSample, metadataValues);

    return nullptr;
}

namespace WavFileHelpers
{
    static b8 slowCopyWavFileWithNewMetadata (const File& file, const StringPairArray& metadata)
    {
        TemporaryFile tempFile (file);
        WavAudioFormat wav;

        std::unique_ptr<AudioFormatReader> reader (wav.createReaderFor (file.createInputStream().release(), true));

        if (reader != nullptr)
        {
            std::unique_ptr<OutputStream> outStream (tempFile.getFile().createOutputStream());

            if (outStream != nullptr)
            {
                std::unique_ptr<AudioFormatWriter> writer (wav.createWriterFor (outStream.get(), reader->sampleRate,
                                                                                reader->numChannels, (i32) reader->bitsPerSample,
                                                                                metadata, 0));

                if (writer != nullptr)
                {
                    outStream.release();

                    b8 ok = writer->writeFromAudioReader (*reader, 0, -1);
                    writer.reset();
                    reader.reset();

                    return ok && tempFile.overwriteTargetFileWithTemporary();
                }
            }
        }

        return false;
    }
}

b8 WavAudioFormat::replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata)
{
    using namespace WavFileHelpers;

    std::unique_ptr<WavAudioFormatReader> reader (static_cast<WavAudioFormatReader*> (createReaderFor (wavFile.createInputStream().release(), true)));

    if (reader != nullptr)
    {
        auto bwavPos  = reader->bwavChunkStart;
        auto bwavSize = reader->bwavSize;
        reader.reset();

        if (bwavSize > 0)
        {
            auto chunk = BWAVChunk::createFrom (toMap (newMetadata));

            if (chunk.getSize() <= (size_t) bwavSize)
            {
                // the new one will fit in the space available, so write it directly..
                auto oldSize = wavFile.getSize();

                {
                    FileOutputStream out (wavFile);

                    if (out.openedOk())
                    {
                        out.setPosition (bwavPos);
                        out << chunk;
                        out.setPosition (oldSize);
                    }
                }

                jassert (wavFile.getSize() == oldSize);
                return true;
            }
        }
    }

    return slowCopyWavFileWithNewMetadata (wavFile, newMetadata);
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct WaveAudioFormatTests final : public UnitTest
{
    WaveAudioFormatTests()
        : UnitTest ("Wave audio format tests", UnitTestCategories::audio)
    {}

    z0 runTest() override
    {
        beginTest ("Setting up metadata");

        auto metadataValues = toMap (WavAudioFormat::createBWAVMetadata ("description",
                                                                         "originator",
                                                                         "originatorRef",
                                                                         Time::getCurrentTime(),
                                                                         numTestAudioBufferSamples,
                                                                         "codingHistory"));

        for (i32 i = numElementsInArray (WavFileHelpers::ListInfoChunk::types); --i >= 0;)
            metadataValues[WavFileHelpers::ListInfoChunk::types[i]] = WavFileHelpers::ListInfoChunk::types[i];

        const Txt prefixCode { "AA6Q7" }; // two letters followed by three alphanumeric characters
        const Txt yearOfReference { "20" }; // two digits, 20 meaning the year 2020
        const Txt designationCode { "00047" }; // five digits

        metadataValues[WavAudioFormat::internationalStandardRecordingCode] = prefixCode + yearOfReference + designationCode;;

        if (metadataValues.size() > 0)
            metadataValues["MetaDataSource"] = "WAV";

        const auto smplMetadata = createDefaultSMPLMetadata();
        metadataValues.insert (smplMetadata.cbegin(), smplMetadata.cend());

        WavAudioFormat format;
        MemoryBlock memoryBlock;

        StringPairArray metadataArray;
        metadataArray.addUnorderedMap (metadataValues);

        {
            beginTest ("Metadata can be written and read");

            const auto newMetadata = getMetadataAfterReading (format, writeToBlock (format, metadataArray));
            expect (newMetadata == metadataArray, "Somehow, the metadata is different!");
        }

        {
            beginTest ("Files containing a riff info source and an empty ISRC associate the source with the riffInfoSource key");
            StringPairArray meta;
            meta.addMap ({ { WavAudioFormat::riffInfoSource, "customsource" },
                           { WavAudioFormat::internationalStandardRecordingCode, "" } });
            const auto mb = writeToBlock (format, meta);
            checkPatternsPresent (mb, { "INFOISRC" });
            checkPatternsNotPresent (mb, { "ISRC:", "<ebucore" });
            const auto a = getMetadataAfterReading (format, mb);
            expect (a[WavAudioFormat::riffInfoSource] == "customsource");
            expect (a[WavAudioFormat::internationalStandardRecordingCode] == "");
        }

        {
            beginTest ("Files containing a riff info source and no ISRC associate the source with both keys "
                       "for backwards compatibility");
            StringPairArray meta;
            meta.addMap ({ { WavAudioFormat::riffInfoSource, "customsource" } });
            const auto mb = writeToBlock (format, meta);
            checkPatternsPresent (mb, { "INFOISRC", "ISRC:customsource", "<ebucore" });
            const auto a = getMetadataAfterReading (format, mb);
            expect (a[WavAudioFormat::riffInfoSource] == "customsource");
            expect (a[WavAudioFormat::internationalStandardRecordingCode] == "customsource");
        }

        {
            beginTest ("Files containing an ISRC associate the value with the internationalStandardRecordingCode key "
                       "and the riffInfoSource key for backwards compatibility");
            StringPairArray meta;
            meta.addMap ({ { WavAudioFormat::internationalStandardRecordingCode, "AABBBCCDDDDD" } });
            const auto mb = writeToBlock (format, meta);
            checkPatternsPresent (mb, { "ISRC:AABBBCCDDDDD", "<ebucore" });
            checkPatternsNotPresent (mb, { "INFOISRC" });
            const auto a = getMetadataAfterReading (format, mb);
            expect (a[WavAudioFormat::riffInfoSource] == "AABBBCCDDDDD");
            expect (a[WavAudioFormat::internationalStandardRecordingCode] == "AABBBCCDDDDD");
        }

        {
            beginTest ("Files containing an ISRC and a riff info source associate the values with the appropriate keys");
            StringPairArray meta;
            meta.addMap ({ { WavAudioFormat::riffInfoSource, "source" } });
            meta.addMap ({ { WavAudioFormat::internationalStandardRecordingCode, "UUVVVXXYYYYY" } });
            const auto mb = writeToBlock (format, meta);
            checkPatternsPresent (mb, { "INFOISRC", "ISRC:UUVVVXXYYYYY", "<ebucore" });
            const auto a = getMetadataAfterReading (format, mb);
            expect (a[WavAudioFormat::riffInfoSource] == "source");
            expect (a[WavAudioFormat::internationalStandardRecordingCode] == "UUVVVXXYYYYY");
        }

        {
            beginTest ("Files containing ASWG metadata read and write correctly");
            MemoryBlock block;
            StringPairArray meta;

            for (const auto& key : WavFileHelpers::IXMLChunk::aswgMetadataKeys)
                meta.set (key, "Test123&<>");

            {
                auto writer = rawToUniquePtr (WavAudioFormat().createWriterFor (new MemoryOutputStream (block, false), 48000, 1, 32, meta, 0));
                expect (writer != nullptr);
            }

            expect ([&]
            {
                auto input = std::make_unique<MemoryInputStream> (block, false);

                while (! input->isExhausted())
                {
                    t8 chunkType[4] {};
                    auto pos = input->getPosition();

                    input->read (chunkType, 4);

                    if (memcmp (chunkType, "iXML", 4) == 0)
                    {
                        auto length = (u32) input->readInt();

                        MemoryBlock xmlBlock;
                        input->readIntoMemoryBlock (xmlBlock, (ssize_t) length);

                        return parseXML (xmlBlock.toString()) != nullptr;
                    }

                    input->setPosition (pos + 1);
                }

                return false;
            }());

            {
                auto reader = rawToUniquePtr (WavAudioFormat().createReaderFor (new MemoryInputStream (block, false), true));
                expect (reader != nullptr);

                for (const auto& key : meta.getAllKeys())
                {
                    const auto oldValue = meta.getValue (key, "!");
                    const auto newValue = reader->metadataValues.getValue (key, "");
                    expectEquals (oldValue, newValue);
                }

                expect (reader->metadataValues.getValue (WavAudioFormat::aswgVersion, "") == "3.01");
            }
        }
    }

private:
    MemoryBlock writeToBlock (WavAudioFormat& format, StringPairArray meta)
    {
        MemoryBlock mb;

        {
            // The destructor of the writer will modify the block, so make sure that we've
            // destroyed the writer before returning the block!
            auto writer = rawToUniquePtr (format.createWriterFor (new MemoryOutputStream (mb, false),
                                                                  44100.0,
                                                                  numTestAudioBufferChannels,
                                                                  16,
                                                                  meta,
                                                                  0));
            expect (writer != nullptr);
            AudioBuffer<f32> buffer (numTestAudioBufferChannels, numTestAudioBufferSamples);
            expect (writer->writeFromAudioSampleBuffer (buffer, 0, numTestAudioBufferSamples));
        }

        return mb;
    }

    StringPairArray getMetadataAfterReading (WavAudioFormat& format, const MemoryBlock& mb)
    {
        auto reader = rawToUniquePtr (format.createReaderFor (new MemoryInputStream (mb, false), true));
        expect (reader != nullptr);
        return reader->metadataValues;
    }

    template <typename Fn>
    z0 checkPatterns (const MemoryBlock& mb, const std::vector<std::string>& patterns, Fn&& fn)
    {
        for (const auto& pattern : patterns)
        {
            const auto begin = static_cast<tukk> (mb.getData());
            const auto end = begin + mb.getSize();
            expect (fn (std::search (begin, end, pattern.begin(), pattern.end()), end));
        }
    }

    z0 checkPatternsPresent (const MemoryBlock& mb, const std::vector<std::string>& patterns)
    {
        checkPatterns (mb, patterns, std::not_equal_to<>{});
    }

    z0 checkPatternsNotPresent (const MemoryBlock& mb, const std::vector<std::string>& patterns)
    {
        checkPatterns (mb, patterns, std::equal_to<>{});
    }

    enum
    {
        numTestAudioBufferChannels = 2,
        numTestAudioBufferSamples = 256
    };

    static StringMap createDefaultSMPLMetadata()
    {
        StringMap m;

        m["Manufacturer"] = "0";
        m["Product"] = "0";
        m["SamplePeriod"] = "0";
        m["MidiUnityNote"] = "60";
        m["MidiPitchFraction"] = "0";
        m["SmpteFormat"] = "0";
        m["SmpteOffset"] = "0";
        m["NumSampleLoops"] = "0";
        m["SamplerData"] = "0";

        return m;
    }

    DRX_DECLARE_NON_COPYABLE (WaveAudioFormatTests)
};

static const WaveAudioFormatTests waveAudioFormatTests;

#endif

} // namespace drx
