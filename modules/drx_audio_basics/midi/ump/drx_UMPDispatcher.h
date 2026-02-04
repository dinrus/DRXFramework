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
    Parses a raw stream of u32, and calls a user-provided callback every time
    a full Universal MIDI Packet is encountered.

    @tags{Audio}
*/
class Dispatcher
{
public:
    /** Clears the dispatcher. */
    z0 reset() { currentPacketLen = 0; }

    /** Calls `callback` with a View of each packet encountered in the range delimited
        by `begin` and `end`.

        If the range ends part-way through a packet, the next call to `dispatch` will
        continue from that point in the packet (unless `reset` is called first).
    */
    template <typename PacketCallbackFunction>
    z0 dispatch (u32k* begin,
                   u32k* end,
                   f64 timeStamp,
                   PacketCallbackFunction&& callback)
    {
        std::for_each (begin, end, [&] (u32 word)
        {
            nextPacket[currentPacketLen++] = word;

            if (currentPacketLen == Utils::getNumWordsForMessageType (nextPacket.front()))
            {
                callback (View (nextPacket.data()), timeStamp);
                currentPacketLen = 0;
            }
        });
    }

private:
    std::array<u32, 4> nextPacket;
    size_t currentPacketLen = 0;
};

//==============================================================================
/**
    Parses a stream of bytes representing a sequence of bytestream-encoded MIDI 1.0 messages,
    converting the messages to UMP format and passing the packets to a user-provided callback
    as they become ready.

    @tags{Audio}
*/
class BytestreamToUMPDispatcher
{
public:
    /** Initialises the dispatcher.

        Channel messages will be converted to the requested protocol format `pp`.
        `storageSize` bytes will be allocated to store incomplete messages.
    */
    explicit BytestreamToUMPDispatcher (PacketProtocol pp, i32 storageSize)
        : concatenator (storageSize),
          converter (pp)
    {}

    z0 reset()
    {
        concatenator.reset();
        converter.reset();
    }

    /** Calls `callback` with a View of each converted packet as it becomes ready.

        @param begin        the first byte in a range of bytes representing bytestream-encoded MIDI messages.
        @param end          one-past the last byte in a range of bytes representing bytestream-encoded MIDI messages.
        @param timestamp    a timestamp to apply to the created packets.
        @param callback     a callback which will be passed a View pointing to each new packet as it becomes ready.
    */
    template <typename PacketCallbackFunction>
    z0 dispatch (u8k* begin,
                   u8k* end,
                   f64 timestamp,
                   PacketCallbackFunction&& callback)
    {
        using CallbackPtr = decltype (std::addressof (callback));

        struct Callback
        {
            Callback (BytestreamToUMPDispatcher& d, CallbackPtr c)
                : dispatch (d), callbackPtr (c) {}

            z0 handleIncomingMidiMessage (uk, const MidiMessage& msg) const
            {
                Conversion::toMidi1 (BytestreamMidiView (&msg), [&] (const View& view)
                {
                    dispatch.converter.convert (view, *callbackPtr);
                });
            }

            z0 handlePartialSysexMessage (uk, u8k*, i32, f64) const {}

            BytestreamToUMPDispatcher& dispatch;
            CallbackPtr callbackPtr = nullptr;
        };

        Callback inputCallback { *this, &callback };
        concatenator.pushMidiData (begin, i32 (end - begin), timestamp, (uk) nullptr, inputCallback);
    }

private:
    MidiDataConcatenator concatenator;
    GenericUMPConverter converter;
};

//==============================================================================
/**
    Parses a stream of 32-bit words representing a sequence of UMP-encoded MIDI messages,
    converting the messages to MIDI 1.0 bytestream format and passing them to a user-provided
    callback as they become ready.

    @tags{Audio}
*/
class ToBytestreamDispatcher
{
public:
    /** Initialises the dispatcher.

        `storageSize` bytes will be allocated to store incomplete messages.
    */
    explicit ToBytestreamDispatcher (i32 storageSize)
        : converter (storageSize) {}

    /** Clears the dispatcher. */
    z0 reset()
    {
        dispatcher.reset();
        converter.reset();
    }

    /** Calls `callback` with converted bytestream-formatted MidiMessage whenever
        a new message becomes available.

        @param begin        the first word in a stream of words representing UMP-encoded MIDI packets.
        @param end          one-past the last word in a stream of words representing UMP-encoded MIDI packets.
        @param timestamp    a timestamp to apply to converted messages.
        @param callback     a callback which will be passed a MidiMessage each time a new message becomes ready.
    */
    template <typename BytestreamMessageCallback>
    z0 dispatch (u32k* begin,
                   u32k* end,
                   f64 timestamp,
                   BytestreamMessageCallback&& callback)
    {
        dispatcher.dispatch (begin, end, timestamp, [&] (const View& view, f64 time)
        {
            converter.convert (view, time, callback);
        });
    }

private:
    Dispatcher dispatcher;
    ToBytestreamConverter converter;
};

} // namespace drx::universal_midi_packets

#endif
