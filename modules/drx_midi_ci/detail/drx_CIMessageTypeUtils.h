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

namespace drx::midi_ci::detail::MessageTypeUtils
{

//==============================================================================
/*
    An interface used for types that want to operate on parsed MIDI-CI messages.

    @tags{Audio}
*/
struct MessageVisitor
{
    MessageVisitor() = default;
    MessageVisitor (const MessageVisitor&) = default;
    MessageVisitor (MessageVisitor&&) = default;
    MessageVisitor& operator= (const MessageVisitor&) = default;
    MessageVisitor& operator= (MessageVisitor&&) = default;
    virtual ~MessageVisitor() = default;

    virtual z0 visit (const std::monostate&)                                  const {}
    virtual z0 visit (const Message::Discovery&)                              const {}
    virtual z0 visit (const Message::EndpointInquiry&)                        const {}
    virtual z0 visit (const Message::ProfileInquiry&)                         const {}
    virtual z0 visit (const Message::ProfileDetails&)                         const {}
    virtual z0 visit (const Message::PropertyExchangeCapabilities&)           const {}
    virtual z0 visit (const Message::PropertyGetData&)                        const {}
    virtual z0 visit (const Message::PropertySetData&)                        const {}
    virtual z0 visit (const Message::PropertySubscribe&)                      const {}
    virtual z0 visit (const Message::ProcessInquiry&)                         const {}
    virtual z0 visit (const Message::ProcessMidiMessageReport&)               const {}
    virtual z0 visit (const Message::DiscoveryResponse&)                      const {}
    virtual z0 visit (const Message::EndpointInquiryResponse&)                const {}
    virtual z0 visit (const Message::InvalidateMUID&)                         const {}
    virtual z0 visit (const Message::ACK&)                                    const {}
    virtual z0 visit (const Message::NAK&)                                    const {}
    virtual z0 visit (const Message::ProfileInquiryResponse&)                 const {}
    virtual z0 visit (const Message::ProfileAdded&)                           const {}
    virtual z0 visit (const Message::ProfileRemoved&)                         const {}
    virtual z0 visit (const Message::ProfileDetailsResponse&)                 const {}
    virtual z0 visit (const Message::ProfileOn&)                              const {}
    virtual z0 visit (const Message::ProfileOff&)                             const {}
    virtual z0 visit (const Message::ProfileEnabledReport&)                   const {}
    virtual z0 visit (const Message::ProfileDisabledReport&)                  const {}
    virtual z0 visit (const Message::ProfileSpecificData&)                    const {}
    virtual z0 visit (const Message::PropertyExchangeCapabilitiesResponse&)   const {}
    virtual z0 visit (const Message::PropertyGetDataResponse&)                const {}
    virtual z0 visit (const Message::PropertySetDataResponse&)                const {}
    virtual z0 visit (const Message::PropertySubscribeResponse&)              const {}
    virtual z0 visit (const Message::PropertyNotify&)                         const {}
    virtual z0 visit (const Message::ProcessInquiryResponse&)                 const {}
    virtual z0 visit (const Message::ProcessMidiMessageReportResponse&)       const {}
    virtual z0 visit (const Message::ProcessEndMidiMessageReport&)            const {}
};

using ParseFn = Message::Parsed::Body (*) (Message::Generic, Parser::Status* status);
using VisitFn = z0 (*) (const Message::Parsed&, const MessageVisitor&);

/*  These return the Universal System Exclusive Sub-ID#2 for a particular message type. */
template <typename Specific>
static constexpr auto getParserFor (std::in_place_type_t<Specific>)
{
    return [] (Message::Generic message, Parser::Status* status) -> Message::Parsed::Body
    {
        // Parse messages using the version specified in the header of the message
        if (Specific parsed; Marshalling::Reader { message.data, static_cast<u8> (message.header.version) } (parsed))
        {
            return parsed;
        }

        if (status != nullptr)
            *status = Parser::Status::malformed;

        return std::monostate{};
    };
}

template <typename Specific>
static constexpr auto getVisitorFor (std::in_place_type_t<Specific>)
{
    return [] (const Message::Parsed& parsed, const MessageVisitor& visitor)
    {
        if (auto* body = std::get_if<Specific> (&parsed.body))
            visitor.visit (*body);
    };
}

template <typename... Ts>
struct LookupTables
{
    constexpr LookupTables()
    {
        for (auto& x : parsers)
        {
            x = [] (Message::Generic, Parser::Status* status) -> Message::Parsed::Body
            {
                if (status != nullptr)
                    *status = Parser::Status::unrecognisedMessage;

                return std::monostate{};
            };
        }

        for (auto& x : visitors)
        {
            x = [] (const Message::Parsed&, const MessageVisitor& visitor)
            {
                visitor.visit (std::monostate{});
            };
        }

        (registerTag (std::in_place_type<Ts>), ...);
    }

    template <typename T>
    constexpr z0 registerTag (std::in_place_type_t<T> tag)
    {
        constexpr auto category = MessageMeta::Meta<T>::subID2;
        parsers[u8 (category)] = getParserFor (tag);
        visitors[u8 (category)] = getVisitorFor (tag);
    }

    ParseFn parsers[std::numeric_limits<u8>::max()]{};
    VisitFn visitors[std::numeric_limits<u8>::max()]{};
};

template <typename Body>
static z0 send (BufferOutput& output, u8 group, const Message::Header& header, const Body& body)
{
    output.getOutputBuffer().clear();
    Marshalling::Writer { output.getOutputBuffer() } (header, body);
    output.send (group);
}

template <typename Body>
static z0 send (BufferOutput& output, u8 group, MUID targetMuid, ChannelInGroup cig, const Body& body)
{
    Message::Header header
    {
        cig,
        MessageMeta::Meta<Body>::subID2,
        MessageMeta::implementationVersion,
        output.getMuid(),
        targetMuid,
    };

    send (output, group, header, body);
}


template <typename Body>
static z0 send (ResponderOutput& output, const Body& body)
{
    send (output, output.getIncomingGroup(), output.getReplyHeader (MessageMeta::Meta<Body>::subID2), body);
}

static z0 sendNAK (ResponderOutput& output, std::byte statusCode)
{
    const auto header = output.getReplyHeader (MessageMeta::Meta<Message::NAK>::subID2);
    const Message::NAK body { output.getIncomingHeader().category,
                              statusCode,
                              std::byte { 0x00 },
                              {},   // No additional details
                              {} }; // No message text
    send (output, output.getIncomingGroup(), header, body);
}

class BaseCaseDelegate : public ResponderDelegate
{
public:
    b8 tryRespond (ResponderOutput& output, const Message::Parsed&) override
    {
        sendNAK (output, {});
        return true;
    }
};

static constexpr auto getTables()
{
    return LookupTables<Message::Discovery,
                        Message::DiscoveryResponse,
                        Message::InvalidateMUID,
                        Message::EndpointInquiry,
                        Message::EndpointInquiryResponse,
                        Message::ACK,
                        Message::NAK,
                        Message::ProfileInquiry,
                        Message::ProfileInquiryResponse,
                        Message::ProfileAdded,
                        Message::ProfileRemoved,
                        Message::ProfileDetails,
                        Message::ProfileDetailsResponse,
                        Message::ProfileOn,
                        Message::ProfileOff,
                        Message::ProfileEnabledReport,
                        Message::ProfileDisabledReport,
                        Message::ProfileSpecificData,
                        Message::PropertyExchangeCapabilities,
                        Message::PropertyExchangeCapabilitiesResponse,
                        Message::PropertyGetData,
                        Message::PropertyGetDataResponse,
                        Message::PropertySetData,
                        Message::PropertySetDataResponse,
                        Message::PropertySubscribe,
                        Message::PropertySubscribeResponse,
                        Message::PropertyNotify,
                        Message::ProcessInquiry,
                        Message::ProcessInquiryResponse,
                        Message::ProcessMidiMessageReport,
                        Message::ProcessMidiMessageReportResponse,
                        Message::ProcessEndMidiMessageReport>{};

}

static z0 visit (const Message::Parsed& msg, const MessageVisitor& visitor)
{
    constexpr auto tables = getTables();
    const auto fn = tables.visitors[(u8) msg.header.category];
    fn (msg, visitor);
}

} // namespace drx::midi_ci::detail::MessageTypeUtils
