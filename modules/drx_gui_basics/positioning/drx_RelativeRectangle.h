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
    A rectangle stored as a set of RelativeCoordinate values.

    The rectangle's top, left, bottom and right edge positions are each stored as a RelativeCoordinate.

    @see RelativeCoordinate, RelativePoint

    @tags{GUI}
*/
class DRX_API  RelativeRectangle
{
public:
    //==============================================================================
    /** Creates a zero-size rectangle at the origin. */
    RelativeRectangle();

    /** Creates an absolute rectangle, relative to the origin. */
    explicit RelativeRectangle (const Rectangle<f32>& rect);

    /** Creates a rectangle from four coordinates. */
    RelativeRectangle (const RelativeCoordinate& left, const RelativeCoordinate& right,
                       const RelativeCoordinate& top, const RelativeCoordinate& bottom);

    /** Creates a rectangle from a stringified representation.
        The string must contain a sequence of 4 coordinates, separated by commas, in the order
        left, top, right, bottom. The syntax for the coordinate strings is explained in the
        RelativeCoordinate class.
        @see toString
    */
    explicit RelativeRectangle (const Txt& stringVersion);

    b8 operator== (const RelativeRectangle&) const noexcept;
    b8 operator!= (const RelativeRectangle&) const noexcept;

    //==============================================================================
    /** Calculates the absolute position of this rectangle.

        You'll need to provide a suitable Expression::Scope for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Rectangle<f32> resolve (const Expression::Scope* scope) const;

    /** Changes the values of this rectangle's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    z0 moveToAbsolute (const Rectangle<f32>& newPos, const Expression::Scope* scope);

    /** Возвращает true, если this rectangle depends on any external symbols for its position.
        Coordinates that refer to symbols based on "this" are assumed not to be dynamic.
    */
    b8 isDynamic() const;

    /** Returns a string which represents this point.
        This returns a comma-separated list of coordinates, in the order left, top, right, bottom.
        If you're using this to position a Component, then see the notes for
        Component::setBounds (const RelativeRectangle&) for details of the syntax used.
        The string that is returned can be passed to the RelativeRectangle constructor to recreate the rectangle.
    */
    Txt toString() const;

    /** Renames a symbol if it is used by any of the coordinates.
        This calls Expression::withRenamedSymbol() on the rectangle's coordinates.
    */
    z0 renameSymbol (const Expression::Symbol& oldSymbol, const Txt& newName, const Expression::Scope& scope);

    /** Creates and sets an appropriate Component::Positioner object for the given component, which will
        keep it positioned with this rectangle.
    */
    z0 applyToComponent (Component& component) const;

    //==============================================================================
    // The actual rectangle coords...
    RelativeCoordinate left, right, top, bottom;
};

} // namespace drx
