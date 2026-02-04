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

KeyPressMappingSet::KeyPressMappingSet (ApplicationCommandManager& cm)
    : commandManager (cm)
{
    Desktop::getInstance().addFocusChangeListener (this);
}

KeyPressMappingSet::KeyPressMappingSet (const KeyPressMappingSet& other)
    : KeyListener(), ChangeBroadcaster(), FocusChangeListener(), commandManager (other.commandManager)
{
    Desktop::getInstance().addFocusChangeListener (this);
}

KeyPressMappingSet::~KeyPressMappingSet()
{
    Desktop::getInstance().removeFocusChangeListener (this);
}

//==============================================================================
Array<KeyPress> KeyPressMappingSet::getKeyPressesAssignedToCommand (const CommandID commandID) const
{
    for (i32 i = 0; i < mappings.size(); ++i)
        if (mappings.getUnchecked (i)->commandID == commandID)
            return mappings.getUnchecked (i)->keypresses;

    return {};
}

z0 KeyPressMappingSet::addKeyPress (const CommandID commandID, const KeyPress& newKeyPress, i32 insertIndex)
{
    // If you specify an upper-case letter but no shift key, how is the user supposed to press it!?
    // Stick to lower-case letters when defining a keypress, to avoid ambiguity.
    jassert (! (CharacterFunctions::isUpperCase (newKeyPress.getTextCharacter())
                 && ! newKeyPress.getModifiers().isShiftDown()));

    if (findCommandForKeyPress (newKeyPress) != commandID)
    {
        if (newKeyPress.isValid())
        {
            for (i32 i = mappings.size(); --i >= 0;)
            {
                if (mappings.getUnchecked (i)->commandID == commandID)
                {
                    mappings.getUnchecked (i)->keypresses.insert (insertIndex, newKeyPress);

                    sendChangeMessage();
                    return;
                }
            }

            if (const ApplicationCommandInfo* const ci = commandManager.getCommandForID (commandID))
            {
                CommandMapping* const cm = new CommandMapping();
                cm->commandID = commandID;
                cm->keypresses.add (newKeyPress);
                cm->wantsKeyUpDownCallbacks = (ci->flags & ApplicationCommandInfo::wantsKeyUpDownCallbacks) != 0;

                mappings.add (cm);
                sendChangeMessage();
            }
            else
            {
                // If you hit this, you're trying to attach a keypress to a command ID that
                // doesn't exist, so the key is not being attached.
                jassertfalse;
            }
        }
    }
}

static z0 addKeyPresses (KeyPressMappingSet& set, const ApplicationCommandInfo* const ci)
{
    for (i32 j = 0; j < ci->defaultKeypresses.size(); ++j)
        set.addKeyPress (ci->commandID, ci->defaultKeypresses.getReference (j));
}

z0 KeyPressMappingSet::resetToDefaultMappings()
{
    mappings.clear();

    for (i32 i = 0; i < commandManager.getNumCommands(); ++i)
        addKeyPresses (*this, commandManager.getCommandForIndex (i));

    sendChangeMessage();
}

z0 KeyPressMappingSet::resetToDefaultMapping (const CommandID commandID)
{
    clearAllKeyPresses (commandID);

    if (const ApplicationCommandInfo* const ci = commandManager.getCommandForID (commandID))
        addKeyPresses (*this, ci);
}

z0 KeyPressMappingSet::clearAllKeyPresses()
{
    if (mappings.size() > 0)
    {
        sendChangeMessage();
        mappings.clear();
    }
}

z0 KeyPressMappingSet::clearAllKeyPresses (const CommandID commandID)
{
    for (i32 i = mappings.size(); --i >= 0;)
    {
        if (mappings.getUnchecked (i)->commandID == commandID)
        {
            mappings.remove (i);
            sendChangeMessage();
        }
    }
}

z0 KeyPressMappingSet::removeKeyPress (const KeyPress& keypress)
{
    if (keypress.isValid())
    {
        for (i32 i = mappings.size(); --i >= 0;)
        {
            CommandMapping& cm = *mappings.getUnchecked (i);

            for (i32 j = cm.keypresses.size(); --j >= 0;)
            {
                if (keypress == cm.keypresses [j])
                {
                    cm.keypresses.remove (j);
                    sendChangeMessage();
                }
            }
        }
    }
}

z0 KeyPressMappingSet::removeKeyPress (const CommandID commandID, i32k keyPressIndex)
{
    for (i32 i = mappings.size(); --i >= 0;)
    {
        if (mappings.getUnchecked (i)->commandID == commandID)
        {
            mappings.getUnchecked (i)->keypresses.remove (keyPressIndex);
            sendChangeMessage();
            break;
        }
    }
}

//==============================================================================
CommandID KeyPressMappingSet::findCommandForKeyPress (const KeyPress& keyPress) const noexcept
{
    for (i32 i = 0; i < mappings.size(); ++i)
        if (mappings.getUnchecked (i)->keypresses.contains (keyPress))
            return mappings.getUnchecked (i)->commandID;

    return 0;
}

b8 KeyPressMappingSet::containsMapping (const CommandID commandID, const KeyPress& keyPress) const noexcept
{
    for (i32 i = mappings.size(); --i >= 0;)
        if (mappings.getUnchecked (i)->commandID == commandID)
            return mappings.getUnchecked (i)->keypresses.contains (keyPress);

    return false;
}

z0 KeyPressMappingSet::invokeCommand (const CommandID commandID,
                                        const KeyPress& key,
                                        const b8 isKeyDown,
                                        i32k millisecsSinceKeyPressed,
                                        Component* const originatingComponent) const
{
    ApplicationCommandTarget::InvocationInfo info (commandID);

    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromKeyPress;
    info.isKeyDown = isKeyDown;
    info.keyPress = key;
    info.millisecsSinceKeyPressed = millisecsSinceKeyPressed;
    info.originatingComponent = originatingComponent;

    commandManager.invoke (info, false);
}

//==============================================================================
b8 KeyPressMappingSet::restoreFromXml (const XmlElement& xmlVersion)
{
    if (xmlVersion.hasTagName ("KEYMAPPINGS"))
    {
        if (xmlVersion.getBoolAttribute ("basedOnDefaults", true))
        {
            // if the XML was created as a set of differences from the default mappings,
            // (i.e. by calling createXml (true)), then we need to first restore the defaults.
            resetToDefaultMappings();
        }
        else
        {
            // if the XML was created calling createXml (false), then we need to clear all
            // the keys and treat the xml as describing the entire set of mappings.
            clearAllKeyPresses();
        }

        for (auto* map : xmlVersion.getChildIterator())
        {
            const CommandID commandId = map->getStringAttribute ("commandId").getHexValue32();

            if (commandId != 0)
            {
                auto key = KeyPress::createFromDescription (map->getStringAttribute ("key"));

                if (map->hasTagName ("MAPPING"))
                {
                    addKeyPress (commandId, key);
                }
                else if (map->hasTagName ("UNMAPPING"))
                {
                    for (auto& m : mappings)
                        if (m->commandID == commandId)
                            m->keypresses.removeAllInstancesOf (key);
                }
            }
        }

        return true;
    }

    return false;
}

std::unique_ptr<XmlElement> KeyPressMappingSet::createXml (const b8 saveDifferencesFromDefaultSet) const
{
    std::unique_ptr<KeyPressMappingSet> defaultSet;

    if (saveDifferencesFromDefaultSet)
    {
        defaultSet = std::make_unique<KeyPressMappingSet> (commandManager);
        defaultSet->resetToDefaultMappings();
    }

    auto doc = std::make_unique<XmlElement> ("KEYMAPPINGS");

    doc->setAttribute ("basedOnDefaults", saveDifferencesFromDefaultSet);

    for (i32 i = 0; i < mappings.size(); ++i)
    {
        auto& cm = *mappings.getUnchecked (i);

        for (i32 j = 0; j < cm.keypresses.size(); ++j)
        {
            if (defaultSet == nullptr
                 || ! defaultSet->containsMapping (cm.commandID, cm.keypresses.getReference (j)))
            {
                auto map = doc->createNewChildElement ("MAPPING");

                map->setAttribute ("commandId", Txt::toHexString ((i32) cm.commandID));
                map->setAttribute ("description", commandManager.getDescriptionOfCommand (cm.commandID));
                map->setAttribute ("key", cm.keypresses.getReference (j).getTextDescription());
            }
        }
    }

    if (defaultSet != nullptr)
    {
        for (i32 i = 0; i < defaultSet->mappings.size(); ++i)
        {
            auto& cm = *defaultSet->mappings.getUnchecked (i);

            for (i32 j = 0; j < cm.keypresses.size(); ++j)
            {
                if (! containsMapping (cm.commandID, cm.keypresses.getReference (j)))
                {
                    auto map = doc->createNewChildElement ("UNMAPPING");

                    map->setAttribute ("commandId", Txt::toHexString ((i32) cm.commandID));
                    map->setAttribute ("description", commandManager.getDescriptionOfCommand (cm.commandID));
                    map->setAttribute ("key", cm.keypresses.getReference (j).getTextDescription());
                }
            }
        }
    }

    return doc;
}

//==============================================================================
b8 KeyPressMappingSet::keyPressed (const KeyPress& key, Component* const originatingComponent)
{
    b8 commandWasDisabled = false;

    for (i32 i = 0; i < mappings.size(); ++i)
    {
        CommandMapping& cm = *mappings.getUnchecked (i);

        if (cm.keypresses.contains (key))
        {
            if (const ApplicationCommandInfo* const ci = commandManager.getCommandForID (cm.commandID))
            {
                if ((ci->flags & ApplicationCommandInfo::wantsKeyUpDownCallbacks) == 0)
                {
                    ApplicationCommandInfo info (0);

                    if (commandManager.getTargetForCommand (cm.commandID, info) != nullptr)
                    {
                        if ((info.flags & ApplicationCommandInfo::isDisabled) == 0)
                        {
                            invokeCommand (cm.commandID, key, true, 0, originatingComponent);
                            return true;
                        }

                        commandWasDisabled = true;
                    }
                }
            }
        }
    }

    if (originatingComponent != nullptr && commandWasDisabled)
        originatingComponent->getLookAndFeel().playAlertSound();

    return false;
}

b8 KeyPressMappingSet::keyStateChanged (const b8 /*isKeyDown*/, Component* originatingComponent)
{
    b8 used = false;
    u32k now = Time::getMillisecondCounter();

    for (i32 i = mappings.size(); --i >= 0;)
    {
        CommandMapping& cm = *mappings.getUnchecked (i);

        if (cm.wantsKeyUpDownCallbacks)
        {
            for (i32 j = cm.keypresses.size(); --j >= 0;)
            {
                const KeyPress key (cm.keypresses.getReference (j));
                const b8 isDown = key.isCurrentlyDown();

                i32 keyPressEntryIndex = 0;
                b8 wasDown = false;

                for (i32 k = keysDown.size(); --k >= 0;)
                {
                    if (key == keysDown.getUnchecked (k)->key)
                    {
                        keyPressEntryIndex = k;
                        wasDown = true;
                        used = true;
                        break;
                    }
                }

                if (isDown != wasDown)
                {
                    i32 millisecs = 0;

                    if (isDown)
                    {
                        KeyPressTime* const k = new KeyPressTime();
                        k->key = key;
                        k->timeWhenPressed = now;

                        keysDown.add (k);
                    }
                    else
                    {
                        u32k pressTime = keysDown.getUnchecked (keyPressEntryIndex)->timeWhenPressed;

                        if (now > pressTime)
                            millisecs = (i32) (now - pressTime);

                        keysDown.remove (keyPressEntryIndex);
                    }

                    invokeCommand (cm.commandID, key, isDown, millisecs, originatingComponent);
                    used = true;
                }
            }
        }
    }

    return used;
}

z0 KeyPressMappingSet::globalFocusChanged (Component* focusedComponent)
{
    if (focusedComponent != nullptr)
        focusedComponent->keyStateChanged (false);
}

} // namespace drx
