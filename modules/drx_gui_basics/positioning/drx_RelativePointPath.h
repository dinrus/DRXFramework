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
    A path object that consists of RelativePoint coordinates rather than the normal fixed ones.

    One of these paths can be converted into a Path object for drawing and manipulation, but
    unlike a Path, its points can be dynamic instead of just fixed.

    @see RelativePoint, RelativeCoordinate

    @tags{GUI}
*/
class DRX_API  RelativePointPath
{
public:
    //==============================================================================
    RelativePointPath();
    RelativePointPath (const RelativePointPath&);
    explicit RelativePointPath (const Path& path);
    ~RelativePointPath();

    b8 operator== (const RelativePointPath&) const noexcept;
    b8 operator!= (const RelativePointPath&) const noexcept;

    //==============================================================================
    /** Resolves this points in this path and adds them to a normal Path object. */
    z0 createPath (Path& path, Expression::Scope* scope) const;

    /** Возвращает true, если the path contains any non-fixed points. */
    b8 containsAnyDynamicPoints() const;

    /** Quickly swaps the contents of this path with another. */
    z0 swapWith (RelativePointPath&) noexcept;

    //==============================================================================
    /** The types of element that may be contained in this path.
        @see RelativePointPath::ElementBase
    */
    enum ElementType
    {
        nullElement,
        startSubPathElement,
        closeSubPathElement,
        lineToElement,
        quadraticToElement,
        cubicToElement
    };

    //==============================================================================
    /** Base class for the elements that make up a RelativePointPath.
    */
    class DRX_API  ElementBase
    {
    public:
        ElementBase (ElementType type);
        virtual ~ElementBase() = default;
        virtual z0 addToPath (Path& path, Expression::Scope*) const = 0;
        virtual RelativePoint* getControlPoints (i32& numPoints) = 0;
        virtual ElementBase* clone() const = 0;
        b8 isDynamic();

        const ElementType type;

    private:
        DRX_DECLARE_NON_COPYABLE (ElementBase)
    };

    //==============================================================================
    /** Class for the start sub path element */
    class DRX_API  StartSubPath  : public ElementBase
    {
    public:
        StartSubPath (const RelativePoint& pos);
        z0 addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (i32& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint startPos;

    private:
        DRX_DECLARE_NON_COPYABLE (StartSubPath)
    };

    //==============================================================================
    /** Class for the close sub path element */
    class DRX_API  CloseSubPath  : public ElementBase
    {
    public:
        CloseSubPath();
        z0 addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (i32& numPoints) override;
        ElementBase* clone() const override;

    private:
        DRX_DECLARE_NON_COPYABLE (CloseSubPath)
    };

    //==============================================================================
    /** Class for the line to element */
    class DRX_API  LineTo  : public ElementBase
    {
    public:
        LineTo (const RelativePoint& endPoint);
        z0 addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (i32& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint endPoint;

    private:
        DRX_DECLARE_NON_COPYABLE (LineTo)
    };

    //==============================================================================
    /** Class for the quadratic to element */
    class DRX_API  QuadraticTo  : public ElementBase
    {
    public:
        QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint);
        ValueTree createTree() const;
        z0 addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (i32& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint controlPoints[2];

    private:
        DRX_DECLARE_NON_COPYABLE (QuadraticTo)
    };

    //==============================================================================
    /** Class for the cubic to element */
    class DRX_API  CubicTo  : public ElementBase
    {
    public:
        CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint);
        ValueTree createTree() const;
        z0 addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (i32& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint controlPoints[3];

    private:
        DRX_DECLARE_NON_COPYABLE (CubicTo)
    };

    //==============================================================================
    z0 addElement (ElementBase* newElement);

    //==============================================================================
    OwnedArray<ElementBase> elements;
    b8 usesNonZeroWinding;

private:
    class Positioner;
    friend class Positioner;
    b8 containsDynamicPoints;

    z0 applyTo (DrawablePath& path) const;

    RelativePointPath& operator= (const RelativePointPath&);
    DRX_LEAK_DETECTOR (RelativePointPath)
};

} // namespace drx
