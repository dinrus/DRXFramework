//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fstreamer.cpp
// Created by  : Steinberg, 15.12.2005
// Description : Extract of typed stream i/o methods from FStream
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

#include "fstreamer.h"

#include "base/source/fstring.h"
#include "base/source/fbuffer.h"
#include "pluginterfaces/base/ibstream.h"

#ifndef UNICODE
#include "pluginterfaces/base/futils.h"
#endif

namespace Steinberg {

//------------------------------------------------------------------------
// IBStreamer
//------------------------------------------------------------------------
IBStreamer::IBStreamer (IBStream* stream, i16 _byteOrder)
: FStreamer (_byteOrder)
, stream (stream)
{}

//------------------------------------------------------------------------
TSize IBStreamer::readRaw (uk buffer, TSize size)
{
	i32 numBytesRead = 0;
	stream->read (buffer, (i32)size, &numBytesRead);
	return numBytesRead;
}

//------------------------------------------------------------------------
TSize IBStreamer::writeRaw (ukk buffer, TSize size)
{
	i32 numBytesWritten = 0;
	stream->write ((uk)buffer, (i32)size, &numBytesWritten);
	return numBytesWritten;
}

//------------------------------------------------------------------------
z64 IBStreamer::seek (z64 pos, FSeekMode mode)
{
	z64 result = -1;
	stream->seek (pos, mode, &result);
	return result;
}

//------------------------------------------------------------------------
z64 IBStreamer::tell ()
{
	z64 pos = 0;
	stream->tell (&pos);
	return pos;
}

//------------------------------------------------------------------------
// FStreamSizeHolder Implementation
//------------------------------------------------------------------------
FStreamSizeHolder::FStreamSizeHolder (FStreamer &s)
: stream (s), sizePos (-1)
{}

//------------------------------------------------------------------------
z0 FStreamSizeHolder::beginWrite ()
{
	sizePos = stream.tell ();
	stream.writeInt32 (0L);
}

//------------------------------------------------------------------------
i32 FStreamSizeHolder::endWrite ()
{
	if (sizePos < 0)
		return 0;
	
	z64 currentPos = stream.tell ();

	stream.seek (sizePos, kSeekSet);
	i32 size = i32 (currentPos - sizePos - sizeof (i32));
	stream.writeInt32 (size);
	
	stream.seek (currentPos, kSeekSet);
	return size;
}

//------------------------------------------------------------------------
i32 FStreamSizeHolder::beginRead ()
{
	sizePos = stream.tell ();
	i32 size = 0;
	stream.readInt32 (size);
	sizePos += size + sizeof (i32);
	return size;
}

//------------------------------------------------------------------------
z0 FStreamSizeHolder::endRead ()
{
	if (sizePos >= 0)
		stream.seek (sizePos, kSeekSet);
}

//------------------------------------------------------------------------
// FStreamer
//------------------------------------------------------------------------
FStreamer::FStreamer (i16 _byteOrder)
: byteOrder (_byteOrder)
{}

// i8 / t8 -----------------------------------------------------------
//------------------------------------------------------------------------
b8 FStreamer::writeChar8 (char8 c)
{
	return writeRaw ((uk)&c, sizeof (char8)) == sizeof (char8);
}

//------------------------------------------------------------------------
b8 FStreamer::readChar8 (char8& c)
{
	return readRaw ((uk)&c, sizeof (char8)) == sizeof (char8);
}

//------------------------------------------------------------------------
b8 FStreamer::writeUChar8 (u8 c)
{
	return writeRaw ((uk)&c, sizeof (u8)) == sizeof (u8);
}

//------------------------------------------------------------------------
b8 FStreamer::readUChar8 (u8& c)
{
	return readRaw ((uk)&c, sizeof (u8)) == sizeof (u8);
}

//------------------------------------------------------------------------
b8 FStreamer::writeChar16 (char16 c)
{
	if (BYTEORDER != byteOrder)
		SWAP_16 (c);
	return writeRaw ((uk)&c, sizeof (char16)) == sizeof (char16);
}

//------------------------------------------------------------------------
b8 FStreamer::readChar16 (char16& c)
{
	if (readRaw ((uk)&c, sizeof (char16)) == sizeof (char16))
	{
		if (BYTEORDER != byteOrder)
			SWAP_16 (c);
		return true;
	}
	c = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt8 (i8 c)
{
	return writeRaw ((uk)&c, sizeof (i8)) == sizeof (i8);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt8 (i8& c)
{
	return readRaw ((uk)&c, sizeof (i8)) == sizeof (i8);
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt8u (u8 c)
{
	return writeRaw ((uk)&c, sizeof (u8)) == sizeof (u8);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt8u (u8& c)
{
	return readRaw ((uk)&c, sizeof (u8)) == sizeof (u8);
}

// i16 -----------------------------------------------------------------
//------------------------------------------------------------------------
b8 FStreamer::writeInt16 (i16 i)
{
	if (BYTEORDER != byteOrder)
		SWAP_16 (i);
	return writeRaw ((uk)&i, sizeof (i16)) == sizeof (i16);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt16 (i16& i)
{
	if (readRaw ((uk)&i, sizeof (i16)) == sizeof (i16))
	{
		if (BYTEORDER != byteOrder)
			SWAP_16 (i);
		return true;
	}
	i = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt16Array (i16k* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeInt16 (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readInt16Array (i16* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readInt16 (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt16u (u16 i)
{
	if (BYTEORDER != byteOrder)
		SWAP_16 (i);
	return writeRaw ((uk)&i, sizeof (u16)) == sizeof (u16);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt16u (u16& i)
{
	if (readRaw ((uk)&i, sizeof (u16)) == sizeof (u16))
	{
		if (BYTEORDER != byteOrder)
			SWAP_16 (i);
		return true;
	}
	i = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt16uArray (u16k* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeInt16u (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readInt16uArray (u16* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readInt16u (array[i]))
			return false;
	}
	return true;
}

// i32 -----------------------------------------------------------------
//------------------------------------------------------------------------
b8 FStreamer::writeInt32 (i32 i)
{
	if (BYTEORDER != byteOrder)
		SWAP_32 (i);
	return writeRaw ((uk)&i, sizeof (i32)) == sizeof (i32);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt32 (i32& i)
{
	if (readRaw ((uk)&i, sizeof (i32)) == sizeof (i32))
	{
		if (BYTEORDER != byteOrder)
			SWAP_32 (i);
		return true;
	}
	i = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt32Array (const i32* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeInt32 (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readInt32Array (i32* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readInt32 (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt32u (u32 i)
{
	if (BYTEORDER != byteOrder)
		SWAP_32 (i);
	return writeRaw ((uk)&i, sizeof (u32)) == sizeof (u32);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt32u (u32& i)
{
	if (readRaw ((uk)&i, sizeof (u32)) == sizeof (u32))
	{
		if (BYTEORDER != byteOrder)
			SWAP_32 (i);
		return true;
	}
	i = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt32uArray (u32k* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeInt32u (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readInt32uArray (u32* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readInt32u (array[i]))
			return false;
	}
	return true;
}

// z64 -----------------------------------------------------------------
//------------------------------------------------------------------------
b8 FStreamer::writeInt64 (z64 i)
{
	if (BYTEORDER != byteOrder)
		SWAP_64 (i);
	return writeRaw ((uk)&i, sizeof (z64)) == sizeof (z64);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt64 (z64& i)
{
	if (readRaw ((uk)&i, sizeof (z64)) == sizeof (z64))
	{
		if (BYTEORDER != byteOrder)
			SWAP_64 (i);
		return true;
	}
	i = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt64Array (const z64* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeInt64 (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readInt64Array (z64* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readInt64 (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt64u (zu64 i)
{
	if (BYTEORDER != byteOrder)
		SWAP_64 (i);
	return writeRaw ((uk)&i, sizeof (zu64)) == sizeof (zu64);
}

//------------------------------------------------------------------------
b8 FStreamer::readInt64u (zu64& i)
{
	if (readRaw ((uk)&i, sizeof (zu64)) == sizeof (zu64))
	{
		if (BYTEORDER != byteOrder)
			SWAP_64 (i);
		return true;
	}
	i = 0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeInt64uArray (const zu64* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeInt64u (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readInt64uArray (zu64* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readInt64u (array[i]))
			return false;
	}
	return true;
}

// f32 / f64 --------------------------------------------------------
//------------------------------------------------------------------------
b8 FStreamer::writeFloat (f32 f)
{
	if (BYTEORDER != byteOrder)
		SWAP_32 (f);
	return writeRaw ((uk)&f, sizeof (f32)) == sizeof (f32);
}

//------------------------------------------------------------------------
b8 FStreamer::readFloat (f32& f)
{
	if (readRaw ((uk)&f, sizeof (f32)) == sizeof (f32))
	{
		if (BYTEORDER != byteOrder)
			SWAP_32 (f);
		return true;
	}
	f = 0.f;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeFloatArray (const f32* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeFloat (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readFloatArray (f32* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readFloat (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::writeDouble (f64 d)
{
	if (BYTEORDER != byteOrder)
		SWAP_64 (d);
	return writeRaw ((uk)&d, sizeof (f64)) == sizeof (f64);
}

//------------------------------------------------------------------------
b8 FStreamer::readDouble (f64& d)
{
	if (readRaw ((uk)&d, sizeof (f64)) == sizeof (f64))
	{
		if (BYTEORDER != byteOrder)
			SWAP_64 (d);
		return true;
	}
	d = 0.0;
	return false;
}

//------------------------------------------------------------------------
b8 FStreamer::writeDoubleArray (const f64* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!writeDouble (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readDoubleArray (f64* array, i32 count)
{
	for (i32 i = 0; i < count; i++)
	{
		if (!readDouble (array[i]))
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::readBool (b8& b)
{
	i16 v = 0;
	b8 res = readInt16 (v);
	b = (v != 0);
	return res;
}

//------------------------------------------------------------------------
b8 FStreamer::writeBool (b8 b)
{
	return writeInt16 ((i16)b);
}

//------------------------------------------------------------------------
TSize FStreamer::writeString8 (const char8* ptr, b8 terminate)
{
	TSize size = strlen (ptr);
	if (terminate) // write \0
		size++;

	return writeRaw ((uk)ptr, size);
}

//------------------------------------------------------------------------
TSize FStreamer::readString8 (char8* ptr, TSize size)
{
	if (size < 1 || ptr == nullptr)
		return 0;

	TSize i = 0;
	char8 c = 0;
	while (i < size)
	{
		if (readRaw ((uk)&c, sizeof (t8)) != sizeof (t8))
			break;
		ptr[i] = c;
		if (c == '\n' || c == '\0')
			break;
		i++;
	}
	// remove at end \n (LF) or \r\n (CR+LF)
	if (c == '\n')
	{
		if (i > 0 && ptr[i - 1] == '\r')
			i--;
	}
	if (i >= size)
		i = size - 1;
	ptr[i] = 0;
	
	return i;
}

//------------------------------------------------------------------------
b8 FStreamer::writeStringUtf8 (const tchar* ptr)
{
	b8 isUtf8 = false;

	Txt str (ptr);
	if (str.isAsciiString () == false) 
	{
		str.toMultiByte (kCP_Utf8);
		isUtf8 = true;
	}
	else
	{
		str.toMultiByte ();
	}

	if (isUtf8) 
		if (writeRaw (kBomUtf8, kBomUtf8Length) != kBomUtf8Length)
			return false;

	TSize size = str.length () + 1;
	if (writeRaw (str.text8 (), size) != size)
		return false;

	return true;
}

//------------------------------------------------------------------------
i32 FStreamer::readStringUtf8 (tchar* ptr, i32 nChars)
{
	char8 c = 0;

	ptr [0] = 0;

	Buffer tmp;
	tmp.setDelta (1024);

	while (true)
	{
		if (readRaw ((uk)&c, sizeof (t8)) != sizeof (t8))
			break;
		tmp.put (c);
		if (c == '\0')
			break;
	}

	char8* source = tmp.str8 ();
	u32 codePage = kCP_Default; // for legacy take default page if no utf8 bom is present...
	if (tmp.getFillSize () > 2)
	{
		if (memcmp (source, kBomUtf8, kBomUtf8Length) == 0)
		{
			codePage = kCP_Utf8;
			source += 3;
		}
	}

	if (tmp.getFillSize () > 1)
	{
#ifdef UNICODE
		ConstString::multiByteToWideString (ptr, source, nChars, codePage);
#else
		if (codePage == kCP_Utf8)
		{
			Buffer wideBuffer (tmp.getFillSize () * 3);
			ConstString::multiByteToWideString (wideBuffer.wcharPtr (), source, wideBuffer.getSize () / 2, kCP_Utf8);
			ConstString::wideStringToMultiByte (ptr, wideBuffer.wcharPtr (), nChars);		
		}
		else
		{
			memcpy (ptr, source, Min<TSize> (nChars, tmp.getFillSize ()));
		}
#endif
	}

	ptr[nChars - 1] = 0;
	return ConstString (ptr).length ();
}

//------------------------------------------------------------------------
b8 FStreamer::writeStr8 (const char8* s)
{
	i32 length = (s) ? (i32) strlen (s) + 1 : 0;
	if (!writeInt32 (length))
		return false;

	if (length > 0)
		return writeRaw (s, sizeof (char8) * length) == static_cast<TSize>(sizeof (char8) * length);

	return true;
}

//------------------------------------------------------------------------
i32 FStreamer::getStr8Size (const char8* s) 
{
	return sizeof (i32) + (i32)strlen (s) + 1;
}

//------------------------------------------------------------------------
char8* FStreamer::readStr8 ()
{
	i32 length;
	if (!readInt32 (length))
		return nullptr;
	
	// check corruption
	if (length > 262144)
		return nullptr;

	char8* s = (length > 0) ? NEWSTR8 (length) : nullptr;
	if (s)
		readRaw (s, length * sizeof (char8));
	return s;
}

//------------------------------------------------------------------------
b8 FStreamer::skip (u32 bytes)
{
    i8 tmp;
	while (bytes-- > 0) 
	{
		if (readInt8 (tmp) == false)
			return false;	
    }
	return true;
}

//------------------------------------------------------------------------
b8 FStreamer::pad (u32 bytes)
{
    while (bytes-- > 0) 
	{
		if (writeInt8 (0) == false)
			return false;	
	}
	return true;
}

//------------------------------------------------------------------------
} // namespace Steinberg
