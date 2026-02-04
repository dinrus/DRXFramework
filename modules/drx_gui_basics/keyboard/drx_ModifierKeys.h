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
    Represents the state of the mouse buttons and modifier keys.

    This is used both by mouse events and by KeyPress objects to describe
    the state of keys such as shift, control, alt, etc.

    @see KeyPress, MouseEvent::mods

    @tags{GUI}
*/
class DRX_API  ModifierKeys
{
public:
    //==============================================================================
    /** Creates a ModifierKeys object with no flags set. */
    ModifierKeys() = default;

    /** Creates a ModifierKeys object from a raw set of flags.

        @param flags to represent the keys that are down
        @see    shiftModifier, ctrlModifier, altModifier, leftButtonModifier,
                rightButtonModifier, commandModifier, popupMenuClickModifier
    */
    ModifierKeys (i32 flags) noexcept;

    /** Creates a copy of another object. */
    ModifierKeys (const ModifierKeys&) = default;

    /** Copies this object from another one. */
    ModifierKeys& operator= (const ModifierKeys&) = default;

    //==============================================================================
    /** Checks whether the 'command' key flag is set (or 'ctrl' on Windows/Linux).

        This is a platform-agnostic way of checking for the operating system's
        preferred command-key modifier - so on the Mac it tests for the cmd key, on
        Windows/Linux, it's actually checking for the CTRL key.
    */
    inline b8 isCommandDown() const noexcept          { return testFlags (commandModifier); }

    /** Checks whether the user is trying to launch a pop-up menu.

        This checks for platform-specific modifiers that might indicate that the user
        is following the operating system's normal method of showing a pop-up menu.

        So on Windows/Linux, this method is really testing for a right-click.
        On the Mac, it tests for either the CTRL key being down, or a right-click.
    */
    inline b8 isPopupMenu() const noexcept            { return testFlags (popupMenuClickModifier); }

    /** Checks whether the flag is set for the left mouse-button. */
    inline b8 isLeftButtonDown() const noexcept       { return testFlags (leftButtonModifier); }

    /** Checks whether the flag is set for the right mouse-button.

        Note that for detecting popup-menu clicks, you should be using isPopupMenu() instead, as
        this is platform-independent (and makes your code more explanatory too).
    */
    inline b8 isRightButtonDown() const noexcept      { return testFlags (rightButtonModifier); }

    inline b8 isMiddleButtonDown() const noexcept     { return testFlags (middleButtonModifier); }

    /** Tests for any of the mouse-button flags. */
    inline b8 isAnyMouseButtonDown() const noexcept   { return testFlags (allMouseButtonModifiers); }

    /** Tests for any of the modifier key flags. */
    inline b8 isAnyModifierKeyDown() const noexcept   { return testFlags ((shiftModifier | ctrlModifier | altModifier | commandModifier)); }

    /** Checks whether the shift key's flag is set. */
    inline b8 isShiftDown() const noexcept            { return testFlags (shiftModifier); }

    /** Checks whether the CTRL key's flag is set.

        Remember that it's better to use the platform-agnostic routines to test for command-key and
        popup-menu modifiers.

        @see isCommandDown, isPopupMenu
    */
    inline b8 isCtrlDown() const noexcept             { return testFlags (ctrlModifier); }

    /** Checks whether the ALT key's flag is set. */
    inline b8 isAltDown() const noexcept              { return testFlags (altModifier); }

    //==============================================================================
    /** Flags that represent the different keys. */
    enum Flags
    {
        /** Indicates no modifier keys. */
        noModifiers                             = 0,

        /** Shift key flag. */
        shiftModifier                           = 1,

        /** CTRL key flag. */
        ctrlModifier                            = 2,

        /** ALT key flag. */
        altModifier                             = 4,

        /** Left mouse button flag. */
        leftButtonModifier                      = 16,

        /** Right mouse button flag. */
        rightButtonModifier                     = 32,

        /** Middle mouse button flag. */
        middleButtonModifier                    = 64,

       #if DRX_MAC || DRX_IOS
        /** Command key flag - on windows this is the same as the CTRL key flag. */
        commandModifier                         = 8,

        /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
            Mac it's the same as (rightButtonModifier | ctrlModifier). */
        popupMenuClickModifier                  = rightButtonModifier | ctrlModifier,
       #else
        /** Command key flag - on windows this is the same as the CTRL key flag. */
        commandModifier                         = ctrlModifier,

        /** Popup menu flag - on windows this is the same as rightButtonModifier, on the
            Mac it's the same as (rightButtonModifier | ctrlModifier). */
        popupMenuClickModifier                  = rightButtonModifier,
       #endif

        /** Represents a combination of all the shift, alt, ctrl and command key modifiers. */
        allKeyboardModifiers                    = shiftModifier | ctrlModifier | altModifier | commandModifier,

        /** Represents a combination of all the mouse buttons at once. */
        allMouseButtonModifiers                 = leftButtonModifier | rightButtonModifier | middleButtonModifier,

        /** Represents a combination of all the alt, ctrl and command key modifiers. */
        ctrlAltCommandModifiers                 = ctrlModifier | altModifier | commandModifier
    };

    //==============================================================================
    /** Returns a copy of only the mouse-button flags */
    [[nodiscard]] ModifierKeys withOnlyMouseButtons() const noexcept                  { return ModifierKeys (flags & allMouseButtonModifiers); }

    /** Returns a copy of only the non-mouse flags */
    [[nodiscard]] ModifierKeys withoutMouseButtons() const noexcept                   { return ModifierKeys (flags & ~allMouseButtonModifiers); }

    b8 operator== (const ModifierKeys other) const noexcept                          { return flags == other.flags; }
    b8 operator!= (const ModifierKeys other) const noexcept                          { return flags != other.flags; }

    //==============================================================================
    /** Returns the raw flags for direct testing. */
    inline i32 getRawFlags() const noexcept                                            { return flags; }

    [[nodiscard]] ModifierKeys withoutFlags (i32 rawFlagsToClear) const noexcept      { return ModifierKeys (flags & ~rawFlagsToClear); }
    [[nodiscard]] ModifierKeys withFlags (i32 rawFlagsToSet) const noexcept           { return ModifierKeys (flags | rawFlagsToSet); }

    /** Tests a combination of flags and returns true if any of them are set. */
    b8 testFlags (i32 flagsToTest) const noexcept                                    { return (flags & flagsToTest) != 0; }

    /** Returns the total number of mouse buttons that are down. */
    i32 getNumMouseButtonsDown() const noexcept;

    //==============================================================================
    /** This object represents the last-known state of the keyboard and mouse buttons. */
    static ModifierKeys currentModifiers;

    /** Creates a ModifierKeys object to represent the last-known state of the
        keyboard and mouse buttons.

        This method is here for backwards compatibility and there's no need to call it anymore,
        you should use the public currentModifiers member directly.
     */
    static ModifierKeys getCurrentModifiers() noexcept                                 { return currentModifiers; }

    /** Creates a ModifierKeys object to represent the current state of the
        keyboard and mouse buttons.

        This method is here for backwards compatibility and you should call ComponentPeer::getCurrentModifiersRealtime()
        instead (which is what this method now does).
    */
    static ModifierKeys getCurrentModifiersRealtime() noexcept;

private:
    i32 flags = 0;
};

} // namespace drx
