//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/funknown.cpp
// Created by  : Steinberg, 01/2004
// Description : Basic Interface
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "funknown.h"

#include "fstrdefs.h"

#include <cstdio>

#if SMTG_OS_WINDOWS
#include <objbase.h>
#endif

#if SMTG_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>

#if !defined (SMTG_USE_STDATOMIC_H)
#if defined(MAC_OS_X_VERSION_10_11) && defined(MAC_OS_X_VERSION_MIN_REQUIRED)
#define SMTG_USE_STDATOMIC_H (MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_11)
#else
#define SMTG_USE_STDATOMIC_H 0
#endif
#endif // !defined (SMTG_USE_STDATOMIC_H)

#if !SMTG_USE_STDATOMIC_H
#include <libkern/OSAtomic.h>
#if defined(__GNUC__) && (__GNUC__ >= 4) && !__LP64__
// on 32 bit Mac OS X we can safely ignore the format warnings as sizeof(i32) == sizeof(i64)
#pragma GCC diagnostic ignored "-Wformat"
#endif 
#endif // !SMTG_USE_STDATOMIC_H
#endif // SMTG_OS_MACOS

#if SMTG_OS_LINUX
#if !defined (SMTG_USE_STDATOMIC_H)
#if defined (__ANDROID__) || defined(_LIBCPP_VERSION)
#define SMTG_USE_STDATOMIC_H 1
#else
#include <ext/atomicity.h>
#endif
#endif // !defined (SMTG_USE_STDATOMIC_H)
#include <stdlib.h>
#endif

#if defined (SMTG_USE_STDATOMIC_H) && SMTG_USE_STDATOMIC_H 
#include <stdatomic.h>
#endif

namespace Steinberg {

//------------------------------------------------------------------------
#if COM_COMPATIBLE
#if SMTG_OS_WINDOWS
#define GuidStruct GUID
#else
struct GuidStruct
{
	u32 Data1;
	u16 Data2;
	u16 Data3;
	u8 Data4[8];
};
#endif
#endif

static z0 toString8 (char8* string, tukk data, i32 i1, i32 i2);
static z0 fromString8 (const char8* string, tuk data, i32 i1, i32 i2);
static u32 makeLong (u8 b1, u8 b2, u8 b3, u8 b4);

//------------------------------------------------------------------------
//  FUnknownPrivate
//------------------------------------------------------------------------
namespace FUnknownPrivate {
//------------------------------------------------------------------------
i32 PLUGIN_API atomicAdd (i32& var, i32 d)
{
#if SMTG_USE_STDATOMIC_H
	return atomic_fetch_add (reinterpret_cast<atomic_int_least32_t*> (&var), d) + d;
#else
#if SMTG_OS_WINDOWS
#ifdef __MINGW32__
	return InterlockedExchangeAdd (reinterpret_cast<i64 volatile*>(&var), d) + d;
#else
	return InterlockedExchangeAdd ((LONG*)&var, d) + d;
#endif
#elif SMTG_OS_MACOS
	return OSAtomicAdd32Barrier (d, (i32*)&var);
#elif defined(__ANDROID__)
	return atomic_fetch_add ((atomic_int*)&var, d) + d;
#elif SMTG_OS_LINUX
	__gnu_cxx::__atomic_add (&var, d);
	return var;
#else
#warning implement me!
	var += d;
	return var;
#endif
#endif
}
} // FUnknownPrivate

//------------------------------------------------------------------------
//	FUID implementation
//------------------------------------------------------------------------

FUID::FUID ()
{
	memset (data, 0, sizeof (TUID));
}

//------------------------------------------------------------------------
FUID::FUID (u32 l1, u32 l2, u32 l3, u32 l4)
{
	from4Int (l1, l2, l3, l4);
}

//------------------------------------------------------------------------
FUID::FUID (const FUID& f)
{
	memcpy (data, f.data, sizeof (TUID));
}

//------------------------------------------------------------------------
#if SMTG_CPP11_STDLIBSUPPORT
FUID::FUID (FUID&& other)
{
	memcpy (data, other.data, sizeof (TUID));
}

FUID& FUID::operator= (FUID&& other)
{
	memcpy (data, other.data, sizeof (TUID));
	return *this;
}
#endif

//------------------------------------------------------------------------
b8 FUID::generate ()
{
#if SMTG_OS_WINDOWS
#if defined(_M_ARM64) || defined(_M_ARM)
	//#warning implement me!
	return false;
#else
	GUID guid;
	HRESULT hr = CoCreateGuid (&guid);
	switch (hr)
	{
		case RPC_S_OK: memcpy (data, (tuk)&guid, sizeof (TUID)); return true;

		case (HRESULT)RPC_S_UUID_LOCAL_ONLY:
		default: return false;
	}
#endif

#elif SMTG_OS_MACOS
	CFUUIDRef uuid = CFUUIDCreate (kCFAllocatorDefault);
	if (uuid)
	{
		CFUUIDBytes bytes = CFUUIDGetUUIDBytes (uuid);
		memcpy (data, (tuk)&bytes, sizeof (TUID));
		CFRelease (uuid);
		return true;
	}
	return false;

#elif SMTG_OS_LINUX
	srand ((size_t)this);
	for (i32 i = 0; i < 16; i++)
		data[i] = static_cast<u8>(rand ());
	return true;
#else
#warning implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
b8 FUID::isValid () const
{
	TUID nulluid = {0};

	return memcmp (data, nulluid, sizeof (TUID)) != 0;
}

//------------------------------------------------------------------------
FUID& FUID::operator= (const FUID& f)
{
	memcpy (data, f.data, sizeof (TUID));
	return *this;
}

//------------------------------------------------------------------------
z0 FUID::from4Int (u32 l1, u32 l2, u32 l3, u32 l4)
{
#if COM_COMPATIBLE
	data [0]  = (t8)((l1 & 0x000000FF)      );
	data [1]  = (t8)((l1 & 0x0000FF00) >>  8);
	data [2]  = (t8)((l1 & 0x00FF0000) >> 16);
	data [3]  = (t8)((l1 & 0xFF000000) >> 24);
	data [4]  = (t8)((l2 & 0x00FF0000) >> 16);
	data [5]  = (t8)((l2 & 0xFF000000) >> 24);
	data [6]  = (t8)((l2 & 0x000000FF)      );
	data [7]  = (t8)((l2 & 0x0000FF00) >>  8);
	data [8]  = (t8)((l3 & 0xFF000000) >> 24);
	data [9]  = (t8)((l3 & 0x00FF0000) >> 16);
	data [10] = (t8)((l3 & 0x0000FF00) >>  8);
	data [11] = (t8)((l3 & 0x000000FF)      );
	data [12] = (t8)((l4 & 0xFF000000) >> 24);
	data [13] = (t8)((l4 & 0x00FF0000) >> 16);
	data [14] = (t8)((l4 & 0x0000FF00) >>  8);
	data [15] = (t8)((l4 & 0x000000FF)      );
#else
	data [0]  = (t8)((l1 & 0xFF000000) >> 24);
	data [1]  = (t8)((l1 & 0x00FF0000) >> 16);
	data [2]  = (t8)((l1 & 0x0000FF00) >>  8);
	data [3]  = (t8)((l1 & 0x000000FF)      );
	data [4]  = (t8)((l2 & 0xFF000000) >> 24);
	data [5]  = (t8)((l2 & 0x00FF0000) >> 16);
	data [6]  = (t8)((l2 & 0x0000FF00) >>  8);
	data [7]  = (t8)((l2 & 0x000000FF)      );
	data [8]  = (t8)((l3 & 0xFF000000) >> 24);
	data [9]  = (t8)((l3 & 0x00FF0000) >> 16);
	data [10] = (t8)((l3 & 0x0000FF00) >>  8);
	data [11] = (t8)((l3 & 0x000000FF)      );
	data [12] = (t8)((l4 & 0xFF000000) >> 24);
	data [13] = (t8)((l4 & 0x00FF0000) >> 16);
	data [14] = (t8)((l4 & 0x0000FF00) >>  8);
	data [15] = (t8)((l4 & 0x000000FF)      );
#endif
}

//------------------------------------------------------------------------
z0 FUID::to4Int (u32& d1, u32& d2, u32& d3, u32& d4) const
{
	d1 = getLong1 ();
	d2 = getLong2 ();
	d3 = getLong3 ();
	d4 = getLong4 ();
}

//------------------------------------------------------------------------
u32 FUID::getLong1 () const
{
#if COM_COMPATIBLE
	return makeLong (data[3], data[2], data[1], data[0]);
#else
	return makeLong (data[0], data[1], data[2], data[3]);
#endif
}

//------------------------------------------------------------------------
u32 FUID::getLong2 () const
{
#if COM_COMPATIBLE
	return makeLong (data[5], data[4], data[7], data[6]);
#else
	return makeLong (data[4], data[5], data[6], data[7]);
#endif
}

//------------------------------------------------------------------------
u32 FUID::getLong3 () const
{
#if COM_COMPATIBLE
	return makeLong (data[8], data[9], data[10], data[11]);
#else
	return makeLong (data[8], data[9], data[10], data[11]);
#endif
}

//------------------------------------------------------------------------
u32 FUID::getLong4 () const
{
#if COM_COMPATIBLE
	return makeLong (data[12], data[13], data[14], data[15]);
#else
	return makeLong (data[12], data[13], data[14], data[15]);
#endif
}

//------------------------------------------------------------------------
z0 FUID::toString (char8* string) const
{
	if (!string)
		return;

#if COM_COMPATIBLE
	auto* g = (GuidStruct*)data;

	char8 s[17];
	Steinberg::toString8 (s, data, 8, 16);

	sprintf (string, "%08X%04X%04X%s", g->Data1, g->Data2, g->Data3, s);
#else
	Steinberg::toString8 (string, data, 0, 16);
#endif
}

//------------------------------------------------------------------------
b8 FUID::fromString (const char8* string)
{
	if (!string || !*string)
		return false;
	if (strlen (string) != 32)
		return false;

#if COM_COMPATIBLE
	GuidStruct g;
	t8 s[33];

	strcpy (s, string);
	s[8] = 0;
	sscanf (s, "%x", &g.Data1);
	strcpy (s, string + 8);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data2);
	strcpy (s, string + 12);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data3);

	memcpy (data, &g, 8);
	Steinberg::fromString8 (string + 16, data, 8, 16);
#else
	Steinberg::fromString8 (string, data, 0, 16);
#endif

	return true;
}

//------------------------------------------------------------------------
b8 FUID::fromRegistryString (const char8* string)
{
	if (!string || !*string)
		return false;
	if (strlen (string) != 38)
		return false;

// e.g. {c200e360-38c5-11ce-ae62-08002b2b79ef}

#if COM_COMPATIBLE
	GuidStruct g;
	char8 s[10];

	strncpy (s, string + 1, 8);
	s[8] = 0;
	sscanf (s, "%x", &g.Data1);
	strncpy (s, string + 10, 4);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data2);
	strncpy (s, string + 15, 4);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data3);
	memcpy (data, &g, 8);

	Steinberg::fromString8 (string + 20, data, 8, 10);
	Steinberg::fromString8 (string + 25, data, 10, 16);
#else
	Steinberg::fromString8 (string + 1, data, 0, 4);
	Steinberg::fromString8 (string + 10, data, 4, 6);
	Steinberg::fromString8 (string + 15, data, 6, 8);
	Steinberg::fromString8 (string + 20, data, 8, 10);
	Steinberg::fromString8 (string + 25, data, 10, 16);
#endif

	return true;
}

//------------------------------------------------------------------------
z0 FUID::toRegistryString (char8* string) const
{
// e.g. {c200e360-38c5-11ce-ae62-08002b2b79ef}

#if COM_COMPATIBLE
	auto* g = (GuidStruct*)data;

	char8 s1[5];
	Steinberg::toString8 (s1, data, 8, 10);

	char8 s2[13];
	Steinberg::toString8 (s2, data, 10, 16);

	sprintf (string, "{%08X-%04X-%04X-%s-%s}", g->Data1, g->Data2, g->Data3, s1, s2);
#else
	char8 s1[9];
	Steinberg::toString8 (s1, data, 0, 4);
	char8 s2[5];
	Steinberg::toString8 (s2, data, 4, 6);
	char8 s3[5];
	Steinberg::toString8 (s3, data, 6, 8);
	char8 s4[5];
	Steinberg::toString8 (s4, data, 8, 10);
	char8 s5[13];
	Steinberg::toString8 (s5, data, 10, 16);

	snprintf (string, 40, "{%s-%s-%s-%s-%s}", s1, s2, s3, s4, s5);
#endif
}

//------------------------------------------------------------------------
z0 FUID::print (i32 style, char8* string, size_t stringBufferSize) const
{
	if (!string || stringBufferSize == 0) // no string: debug output
	{
		char8 str[128];
		print (style, str, 128);

#if SMTG_OS_WINDOWS
		OutputDebugStringA (str);
		OutputDebugStringA ("\n");
#else
		fprintf (stdout, "%s\n", str);
#endif
		return;
	}

	u32 l1, l2, l3, l4;
	to4Int (l1, l2, l3, l4);

	switch (style)
	{
		case kINLINE_UID:
			// length is 60 (with null-terminate)
			snprintf (string, stringBufferSize, "INLINE_UID (0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1,
			          l2, l3, l4);
			break;

		case kDECLARE_UID:
			// length is 61 (with null-terminate)
			snprintf (string, stringBufferSize, "DECLARE_UID (0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1,
			          l2, l3, l4);
			break;

		case kFUID:
			// length is 54 (with null-terminate)
			snprintf (string, stringBufferSize, "FUID (0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1, l2, l3,
			          l4);
			break;

		case kCLASS_UID:
		default:
			// length is 78 (with null-terminate)
			snprintf (string, stringBufferSize,
			          "DECLARE_CLASS_IID (Interface, 0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1, l2, l3,
			          l4);
			break;
	}
}

//------------------------------------------------------------------------
//  helpers
//------------------------------------------------------------------------
static u32 makeLong (u8 b1, u8 b2, u8 b3, u8 b4)
{
	return (u32 (b1) << 24) | (u32 (b2) << 16) | (u32 (b3) << 8) | u32 (b4);
}

//------------------------------------------------------------------------
static z0 toString8 (char8* string, tukk data, i32 i1, i32 i2)
{
	*string = 0;
	for (i32 i = i1; i < i2; i++)
	{
		char8 s[3];
		snprintf (s, 3, "%02X", (u8)data[i]);
		strcat (string, s);
	}
}

//------------------------------------------------------------------------
static z0 fromString8 (const char8* string, tuk data, i32 i1, i32 i2)
{
	for (i32 i = i1; i < i2; i++)
	{
		char8 s[3];
		s[0] = *string++;
		s[1] = *string++;
		s[2] = 0;

		i32 d = 0;
		sscanf (s, "%2x", &d);
		data[i] = (t8)d;
	}
}

//------------------------------------------------------------------------
} // namespace Steinberg
