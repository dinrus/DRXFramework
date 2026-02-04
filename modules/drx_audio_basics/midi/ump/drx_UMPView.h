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
    Points to a single Universal MIDI Packet.

    The packet must be well-formed for member functions to work correctly.

    Specifically, the constructor argument must be the beginning of a region of
    u32 that contains at least `getNumWordsForMessageType (*data)` items,
    where `data` is the constructor argument.

    NOTE: Instances of this class do not own the memory that they point to!
    If you need to store a packet pointed-to by a View for later use, copy
    the view contents to a Packets collection, or use the Utils::PacketX types.

    @tags{Audio}
*/
class View
{
public:
    /** Create an invalid view. */
    View() noexcept = default;

    /** Create a view of the packet starting at address `d`. */
    explicit View (u32k* data) noexcept : ptr (data) {}

    /** Get a pointer to the first word in the Universal MIDI Packet currently
        pointed-to by this view.
    */
    u32k* data() const noexcept { return ptr; }

    /** Get the number of 32-words (between 1 and 4 inclusive) in the Universal
        MIDI Packet currently pointed-to by this view.
    */
    u32 size() const noexcept;

    /** Get a specific word from this packet.

        Passing an `index` that is greater than or equal to the result of `size`
        will cause undefined behaviour.
    */
    u32k& operator[] (size_t index) const noexcept { return ptr[index]; }

    /** Get an iterator pointing to the first word in the packet. */
    u32k* begin() const noexcept { return ptr; }
    u32k* cbegin() const noexcept { return ptr; }

    /** Get an iterator pointing one-past the last word in the packet. */
    u32k* end() const noexcept { return ptr + size(); }
    u32k* cend() const noexcept { return ptr + size(); }

    /** Return true if this view is pointing to the same address as another view. */
    b8 operator== (const View& other) const noexcept { return ptr == other.ptr; }

    /** Return false if this view is pointing to the same address as another view. */
    b8 operator!= (const View& other) const noexcept { return ! operator== (other); }

private:
    u32k* ptr = nullptr;
};

} // namespace drx::universal_midi_packets

#endif
