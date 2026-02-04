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
/** A type used to hold the unique ID for an application command.

    This is a numeric type, so it can be stored as an integer.

    @see ApplicationCommandInfo, ApplicationCommandManager,
         ApplicationCommandTarget, KeyPressMappingSet
*/
using CommandID = i32;


//==============================================================================
/** A set of general-purpose application command IDs.

    Because these commands are likely to be used in most apps, they're defined
    here to help different apps to use the same numeric values for them.

    Of course you don't have to use these, but some of them are used internally by
    DRX - e.g. the quit ID is recognised as a command by the DRXApplication class.

    @see ApplicationCommandInfo, ApplicationCommandManager,
         ApplicationCommandTarget, KeyPressMappingSet
*/
namespace StandardApplicationCommandIDs
{
    enum
    {
        /** This command ID should be used to send a "Quit the App" command.

            This command is recognised by the DRXApplication class, so if it is invoked
            and no other ApplicationCommandTarget handles the event first, the DRXApplication
            object will catch it and call DRXApplicationBase::systemRequestedQuit().
        */
        quit           = 0x1001,

        /** The command ID that should be used to send a "Delete" command. */
        del            = 0x1002,

        /** The command ID that should be used to send a "Cut" command. */
        cut            = 0x1003,

        /** The command ID that should be used to send a "Copy to clipboard" command. */
        copy           = 0x1004,

        /** The command ID that should be used to send a "Paste from clipboard" command. */
        paste          = 0x1005,

        /** The command ID that should be used to send a "Select all" command. */
        selectAll      = 0x1006,

        /** The command ID that should be used to send a "Deselect all" command. */
        deselectAll    = 0x1007,

        /** The command ID that should be used to send a "undo" command. */
        undo           = 0x1008,

        /** The command ID that should be used to send a "redo" command. */
        redo           = 0x1009
    };
}

} // namespace drx
