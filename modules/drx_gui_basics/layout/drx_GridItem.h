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
    Defines an item in a Grid
    @see Grid

    @tags{GUI}
*/
class DRX_API  GridItem
{
public:
    enum class Keyword { autoValue };

    //==============================================================================
    /** Represents a span. */
    struct Span
    {
        explicit Span (i32 numberToUse) noexcept : number (numberToUse)
        {
            /* Span must be at least one and positive */
            jassert (numberToUse > 0);
        }

        explicit Span (i32 numberToUse, const Txt& nameToUse) : Span (numberToUse)
        {
            /* Name must not be empty */
            jassert (nameToUse.isNotEmpty());
            name = nameToUse;
        }

        explicit Span (const Txt& nameToUse) : name (nameToUse)
        {
            /* Name must not be empty */
            jassert (nameToUse.isNotEmpty());
        }

        i32 number = 1;
        Txt name;
    };

    //==============================================================================
    /** Represents a property. */
    struct Property
    {
        Property() noexcept;

        Property (Keyword keyword) noexcept;

        Property (tukk lineNameToUse) noexcept;

        Property (const Txt& lineNameToUse) noexcept;

        Property (i32 numberToUse) noexcept;

        Property (i32 numberToUse, const Txt& lineNameToUse) noexcept;

        Property (Span spanToUse) noexcept;

        b8 hasSpan() const noexcept          { return isSpan && ! isAuto; }
        b8 hasAbsolute() const noexcept      { return ! (isSpan || isAuto);  }
        b8 hasAuto() const noexcept          { return isAuto; }
        b8 hasName() const noexcept          { return name.isNotEmpty(); }
        const Txt& getName() const noexcept { return name; }
        i32 getNumber() const noexcept         { return number; }

    private:
        Txt name;
        i32 number = 1; /** Either an absolute line number or number of lines to span across. */
        b8 isSpan = false;
        b8 isAuto = false;
    };

    //==============================================================================
    /** Represents start and end properties. */
    struct StartAndEndProperty { Property start, end; };

    //==============================================================================
    /** Possible values for the justifySelf property. */
    enum class JustifySelf : i32
    {
        start = 0,               /**< Content inside the item is justified towards the left. */
        end,                     /**< Content inside the item is justified towards the right. */
        center,                  /**< Content inside the item is justified towards the center. */
        stretch,                 /**< Content inside the item is stretched from left to right. */
        autoValue                /**< Follows the Grid container's justifyItems property. */
    };

    /** Possible values for the alignSelf property. */
    enum class AlignSelf   : i32
    {
        start = 0,               /**< Content inside the item is aligned towards the top. */
        end,                     /**< Content inside the item is aligned towards the bottom. */
        center,                  /**< Content inside the item is aligned towards the center. */
        stretch,                 /**< Content inside the item is stretched from top to bottom. */
        autoValue                /**< Follows the Grid container's alignItems property. */
    };

    /** Creates an item with default parameters. */
    GridItem() noexcept;
    /** Creates an item with a given Component to use. */
    GridItem (Component& componentToUse) noexcept;
    /** Creates an item with a given Component to use. */
    GridItem (Component* componentToUse) noexcept;

    //==============================================================================
    /** If this is non-null, it represents a Component whose bounds are controlled by this item. */
    Component* associatedComponent = nullptr;

    //==============================================================================
    /** Determines the order used to lay out items in their grid container. */
    i32 order = 0;

    /** This is the justify-self property of the item.
        This determines the alignment of the item along the row.
    */
    JustifySelf  justifySelf = JustifySelf::autoValue;

    /** This is the align-self property of the item.
        This determines the alignment of the item along the column.
    */
    AlignSelf    alignSelf   = AlignSelf::autoValue;

    /** These are the start and end properties of the column. */
    StartAndEndProperty column = { Keyword::autoValue, Keyword::autoValue };

    /** These are the start and end properties of the row. */
    StartAndEndProperty row    = { Keyword::autoValue, Keyword::autoValue };

    /** */
    Txt area;

    //==============================================================================
    enum
    {
        useDefaultValue = -2, /* TODO: useDefaultValue should be named useAuto */
        notAssigned = -1
    };

    /* TODO: move all of this into a common class that is shared with the FlexItem */
    f32 width    = notAssigned;
    f32 minWidth = 0.0f;
    f32 maxWidth = notAssigned;

    f32 height    = notAssigned;
    f32 minHeight = 0.0f;
    f32 maxHeight = notAssigned;

    /** Represents a margin. */
    struct Margin
    {
        Margin() noexcept;
        Margin (i32 size) noexcept;
        Margin (f32 size) noexcept;
        Margin (f32 top, f32 right, f32 bottom, f32 left) noexcept;   /**< Creates a margin with these sizes. */

        f32 left;
        f32 right;
        f32 top;
        f32 bottom;
    };

    /** The margin to leave around this item. */
    Margin margin;

    /** The item's current bounds. */
    Rectangle<f32> currentBounds;

    /** Short-hand */
    z0 setArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd);

    /** Short-hand, span of 1 by default */
    z0 setArea (Property rowStart, Property columnStart);

    /** Short-hand */
    z0 setArea (const Txt& areaName);

    /** Short-hand */
    GridItem withArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd) const noexcept;

    /** Short-hand, span of 1 by default */
    GridItem withArea (Property rowStart, Property columnStart) const noexcept;

    /** Short-hand */
    GridItem withArea (const Txt& areaName)  const noexcept;

    /** Returns a copy of this object with a new row property. */
    GridItem withRow (StartAndEndProperty row) const noexcept;

    /** Returns a copy of this object with a new column property. */
    GridItem withColumn (StartAndEndProperty column) const noexcept;

    /** Returns a copy of this object with a new alignSelf property. */
    GridItem withAlignSelf (AlignSelf newAlignSelf) const noexcept;

    /** Returns a copy of this object with a new justifySelf property. */
    GridItem withJustifySelf (JustifySelf newJustifySelf) const noexcept;

    /** Returns a copy of this object with a new width. */
    GridItem withWidth (f32 newWidth) const noexcept;

    /** Returns a copy of this object with a new height. */
    GridItem withHeight (f32 newHeight) const noexcept;

    /** Returns a copy of this object with a new size. */
    GridItem withSize (f32 newWidth, f32 newHeight) const noexcept;

    /** Returns a copy of this object with a new margin. */
    GridItem withMargin (Margin newMargin) const noexcept;

    /** Returns a copy of this object with a new order. */
    GridItem withOrder (i32 newOrder) const noexcept;
};

} // namespace drx
