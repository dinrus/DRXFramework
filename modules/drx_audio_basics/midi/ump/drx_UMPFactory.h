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
    This struct holds functions that can be used to create different kinds
    of Universal MIDI Packet.

    @tags{Audio}
*/
struct Factory
{
    /** @internal */
    struct Detail
    {
        static PacketX1 makeSystem()   { return PacketX1{}.withMessageType (1); }
        static PacketX1 makeV1()       { return PacketX1{}.withMessageType (2); }
        static PacketX2 makeV2()       { return PacketX2{}.withMessageType (4); }

        static PacketX2 makeSysEx (u8 group,
                                   u8 status,
                                   u8 numBytes,
                                   const std::byte* data)
        {
            jassert (numBytes <= 6);

            std::array<u8, 8> bytes{{}};
            bytes[0] = (0x3 << 0x4) | group;
            bytes[1] = (u8) (status << 0x4) | numBytes;

            std::memcpy (bytes.data() + 2, data, numBytes);

            std::array<u32, 2> words;

            for (const auto [index, word] : enumerate (words))
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index);

            return PacketX2 { words };
        }

        static PacketX4 makeSysEx8 (u8 group,
                                    u8 status,
                                    u8 numBytes,
                                    u8 dataStart,
                                    u8k* data)
        {
            jassert (numBytes <= 16 - dataStart);

            std::array<u8, 16> bytes{{}};
            bytes[0] = (0x5 << 0x4) | group;
            bytes[1] = (u8) (status << 0x4) | numBytes;

            std::memcpy (bytes.data() + dataStart, data, numBytes);

            std::array<u32, 4> words;

            for (const auto [index, word] : enumerate (words))
                word = ByteOrder::bigEndianInt (bytes.data() + 4 * index);

            return PacketX4 { words };
        }
    };

    static PacketX1 makeNoop (u8 group)
    {
        return PacketX1{}.withGroup (group);
    }

    static PacketX1 makeJRClock (u8 group, u16 time)
    {
        return PacketX1 { time }.withStatus (1).withGroup (group);
    }

    static PacketX1 makeJRTimestamp (u8 group, u16 time)
    {
        return PacketX1 { time }.withStatus (2).withGroup (group);
    }

    static PacketX1 makeTimeCode (u8 group, u8 code)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf1)
                                   .withU8<2> (code & 0x7f);
    }

    static PacketX1 makeSongPositionPointer (u8 group, u16 pos)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf2)
                                   .withU8<2> (pos & 0x7f)
                                   .withU8<3> ((pos >> 7) & 0x7f);
    }

    static PacketX1 makeSongSelect (u8 group, u8 song)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf3)
                                   .withU8<2> (song & 0x7f);
    }

    static PacketX1 makeTuneRequest (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf6);
    }

    static PacketX1 makeTimingClock (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xf8);
    }

    static PacketX1 makeStart (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfa);
    }

    static PacketX1 makeContinue (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfb);
    }

    static PacketX1 makeStop (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfc);
    }

    static PacketX1 makeActiveSensing (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xfe);
    }

    static PacketX1 makeReset (u8 group)
    {
        return Detail::makeSystem().withGroup (group)
                                   .withU8<1> (0xff);
    }

    static PacketX1 makeNoteOffV1 (u8 group,
                                   u8 channel,
                                   u8 note,
                                   u8 velocity)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0x8)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (velocity & 0x7f);
    }

    static PacketX1 makeNoteOnV1 (u8 group,
                                  u8 channel,
                                  u8 note,
                                  u8 velocity)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0x9)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (velocity & 0x7f);
    }

    static PacketX1 makePolyPressureV1 (u8 group,
                                        u8 channel,
                                        u8 note,
                                        u8 pressure)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xa)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (pressure & 0x7f);
    }

    static PacketX1 makeControlChangeV1 (u8 group,
                                         u8 channel,
                                         u8 controller,
                                         u8 value)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xb)
                               .withChannel (channel)
                               .withU8<2> (controller & 0x7f)
                               .withU8<3> (value & 0x7f);
    }

    static PacketX1 makeProgramChangeV1 (u8 group,
                                         u8 channel,
                                         u8 program)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xc)
                               .withChannel (channel)
                               .withU8<2> (program & 0x7f);
    }

    static PacketX1 makeChannelPressureV1 (u8 group,
                                           u8 channel,
                                           u8 pressure)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xd)
                               .withChannel (channel)
                               .withU8<2> (pressure & 0x7f);
    }

    static PacketX1 makePitchBend (u8 group,
                                   u8 channel,
                                   u16 pitchbend)
    {
        return Detail::makeV1().withGroup (group)
                               .withStatus (0xe)
                               .withChannel (channel)
                               .withU8<2> (pitchbend & 0x7f)
                               .withU8<3> ((pitchbend >> 7) & 0x7f);
    }

    static PacketX2 makeSysExIn1Packet (u8 group,
                                        u8 numBytes,
                                        const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x0, numBytes, data);
    }

    static PacketX2 makeSysExStart (u8 group,
                                    u8 numBytes,
                                    const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x1, numBytes, data);
    }

    static PacketX2 makeSysExContinue (u8 group,
                                       u8 numBytes,
                                       const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x2, numBytes, data);
    }

    static PacketX2 makeSysExEnd (u8 group,
                                  u8 numBytes,
                                  const std::byte* data)
    {
        return Detail::makeSysEx (group, 0x3, numBytes, data);
    }

    static PacketX2 makeRegisteredPerNoteControllerV2 (u8 group,
                                                       u8 channel,
                                                       u8 note,
                                                       u8 controller,
                                                       u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x0)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (controller & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeAssignablePerNoteControllerV2 (u8 group,
                                                       u8 channel,
                                                       u8 note,
                                                       u8 controller,
                                                       u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x1)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> (controller & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeRegisteredControllerV2 (u8 group,
                                                u8 channel,
                                                u8 bank,
                                                u8 index,
                                                u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x2)
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeAssignableControllerV2 (u8 group,
                                                u8 channel,
                                                u8 bank,
                                                u8 index,
                                                u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x3)
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeRelativeRegisteredControllerV2 (u8 group,
                                                        u8 channel,
                                                        u8 bank,
                                                        u8 index,
                                                        u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x4)
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeRelativeAssignableControllerV2 (u8 group,
                                                        u8 channel,
                                                        u8 bank,
                                                        u8 index,
                                                        u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x5)
                               .withChannel (channel)
                               .withU8<2> (bank & 0x7f)
                               .withU8<3> (index & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makePerNotePitchBendV2 (u8 group,
                                            u8 channel,
                                            u8 note,
                                            u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x6)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU32<1> (data);
    }

    enum class NoteAttributeKind : u8
    {
        none            = 0x00,
        manufacturer    = 0x01,
        profile         = 0x02,
        pitch7_9        = 0x03
    };

    static PacketX2 makeNoteOffV2 (u8 group,
                                   u8 channel,
                                   u8 note,
                                   NoteAttributeKind attribute,
                                   u16 velocity,
                                   u16 attributeValue)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x8)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> ((u8) attribute)
                               .withU16<2> (velocity)
                               .withU16<3> (attributeValue);
    }

    static PacketX2 makeNoteOnV2 (u8 group,
                                  u8 channel,
                                  u8 note,
                                  NoteAttributeKind attribute,
                                  u16 velocity,
                                  u16 attributeValue)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0x9)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU8<3> ((u8) attribute)
                               .withU16<2> (velocity)
                               .withU16<3> (attributeValue);
    }

    static PacketX2 makePolyPressureV2 (u8 group,
                                        u8 channel,
                                        u8 note,
                                        u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xa)
                               .withChannel (channel)
                               .withU8<2> (note & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeControlChangeV2 (u8 group,
                                         u8 channel,
                                         u8 controller,
                                         u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xb)
                               .withChannel (channel)
                               .withU8<2> (controller & 0x7f)
                               .withU32<1> (data);
    }

    static PacketX2 makeProgramChangeV2 (u8 group,
                                         u8 channel,
                                         u8 optionFlags,
                                         u8 program,
                                         u8 bankMsb,
                                         u8 bankLsb)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xc)
                               .withChannel (channel)
                               .withU8<3> (optionFlags)
                               .withU8<4> (program)
                               .withU8<6> (bankMsb)
                               .withU8<7> (bankLsb);
    }

    static PacketX2 makeChannelPressureV2 (u8 group,
                                           u8 channel,
                                           u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xd)
                               .withChannel (channel)
                               .withU32<1> (data);
    }

    static PacketX2 makePitchBendV2 (u8 group,
                                     u8 channel,
                                     u32 data)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xe)
                               .withChannel (channel)
                               .withU32<1> (data);
    }

    static PacketX2 makePerNoteManagementV2 (u8 group,
                                             u8 channel,
                                             u8 note,
                                             u8 optionFlags)
    {
        return Detail::makeV2().withGroup (group)
                               .withStatus (0xf)
                               .withChannel (channel)
                               .withU8<2> (note)
                               .withU8<3> (optionFlags);
    }


    static PacketX4 makeSysEx8in1Packet (u8 group,
                                         u8 numBytes,
                                         u8 streamId,
                                         u8k* data)
    {
        return Detail::makeSysEx8 (group, 0x0, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8Start (u8 group,
                                     u8 numBytes,
                                     u8 streamId,
                                     u8k* data)
    {
        return Detail::makeSysEx8 (group, 0x1, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8Continue (u8 group,
                                        u8 numBytes,
                                        u8 streamId,
                                        u8k* data)
    {
        return Detail::makeSysEx8 (group, 0x2, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeSysEx8End (u8 group,
                                   u8 numBytes,
                                   u8 streamId,
                                   u8k* data)
    {
        return Detail::makeSysEx8 (group, 0x3, numBytes, 3, data).withU8<2> (streamId);
    }

    static PacketX4 makeMixedDataSetHeader (u8 group,
                                            u8 dataSetId,
                                            u8k* data)
    {
        return Detail::makeSysEx8 (group, 0x8, 14, 2, data).withChannel (dataSetId);
    }

    static PacketX4 makeDataSetPayload (u8 group,
                                        u8 dataSetId,
                                        u8k* data)
    {
        return Detail::makeSysEx8 (group, 0x9, 14, 2, data).withChannel (dataSetId);
    }
};

} // namespace drx::universal_midi_packets

#endif
