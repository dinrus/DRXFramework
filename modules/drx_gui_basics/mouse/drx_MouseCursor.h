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
    Represents a mouse cursor image.

    This object can either be used to represent one of the standard mouse
    cursor shapes, or a custom one generated from an image.

    @tags{GUI}
*/
class DRX_API  MouseCursor  final
{
public:
    //==============================================================================
    /** The set of available standard mouse cursors. */
    enum StandardCursorType
    {
        ParentCursor = 0,               /**< Indicates that the component's parent's cursor should be used. */

        NoCursor,                       /**< An invisible cursor. */
        NormalCursor,                   /**< The standard arrow cursor. */

        WaitCursor,                     /**< The normal hourglass or spinning-beachball 'busy' cursor. */
        IBeamCursor,                    /**< A vertical I-beam for positioning within text. */
        CrosshairCursor,                /**< A pair of crosshairs. */
        CopyingCursor,                  /**< The normal arrow cursor, but with a "+" on it to indicate
                                             that you're dragging a copy of something. */

        PointingHandCursor,             /**< A hand with a pointing finger, for clicking on web-links. */
        DraggingHandCursor,             /**< An open flat hand for dragging heavy objects around. */

        LeftRightResizeCursor,          /**< An arrow pointing left and right. */
        UpDownResizeCursor,             /**< an arrow pointing up and down. */
        UpDownLeftRightResizeCursor,    /**< An arrow pointing up, down, left and right. */

        TopEdgeResizeCursor,            /**< A platform-specific cursor for resizing the top-edge of a window. */
        BottomEdgeResizeCursor,         /**< A platform-specific cursor for resizing the bottom-edge of a window. */
        LeftEdgeResizeCursor,           /**< A platform-specific cursor for resizing the left-edge of a window. */
        RightEdgeResizeCursor,          /**< A platform-specific cursor for resizing the right-edge of a window. */
        TopLeftCornerResizeCursor,      /**< A platform-specific cursor for resizing the top-left-corner of a window. */
        TopRightCornerResizeCursor,     /**< A platform-specific cursor for resizing the top-right-corner of a window. */
        BottomLeftCornerResizeCursor,   /**< A platform-specific cursor for resizing the bottom-left-corner of a window. */
        BottomRightCornerResizeCursor,  /**< A platform-specific cursor for resizing the bottom-right-corner of a window. */

        NumStandardCursorTypes
    };

    //==============================================================================
    /** Creates the standard arrow cursor. */
    MouseCursor() noexcept;

    /** Creates one of the standard mouse cursor */
    MouseCursor (StandardCursorType);

    /** Creates a custom cursor from an image.

        @param image    the image to use for the cursor - if this is bigger than the
                        system can manage, it might get scaled down first, and might
                        also have to be turned to black-and-white if it can't do colour
                        cursors.
        @param hotSpotX the x position of the cursor's hotspot within the image
        @param hotSpotY the y position of the cursor's hotspot within the image
    */
    MouseCursor (const Image& image, i32 hotSpotX, i32 hotSpotY);

    /** Creates a custom cursor from an image.

        @param image    the image to use for the cursor - if this is bigger than the
                        system can manage, it might get scaled down first, and might
                        also have to be turned to black-and-white if it can't do colour
                        cursors.
        @param hotSpotX the x position of the cursor's hotspot within the image
        @param hotSpotY the y position of the cursor's hotspot within the image
        @param scaleFactor the factor by which this image is larger than the target
                           screen size of the cursor.
    */
    MouseCursor (const Image& image, i32 hotSpotX, i32 hotSpotY, f32 scaleFactor);

    /** Creates a custom cursor from an image.

        @param image    the image to use for the cursor - if this is bigger than the
                        system can manage, it might get scaled down first, and might
                        also have to be turned to black-and-white if it can't do colour
                        cursors.
        @param hotSpot the position of the cursor's hotspot within the image
    */
    MouseCursor (const ScaledImage& image, Point<i32> hotSpot);

    //==============================================================================
    /** Creates a copy of another cursor object. */
    MouseCursor (const MouseCursor&);

    /** Copies this cursor from another object. */
    MouseCursor& operator= (const MouseCursor&);

    /** Destructor. */
    ~MouseCursor();

    /** Move constructor */
    MouseCursor (MouseCursor&&) noexcept;

    /** Move assignment operator */
    MouseCursor& operator= (MouseCursor&&) noexcept;

    /** Checks whether two mouse cursors are the same.

        For custom cursors, two cursors created from the same image won't be
        recognised as the same, only MouseCursor objects that have been
        copied from the same object.
    */
    b8 operator== (const MouseCursor&) const noexcept;

    /** Checks whether two mouse cursors are the same.

        For custom cursors, two cursors created from the same image won't be
        recognised as the same, only MouseCursor objects that have been
        copied from the same object.
    */
    b8 operator!= (const MouseCursor&) const noexcept;

    /** Checks whether this cursor is of the standard type mentioned. */
    b8 operator== (StandardCursorType type) const noexcept;

    /** Checks whether this cursor is of the standard type mentioned. */
    b8 operator!= (StandardCursorType type) const noexcept;

    //==============================================================================
    /** Makes the system show its default 'busy' cursor.

        This will turn the system cursor to an hourglass or spinning beachball
        until the next time the mouse is moved, or hideWaitCursor() is called.

        This is handy if the message loop is about to block for a couple of
        seconds while busy and you want to give the user feedback about this.
    */
    static z0 showWaitCursor();

    /** If showWaitCursor has been called, this will return the mouse to its
        normal state.

        This will look at what component is under the mouse, and update the
        cursor to be the correct one for that component.

        @see showWaitCursor
    */
    static z0 hideWaitCursor();


private:
    //==============================================================================
    class SharedCursorHandle;
    std::shared_ptr<SharedCursorHandle> cursorHandle;

    class PlatformSpecificHandle;

    friend class detail::MouseInputSourceImpl;
    z0 showInWindow (ComponentPeer*) const;
    PlatformSpecificHandle* getHandle() const noexcept;

    DRX_LEAK_DETECTOR (MouseCursor)
};

} // namespace drx
