//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fbuffer.h
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

#pragma once

#include "pluginterfaces/base/ftypes.h"
#include <cstring> 

namespace Steinberg {
class Txt;

//------------------------------------------------------------------------
/** Buffer.
@ingroup adt

A Buffer is an object-oriented wrapper for a piece of memory.
It adds several utility functions, e.g. for managing the size of the Buffer,
appending or prepending values or strings to it.
Internally it uses the standard memory functions malloc(), free(), etc. */
//------------------------------------------------------------------------
class Buffer
{
public:
//---------------------------------------------------------------------
	
	/**	Default constructor, allocates no memory at all.
	*/
	Buffer ();

	/**	Constructor - creates a new Buffer with a given size and copies contents from optional memory pointer.
	\param[in] b : optional memory pointer with the size of at least the given size
	\param[in] size : the size of the new Buffer to be allocated, in bytes.
	*/
	Buffer (ukk b, u32 size);

	/**	Constructor - creates a new Buffer with a given size and fills it all with a given value.
	\param[in] size : the size of the new Buffer to be allocated, in bytes.
	\param[in] initVal : the initial value the Buffer will be completely filled with
	*/
	Buffer (u32 size, u8 initVal);

	/**	Constructor - creates a new Buffer with a given size.
	\param[in] size : the size of the new Buffer to be allocated, in bytes.
	*/
	Buffer (u32 size);

	/**	Copy constructor - creates a new Buffer from a given Buffer.
	\param[in] buff : the Buffer from which all memory will be copied to the new one
	*/
	Buffer (const Buffer& buff);

	/**	Destructor - deallocates the internal memory.
	*/
	virtual ~Buffer ();
	
	/**	Assignment operator - copies contents from a given Buffer and increases the size if necessary.
	\param[in] buff : the Buffer from which all memory will be copied
	*/
	z0 operator = (const Buffer& buff);
	
	/**	Comparison operator - copies contents from a given Buffer and increases the size if necessary.
	\param[in] buff : the Buffer to be compared to
	\return true, if the given Buffer's content is equal to this one, else false
	*/
	b8 operator == (const Buffer& buff)const;

	u32 getSize () const {return memSize;}		///< \return the actual size of the Buffer's memory, in bytes.

	/**	Sets a new size for this Buffer, keeping as much content as possible.
	\param[in] newSize : the new size for the Buffer, in bytes, newSize maybe zero
	\return true, if the new size could be adapted, else false
	*/
	b8 setSize (u32 newSize);

	/**	Increases the Buffer to the next block, block size given by delta.
	\param[in] memSize : the new minimum size of the Buffer, newSize maybe zero
	\return true, if the Buffer could be grown successfully, else false
	*/
	b8 grow (u32 memSize);
	b8 setMaxSize (u32 size) {return grow (size);}	///< see \ref grow()

	z0 fillup (u8 initVal = 0);				///< set from fillSize to end
	u32 getFillSize ()const {return fillSize;}	///< \return the actual fill size
	b8 setFillSize (u32 c);					///< sets a new fill size, does not change any memory
	inline z0 flush () {setFillSize (0);}			///< sets fill size to zero
	b8 truncateToFillSize ();						///< \return always true, truncates the size of the Buffer to the actual fill size

	b8 isFull () const { return (fillSize == memSize); }	///< \return true, if all memory is filled up, else false
	u32 getFree () const { return (memSize - fillSize); }///< \return remaining memory

	inline z0 shiftStart (i32 amount) {return shiftAt (0, amount);} ///< moves all memory by given amount, grows the Buffer if necessary
	z0 shiftAt (u32 position, i32 amount);						///< moves memory starting at the given position
	z0 move (i32 amount, u8 initVal = 0);						///< shifts memory at start without growing the buffer, so data is lost and initialized with init val

	b8 copy (u32 from, u32 to, u32 bytes);	///< copies a number of bytes from one position to another, the size may be adapted
	u32 get (uk b, u32 size);					///< copy to buffer from fillSize, and shift fillSize

	z0 setDelta (u32 d) {delta = d;}				///< define the block size by which the Buffer grows, see \ref grow()

	b8 put (u8);							///< append value at end, grows Buffer if necessary 
	b8 put (char16 c);                        ///< append value at end, grows Buffer if necessary
	b8 put (t8 c);							///< append value at end, grows Buffer if necessary
	b8 put (ukk , u32 size);		///< append bytes from a given buffer, grows Buffer if necessary
	b8 put (uk , u32 size);				///< append bytes from a given buffer, grows Buffer if necessary
	b8 put (u8* , u32 size);			///< append bytes from a given buffer, grows Buffer if necessary
	b8 put (char8* , u32 size);			///< append bytes from a given buffer, grows Buffer if necessary
	b8 put (u8k* , u32 size);		///< append bytes from a given buffer, grows Buffer if necessary
	b8 put (const char8* , u32 size);		///< append bytes from a given buffer, grows Buffer if necessary
	b8 put (const Txt&);					///< append Txt at end, grows Buffer if necessary

	z0 set (u8 value);		///< fills complete Buffer with given value
	
	// strings ----------------
	b8 appendString (const tchar* s);
	b8 appendString (tchar* s);
	b8 appendString (tchar c)                   { return put (c); }

	b8 appendString8 (const char8* s);	
	b8 appendString16 (const char16* s);

	b8 appendString8 (char8* s)                 { return appendString8 ((const char8*)s); }
	b8 appendString8 (u8* s)		  { return appendString8 ((const char8*)s); }         
	b8 appendString8 (u8k* s)   { return appendString8 ((const char8*)s); }

	b8 appendString8 (char8 c)                  { return put ((u8)c); }
	b8 appendString8 (u8 c)          { return put (c); }
	b8 appendString16 (char16 c)                { return put (c); }
	b8 appendString16 (char16* s)               { return appendString16 ((const char16*)s); }

	b8 prependString (const tchar* s);   
	b8 prependString (tchar* s);   
	b8 prependString (tchar c);                

	b8 prependString8 (const char8* s);           
	b8 prependString16 (const char16* s);

	b8 prependString8 (char8 c);
	b8 prependString8 (u8 c)         { return prependString8 ((char8)c); }
	b8 prependString8 (char8* s)                { return prependString8 ((const char8*)s); }
	b8 prependString8 (u8* s)        { return prependString8((const char8*)s); }           
	b8 prependString8 (u8k* s)  { return prependString8 ((const char8*)s); } 
	b8 prependString16 (char16 c);
	b8 prependString16 (char16* s)              { return prependString16 ((const char16*)s); }

	b8 operator+= (tukk s)               { return appendString8 (s); }
	b8 operator+= (t8 c)                      { return appendString8 (c); }
	b8 operator+= (const char16* s)             { return appendString16 (s); }
	b8 operator+= (char16 c)                    { return appendString16 (c); }

	b8 operator= (tukk s)                { flush (); return appendString8 (s); }
	b8 operator= (const char16* s)              { flush (); return appendString16 (s); }
	b8 operator= (char8 c)                      { flush (); return appendString8 (c); }
	b8 operator= (char16 c)                     { flush (); return appendString16 (c); }

	z0 endString () {put (tchar (0));}
	z0 endString8 () {put (char8 (0));}
	z0 endString16 () {put (char16 (0));}

	b8 makeHexString (Txt& result);
	b8 fromHexString (const char8* string);

	// conversion
	operator uk () const { return (uk)buffer; }				///< conversion
	inline tchar*   str ()   const {return (tchar*)buffer;}			///< conversion
	inline char8*   str8 ()   const {return (char8*)buffer;}		///< conversion
	inline char16*  str16 ()   const {return (char16*)buffer;}		///< conversion
	inline i8*   int8Ptr ()   const {return (i8*)buffer;}		///< conversion
	inline u8*  uint8Ptr ()  const {return (u8*)buffer; }		///< conversion
	inline i16*  int16Ptr ()  const {return (i16*)buffer; }		///< conversion
    inline u16* u16Ptr () const {return (u16*)buffer; }	///< conversion
	inline i32*  int32Ptr ()  const {return (i32*)buffer; }		///< conversion
	inline u32* uint32Ptr () const {return (u32*)buffer; }	///< conversion
	inline f32*  floatPtr ()  const {return (f32*)buffer; }		///< conversion
	inline f64* doublePtr () const {return (f64*)buffer; }	///< conversion
	inline char16*  wcharPtr ()  const {return (char16*)buffer;}	///< conversion

	i8* operator + (u32 i);	///< \return the internal Buffer's address plus the given offset i, zero if offset is out of range
	
	i32 operator ! ()  { return buffer == nullptr; }
	
	enum swapSize 
	{
		kSwap16 = 2,
		kSwap32 = 4,
		kSwap64 = 8
	};
	b8 swap (i16 swapSize);											///< swap all bytes of this Buffer by the given swapSize
	static b8 swap (uk buffer, u32 bufferSize, i16 swapSize);	///< utility, swap given number of bytes in given buffer by the given swapSize

	z0 take (Buffer& from);	///< takes another Buffer's memory, frees the current Buffer's memory
	i8* pass ();				///< pass the current Buffer's memory

	/**	Converts a Buffer's content to UTF-16 from a given multi-byte code page, Buffer must contain char8 of given encoding.
		\param[in] sourceCodePage : the actual code page of the Buffer's content
		\return true, if the conversion was successful, else false
	*/
	virtual b8 toWideString (i32 sourceCodePage); // Buffer contains char8 of given encoding -> utf16

	/**	Converts a Buffer's content from UTF-16 to a given multi-byte code page, Buffer must contain UTF-16 encoded characters.
		\param[in] destCodePage : the desired code page to convert the Buffer's content to
		\return true, if the conversion was successful, else false
	*/
	virtual b8 toMultibyteString (i32 destCodePage); // Buffer contains utf16 -> char8 of given encoding

//------------------------------------------------------------------------
protected:
	static u32k defaultDelta = 0x1000; // 0x1000
	
	i8* buffer;
	u32 memSize;
	u32 fillSize;
	u32 delta;
};

inline b8 Buffer::put (uk p, u32 count)     { return put ((ukk)p , count ); }
inline b8 Buffer::put (u8 * p, u32 count)   { return put ((ukk)p , count ); }
inline b8 Buffer::put (char8* p, u32 count)    { return put ((ukk)p , count ); }
inline b8 Buffer::put (u8k* p, u32 count) { return put ((ukk)p , count ); }
inline b8 Buffer::put (const char8* p, u32 count) { return put ((ukk)p , count ); }

//------------------------------------------------------------------------
inline b8 Buffer::appendString (const tchar* s)
{
#ifdef UNICODE
	return appendString16 (s);
#else
	return appendString8 (s);
#endif
}

//------------------------------------------------------------------------
inline b8 Buffer::appendString (tchar* s)
{
#ifdef UNICODE
	return appendString16 (s);
#else
	return appendString8 (s);
#endif
}

//------------------------------------------------------------------------
inline b8 Buffer::prependString (const tchar* s)
{
#ifdef UNICODE
	return prependString16 (s);
#else
	return prependString8 (s);
#endif
}

//------------------------------------------------------------------------
inline b8 Buffer::prependString (tchar* s)
{
#ifdef UNICODE
	return prependString16 (s);
#else
	return prependString8 (s);
#endif
}

//------------------------------------------------------------------------
inline b8 Buffer::prependString (tchar c)
{
#ifdef UNICODE
	return prependString16 (c);
#else
	return prependString8 (c);
#endif
}

//------------------------------------------------------------------------
} // namespace Steinberg
