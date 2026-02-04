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

ChangeBroadcaster::ChangeBroadcaster() noexcept
{
    broadcastCallback.owner = this;
}

ChangeBroadcaster::~ChangeBroadcaster()
{
}

z0 ChangeBroadcaster::addChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely added when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    changeListeners.add (listener);
    anyListeners = true;
}

z0 ChangeBroadcaster::removeChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely removed when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    changeListeners.remove (listener);
    anyListeners = changeListeners.size() > 0;
}

z0 ChangeBroadcaster::removeAllChangeListeners()
{
    // Listeners can only be safely removed when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    changeListeners.clear();
    anyListeners = false;
}

z0 ChangeBroadcaster::sendChangeMessage()
{
    if (anyListeners)
        broadcastCallback.triggerAsyncUpdate();
}

z0 ChangeBroadcaster::sendSynchronousChangeMessage()
{
    // This can only be called by the event thread.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    broadcastCallback.cancelPendingUpdate();
    callListeners();
}

z0 ChangeBroadcaster::dispatchPendingMessages()
{
    broadcastCallback.handleUpdateNowIfNeeded();
}

z0 ChangeBroadcaster::callListeners()
{
    changeListeners.call ([this] (ChangeListener& l) { l.changeListenerCallback (this); });
}

//==============================================================================
ChangeBroadcaster::ChangeBroadcasterCallback::ChangeBroadcasterCallback()
    : owner (nullptr)
{
}

z0 ChangeBroadcaster::ChangeBroadcasterCallback::handleAsyncUpdate()
{
    jassert (owner != nullptr);
    owner->callListeners();
}

} // namespace drx
