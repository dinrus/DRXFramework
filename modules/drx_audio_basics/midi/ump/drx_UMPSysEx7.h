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
    This struct acts as a single-file namespace for Universal MIDI Packet
    functionality related to 7-bit SysEx.

    @tags{Audio}
*/
struct SysEx7
{
    /** Returns the number of 64-bit packets required to hold a series of
        SysEx bytes.

        The number passed to this function should exclude the leading/trailing
        SysEx bytes used in an old midi bytestream, as these are not required
        when using Universal MIDI Packets.
    */
    static u32 getNumPacketsRequiredForDataSize (u32);

    /** The different kinds of UMP SysEx-7 message. */
    enum class Kind : u8
    {
        /** The whole message fits in a single 2-word packet. */
        complete     = 0,

        /** The packet begins a SysEx message that will continue in subsequent packets. */
        begin        = 1,

        /** The packet is a continuation of an ongoing SysEx message. */
        continuation = 2,

        /** The packet terminates an ongoing SysEx message. */
        end          = 3
    };

    /** Holds the bytes from a single SysEx-7 packet. */
    struct PacketBytes
    {
        std::array<std::byte, 6> data;
        u8 size;
    };

    /** Extracts the data bytes from a 64-bit data message. */
    static PacketBytes getDataBytes (const PacketX2& packet);
};

} // namespace drx::universal_midi_packets

#endif
