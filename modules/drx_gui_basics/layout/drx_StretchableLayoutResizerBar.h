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
    A component that acts as one of the vertical or horizontal bars you see being
    used to resize panels in a window.

    One of these acts with a StretchableLayoutManager to resize the other components.

    @see StretchableLayoutManager

    @tags{GUI}
*/
class DRX_API  StretchableLayoutResizerBar  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer bar for use on a specified layout.

        @param layoutToUse          the layout that will be affected when this bar
                                    is dragged
        @param itemIndexInLayout    the item index in the layout that corresponds to
                                    this bar component. You'll need to set up the item
                                    properties in a suitable way for a divider bar, e.g.
                                    for an 8-pixel wide bar which, you could call
                                    myLayout->setItemLayout (barIndex, 8, 8, 8)
        @param isBarVertical        true if it's an upright bar that you drag left and
                                    right; false for a horizontal one that you drag up and
                                    down
    */
    StretchableLayoutResizerBar (StretchableLayoutManager* layoutToUse,
                                 i32 itemIndexInLayout,
                                 b8 isBarVertical);

    /** Destructor. */
    ~StretchableLayoutResizerBar() override;

    //==============================================================================
    /** This is called when the bar is dragged.

        This method must update the positions of any components whose position is
        determined by the StretchableLayoutManager, because they might have just
        moved.

        The default implementation calls the resized() method of this component's
        parent component, because that's often where you're likely to apply the
        layout, but it can be overridden for more specific needs.
    */
    virtual z0 hasBeenMoved();

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawStretchableLayoutResizerBar (Graphics&, i32 w, i32 h,
                                                      b8 isVerticalBar, b8 isMouseOver, b8 isMouseDragging) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;


private:
    //==============================================================================
    StretchableLayoutManager* layout;
    i32 itemIndex, mouseDownPos;
    b8 isVertical;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StretchableLayoutResizerBar)
};

} // namespace drx
