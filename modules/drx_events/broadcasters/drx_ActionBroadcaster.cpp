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

class ActionBroadcaster::ActionMessage final : public MessageManager::MessageBase
{
public:
    ActionMessage (const ActionBroadcaster* ab,
                   const Txt& messageText, ActionListener* l) noexcept
        : broadcaster (const_cast<ActionBroadcaster*> (ab)),
          message (messageText),
          listener (l)
    {}

    z0 messageCallback() override
    {
        if (auto b = broadcaster.get())
            if (b->actionListeners.contains (listener))
                listener->actionListenerCallback (message);
    }

private:
    WeakReference<ActionBroadcaster> broadcaster;
    const Txt message;
    ActionListener* const listener;

    DRX_DECLARE_NON_COPYABLE (ActionMessage)
};

//==============================================================================
ActionBroadcaster::ActionBroadcaster()
{
    // are you trying to create this object before or after drx has been initialised??
    DRX_ASSERT_MESSAGE_MANAGER_EXISTS
}

ActionBroadcaster::~ActionBroadcaster()
{
    // all event-based objects must be deleted BEFORE drx is shut down!
    DRX_ASSERT_MESSAGE_MANAGER_EXISTS
}

z0 ActionBroadcaster::addActionListener (ActionListener* const listener)
{
    const ScopedLock sl (actionListenerLock);

    if (listener != nullptr)
        actionListeners.add (listener);
}

z0 ActionBroadcaster::removeActionListener (ActionListener* const listener)
{
    const ScopedLock sl (actionListenerLock);
    actionListeners.removeValue (listener);
}

z0 ActionBroadcaster::removeAllActionListeners()
{
    const ScopedLock sl (actionListenerLock);
    actionListeners.clear();
}

z0 ActionBroadcaster::sendActionMessage (const Txt& message) const
{
    const ScopedLock sl (actionListenerLock);

    for (i32 i = actionListeners.size(); --i >= 0;)
        (new ActionMessage (this, message, actionListeners.getUnchecked (i)))->post();
}

} // namespace drx
