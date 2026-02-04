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

ApplicationCommandManager::ApplicationCommandManager()
{
    keyMappings.reset (new KeyPressMappingSet (*this));
    Desktop::getInstance().addFocusChangeListener (this);
}

ApplicationCommandManager::~ApplicationCommandManager()
{
    Desktop::getInstance().removeFocusChangeListener (this);
    keyMappings.reset();
}

//==============================================================================
z0 ApplicationCommandManager::clearCommands()
{
    commands.clear();
    keyMappings->clearAllKeyPresses();
    triggerAsyncUpdate();
}

z0 ApplicationCommandManager::registerCommand (const ApplicationCommandInfo& newCommand)
{
    // zero isn't a valid command ID!
    jassert (newCommand.commandID != 0);

    // the name isn't optional!
    jassert (newCommand.shortName.isNotEmpty());

    if (auto* command = getMutableCommandForID (newCommand.commandID))
    {
        // Trying to re-register the same command ID with different parameters can often indicate a typo.
        // This assertion is here because I've found it useful catching some mistakes, but it may also cause
        // false alarms if you're deliberately updating some flags for a command.
        jassert (newCommand.shortName == getCommandForID (newCommand.commandID)->shortName
                  && newCommand.categoryName == getCommandForID (newCommand.commandID)->categoryName
                  && newCommand.defaultKeypresses == getCommandForID (newCommand.commandID)->defaultKeypresses
                  && (newCommand.flags & (ApplicationCommandInfo::wantsKeyUpDownCallbacks | ApplicationCommandInfo::hiddenFromKeyEditor | ApplicationCommandInfo::readOnlyInKeyEditor))
                       == (getCommandForID (newCommand.commandID)->flags & (ApplicationCommandInfo::wantsKeyUpDownCallbacks | ApplicationCommandInfo::hiddenFromKeyEditor | ApplicationCommandInfo::readOnlyInKeyEditor)));

        *command = newCommand;
    }
    else
    {
        auto* newInfo = new ApplicationCommandInfo (newCommand);
        newInfo->flags &= ~ApplicationCommandInfo::isTicked;
        commands.add (newInfo);

        keyMappings->resetToDefaultMapping (newCommand.commandID);

        triggerAsyncUpdate();
    }
}

z0 ApplicationCommandManager::registerAllCommandsForTarget (ApplicationCommandTarget* target)
{
    if (target != nullptr)
    {
        Array<CommandID> commandIDs;
        target->getAllCommands (commandIDs);

        for (i32 i = 0; i < commandIDs.size(); ++i)
        {
            ApplicationCommandInfo info (commandIDs.getUnchecked (i));
            target->getCommandInfo (info.commandID, info);

            registerCommand (info);
        }
    }
}

z0 ApplicationCommandManager::removeCommand (const CommandID commandID)
{
    for (i32 i = commands.size(); --i >= 0;)
    {
        if (commands.getUnchecked (i)->commandID == commandID)
        {
            commands.remove (i);
            triggerAsyncUpdate();

            const Array<KeyPress> keys (keyMappings->getKeyPressesAssignedToCommand (commandID));

            for (i32 j = keys.size(); --j >= 0;)
                keyMappings->removeKeyPress (keys.getReference (j));
        }
    }
}

z0 ApplicationCommandManager::commandStatusChanged()
{
    triggerAsyncUpdate();
}

//==============================================================================
ApplicationCommandInfo* ApplicationCommandManager::getMutableCommandForID (CommandID commandID) const noexcept
{
    for (i32 i = commands.size(); --i >= 0;)
        if (commands.getUnchecked (i)->commandID == commandID)
            return commands.getUnchecked (i);

    return nullptr;
}

const ApplicationCommandInfo* ApplicationCommandManager::getCommandForID (CommandID commandID) const noexcept
{
    return getMutableCommandForID (commandID);
}

Txt ApplicationCommandManager::getNameOfCommand (CommandID commandID) const noexcept
{
    if (auto* ci = getCommandForID (commandID))
        return ci->shortName;

    return {};
}

Txt ApplicationCommandManager::getDescriptionOfCommand (CommandID commandID) const noexcept
{
    if (auto* ci = getCommandForID (commandID))
        return ci->description.isNotEmpty() ? ci->description
                                            : ci->shortName;

    return {};
}

StringArray ApplicationCommandManager::getCommandCategories() const
{
    StringArray s;

    for (i32 i = 0; i < commands.size(); ++i)
        s.addIfNotAlreadyThere (commands.getUnchecked (i)->categoryName, false);

    return s;
}

Array<CommandID> ApplicationCommandManager::getCommandsInCategory (const Txt& categoryName) const
{
    Array<CommandID> results;

    for (i32 i = 0; i < commands.size(); ++i)
        if (commands.getUnchecked (i)->categoryName == categoryName)
            results.add (commands.getUnchecked (i)->commandID);

    return results;
}

//==============================================================================
b8 ApplicationCommandManager::invokeDirectly (CommandID commandID, b8 asynchronously)
{
    ApplicationCommandTarget::InvocationInfo info (commandID);
    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::direct;

    return invoke (info, asynchronously);
}

b8 ApplicationCommandManager::invoke (const ApplicationCommandTarget::InvocationInfo& inf, b8 asynchronously)
{
    // This call isn't thread-safe for use from a non-UI thread without locking the message
    // manager first..
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    b8 ok = false;
    ApplicationCommandInfo commandInfo (0);

    if (auto* target = getTargetForCommand (inf.commandID, commandInfo))
    {
        ApplicationCommandTarget::InvocationInfo info (inf);
        info.commandFlags = commandInfo.flags;

        sendListenerInvokeCallback (info);
        ok = target->invoke (info, asynchronously);
        commandStatusChanged();
    }

    return ok;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::getFirstCommandTarget (CommandID)
{
    return firstTarget != nullptr ? firstTarget
                                  : findDefaultComponentTarget();
}

z0 ApplicationCommandManager::setFirstCommandTarget (ApplicationCommandTarget* newTarget) noexcept
{
    firstTarget = newTarget;
}

ApplicationCommandTarget* ApplicationCommandManager::getTargetForCommand (CommandID commandID,
                                                                          ApplicationCommandInfo& upToDateInfo)
{
    auto* target = getFirstCommandTarget (commandID);

    if (target == nullptr)
        target = DRXApplication::getInstance();

    if (target != nullptr)
        target = target->getTargetForCommand (commandID);

    if (target != nullptr)
    {
        upToDateInfo.commandID = commandID;
        target->getCommandInfo (commandID, upToDateInfo);
    }

    return target;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::findTargetForComponent (Component* c)
{
    auto* target = dynamic_cast<ApplicationCommandTarget*> (c);

    if (target == nullptr && c != nullptr)
        target = c->findParentComponentOfClass<ApplicationCommandTarget>();

    return target;
}

ApplicationCommandTarget* ApplicationCommandManager::findDefaultComponentTarget()
{
    auto* c = Component::getCurrentlyFocusedComponent();

    if (c == nullptr)
    {
        if (auto* activeWindow = TopLevelWindow::getActiveTopLevelWindow())
        {
            if (auto* peer = activeWindow->getPeer())
            {
                c = peer->getLastFocusedSubcomponent();

                if (c == nullptr)
                    c = activeWindow;
            }
        }
    }

    if (c == nullptr)
    {
        auto& desktop = Desktop::getInstance();

        // getting a bit desperate now: try all desktop comps..
        for (i32 i = desktop.getNumComponents(); --i >= 0;)
            if (auto* component = desktop.getComponent (i))
                if (detail::WindowingHelpers::isForegroundOrEmbeddedProcess (component))
                    if (auto* peer = component->getPeer())
                        if (auto* target = findTargetForComponent (peer->getLastFocusedSubcomponent()))
                            return target;
    }

    if (c != nullptr)
    {
        // if we're focused on a ResizableWindow, chances are that it's the content
        // component that really should get the event. And if not, the event will
        // still be passed up to the top level window anyway, so let's send it to the
        // content comp.
        if (auto* resizableWindow = dynamic_cast<ResizableWindow*> (c))
            if (auto* content = resizableWindow->getContentComponent())
                c = content;

        if (auto* target = findTargetForComponent (c))
            return target;
    }

    return DRXApplication::getInstance();
}

//==============================================================================
z0 ApplicationCommandManager::addListener (ApplicationCommandManagerListener* listener)
{
    listeners.add (listener);
}

z0 ApplicationCommandManager::removeListener (ApplicationCommandManagerListener* listener)
{
    listeners.remove (listener);
}

z0 ApplicationCommandManager::sendListenerInvokeCallback (const ApplicationCommandTarget::InvocationInfo& info)
{
    listeners.call ([&] (ApplicationCommandManagerListener& l) { l.applicationCommandInvoked (info); });
}

z0 ApplicationCommandManager::handleAsyncUpdate()
{
    listeners.call ([] (ApplicationCommandManagerListener& l) { l.applicationCommandListChanged(); });
}

z0 ApplicationCommandManager::globalFocusChanged (Component*)
{
    commandStatusChanged();
}

} // namespace drx
