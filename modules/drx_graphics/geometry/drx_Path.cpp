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

// tests that some coordinates aren't NaNs
#define DRX_CHECK_COORDS_ARE_VALID(x, y) \
    jassert (! std::isnan (x) && ! std::isnan (y));

//==============================================================================
namespace PathHelpers
{
    const f32 ellipseAngularIncrement = 0.05f;

    static Txt nextToken (Txt::CharPointerType& t)
    {
        t.incrementToEndOfWhitespace();

        auto start = t;
        size_t numChars = 0;

        while (! (t.isEmpty() || t.isWhitespace()))
        {
            ++t;
            ++numChars;
        }

        return { start, numChars };
    }

    inline f64 lengthOf (f32 x1, f32 y1, f32 x2, f32 y2) noexcept
    {
        return drx_hypot ((f64) (x1 - x2), (f64) (y1 - y2));
    }
}

//==============================================================================

const f32 Path::defaultToleranceForTesting = 1.0f;
const f32 Path::defaultToleranceForMeasurement = 0.6f;

static b8 isMarker (f32 value, f32 marker) noexcept
{
    return exactlyEqual (value, marker);
}

//==============================================================================
Path::PathBounds::PathBounds() noexcept = default;

Rectangle<f32> Path::PathBounds::getRectangle() const noexcept
{
    return { pathXMin, pathYMin, pathXMax - pathXMin, pathYMax - pathYMin };
}

z0 Path::PathBounds::reset() noexcept
{
    *this = {};
}

z0 Path::PathBounds::reset (f32 x, f32 y) noexcept
{
    pathXMin = pathXMax = x;
    pathYMin = pathYMax = y;
}

z0 Path::PathBounds::extend (f32 x, f32 y) noexcept
{
    if (x < pathXMin)      pathXMin = x;
    else if (x > pathXMax) pathXMax = x;

    if (y < pathYMin)      pathYMin = y;
    else if (y > pathYMax) pathYMax = y;
}

//==============================================================================
Path::Path() = default;

Path::~Path() = default;

Path::Path (const Path& other)
    : data (other.data),
      bounds (other.bounds),
      useNonZeroWinding (other.useNonZeroWinding)
{
}

Path& Path::operator= (const Path& other)
{
    auto copy = other;
    *this = std::move (copy);
    return *this;
}

Path::Path (Path&& other) noexcept
    : data (std::exchange (other.data, {})),
      bounds (std::exchange (other.bounds, {})),
      useNonZeroWinding (std::exchange (other.useNonZeroWinding, {}))
{
}

Path& Path::operator= (Path&& other) noexcept
{
    auto copy = std::move (other);
    swapWithPath (copy);
    return *this;
}

b8 Path::operator== (const Path& other) const noexcept
{
    const auto tie = [] (const auto& x) { return std::tie (x.useNonZeroWinding, x.data); };
    return tie (*this) == tie (other);
}

b8 Path::operator!= (const Path& other) const noexcept    { return ! operator== (other); }

z0 Path::clear() noexcept
{
    data.clearQuick();
    bounds.reset();
}

z0 Path::swapWithPath (Path& other) noexcept
{
    data.swapWith (other.data);
    std::swap (bounds, other.bounds);
    std::swap (useNonZeroWinding, other.useNonZeroWinding);
}

//==============================================================================
z0 Path::setUsingNonZeroWinding (const b8 isNonZero) noexcept
{
    useNonZeroWinding = isNonZero;
}

z0 Path::scaleToFit (f32 x, f32 y, f32 w, f32 h, b8 preserveProportions) noexcept
{
    applyTransform (getTransformToScaleToFit (x, y, w, h, preserveProportions));
}

//==============================================================================
b8 Path::isEmpty() const noexcept
{
    for (auto i = data.begin(), e = data.end(); i != e; ++i)
    {
        auto type = *i;

        if (isMarker (type, moveMarker))
        {
            i += 2;
        }
        else if (isMarker (type, lineMarker)
                 || isMarker (type, quadMarker)
                 || isMarker (type, cubicMarker))
        {
            return false;
        }
    }

    return true;
}

Rectangle<f32> Path::getBounds() const noexcept
{
    return bounds.getRectangle();
}

Rectangle<f32> Path::getBoundsTransformed (const AffineTransform& transform) const noexcept
{
    return getBounds().transformedBy (transform);
}

//==============================================================================
z0 Path::preallocateSpace (i32 numExtraCoordsToMakeSpaceFor)
{
    data.ensureStorageAllocated (data.size() + numExtraCoordsToMakeSpaceFor);
}

z0 Path::startNewSubPath (const f32 x, const f32 y)
{
    DRX_CHECK_COORDS_ARE_VALID (x, y)

    if (data.isEmpty())
        bounds.reset (x, y);
    else
        bounds.extend (x, y);

    data.add (moveMarker, x, y);
}

z0 Path::startNewSubPath (Point<f32> start)
{
    startNewSubPath (start.x, start.y);
}

z0 Path::lineTo (const f32 x, const f32 y)
{
    DRX_CHECK_COORDS_ARE_VALID (x, y)

    if (data.isEmpty())
        startNewSubPath (0, 0);

    data.add (lineMarker, x, y);
    bounds.extend (x, y);
}

z0 Path::lineTo (Point<f32> end)
{
    lineTo (end.x, end.y);
}

z0 Path::quadraticTo (const f32 x1, const f32 y1,
                        const f32 x2, const f32 y2)
{
    DRX_CHECK_COORDS_ARE_VALID (x1, y1)
    DRX_CHECK_COORDS_ARE_VALID (x2, y2)

    if (data.isEmpty())
        startNewSubPath (0, 0);

    data.add (quadMarker, x1, y1, x2, y2);
    bounds.extend (x1, y1, x2, y2);
}

z0 Path::quadraticTo (Point<f32> controlPoint, Point<f32> endPoint)
{
    quadraticTo (controlPoint.x, controlPoint.y,
                 endPoint.x, endPoint.y);
}

z0 Path::cubicTo (const f32 x1, const f32 y1,
                    const f32 x2, const f32 y2,
                    const f32 x3, const f32 y3)
{
    DRX_CHECK_COORDS_ARE_VALID (x1, y1)
    DRX_CHECK_COORDS_ARE_VALID (x2, y2)
    DRX_CHECK_COORDS_ARE_VALID (x3, y3)

    if (data.isEmpty())
        startNewSubPath (0, 0);

    data.add (cubicMarker, x1, y1, x2, y2, x3, y3);
    bounds.extend (x1, y1, x2, y2, x3, y3);
}

z0 Path::cubicTo (Point<f32> controlPoint1,
                    Point<f32> controlPoint2,
                    Point<f32> endPoint)
{
    cubicTo (controlPoint1.x, controlPoint1.y,
             controlPoint2.x, controlPoint2.y,
             endPoint.x, endPoint.y);
}

z0 Path::closeSubPath()
{
    if (! (data.isEmpty() || isMarker (data.getLast(), closeSubPathMarker)))
        data.add (closeSubPathMarker);
}

Point<f32> Path::getCurrentPosition() const
{
    if (data.isEmpty())
        return {};

    auto* i = data.end() - 1;

    if (isMarker (*i, closeSubPathMarker))
    {
        while (i != data.begin())
        {
            if (isMarker (*--i, moveMarker))
            {
                i += 2;
                break;
            }
        }
    }

    if (i != data.begin())
        return { *(i - 1), *i };

    return {};
}

z0 Path::addRectangle (f32 x, f32 y, f32 w, f32 h)
{
    auto x1 = x, y1 = y, x2 = x + w, y2 = y + h;

    if (w < 0) std::swap (x1, x2);
    if (h < 0) std::swap (y1, y2);

    if (data.isEmpty())
    {
        bounds.pathXMin = x1;
        bounds.pathXMax = x2;
        bounds.pathYMin = y1;
        bounds.pathYMax = y2;
    }
    else
    {
        bounds.pathXMin = jmin (bounds.pathXMin, x1);
        bounds.pathXMax = jmax (bounds.pathXMax, x2);
        bounds.pathYMin = jmin (bounds.pathYMin, y1);
        bounds.pathYMax = jmax (bounds.pathYMax, y2);
    }

    data.add (moveMarker, x1, y2,
              lineMarker, x1, y1,
              lineMarker, x2, y1,
              lineMarker, x2, y2,
              closeSubPathMarker);
}

z0 Path::addRoundedRectangle (f32 x, f32 y, f32 w, f32 h, f32 csx, f32 csy)
{
    addRoundedRectangle (x, y, w, h, csx, csy, true, true, true, true);
}

z0 Path::addRoundedRectangle (const f32 x, const f32 y, const f32 w, const f32 h,
                                f32 csx, f32 csy,
                                const b8 curveTopLeft, const b8 curveTopRight,
                                const b8 curveBottomLeft, const b8 curveBottomRight)
{
    csx = jmin (csx, w * 0.5f);
    csy = jmin (csy, h * 0.5f);
    auto cs45x = csx * 0.45f;
    auto cs45y = csy * 0.45f;
    auto x2 = x + w;
    auto y2 = y + h;

    if (curveTopLeft)
    {
        startNewSubPath (x, y + csy);
        cubicTo (x, y + cs45y, x + cs45x, y, x + csx, y);
    }
    else
    {
        startNewSubPath (x, y);
    }

    if (curveTopRight)
    {
        lineTo (x2 - csx, y);
        cubicTo (x2 - cs45x, y, x2, y + cs45y, x2, y + csy);
    }
    else
    {
        lineTo (x2, y);
    }

    if (curveBottomRight)
    {
        lineTo (x2, y2 - csy);
        cubicTo (x2, y2 - cs45y, x2 - cs45x, y2, x2 - csx, y2);
    }
    else
    {
        lineTo (x2, y2);
    }

    if (curveBottomLeft)
    {
        lineTo (x + csx, y2);
        cubicTo (x + cs45x, y2, x, y2 - cs45y, x, y2 - csy);
    }
    else
    {
        lineTo (x, y2);
    }

    closeSubPath();
}

z0 Path::addRoundedRectangle (f32 x, f32 y, f32 w, f32 h, f32 cs)
{
    addRoundedRectangle (x, y, w, h, cs, cs);
}

z0 Path::addTriangle (f32 x1, f32 y1,
                        f32 x2, f32 y2,
                        f32 x3, f32 y3)
{
    addTriangle ({ x1, y1 },
                 { x2, y2 },
                 { x3, y3 });
}

z0 Path::addTriangle (Point<f32> p1, Point<f32> p2, Point<f32> p3)
{
    startNewSubPath (p1);
    lineTo (p2);
    lineTo (p3);
    closeSubPath();
}

z0 Path::addQuadrilateral (f32 x1, f32 y1,
                             f32 x2, f32 y2,
                             f32 x3, f32 y3,
                             f32 x4, f32 y4)
{
    startNewSubPath (x1, y1);
    lineTo (x2, y2);
    lineTo (x3, y3);
    lineTo (x4, y4);
    closeSubPath();
}

z0 Path::addEllipse (f32 x, f32 y, f32 w, f32 h)
{
    addEllipse ({ x, y, w, h });
}

z0 Path::addEllipse (Rectangle<f32> area)
{
    auto hw = area.getWidth() * 0.5f;
    auto hw55 = hw * 0.55f;
    auto hh = area.getHeight() * 0.5f;
    auto hh55 = hh * 0.55f;
    auto cx = area.getX() + hw;
    auto cy = area.getY() + hh;

    startNewSubPath (cx, cy - hh);
    cubicTo (cx + hw55, cy - hh, cx + hw, cy - hh55, cx + hw, cy);
    cubicTo (cx + hw, cy + hh55, cx + hw55, cy + hh, cx, cy + hh);
    cubicTo (cx - hw55, cy + hh, cx - hw, cy + hh55, cx - hw, cy);
    cubicTo (cx - hw, cy - hh55, cx - hw55, cy - hh, cx, cy - hh);
    closeSubPath();
}

z0 Path::addArc (f32 x, f32 y, f32 w, f32 h,
                   f32 fromRadians, f32 toRadians,
                   b8 startAsNewSubPath)
{
    auto radiusX = w / 2.0f;
    auto radiusY = h / 2.0f;

    addCentredArc (x + radiusX,
                   y + radiusY,
                   radiusX, radiusY,
                   0.0f,
                   fromRadians, toRadians,
                   startAsNewSubPath);
}

z0 Path::addCentredArc (f32 centreX, f32 centreY,
                          f32 radiusX, f32 radiusY,
                          f32 rotationOfEllipse,
                          f32 fromRadians, f32 toRadians,
                          b8 startAsNewSubPath)
{
    if (radiusX > 0.0f && radiusY > 0.0f)
    {
        Point<f32> centre (centreX, centreY);
        auto rotation = AffineTransform::rotation (rotationOfEllipse, centreX, centreY);
        auto angle = fromRadians;

        if (startAsNewSubPath)
            startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));

        if (fromRadians < toRadians)
        {
            if (startAsNewSubPath)
                angle += PathHelpers::ellipseAngularIncrement;

            while (angle < toRadians)
            {
                lineTo (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));
                angle += PathHelpers::ellipseAngularIncrement;
            }
        }
        else
        {
            if (startAsNewSubPath)
                angle -= PathHelpers::ellipseAngularIncrement;

            while (angle > toRadians)
            {
                lineTo (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));
                angle -= PathHelpers::ellipseAngularIncrement;
            }
        }

        lineTo (centre.getPointOnCircumference (radiusX, radiusY, toRadians).transformedBy (rotation));
    }
}

z0 Path::addPieSegment (f32 x, f32 y, f32 width, f32 height,
                          f32 fromRadians, f32 toRadians,
                          f32 innerCircleProportionalSize)
{
    auto radiusX = width * 0.5f;
    auto radiusY = height * 0.5f;
    Point<f32> centre (x + radiusX, y + radiusY);

    startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, fromRadians));
    addArc (x, y, width, height, fromRadians, toRadians);

    if (std::abs (fromRadians - toRadians) > MathConstants<f32>::pi * 1.999f)
    {
        closeSubPath();

        if (innerCircleProportionalSize > 0)
        {
            radiusX *= innerCircleProportionalSize;
            radiusY *= innerCircleProportionalSize;

            startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, toRadians));
            addArc (centre.x - radiusX, centre.y - radiusY, radiusX * 2.0f, radiusY * 2.0f, toRadians, fromRadians);
        }
    }
    else
    {
        if (innerCircleProportionalSize > 0)
        {
            radiusX *= innerCircleProportionalSize;
            radiusY *= innerCircleProportionalSize;

            addArc (centre.x - radiusX, centre.y - radiusY, radiusX * 2.0f, radiusY * 2.0f, toRadians, fromRadians);
        }
        else
        {
            lineTo (centre);
        }
    }

    closeSubPath();
}

z0 Path::addPieSegment (Rectangle<f32> segmentBounds,
                          f32 fromRadians, f32 toRadians,
                          f32 innerCircleProportionalSize)
{
    addPieSegment (segmentBounds.getX(),
                   segmentBounds.getY(),
                   segmentBounds.getWidth(),
                   segmentBounds.getHeight(),
                   fromRadians,
                   toRadians,
                   innerCircleProportionalSize);
}

//==============================================================================
z0 Path::addLineSegment (Line<f32> line, f32 lineThickness)
{
    auto reversed = line.reversed();
    lineThickness *= 0.5f;

    startNewSubPath (line.getPointAlongLine (0, lineThickness));
    lineTo (line.getPointAlongLine (0, -lineThickness));
    lineTo (reversed.getPointAlongLine (0, lineThickness));
    lineTo (reversed.getPointAlongLine (0, -lineThickness));
    closeSubPath();
}

z0 Path::addArrow (Line<f32> line, f32 lineThickness,
                     f32 arrowheadWidth, f32 arrowheadLength)
{
    auto reversed = line.reversed();
    lineThickness *= 0.5f;
    arrowheadWidth *= 0.5f;
    arrowheadLength = jmin (arrowheadLength, 0.8f * line.getLength());

    startNewSubPath (line.getPointAlongLine (0, lineThickness));
    lineTo (line.getPointAlongLine (0, -lineThickness));
    lineTo (reversed.getPointAlongLine (arrowheadLength, lineThickness));
    lineTo (reversed.getPointAlongLine (arrowheadLength, arrowheadWidth));
    lineTo (line.getEnd());
    lineTo (reversed.getPointAlongLine (arrowheadLength, -arrowheadWidth));
    lineTo (reversed.getPointAlongLine (arrowheadLength, -lineThickness));
    closeSubPath();
}

z0 Path::addPolygon (Point<f32> centre, i32 numberOfSides,
                       f32 radius, f32 startAngle)
{
    jassert (numberOfSides > 1); // this would be silly.

    if (numberOfSides > 1)
    {
        auto angleBetweenPoints = MathConstants<f32>::twoPi / (f32) numberOfSides;

        for (i32 i = 0; i < numberOfSides; ++i)
        {
            auto angle = startAngle + (f32) i * angleBetweenPoints;
            auto p = centre.getPointOnCircumference (radius, angle);

            if (i == 0)
                startNewSubPath (p);
            else
                lineTo (p);
        }

        closeSubPath();
    }
}

z0 Path::addStar (Point<f32> centre, i32 numberOfPoints, f32 innerRadius,
                    f32 outerRadius, f32 startAngle)
{
    jassert (numberOfPoints > 1); // this would be silly.

    if (numberOfPoints > 1)
    {
        auto angleBetweenPoints = MathConstants<f32>::twoPi / (f32) numberOfPoints;

        for (i32 i = 0; i < numberOfPoints; ++i)
        {
            auto angle = startAngle + (f32) i * angleBetweenPoints;
            auto p = centre.getPointOnCircumference (outerRadius, angle);

            if (i == 0)
                startNewSubPath (p);
            else
                lineTo (p);

            lineTo (centre.getPointOnCircumference (innerRadius, angle + angleBetweenPoints * 0.5f));
        }

        closeSubPath();
    }
}

z0 Path::addBubble (Rectangle<f32> bodyArea,
                      Rectangle<f32> maximumArea,
                      Point<f32> arrowTip,
                      f32 cornerSize,
                      f32 arrowBaseWidth)
{
    auto halfW = bodyArea.getWidth() / 2.0f;
    auto halfH = bodyArea.getHeight() / 2.0f;
    auto cornerSizeW = jmin (cornerSize, halfW);
    auto cornerSizeH = jmin (cornerSize, halfH);
    auto cornerSizeW2 = 2.0f * cornerSizeW;
    auto cornerSizeH2 = 2.0f * cornerSizeH;

    startNewSubPath (bodyArea.getX() + cornerSizeW, bodyArea.getY());

    auto targetLimit = bodyArea.reduced (jmin (halfW - 1.0f, cornerSizeW + arrowBaseWidth),
                                         jmin (halfH - 1.0f, cornerSizeH + arrowBaseWidth));

    if (Rectangle<f32> (targetLimit.getX(), maximumArea.getY(),
                          targetLimit.getWidth(), bodyArea.getY() - maximumArea.getY()).contains (arrowTip))
    {
        lineTo (arrowTip.x - arrowBaseWidth, bodyArea.getY());
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (arrowTip.x + arrowBaseWidth, bodyArea.getY());
    }

    lineTo (bodyArea.getRight() - cornerSizeW, bodyArea.getY());
    addArc (bodyArea.getRight() - cornerSizeW2, bodyArea.getY(), cornerSizeW2, cornerSizeH2, 0, MathConstants<f32>::halfPi);

    if (Rectangle<f32> (bodyArea.getRight(), targetLimit.getY(),
                          maximumArea.getRight() - bodyArea.getRight(), targetLimit.getHeight()).contains (arrowTip))
    {
        lineTo (bodyArea.getRight(), arrowTip.y - arrowBaseWidth);
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (bodyArea.getRight(), arrowTip.y + arrowBaseWidth);
    }

    lineTo (bodyArea.getRight(), bodyArea.getBottom() - cornerSizeH);
    addArc (bodyArea.getRight() - cornerSizeW2, bodyArea.getBottom() - cornerSizeH2, cornerSizeW2, cornerSizeH2, MathConstants<f32>::halfPi, MathConstants<f32>::pi);

    if (Rectangle<f32> (targetLimit.getX(), bodyArea.getBottom(),
                          targetLimit.getWidth(), maximumArea.getBottom() - bodyArea.getBottom()).contains (arrowTip))
    {
        lineTo (arrowTip.x + arrowBaseWidth, bodyArea.getBottom());
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (arrowTip.x - arrowBaseWidth, bodyArea.getBottom());
    }

    lineTo (bodyArea.getX() + cornerSizeW, bodyArea.getBottom());
    addArc (bodyArea.getX(), bodyArea.getBottom() - cornerSizeH2, cornerSizeW2, cornerSizeH2, MathConstants<f32>::pi, MathConstants<f32>::pi * 1.5f);

    if (Rectangle<f32> (maximumArea.getX(), targetLimit.getY(),
                          bodyArea.getX() - maximumArea.getX(), targetLimit.getHeight()).contains (arrowTip))
    {
        lineTo (bodyArea.getX(), arrowTip.y + arrowBaseWidth);
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (bodyArea.getX(), arrowTip.y - arrowBaseWidth);
    }

    lineTo (bodyArea.getX(), bodyArea.getY() + cornerSizeH);
    addArc (bodyArea.getX(), bodyArea.getY(), cornerSizeW2, cornerSizeH2, MathConstants<f32>::pi * 1.5f, MathConstants<f32>::twoPi - 0.05f);

    closeSubPath();
}

z0 Path::addPath (const Path& other)
{
    const auto* d = other.data.begin();
    const auto size = other.data.size();

    for (i32 i = 0; i < size;)
    {
        const auto type = d[i++];

        if (isMarker (type, moveMarker))
        {
            startNewSubPath (d[i], d[i + 1]);
            i += 2;
        }
        else if (isMarker (type, lineMarker))
        {
            lineTo (d[i], d[i + 1]);
            i += 2;
        }
        else if (isMarker (type, quadMarker))
        {
            quadraticTo (d[i], d[i + 1], d[i + 2], d[i + 3]);
            i += 4;
        }
        else if (isMarker (type, cubicMarker))
        {
            cubicTo (d[i], d[i + 1], d[i + 2], d[i + 3], d[i + 4], d[i + 5]);
            i += 6;
        }
        else if (isMarker (type, closeSubPathMarker))
        {
            closeSubPath();
        }
        else
        {
            // something's gone wrong with the element list!
            jassertfalse;
        }
    }
}

z0 Path::addPath (const Path& other,
                    const AffineTransform& transformToApply)
{
    const auto* d = other.data.begin();
    const auto size = other.data.size();

    for (i32 i = 0; i < size;)
    {
        const auto type = d[i++];

        if (isMarker (type, closeSubPathMarker))
        {
            closeSubPath();
        }
        else
        {
            auto x = d[i++];
            auto y = d[i++];
            transformToApply.transformPoint (x, y);

            if (isMarker (type, moveMarker))
            {
                startNewSubPath (x, y);
            }
            else if (isMarker (type, lineMarker))
            {
                lineTo (x, y);
            }
            else if (isMarker (type, quadMarker))
            {
                auto x2 = d[i++];
                auto y2 = d[i++];
                transformToApply.transformPoint (x2, y2);

                quadraticTo (x, y, x2, y2);
            }
            else if (isMarker (type, cubicMarker))
            {
                auto x2 = d[i++];
                auto y2 = d[i++];
                auto x3 = d[i++];
                auto y3 = d[i++];
                transformToApply.transformPoints (x2, y2, x3, y3);

                cubicTo (x, y, x2, y2, x3, y3);
            }
            else
            {
                // something's gone wrong with the element list!
                jassertfalse;
            }
        }
    }
}

//==============================================================================
z0 Path::applyTransform (const AffineTransform& transform) noexcept
{
    bounds.reset();
    b8 firstPoint = true;
    f32* d = data.begin();
    auto* end = data.end();

    while (d < end)
    {
        auto type = *d++;

        if (isMarker (type, moveMarker))
        {
            transform.transformPoint (d[0], d[1]);
            DRX_CHECK_COORDS_ARE_VALID (d[0], d[1])

            if (firstPoint)
            {
                firstPoint = false;
                bounds.reset (d[0], d[1]);
            }
            else
            {
                bounds.extend (d[0], d[1]);
            }

            d += 2;
        }
        else if (isMarker (type, lineMarker))
        {
            transform.transformPoint (d[0], d[1]);
            DRX_CHECK_COORDS_ARE_VALID (d[0], d[1])
            bounds.extend (d[0], d[1]);
            d += 2;
        }
        else if (isMarker (type, quadMarker))
        {
            transform.transformPoints (d[0], d[1], d[2], d[3]);
            DRX_CHECK_COORDS_ARE_VALID (d[0], d[1])
            DRX_CHECK_COORDS_ARE_VALID (d[2], d[3])
            bounds.extend (d[0], d[1], d[2], d[3]);
            d += 4;
        }
        else if (isMarker (type, cubicMarker))
        {
            transform.transformPoints (d[0], d[1], d[2], d[3], d[4], d[5]);
            DRX_CHECK_COORDS_ARE_VALID (d[0], d[1])
            DRX_CHECK_COORDS_ARE_VALID (d[2], d[3])
            DRX_CHECK_COORDS_ARE_VALID (d[4], d[5])
            bounds.extend (d[0], d[1], d[2], d[3], d[4], d[5]);
            d += 6;
        }
    }
}


//==============================================================================
AffineTransform Path::getTransformToScaleToFit (Rectangle<f32> area, b8 preserveProportions,
                                                Justification justification) const
{
    return getTransformToScaleToFit (area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                                     preserveProportions, justification);
}

AffineTransform Path::getTransformToScaleToFit (f32 x, f32 y, f32 w, f32 h,
                                                b8 preserveProportions,
                                                Justification justification) const
{
    auto boundsRect = getBounds();

    if (preserveProportions)
    {
        if (w <= 0 || h <= 0 || boundsRect.isEmpty())
            return AffineTransform();

        f32 newW, newH;
        auto srcRatio = boundsRect.getHeight() / boundsRect.getWidth();

        if (srcRatio > h / w)
        {
            newW = h / srcRatio;
            newH = h;
        }
        else
        {
            newW = w;
            newH = w * srcRatio;
        }

        auto newXCentre = x;
        auto newYCentre = y;

        if (justification.testFlags (Justification::left))          newXCentre += newW * 0.5f;
        else if (justification.testFlags (Justification::right))    newXCentre += w - newW * 0.5f;
        else                                                        newXCentre += w * 0.5f;

        if (justification.testFlags (Justification::top))           newYCentre += newH * 0.5f;
        else if (justification.testFlags (Justification::bottom))   newYCentre += h - newH * 0.5f;
        else                                                        newYCentre += h * 0.5f;

        return AffineTransform::translation (boundsRect.getWidth()  * -0.5f - boundsRect.getX(),
                                             boundsRect.getHeight() * -0.5f - boundsRect.getY())
                    .scaled (newW / boundsRect.getWidth(),
                             newH / boundsRect.getHeight())
                    .translated (newXCentre, newYCentre);
    }
    else
    {
        return AffineTransform::translation (-boundsRect.getX(), -boundsRect.getY())
                    .scaled (w / boundsRect.getWidth(),
                             h / boundsRect.getHeight())
                    .translated (x, y);
    }
}

//==============================================================================
b8 Path::contains (f32 x, f32 y, f32 tolerance) const
{
    if (x <= bounds.pathXMin || x >= bounds.pathXMax
         || y <= bounds.pathYMin || y >= bounds.pathYMax)
        return false;

    PathFlatteningIterator i (*this, AffineTransform(), tolerance);

    i32 positiveCrossings = 0;
    i32 negativeCrossings = 0;

    while (i.next())
    {
        if ((i.y1 <= y && i.y2 > y) || (i.y2 <= y && i.y1 > y))
        {
            auto intersectX = i.x1 + (i.x2 - i.x1) * (y - i.y1) / (i.y2 - i.y1);

            if (intersectX <= x)
            {
                if (i.y1 < i.y2)
                    ++positiveCrossings;
                else
                    ++negativeCrossings;
            }
        }
    }

    return isUsingNonZeroWinding() ? (negativeCrossings != positiveCrossings)
                                   : ((negativeCrossings + positiveCrossings) & 1) != 0;
}

b8 Path::contains (Point<f32> point, f32 tolerance) const
{
    return contains (point.x, point.y, tolerance);
}

b8 Path::intersectsLine (Line<f32> line, f32 tolerance) const
{
    PathFlatteningIterator i (*this, AffineTransform(), tolerance);
    Point<f32> intersection;

    while (i.next())
        if (line.intersects (Line<f32> (i.x1, i.y1, i.x2, i.y2), intersection))
            return true;

    return false;
}

Line<f32> Path::getClippedLine (Line<f32> line, b8 keepSectionOutsidePath) const
{
    Line<f32> result (line);
    const b8 startInside = contains (line.getStart());
    const b8 endInside   = contains (line.getEnd());

    if (startInside == endInside)
    {
        if (keepSectionOutsidePath == startInside)
            result = Line<f32>();
    }
    else
    {
        PathFlatteningIterator i (*this, AffineTransform());
        Point<f32> intersection;

        while (i.next())
        {
            if (line.intersects ({ i.x1, i.y1, i.x2, i.y2 }, intersection))
            {
                if ((startInside && keepSectionOutsidePath) || (endInside && ! keepSectionOutsidePath))
                    result.setStart (intersection);
                else
                    result.setEnd (intersection);
            }
        }
    }

    return result;
}

f32 Path::getLength (const AffineTransform& transform, f32 tolerance) const
{
    f32 length = 0;
    PathFlatteningIterator i (*this, transform, tolerance);

    while (i.next())
        length += Line<f32> (i.x1, i.y1, i.x2, i.y2).getLength();

    return length;
}

Point<f32> Path::getPointAlongPath (f32 distanceFromStart,
                                      const AffineTransform& transform,
                                      f32 tolerance) const
{
    PathFlatteningIterator i (*this, transform, tolerance);

    while (i.next())
    {
        const Line<f32> line (i.x1, i.y1, i.x2, i.y2);
        auto lineLength = line.getLength();

        if (distanceFromStart <= lineLength)
            return line.getPointAlongLine (distanceFromStart);

        distanceFromStart -= lineLength;
    }

    return { i.x2, i.y2 };
}

f32 Path::getNearestPoint (Point<f32> targetPoint, Point<f32>& pointOnPath,
                             const AffineTransform& transform,
                             f32 tolerance) const
{
    PathFlatteningIterator i (*this, transform, tolerance);
    f32 bestPosition = 0, bestDistance = std::numeric_limits<f32>::max();
    f32 length = 0;
    Point<f32> pointOnLine;

    while (i.next())
    {
        const Line<f32> line (i.x1, i.y1, i.x2, i.y2);
        auto distance = line.getDistanceFromPoint (targetPoint, pointOnLine);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestPosition = length + pointOnLine.getDistanceFrom (line.getStart());
            pointOnPath = pointOnLine;
        }

        length += line.getLength();
    }

    return bestPosition;
}

//==============================================================================
Path Path::createPathWithRoundedCorners (const f32 cornerRadius) const
{
    if (cornerRadius <= 0.01f)
        return *this;

    Path p;
    i32 n = 0, indexOfPathStart = 0, indexOfPathStartThis = 0;
    auto* elements = data.begin();
    b8 lastWasLine = false, firstWasLine = false;

    while (n < data.size())
    {
        auto type = elements[n++];

        if (isMarker (type, moveMarker))
        {
            indexOfPathStart = p.data.size();
            indexOfPathStartThis = n - 1;
            auto x = elements[n++];
            auto y = elements[n++];
            p.startNewSubPath (x, y);
            lastWasLine = false;
            firstWasLine = (isMarker (elements[n], lineMarker));
        }
        else if (isMarker (type, lineMarker) || isMarker (type, closeSubPathMarker))
        {
            f32 startX = 0, startY = 0, joinX = 0, joinY = 0, endX, endY;

            if (isMarker (type, lineMarker))
            {
                endX = elements[n++];
                endY = elements[n++];

                if (n > 8)
                {
                    startX = elements[n - 8];
                    startY = elements[n - 7];
                    joinX  = elements[n - 5];
                    joinY  = elements[n - 4];
                }
            }
            else
            {
                endX = elements[indexOfPathStartThis + 1];
                endY = elements[indexOfPathStartThis + 2];

                if (n > 6)
                {
                    startX = elements[n - 6];
                    startY = elements[n - 5];
                    joinX  = elements[n - 3];
                    joinY  = elements[n - 2];
                }
            }

            if (lastWasLine)
            {
                auto len1 = PathHelpers::lengthOf (startX, startY, joinX, joinY);

                if (len1 > 0)
                {
                    auto propNeeded = jmin (0.5, cornerRadius / len1);

                    *(p.data.end() - 2) = (f32) (joinX - (joinX - startX) * propNeeded);
                    *(p.data.end() - 1) = (f32) (joinY - (joinY - startY) * propNeeded);
                }

                auto len2 = PathHelpers::lengthOf (endX, endY, joinX, joinY);

                if (len2 > 0)
                {
                    auto propNeeded = jmin (0.5, cornerRadius / len2);

                    p.quadraticTo (joinX, joinY,
                                   (f32) (joinX + (endX - joinX) * propNeeded),
                                   (f32) (joinY + (endY - joinY) * propNeeded));
                }

                p.lineTo (endX, endY);
            }
            else if (isMarker (type, lineMarker))
            {
                p.lineTo (endX, endY);
                lastWasLine = true;
            }

            if (isMarker (type, closeSubPathMarker))
            {
                if (firstWasLine)
                {
                    startX = elements[n - 3];
                    startY = elements[n - 2];
                    joinX = endX;
                    joinY = endY;
                    endX = elements[indexOfPathStartThis + 4];
                    endY = elements[indexOfPathStartThis + 5];

                    auto len1 = PathHelpers::lengthOf (startX, startY, joinX, joinY);

                    if (len1 > 0)
                    {
                        auto propNeeded = jmin (0.5, cornerRadius / len1);

                        *(p.data.end() - 2) = (f32) (joinX - (joinX - startX) * propNeeded);
                        *(p.data.end() - 1) = (f32) (joinY - (joinY - startY) * propNeeded);
                    }

                    auto len2 = PathHelpers::lengthOf (endX, endY, joinX, joinY);

                    if (len2 > 0)
                    {
                        auto propNeeded = jmin (0.5, cornerRadius / len2);

                        endX = (f32) (joinX + (endX - joinX) * propNeeded);
                        endY = (f32) (joinY + (endY - joinY) * propNeeded);

                        p.quadraticTo (joinX, joinY, endX, endY);

                        p.data.begin()[indexOfPathStart + 1] = endX;
                        p.data.begin()[indexOfPathStart + 2] = endY;
                    }
                }

                p.closeSubPath();
            }
        }
        else if (isMarker (type, quadMarker))
        {
            lastWasLine = false;
            auto x1 = elements[n++];
            auto y1 = elements[n++];
            auto x2 = elements[n++];
            auto y2 = elements[n++];
            p.quadraticTo (x1, y1, x2, y2);
        }
        else if (isMarker (type, cubicMarker))
        {
            lastWasLine = false;
            auto x1 = elements[n++];
            auto y1 = elements[n++];
            auto x2 = elements[n++];
            auto y2 = elements[n++];
            auto x3 = elements[n++];
            auto y3 = elements[n++];
            p.cubicTo (x1, y1, x2, y2, x3, y3);
        }
    }

    return p;
}

//==============================================================================
z0 Path::loadPathFromStream (InputStream& source)
{
    while (! source.isExhausted())
    {
        switch (source.readByte())
        {
        case 'm':
        {
            auto x = source.readFloat();
            auto y = source.readFloat();
            startNewSubPath (x, y);
            break;
        }

        case 'l':
        {
            auto x = source.readFloat();
            auto y = source.readFloat();
            lineTo (x, y);
            break;
        }

        case 'q':
        {
            auto x1 = source.readFloat();
            auto y1 = source.readFloat();
            auto x2 = source.readFloat();
            auto y2 = source.readFloat();
            quadraticTo (x1, y1, x2, y2);
            break;
        }

        case 'b':
        {
            auto x1 = source.readFloat();
            auto y1 = source.readFloat();
            auto x2 = source.readFloat();
            auto y2 = source.readFloat();
            auto x3 = source.readFloat();
            auto y3 = source.readFloat();
            cubicTo (x1, y1, x2, y2, x3, y3);
            break;
        }

        case 'c':
            closeSubPath();
            break;

        case 'n':
            setUsingNonZeroWinding (true);
            break;

        case 'z':
            setUsingNonZeroWinding (false);
            break;

        case 'e':
            return; // end of path marker

        default:
            jassertfalse; // illegal t8 in the stream
            break;
        }
    }
}

z0 Path::loadPathFromData (ukk const pathData, const size_t numberOfBytes)
{
    MemoryInputStream in (pathData, numberOfBytes, false);
    loadPathFromStream (in);
}

z0 Path::writePathToStream (OutputStream& dest) const
{
    dest.writeByte (isUsingNonZeroWinding() ? 'n' : 'z');

    for (auto* i = data.begin(); i != data.end();)
    {
        auto type = *i++;

        if (isMarker (type, moveMarker))
        {
            dest.writeByte ('m');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, lineMarker))
        {
            dest.writeByte ('l');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, quadMarker))
        {
            dest.writeByte ('q');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, cubicMarker))
        {
            dest.writeByte ('b');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, closeSubPathMarker))
        {
            dest.writeByte ('c');
        }
    }

    dest.writeByte ('e'); // marks the end-of-path
}

Txt Path::toString() const
{
    MemoryOutputStream s (2048);
    if (! isUsingNonZeroWinding())
        s << 'a';

    f32 lastMarker = 0.0f;

    for (i32 i = 0; i < data.size();)
    {
        auto type = data.begin()[i++];
        t8 markerChar = 0;
        i32 numCoords = 0;

        if (isMarker (type, moveMarker))
        {
            markerChar = 'm';
            numCoords = 2;
        }
        else if (isMarker (type, lineMarker))
        {
            markerChar = 'l';
            numCoords = 2;
        }
        else if (isMarker (type, quadMarker))
        {
            markerChar = 'q';
            numCoords = 4;
        }
        else if (isMarker (type, cubicMarker))
        {
            markerChar = 'c';
            numCoords = 6;
        }
        else
        {
            jassert (isMarker (type, closeSubPathMarker));
            markerChar = 'z';
        }

        if (! isMarker (type, lastMarker))
        {
            if (s.getDataSize() != 0)
                s << ' ';

            s << markerChar;
            lastMarker = type;
        }

        while (--numCoords >= 0 && i < data.size())
        {
            Txt coord (data.begin()[i++], 3);

            while (coord.endsWithChar ('0') && coord != "0")
                coord = coord.dropLastCharacters (1);

            if (coord.endsWithChar ('.'))
                coord = coord.dropLastCharacters (1);

            if (s.getDataSize() != 0)
                s << ' ';

            s << coord;
        }
    }

    return s.toUTF8();
}

z0 Path::restoreFromString (StringRef stringVersion)
{
    clear();
    setUsingNonZeroWinding (true);

    auto t = stringVersion.text;
    t32 marker = 'm';
    i32 numValues = 2;
    f32 values[6];

    for (;;)
    {
        auto token = PathHelpers::nextToken (t);
        auto firstChar = token[0];
        i32 startNum = 0;

        if (firstChar == 0)
            break;

        if (firstChar == 'm' || firstChar == 'l')
        {
            marker = firstChar;
            numValues = 2;
        }
        else if (firstChar == 'q')
        {
            marker = firstChar;
            numValues = 4;
        }
        else if (firstChar == 'c')
        {
            marker = firstChar;
            numValues = 6;
        }
        else if (firstChar == 'z')
        {
            marker = firstChar;
            numValues = 0;
        }
        else if (firstChar == 'a')
        {
            setUsingNonZeroWinding (false);
            continue;
        }
        else
        {
            ++startNum;
            values [0] = token.getFloatValue();
        }

        for (i32 i = startNum; i < numValues; ++i)
            values [i] = PathHelpers::nextToken (t).getFloatValue();

        switch (marker)
        {
            case 'm':   startNewSubPath (values[0], values[1]); break;
            case 'l':   lineTo (values[0], values[1]); break;
            case 'q':   quadraticTo (values[0], values[1], values[2], values[3]); break;
            case 'c':   cubicTo (values[0], values[1], values[2], values[3], values[4], values[5]); break;
            case 'z':   closeSubPath(); break;
            default:    jassertfalse; break; // illegal string format?
        }
    }
}

//==============================================================================
Path::Iterator::Iterator (const Path& p) noexcept
    : elementType (startNewSubPath), path (p), index (path.data.begin())
{
}

b8 Path::Iterator::next() noexcept
{
    if (index != path.data.end())
    {
        auto type = *index++;

        if (isMarker (type, moveMarker))
        {
            elementType = startNewSubPath;
            x1 = *index++;
            y1 = *index++;
        }
        else if (isMarker (type, lineMarker))
        {
            elementType = lineTo;
            x1 = *index++;
            y1 = *index++;
        }
        else if (isMarker (type, quadMarker))
        {
            elementType = quadraticTo;
            x1 = *index++;
            y1 = *index++;
            x2 = *index++;
            y2 = *index++;
        }
        else if (isMarker (type, cubicMarker))
        {
            elementType = cubicTo;
            x1 = *index++;
            y1 = *index++;
            x2 = *index++;
            y2 = *index++;
            x3 = *index++;
            y3 = *index++;
        }
        else if (isMarker (type, closeSubPathMarker))
        {
            elementType = closePath;
        }

        return true;
    }

    return false;
}

#undef DRX_CHECK_COORDS_ARE_VALID

} // namespace drx
