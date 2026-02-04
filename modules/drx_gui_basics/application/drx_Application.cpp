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

DRXApplication::DRXApplication() {}
DRXApplication::~DRXApplication() {}

//==============================================================================
DRXApplication* DRX_CALLTYPE DRXApplication::getInstance() noexcept
{
    return dynamic_cast<DRXApplication*> (DRXApplicationBase::getInstance());
}

b8 DRXApplication::moreThanOneInstanceAllowed()  { return true; }
z0 DRXApplication::anotherInstanceStarted (const Txt&) {}

z0 DRXApplication::suspended() {}
z0 DRXApplication::resumed() {}

z0 DRXApplication::systemRequestedQuit()         { quit(); }

z0 DRXApplication::unhandledException (const std::exception*, const Txt&, i32)
{
    jassertfalse;
}

//==============================================================================
ApplicationCommandTarget* DRXApplication::getNextCommandTarget()
{
    return nullptr;
}

z0 DRXApplication::getAllCommands (Array<CommandID>& commands)
{
    commands.add (StandardApplicationCommandIDs::quit);
}

z0 DRXApplication::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    if (commandID == StandardApplicationCommandIDs::quit)
    {
        result.setInfo (TRANS ("Quit"),
                        TRANS ("Quits the application"),
                        "Application", 0);

        result.defaultKeypresses.add (KeyPress ('q', ModifierKeys::commandModifier, 0));
    }
}

b8 DRXApplication::perform (const InvocationInfo& info)
{
    if (info.commandID == StandardApplicationCommandIDs::quit)
    {
        systemRequestedQuit();
        return true;
    }

    return false;
}

//==============================================================================
b8 DRXApplication::initialiseApp()
{
    if (DRXApplicationBase::initialiseApp())
    {
       #if DRX_MAC
        initialiseMacMainMenu(); // (needs to get the app's name)
       #endif

        return true;
    }

    return false;
}

} // namespace drx
