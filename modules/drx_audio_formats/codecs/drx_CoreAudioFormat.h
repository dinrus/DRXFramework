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

#if DRX_MAC || DRX_IOS || DOXYGEN

//==============================================================================
/**
    OSX and iOS only - This uses the AudioToolbox framework to read any audio
    format that the system has a codec for.

    This should be able to understand formats such as mp3, m4a, etc.

    @see AudioFormat

    @tags{Audio}
*/
class DRX_API  CoreAudioFormat     : public AudioFormat
{
public:
    /** File type hints. */
    enum class StreamKind
    {
        kNone,
        kAiff,
        kAifc,
        kWave,
        kSoundDesigner2,
        kNext,
        kMp3,
        kMp2,
        kMp1,
        kAc3,
        kAacAdts,
        kMpeg4,
        kM4a,
        kM4b,
        kCaf,
        k3gp,
        k3gp2,
        kAmr,
    };

    //==============================================================================
    /** Creates a format object. */
    CoreAudioFormat();

    /** Creates a format object and provides a hint as to the format of data
        to be read or written.
    */
    explicit CoreAudioFormat (StreamKind);

    /** Destructor. */
    ~CoreAudioFormat() override;

    //==============================================================================
    /** Metadata property name used when reading a caf file with a MIDI chunk. */
    static tukk const midiDataBase64;
    /** Metadata property name used when reading a caf file with tempo information. */
    static tukk const tempo;
    /** Metadata property name used when reading a caf file time signature information. */
    static tukk const timeSig;
    /** Metadata property name used when reading a caf file time signature information. */
    static tukk const keySig;

    //==============================================================================
    Array<i32> getPossibleSampleRates() override;
    Array<i32> getPossibleBitDepths() override;
    b8 canDoStereo() override;
    b8 canDoMono() override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream*,
                                        b8 deleteStreamIfOpeningFails) override;

    AudioFormatWriter* createWriterFor (OutputStream*,
                                        f64 sampleRateToUse,
                                        u32 numberOfChannels,
                                        i32 bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        i32 qualityOptionIndex) override;
    using AudioFormat::createWriterFor;

private:
    StreamKind streamKind = StreamKind::kNone;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioFormat)
};

#endif

} // namespace drx
