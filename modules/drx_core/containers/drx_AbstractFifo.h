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
    Encapsulates the logic required to implement a lock-free FIFO.

    This class handles the logic needed when building a single-reader, single-writer FIFO.

    It doesn't actually hold any data itself, but your FIFO class can use one of these to manage
    its position and status when reading or writing to it.

    To use it, you can call prepareToWrite() to determine the position within your own buffer that
    an incoming block of data should be stored, and prepareToRead() to find out when the next
    outgoing block should be read from.

    e.g.
    @code
    struct MyFifo
    {
        z0 addToFifo (i32k* someData, i32 numItems)
        {
            const auto scope = abstractFifo.write (numItems);

            if (scope.blockSize1 > 0)
                copySomeData (myBuffer + scope.startIndex1, someData, scope.blockSize1);

            if (scope.blockSize2 > 0)
                copySomeData (myBuffer + scope.startIndex2, someData + scope.blockSize1, scope.blockSize2);
        }

        z0 readFromFifo (i32* someData, i32 numItems)
        {
            const auto scope = abstractFifo.read (numItems);

            if (scope.blockSize1 > 0)
                copySomeData (someData, myBuffer + scope.startIndex1, scope.blockSize1);

            if (scope.blockSize2 > 0)
                copySomeData (someData + scope.blockSize1, myBuffer + scope.startIndex2, scope.blockSize2);
        }

        AbstractFifo abstractFifo { 1024 };
        i32 myBuffer[1024];
    };
    @endcode

    @tags{Core}
*/
class DRX_API  AbstractFifo
{
public:
    //==============================================================================
    /** Creates a FIFO to manage a buffer with the specified capacity. */
    AbstractFifo (i32 capacity) noexcept;

    //==============================================================================
    /** Returns the total size of the buffer being managed. */
    i32 getTotalSize() const noexcept;

    /** Returns the number of items that can currently be added to the buffer without it overflowing. */
    i32 getFreeSpace() const noexcept;

    /** Returns the number of items that can currently be read from the buffer. */
    i32 getNumReady() const noexcept;

    /** Clears the buffer positions, so that it appears empty. */
    z0 reset() noexcept;

    /** Changes the buffer's total size.
        Note that this isn't thread-safe, so don't call it if there's any danger that it
        might overlap with a call to any other method in this class!
    */
    z0 setTotalSize (i32 newSize) noexcept;

    //==============================================================================
    /** Returns the location within the buffer at which an incoming block of data should be written.

        Because the section of data that you want to add to the buffer may overlap the end
        and wrap around to the start, two blocks within your buffer are returned, and you
        should copy your data into the first one, with any remaining data spilling over into
        the second.

        If the number of items you ask for is too large to fit within the buffer's free space, then
        blockSize1 + blockSize2 may add up to a lower value than numToWrite. If this happens, you
        may decide to keep waiting and re-trying the method until there's enough space available.

        After calling this method, if you choose to write your data into the blocks returned, you
        must call finishedWrite() to tell the FIFO how much data you actually added.

        e.g.
        @code
        z0 addToFifo (i32k* someData, i32 numItems)
        {
            i32 start1, size1, start2, size2;
            prepareToWrite (numItems, start1, size1, start2, size2);

            if (size1 > 0)
                copySomeData (myBuffer + start1, someData, size1);

            if (size2 > 0)
                copySomeData (myBuffer + start2, someData + size1, size2);

            finishedWrite (size1 + size2);
        }
        @endcode

        @param numToWrite       indicates how many items you'd like to add to the buffer
        @param startIndex1      on exit, this will contain the start index in your buffer at which your data should be written
        @param blockSize1       on exit, this indicates how many items can be written to the block starting at startIndex1
        @param startIndex2      on exit, this will contain the start index in your buffer at which any data that didn't fit into
                                the first block should be written
        @param blockSize2       on exit, this indicates how many items can be written to the block starting at startIndex2
        @see finishedWrite
    */
    z0 prepareToWrite (i32 numToWrite, i32& startIndex1, i32& blockSize1, i32& startIndex2, i32& blockSize2) const noexcept;

    /** Called after writing from the FIFO, to indicate that this many items have been added.
        @see prepareToWrite
    */
    z0 finishedWrite (i32 numWritten) noexcept;

    /** Returns the location within the buffer from which the next block of data should be read.

        Because the section of data that you want to read from the buffer may overlap the end
        and wrap around to the start, two blocks within your buffer are returned, and you
        should read from both of them.

        If the number of items you ask for is greater than the amount of data available, then
        blockSize1 + blockSize2 may add up to a lower value than numWanted. If this happens, you
        may decide to keep waiting and re-trying the method until there's enough data available.

        After calling this method, if you choose to read the data, you must call finishedRead() to
        tell the FIFO how much data you have consumed.

        e.g.
        @code
        z0 readFromFifo (i32* someData, i32 numItems)
        {
            i32 start1, size1, start2, size2;
            prepareToRead (numSamples, start1, size1, start2, size2);

            if (size1 > 0)
                copySomeData (someData, myBuffer + start1, size1);

            if (size2 > 0)
                copySomeData (someData + size1, myBuffer + start2, size2);

            finishedRead (size1 + size2);
        }
        @endcode

        @param numWanted        indicates how many items you'd like to read from the buffer
        @param startIndex1      on exit, this will contain the start index in your buffer at which your data should be written
        @param blockSize1       on exit, this indicates how many items can be written to the block starting at startIndex1
        @param startIndex2      on exit, this will contain the start index in your buffer at which any data that didn't fit into
                                the first block should be written
        @param blockSize2       on exit, this indicates how many items can be written to the block starting at startIndex2
        @see finishedRead
    */
    z0 prepareToRead (i32 numWanted, i32& startIndex1, i32& blockSize1, i32& startIndex2, i32& blockSize2) const noexcept;

    /** Called after reading from the FIFO, to indicate that this many items have now been consumed.
        @see prepareToRead
    */
    z0 finishedRead (i32 numRead) noexcept;

    //==============================================================================

private:
    enum class ReadOrWrite
    {
        read,
        write
    };

public:
    /** Class for a scoped reader/writer */
    template <ReadOrWrite mode>
    class ScopedReadWrite final
    {
    public:
        /** Construct an unassigned reader/writer. Doesn't do anything upon destruction. */
        ScopedReadWrite() = default;

        /** Construct a reader/writer and immediately call prepareRead/prepareWrite
            on the abstractFifo which was passed in.
            This object will hold a pointer back to the fifo, so make sure that
            the fifo outlives this object.
        */
        ScopedReadWrite (AbstractFifo& f, i32 num) noexcept  : fifo (&f)
        {
            prepare (*fifo, num);
        }

        ScopedReadWrite (const ScopedReadWrite&) = delete;
        ScopedReadWrite (ScopedReadWrite&&) noexcept;

        ScopedReadWrite& operator= (const ScopedReadWrite&) = delete;
        ScopedReadWrite& operator= (ScopedReadWrite&&) noexcept;

        /** Calls finishedRead or finishedWrite if this is a non-null scoped
            reader/writer.
        */
        ~ScopedReadWrite() noexcept
        {
            if (fifo != nullptr)
                finish (*fifo, blockSize1 + blockSize2);
        }

        /** Calls the passed function with each index that was deemed valid
            for the current read/write operation.
        */
        template <typename FunctionToApply>
        z0 forEach (FunctionToApply&& func) const
        {
            for (auto i = startIndex1, e = startIndex1 + blockSize1; i != e; ++i)  func (i);
            for (auto i = startIndex2, e = startIndex2 + blockSize2; i != e; ++i)  func (i);
        }

        i32 startIndex1, blockSize1, startIndex2, blockSize2;

    private:
        z0 prepare (AbstractFifo&, i32) noexcept;
        static z0 finish (AbstractFifo&, i32) noexcept;
        z0 swap (ScopedReadWrite&) noexcept;

        AbstractFifo* fifo = nullptr;
    };

    using ScopedRead  = ScopedReadWrite<ReadOrWrite::read>;
    using ScopedWrite = ScopedReadWrite<ReadOrWrite::write>;

    /** Replaces prepareToRead/finishedRead with a single function.
        This function returns an object which contains the start indices and
        block sizes, and also automatically finishes the read operation when
        it goes out of scope.
        @code
        {
            auto readHandle = fifo.read (4);

            for (auto i = 0; i != readHandle.blockSize1; ++i)
            {
                // read the item at index readHandle.startIndex1 + i
            }

            for (auto i = 0; i != readHandle.blockSize2; ++i)
            {
                // read the item at index readHandle.startIndex2 + i
            }
        } // readHandle goes out of scope here, finishing the read operation
        @endcode
    */
    ScopedRead read (i32 numToRead) noexcept;

    /** Replaces prepareToWrite/finishedWrite with a single function.
        This function returns an object which contains the start indices and
        block sizes, and also automatically finishes the write operation when
        it goes out of scope.
        @code
        {
            auto writeHandle = fifo.write (5);

            for (auto i = 0; i != writeHandle.blockSize1; ++i)
            {
                // write the item at index writeHandle.startIndex1 + i
            }

            for (auto i = 0; i != writeHandle.blockSize2; ++i)
            {
                // write the item at index writeHandle.startIndex2 + i
            }
        } // writeHandle goes out of scope here, finishing the write operation
        @endcode
    */
    ScopedWrite write (i32 numToWrite) noexcept;

private:
    //==============================================================================
    i32 bufferSize;
    Atomic<i32> validStart, validEnd;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AbstractFifo)
};

template <>
inline z0 AbstractFifo::ScopedReadWrite<AbstractFifo::ReadOrWrite::read>::finish (AbstractFifo& f, i32 num) noexcept
{
    f.finishedRead (num);
}

template <>
inline z0 AbstractFifo::ScopedReadWrite<AbstractFifo::ReadOrWrite::write>::finish (AbstractFifo& f, i32 num) noexcept
{
    f.finishedWrite (num);
}

template <>
inline z0 AbstractFifo::ScopedReadWrite<AbstractFifo::ReadOrWrite::read>::prepare (AbstractFifo& f, i32 num) noexcept
{
    f.prepareToRead (num, startIndex1, blockSize1, startIndex2, blockSize2);
}

template <>
inline z0 AbstractFifo::ScopedReadWrite<AbstractFifo::ReadOrWrite::write>::prepare (AbstractFifo& f, i32 num) noexcept
{
    f.prepareToWrite (num, startIndex1, blockSize1, startIndex2, blockSize2);
}


} // namespace drx
