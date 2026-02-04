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
    Has a callback method that is triggered asynchronously.

    This object allows an asynchronous callback function to be triggered, for
    tasks such as coalescing multiple updates into a single callback later on.

    Basically, one or more calls to the triggerAsyncUpdate() will result in the
    message thread calling handleAsyncUpdate() as soon as it can.

    @tags{Events}
*/
class DRX_API  AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an AsyncUpdater object. */
    AsyncUpdater();

    /** Destructor.
        If there are any pending callbacks when the object is deleted, these are lost.
    */
    virtual ~AsyncUpdater();

    //==============================================================================
    /** Causes the callback to be triggered at a later time.

        This method returns immediately, after which a callback to the
        handleAsyncUpdate() method will be made by the message thread as
        soon as possible.

        If an update callback is already pending but hasn't happened yet, calling
        this method will have no effect.

        It's thread-safe to call this method from any thread, BUT beware of calling
        it from a real-time (e.g. audio) thread, because it involves posting a message
        to the system queue, which means it may block (and in general will do on
        most OSes).
    */
    z0 triggerAsyncUpdate();

    /** This will stop any pending updates from happening.

        If called after triggerAsyncUpdate() and before the handleAsyncUpdate()
        callback happens, this will cancel the handleAsyncUpdate() callback.

        Note that this method simply cancels the next callback - if a callback is already
        in progress on a different thread, this won't block until the callback finishes, so
        there's no guarantee that the callback isn't still running when the method returns.
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

    //==============================================================================
    /** Called back to do whatever your class needs to do.

        This method is called by the message thread at the next convenient time
        after the triggerAsyncUpdate() method has been called.
    */
    virtual z0 handleAsyncUpdate() = 0;

private:
    //==============================================================================
    class AsyncUpdaterMessage;
    friend class ReferenceCountedObjectPtr<AsyncUpdaterMessage>;
    ReferenceCountedObjectPtr<AsyncUpdaterMessage> activeMessage;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AsyncUpdater)
};

} // namespace drx
