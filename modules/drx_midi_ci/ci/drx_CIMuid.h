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

namespace drx::midi_ci
{

/**
    A 28-bit ID that uniquely identifies a device taking part in a series of
    MIDI-CI transactions.

    @tags{Audio}
*/
class MUID
{
    constexpr explicit MUID (u32 v) : value (v) {}

    // 0x0fffff00 to 0x0ffffffe are reserved, 0x0fffffff is 'broadcast'
    static constexpr u32 userMuidEnd = 0x0fffff00;
    static constexpr u32 mask = 0x0fffffff;
    u32 value{};

public:
    /** Returns the ID as a plain integer. */
    constexpr u32 get() const { return value; }

    /** Converts the provided integer to a MUID without validation that it
        is within the allowed range.
    */
    static MUID makeUnchecked (u32 v)
    {
        // If this is hit, the MUID has too many bits set!
        jassert ((v & mask) == v);
        return MUID (v);
    }

    /** Returns a MUID if the provided value is within the valid range for
        MUID values; otherwise returns nullopt.
    */
    static std::optional<MUID> make (u32 v)
    {
        if ((v & mask) == v)
            return makeUnchecked (v);

        return {};
    }

    /** Makes a random MUID using the provided random engine. */
    static MUID makeRandom (Random& r)
    {
        return makeUnchecked ((u32) r.nextInt (userMuidEnd));
    }

    b8 operator== (const MUID other) const { return value == other.value; }
    b8 operator!= (const MUID other) const { return value != other.value; }
    b8 operator<  (const MUID other) const { return value <  other.value; }

    /** Returns the special MUID representing the broadcast address. */
    static constexpr MUID getBroadcast() { return MUID { mask }; }
};

} // namespace drx::midi_ci
