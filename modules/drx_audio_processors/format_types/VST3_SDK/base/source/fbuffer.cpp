//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fbuffer.cpp
// Created by  : Steinberg, 2008
// Description :
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "base/source/fbuffer.h"
#include "base/source/fstring.h"
#include <cstdlib>

namespace Steinberg {

//-------------------------------------------------------------------------------------
Buffer::Buffer ()
: buffer (nullptr)
, memSize (0)
, fillSize (0)
, delta (defaultDelta)
{}

//-------------------------------------------------------------------------------------
Buffer::Buffer (u32 s, u8 initVal)
: buffer (nullptr)
, memSize (s)
, fillSize (0)
, delta (defaultDelta)
{
    if (memSize == 0)
        return;
    buffer = (i8*)::malloc (memSize);
    if (buffer)
        memset (buffer, initVal, memSize);
    else
        memSize = 0;
}

//-------------------------------------------------------------------------------------
Buffer::Buffer (u32 s)
: buffer (nullptr)
, memSize (s)
, fillSize (0)
, delta (defaultDelta)
{
    if (memSize == 0)
        return;
    buffer = (i8*)::malloc (memSize);
    if (!buffer)
        memSize = 0;
}

//-------------------------------------------------------------------------------------
Buffer::Buffer (ukk b , u32 s)
: buffer (nullptr)
, memSize (s)
, fillSize (s)
, delta (defaultDelta)
{
    if (memSize == 0)
        return;
    buffer = (i8*)::malloc (memSize);
    if (buffer)
        memcpy (buffer, b, memSize);
    else
    {
        memSize = 0;
        fillSize = 0;
    }
}

//-------------------------------------------------------------------------------------
Buffer::Buffer (const Buffer& bufferR)
: buffer (nullptr)
, memSize (bufferR.memSize)
, fillSize (bufferR.fillSize)
, delta (bufferR.delta)
{
    if (memSize == 0)
        return;

    buffer = (i8*)::malloc (memSize);
    if (buffer)
        memcpy (buffer, bufferR.buffer, memSize);
    else
        memSize = 0;
}

//-------------------------------------------------------------------------------------
Buffer::~Buffer ()
{
    if (buffer)
        ::free (buffer);
    buffer = nullptr;
}

//-------------------------------------------------------------------------------------
z0 Buffer::operator = (const Buffer& b2)
{
    if (&b2 != this)
    {
        setSize (b2.memSize);
        if (b2.memSize > 0 && buffer)
            memcpy (buffer, b2.buffer, b2.memSize);
        fillSize = b2.fillSize;
        delta = b2.delta;
    }
}

//-------------------------------------------------------------------------------------
b8 Buffer::operator == (const Buffer& b2)const
{
    if (&b2 == this)
        return true;
    if (b2.getSize () != getSize ())
        return false;
    return memcmp (this->int8Ptr (), b2.int8Ptr (), getSize ()) == 0 ? true : false;
}

//-------------------------------------------------------------------------------------
u32 Buffer::get (uk b, u32 size)
{
    u32 maxGet = memSize - fillSize;
    if (size > maxGet)
        size = maxGet;
    if (size > 0)
        memcpy (b, buffer + fillSize, size);
    fillSize += size;
    return size;
}

//-------------------------------------------------------------------------------------
b8 Buffer::put (char16 c)
{
    return put ((ukk)&c, sizeof (c));
}

//-------------------------------------------------------------------------------------
b8 Buffer::put (u8 byte)
{
    if (grow (fillSize + 1) == false)
        return false;

    buffer [fillSize++] = byte;
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::put (t8 c)
{
    if (grow (fillSize + 1) == false)
        return false;

    buffer [fillSize++] = c;
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::put (ukk toPut, u32 s)
{
    if (!toPut)
        return false;

    if (grow (fillSize + s) == false)
        return false;

    memcpy (buffer + fillSize, toPut, s);
    fillSize += s;
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::put (const Txt& str)
{
    return put ((ukk)str.text () , (str.length () + 1) * sizeof (tchar));
}

//-------------------------------------------------------------------------------------
b8 Buffer::appendString8 (const char8* s)
{
    if (!s)
        return false;

    u32 len = (u32) strlen (s);
    return put (s, len);
}

//-------------------------------------------------------------------------------------
b8 Buffer::appendString16 (const char16* s)
{
    if (!s)
        return false;
    ConstString str (s);
    u32 len = (u32) str.length () * sizeof (char16);
    return put (s, len);
}

//-------------------------------------------------------------------------------------
b8 Buffer::prependString8 (const char8* s)
{
    if (!s)
        return false;

    u32 len = (u32) strlen (s);
    if (len > 0)
    {
        shiftStart (len);
        memcpy (buffer, s, len);
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------
b8 Buffer::prependString16 (const char16* s)
{
    if (!s)
        return false;

    ConstString str (s);
    u32 len = (u32) str.length () * sizeof (char16);

    if (len > 0)
    {
        shiftStart (len);
        memcpy (buffer, s, len);
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------
b8 Buffer::prependString8 (char8 c)
{
    shiftStart (sizeof (t8));
    tuk b = (tuk)buffer;
    b [0] = c;
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::prependString16 (char16 c)
{
    shiftStart (sizeof (char16));
    char16* b = (char16*)buffer;
    b [0] = c;
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::copy (u32 from, u32 to, u32 bytes)
{
    if (from + bytes > memSize || bytes == 0)
        return false;

    if (to + bytes > memSize)
        setSize (to + bytes);

    if (from + bytes > to && from < to)
    {              // overlap
        Buffer tmp (buffer + from, bytes);
        memcpy (buffer + to, tmp, bytes);
    }
    else
        memcpy (buffer + to, buffer + from, bytes);
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::makeHexString (Txt& result)
{
    u8* data = uint8Ptr ();
    u32 bytes = getSize ();

    if (data == nullptr || bytes == 0)
        return false;

    char8* stringBuffer = NEWSTR8 ((bytes * 2) + 1);
    if (!stringBuffer)
        return false;

    i32 count = 0;
    while (bytes > 0)
    {
        u8 t1 = ((*data) >> 4) & 0x0F;
        u8 t2 = (*data) & 0x0F;
        if (t1 < 10)
            t1 += '0';
        else
            t1 = t1 - 10 + 'A';
        if (t2 < 10)
            t2 += '0';
        else
            t2 = t2 - 10 + 'A';

        stringBuffer [count++] = t1;
        stringBuffer [count++] = t2;
        data++;
        bytes--;
    }
    stringBuffer [count] = 0;

    result.take ((uk)stringBuffer, false);
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::fromHexString (const char8* string)
{
    flush ();
    if (string == nullptr)
        return false;

    i32 len = strlen8 (string);
    if (len == 0 || ((len & 1) == 1)/*odd number*/ )
        return false;

    setSize (len / 2);
    u8* data = uint8Ptr ();

    b8 upper = true;
    i32 count = 0;
    while (count < len)
    {
        t8 c = string [count];

        u8 d = 0;
        if (c >= '0' && c <= '9')       d += c - '0';
        else if (c >= 'A' && c <= 'F')  d += c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')  d += c - 'a' + 10;
        else return false; // no hex string

        if (upper)
            data [count >> 1] = static_cast<u8> (d << 4);
        else
            data [count >> 1] += d;

        upper = !upper;
        count++;
    }
    setFillSize (len / 2);
    return true;
}

//------------------------------------------------------------------------
z0 Buffer::set (u8 value)
{
    if (buffer)
        memset (buffer, value, memSize);
}

//-------------------------------------------------------------------------------------
b8 Buffer::setFillSize (u32 c)
{
    if (c <= memSize)
    {
        fillSize = c;
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------
b8 Buffer::truncateToFillSize ()
{
    if (fillSize < memSize)
        setSize (fillSize);

    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::grow (u32 newSize)
{
    if (newSize > memSize)
    {
        if (delta == 0)
            delta = defaultDelta;
        u32 s = ((newSize + delta - 1) / delta) * delta;
        return setSize (s);
    }
    return true;
}

//------------------------------------------------------------------------
z0 Buffer::shiftAt (u32 position, i32 amount)
{
    if (amount > 0)
    {
        if (grow (fillSize + amount))
        {
            if (position < fillSize)
                memmove (buffer + amount + position, buffer + position, fillSize - position);

            fillSize += amount;
        }
    }
    else if (amount < 0 && fillSize > 0)
    {
        u32 toRemove = -amount;

        if (toRemove < fillSize)
        {
            if (position < fillSize)
                memmove (buffer + position, buffer + toRemove + position, fillSize - position - toRemove);
            fillSize -= toRemove;
        }
    }
}

//-------------------------------------------------------------------------------------
z0 Buffer::move (i32 amount, u8 initVal)
{
    if (memSize == 0)
        return;

    if (amount > 0)
    {
        if ((u32)amount < memSize)
        {
            memmove (buffer + amount, buffer, memSize - amount);
            memset (buffer, initVal, amount);
        }
        else
            memset (buffer, initVal, memSize);
    }
    else
    {
        u32 toRemove = -amount;
        if (toRemove < memSize)
        {
            memmove (buffer, buffer + toRemove, memSize - toRemove);
            memset (buffer + memSize - toRemove, initVal, toRemove);
        }
        else
            memset (buffer, initVal, memSize);
    }
}

//-------------------------------------------------------------------------------------
b8 Buffer::setSize (u32 newSize)
{
    if (memSize != newSize)
    {
        if (buffer)
        {
            if (newSize > 0)
            {
                i8* newBuffer = (i8*) ::realloc (buffer, newSize);
                if (newBuffer == nullptr)
                {
                    newBuffer = (i8*)::malloc (newSize);
                    if (newBuffer)
                    {
                        u32 tmp = newSize;
                        if (tmp > memSize)
                            tmp = memSize;
                        memcpy (newBuffer, buffer, tmp);
                        ::free (buffer);
                        buffer = newBuffer;
                    }
                    else
                    {
                        ::free (buffer);
                        buffer = nullptr;
                    }
                }
                else
                    buffer = newBuffer;
            }
            else
            {
                ::free (buffer);
                buffer = nullptr;
            }
        }
        else
            buffer = (i8*)::malloc (newSize);

        if (newSize > 0 && !buffer)
            memSize = 0;
        else
            memSize = newSize;
        if (fillSize > memSize)
            fillSize = memSize;
    }

    return (newSize > 0) == (buffer != nullptr);
}

//-------------------------------------------------------------------------------------
z0 Buffer::fillup (u8 value)
{
    if (getFree () > 0)
        memset (buffer + fillSize, value, getFree ());
}

//-------------------------------------------------------------------------------------
i8* Buffer::operator + (u32 i)
{
    if (i < memSize)
        return buffer + i;

    static i8 eof;
    eof = 0;
    return &eof;
}

//-------------------------------------------------------------------------------------
b8 Buffer::swap (i16 swapSize)
{
    return swap (buffer, memSize, swapSize);
}

//-------------------------------------------------------------------------------------
b8 Buffer::swap (uk buffer, u32 bufferSize, i16 swapSize)
{
    if (swapSize != kSwap16 && swapSize != kSwap32 && swapSize != kSwap64)
        return false;

    if (swapSize == kSwap16)
    {
        for (u32 count = 0 ; count < bufferSize ; count += 2)
        {
            SWAP_16 ( * (((i16*)buffer) + count) );
        }
    }
    else if (swapSize == kSwap32)
    {
        for (u32 count = 0 ; count < bufferSize ; count += 4)
        {
            SWAP_32 ( * (((i32*)buffer) + count) );
        }
    }
    else if (swapSize == kSwap64)
    {
        for (u32 count = 0 ; count < bufferSize ; count += 8)
        {
            SWAP_64 ( * (((z64*)buffer) + count) );
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------
z0 Buffer::take (Buffer& from)
{
    setSize (0);
    memSize = from.memSize;
    fillSize = from.fillSize;
    buffer = from.buffer;
    from.buffer = nullptr;
    from.memSize = 0;
    from.fillSize = 0;
}

//-------------------------------------------------------------------------------------
i8* Buffer::pass ()
{
    i8* res = buffer;
    buffer = nullptr;
    memSize = 0;
    fillSize = 0;
    return res;
}

//-------------------------------------------------------------------------------------
b8 Buffer::toWideString (i32 sourceCodePage)
{
    if (getFillSize () > 0)
    {
        if (str8 () [getFillSize () - 1] != 0) // multiByteToWideString only works with 0-terminated strings
            endString8 ();

        Buffer dest (getFillSize () * sizeof (char16));
        i32 result = Txt::multiByteToWideString (dest.str16 (), str8 (), dest.getFree () / sizeof (char16), sourceCodePage);
        if (result > 0)
        {
            dest.setFillSize ((result - 1) * sizeof (char16));
            take (dest);
            return true;
        }
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------
b8 Buffer::toMultibyteString (i32 destCodePage)
{
    if (getFillSize () > 0)
    {
        i32 textLength = getFillSize () / sizeof (char16); // wideStringToMultiByte only works with 0-terminated strings
        if (str16 () [textLength - 1] != 0)
            endString16 ();

        Buffer dest (getFillSize ());
        i32 result = Txt::wideStringToMultiByte (dest.str8 (), str16 (), dest.getFree (), destCodePage);
        if (result > 0)
        {
            dest.setFillSize (result - 1);
            take (dest);
            return true;
        }
        return false;
    }
    return true;
}

//------------------------------------------------------------------------
} // namespace Steinberg
