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
    A bit like an AsyncUpdater, but guarantees that after cancelPendingUpdate() returns,
    the async function will never be called until triggerAsyncUpdate() is called again.
    This is an important guarantee for writing classes with async behaviour that can
    still be destroyed safely from a background thread.

    Note that all of the member functions of this type have a chance of blocking, so
    this class is unsuitable for broadcasting changes from a realtime thread.

    @tags{Events}
*/
class DRX_API  LockingAsyncUpdater final
{
public:
    //==============================================================================
    /** Creates a LockingAsyncUpdater object that will call the provided callback
        on the main thread when triggered.

        Note that the LockingAsyncUpdater takes an internal mutex before calling
        the provided callback. Therefore, in order to avoid deadlocks, you should
        (ideally) ensure that no locks are taken inside the callbackToUse. If you
        do need to take a lock inside the callback, make sure that you do not
        hold the same lock while calling any of the LockingAsyncUpdater member
        functions.
    */
    explicit LockingAsyncUpdater (std::function<z0()> callbackToUse);

    /** Move constructor. */
    LockingAsyncUpdater (LockingAsyncUpdater&& other) noexcept;

    /** Move assignment operator. */
    LockingAsyncUpdater& operator= (LockingAsyncUpdater&& other) noexcept;

    /** Destructor.
        If there are any pending callbacks when the object is deleted, these are lost.
        The async callback is guaranteed not to be called again once the destructor has
        completed.
    */
    ~LockingAsyncUpdater();

    //==============================================================================
    /** Causes the callback to be triggered at a later time.

        This method returns quickly, after which a callback to the
        handleAsyncUpdate() method will be made by the impl thread as
        soon as possible.

        If an update callback is already pending but hasn't started yet, calling
        this method will have no effect.

        It's thread-safe to call this method from any thread, BUT beware of calling
        it from a real-time (e.g. audio) thread, because it unconditionally locks
        a mutex. This may block, e.g. if this is called from a background thread
        while the async callback is in progress on the main thread.
    */
    z0 triggerAsyncUpdate();

    /** This will stop any pending updates from happening.

        If a callback is already in progress on another thread, this will block until
        the callback has finished before returning.
    */
    z0 cancelPendingUpdate() noexcept;

    /** If an update has been triggered and is pending, this will invoke it
        synchronously.

        Use this as a kind of "flush" operation - if an update is pending, the
        handleAsyncUpdate() method will be called immediately; if no update is
        pending, then nothing will be done.

        Because this may invoke the callback, this method must only be called on
        the main event thread.
    */
    z0 handleUpdateNowIfNeeded();

    /** Возвращает true, если there's an update callback in the pipeline. */
    b8 isUpdatePending() const noexcept;

private:
    class Impl;
    ReferenceCountedObjectPtr<Impl> impl;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LockingAsyncUpdater)
};

} // namespace drx
