//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fstreamer.h
// Created by  : Steinberg, 12/2005
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

#pragma once

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {

//------------------------------------------------------------------------
enum FSeekMode
{
	kSeekSet,
	kSeekCurrent,
	kSeekEnd
};

//------------------------------------------------------------------------
// FStreamer
//------------------------------------------------------------------------
/** Byteorder-aware base class for typed stream i/o. */
//------------------------------------------------------------------------
class FStreamer
{
public:
//------------------------------------------------------------------------
	FStreamer (i16 byteOrder = BYTEORDER);
	virtual ~FStreamer () {}

	/** @name Implementing class must override. */
	///@{
	virtual TSize readRaw (uk, TSize) = 0;          ///< Read one buffer of size.
	virtual TSize writeRaw (ukk, TSize) = 0;   ///< Write one buffer of size.
	virtual z64 seek (z64, FSeekMode) = 0;         ///< Set file position for stream.
	virtual z64 tell () = 0;                         ///< Return current file position.
	///@}

	/** @name Streams are byteOrder aware. */
	///@{
	inline z0 setByteOrder (i32 e) { byteOrder = (i16)e; }
	inline i32 getByteOrder () const { return byteOrder; }
	///@}

	/** @name read and write i8 and t8. */
	///@{
	b8 writeChar8 (char8); 
	b8 readChar8 (char8&);                         
	b8 writeUChar8 (u8);   
	b8 readUChar8 (u8&);
	b8 writeChar16 (char16 c); 
	b8 readChar16 (char16& c); 

	b8 writeInt8 (i8 c);   
	b8 readInt8 (i8& c);
	b8 writeInt8u (u8 c);   
	b8 readInt8u (u8& c);
	///@}

	/** @name read and write i16. */
	///@{
	b8 writeInt16 (i16);
	b8 readInt16 (i16&);
	b8 writeInt16Array (i16k* array, i32 count);  
	b8 readInt16Array (i16* array, i32 count);  
	b8 writeInt16u (u16);
	b8 readInt16u (u16&);
	b8 writeInt16uArray (u16k* array, i32 count);  
	b8 readInt16uArray (u16* array, i32 count);  
	///@}

	/** @name read and write i32. */
	///@{
	b8 writeInt32 (i32);  
	b8 readInt32 (i32&);  
	b8 writeInt32Array (const i32* array, i32 count);  
	b8 readInt32Array (i32* array, i32 count);  
	b8 writeInt32u (u32);
	b8 readInt32u (u32&); 
	b8 writeInt32uArray (u32k* array, i32 count);  
	b8 readInt32uArray (u32* array, i32 count);  
	///@}

	/** @name read and write z64. */
	///@{
	b8 writeInt64 (z64);
	b8 readInt64 (z64&);
	b8 writeInt64Array (const z64* array, i32 count);  
	b8 readInt64Array (z64* array, i32 count);  
	b8 writeInt64u (zu64);
	b8 readInt64u (zu64&);
	b8 writeInt64uArray (const zu64* array, i32 count);  
	b8 readInt64uArray (zu64* array, i32 count);  
	///@}

	/** @name read and write f32 and f32 array. */
	///@{
	b8 writeFloat (f32);
	b8 readFloat (f32&);
	b8 writeFloatArray (const f32* array, i32 count);  
	b8 readFloatArray (f32* array, i32 count);  
	///@}

	/** @name read and write f64 and f64 array. */
	///@{
	b8 writeDouble (f64);                         
	b8 readDouble (f64&);                         
	b8 writeDoubleArray (const f64* array, i32 count);  
	b8 readDoubleArray (f64* array, i32 count);  
	///@}

	/** @name read and write Boolean. */
	///@{
	b8 writeBool (b8);                                   ///< Write one boolean
	b8 readBool (b8&);                                   ///< Read one b8.
	///@}

	/** @name read and write Strings. */
	///@{
	TSize writeString8 (const char8* ptr, b8 terminate = false); ///< a direct output function writing only one string (ascii 8bit)
	TSize readString8 (char8* ptr, TSize size);				///< a direct input function reading only one string (ascii) (ended by a \n or \0 or eof)
	
	b8 writeStr8 (const char8* ptr);				       ///< write a string length (strlen) and string itself
	char8* readStr8 ();									   ///< read a string length and string text (The return string must be deleted when use is finished)

	static i32 getStr8Size (const char8* ptr);	       ///< returns the size of a saved string

	b8 writeStringUtf8 (const tchar* ptr);               ///< always terminated, converts to utf8 if non ascii characters are in string
	i32 readStringUtf8 (tchar* ptr, i32 maxSize);      ///< read a UTF8 string
	///@}

	b8 skip (u32 bytes);
	b8 pad (u32 bytes);


//------------------------------------------------------------------------
protected:
	i16 byteOrder;
};


//------------------------------------------------------------------------
/** FStreamSizeHolder Declaration
	remembers size of stream chunk for backward compatibility.
	
	<b>Example:</b>
	@code
	externalize (a)
	{
		FStreamSizeHolder sizeHolder;
		sizeHolder.beginWrite ();	// sets start mark, writes dummy size
		a << ....
		sizeHolder.endWrite ();		// jumps to start mark, updates size, jumps back here
	}

	internalize (a)
	{
		FStreamSizeHolder sizeHolder;
		sizeHolder.beginRead ();	// reads size, mark
		a >> ....
		sizeHolder.endRead ();		// jumps forward if new version has larger size
	}
	@endcode
*/
//------------------------------------------------------------------------
class FStreamSizeHolder
{
public:
	FStreamSizeHolder (FStreamer &s);

	z0 beginWrite ();	///< remembers position and writes 0
	i32 endWrite ();	///< writes and returns size (since the start marker)
	i32 beginRead ();	///< returns size
	z0 endRead ();	///< jump to end of chunk

protected:
	FStreamer &stream;
	z64 sizePos;
};

class IBStream;

//------------------------------------------------------------------------
// IBStreamer
//------------------------------------------------------------------------
/** Wrapper class for typed reading/writing from or to IBStream. 
	Can be used framework-independent in plug-ins. */
//------------------------------------------------------------------------
class IBStreamer: public FStreamer
{
public:
//------------------------------------------------------------------------
	/** Constructor for a given IBSTream and a byteOrder. */
	IBStreamer (IBStream* stream, i16 byteOrder = BYTEORDER);

	IBStream* getStream () { return stream; }	///< Returns the associated IBStream.

	// FStreamer overrides:					
	TSize readRaw (uk, TSize) SMTG_OVERRIDE;				///< Read one buffer of size.
	TSize writeRaw (ukk, TSize) SMTG_OVERRIDE;		///< Write one buffer of size.
	z64 seek (z64, FSeekMode) SMTG_OVERRIDE;			///< Set file position for stream.
	z64 tell () SMTG_OVERRIDE;							///< Return current file position.
//------------------------------------------------------------------------
protected:
	IBStream* stream;
};

//------------------------------------------------------------------------
} // namespace Steinberg
