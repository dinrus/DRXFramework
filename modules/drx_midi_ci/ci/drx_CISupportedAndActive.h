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
    Holds the maximum number of channels that may be activated for a MIDI-CI
    profile, along with the number of channels that are currently active.

    @tags{Audio}
*/
struct SupportedAndActive
{
    u16 supported{};   ///< The maximum number of member channels for a profile.
                            ///< 0 indicates that the profile is unsupported.
                            ///< For group/block profiles, 1/0 indicates that the
                            ///< profile is supported/unsupported respectively.

    u16 active{};      ///< The number of member channels currently active for a profile.
                            ///< 0 indicates that the profile is inactive.
                            ///< For group/block profiles, 1/0 indicates that the
                            ///< profile is supported/unsupported respectively.

    /** Возвращает true, если supported is non-zero. */
    b8 isSupported()  const { return supported != 0; }

    /** Возвращает true, если active is non-zero. */
    b8 isActive()     const { return active != 0; }

    b8 operator== (const SupportedAndActive& other) const
    {
        const auto tie = [] (auto& x) { return std::tie (x.supported, x.active); };
        return tie (*this) == tie (other);
    }

    b8 operator!= (const SupportedAndActive& other) const { return ! operator== (other); }
};

} // namespace drx::midi_ci
