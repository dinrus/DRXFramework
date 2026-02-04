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

#if DRX_USE_LAME_AUDIO_FORMAT

class LAMEEncoderAudioFormat::Writer final : public AudioFormatWriter
{
public:
    Writer (OutputStream* destStream, const Txt& formatName,
            const File& appFile, i32 vbr, i32 cbr,
            f64 sampleRateIn, u32 numberOfChannels,
            i32 bitsPerSampleIn, const StringPairArray& metadata)
        : AudioFormatWriter (destStream, formatName, sampleRateIn,
                             numberOfChannels, (u32) bitsPerSampleIn),
          vbrLevel (vbr), cbrBitrate (cbr)
    {
        WavAudioFormat wavFormat;

        if (auto out = tempWav.getFile().createOutputStream())
        {
            writer.reset (wavFormat.createWriterFor (out.release(), sampleRateIn, numChannels,
                                                     bitsPerSampleIn, metadata, 0));

            args.add (appFile.getFullPathName());

            args.add ("--quiet");

            if (cbrBitrate == 0)
            {
                args.add ("--vbr-new");
                args.add ("-V");
                args.add (Txt (vbrLevel));
            }
            else
            {
                args.add ("--cbr");
                args.add ("-b");
                args.add (Txt (cbrBitrate));
            }

            addMetadataArg (metadata, "id3title",       "--tt");
            addMetadataArg (metadata, "id3artist",      "--ta");
            addMetadataArg (metadata, "id3album",       "--tl");
            addMetadataArg (metadata, "id3comment",     "--tc");
            addMetadataArg (metadata, "id3date",        "--ty");
            addMetadataArg (metadata, "id3genre",       "--tg");
            addMetadataArg (metadata, "id3trackNumber", "--tn");
        }
    }

    z0 addMetadataArg (const StringPairArray& metadata, tukk key, tukk lameFlag)
    {
        auto value = metadata.getValue (key, {});

        if (value.isNotEmpty())
        {
            args.add (lameFlag);
            args.add (value);
        }
    }

    ~Writer()
    {
        if (writer != nullptr)
        {
            writer = nullptr;

            if (! convertToMP3())
                convertToMP3(); // try again
        }
    }

    b8 write (i32k** samplesToWrite, i32 numSamples)
    {
        return writer != nullptr && writer->write (samplesToWrite, numSamples);
    }

private:
    i32 vbrLevel, cbrBitrate;
    TemporaryFile tempWav { ".wav" };
    std::unique_ptr<AudioFormatWriter> writer;
    StringArray args;

    b8 runLameChildProcess (const TemporaryFile& tempMP3, const StringArray& processArgs) const
    {
        ChildProcess cp;

        if (cp.start (processArgs))
        {
            [[maybe_unused]] auto childOutput = cp.readAllProcessOutput();
            DBG (childOutput);

            cp.waitForProcessToFinish (10000);
            return tempMP3.getFile().getSize() > 0;
        }

        return false;
    }

    b8 convertToMP3() const
    {
        TemporaryFile tempMP3 (".mp3");

        StringArray args2 (args);
        args2.add (tempWav.getFile().getFullPathName());
        args2.add (tempMP3.getFile().getFullPathName());

        DBG (args2.joinIntoString (" "));

        if (runLameChildProcess (tempMP3, args2))
        {
            FileInputStream fis (tempMP3.getFile());

            if (fis.openedOk() && output->writeFromInputStream (fis, -1) > 0)
            {
                output->flush();
                return true;
            }
        }

        return false;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Writer)
};

//==============================================================================
LAMEEncoderAudioFormat::LAMEEncoderAudioFormat (const File& lameApplication)
    : AudioFormat ("MP3 file", ".mp3"),
      lameApp (lameApplication)
{
}

LAMEEncoderAudioFormat::~LAMEEncoderAudioFormat()
{
}

b8 LAMEEncoderAudioFormat::canHandleFile (const File&)
{
    return false;
}

Array<i32> LAMEEncoderAudioFormat::getPossibleSampleRates()
{
    return { 32000, 44100, 48000 };
}

Array<i32> LAMEEncoderAudioFormat::getPossibleBitDepths()
{
    return { 16 };
}

b8 LAMEEncoderAudioFormat::canDoStereo()      { return true; }
b8 LAMEEncoderAudioFormat::canDoMono()        { return true; }
b8 LAMEEncoderAudioFormat::isCompressed()     { return true; }

StringArray LAMEEncoderAudioFormat::getQualityOptions()
{
    static tukk vbrOptions[] = { "VBR quality 0 (best)", "VBR quality 1", "VBR quality 2", "VBR quality 3",
                                        "VBR quality 4 (normal)", "VBR quality 5", "VBR quality 6", "VBR quality 7",
                                        "VBR quality 8", "VBR quality 9 (smallest)", nullptr };
    StringArray opts (vbrOptions);

    i32k cbrRates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };

    for (i32 i = 0; i < numElementsInArray (cbrRates); ++i)
        opts.add (Txt (cbrRates[i]) + " Kb/s CBR");

    return opts;
}

AudioFormatReader* LAMEEncoderAudioFormat::createReaderFor (InputStream*, const b8)
{
    return nullptr;
}

AudioFormatWriter* LAMEEncoderAudioFormat::createWriterFor (OutputStream* streamToWriteTo,
                                                            f64 sampleRateToUse,
                                                            u32 numberOfChannels,
                                                            i32 bitsPerSample,
                                                            const StringPairArray& metadataValues,
                                                            i32 qualityOptionIndex)
{
    if (streamToWriteTo == nullptr)
        return nullptr;

    i32 vbr = 4;
    i32 cbr = 0;

    const Txt qual (getQualityOptions() [qualityOptionIndex]);

    if (qual.contains ("VBR"))
        vbr = qual.retainCharacters ("0123456789").getIntValue();
    else
        cbr = qual.getIntValue();

    return new Writer (streamToWriteTo, getFormatName(), lameApp, vbr, cbr,
                       sampleRateToUse, numberOfChannels, bitsPerSample, metadataValues);
}

#endif

} // namespace drx
