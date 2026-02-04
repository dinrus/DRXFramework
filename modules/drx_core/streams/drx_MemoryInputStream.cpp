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

MemoryInputStream::MemoryInputStream (ukk sourceData, size_t sourceDataSize, b8 keepCopy)
    : data (sourceData),
      dataSize (sourceDataSize)
{
    if (keepCopy)
    {
        internalCopy = MemoryBlock (sourceData, sourceDataSize);
        data = internalCopy.getData();
    }
}

MemoryInputStream::MemoryInputStream (const MemoryBlock& sourceData, b8 keepCopy)
    : data (sourceData.getData()),
      dataSize (sourceData.getSize())
{
    if (keepCopy)
    {
        internalCopy = sourceData;
        data = internalCopy.getData();
    }
}

MemoryInputStream::MemoryInputStream (MemoryBlock&& source)
    : internalCopy (std::move (source))
{
    data = internalCopy.getData();
    dataSize = internalCopy.getSize();
}

MemoryInputStream::~MemoryInputStream() = default;

z64 MemoryInputStream::getTotalLength()
{
    return (z64) dataSize;
}

i32 MemoryInputStream::read (uk buffer, i32 howMany)
{
    jassert (buffer != nullptr && howMany >= 0);

    if (howMany <= 0 || position >= dataSize)
        return 0;

    auto num = jmin ((size_t) howMany, dataSize - position);

    if (num > 0)
    {
        memcpy (buffer, addBytesToPointer (data, position), num);
        position += num;
    }

    return (i32) num;
}

b8 MemoryInputStream::isExhausted()
{
    return position >= dataSize;
}

b8 MemoryInputStream::setPosition (const z64 pos)
{
    position = (size_t) jlimit ((z64) 0, (z64) dataSize, pos);
    return true;
}

z64 MemoryInputStream::getPosition()
{
    return (z64) position;
}

z0 MemoryInputStream::skipNextBytes (z64 numBytesToSkip)
{
    if (numBytesToSkip > 0)
        setPosition (getPosition() + numBytesToSkip);
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class MemoryStreamTests final : public UnitTest
{
public:
    MemoryStreamTests()
        : UnitTest ("MemoryInputStream & MemoryOutputStream", UnitTestCategories::streams)
    {}

    z0 runTest() override
    {
        beginTest ("Basics");
        Random r = getRandom();

        i32 randomInt = r.nextInt();
        z64 randomInt64 = r.nextInt64();
        f64 randomDouble = r.nextDouble();
        Txt randomString (createRandomWideCharString (r));

        MemoryOutputStream mo;
        mo.writeInt (randomInt);
        mo.writeIntBigEndian (randomInt);
        mo.writeCompressedInt (randomInt);
        mo.writeString (randomString);
        mo.writeInt64 (randomInt64);
        mo.writeInt64BigEndian (randomInt64);
        mo.writeDouble (randomDouble);
        mo.writeDoubleBigEndian (randomDouble);

        MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
        expect (mi.readInt() == randomInt);
        expect (mi.readIntBigEndian() == randomInt);
        expect (mi.readCompressedInt() == randomInt);
        expectEquals (mi.readString(), randomString);
        expect (mi.readInt64() == randomInt64);
        expect (mi.readInt64BigEndian() == randomInt64);
        expectEquals (mi.readDouble(), randomDouble);
        expectEquals (mi.readDoubleBigEndian(), randomDouble);

        const MemoryBlock data ("abcdefghijklmnopqrstuvwxyz", 26);
        MemoryInputStream stream (data, true);

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

    static Txt createRandomWideCharString (Random& r)
    {
        t32 buffer [50] = { 0 };

        for (i32 i = 0; i < numElementsInArray (buffer) - 1; ++i)
        {
            if (r.nextBool())
            {
                do
                {
                    buffer[i] = (t32) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (t32) (1 + r.nextInt (0xff));
        }

        return CharPointer_UTF32 (buffer);
    }
};

static MemoryStreamTests memoryInputStreamUnitTests;

#endif

} // namespace drx
