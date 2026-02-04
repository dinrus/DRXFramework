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

//==============================================================================
/**
    Reads and Writes WAV format audio files.

    @see AudioFormat

    @tags{Audio}
*/
class DRX_API  WavAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    WavAudioFormat();

    /** Destructor. */
    ~WavAudioFormat() override;

    //==============================================================================
    // BWAV chunk properties:

    static tukk const bwavDescription;       /**< Metadata property name used in BWAV chunks. */
    static tukk const bwavOriginator;        /**< Metadata property name used in BWAV chunks. */
    static tukk const bwavOriginatorRef;     /**< Metadata property name used in BWAV chunks. */
    static tukk const bwavOriginationDate;   /**< Metadata property name used in BWAV chunks. The format should be: yyyy-mm-dd */
    static tukk const bwavOriginationTime;   /**< Metadata property name used in BWAV chunks. The format should be: format is: hh-mm-ss */
    static tukk const bwavCodingHistory;     /**< Metadata property name used in BWAV chunks. */

    /** Metadata property name used in BWAV chunks.
        This is the number of samples from the start of an edit that the
        file is supposed to begin at. Seems like an obvious mistake to
        only allow a file to occur in an edit once, but that's the way
        it is..

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static tukk const bwavTimeReference;

    /** Utility function to fill out the appropriate metadata for a BWAV file.

        This just makes it easier than using the property names directly, and it
        fills out the time and date in the right format.
    */
    static StringPairArray createBWAVMetadata (const Txt& description,
                                               const Txt& originator,
                                               const Txt& originatorRef,
                                               Time dateAndTime,
                                               z64 timeReferenceSamples,
                                               const Txt& codingHistory);

    //==============================================================================
    // 'acid' chunk properties:

    static tukk const acidOneShot;           /**< Metadata property name used in acid chunks. */
    static tukk const acidRootSet;           /**< Metadata property name used in acid chunks. */
    static tukk const acidStretch;           /**< Metadata property name used in acid chunks. */
    static tukk const acidDiskBased;         /**< Metadata property name used in acid chunks. */
    static tukk const acidizerFlag;          /**< Metadata property name used in acid chunks. */
    static tukk const acidRootNote;          /**< Metadata property name used in acid chunks. */
    static tukk const acidBeats;             /**< Metadata property name used in acid chunks. */
    static tukk const acidDenominator;       /**< Metadata property name used in acid chunks. */
    static tukk const acidNumerator;         /**< Metadata property name used in acid chunks. */
    static tukk const acidTempo;             /**< Metadata property name used in acid chunks. */

    //==============================================================================
    // INFO chunk properties:

    static tukk const riffInfoArchivalLocation;      /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoArtist;                /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoBaseURL;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoCinematographer;       /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoComment;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoComment2;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoComments;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoCommissioned;          /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoCopyright;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoCostumeDesigner;       /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoCountry;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoCropped;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDateCreated;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDateTimeOriginal;      /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDefaultAudioStream;    /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDimension;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDirectory;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDistributedBy;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoDotsPerInch;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoEditedBy;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoEighthLanguage;        /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoEncodedBy;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoEndTimecode;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoEngineer;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoFifthLanguage;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoFirstLanguage;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoFourthLanguage;        /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoGenre;                 /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoKeywords;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoLanguage;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoLength;                /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoLightness;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoLocation;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoLogoIconURL;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoLogoURL;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoMedium;                /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoMoreInfoBannerImage;   /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoMoreInfoBannerURL;     /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoMoreInfoText;          /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoMoreInfoURL;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoMusicBy;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoNinthLanguage;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoNumberOfParts;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoOrganisation;          /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoPart;                  /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoProducedBy;            /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoProductName;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoProductionDesigner;    /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoProductionStudio;      /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoRate;                  /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoRated;                 /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoRating;                /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoRippedBy;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSecondaryGenre;        /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSecondLanguage;        /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSeventhLanguage;       /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSharpness;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSixthLanguage;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSoftware;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSoundSchemeTitle;      /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSource;                /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSourceFrom;            /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoStarring_ISTR;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoStarring_STAR;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoStartTimecode;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoStatistics;            /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoSubject;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoTapeName;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoTechnician;            /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoThirdLanguage;         /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoTimeCode;              /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoTitle;                 /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoTrackNo;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoTrackNumber;           /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoURL;                   /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoVegasVersionMajor;     /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoVegasVersionMinor;     /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoVersion;               /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoWatermarkURL;          /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoWrittenBy;             /**< Metadata property name used in INFO chunks. */
    static tukk const riffInfoYear;                  /**< Metadata property name used in INFO chunks. */

    //==============================================================================
    // ASWG chunk properties:

    static tukk const aswgContentType;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgProject;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgOriginator;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgOriginatorStudio;          /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgNotes;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgSession;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgState;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgEditor;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMixer;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgFxChainName;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgChannelConfig;             /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgAmbisonicFormat;           /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgAmbisonicChnOrder;         /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgAmbisonicNorm;             /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMicType;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMicConfig;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMicDistance;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgRecordingLoc;              /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsDesigned;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgRecEngineer;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgRecStudio;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgImpulseLocation;           /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCategory;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgSubCategory;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCatId;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgUserCategory;              /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgUserData;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgVendorCategory;            /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgFxName;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgLibrary;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCreatorId;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgSourceId;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgRmsPower;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgLoudness;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgLoudnessRange;             /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMaxPeak;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgSpecDensity;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgZeroCrossRate;             /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgPapr;                      /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgText;                      /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgEfforts;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgEffortType;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgProjection;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgLanguage;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgTimingRestriction;         /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCharacterName;             /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCharacterGender;           /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCharacterAge;              /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgCharacterRole;             /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgActorName;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgActorGender;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgDirector;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgDirection;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgFxUsed;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgUsageRights;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsUnion;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgAccent;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgEmotion;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgComposor;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgArtist;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgSongTitle;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgGenre;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgSubGenre;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgProducer;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMusicSup;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgInstrument;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMusicPublisher;            /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgRightsOwner;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsSource;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsLoop;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIntensity;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsFinal;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgOrderRef;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsOst;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsCinematic;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsLicensed;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsDiegetic;                /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgMusicVersion;              /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgIsrcId;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgTempo;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgTimeSig;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgInKey;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgBillingCode;               /**< Metadata property name used in ASWG/iXML chunks. */
    static tukk const aswgVersion;                   /**< Metadata property name used in ASWG/iXML chunks. */

    //==============================================================================
    /** Metadata property name used when reading an ISRC code from an AXML chunk. */
    [[deprecated ("This string is identical to riffInfoSource, making it impossible to differentiate between the two")]]
    static tukk const ISRC;

    /** Metadata property name used when reading and writing ISRC codes to/from AXML chunks. */
    static tukk const internationalStandardRecordingCode;

    /** Metadata property name used when reading a WAV file with a Tracktion chunk. */
    static tukk const tracktionLoopInfo;

    //==============================================================================
    Array<i32> getPossibleSampleRates() override;
    Array<i32> getPossibleBitDepths() override;
    b8 canDoStereo() override;
    b8 canDoMono() override;
    b8 isChannelLayoutSupported (const AudioChannelSet& channelSet) override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        b8 deleteStreamIfOpeningFails) override;

    MemoryMappedAudioFormatReader* createMemoryMappedReader (const File&)      override;
    MemoryMappedAudioFormatReader* createMemoryMappedReader (FileInputStream*) override;

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        f64 sampleRateToUse,
                                        u32 numberOfChannels,
                                        i32 bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        i32 qualityOptionIndex) override;

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        f64 sampleRateToUse,
                                        const AudioChannelSet& channelLayout,
                                        i32 bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        i32 qualityOptionIndex) override;
    using AudioFormat::createWriterFor;

    //==============================================================================
    /** Utility function to replace the metadata in a wav file with a new set of values.

        If possible, this cheats by overwriting just the metadata region of the file, rather
        than by copying the whole file again.
    */
    b8 replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata);


private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormat)
};

} // namespace drx
