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
    A component that resizes its parent component when dragged.

    This component forms a bar along one edge of a component, allowing it to
    be dragged by that edges to resize it.

    To use it, just add it to your component, positioning it along the appropriate
    edge. Make sure you reposition the resizer component each time the parent's size
    changes, to keep it in the correct position.

    @see ResizableBorderComponent, ResizableCornerComponent

    @tags{GUI}
*/
class DRX_API  ResizableEdgeComponent  : public Component
{
public:
    //==============================================================================
    enum Edge
    {
        leftEdge,   /**< Indicates a vertical bar that can be dragged left/right to move the component's left-hand edge. */
        rightEdge,  /**< Indicates a vertical bar that can be dragged left/right to move the component's right-hand edge. */
        topEdge,    /**< Indicates a horizontal bar that can be dragged up/down to move the top of the component. */
        bottomEdge  /**< Indicates a horizontal bar that can be dragged up/down to move the bottom of the component. */
    };

    /** Creates a resizer bar.

        Pass in the target component which you want to be resized when this one is
        dragged. The target component will usually be this component's parent, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If the constrainer parameter is not a nullptr, then this object will be used to
        enforce limits on the size and position that the component can be stretched to.
        Make sure that the constrainer isn't deleted while still in use by this object.

        @see ComponentBoundsConstrainer
    */
    ResizableEdgeComponent (Component* componentToResize,
                            ComponentBoundsConstrainer* constrainer,
                            Edge edgeToResize);

    /** Destructor. */
    ~ResizableEdgeComponent() override;

    b8 isVertical() const noexcept;

protected:
    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;

private:
    WeakReference<Component> component;
    ComponentBoundsConstrainer* constrainer;
    Rectangle<i32> originalBounds;
    const Edge edge;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableEdgeComponent)
};

} // namespace drx
