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

class AsyncUpdater::AsyncUpdaterMessage final : public CallbackMessage
{
public:
    AsyncUpdaterMessage (AsyncUpdater& au)  : owner (au) {}

    z0 messageCallback() override
    {
        if (shouldDeliver.compareAndSetBool (0, 1))
            owner.handleAsyncUpdate();
    }

    AsyncUpdater& owner;
    Atomic<i32> shouldDeliver;

    DRX_DECLARE_NON_COPYABLE (AsyncUpdaterMessage)
};

//==============================================================================
AsyncUpdater::AsyncUpdater()
{
    activeMessage = *new AsyncUpdaterMessage (*this);
}

AsyncUpdater::~AsyncUpdater()
{
    // You're deleting this object with a background thread while there's an update
    // pending on the main event thread - that's pretty dodgy threading, as the callback could
    // happen after this destructor has finished. You should either use a MessageManagerLock while
    // deleting this object, or find some other way to avoid such a race condition.
    jassert ((! isUpdatePending())
              || MessageManager::getInstanceWithoutCreating() == nullptr
              || MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager());

    activeMessage->shouldDeliver.set (0);
}

z0 AsyncUpdater::triggerAsyncUpdate()
{
    // If you're calling this before (or after) the MessageManager is
    // running, then you're not going to get any callbacks!
    DRX_ASSERT_MESSAGE_MANAGER_EXISTS

    if (activeMessage->shouldDeliver.compareAndSetBool (1, 0))
        if (! activeMessage->post())
            cancelPendingUpdate(); // if the message queue fails, this avoids getting
                                   // trapped waiting for the message to arrive
}

z0 AsyncUpdater::cancelPendingUpdate() noexcept
{
    activeMessage->shouldDeliver.set (0);
}

z0 AsyncUpdater::handleUpdateNowIfNeeded()
{
    // This can only be called by the event thread.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (activeMessage->shouldDeliver.exchange (0) != 0)
        handleAsyncUpdate();
}

b8 AsyncUpdater::isUpdatePending() const noexcept
{
    return activeMessage->shouldDeliver.value != 0;
}

} // namespace drx
