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

ReadWriteLock::ReadWriteLock() noexcept
{
    readerThreads.ensureStorageAllocated (16);
}

ReadWriteLock::~ReadWriteLock() noexcept
{
    jassert (readerThreads.size() == 0);
    jassert (numWriters == 0);
}

//==============================================================================
z0 ReadWriteLock::enterRead() const noexcept
{
    while (! tryEnterRead())
        readWaitEvent.wait (100);
}

b8 ReadWriteLock::tryEnterRead() const noexcept
{
    auto threadId = Thread::getCurrentThreadId();

    const SpinLock::ScopedLockType sl (accessLock);

    for (auto& readerThread : readerThreads)
    {
        if (readerThread.threadID == threadId)
        {
            readerThread.count++;
            return true;
        }
    }

    if (numWriters + numWaitingWriters == 0
         || (threadId == writerThreadId && numWriters > 0))
    {
        readerThreads.add ({ threadId, 1 });
        return true;
    }

    return false;
}

z0 ReadWriteLock::exitRead() const noexcept
{
    auto threadId = Thread::getCurrentThreadId();
    const SpinLock::ScopedLockType sl (accessLock);

    for (i32 i = 0; i < readerThreads.size(); ++i)
    {
        auto& readerThread = readerThreads.getReference (i);

        if (readerThread.threadID == threadId)
        {
            if (--(readerThread.count) == 0)
            {
                readerThreads.remove (i);

                readWaitEvent.signal();
                writeWaitEvent.signal();
            }

            return;
        }
    }

    jassertfalse; // unlocking a lock that wasn't locked..
}

//==============================================================================
z0 ReadWriteLock::enterWrite() const noexcept
{
    auto threadId = Thread::getCurrentThreadId();
    const SpinLock::ScopedLockType sl (accessLock);

    while (! tryEnterWriteInternal (threadId))
    {
        ++numWaitingWriters;
        accessLock.exit();
        writeWaitEvent.wait (100);
        accessLock.enter();
        --numWaitingWriters;
    }
}

b8 ReadWriteLock::tryEnterWrite() const noexcept
{
    const SpinLock::ScopedLockType sl (accessLock);
    return tryEnterWriteInternal (Thread::getCurrentThreadId());
}

b8 ReadWriteLock::tryEnterWriteInternal (Thread::ThreadID threadId) const noexcept
{
    if (readerThreads.size() + numWriters == 0
         || threadId == writerThreadId
         || (readerThreads.size() == 1 && readerThreads.getReference (0).threadID == threadId))
    {
        writerThreadId = threadId;
        ++numWriters;
        return true;
    }

    return false;
}

z0 ReadWriteLock::exitWrite() const noexcept
{
    const SpinLock::ScopedLockType sl (accessLock);

    // check this thread actually had the lock..
    jassert (numWriters > 0 && writerThreadId == Thread::getCurrentThreadId());

    if (--numWriters == 0)
    {
        writerThreadId = {};

        readWaitEvent.signal();
        writeWaitEvent.signal();
    }
}

} // namespace drx
