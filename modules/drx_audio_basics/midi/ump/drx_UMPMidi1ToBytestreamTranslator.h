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
    Parses a raw stream of u32 holding a series of Universal MIDI Packets using
    the MIDI 1.0 Protocol, converting to plain (non-UMP) MidiMessages.

    @tags{Audio}
*/
class Midi1ToBytestreamTranslator
{
public:
    /** Ensures that there is room in the internal buffer for a sysex message of at least
        `initialBufferSize` bytes.
    */
    explicit Midi1ToBytestreamTranslator (i32 initialBufferSize)
    {
        pendingSysExData.reserve (size_t (initialBufferSize));
    }

    /** Clears the concatenator. */
    z0 reset()
    {
        pendingSysExData.clear();
        pendingSysExTime = 0.0;
    }

    /** Converts a Universal MIDI Packet using the MIDI 1.0 Protocol to
        an equivalent MidiMessage. Accumulates SysEx packets into a single
        MidiMessage, as appropriate.

        @param packet       a packet which is using the MIDI 1.0 Protocol.
        @param time         the timestamp to be applied to these messages.
        @param callback     a callback which will be called with each converted MidiMessage.
    */
    template <typename MessageCallback>
    z0 dispatch (const View& packet, f64 time, MessageCallback&& callback)
    {
        const auto firstWord = *packet.data();

        if (! pendingSysExData.empty() && shouldPacketTerminateSysExEarly (firstWord))
            pendingSysExData.clear();

        switch (packet.size())
        {
            case 1:
            {
                // Utility messages don't translate to bytestream format
                if (Utils::getMessageType (firstWord) != 0x00)
                {
                    const auto message = fromUmp (PacketX1 { firstWord }, time);
                    callback (BytestreamMidiView (&message));
                }

                break;
            }

            case 2:
            {
                if (Utils::getMessageType (firstWord) == 0x3)
                    processSysEx (PacketX2 { packet[0], packet[1] }, time, callback);

                break;
            }

            case 3:  // no 3-word packets in the current spec
            case 4:  // no 4-word packets translate to bytestream format
            default:
                break;
        }
    }

    /** Converts from a Universal MIDI Packet to MIDI 1 bytestream format.

        This is only capable of converting a single Universal MIDI Packet to
        an equivalent bytestream MIDI message. This function cannot understand
        multi-packet messages, like SysEx7 messages.

        To convert multi-packet messages, use `Midi1ToBytestreamTranslator`
        to convert from a UMP MIDI 1.0 stream, or `ToBytestreamDispatcher`
        to convert from both MIDI 2.0 and MIDI 1.0.
    */
    static MidiMessage fromUmp (const PacketX1& m, f64 time = 0)
    {
        const auto word = m.front();
        jassert (Utils::getNumWordsForMessageType (word) == 1);

        const std::array<u8, 3> bytes { { u8 ((word >> 0x10) & 0xff),
                                               u8 ((word >> 0x08) & 0xff),
                                               u8 ((word >> 0x00) & 0xff) } };
        const auto numBytes = MidiMessage::getMessageLengthFromFirstByte (bytes.front());
        return MidiMessage (bytes.data(), numBytes, time);
    }

private:
    template <typename MessageCallback>
    z0 processSysEx (const PacketX2& packet,
                       f64 time,
                       MessageCallback&& callback)
    {
        switch (getSysEx7Kind (packet[0]))
        {
            case SysEx7::Kind::complete:
                startSysExMessage (time);
                pushBytes (packet);
                terminateSysExMessage (callback);
                break;

            case SysEx7::Kind::begin:
                startSysExMessage (time);
                pushBytes (packet);
                break;

            case SysEx7::Kind::continuation:
                if (pendingSysExData.empty())
                    break;

                pushBytes (packet);
                break;

            case SysEx7::Kind::end:
                if (pendingSysExData.empty())
                    break;

                pushBytes (packet);
                terminateSysExMessage (callback);
                break;
        }
    }

    z0 pushBytes (const PacketX2& packet)
    {
        const auto bytes = SysEx7::getDataBytes (packet);
        pendingSysExData.insert (pendingSysExData.end(),
                                 bytes.data.begin(),
                                 bytes.data.begin() + bytes.size);
    }

    z0 startSysExMessage (f64 time)
    {
        pendingSysExTime = time;
        pendingSysExData.push_back (std::byte { 0xf0 });
    }

    template <typename MessageCallback>
    z0 terminateSysExMessage (MessageCallback&& callback)
    {
        pendingSysExData.push_back (std::byte { 0xf7 });
        callback (BytestreamMidiView (pendingSysExData, pendingSysExTime));
        pendingSysExData.clear();
    }

    static b8 shouldPacketTerminateSysExEarly (u32 firstWord)
    {
        return ! (isSysExContinuation (firstWord)
                  || isSystemRealTime (firstWord)
                  || isJROrNOP (firstWord));
    }

    static SysEx7::Kind getSysEx7Kind (u32 word)
    {
        return SysEx7::Kind ((word >> 0x14) & 0xf);
    }

    static b8 isJROrNOP (u32 word)
    {
        return Utils::getMessageType (word) == 0x0;
    }

    static b8 isSysExContinuation (u32 word)
    {
        if (Utils::getMessageType (word) != 0x3)
            return false;

        const auto kind = getSysEx7Kind (word);
        return kind == SysEx7::Kind::continuation || kind == SysEx7::Kind::end;
    }

    static b8 isSystemRealTime (u32 word)
    {
        return Utils::getMessageType (word) == 0x1 && ((word >> 0x10) & 0xff) >= 0xf8;
    }

    std::vector<std::byte> pendingSysExData;

    f64 pendingSysExTime = 0.0;
};

} // namespace drx::universal_midi_packets

#endif
