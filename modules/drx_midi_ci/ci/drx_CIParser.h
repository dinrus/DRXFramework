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
    Parses CI messages.

    @tags{Audio}
*/
class Parser
{
public:
    Parser() = delete;

    enum class Status
    {
        noError,                ///< Parsing was successful
        mismatchedMUID,         ///< The message destination MUID doesn't match the provided MUID
        collidingMUID,          ///< The message source MUID matches the provided MUID
        unrecognisedMessage,    ///< The message ID doesn't correspond to a known message
        reservedVersion,        ///< The MIDI CI version uses an unrecognised major version
        malformed,              ///< The message (whole message, or just body) could not be parsed
    };

    /** Parses the provided message;

        Call this with a full CI message. Don't include any "extra" bytes such as
        the leading/trailing 0xf0/0xf7 for messages that were originally in bytestream midi format,
        or the packet-header bytes from UMP-formatted sysex messages.

        Returns nullopt if the message doesn't need to be acknowledged by the entity with the provided MUID,
        or if the message is malformed.
        Otherwise, returns a parsed header, and optionally a body.
        If the body is std::monostate, then something went wrong while parsing. For example, the body
        may be malformed, or the CI version might be unrecognised.
    */
    static std::optional<Message::Parsed> parse (MUID ourMUID, Span<const std::byte> message, Status* = nullptr);

    /** Parses the provided message;

        Call this with a full CI message. Don't include any "extra" bytes such as
        the leading/trailing 0xf0/0xf7 for messages that were originally in bytestream midi format,
        or the packet-header bytes from UMP-formatted sysex messages.

        Returns nullopt if the message is malformed.
        Otherwise, returns a parsed header, and optionally a body.
        If the body is std::monostate, then something went wrong while parsing. For example, the body
        may be malformed, or the CI version might be unrecognised.
    */
    static std::optional<Message::Parsed> parse (Span<const std::byte> message, Status* = nullptr);

    /** Returns a human-readable string describing the message. */
    static Txt getMessageDescription (const Message::Parsed& message);
};

} // namespace drx::midi_ci
