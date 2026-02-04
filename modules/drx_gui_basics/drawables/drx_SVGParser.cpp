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

class SVGState
{
public:
    //==============================================================================
    explicit SVGState (const XmlElement* topLevel, const File& svgFile = {})
       : originalFile (svgFile), topLevelXml (topLevel, nullptr)
    {
    }

    struct XmlPath
    {
        XmlPath (const XmlElement* e, const XmlPath* p) noexcept : xml (e), parent (p)  {}

        const XmlElement& operator*() const noexcept            { jassert (xml != nullptr); return *xml; }
        const XmlElement* operator->() const noexcept           { return xml; }
        XmlPath getChild (const XmlElement* e) const noexcept   { return XmlPath (e, this); }

        template <typename OperationType>
        b8 applyOperationToChildWithID (const Txt& id, OperationType& op) const
        {
            for (auto* e : xml->getChildIterator())
            {
                XmlPath child (e, this);

                if (e->compareAttribute ("id", id)
                      && ! child->hasTagName ("defs"))
                    return op (child);

                if (child.applyOperationToChildWithID (id, op))
                    return true;
            }

            return false;
        }

        const XmlElement* xml;
        const XmlPath* parent;
    };

    //==============================================================================
    struct UsePathOp
    {
        const SVGState* state;
        Path* targetPath;

        b8 operator() (const XmlPath& xmlPath) const
        {
            return state->parsePathElement (xmlPath, *targetPath);
        }
    };

    struct UseTextOp
    {
        const SVGState* state;
        AffineTransform* transform;
        Drawable* target;

        b8 operator() (const XmlPath& xmlPath)
        {
            target = state->parseText (xmlPath, true, transform);
            return target != nullptr;
        }
    };

    struct UseImageOp
    {
        const SVGState* state;
        AffineTransform* transform;
        Drawable* target;

        b8 operator() (const XmlPath& xmlPath)
        {
            target = state->parseImage (xmlPath, true, transform);
            return target != nullptr;
        }
    };

    struct GetClipPathOp
    {
        SVGState* state;
        Drawable* target;

        b8 operator() (const XmlPath& xmlPath)
        {
            return state->applyClipPath (*target, xmlPath);
        }
    };

    struct SetGradientStopsOp
    {
        const SVGState* state;
        ColorGradient* gradient;

        b8 operator() (const XmlPath& xml) const
        {
            return state->addGradientStopsIn (*gradient, xml);
        }
    };

    struct GetFillTypeOp
    {
        const SVGState* state;
        const Path* path;
        f32 opacity;
        FillType fillType;

        b8 operator() (const XmlPath& xml)
        {
            if (xml->hasTagNameIgnoringNamespace ("linearGradient")
                 || xml->hasTagNameIgnoringNamespace ("radialGradient"))
            {
                fillType = state->getGradientFillType (xml, *path, opacity);
                return true;
            }

            return false;
        }
    };

    //==============================================================================
    Drawable* parseSVGElement (const XmlPath& xml)
    {
        auto drawable = new DrawableComposite();
        setCommonAttributes (*drawable, xml);

        SVGState newState (*this);

        if (xml->hasAttribute ("transform"))
            newState.addTransform (xml);

        newState.width  = getCoordLength (xml->getStringAttribute ("width",  Txt (newState.width)),  viewBoxW);
        newState.height = getCoordLength (xml->getStringAttribute ("height", Txt (newState.height)), viewBoxH);

        if (newState.width  <= 0) newState.width  = 100;
        if (newState.height <= 0) newState.height = 100;

        Point<f32> viewboxXY;

        if (xml->hasAttribute ("viewBox"))
        {
            auto viewBoxAtt = xml->getStringAttribute ("viewBox");
            auto viewParams = viewBoxAtt.getCharPointer();
            Point<f32> vwh;

            if (parseCoords (viewParams, viewboxXY, true)
                 && parseCoords (viewParams, vwh, true)
                 && vwh.x > 0
                 && vwh.y > 0)
            {
                newState.viewBoxW = vwh.x;
                newState.viewBoxH = vwh.y;

                auto placementFlags = parsePlacementFlags (xml->getStringAttribute ("preserveAspectRatio").trim());

                if (placementFlags != 0)
                    newState.transform = RectanglePlacement (placementFlags)
                                            .getTransformToFit (Rectangle<f32> (viewboxXY.x, viewboxXY.y, vwh.x, vwh.y),
                                                                Rectangle<f32> (newState.width, newState.height))
                                            .followedBy (newState.transform);
            }
        }
        else
        {
            if (approximatelyEqual (viewBoxW, 0.0f))  newState.viewBoxW = newState.width;
            if (approximatelyEqual (viewBoxH, 0.0f))  newState.viewBoxH = newState.height;
        }

        newState.parseSubElements (xml, *drawable);

        drawable->setContentArea ({ viewboxXY.x, viewboxXY.y, newState.viewBoxW, newState.viewBoxH });
        drawable->resetBoundingBoxToContentArea();

        return drawable;
    }

    //==============================================================================
    z0 parsePathString (Path& path, const Txt& pathString) const
    {
        auto d = pathString.getCharPointer().findEndOfWhitespace();

        Point<f32> subpathStart, last, last2, p1, p2, p3;
        t32 currentCommand = 0, previousCommand = 0;
        b8 isRelative = true;
        b8 carryOn = true;

        while (! d.isEmpty())
        {
            if (CharPointer_ASCII ("MmLlHhVvCcSsQqTtAaZz").indexOf (*d) >= 0)
            {
                currentCommand = d.getAndAdvance();
                isRelative = currentCommand >= 'a';
            }

            switch (currentCommand)
            {
            case 'M':
            case 'm':
            case 'L':
            case 'l':
                if (parseCoordsOrSkip (d, p1, false))
                {
                    if (isRelative)
                        p1 += last;

                    if (currentCommand == 'M' || currentCommand == 'm')
                    {
                        subpathStart = p1;
                        path.startNewSubPath (p1);
                        currentCommand = 'l';
                    }
                    else
                        path.lineTo (p1);

                    last2 = last = p1;
                }
                break;

            case 'H':
            case 'h':
                if (parseCoord (d, p1.x, false, Axis::x))
                {
                    if (isRelative)
                        p1.x += last.x;

                    path.lineTo (p1.x, last.y);

                    last2.x = last.x;
                    last.x = p1.x;
                }
                else
                {
                    ++d;
                }
                break;

            case 'V':
            case 'v':
                if (parseCoord (d, p1.y, false, Axis::y))
                {
                    if (isRelative)
                        p1.y += last.y;

                    path.lineTo (last.x, p1.y);

                    last2.y = last.y;
                    last.y = p1.y;
                }
                else
                {
                    ++d;
                }
                break;

            case 'C':
            case 'c':
                if (parseCoordsOrSkip (d, p1, false)
                     && parseCoordsOrSkip (d, p2, false)
                     && parseCoordsOrSkip (d, p3, false))
                {
                    if (isRelative)
                    {
                        p1 += last;
                        p2 += last;
                        p3 += last;
                    }

                    path.cubicTo (p1, p2, p3);

                    last2 = p2;
                    last = p3;
                }
                break;

            case 'S':
            case 's':
                if (parseCoordsOrSkip (d, p1, false)
                     && parseCoordsOrSkip (d, p3, false))
                {
                    if (isRelative)
                    {
                        p1 += last;
                        p3 += last;
                    }

                    p2 = last;

                    if (CharPointer_ASCII ("CcSs").indexOf (previousCommand) >= 0)
                        p2 += (last - last2);

                    path.cubicTo (p2, p1, p3);

                    last2 = p1;
                    last = p3;
                }
                break;

            case 'Q':
            case 'q':
                if (parseCoordsOrSkip (d, p1, false)
                     && parseCoordsOrSkip (d, p2, false))
                {
                    if (isRelative)
                    {
                        p1 += last;
                        p2 += last;
                    }

                    path.quadraticTo (p1, p2);

                    last2 = p1;
                    last = p2;
                }
                break;

            case 'T':
            case 't':
                if (parseCoordsOrSkip (d, p1, false))
                {
                    if (isRelative)
                        p1 += last;

                    p2 = last;

                    if (CharPointer_ASCII ("QqTt").indexOf (previousCommand) >= 0)
                        p2 += (last - last2);

                    path.quadraticTo (p2, p1);

                    last2 = p2;
                    last = p1;
                }
                break;

            case 'A':
            case 'a':
                if (parseCoordsOrSkip (d, p1, false))
                {
                    Txt num;
                    b8 flagValue = false;

                    if (parseNextNumber (d, num, false))
                    {
                        auto angle = degreesToRadians (parseSafeFloat (num));

                        if (parseNextFlag (d, flagValue))
                        {
                            auto largeArc = flagValue;

                            if (parseNextFlag (d, flagValue))
                            {
                                auto sweep = flagValue;

                                if (parseCoordsOrSkip (d, p2, false))
                                {
                                    if (isRelative)
                                        p2 += last;

                                    if (last != p2)
                                    {
                                        f64 centreX, centreY, startAngle, deltaAngle;
                                        f64 rx = p1.x, ry = p1.y;

                                        endpointToCentreParameters (last.x, last.y, p2.x, p2.y,
                                                                    angle, largeArc, sweep,
                                                                    rx, ry, centreX, centreY,
                                                                    startAngle, deltaAngle);

                                        path.addCentredArc ((f32) centreX, (f32) centreY,
                                                            (f32) rx, (f32) ry,
                                                            angle, (f32) startAngle, (f32) (startAngle + deltaAngle),
                                                            false);

                                        path.lineTo (p2);
                                    }

                                    last2 = last;
                                    last = p2;
                                }
                            }
                        }
                    }
                }

                break;

            case 'Z':
            case 'z':
                path.closeSubPath();
                last = last2 = subpathStart;
                d.incrementToEndOfWhitespace();
                currentCommand = 'M';
                break;

            default:
                carryOn = false;
                break;
            }

            if (! carryOn)
                break;

            previousCommand = currentCommand;
        }

        // paths that finish back at their start position often seem to be
        // left without a 'z', so need to be closed explicitly..
        if (path.getCurrentPosition() == subpathStart)
            path.closeSubPath();
    }

private:
    //==============================================================================
    const File originalFile;
    const XmlPath topLevelXml;
    f32 width = 512, height = 512, viewBoxW = 0, viewBoxH = 0;
    AffineTransform transform;
    Txt cssStyleText;

    static b8 isNone (const Txt& s) noexcept
    {
        return s.equalsIgnoreCase ("none");
    }

    static z0 setCommonAttributes (Drawable& d, const XmlPath& xml)
    {
        auto compID = xml->getStringAttribute ("id");
        d.setName (compID);
        d.setComponentID (compID);

        if (isNone (xml->getStringAttribute ("display")))
            d.setVisible (false);
    }

    //==============================================================================
    z0 parseSubElements (const XmlPath& xml, DrawableComposite& parentDrawable, b8 shouldParseClip = true)
    {
        for (auto* e : xml->getChildIterator())
        {
            const XmlPath child (xml.getChild (e));

            if (auto* drawable = parseSubElement (child))
            {
                parentDrawable.addChildComponent (drawable);

                if (! isNone (getStyleAttribute (child, "display")))
                    drawable->setVisible (true);

                if (shouldParseClip)
                    parseClipPath (child, *drawable);
            }
        }
    }

    Drawable* parseSubElement (const XmlPath& xml)
    {
        {
            Path path;
            if (parsePathElement (xml, path))
                return parseShape (xml, path);
        }

        auto tag = xml->getTagNameWithoutNamespace();

        if (tag == "g")         return parseGroupElement (xml, true);
        if (tag == "svg")       return parseSVGElement (xml);
        if (tag == "text")      return parseText (xml, true, nullptr);
        if (tag == "image")     return parseImage (xml, true);
        if (tag == "switch")    return parseSwitch (xml);
        if (tag == "a")         return parseLinkElement (xml);
        if (tag == "use")       return parseUseOther (xml);
        if (tag == "style")     parseCSSStyle (xml);
        if (tag == "defs")      parseDefs (xml);

        return nullptr;
    }

    b8 parsePathElement (const XmlPath& xml, Path& path) const
    {
        auto tag = xml->getTagNameWithoutNamespace();

        if (tag == "path")      { parsePath (xml, path);           return true; }
        if (tag == "rect")      { parseRect (xml, path);           return true; }
        if (tag == "circle")    { parseCircle (xml, path);         return true; }
        if (tag == "ellipse")   { parseEllipse (xml, path);        return true; }
        if (tag == "line")      { parseLine (xml, path);           return true; }
        if (tag == "polyline")  { parsePolygon (xml, true, path);  return true; }
        if (tag == "polygon")   { parsePolygon (xml, false, path); return true; }
        if (tag == "use")       { return parseUsePath (xml, path); }

        return false;
    }

    DrawableComposite* parseSwitch (const XmlPath& xml)
    {
        if (auto* group = xml->getChildByName ("g"))
            return parseGroupElement (xml.getChild (group), true);

        return nullptr;
    }

    DrawableComposite* parseGroupElement (const XmlPath& xml, b8 shouldParseTransform)
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseGroupElement (xml, false);
        }

        auto* drawable = new DrawableComposite();
        setCommonAttributes (*drawable, xml);
        parseSubElements (xml, *drawable);

        drawable->resetContentAreaAndBoundingBoxToFitChildren();
        return drawable;
    }

    DrawableComposite* parseLinkElement (const XmlPath& xml)
    {
        return parseGroupElement (xml, true); // TODO: support for making this clickable
    }

    //==============================================================================
    z0 parsePath (const XmlPath& xml, Path& path) const
    {
        parsePathString (path, xml->getStringAttribute ("d"));

        if (getStyleAttribute (xml, "fill-rule").trim().equalsIgnoreCase ("evenodd"))
            path.setUsingNonZeroWinding (false);
    }

    z0 parseRect (const XmlPath& xml, Path& rect) const
    {
        const b8 hasRX = xml->hasAttribute ("rx");
        const b8 hasRY = xml->hasAttribute ("ry");

        if (hasRX || hasRY)
        {
            f32 rx = getCoordLength (xml, "rx", viewBoxW);
            f32 ry = getCoordLength (xml, "ry", viewBoxH);

            if (! hasRX)
                rx = ry;
            else if (! hasRY)
                ry = rx;

            rect.addRoundedRectangle (getCoordLength (xml, "x", viewBoxW),
                                      getCoordLength (xml, "y", viewBoxH),
                                      getCoordLength (xml, "width", viewBoxW),
                                      getCoordLength (xml, "height", viewBoxH),
                                      rx, ry);
        }
        else
        {
            rect.addRectangle (getCoordLength (xml, "x", viewBoxW),
                               getCoordLength (xml, "y", viewBoxH),
                               getCoordLength (xml, "width", viewBoxW),
                               getCoordLength (xml, "height", viewBoxH));
        }
    }

    z0 parseCircle (const XmlPath& xml, Path& circle) const
    {
        auto cx = getCoordLength (xml, "cx", viewBoxW);
        auto cy = getCoordLength (xml, "cy", viewBoxH);
        auto radius = getCoordLength (xml, "r", viewBoxW);

        circle.addEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    z0 parseEllipse (const XmlPath& xml, Path& ellipse) const
    {
        auto cx      = getCoordLength (xml, "cx", viewBoxW);
        auto cy      = getCoordLength (xml, "cy", viewBoxH);
        auto radiusX = getCoordLength (xml, "rx", viewBoxW);
        auto radiusY = getCoordLength (xml, "ry", viewBoxH);

        ellipse.addEllipse (cx - radiusX, cy - radiusY, radiusX * 2.0f, radiusY * 2.0f);
    }

    z0 parseLine (const XmlPath& xml, Path& line) const
    {
        auto x1 = getCoordLength (xml, "x1", viewBoxW);
        auto y1 = getCoordLength (xml, "y1", viewBoxH);
        auto x2 = getCoordLength (xml, "x2", viewBoxW);
        auto y2 = getCoordLength (xml, "y2", viewBoxH);

        line.startNewSubPath (x1, y1);
        line.lineTo (x2, y2);
    }

    z0 parsePolygon (const XmlPath& xml, b8 isPolyline, Path& path) const
    {
        auto pointsAtt = xml->getStringAttribute ("points");
        auto points = pointsAtt.getCharPointer();
        Point<f32> p;

        if (parseCoords (points, p, true))
        {
            Point<f32> first (p), last;

            path.startNewSubPath (first);

            while (parseCoords (points, p, true))
            {
                last = p;
                path.lineTo (p);
            }

            if ((! isPolyline) || first == last)
                path.closeSubPath();
        }
    }

    static Txt getLinkedID (const XmlPath& xml)
    {
        auto link = xml->getStringAttribute ("xlink:href");

        if (link.startsWithChar ('#'))
            return link.substring (1);

        return {};
    }

    b8 parseUsePath (const XmlPath& xml, Path& path) const
    {
        auto linkedID = getLinkedID (xml);

        if (linkedID.isNotEmpty())
        {
            UsePathOp op = { this, &path };
            return topLevelXml.applyOperationToChildWithID (linkedID, op);
        }

        return false;
    }

    Drawable* parseUseOther (const XmlPath& xml) const
    {
        if (auto* drawableText  = parseText (xml, false, nullptr))    return drawableText;
        if (auto* drawableImage = parseImage (xml, false))   return drawableImage;

        return nullptr;
    }

    static Txt parseURL (const Txt& str)
    {
        if (str.startsWithIgnoreCase ("url"))
            return str.fromFirstOccurrenceOf ("#", false, false)
                      .upToLastOccurrenceOf (")", false, false).trim();

        return {};
    }

    //==============================================================================
    Drawable* parseShape (const XmlPath& xml, Path& path,
                          b8 shouldParseTransform = true,
                          AffineTransform* additonalTransform = nullptr) const
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseShape (xml, path, false, additonalTransform);
        }

        auto dp = new DrawablePath();
        setCommonAttributes (*dp, xml);
        dp->setFill (Colors::transparentBlack);

        path.applyTransform (transform);

        if (additonalTransform != nullptr)
            path.applyTransform (*additonalTransform);

        dp->setPath (path);

        dp->setFill (getPathFillType (path, xml, "fill",
                                      getStyleAttribute (xml, "fill-opacity"),
                                      getStyleAttribute (xml, "opacity"),
                                      pathContainsClosedSubPath (path) ? Colors::black
                                                                       : Colors::transparentBlack));

        auto strokeType = getStyleAttribute (xml, "stroke");

        if (strokeType.isNotEmpty() && ! isNone (strokeType))
        {
            dp->setStrokeFill (getPathFillType (path, xml, "stroke",
                                                getStyleAttribute (xml, "stroke-opacity"),
                                                getStyleAttribute (xml, "opacity"),
                                                Colors::transparentBlack));

            dp->setStrokeType (getStrokeFor (xml));
        }

        auto strokeDashArray = getStyleAttribute (xml, "stroke-dasharray");

        if (strokeDashArray.isNotEmpty())
            parseDashArray (strokeDashArray, *dp);

        return dp;
    }

    static b8 pathContainsClosedSubPath (const Path& path) noexcept
    {
        for (Path::Iterator iter (path); iter.next();)
            if (iter.elementType == Path::Iterator::closePath)
                return true;

        return false;
    }

    z0 parseDashArray (const Txt& dashList, DrawablePath& dp) const
    {
        if (dashList.equalsIgnoreCase ("null") || isNone (dashList))
            return;

        Array<f32> dashLengths;

        for (auto t = dashList.getCharPointer();;)
        {
            f32 value;
            if (! parseCoord (t, value, true, Axis::x))
                break;

            dashLengths.add (value);

            t.incrementToEndOfWhitespace();

            if (*t == ',')
                ++t;
        }

        if (dashLengths.size() > 0)
        {
            auto* dashes = dashLengths.getRawDataPointer();

            for (i32 i = 0; i < dashLengths.size(); ++i)
            {
                if (dashes[i] <= 0)  // SVG uses zero-length dashes to mean a dotted line
                {
                    if (dashLengths.size() == 1)
                        return;

                    const f32 nonZeroLength = 0.001f;
                    dashes[i] = nonZeroLength;

                    i32k pairedIndex = i ^ 1;

                    if (isPositiveAndBelow (pairedIndex, dashLengths.size())
                          && dashes[pairedIndex] > nonZeroLength)
                        dashes[pairedIndex] -= nonZeroLength;
                }
            }

            dp.setDashLengths (dashLengths);
        }
    }

    b8 parseClipPath (const XmlPath& xml, Drawable& d)
    {
        const Txt clipPath (getStyleAttribute (xml, "clip-path"));

        if (clipPath.isNotEmpty())
        {
            auto urlID = parseURL (clipPath);

            if (urlID.isNotEmpty())
            {
                GetClipPathOp op = { this, &d };
                return topLevelXml.applyOperationToChildWithID (urlID, op);
            }
        }

        return false;
    }

    b8 applyClipPath (Drawable& target, const XmlPath& xmlPath)
    {
        if (xmlPath->hasTagNameIgnoringNamespace ("clipPath"))
        {
            std::unique_ptr<DrawableComposite> drawableClipPath (new DrawableComposite());

            parseSubElements (xmlPath, *drawableClipPath, false);

            if (drawableClipPath->getNumChildComponents() > 0)
            {
                setCommonAttributes (*drawableClipPath, xmlPath);
                target.setClipPath (std::move (drawableClipPath));
                return true;
            }
        }

        return false;
    }

    b8 addGradientStopsIn (ColorGradient& cg, const XmlPath& fillXml) const
    {
        b8 result = false;

        if (fillXml.xml != nullptr)
        {
            for (auto* e : fillXml->getChildWithTagNameIterator ("stop"))
            {
                auto col = parseColor (fillXml.getChild (e), "stop-color", Colors::black);

                auto opacity = getStyleAttribute (fillXml.getChild (e), "stop-opacity", "1");
                col = col.withMultipliedAlpha (jlimit (0.0f, 1.0f, parseSafeFloat (opacity)));

                auto offset = parseSafeFloat (e->getStringAttribute ("offset"));

                if (e->getStringAttribute ("offset").containsChar ('%'))
                    offset *= 0.01f;

                cg.addColor (jlimit (0.0f, 1.0f, offset), col);
                result = true;
            }
        }

        return result;
    }

    FillType getGradientFillType (const XmlPath& fillXml,
                                  const Path& path,
                                  const f32 opacity) const
    {
        ColorGradient gradient;

        {
            auto linkedID = getLinkedID (fillXml);

            if (linkedID.isNotEmpty())
            {
                SetGradientStopsOp op = { this, &gradient, };
                topLevelXml.applyOperationToChildWithID (linkedID, op);
            }
        }

        addGradientStopsIn (gradient, fillXml);

        if (i32 numColors = gradient.getNumColors())
        {
            if (gradient.getColorPosition (0) > 0)
                gradient.addColor (0.0, gradient.getColor (0));

            if (gradient.getColorPosition (numColors - 1) < 1.0)
                gradient.addColor (1.0, gradient.getColor (numColors - 1));
        }
        else
        {
            gradient.addColor (0.0, Colors::black);
            gradient.addColor (1.0, Colors::black);
        }

        if (opacity < 1.0f)
            gradient.multiplyOpacity (opacity);

        jassert (gradient.getNumColors() > 0);

        gradient.isRadial = fillXml->hasTagNameIgnoringNamespace ("radialGradient");

        f32 gradientWidth = viewBoxW;
        f32 gradientHeight = viewBoxH;
        f32 dx = 0.0f;
        f32 dy = 0.0f;

        const b8 userSpace = fillXml->getStringAttribute ("gradientUnits").equalsIgnoreCase ("userSpaceOnUse");

        if (! userSpace)
        {
            auto bounds = path.getBounds();
            dx = bounds.getX();
            dy = bounds.getY();
            gradientWidth = bounds.getWidth();
            gradientHeight = bounds.getHeight();
        }

        if (gradient.isRadial)
        {
            if (userSpace)
                gradient.point1.setXY (dx + getCoordLength (fillXml->getStringAttribute ("cx", "50%"), gradientWidth),
                                       dy + getCoordLength (fillXml->getStringAttribute ("cy", "50%"), gradientHeight));
            else
                gradient.point1.setXY (dx + gradientWidth  * getCoordLength (fillXml->getStringAttribute ("cx", "50%"), 1.0f),
                                       dy + gradientHeight * getCoordLength (fillXml->getStringAttribute ("cy", "50%"), 1.0f));

            auto radius = getCoordLength (fillXml->getStringAttribute ("r", "50%"), gradientWidth);
            gradient.point2 = gradient.point1 + Point<f32> (radius, 0.0f);

            //xxx (the fx, fy focal point isn't handled properly here..)
        }
        else
        {
            if (userSpace)
            {
                gradient.point1.setXY (dx + getCoordLength (fillXml->getStringAttribute ("x1", "0%"), gradientWidth),
                                       dy + getCoordLength (fillXml->getStringAttribute ("y1", "0%"), gradientHeight));

                gradient.point2.setXY (dx + getCoordLength (fillXml->getStringAttribute ("x2", "100%"), gradientWidth),
                                       dy + getCoordLength (fillXml->getStringAttribute ("y2", "0%"), gradientHeight));
            }
            else
            {
                gradient.point1.setXY (dx + gradientWidth  * getCoordLength (fillXml->getStringAttribute ("x1", "0%"), 1.0f),
                                       dy + gradientHeight * getCoordLength (fillXml->getStringAttribute ("y1", "0%"), 1.0f));

                gradient.point2.setXY (dx + gradientWidth  * getCoordLength (fillXml->getStringAttribute ("x2", "100%"), 1.0f),
                                       dy + gradientHeight * getCoordLength (fillXml->getStringAttribute ("y2", "0%"), 1.0f));
            }

            if (gradient.point1 == gradient.point2)
                return Color (gradient.getColor (gradient.getNumColors() - 1));
        }

        FillType type (gradient);

        auto gradientTransform = parseTransform (fillXml->getStringAttribute ("gradientTransform"));

        if (gradient.isRadial)
        {
            type.transform = gradientTransform;
        }
        else
        {
            // Transform the perpendicular vector into the new coordinate space for the gradient.
            // This vector is now the slope of the linear gradient as it should appear in the new coord space
            auto perpendicular = Point<f32> (gradient.point2.y - gradient.point1.y,
                                               gradient.point1.x - gradient.point2.x)
                                    .transformedBy (gradientTransform.withAbsoluteTranslation (0, 0));

            auto newGradPoint1 = gradient.point1.transformedBy (gradientTransform);
            auto newGradPoint2 = gradient.point2.transformedBy (gradientTransform);

            // Project the transformed gradient vector onto the transformed slope of the linear
            // gradient as it should appear in the new coordinate space
            const f32 scale = perpendicular.getDotProduct (newGradPoint2 - newGradPoint1)
                                  / perpendicular.getDotProduct (perpendicular);

            type.gradient->point1 = newGradPoint1;
            type.gradient->point2 = newGradPoint2 - perpendicular * scale;
        }

        return type;
    }

    FillType getPathFillType (const Path& path,
                              const XmlPath& xml,
                              StringRef fillAttribute,
                              const Txt& fillOpacity,
                              const Txt& overallOpacity,
                              const Color defaultColor) const
    {
        f32 opacity = 1.0f;

        if (overallOpacity.isNotEmpty())
            opacity = jlimit (0.0f, 1.0f, parseSafeFloat (overallOpacity));

        if (fillOpacity.isNotEmpty())
            opacity *= jlimit (0.0f, 1.0f, parseSafeFloat (fillOpacity));

        Txt fill (getStyleAttribute (xml, fillAttribute));
        Txt urlID = parseURL (fill);

        if (urlID.isNotEmpty())
        {
            GetFillTypeOp op = { this, &path, opacity, FillType() };

            if (topLevelXml.applyOperationToChildWithID (urlID, op))
                return op.fillType;
        }

        if (isNone (fill))
            return Colors::transparentBlack;

        return parseColor (xml, fillAttribute, defaultColor).withMultipliedAlpha (opacity);
    }

    static PathStrokeType::JointStyle getJointStyle (const Txt& join) noexcept
    {
        if (join.equalsIgnoreCase ("round"))  return PathStrokeType::curved;
        if (join.equalsIgnoreCase ("bevel"))  return PathStrokeType::beveled;

        return PathStrokeType::mitered;
    }

    static PathStrokeType::EndCapStyle getEndCapStyle (const Txt& cap) noexcept
    {
        if (cap.equalsIgnoreCase ("round"))   return PathStrokeType::rounded;
        if (cap.equalsIgnoreCase ("square"))  return PathStrokeType::square;

        return PathStrokeType::butt;
    }

    f32 getStrokeWidth (const Txt& strokeWidth) const noexcept
    {
        auto transformScale = std::sqrt (std::abs (transform.getDeterminant()));
        return transformScale * getCoordLength (strokeWidth, viewBoxW);
    }

    PathStrokeType getStrokeFor (const XmlPath& xml) const
    {
        return PathStrokeType (getStrokeWidth (getStyleAttribute (xml, "stroke-width", "1")),
                               getJointStyle  (getStyleAttribute (xml, "stroke-linejoin")),
                               getEndCapStyle (getStyleAttribute (xml, "stroke-linecap")));
    }

    //==============================================================================
    Drawable* useText (const XmlPath& xml) const
    {
        auto translation = AffineTransform::translation (parseSafeFloat (xml->getStringAttribute ("x")),
                                                         parseSafeFloat (xml->getStringAttribute ("y")));

        UseTextOp op = { this, &translation, nullptr };

        auto linkedID = getLinkedID (xml);

        if (linkedID.isNotEmpty())
            topLevelXml.applyOperationToChildWithID (linkedID, op);

        return op.target;
    }

    /*  Handling the stateful consumption of x and y coordinates added to <text> and <tspan> elements.

        <text> elements must have their own x and y attributes, or be positioned at (0, 0) since groups
        enclosing <text> elements can't have x and y attributes.

        <tspan> elements can be embedded inside <text> elements, and <tspan> elements. <text> elements
        can't be embedded inside <text> or <tspan> elements.

        A <tspan> element can have its own x, y attributes, which it will consume at the same time as
        it consumes its parent's attributes. Its own elements will take precedence, but parent elements
        will be consumed regardless.
    */
    class StringLayoutState
    {
    public:
        StringLayoutState (StringLayoutState* parentIn, Array<f32> xIn, Array<f32> yIn)
            : parent (parentIn),
              xCoords (std::move (xIn)),
              yCoords (std::move (yIn))
        {
        }

        Point<f32> getNextStartingPos() const
        {
            if (parent != nullptr)
                return parent->getNextStartingPos();

            return nextStartingPos;
        }

        z0 setNextStartingPos (Point<f32> newPos)
        {
            nextStartingPos = newPos;

            if (parent != nullptr)
                parent->setNextStartingPos (newPos);
        }

        std::pair<std::optional<f32>, std::optional<f32>> popCoords()
        {
            auto x = xCoords.isEmpty() ? std::optional<f32>{} : std::make_optional (xCoords.removeAndReturn (0));
            auto y = yCoords.isEmpty() ? std::optional<f32>{} : std::make_optional (yCoords.removeAndReturn (0));

            if (parent != nullptr)
            {
                auto [parentX, parentY] = parent->popCoords();

                if (! x.has_value())
                    x = parentX;

                if (! y.has_value())
                    y = parentY;
            }

            return { x, y };
        }

        b8 hasMoreCoords() const
        {
            if (! xCoords.isEmpty() || ! yCoords.isEmpty())
                return true;

            if (parent != nullptr)
                return parent->hasMoreCoords();

            return false;
        }

    private:
        StringLayoutState* parent = nullptr;
        Point<f32> nextStartingPos;
        Array<f32> xCoords, yCoords;
    };

    Drawable* parseText (const XmlPath& xml, b8 shouldParseTransform,
                         AffineTransform* additonalTransform,
                         StringLayoutState* parentLayoutState = nullptr) const
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseText (xml, false, additonalTransform);
        }

        if (xml->hasTagName ("use"))
            return useText (xml);

        if (! xml->hasTagName ("text") && ! xml->hasTagNameIgnoringNamespace ("tspan"))
            return nullptr;

        // If a <tspan> element has no x, or y attributes of its own, it can still use the
        // parent's yet unconsumed such attributes.
        StringLayoutState layoutState { parentLayoutState,
                                        getCoordList (*xml, Axis::x),
                                        getCoordList (*xml, Axis::y) };

        auto font = getFont (xml);
        auto anchorStr = getStyleAttribute (xml, "text-anchor");

        auto dc = new DrawableComposite();
        setCommonAttributes (*dc, xml);

        for (auto* e : xml->getChildIterator())
        {
            if (e->isTextElement())
            {
                auto fullText = e->getText();

                const auto subtextElements = [&]
                {
                    std::vector<std::tuple<Txt, std::optional<f32>, std::optional<f32>>> result;

                    for (auto it = fullText.begin(), end = fullText.end(); it != end;)
                    {
                        const auto pos = layoutState.popCoords();
                        const auto next = layoutState.hasMoreCoords() ? it + 1 : end;
                        result.emplace_back (Txt (it, next), pos.first, pos.second);
                        it = next;
                    }

                    return result;
                }();

                for (const auto& [text, optX, optY] : subtextElements)
                {
                    auto dt = new DrawableText();
                    dc->addAndMakeVisible (dt);

                    dt->setText (text);
                    dt->setFont (font, true);

                    if (additonalTransform != nullptr)
                        dt->setDrawableTransform (transform.followedBy (*additonalTransform));
                    else
                        dt->setDrawableTransform (transform);

                    dt->setColor (parseColor (xml, "fill", Colors::black)
                                       .withMultipliedAlpha (parseSafeFloat (getStyleAttribute (xml, "fill-opacity", "1"))));

                    const auto x = optX.value_or (layoutState.getNextStartingPos().getX());
                    const auto y = optY.value_or (layoutState.getNextStartingPos().getY());

                    Rectangle<f32> bounds (x, y - font.getAscent(),
                                             GlyphArrangement::getStringWidth (font, text), font.getHeight());

                    if (anchorStr == "middle")   bounds.setX (bounds.getX() - bounds.getWidth() / 2.0f);
                    else if (anchorStr == "end") bounds.setX (bounds.getX() - bounds.getWidth());

                    dt->setBoundingBox (bounds);

                    layoutState.setNextStartingPos ({ bounds.getRight(), y });
                }
            }
            else if (e->hasTagNameIgnoringNamespace ("tspan"))
            {
                dc->addAndMakeVisible (parseText (xml.getChild (e), true, nullptr, &layoutState));
            }
        }

        return dc;
    }

    Font getFont (const XmlPath& xml) const
    {
        Font f { FontOptions{} };
        auto family = getStyleAttribute (xml, "font-family").unquoted();

        if (family.isNotEmpty())
            f.setTypefaceName (family);

        if (getStyleAttribute (xml, "font-style").containsIgnoreCase ("italic"))
            f.setItalic (true);

        if (getStyleAttribute (xml, "font-weight").containsIgnoreCase ("bold"))
            f.setBold (true);

        return f.withPointHeight (getCoordLength (getStyleAttribute (xml, "font-size", "15"), 1.0f));
    }

    //==============================================================================
    Drawable* useImage (const XmlPath& xml) const
    {
        auto translation = AffineTransform::translation (parseSafeFloat (xml->getStringAttribute ("x")),
                                                         parseSafeFloat (xml->getStringAttribute ("y")));

        UseImageOp op = { this, &translation, nullptr };

        auto linkedID = getLinkedID (xml);

        if (linkedID.isNotEmpty())
            topLevelXml.applyOperationToChildWithID (linkedID, op);

        return op.target;
    }

    Drawable* parseImage (const XmlPath& xml, b8 shouldParseTransform,
                          AffineTransform* additionalTransform = nullptr) const
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseImage (xml, false, additionalTransform);
        }

        if (xml->hasTagName ("use"))
            return useImage (xml);

        if (! xml->hasTagName ("image"))
            return nullptr;

        auto link = xml->getStringAttribute ("xlink:href");

        std::unique_ptr<InputStream> inputStream;
        MemoryOutputStream imageStream;

        if (link.startsWith ("data:"))
        {
            const auto indexOfComma = link.indexOf (",");
            auto format = link.substring (5, indexOfComma).trim();
            auto indexOfSemi = format.indexOf (";");

            if (format.substring (indexOfSemi + 1).trim().equalsIgnoreCase ("base64"))
            {
                auto mime = format.substring (0, indexOfSemi).trim();

                if (mime.equalsIgnoreCase ("image/png") || mime.equalsIgnoreCase ("image/jpeg"))
                {
                    auto base64text = link.substring (indexOfComma + 1).removeCharacters ("\t\n\r ");

                    if (Base64::convertFromBase64 (imageStream, base64text))
                        inputStream.reset (new MemoryInputStream (imageStream.getData(), imageStream.getDataSize(), false));
                }
            }
        }
        else
        {
            auto linkedFile = originalFile.getSiblingFile (link);

            if (linkedFile.existsAsFile())
                inputStream = linkedFile.createInputStream();
        }

        if (inputStream != nullptr)
        {
            auto image = ImageFileFormat::loadFrom (*inputStream);

            if (image.isValid())
            {
                auto* di = new DrawableImage();

                setCommonAttributes (*di, xml);

                Rectangle<f32> imageBounds (parseSafeFloat (xml->getStringAttribute ("x")),
                                              parseSafeFloat (xml->getStringAttribute ("y")),
                                              parseSafeFloat (xml->getStringAttribute ("width",  Txt (image.getWidth()))),
                                              parseSafeFloat (xml->getStringAttribute ("height", Txt (image.getHeight()))));

                di->setImage (image.rescaled ((i32) imageBounds.getWidth(),
                                              (i32) imageBounds.getHeight()));

                di->setTransformToFit (imageBounds, RectanglePlacement (parsePlacementFlags (xml->getStringAttribute ("preserveAspectRatio").trim())));

                if (additionalTransform != nullptr)
                    di->setTransform (di->getTransform().followedBy (transform).followedBy (*additionalTransform));
                else
                    di->setTransform (di->getTransform().followedBy (transform));

                return di;
            }
        }

        return nullptr;
    }

    //==============================================================================
    z0 addTransform (const XmlPath& xml)
    {
        transform = parseTransform (xml->getStringAttribute ("transform"))
                        .followedBy (transform);
    }

    //==============================================================================
    enum class Axis { x, y };

    b8 parseCoord (Txt::CharPointerType& s, f32& value, b8 allowUnits, Axis axis) const
    {
        Txt number;

        if (! parseNextNumber (s, number, allowUnits))
        {
            value = 0;
            return false;
        }

        value = getCoordLength (number, axis == Axis::x ? viewBoxW : viewBoxH);
        return true;
    }

    b8 parseCoords (Txt::CharPointerType& s, Point<f32>& p, b8 allowUnits) const
    {
        return parseCoord (s, p.x, allowUnits, Axis::x)
            && parseCoord (s, p.y, allowUnits, Axis::y);
    }

    b8 parseCoordsOrSkip (Txt::CharPointerType& s, Point<f32>& p, b8 allowUnits) const
    {
        if (parseCoords (s, p, allowUnits))
            return true;

        if (! s.isEmpty()) ++s;
        return false;
    }

    f32 getCoordLength (const Txt& s, const f32 sizeForProportions) const noexcept
    {
        auto n = parseSafeFloat (s);
        auto len = s.length();

        if (len > 2)
        {
            auto dpi = 96.0f;

            auto n1 = s[len - 2];
            auto n2 = s[len - 1];

            if (n1 == 'i' && n2 == 'n')         n *= dpi;
            else if (n1 == 'm' && n2 == 'm')    n *= dpi / 25.4f;
            else if (n1 == 'c' && n2 == 'm')    n *= dpi / 2.54f;
            else if (n1 == 'p' && n2 == 'c')    n *= 15.0f;
            else if (n2 == '%')                 n *= 0.01f * sizeForProportions;
        }

        return n;
    }

    f32 getCoordLength (const XmlPath& xml, tukk attName, const f32 sizeForProportions) const noexcept
    {
        return getCoordLength (xml->getStringAttribute (attName), sizeForProportions);
    }

    Array<f32> getCoordList (const XmlElement& xml, Axis axis) const
    {
        const Txt attributeName { axis == Axis::x ? "x" : "y" };

        if (! xml.hasAttribute (attributeName))
            return {};

        return getCoordList (xml.getStringAttribute (attributeName), true, axis);
    }

    Array<f32> getCoordList (const Txt& list, b8 allowUnits, Axis axis) const
    {
        auto text = list.getCharPointer();
        f32 value;
        Array<f32> coords;

        while (parseCoord (text, value, allowUnits, axis))
            coords.add (value);

        return coords;
    }

    static f32 parseSafeFloat (const Txt& s)
    {
        auto n = s.getFloatValue();
        return (std::isnan (n) || std::isinf (n)) ? 0.0f : n;
    }

    //==============================================================================
    z0 parseCSSStyle (const XmlPath& xml)
    {
        cssStyleText = xml->getAllSubText() + "\n" + cssStyleText;
    }

    z0 parseDefs (const XmlPath& xml)
    {
        if (auto* style = xml->getChildByName ("style"))
            parseCSSStyle (xml.getChild (style));
    }

    static Txt::CharPointerType findStyleItem (Txt::CharPointerType source, Txt::CharPointerType name)
    {
        auto nameLength = (i32) name.length();

        while (! source.isEmpty())
        {
            if (source.getAndAdvance() == '.'
                 && CharacterFunctions::compareIgnoreCaseUpTo (source, name, nameLength) == 0)
            {
                auto endOfName = (source + nameLength).findEndOfWhitespace();

                if (*endOfName == '{')
                    return endOfName;

                if (*endOfName == ',')
                    return CharacterFunctions::find (endOfName, (t32) '{');
            }
        }

        return source;
    }

    Txt getStyleAttribute (const XmlPath& xml, StringRef attributeName, const Txt& defaultValue = Txt()) const
    {
        if (xml->hasAttribute (attributeName))
            return xml->getStringAttribute (attributeName, defaultValue);

        auto styleAtt = xml->getStringAttribute ("style");

        if (styleAtt.isNotEmpty())
        {
            auto value = getAttributeFromStyleList (styleAtt, attributeName, {});

            if (value.isNotEmpty())
                return value;
        }
        else if (xml->hasAttribute ("class"))
        {
            for (auto i = cssStyleText.getCharPointer();;)
            {
                auto openBrace = findStyleItem (i, xml->getStringAttribute ("class").getCharPointer());

                if (openBrace.isEmpty())
                    break;

                auto closeBrace = CharacterFunctions::find (openBrace, (t32) '}');

                if (closeBrace.isEmpty())
                    break;

                auto value = getAttributeFromStyleList (Txt (openBrace + 1, closeBrace),
                                                        attributeName, defaultValue);
                if (value.isNotEmpty())
                    return value;

                i = closeBrace + 1;
            }
        }

        if (xml.parent != nullptr)
            return getStyleAttribute (*xml.parent, attributeName, defaultValue);

        return defaultValue;
    }

    Txt getInheritedAttribute (const XmlPath& xml, StringRef attributeName) const
    {
        if (xml->hasAttribute (attributeName))
            return xml->getStringAttribute (attributeName);

        if (xml.parent != nullptr)
            return getInheritedAttribute (*xml.parent, attributeName);

        return {};
    }

    static i32 parsePlacementFlags (const Txt& align) noexcept
    {
        if (align.isEmpty())
            return 0;

        if (isNone (align))
            return RectanglePlacement::stretchToFit;

        return (align.containsIgnoreCase ("slice") ? RectanglePlacement::fillDestination : 0)
             | (align.containsIgnoreCase ("xMin")  ? RectanglePlacement::xLeft
                                                   : (align.containsIgnoreCase ("xMax") ? RectanglePlacement::xRight
                                                                                        : RectanglePlacement::xMid))
             | (align.containsIgnoreCase ("yMin")  ? RectanglePlacement::yTop
                                                   : (align.containsIgnoreCase ("yMax") ? RectanglePlacement::yBottom
                                                                                        : RectanglePlacement::yMid));
    }

    //==============================================================================
    static b8 isIdentifierChar (t32 c)
    {
        return CharacterFunctions::isLetter (c) || c == '-';
    }

    static Txt getAttributeFromStyleList (const Txt& list, StringRef attributeName, const Txt& defaultValue)
    {
        i32 i = 0;

        for (;;)
        {
            i = list.indexOf (i, attributeName);

            if (i < 0)
                break;

            if ((i == 0 || (i > 0 && ! isIdentifierChar (list [i - 1])))
                 && ! isIdentifierChar (list [i + attributeName.length()]))
            {
                i = list.indexOfChar (i, ':');

                if (i < 0)
                    break;

                i32 end = list.indexOfChar (i, ';');

                if (end < 0)
                    end = 0x7ffff;

                return list.substring (i + 1, end).trim();
            }

            ++i;
        }

        return defaultValue;
    }

    //==============================================================================
    static b8 isStartOfNumber (t32 c) noexcept
    {
        return CharacterFunctions::isDigit (c) || c == '-' || c == '+';
    }

    static b8 parseNextNumber (Txt::CharPointerType& text, Txt& value, b8 allowUnits)
    {
        auto s = text;

        while (s.isWhitespace() || *s == ',')
            ++s;

        auto start = s;

        if (isStartOfNumber (*s))
            ++s;

        while (s.isDigit())
            ++s;

        if (*s == '.')
        {
            ++s;

            while (s.isDigit())
                ++s;
        }

        if ((*s == 'e' || *s == 'E') && isStartOfNumber (s[1]))
        {
            s += 2;

            while (s.isDigit())
                ++s;
        }

        if (allowUnits)
            while (s.isLetter())
                ++s;

        if (s == start)
        {
            text = s;
            return false;
        }

        value = Txt (start, s);

        while (s.isWhitespace() || *s == ',')
            ++s;

        text = s;
        return true;
    }

    static b8 parseNextFlag (Txt::CharPointerType& text, b8& value)
    {
        while (text.isWhitespace() || *text == ',')
            ++text;

        if (*text != '0' && *text != '1')
            return false;

        value = *(text++) != '0';

        while (text.isWhitespace() || *text == ',')
             ++text;

        return true;
    }

    //==============================================================================
    Color parseColor (const XmlPath& xml, StringRef attributeName, const Color defaultColor) const
    {
        auto text = getStyleAttribute (xml, attributeName);

        if (text.startsWithChar ('#'))
        {
            u32 hex[8] = { 0 };
            hex[6] = hex[7] = 15;

            i32 numChars = 0;
            auto s = text.getCharPointer();

            while (numChars < 8)
            {
                auto hexValue = CharacterFunctions::getHexDigitValue (*++s);

                if (hexValue >= 0)
                    hex[numChars++] = (u32) hexValue;
                else
                    break;
            }

            if (numChars <= 3)
                return Color ((u8) (hex[0] * 0x11),
                               (u8) (hex[1] * 0x11),
                               (u8) (hex[2] * 0x11));

            return Color ((u8) ((hex[0] << 4) + hex[1]),
                           (u8) ((hex[2] << 4) + hex[3]),
                           (u8) ((hex[4] << 4) + hex[5]),
                           (u8) ((hex[6] << 4) + hex[7]));
        }

        if (text.startsWith ("rgb") || text.startsWith ("hsl"))
        {
            auto tokens = [&text]
            {
                auto openBracket = text.indexOfChar ('(');
                auto closeBracket = text.indexOfChar (openBracket, ')');

                StringArray arr;

                if (openBracket >= 3 && closeBracket > openBracket)
                {
                    arr.addTokens (text.substring (openBracket + 1, closeBracket), ",", "");
                    arr.trim();
                    arr.removeEmptyStrings();
                }

                return arr;
            }();

            auto alpha = [&tokens, &text]
            {
                if ((text.startsWith ("rgba") || text.startsWith ("hsla")) && tokens.size() == 4)
                    return parseSafeFloat (tokens[3]);

                return 1.0f;
            }();

            if (text.startsWith ("hsl"))
                return Color::fromHSL (parseSafeFloat (tokens[0]) / 360.0f,
                                        parseSafeFloat (tokens[1]) / 100.0f,
                                        parseSafeFloat (tokens[2]) / 100.0f,
                                        alpha);

            if (tokens[0].containsChar ('%'))
                return Color ((u8) roundToInt (2.55f * parseSafeFloat (tokens[0])),
                               (u8) roundToInt (2.55f * parseSafeFloat (tokens[1])),
                               (u8) roundToInt (2.55f * parseSafeFloat (tokens[2])),
                               alpha);

            return Color ((u8) tokens[0].getIntValue(),
                           (u8) tokens[1].getIntValue(),
                           (u8) tokens[2].getIntValue(),
                           alpha);
        }

        if (text == "inherit")
        {
            for (const XmlPath* p = xml.parent; p != nullptr; p = p->parent)
                if (getStyleAttribute (*p, attributeName).isNotEmpty())
                    return parseColor (*p, attributeName, defaultColor);
        }

        return Colors::findColorForName (text, defaultColor);
    }

    static AffineTransform parseTransform (Txt t)
    {
        AffineTransform result;

        while (t.isNotEmpty())
        {
            StringArray tokens;
            tokens.addTokens (t.fromFirstOccurrenceOf ("(", false, false)
                               .upToFirstOccurrenceOf (")", false, false),
                              ", ", "");

            tokens.removeEmptyStrings (true);

            f32 numbers[6];

            for (i32 i = 0; i < numElementsInArray (numbers); ++i)
                numbers[i] = parseSafeFloat (tokens[i]);

            AffineTransform trans;

            if (t.startsWithIgnoreCase ("matrix"))
            {
                trans = AffineTransform (numbers[0], numbers[2], numbers[4],
                                         numbers[1], numbers[3], numbers[5]);
            }
            else if (t.startsWithIgnoreCase ("translate"))
            {
                trans = AffineTransform::translation (numbers[0], numbers[1]);
            }
            else if (t.startsWithIgnoreCase ("scale"))
            {
                trans = AffineTransform::scale (numbers[0], numbers[tokens.size() > 1 ? 1 : 0]);
            }
            else if (t.startsWithIgnoreCase ("rotate"))
            {
                trans = AffineTransform::rotation (degreesToRadians (numbers[0]), numbers[1], numbers[2]);
            }
            else if (t.startsWithIgnoreCase ("skewX"))
            {
                trans = AffineTransform::shear (std::tan (degreesToRadians (numbers[0])), 0.0f);
            }
            else if (t.startsWithIgnoreCase ("skewY"))
            {
                trans = AffineTransform::shear (0.0f, std::tan (degreesToRadians (numbers[0])));
            }

            result = trans.followedBy (result);
            t = t.fromFirstOccurrenceOf (")", false, false).trimStart();
        }

        return result;
    }

    static z0 endpointToCentreParameters (f64 x1, f64 y1,
                                            f64 x2, f64 y2,
                                            f64 angle,
                                            b8 largeArc, b8 sweep,
                                            f64& rx, f64& ry,
                                            f64& centreX, f64& centreY,
                                            f64& startAngle, f64& deltaAngle) noexcept
    {
        const f64 midX = (x1 - x2) * 0.5;
        const f64 midY = (y1 - y2) * 0.5;

        const f64 cosAngle = std::cos (angle);
        const f64 sinAngle = std::sin (angle);
        const f64 xp = cosAngle * midX + sinAngle * midY;
        const f64 yp = cosAngle * midY - sinAngle * midX;
        const f64 xp2 = xp * xp;
        const f64 yp2 = yp * yp;

        f64 rx2 = rx * rx;
        f64 ry2 = ry * ry;

        const f64 s = (xp2 / rx2) + (yp2 / ry2);
        f64 c;

        if (s <= 1.0)
        {
            c = std::sqrt (jmax (0.0, ((rx2 * ry2) - (rx2 * yp2) - (ry2 * xp2))
                                         / (( rx2 * yp2) + (ry2 * xp2))));

            if (largeArc == sweep)
                c = -c;
        }
        else
        {
            const f64 s2 = std::sqrt (s);
            rx *= s2;
            ry *= s2;
            c = 0;
        }

        const f64 cpx = ((rx * yp) / ry) * c;
        const f64 cpy = ((-ry * xp) / rx) * c;

        centreX = ((x1 + x2) * 0.5) + (cosAngle * cpx) - (sinAngle * cpy);
        centreY = ((y1 + y2) * 0.5) + (sinAngle * cpx) + (cosAngle * cpy);

        const f64 ux = (xp - cpx) / rx;
        const f64 uy = (yp - cpy) / ry;
        const f64 vx = (-xp - cpx) / rx;
        const f64 vy = (-yp - cpy) / ry;

        const f64 length = drx_hypot (ux, uy);

        startAngle = acos (jlimit (-1.0, 1.0, ux / length));

        if (uy < 0)
            startAngle = -startAngle;

        startAngle += MathConstants<f64>::halfPi;

        deltaAngle = acos (jlimit (-1.0, 1.0, ((ux * vx) + (uy * vy))
                                                / (length * drx_hypot (vx, vy))));

        if ((ux * vy) - (uy * vx) < 0)
            deltaAngle = -deltaAngle;

        if (sweep)
        {
            if (deltaAngle < 0)
                deltaAngle += MathConstants<f64>::twoPi;
        }
        else
        {
            if (deltaAngle > 0)
                deltaAngle -= MathConstants<f64>::twoPi;
        }

        deltaAngle = fmod (deltaAngle, MathConstants<f64>::twoPi);
    }

    SVGState (const SVGState&) = default;
    SVGState& operator= (const SVGState&) = delete;
};


//==============================================================================
std::unique_ptr<Drawable> Drawable::createFromSVG (const XmlElement& svgDocument)
{
    if (! svgDocument.hasTagNameIgnoringNamespace ("svg"))
        return {};

    SVGState state (&svgDocument);
    return std::unique_ptr<Drawable> (state.parseSVGElement (SVGState::XmlPath (&svgDocument, {})));
}

std::unique_ptr<Drawable> Drawable::createFromSVGFile (const File& svgFile)
{
    if (auto xml = parseXMLIfTagMatches (svgFile, "svg"))
        return createFromSVG (*xml);

    return {};
}

Path Drawable::parseSVGPath (const Txt& svgPath)
{
    SVGState state (nullptr);
    Path p;
    state.parsePathString (p, svgPath);
    return p;
}

} // namespace drx
