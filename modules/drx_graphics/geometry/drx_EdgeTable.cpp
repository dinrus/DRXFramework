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

DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255 6263 6386)

EdgeTable::EdgeTable (Rectangle<i32> area, const Path& path, const AffineTransform& transform)
   : bounds (area),
     // this is a very vague heuristic to make a rough guess at a good table size
     // for a given path, such that it's big enough to mostly avoid remapping, but also
     // not so big that it's wasteful for simple paths.
     maxEdgesPerLine (jmax (defaultEdgesPerLine / 2,
                            4 * (i32) std::sqrt (path.data.size()))),
     lineStrideElements (maxEdgesPerLine * 2 + 1)
{
    allocate();
    i32* t = table.data();

    for (i32 i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }

    auto leftLimit   = scale * static_cast<z64> (bounds.getX());
    auto topLimit    = scale * static_cast<z64> (bounds.getY());
    auto rightLimit  = scale * static_cast<z64> (bounds.getRight());
    auto heightLimit = scale * static_cast<z64> (bounds.getHeight());

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        const auto scaleIterY = [] (auto y)
        {
            return static_cast<z64> (y * 256.0f + (y >= 0 ? 0.5f : -0.5f));
        };

        auto y1 = scaleIterY (iter.y1);
        auto y2 = scaleIterY (iter.y2);

        if (y1 != y2)
        {
            y1 -= topLimit;
            y2 -= topLimit;

            auto startY = y1;
            i32 direction = -1;

            if (y1 > y2)
            {
                std::swap (y1, y2);
                direction = 1;
            }

            if (y1 < 0)
                y1 = 0;

            if (y2 > heightLimit)
                y2 = heightLimit;

            if (y1 < y2)
            {
                const f64 startX = 256.0f * iter.x1;
                const f64 multiplier = (iter.x2 - iter.x1) / (iter.y2 - iter.y1);
                auto stepSize = static_cast<z64> (jlimit (1, 256, 256 / (1 + (i32) std::abs (multiplier))));

                do
                {
                    auto step = jmin (stepSize, y2 - y1, 256 - (y1 & 255));
                    auto x = static_cast<z64> (startX + multiplier * static_cast<f64> ((y1 + (step >> 1)) - startY));
                    auto clampedX = static_cast<i32> (jlimit (leftLimit, rightLimit, x));

                    addEdgePoint (clampedX, static_cast<i32> (y1 / scale), static_cast<i32> (direction * step));
                    y1 += step;
                }
                while (y1 < y2);
            }
        }
    }

    sanitiseLevels (path.isUsingNonZeroWinding());
}

EdgeTable::EdgeTable (Rectangle<i32> rectangleToAdd)
   : bounds (rectangleToAdd),
     maxEdgesPerLine (defaultEdgesPerLine),
     lineStrideElements (defaultEdgesPerLine * 2 + 1)
{
    allocate();
    table[0] = 0;

    auto x1 = scale * rectangleToAdd.getX();
    auto x2 = scale * rectangleToAdd.getRight();
    i32* t = table.data();

    for (i32 i = rectangleToAdd.getHeight(); --i >= 0;)
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = 255;
        t[3] = x2;
        t[4] = 0;
        t += lineStrideElements;
    }
}

EdgeTable::EdgeTable (const RectangleList<i32>& rectanglesToAdd)
   : bounds (rectanglesToAdd.getBounds()),
     maxEdgesPerLine (defaultEdgesPerLine),
     lineStrideElements (defaultEdgesPerLine * 2 + 1),
     needToCheckEmptiness (true)
{
    allocate();
    clearLineSizes();

    for (auto& r : rectanglesToAdd)
    {
        auto x1 = scale * r.getX();
        auto x2 = scale * r.getRight();
        auto y = r.getY() - bounds.getY();

        for (i32 j = r.getHeight(); --j >= 0;)
            addEdgePointPair (x1, x2, y++, 255);
    }

    sanitiseLevels (true);
}

EdgeTable::EdgeTable (const RectangleList<f32>& rectanglesToAdd)
   : bounds (rectanglesToAdd.getBounds().getSmallestIntegerContainer()),
     maxEdgesPerLine (rectanglesToAdd.getNumRectangles() * 2),
     lineStrideElements (rectanglesToAdd.getNumRectangles() * 4 + 1)
{
    bounds.setHeight (bounds.getHeight() + 1);
    allocate();
    clearLineSizes();

    for (auto& r : rectanglesToAdd)
    {
        auto x1 = roundToInt ((f32) scale * r.getX());
        auto x2 = roundToInt ((f32) scale * r.getRight());

        auto y1 = roundToInt ((f32) scale * r.getY())      - (bounds.getY() * scale);
        auto y2 = roundToInt ((f32) scale * r.getBottom()) - (bounds.getY() * scale);

        if (x2 <= x1 || y2 <= y1)
            continue;

        auto y        = y1 / scale;
        auto lastLine = y2 / scale;

        if (y == lastLine)
        {
            addEdgePointPair (x1, x2, y, y2 - y1);
        }
        else
        {
            addEdgePointPair (x1, x2, y++, 255 - (y1 & 255));

            while (y < lastLine)
                addEdgePointPair (x1, x2, y++, 255);

            jassert (y < bounds.getHeight());
            addEdgePointPair (x1, x2, y, y2 & 255);
        }
    }

    sanitiseLevels (true);
}

EdgeTable::EdgeTable (Rectangle<f32> rectangleToAdd)
   : bounds ((i32) std::floor (rectangleToAdd.getX()),
             roundToInt (rectangleToAdd.getY() * 256.0f) / scale,
             2 + (i32) rectangleToAdd.getWidth(),
             2 + (i32) rectangleToAdd.getHeight()),
     maxEdgesPerLine (defaultEdgesPerLine),
     lineStrideElements ((defaultEdgesPerLine * 2) + 1)
{
    jassert (! rectangleToAdd.isEmpty());
    allocate();
    table[0] = 0;

    auto x1 = roundToInt ((f32) scale * rectangleToAdd.getX());
    auto x2 = roundToInt ((f32) scale * rectangleToAdd.getRight());
    auto y1 = roundToInt ((f32) scale * rectangleToAdd.getY())      - (bounds.getY() * scale);
    auto y2 = roundToInt ((f32) scale * rectangleToAdd.getBottom()) - (bounds.getY() * scale);
    jassert (y1 < 256);

    if (x2 <= x1 || y2 <= y1)
    {
        bounds.setHeight (0);
        return;
    }

    i32 lineY = 0;
    i32* t = table.data();

    if ((y1 / scale) == (y2 / scale))
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = y2 - y1;
        t[3] = x2;
        t[4] = 0;
        ++lineY;
        t += lineStrideElements;
    }
    else
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = 255 - (y1 & 255);
        t[3] = x2;
        t[4] = 0;
        ++lineY;
        t += lineStrideElements;

        while (lineY < (y2 / scale))
        {
            t[0] = 2;
            t[1] = x1;
            t[2] = 255;
            t[3] = x2;
            t[4] = 0;
            ++lineY;
            t += lineStrideElements;
        }

        jassert (lineY < bounds.getHeight());
        t[0] = 2;
        t[1] = x1;
        t[2] = y2 & 255;
        t[3] = x2;
        t[4] = 0;
        ++lineY;
        t += lineStrideElements;
    }

    while (lineY < bounds.getHeight())
    {
        t[0] = 0;
        t += lineStrideElements;
        ++lineY;
    }
}

static z0 copyEdgeTableData (i32* dest,
                               size_t destLineStride,
                               i32k* src,
                               size_t srcLineStride,
                               size_t numLines) noexcept
{
    for (size_t line = 0; line < numLines; ++line)
    {
        const auto* srcLine = src + line * srcLineStride;
        std::copy (srcLine, srcLine + *srcLine * 2 + 1, dest + line * destLineStride);
    }
}

//==============================================================================
static size_t getEdgeTableAllocationSize (i32 lineStride, i32 height) noexcept
{
    // (leave an extra line at the end for use as scratch space)
    return (size_t) (lineStride * (2 + jmax (0, height)));
}

z0 EdgeTable::allocate()
{
    table = CopyableHeapBlock<i32> (getEdgeTableAllocationSize (lineStrideElements, bounds.getHeight()));
}

z0 EdgeTable::clearLineSizes() noexcept
{
    i32* t = table.data();

    for (i32 i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }
}

z0 EdgeTable::sanitiseLevels (const b8 useNonZeroWinding) noexcept
{
    // Convert the table from relative windings to absolute levels..
    i32* lineStart = table.data();

    for (i32 y = bounds.getHeight(); --y >= 0;)
    {
        auto num = lineStart[0];

        if (num > 0)
        {
            auto* items = reinterpret_cast<LineItem*> (lineStart + 1);
            auto* itemsEnd = items + num;

            // sort the X coords
            std::sort (items, itemsEnd);

            auto* src = items;
            auto correctedNum = num;
            i32 level = 0;

            while (src < itemsEnd)
            {
                level += src->level;
                auto x = src->x;
                ++src;

                while (src < itemsEnd && src->x == x)
                {
                    level += src->level;
                    ++src;
                    --correctedNum;
                }

                auto corrected = std::abs (level);

                if (corrected / scale)
                {
                    if (useNonZeroWinding)
                    {
                        corrected = 255;
                    }
                    else
                    {
                        corrected &= 511;

                        if (corrected / scale)
                            corrected = 511 - corrected;
                    }
                }

                items->x = x;
                items->level = corrected;
                ++items;
            }

            lineStart[0] = correctedNum;
            (items - 1)->level = 0; // force the last level to 0, just in case something went wrong in creating the table
        }

        lineStart += lineStrideElements;
    }
}

z0 EdgeTable::remapTableForNumEdges (i32k newNumEdgesPerLine)
{
    if (newNumEdgesPerLine != maxEdgesPerLine)
    {
        jassert (newNumEdgesPerLine > maxEdgesPerLine);
        maxEdgesPerLine = newNumEdgesPerLine;

        jassert (bounds.getHeight() > 0);
        auto newLineStrideElements = maxEdgesPerLine * 2 + 1;

        CopyableHeapBlock<i32> newTable (getEdgeTableAllocationSize (newLineStrideElements, bounds.getHeight()));
        copyEdgeTableData (newTable.data(),
                           (size_t) newLineStrideElements,
                           table.data(),
                           (size_t) lineStrideElements,
                           (size_t) bounds.getHeight());

        table = std::move (newTable);
        lineStrideElements = newLineStrideElements;
    }
}

inline z0 EdgeTable::remapWithExtraSpace (i32 numPoints)
{
    remapTableForNumEdges (numPoints * 2);
    jassert (numPoints < maxEdgesPerLine);
}

z0 EdgeTable::optimiseTable()
{
    i32 maxLineElements = 0;

    for (i32 i = bounds.getHeight(); --i >= 0;)
        maxLineElements = jmax (maxLineElements, table[(size_t) i * (size_t) lineStrideElements]);

    remapTableForNumEdges (maxLineElements);
}

z0 EdgeTable::addEdgePoint (i32k x, i32k y, i32k winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* line = table.data() + lineStrideElements * y;
    auto numPoints = line[0];

    if (numPoints >= maxEdgesPerLine)
    {
        remapWithExtraSpace (numPoints);
        line = table.data() + lineStrideElements * y;
    }

    line[0] = numPoints + 1;
    line += numPoints * 2;
    line[1] = x;
    line[2] = winding;
}

z0 EdgeTable::addEdgePointPair (i32 x1, i32 x2, i32 y, i32 winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* line = table.data() + lineStrideElements * y;
    auto numPoints = line[0];

    if (numPoints + 1 >= maxEdgesPerLine)
    {
        remapWithExtraSpace (numPoints + 1);
        line = table.data() + lineStrideElements * y;
    }

    line[0] = numPoints + 2;
    line += numPoints * 2;
    line[1] = x1;
    line[2] = winding;
    line[3] = x2;
    line[4] = -winding;
}

z0 EdgeTable::translate (f32 dx, i32 dy) noexcept
{
    bounds.translate ((i32) std::floor (dx), dy);

    i32* lineStart = table.data();
    auto intDx = (i32) (dx * 256.0f);

    for (i32 i = bounds.getHeight(); --i >= 0;)
    {
        auto* line = lineStart;
        lineStart += lineStrideElements;
        auto num = *line++;

        while (--num >= 0)
        {
            *line += intDx;
            line += 2;
        }
    }
}

z0 EdgeTable::multiplyLevels (f32 amount)
{
    i32* lineStart = table.data();
    auto multiplier = (i32) (amount * 256.0f);

    for (i32 y = 0; y < bounds.getHeight(); ++y)
    {
        auto numPoints = lineStart[0];

        for (auto i = 0; i < numPoints; ++i)
        {
            auto* ptr = lineStart + 1 + (2 * i);
            auto item = readUnaligned<LineItem> (ptr);
            item.level = jmin (255, (item.level * multiplier) / scale);
            writeUnaligned (ptr, item);
        }
    }
}

z0 EdgeTable::intersectWithEdgeTableLine (i32k y, i32k* const otherLine)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* srcLine = table.data() + lineStrideElements * y;
    const auto srcNum1 = *srcLine;

    if (srcNum1 == 0)
        return;

    const auto srcNum2 = *otherLine;

    if (srcNum2 == 0)
    {
        *srcLine = 0;
        return;
    }

    Span srcLine1 { srcLine   + 1, (size_t) srcNum1 * 2 };
    Span srcLine2 { otherLine + 1, (size_t) srcNum2 * 2 };

    const auto popHead = [] (auto& s)
    {
        if (s.empty())
            return 0;

        const auto result = s.front();
        s = Span { s.data() + 1, s.size() - 1 };
        return result;
    };

    const auto reseat = [] (auto& s, auto* ptr)
    {
        s = Span { ptr, s.size() };
    };

    const auto right = bounds.getRight() * scale;

    // optimise for the common case where our line lies entirely within a
    // single pair of points, as happens when clipping to a simple rect.
    if (srcLine2.size() == 4 && srcLine2[1] >= 255)
    {
        clipEdgeTableLineToRange (srcLine, srcLine2[0], jmin (right, srcLine2[2]));
        return;
    }

    b8 isUsingTempSpace = false;

    auto x1 = popHead (srcLine1);
    auto x2 = popHead (srcLine2);

    i32 destIndex = 0, destTotal = 0;
    i32 level1 = 0, level2 = 0;
    i32 lastLevel = 0;

    while (! srcLine1.empty() && ! srcLine2.empty())
    {
        i32 nextX;

        if (x1 <= x2)
        {
            if (x1 == x2)
            {
                level2 = popHead (srcLine2);
                x2 = popHead (srcLine2);
            }

            nextX = x1;
            level1 = popHead (srcLine1);
            x1 = popHead (srcLine1);
        }
        else
        {
            nextX = x2;
            level2 = popHead (srcLine2);
            x2 = popHead (srcLine2);
        }

        if (right <= nextX)
            break;

        const auto nextLevel = (level1 * (level2 + 1)) / scale;

        if (std::exchange (lastLevel, nextLevel) != nextLevel)
        {
            jassert (isPositiveAndBelow (nextLevel, 256));

            if (destTotal >= maxEdgesPerLine)
            {
                srcLine[0] = destTotal;

                if (isUsingTempSpace)
                {
                    auto* stackBuffer = static_cast<i32*> (alloca (sizeof (i32) * srcLine1.size()));
                    std::copy (srcLine1.begin(), srcLine1.end(), stackBuffer);

                    remapTableForNumEdges (jmax (256, destTotal * 2));
                    srcLine = table.data() + lineStrideElements * y;

                    reseat (srcLine1, table.data() + lineStrideElements * bounds.getHeight());
                    std::copy (stackBuffer, stackBuffer + srcLine1.size(), srcLine1.data());
                }
                else
                {
                    remapTableForNumEdges (jmax (256, destTotal * 2));
                    srcLine = table.data() + lineStrideElements * y;
                }
            }

            ++destTotal;

            if (! isUsingTempSpace)
            {
                isUsingTempSpace = true;
                auto* temp = table.data() + lineStrideElements * bounds.getHeight();
                std::copy (srcLine1.begin(), srcLine1.end(), temp);
                reseat (srcLine1, temp);
            }

            srcLine[++destIndex] = nextX;
            srcLine[++destIndex] = nextLevel;
        }
    }

    if (lastLevel > 0)
    {
        if (destTotal >= maxEdgesPerLine)
        {
            srcLine[0] = destTotal;
            remapTableForNumEdges (jmax (256, destTotal * 2));
            srcLine = table.data() + lineStrideElements * y;
        }

        ++destTotal;
        srcLine[++destIndex] = right;
        srcLine[++destIndex] = 0;
    }

    srcLine[0] = destTotal;
}

z0 EdgeTable::clipEdgeTableLineToRange (i32* dest, i32k x1, i32k x2) noexcept
{
    i32* lastItem = dest + (dest[0] * 2 - 1);

    if (x2 < lastItem[0])
    {
        if (x2 <= dest[1])
        {
            dest[0] = 0;
            return;
        }

        while (x2 < lastItem[-2])
        {
            --(dest[0]);
            lastItem -= 2;
        }

        lastItem[0] = x2;
        lastItem[1] = 0;
    }

    if (x1 > dest[1])
    {
        while (lastItem[0] > x1)
            lastItem -= 2;

        auto itemsRemoved = (i32) (lastItem - (dest + 1)) / 2;

        if (itemsRemoved > 0)
        {
            dest[0] -= itemsRemoved;
            memmove (dest + 1, lastItem, (size_t) dest[0] * (sizeof (i32) * 2));
        }

        dest[1] = x1;
    }
}


//==============================================================================
z0 EdgeTable::clipToRectangle (Rectangle<i32> r)
{
    auto clipped = r.getIntersection (bounds);

    if (clipped.isEmpty())
    {
        needToCheckEmptiness = false;
        bounds.setHeight (0);
    }
    else
    {
        auto top = clipped.getY() - bounds.getY();
        auto bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        for (i32 i = 0; i < top; ++i)
            table[(size_t) lineStrideElements * (size_t) i] = 0;

        if (clipped.getX() > bounds.getX() || clipped.getRight() < bounds.getRight())
        {
            auto x1 = scale * clipped.getX();
            auto x2 = scale * jmin (bounds.getRight(), clipped.getRight());
            i32* line = table.data() + lineStrideElements * top;

            for (i32 i = bottom - top; --i >= 0;)
            {
                if (line[0] != 0)
                    clipEdgeTableLineToRange (line, x1, x2);

                line += lineStrideElements;
            }
        }

        needToCheckEmptiness = true;
    }
}

z0 EdgeTable::excludeRectangle (Rectangle<i32> r)
{
    auto clipped = r.getIntersection (bounds);

    if (! clipped.isEmpty())
    {
        auto top = clipped.getY() - bounds.getY();
        auto bottom = clipped.getBottom() - bounds.getY();

        i32k rectLine[] = { 4, std::numeric_limits<i32>::min(), 255,
                                 scale * clipped.getX(), 0,
                                 scale * clipped.getRight(), 255,
                                 std::numeric_limits<i32>::max(), 0 };

        for (i32 i = top; i < bottom; ++i)
            intersectWithEdgeTableLine (i, rectLine);

        needToCheckEmptiness = true;
    }
}

z0 EdgeTable::clipToEdgeTable (const EdgeTable& other)
{
    auto clipped = other.bounds.getIntersection (bounds);

    if (clipped.isEmpty())
    {
        needToCheckEmptiness = false;
        bounds.setHeight (0);
    }
    else
    {
        auto top = clipped.getY() - bounds.getY();
        auto bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        if (clipped.getRight() < bounds.getRight())
            bounds.setRight (clipped.getRight());

        for (i32 i = 0; i < top; ++i)
            table[(size_t) lineStrideElements * (size_t) i] = 0;

        auto* otherLine = other.table.data() + other.lineStrideElements * (clipped.getY() - other.bounds.getY());

        for (i32 i = top; i < bottom; ++i)
        {
            intersectWithEdgeTableLine (i, otherLine);
            otherLine += other.lineStrideElements;
        }

        needToCheckEmptiness = true;
    }
}

z0 EdgeTable::clipLineToMask (i32 x, i32 y, u8k* mask, i32 maskStride, i32 numPixels)
{
    y -= bounds.getY();

    if (y < 0 || y >= bounds.getHeight())
        return;

    needToCheckEmptiness = true;

    if (numPixels <= 0)
    {
        table[(size_t) lineStrideElements * (size_t) y] = 0;
        return;
    }

    auto* tempLine = static_cast<i32*> (alloca ((size_t) (numPixels * 2 + 4) * sizeof (i32)));
    i32 destIndex = 0, lastLevel = 0;

    while (--numPixels >= 0)
    {
        auto alpha = *mask;
        mask += maskStride;

        if (alpha != lastLevel)
        {
            tempLine[++destIndex] = (x * scale);
            tempLine[++destIndex] = alpha;
            lastLevel = alpha;
        }

        ++x;
    }

    if (lastLevel > 0)
    {
        tempLine[++destIndex] = (x * scale);
        tempLine[++destIndex] = 0;
    }

    tempLine[0] = destIndex >> 1;

    intersectWithEdgeTableLine (y, tempLine);
}

b8 EdgeTable::isEmpty() noexcept
{
    if (needToCheckEmptiness)
    {
        needToCheckEmptiness = false;
        i32* t = table.data();

        for (i32 i = bounds.getHeight(); --i >= 0;)
        {
            if (t[0] > 1)
                return false;

            t += lineStrideElements;
        }

        bounds.setHeight (0);
    }

    return bounds.getHeight() == 0;
}

DRX_END_IGNORE_WARNINGS_MSVC

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class EdgeTableTests : public UnitTest
{
public:
    EdgeTableTests() : UnitTest ("EdgeTable", UnitTestCategories::graphics) {}

    z0 runTest() override
    {
        beginTest ("The result of clipToEdgeTable() shouldn't contain any point that's not present in both operands");
        {
            // The way this EdgeTable is constructed is significant in triggering a certain corner
            // case.
            const auto edgeTableContainingAPath = [&]
            {
                RectangleList<i32> rl;
                rl.add (Rectangle<i32> (6, 1, 1, 4));
                rl.add (Rectangle<i32> (1, 1, 5, 5));
                EdgeTable rectListEdgeTable { rl };

                Path p;
                p.startNewSubPath (2.0f, 6.0f);
                p.lineTo (2.0f, 1.0f);
                p.lineTo (6.0f, 1.0f);
                p.lineTo (6.0f, 6.0f);
                p.closeSubPath();

                const EdgeTable pathEdgeTable { Rectangle<i32> { 1, 1, 6, 5 }, p, {} };

                rectListEdgeTable.clipToEdgeTable (pathEdgeTable);
                return rectListEdgeTable;
            }();

            const EdgeTable edgeTableFromRectangle (Rectangle<f32> (1.0f, 1.0f, 6.0f, 5.0f));

            const auto intersection = [&]
            {
                auto result = edgeTableFromRectangle;
                result.clipToEdgeTable (edgeTableContainingAPath);
                return result;
            }();

            expect (! contains (edgeTableContainingAPath, { 6, 2 }),
                    "The path doesn't enclose the point (6, 2) so its EdgeTable shouldn't contain it");

            expect (contains (edgeTableFromRectangle, { 6, 2 }),
                    "The Rectangle covers the point (6, 2) so its EdgeTable should contain it");

            expect (! contains (intersection, { 6, 2 }),
                    "The intersecting EdgeTable shouldn't contain (6, 2) because one of its constituents doesn't contain it either");
        }

        beginTest ("An EdgeTable constructed from a pixel-aligned Rectangle should not anti-alias");
        {
            Rectangle<i32> area { 5, 5 };
            Path p;
            p.addRectangle (area.reduced (1));
            EdgeTable bordered { area, p, {} };

            // Pixels at edges should be clear
            expect (getLevel (bordered, { 0, 0 }) == 0);
            expect (getLevel (bordered, { 0, 4 }) == 0);
            expect (getLevel (bordered, { 4, 0 }) == 0);
            expect (getLevel (bordered, { 4, 4 }) == 0);

            // Corners of filled area should have max level
            expect (getLevel (bordered, { 1, 1 }) == 255);
            expect (getLevel (bordered, { 1, 3 }) == 255);
            expect (getLevel (bordered, { 3, 1 }) == 255);
            expect (getLevel (bordered, { 3, 3 }) == 255);

            Path q;
            q.addRectangle (area);
            EdgeTable filled { area, q, {} };

            // Pixels at edges should have max level
            expect (getLevel (filled, { 0, 0 }) == 255);
            expect (getLevel (filled, { 0, 4 }) == 255);
            expect (getLevel (filled, { 4, 0 }) == 255);
            expect (getLevel (filled, { 4, 4 }) == 255);
        }
    }

private:
    class EdgeTableFiller
    {
    public:
        EdgeTableFiller (i32 w, i32 h)
            : width (w), height (h), data ((size_t) (w * h))
        {
        }

        z0 setEdgeTableYPos (i32 yIn)
        {
            y = yIn;
        }

        z0 handleEdgeTablePixelFull (i32 x)
        {
            handleEdgeTablePixel (x, 255);
        }

        z0 handleEdgeTablePixel (i32 x, u8 level)
        {
            if (! (y < height && x < width))
                return;

            auto* ptr = data.data() + width * y + x;
            *ptr = level;
        }

        z0 handleEdgeTableLineFull (i32 x, i32 w)
        {
            handleEdgeTableLine (x, w, 255);
        }

        z0 handleEdgeTableLine (i32 x, i32 w, u8 level)
        {
            if (! (y < height && x < width))
                return;

            auto* ptr = data.data() + width * y + x;
            std::fill (ptr, ptr + std::min (w, width - x), level);
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 yIn, i32 w, i32 h) noexcept
        {
            handleEdgeTableRectangle (x, yIn, w, h, 255);
        }

        z0 handleEdgeTableRectangle (i32 x, i32 yIn, i32 w, i32 h, u8 level) noexcept
        {
            for (i32 j = yIn; j < std::min (yIn + h, height); ++j)
            {
                auto* ptr = data.data() + width * j + x;
                std::fill (ptr, ptr + std::min (w, width - x), level);
            }
        }

        u8 get (i32 x, i32 yIn) const
        {
            const auto index = (size_t) (width * yIn + x);

            if (index >= data.size())
                return 0;

            return data[index];
        }

    private:
        i32k width, height = 0;
        std::vector<u8> data;
        i32 y = 0;
    };

    static u8 getLevel (const EdgeTable& et, Point<i32> p)
    {
        EdgeTableFiller filler { p.getX() + 2, p.getY() + 2 };
        et.iterate (filler);
        return filler.get (p.getX(), p.getY());
    }

    static b8 contains (const EdgeTable& et, Point<i32> p)
    {
        return getLevel (et, p) == 255;
    }

};

static EdgeTableTests edgeTableTests;

#endif

} // namespace drx
