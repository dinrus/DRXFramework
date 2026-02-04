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

ComponentMovementWatcher::ComponentMovementWatcher (Component* const comp)
    : component (comp),
      wasShowing (comp->isShowing())
{
    jassert (component != nullptr); // can't use this with a null pointer..

    component->addComponentListener (this);
    registerWithParentComps();
}

ComponentMovementWatcher::~ComponentMovementWatcher()
{
    if (component != nullptr)
        component->removeComponentListener (this);

    unregister();
}

//==============================================================================
z0 ComponentMovementWatcher::componentParentHierarchyChanged (Component&)
{
    if (component != nullptr && ! reentrant)
    {
        const ScopedValueSetter<b8> setter (reentrant, true);

        auto* peer = component->getPeer();
        auto peerID = peer != nullptr ? peer->getUniqueID() : 0;

        if (peerID != lastPeerID)
        {
            componentPeerChanged();

            if (component == nullptr)
                return;

            lastPeerID = peerID;
        }

        unregister();
        registerWithParentComps();

        componentMovedOrResized (*component, true, true);

        if (component != nullptr)
            componentVisibilityChanged (*component);
    }
}

z0 ComponentMovementWatcher::componentMovedOrResized (Component&, b8 wasMoved, b8 wasResized)
{
    if (component != nullptr)
    {
        if (wasMoved)
        {
            Point<i32> newPos;
            auto* top = component->getTopLevelComponent();

            if (top != component)
                newPos = top->getLocalPoint (component, Point<i32>());
            else
                newPos = top->getPosition();

            wasMoved = lastBounds.getPosition() != newPos;
            lastBounds.setPosition (newPos);
        }

        wasResized = (lastBounds.getWidth() != component->getWidth() || lastBounds.getHeight() != component->getHeight());
        lastBounds.setSize (component->getWidth(), component->getHeight());

        if (wasMoved || wasResized)
            componentMovedOrResized (wasMoved, wasResized);
    }
}

z0 ComponentMovementWatcher::componentBeingDeleted (Component& comp)
{
    registeredParentComps.removeFirstMatchingValue (&comp);

    if (component == &comp)
        unregister();
}

z0 ComponentMovementWatcher::componentVisibilityChanged (Component&)
{
    if (component != nullptr)
    {
        const b8 isShowingNow = component->isShowing();

        if (wasShowing != isShowingNow)
        {
            wasShowing = isShowingNow;
            componentVisibilityChanged();
        }
    }
}

z0 ComponentMovementWatcher::registerWithParentComps()
{
    for (auto* p = component->getParentComponent(); p != nullptr; p = p->getParentComponent())
    {
        p->addComponentListener (this);
        registeredParentComps.add (p);
    }
}

z0 ComponentMovementWatcher::unregister()
{
    for (auto* c : registeredParentComps)
        c->removeComponentListener (this);

    registeredParentComps.clear();
}

} // namespace drx
