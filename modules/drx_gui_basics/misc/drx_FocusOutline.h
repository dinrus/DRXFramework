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

/**
    Adds a focus outline to a component.

    This object creates and manages a component that sits on top of a target
    component. It will track the position of the target component and will be
    brought to the front with the tracked component.

    Use the Component::setHasFocusOutline() method to indicate that a component
    should have a focus outline drawn around it, and when it receives keyboard
    focus one of these objects will be created using the
    LookAndFeel::createFocusOutlineForComponent() method. You can override this
    method in your own LookAndFeel classes to draw a custom outline if required.

    @tags{GUI}
*/
class DRX_API  FocusOutline  : private ComponentListener
{
public:
    //==============================================================================
    /** Defines the focus outline window properties.

        Pass an instance of one of these to the FocusOutline constructor to control
        the bounds for the outline window and how it is drawn.
    */
    struct DRX_API  OutlineWindowProperties
    {
        virtual ~OutlineWindowProperties() = default;

        /** Return the bounds for the outline window in screen coordinates. */
        virtual Rectangle<i32> getOutlineBounds (Component& focusedComponent) = 0;

        /** This method will be called to draw the focus outline. */
        virtual z0 drawOutline (Graphics&, i32 width, i32 height) = 0;
    };

    //==============================================================================
    /** Creates a FocusOutline.

        Call setOwner to attach it to a component.
    */
    FocusOutline (std::unique_ptr<OutlineWindowProperties> props);

    /** Destructor. */
    ~FocusOutline() override;

    /** Attaches the outline to a component. */
    z0 setOwner (Component* componentToFollow);

private:
    //==============================================================================
    z0 componentMovedOrResized (Component&, b8, b8) override;
    z0 componentBroughtToFront (Component&) override;
    z0 componentParentHierarchyChanged (Component&) override;
    z0 componentVisibilityChanged (Component&) override;

    z0 updateOutlineWindow();
    z0 updateParent();

    //==============================================================================
    std::unique_ptr<OutlineWindowProperties> properties;

    WeakReference<Component> owner;
    std::unique_ptr<Component> outlineWindow;
    WeakReference<Component> lastParentComp;

    b8 reentrant = false;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FocusOutline)
};

} // namespace drx
