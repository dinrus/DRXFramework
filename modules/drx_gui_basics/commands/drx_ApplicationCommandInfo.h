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
    Holds information describing an application command.

    This object is used to pass information about a particular command, such as its
    name, description and other usage flags.

    When an ApplicationCommandTarget is asked to provide information about the commands
    it can perform, this is the structure gets filled-in to describe each one.

    @see ApplicationCommandTarget, ApplicationCommandTarget::getCommandInfo(),
         ApplicationCommandManager

    @tags{GUI}
*/
struct DRX_API  ApplicationCommandInfo
{
    //==============================================================================
    explicit ApplicationCommandInfo (CommandID commandID) noexcept;

    //==============================================================================
    /** Sets a number of the structures values at once.

        The meanings of each of the parameters is described below, in the appropriate
        member variable's description.
    */
    z0 setInfo (const Txt& shortName,
                  const Txt& description,
                  const Txt& categoryName,
                  i32 flags) noexcept;

    /** An easy way to set or remove the isDisabled bit in the structure's flags field.

        If isActive is true, the flags member has the isDisabled bit cleared; if isActive
        is false, the bit is set.
    */
    z0 setActive (b8 isActive) noexcept;

    /** An easy way to set or remove the isTicked bit in the structure's flags field.
    */
    z0 setTicked (b8 isTicked) noexcept;

    /** Handy method for adding a keypress to the defaultKeypresses array.

        This is just so you can write things like:
        @code
        myinfo.addDefaultKeypress ('s', ModifierKeys::commandModifier);
        @endcode
        instead of
        @code
        myinfo.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier));
        @endcode
    */
    z0 addDefaultKeypress (i32 keyCode, ModifierKeys modifiers) noexcept;

    //==============================================================================
    /** The command's unique ID number.
    */
    CommandID commandID;

    /** A short name to describe the command.

        This should be suitable for use in menus, on buttons that trigger the command, etc.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    Txt shortName;

    /** A longer description of the command.

        This should be suitable for use in contexts such as a KeyMappingEditorComponent or
        pop-up tooltip describing what the command does.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    Txt description;

    /** A named category that the command fits into.

        You can give your commands any category you like, and these will be displayed in
        contexts such as the KeyMappingEditorComponent, where the category is used to group
        commands together.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    Txt categoryName;

    /** A list of zero or more keypresses that should be used as the default keys for
        this command.

        Methods such as KeyPressMappingSet::resetToDefaultMappings() will use the keypresses in
        this list to initialise the default set of key-to-command mappings.

        @see addDefaultKeypress
    */
    Array<KeyPress> defaultKeypresses;

    //==============================================================================
    /** Flags describing the ways in which this command should be used.

        A bitwise-OR of these values is stored in the ApplicationCommandInfo::flags
        variable.
    */
    enum CommandFlags
    {
        /** Indicates that the command can't currently be performed.

            The ApplicationCommandTarget::getCommandInfo() method must set this flag if it's
            not currently permissible to perform the command. If the flag is set, then
            components that trigger the command, e.g. PopupMenu, may choose to grey-out the
            command or show themselves as not being enabled.

            @see ApplicationCommandInfo::setActive
        */
        isDisabled                  = 1 << 0,

        /** Indicates that the command should have a tick next to it on a menu.

            If your command is shown on a menu and this is set, it'll show a tick next to
            it. Other components such as buttons may also use this flag to indicate that it
            is a value that can be toggled, and is currently in the 'on' state.

            @see ApplicationCommandInfo::setTicked
        */
        isTicked                    = 1 << 1,

        /** If this flag is present, then when a KeyPressMappingSet invokes the command,
            it will call the command twice, once on key-down and again on key-up.

            @see ApplicationCommandTarget::InvocationInfo
        */
        wantsKeyUpDownCallbacks     = 1 << 2,

        /** If this flag is present, then a KeyMappingEditorComponent will not display the
            command in its list.
        */
        hiddenFromKeyEditor         = 1 << 3,

        /** If this flag is present, then a KeyMappingEditorComponent will display the
            command in its list, but won't allow the assigned keypress to be changed.
        */
        readOnlyInKeyEditor         = 1 << 4,

        /** If this flag is present and the command is invoked from a keypress, then any
            buttons or menus that are also connected to the command will not flash to
            indicate that they've been triggered.
        */
        dontTriggerVisualFeedback   = 1 << 5
    };

    /** A bitwise-OR of the values specified in the CommandFlags enum.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    i32 flags;
};

} // namespace drx
