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
    Мютекст повторного входа (реэнтранный).

    CriticalSection действует как объект реэнтранного мютекса (заглушки). Лучшим
    способом его блокировки/разблокировки является использование RAII в форме
    локального объекта ScopedLock - в базе кода есть множества примеров тому,
    как это делается.

    Почти во всех случаях нужно объявить CriticalSection как переменную-член.
    Иногда нужно объявлять его как статическую переменную, но при этом
    могут появиться предупреждения о порядке построения статического объекта C++.

    @see ScopedLock, ScopedTryLock, ScopedUnlock, SpinLock, ReadWriteLock, Thread, InterProcessLock

    @tags{Core}
*/
class DRX_API  CriticalSection
{
public:
    //==============================================================================
    /** Создаёт объект CriticalSection. */
    CriticalSection() noexcept;

    /** Деструктор.  Когда критическая секция удаляется, будучи заблокированной,
    последующее поведение становится непредсказуемым.
    */
    ~CriticalSection() noexcept;

    //==============================================================================
    /** Приобретает замок.

        If the lock is already held by the caller thread, the method returns immediately.
        If the lock is currently held by another thread, this will wait until it becomes free.

        It's strongly recommended that you never call this method directly - instead use the
        ScopedLock class to manage the locking using an RAII pattern instead.

        @see exit, tryEnter, ScopedLock
    */
    z0 enter() const noexcept;

    /** Пытается запереть данную критическую секцию без блокировки.

        This method behaves identically to CriticalSection::enter, except that the caller thread
        does not wait if the lock is currently held by another thread but returns false immediately.

        @returns false if the lock is currently held by another thread, true otherwise.
        @see enter
    */
    b8 tryEnter() const noexcept;

    /** Снимает замок.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enter() method has been called multiple times by the thread, each
        call must be matched by a call to exit() before other threads will be allowed
        to take over the lock.

        @see enter, ScopedLock
    */
    z0 exit() const noexcept;


    //==============================================================================
    /** Provides the type of scoped lock to use with a CriticalSection. */
    using ScopedLockType = GenericScopedLock<CriticalSection>;

    /** Provides the type of scoped unlocker to use with a CriticalSection. */
    using ScopedUnlockType = GenericScopedUnlock<CriticalSection>;

    /** Provides the type of scoped try-locker to use with a CriticalSection. */
    using ScopedTryLockType = GenericScopedTryLock<CriticalSection>;


private:
    //==============================================================================
   #if DRX_WINDOWS
    // To avoid including windows.h in the public DRX headers, we'll just allocate
    // a block of memory here that's big enough to be used internally as a windows
    // CRITICAL_SECTION structure.
    #if DRX_64BIT
     alignas (8) std::byte lock[44];
    #else
     alignas (8) std::byte lock[24];
    #endif
   #else
    mutable pthread_mutex_t lock;
   #endif

    DRX_DECLARE_NON_COPYABLE (CriticalSection)
};


//==============================================================================
/**
    A class that can be used in place of a real CriticalSection object, but which
    doesn't perform any locking.

    This is currently used by some templated classes, and most compilers should
    manage to optimise it out of existence.

    @see CriticalSection, Array, OwnedArray, ReferenceCountedArray

    @tags{Core}
*/
class DRX_API  DummyCriticalSection
{
public:
    inline DummyCriticalSection() = default;
    inline ~DummyCriticalSection() = default;

    inline z0 enter() const noexcept          {}
    inline b8 tryEnter() const noexcept       { return true; }
    inline z0 exit() const noexcept           {}

    //==============================================================================
    /** A dummy scoped-lock type to use with a dummy critical section. */
    struct ScopedLockType
    {
        ScopedLockType (const DummyCriticalSection&) noexcept {}
    };

    /** A dummy scoped-unlocker type to use with a dummy critical section. */
    using ScopedUnlockType = ScopedLockType;

private:
    DRX_DECLARE_NON_COPYABLE (DummyCriticalSection)
};

//==============================================================================
/**
    Automatically locks and unlocks a CriticalSection object.

    You can use a ScopedLock as a local variable to provide RAII-based locking of a CriticalSection.

    e.g. @code

    struct MyObject
    {
        CriticalSection objectLock;

        // assuming that this example function will be called by multiple threads
        z0 foo()
        {
            const ScopedLock myScopedLock (objectLock);

            // objectLock is now locked..

            ...do some thread-safe work here...

            // ..and objectLock gets unlocked here, as myScopedLock goes out of
            // scope at the end of the block
        }
    };
    @endcode

    @see CriticalSection, ScopedUnlock
*/
using ScopedLock = CriticalSection::ScopedLockType;

//==============================================================================
/**
    Automatically unlocks and re-locks a CriticalSection object.

    This is the reverse of a ScopedLock object - instead of locking the critical
    section for the lifetime of this object, it unlocks it.

    Make sure you don't try to unlock critical sections that aren't actually locked!

    e.g. @code

    struct MyObject
    {
        CriticalSection objectLock;

        z0 foo()
        {
            {
                const ScopedLock myScopedLock (objectLock);

                // objectLock is now locked..

                {
                    ScopedUnlock myUnlocker (objectLock);

                    // ..and now unlocked..
                }

                // ..and now locked again..
            }

            // ..and finally unlocked.
        }
    };
    @endcode

    @see CriticalSection, ScopedLock
*/
using ScopedUnlock = CriticalSection::ScopedUnlockType;

//==============================================================================
/**
    Automatically tries to lock and unlock a CriticalSection object.

    Use one of these as a local variable to control access to a CriticalSection.

    e.g. @code

    struct MyObject
    {
        CriticalSection objectLock;

        z0 foo()
        {
            const ScopedTryLock myScopedTryLock (objectLock);

            // Unlike using a ScopedLock, this may fail to actually get the lock, so you
            // must call the isLocked() method before making any assumptions..
            if (myScopedTryLock.isLocked())
            {
               ...safely do some work...
            }
            else
            {
                // If we get here, then our attempt at locking failed because another thread had already locked it..
            }
        }
    };
    @endcode

    @see CriticalSection::tryEnter, ScopedLock, ScopedUnlock, ScopedReadLock
*/
using ScopedTryLock = CriticalSection::ScopedTryLockType;

} // namespace drx
