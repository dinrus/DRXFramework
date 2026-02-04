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

#if DRX_USE_LAME_AUDIO_FORMAT || DOXYGEN

//==============================================================================
/**
    An AudioFormat class which can use an installed version of the LAME mp3
    encoder to encode a file.

    This format can't read MP3s, it just writes them. Internally, the
    AudioFormatWriter object that is returned writes the incoming audio data
    to a temporary WAV file, and then when the writer is deleted, it invokes
    the LAME executable to convert the data to an MP3, whose data is then
    piped into the original OutputStream that was used when first creating
    the writer.

    @see AudioFormat

    @tags{Audio}
*/
class DRX_API  LAMEEncoderAudioFormat    : public AudioFormat
{
public:
    /** Creates a LAMEEncoderAudioFormat that expects to find a working LAME
        executable at the location given.
    */
    LAMEEncoderAudioFormat (const File& lameExecutableToUse);
    ~LAMEEncoderAudioFormat();

    b8 canHandleFile (const File&);
    Array<i32> getPossibleSampleRates();
    Array<i32> getPossibleBitDepths();
    b8 canDoStereo();
    b8 canDoMono();
    b8 isCompressed();
    StringArray getQualityOptions();

    AudioFormatReader* createReaderFor (InputStream*, b8 deleteStreamIfOpeningFails);

    AudioFormatWriter* createWriterFor (OutputStream*, f64 sampleRateToUse,
                                        u32 numberOfChannels, i32 bitsPerSample,
                                        const StringPairArray& metadataValues, i32 qualityOptionIndex);
    using AudioFormat::createWriterFor;

private:
    File lameApp;
    class Writer;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LAMEEncoderAudioFormat)
};

#endif

} // namespace drx
