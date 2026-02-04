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
    An OSC time tag.

    OSC time tags are part of OSCBundle objects.

    In accordance with the OSC 1.0 specification, the internal timestamp stored in
    OSCTimeTag uses the same binary format as NTP timestamps. The representation
    is by a 64 bit fixed point number. The first 32 bits specify the number of
    seconds since midnight on January 1, 1900, and the last 32 bits specify
    fractional parts of a second to a precision of about 200 picoseconds.

    The time tag value consisting of 63 zero bits followed by a one in the least
    significant bit is a special case meaning "immediately".

    For a more user-friendly time format, convert OSCTimeTag to a drx::Time object
    using toTime().

    @tags{OSC}
*/
class DRX_API  OSCTimeTag
{
public:
    //==============================================================================
    /** Default constructor.
        Constructs an OSCTimeTag object with the special value representing "immediately".
    */
    OSCTimeTag() noexcept;

    /** Constructs an OSCTimeTag object from a raw binary OSC time tag. */
    OSCTimeTag (zu64 rawTimeTag) noexcept;

    /** Constructs an OSCTimeTag object from a drx::Time object. */
    OSCTimeTag (Time time) noexcept;

    /** Returns a drx::Time object representing the same time as the OSCTimeTag.

        If the OSCTimeTag has the special value representing "immediately", the
        resulting drx::Time object will represent an arbitrary point of time (but
        guaranteed to be in the past), since drx::Time does not have such a special value.
    */
    Time toTime() const noexcept;

    /** Возвращает true, если the OSCTimeTag object has the special value representing "immediately". */
    b8 isImmediately() const noexcept;

    /** Returns the raw binary OSC time tag representation. */
    zu64 getRawTimeTag() const noexcept               { return rawTimeTag; }

    /** The special value representing "immediately". */
    static const OSCTimeTag immediately;

private:
    //==============================================================================
    zu64 rawTimeTag;
};

} // namespace drx
