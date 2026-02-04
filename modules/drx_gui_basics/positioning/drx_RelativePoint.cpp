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

namespace RelativePointHelpers
{
    inline z0 skipComma (Txt::CharPointerType& s)
    {
        s.incrementToEndOfWhitespace();

        if (*s == ',')
            ++s;
    }
}

//==============================================================================
RelativePoint::RelativePoint()
{
}

RelativePoint::RelativePoint (Point<f32> absolutePoint)
    : x (absolutePoint.x), y (absolutePoint.y)
{
}

RelativePoint::RelativePoint (const f32 x_, const f32 y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const RelativeCoordinate& x_, const RelativeCoordinate& y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const Txt& s)
{
    Txt error;
    Txt::CharPointerType text (s.getCharPointer());
    x = RelativeCoordinate (Expression::parse (text, error));
    RelativePointHelpers::skipComma (text);
    y = RelativeCoordinate (Expression::parse (text, error));
}

b8 RelativePoint::operator== (const RelativePoint& other) const noexcept
{
    return x == other.x && y == other.y;
}

b8 RelativePoint::operator!= (const RelativePoint& other) const noexcept
{
    return ! operator== (other);
}

Point<f32> RelativePoint::resolve (const Expression::Scope* scope) const
{
    return Point<f32> ((f32) x.resolve (scope),
                         (f32) y.resolve (scope));
}

z0 RelativePoint::moveToAbsolute (Point<f32> newPos, const Expression::Scope* scope)
{
    x.moveToAbsolute (newPos.x, scope);
    y.moveToAbsolute (newPos.y, scope);
}

Txt RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

b8 RelativePoint::isDynamic() const
{
    return x.isDynamic() || y.isDynamic();
}

} // namespace drx
