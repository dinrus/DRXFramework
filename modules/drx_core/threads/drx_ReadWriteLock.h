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
    A critical section that allows multiple simultaneous readers.

    Features of this type of lock are:

    - Multiple readers can hold the lock at the same time, but only one writer
      can hold it at once.
    - Writers trying to gain the lock will be blocked until all readers and writers
      have released it
    - Readers trying to gain the lock while a writer is waiting to acquire it will be
      blocked until the writer has obtained and released it
    - If a thread already has a read lock and tries to obtain a write lock, it will succeed if
      there are no other readers
    - If a thread already has the write lock and tries to obtain a read lock, this will succeed.
    - Recursive locking is supported.

    @see ScopedReadLock, ScopedWriteLock, CriticalSection

    @tags{Core}
*/
class DRX_API  ReadWriteLock
{
public:
    //==============================================================================
    /**
        Creates a ReadWriteLock object.
    */
    ReadWriteLock() noexcept;

    /** Destructor.
        If the object is deleted whilst locked, any subsequent behaviour is undefined.
    */
    ~ReadWriteLock() noexcept;

    //==============================================================================
    /** Locks this object for reading.

        Multiple threads can simultaneously lock the object for reading, but if another
        thread has it locked for writing, then this will block until it releases the lock.

        @see exitRead, ScopedReadLock
    */
    z0 enterRead() const noexcept;

    /** Tries to lock this object for reading.

        Multiple threads can simultaneously lock the object for reading, but if another
        thread has it locked for writing, then this will fail and return false.

        @returns true if the lock is successfully gained.
        @see exitRead, ScopedTryReadLock
    */
    b8 tryEnterRead() const noexcept;

    /** Releases the read-lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enterRead() method has been called multiple times by the thread, each
        call must be matched by a call to exitRead() before other threads will be allowed
        to take over the lock.

        @see enterRead, ScopedReadLock
    */
    z0 exitRead() const noexcept;

    //==============================================================================
    /** Locks this object for writing.

        This will block until any other threads that have it locked for reading or
        writing have released their lock.

        @see exitWrite, ScopedWriteLock
    */
    z0 enterWrite() const noexcept;

    /** Tries to lock this object for writing.

        This is like enterWrite(), but doesn't block - it returns true if it manages
        to obtain the lock.

        @returns true if the lock is successfully gained.
        @see enterWrite, ScopedTryWriteLock
    */
    b8 tryEnterWrite() const noexcept;

    /** Releases the write-lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enterWrite() method has been called multiple times by the thread, each
        call must be matched by a call to exit() before other threads will be allowed
        to take over the lock.

        @see enterWrite, ScopedWriteLock
    */
    z0 exitWrite() const noexcept;


private:
    //==============================================================================
    SpinLock accessLock;
    WaitableEvent readWaitEvent, writeWaitEvent;
    mutable i32 numWaitingWriters = 0, numWriters = 0;
    mutable Thread::ThreadID writerThreadId = {};

    struct ThreadRecursionCount
    {
        Thread::ThreadID threadID;
        i32 count;
    };

    mutable Array <ThreadRecursionCount> readerThreads;

    b8 tryEnterWriteInternal (Thread::ThreadID) const noexcept;

    DRX_DECLARE_NON_COPYABLE (ReadWriteLock)
};

} // namespace drx
