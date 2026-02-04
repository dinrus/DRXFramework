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
    Utility functions for working with data formats used by property exchange
    messages.

    @tags{Audio}
*/
struct Encodings
{
    /** Text in ACK and NAK messages can't be utf-8 or ASCII because each byte only has 7 usable bits.
        The encoding rules are in section 5.10.4 of the CI spec.
    */
    static Txt stringFrom7BitText (Span<const std::byte> bytes);

    /** Text in ACK and NAK messages can't be utf-8 or ASCII because each byte only has 7 usable bits.
        The encoding rules are in section 5.10.4 of the CI spec.
    */
    static std::vector<std::byte> stringTo7BitText (const Txt& text);

    /** Converts a list of bytes representing a 7-bit ASCII string to JSON. */
    static var jsonFrom7BitText (Span<const std::byte> bytes)
    {
        return JSON::parse (stringFrom7BitText (bytes));
    }

    /** Converts a JSON object to a list of bytes in 7-bit ASCII format. */
    static std::vector<std::byte> jsonTo7BitText (const var& v)
    {
        return stringTo7BitText (JSON::toString (v, JSON::FormatOptions{}.withSpacing (JSON::Spacing::none)));
    }

    /** Each group of seven stored bytes is transmitted as eight bytes.
        First, the sign bits of the seven bytes are sent, followed by the low-order 7 bits of each byte.
    */
    static std::vector<std::byte> toMcoded7 (Span<const std::byte> bytes);

    /** Each group of seven stored bytes is transmitted as eight bytes.
        First, the sign bits of the seven bytes are sent, followed by the low-order 7 bits of each byte.
    */
    static std::vector<std::byte> fromMcoded7 (Span<const std::byte> bytes);

    /** Attempts to encode the provided byte span using the specified encoding.

        The ASCII encoding does not make any changes to the input stream, but
        encoding will fail if any byte has its most significant bit set.
    */
    static std::optional<std::vector<std::byte>> tryEncode (Span<const std::byte> bytes,
                                                            Encoding mutualEncoding);

    /** Decodes the provided byte span using the specified encoding.

        All bytes of the input must be 7-bit values, i.e. all most-significant bits
        are unset.
    */
    static std::vector<std::byte> decode (Span<const std::byte> bytes, Encoding mutualEncoding);

    Encodings() = delete;
};

} // namespace drx::midi_ci
