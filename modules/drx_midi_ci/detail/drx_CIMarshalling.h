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

/*
    Utilities for converting sequences of bytes to and from
    C++ struct types.
*/
namespace drx::midi_ci::detail::Marshalling
{

template <u8> struct IntForNumBytes;
template <> struct IntForNumBytes<1> { using Type = u8; };
template <> struct IntForNumBytes<2> { using Type = u16; };
template <> struct IntForNumBytes<4> { using Type = u32; };

template <u8 NumBytes> using IntForNumBytesT = typename IntForNumBytes<NumBytes>::Type;

//==============================================================================
/*
    Reads a sequence of bytes representing a MIDI-CI message, and populates
    structs with the information contained in the message.
*/
class Reader
{
public:
    /*  Constructs a reader that will parse the provided buffer, using the most
        recent known MIDI-CI version.
    */
    explicit Reader (Span<const std::byte> b)
        : Reader (b, static_cast<u8> (MessageMeta::implementationVersion)) {}

    /*  Constructs a reader for the provided MIDI-CI version that will parse
        the provided buffer. Fields introduced in later versions will be ignored,
        and so left with their default values.
    */
    Reader (Span<const std::byte> b, i32 v)
        : bytes (b), version (v) {}

    std::optional<i32> getVersion() const { return version; }

    /*  Attempts to interpret the byte sequence passed to the constructor
        as a sequence of structs 'T'.

        Возвращает true, если parsing succeeds, otherwise returns false.
    */
    template <typename... T>
    b8 operator() (T&&... t)
    {
        return (doArchiveChecked (std::forward<T> (t)) && ...);
    }

private:
    template <typename T>
    b8 doArchiveChecked (T&& t)
    {
        if (failed)
            return false;

        doArchive (t);
        return ! failed;
    }

    z0 doArchive (ChannelInGroup& x)
    {
        if (const auto popped = popBytes (1))
        {
            const auto p = *popped;
            x = ChannelInGroup (p[0] & std::byte { 0x7f });
            return;
        }

        failed = true;
    }

    // If we're trying to parse into a constant, then we should check that the next byte(s)
    // match that constant.
    z0 doArchive (const std::byte& x)
    {
        std::byte temp{};

        if (! doArchiveChecked (temp))
            return;

        failed |= x != temp;
    }

    z0 doArchive (std::byte& x)
    {
        if (const auto popped = popBytes (1))
        {
            const auto p = *popped;
            x = p[0] & std::byte { 0x7f };
            return;
        }

        failed = true;
    }

    z0 doArchive (u16k& x)
    {
        u16 temp{};

        if (! doArchiveChecked (temp))
            return;

        failed |= temp != x;
    }

    z0 doArchive (u16& x)
    {
        if (const auto popped = popBytes (2))
        {
            const auto p = *popped;
            x = (u16) (((u16) p[0] & 0x7f) << 0x00)
              | (u16) (((u16) p[1] & 0x7f) << 0x07);
            return;
        }

        failed = true;
    }

    z0 doArchive (u32k& x)
    {
        u32 temp{};

        if (! doArchiveChecked (temp))
            return;

        failed |= temp != x;
    }

    z0 doArchive (u32& x)
    {
        if (const auto popped = popBytes (4))
        {
            const auto p = *popped;
            x = (((u32) p[0] & 0x7f) << 0x00)
              | (((u32) p[1] & 0x7f) << 0x07)
              | (((u32) p[2] & 0x7f) << 0x0e)
              | (((u32) p[3] & 0x7f) << 0x15);
            return;
        }

        failed = true;
    }

    template <u8 NumBytes, b8 B>
    z0 doArchive (MessageMeta::SpanWithSizeBytes<NumBytes, Span<const std::byte>, B> x)
    {
        IntForNumBytesT<NumBytes> numBytes{};

        // Read the number of bytes in the field
        if (! doArchiveChecked (numBytes))
            return;

        // Attempt to pop that many bytes
        if (const auto popped = popBytes (numBytes))
        {
            x.span = *popped;
            return;
        }

        failed = true;
    }

    template <u8 NumBytes, size_t N>
    z0 doArchive (MessageMeta::SpanWithSizeBytes<NumBytes, Span<const std::array<std::byte, N>>> x)
    {
        IntForNumBytesT<NumBytes> numItems{};

        // Read the number of items in the field
        if (! doArchiveChecked (numItems))
            return;

        if (const auto popped = popBytes (numItems * N))
        {
            x.span = Span (unalignedPointerCast<const std::array<std::byte, N>*> (popped->data()), numItems);
            return;
        }

        failed = true;
    }

    template <size_t N>
    z0 doArchive (Span<const std::byte, N>& x)
    {
        if (const auto popped = popBytes (bytes.size()))
        {
            x = *popped;
            return;
        }

        failed = true;
    }

    template <size_t N>
    z0 doArchive (std::array<std::byte, N>& x)
    {
        if (const auto popped = popBytes (x.size()))
        {
            const auto p = *popped;
            std::transform (p.begin(), p.end(), x.begin(), [] (std::byte b)
            {
                return b & std::byte { 0x7f };
            });
            return;
        }

        failed = true;
    }

    template <typename T>
    z0 doArchive (T& t)
    {
        drx::detail::doLoad (*this, t);
    }

    template <typename T>
    z0 doArchive (Named<T> named)
    {
        doArchiveChecked (named.value);
    }

    std::optional<Span<const std::byte>> popBytes (size_t num)
    {
        if (bytes.size() < num)
            return {};

        const Span result { bytes.data(), num };
        bytes = Span { bytes.data() + num, bytes.size() - num };
        return result;
    }

    Span<const std::byte> bytes;    /*   Bytes making up a CI message. */
    i32 version{};                  /*   The version to assume when parsing the message, specified in the message header. */
    b8 failed = false;
};

//==============================================================================
/*
    Converts one or more structs into a byte sequence suitable for transmission
    as a MIDI-CI message.
*/
class Writer
{
public:
    /*  Constructs a writer that will write into the provided buffer. */
    explicit Writer (std::vector<std::byte>& b)
        : Writer (b, static_cast<u8> (MessageMeta::implementationVersion)) {}

    /*  Constructs a writer that will write a MIDI-CI message of the requested
        version to the provided buffer.

        Fields introduced in later MIDI-CI versions will be ignored.
    */
    Writer (std::vector<std::byte>& b, i32 v)
        : bytes (b), version (v) {}

    std::optional<i32> getVersion() const { return version; }

    /*  Formats the information contained in the provided structs into a
        MIDI-CI message, and returns a b8 indicating success or failure.
    */
    template <typename... T>
    b8 operator() (const T&... t)
    {
        return (doArchiveChecked (t) && ...);
    }

private:
    template <typename T>
    b8 doArchiveChecked (T&& t)
    {
        if (failed)
            return false;

        doArchive (t);
        return ! failed;
    }

    z0 doArchive (ChannelInGroup x)
    {
        doArchiveChecked (std::byte (x));
    }

    z0 doArchive (std::byte x)
    {
        bytes.push_back (x);
    }

    z0 doArchive (u16 x)
    {
        bytes.insert (bytes.end(), { (std::byte) ((x >> 0x00) & 0x7f),
                                     (std::byte) ((x >> 0x07) & 0x7f) });
    }

    z0 doArchive (u32 x)
    {
        bytes.insert (bytes.end(), { (std::byte) ((x >> 0x00) & 0x7f),
                                     (std::byte) ((x >> 0x07) & 0x7f),
                                     (std::byte) ((x >> 0x0e) & 0x7f),
                                     (std::byte) ((x >> 0x15) & 0x7f) });
    }

    template <u8 NumBytes, typename T, b8 B>
    z0 doArchive (MessageMeta::SpanWithSizeBytes<NumBytes, T, B> x)
    {
        if (x.span.size() >= (1 << (7 * NumBytes)))
        {
            // Unable to express the size of the field in the requested number of bytes
            jassertfalse;
            failed = true;
            return;
        }

        // Write the number of bytes, followed by the bytes themselves.
        const auto numBytes = (IntForNumBytesT<NumBytes>) x.span.size();
        doArchiveChecked (numBytes);
        doArchiveChecked (x.span);
    }

    template <typename T, size_t N>
    z0 doArchive (Span<const T, N> x)
    {
        failed = ! std::all_of (x.begin(), x.end(), [&] (const auto& item)
        {
            return doArchiveChecked (item);
        });
    }

    template <size_t N>
    z0 doArchive (const std::array<std::byte, N>& x)
    {
        bytes.insert (bytes.end(), x.begin(), x.end());
    }

    template <typename T>
    z0 doArchive (const T& t)
    {
        drx::detail::doSave (*this, t);
    }

    template <typename T>
    z0 doArchive (Named<T> named)
    {
        doArchiveChecked (named.value);
    }

    std::vector<std::byte>& bytes;    /*   The buffer that will hold the completed message. */
    i32 version{};                    /*   The version to assume when writing the message, specified in the message header. */
    b8 failed = false;
};

} // namespace drx::midi_ci::detail::Marshalling
