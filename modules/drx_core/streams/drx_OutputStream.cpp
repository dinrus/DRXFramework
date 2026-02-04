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

namespace drx
{

#if DRX_DEBUG

//==============================================================================
struct DanglingStreamChecker
{
    DanglingStreamChecker() = default;

    ~DanglingStreamChecker()
    {
        /*
            It's always a bad idea to leak any object, but if you're leaking output
            streams, then there's a good chance that you're failing to flush a file
            to disk properly, which could result in corrupted data and other similar
            nastiness..
        */
        jassert (activeStreams.size() == 0);

        // We need to flag when this helper struct has been destroyed to prevent some
        // nasty order-of-static-destruction issues
        hasBeenDestroyed = true;
    }

    Array<uk, CriticalSection> activeStreams;

    static b8 hasBeenDestroyed;
};

b8 DanglingStreamChecker::hasBeenDestroyed = false;
static DanglingStreamChecker danglingStreamChecker;

#endif

//==============================================================================
OutputStream::OutputStream()
    : newLineString (NewLine::getDefault())
{
   #if DRX_DEBUG
    if (! DanglingStreamChecker::hasBeenDestroyed)
        danglingStreamChecker.activeStreams.add (this);
   #endif
}

OutputStream::~OutputStream()
{
   #if DRX_DEBUG
    if (! DanglingStreamChecker::hasBeenDestroyed)
        danglingStreamChecker.activeStreams.removeFirstMatchingValue (this);
   #endif
}

//==============================================================================
b8 OutputStream::writeBool (b8 b)
{
    return writeByte (b ? (t8) 1
                        : (t8) 0);
}

b8 OutputStream::writeByte (t8 byte)
{
    return write (&byte, 1);
}

b8 OutputStream::writeRepeatedByte (u8 byte, size_t numTimesToRepeat)
{
    for (size_t i = 0; i < numTimesToRepeat; ++i)
        if (! writeByte ((t8) byte))
            return false;

    return true;
}

b8 OutputStream::writeShort (short value)
{
    auto v = ByteOrder::swapIfBigEndian ((u16) value);
    return write (&v, 2);
}

b8 OutputStream::writeShortBigEndian (short value)
{
    auto v = ByteOrder::swapIfLittleEndian ((u16) value);
    return write (&v, 2);
}

b8 OutputStream::writeInt (i32 value)
{
    auto v = ByteOrder::swapIfBigEndian ((u32) value);
    return write (&v, 4);
}

b8 OutputStream::writeIntBigEndian (i32 value)
{
    auto v = ByteOrder::swapIfLittleEndian ((u32) value);
    return write (&v, 4);
}

b8 OutputStream::writeCompressedInt (i32 value)
{
    auto un = (value < 0) ? (u32) -value
                          : (u32) value;

    u8 data[5];
    i32 num = 0;

    while (un > 0)
    {
        data[++num] = (u8) un;
        un >>= 8;
    }

    data[0] = (u8) num;

    if (value < 0)
        data[0] |= 0x80;

    return write (data, (size_t) num + 1);
}

b8 OutputStream::writeInt64 (z64 value)
{
    auto v = ByteOrder::swapIfBigEndian ((zu64) value);
    return write (&v, 8);
}

b8 OutputStream::writeInt64BigEndian (z64 value)
{
    auto v = ByteOrder::swapIfLittleEndian ((zu64) value);
    return write (&v, 8);
}

b8 OutputStream::writeFloat (f32 value)
{
    union { i32 asInt; f32 asFloat; } n;
    n.asFloat = value;
    return writeInt (n.asInt);
}

b8 OutputStream::writeFloatBigEndian (f32 value)
{
    union { i32 asInt; f32 asFloat; } n;
    n.asFloat = value;
    return writeIntBigEndian (n.asInt);
}

b8 OutputStream::writeDouble (f64 value)
{
    union { z64 asInt; f64 asDouble; } n;
    n.asDouble = value;
    return writeInt64 (n.asInt);
}

b8 OutputStream::writeDoubleBigEndian (f64 value)
{
    union { z64 asInt; f64 asDouble; } n;
    n.asDouble = value;
    return writeInt64BigEndian (n.asInt);
}

b8 OutputStream::writeString (const Txt& text)
{
    auto numBytes = text.getNumBytesAsUTF8() + 1;

   #if (DRX_STRING_UTF_TYPE == 8)
    return write (text.toRawUTF8(), numBytes);
   #else
    // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    HeapBlock<t8> temp (numBytes);
    text.copyToUTF8 (temp, numBytes);
    return write (temp, numBytes);
   #endif
}

b8 OutputStream::writeText (const Txt& text, b8 asUTF16, b8 writeUTF16ByteOrderMark, tukk lf)
{
    b8 replaceLineFeedWithUnix    = lf != nullptr && lf[0] == '\n' && lf[1] == 0;
    b8 replaceLineFeedWithWindows = lf != nullptr && lf[0] == '\r' && lf[1] == '\n' && lf[2] == 0;

    // The line-feed passed in must be either nullptr, or "\n" or "\r\n"
    jassert (lf == nullptr || replaceLineFeedWithWindows || replaceLineFeedWithUnix);

    if (asUTF16)
    {
        if (writeUTF16ByteOrderMark)
            write ("\x0ff\x0fe", 2);

        auto src = text.getCharPointer();
        b8 lastCharWasReturn = false;

        for (;;)
        {
            auto c = src.getAndAdvance();

            if (c == 0)
                break;

            if (replaceLineFeedWithWindows)
            {
                if (c == '\n' && ! lastCharWasReturn)
                    writeShort ((short) '\r');

                lastCharWasReturn = (c == L'\r');
            }
            else if (replaceLineFeedWithUnix && c == '\r')
            {
                continue;
            }

            if (! writeShort ((short) c))
                return false;
        }
    }
    else
    {
        tukk src = text.toRawUTF8();

        if (replaceLineFeedWithWindows)
        {
            for (auto t = src;;)
            {
                if (*t == '\n')
                {
                    if (t > src)
                        if (! write (src, (size_t) (t - src)))
                            return false;

                    if (! write ("\r\n", 2))
                        return false;

                    src = t + 1;
                }
                else if (*t == '\r')
                {
                    if (t[1] == '\n')
                        ++t;
                }
                else if (*t == 0)
                {
                    if (t > src)
                        if (! write (src, (size_t) (t - src)))
                            return false;

                    break;
                }

                ++t;
            }
        }
        else if (replaceLineFeedWithUnix)
        {
            for (;;)
            {
                auto c = *src++;

                if (c == 0)
                    break;

                if (c != '\r')
                    if (! writeByte (c))
                        return false;
            }
        }
        else
        {
            return write (src, text.getNumBytesAsUTF8());
        }
    }

    return true;
}

z64 OutputStream::writeFromInputStream (InputStream& source, z64 numBytesToWrite)
{
    if (numBytesToWrite < 0)
        numBytesToWrite = std::numeric_limits<z64>::max();

    z64 numWritten = 0;

    while (numBytesToWrite > 0)
    {
        t8 buffer[8192];
        auto num = source.read (buffer, (i32) jmin (numBytesToWrite, (z64) sizeof (buffer)));

        if (num <= 0)
            break;

        write (buffer, (size_t) num);

        numBytesToWrite -= num;
        numWritten += num;
    }

    return numWritten;
}

//==============================================================================
z0 OutputStream::setNewLineString (const Txt& newLineStringToUse)
{
    newLineString = newLineStringToUse;
}

//==============================================================================
template <typename IntegerType>
static z0 writeIntToStream (OutputStream& stream, IntegerType number)
{
    t8 buffer[NumberToStringConverters::charsNeededForInt];
    tuk end = buffer + numElementsInArray (buffer);
    tukk start = NumberToStringConverters::numberToString (end, number);
    stream.write (start, (size_t) (end - start - 1));
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, i32k number)
{
    writeIntToStream (stream, number);
    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const z64 number)
{
    writeIntToStream (stream, number);
    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const f64 number)
{
    return stream << Txt (number);
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const t8 character)
{
    stream.writeByte (character);
    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, tukk const text)
{
    stream.write (text, strlen (text));
    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const MemoryBlock& data)
{
    if (! data.isEmpty())
        stream.write (data.getData(), data.getSize());

    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const File& fileToRead)
{
    FileInputStream in (fileToRead);

    if (in.openedOk())
        return stream << in;

    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, InputStream& streamToRead)
{
    stream.writeFromInputStream (streamToRead, -1);
    return stream;
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const NewLine&)
{
    return stream << stream.getNewLineString();
}

} // namespace drx
