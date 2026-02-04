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

#ifndef DOXYGEN

namespace drx::universal_midi_packets
{

/**
    Translates a series of MIDI 1 Universal MIDI Packets to corresponding MIDI 2
    packets.

    @tags{Audio}
*/
class Midi1ToMidi2DefaultTranslator
{
public:
    Midi1ToMidi2DefaultTranslator() = default;

    /** Converts MIDI 1 Universal MIDI Packets to corresponding MIDI 2 packets,
        calling `callback` with each converted packet.

        In some cases (such as RPN/NRPN messages) multiple MIDI 1 packets will
        convert to a single MIDI 2 packet. In these cases, the translator will
        accumulate the full message internally, and send a single callback with
        the completed message, once all the individual MIDI 1 packets have been
        processed.
    */
    template <typename PacketCallback>
    z0 dispatch (const View& v, PacketCallback&& callback)
    {
        const auto firstWord = v[0];
        const auto messageType = Utils::getMessageType (firstWord);

        if (messageType != 0x2)
        {
            callback (v);
            return;
        }

        const HelperValues helperValues
        {
            std::byte ((0x4 << 0x4) | Utils::getGroup (firstWord)),
            std::byte ((firstWord >> 0x10) & 0xff),
            std::byte ((firstWord >> 0x08) & 0x7f),
            std::byte ((firstWord >> 0x00) & 0x7f),
        };

        switch (Utils::getStatus (firstWord))
        {
            case 0x8:
            case 0x9:
            {
                const auto packet = processNoteOnOrOff (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xa:
            {
                const auto packet = processPolyPressure (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xb:
            {
                PacketX2 packet;

                if (processControlChange (helperValues, packet))
                    callback (View (packet.data()));

                return;
            }

            case 0xc:
            {
                const auto packet = processProgramChange (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xd:
            {
                const auto packet = processChannelPressure (helperValues);
                callback (View (packet.data()));
                return;
            }

            case 0xe:
            {
                const auto packet = processPitchBend (helperValues);
                callback (View (packet.data()));
                return;
            }
        }
    }

    z0 reset()
    {
        groupAccumulators = {};
        groupBanks = {};
    }

private:
    enum class PnKind { nrpn, rpn };

    struct HelperValues
    {
        std::byte typeAndGroup;
        std::byte byte0;
        std::byte byte1;
        std::byte byte2;
    };

    static PacketX2 processNoteOnOrOff (HelperValues helpers);
    static PacketX2 processPolyPressure (HelperValues helpers);

    b8 processControlChange (HelperValues helpers, PacketX2& packet);

    PacketX2 processProgramChange (HelperValues helpers) const;

    static PacketX2 processChannelPressure (HelperValues helpers);
    static PacketX2 processPitchBend (HelperValues helpers);

    class PnAccumulator
    {
    public:
        b8 addByte (u8 cc, std::byte byte);

        const std::array<std::byte, 4>& getBytes() const noexcept { return bytes; }
        PnKind getKind() const noexcept { return kind; }

    private:
        std::array<std::byte, 4> bytes;
        u8 index = 0;
        PnKind kind = PnKind::nrpn;
    };

    class Bank
    {
    public:
        b8 isValid() const noexcept { return ! (msb & 0x80); }

        u8 getMsb() const noexcept { return msb & 0x7f; }
        u8 getLsb() const noexcept { return lsb & 0x7f; }

        z0 setMsb (u8 i) noexcept { msb = i & 0x7f; }
        z0 setLsb (u8 i) noexcept { msb &= 0x7f; lsb = i & 0x7f; }

    private:
        // We use the top bit to indicate whether this bank is valid.
        // After reading the spec, it's not clear how we should determine whether
        // there are valid values, so we'll just assume that the bank is valid
        // once either the lsb or msb have been written.
        u8 msb = 0x80;
        u8 lsb = 0x00;
    };

    using ChannelAccumulators = std::array<PnAccumulator, 16>;
    std::array<ChannelAccumulators, 16> groupAccumulators;

    using ChannelBanks = std::array<Bank, 16>;
    std::array<ChannelBanks, 16> groupBanks;
};

} // namespace drx::universal_midi_packets

#endif
