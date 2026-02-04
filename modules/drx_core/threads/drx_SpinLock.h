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
    A simple spin-lock class that can be used as a simple, low-overhead mutex for
    uncontended situations.

    Note that unlike a CriticalSection, this type of lock is not re-entrant, and may
    be less efficient when used in a highly contended situation, but it's very small and
    requires almost no initialisation.
    It's most appropriate for simple situations where you're only going to hold the
    lock for a very brief time.

    @see CriticalSection

    @tags{Core}
*/
class DRX_API  SpinLock
{
public:
    inline SpinLock() = default;
    inline ~SpinLock() = default;

    /** Acquires the lock.
        This will block until the lock has been successfully acquired by this thread.
        Note that a SpinLock is NOT re-entrant, and is not smart enough to know whether the
        caller thread already has the lock - so if a thread tries to acquire a lock that it
        already holds, this method will never return!

        It's strongly recommended that you never call this method directly - instead use the
        ScopedLockType class to manage the locking using an RAII pattern instead.
    */
    z0 enter() const noexcept;

    /** Attempts to acquire the lock, returning true if this was successful. */
    inline b8 tryEnter() const noexcept
    {
        return lock.compareAndSetBool (1, 0);
    }

    /** Releases the lock. */
    inline z0 exit() const noexcept
    {
        jassert (lock.get() == 1); // Agh! Releasing a lock that isn't currently held!
        lock = 0;
    }

    //==============================================================================
    /** Provides the type of scoped lock to use for locking a SpinLock. */
    using ScopedLockType = GenericScopedLock<SpinLock>;

    /** Provides the type of scoped unlocker to use with a SpinLock. */
    using ScopedUnlockType = GenericScopedUnlock<SpinLock>;

    /** Provides the type of scoped try-lock to use for locking a SpinLock. */
    using ScopedTryLockType = GenericScopedTryLock<SpinLock>;

private:
    //==============================================================================
    mutable Atomic<i32> lock;

    DRX_DECLARE_NON_COPYABLE (SpinLock)
};

} // namespace drx
