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

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4127 4244 4309 4305 4365 6385 6326 6340)

namespace zlibNamespace
{
 #if DRX_INCLUDE_ZLIB_CODE
  DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                       "-Wsign-conversion",
                                       "-Wshadow",
                                       "-Wdeprecated-register",
                                       "-Wswitch-enum",
                                       "-Wswitch-default",
                                       "-Wredundant-decls",
                                       "-Wimplicit-fallthrough",
                                       "-Wzero-as-null-pointer-constant",
                                       "-Wcomma",
                                       "-Wcast-align",
                                       "-Wkeyword-macro",
                                       "-Wmissing-prototypes")

  #pragma push_macro ("register")
  #define register

  #pragma push_macro ("MIN")
  #undef MIN

  #pragma push_macro ("read")
  #pragma push_macro ("write")
  #pragma push_macro ("open")
  #pragma push_macro ("close")

  #undef OS_CODE
  #undef fdopen
  #define ZLIB_INTERNAL
  #define NO_DUMMY_DECL
  #include <drx_core/zip/zlib/adler32.c>
  #include <drx_core/zip/zlib/compress.c>
  #undef DO1
  #undef DO8
  #include <drx_core/zip/zlib/crc32.c>
  #include <drx_core/zip/zlib/deflate.c>
  #include <drx_core/zip/zlib/inffast.c>
  #undef PULLBYTE
  #undef LOAD
  #undef RESTORE
  #undef INITBITS
  #undef NEEDBITS
  #undef DROPBITS
  #undef BYTEBITS
  #undef GZIP
  #include <drx_core/zip/zlib/inflate.c>
  #include <drx_core/zip/zlib/inftrees.c>
  #include <drx_core/zip/zlib/trees.c>
  #include <drx_core/zip/zlib/zutil.c>
  #undef Byte
  #undef fdopen
  #undef local
  #undef Freq
  #undef Code
  #undef Dad
  #undef Len

  #pragma pop_macro ("close")
  #pragma pop_macro ("open")
  #pragma pop_macro ("write")
  #pragma pop_macro ("read")
  #pragma pop_macro ("MIN")
  #pragma pop_macro ("register")

  DRX_END_IGNORE_WARNINGS_GCC_LIKE
 #else
  #include DRX_ZLIB_INCLUDE_PATH
 #endif

#ifndef z_uInt
 #ifdef uInt
  #define z_uInt uInt
 #else
  #define z_uInt u32
 #endif
#endif

}

DRX_END_IGNORE_WARNINGS_MSVC

//==============================================================================
// internal helper object that holds the zlib structures so they don't have to be
// included publicly.
class GZIPDecompressorInputStream::GZIPDecompressHelper
{
public:
    GZIPDecompressHelper (Format f)
    {
        using namespace zlibNamespace;
        zerostruct (stream);
        streamIsValid = (inflateInit2 (&stream, getBitsForFormat (f)) == Z_OK);
        finished = error = ! streamIsValid;
    }

    ~GZIPDecompressHelper()
    {
        if (streamIsValid)
            zlibNamespace::inflateEnd (&stream);
    }

    b8 needsInput() const noexcept        { return dataSize <= 0; }

    z0 setInput (u8* const data_, const size_t size) noexcept
    {
        data = data_;
        dataSize = size;
    }

    i32 doNextBlock (u8* const dest, u32k destSize)
    {
        using namespace zlibNamespace;

        if (streamIsValid && data != nullptr && ! finished)
        {
            stream.next_in  = data;
            stream.next_out = dest;
            stream.avail_in  = (z_uInt) dataSize;
            stream.avail_out = (z_uInt) destSize;

            switch (inflate (&stream, Z_PARTIAL_FLUSH))
            {
            case Z_STREAM_END:
                finished = true;
                DRX_FALLTHROUGH
            case Z_OK:
                data += dataSize - stream.avail_in;
                dataSize = (z_uInt) stream.avail_in;
                return (i32) (destSize - stream.avail_out);

            case Z_NEED_DICT:
                needsDictionary = true;
                data += dataSize - stream.avail_in;
                dataSize = (size_t) stream.avail_in;
                break;

            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                error = true;
                DRX_FALLTHROUGH
            default:
                break;
            }
        }

        return 0;
    }

    static i32 getBitsForFormat (Format f) noexcept
    {
        switch (f)
        {
            case zlibFormat:     return  MAX_WBITS;
            case deflateFormat:  return -MAX_WBITS;
            case gzipFormat:     return  MAX_WBITS | 16;
            default:             jassertfalse; break;
        }

        return MAX_WBITS;
    }

    b8 finished = true, needsDictionary = false, error = true, streamIsValid = false;

    enum { gzipDecompBufferSize = 32768 };

private:
    zlibNamespace::z_stream stream;
    u8* data = nullptr;
    size_t dataSize = 0;

    DRX_DECLARE_NON_COPYABLE (GZIPDecompressHelper)
};

//==============================================================================
GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream* source, b8 deleteSourceWhenDestroyed,
                                                          Format f, z64 uncompressedLength)
  : sourceStream (source, deleteSourceWhenDestroyed),
    uncompressedStreamLength (uncompressedLength),
    format (f),
    originalSourcePos (source->getPosition()),
    buffer ((size_t) GZIPDecompressHelper::gzipDecompBufferSize),
    helper (new GZIPDecompressHelper (f))
{
}

GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream& source)
  : sourceStream (&source, false),
    uncompressedStreamLength (-1),
    format (zlibFormat),
    originalSourcePos (source.getPosition()),
    buffer ((size_t) GZIPDecompressHelper::gzipDecompBufferSize),
    helper (new GZIPDecompressHelper (zlibFormat))
{
}

GZIPDecompressorInputStream::~GZIPDecompressorInputStream()
{
}

z64 GZIPDecompressorInputStream::getTotalLength()
{
    return uncompressedStreamLength;
}

i32 GZIPDecompressorInputStream::read (uk destBuffer, i32 howMany)
{
    jassert (destBuffer != nullptr && howMany >= 0);

    if (howMany > 0 && ! isEof)
    {
        i32 numRead = 0;
        auto d = static_cast<u8*> (destBuffer);

        while (! helper->error)
        {
            auto n = helper->doNextBlock (d, (u32) howMany);
            currentPos += n;

            if (n == 0)
            {
                if (helper->finished || helper->needsDictionary)
                {
                    isEof = true;
                    return numRead;
                }

                if (helper->needsInput())
                {
                    activeBufferSize = sourceStream->read (buffer, (i32) GZIPDecompressHelper::gzipDecompBufferSize);

                    if (activeBufferSize > 0)
                    {
                        helper->setInput (buffer, (size_t) activeBufferSize);
                    }
                    else
                    {
                        isEof = true;
                        return numRead;
                    }
                }
            }
            else
            {
                numRead += n;
                howMany -= n;
                d += n;

                if (howMany <= 0)
                    return numRead;
            }
        }
    }

    return 0;
}

b8 GZIPDecompressorInputStream::isExhausted()
{
    return helper->error || helper->finished || isEof;
}

z64 GZIPDecompressorInputStream::getPosition()
{
    return currentPos;
}

b8 GZIPDecompressorInputStream::setPosition (z64 newPos)
{
    if (newPos < currentPos)
    {
        // to go backwards, reset the stream and start again..
        isEof = false;
        activeBufferSize = 0;
        currentPos = 0;
        helper.reset (new GZIPDecompressHelper (format));

        sourceStream->setPosition (originalSourcePos);
    }

    skipNextBytes (newPos - currentPos);
    return true;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct GZIPDecompressorInputStreamTests final : public UnitTest
{
    GZIPDecompressorInputStreamTests()
        : UnitTest ("GZIPDecompressorInputStreamTests", UnitTestCategories::streams)
    {}

    z0 runTest() override
    {
        const MemoryBlock data ("abcdefghijklmnopqrstuvwxyz", 26);

        MemoryOutputStream mo;
        GZIPCompressorOutputStream gzipOutputStream (mo);
        gzipOutputStream.write (data.getData(), data.getSize());
        gzipOutputStream.flush();

        MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
        GZIPDecompressorInputStream stream (&mi, false, GZIPDecompressorInputStream::zlibFormat, (z64) data.getSize());

        beginTest ("Read");

        expectEquals (stream.getPosition(), (z64) 0);
        expectEquals (stream.getTotalLength(), (z64) data.getSize());
        expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
        expect (! stream.isExhausted());

        size_t numBytesRead = 0;
        MemoryBlock readBuffer (data.getSize());

        while (numBytesRead < data.getSize())
        {
            numBytesRead += (size_t) stream.read (&readBuffer[numBytesRead], 3);

            expectEquals (stream.getPosition(), (z64) numBytesRead);
            expectEquals (stream.getNumBytesRemaining(), (z64) (data.getSize() - numBytesRead));
            expect (stream.isExhausted() == (numBytesRead == data.getSize()));
        }

        expectEquals (stream.getPosition(), (z64) data.getSize());
        expectEquals (stream.getNumBytesRemaining(), (z64) 0);
        expect (stream.isExhausted());

        expect (readBuffer == data);

        beginTest ("Skip");

        stream.setPosition (0);
        expectEquals (stream.getPosition(), (z64) 0);
        expectEquals (stream.getTotalLength(), (z64) data.getSize());
        expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
        expect (! stream.isExhausted());

        numBytesRead = 0;
        i32k numBytesToSkip = 5;

        while (numBytesRead < data.getSize())
        {
            stream.skipNextBytes (numBytesToSkip);
            numBytesRead += numBytesToSkip;
            numBytesRead = std::min (numBytesRead, data.getSize());

            expectEquals (stream.getPosition(), (z64) numBytesRead);
            expectEquals (stream.getNumBytesRemaining(), (z64) (data.getSize() - numBytesRead));
            expect (stream.isExhausted() == (numBytesRead == data.getSize()));
        }

        expectEquals (stream.getPosition(), (z64) data.getSize());
        expectEquals (stream.getNumBytesRemaining(), (z64) 0);
        expect (stream.isExhausted());
    }
};

static GZIPDecompressorInputStreamTests gzipDecompressorInputStreamTests;

#endif

} // namespace drx
