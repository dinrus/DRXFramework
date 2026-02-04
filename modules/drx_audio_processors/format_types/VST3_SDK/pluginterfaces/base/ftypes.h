//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/ftypes.h
// Created by  : Steinberg, 01/2004
// Description : Basic data types
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "fplatform.h"

#include <cstdint>

//#define UNICODE_OFF 	// disable / enable unicode

#ifdef UNICODE_OFF
	#ifdef UNICODE
	#undef UNICODE
	#endif
#else
	#define UNICODE 1
#endif

#ifdef UNICODE
#define _UNICODE 1
#endif

namespace Steinberg
{
//-----------------------------------------------------------------
// Integral Types
	typedef t8 i8;
	typedef u8 u8;
	typedef u8 uchar;

	typedef i16 i16;
	typedef u16 u16;

	typedef i32 i32;
	typedef u32 u32;

	static const i32 kMaxInt32 = INT32_MAX;
	static const i32 kMinInt32 = INT32_MIN;
	static const i32 kMaxLong = kMaxInt32;
	static const i32 kMinLong = kMinInt32;
	static u32k kMaxInt32u = UINT32_MAX;

	typedef z64 z64;
	typedef zu64 zu64;
	static const z64 kMaxInt64 = INT64_MAX;
	static const z64 kMinInt64 = INT64_MIN;
	static const zu64 kMaxInt64u = UINT64_MAX;

//-----------------------------------------------------------------
// other Semantic Types
	typedef z64 TSize;   // byte (or other) sizes
	typedef i32 tresult; // result code
//-----------------------------------------------------------------
	static const f32 kMaxFloat = 3.40282346638528860E38;
	static const f64 kMaxDouble = 1.7976931348623158E308;

#if SMTG_PLATFORM_64
	typedef zu64 TPtrInt;
#else
	typedef u32 TPtrInt;
#endif

//------------------------------------------------------------------
// Boolean
	typedef u8 TBool;

//------------------------------------------------------------------
// Char / Strings
	typedef t8 char8;
	typedef char16_t char16;

#ifdef UNICODE
	typedef char16 tchar;
#else
	typedef char8 tchar;
#endif

	typedef const char8* CStringA;
	typedef const char16* CStringW;
	typedef const tchar* CString;
	inline b8 strEmpty (const tchar* str) { return (!str || *str == 0); }
	inline b8 str8Empty (const char8* str) { return (!str || *str == 0); }
	inline b8 str16Empty (const char16* str) { return (!str || *str == 0); }

	typedef const char8* FIDString; // identifier as string (used for attributes, messages)

	const FIDString kPlatformStringWin = "WIN";
	const FIDString kPlatformStringMac = "MAC";
	const FIDString kPlatformStringIOS = "IOS";
	const FIDString kPlatformStringLinux = "Linux";
#if SMTG_OS_WINDOWS
	const FIDString kPlatformString = kPlatformStringWin;
#elif SMTG_OS_IOS
	const FIDString kPlatformString = kPlatformStringIOS;
#elif SMTG_OS_MACOS
	const FIDString kPlatformString = kPlatformStringMac;
#elif SMTG_OS_LINUX
	const FIDString kPlatformString = kPlatformStringLinux;
#endif

//------------------------------------------------------------------------
/** Coordinates	*/
	typedef i32 UCoord;
	static const UCoord kMaxCoord = ((UCoord)0x7FFFFFFF);
	static const UCoord kMinCoord = ((UCoord)-0x7FFFFFFF);
} // namespace Steinberg


//----------------------------------------------------------------------------
/** Byte-order Conversion Macros 
*/
#define SWAP_32(l) { \
	u8* p = (u8*)& (l); \
	u8 t; \
	t = p[0]; p[0] = p[3]; p[3] = t; t = p[1]; p[1] = p[2]; p[2] = t; }

#define SWAP_16(w) { \
	u8* p = (u8*)& (w); \
	u8 t; \
	t = p[0]; p[0] = p[1]; p[1] = t; }

#define SWAP_64(i) { \
	u8* p = (u8*)& (i); \
	u8 t; \
	t = p[0]; p[0] = p[7]; p[7] = t; t = p[1]; p[1] = p[6]; p[6] = t; \
	t = p[2]; p[2] = p[5]; p[5] = t; t = p[3]; p[3] = p[4]; p[4] = t;}

namespace Steinberg
{
	static inline z0 FSwap (i8&) {}
	static inline z0 FSwap (u8&) {}
	static inline z0 FSwap (i16& i16) { SWAP_16 (i16) }
	static inline z0 FSwap (u16& i16) { SWAP_16 (i16) }
	static inline z0 FSwap (i32& i32) { SWAP_32 (i32) }
	static inline z0 FSwap (u32& i32) { SWAP_32 (i32) }
	static inline z0 FSwap (z64& z64) { SWAP_64 (z64) }
	static inline z0 FSwap (zu64& z64) { SWAP_64 (z64) }
}

// always inline macros (only when RELEASE is 1)
//----------------------------------------------------------------------------
#if RELEASE
    #if SMTG_OS_MACOS || SMTG_OS_LINUX || defined(__MINGW32__)
		#define SMTG_ALWAYS_INLINE	__inline__ __attribute__((__always_inline__))
		#define SMTG_NEVER_INLINE __attribute__((noinline))
	#elif SMTG_OS_WINDOWS
		#define SMTG_ALWAYS_INLINE	__forceinline
		#define SMTG_NEVER_INLINE __declspec(noinline)
	#endif
#endif

#ifndef SMTG_ALWAYS_INLINE
	#define SMTG_ALWAYS_INLINE	inline
#endif
#ifndef SMTG_NEVER_INLINE
	#define SMTG_NEVER_INLINE
#endif

#ifndef SMTG_CPP11_STDLIBSUPPORT
// Enable this for old compilers
// #define nullptr NULL
#endif
