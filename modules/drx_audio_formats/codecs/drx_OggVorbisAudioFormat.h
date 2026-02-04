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

#if DRX_USE_OGGVORBIS || DOXYGEN

//==============================================================================
/**
    Reads and writes the Ogg-Vorbis audio format.

    To compile this, you'll need to set the DRX_USE_OGGVORBIS flag.

    @see AudioFormat,

    @tags{Audio}
*/
class DRX_API  OggVorbisAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    OggVorbisAudioFormat();
    ~OggVorbisAudioFormat() override;

    //==============================================================================
    Array<i32> getPossibleSampleRates() override;
    Array<i32> getPossibleBitDepths() override;
    b8 canDoStereo() override;
    b8 canDoMono() override;
    b8 isCompressed() override;
    StringArray getQualityOptions() override;

    //==============================================================================
    /** Tries to estimate the quality level of an ogg file based on its size.

        If it can't read the file for some reason, this will just return 1 (medium quality),
        otherwise it will return the approximate quality setting that would have been used
        to create the file.

        @see getQualityOptions
    */
    i32 estimateOggFileQuality (const File& source);

    //==============================================================================
    /** Metadata property name used by the Ogg writer - if you set a string for this
        value, it will be written into the ogg file as the name of the encoder app.

        @see createWriterFor
    */
    static tukk const encoderName;

    static tukk const id3title;          /**< Metadata key for setting an ID3 title. */
    static tukk const id3artist;         /**< Metadata key for setting an ID3 artist name. */
    static tukk const id3album;          /**< Metadata key for setting an ID3 album. */
    static tukk const id3comment;        /**< Metadata key for setting an ID3 comment. */
    static tukk const id3date;           /**< Metadata key for setting an ID3 date. */
    static tukk const id3genre;          /**< Metadata key for setting an ID3 genre. */
    static tukk const id3trackNumber;    /**< Metadata key for setting an ID3 track number. */

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        b8 deleteStreamIfOpeningFails) override;

    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        f64 sampleRateToUse,
                                        u32 numberOfChannels,
                                        i32 bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        i32 qualityOptionIndex) override;
    using AudioFormat::createWriterFor;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggVorbisAudioFormat)
};


#endif

} // namespace drx
