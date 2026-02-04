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
    Adds a drop-shadow to a component.

    This object creates and manages a set of components which sit around a
    component, creating a gaussian shadow around it. The components will track
    the position of the component and if it's brought to the front they'll also
    follow this.

    For desktop windows you don't need to use this class directly - just
    set the Component::windowHasDropShadow flag when calling
    Component::addToDesktop(), and the system will create one of these if it's
    needed (which it obviously isn't on the Mac, for example).

    @tags{GUI}
*/
class DRX_API  DropShadower  : private ComponentListener
{
public:
    //==============================================================================
    /** Creates a DropShadower. */
    DropShadower (const DropShadow& shadowType);

    /** Destructor. */
    ~DropShadower() override;

    /** Attaches the DropShadower to the component you want to shadow. */
    z0 setOwner (Component* componentToFollow);

private:
    //==============================================================================
    z0 componentMovedOrResized (Component&, b8, b8) override;
    z0 componentBroughtToFront (Component&) override;
    z0 componentChildrenChanged (Component&) override;
    z0 componentParentHierarchyChanged (Component&) override;
    z0 componentVisibilityChanged (Component&) override;

    z0 updateParent();
    z0 updateShadows();

    class ShadowWindow;

    WeakReference<Component> owner;
    OwnedArray<Component> shadowWindows;
    DropShadow shadow;
    b8 reentrant = false;
    WeakReference<Component> lastParentComp;

    class ParentVisibilityChangedListener;
    std::unique_ptr<ParentVisibilityChangedListener> visibilityChangedListener;

    class VirtualDesktopWatcher;
    std::unique_ptr<VirtualDesktopWatcher> virtualDesktopWatcher;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DropShadower)
    DRX_DECLARE_WEAK_REFERENCEABLE (DropShadower)
};

} // namespace drx
