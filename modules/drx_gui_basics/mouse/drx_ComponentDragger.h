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
    An object to take care of the logic for dragging components around with the mouse.

    Very easy to use - in your mouseDown() callback, call startDraggingComponent(),
    then in your mouseDrag() callback, call dragComponent().

    When starting a drag, you can give it a ComponentBoundsConstrainer to use
    to limit the component's position and keep it on-screen.

    e.g. @code
    class MyDraggableComp
    {
        ComponentDragger myDragger;

        z0 mouseDown (const MouseEvent& e)
        {
            myDragger.startDraggingComponent (this, e);
        }

        z0 mouseDrag (const MouseEvent& e)
        {
            myDragger.dragComponent (this, e, nullptr);
        }
    };
    @endcode

    @tags{GUI}
*/
class DRX_API  ComponentDragger
{
public:
    //==============================================================================
    /** Creates a ComponentDragger. */
    ComponentDragger();

    /** Destructor. */
    virtual ~ComponentDragger();

    //==============================================================================
    /** Call this from your component's mouseDown() method, to prepare for dragging.

        @param componentToDrag      the component that you want to drag
        @param e                    the mouse event that is triggering the drag
        @see dragComponent
    */
    z0 startDraggingComponent (Component* componentToDrag,
                                 const MouseEvent& e);

    /** Call this from your mouseDrag() callback to move the component.

        This will move the component, using the given constrainer object to check
        the new position.

        @param componentToDrag      the component that you want to drag
        @param e                    the current mouse-drag event
        @param constrainer          an optional constrainer object that should be used
                                    to apply limits to the component's position. Pass
                                    null if you don't want to constrain the movement.
        @see startDraggingComponent
    */
    z0 dragComponent (Component* componentToDrag,
                        const MouseEvent& e,
                        ComponentBoundsConstrainer* constrainer);

private:
    //==============================================================================
    Point<i32> mouseDownWithinTarget;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentDragger)
};

} // namespace drx
