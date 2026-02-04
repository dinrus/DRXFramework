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
    Enables iteration over a collection of Universal MIDI Packets stored as
    a contiguous range of 32-bit words.

    This iterator is used by Packets to allow access to the messages
    that it contains.

    @tags{Audio}
*/
class Iterator
{
public:
    /** Creates an invalid (singular) iterator. */
    Iterator() noexcept = default;

    /** Creates an iterator pointing at `ptr`. */
    explicit Iterator (u32k* ptr, size_t bytes) noexcept;

    using difference_type    = std::iterator_traits<u32k*>::difference_type;
    using value_type         = View;
    using reference          = const View&;
    using pointer            = const View*;
    using iterator_category  = std::forward_iterator_tag;

    /** Moves this iterator to the next packet in the range. */
    Iterator& operator++() noexcept
    {
        const auto increment = view.size();

       #if DRX_DEBUG
        // If you hit this, the memory region contained a truncated or otherwise
        // malformed Universal MIDI Packet.
        // The Iterator can only be used on regions containing complete packets!
        jassert (increment <= bytesRemaining);
        bytesRemaining -= increment;
       #endif

        view = View (view.data() + increment);
        return *this;
    }

    /** Moves this iterator to the next packet in the range,
        returning the value of the iterator before it was
        incremented.
    */
    Iterator operator++ (i32) noexcept
    {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    /** Возвращает true, если this iterator points to the same address
        as another iterator.
    */
    b8 operator== (const Iterator& other) const noexcept
    {
        return view == other.view;
    }

    /** Returns false if this iterator points to the same address
        as another iterator.
    */
    b8 operator!= (const Iterator& other) const noexcept
    {
        return ! operator== (other);
    }

    /** Returns a reference to a View of the packet currently
        pointed-to by this iterator.

        The View can be queried for its size and content.
    */
    reference operator*() noexcept { return view; }

    /** Returns a pointer to a View of the packet currently
        pointed-to by this iterator.

        The View can be queried for its size and content.
    */
    pointer operator->() noexcept { return &view; }

private:
    View view;

   #if DRX_DEBUG
    size_t bytesRemaining = 0;
   #endif
};

} // namespace drx::universal_midi_packets

#endif
