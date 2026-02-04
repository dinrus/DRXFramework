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

//==============================================================================
/**
    Byte values representing different addresses within a group.

    @tags{Audio}
*/
enum class ChannelInGroup : u8
{
    channel0 = 0x0,
    channel1 = 0x1,
    channel2 = 0x2,
    channel3 = 0x3,
    channel4 = 0x4,
    channel5 = 0x5,
    channel6 = 0x6,
    channel7 = 0x7,
    channel8 = 0x8,
    channel9 = 0x9,
    channelA = 0xA,
    channelB = 0xB,
    channelC = 0xC,
    channelD = 0xD,
    channelE = 0xE,
    channelF = 0xF,
    wholeGroup = 0x7e, ///< Refers to all channels in the UMP group
    wholeBlock = 0x7f, ///< Refers to all channels in the function block that contains the UMP group
};

/**
    Utility functions for working with the ChannelInGroup enum.

    @tags{Audio}
*/
struct ChannelInGroupUtils
{
    ChannelInGroupUtils() = delete;

    /** Converts a ChannelInGroup to a descriptive string. */
    static Txt toString (ChannelInGroup c)
    {
        if (c == ChannelInGroup::wholeGroup)
            return "Group";

        if (c == ChannelInGroup::wholeBlock)
            return "Function Block";

        const auto underlying = (std::underlying_type_t<ChannelInGroup>) c;
        return "Channel " + Txt (underlying + 1);
    }
};

using Profile = std::array<std::byte, 5>;

//==============================================================================
/**
    Namespace containing structs representing different kinds of MIDI-CI message.

    @tags{Audio}
*/
namespace Message
{
    /** Wraps a span, providing equality operators that compare the span
        contents elementwise.

        @tags{Audio}
    */
    template <typename T>
    struct ComparableRange
    {
        T& data;

        b8 operator== (const ComparableRange& other) const
        {
            return std::equal (data.begin(), data.end(), other.data.begin(), other.data.end());
        }

        b8 operator!= (const ComparableRange& other) const { return ! operator== (other); }
    };

    template <typename T> static constexpr auto makeComparableRange (      T& t) { return ComparableRange<      T> { t }; }
    template <typename T> static constexpr auto makeComparableRange (const T& t) { return ComparableRange<const T> { t }; }

    //==============================================================================
    /**
        Holds fields that can be found at the beginning of every MIDI CI message.

        @tags{Audio}
    */
    struct Header
    {
        ChannelInGroup deviceID{};
        std::byte category{};
        std::byte version{};
        MUID source = MUID::makeUnchecked (0);
        MUID destination = MUID::makeUnchecked (0);

        auto tie() const
        {
            return std::tuple (deviceID, category, version, source, destination);
        }

        b8 operator== (const Header& x) const { return tie() == x.tie(); }
        b8 operator!= (const Header& x) const { return ! operator== (x); }
    };

    /**
        Groups together a CI message header, and some number of trailing bytes.

        @tags{Audio}
    */
    struct Generic
    {
        Header header;
        Span<const std::byte> data;
    };

    //==============================================================================
    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct DiscoveryResponse
    {
        ump::DeviceInfo device;
        std::byte capabilities{};
        u32 maximumSysexSize{};
        std::byte outputPathID{};       /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte functionBlock{};      /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (device, capabilities, maximumSysexSize, outputPathID, functionBlock);
        }

        b8 operator== (const DiscoveryResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const DiscoveryResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct Discovery
    {
        ump::DeviceInfo device;
        std::byte capabilities{};
        u32 maximumSysexSize{};
        std::byte outputPathID{};       /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (device, capabilities, maximumSysexSize, outputPathID);
        }

        b8 operator== (const Discovery& x) const { return tie() == x.tie(); }
        b8 operator!= (const Discovery& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct EndpointInquiryResponse
    {
        std::byte status;
        Span<const std::byte> data;

        auto tie() const
        {
            return std::tuple (status, makeComparableRange (data));
        }

        b8 operator== (const EndpointInquiryResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const EndpointInquiryResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct EndpointInquiry
    {
        std::byte status;

        auto tie() const
        {
            return std::tuple (status);
        }

        b8 operator== (const EndpointInquiry& x) const { return tie() == x.tie(); }
        b8 operator!= (const EndpointInquiry& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct InvalidateMUID
    {
        MUID target = MUID::makeUnchecked (0);

        auto tie() const
        {
            return std::tuple (target);
        }

        b8 operator== (const InvalidateMUID& x) const { return tie() == x.tie(); }
        b8 operator!= (const InvalidateMUID& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ACK
    {
        std::byte originalCategory{};
        std::byte statusCode{};
        std::byte statusData{};
        std::array<std::byte, 5> details{};
        Span<const std::byte> messageText{};

        /** Convenience function that returns the message's text as a Txt. */
        Txt getMessageTextAsString() const
        {
            return Encodings::stringFrom7BitText (messageText);
        }

        auto tie() const
        {
            return std::tuple (originalCategory, statusCode, statusData, details, makeComparableRange (messageText));
        }

        b8 operator== (const ACK& x) const { return tie() == x.tie(); }
        b8 operator!= (const ACK& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct NAK
    {
        std::byte originalCategory{};        /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte statusCode{};              /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte statusData{};              /**< Only valid if the message header specifies version 0x02 or greater. */
        std::array<std::byte, 5> details{};  /**< Only valid if the message header specifies version 0x02 or greater. */
        Span<const std::byte> messageText{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        /** Convenience function that returns the message's text as a Txt. */
        Txt getMessageTextAsString() const
        {
            return Encodings::stringFrom7BitText (messageText);
        }

        auto tie() const
        {
            return std::tuple (originalCategory, statusCode, statusData, details, makeComparableRange (messageText));
        }

        b8 operator== (const NAK& x) const { return tie() == x.tie(); }
        b8 operator!= (const NAK& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileInquiryResponse
    {
        Span<const Profile> enabledProfiles;
        Span<const Profile> disabledProfiles;

        auto tie() const
        {
            return std::tuple (makeComparableRange (enabledProfiles), makeComparableRange (disabledProfiles));
        }

        b8 operator== (const ProfileInquiryResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileInquiryResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileInquiry
    {
        auto tie() const
        {
            return std::tuple<>();
        }

        b8 operator== (const ProfileInquiry& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileInquiry& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileAdded
    {
        Profile profile{};

        auto tie() const
        {
            return std::tuple (profile);
        }

        b8 operator== (const ProfileAdded& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileAdded& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileRemoved
    {
        Profile profile{};

        auto tie() const
        {
            return std::tuple (profile);
        }

        b8 operator== (const ProfileRemoved& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileRemoved& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileDetailsResponse
    {
        Profile profile{};
        std::byte target{};
        Span<const std::byte> data;

        auto tie() const
        {
            return std::tuple (profile, target, makeComparableRange (data));
        }

        b8 operator== (const ProfileDetailsResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileDetailsResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileDetails
    {
        Profile profile{};
        std::byte target{};

        auto tie() const
        {
            return std::tuple (profile, target);
        }

        b8 operator== (const ProfileDetails& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileDetails& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileOn
    {
        Profile profile{};
        u16 numChannels{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (profile, numChannels);
        }

        b8 operator== (const ProfileOn& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileOn& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileOff
    {
        Profile profile{};

        auto tie() const
        {
            return std::tuple (profile);
        }

        b8 operator== (const ProfileOff& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileOff& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileEnabledReport
    {
        Profile profile{};
        u16 numChannels{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (profile, numChannels);
        }

        b8 operator== (const ProfileEnabledReport& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileEnabledReport& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileDisabledReport
    {
        Profile profile{};
        u16 numChannels{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (profile, numChannels);
        }

        b8 operator== (const ProfileDisabledReport& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileDisabledReport& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProfileSpecificData
    {
        Profile profile{};
        Span<const std::byte> data;

        auto tie() const
        {
            return std::tuple (profile, makeComparableRange (data));
        }

        b8 operator== (const ProfileSpecificData& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProfileSpecificData& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyExchangeCapabilitiesResponse
    {
        std::byte numSimultaneousRequestsSupported{};
        std::byte majorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte minorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (numSimultaneousRequestsSupported, majorVersion, minorVersion);
        }

        b8 operator== (const PropertyExchangeCapabilitiesResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertyExchangeCapabilitiesResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyExchangeCapabilities
    {
        std::byte numSimultaneousRequestsSupported{};
        std::byte majorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */
        std::byte minorVersion{}; /**< Only valid if the message header specifies version 0x02 or greater. */

        auto tie() const
        {
            return std::tuple (numSimultaneousRequestsSupported, majorVersion, minorVersion);
        }

        b8 operator== (const PropertyExchangeCapabilities& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertyExchangeCapabilities& x) const { return ! operator== (x); }
    };

    /** A property-exchange message that has no payload, and must therefore
        be contained in a single chunk.

        @tags{Audio}
    */
    struct StaticSizePropertyExchange
    {
        std::byte requestID{};
        Span<const std::byte> header;

        auto tie() const
        {
            return std::tuple (requestID, makeComparableRange (header));
        }
    };

    /** A property-exchange message that may form part of a multi-chunk
        message sequence.

        @tags{Audio}
    */
    struct DynamicSizePropertyExchange
    {
        std::byte requestID{};
        Span<const std::byte> header;
        u16 totalNumChunks{};
        u16 thisChunkNum{};
        Span<const std::byte> data;

        auto tie() const
        {
            return std::tuple (requestID,
                               makeComparableRange (header),
                               totalNumChunks,
                               thisChunkNum,
                               makeComparableRange (data));
        }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyGetDataResponse : public DynamicSizePropertyExchange
    {
        b8 operator== (const PropertyGetDataResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertyGetDataResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyGetData : public StaticSizePropertyExchange
    {
        b8 operator== (const PropertyGetData& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertyGetData& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySetDataResponse : public StaticSizePropertyExchange
    {
        b8 operator== (const PropertySetDataResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertySetDataResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySetData : public DynamicSizePropertyExchange
    {
        b8 operator== (const PropertySetData& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertySetData& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySubscribeResponse : public DynamicSizePropertyExchange
    {
        b8 operator== (const PropertySubscribeResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertySubscribeResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertySubscribe : public DynamicSizePropertyExchange
    {
        b8 operator== (const PropertySubscribe& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertySubscribe& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct PropertyNotify : public DynamicSizePropertyExchange
    {
        b8 operator== (const PropertyNotify& x) const { return tie() == x.tie(); }
        b8 operator!= (const PropertyNotify& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessInquiryResponse
    {
        std::byte supportedFeatures{};

        auto tie() const
        {
            return std::tuple (supportedFeatures);
        }

        b8 operator== (const ProcessInquiryResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProcessInquiryResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessInquiry
    {
        auto tie() const
        {
            return std::tuple<>();
        }

        b8 operator== (const ProcessInquiry& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProcessInquiry& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessMidiMessageReportResponse
    {
        std::byte messageDataControl{};
        std::byte requestedMessages{};
        std::byte channelControllerMessages{};
        std::byte noteDataMessages{};

        auto tie() const
        {
            return std::tuple (messageDataControl, requestedMessages, channelControllerMessages, noteDataMessages);
        }

        b8 operator== (const ProcessMidiMessageReportResponse& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProcessMidiMessageReportResponse& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessMidiMessageReport
    {
        std::byte messageDataControl{};
        std::byte requestedMessages{};
        std::byte channelControllerMessages{};
        std::byte noteDataMessages{};

        auto tie() const
        {
            return std::tuple (messageDataControl, requestedMessages, channelControllerMessages, noteDataMessages);
        }

        b8 operator== (const ProcessMidiMessageReport& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProcessMidiMessageReport& x) const { return ! operator== (x); }
    };

    /** See the MIDI-CI specification.

        @tags{Audio}
    */
    struct ProcessEndMidiMessageReport
    {
        auto tie() const
        {
            return std::tuple<>();
        }

        b8 operator== (const ProcessEndMidiMessageReport& x) const { return tie() == x.tie(); }
        b8 operator!= (const ProcessEndMidiMessageReport& x) const { return ! operator== (x); }
    };

    /**
        A message with a header and optional body.

        The body may be set to std::monostate to indicate some kind of failure, such as a malformed
        incoming message.

        @tags{Audio}
    */
    struct Parsed
    {
        using Body = std::variant<std::monostate,
                                  Discovery,
                                  DiscoveryResponse,
                                  InvalidateMUID,
                                  EndpointInquiry,
                                  EndpointInquiryResponse,
                                  ACK,
                                  NAK,
                                  ProfileInquiry,
                                  ProfileInquiryResponse,
                                  ProfileAdded,
                                  ProfileRemoved,
                                  ProfileDetails,
                                  ProfileDetailsResponse,
                                  ProfileOn,
                                  ProfileOff,
                                  ProfileEnabledReport,
                                  ProfileDisabledReport,
                                  ProfileSpecificData,
                                  PropertyExchangeCapabilities,
                                  PropertyExchangeCapabilitiesResponse,
                                  PropertyGetData,
                                  PropertyGetDataResponse,
                                  PropertySetData,
                                  PropertySetDataResponse,
                                  PropertySubscribe,
                                  PropertySubscribeResponse,
                                  PropertyNotify,
                                  ProcessInquiry,
                                  ProcessInquiryResponse,
                                  ProcessMidiMessageReport,
                                  ProcessMidiMessageReportResponse,
                                  ProcessEndMidiMessageReport>;

        Header header;
        Body body;

        b8 operator== (const Parsed& other) const
        {
            const auto tie = [] (const auto& x) { return std::tie (x.header, x.body); };
            return tie (*this) == tie (other);
        }

        b8 operator!= (const Parsed& other) const { return ! operator== (other); }
    };
}

} // namespace drx::midi_ci
