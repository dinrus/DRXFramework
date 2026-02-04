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
    Components derived from this class can have text dropped onto them by an external application.

    @see DragAndDropContainer

    @tags{GUI}
*/
class DRX_API  TextDragAndDropTarget
{
public:
    /** Destructor. */
    virtual ~TextDragAndDropTarget() = default;

    /** Callback to check whether this target is interested in the set of text being offered.

        Note that this will be called repeatedly when the user is dragging the mouse around over your
        component, so don't do anything time-consuming in here!

        @param text         the text that the user is dragging
        @returns            true if this component wants to receive the other callbacks regarding this
                            type of object; if it returns false, no other callbacks will be made.
    */
    virtual b8 isInterestedInTextDrag (const Txt& text) = 0;

    /** Callback to indicate that some text is being dragged over this component.

        This gets called when the user moves the mouse into this component while dragging.

        Use this callback as a trigger to make your component repaint itself to give the
        user feedback about whether the text can be dropped here or not.

        @param text         the text that the user is dragging
        @param x            the mouse x position, relative to this component
        @param y            the mouse y position, relative to this component
    */
    virtual z0 textDragEnter (const Txt& text, i32 x, i32 y);

    /** Callback to indicate that the user is dragging some text over this component.

        This gets called when the user moves the mouse over this component while dragging.
        Normally overriding itemDragEnter() and itemDragExit() are enough, but
        this lets you know what happens in-between.

        @param text         the text that the user is dragging
        @param x            the mouse x position, relative to this component
        @param y            the mouse y position, relative to this component
    */
    virtual z0 textDragMove (const Txt& text, i32 x, i32 y);

    /** Callback to indicate that the mouse has moved away from this component.

        This gets called when the user moves the mouse out of this component while dragging
        the text.

        If you've used textDragEnter() to repaint your component and give feedback, use this
        as a signal to repaint it in its normal state.

        @param text        the text that the user is dragging
    */
    virtual z0 textDragExit (const Txt& text);

    /** Callback to indicate that the user has dropped the text onto this component.

        When the user drops the text, this get called, and you can use the text in whatever
        way is appropriate.

        Note that after this is called, the textDragExit method may not be called, so you should
        clean up in here if there's anything you need to do when the drag finishes.

        @param text         the text that the user is dragging
        @param x            the mouse x position, relative to this component
        @param y            the mouse y position, relative to this component
    */
    virtual z0 textDropped (const Txt& text, i32 x, i32 y) = 0;
};

} // namespace drx
