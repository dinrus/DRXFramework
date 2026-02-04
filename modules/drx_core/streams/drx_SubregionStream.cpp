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

SubregionStream::SubregionStream (InputStream* sourceStream,
                                  z64 start, z64 length,
                                  b8 deleteSourceWhenDestroyed)
  : source (sourceStream, deleteSourceWhenDestroyed),
    startPositionInSourceStream (start),
    lengthOfSourceStream (length)
{
    SubregionStream::setPosition (0);
}

SubregionStream::~SubregionStream()
{
}

z64 SubregionStream::getTotalLength()
{
    auto srcLen = source->getTotalLength() - startPositionInSourceStream;

    return lengthOfSourceStream >= 0 ? jmin (lengthOfSourceStream, srcLen)
                                     : srcLen;
}

z64 SubregionStream::getPosition()
{
    return source->getPosition() - startPositionInSourceStream;
}

b8 SubregionStream::setPosition (z64 newPosition)
{
    return source->setPosition (jmax ((z64) 0, newPosition + startPositionInSourceStream));
}

i32 SubregionStream::read (uk destBuffer, i32 maxBytesToRead)
{
    jassert (destBuffer != nullptr && maxBytesToRead >= 0);

    if (lengthOfSourceStream < 0)
        return source->read (destBuffer, maxBytesToRead);

    maxBytesToRead = (i32) jmin ((z64) maxBytesToRead, lengthOfSourceStream - getPosition());

    if (maxBytesToRead <= 0)
        return 0;

    return source->read (destBuffer, maxBytesToRead);
}

b8 SubregionStream::isExhausted()
{
    if (lengthOfSourceStream >= 0 && getPosition() >= lengthOfSourceStream)
        return true;

    return source->isExhausted();
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct SubregionInputStreamTests final : public UnitTest
{
    SubregionInputStreamTests()
        : UnitTest ("SubregionInputStream", UnitTestCategories::streams)
    {}

    z0 runTest() override
    {
        const MemoryBlock data ("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", 52);
        MemoryInputStream mi (data, true);

        i32k offset = getRandom().nextInt ((i32) data.getSize());
        const size_t subregionSize = data.getSize() - (size_t) offset;

        SubregionStream stream (&mi, offset, (i32) subregionSize, false);

        beginTest ("Read");

        expectEquals (stream.getPosition(), (z64) 0);
        expectEquals (stream.getTotalLength(), (z64) subregionSize);
        expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
        expect (! stream.isExhausted());

        size_t numBytesRead = 0;
        MemoryBlock readBuffer (subregionSize);

        while (numBytesRead < subregionSize)
        {
            numBytesRead += (size_t) stream.read (&readBuffer[numBytesRead], 3);

            expectEquals (stream.getPosition(), (z64) numBytesRead);
            expectEquals (stream.getNumBytesRemaining(), (z64) (subregionSize - numBytesRead));
            expect (stream.isExhausted() == (numBytesRead == subregionSize));
        }

        expectEquals (stream.getPosition(), (z64) subregionSize);
        expectEquals (stream.getNumBytesRemaining(), (z64) 0);
        expect (stream.isExhausted());

        const MemoryBlock memoryBlockToCheck (data.begin() + (size_t) offset, data.getSize() - (size_t) offset);
        expect (readBuffer == memoryBlockToCheck);

        beginTest ("Skip");

        stream.setPosition (0);
        expectEquals (stream.getPosition(), (z64) 0);
        expectEquals (stream.getTotalLength(), (z64) subregionSize);
        expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
        expect (! stream.isExhausted());

        numBytesRead = 0;
        const z64 numBytesToSkip = 5;

        while (numBytesRead < subregionSize)
        {
            stream.skipNextBytes (numBytesToSkip);
            numBytesRead += numBytesToSkip;
            numBytesRead = std::min (numBytesRead, subregionSize);

            expectEquals (stream.getPosition(), (z64) numBytesRead);
            expectEquals (stream.getNumBytesRemaining(), (z64) (subregionSize - numBytesRead));
            expect (stream.isExhausted() == (numBytesRead == subregionSize));
        }

        expectEquals (stream.getPosition(), (z64) subregionSize);
        expectEquals (stream.getNumBytesRemaining(), (z64) 0);
        expect (stream.isExhausted());
    }
};

static SubregionInputStreamTests subregionInputStreamTests;

#endif

} // namespace drx
