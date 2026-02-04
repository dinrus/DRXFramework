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
    Holds a single Universal MIDI Packet.

    @tags{Audio}
*/
template <size_t numWords>
class Packet
{
public:
    Packet() = default;

    template <size_t w = numWords, std::enable_if_t<w == 1, i32> = 0>
    Packet (u32 a)
        : contents { { a } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 1);
    }

    template <size_t w = numWords, std::enable_if_t<w == 2, i32> = 0>
    Packet (u32 a, u32 b)
        : contents { { a, b } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 2);
    }

    template <size_t w = numWords, std::enable_if_t<w == 3, i32> = 0>
    Packet (u32 a, u32 b, u32 c)
        : contents { { a, b, c } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 3);
    }

    template <size_t w = numWords, std::enable_if_t<w == 4, i32> = 0>
    Packet (u32 a, u32 b, u32 c, u32 d)
        : contents { { a, b, c, d } }
    {
        jassert (Utils::getNumWordsForMessageType (a) == 4);
    }

    template <size_t w, std::enable_if_t<w == numWords, i32> = 0>
    explicit Packet (const std::array<u32, w>& fullPacket)
        : contents (fullPacket)
    {
        jassert (Utils::getNumWordsForMessageType (fullPacket.front()) == numWords);
    }

    Packet withMessageType (u8 type) const noexcept
    {
        return withU4<0> (type);
    }

    Packet withGroup (u8 group) const noexcept
    {
        return withU4<1> (group);
    }

    Packet withStatus (u8 status) const noexcept
    {
        return withU4<2> (status);
    }

    Packet withChannel (u8 channel) const noexcept
    {
        return withU4<3> (channel);
    }

    u8 getMessageType() const noexcept { return getU4<0>(); }

    u8 getGroup() const noexcept { return getU4<1>(); }

    u8 getStatus() const noexcept { return getU4<2>(); }

    u8 getChannel() const noexcept { return getU4<3>(); }

    template <size_t index>
    Packet withU4 (u8 value) const noexcept
    {
        constexpr auto word = index / 8;
        auto copy = *this;
        std::get<word> (copy.contents) = Utils::U4<index % 8>::set (copy.template getU32<word>(), value);
        return copy;
    }

    template <size_t index>
    Packet withU8 (u8 value) const noexcept
    {
        constexpr auto word = index / 4;
        auto copy = *this;
        std::get<word> (copy.contents) = Utils::U8<index % 4>::set (copy.template getU32<word>(), value);
        return copy;
    }

    template <size_t index>
    Packet withU16 (u16 value) const noexcept
    {
        constexpr auto word = index / 2;
        auto copy = *this;
        std::get<word> (copy.contents) = Utils::U16<index % 2>::set (copy.template getU32<word>(), value);
        return copy;
    }

    template <size_t index>
    Packet withU32 (u32 value) const noexcept
    {
        auto copy = *this;
        std::get<index> (copy.contents) = value;
        return copy;
    }

    template <size_t index>
    u8 getU4() const noexcept
    {
        return Utils::U4<index % 8>::get (this->template getU32<index / 8>());
    }

    template <size_t index>
    u8 getU8() const noexcept
    {
        return Utils::U8<index % 4>::get (this->template getU32<index / 4>());
    }

    template <size_t index>
    u16 getU16() const noexcept
    {
        return Utils::U16<index % 2>::get (this->template getU32<index / 2>());
    }

    template <size_t index>
    u32 getU32() const noexcept
    {
        return std::get<index> (contents);
    }

    //==============================================================================
    using Contents = std::array<u32, numWords>;

    using const_iterator    = typename Contents::const_iterator;

    const_iterator  begin()                   const noexcept { return contents.begin(); }
    const_iterator cbegin()                   const noexcept { return contents.begin(); }

    const_iterator  end()                     const noexcept { return contents.end(); }
    const_iterator cend()                     const noexcept { return contents.end(); }

    u32k* data()                    const noexcept { return contents.data(); }

    u32k& front()                   const noexcept { return contents.front(); }
    u32k& back()                    const noexcept { return contents.back(); }

    u32k& operator[] (size_t index) const noexcept { return contents[index]; }

private:
    Contents contents { {} };
};

using PacketX1 = Packet<1>;
using PacketX2 = Packet<2>;
using PacketX3 = Packet<3>;
using PacketX4 = Packet<4>;

} // namespace drx::universal_midi_packets

#endif
