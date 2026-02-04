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

#if DRX_USE_OGGVORBIS

#if DRX_MAC && ! defined (__MACOSX__)
 #define __MACOSX__ 1
#endif

namespace OggVorbisNamespace
{
#if DRX_INCLUDE_OGGVORBIS_CODE || ! defined (DRX_INCLUDE_OGGVORBIS_CODE)
 DRX_BEGIN_IGNORE_WARNINGS_MSVC (4267 4127 4244 4100 4701 4702 4013 4133 4206 4305 4189 4706 4995 4365 4456 4457 4459 6297 6011 6001 6308 6255 6386 6385 6246 6387 6263 6262 28182)

 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                      "-Wconversion",
                                      "-Wdeprecated-register",
                                      "-Wfloat-conversion",
                                      "-Wfloat-equal",
                                      "-Wmaybe-uninitialized",
                                      "-Wmisleading-indentation",
                                      "-Wmissing-prototypes",
                                      "-Wredundant-decls",
                                      "-Wshadow",
                                      "-Wsign-conversion",
                                      "-Wswitch-default",
                                      "-Wswitch-enum",
                                      "-Wzero-as-null-pointer-constant")

 DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

 DRX_BEGIN_NO_SANITIZE ("undefined")

 #include "oggvorbis/vorbisenc.h"
 #include "oggvorbis/codec.h"
 #include "oggvorbis/vorbisfile.h"

 #include "oggvorbis/bitwise.c"
 #include "oggvorbis/framing.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/analysis.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/bitrate.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/block.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/codebook.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/envelope.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/floor0.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/floor1.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/info.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/lpc.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/lsp.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/mapping0.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/mdct.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/psy.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/registry.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/res0.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/sharedbook.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/smallft.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/synthesis.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/vorbisenc.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/vorbisfile.c"
 #include "oggvorbis/libvorbis-1.3.7/lib/window.c"

 DRX_END_NO_SANITIZE
 DRX_END_IGNORE_DEPRECATION_WARNINGS
 DRX_END_IGNORE_WARNINGS_MSVC
 DRX_END_IGNORE_WARNINGS_GCC_LIKE
#else
 #include <vorbis/vorbisenc.h>
 #include <vorbis/codec.h>
 #include <vorbis/vorbisfile.h>
#endif
}

#undef max
#undef min

//==============================================================================
static tukk const oggFormatName = "Ogg-Vorbis file";

tukk const OggVorbisAudioFormat::encoderName = "encoder";
tukk const OggVorbisAudioFormat::id3title = "id3title";
tukk const OggVorbisAudioFormat::id3artist = "id3artist";
tukk const OggVorbisAudioFormat::id3album = "id3album";
tukk const OggVorbisAudioFormat::id3comment = "id3comment";
tukk const OggVorbisAudioFormat::id3date = "id3date";
tukk const OggVorbisAudioFormat::id3genre = "id3genre";
tukk const OggVorbisAudioFormat::id3trackNumber = "id3trackNumber";


//==============================================================================
class OggReader final : public AudioFormatReader
{
public:
    OggReader (InputStream* inp)  : AudioFormatReader (inp, oggFormatName)
    {
        sampleRate = 0;
        usesFloatingPointData = true;

        callbacks.read_func  = &oggReadCallback;
        callbacks.seek_func  = &oggSeekCallback;
        callbacks.close_func = &oggCloseCallback;
        callbacks.tell_func  = &oggTellCallback;

        auto err = ov_open_callbacks (input, &ovFile, nullptr, 0, callbacks);

        if (err == 0)
        {
            auto* info = ov_info (&ovFile, -1);

            auto* comment = ov_comment (&ovFile, -1);
            addMetadataItem (comment, "ENCODER",     OggVorbisAudioFormat::encoderName);
            addMetadataItem (comment, "TITLE",       OggVorbisAudioFormat::id3title);
            addMetadataItem (comment, "ARTIST",      OggVorbisAudioFormat::id3artist);
            addMetadataItem (comment, "ALBUM",       OggVorbisAudioFormat::id3album);
            addMetadataItem (comment, "COMMENT",     OggVorbisAudioFormat::id3comment);
            addMetadataItem (comment, "DATE",        OggVorbisAudioFormat::id3date);
            addMetadataItem (comment, "GENRE",       OggVorbisAudioFormat::id3genre);
            addMetadataItem (comment, "TRACKNUMBER", OggVorbisAudioFormat::id3trackNumber);

            lengthInSamples = (u32) ov_pcm_total (&ovFile, -1);
            numChannels = (u32) info->channels;
            bitsPerSample = 16;
            sampleRate = (f64) info->rate;

            reservoir.setSize ((i32) numChannels, (i32) jmin (lengthInSamples, (z64) 4096));
        }
    }

    ~OggReader() override
    {
        ov_clear (&ovFile);
    }

    z0 addMetadataItem (OggVorbisNamespace::vorbis_comment* comment, tukk name, tukk metadataName)
    {
        if (auto* value = vorbis_comment_query (comment, name, 0))
            metadataValues.set (metadataName, value);
    }

    //==============================================================================
    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override
    {
        const auto getBufferedRange = [this] { return bufferedRange; };

        const auto readFromReservoir = [this, &destSamples, &numDestChannels, &startOffsetInDestBuffer, &startSampleInFile] (const Range<z64> rangeToRead)
        {
            const auto bufferIndices = rangeToRead - bufferedRange.getStart();
            const auto writePos = (z64) startOffsetInDestBuffer + (rangeToRead.getStart() - startSampleInFile);

            for (i32 i = jmin (numDestChannels, reservoir.getNumChannels()); --i >= 0;)
                if (destSamples[i] != nullptr)
                    memcpy (destSamples[i] + writePos,
                            reservoir.getReadPointer (i) + bufferIndices.getStart(),
                            (size_t) bufferIndices.getLength() * sizeof (f32));
        };

        const auto fillReservoir = [this] (z64 requestedStart)
        {
            const auto newStart = jmax ((z64) 0, requestedStart);
            bufferedRange = Range<z64> { newStart, newStart + reservoir.getNumSamples() };

            if (bufferedRange.getStart() != ov_pcm_tell (&ovFile))
                ov_pcm_seek (&ovFile, bufferedRange.getStart());

            i32 bitStream = 0;
            i32 offset = 0;
            i32 numToRead = (i32) bufferedRange.getLength();

            while (numToRead > 0)
            {
                f32** dataIn = nullptr;
                auto samps = static_cast<i32> (ov_read_float (&ovFile, &dataIn, numToRead, &bitStream));

                if (samps <= 0)
                    break;

                jassert (samps <= numToRead);

                for (i32 i = jmin ((i32) numChannels, reservoir.getNumChannels()); --i >= 0;)
                    memcpy (reservoir.getWritePointer (i, offset), dataIn[i], (size_t) samps * sizeof (f32));

                numToRead -= samps;
                offset += samps;
            }

            if (numToRead > 0)
                reservoir.clear (offset, numToRead);
        };

        const auto remainingSamples = Reservoir::doBufferedRead (Range<z64> { startSampleInFile, startSampleInFile + numSamples },
                                                                 getBufferedRange,
                                                                 readFromReservoir,
                                                                 fillReservoir);

        if (! remainingSamples.isEmpty())
            for (i32 i = numDestChannels; --i >= 0;)
                if (destSamples[i] != nullptr)
                    zeromem (destSamples[i] + startOffsetInDestBuffer + (remainingSamples.getStart() - startSampleInFile),
                             (size_t) remainingSamples.getLength() * sizeof (i32));

        return true;
    }

    //==============================================================================
    static size_t oggReadCallback (uk ptr, size_t size, size_t nmemb, uk datasource)
    {
        return (size_t) (static_cast<InputStream*> (datasource)->read (ptr, (i32) (size * nmemb))) / size;
    }

    static i32 oggSeekCallback (uk datasource, OggVorbisNamespace::ogg_int64_t offset, i32 whence)
    {
        auto* in = static_cast<InputStream*> (datasource);

        if (whence == SEEK_CUR)
            offset += in->getPosition();
        else if (whence == SEEK_END)
            offset += in->getTotalLength();

        in->setPosition (offset);
        return 0;
    }

    static i32 oggCloseCallback (uk)
    {
        return 0;
    }

    static i64 oggTellCallback (uk datasource)
    {
        return (i64) static_cast<InputStream*> (datasource)->getPosition();
    }

private:
    OggVorbisNamespace::OggVorbis_File ovFile;
    OggVorbisNamespace::ov_callbacks callbacks;
    AudioBuffer<f32> reservoir;
    Range<z64> bufferedRange;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggReader)
};

//==============================================================================
class OggWriter final : public AudioFormatWriter
{
public:
    OggWriter (OutputStream* out, f64 rate,
               u32 numChans, u32 bitsPerSamp,
               i32 qualityIndex, const StringPairArray& metadata)
        : AudioFormatWriter (out, oggFormatName, rate, numChans, bitsPerSamp)
    {
        vorbis_info_init (&vi);

        if (vorbis_encode_init_vbr (&vi, (i32) numChans, (i32) rate,
                                    jlimit (0.0f, 1.0f, (f32) qualityIndex * 0.1f)) == 0)
        {
            vorbis_comment_init (&vc);

            addMetadata (metadata, OggVorbisAudioFormat::encoderName,    "ENCODER");
            addMetadata (metadata, OggVorbisAudioFormat::id3title,       "TITLE");
            addMetadata (metadata, OggVorbisAudioFormat::id3artist,      "ARTIST");
            addMetadata (metadata, OggVorbisAudioFormat::id3album,       "ALBUM");
            addMetadata (metadata, OggVorbisAudioFormat::id3comment,     "COMMENT");
            addMetadata (metadata, OggVorbisAudioFormat::id3date,        "DATE");
            addMetadata (metadata, OggVorbisAudioFormat::id3genre,       "GENRE");
            addMetadata (metadata, OggVorbisAudioFormat::id3trackNumber, "TRACKNUMBER");

            vorbis_analysis_init (&vd, &vi);
            vorbis_block_init (&vd, &vb);

            ogg_stream_init (&os, Random::getSystemRandom().nextInt());

            OggVorbisNamespace::ogg_packet header, header_comm, header_code;
            vorbis_analysis_headerout (&vd, &vc, &header, &header_comm, &header_code);

            ogg_stream_packetin (&os, &header);
            ogg_stream_packetin (&os, &header_comm);
            ogg_stream_packetin (&os, &header_code);

            for (;;)
            {
                if (ogg_stream_flush (&os, &og) == 0)
                    break;

                output->write (og.header, (size_t) og.header_len);
                output->write (og.body,   (size_t) og.body_len);
            }

            ok = true;
        }
    }

    ~OggWriter() override
    {
        if (ok)
        {
            // write a zero-length packet to show ogg that we're finished..
            writeSamples (0);

            ogg_stream_clear (&os);
            vorbis_block_clear (&vb);
            vorbis_dsp_clear (&vd);
            vorbis_comment_clear (&vc);

            vorbis_info_clear (&vi);
            output->flush();
        }
        else
        {
            vorbis_info_clear (&vi);
            output = nullptr; // to stop the base class deleting this, as it needs to be returned
                              // to the caller of createWriter()
        }
    }

    //==============================================================================
    b8 write (i32k** samplesToWrite, i32 numSamples) override
    {
        if (ok)
        {
            if (numSamples > 0)
            {
                const f64 gain = 1.0 / 0x80000000u;
                f32** const vorbisBuffer = vorbis_analysis_buffer (&vd, numSamples);

                for (i32 i = (i32) numChannels; --i >= 0;)
                {
                    if (auto* dst = vorbisBuffer[i])
                    {
                        if (i32k* src = samplesToWrite [i])
                        {
                            for (i32 j = 0; j < numSamples; ++j)
                                dst[j] = (f32) (src[j] * gain);
                        }
                    }
                }
            }

            writeSamples (numSamples);
        }

        return ok;
    }

    z0 writeSamples (i32 numSamples)
    {
        vorbis_analysis_wrote (&vd, numSamples);

        while (vorbis_analysis_blockout (&vd, &vb) == 1)
        {
            vorbis_analysis (&vb, nullptr);
            vorbis_bitrate_addblock (&vb);

            while (vorbis_bitrate_flushpacket (&vd, &op))
            {
                ogg_stream_packetin (&os, &op);

                for (;;)
                {
                    if (ogg_stream_pageout (&os, &og) == 0)
                        break;

                    output->write (og.header, (size_t) og.header_len);
                    output->write (og.body,   (size_t) og.body_len);

                    if (ogg_page_eos (&og))
                        break;
                }
            }
        }
    }

    b8 ok = false;

private:
    OggVorbisNamespace::ogg_stream_state os;
    OggVorbisNamespace::ogg_page og;
    OggVorbisNamespace::ogg_packet op;
    OggVorbisNamespace::vorbis_info vi;
    OggVorbisNamespace::vorbis_comment vc;
    OggVorbisNamespace::vorbis_dsp_state vd;
    OggVorbisNamespace::vorbis_block vb;

    z0 addMetadata (const StringPairArray& metadata, tukk name, tukk vorbisName)
    {
        auto s = metadata [name];

        if (s.isNotEmpty())
            vorbis_comment_add_tag (&vc, vorbisName, const_cast<tuk> (s.toRawUTF8()));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggWriter)
};


//==============================================================================
OggVorbisAudioFormat::OggVorbisAudioFormat()  : AudioFormat (oggFormatName, ".ogg")
{
}

OggVorbisAudioFormat::~OggVorbisAudioFormat()
{
}

Array<i32> OggVorbisAudioFormat::getPossibleSampleRates()
{
    return { 8000, 11025, 12000, 16000, 22050, 32000,
             44100, 48000, 88200, 96000, 176400, 192000 };
}

Array<i32> OggVorbisAudioFormat::getPossibleBitDepths()
{
    return { 32 };
}

b8 OggVorbisAudioFormat::canDoStereo()    { return true; }
b8 OggVorbisAudioFormat::canDoMono()      { return true; }
b8 OggVorbisAudioFormat::isCompressed()   { return true; }

AudioFormatReader* OggVorbisAudioFormat::createReaderFor (InputStream* in, b8 deleteStreamIfOpeningFails)
{
    std::unique_ptr<OggReader> r (new OggReader (in));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

AudioFormatWriter* OggVorbisAudioFormat::createWriterFor (OutputStream* out,
                                                          f64 sampleRate,
                                                          u32 numChannels,
                                                          i32 bitsPerSample,
                                                          const StringPairArray& metadataValues,
                                                          i32 qualityOptionIndex)
{
    if (out == nullptr)
        return nullptr;

    std::unique_ptr<OggWriter> w (new OggWriter (out, sampleRate, numChannels,
                                                 (u32) bitsPerSample,
                                                 qualityOptionIndex, metadataValues));

    return w->ok ? w.release() : nullptr;
}

StringArray OggVorbisAudioFormat::getQualityOptions()
{
    return { "64 kbps", "80 kbps", "96 kbps", "112 kbps", "128 kbps", "160 kbps",
             "192 kbps", "224 kbps", "256 kbps", "320 kbps", "500 kbps" };
}

i32 OggVorbisAudioFormat::estimateOggFileQuality (const File& source)
{
    if (auto in = source.createInputStream())
    {
        if (auto r = std::unique_ptr<AudioFormatReader> (createReaderFor (in.release(), true)))
        {
            auto lengthSecs = (f64) r->lengthInSamples / r->sampleRate;
            auto approxBitsPerSecond = (i32) ((f64) source.getSize() * 8 / lengthSecs);

            auto qualities = getQualityOptions();
            i32 bestIndex = 0;
            i32 bestDiff = 10000;

            for (i32 i = qualities.size(); --i >= 0;)
            {
                auto diff = std::abs (qualities[i].getIntValue() - approxBitsPerSecond);

                if (diff < bestDiff)
                {
                    bestDiff = diff;
                    bestIndex = i;
                }
            }

            return bestIndex;
        }
    }

    return 0;
}

#endif

} // namespace drx
