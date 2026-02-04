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

template <typename Item>
static Array<Item> operator+ (const Array<Item>& a, const Array<Item>& b)
{
    auto copy = a;
    copy.addArray (b);
    return copy;
}

struct Grid::Helpers
{

    struct AllTracksIncludingImplicit
    {
        Array<TrackInfo> items;
        i32 numImplicitLeading; // The number of implicit items before the explicit items
    };

    struct Tracks
    {
        AllTracksIncludingImplicit columns, rows;
    };

    struct NoRounding
    {
        template <typename T>
        T operator() (T t) const { return t; }
    };

    struct StandardRounding
    {
        template <typename T>
        T operator() (T t) const { return std::round (t); }
    };

    template <typename RoundingFunction>
    struct SizeCalculation
    {
        f32 getTotalAbsoluteSize (const Array<TrackInfo>& tracks, Px gapSize) noexcept
        {
            f32 totalCellSize = 0.0f;

            for (const auto& trackInfo : tracks)
                if (! trackInfo.isFractional() || trackInfo.isAuto())
                    totalCellSize += roundingFunction (trackInfo.getSize());

            f32 totalGap = tracks.size() > 1 ? (f32) (tracks.size() - 1) * roundingFunction ((f32) gapSize.pixels)
                                               : 0.0f;

            return totalCellSize + totalGap;
        }

        static f32 getRelativeUnitSize (f32 size, f32 totalAbsolute, const Array<TrackInfo>& tracks) noexcept
        {
            const f32 totalRelative = jlimit (0.0f, size, size - totalAbsolute);
            f32 factorsSum = 0.0f;

            for (const auto& trackInfo : tracks)
                if (trackInfo.isFractional())
                    factorsSum += trackInfo.getSize();

            jassert (! approximatelyEqual (factorsSum, 0.0f));
            return totalRelative / factorsSum;
        }

        //==============================================================================
        f32 getTotalAbsoluteHeight (const Array<TrackInfo>& rowTracks, Px rowGapSize)
        {
            return getTotalAbsoluteSize (rowTracks, rowGapSize);
        }

        f32 getTotalAbsoluteWidth (const Array<TrackInfo>& columnTracks, Px columnGapSize)
        {
            return getTotalAbsoluteSize (columnTracks, columnGapSize);
        }

        f32 getRelativeWidthUnit (f32 gridWidth, Px columnGapSize, const Array<TrackInfo>& columnTracks)
        {
            return getRelativeUnitSize (gridWidth, getTotalAbsoluteWidth (columnTracks, columnGapSize), columnTracks);
        }

        f32 getRelativeHeightUnit (f32 gridHeight, Px rowGapSize, const Array<TrackInfo>& rowTracks)
        {
            return getRelativeUnitSize (gridHeight, getTotalAbsoluteHeight (rowTracks, rowGapSize), rowTracks);
        }

        //==============================================================================
        static b8 hasAnyFractions (const Array<TrackInfo>& tracks)
        {
            return std::any_of (tracks.begin(),
                                tracks.end(),
                                [] (const auto& t) { return t.isFractional(); });
        }

        z0 computeSizes (f32 gridWidth, f32 gridHeight,
                           Px columnGapToUse, Px rowGapToUse,
                           const Tracks& tracks)
        {
            if (hasAnyFractions (tracks.columns.items))
            {
                relativeWidthUnit = getRelativeWidthUnit (gridWidth, columnGapToUse, tracks.columns.items);
                fractionallyDividedWidth = gridWidth - getTotalAbsoluteSize (tracks.columns.items, columnGapToUse);
            }
            else
            {
                remainingWidth = gridWidth - getTotalAbsoluteSize (tracks.columns.items, columnGapToUse);
            }

            if (hasAnyFractions (tracks.rows.items))
            {
                relativeHeightUnit = getRelativeHeightUnit (gridHeight, rowGapToUse, tracks.rows.items);
                fractionallyDividedHeight = gridHeight - getTotalAbsoluteSize (tracks.rows.items, rowGapToUse);
            }
            else
            {
                remainingHeight = gridHeight - getTotalAbsoluteSize (tracks.rows.items, rowGapToUse);
            }

            const auto calculateTrackBounds = [&] (auto& outBounds,
                                                   const auto& trackItems,
                                                   auto relativeUnit,
                                                   auto totalSizeForFractionalItems,
                                                   auto gap)
            {
                const auto lastFractionalIndex = [&]
                {
                    for (i32 i = trackItems.size() - 1; 0 <= i; --i)
                        if (trackItems[i].isFractional())
                            return i;

                    return -1;
                }();

                f32 start = 0.0f;
                f32 carriedError = 0.0f;

                for (i32 i = 0; i < trackItems.size(); ++i)
                {
                    const auto& currentItem = trackItems[i];

                    const auto currentTrackSize = [&]
                    {
                        if (i == lastFractionalIndex)
                            return totalSizeForFractionalItems;

                        const auto absoluteSize = currentItem.getAbsoluteSize (relativeUnit);

                        if (! currentItem.isFractional())
                            return roundingFunction (absoluteSize);

                        const auto result = roundingFunction (absoluteSize - carriedError);
                        carriedError += result - absoluteSize;
                        return result;
                    }();

                    if (currentItem.isFractional())
                        totalSizeForFractionalItems -= currentTrackSize;

                    const auto end = start + currentTrackSize;
                    outBounds.emplace_back (start, end);
                    start = end + roundingFunction (static_cast<f32> (gap.pixels));
                }
            };

            calculateTrackBounds (columnTrackBounds,
                                  tracks.columns.items,
                                  relativeWidthUnit,
                                  fractionallyDividedWidth,
                                  columnGapToUse);

            calculateTrackBounds (rowTrackBounds,
                                  tracks.rows.items,
                                  relativeHeightUnit,
                                  fractionallyDividedHeight,
                                  rowGapToUse);
        }

        f32 relativeWidthUnit         = 0.0f;
        f32 relativeHeightUnit        = 0.0f;
        f32 fractionallyDividedWidth  = 0.0f;
        f32 fractionallyDividedHeight = 0.0f;
        f32 remainingWidth            = 0.0f;
        f32 remainingHeight           = 0.0f;

        std::vector<Range<f32>> columnTrackBounds;
        std::vector<Range<f32>> rowTrackBounds;
        RoundingFunction roundingFunction;
    };

    //==============================================================================
    struct PlacementHelpers
    {
        enum { invalid = -999999 };
        static constexpr auto emptyAreaCharacter = ".";

        //==============================================================================
        struct LineRange { i32 start, end; };
        struct LineArea  { LineRange column, row; };
        struct LineInfo  { StringArray lineNames; };

        struct NamedArea
        {
            Txt name;
            LineArea lines;
        };

        //==============================================================================
        static Array<LineInfo> getArrayOfLinesFromTracks (const Array<TrackInfo>& tracks)
        {
            // fill line info array
            Array<LineInfo> lines;

            for (i32 i = 1; i <= tracks.size(); ++i)
            {
                const auto& currentTrack = tracks.getReference (i - 1);

                if (i == 1) // start line
                {
                    LineInfo li;
                    li.lineNames.add (currentTrack.getStartLineName());
                    lines.add (li);
                }

                if (i > 1 && i <= tracks.size()) // two lines in between tracks
                {
                    const auto& prevTrack = tracks.getReference (i - 2);

                    LineInfo li;
                    li.lineNames.add (prevTrack.getEndLineName());
                    li.lineNames.add (currentTrack.getStartLineName());

                    lines.add (li);
                }

                if (i == tracks.size()) // end line
                {
                    LineInfo li;
                    li.lineNames.add (currentTrack.getEndLineName());
                    lines.add (li);
                }
            }

            jassert (lines.size() == tracks.size() + 1);

            return lines;
        }

        //==============================================================================
        static i32 deduceAbsoluteLineNumberFromLineName (GridItem::Property prop,
                                                         const Array<TrackInfo>& tracks)
        {
            jassert (prop.hasAbsolute());

            const auto lines = getArrayOfLinesFromTracks (tracks);
            i32 count = 0;

            for (const auto [index, line] : enumerate (lines))
            {
                for (const auto& name : line.lineNames)
                {
                    if (prop.getName() == name)
                    {
                        ++count;
                        break;
                    }
                }

                if (count == prop.getNumber())
                    return (i32) index + 1;
            }

            jassertfalse;
            return count;
        }

        static i32 deduceAbsoluteLineNumber (GridItem::Property prop,
                                             const Array<TrackInfo>& tracks)
        {
            jassert (prop.hasAbsolute());

            if (prop.hasName())
                return deduceAbsoluteLineNumberFromLineName (prop, tracks);

            if (prop.getNumber() > 0)
                return prop.getNumber();

            if (prop.getNumber() < 0)
                return tracks.size() + 2 + prop.getNumber();

            // An integer value of 0 is invalid
            jassertfalse;
            return 1;
        }

        static i32 deduceAbsoluteLineNumberFromNamedSpan (i32 startLineNumber,
                                                          GridItem::Property propertyWithSpan,
                                                          const Array<TrackInfo>& tracks)
        {
            jassert (propertyWithSpan.hasSpan());

            const auto lines = getArrayOfLinesFromTracks (tracks);
            i32 count = 0;

            const auto enumerated = enumerate (lines);

            for (const auto [index, line] : makeRange (enumerated.begin() + startLineNumber, enumerated.end()))
            {
                for (const auto& name : line.lineNames)
                {
                    if (propertyWithSpan.getName() == name)
                    {
                        ++count;
                        break;
                    }
                }

                if (count == propertyWithSpan.getNumber())
                    return (i32) index + 1;
            }

            jassertfalse;
            return count;
        }

        static i32 deduceAbsoluteLineNumberBasedOnSpan (i32 startLineNumber,
                                                        GridItem::Property propertyWithSpan,
                                                        const Array<TrackInfo>& tracks)
        {
            jassert (propertyWithSpan.hasSpan());

            if (propertyWithSpan.hasName())
                return deduceAbsoluteLineNumberFromNamedSpan (startLineNumber, propertyWithSpan, tracks);

            return startLineNumber + propertyWithSpan.getNumber();
        }

        //==============================================================================
        static LineRange deduceLineRange (GridItem::StartAndEndProperty prop, const Array<TrackInfo>& tracks)
        {
            jassert (! (prop.start.hasAuto() && prop.end.hasAuto()));

            if (prop.start.hasAbsolute() && prop.end.hasAuto())
            {
                prop.end = GridItem::Span (1);
            }
            else if (prop.start.hasAuto() && prop.end.hasAbsolute())
            {
                prop.start = GridItem::Span (1);
            }

            auto s = [&]() -> LineRange
            {
                if (prop.start.hasAbsolute() && prop.end.hasAbsolute())
                {
                    return { deduceAbsoluteLineNumber (prop.start, tracks),
                             deduceAbsoluteLineNumber (prop.end, tracks) };
                }

                if (prop.start.hasAbsolute() && prop.end.hasSpan())
                {
                    const auto start = deduceAbsoluteLineNumber (prop.start, tracks);
                    return { start, deduceAbsoluteLineNumberBasedOnSpan (start, prop.end, tracks) };
                }

                if (prop.start.hasSpan() && prop.end.hasAbsolute())
                {
                    const auto start = deduceAbsoluteLineNumber (prop.end, tracks);
                    return { start, deduceAbsoluteLineNumberBasedOnSpan (start, prop.start, tracks) };
                }

                // Can't have an item with spans on both start and end.
                jassertfalse;
                return {};
            }();

            // swap if start overtakes end
            if (s.start > s.end)
                std::swap (s.start, s.end);
            else if (s.start == s.end)
                s.end = s.start + 1;

            return s;
        }

        static LineArea deduceLineArea (const GridItem& item,
                                        const Grid& grid,
                                        const std::map<Txt, LineArea>& namedAreas)
        {
            if (item.area.isNotEmpty() && ! grid.templateAreas.isEmpty())
            {
                // Must be a named area!
                jassert (namedAreas.count (item.area) != 0);

                return namedAreas.at (item.area);
            }

            return { deduceLineRange (item.column, grid.templateColumns),
                     deduceLineRange (item.row,    grid.templateRows) };
        }

        //==============================================================================
        static Array<StringArray> parseAreasProperty (const StringArray& areasStrings)
        {
            Array<StringArray> strings;

            for (const auto& areaString : areasStrings)
                strings.add (StringArray::fromTokens (areaString, false));

            if (strings.size() > 0)
            {
                for (auto s : strings)
                {
                    jassert (s.size() == strings[0].size()); // all rows must have the same number of columns
                }
            }

            return strings;
        }

        static NamedArea findArea (Array<StringArray>& stringsArrays)
        {
            NamedArea area;

            for (auto& stringArray : stringsArrays)
            {
                for (auto& string : stringArray)
                {
                    // find anchor
                    if (area.name.isEmpty())
                    {
                        if (string != emptyAreaCharacter)
                        {
                            area.name = string;
                            area.lines.row.start = stringsArrays.indexOf (stringArray) + 1; // non-zero indexed;
                            area.lines.column.start = stringArray.indexOf (string) + 1; // non-zero indexed;

                            area.lines.row.end = stringsArrays.indexOf (stringArray) + 2;
                            area.lines.column.end = stringArray.indexOf (string) + 2;

                            // mark as visited
                            string = emptyAreaCharacter;
                        }
                    }
                    else
                    {
                        if (string == area.name)
                        {
                            area.lines.row.end = stringsArrays.indexOf (stringArray) + 2;
                            area.lines.column.end = stringArray.indexOf (string) + 2;

                            // mark as visited
                            string = emptyAreaCharacter;
                        }
                    }
                }
            }

            return area;
        }

        //==============================================================================
        static std::map<Txt, LineArea> deduceNamedAreas (const StringArray& areasStrings)
        {
            auto stringsArrays = parseAreasProperty (areasStrings);

            std::map<Txt, LineArea> areas;

            for (auto area = findArea (stringsArrays); area.name.isNotEmpty(); area = findArea (stringsArrays))
            {
                if (areas.count (area.name) == 0)
                    areas[area.name] = area.lines;
                else
                    // Make sure your template-areas property only has one area with the same name and is well-formed
                    jassertfalse;
            }

            return areas;
        }

        //==============================================================================
        template <typename RoundingFunction>
        static Rectangle<f32> getCellBounds (i32 columnNumber, i32 rowNumber,
                                               const Tracks& tracks,
                                               const SizeCalculation<RoundingFunction>& calculation)
        {
            const auto correctedColumn = columnNumber - 1 + tracks.columns.numImplicitLeading;
            const auto correctedRow    = rowNumber    - 1 + tracks.rows   .numImplicitLeading;

            jassert (isPositiveAndBelow (correctedColumn, tracks.columns.items.size()));
            jassert (isPositiveAndBelow (correctedRow,    tracks.rows   .items.size()));

            return
            {
                calculation.columnTrackBounds[(size_t) correctedColumn].getStart(),
                calculation.rowTrackBounds[(size_t) correctedRow].getStart(),
                calculation.columnTrackBounds[(size_t) correctedColumn].getEnd() - calculation.columnTrackBounds[(size_t) correctedColumn].getStart(),
                calculation.rowTrackBounds[(size_t) correctedRow].getEnd() - calculation.rowTrackBounds[(size_t) correctedRow].getStart()
            };
        }

        template <typename RoundingFunction>
        static Rectangle<f32> alignCell (Rectangle<f32> area,
                                           i32 columnNumber, i32 rowNumber,
                                           i32 numberOfColumns, i32 numberOfRows,
                                           const SizeCalculation<RoundingFunction>& calculation,
                                           AlignContent alignContent,
                                           JustifyContent justifyContent)
        {
            if (alignContent == AlignContent::end)
                area.setY (area.getY() + calculation.remainingHeight);

            if (justifyContent == JustifyContent::end)
                area.setX (area.getX() + calculation.remainingWidth);

            if (alignContent == AlignContent::center)
                area.setY (area.getY() + calculation.remainingHeight / 2);

            if (justifyContent == JustifyContent::center)
                area.setX (area.getX() + calculation.remainingWidth / 2);

            if (alignContent == AlignContent::spaceBetween)
            {
                const auto shift = ((f32) (rowNumber - 1) * (calculation.remainingHeight / f32 (numberOfRows - 1)));
                area.setY (area.getY() + shift);
            }

            if (justifyContent == JustifyContent::spaceBetween)
            {
                const auto shift = ((f32) (columnNumber - 1) * (calculation.remainingWidth / f32 (numberOfColumns - 1)));
                area.setX (area.getX() + shift);
            }

            if (alignContent == AlignContent::spaceEvenly)
            {
                const auto shift = ((f32) rowNumber * (calculation.remainingHeight / f32 (numberOfRows + 1)));
                area.setY (area.getY() + shift);
            }

            if (justifyContent == JustifyContent::spaceEvenly)
            {
                const auto shift = ((f32) columnNumber * (calculation.remainingWidth / f32 (numberOfColumns + 1)));
                area.setX (area.getX() + shift);
            }

            if (alignContent == AlignContent::spaceAround)
            {
                const auto inbetweenShift = calculation.remainingHeight / f32 (numberOfRows);
                const auto sidesShift = inbetweenShift / 2;
                auto shift = (f32) (rowNumber - 1) * inbetweenShift + sidesShift;

                area.setY (area.getY() + shift);
            }

            if (justifyContent == JustifyContent::spaceAround)
            {
                const auto inbetweenShift = calculation.remainingWidth / f32 (numberOfColumns);
                const auto sidesShift = inbetweenShift / 2;
                auto shift = (f32) (columnNumber - 1) * inbetweenShift + sidesShift;

                area.setX (area.getX() + shift);
            }

            return area;
        }

        template <typename RoundingFunction>
        static Rectangle<f32> getAreaBounds (PlacementHelpers::LineRange columnRange,
                                               PlacementHelpers::LineRange rowRange,
                                               const Tracks& tracks,
                                               const SizeCalculation<RoundingFunction>& calculation,
                                               AlignContent alignContent,
                                               JustifyContent justifyContent)
        {
            const auto findAlignedCell = [&] (i32 column, i32 row)
            {
                const auto cell = getCellBounds (column, row, tracks, calculation);
                return alignCell (cell,
                                  column,
                                  row,
                                  tracks.columns.items.size(),
                                  tracks.rows.items.size(),
                                  calculation,
                                  alignContent,
                                  justifyContent);
            };

            const auto startCell = findAlignedCell (columnRange.start,   rowRange.start);
            const auto endCell   = findAlignedCell (columnRange.end - 1, rowRange.end - 1);

            const auto horizontalRange = startCell.getHorizontalRange().getUnionWith (endCell.getHorizontalRange());
            const auto verticalRange   = startCell.getVerticalRange()  .getUnionWith (endCell.getVerticalRange());
            return { horizontalRange.getStart(),  verticalRange.getStart(),
                     horizontalRange.getLength(), verticalRange.getLength() };
        }
    };

    //==============================================================================
    struct AutoPlacement
    {
        AutoPlacement() = delete;

        using ItemPlacementArray = Array<std::pair<GridItem*, PlacementHelpers::LineArea>>;

        //==============================================================================
        struct OccupancyPlane
        {
            struct Cell { i32 column, row; };

            OccupancyPlane (i32 highestColumnToUse, i32 highestRowToUse, b8 isColumnFirst)
                : highestCrossDimension (isColumnFirst ? highestRowToUse : highestColumnToUse),
                  columnFirst (isColumnFirst)
            {}

            PlacementHelpers::LineArea setCell (Cell cell, i32 columnSpan, i32 rowSpan)
            {
                for (i32 i = 0; i < columnSpan; i++)
                    for (i32 j = 0; j < rowSpan; j++)
                        occupiedCells.insert ({ cell.column + i, cell.row + j });

                return { { cell.column, cell.column + columnSpan }, { cell.row, cell.row + rowSpan } };
            }

            PlacementHelpers::LineArea setCell (Cell start, Cell end)
            {
                return setCell (start, std::abs (end.column - start.column),
                                       std::abs (end.row - start.row));
            }

            Cell nextAvailable (Cell referenceCell, i32 columnSpan, i32 rowSpan)
            {
                while (isOccupied (referenceCell, columnSpan, rowSpan) || isOutOfBounds (referenceCell, columnSpan, rowSpan))
                    referenceCell = advance (referenceCell);

                return referenceCell;
            }

            Cell nextAvailableOnRow (Cell referenceCell, i32 columnSpan, i32 rowSpan, i32 rowNumber)
            {
                if (columnFirst && (rowNumber + rowSpan) > highestCrossDimension)
                    highestCrossDimension = rowNumber + rowSpan;

                while (isOccupied (referenceCell, columnSpan, rowSpan)
                       || (referenceCell.row != rowNumber))
                    referenceCell = advance (referenceCell);

                return referenceCell;
            }

            Cell nextAvailableOnColumn (Cell referenceCell, i32 columnSpan, i32 rowSpan, i32 columnNumber)
            {
                if (! columnFirst && (columnNumber + columnSpan) > highestCrossDimension)
                    highestCrossDimension = columnNumber + columnSpan;

                while (isOccupied (referenceCell, columnSpan, rowSpan)
                       || (referenceCell.column != columnNumber))
                    referenceCell = advance (referenceCell);

                return referenceCell;
            }

            z0 updateMaxCrossDimensionFromAutoPlacementItem (i32 columnSpan, i32 rowSpan)
            {
                highestCrossDimension = jmax (highestCrossDimension, 1 + getCrossDimension ({ columnSpan, rowSpan }));
            }

        private:
            struct Comparator
            {
                using Tie = std::tuple<i32, i32> (*) (const Cell&);

                explicit Comparator (b8 columnFirstIn)
                    : tie (columnFirstIn ? Tie { [] (const Cell& x) { return std::tuple (x.row, x.column); } }
                                         : Tie { [] (const Cell& x) { return std::tuple (x.column, x.row); } })
                {}

                b8 operator() (const Cell& a, const Cell& b) const
                {
                    return tie (a) < tie (b);
                }

                const Tie tie;
            };

            b8 isOccupied (Cell cell) const
            {
                return occupiedCells.count (cell) > 0;
            }

            b8 isOccupied (Cell cell, i32 columnSpan, i32 rowSpan) const
            {
                for (i32 i = 0; i < columnSpan; i++)
                    for (i32 j = 0; j < rowSpan; j++)
                        if (isOccupied ({ cell.column + i, cell.row + j }))
                            return true;

                return false;
            }

            b8 isOutOfBounds (Cell cell, i32 columnSpan, i32 rowSpan) const
            {
                const auto highestIndexOfCell = getCrossDimension (cell) + getCrossDimension ({ columnSpan, rowSpan });
                const auto highestIndexOfGrid = getHighestCrossDimension();

                return highestIndexOfGrid < highestIndexOfCell;
            }

            i32 getHighestCrossDimension() const
            {
                const auto cell = occupiedCells.empty() ? Cell { 1, 1 }
                                                        : *std::prev (occupiedCells.end());

                return std::max (getCrossDimension (cell) + 1, highestCrossDimension);
            }

            Cell advance (Cell cell) const
            {
                const auto next = getCrossDimension (cell) + 1;

                if (next >= getHighestCrossDimension())
                    return fromDimensions (getMainDimension (cell) + 1, 1);

                return fromDimensions (getMainDimension (cell), next);
            }

            i32 getMainDimension (Cell cell) const   { return columnFirst ? cell.column : cell.row; }
            i32 getCrossDimension (Cell cell) const  { return columnFirst ? cell.row : cell.column; }

            Cell fromDimensions (i32 mainDimension, i32 crossDimension) const
            {
                if (columnFirst)
                    return { mainDimension, crossDimension };

                return { crossDimension, mainDimension };
            }

            i32 highestCrossDimension;
            const b8 columnFirst;
            std::set<Cell, Comparator> occupiedCells { Comparator { columnFirst } };

            DRX_DECLARE_NON_COPYABLE (OccupancyPlane)
            DRX_DECLARE_NON_MOVEABLE (OccupancyPlane)
        };

        //==============================================================================
        static b8 isFixed (GridItem::StartAndEndProperty prop)
        {
            return prop.start.hasName() || prop.start.hasAbsolute() || prop.end.hasName() || prop.end.hasAbsolute();
        }

        static b8 hasFullyFixedPlacement (const GridItem& item)
        {
            if (item.area.isNotEmpty())
                return true;

            if (isFixed (item.column) && isFixed (item.row))
                return true;

            return false;
        }

        static b8 hasPartialFixedPlacement (const GridItem& item)
        {
            if (item.area.isNotEmpty())
                return false;

            if (isFixed (item.column) ^ isFixed (item.row))
                return true;

            return false;
        }

        static b8 hasAutoPlacement (const GridItem& item)
        {
            return ! hasFullyFixedPlacement (item) && ! hasPartialFixedPlacement (item);
        }

        //==============================================================================
        static b8 hasDenseAutoFlow (AutoFlow autoFlow)
        {
            return autoFlow == AutoFlow::columnDense
                || autoFlow == AutoFlow::rowDense;
        }

        static b8 isColumnAutoFlow (AutoFlow autoFlow)
        {
            return autoFlow == AutoFlow::column
                || autoFlow == AutoFlow::columnDense;
        }

        //==============================================================================
        static i32 getSpanFromAuto (GridItem::StartAndEndProperty prop)
        {
            if (prop.end.hasSpan())
                return prop.end.getNumber();

            if (prop.start.hasSpan())
                return prop.start.getNumber();

            return 1;
        }

        //==============================================================================
        static ItemPlacementArray deduceAllItems (Grid& grid)
        {
            const auto namedAreas = PlacementHelpers::deduceNamedAreas (grid.templateAreas);

            OccupancyPlane plane (jmax (grid.templateColumns.size() + 1, 2),
                                  jmax (grid.templateRows.size() + 1, 2),
                                  isColumnAutoFlow (grid.autoFlow));

            ItemPlacementArray itemPlacementArray;
            Array<GridItem*> sortedItems;

            for (auto& item : grid.items)
                sortedItems.add (&item);

            std::stable_sort (sortedItems.begin(), sortedItems.end(),
                              [] (const GridItem* i1, const GridItem* i2)  { return i1->order < i2->order; });

            // place fixed items first
            for (auto* item : sortedItems)
            {
                if (hasFullyFixedPlacement (*item))
                {
                    const auto a = PlacementHelpers::deduceLineArea (*item, grid, namedAreas);
                    plane.setCell ({ a.column.start, a.row.start }, { a.column.end, a.row.end });
                    itemPlacementArray.add ({ item, a });
                }
            }

            OccupancyPlane::Cell lastInsertionCell = { 1, 1 };

            for (auto* item : sortedItems)
            {
                if (hasPartialFixedPlacement (*item))
                {
                    if (isFixed (item->column))
                    {
                        const auto p = PlacementHelpers::deduceLineRange (item->column, grid.templateColumns);
                        const auto columnSpan = std::abs (p.start - p.end);
                        const auto rowSpan = getSpanFromAuto (item->row);

                        const auto insertionCell = hasDenseAutoFlow (grid.autoFlow) ? OccupancyPlane::Cell { p.start, 1 }
                                                                                    : lastInsertionCell;
                        const auto nextAvailableCell = plane.nextAvailableOnColumn (insertionCell, columnSpan, rowSpan, p.start);
                        const auto lineArea = plane.setCell (nextAvailableCell, columnSpan, rowSpan);
                        lastInsertionCell = nextAvailableCell;

                        itemPlacementArray.add ({ item, lineArea });
                    }
                    else if (isFixed (item->row))
                    {
                        const auto p = PlacementHelpers::deduceLineRange (item->row, grid.templateRows);
                        const auto columnSpan = getSpanFromAuto (item->column);
                        const auto rowSpan = std::abs (p.start - p.end);

                        const auto insertionCell = hasDenseAutoFlow (grid.autoFlow) ? OccupancyPlane::Cell { 1, p.start }
                                                                                    : lastInsertionCell;

                        const auto nextAvailableCell = plane.nextAvailableOnRow (insertionCell, columnSpan, rowSpan, p.start);
                        const auto lineArea = plane.setCell (nextAvailableCell, columnSpan, rowSpan);

                        lastInsertionCell = nextAvailableCell;

                        itemPlacementArray.add ({ item, lineArea });
                    }
                }
            }

            // https://www.w3.org/TR/css-grid-1/#auto-placement-algo step 3.3
            for (auto* item : sortedItems)
                if (hasAutoPlacement (*item))
                    plane.updateMaxCrossDimensionFromAutoPlacementItem (getSpanFromAuto (item->column), getSpanFromAuto (item->row));

            lastInsertionCell = { 1, 1 };

            for (auto* item : sortedItems)
            {
                if (hasAutoPlacement (*item))
                {
                    const auto columnSpan = getSpanFromAuto (item->column);
                    const auto rowSpan = getSpanFromAuto (item->row);

                    const auto nextAvailableCell = plane.nextAvailable (lastInsertionCell, columnSpan, rowSpan);
                    const auto lineArea = plane.setCell (nextAvailableCell, columnSpan, rowSpan);

                    if (! hasDenseAutoFlow (grid.autoFlow))
                        lastInsertionCell = nextAvailableCell;

                    itemPlacementArray.add ({ item,  lineArea });
                }
            }

            return itemPlacementArray;
        }

        //==============================================================================
        template <typename Accessor>
        static PlacementHelpers::LineRange findFullLineRange (const ItemPlacementArray& items, Accessor&& accessor)
        {
            if (items.isEmpty())
                return { 1, 1 };

            const auto combine = [&accessor] (const auto& acc, const auto& item)
            {
                const auto newRange = accessor (item);
                return PlacementHelpers::LineRange { std::min (acc.start, newRange.start),
                                                     std::max (acc.end,   newRange.end) };
            };

            return std::accumulate (std::next (items.begin()), items.end(), accessor (*items.begin()), combine);
        }

        static PlacementHelpers::LineArea findFullLineArea (const ItemPlacementArray& items)
        {
            return { findFullLineRange (items, [] (const auto& item) { return item.second.column; }),
                     findFullLineRange (items, [] (const auto& item) { return item.second.row; }) };
        }

        template <typename Item>
        static Array<Item> repeated (i32 repeats, const Item& item)
        {
            Array<Item> result;
            result.insertMultiple (-1, item, repeats);
            return result;
        }

        static Tracks createImplicitTracks (const Grid& grid, const ItemPlacementArray& items)
        {
            const auto fullArea = findFullLineArea (items);

            const auto leadingColumns = std::max (0, 1 - fullArea.column.start);
            const auto leadingRows    = std::max (0, 1 - fullArea.row.start);

            const auto trailingColumns = std::max (0, fullArea.column.end - grid.templateColumns.size() - 1);
            const auto trailingRows    = std::max (0, fullArea.row   .end - grid.templateRows   .size() - 1);

            return  { { repeated (leadingColumns, grid.autoColumns) + grid.templateColumns + repeated (trailingColumns, grid.autoColumns),
                        leadingColumns },
                      { repeated (leadingRows,    grid.autoRows)    + grid.templateRows    + repeated (trailingRows,    grid.autoRows),
                        leadingRows } };
        }

        //==============================================================================
        static z0 applySizeForAutoTracks (Tracks& tracks, const ItemPlacementArray& placements)
        {
            const auto setSizes = [&placements] (auto& tracksInDirection, const auto& getItem, const auto& getItemSize)
            {
                auto& array = tracksInDirection.items;

                for (i32 index = 0; index < array.size(); ++index)
                {
                    if (array.getReference (index).isAuto())
                    {
                        const auto combiner = [&] (const auto acc, const auto& element)
                        {
                            const auto item = getItem (element.second);
                            const auto isNotSpan = std::abs (item.end - item.start) <= 1;
                            return isNotSpan && item.start == index + 1 - tracksInDirection.numImplicitLeading
                                   ? std::max (acc, getItemSize (*element.first))
                                   : acc;
                        };

                        array.getReference (index).size = std::accumulate (placements.begin(), placements.end(), 0.0f, combiner);
                    }
                }
            };

            setSizes (tracks.rows,
                      [] (const auto& i) { return i.row; },
                      [] (const auto& i) { return i.height + i.margin.top + i.margin.bottom; });

            setSizes (tracks.columns,
                      [] (const auto& i) { return i.column; },
                      [] (const auto& i) { return i.width + i.margin.left + i.margin.right; });
        }
    };

    //==============================================================================
    struct BoxAlignment
    {
        static Rectangle<f32> alignItem (const GridItem& item, const Grid& grid, Rectangle<f32> area)
        {
            // if item align is auto, inherit value from grid
            const auto alignType = item.alignSelf == GridItem::AlignSelf::autoValue
                                 ? grid.alignItems
                                 : static_cast<AlignItems> (item.alignSelf);

            const auto justifyType = item.justifySelf == GridItem::JustifySelf::autoValue
                                   ? grid.justifyItems
                                   : static_cast<JustifyItems> (item.justifySelf);

            // subtract margin from area
            area = BorderSize<f32> (item.margin.top, item.margin.left, item.margin.bottom, item.margin.right)
                      .subtractedFrom (area);

            // align and justify
            auto r = area;

            if (! approximatelyEqual (item.width,     (f32) GridItem::notAssigned))  r.setWidth  (item.width);
            if (! approximatelyEqual (item.height,    (f32) GridItem::notAssigned))  r.setHeight (item.height);
            if (! approximatelyEqual (item.maxWidth,  (f32) GridItem::notAssigned))  r.setWidth  (jmin (item.maxWidth,  r.getWidth()));
            if (item.minWidth  > 0.0f)                            r.setWidth  (jmax (item.minWidth,  r.getWidth()));
            if (! approximatelyEqual (item.maxHeight, (f32) GridItem::notAssigned))  r.setHeight (jmin (item.maxHeight, r.getHeight()));
            if (item.minHeight > 0.0f)                            r.setHeight (jmax (item.minHeight, r.getHeight()));

            if (alignType == AlignItems::start && justifyType == JustifyItems::start)
                return r;

            if (alignType   == AlignItems::end)       r.setY (r.getY() + (area.getHeight() - r.getHeight()));
            if (justifyType == JustifyItems::end)     r.setX (r.getX() + (area.getWidth()  - r.getWidth()));
            if (alignType   == AlignItems::center)    r.setCentre (r.getCentreX(),    area.getCentreY());
            if (justifyType == JustifyItems::center)  r.setCentre (area.getCentreX(), r.getCentreY());

            return r;
        }
    };

};

//==============================================================================
Grid::TrackInfo::TrackInfo() noexcept : hasKeyword (true) {}

Grid::TrackInfo::TrackInfo (Px sizeInPixels) noexcept
    : size (static_cast<f32> (sizeInPixels.pixels)), isFraction (false) {}

Grid::TrackInfo::TrackInfo (Fr fractionOfFreeSpace) noexcept
    : size ((f32)fractionOfFreeSpace.fraction), isFraction (true) {}

Grid::TrackInfo::TrackInfo (Px sizeInPixels, const Txt& endLineNameToUse) noexcept
    : TrackInfo (sizeInPixels)
{
    endLineName = endLineNameToUse;
}

Grid::TrackInfo::TrackInfo (Fr fractionOfFreeSpace, const Txt& endLineNameToUse) noexcept
    : TrackInfo (fractionOfFreeSpace)
{
    endLineName = endLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const Txt& startLineNameToUse, Px sizeInPixels) noexcept
    : TrackInfo (sizeInPixels)
{
    startLineName = startLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const Txt& startLineNameToUse, Fr fractionOfFreeSpace) noexcept
    : TrackInfo (fractionOfFreeSpace)
{
    startLineName = startLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const Txt& startLineNameToUse, Px sizeInPixels, const Txt& endLineNameToUse) noexcept
    : TrackInfo (startLineNameToUse, sizeInPixels)
{
    endLineName = endLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const Txt& startLineNameToUse, Fr fractionOfFreeSpace, const Txt& endLineNameToUse) noexcept
    : TrackInfo (startLineNameToUse, fractionOfFreeSpace)
{
    endLineName = endLineNameToUse;
}

f32 Grid::TrackInfo::getAbsoluteSize (f32 relativeFractionalUnit) const
{
    return isFractional() ? size * relativeFractionalUnit : size;
}

//==============================================================================
z0 Grid::performLayout (Rectangle<i32> targetArea)
{
    const auto itemsAndAreas = Helpers::AutoPlacement::deduceAllItems (*this);

    auto implicitTracks = Helpers::AutoPlacement::createImplicitTracks (*this, itemsAndAreas);

    Helpers::AutoPlacement::applySizeForAutoTracks (implicitTracks, itemsAndAreas);

    Helpers::SizeCalculation<Helpers::NoRounding> calculation;
    Helpers::SizeCalculation<Helpers::StandardRounding> roundedCalculation;

    const auto doComputeSizes = [&] (auto& sizeCalculation)
    {
        sizeCalculation.computeSizes (targetArea.toFloat().getWidth(),
                                      targetArea.toFloat().getHeight(),
                                      columnGap,
                                      rowGap,
                                      implicitTracks);
    };

    doComputeSizes (calculation);
    doComputeSizes (roundedCalculation);

    for (auto& itemAndArea : itemsAndAreas)
    {
        auto* item = itemAndArea.first;

        const auto getBounds = [&] (const auto& sizeCalculation)
        {
            const auto a = itemAndArea.second;

            const auto areaBounds = Helpers::PlacementHelpers::getAreaBounds (a.column,
                                                                              a.row,
                                                                              implicitTracks,
                                                                              sizeCalculation,
                                                                              alignContent,
                                                                              justifyContent);

            const auto rounded = [&] (auto rect) -> decltype (rect)
            {
                return { sizeCalculation.roundingFunction (rect.getX()),
                         sizeCalculation.roundingFunction (rect.getY()),
                         sizeCalculation.roundingFunction (rect.getWidth()),
                         sizeCalculation.roundingFunction (rect.getHeight()) };
            };

            return rounded (Helpers::BoxAlignment::alignItem (*item, *this, areaBounds));
        };

        item->currentBounds = getBounds (calculation) + targetArea.toFloat().getPosition();

        if (auto* c = item->associatedComponent)
            c->setBounds (getBounds (roundedCalculation).toNearestIntEdges() + targetArea.getPosition());
    }
}

//==============================================================================
#if DRX_UNIT_TESTS

struct GridTests final : public UnitTest
{
    GridTests()
        : UnitTest ("Grid", UnitTestCategories::gui)
    {}

    z0 runTest() override
    {
        using Fr = Grid::Fr;
        using Tr = Grid::TrackInfo;
        using Rect = Rectangle<f32>;

        beginTest ("Layout calculation of an empty grid is a no-op");
        {
            const Rectangle<i32> bounds { 100, 200 };
            Grid grid;
            grid.performLayout (bounds);
        }

        {
            Grid grid;

            grid.templateColumns.add (Tr (1_fr));
            grid.templateRows.addArray ({ Tr (20_px), Tr (1_fr) });

            grid.items.addArray ({ GridItem().withArea (1, 1),
                                   GridItem().withArea (2, 1) });

            grid.performLayout (Rectangle<i32> (200, 400));

            beginTest ("Layout calculation test: 1 column x 2 rows: no gap");
            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  200.f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 20.0f, 200.f, 380.0f));

            grid.templateColumns.add (Tr (50_px));
            grid.templateRows.add (Tr (2_fr));

            grid.items.addArray ( { GridItem().withArea (1, 2),
                                    GridItem().withArea (2, 2),
                                    GridItem().withArea (3, 1),
                                    GridItem().withArea (3, 2) });

            grid.performLayout (Rectangle<i32> (150, 170));

            beginTest ("Layout calculation test: 2 columns x 3 rows: no gap");
            expect (grid.items[0].currentBounds == Rect (0.0f,   0.0f,  100.0f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f,   20.0f, 100.0f, 50.0f));
            expect (grid.items[2].currentBounds == Rect (100.0f, 0.0f,  50.0f,  20.0f));
            expect (grid.items[3].currentBounds == Rect (100.0f, 20.0f, 50.0f,  50.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f,   70.0f, 100.0f, 100.0f));
            expect (grid.items[5].currentBounds == Rect (100.0f, 70.0f, 50.0f,  100.0f));

            grid.columnGap = 20_px;
            grid.rowGap    = 10_px;

            grid.performLayout (Rectangle<i32> (200, 310));

            beginTest ("Layout calculation test: 2 columns x 3 rows: rowGap of 10 and columnGap of 20");
            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f, 130.0f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 30.0f, 130.0f, 90.0f));
            expect (grid.items[2].currentBounds == Rect (150.0f, 0.0f, 50.0f, 20.0f));
            expect (grid.items[3].currentBounds == Rect (150.0f, 30.0f, 50.0f, 90.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f, 130.0f, 130.0f, 180.0f));
            expect (grid.items[5].currentBounds == Rect (150.0f, 130.0f, 50.0f,  180.0f));
        }

        {
            Grid grid;

            grid.templateColumns.addArray ({ Tr ("first", 20_px, "in"), Tr ("in", 1_fr, "in"), Tr (20_px, "last") });
            grid.templateRows.addArray ({ Tr (1_fr),
                                          Tr (20_px)});

            {
                beginTest ("Grid items placement tests: integer and custom ident, counting forward");

                GridItem i1, i2, i3, i4, i5;
                i1.column = { 1, 4 };
                i1.row    = { 1, 2 };

                i2.column = { 1, 3 };
                i2.row    = { 1, 3 };

                i3.column = { "first", "in" };
                i3.row    = { 2, 3 };

                i4.column = { "first", { 2, "in" } };
                i4.row    = { 1, 2 };

                i5.column = { "first", "last" };
                i5.row    = { 1, 2 };

                grid.items.addArray ({ i1, i2, i3, i4, i5 });

                grid.performLayout ({ 140, 100 });

                expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
                expect (grid.items[1].currentBounds == Rect (0.0f, 0.0f,  120.0f, 100.0f));
                expect (grid.items[2].currentBounds == Rect (0.0f, 80.0f, 20.0f,  20.0f));
                expect (grid.items[3].currentBounds == Rect (0.0f, 0.0f,  120.0f, 80.0f));
                expect (grid.items[4].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
            }
        }

        {
            Grid grid;

            grid.templateColumns.addArray ({ Tr ("first", 20_px, "in"), Tr ("in", 1_fr, "in"), Tr (20_px, "last") });
            grid.templateRows.addArray ({ Tr (1_fr),
                                          Tr (20_px)});

            beginTest ("Grid items placement tests: integer and custom ident, counting forward, reversed end and start");

            GridItem i1, i2, i3, i4, i5;
            i1.column = { 4, 1 };
            i1.row    = { 2, 1 };

            i2.column = { 3, 1 };
            i2.row    = { 3, 1 };

            i3.column = { "in", "first" };
            i3.row    = { 3, 2 };

            i4.column = { "first", { 2, "in" } };
            i4.row    = { 1, 2 };

            i5.column = { "last", "first" };
            i5.row    = { 1, 2 };

            grid.items.addArray ({ i1, i2, i3, i4, i5 });

            grid.performLayout ({ 140, 100 });

            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 0.0f,  120.0f, 100.0f));
            expect (grid.items[2].currentBounds == Rect (0.0f, 80.0f, 20.0f,  20.0f));
            expect (grid.items[3].currentBounds == Rect (0.0f, 0.0f,  120.0f, 80.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
        }

        {
            Grid grid;

            grid.templateColumns = { Tr ("first", 20_px, "in"), Tr ("in", 1_fr, "in"), Tr (20_px, "last") };
            grid.templateRows = { Tr (1_fr), Tr (20_px) };

            beginTest ("Grid items placement tests: integer, counting backward");

            grid.items = { GridItem{}.withColumn ({  -2, -1 }).withRow ({ 1,  3 }),
                           GridItem{}.withColumn ({ -10, -1 }).withRow ({ 1, -1 }) };

            grid.performLayout ({ 140, 100 });

            expect (grid.items[0].currentBounds == Rect (120.0f, 0.0f, 20.0f, 100.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 0.0f,  140.0f, 100.0f));
        }

        {
            beginTest ("Grid items placement tests: areas");

            Grid grid;

            grid.templateColumns =       { Tr (50_px), Tr (100_px), Tr (Fr (1_fr)), Tr (50_px) };
            grid.templateRows = { Tr (50_px),
                                  Tr (1_fr),
                                  Tr (50_px) };

            grid.templateAreas = { "header header header header",
                                   "main main . sidebar",
                                   "footer footer footer footer" };

            grid.items.addArray ({ GridItem().withArea ("header"),
                                   GridItem().withArea ("main"),
                                   GridItem().withArea ("sidebar"),
                                   GridItem().withArea ("footer"),
                                });

            grid.performLayout ({ 300, 150 });

            expect (grid.items[0].currentBounds == Rect (0.f,   0.f,   300.f, 50.f));
            expect (grid.items[1].currentBounds == Rect (0.f,   50.f,  150.f, 50.f));
            expect (grid.items[2].currentBounds == Rect (250.f, 50.f,  50.f,  50.f));
            expect (grid.items[3].currentBounds == Rect (0.f,   100.f, 300.f, 50.f));
        }

        {
            beginTest ("Grid implicit rows and columns: triggered by areas");

            Grid grid;

            grid.templateColumns =       { Tr (50_px), Tr (100_px), Tr (1_fr), Tr (50_px) };
            grid.templateRows = { Tr (50_px),
                                  Tr (1_fr),
                                  Tr (50_px) };

            grid.autoRows = Tr (30_px);
            grid.autoColumns = Tr (30_px);

            grid.templateAreas = { "header header header header header",
                                   "main main . sidebar sidebar",
                                   "footer footer footer footer footer",
                                   "sub sub sub sub sub"};

            grid.items.addArray ({ GridItem().withArea ("header"),
                                   GridItem().withArea ("main"),
                                   GridItem().withArea ("sidebar"),
                                   GridItem().withArea ("footer"),
                                   GridItem().withArea ("sub"),
                                });

            grid.performLayout ({ 330, 180 });

            expect (grid.items[0].currentBounds == Rect (0.f,   0.f,   330.f, 50.f));
            expect (grid.items[1].currentBounds == Rect (0.f,   50.f,  150.f, 50.f));
            expect (grid.items[2].currentBounds == Rect (250.f, 50.f,  80.f,  50.f));
            expect (grid.items[3].currentBounds == Rect (0.f,   100.f, 330.f, 50.f));
            expect (grid.items[4].currentBounds == Rect (0.f,   150.f, 330.f, 30.f));
        }

        {
            beginTest ("Grid implicit rows and columns: triggered by areas");

            Grid grid;

            grid.templateColumns =       { Tr (50_px), Tr (100_px), Tr (1_fr), Tr (50_px) };
            grid.templateRows = { Tr (50_px),
                                  Tr (1_fr),
                                  Tr (50_px) };

            grid.autoRows = Tr (1_fr);
            grid.autoColumns = Tr (1_fr);

            grid.templateAreas = { "header header header header",
                                   "main main . sidebar",
                                   "footer footer footer footer" };

            grid.items.addArray ({ GridItem().withArea ("header"),
                                   GridItem().withArea ("main"),
                                   GridItem().withArea ("sidebar"),
                                   GridItem().withArea ("footer"),
                                   GridItem().withArea (4, 5, 6, 7)
                                });

            grid.performLayout ({ 350, 250 });

            expect (grid.items[0].currentBounds == Rect (0.f,   0.f,   250.f, 50.f));
            expect (grid.items[1].currentBounds == Rect (0.f,   50.f,  150.f, 50.f));
            expect (grid.items[2].currentBounds == Rect (200.f, 50.f,  50.f,  50.f));
            expect (grid.items[3].currentBounds == Rect (0.f,   100.f, 250.f, 50.f));
            expect (grid.items[4].currentBounds == Rect (250.f, 150.f, 100.f, 100.f));
        }

        {
            beginTest ("Grid implicit rows and columns: triggered by out-of-bounds indices");

            Grid grid;

            grid.templateColumns = { Tr (1_fr),  Tr (1_fr) };
            grid.templateRows    = { Tr (60_px), Tr (60_px) };

            grid.autoColumns = Tr (20_px);
            grid.autoRows    = Tr (1_fr);

            grid.items = { GridItem{}.withColumn ({  5,  8 }).withRow ({ -5, -4 }),
                           GridItem{}.withColumn ({  4,  7 }).withRow ({ -4, -3 }),
                           GridItem{}.withColumn ({ -2, -1 }).withRow ({  4,  5 }) };

            grid.performLayout ({ 500, 400 });

            //       -3  -2  -1
            //        1   2   3   4   5   6   7   8
            //  -5    +---+---+---+---+---+---+---+   0
            //        |   |   |   |   | 0 | 0 | 0 |
            //  -4    +---+---+---+---+---+---+---+  70
            //        |   |   |   | 1 | 1 | 1 |   |
            //  -3  1 +---+---+---+---+---+---+---+ 140
            //        | x | x |   |   |   |   |   |
            //  -2  2 +---+---+---+---+---+---+---+ 200  y positions
            //        | x | x |   |   |   |   |   |
            //  -1  3 +---+---+---+---+---+---+---+ 260
            //        |   |   |   |   |   |   |   |
            //      4 +---+---+---+---+---+---+---+ 330
            //        |   | 2 |   |   |   |   |   |
            //      5 +---+---+---+---+---+---+---+ 400
            //
            //        0  200 400 420 440 460 480 500
            //                 x positions
            //
            // The cells marked "x" are the explicit cells specified by the template rows
            // and columns.
            //
            // The cells marked 0/1/2 correspond to the GridItems at those indices in the
            // items array.
            //
            // Note that negative indices count back from the last explicit line
            // number in that direction, so "2" and "-2" both correspond to the same line.

            expect (grid.items[0].currentBounds == Rect (440.0f,   0.0f,  60.0f, 70.0f));
            expect (grid.items[1].currentBounds == Rect (420.0f,  70.0f,  60.0f, 70.0f));
            expect (grid.items[2].currentBounds == Rect (200.0f, 330.0f, 200.0f, 70.0f));
        }

        {
            beginTest ("Items with specified sizes should translate to correctly rounded Component dimensions");

            static constexpr i32 targetSize = 100;

            drx::Component component;
            drx::GridItem item { component };
            item.alignSelf   = drx::GridItem::AlignSelf::center;
            item.justifySelf = drx::GridItem::JustifySelf::center;
            item.width       = (f32) targetSize;
            item.height      = (f32) targetSize;

            drx::Grid grid;
            grid.templateColumns = { drx::Grid::Fr { 1 } };
            grid.templateRows    = { drx::Grid::Fr { 1 } };
            grid.items           = { item };

            for (i32 totalSize = 100 - 20; totalSize < 100 + 20; ++totalSize)
            {
                Rectangle<i32> bounds { 0, 0, totalSize, totalSize };
                grid.performLayout (bounds);

                expectEquals (component.getWidth(), targetSize);
                expectEquals (component.getHeight(), targetSize);
            }
        }

        {
            beginTest ("Track sizes specified in Px should translate to correctly rounded Component dimensions");

            static constexpr i32 targetSize = 100;

            drx::Component component;
            drx::GridItem item { component };
            item.alignSelf   = drx::GridItem::AlignSelf::center;
            item.justifySelf = drx::GridItem::JustifySelf::center;
            item.setArea (1, 3);

            drx::Grid grid;
            grid.templateColumns = { drx::Grid::Fr { 1 },
                                     drx::Grid::Fr { 1 },
                                     drx::Grid::Px { targetSize },
                                     drx::Grid::Fr { 1 } };
            grid.templateRows    = { drx::Grid::Fr { 1 } };
            grid.items           = { item };

            for (i32 totalSize = 100 - 20; totalSize < 100 + 20; ++totalSize)
            {
                Rectangle<i32> bounds { 0, 0, totalSize, totalSize };
                grid.performLayout (bounds);

                expectEquals (component.getWidth(), targetSize);
            }
        }

        {
            beginTest ("Evaluate invariants on randomised Grid layouts");

            struct Solution
            {
                Grid grid;
                std::deque<Component> components;
                i32 absoluteWidth;
                Rectangle<i32> bounds;
            };

            auto createSolution = [this] (i32 numColumns,
                                          f32 probabilityOfFractionalColumn,
                                          Rectangle<i32> bounds) -> Solution
            {
                auto random = getRandom();

                Grid grid;
                grid.templateRows = { Grid::Fr { 1 } };

                // Ensuring that the sum of absolute item widths never exceed total width
                const auto widthOfAbsolute = (i32) ((f32) bounds.getWidth() / (f32) (numColumns + 1));

                for (i32 i = 0; i < numColumns; ++i)
                {
                    if (random.nextFloat() < probabilityOfFractionalColumn)
                        grid.templateColumns.add (Grid::Fr { 1 });
                    else
                        grid.templateColumns.add (Grid::Px { widthOfAbsolute });
                }

                std::deque<Component> itemComponents (static_cast<size_t> (grid.templateColumns.size()));

                for (auto& c : itemComponents)
                    grid.items.add (GridItem { c });

                grid.performLayout (bounds);

                return { std::move (grid), std::move (itemComponents), widthOfAbsolute, bounds };
            };

            const auto getFractionalComponentWidths = [] (const Solution& solution)
            {
                std::vector<i32> result;

                for (i32 i = 0; i < solution.grid.templateColumns.size(); ++i)
                    if (solution.grid.templateColumns[i].isFractional())
                        result.push_back (solution.components[(size_t) i].getWidth());

                return result;
            };

            const auto getAbsoluteComponentWidths = [] (const Solution& solution)
            {
                std::vector<i32> result;

                for (i32 i = 0; i < solution.grid.templateColumns.size(); ++i)
                    if (! solution.grid.templateColumns[i].isFractional())
                        result.push_back (solution.components[(size_t) i].getWidth());

                return result;
            };

            const auto evaluateInvariants = [&] (const Solution& solution)
            {
                const auto fractionalWidths = getFractionalComponentWidths (solution);

                if (! fractionalWidths.empty())
                {
                    const auto [min, max] = std::minmax_element (fractionalWidths.begin(),
                                                                 fractionalWidths.end());
                    expectLessOrEqual (*max - *min, 1, "Fr { 1 } items are expected to share the "
                                                       "rounding errors equally and hence couldn't "
                                                       "deviate in size by more than 1 px");
                }

                const auto absoluteWidths = getAbsoluteComponentWidths (solution);

                for (const auto& w : absoluteWidths)
                    expectEquals (w, solution.absoluteWidth, "Sizes specified in absolute dimensions should "
                                                             "be preserved");

                Rectangle<i32> unionOfComponentBounds;

                for (const auto& c : solution.components)
                    unionOfComponentBounds = unionOfComponentBounds.getUnion (c.getBoundsInParent());

                if ((size_t) solution.grid.templateColumns.size() == absoluteWidths.size())
                    expect (solution.bounds.contains (unionOfComponentBounds), "Non-oversized absolute Components "
                                                                               "should never be placed outside the "
                                                                               "provided bounds.");
                else
                    expect (unionOfComponentBounds == solution.bounds, "With fractional items, positioned items "
                                                                       "should cover the provided bounds exactly");
            };

            const auto knownPreviousBad = createSolution (5, 1.0f, Rectangle<i32> { 0, 0, 600, 200 }.reduced (16));
            evaluateInvariants (knownPreviousBad);

            auto random = getRandom();

            for (i32 i = 0; i < 1000; ++i)
            {
                const auto numColumns = random.nextInt (Range<i32> { 1, 26 });
                const auto probabilityOfFractionalColumn = random.nextFloat();
                const auto bounds = Rectangle<i32> { random.nextInt (Range<i32> { 0, 3 }),
                                                     random.nextInt (Range<i32> { 0, 3 }),
                                                     random.nextInt (Range<i32> { 300, 1200 }),
                                                     random.nextInt (Range<i32> { 100, 500 }) }
                                        .reduced (random.nextInt (Range<i32> { 0, 16 }));

                const auto randomSolution = createSolution (numColumns, probabilityOfFractionalColumn, bounds);
                evaluateInvariants (randomSolution);
            }
        }

        {
            beginTest ("Cell orderings for row and columns work correctly");

            const Rectangle<i32> bounds { 0, 0, 200, 200 };

            Grid grid;
            grid.autoColumns = Grid::TrackInfo { Grid::Fr { 1 } };
            grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };

            grid.items = { GridItem{}.withArea (1, 20),
                           GridItem{}.withArea (2, 10),
                           GridItem{}.withArea (GridItem::Span { 1 }, GridItem::Span { 15 }),
                           GridItem{} };

            grid.autoFlow = Grid::AutoFlow::row;
            grid.performLayout (bounds);
            expect (grid.items.getLast().currentBounds == Rect { 150, 0, 10, 100 });

            grid.autoFlow = Grid::AutoFlow::column;
            grid.performLayout (bounds);
            expect (grid.items.getLast().currentBounds == Rect { 0, 100, 10, 100 });

            grid.items = { GridItem{}.withArea (20, 1),
                           GridItem{}.withArea (10, 2),
                           GridItem{}.withArea (GridItem::Span { 15 }, GridItem::Span { 1 }),
                           GridItem{} };

            grid.autoFlow = Grid::AutoFlow::row;
            grid.performLayout (bounds);
            expect (grid.items.getLast().currentBounds == Rect { 100, 0, 100, 10 });

            grid.autoFlow = Grid::AutoFlow::column;
            grid.performLayout (bounds);
            expect (grid.items.getLast().currentBounds == Rect { 0, 150, 100, 10 });
        }

        beginTest ("Complex grid layout");
        {
            Grid grid;

            using Track = Grid::TrackInfo;

            grid.templateRows    = { Track (1_fr), Track (1_fr), Track (1_fr) };
            grid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr) };

            grid.autoColumns = Track (1_fr);
            grid.autoRows    = Track (1_fr);

            grid.autoFlow = Grid::AutoFlow::column;

            grid.items.addArray ({ GridItem().withArea (2, 2, 4, 4),
                                   GridItem(),
                                   GridItem().withArea ({}, 3),
                                   GridItem(),
                                   GridItem().withArea (GridItem::Span (2), {}),
                                   GridItem(),
                                   GridItem(),
                                   GridItem(),
                                   GridItem(),
                                   GridItem(),
                                   GridItem(),
                                   GridItem() });

            grid.performLayout ({ 60, 30 });

            expect (grid.items[0] .currentBounds == Rect { 10, 10, 20, 20 });
            expect (grid.items[1] .currentBounds == Rect {  0,  0, 10, 10 });
            expect (grid.items[2] .currentBounds == Rect { 20,  0, 10, 10 });
            expect (grid.items[3] .currentBounds == Rect {  0, 10, 10, 10 });
            expect (grid.items[4] .currentBounds == Rect { 30,  0, 10, 20 });
            expect (grid.items[5] .currentBounds == Rect { 30, 20, 10, 10 });
            expect (grid.items[6] .currentBounds == Rect { 40,  0, 10, 10 });
            expect (grid.items[7] .currentBounds == Rect { 40, 10, 10, 10 });
            expect (grid.items[8] .currentBounds == Rect { 40, 20, 10, 10 });
            expect (grid.items[9] .currentBounds == Rect { 50,  0, 10, 10 });
            expect (grid.items[10].currentBounds == Rect { 50, 10, 10, 10 });
            expect (grid.items[11].currentBounds == Rect { 50, 20, 10, 10 });
        }
    }
};

static GridTests gridUnitTests;

#endif

} // namespace drx
