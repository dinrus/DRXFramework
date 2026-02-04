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

const Txt RelativeCoordinate::Strings::parent ("parent");
const Txt RelativeCoordinate::Strings::left ("left");
const Txt RelativeCoordinate::Strings::right ("right");
const Txt RelativeCoordinate::Strings::top ("top");
const Txt RelativeCoordinate::Strings::bottom ("bottom");
const Txt RelativeCoordinate::Strings::x ("x");
const Txt RelativeCoordinate::Strings::y ("y");
const Txt RelativeCoordinate::Strings::width ("width");
const Txt RelativeCoordinate::Strings::height ("height");

RelativeCoordinate::StandardStrings::Type RelativeCoordinate::StandardStrings::getTypeOf (const Txt& s) noexcept
{
    if (s == Strings::left)    return left;
    if (s == Strings::right)   return right;
    if (s == Strings::top)     return top;
    if (s == Strings::bottom)  return bottom;
    if (s == Strings::x)       return x;
    if (s == Strings::y)       return y;
    if (s == Strings::width)   return width;
    if (s == Strings::height)  return height;
    if (s == Strings::parent)  return parent;
    return unknown;
}

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
{
}

RelativeCoordinate::RelativeCoordinate (const Expression& term_)
    : term (term_)
{
}

RelativeCoordinate::RelativeCoordinate (const RelativeCoordinate& other)
    : term (other.term)
{
}

RelativeCoordinate& RelativeCoordinate::operator= (const RelativeCoordinate& other)
{
    term = other.term;
    return *this;
}

RelativeCoordinate::RelativeCoordinate (RelativeCoordinate&& other) noexcept
    : term (std::move (other.term))
{
}

RelativeCoordinate& RelativeCoordinate::operator= (RelativeCoordinate&& other) noexcept
{
    term = std::move (other.term);
    return *this;
}

RelativeCoordinate::RelativeCoordinate (const f64 absoluteDistanceFromOrigin)
    : term (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const Txt& s)
{
    Txt error;
    term = Expression (s, error);
}

RelativeCoordinate::~RelativeCoordinate()
{
}

b8 RelativeCoordinate::operator== (const RelativeCoordinate& other) const noexcept
{
    return term.toString() == other.term.toString();
}

b8 RelativeCoordinate::operator!= (const RelativeCoordinate& other) const noexcept
{
    return ! operator== (other);
}

f64 RelativeCoordinate::resolve (const Expression::Scope* scope) const
{
    if (scope != nullptr)
        return term.evaluate (*scope);

    return term.evaluate();
}

b8 RelativeCoordinate::isRecursive (const Expression::Scope* scope) const
{
    Txt error;

    if (scope != nullptr)
        term.evaluate (*scope, error);
    else
        term.evaluate (Expression::Scope(), error);

    return error.isNotEmpty();
}

z0 RelativeCoordinate::moveToAbsolute (f64 newPos, const Expression::Scope* scope)
{
    if (scope != nullptr)
    {
        term = term.adjustedToGiveNewResult (newPos, *scope);
    }
    else
    {
        Expression::Scope defaultScope;
        term = term.adjustedToGiveNewResult (newPos, defaultScope);
    }
}

b8 RelativeCoordinate::isDynamic() const
{
    return term.usesAnySymbols();
}

Txt RelativeCoordinate::toString() const
{
    return term.toString();
}

} // namespace drx
