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
/** A component that resizes a parent component when dragged.

    This is the small triangular stripey resizer component you get in the bottom-right
    of windows (more commonly on the Mac than Windows). Put one in the corner of
    a larger component and it will automatically resize its parent when it gets dragged
    around.

    @see ResizableBorderComponent

    @tags{GUI}
*/
class DRX_API  ResizableCornerComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer.

        Pass in the target component which you want to be resized when this one is
        dragged.

        The target component will usually be a parent of the resizer component, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If a constrainer object is provided, then this object will be used to enforce
        limits on the size and position that the component can be stretched to. Make sure
        that the constrainer isn't deleted while still in use by this object. If you
        pass a nullptr in here, no limits will be put on the sizes it can be stretched to.

        @see ComponentBoundsConstrainer
    */
    ResizableCornerComponent (Component* componentToResize,
                              ComponentBoundsConstrainer* constrainer);

    /** Destructor. */
    ~ResizableCornerComponent() override;


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
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;

private:
    //==============================================================================
    WeakReference<Component> component;
    ComponentBoundsConstrainer* constrainer;
    Rectangle<i32> originalBounds;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableCornerComponent)
};

} // namespace drx
