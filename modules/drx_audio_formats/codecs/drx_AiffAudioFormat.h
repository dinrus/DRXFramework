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
    Reads and Writes AIFF format audio files.

    @see AudioFormat

    @tags{Audio}
*/
class DRX_API  AiffAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates an format object. */
    AiffAudioFormat();

    /** Destructor. */
    ~AiffAudioFormat() override;

    //==============================================================================
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleOneShot;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleRootSet;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleRootNote;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleBeats;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleDenominator;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleNumerator;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleTag;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static tukk const appleKey;

    //==============================================================================
    Array<i32> getPossibleSampleRates() override;
    Array<i32> getPossibleBitDepths() override;
    b8 canDoStereo() override;
    b8 canDoMono() override;

   #if DRX_MAC
    b8 canHandleFile (const File& fileToTest) override;
   #endif

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
    using AudioFormat::createWriterFor;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AiffAudioFormat)
};

} // namespace drx
