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

/** Represents the state of an accessible UI element.

    An instance of this class is returned by `AccessibilityHandler::getCurrentState()`
    to convey its current state to an accessibility client.

    @see AccessibilityHandler

    @tags{Accessibility}
*/
class DRX_API  AccessibleState
{
public:
    /** Constructor.

        Represents a "default" state with no flags set. To set a flag, use one of the
        `withX()` methods - these can be chained together to set multiple flags.
    */
    AccessibleState() = default;

    //==============================================================================
    /** Sets the checkable flag and returns the new state.

        @see isCheckable
    */
    [[nodiscard]] AccessibleState withCheckable() const noexcept            { return withFlag (Flags::checkable); }

    /** Sets the checked flag and returns the new state.

        @see isChecked
    */
    [[nodiscard]] AccessibleState withChecked() const noexcept              { return withFlag (Flags::checked); }

    /** Sets the collapsed flag and returns the new state.

        @see isCollapsed
    */
    [[nodiscard]] AccessibleState withCollapsed() const noexcept            { return withFlag (Flags::collapsed); }

    /** Sets the expandable flag and returns the new state.

        @see isExpandable
    */
    [[nodiscard]] AccessibleState withExpandable() const noexcept           { return withFlag (Flags::expandable); }

    /** Sets the expanded flag and returns the new state.

        @see isExpanded
    */
    [[nodiscard]] AccessibleState withExpanded() const noexcept             { return withFlag (Flags::expanded); }

    /** Sets the focusable flag and returns the new state.

        @see isFocusable
    */
    [[nodiscard]] AccessibleState withFocusable() const noexcept            { return withFlag (Flags::focusable); }

    /** Sets the focused flag and returns the new state.

        @see isFocused
    */
    [[nodiscard]] AccessibleState withFocused() const noexcept              { return withFlag (Flags::focused); }

    /** Sets the ignored flag and returns the new state.

        @see isIgnored
    */
    [[nodiscard]] AccessibleState withIgnored() const noexcept              { return withFlag (Flags::ignored); }

    /** Sets the selectable flag and returns the new state.

        @see isSelectable
    */
    [[nodiscard]] AccessibleState withSelectable() const noexcept           { return withFlag (Flags::selectable); }

    /** Sets the multiSelectable flag and returns the new state.

        @see isMultiSelectable
    */
    [[nodiscard]] AccessibleState withMultiSelectable() const noexcept      { return withFlag (Flags::multiSelectable); }

    /** Sets the selected flag and returns the new state.

        @see isSelected
    */
    [[nodiscard]] AccessibleState withSelected() const noexcept             { return withFlag (Flags::selected); }

    /** Sets the accessible offscreen flag and returns the new state.

        @see isSelected
    */
    [[nodiscard]] AccessibleState withAccessibleOffscreen() const noexcept  { return withFlag (Flags::accessibleOffscreen); }

    //==============================================================================
    /** Возвращает true, если the UI element is checkable.

        @see withCheckable
    */
    b8 isCheckable() const noexcept            { return isFlagSet (Flags::checkable); }

    /** Возвращает true, если the UI element is checked.

        @see withChecked
    */
    b8 isChecked() const noexcept              { return isFlagSet (Flags::checked); }

    /** Возвращает true, если the UI element is collapsed.

        @see withCollapsed
    */
    b8 isCollapsed() const noexcept            { return isFlagSet (Flags::collapsed); }

    /** Возвращает true, если the UI element is expandable.

        @see withExpandable
    */
    b8 isExpandable() const noexcept           { return isFlagSet (Flags::expandable); }

    /** Возвращает true, если the UI element is expanded.

        @see withExpanded
    */
    b8 isExpanded() const noexcept             { return isFlagSet (Flags::expanded); }

    /** Возвращает true, если the UI element is focusable.

        @see withFocusable
    */
    b8 isFocusable() const noexcept            { return isFlagSet (Flags::focusable); }

    /** Возвращает true, если the UI element is focused.

        @see withFocused
    */
    b8 isFocused() const noexcept              { return isFlagSet (Flags::focused); }

    /** Возвращает true, если the UI element is ignored.

        @see withIgnored
    */
    b8 isIgnored() const noexcept              { return isFlagSet (Flags::ignored); }

    /** Возвращает true, если the UI element supports multiple item selection.

        @see withMultiSelectable
    */
    b8 isMultiSelectable() const noexcept      { return isFlagSet (Flags::multiSelectable); }

    /** Возвращает true, если the UI element is selectable.

        @see withSelectable
    */
    b8 isSelectable() const noexcept           { return isFlagSet (Flags::selectable); }

    /** Возвращает true, если the UI element is selected.

        @see withSelected
    */
    b8 isSelected() const noexcept             { return isFlagSet (Flags::selected); }

    /** Возвращает true, если the UI element is accessible offscreen.

        @see withSelected
    */
    b8 isAccessibleOffscreen() const noexcept  { return isFlagSet (Flags::accessibleOffscreen); }

private:
    enum Flags
    {
        checkable           = (1 << 0),
        checked             = (1 << 1),
        collapsed           = (1 << 2),
        expandable          = (1 << 3),
        expanded            = (1 << 4),
        focusable           = (1 << 5),
        focused             = (1 << 6),
        ignored             = (1 << 7),
        multiSelectable     = (1 << 8),
        selectable          = (1 << 9),
        selected            = (1 << 10),
        accessibleOffscreen = (1 << 11)
    };

    [[nodiscard]] AccessibleState withFlag (i32 flag) const noexcept
    {
        auto copy = *this;
        copy.flags |= flag;

        return copy;
    }

    b8 isFlagSet (i32 flag) const noexcept
    {
        return (flags & flag) != 0;
    }

    i32 flags = 0;
};

} // namespace drx
