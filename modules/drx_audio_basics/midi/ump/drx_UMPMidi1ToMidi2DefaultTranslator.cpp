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

namespace drx::universal_midi_packets
{

PacketX2 Midi1ToMidi2DefaultTranslator::processNoteOnOrOff (const HelperValues helpers)
{
    const auto velocity = helpers.byte2;
    const auto needsConversion = (helpers.byte0 & std::byte { 0xf0 }) == std::byte { 0x90 } && velocity == std::byte { 0 };
    const auto firstByte = needsConversion ? (std::byte { 0x80 } | (helpers.byte0 & std::byte { 0xf }))
                                           : helpers.byte0;

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, firstByte, helpers.byte1, std::byte { 0 }),
        (u32) (Conversion::scaleTo16 (u8 (velocity)) << 0x10)
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processPolyPressure (const HelperValues helpers)
{
    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, helpers.byte1, std::byte { 0 }),
        Conversion::scaleTo32 (u8 (helpers.byte2))
    };
}

b8 Midi1ToMidi2DefaultTranslator::processControlChange (const HelperValues helpers,
                                                          PacketX2& packet)
{
    const auto statusAndChannel = helpers.byte0;
    const auto cc               = u8 (helpers.byte1);

    const auto shouldAccumulate = [&]
    {
        switch (cc)
        {
            case 6:
            case 38:
            case 98:
            case 99:
            case 100:
            case 101:
                return true;
        }

        return false;
    }();

    const auto group   = (u8) (helpers.typeAndGroup & std::byte { 0xf });
    const auto channel = (u8) (statusAndChannel & std::byte { 0xf });
    const auto byte    = helpers.byte2;

    if (shouldAccumulate)
    {
        auto& accumulator = groupAccumulators[group][channel];

        if (accumulator.addByte (cc, byte))
        {
            const auto& bytes = accumulator.getBytes();
            const auto bank   = bytes[0];
            const auto index  = bytes[1];
            const auto msb    = bytes[2];
            const auto lsb    = bytes[3];

            const auto value = u16 ((u16 (msb & std::byte { 0x7f }) << 7) | u16 (lsb & std::byte { 0x7f }));

            const auto newStatus = (u8) (accumulator.getKind() == PnKind::nrpn ? 0x3 : 0x2);

            packet = PacketX2
            {
                Utils::bytesToWord (helpers.typeAndGroup, std::byte ((newStatus << 0x4) | channel), bank, index),
                Conversion::scaleTo32 (value)
            };
            return true;
        }

        return false;
    }

    if (cc == 0)
    {
        groupBanks[group][channel].setMsb (u8 (byte));
        return false;
    }

    if (cc == 32)
    {
        groupBanks[group][channel].setLsb (u8 (byte));
        return false;
    }

    packet = PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, statusAndChannel, std::byte { cc }, std::byte { 0 }),
        Conversion::scaleTo32 (u8 (helpers.byte2))
    };
    return true;
}

PacketX2 Midi1ToMidi2DefaultTranslator::processProgramChange (const HelperValues helpers) const
{
    const auto group   = (u8) (helpers.typeAndGroup & std::byte { 0xf });
    const auto channel = (u8) (helpers.byte0 & std::byte { 0xf });
    const auto bank    = groupBanks[group][channel];
    const auto valid   = bank.isValid();

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup,
                            helpers.byte0,
                            std::byte { 0 },
                            valid ? std::byte { 1 } : std::byte { 0 }),
        Utils::bytesToWord (helpers.byte1,
                            std::byte { 0 },
                            valid ? std::byte { bank.getMsb() } : std::byte { 0 },
                            valid ? std::byte { bank.getLsb() } : std::byte { 0 })
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processChannelPressure (const HelperValues helpers)
{
    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, std::byte { 0 }, std::byte { 0 }),
        Conversion::scaleTo32 (u8 (helpers.byte1))
    };
}

PacketX2 Midi1ToMidi2DefaultTranslator::processPitchBend (const HelperValues helpers)
{
    const auto lsb   = helpers.byte1;
    const auto msb   = helpers.byte2;
    const auto value = u16 (u16 (msb) << 7 | u16 (lsb));

    return PacketX2
    {
        Utils::bytesToWord (helpers.typeAndGroup, helpers.byte0, std::byte { 0 }, std::byte { 0 }),
        Conversion::scaleTo32 (value)
    };
}

b8 Midi1ToMidi2DefaultTranslator::PnAccumulator::addByte (u8 cc, std::byte byte)
{
    const auto isStart = cc == 99 || cc == 101;

    if (isStart)
    {
        kind = cc == 99 ? PnKind::nrpn : PnKind::rpn;
        index = 0;
    }

    bytes[index] = byte;

    const auto shouldContinue = [&]
    {
        switch (index)
        {
            case 0: return isStart;
            case 1: return kind == PnKind::nrpn ? cc == 98 : cc == 100;
            case 2: return cc == 6;
            case 3: return cc == 38;
        }

        return false;
    }();

    index = shouldContinue ? index + 1 : 0;

    if (index != bytes.size())
        return false;

    index = 0;
    return true;
}

} // namespace drx::universal_midi_packets
