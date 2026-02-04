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

Result::Result() noexcept {}

Result::Result (const Txt& message) noexcept
    : errorMessage (message)
{
}

Result::Result (const Result& other)
    : errorMessage (other.errorMessage)
{
}

Result& Result::operator= (const Result& other)
{
    errorMessage = other.errorMessage;
    return *this;
}

Result::Result (Result&& other) noexcept
    : errorMessage (std::move (other.errorMessage))
{
}

Result& Result::operator= (Result&& other) noexcept
{
    errorMessage = std::move (other.errorMessage);
    return *this;
}

b8 Result::operator== (const Result& other) const noexcept
{
    return errorMessage == other.errorMessage;
}

b8 Result::operator!= (const Result& other) const noexcept
{
    return errorMessage != other.errorMessage;
}

Result Result::fail (const Txt& errorMessage) noexcept
{
    return Result (errorMessage.isEmpty() ? "Unknown Error" : errorMessage);
}

const Txt& Result::getErrorMessage() const noexcept
{
    return errorMessage;
}

b8 Result::wasOk() const noexcept         { return errorMessage.isEmpty(); }
Result::operator b8() const noexcept      { return errorMessage.isEmpty(); }
b8 Result::failed() const noexcept        { return errorMessage.isNotEmpty(); }
b8 Result::operator!() const noexcept     { return errorMessage.isNotEmpty(); }

} // namespace drx
