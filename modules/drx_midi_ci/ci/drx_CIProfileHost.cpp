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

class ProfileHost::Visitor : public detail::MessageTypeUtils::MessageVisitor
{
public:
    Visitor (ProfileHost* h, ResponderOutput* o, b8* b)
        : host (h), output (o), handled (b) {}

    z0 visit (const Message::ProfileInquiry& body) const override { visitImpl (body); }
    z0 visit (const Message::ProfileDetails& body) const override { visitImpl (body); }
    z0 visit (const Message::ProfileOn& body)      const override { visitImpl (body); }
    z0 visit (const Message::ProfileOff& body)     const override { visitImpl (body); }
    using MessageVisitor::visit;

    static auto getNumChannels (Message::Header header, Message::ProfileOn p)
    {
        return (u8) header.version >= 2 ? p.numChannels : 1;
    }

    static auto getNumChannels (Message::Header, Message::ProfileOff)  { return 0; }

private:
    template <typename Body>
    z0 visitImpl (const Body& body) const { *handled = messageReceived (body); }

    b8 messageReceived (const Message::ProfileInquiry&) const
    {
        host->isResponder = true;

        if ((u8) output->getIncomingHeader().deviceID < 16
            || output->getIncomingHeader().deviceID == ChannelInGroup::wholeGroup)
        {
            if (const auto* state = host->getProfileStates().groupStates[output->getIncomingGroup()].getStateForDestination (output->getIncomingHeader().deviceID))
            {
                const auto active   = state->getActive();
                const auto inactive = state->getInactive();
                detail::MessageTypeUtils::send (*output, Message::ProfileInquiryResponse { active, inactive });
            }
        }
        else if (output->getIncomingHeader().deviceID == ChannelInGroup::wholeBlock)
        {
            auto header = output->getReplyHeader (detail::MessageMeta::Meta<Message::ProfileInquiryResponse>::subID2);

            const auto sendIfNonEmpty = [&] (const auto group, const auto& state)
            {
                if (! state.empty())
                {
                    const auto active   = state.getActive();
                    const auto inactive = state.getInactive();
                    detail::MessageTypeUtils::send (*output, (u8) group, header, Message::ProfileInquiryResponse { active, inactive });
                }
            };

            for (auto groupNum = 0; groupNum < host->functionBlock.numGroups; ++groupNum)
            {
                const auto group = host->functionBlock.firstGroup + groupNum;
                const auto& groupState = host->getProfileStates().groupStates[(size_t) group];

                for (size_t channel = 0; channel < groupState.channelStates.size(); ++channel)
                {
                    header.deviceID = ChannelInGroup (channel);
                    sendIfNonEmpty (group, groupState.channelStates[channel]);
                }
            }

            header.deviceID = ChannelInGroup::wholeGroup;

            for (auto i = 0; i < host->functionBlock.numGroups; ++i)
            {
                const auto group = host->functionBlock.firstGroup + i;
                const auto& groupState = host->getProfileStates().groupStates[(size_t) group];
                sendIfNonEmpty (group, groupState.groupState);
            }

            // Always send the block response to indicate that no further replies will follow
            header.deviceID = ChannelInGroup::wholeBlock;
            const auto state = host->getProfileStates().blockState;
            const auto active   = state.getActive();
            const auto inactive = state.getInactive();
            detail::MessageTypeUtils::send (*output, output->getIncomingGroup(), header, Message::ProfileInquiryResponse { active, inactive });
        }

        return true;
    }

    b8 messageReceived (const Message::ProfileDetails& body) const
    {
        if (body.target == std::byte{})
        {
            const auto address = ChannelAddress{}.withGroup (output->getIncomingGroup())
                                                 .withChannel (output->getIncomingHeader().deviceID);
            const ProfileAtAddress profileAtAddress { body.profile, address };
            const auto state = host->getState (profileAtAddress);
            std::vector<std::byte> extraData;
            detail::Marshalling::Writer { extraData } (state.active, state.supported);
            detail::MessageTypeUtils::send (*output, Message::ProfileDetailsResponse { body.profile, body.target, extraData });
        }
        else
        {
            detail::MessageTypeUtils::sendNAK (*output, std::byte { 0x04 });
        }

        return true;
    }

    template <typename Body>
    b8 profileEnablementReceived (const Body& request) const
    {
        const auto destination = ChannelAddress{}.withGroup (output->getIncomingGroup())
                                                 .withChannel (output->getIncomingHeader().deviceID);
        if (auto* state = host->states.getStateForDestination (destination))
        {
            const auto previousState = state->get (request.profile);

            if (previousState.isSupported())
            {
                const ProfileAtAddress profileAtAddress { request.profile, destination };

                {
                    const ScopedValueSetter scope { host->currentEnablementMessage,
                                                    std::optional<ProfileAtAddress> (profileAtAddress) };
                    host->delegate.profileEnablementRequested (output->getIncomingHeader().source,
                                                               profileAtAddress,
                                                               getNumChannels (output->getIncomingHeader(), request),
                                                               std::is_same_v<Message::ProfileOn, Body>);
                }

                const auto currentState = host->getState (profileAtAddress);

                const auto sendResponse = [&] (auto response)
                {
                    const Message::Header header
                    {
                        profileAtAddress.address.getChannel(),
                        detail::MessageMeta::Meta<decltype (response)>::subID2,
                        detail::MessageMeta::implementationVersion,
                        output->getMuid(),
                        MUID::getBroadcast(),
                    };

                    detail::MessageTypeUtils::send (*output, profileAtAddress.address.getGroup(), header, response);
                };

                const auto numIndividualChannels = (std::is_same_v<Message::ProfileOn, Body> ? currentState : previousState).active;

                const auto numChannelsToSend = destination.isSingleChannel()
                                             ? numIndividualChannels
                                             : u16{};

                if (currentState.isActive())
                    sendResponse (Message::ProfileEnabledReport { profileAtAddress.profile, numChannelsToSend });
                else
                    sendResponse (Message::ProfileDisabledReport { profileAtAddress.profile, numChannelsToSend });

                host->isResponder = true;
                return true;
            }
        }

        detail::MessageTypeUtils::sendNAK (*output, {});
        return true;
    }

    b8 messageReceived (const Message::ProfileOn& request) const
    {
        return profileEnablementReceived (request);
    }

    b8 messageReceived (const Message::ProfileOff& request) const
    {
        return profileEnablementReceived (request);
    }

    ProfileHost* host = nullptr;
    ResponderOutput* output = nullptr;
    b8* handled = nullptr;
};

z0 ProfileHost::setProfileEnablement (ProfileAtAddress profileAtAddress, i32 numChannels)
{
    if (numChannels > 0)
        enableProfileImpl (profileAtAddress, numChannels);
    else
        disableProfileImpl (profileAtAddress);
}

z0 ProfileHost::addProfile (ProfileAtAddress profileAtAddress, i32 maxNumChannels)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr || state->get (profileAtAddress.profile).isSupported())
        return;

    // There are only 256 channels on a UMP endpoint, so requesting more probably doesn't make sense!
    jassert (maxNumChannels <= 256);

    state->set (profileAtAddress.profile, { (u16) jmax (1, maxNumChannels), 0 });

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileAdded>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileAdded { profileAtAddress.profile });
}

z0 ProfileHost::removeProfile (ProfileAtAddress profileAtAddress)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr)
        return;

    setProfileEnablement (profileAtAddress, 0);

    if (! state->get (profileAtAddress.profile).isSupported())
        return;

    state->erase (profileAtAddress.profile);

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileRemoved>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileRemoved { profileAtAddress.profile });
}

z0 ProfileHost::enableProfileImpl (ProfileAtAddress profileAtAddress, i32 numChannels)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr)
        return;

    const auto old = state->get (profileAtAddress.profile);

    if (! old.isSupported())
        return;

    // There are only 256 channels on a UMP endpoint, so requesting more probably doesn't make sense!
    jassert (numChannels <= 256);

    const auto enabledChannels = jmax ((u16) 1, jmin (old.supported, (u16) numChannels));
    state->set (profileAtAddress.profile, { old.supported, enabledChannels });

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileEnabledReport>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    const auto numChannelsToSend = profileAtAddress.address.isSingleChannel() ? enabledChannels : u16{};

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileEnabledReport { profileAtAddress.profile, numChannelsToSend });
}

z0 ProfileHost::disableProfileImpl (ProfileAtAddress profileAtAddress)
{
    auto* state = states.getStateForDestination (profileAtAddress.address);

    if (state == nullptr)
        return;

    const auto old = state->get (profileAtAddress.profile);

    if (! old.isActive())
        return;

    state->set (profileAtAddress.profile, { old.supported, 0 });

    if (! isResponder || profileAtAddress == currentEnablementMessage)
        return;

    const Message::Header header
    {
        profileAtAddress.address.getChannel(),
        detail::MessageMeta::Meta<Message::ProfileDisabledReport>::subID2,
        detail::MessageMeta::implementationVersion,
        output.getMuid(),
        MUID::getBroadcast(),
    };

    const auto numChannelsToSend = profileAtAddress.address.isSingleChannel() ? old.active : u16{};

    detail::MessageTypeUtils::send (output,
                                    profileAtAddress.address.getGroup(),
                                    header,
                                    Message::ProfileDisabledReport { profileAtAddress.profile, numChannelsToSend });
}

b8 ProfileHost::tryRespond (ResponderOutput& responderOutput, const Message::Parsed& message)
{
    b8 result = false;
    detail::MessageTypeUtils::visit (message, Visitor { this, &responderOutput, &result });
    return result;
}

} // namespace drx::midi_ci
