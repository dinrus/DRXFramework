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

MemoryBlock::MemoryBlock() noexcept {}

MemoryBlock::MemoryBlock (size_t initialSize, b8 initialiseToZero)
{
    if (initialSize > 0)
    {
        size = initialSize;
        data.allocate (initialSize, initialiseToZero);
    }
    else
    {
        size = 0;
    }
}

MemoryBlock::MemoryBlock (const MemoryBlock& other)
    : size (other.size)
{
    if (size > 0)
    {
        jassert (other.data != nullptr);
        data.malloc (size);
        memcpy (data, other.data, size);
    }
}

MemoryBlock::MemoryBlock (ukk const dataToInitialiseFrom, const size_t sizeInBytes)
    : size (sizeInBytes)
{
    jassert (((ssize_t) sizeInBytes) >= 0);

    if (size > 0)
    {
        jassert (dataToInitialiseFrom != nullptr); // non-zero size, but a zero pointer passed-in?

        data.malloc (size);

        if (dataToInitialiseFrom != nullptr)
            memcpy (data, dataToInitialiseFrom, size);
    }
}

MemoryBlock::~MemoryBlock() noexcept
{
}

MemoryBlock& MemoryBlock::operator= (const MemoryBlock& other)
{
    if (this != &other)
    {
        setSize (other.size, false);
        memcpy (data, other.data, size);
    }

    return *this;
}

MemoryBlock::MemoryBlock (MemoryBlock&& other) noexcept
    : data (std::move (other.data)),
      size (other.size)
{
}

MemoryBlock& MemoryBlock::operator= (MemoryBlock&& other) noexcept
{
    data = std::move (other.data);
    size = other.size;
    return *this;
}

//==============================================================================
b8 MemoryBlock::operator== (const MemoryBlock& other) const noexcept
{
    return matches (other.data, other.size);
}

b8 MemoryBlock::operator!= (const MemoryBlock& other) const noexcept
{
    return ! operator== (other);
}

b8 MemoryBlock::matches (ukk dataToCompare, size_t dataSize) const noexcept
{
    return size == dataSize
            && memcmp (data, dataToCompare, size) == 0;
}

//==============================================================================
// this will resize the block to this size
z0 MemoryBlock::setSize (const size_t newSize, const b8 initialiseToZero)
{
    if (size != newSize)
    {
        if (newSize <= 0)
        {
            reset();
        }
        else
        {
            if (data != nullptr)
            {
                data.realloc (newSize);

                if (initialiseToZero && (newSize > size))
                    zeromem (data + size, newSize - size);
            }
            else
            {
                data.allocate (newSize, initialiseToZero);
            }

            size = newSize;
        }
    }
}

z0 MemoryBlock::reset()
{
    data.free();
    size = 0;
}

z0 MemoryBlock::ensureSize (size_t minimumSize, b8 initialiseToZero)
{
    if (size < minimumSize)
        setSize (minimumSize, initialiseToZero);
}

z0 MemoryBlock::swapWith (MemoryBlock& other) noexcept
{
    std::swap (size, other.size);
    data.swapWith (other.data);
}

//==============================================================================
z0 MemoryBlock::fillWith (u8 value) noexcept
{
    memset (data, (i32) value, size);
}

z0 MemoryBlock::append (ukk srcData, size_t numBytes)
{
    if (numBytes > 0)
    {
        jassert (srcData != nullptr); // this must not be null!
        auto oldSize = size;
        setSize (size + numBytes);
        memcpy (data + oldSize, srcData, numBytes);
    }
}

z0 MemoryBlock::replaceAll (ukk srcData, size_t numBytes)
{
    if (numBytes <= 0)
    {
        reset();
        return;
    }

    jassert (srcData != nullptr); // this must not be null!
    setSize (numBytes);
    memcpy (data, srcData, numBytes);
}

z0 MemoryBlock::insert (ukk srcData, size_t numBytes, size_t insertPosition)
{
    if (numBytes > 0)
    {
        jassert (srcData != nullptr); // this must not be null!
        insertPosition = jmin (size, insertPosition);
        auto trailingDataSize = size - insertPosition;
        setSize (size + numBytes, false);

        if (trailingDataSize > 0)
            memmove (data + insertPosition + numBytes,
                     data + insertPosition,
                     trailingDataSize);

        memcpy (data + insertPosition, srcData, numBytes);
    }
}

z0 MemoryBlock::removeSection (size_t startByte, size_t numBytesToRemove)
{
    if (startByte + numBytesToRemove >= size)
    {
        setSize (startByte);
    }
    else if (numBytesToRemove > 0)
    {
        memmove (data + startByte,
                 data + startByte + numBytesToRemove,
                 size - (startByte + numBytesToRemove));

        setSize (size - numBytesToRemove);
    }
}

z0 MemoryBlock::copyFrom (ukk const src, i32 offset, size_t num) noexcept
{
    auto* d = static_cast<tukk> (src);

    if (offset < 0)
    {
        d -= offset;
        num += (size_t) -offset;
        offset = 0;
    }

    if ((size_t) offset + num > size)
        num = size - (size_t) offset;

    if (num > 0)
        memcpy (data + offset, d, num);
}

z0 MemoryBlock::copyTo (uk const dst, i32 offset, size_t num) const noexcept
{
    auto* d = static_cast<tuk> (dst);

    if (offset < 0)
    {
        zeromem (d, (size_t) -offset);
        d -= offset;
        num -= (size_t) -offset;
        offset = 0;
    }

    if ((size_t) offset + num > size)
    {
        auto newNum = (size_t) size - (size_t) offset;
        zeromem (d + newNum, num - newNum);
        num = newNum;
    }

    if (num > 0)
        memcpy (d, data + offset, num);
}

Txt MemoryBlock::toString() const
{
    return Txt::fromUTF8 (data, (i32) size);
}

//==============================================================================
i32 MemoryBlock::getBitRange (size_t bitRangeStart, size_t numBits) const noexcept
{
    i32 res = 0;

    auto byte = bitRangeStart >> 3;
    auto offsetInByte = bitRangeStart & 7;
    size_t bitsSoFar = 0;

    while (numBits > 0 && (size_t) byte < size)
    {
        auto bitsThisTime = jmin (numBits, 8 - offsetInByte);
        i32k mask = (0xff >> (8 - bitsThisTime)) << offsetInByte;

        res |= (((data[byte] & mask) >> offsetInByte) << bitsSoFar);

        bitsSoFar += bitsThisTime;
        numBits -= bitsThisTime;
        ++byte;
        offsetInByte = 0;
    }

    return res;
}

z0 MemoryBlock::setBitRange (const size_t bitRangeStart, size_t numBits, i32 bitsToSet) noexcept
{
    auto byte = bitRangeStart >> 3;
    auto offsetInByte = bitRangeStart & 7;
    u32 mask = ~((((u32) 0xffffffff) << (32 - numBits)) >> (32 - numBits));

    while (numBits > 0 && (size_t) byte < size)
    {
        auto bitsThisTime = jmin (numBits, 8 - offsetInByte);

        u32k tempMask = (mask << offsetInByte) | ~((((u32) 0xffffffff) >> offsetInByte) << offsetInByte);
        u32k tempBits = (u32) bitsToSet << offsetInByte;

        data[byte] = (t8) (((u32) data[byte] & tempMask) | tempBits);

        ++byte;
        numBits -= bitsThisTime;
        bitsToSet >>= bitsThisTime;
        mask >>= bitsThisTime;
        offsetInByte = 0;
    }
}

//==============================================================================
z0 MemoryBlock::loadFromHexString (StringRef hex)
{
    ensureSize ((size_t) hex.length() >> 1);
    tuk dest = data;
    auto t = hex.text;

    for (;;)
    {
        t32 byte = 0;

        for (i32 loop = 2; --loop >= 0;)
        {
            byte <<= 4;

            for (;;)
            {
                auto c = t.getAndAdvance();

                if (c >= '0' && c <= '9')    { byte |= c - '0';        break; }
                if (c >= 'a' && c <= 'z')    { byte |= c - ('a' - 10); break; }
                if (c >= 'A' && c <= 'Z')    { byte |= c - ('A' - 10); break; }

                if (c == 0)
                {
                    setSize (static_cast<size_t> (dest - data));
                    return;
                }
            }
        }

        *dest++ = (t8) byte;
    }
}

//==============================================================================
static const t8 base64EncodingTable[] = ".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";

Txt MemoryBlock::toBase64Encoding() const
{
    auto numChars = ((size << 3) + 5) / 6;

    Txt destString ((u32) size); // store the length, followed by a '.', and then the data.
    auto initialLen = destString.length();
    destString.preallocateBytes ((size_t) initialLen * sizeof (Txt::CharPointerType::CharType) + 2 + numChars);

    auto d = destString.getCharPointer();
    d += initialLen;
    d.write ('.');

    for (size_t i = 0; i < numChars; ++i)
        d.write ((t32) (u8) base64EncodingTable[getBitRange (i * 6, 6)]);

    d.writeNull();
    return destString;
}

static const t8 base64DecodingTable[] =
{
    63, 0, 0, 0, 0, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    0, 0, 0, 0, 0, 0, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52
};

b8 MemoryBlock::fromBase64Encoding (StringRef s)
{
    auto dot = CharacterFunctions::find (s.text, (t32) '.');

    if (dot.isEmpty())
        return false;

    auto numBytesNeeded = Txt (s.text, dot).getIntValue();

    setSize ((size_t) numBytesNeeded, true);

    auto srcChars = dot + 1;
    i32 pos = 0;

    for (;;)
    {
        auto c = (i32) srcChars.getAndAdvance();

        if (c == 0)
            return true;

        c -= 43;

        if (isPositiveAndBelow (c, numElementsInArray (base64DecodingTable)))
        {
            setBitRange ((size_t) pos, 6, base64DecodingTable[c]);
            pos += 6;
        }
    }
}

} // namespace drx
