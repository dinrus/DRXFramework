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
    A table of horizontal scan-line segments - used for rasterising Paths.

    @see Path, Graphics

    @tags{Graphics}
*/
class DRX_API  EdgeTable
{
public:
    //==============================================================================
    /** Creates an edge table containing a path.

        A table is created with a fixed vertical range, and only sections of the path
        which lie within this range will be added to the table.

        @param clipLimits               only the region of the path that lies within this area will be added
        @param pathToAdd                the path to add to the table
        @param transform                a transform to apply to the path being added
    */
    EdgeTable (Rectangle<i32> clipLimits,
               const Path& pathToAdd,
               const AffineTransform& transform);

    /** Creates an edge table containing a rectangle. */
    explicit EdgeTable (Rectangle<i32> rectangleToAdd);

    /** Creates an edge table containing a rectangle. */
    explicit EdgeTable (Rectangle<f32> rectangleToAdd);

    /** Creates an edge table containing a rectangle list. */
    explicit EdgeTable (const RectangleList<i32>& rectanglesToAdd);

    /** Creates an edge table containing a rectangle list. */
    explicit EdgeTable (const RectangleList<f32>& rectanglesToAdd);

    //==============================================================================
    z0 clipToRectangle (Rectangle<i32> r);
    z0 excludeRectangle (Rectangle<i32> r);
    z0 clipToEdgeTable (const EdgeTable&);
    z0 clipLineToMask (i32 x, i32 y, u8k* mask, i32 maskStride, i32 numPixels);
    b8 isEmpty() noexcept;
    const Rectangle<i32>& getMaximumBounds() const noexcept      { return bounds; }
    z0 translate (f32 dx, i32 dy) noexcept;

    /** Scales all the alpha-levels in the table by the given multiplier. */
    z0 multiplyLevels (f32 factor);

    /** Reduces the amount of space the table has allocated.

        This will shrink the table down to use as little memory as possible - useful for
        read-only tables that get stored and re-used for rendering.
    */
    z0 optimiseTable();


    //==============================================================================
    /** Iterates the lines in the table, for rendering.

        This function will iterate each line in the table, and call a user-defined class
        to render each pixel or continuous line of pixels that the table contains.

        @param iterationCallback    this templated class must contain the following methods:
                                        @code
                                        inline z0 setEdgeTableYPos (i32 y);
                                        inline z0 handleEdgeTablePixel (i32 x, i32 alphaLevel) const;
                                        inline z0 handleEdgeTablePixelFull (i32 x) const;
                                        inline z0 handleEdgeTableLine (i32 x, i32 width, i32 alphaLevel) const;
                                        inline z0 handleEdgeTableLineFull (i32 x, i32 width) const;
                                        @endcode
                                        (these don't necessarily have to be 'const', but it might help it go faster)
    */
    template <class EdgeTableIterationCallback>
    z0 iterate (EdgeTableIterationCallback& iterationCallback) const noexcept
    {
        i32k* lineStart = table.data();

        for (i32 y = 0; y < bounds.getHeight(); ++y)
        {
            i32k* line = lineStart;
            lineStart += lineStrideElements;
            i32 numPoints = line[0];

            if (--numPoints > 0)
            {
                i32 x = *++line;
                jassert ((x / scale) >= bounds.getX() && (x / scale) < bounds.getRight());
                i32 levelAccumulator = 0;

                iterationCallback.setEdgeTableYPos (bounds.getY() + y);

                while (--numPoints >= 0)
                {
                    i32k level = *++line;
                    jassert (isPositiveAndBelow (level, scale));
                    i32k endX = *++line;
                    jassert (endX >= x);
                    i32k endOfRun = (endX / scale);

                    if (endOfRun == (x / scale))
                    {
                        // small segment within the same pixel, so just save it for the next
                        // time round..
                        levelAccumulator += (endX - x) * level;
                    }
                    else
                    {
                        // plot the fist pixel of this segment, including any accumulated
                        // levels from smaller segments that haven't been drawn yet
                        levelAccumulator += (0x100 - (x & 0xff)) * level;
                        levelAccumulator /= scale;
                        x /= scale;

                        if (levelAccumulator > 0)
                        {
                            if (levelAccumulator >= 255)
                                iterationCallback.handleEdgeTablePixelFull (x);
                            else
                                iterationCallback.handleEdgeTablePixel (x, static_cast<u8> (levelAccumulator));
                        }

                        // if there's a run of similar pixels, do it all in one go..
                        if (level > 0)
                        {
                            jassert (endOfRun <= bounds.getRight());
                            i32k numPix = endOfRun - ++x;

                            if (numPix > 0)
                                iterationCallback.handleEdgeTableLine (x, numPix, static_cast<u8> (level));
                        }

                        // save the bit at the end to be drawn next time round the loop.
                        levelAccumulator = (endX & 0xff) * level;
                    }

                    x = endX;
                }

                levelAccumulator /= scale;

                if (levelAccumulator > 0)
                {
                    x /= scale;
                    jassert (x >= bounds.getX() && x < bounds.getRight());

                    if (levelAccumulator >= 255)
                        iterationCallback.handleEdgeTablePixelFull (x);
                    else
                        iterationCallback.handleEdgeTablePixel (x, static_cast<u8> (levelAccumulator));
                }
            }
        }
    }

private:
    //==============================================================================
    static constexpr auto defaultEdgesPerLine = 32;
    static constexpr auto scale = 256;

    //==============================================================================
    // table line format: number of points; point0 x, point0 levelDelta, point1 x, point1 levelDelta, etc
    struct LineItem
    {
        i32 x, level;

        b8 operator< (const LineItem& other) const noexcept   { return x < other.x; }
    };

    CopyableHeapBlock<i32> table;
    Rectangle<i32> bounds;
    i32 maxEdgesPerLine, lineStrideElements;
    b8 needToCheckEmptiness = true;

    z0 allocate();
    z0 clearLineSizes() noexcept;
    z0 addEdgePoint (i32 x, i32 y, i32 winding);
    z0 addEdgePointPair (i32 x1, i32 x2, i32 y, i32 winding);
    z0 remapTableForNumEdges (i32 newNumEdgesPerLine);
    z0 remapWithExtraSpace (i32 numPointsNeeded);
    z0 intersectWithEdgeTableLine (i32 y, i32k* otherLine);
    z0 clipEdgeTableLineToRange (i32* line, i32 x1, i32 x2) noexcept;
    z0 sanitiseLevels (b8 useNonZeroWinding) noexcept;

    DRX_LEAK_DETECTOR (EdgeTable)
};

} // namespace drx
