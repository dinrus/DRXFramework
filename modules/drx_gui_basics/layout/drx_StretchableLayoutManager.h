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
    For laying out a set of components, where the components have preferred sizes
    and size limits, but where they are allowed to stretch to fill the available
    space.

    For example, if you have a component containing several other components, and
    each one should be given a share of the total size, you could use one of these
    to resize the child components when the parent component is resized. Then
    you could add a StretchableLayoutResizerBar to easily let the user rescale them.

    A StretchableLayoutManager operates only in one dimension, so if you have a set
    of components stacked vertically on top of each other, you'd use one to manage their
    heights. To build up complex arrangements of components, e.g. for applications
    with multiple nested panels, you would use more than one StretchableLayoutManager.
    E.g. by using two (one vertical, one horizontal), you could create a resizable
    spreadsheet-style table.

    E.g.
    @code
    class MyComp  : public Component
    {
        StretchableLayoutManager myLayout;

        MyComp()
        {
            myLayout.setItemLayout (0,          // for item 0
                                    50, 100,    // must be between 50 and 100 pixels in size
                                    -0.6);      // and its preferred size is 60% of the total available space

            myLayout.setItemLayout (1,          // for item 1
                                    -0.2, -0.6, // size must be between 20% and 60% of the available space
                                    50);        // and its preferred size is 50 pixels
        }

        z0 resized()
        {
            // make a list of two of our child components that we want to reposition
            Component* comps[] = { myComp1, myComp2 };

            // this will position the 2 components, one above the other, to fit
            // vertically into the rectangle provided.
            myLayout.layOutComponents (comps, 2,
                                       0, 0, getWidth(), getHeight(),
                                       true);
        }
    };
    @endcode

    @see StretchableLayoutResizerBar

    @tags{GUI}
*/
class DRX_API  StretchableLayoutManager
{
public:
    //==============================================================================
    /** Creates an empty layout.

        You'll need to add some item properties to the layout before it can be used
        to resize things - see setItemLayout().
    */
    StretchableLayoutManager();

    /** Destructor. */
    ~StretchableLayoutManager();

    //==============================================================================
    /** For a numbered item, this sets its size limits and preferred size.

        @param itemIndex        the index of the item to change.
        @param minimumSize      the minimum size that this item is allowed to be - a positive number
                                indicates an absolute size in pixels. A negative number indicates a
                                proportion of the available space (e.g -0.5 is 50%)
        @param maximumSize      the maximum size that this item is allowed to be - a positive number
                                indicates an absolute size in pixels. A negative number indicates a
                                proportion of the available space
        @param preferredSize    the size that this item would like to be, if there's enough room. A
                                positive number indicates an absolute size in pixels. A negative number
                                indicates a proportion of the available space
        @see getItemLayout
    */
    z0 setItemLayout (i32 itemIndex,
                        f64 minimumSize,
                        f64 maximumSize,
                        f64 preferredSize);

    /** For a numbered item, this returns its size limits and preferred size.

        @param itemIndex        the index of the item.
        @param minimumSize      the minimum size that this item is allowed to be - a positive number
                                indicates an absolute size in pixels. A negative number indicates a
                                proportion of the available space (e.g -0.5 is 50%)
        @param maximumSize      the maximum size that this item is allowed to be - a positive number
                                indicates an absolute size in pixels. A negative number indicates a
                                proportion of the available space
        @param preferredSize    the size that this item would like to be, if there's enough room. A
                                positive number indicates an absolute size in pixels. A negative number
                                indicates a proportion of the available space
        @returns false if the item's properties hadn't been set
        @see setItemLayout
    */
    b8 getItemLayout (i32 itemIndex,
                        f64& minimumSize,
                        f64& maximumSize,
                        f64& preferredSize) const;

    /** Clears all the properties that have been set with setItemLayout() and resets
        this object to its initial state.
    */
    z0 clearAllItems();

    //==============================================================================
    /** Takes a set of components that correspond to the layout's items, and positions
        them to fill a space.

        This will try to give each item its preferred size, whether that's a relative size
        or an absolute one.

        @param components       an array of components that correspond to each of the
                                numbered items that the StretchableLayoutManager object
                                has been told about with setItemLayout()
        @param numComponents    the number of components in the array that is passed-in. This
                                should be the same as the number of items this object has been
                                told about.
        @param x                the left of the rectangle in which the components should
                                be laid out
        @param y                the top of the rectangle in which the components should
                                be laid out
        @param width            the width of the rectangle in which the components should
                                be laid out
        @param height           the height of the rectangle in which the components should
                                be laid out
        @param vertically       if true, the components will be positioned in a vertical stack,
                                so that they fill the height of the rectangle. If false, they
                                will be placed side-by-side in a horizontal line, filling the
                                available width
        @param resizeOtherDimension     if true, this means that the components will have their
                                other dimension resized to fit the space - i.e. if the 'vertically'
                                parameter is true, their x-positions and widths are adjusted to fit
                                the x and width parameters; if 'vertically' is false, their y-positions
                                and heights are adjusted to fit the y and height parameters.
    */
    z0 layOutComponents (Component** components,
                           i32 numComponents,
                           i32 x, i32 y, i32 width, i32 height,
                           b8 vertically,
                           b8 resizeOtherDimension);

    //==============================================================================
    /** Returns the current position of one of the items.

        This is only a valid call after layOutComponents() has been called, as it
        returns the last position that this item was placed at. If the layout was
        vertical, the value returned will be the y position of the top of the item,
        relative to the top of the rectangle in which the items were placed (so for
        example, item 0 will always have position of 0, even in the rectangle passed
        in to layOutComponents() wasn't at y = 0). If the layout was done horizontally,
        the position returned is the item's left-hand position, again relative to the
        x position of the rectangle used.

        @see getItemCurrentSize, setItemPosition
    */
    i32 getItemCurrentPosition (i32 itemIndex) const;

    /** Returns the current size of one of the items.

        This is only meaningful after layOutComponents() has been called, as it
        returns the last size that this item was given. If the layout was done
        vertically, it'll return the item's height in pixels; if it was horizontal,
        it'll return its width.

        @see getItemCurrentRelativeSize
    */
    i32 getItemCurrentAbsoluteSize (i32 itemIndex) const;

    /** Returns the current size of one of the items.

        This is only meaningful after layOutComponents() has been called, as it
        returns the last size that this item was given. If the layout was done
        vertically, it'll return a negative value representing the item's height relative
        to the last size used for laying the components out; if the layout was done
        horizontally it'll be the proportion of its width.

        @see getItemCurrentAbsoluteSize
    */
    f64 getItemCurrentRelativeSize (i32 itemIndex) const;

    //==============================================================================
    /** Moves one of the items, shifting along any other items as necessary in
        order to get it to the desired position.

        Calling this method will also update the preferred sizes of the items it
        shuffles along, so that they reflect their new positions.

        (This is the method that a StretchableLayoutResizerBar uses to shift the items
        about when it's dragged).

        @param itemIndex        the item to move
        @param newPosition      the absolute position that you'd like this item to move
                                to. The item might not be able to always reach exactly this position,
                                because other items may have minimum sizes that constrain how
                                far it can go
    */
    z0 setItemPosition (i32 itemIndex,
                          i32 newPosition);


private:
    //==============================================================================
    struct ItemLayoutProperties
    {
        i32 itemIndex;
        i32 currentSize;
        f64 minSize, maxSize, preferredSize;
    };

    OwnedArray<ItemLayoutProperties> items;
    i32 totalSize = 0;

    //==============================================================================
    static i32 sizeToRealSize (f64 size, i32 totalSpace);
    ItemLayoutProperties* getInfoFor (i32 itemIndex) const;
    z0 setTotalSize (i32 newTotalSize);
    i32 fitComponentsIntoSpace (i32 startIndex, i32 endIndex, i32 availableSpace, i32 startPos);
    i32 getMinimumSizeOfItems (i32 startIndex, i32 endIndex) const;
    i32 getMaximumSizeOfItems (i32 startIndex, i32 endIndex) const;
    z0 updatePrefSizesToMatchCurrentPositions();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StretchableLayoutManager)
};

} // namespace drx
