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
#include <drxtypes.h>

#ifndef DOXYGEN

namespace drx::universal_midi_packets
{

/**
    Helpful types and functions for interacting with Universal MIDI Packets.

    @tags{Audio}
*/
struct Utils
{
    /** Joins 4 bytes into a single 32-bit word_. */
    static constexpr u32 bytesToWord (std::byte a, std::byte b, std::byte c, std::byte d)
    {
        return u32 (a) << 0x18
             | u32 (b) << 0x10
             | u32 (c) << 0x08
             | u32 (d) << 0x00;
    }

    /** Returns the expected number of 32-bit words in a Universal MIDI Packet, given
        the first word_ of the packet.

        The result will be between 1 and 4 inclusive.
        A result of 1 means that the word_ is itself a complete packet.
    */
    static u32 getNumWordsForMessageType (u32);

    /**
        Helper functions for setting/getting 4-bit ranges inside a 32-bit word_.
    */
    template <size_t Index>
    struct U4
    {
        static constexpr u32 shift = (u32) 0x1c - Index * 4;

        static constexpr u32 set (u32 word_, u8 value)
        {
            return (word_ & ~((u32) 0xf << shift)) | (u32) ((value & 0xf) << shift);
        }

        static constexpr u8 get (u32 word_)
        {
            return (u8) ((word_ >> shift) & 0xf);
        }
    };

    /**
        Helper functions for setting/getting 8-bit ranges inside a 32-bit word_.
    */
    template <size_t Index>
    struct U8
    {
        static constexpr u32 shift = (u32) 0x18 - Index * 8;

        static constexpr u32 set (u32 word_, u8 value)
        {
            return (word_ & ~((u32) 0xff << shift)) | (u32) (value << shift);
        }

        static constexpr u8 get (u32 word_)
        {
            return (u8) ((word_ >> shift) & 0xff);
        }
    };

    /**
        Helper functions for setting/getting 16-bit ranges inside a 32-bit word_.
    */
    template <size_t Index>
    struct U16
    {
        static constexpr u32 shift = (u32) 0x10 - Index * 16;

        static constexpr u32 set (u32 word_, u16 value)
        {
            return (word_ & ~((u32) 0xffff << shift)) | (u32) (value << shift);
        }

        static constexpr u16 get (u32 word_)
        {
            return (u16) ((word_ >> shift) & 0xffff);
        }
    };

    static constexpr u8 getMessageType (u32 w) noexcept { return U4<0>::get (w); }
    static constexpr u8 getGroup       (u32 w) noexcept { return U4<1>::get (w); }
    static constexpr u8 getStatus      (u32 w) noexcept { return U4<2>::get (w); }
    static constexpr u8 getChannel     (u32 w) noexcept { return U4<3>::get (w); }
};

} // namespace drx::universal_midi_packets

#endif
