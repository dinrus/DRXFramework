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
/** An object that watches for any movement of a component or any of its parent components.

    This makes it easy to check when a component is moved relative to its top-level
    peer window. The normal Component::moved() method is only called when a component
    moves relative to its immediate parent, and sometimes you want to know if any of
    components higher up the tree have moved (which of course will affect the overall
    position of all their sub-components).

    It also includes a callback that lets you know when the top-level peer is changed.

    This class is used by specialised components like WebBrowserComponent
    because they need to keep their custom windows in the right place and respond to
    changes in the peer.

    @tags{GUI}
*/
class DRX_API  ComponentMovementWatcher    : public ComponentListener
{
public:
    //==============================================================================
    /** Creates a ComponentMovementWatcher to watch a given target component. */
    ComponentMovementWatcher (Component* componentToWatch);

    /** Destructor. */
    ~ComponentMovementWatcher() override;

    //==============================================================================
    /** This callback happens when the component that is being watched is moved
        relative to its top-level peer window, or when it is resized. */
    virtual z0 componentMovedOrResized (b8 wasMoved, b8 wasResized) = 0;

    /** This callback happens when the component's top-level peer is changed. */
    virtual z0 componentPeerChanged() = 0;

    /** This callback happens when the component's visibility state changes, possibly due to
        one of its parents being made visible or invisible.
    */
    virtual z0 componentVisibilityChanged() = 0;

    /** Returns the component that's being watched. */
    Component* getComponent() const noexcept         { return component.get(); }

    //==============================================================================
    /** @internal */
    z0 componentParentHierarchyChanged (Component&) override;
    /** @internal */
    z0 componentMovedOrResized (Component&, b8 wasMoved, b8 wasResized) override;
    /** @internal */
    z0 componentBeingDeleted (Component&) override;
    /** @internal */
    z0 componentVisibilityChanged (Component&) override;

private:
    //==============================================================================
    WeakReference<Component> component;
    u32 lastPeerID = 0;
    Array<Component*> registeredParentComps;
    b8 reentrant = false, wasShowing;
    Rectangle<i32> lastBounds;

    z0 unregister();
    z0 registerWithParentComps();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentMovementWatcher)
};

} // namespace drx
