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

class ApplicationCommandTarget::CommandMessage final : public MessageManager::MessageBase
{
public:
    CommandMessage (ApplicationCommandTarget* const target, const InvocationInfo& inf)
        : owner (target), info (inf)
    {
    }

    z0 messageCallback() override
    {
        if (ApplicationCommandTarget* const target = owner)
            target->tryToInvoke (info, false);
    }

private:
    WeakReference<ApplicationCommandTarget> owner;
    const InvocationInfo info;

    DRX_DECLARE_NON_COPYABLE (CommandMessage)
};

//==============================================================================
ApplicationCommandTarget::ApplicationCommandTarget() {}
ApplicationCommandTarget::~ApplicationCommandTarget() {}

//==============================================================================
b8 ApplicationCommandTarget::tryToInvoke (const InvocationInfo& info, const b8 async)
{
    if (isCommandActive (info.commandID))
    {
        if (async)
        {
            (new CommandMessage (this, info))->post();
            return true;
        }

        if (perform (info))
            return true;

        // Hmm.. your target claimed that it could perform this command, but failed to do so.
        // If it can't do it at the moment for some reason, it should clear the 'isActive' flag
        // when it returns the command's info.
        jassertfalse;
    }

    return false;
}

ApplicationCommandTarget* ApplicationCommandTarget::findFirstTargetParentComponent()
{
    if (Component* const c = dynamic_cast<Component*> (this))
        return c->findParentComponentOfClass<ApplicationCommandTarget>();

    return nullptr;
}

ApplicationCommandTarget* ApplicationCommandTarget::getTargetForCommand (const CommandID commandID)
{
    ApplicationCommandTarget* target = this;
    i32 depth = 0;

    while (target != nullptr)
    {
        Array<CommandID> commandIDs;
        target->getAllCommands (commandIDs);

        if (commandIDs.contains (commandID))
            return target;

        target = target->getNextCommandTarget();

        ++depth;
        jassert (depth < 100); // could be a recursive command chain??
        jassert (target != this); // definitely a recursive command chain!

        if (depth > 100 || target == this)
            break;
    }

    if (target == nullptr)
    {
        target = DRXApplication::getInstance();

        if (target != nullptr)
        {
            Array<CommandID> commandIDs;
            target->getAllCommands (commandIDs);

            if (commandIDs.contains (commandID))
                return target;
        }
    }

    return nullptr;
}

b8 ApplicationCommandTarget::isCommandActive (const CommandID commandID)
{
    ApplicationCommandInfo info (commandID);
    info.flags = ApplicationCommandInfo::isDisabled;

    getCommandInfo (commandID, info);

    return (info.flags & ApplicationCommandInfo::isDisabled) == 0;
}

//==============================================================================
b8 ApplicationCommandTarget::invoke (const InvocationInfo& info, const b8 async)
{
    ApplicationCommandTarget* target = this;
    i32 depth = 0;

    while (target != nullptr)
    {
        if (target->tryToInvoke (info, async))
            return true;

        target = target->getNextCommandTarget();

        ++depth;
        jassert (depth < 100); // could be a recursive command chain??
        jassert (target != this); // definitely a recursive command chain!

        if (depth > 100 || target == this)
            break;
    }

    if (target == nullptr)
    {
        target = DRXApplication::getInstance();

        if (target != nullptr)
            return target->tryToInvoke (info, async);
    }

    return false;
}

b8 ApplicationCommandTarget::invokeDirectly (const CommandID commandID, const b8 asynchronously)
{
    ApplicationCommandTarget::InvocationInfo info (commandID);
    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::direct;

    return invoke (info, asynchronously);
}

//==============================================================================
ApplicationCommandTarget::InvocationInfo::InvocationInfo (const CommandID command)
    : commandID (command),
      commandFlags (0),
      invocationMethod (direct),
      originatingComponent (nullptr),
      isKeyDown (false),
      millisecsSinceKeyPressed (0)
{
}

} // namespace drx
