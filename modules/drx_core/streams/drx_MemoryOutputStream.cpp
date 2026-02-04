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

MemoryOutputStream::MemoryOutputStream (const size_t initialSize)
  : blockToUse (&internalBlock)
{
    internalBlock.setSize (initialSize, false);
}

MemoryOutputStream::MemoryOutputStream (MemoryBlock& memoryBlockToWriteTo,
                                        const b8 appendToExistingBlockContent)
  : blockToUse (&memoryBlockToWriteTo)
{
    if (appendToExistingBlockContent)
        position = size = memoryBlockToWriteTo.getSize();
}

MemoryOutputStream::MemoryOutputStream (uk destBuffer, size_t destBufferSize)
  : externalData (destBuffer), availableSize (destBufferSize)
{
    jassert (externalData != nullptr); // This must be a valid pointer.
}

MemoryOutputStream::~MemoryOutputStream()
{
    trimExternalBlockSize();
}

z0 MemoryOutputStream::flush()
{
    trimExternalBlockSize();
}

z0 MemoryOutputStream::trimExternalBlockSize()
{
    if (blockToUse != &internalBlock && blockToUse != nullptr)
        blockToUse->setSize (size, false);
}

z0 MemoryOutputStream::preallocate (const size_t bytesToPreallocate)
{
    if (blockToUse != nullptr)
        blockToUse->ensureSize (bytesToPreallocate + 1);
}

z0 MemoryOutputStream::reset() noexcept
{
    position = 0;
    size = 0;
}

tuk MemoryOutputStream::prepareToWrite (size_t numBytes)
{
    jassert ((ssize_t) numBytes >= 0);
    auto storageNeeded = position + numBytes;

    tuk data;

    if (blockToUse != nullptr)
    {
        if (storageNeeded >= blockToUse->getSize())
            blockToUse->ensureSize ((storageNeeded + jmin (storageNeeded / 2, (size_t) (1024 * 1024)) + 32) & ~31u);

        data = static_cast<tuk> (blockToUse->getData());
    }
    else
    {
        if (storageNeeded > availableSize)
            return nullptr;

        data = static_cast<tuk> (externalData);
    }

    auto* writePointer = data + position;
    position += numBytes;
    size = jmax (size, position);
    return writePointer;
}

b8 MemoryOutputStream::write (ukk const buffer, size_t howMany)
{
    if (howMany == 0)
        return true;

    jassert (buffer != nullptr);

    if (auto* dest = prepareToWrite (howMany))
    {
        memcpy (dest, buffer, howMany);
        return true;
    }

    return false;
}

b8 MemoryOutputStream::writeRepeatedByte (u8 byte, size_t howMany)
{
    if (howMany == 0)
        return true;

    if (auto* dest = prepareToWrite (howMany))
    {
        memset (dest, byte, howMany);
        return true;
    }

    return false;
}

b8 MemoryOutputStream::appendUTF8Char (t32 c)
{
    if (auto* dest = prepareToWrite (CharPointer_UTF8::getBytesRequiredFor (c)))
    {
        CharPointer_UTF8 (dest).write (c);
        return true;
    }

    return false;
}

MemoryBlock MemoryOutputStream::getMemoryBlock() const
{
    return MemoryBlock (getData(), getDataSize());
}

ukk MemoryOutputStream::getData() const noexcept
{
    if (blockToUse == nullptr)
        return externalData;

    if (blockToUse->getSize() > size)
        static_cast<tuk> (blockToUse->getData()) [size] = 0;

    return blockToUse->getData();
}

b8 MemoryOutputStream::setPosition (z64 newPosition)
{
    if (newPosition <= (z64) size)
    {
        // ok to seek backwards
        position = jlimit ((size_t) 0, size, (size_t) newPosition);
        return true;
    }

    // can't move beyond the end of the stream..
    return false;
}

z64 MemoryOutputStream::writeFromInputStream (InputStream& source, z64 maxNumBytesToWrite)
{
    // before writing from an input, see if we can preallocate to make it more efficient..
    const auto availableData = source.getTotalLength() - source.getPosition();

    if (availableData > 0)
    {
        if (maxNumBytesToWrite < 0 || availableData < maxNumBytesToWrite)
            maxNumBytesToWrite = availableData;

        if (blockToUse != nullptr)
            preallocate (position + (size_t) maxNumBytesToWrite);
    }

    return OutputStream::writeFromInputStream (source, maxNumBytesToWrite);
}

Txt MemoryOutputStream::toUTF8() const
{
    auto* d = static_cast<tukk> (getData());
    return Txt (CharPointer_UTF8 (d), CharPointer_UTF8 (d + getDataSize()));
}

Txt MemoryOutputStream::toString() const
{
    return Txt::createStringFromData (getData(), (i32) getDataSize());
}

OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const MemoryOutputStream& streamToRead)
{
    auto dataSize = streamToRead.getDataSize();

    if (dataSize > 0)
        stream.write (streamToRead.getData(), dataSize);

    return stream;
}

} // namespace drx
