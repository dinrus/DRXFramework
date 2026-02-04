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
    Container that handles geometry for grid layouts (fixed columns and rows) using a set of declarative rules.

    Implemented from the `CSS Grid Layout` specification as described at:
    https://css-tricks.com/snippets/css/complete-guide-grid/

    @see GridItem

    @tags{GUI}
*/
class DRX_API  Grid  final
{
public:
    //==============================================================================
    /** A size in pixels */
    struct Px  final
    {
        explicit Px (f32 p) : pixels (static_cast<real_t> (p)) { /*sta (p >= 0.0f);*/ }
        explicit Px (i32 p)   : pixels (static_cast<real_t> (p)) { /*sta (p >= 0.0f);*/ }
        explicit constexpr Px (real_t p)        : pixels (p) {}
        explicit constexpr Px (zu64 p) : pixels (static_cast<real_t> (p)) {}

        real_t pixels;
    };

    /** A fractional ratio integer */
    struct Fr  final
    {
        explicit Fr (i32 f) : fraction (static_cast<zu64> (f)) {}
        explicit constexpr Fr (zu64 p) : fraction (p) {}

        zu64 fraction;
    };

    //==============================================================================
    /** Represents a track. */
    struct TrackInfo  final
    {
        /** Creates a track with auto dimension. */
        TrackInfo() noexcept;

        TrackInfo (Px sizeInPixels) noexcept;
        TrackInfo (Fr fractionOfFreeSpace) noexcept;

        TrackInfo (Px sizeInPixels, const Txt& endLineNameToUse) noexcept;
        TrackInfo (Fr fractionOfFreeSpace, const Txt& endLineNameToUse) noexcept;

        TrackInfo (const Txt& startLineNameToUse, Px sizeInPixels) noexcept;
        TrackInfo (const Txt& startLineNameToUse, Fr fractionOfFreeSpace) noexcept;

        TrackInfo (const Txt& startLineNameToUse, Px sizeInPixels, const Txt& endLineNameToUse) noexcept;
        TrackInfo (const Txt& startLineNameToUse, Fr fractionOfFreeSpace, const Txt& endLineNameToUse) noexcept;

        b8 isAuto() const noexcept { return hasKeyword; }
        b8 isFractional() const noexcept { return isFraction; }
        b8 isPixels() const noexcept { return ! isFraction; }
        const Txt& getStartLineName() const noexcept { return startLineName; }
        const Txt& getEndLineName() const noexcept { return endLineName; }

        /** Get the track's size - which might mean an absolute pixels value or a fractional ratio. */
        f32 getSize() const noexcept { return size; }

    private:
        friend class Grid;
        f32 getAbsoluteSize (f32 relativeFractionalUnit) const;

        f32 size = 0; // Either a fraction or an absolute size in pixels
        b8 isFraction = false;
        b8 hasKeyword = false;

        Txt startLineName, endLineName;
    };

    //==============================================================================
    /** Possible values for the justifyItems property. */
    enum class JustifyItems : i32
    {
        start = 0,                /**< Content inside the item is justified towards the left. */
        end,                      /**< Content inside the item is justified towards the right. */
        center,                   /**< Content inside the item is justified towards the center. */
        stretch                   /**< Content inside the item is stretched from left to right. */
    };

    /** Possible values for the alignItems property. */
    enum class AlignItems : i32
    {
        start = 0,                /**< Content inside the item is aligned towards the top. */
        end,                      /**< Content inside the item is aligned towards the bottom. */
        center,                   /**< Content inside the item is aligned towards the center. */
        stretch                   /**< Content inside the item is stretched from top to bottom. */
    };

    /** Possible values for the justifyContent property. */
    enum class JustifyContent
    {
        start,                    /**< Items are justified towards the left of the container. */
        end,                      /**< Items are justified towards the right of the container. */
        center,                   /**< Items are justified towards the center of the container. */
        stretch,                  /**< Items are stretched from left to right of the container. */
        spaceAround,              /**< Items are evenly spaced along the row with spaces between them. */
        spaceBetween,             /**< Items are evenly spaced along the row with spaces around them. */
        spaceEvenly               /**< Items are evenly spaced along the row with even amount of spaces between them. */
    };

    /** Possible values for the alignContent property. */
    enum class AlignContent
    {
        start,                    /**< Items are aligned towards the top of the container. */
        end,                      /**< Items are aligned towards the bottom of the container. */
        center,                   /**< Items are aligned towards the center of the container. */
        stretch,                  /**< Items are stretched from top to bottom of the container. */
        spaceAround,              /**< Items are evenly spaced along the column with spaces between them. */
        spaceBetween,             /**< Items are evenly spaced along the column with spaces around them. */
        spaceEvenly               /**< Items are evenly spaced along the column with even amount of spaces between them. */
    };

    /** Possible values for the autoFlow property. */
    enum class AutoFlow
    {
        row,                      /**< Fills the grid by adding rows of items. */
        column,                   /**< Fills the grid by adding columns of items. */
        rowDense,                 /**< Fills the grid by adding rows of items and attempts to fill in gaps. */
        columnDense               /**< Fills the grid by adding columns of items and attempts to fill in gaps. */
    };


    //==============================================================================
    /** Creates an empty Grid container with default parameters. */
    Grid() = default;

    //==============================================================================
    /** Specifies the alignment of content inside the items along the rows. */
    JustifyItems   justifyItems   = JustifyItems::stretch;

    /** Specifies the alignment of content inside the items along the columns. */
    AlignItems     alignItems     = AlignItems::stretch;

    /** Specifies the alignment of items along the rows. */
    JustifyContent justifyContent = JustifyContent::stretch;

    /** Specifies the alignment of items along the columns. */
    AlignContent   alignContent   = AlignContent::stretch;

    /** Specifies how the auto-placement algorithm places items. */
    AutoFlow       autoFlow       = AutoFlow::row;


    //==============================================================================
    /** The set of column tracks to lay out. */
    Array<TrackInfo> templateColumns;

    /** The set of row tracks to lay out. */
    Array<TrackInfo> templateRows;

    /** Template areas */
    StringArray templateAreas;

    /** The row track for auto dimension. */
    TrackInfo autoRows;

    /** The column track for auto dimension. */
    TrackInfo autoColumns;

    /** The gap in pixels between columns. */
    Px columnGap { 0 };
    /** The gap in pixels between rows. */
    Px rowGap { 0 };

    /** Sets the gap between rows and columns in pixels. */
    z0 setGap (Px sizeInPixels) noexcept          { rowGap = columnGap = sizeInPixels; }

    //==============================================================================
    /** The set of items to lay-out. */
    Array<GridItem> items;

    //==============================================================================
    /** Lays-out the grid's items within the given rectangle. */
    z0 performLayout (Rectangle<i32>);

    //==============================================================================
    /** Returns the number of columns. */
    i32 getNumberOfColumns() const noexcept         { return templateColumns.size(); }
    /** Returns the number of rows. */
    i32 getNumberOfRows() const noexcept            { return templateRows.size(); }

private:
    //==============================================================================
    struct Helpers;
};

constexpr Grid::Px operator""_px (real_t px)          { return Grid::Px { px }; }
constexpr Grid::Px operator""_px (zu64 px)   { return Grid::Px { px }; }
constexpr Grid::Fr operator""_fr (zu64 fr)   { return Grid::Fr { fr }; }

} // namespace drx
