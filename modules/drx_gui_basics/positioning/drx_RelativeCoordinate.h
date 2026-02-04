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
    Expresses a coordinate as a dynamically evaluated expression.

    When using relative coordinates to position components, the following symbols are available:
     - "left", "right", "top", "bottom" refer to the position of those edges in this component, so
       e.g. for a component whose width is always 100, you might set the right edge to the "left + 100".
     - "[id].left", "[id].right", "[id].top", "[id].bottom", "[id].width", "[id].height", where [id] is
       the identifier of one of this component's siblings. A component's identifier is set with
       Component::setComponentID(). So for example if you want your component to always be 50 pixels to the
       right of the one called "xyz", you could set your left edge to be "xyz.right + 50".
     - Instead of an [id], you can use the name "parent" to refer to this component's parent. Like
       any other component, these values are relative to their component's parent, so "parent.right" won't be
       very useful for positioning a component because it refers to a position with the parent's parent.. but
       "parent.width" can be used for setting positions relative to the parent's size. E.g. to make a 10x10
       component which remains 1 pixel away from its parent's bottom-right, you could use
       "right - 10, bottom - 10, parent.width - 1, parent.height - 1".
     - The name of one of the parent component's markers can also be used as a symbol. For markers to be
       used, the parent component must implement its Component::getMarkers() method, and return at least one
       valid MarkerList. So if you want your component's top edge to be 10 pixels below the
       marker called "foobar", you'd set it to "foobar + 10".

    See the Expression class for details about the operators that are supported, but for example
    if you wanted to make your component remains centred within its parent with a size of 100, 100,
    you could express it as:
    @code myComp.setBounds (RelativeBounds ("parent.width / 2 - 50, parent.height / 2 - 50, left + 100, top + 100"));
    @endcode
    ..or an alternative way to achieve the same thing:
    @code myComp.setBounds (RelativeBounds ("right - 100, bottom - 100, parent.width / 2 + 50, parent.height / 2 + 50"));
    @endcode

    Or if you wanted a 100x100 component whose top edge is lined up to a marker called "topMarker" and
    which is positioned 50 pixels to the right of another component called "otherComp", you could write:
    @code myComp.setBounds (RelativeBounds ("otherComp.right + 50, topMarker, left + 100, top + 100"));
    @endcode

    Be careful not to make your coordinate expressions recursive, though, or exceptions and assertions will
    be thrown!

    @see RelativePoint, RelativeRectangle

    @tags{GUI}
*/
class DRX_API  RelativeCoordinate
{
public:
    //==============================================================================
    /** Creates a zero coordinate. */
    RelativeCoordinate();
    RelativeCoordinate (const Expression& expression);
    RelativeCoordinate (const RelativeCoordinate&);
    RelativeCoordinate& operator= (const RelativeCoordinate&);
    RelativeCoordinate (RelativeCoordinate&&) noexcept;
    RelativeCoordinate& operator= (RelativeCoordinate&&) noexcept;

    /** Creates an absolute position from the parent origin on either the X or Y axis.

        @param absoluteDistanceFromOrigin   the distance from the origin
    */
    RelativeCoordinate (f64 absoluteDistanceFromOrigin);

    /** Recreates a coordinate from a string description.
        The string will be parsed by ExpressionParser::parse().
        @param stringVersion    the expression to use
        @see toString
    */
    RelativeCoordinate (const Txt& stringVersion);

    /** Destructor. */
    ~RelativeCoordinate();

    b8 operator== (const RelativeCoordinate&) const noexcept;
    b8 operator!= (const RelativeCoordinate&) const noexcept;

    //==============================================================================
    /** Calculates the absolute position of this coordinate.

        You'll need to provide a suitable Expression::Scope for looking up any coordinates that may
        be needed to calculate the result.
    */
    f64 resolve (const Expression::Scope* evaluationScope) const;

    /** Возвращает true, если this coordinate uses the specified coord name at any level in its evaluation.
        This will recursively check any coordinates upon which this one depends.
    */
    b8 references (const Txt& coordName, const Expression::Scope* evaluationScope) const;

    /** Возвращает true, если there's a recursive loop when trying to resolve this coordinate's position. */
    b8 isRecursive (const Expression::Scope* evaluationScope) const;

    /** Возвращает true, если this coordinate depends on any other coordinates for its position. */
    b8 isDynamic() const;

    //==============================================================================
    /** Changes the value of this coord to make it resolve to the specified position.

        Calling this will leave the anchor points unchanged, but will set this coordinate's absolute
        or relative position to whatever value is necessary to make its resultant position
        match the position that is provided.
    */
    z0 moveToAbsolute (f64 absoluteTargetPosition, const Expression::Scope* evaluationScope);

    /** Returns the expression that defines this coordinate. */
    const Expression& getExpression() const         { return term; }


    //==============================================================================
    /** Returns a string which represents this coordinate.
        For details of the string syntax, see the constructor notes.
    */
    Txt toString() const;

    //==============================================================================
    /** A set of static strings that are commonly used by the RelativeCoordinate class.

        As well as avoiding using string literals in your code, using these preset values
        has the advantage that all instances of the same string will share the same, reference-counted
        Txt object, so if you have thousands of points which all refer to the same
        anchor points, this can save a significant amount of memory allocation.
    */
    struct Strings
    {
        static const Txt parent;         /**< "parent" */
        static const Txt left;           /**< "left" */
        static const Txt right;          /**< "right" */
        static const Txt top;            /**< "top" */
        static const Txt bottom;         /**< "bottom" */
        static const Txt x;              /**< "x" */
        static const Txt y;              /**< "y" */
        static const Txt width;          /**< "width" */
        static const Txt height;         /**< "height" */
    };

    /** @internal */
    struct StandardStrings
    {
        enum Type
        {
            left, right, top, bottom,
            x, y, width, height,
            parent,
            unknown
        };

        static Type getTypeOf (const Txt& s) noexcept;
    };

private:
    //==============================================================================
    Expression term;
};

} // namespace drx
