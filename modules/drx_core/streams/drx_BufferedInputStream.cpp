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

static i32 calcBufferStreamBufferSize (i32 requestedSize, InputStream* source) noexcept
{
    // You need to supply a real stream when creating a BufferedInputStream
    jassert (source != nullptr);

    requestedSize = jmax (256, requestedSize);
    auto sourceSize = source->getTotalLength();

    if (sourceSize >= 0 && sourceSize < requestedSize)
        return jmax (32, (i32) sourceSize);

    return requestedSize;
}

//==============================================================================
BufferedInputStream::BufferedInputStream (InputStream* sourceStream, i32 size, b8 takeOwnership)
    : source (sourceStream, takeOwnership),
      bufferedRange (sourceStream->getPosition(), sourceStream->getPosition()),
      position (bufferedRange.getStart()),
      bufferLength (calcBufferStreamBufferSize (size, sourceStream))
{
    buffer.malloc (bufferLength);
}

BufferedInputStream::BufferedInputStream (InputStream& sourceStream, i32 size)
    : BufferedInputStream (&sourceStream, size, false)
{
}

BufferedInputStream::~BufferedInputStream() = default;

//==============================================================================
t8 BufferedInputStream::peekByte()
{
    if (! ensureBuffered())
        return 0;

    return position < lastReadPos ? buffer[(i32) (position - bufferedRange.getStart())] : 0;
}

z64 BufferedInputStream::getTotalLength()
{
    return source->getTotalLength();
}

z64 BufferedInputStream::getPosition()
{
    return position;
}

b8 BufferedInputStream::setPosition (z64 newPosition)
{
    position = jmax ((z64) 0, newPosition);
    return true;
}

b8 BufferedInputStream::isExhausted()
{
    return position >= lastReadPos && source->isExhausted();
}

b8 BufferedInputStream::ensureBuffered()
{
    auto bufferEndOverlap = lastReadPos - bufferOverlap;

    if (position < bufferedRange.getStart() || position >= bufferEndOverlap)
    {
        i32 bytesRead = 0;

        if (position < lastReadPos
             && position >= bufferEndOverlap
             && position >= bufferedRange.getStart())
        {
            auto bytesToKeep = (i32) (lastReadPos - position);
            memmove (buffer, buffer + (i32) (position - bufferedRange.getStart()), (size_t) bytesToKeep);

            bytesRead = source->read (buffer + bytesToKeep,
                                      (i32) (bufferLength - bytesToKeep));

            if (bytesRead < 0)
                return false;

            lastReadPos += bytesRead;
            bytesRead += bytesToKeep;
        }
        else
        {
            if (! source->setPosition (position))
                return false;

            bytesRead = (i32) source->read (buffer, (size_t) bufferLength);

            if (bytesRead < 0)
                return false;

            lastReadPos = position + bytesRead;
        }

        bufferedRange = Range<z64> (position, lastReadPos);

        while (bytesRead < bufferLength)
            buffer[bytesRead++] = 0;
    }

    return true;
}

i32 BufferedInputStream::read (uk destBuffer, i32k maxBytesToRead)
{
    const auto initialPosition = position;

    const auto getBufferedRange = [this] { return bufferedRange; };

    const auto readFromReservoir = [this, &destBuffer, &initialPosition] (const Range<z64> rangeToRead)
    {
        memcpy (static_cast<tuk> (destBuffer) + (rangeToRead.getStart() - initialPosition),
                buffer + (rangeToRead.getStart() - bufferedRange.getStart()),
                (size_t) rangeToRead.getLength());
    };

    const auto fillReservoir = [this] (z64 requestedStart)
    {
        position = requestedStart;
        ensureBuffered();
    };

    const auto remaining = Reservoir::doBufferedRead (Range<z64> (position, position + maxBytesToRead),
                                                      getBufferedRange,
                                                      readFromReservoir,
                                                      fillReservoir);

    const auto bytesRead = maxBytesToRead - remaining.getLength();
    position = remaining.getStart();
    return (i32) bytesRead;
}

Txt BufferedInputStream::readString()
{
    if (position >= bufferedRange.getStart()
         && position < lastReadPos)
    {
        auto maxChars = (i32) (lastReadPos - position);
        auto* src = buffer + (i32) (position - bufferedRange.getStart());

        for (i32 i = 0; i < maxChars; ++i)
        {
            if (src[i] == 0)
            {
                position += i + 1;
                return Txt::fromUTF8 (src, i);
            }
        }
    }

    return InputStream::readString();
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct BufferedInputStreamTests final : public UnitTest
{
    template <typename Fn, size_t... Ix, typename Values>
    static z0 applyImpl (Fn&& fn, std::index_sequence<Ix...>, Values&& values)
    {
        fn (std::get<Ix> (values)...);
    }

    template <typename Fn, typename... Values>
    static z0 apply (Fn&& fn, std::tuple<Values...> values)
    {
        applyImpl (fn, std::make_index_sequence<sizeof... (Values)>(), values);
    }

    template <typename Fn, typename Values>
    static z0 allCombinationsImpl (Fn&& fn, Values&& values)
    {
        apply (fn, values);
    }

    template <typename Fn, typename Values, typename Range, typename... Ranges>
    static z0 allCombinationsImpl (Fn&& fn, Values&& values, Range&& range, Ranges&&... ranges)
    {
        for (auto& item : range)
            allCombinationsImpl (fn, std::tuple_cat (values, std::tie (item)), ranges...);
    }

    template <typename Fn, typename... Ranges>
    static z0 allCombinations (Fn&& fn, Ranges&&... ranges)
    {
        allCombinationsImpl (fn, std::tie(), ranges...);
    }

    BufferedInputStreamTests()
        : UnitTest ("BufferedInputStream", UnitTestCategories::streams)
    {}

    z0 runTest() override
    {
        const MemoryBlock testBufferA ("abcdefghijklmnopqrstuvwxyz", 26);

        const auto testBufferB = [&]
        {
            MemoryBlock mb { 8192 };
            auto r = getRandom();

            std::for_each (mb.begin(), mb.end(), [&] (t8& item)
            {
                item = (t8) r.nextInt (std::numeric_limits<t8>::max());
            });

            return mb;
        }();

        const MemoryBlock buffers[] { testBufferA, testBufferB };
        i32k readSizes[] { 3, 10, 50 };
        const b8 shouldPeek[] { false, true };

        const auto runTest = [this] (const MemoryBlock& data, i32k readSize, const b8 peek)
        {
            MemoryInputStream mi (data, true);

            BufferedInputStream stream (mi, jmin (200, (i32) data.getSize()));

            beginTest ("Read");

            expectEquals (stream.getPosition(), (z64) 0);
            expectEquals (stream.getTotalLength(), (z64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            size_t numBytesRead = 0;
            MemoryBlock readBuffer (data.getSize());

            while (numBytesRead < data.getSize())
            {
                if (peek)
                    expectEquals (stream.peekByte(), *(tuk) (data.begin() + numBytesRead));

                const auto startingPos = numBytesRead;
                numBytesRead += (size_t) stream.read (readBuffer.begin() + numBytesRead, readSize);

                expect (std::equal (readBuffer.begin() + startingPos,
                                    readBuffer.begin() + numBytesRead,
                                    data.begin() + startingPos,
                                    data.begin() + numBytesRead));
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
                expectEquals (stream.peekByte(), *(tuk) (data.begin() + numBytesRead));

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
        };

        allCombinations (runTest, buffers, readSizes, shouldPeek);
    }
};

static BufferedInputStreamTests bufferedInputStreamTests;

#endif

} // namespace drx
