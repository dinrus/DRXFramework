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
#include <drx_core/drx_core.h>
#include <drx_core/zip/drx_zlib.h>

namespace drx
{

class GZIPCompressorOutputStream::GZIPCompressorHelper
{
public:
    GZIPCompressorHelper (i32 compressionLevel, i32 windowBits)
        : compLevel ((compressionLevel < 0 || compressionLevel > 9) ? -1 : compressionLevel)
    {
        using namespace zlibNamespace;
        zerostruct (stream);

        streamIsValid = (deflateInit2 (&stream, compLevel, Z_DEFLATED,
                                       windowBits != 0 ? windowBits : MAX_WBITS,
                                       8, strategy) == Z_OK);
    }

    ~GZIPCompressorHelper()
    {
        if (streamIsValid)
            deflateEnd (&stream);
    }

    b8 write (u8k* data, size_t dataSize, OutputStream& out)
    {
        // When you call flush() on a gzip stream, the stream is closed, and you can
        // no longer continue to write data to it!
        jassert (! finished);

        while (dataSize > 0)
            if (! doNextBlock (data, dataSize, out, Z_NO_FLUSH))
                return false;

        return true;
    }

    z0 finish (OutputStream& out)
    {
        u8k* data = nullptr;
        size_t dataSize = 0;

        while (! finished)
            doNextBlock (data, dataSize, out, Z_FINISH);
    }

private:
    enum { strategy = 0 };

    zlibNamespace::z_stream stream;
    i32k compLevel;
    b8 isFirstDeflate = true, streamIsValid = false, finished = false;
    zlibNamespace::Bytef buffer[32768];

    b8 doNextBlock (u8k*& data, size_t& dataSize, OutputStream& out, i32k flushMode)
    {
        using namespace zlibNamespace;

        if (streamIsValid)
        {
            stream.next_in   = const_cast<u8*> (data);
            stream.next_out  = buffer;
            stream.avail_in  = (uInt) dataSize;
            stream.avail_out = (uInt) sizeof (buffer);

            auto result = isFirstDeflate ? deflateParams (&stream, compLevel, strategy)
                                         : deflate (&stream, flushMode);
            isFirstDeflate = false;

            switch (result)
            {
                case Z_STREAM_END:
                    finished = true;
                    DRX_FALLTHROUGH
                case Z_OK:
                {
                    data += dataSize - stream.avail_in;
                    dataSize = stream.avail_in;
                    auto bytesDone = (ssize_t) sizeof (buffer) - (ssize_t) stream.avail_out;
                    return bytesDone <= 0 || out.write (buffer, (size_t) bytesDone);
                }

                default:
                    break;
            }
        }

        return false;
    }

    DRX_DECLARE_NON_COPYABLE (GZIPCompressorHelper)
};

//==============================================================================
GZIPCompressorOutputStream::GZIPCompressorOutputStream (OutputStream& s, i32 compressionLevel, i32 windowBits)
   : GZIPCompressorOutputStream (&s, compressionLevel, false, windowBits)
{
}

GZIPCompressorOutputStream::GZIPCompressorOutputStream (OutputStream* out, i32 compressionLevel, b8 deleteDestStream, i32 windowBits)
   : destStream (out, deleteDestStream),
     helper (new GZIPCompressorHelper (compressionLevel, windowBits))
{
    jassert (out != nullptr);
}

GZIPCompressorOutputStream::~GZIPCompressorOutputStream()
{
    flush();
}

z0 GZIPCompressorOutputStream::flush()
{
    helper->finish (*destStream);
    destStream->flush();
}

b8 GZIPCompressorOutputStream::write (ukk destBuffer, size_t howMany)
{
    jassert (destBuffer != nullptr && (ssize_t) howMany >= 0);

    return helper->write (static_cast<u8k*> (destBuffer), howMany, *destStream);
}

z64 GZIPCompressorOutputStream::getPosition()
{
    return destStream->getPosition();
}

b8 GZIPCompressorOutputStream::setPosition (z64 /*newPosition*/)
{
    jassertfalse; // can't do it!
    return false;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct GZIPTests final : public UnitTest
{
    GZIPTests()
        : UnitTest ("GZIP", UnitTestCategories::compression)
    {}

    z0 runTest() override
    {
        beginTest ("GZIP");
        Random rng = getRandom();

        for (i32 i = 100; --i >= 0;)
        {
            MemoryOutputStream original, compressed, uncompressed;

            {
                GZIPCompressorOutputStream zipper (compressed, rng.nextInt (10));

                for (i32 j = rng.nextInt (100); --j >= 0;)
                {
                    MemoryBlock data ((u32) (rng.nextInt (2000) + 1));

                    for (i32 k = (i32) data.getSize(); --k >= 0;)
                        data[k] = (t8) rng.nextInt (255);

                    original << data;
                    zipper   << data;
                }
            }

            {
                MemoryInputStream compressedInput (compressed.getData(), compressed.getDataSize(), false);
                GZIPDecompressorInputStream unzipper (compressedInput);

                uncompressed << unzipper;
            }

            expectEquals ((i32) uncompressed.getDataSize(),
                          (i32) original.getDataSize());

            if (original.getDataSize() == uncompressed.getDataSize())
                expect (memcmp (uncompressed.getData(),
                                original.getData(),
                                original.getDataSize()) == 0);
        }
    }
};

static GZIPTests gzipTests;

#endif

} // namespace drx
