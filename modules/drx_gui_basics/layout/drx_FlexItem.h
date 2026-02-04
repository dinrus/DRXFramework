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
    Describes the properties of an item inside a FlexBox container.

    @see FlexBox

    @tags{GUI}
*/
class DRX_API  FlexItem  final
{
public:
    //==============================================================================
    /** Creates an item with default parameters, and zero size. */
    FlexItem() noexcept;

    /** Creates an item with the given size. */
    FlexItem (f32 width, f32 height) noexcept;

    /** Creates an item with the given size and target Component. */
    FlexItem (f32 width, f32 height, Component& targetComponent) noexcept;

    /** Creates an item that represents an embedded FlexBox with a given size. */
    FlexItem (f32 width, f32 height, FlexBox& flexBoxToControl) noexcept;

    /** Creates an item with a given target Component. */
    FlexItem (Component& componentToControl) noexcept;

    /** Creates an item that represents an embedded FlexBox. This class will not
        create a copy of the supplied flex box. You need to ensure that the
        life-time of flexBoxToControl is longer than the FlexItem. */
    FlexItem (FlexBox& flexBoxToControl) noexcept;

    //==============================================================================
    /** The item's current bounds. */
    Rectangle<f32> currentBounds;

    /** If this is non-null, it represents a Component whose bounds are controlled by this item. */
    Component* associatedComponent = nullptr;

    /** If this is non-null, it represents a FlexBox whose bounds are controlled by this item. */
    FlexBox* associatedFlexBox = nullptr;

    /** Determines the order used to lay out items in their flex container.
        Elements are laid out in ascending order of thus order value. Elements with the same order value
        are laid out in the order in which they appear in the array.
    */
    i32 order = 0;

    /** Specifies the flex grow factor of this item.
        This indicates the amount of space inside the flex container the item should take up.
    */
    f32 flexGrow = 0.0f;

    /** Specifies the flex shrink factor of the item.
        This indicates the rate at which the item shrinks if there is insufficient space in
        the container.
    */
    f32 flexShrink = 1.0f;

    /** Specifies the flex-basis of the item.
        This is the initial main size of a flex item in the direction of flow. It determines the size
        of the content-box unless specified otherwise using box-sizing.
    */
    f32 flexBasis = 0.0f;

    /** Possible value for the alignSelf property */
    enum class AlignSelf
    {
        autoAlign,       /**< Follows the FlexBox container's alignItems property. */
        flexStart,       /**< Item is aligned towards the start of the cross axis. */
        flexEnd,         /**< Item is aligned towards the end of the cross axis. */
        center,          /**< Item is aligned towards the center of the cross axis. */
        stretch          /**< Item is stretched from start to end of the cross axis. */
    };

    /** This is the align-self property of the item.
        This determines the alignment of the item along the cross-axis (perpendicular to the direction
        of flow).
    */
    AlignSelf alignSelf = AlignSelf::autoAlign;

    //==============================================================================
    /** This constant can be used for sizes to indicate that 'auto' mode should be used. */
    static i32k autoValue    = -2;
    /** This constant can be used for sizes to indicate that no value has been set. */
    static i32k notAssigned  = -1;

    f32 width     = (f32) notAssigned;  /**< The item's width. */
    f32 minWidth  = 0.0f;                 /**< The item's minimum width */
    f32 maxWidth  = (f32) notAssigned;  /**< The item's maximum width */

    f32 height    = (f32) notAssigned;  /**< The item's height */
    f32 minHeight = 0.0f;                 /**< The item's minimum height */
    f32 maxHeight = (f32) notAssigned;  /**< The item's maximum height */

    /** Represents a margin. */
    struct Margin  final
    {
        Margin() noexcept;              /**< Creates a margin of size zero. */
        Margin (f32 size) noexcept;   /**< Creates a margin with this size on all sides. */
        Margin (f32 top, f32 right, f32 bottom, f32 left) noexcept;   /**< Creates a margin with these sizes. */

        f32 left;   /**< Left margin size */
        f32 right;  /**< Right margin size */
        f32 top;    /**< Top margin size */
        f32 bottom; /**< Bottom margin size */
    };

    /** The margin to leave around this item. */
    Margin margin;

    //==============================================================================
    /** Returns a copy of this object with a new flex-grow value. */
    FlexItem withFlex (f32 newFlexGrow) const noexcept;

    /** Returns a copy of this object with new flex-grow and flex-shrink values. */
    FlexItem withFlex (f32 newFlexGrow, f32 newFlexShrink) const noexcept;

    /** Returns a copy of this object with new flex-grow, flex-shrink and flex-basis values. */
    FlexItem withFlex (f32 newFlexGrow, f32 newFlexShrink, f32 newFlexBasis) const noexcept;

    /** Returns a copy of this object with a new width. */
    FlexItem withWidth (f32 newWidth) const noexcept;

    /** Returns a copy of this object with a new minimum width. */
    FlexItem withMinWidth (f32 newMinWidth) const noexcept;

    /** Returns a copy of this object with a new maximum width. */
    FlexItem withMaxWidth (f32 newMaxWidth) const noexcept;

    /** Returns a copy of this object with a new height. */
    FlexItem withHeight (f32 newHeight) const noexcept;

    /** Returns a copy of this object with a new minimum height. */
    FlexItem withMinHeight (f32 newMinHeight) const noexcept;

    /** Returns a copy of this object with a new maximum height. */
    FlexItem withMaxHeight (f32 newMaxHeight) const noexcept;

    /** Returns a copy of this object with a new margin. */
    FlexItem withMargin (Margin) const noexcept;

    /** Returns a copy of this object with a new order. */
    FlexItem withOrder (i32 newOrder) const noexcept;

    /** Returns a copy of this object with a new alignSelf value. */
    FlexItem withAlignSelf (AlignSelf newAlignSelf) const noexcept;
};

} // namespace drx
