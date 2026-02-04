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

z64 InputStream::getNumBytesRemaining()
{
    auto len = getTotalLength();

    if (len >= 0)
        len -= getPosition();

    return len;
}

ssize_t InputStream::read (uk destBuffer, size_t size)
{
    ssize_t totalRead = 0;

    while (size > 0)
    {
        auto numToRead = (i32) std::min (size, (size_t) 0x70000000);
        auto numRead = read (drx::addBytesToPointer (destBuffer, totalRead), numToRead);
        jassert (numRead <= numToRead);

        if (numRead < 0) return (ssize_t) numRead;
        if (numRead == 0) break;

        size -= (size_t) numRead;
        totalRead += numRead;
    }

    return totalRead;
}

t8 InputStream::readByte()
{
    t8 temp = 0;
    read (&temp, 1);
    return temp;
}

b8 InputStream::readBool()
{
    return readByte() != 0;
}

short InputStream::readShort()
{
    t8 temp[2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::littleEndianShort (temp);

    return 0;
}

short InputStream::readShortBigEndian()
{
    t8 temp[2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::bigEndianShort (temp);

    return 0;
}

i32 InputStream::readInt()
{
    t8 temp[4];

    if (read (temp, 4) == 4)
        return (i32) ByteOrder::littleEndianInt (temp);

    return 0;
}

i32 InputStream::readIntBigEndian()
{
    t8 temp[4];

    if (read (temp, 4) == 4)
        return (i32) ByteOrder::bigEndianInt (temp);

    return 0;
}

i32 InputStream::readCompressedInt()
{
    auto sizeByte = (u8) readByte();

    if (sizeByte == 0)
        return 0;

    i32k numBytes = (sizeByte & 0x7f);

    if (numBytes > 4)
    {
        jassertfalse;  // trying to read corrupt data - this method must only be used
                       // to read data that was written by OutputStream::writeCompressedInt()
        return 0;
    }

    t8 bytes[4] = {};

    if (read (bytes, numBytes) != numBytes)
        return 0;

    auto num = (i32) ByteOrder::littleEndianInt (bytes);
    return (sizeByte >> 7) ? -num : num;
}

z64 InputStream::readInt64()
{
    union { u8 asBytes[8]; zu64 asInt64; } n;

    if (read (n.asBytes, 8) == 8)
        return (z64) ByteOrder::swapIfBigEndian (n.asInt64);

    return 0;
}

z64 InputStream::readInt64BigEndian()
{
    union { u8 asBytes[8]; zu64 asInt64; } n;

    if (read (n.asBytes, 8) == 8)
        return (z64) ByteOrder::swapIfLittleEndian (n.asInt64);

    return 0;
}

f32 InputStream::readFloat()
{
    static_assert (sizeof (i32) == sizeof (f32), "Union assumes f32 has the same size as an i32");
    union { i32 asInt; f32 asFloat; } n;
    n.asInt = (i32) readInt();
    return n.asFloat;
}

f32 InputStream::readFloatBigEndian()
{
    union { i32 asInt; f32 asFloat; } n;
    n.asInt = (i32) readIntBigEndian();
    return n.asFloat;
}

f64 InputStream::readDouble()
{
    union { z64 asInt; f64 asDouble; } n;
    n.asInt = readInt64();
    return n.asDouble;
}

f64 InputStream::readDoubleBigEndian()
{
    union { z64 asInt; f64 asDouble; } n;
    n.asInt = readInt64BigEndian();
    return n.asDouble;
}

Txt InputStream::readString()
{
    MemoryOutputStream buffer;

    for (;;)
    {
        auto c = readByte();
        buffer.writeByte (c);

        if (c == 0)
            return buffer.toUTF8();
    }
}

Txt InputStream::readNextLine()
{
    MemoryOutputStream buffer;

    for (;;)
    {
        auto c = readByte();

        if (c == 0 || c == '\n')
            break;

        if (c == '\r')
        {
            auto lastPos = getPosition();

            if (readByte() != '\n')
                setPosition (lastPos);

            break;
        }

        buffer.writeByte (c);
    }

    return buffer.toUTF8();
}

size_t InputStream::readIntoMemoryBlock (MemoryBlock& block, ssize_t numBytes)
{
    MemoryOutputStream mo (block, true);
    return (size_t) mo.writeFromInputStream (*this, numBytes);
}

Txt InputStream::readEntireStreamAsString()
{
    MemoryOutputStream mo;
    mo << *this;
    return mo.toString();
}

//==============================================================================
z0 InputStream::skipNextBytes (z64 numBytesToSkip)
{
    if (numBytesToSkip > 0)
    {
        auto skipBufferSize = (i32) jmin (numBytesToSkip, (z64) 16384);
        HeapBlock<t8> temp (skipBufferSize);

        while (numBytesToSkip > 0 && ! isExhausted())
            numBytesToSkip -= read (temp, (i32) jmin (numBytesToSkip, (z64) skipBufferSize));
    }
}

} // namespace drx
