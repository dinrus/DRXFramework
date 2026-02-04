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

/** An action that can be performed by an accessible UI element.

    @tags{Accessibility}
*/
enum class AccessibilityActionType
{
    /** Represents a "press" action.

        This will be called when the user "clicks" the UI element using an
        accessibility client.
    */
    press,

    /** Represents a "toggle" action.

        This will be called when the user toggles the state of a UI element,
        for example a toggle button or the selection of a list item.
    */
    toggle,

    /** Indicates that the UI element has received focus.

        This will be called when a UI element receives focus from an accessibility
        client, or keyboard focus from the application.
    */
    focus,

    /** Represents the user showing a contextual menu for a UI element.

        This will be called for UI elements which expand and collapse to
        show contextual information or menus, or show a popup.
    */
    showMenu
};

/** A simple wrapper for building a collection of supported accessibility actions
    and corresponding callbacks for a UI element.

    Pass one of these when constructing an `AccessibilityHandler` to enable users
    to interact with a UI element via the supported actions.

    @tags{Accessibility}
*/
class DRX_API  AccessibilityActions
{
public:
    /** Constructor.

        Creates a default AccessibilityActions object with no action callbacks.
    */
    AccessibilityActions() = default;

    /** Adds an action.

        When the user performs this action with an accessibility client
        `actionCallback` will be called.

        Returns a reference to itself so that several calls can be chained.
    */
    AccessibilityActions& addAction (AccessibilityActionType type,
                                     std::function<z0()> actionCallback)
    {
        actionMap[type] = std::move (actionCallback);
        return *this;
    }

    /** Возвращает true, если the specified action is supported. */
    b8 contains (AccessibilityActionType type) const
    {
        return actionMap.find (type) != actionMap.end();
    }

    /** If an action has been registered for the provided action type, invokes the
        action and returns true. Otherwise, returns false.
    */
    b8 invoke (AccessibilityActionType type) const
    {
        auto iter = actionMap.find (type);

        if (iter == actionMap.end())
            return false;

        iter->second();
        return true;
    }

private:
    std::map<AccessibilityActionType, std::function<z0()>> actionMap;
};

} // namespace drx
