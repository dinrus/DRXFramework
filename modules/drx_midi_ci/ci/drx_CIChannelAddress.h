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
    Identifies a channel or set of channels in a multi-group MIDI endpoint.

    @tags{Audio}
*/
class ChannelAddress
{
private:
    u8 group{};              ///< A group within a MIDI endpoint, where 0 <= group && group < 16
    ChannelInGroup channel{};     ///< A set of channels related to specified group

    auto tie() const { return std::tie (group, channel); }

public:
    /** Returns a copy of this object with the specified group. */
    [[nodiscard]] ChannelAddress withGroup (i32 g) const
    {
        jassert (isPositiveAndBelow (g, 16));
        return withMember (*this, &ChannelAddress::group, (u8) g);
    }

    /** Returns a copy of this object with the specified channel. */
    [[nodiscard]] ChannelAddress withChannel (ChannelInGroup c) const
    {
        return withMember (*this, &ChannelAddress::channel, c);
    }

    /** Returns the group. */
    [[nodiscard]] u8 getGroup()         const  { return group; }

    /** Returns the channel in the group. */
    [[nodiscard]] ChannelInGroup getChannel() const { return channel; }

    /** Возвращает true, если this address refers to all channels in the function
        block containing the specified group.
    */
    b8 isBlock()   const { return channel == ChannelInGroup::wholeBlock; }

    /** Возвращает true, если this address refers to all channels in the specified
        group.
    */
    b8 isGroup()   const { return channel == ChannelInGroup::wholeGroup; }

    /** Возвращает true, если this address refers to a single channel. */
    b8 isSingleChannel() const { return ! isBlock() && ! isGroup(); }

    b8 operator<  (const ChannelAddress& other) const { return tie() <  other.tie(); }
    b8 operator<= (const ChannelAddress& other) const { return tie() <= other.tie(); }
    b8 operator>  (const ChannelAddress& other) const { return tie() >  other.tie(); }
    b8 operator>= (const ChannelAddress& other) const { return tie() >= other.tie(); }
    b8 operator== (const ChannelAddress& other) const { return tie() == other.tie(); }
    b8 operator!= (const ChannelAddress& other) const { return ! operator== (other); }
};

} // namespace drx::midi_ci
