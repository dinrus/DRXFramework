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
    Allows a block of data to be accessed as a stream.

    This can either be used to refer to a shared block of memory, or can make its
    own internal copy of the data when the MemoryInputStream is created.

    @tags{Core}
*/
class DRX_API  MemoryInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a MemoryInputStream.

        @param sourceData               the block of data to use as the stream's source
        @param sourceDataSize           the number of bytes in the source data block
        @param keepInternalCopyOfData   if false, the stream will just keep a pointer to
                                        the source data, so this data shouldn't be changed
                                        for the lifetime of the stream; if this parameter is
                                        true, the stream will make its own copy of the
                                        data and use that.
    */
    MemoryInputStream (ukk sourceData,
                       size_t sourceDataSize,
                       b8 keepInternalCopyOfData);

    /** Creates a MemoryInputStream.

        @param data                     a block of data to use as the stream's source
        @param keepInternalCopyOfData   if false, the stream will just keep a reference to
                                        the source data, so this data shouldn't be changed
                                        for the lifetime of the stream; if this parameter is
                                        true, the stream will make its own copy of the
                                        data and use that.
    */
    MemoryInputStream (const MemoryBlock& data,
                       b8 keepInternalCopyOfData);

    /** Creates a stream by moving from a MemoryBlock. */
    MemoryInputStream (MemoryBlock&& blockToTake);

    /** Destructor. */
    ~MemoryInputStream() override;

    /** Returns a pointer to the source data block from which this stream is reading. */
    ukk getData() const noexcept        { return data; }

    /** Returns the number of bytes of source data in the block from which this stream is reading. */
    size_t getDataSize() const noexcept         { return dataSize; }

    //==============================================================================
    z64 getPosition() override;
    b8 setPosition (z64) override;
    z64 getTotalLength() override;
    b8 isExhausted() override;
    i32 read (uk destBuffer, i32 maxBytesToRead) override;
    z0 skipNextBytes (z64 numBytesToSkip) override;

private:
    //==============================================================================
    ukk data;
    size_t dataSize, position = 0;
    MemoryBlock internalCopy;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryInputStream)
};

} // namespace drx
