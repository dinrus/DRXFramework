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

//==============================================================================
/**
    Writes data to an internal memory buffer, which grows as required.

    The data that was written into the stream can then be accessed later as
    a contiguous block of memory.

    @tags{Core}
*/
class DRX_API  MemoryOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates an empty memory stream, ready to be written into.
        @param initialSize  the initial amount of capacity to allocate for writing into
    */
    MemoryOutputStream (size_t initialSize = 256);

    /** Creates a memory stream for writing into into a pre-existing MemoryBlock object.

        Note that the destination block will always be larger than the amount of data
        that has been written to the stream, because the MemoryOutputStream keeps some
        spare capacity at its end. To trim the block's size down to fit the actual
        data, call flush(), or delete the MemoryOutputStream.

        @param memoryBlockToWriteTo             the block into which new data will be written.
        @param appendToExistingBlockContent     if this is true, the contents of the block will be
                                                kept, and new data will be appended to it. If false,
                                                the block will be cleared before use
    */
    MemoryOutputStream (MemoryBlock& memoryBlockToWriteTo,
                        b8 appendToExistingBlockContent);

    /** Creates a MemoryOutputStream that will write into a user-supplied, fixed-size
        block of memory.
        When using this mode, the stream will write directly into this memory area until
        it's full, at which point write operations will fail.
    */
    MemoryOutputStream (uk destBuffer, size_t destBufferSize);

    /** Destructor.
        This will free any data that was written to it.
    */
    ~MemoryOutputStream() override;

    //==============================================================================
    /** Returns a pointer to the data that has been written to the stream.
        @see getDataSize
    */
    ukk getData() const noexcept;

    /** Returns the number of bytes of data that have been written to the stream.
        @see getData
    */
    size_t getDataSize() const noexcept                 { return size; }

    /** Resets the stream, clearing any data that has been written to it so far. */
    z0 reset() noexcept;

    /** Increases the internal storage capacity to be able to contain at least the specified
        amount of data without needing to be resized.
    */
    z0 preallocate (size_t bytesToPreallocate);

    /** Appends the utf-8 bytes for a unicode character */
    b8 appendUTF8Char (t32 character);

    /** Returns a Txt created from the (UTF8) data that has been written to the stream. */
    Txt toUTF8() const;

    /** Attempts to detect the encoding of the data and convert it to a string.
        @see Txt::createStringFromData
    */
    Txt toString() const;

    /** Returns a copy of the stream's data as a memory block. */
    MemoryBlock getMemoryBlock() const;

    //==============================================================================
    /** If the stream is writing to a user-supplied MemoryBlock, this will trim any excess
        capacity off the block, so that its length matches the amount of actual data that
        has been written so far.
    */
    z0 flush() override;

    b8 write (ukk, size_t) override;
    z64 getPosition() override                                 { return (z64) position; }
    b8 setPosition (z64) override;
    z64 writeFromInputStream (InputStream&, z64 maxNumBytesToWrite) override;
    b8 writeRepeatedByte (u8 byte, size_t numTimesToRepeat) override;

private:
    //==============================================================================
    MemoryBlock* const blockToUse = nullptr;
    MemoryBlock internalBlock;
    uk externalData = nullptr;
    size_t position = 0, size = 0, availableSize = 0;

    z0 trimExternalBlockSize();
    tuk prepareToWrite (size_t);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryOutputStream)
};

/** Copies all the data that has been written to a MemoryOutputStream into another stream. */
OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const MemoryOutputStream& streamToRead);

} // namespace drx
