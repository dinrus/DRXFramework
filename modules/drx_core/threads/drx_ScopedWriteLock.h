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
    Automatically locks and unlocks a ReadWriteLock object.

    Use one of these as a local variable to control access to a ReadWriteLock.

    e.g. @code

    ReadWriteLock myLock;

    for (;;)
    {
        const ScopedWriteLock myScopedLock (myLock);
        // myLock is now locked

        ...do some stuff...

        // myLock gets unlocked here.
    }
    @endcode

    @see ReadWriteLock, ScopedReadLock

    @tags{Core}
*/
class DRX_API  ScopedWriteLock
{
public:
    //==============================================================================
    /** Creates a ScopedWriteLock.

        As soon as it is created, this will call ReadWriteLock::enterWrite(), and
        when the ScopedWriteLock object is deleted, the ReadWriteLock will
        be unlocked.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline explicit ScopedWriteLock (const ReadWriteLock& lock) noexcept : lock_ (lock) { lock.enterWrite(); }

    /** Destructor.

        The ReadWriteLock's exitWrite() method will be called when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~ScopedWriteLock() noexcept                                   { lock_.exitWrite(); }


private:
    //==============================================================================
    const ReadWriteLock& lock_;

    DRX_DECLARE_NON_COPYABLE (ScopedWriteLock)
};

//==============================================================================
/**
    Automatically locks and unlocks a ReadWriteLock object.

    Use one of these as a local variable to control access to a ReadWriteLock.

    e.g. @code

    ReadWriteLock myLock;

    for (;;)
    {
        const ScopedTryWriteLock myScopedTryLock (myLock);

        // Unlike using a ScopedWriteLock, this may fail to actually get the lock, so you
        // should test this with the isLocked() method before doing your thread-unsafe
        // action.

        if (myScopedTryLock.isLocked())
        {
            ...do some stuff...
        }
        else
        {
            ..our attempt at locking failed because some other thread has already locked the object..
        }

        // myLock gets unlocked here (if it was locked).
    }
    @endcode

    @see ReadWriteLock, ScopedTryWriteLock

    @tags{Core}
*/
class DRX_API  ScopedTryWriteLock
{
public:
    //==============================================================================
    /** Creates a ScopedTryWriteLock and calls ReadWriteLock::tryEnterWrite() immediately.
        When the ScopedTryWriteLock object is destructed, the ReadWriteLock will be unlocked
        (if it was successfully acquired).

        Make sure this object is created and destructed by the same thread, otherwise there are no
        guarantees what will happen! Best just to use it as a local stack object, rather than creating
        one with the new() operator.
    */
    ScopedTryWriteLock (ReadWriteLock& lockIn) noexcept
            : ScopedTryWriteLock (lockIn, true) {}

    /** Creates a ScopedTryWriteLock.

        If acquireLockOnInitialisation is true then as soon as it is created, this will call
        ReadWriteLock::tryEnterWrite(), and when the ScopedTryWriteLock object is destructed, the
        ReadWriteLock will be unlocked (if it was successfully acquired).

        Make sure this object is created and destructed by the same thread, otherwise there are no
        guarantees what will happen! Best just to use it as a local stack object, rather than creating
        one with the new() operator.
    */
    ScopedTryWriteLock (ReadWriteLock& lockIn, b8 acquireLockOnInitialisation) noexcept
        : lock (lockIn), lockWasSuccessful (acquireLockOnInitialisation && lock.tryEnterWrite()) {}

    /** Destructor.

        The ReadWriteLock's exitWrite() method will be called when the destructor is called.

        Make sure this object is created and destructed by the same thread,
        otherwise there are no guarantees what will happen!
    */
    ~ScopedTryWriteLock() noexcept                  { if (lockWasSuccessful) lock.exitWrite(); }

    /** Возвращает true, если the mutex was successfully locked. */
    b8 isLocked() const noexcept                  { return lockWasSuccessful; }

    /** Retry gaining the lock by calling tryEnter on the underlying lock. */
    b8 retryLock() noexcept                       { return lockWasSuccessful = lock.tryEnterWrite(); }

private:
    //==============================================================================
    ReadWriteLock& lock;
    b8 lockWasSuccessful;

    DRX_DECLARE_NON_COPYABLE (ScopedTryWriteLock)
};

}
