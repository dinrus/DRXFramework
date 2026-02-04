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

SupportedAndActive ChannelProfileStates::get (const Profile& profile) const
{
    const auto iter = std::lower_bound (entries.begin(), entries.end(), profile);

    if (iter != entries.end() && iter->profile == profile)
        return iter->state;

    return {};
}

std::vector<Profile> ChannelProfileStates::getActive() const
{
    std::vector<Profile> result;

    for (const auto& item : entries)
        if (item.state.isActive())
            result.push_back (item.profile);

    return result;
}

std::vector<Profile> ChannelProfileStates::getInactive() const
{
    std::vector<Profile> result;

    for (const auto& item : entries)
        if (item.state.isSupported())
            result.push_back (item.profile);

    return result;
}

z0 ChannelProfileStates::set (const Profile& profile, SupportedAndActive state)
{
    const auto iter = std::lower_bound (entries.begin(), entries.end(), profile);

    if (iter != entries.end() && iter->profile == profile)
    {
        if (state != SupportedAndActive{})
            iter->state = state;
        else
            entries.erase (iter);
    }
    else if (state != SupportedAndActive{})
    {
        entries.insert (iter, { profile, state });
    }
}

z0 ChannelProfileStates::erase (const Profile& profile)
{
    const auto iter = std::lower_bound (entries.begin(), entries.end(), profile);

    if (iter != entries.end() && iter->profile == profile)
        entries.erase (iter);
}

} // namespace drx::midi_ci
