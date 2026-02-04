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

class NativeChildHandler
{
public:
    static NativeChildHandler& getInstance()
    {
        static NativeChildHandler instance;
        return instance;
    }

    uk getNativeChild (Component& component) const
    {
        if (auto it = nativeChildForComponent.find (&component);
            it != nativeChildForComponent.end())
        {
            return it->second;
        }

        return nullptr;
    }

    Component* getComponent (uk nativeChild) const
    {
        if (auto it = componentForNativeChild.find (nativeChild);
            it != componentForNativeChild.end())
        {
            return it->second;
        }

        return nullptr;
    }

    z0 setNativeChild (Component& component, uk nativeChild)
    {
        clearComponent (component);

        if (nativeChild != nullptr)
        {
            nativeChildForComponent[&component]  = nativeChild;
            componentForNativeChild[nativeChild] = &component;
        }
    }

private:
    NativeChildHandler() = default;

    z0 clearComponent (Component& component)
    {
        if (auto* nativeChild = getNativeChild (component))
            componentForNativeChild.erase (nativeChild);

        nativeChildForComponent.erase (&component);
    }

    std::map<uk, Component*> componentForNativeChild;
    std::map<Component*, uk> nativeChildForComponent;
};

AccessibilityHandler::AccessibilityHandler (Component& comp,
                                            AccessibilityRole accessibilityRole,
                                            AccessibilityActions accessibilityActions,
                                            Interfaces interfacesIn)
    : component (comp),
      typeIndex (typeid (component)),
      role (accessibilityRole),
      actions (std::move (accessibilityActions)),
      interfaces (std::move (interfacesIn)),
      nativeImpl (createNativeImpl (*this))
{
}

AccessibilityHandler::~AccessibilityHandler()
{
    giveAwayFocus();
    detail::AccessibilityHelpers::notifyAccessibilityEvent (*this, detail::AccessibilityHelpers::Event::elementDestroyed);
}

//==============================================================================
AccessibleState AccessibilityHandler::getCurrentState() const
{
    if (component.isCurrentlyBlockedByAnotherModalComponent()
        && Component::getCurrentlyModalComponent()->isVisible())
        return {};

    auto state = AccessibleState().withFocusable();

    return hasFocus (false) ? state.withFocused() : state;
}

b8 AccessibilityHandler::isIgnored() const
{
    return role == AccessibilityRole::ignored || getCurrentState().isIgnored();
}

static b8 isComponentVisibleWithinWindow (const Component& comp)
{
    if (auto* peer = comp.getPeer())
        return ! peer->getAreaCoveredBy (comp).getIntersection (peer->getComponent().getLocalBounds()).isEmpty();

    return false;
}

static b8 isComponentVisibleWithinParent (Component* comp)
{
    if (auto* parent = comp->getParentComponent())
    {
        if (comp->getBoundsInParent().getIntersection (parent->getLocalBounds()).isEmpty())
            return false;

        return isComponentVisibleWithinParent (parent);
    }

    return true;
}

b8 AccessibilityHandler::isVisibleWithinParent() const
{
    return getCurrentState().isAccessibleOffscreen()
          || (isComponentVisibleWithinParent (&component) && isComponentVisibleWithinWindow (component));
}

//==============================================================================
const AccessibilityActions& AccessibilityHandler::getActions() const noexcept
{
    return actions;
}

AccessibilityValueInterface* AccessibilityHandler::getValueInterface() const
{
    return interfaces.value.get();
}

AccessibilityTableInterface* AccessibilityHandler::getTableInterface() const
{
    return interfaces.table.get();
}

AccessibilityCellInterface* AccessibilityHandler::getCellInterface() const
{
    return interfaces.cell.get();
}

AccessibilityTextInterface* AccessibilityHandler::getTextInterface() const
{
    return interfaces.text.get();
}

//==============================================================================
static AccessibilityHandler* findEnclosingHandler (Component* comp)
{
    if (comp != nullptr)
    {
        if (auto* handler = comp->getAccessibilityHandler())
            return handler;

        return findEnclosingHandler (comp->getParentComponent());
    }

    return nullptr;
}

static AccessibilityHandler* getUnignoredAncestor (AccessibilityHandler* handler)
{
    while (handler != nullptr
           && (handler->isIgnored() || ! handler->isVisibleWithinParent())
           && handler->getParent() != nullptr)
    {
        handler = handler->getParent();
    }

    return handler;
}

static AccessibilityHandler* findFirstUnignoredChild (const std::vector<AccessibilityHandler*>& handlers)
{
    if (! handlers.empty())
    {
        const auto iter = std::find_if (handlers.cbegin(), handlers.cend(),
                                        [] (const AccessibilityHandler* handler) { return ! handler->isIgnored() && handler->isVisibleWithinParent(); });

        if (iter != handlers.cend())
            return *iter;

        for (auto* handler : handlers)
            if (auto* unignored = findFirstUnignoredChild (handler->getChildren()))
                return unignored;
    }

    return nullptr;
}

static AccessibilityHandler* getFirstUnignoredDescendant (AccessibilityHandler* handler)
{
    if (handler != nullptr && (handler->isIgnored() || ! handler->isVisibleWithinParent()))
        return findFirstUnignoredChild (handler->getChildren());

    return handler;
}

AccessibilityHandler* AccessibilityHandler::getParent() const
{
    if (auto* focusContainer = component.findFocusContainer())
        return getUnignoredAncestor (findEnclosingHandler (focusContainer));

    return nullptr;
}

std::vector<AccessibilityHandler*> AccessibilityHandler::getChildren() const
{
    if (! component.isFocusContainer() && component.getParentComponent() != nullptr)
        return {};

    const auto addChildComponentHandler = [this] (Component* focusableComponent,
                                                  std::vector<AccessibilityHandler*>& childHandlers)
    {
        if (focusableComponent == nullptr)
            return;

        if (auto* handler = findEnclosingHandler (focusableComponent))
        {
            if (! handler->getCurrentState().isFocusable() || ! isParentOf (handler))
                return;

            if (auto* unignored = getFirstUnignoredDescendant (handler))
                if (std::find (childHandlers.cbegin(), childHandlers.cend(), unignored) == childHandlers.cend())
                    childHandlers.push_back (unignored);
        }
    };

    std::vector<AccessibilityHandler*> children;

    if (auto traverser = component.createFocusTraverser())
    {
        addChildComponentHandler (traverser->getDefaultComponent (&component), children);

        for (auto* focusableChild : traverser->getAllComponents (&component))
            addChildComponentHandler (focusableChild, children);
    }

    return children;
}

b8 AccessibilityHandler::isParentOf (const AccessibilityHandler* possibleChild) const noexcept
{
    while (possibleChild != nullptr)
    {
        possibleChild = possibleChild->getParent();

        if (possibleChild == this)
            return true;
    }

    return false;
}

AccessibilityHandler* AccessibilityHandler::getChildAt (Point<i32> screenPoint)
{
    if (auto* comp = Desktop::getInstance().findComponentAt (screenPoint))
    {
        if (auto* handler = getUnignoredAncestor (findEnclosingHandler (comp)))
            if (isParentOf (handler))
                return handler;
    }

    return nullptr;
}

AccessibilityHandler* AccessibilityHandler::getChildFocus()
{
    return hasFocus (true) ? getUnignoredAncestor (currentlyFocusedHandler)
                           : nullptr;
}

b8 AccessibilityHandler::hasFocus (b8 trueIfChildFocused) const
{
    return currentlyFocusedHandler != nullptr
            && (currentlyFocusedHandler == this
                || (trueIfChildFocused && isParentOf (currentlyFocusedHandler)));
}

z0 AccessibilityHandler::grabFocus()
{
    if (! hasFocus (false))
        grabFocusInternal (true);
}

z0 AccessibilityHandler::giveAwayFocus() const
{
    if (hasFocus (true))
        giveAwayFocusInternal();
}

z0 AccessibilityHandler::grabFocusInternal (b8 canTryParent)
{
    if (getCurrentState().isFocusable() && ! isIgnored())
    {
        takeFocus();
        return;
    }

    if (isParentOf (currentlyFocusedHandler))
        return;

    if (auto traverser = component.createFocusTraverser())
    {
        if (auto* defaultComp = traverser->getDefaultComponent (&component))
        {
            if (auto* handler = getUnignoredAncestor (findEnclosingHandler (defaultComp)))
            {
                if (isParentOf (handler))
                {
                    handler->grabFocusInternal (false);
                    return;
                }
            }
        }
    }

    if (canTryParent)
        if (auto* parent = getParent())
            parent->grabFocusInternal (true);
}

z0 AccessibilityHandler::giveAwayFocusInternal() const
{
    currentlyFocusedHandler = nullptr;
    detail::AccessibilityHelpers::notifyAccessibilityEvent (*this, detail::AccessibilityHelpers::Event::focusChanged);
}

z0 AccessibilityHandler::takeFocus()
{
    currentlyFocusedHandler = this;
    detail::AccessibilityHelpers::notifyAccessibilityEvent (*this, detail::AccessibilityHelpers::Event::focusChanged);

    if ((component.isShowing() || component.isOnDesktop())
        && component.getWantsKeyboardFocus()
        && ! component.hasKeyboardFocus (true))
    {
        component.grabKeyboardFocus();
    }
}

std::unique_ptr<AccessibilityHandler::AccessibilityNativeImpl> AccessibilityHandler::createNativeImpl (AccessibilityHandler& handler)
{
   #if DRX_NATIVE_ACCESSIBILITY_INCLUDED
    return std::make_unique<AccessibilityNativeImpl> (handler);
   #else
    ignoreUnused (handler);
    return nullptr;
   #endif
}

uk AccessibilityHandler::getNativeChildForComponent (Component& component)
{
    return NativeChildHandler::getInstance().getNativeChild (component);
}

Component* AccessibilityHandler::getComponentForNativeChild (uk nativeChild)
{
    return NativeChildHandler::getInstance().getComponent (nativeChild);
}

z0 AccessibilityHandler::setNativeChildForComponent (Component& component, uk nativeChild)
{
    NativeChildHandler::getInstance().setNativeChild (component, nativeChild);
}

#if DRX_MODULE_AVAILABLE_drx_gui_extra
z0 privatePostSystemNotification (const Txt&, const Txt&);
#endif

z0 AccessibilityHandler::postSystemNotification ([[maybe_unused]] const Txt& notificationTitle,
                                                   [[maybe_unused]] const Txt& notificationBody)
{
   #if DRX_MODULE_AVAILABLE_drx_gui_extra
    if (areAnyAccessibilityClientsActive())
        privatePostSystemNotification (notificationTitle, notificationBody);
   #endif
}

#if ! DRX_NATIVE_ACCESSIBILITY_INCLUDED
 z0 AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent) const {}
 z0 AccessibilityHandler::postAnnouncement (const Txt&, AnnouncementPriority) {}
 AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const { return nullptr; }
 b8 AccessibilityHandler::areAnyAccessibilityClientsActive() { return false; }
#endif

} // namespace drx
