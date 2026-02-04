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
    An interface with methods that can be overridden to customise how a Device
    implementing profiles responds to profile inquiries.

    @tags{Audio}
*/
struct ProfileDelegate
{
    ProfileDelegate() = default;
    virtual ~ProfileDelegate() = default;
    ProfileDelegate (const ProfileDelegate&) = default;
    ProfileDelegate (ProfileDelegate&&) = default;
    ProfileDelegate& operator= (const ProfileDelegate&) = default;
    ProfileDelegate& operator= (ProfileDelegate&&) = default;

    /** Called when a remote device requests that a profile is enabled or disabled.

        Old MIDI-CI implementations on remote devices may request that a profile
        is enabled with zero channels active - in this situation, it is
        recommended that you use ProfileHost::enableProfile to enable the
        default number of channels for that profile.

        Additionally, profiles for entire groups or function blocks may be enabled with zero
        active channels. In this case, the profile should be enabled on the entire group or
        function block.
    */
    virtual z0 profileEnablementRequested ([[maybe_unused]] MUID x,
                                             [[maybe_unused]] ProfileAtAddress profileAtAddress,
                                             [[maybe_unused]] i32 numChannels,
                                             [[maybe_unused]] b8 enabled) = 0;
};

} // namespace drx::midi_ci
