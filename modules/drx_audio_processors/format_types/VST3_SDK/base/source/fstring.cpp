//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fstring.cpp
// Created by  : Steinberg, 2008
// Description : Txt class
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

#include "base/source/fstring.h"
#include "base/source/fdebug.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/fvariant.h"

#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <utility>
#include <complex>
#include <cmath>
#include <algorithm>
#include <cassert>

#if SMTG_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning (disable : 4244)
#pragma warning (disable : 4267)
#pragma warning (disable : 4996)

#if DEVELOPMENT
#include <crtdbg.h>

#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p,s) _realloc_dbg(p,s,  _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p) _free_dbg(p, _NORMAL_BLOCK)

#endif // DEVELOPMENT
#endif // _MSC_VER
#endif // SMTG_OS_WINDOWS

#ifndef kPrintfBufferSize
#define kPrintfBufferSize 4096
#endif

#if SMTG_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>
#include <wchar.h>

#if defined (__GNUC__) && (__GNUC__ >= 4) && !__LP64__
// on 32 bit Mac OS X we can safely ignore the format warnings as sizeof(i32) == sizeof(i64)
#pragma GCC diagnostic ignored "-Wformat"
#endif

#define SMTG_ENABLE_DEBUG_CFALLOCATOR 0
#define SMTG_DEBUG_CFALLOCATOR	(DEVELOPMENT && SMTG_ENABLE_DEBUG_CFALLOCATOR)

#if SMTG_DEBUG_CFALLOCATOR
#include <libkern/OSAtomic.h>
#include <dlfcn.h>
#endif

namespace Steinberg {
#if SMTG_DEBUG_CFALLOCATOR
static CFAllocatorRef kCFAllocator = NULL;

struct CFStringDebugAllocator : CFAllocatorContext
{
	CFStringDebugAllocator ()
	{
		version = 0;
		info = this;
		retain = nullptr;
		release = nullptr;
		copyDescription = nullptr;
		allocate = allocateCallBack;
		reallocate = reallocateCallBack;
		deallocate = deallocateCallBack;
		preferredSize = preferredSizeCallBack;

		numAllocations = allocationSize = numDeallocations = 0;
		cfAllocator = CFAllocatorCreate (kCFAllocatorUseContext, this);

		Dl_info info;
		if (dladdr ((ukk)CFStringDebugAllocator::allocateCallBack, &info))
		{
			moduleName = info.dli_fname;
		}
		kCFAllocator = cfAllocator;
	}

	~CFStringDebugAllocator ()
	{
		kCFAllocator = kCFAllocatorDefault;
		CFRelease (cfAllocator);
		FDebugPrint ("CFStringDebugAllocator (%s):\n", moduleName.text8 ());
		FDebugPrint ("\tNumber of allocations  : %u\n", numAllocations);
		FDebugPrint ("\tNumber of deallocations: %u\n", numDeallocations);
		FDebugPrint ("\tAllocated Bytes        : %u\n", allocationSize);
	}

	Txt moduleName;
	CFAllocatorRef cfAllocator;
	volatile z64 numAllocations;
	volatile z64 numDeallocations;
	volatile z64 allocationSize;

	uk doAllocate (CFIndex allocSize, CFOptionFlags hint)
	{
		uk ptr = CFAllocatorAllocate (kCFAllocatorDefault, allocSize, hint);
		OSAtomicIncrement64 (&numAllocations);
		OSAtomicAdd64 (allocSize, &allocationSize);
		return ptr;
	}
	uk doReallocate (uk ptr, CFIndex newsize, CFOptionFlags hint)
	{
		uk newPtr = CFAllocatorReallocate (kCFAllocatorDefault, ptr, newsize, hint);
		return newPtr;
	}
	z0 doDeallocate (uk ptr)
	{
		CFAllocatorDeallocate (kCFAllocatorDefault, ptr);
		OSAtomicIncrement64 (&numDeallocations);
	}
	CFIndex getPreferredSize (CFIndex size, CFOptionFlags hint)
	{
		return CFAllocatorGetPreferredSizeForSize (kCFAllocatorDefault, size, hint);
	}

	static uk allocateCallBack (CFIndex allocSize, CFOptionFlags hint, uk info)
	{
		return static_cast<CFStringDebugAllocator*> (info)->doAllocate (allocSize, hint);
	}
	static uk reallocateCallBack (uk ptr, CFIndex newsize, CFOptionFlags hint, uk info)
	{
		return static_cast<CFStringDebugAllocator*> (info)->doReallocate (ptr, newsize, hint);
	}

	static z0 deallocateCallBack (uk ptr, uk info)
	{
		static_cast<CFStringDebugAllocator*> (info)->doDeallocate (ptr);
	}
	static CFIndex preferredSizeCallBack (CFIndex size, CFOptionFlags hint, uk info)
	{
		return static_cast<CFStringDebugAllocator*> (info)->getPreferredSize (size, hint);
	}
};
static CFStringDebugAllocator gDebugAllocator;
#else

static const CFAllocatorRef kCFAllocator = ::kCFAllocatorDefault;
#endif // SMTG_DEBUG_CFALLOCATOR
}

//-----------------------------------------------------------------------------
static uk toCFStringRef (const Steinberg::char8* source, Steinberg::u32 encoding)
{
	if (encoding == 0xFFFF)
		encoding = kCFStringEncodingASCII;
	if (source)
		return (uk)CFStringCreateWithCString (Steinberg::kCFAllocator, source, encoding);
	else
		return (uk)CFStringCreateWithCString (Steinberg::kCFAllocator, "", encoding);
}

//-----------------------------------------------------------------------------
static b8 fromCFStringRef (Steinberg::char8* dest, Steinberg::i32 destSize, ukk cfStr, Steinberg::u32 encoding)
{
	CFIndex usedBytes;
	CFRange range = {0, CFStringGetLength ((CFStringRef)cfStr)};
	b8 result = CFStringGetBytes ((CFStringRef)cfStr, range, encoding, '?', false, (UInt8*)dest, destSize, &usedBytes);
	dest[usedBytes] = 0;
	return result;
}
#endif // SMTG_OS_MACOS

#if SMTG_OS_WINDOWS
//-----------------------------------------------------------------------------
static inline i32 stricmp16 (const Steinberg::tchar* s1, const Steinberg::tchar* s2)
{
	return wcsicmp (Steinberg::wscast (s1), Steinberg::wscast (s2));
}

//-----------------------------------------------------------------------------
static inline i32 strnicmp16 (const Steinberg::tchar* s1, const Steinberg::tchar* s2, size_t l)
{
	return wcsnicmp (Steinberg::wscast (s1), Steinberg::wscast (s2), l);
}

//-----------------------------------------------------------------------------
static inline i32 vsnwprintf (Steinberg::char16* buffer, size_t bufferSize,
                              const Steinberg::char16* format, va_list args)
{
	return _vsnwprintf (Steinberg::wscast (buffer), bufferSize, Steinberg::wscast (format), args);
}

//-----------------------------------------------------------------------------
static inline Steinberg::i32 sprintf16 (Steinberg::char16* str, const Steinberg::char16* format, ...)
{
	va_list marker;
	va_start (marker, format);
	return vsnwprintf (str, static_cast<size_t> (-1), format, marker);
}

#elif SMTG_OS_LINUX
#include <codecvt>
#include <locale>
#include <cstring>
#include <string>
#include <limits>
#include <cassert>
#include <wchar.h>

using ConverterFacet = std::codecvt_utf8_utf16<char16_t>;
using Converter = std::wstring_convert<ConverterFacet, char16_t>;

//------------------------------------------------------------------------
static ConverterFacet& converterFacet ()
{
	static ConverterFacet gFacet;
	return gFacet;
}

//------------------------------------------------------------------------
static Converter& converter ()
{
	static Converter gConverter;
	return gConverter;
}

//-----------------------------------------------------------------------------
static inline i32 stricasecmp (const Steinberg::char8* s1, const Steinberg::char8* s2)
{
	return ::strcasecmp (s1, s2);
}

//-----------------------------------------------------------------------------
static inline i32 strnicasecmp (const Steinberg::char8* s1, const Steinberg::char8* s2, size_t n)
{
	return ::strncasecmp (s1, s2, n);
}

//-----------------------------------------------------------------------------
static inline i32 stricmp16 (const Steinberg::char16* s1, const Steinberg::char16* s2)
{
	auto str1 = converter ().to_bytes (s1);
	auto str2 = converter ().to_bytes (s2);
	return stricasecmp (str1.data (), str2.data ());
}

//-----------------------------------------------------------------------------
static inline i32 strnicmp16 (const Steinberg::char16* s1, const Steinberg::char16* s2, i32 n)
{
	auto str1 = converter ().to_bytes (s1);
	auto str2 = converter ().to_bytes (s2);
	return strnicasecmp (str1.data (), str2.data (), n);
}

//-----------------------------------------------------------------------------
static inline i32 sprintf16 (Steinberg::char16* wcs, const Steinberg::char16* format, ...)
{
	assert (false && "DEPRECATED No Linux implementation");
	return 0;
}

//-----------------------------------------------------------------------------
static inline i32 vsnwprintf (Steinberg::char16* wcs, size_t maxlen,
							  const Steinberg::char16* format, va_list args)
{
	Steinberg::char8 str8[kPrintfBufferSize];
	auto format_utf8 = converter ().to_bytes(format);
	auto len = vsnprintf (str8, kPrintfBufferSize, format_utf8.data (), args);

	auto tmp_str = converter ().from_bytes (str8, str8 + len);
	auto target_len = std::min (tmp_str.size (), maxlen - 1);
	tmp_str.copy (wcs, target_len);
	wcs[target_len] = '\0';

	return tmp_str.size ();
}

//-----------------------------------------------------------------------------
static inline Steinberg::char16* strrchr16 (const Steinberg::char16* str, Steinberg::char16 c)
{
	assert (false && "DEPRECATED No Linux implementation");
	return nullptr;
}

#elif SMTG_OS_MACOS
#define tstrtoi64 strtoll
#define stricmp strcasecmp
#define strnicmp strncasecmp

//-----------------------------------------------------------------------------
static inline Steinberg::i32 strnicmp16 (const Steinberg::char16* str1, const Steinberg::char16* str2, size_t size)
{
	if (size == 0)
		return 0;

	CFIndex str1Len = Steinberg::strlen16 (str1);
	CFIndex str2Len = Steinberg::strlen16 (str2);
	if (static_cast<CFIndex> (size) < str2Len) // range is not applied to second string
		str2Len = size;
	CFStringRef cfStr1 = CFStringCreateWithCharactersNoCopy (Steinberg::kCFAllocator, (UniChar*)str1, str1Len, kCFAllocatorNull);
	CFStringRef cfStr2 = CFStringCreateWithCharactersNoCopy (Steinberg::kCFAllocator, (UniChar*)str2, str2Len, kCFAllocatorNull);
	CFComparisonResult result = CFStringCompareWithOptions (cfStr1, cfStr2, CFRangeMake (0, size), kCFCompareCaseInsensitive);
	CFRelease (cfStr1);
	CFRelease (cfStr2);
	switch (result)
	{
		case kCFCompareEqualTo:	return 0;
		case kCFCompareLessThan: return -1;
		case kCFCompareGreaterThan: 
		default: return 1;
	};
}

//-----------------------------------------------------------------------------
static inline Steinberg::i32 stricmp16 (const Steinberg::char16* str1, CFIndex str1Len, const Steinberg::char16* str2, CFIndex str2Len)
{
	CFStringRef cfStr1 = CFStringCreateWithCharactersNoCopy (Steinberg::kCFAllocator, (UniChar*)str1, str1Len, kCFAllocatorNull);
	CFStringRef cfStr2 = CFStringCreateWithCharactersNoCopy (Steinberg::kCFAllocator, (UniChar*)str2, str2Len, kCFAllocatorNull);
	CFComparisonResult result = CFStringCompare (cfStr1, cfStr2, kCFCompareCaseInsensitive);
	CFRelease (cfStr1);
	CFRelease (cfStr2);
	switch (result)
	{
		case kCFCompareEqualTo:	return 0;
		case kCFCompareLessThan: return -1;
		case kCFCompareGreaterThan: 
		default: return 1;
	};
}

//-----------------------------------------------------------------------------
static inline Steinberg::i32 stricmp16 (const Steinberg::ConstString& str1, const Steinberg::ConstString& str2)
{
	return stricmp16 (str1.text16 (), str1.length (), str2.text16 (), str2.length ());
}

//-----------------------------------------------------------------------------
static inline Steinberg::i32 stricmp16 (const Steinberg::char16* str1, const Steinberg::char16* str2)
{
	CFIndex str1Len = Steinberg::strlen16 (str1);
	CFIndex str2Len = Steinberg::strlen16 (str2);
	return stricmp16 (str1, str1Len, str2, str2Len);
}

//-----------------------------------------------------------------------------
static inline Steinberg::char16* strrchr16 (const Steinberg::char16* str, Steinberg::char16 c)
{
	Steinberg::i32 len = Steinberg::ConstString (str).length ();
	while (len > 0)
	{
		if (str[len] == c)
			return const_cast<Steinberg::char16*>(str + len);
		len--;
	}
	return 0;
}

//-----------------------------------------------------------------------------
static inline Steinberg::i32 vsnwprintf (Steinberg::char16* str, Steinberg::i32 size, const Steinberg::char16* format, va_list ap)
{
	// wrapped using CoreFoundation's CFString
	CFMutableStringRef formatString = (CFMutableStringRef)Steinberg::ConstString (format).toCFStringRef (0xFFFF, true);
	CFStringFindAndReplace (formatString, CFSTR("%s"), CFSTR("%S"), CFRangeMake (0, CFStringGetLength (formatString)), 0);
	CFStringRef resultString = CFStringCreateWithFormatAndArguments (Steinberg::kCFAllocator, 0, formatString, ap);
	CFRelease (formatString);
	if (resultString)
	{
		Steinberg::Txt res;
		res.fromCFStringRef (resultString);
		res.copyTo16 (str, 0, size);
		CFRelease (resultString);
		return 0;
	}
	return 1;
}

//-----------------------------------------------------------------------------
static inline Steinberg::i32 sprintf16 (Steinberg::char16* str, const Steinberg::char16* format, ...)
{
	va_list marker;
	va_start (marker, format);
	return vsnwprintf (str, -1, format, marker);
}

#endif // SMTG_OS_LINUX

/*
UTF-8                EF BB BF 
UTF-16 Big Endian    FE FF 
UTF-16 Little Endian FF FE 
UTF-32 Big Endian    00 00 FE FF 
UTF-32 Little Endian FF FE 00 00 
*/

namespace Steinberg {

//-----------------------------------------------------------------------------
static inline b8 isCaseSensitive (ConstString::CompareMode mode)
{
	return mode == ConstString::kCaseSensitive;
}

//-----------------------------------------------------------------------------
//	ConstString
//-----------------------------------------------------------------------------
ConstString::ConstString (const char8* str, i32 length)
: buffer8 ((char8*)str)
, len (length < 0 ? (str ? static_cast<u32> (strlen (str)) : 0) : length)
, isWide (0) 
{
}

//-----------------------------------------------------------------------------
ConstString::ConstString (const char16* str, i32 length)
: buffer16 ((char16*)str)
, len (length < 0 ? (str ? strlen16 (str) : 0) : length)
, isWide (1) 
{
}

//-----------------------------------------------------------------------------
ConstString::ConstString (const ConstString& str, i32 offset, i32 length)
: buffer (str.buffer)
, len (length < 0 ? (str.len - (offset > 0 ? offset : 0)) : length)
, isWide (str.isWide)
{
	if (offset > 0)
	{
		if (isWide)
			buffer16 += offset;
		else
			buffer8 += offset;
	}
}

//-----------------------------------------------------------------------------
ConstString::ConstString (const FVariant& var)
: buffer (nullptr)
, len (0)
, isWide (0) 
{
	switch (var.getType ())
	{
		case FVariant::kString8:
			buffer8 = (char8*)var.getString8 ();
			len = buffer8 ? strlen8 (buffer8) : 0;
			isWide = false;
			break;

		case FVariant::kString16:
			buffer16 = (char16*)var.getString16 ();
			len = buffer16 ? strlen16 (buffer16) : 0;
			isWide = true;
			break;
	}
}

//-----------------------------------------------------------------------------
ConstString::ConstString ()
: buffer (nullptr)
, len (0)
, isWide (0) 
{
}

//-----------------------------------------------------------------------------
b8 ConstString::testChar8 (u32 index, char8 c) const
{
	if (index >= len)
		return c == 0;
	if (isWide)
	{
		// make c wide
		char8 src[] = {c, 0};
		char16 dest[2] = {0};
		if (multiByteToWideString (dest, src, 2) > 0)
			return buffer16[index] == dest[0];
		return false;
	}
	return buffer8[index] == c;
}

//-----------------------------------------------------------------------------
b8 ConstString::testChar16 (u32 index, char16 c) const
{
	if (index >= len)
		return c == 0;
	if (!isWide)
	{
		// make c ansi
		char16 src[] = {c, 0};
		char8 dest[8] = {0};
		if (wideStringToMultiByte (dest, src, 2) > 0 && dest[1] == 0)
			return buffer8[index] == dest[0];
		return false;
	}
	return buffer16[index] == c;
}

//-----------------------------------------------------------------------------
b8 ConstString::extract (Txt& result, u32 idx, i32 n) const
{
	// AddressSanitizer : when extracting part of "this" on itself, it can lead to heap-use-after-free.
	SMTG_ASSERT (this != static_cast<ConstString*> (&result))
	
	if (len == 0|| idx >= len)
		return false;

	if ((idx + n > len) || n < 0)
		n = len - idx;

	if (isWide)
		result.assign (buffer16 + idx, n);
	else
		result.assign (buffer8 + idx, n);

	return true;
}

//-----------------------------------------------------------------------------
i32 ConstString::copyTo8 (char8* str, u32 idx, i32 n) const
{
	if (!str)
		return 0;

	if (isWide)
	{
		Txt tmp (text16 ());
		if (tmp.toMultiByte () == false)
			return 0;
		return tmp.copyTo8 (str, idx, n);
	}

	if (isEmpty () || idx >= len || !buffer8)
	{
		str[0] = 0;
		return 0;
	}

	if ((idx + n > len) || n < 0)
		n = len - idx;

	memcpy (str, &(buffer8[idx]), n * sizeof (char8));
	str[n] = 0;
	return n;
}

//-----------------------------------------------------------------------------
i32 ConstString::copyTo16 (char16* str, u32 idx, i32 n) const
{
	if (!str)
		return 0;

	if (!isWide)
	{
		Txt tmp (text8 ());
		if (tmp.toWideString () == false)
			return 0;
		return tmp.copyTo16 (str, idx, n);
	}
	
	if (isEmpty () || idx >= len || !buffer16)
	{
		str[0] = 0;
		return 0;
	}

	if ((idx + n > len) || n < 0)
		n = len - idx;
	
	memcpy (str, &(buffer16[idx]), n * sizeof (char16));
	str[n] = 0;
	return n;
}

//-----------------------------------------------------------------------------
i32 ConstString::copyTo (tchar* str, u32 idx, i32 n) const
{
#ifdef UNICODE
	return copyTo16 (str, idx, n);
#else
	return copyTo8 (str, idx, n);
#endif
}

//-----------------------------------------------------------------------------
z0 ConstString::copyTo (IStringResult* result) const
{
	if (isWideString () == false)
	{
		result->setText (text8 ());	
	}
	else
	{
		FUnknownPtr<IString> iStr (result);
		if (iStr)
		{
			iStr->setText16 (text16 ());
		}
		else
		{
			Txt tmp (*this);
			tmp.toMultiByte ();
			result->setText (tmp.text8 ());
		}
	}
}

//-----------------------------------------------------------------------------
z0 ConstString::copyTo (IString& string) const
{
	if (isWideString ())
		string.setText16 (text16 ());
	else
		string.setText8 (text8 ());
}



//-----------------------------------------------------------------------------
i32 ConstString::compare (const ConstString& str, i32 n, CompareMode mode) const
{
	if (n == 0)
		return 0;

	if (str.isEmpty ())
	{
		if (isEmpty ())
			return 0;
		return 1;
	}
	if (isEmpty ())
		return -1;

	if (!isWide && !str.isWide)
	{
		if (n < 0)
		{
			if (isCaseSensitive (mode))
				return strcmp (*this, str);
			return stricmp (*this, str);
		}
		if (isCaseSensitive (mode))
			return strncmp (*this, str, n);
		return strnicmp (*this, str, n);
	}
	if (isWide && str.isWide)
	{
		if (n < 0)
		{
			if (isCaseSensitive (mode))
				return strcmp16 (*this, str);
			return stricmp16 (*this, str);
		}
		if (isCaseSensitive (mode))
			return strncmp16 (*this, str, n);
		return strnicmp16 (*this, str, n);
	}
	return compareAt (0, str, n, mode);
}

//-----------------------------------------------------------------------------
i32 ConstString::compare (const ConstString& str, CompareMode mode) const
{
	return compare (str, -1, mode);
}

//-----------------------------------------------------------------------------
i32 ConstString::compareAt (u32 index, const ConstString& str, i32 n, CompareMode mode) const
{
	if (n == 0)
		return 0;

	if (str.isEmpty ())
	{
		if (isEmpty ())
			return 0;
		return 1;
	}
	if (isEmpty ())
		return -1;

	if (!isWide && !str.isWide)
	{
		char8* toCompare = buffer8;
		if (index > 0)
		{
			if (index >= len)
			{
				if (str.isEmpty ())
					return 0;
				return -1;
			}
			toCompare += index;
		}

		if (n < 0)
		{
			if (isCaseSensitive (mode))
				return strcmp (toCompare, str);
			return stricmp (toCompare, str);
		}
		if (isCaseSensitive (mode))
			return strncmp (toCompare, str, n);
		return strnicmp (toCompare, str, n);
	}
	if (isWide && str.isWide)
	{
		char16* toCompare = buffer16;
		if (index > 0)
		{
			if (index >= len)
			{
				if (str.isEmpty ())
					return 0;
				return -1;
			}
			toCompare += index;
		}

		if (n < 0)
		{
			if (isCaseSensitive (mode))
				return strcmp16 (toCompare, str.text16 ());
			return stricmp16 (toCompare, str.text16 ());
		}
		if (isCaseSensitive (mode))
			return strncmp16 (toCompare, str.text16 (), n);
		return strnicmp16 (toCompare, str.text16 (), n);
	}
	
	if (isWide)
	{
		Txt tmp (str.text8 ());
		if (tmp.toWideString () == false)
			return -1;
		return compareAt (index, tmp, n, mode);
	}
		
	Txt tmp (text8 ());
	if (tmp.toWideString () == false)
		return 1;
	return tmp.compareAt (index, str, n, mode);
}

//------------------------------------------------------------------------
Steinberg::i32 ConstString::naturalCompare (const ConstString& str, CompareMode mode /*= kCaseSensitive*/) const
{
	if (str.isEmpty ())
	{
		if (isEmpty ())
			return 0;
		return 1;
	}
	if (isEmpty ())
		return -1;

	if (!isWide && !str.isWide)
		return strnatcmp8 (buffer8, str.text8 (), isCaseSensitive (mode));
	if (isWide && str.isWide)
		return strnatcmp16 (buffer16, str.text16 (), isCaseSensitive (mode));
	
	if (isWide)
	{
		Txt tmp (str.text8 ());
		tmp.toWideString ();
		return strnatcmp16 (buffer16, tmp.text16 (), isCaseSensitive (mode));
	}
	Txt tmp (text8 ());
	tmp.toWideString ();
	return strnatcmp16 (tmp.text16 (), str.text16 (), isCaseSensitive (mode));
}

//-----------------------------------------------------------------------------
b8 ConstString::startsWith (const ConstString& str, CompareMode mode /*= kCaseSensitive*/) const
{
	if (str.isEmpty ())
	{
		return isEmpty ();
	}
	if (isEmpty ())
	{
		return false;
	}
	if (length () < str.length ())
	{
		return false;
	}
	if (!isWide && !str.isWide)
	{
		if (isCaseSensitive (mode))
			return strncmp (buffer8, str.buffer8, str.length ()) == 0;
		return strnicmp (buffer8, str.buffer8, str.length ()) == 0;
	}
	if (isWide && str.isWide)
	{
		if (isCaseSensitive (mode))
			return strncmp16 (buffer16, str.buffer16, str.length ()) == 0;
		return strnicmp16 (buffer16, str.buffer16, str.length ()) == 0;
	}
	if (isWide)
	{
		Txt tmp (str.text8 ());
		tmp.toWideString ();
		if (tmp.length () > length ())
			return false;
		if (isCaseSensitive (mode))
			return strncmp16 (buffer16, tmp.buffer16, tmp.length ()) == 0;
		return strnicmp16 (buffer16, tmp.buffer16, tmp.length ()) == 0;
	}
	Txt tmp (text8 ());
	tmp.toWideString ();
	if (str.length () > tmp.length ())
		return false;
	if (isCaseSensitive (mode))
		return strncmp16 (tmp.buffer16, str.buffer16, str.length ()) == 0;
	return strnicmp16 (tmp.buffer16, str.buffer16, str.length ()) == 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::endsWith (const ConstString& str, CompareMode mode /*= kCaseSensitive*/) const
{
	if (str.isEmpty ())
	{
		return isEmpty ();
	}
	if (isEmpty ())
	{
		return false;
	}
	if (length () < str.length ())
	{
		return false;
	}
	if (!isWide && !str.isWide)
	{
		if (isCaseSensitive (mode))
			return strncmp (buffer8 + (length () - str.length ()), str.buffer8, str.length ()) == 0;
		return strnicmp (buffer8 + (length () - str.length ()), str.buffer8, str.length ()) == 0;
	}
	if (isWide && str.isWide)
	{
		if (isCaseSensitive (mode))
			return strncmp16 (buffer16 + (length () - str.length ()), str.buffer16, str.length ()) == 0;
		return strnicmp16 (buffer16 + (length () - str.length ()), str.buffer16, str.length ()) == 0;
	}
	if (isWide)
	{
		Txt tmp (str.text8 ());
		tmp.toWideString ();
		if (tmp.length () > length ())
			return false;
		if (isCaseSensitive (mode))
			return strncmp16 (buffer16 + (length () - tmp.length ()), tmp.buffer16, tmp.length ()) == 0;
		return strnicmp16 (buffer16 + (length () - tmp.length ()), tmp.buffer16, tmp.length ()) == 0;
	}
	Txt tmp (text8 ());
	tmp.toWideString ();
	if (str.length () > tmp.length ())
		return false;
	if (isCaseSensitive (mode))
		return strncmp16 (tmp.buffer16 + (tmp.length () - str.length ()), str.buffer16, str.length ()) == 0;
	return strnicmp16 (tmp.buffer16 + (tmp.length () - str.length ()), str.buffer16, str.length ()) == 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::contains (const ConstString& str, CompareMode m) const
{
	return findFirst (str, -1, m) != -1;
}

//-----------------------------------------------------------------------------
i32 ConstString::findNext (i32 startIndex, const ConstString& str, i32 n, CompareMode mode, i32 endIndex) const
{
	u32 endLength = len;
	if (endIndex > -1 && (u32)endIndex < len)
		endLength = endIndex + 1;

	if (isWide && str.isWide)
	{
		if (startIndex < 0)
			startIndex = 0;

		u32 stringLength = str.length ();
		n = n < 0 ? stringLength : Min<u32> (n, stringLength);

		if (n > 0)
		{
			u32 i = 0;

			if (isCaseSensitive (mode))
			{
				for (i = startIndex; i < endLength; i++)
					if (strncmp16 (buffer16 + i, str, n) == 0)
						return i;
			}
			else
			{
				for (i = startIndex; i < endLength; i++)
					if (strnicmp16 (buffer16 + i, str, n) == 0)
						return i;
			}
		}
		return -1;
	}
	if (!isWide && !str.isWide)
	{
		u32 stringLength = str.length ();
		n = n < 0 ? stringLength : Min<u32> (n, stringLength);

		if (startIndex < 0)
			startIndex = 0;

		if (n > 0)
		{
			u32 i = 0;

			if (isCaseSensitive (mode))
			{
				for (i = startIndex; i < endLength; i++)
					if (strncmp (buffer8 + i, str, n) == 0)
						return i;
			}
			else
			{
				for (i = startIndex; i < endLength; i++)
					if (strnicmp (buffer8 + i, str, n) == 0)
						return i;
			}
		}
		return -1;
	}
	Txt tmp;
	if (isWide)
	{
		tmp = str.text8 ();
		tmp.toWideString ();
		return findNext (startIndex, tmp, n , mode, endIndex);
	}
	tmp = text8 ();
	tmp.toWideString ();
	return tmp.findNext (startIndex, str, n, mode, endIndex);
}

//------------------------------------------------------------------------------------------------
i32 ConstString::findNext (i32 startIndex, char8 c, CompareMode mode, i32 endIndex) const
{
	u32 endLength = len;
	if (endIndex > -1 && (u32)endIndex < len)
		endLength = endIndex + 1;

	if (isWide)
	{
		char8 src[] = {c, 0};
		char16 dest[8] = {0};
		if (multiByteToWideString (dest, src, 2) > 0)
			return findNext (startIndex, dest[0], mode, endIndex);
		return -1;
	}

	if (startIndex < 0)
		startIndex = 0;
	u32 i;

	if (isCaseSensitive (mode))
	{
		for (i = startIndex; i < endLength; i++)
		{
			if (buffer8[i] == c)
				return i;
		}
	}
	else
	{
		c = toLower (c);
		for (i = startIndex; i < endLength; i++)
		{
			if (toLower (buffer8[i]) == c)
				return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
i32 ConstString::findNext (i32 startIndex, char16 c, CompareMode mode, i32 endIndex) const
{
	u32 endLength = len;
	if (endIndex > -1 && (u32)endIndex < len)
		endLength = endIndex + 1;

	if (!isWide)
	{
		char16 src[] = {c, 0};
		char8 dest[8] = {0};
		if (wideStringToMultiByte (dest, src, 2) > 0 && dest[1] == 0)
			return findNext (startIndex, dest[0], mode, endIndex);

		return -1;
	}

	u32 i;
	if (startIndex < 0)
		startIndex = 0;

	if (isCaseSensitive (mode))
	{
		for (i = startIndex; i < endLength; i++)
		{
			if (buffer16[i] == c)
				return i;
		}
	}
	else
	{
		c = toLower (c);
		for (i = startIndex; i < endLength; i++)
		{
			if (toLower (buffer16[i]) == c)
				return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
i32 ConstString::findPrev (i32 startIndex, char8 c, CompareMode mode) const
{
	if (len == 0)
		return -1;

	if (isWide)
	{
		char8 src[] = {c, 0};
		char16 dest[8] = {0};
		if (multiByteToWideString (dest, src, 2) > 0)
			return findPrev (startIndex, dest[0], mode);
		return -1;
	}

	if (startIndex < 0 || startIndex > (i32)len)
		startIndex = len;

	i32 i;

	if (isCaseSensitive (mode))
	{
		for (i = startIndex; i >= 0; i--)
		{
			if (buffer8[i] == c)
				return i;
		}
	}
	else
	{
		c = toLower (c);
		for (i = startIndex; i >= 0; i--)
		{
			if (toLower (buffer8[i]) == c)
				return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
i32 ConstString::findPrev (i32 startIndex, char16 c, CompareMode mode) const
{
	if (len == 0)
		return -1;

	if (!isWide)
	{
		char16 src[] = {c, 0};
		char8 dest[8] = {0};
		if (wideStringToMultiByte (dest, src, 2) > 0 && dest[1] == 0)
			return findPrev (startIndex, dest[0], mode);

		return -1;
	}

	if (startIndex < 0 || startIndex > (i32)len)
		startIndex = len;

	i32 i;

	if (isCaseSensitive (mode))
	{
		for (i = startIndex; i >= 0; i--)
		{
			if (buffer16[i] == c)
				return i;
		}
	}
	else
	{
		c = toLower (c);
		for (i = startIndex; i >= 0; i--)
		{
			if (toLower (buffer16[i]) == c)
				return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
i32 ConstString::findPrev (i32 startIndex, const ConstString& str, i32 n, CompareMode mode) const
{
	if (isWide && str.isWide)
	{
		u32 stringLength = str.length ();
		n = n < 0 ? stringLength : Min<u32> (n, stringLength);

		if (startIndex < 0 || startIndex >= (i32)len)
			startIndex = len - 1;

		if (n > 0)
		{
			i32 i = 0;

			if (isCaseSensitive (mode))
			{
				for (i = startIndex; i >= 0; i--)
					if (strncmp16 (buffer16 + i, str, n) == 0)
						return i;
			}
			else
			{
				for (i = startIndex; i >= 0; i--)
					if (strnicmp16 (buffer16 + i, str, n) == 0)
						return i;
			}
		}
		return -1;
	}
	if (!isWide && !str.isWide)
	{
		u32 stringLength = str.length ();
		n = n < 0 ? stringLength : Min<u32> (n, stringLength);

		if (startIndex < 0 || startIndex >= (i32)len)
			startIndex = len - 1;

		if (n > 0)
		{
			i32 i = 0;

			if (isCaseSensitive (mode))
			{
				for (i = startIndex; i >= 0; i--)
					if (strncmp (buffer8 + i, str, n) == 0)
						return i;
			}
			else
			{
				for (i = startIndex; i >= 0; i--)
					if (strnicmp (buffer8 + i, str, n) == 0)
						return i;
			}
		}
		return -1;
	}
	if (isWide)
	{
		Txt tmp (str.text8 ());
		tmp.toWideString ();
		return findPrev (startIndex, tmp, n, mode);
	}
	Txt tmp (text8 ());
	tmp.toWideString ();
	return tmp.findPrev (startIndex, str, n, mode);
}

//-----------------------------------------------------------------------------
i32 ConstString::countOccurences (char8 c, u32 startIndex, CompareMode mode) const
{
	if (isWide)
	{
		char8 src[] = {c, 0};
		char16 dest[8] = {0};
		if (multiByteToWideString (dest, src, 2) > 0)
			return countOccurences (dest[0], startIndex, mode);
		return -1;
	}

	i32 result = 0;
	i32 next = startIndex;
	while (true)
	{
		next = findNext (next, c, mode);
		if (next >= 0)
		{
			next++;
			result++;
		}
		else
			break;
	}
	return result;
}

//-----------------------------------------------------------------------------
i32 ConstString::countOccurences (char16 c, u32 startIndex, CompareMode mode) const
{
	if (!isWide)
	{
		char16 src[] = {c, 0};
		char8 dest[8] = {0};
		if (wideStringToMultiByte (dest, src, 2) > 0 && dest[1] == 0)
			return countOccurences (dest[0], startIndex, mode);

		return -1;
	}
	i32 result = 0;
	i32 next = startIndex;
	while (true)
	{
		next = findNext (next, c, mode);
		if (next >= 0)
		{
			next++;
			result++;
		}
		else
			break;
	}
	return result;
}

//-----------------------------------------------------------------------------
i32 ConstString::getFirstDifferent (const ConstString& str, CompareMode mode) const
{
	if (str.isWide != isWide)
	{
		if (isWide)
		{
			Txt tmp (str.text8 ());
			if (tmp.toWideString () == false)
				return -1;
			return getFirstDifferent (tmp, mode);
		}
		
		Txt tmp (text8 ());
		if (tmp.toWideString () == false)
			return -1;
		return tmp.getFirstDifferent (str, mode);
	}

	u32 len1 = len;
	u32 len2 = str.len;
	u32 i;

	if (isWide)
	{
		if (isCaseSensitive (mode))
		{
			for (i = 0; i <= len1 && i <= len2; i++)
			{
				if (buffer16[i] != str.buffer16[i])
					return i;
			}
		}
		else
		{
			for (i = 0; i <= len1 && i <= len2; i++)
			{
				if (toLower (buffer16[i]) != toLower (str.buffer16[i]))
					return i;
			}
		}
	}
	else
	{
		if (isCaseSensitive (mode))
		{
			for (i = 0; i <= len1 && i <= len2; i++)
			{
				if (buffer8[i] != str.buffer8[i])
					return i;
			}
		}
		else
		{
			for (i = 0; i <= len1 && i <= len2; i++)
			{
				if (toLower (buffer8[i]) != toLower (str.buffer8[i]))
					return i;
			}
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanInt64 (z64& value, u32 offset, b8 scanToEnd) const
{
	if (isEmpty () || offset >= len)
		return false;

	if (isWide)
		return scanInt64_16 (buffer16 + offset, value, scanToEnd);
	return scanInt64_8 (buffer8 + offset, value, scanToEnd);
}

//-----------------------------------------------------------------------------
b8 ConstString::scanUInt64 (zu64& value, u32 offset, b8 scanToEnd) const
{
	if (isEmpty () || offset >= len)
		return false;

	if (isWide)
		return scanUInt64_16 (buffer16 + offset, value, scanToEnd);
	return scanUInt64_8 (buffer8 + offset, value, scanToEnd);
}

//-----------------------------------------------------------------------------
b8 ConstString::scanHex (u8& value, u32 offset, b8 scanToEnd) const
{
	if (isEmpty () || offset >= len)
		return false;

	if (isWide)
		return scanHex_16 (buffer16 + offset, value, scanToEnd);
	return scanHex_8 (buffer8 + offset, value, scanToEnd);
}

//-----------------------------------------------------------------------------
b8 ConstString::scanInt32 (i32& value, u32 offset, b8 scanToEnd) const
{
	if (isEmpty () || offset >= len)
		return false;

	if (isWide)
		return scanInt32_16 (buffer16 + offset, value, scanToEnd);
	return scanInt32_8 (buffer8 + offset, value, scanToEnd);
}

//-----------------------------------------------------------------------------
b8 ConstString::scanUInt32 (u32& value, u32 offset, b8 scanToEnd) const
{
	if (isEmpty () || offset >= len)
		return false;

	if (isWide)
		return scanUInt32_16 (buffer16 + offset, value, scanToEnd);
	return scanUInt32_8 (buffer8 + offset, value, scanToEnd);
}

//-----------------------------------------------------------------------------
b8 ConstString::scanInt64_8 (const char8* text, z64& value, b8 scanToEnd)
{	
	while (text && text[0])
	{
		if (sscanf (text, "%" FORMAT_INT64A, &value) == 1)
			return true;
		if (scanToEnd == false)
			return false;
		text++;
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanInt64_16 (const char16* text, z64& value, b8 scanToEnd)
{
	if (text && text[0])
	{
		Txt str (text);
		str.toMultiByte (kCP_Default);
		return scanInt64_8 (str, value, scanToEnd);
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanUInt64_8 (const char8* text, zu64& value, b8 scanToEnd)
{
	while (text && text[0])
	{
		if (sscanf (text, "%" FORMAT_UINT64A, &value) == 1)
			return true;
		if (scanToEnd == false)
			return false;
		text++;
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanUInt64_16 (const char16* text, zu64& value, b8 scanToEnd)
{
	if (text && text[0])
	{
		Txt str (text);
		str.toMultiByte (kCP_Default);
		return scanUInt64_8 (str, value, scanToEnd);
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanInt64 (const tchar* text, z64& value, b8 scanToEnd)
{
#ifdef UNICODE
	return scanInt64_16 (text, value,scanToEnd);
#else
	return scanInt64_8 (text, value, scanToEnd);
#endif
}

//-----------------------------------------------------------------------------
b8 ConstString::scanUInt64 (const tchar* text, zu64& value, b8 scanToEnd)
{
#ifdef UNICODE
	return scanUInt64_16 (text, value, scanToEnd);
#else
	return scanUInt64_8 (text, value, scanToEnd);
#endif
}

//-----------------------------------------------------------------------------
b8 ConstString::scanHex_8 (const char8* text, u8& value, b8 scanToEnd)
{
	while (text && text[0])
	{
		u32 v; // scanf expects an u32 for %x
		if (sscanf (text, "%x", &v) == 1)
		{
			value = (u8)v;
			return true;
		}
		if (scanToEnd == false)
			return false;
		text++;
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanHex_16 (const char16* text, u8& value, b8 scanToEnd)
{
	if (text && text[0])
	{
		Txt str (text);
		str.toMultiByte (kCP_Default); // scanf uses default codepage
		return scanHex_8 (str, value, scanToEnd);
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::scanHex (const tchar* text, u8& value, b8 scanToEnd)
{
#ifdef UNICODE
	return scanHex_16 (text, value, scanToEnd);
#else
	return scanHex_8 (text, value, scanToEnd);
#endif
}

//-----------------------------------------------------------------------------
b8 ConstString::scanFloat (f64& value, u32 offset, b8 scanToEnd) const
{
	if (isEmpty () || offset >= len)
		return false;

	Txt str (*this);
	i32 pos = -1;
	if (isWide)
	{
		if ((pos = str.findNext (offset, STR(','))) >= 0 && ((u32)pos) >= offset)
			str.setChar (pos, STR('.'));

		str.toMultiByte (kCP_Default); // scanf uses default codepage
	}
	else
	{
		if ((pos = str.findNext (offset, ',')) >= 0 && ((u32)pos) >= offset)
			str.setChar (pos, '.');
	}

	const char8* txt = str.text8 () + offset;
	while (txt && txt[0])
	{
		if (sscanf (txt, "%lf", &value) == 1)
			return true;
		if (scanToEnd == false)
			return false;
		txt++;
	}
	return false;
}

//-----------------------------------------------------------------------------
char16 ConstString::toLower (char16 c)
{
	#if SMTG_OS_WINDOWS
		WCHAR temp[2] = {c, 0};
        ::CharLowerW (temp);
        return temp[0];
	#elif SMTG_OS_MACOS
		// only convert characters which in lowercase are also single characters
		UniChar characters [2] = {0};
		characters[0] = c;
		CFMutableStringRef str = CFStringCreateMutableWithExternalCharactersNoCopy (kCFAllocator, characters, 1, 2, kCFAllocatorNull);
		if (str)
		{
			CFStringLowercase (str, NULL);
			CFRelease (str);
			if (characters[1] == 0)
				return characters[0];
		}
		return c;
	#elif SMTG_OS_LINUX
	assert (false && "DEPRECATED No Linux implementation");
		return c;
	#else
		return towlower (c);
	#endif
}

//-----------------------------------------------------------------------------
char16 ConstString::toUpper (char16 c)
{
	#if SMTG_OS_WINDOWS
		WCHAR temp[2] = {c, 0};
        ::CharUpperW (temp);
        return temp[0];
	#elif SMTG_OS_MACOS
		// only convert characters which in uppercase are also single characters (don't translate a sharp-s which would result in SS)
		UniChar characters [2] = {0};
		characters[0] = c;
		CFMutableStringRef str = CFStringCreateMutableWithExternalCharactersNoCopy (kCFAllocator, characters, 1, 2, kCFAllocatorNull);
		if (str)
		{
			CFStringUppercase (str, NULL);
			CFRelease (str);
			if (characters[1] == 0)
				return characters[0];
		}
		return c;
    #elif SMTG_OS_LINUX
	assert (false && "DEPRECATED No Linux implementation");
		return c;
	#else
		return towupper (c);
	#endif
}

//-----------------------------------------------------------------------------
char8 ConstString::toLower (char8 c)
{
	if ((c >= 'A') && (c <= 'Z'))
		return c + ('a' - 'A');
	#if SMTG_OS_WINDOWS
		CHAR temp[2] = {c, 0};
        ::CharLowerA (temp);
        return temp[0];
	#else
		return static_cast<char8> (tolower (c));
	#endif
}

//-----------------------------------------------------------------------------
char8 ConstString::toUpper (char8 c)
{
	if ((c >= 'a') && (c <= 'z'))
		return c - ('a' - 'A');
	#if SMTG_OS_WINDOWS
		CHAR temp[2] = {c, 0};
        ::CharUpperA (temp);
        return temp[0];
	#else
		return static_cast<char8> (toupper (c));
	#endif
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharSpace (const char8 character)
{
	return isspace (character) != 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharSpace (const char16 character)
{
	switch (character)
	{
		case 0x0020:
		case 0x00A0:
		case 0x2002:
		case 0x2003:
		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2007:
		case 0x2008:
		case 0x2009:
		case 0x200A:
		case 0x200B:
		case 0x202F:
		case 0x205F:
		case 0x3000:
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharAlpha (const char8 character)
{
	return isalpha (character) != 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharAlpha (const char16 character)
{
	return iswalpha (character) != 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharAlphaNum (const char8 character)
{
	return isalnum (character) != 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharAlphaNum (const char16 character)
{
	return iswalnum (character) != 0; // this may not work on macOSX when another locale is set inside the c-lib
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharDigit (const char8 character)
{
	return isdigit (character) != 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharDigit (const char16 character)
{
	return iswdigit (character) != 0;	// this may not work on macOSX when another locale is set inside the c-lib
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharAscii (char8 character)
{
	return character >= 0;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharAscii (char16 character)
{
	return character < 128;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharUpper (char8 character)
{
	return toUpper (character) == character;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharUpper (char16 character)
{
	return toUpper (character) == character;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharLower (char8 character)
{
	return toLower (character) == character;
}

//-----------------------------------------------------------------------------
b8 ConstString::isCharLower (char16 character)
{
	return toLower (character) == character;
}

//-----------------------------------------------------------------------------
b8 ConstString::isDigit (u32 index) const
{
	if (isEmpty () || index >= len)
		return false;

	if (isWide)
		return ConstString::isCharDigit (buffer16[index]);
	return ConstString::isCharDigit (buffer8[index]);
}

//-----------------------------------------------------------------------------
i32 ConstString::getTrailingNumberIndex (u32 width) const
{
	if (isEmpty ())
		return -1;

	i32 endIndex = len - 1;
	i32 i = endIndex;
	while (isDigit ((u32) i) && i >= 0)
		i--;

	// now either all are digits or i is on the first non digit
	if (i < endIndex)
	{
		if (width > 0 && (endIndex - i != static_cast<i32> (width)))
			return -1;

		return i + 1;
	}

	return -1;
}

//-----------------------------------------------------------------------------
z64 ConstString::getTrailingNumber (z64 fallback) const
{
	i32 index = getTrailingNumberIndex ();

	z64 number = 0;

	if (index >= 0)
		if (scanInt64 (number, index))
			return number;

	return fallback;
}



//-----------------------------------------------------------------------------
z0 ConstString::toVariant (FVariant& var) const
{
	if (isWide)
	{
		var.setString16 (buffer16);
	}
	else
	{
		var.setString8 (buffer8);
	}
}

//-----------------------------------------------------------------------------
b8 ConstString::isAsciiString () const
{
	u32 i;
	if (isWide)
	{
		for (i = 0; i < len; i++)
			if (ConstString::isCharAscii (buffer16 [i]) == false)
				return false;
	}
	else
	{
		for (i = 0; i < len; i++)
			if (ConstString::isCharAscii (buffer8 [i]) == false)
				return false;
	}
	return true;
}


#if SMTG_OS_MACOS
u32 kDefaultSystemEncoding = kCFStringEncodingMacRoman;
//-----------------------------------------------------------------------------
static CFStringEncoding MBCodePageToCFStringEncoding (u32 codePage)
{
	switch (codePage)
	{
		case kCP_ANSI:		return kDefaultSystemEncoding; // MacRoman or JIS
		case kCP_MAC_ROMAN:	return kCFStringEncodingMacRoman;
		case kCP_ANSI_WEL:	return kCFStringEncodingWindowsLatin1;
		case kCP_MAC_CEE:	return kCFStringEncodingMacCentralEurRoman;
		case kCP_Utf8:		return kCFStringEncodingUTF8;
		case kCP_ShiftJIS:	return kCFStringEncodingShiftJIS_X0213_00;
		case kCP_US_ASCII:	return kCFStringEncodingASCII;
	}
	return kCFStringEncodingASCII;
}
#endif

//-----------------------------------------------------------------------------
i32 ConstString::multiByteToWideString (char16* dest, const char8* source, i32 charCount, u32 sourceCodePage)
{
	if (source == nullptr || source[0] == 0)
	{
		if (dest && charCount > 0)
		{
			dest[0] = 0;
		}
		return 0;
	}
	i32 result = 0;
#if SMTG_OS_WINDOWS
	result = MultiByteToWideChar (sourceCodePage, MB_ERR_INVALID_CHARS, source, -1, wscast (dest), charCount);
#endif

#if SMTG_OS_MACOS
	CFStringRef cfStr =
	    (CFStringRef)::toCFStringRef (source, MBCodePageToCFStringEncoding (sourceCodePage));
	if (cfStr)
	{
		CFRange range = {0, CFStringGetLength (cfStr)};
		CFIndex usedBytes;
		if (CFStringGetBytes (cfStr, range, kCFStringEncodingUnicode, ' ', false, (UInt8*)dest,
		                      charCount * 2, &usedBytes) > 0)
		{
			result = static_cast<i32> (usedBytes / 2 + 1);
			if (dest)
				dest[usedBytes / 2] = 0;
		}

		CFRelease (cfStr);
	}
#endif

#if SMTG_OS_LINUX
	if (sourceCodePage == kCP_ANSI || sourceCodePage == kCP_US_ASCII || sourceCodePage == kCP_Utf8)
	{
		if (dest == nullptr)
		{
			auto state = std::mbstate_t ();
			auto maxChars = charCount ? charCount : std::numeric_limits<i32>::max () - 1;
			result = converterFacet ().length (state, source, source + strlen (source), maxChars);
		}
		else
		{
			auto utf16Str = converter ().from_bytes (source);
			if (!utf16Str.empty ())
			{
				result = std::min<i32> (charCount, utf16Str.size ());
				memcpy (dest, utf16Str.data (), result * sizeof (char16));
				dest[result] = 0;
			}
		}
	}
	else
	{
		assert (false && "DEPRECATED No Linux implementation");
	}

#endif

	SMTG_ASSERT (result > 0)
	return result;
}

//-----------------------------------------------------------------------------
i32 ConstString::wideStringToMultiByte (char8* dest, const char16* wideString, i32 charCount, u32 destCodePage)
{
#if SMTG_OS_WINDOWS
	return WideCharToMultiByte (destCodePage, 0, wscast (wideString), -1, dest, charCount, nullptr, nullptr);

#elif SMTG_OS_MACOS
	i32 result = 0;
	if (wideString != 0)
	{
		if (dest)
		{
			CFStringRef cfStr = CFStringCreateWithCharactersNoCopy (kCFAllocator, (const UniChar*)wideString, strlen16 (wideString), kCFAllocatorNull);
			if (cfStr)
			{
				if (fromCFStringRef (dest, charCount, cfStr, MBCodePageToCFStringEncoding (destCodePage)))
					result = static_cast<i32> (strlen (dest) + 1);
				CFRelease (cfStr);
			}
		}
		else
		{
			return static_cast<i32> (CFStringGetMaximumSizeForEncoding (strlen16 (wideString), MBCodePageToCFStringEncoding (destCodePage)));
		}
	}
	return result;

#elif SMTG_OS_LINUX
	i32 result = 0;
	if (destCodePage == kCP_Utf8)
	{
		if (dest == nullptr)
		{
			auto maxChars = charCount ? charCount : tstrlen (wideString);
			result = converterFacet ().max_length () * maxChars;
		}
		else
		{
			auto utf8Str = converter ().to_bytes (wideString);
			if (!utf8Str.empty ())
			{
				result = std::min<i32> (charCount, utf8Str.size ());
				memcpy (dest, utf8Str.data (), result * sizeof (char8));
				dest[result] = 0;
			}
		}
	}
	else if (destCodePage == kCP_ANSI || destCodePage == kCP_US_ASCII)
	{
		if (dest == nullptr)
		{
			result = strlen16 (wideString) + 1;
		}
		else
		{
			i32 i = 0;
			for (; i < charCount; ++i)
			{
				if (wideString[i] == 0)
					break;
				if (wideString[i] <= 0x007F)
					dest[i] = wideString[i];
				else
					dest[i] = '_';
			}
			dest[i] = 0;
			result = i;
		}
	}
	else
	{
		assert (false && "DEPRECATED No Linux implementation");
	}
	return result;

#else
	assert (false && "DEPRECATED No Linux implementation");
	return 0;
#endif

}

//-----------------------------------------------------------------------------
b8 ConstString::isNormalized (UnicodeNormalization n)
{
	if (isWide == false)
		return false;

#if SMTG_OS_WINDOWS
#ifdef UNICODE
	if (n != kUnicodeNormC)
		return false;
	u32 normCharCount = static_cast<u32> (FoldString (MAP_PRECOMPOSED, wscast (buffer16), len, nullptr, 0));
	return (normCharCount == len);
#else
	return false; 
#endif

#elif SMTG_OS_MACOS
	if (n != kUnicodeNormC)
		return false;

	CFStringRef cfStr = (CFStringRef)toCFStringRef ();
	CFIndex charCount = CFStringGetLength (cfStr);
	CFRelease (cfStr);
	return (charCount == len);
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
//	Txt
//-----------------------------------------------------------------------------
Txt::Txt ()
{
	isWide = kWideStringDefault ? 1 : 0;
}

//-----------------------------------------------------------------------------
Txt::Txt (const char8* str, MBCodePage codePage, i32 n, b8 isTerminated)
{
	isWide = false;
	if (str)
	{
		if (isTerminated && n >= 0 && str[n] != 0)
		{
			// isTerminated is not always set correctly
			isTerminated = false;
		}

		if (!isTerminated)
		{
			assign (str, n, isTerminated);
			toWideString (codePage);
		}
		else
		{
			if (n < 0)
				n = static_cast<i32> (strlen (str));
			if (n > 0)
				_toWideString (str, n, codePage);
		}
	}
}

//-----------------------------------------------------------------------------
Txt::Txt (const char8* str, i32 n, b8 isTerminated)
{
	if (str)
		assign (str, n, isTerminated);
}

//-----------------------------------------------------------------------------
Txt::Txt (const char16* str, i32 n, b8 isTerminated)
{
	isWide = 1;
	if (str)
		assign (str, n, isTerminated);
}

//-----------------------------------------------------------------------------
Txt::Txt (const Txt& str, i32 n)
{
	isWide = str.isWideString ();
	if (!str.isEmpty ())
		assign (str, n);
}

//-----------------------------------------------------------------------------
Txt::Txt (const ConstString& str, i32 n)
{
	isWide = str.isWideString ();
	if (!str.isEmpty ())
		assign (str, n);
}

//-----------------------------------------------------------------------------
Txt::Txt (const FVariant& var)
{
	isWide = kWideStringDefault ? 1 : 0;
	fromVariant (var);
}

//-----------------------------------------------------------------------------
Txt::Txt (IString* str)
{
	isWide = str->isWideString ();
	if (isWide)
		assign (str->getText16 ());
	else
		assign (str->getText8 ());
}

//-----------------------------------------------------------------------------
Txt::~Txt ()
{
	if (buffer)
		resize (0, false);
}

#if SMTG_CPP11_STDLIBSUPPORT
//-----------------------------------------------------------------------------
Txt::Txt (Txt&& str)
{
	*this = std::move (str);
}

//-----------------------------------------------------------------------------
Txt& Txt::operator= (Txt&& str)
{
	SMTG_ASSERT (buffer == nullptr || buffer != str.buffer);
	tryFreeBuffer ();
	
	isWide = str.isWide;
	buffer = str.buffer;
	len = str.len;
	str.buffer = nullptr;
	str.len = 0;
	return *this;
}
#endif

//-----------------------------------------------------------------------------
z0 Txt::updateLength ()
{
	if (isWide)
		len = strlen16 (text16 ());
	else
		len = strlen8 (text8 ());
}

//-----------------------------------------------------------------------------
b8 Txt::toWideString (u32 sourceCodePage)
{
	if (!isWide && buffer8 && len > 0)
		return _toWideString (buffer8, len, sourceCodePage);
	isWide = true;
	return true;
}

//-----------------------------------------------------------------------------
b8 Txt::_toWideString (const char8* src, i32 length, u32 sourceCodePage)
{
	if (!isWide)
	{
		if (src && length > 0)
		{
			i32 bytesNeeded = multiByteToWideString (nullptr, src, 0, sourceCodePage) * sizeof (char16);
			if (bytesNeeded)
			{
				bytesNeeded += sizeof (char16);
				char16* newStr = (char16*)malloc (bytesNeeded);
				if (multiByteToWideString (newStr, src, length + 1, sourceCodePage) < 0)
				{
					free (newStr);
					return false;
				}
				if (buffer8)
					free (buffer8);

				buffer16 = newStr;
				isWide = true;
				updateLength ();
			}
			else
			{
				return false;
			}
		}
		isWide = true;
	}
	return true;
}

#define SMTG_STRING_CHECK_CONVERSION 1
#define SMTG_STRING_CHECK_CONVERSION_NO_BREAK 1

#if SMTG_STRING_CHECK_CONVERSION_NO_BREAK
	#define SMTG_STRING_CHECK_MSG FDebugPrint
#else
	#define SMTG_STRING_CHECK_MSG FDebugBreak
#endif
//-----------------------------------------------------------------------------
b8 Txt::checkToMultiByte (u32 destCodePage) const
{
	if (!isWide || isEmpty ())
		return true;

#if DEVELOPMENT && SMTG_STRING_CHECK_CONVERSION
	i32 debugLen = length ();
	i32 debugNonASCII = 0;
	for (i32 i = 0; i < length (); i++)
	{
		if (buffer16[i] > 127)
			++debugNonASCII;
	}
	
	Txt* backUp = nullptr;
	if (debugNonASCII > 0)
		backUp = NEW Txt (*this);
#endif

	// this should be avoided, since it can lead to information loss
	b8 result = const_cast <Txt&> (*this).toMultiByte (destCodePage);

#if DEVELOPMENT && SMTG_STRING_CHECK_CONVERSION
	if (backUp)
	{
		Txt temp (*this);
		temp.toWideString (destCodePage);
		
		if (temp != *backUp)
		{
			backUp->toMultiByte (kCP_Utf8);
			SMTG_STRING_CHECK_MSG ("Indirect string conversion information loss !   %d/%d non ASCII chars:   \"%s\"   ->    \"%s\"\n", debugNonASCII, debugLen, backUp->buffer8, buffer8);
		}
		else
			SMTG_STRING_CHECK_MSG ("Indirect string potential conversion information loss !   %d/%d non ASCII chars   result: \"%s\"\n", debugNonASCII, debugLen, buffer8);

		delete backUp;
	}
#endif

	return result;
}

//-----------------------------------------------------------------------------
b8 Txt::toMultiByte (u32 destCodePage)
{
	if (isWide)
	{
		if (buffer16 && len > 0)
		{
			i32 numChars = wideStringToMultiByte (nullptr, buffer16, 0, destCodePage) + sizeof (char8);
			char8* newStr = (char8*) malloc (numChars * sizeof (char8));
			if (wideStringToMultiByte (newStr, buffer16, numChars, destCodePage) <= 0)
			{
				free (newStr);
				return false;
			}
			free (buffer16);
			buffer8 = newStr;
			isWide = false;
			updateLength ();
		}
		isWide = false;
	}
	else if (destCodePage != kCP_Default)
	{
		if (toWideString () == false)
			return false;
		return toMultiByte (destCodePage);
	}
	return true;
}

//-----------------------------------------------------------------------------
z0 Txt::fromUTF8 (const char8* utf8String)
{
	if (buffer8 != utf8String)
		resize (0, false);
	_toWideString (utf8String, static_cast<i32> (strlen (utf8String)), kCP_Utf8);
}

//-----------------------------------------------------------------------------
b8 Txt::normalize (UnicodeNormalization n)
{
	if (isWide == false)
		return false;

	if (buffer16 == nullptr)
		return true;

#if SMTG_OS_WINDOWS
#ifdef UNICODE
	if (n != kUnicodeNormC)
		return false;

	u32 normCharCount = static_cast<u32> (FoldString (MAP_PRECOMPOSED, wscast (buffer16), len, nullptr, 0));
	if (normCharCount == len)
		return true;

	char16* newString = (char16*)malloc ((normCharCount + 1) * sizeof (char16));
	u32 converterCount = static_cast<u32> (FoldString (MAP_PRECOMPOSED, wscast (buffer16), len, wscast (newString), normCharCount + 1));
	if (converterCount != normCharCount)
	{
		free (newString);
		return false;
	}
	newString [converterCount] = 0;
	free (buffer16);
	buffer16 = newString;
	updateLength ();
	return true;
#else
	return false;
#endif

#elif SMTG_OS_MACOS
	CFMutableStringRef origStr = (CFMutableStringRef)toCFStringRef (0xFFFF, true);
	if (origStr)
	{
		CFStringNormalizationForm normForm = kCFStringNormalizationFormD;
		switch (n)
		{
			case kUnicodeNormC: normForm = kCFStringNormalizationFormC; break;
			case kUnicodeNormD: normForm = kCFStringNormalizationFormD; break;
			case kUnicodeNormKC: normForm = kCFStringNormalizationFormKC; break;
			case kUnicodeNormKD: normForm = kCFStringNormalizationFormKD; break;
		}
		CFStringNormalize (origStr, normForm);
		b8 result = fromCFStringRef (origStr);
		CFRelease (origStr);
		return result;
	}
	return false;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
z0 Txt::tryFreeBuffer ()
{
	if (buffer)
	{
		free (buffer);
		buffer = nullptr;
	}
}

//-----------------------------------------------------------------------------
b8 Txt::resize (u32 newLength, b8 wide, b8 fill)
{
	if (newLength == 0)
	{
		tryFreeBuffer ();
		len = 0;
		isWide = wide ? 1 : 0;
	}
	else
	{
		size_t newCharSize = wide ? sizeof (char16) : sizeof (char8);
		size_t oldCharSize = (isWide != 0) ? sizeof (char16) : sizeof (char8);

		size_t newBufferSize = (newLength + 1) * newCharSize;
		size_t oldBufferSize = (len + 1) * oldCharSize;

		isWide = wide ? 1 : 0;

		if (buffer)
		{
			if (newBufferSize != oldBufferSize)
			{
				uk newstr = realloc (buffer, newBufferSize);
				if (newstr == nullptr)
					return false;
				buffer = newstr;
				if (isWide)
					buffer16[newLength] = 0;
				else
					buffer8[newLength] = 0;
			}
			else if (wide && newCharSize != oldCharSize)
				buffer16[newLength] = 0;
		}
		else
		{
			uk newstr = malloc (newBufferSize);
			if (newstr == nullptr)
				return false;
			buffer = newstr;
			if (isWide)
			{
				buffer16[0] = 0;
				buffer16[newLength] = 0;
			}
			else
			{
				buffer8[0] = 0;
				buffer8[newLength] = 0;
			}
		}

		if (fill && len < newLength && buffer)
		{
			if (isWide)
			{
				char16 c = ' ';	
				for (u32 i = len; i < newLength; i++)
					buffer16 [i] = c;
			}
			else
			{
				memset (buffer8 + len, ' ', newLength - len);
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
b8 Txt::setChar8 (u32 index, char8 c)
{
	if (index == len && c == 0)
		return true;

	if (index >= len)
	{
		if (c == 0)
		{
			if (resize (index, isWide, true) == false)
				return false;
			len = index;
			return true;
		}
		
		if (resize (index + 1, isWide, true) == false)
			return false;
		len = index + 1;
	}
	
	if (index < len && buffer)
	{
		if (isWide)
		{
			if (c == 0)
				buffer16[index] = 0;
			else
			{
				char8 src[] = {c, 0};
				char16 dest[8] = {0};
				if (multiByteToWideString (dest, src, 2) > 0)
					buffer16[index] = dest[0];
			}
			SMTG_ASSERT (buffer16[len] == 0)
		}
		else
		{
			buffer8[index] = c;
			SMTG_ASSERT (buffer8[len] == 0)
		}

		if (c == 0)
			updateLength ();

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
b8 Txt::setChar16 (u32 index, char16 c)
{
	if (index == len && c == 0)
		return true;

	if (index >= len)
	{
		if (c == 0)
		{
			if (resize (index, isWide, true) == false)
				return false;
			len = index;
			return true;
		}
		if (resize (index + 1, isWide, true) == false)
			return false;
		len = index + 1;
	}

	if (index < len && buffer)
	{
		if (isWide)
		{
			buffer16[index] = c;
			SMTG_ASSERT (buffer16[len] == 0)
		}
		else
		{
			SMTG_ASSERT (buffer8[len] == 0)
			char16 src[] = {c, 0};
			char8 dest[8] = {0};
			if (wideStringToMultiByte (dest, src, 2) > 0 && dest[1] == 0)
				buffer8[index] = dest[0];
			else
				return false;
		}

		if (c == 0)
			updateLength ();

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
Txt& Txt::assign (const ConstString& str, i32 n)
{
	if (str.isWideString ())
		return assign (str.text16 (), n < 0 ? str.length () : n);
	return assign (str.text8 (), n < 0 ? str.length () : n);
}

//-----------------------------------------------------------------------------
Txt& Txt::assign (const char8* str, i32 n, b8 isTerminated)
{
	if (str == buffer8)
		return *this;

	if (isTerminated)
	{
		u32 stringLength = (u32)((str) ? strlen (str) : 0);
		n = n < 0 ? stringLength : Min<u32> (n, stringLength);
	}
	else if (n < 0)
		return *this;

	if (resize (n, false))
	{
		if (buffer8 && n > 0 && str)
		{
			memcpy (buffer8, str, n * sizeof (char8));
			SMTG_ASSERT (buffer8[n] == 0)
		}
		isWide = 0;
		len = n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::assign (const char16* str, i32 n, b8 isTerminated)
{
	if (str == buffer16)
		return *this;

	if (isTerminated)
	{
		u32 stringLength = (u32)((str) ? strlen16 (str) : 0);
		n = n < 0 ? stringLength : Min<u32> (n, stringLength);
	}
	else if (n < 0)
		return *this;

	if (resize (n, true))
	{
		if (buffer16 && n > 0 && str)
		{
			memcpy (buffer16, str, n * sizeof (char16));
			SMTG_ASSERT (buffer16[n] == 0)
		}
		isWide = 1;
		len = n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::assign (char8 c, i32 n)
{
	if (resize (n, false))
	{
		if (buffer8 && n > 0)
		{
			memset (buffer8, c, n * sizeof (char8));
			SMTG_ASSERT (buffer8[n] == 0)
		}
		isWide = 0;
		len = n;
	}
	return *this;

}

//-----------------------------------------------------------------------------
Txt& Txt::assign (char16 c, i32 n)
{
	if (resize (n, true))
	{
		if (buffer && n > 0)
		{
			for (i32 i = 0; i < n; i++)
				buffer16[i] = c;
			SMTG_ASSERT (buffer16[n] == 0)
		}
		isWide = 1;
		len = n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::append (const ConstString& str, i32 n)
{
	if (str.isWideString ())
		return append (str.text16 (), n);
	return append (str.text8 (), n);
}

//-----------------------------------------------------------------------------
Txt& Txt::append (const char8* str, i32 n)
{
	if (str == buffer8)
		return *this;

	if (len == 0)
		return assign (str, n);

	if (isWide)
	{
		Txt tmp (str);
		if (tmp.toWideString () == false)
			return *this;

		return append (tmp.buffer16, n);
	}

	u32 stringLength = (u32)((str) ? strlen (str) : 0);
	n = n < 0 ? stringLength : Min<u32> (n, stringLength);

	if (n > 0)
	{
		i32 newlen = n + len;
		if (!resize (newlen, false))
			return *this;

		if (buffer && str)
		{
			memcpy (buffer8 + len, str, n * sizeof (char8));
			SMTG_ASSERT (buffer8[newlen] == 0)
		}

		len += n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::append (const char16* str, i32 n)
{
	if (str == buffer16)
		return *this;

	if (len == 0)
		return assign (str, n);

	if (!isWide)
	{
		if (toWideString () == false)
			return *this;
	}

	u32 stringLength = (u32)((str) ? strlen16 (str) : 0);
	n = n < 0 ? stringLength : Min<u32> (n, stringLength);

	if (n > 0)
	{
		i32 newlen = n + len;
		if (!resize (newlen, true))
			return *this;

		if (buffer16 && str)
		{
			memcpy (buffer16 + len, str, n * sizeof (char16));
			SMTG_ASSERT (buffer16[newlen] == 0)
		}

		len += n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::append (const char8 c, i32 n)
{
	char8 str[] = {c, 0};
	if (n == 1)
	{
		return append (str, 1);
	}
	if (n > 1)
	{
		if (isWide)
		{
			Txt tmp (str);
			if (tmp.toWideString () == false)
				return *this;

			return append (tmp.buffer16[0], n);
		}

		i32 newlen = n + len;
		if (!resize (newlen, false))
			return *this;

		if (buffer)
		{
			memset (buffer8 + len, c, n * sizeof (char8));
			SMTG_ASSERT (buffer8[newlen] == 0)
		}

		len += n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::append (const char16 c, i32 n)
{
	if (n == 1)
	{
		char16 str[] = {c, 0};
		return append (str, 1);
	}
	if (n > 1)
	{
		if (!isWide)
		{
			if (toWideString () == false)
				return *this;
		}

		i32 newlen = n + len;
		if (!resize (newlen, true))
			return *this;

		if (buffer16)
		{
			for (i32 i = len; i < newlen; i++)
				buffer16[i] = c;
			SMTG_ASSERT (buffer16[newlen] == 0)
		}

		len += n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::insertAt (u32 idx, const ConstString& str, i32 n)
{
	if (str.isWideString ())
		return insertAt (idx, str.text16 (), n);
	return insertAt (idx, str.text8 (), n);
}

//-----------------------------------------------------------------------------
Txt& Txt::insertAt (u32 idx, const char8* str, i32 n)
{
	if (idx > len)
		return *this;

	if (isWide)
	{
		Txt tmp (str);
		if (tmp.toWideString () == false)
			return *this;
		return insertAt (idx, tmp.buffer16, n);
	}

	u32 stringLength = (u32)((str) ? strlen (str) : 0);
	n = n < 0 ? stringLength : Min<u32> (n, stringLength);

	if (n > 0)
	{
		i32 newlen = len + n;
		if (!resize (newlen, false))
			return *this;

		if (buffer && str)
		{
			if (idx < len)
				memmove (buffer8 + idx + n, buffer8 + idx, (len - idx) * sizeof (char8));
			memcpy (buffer8 + idx, str, n * sizeof (char8));
			SMTG_ASSERT (buffer8[newlen] == 0)
		}

		len += n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::insertAt (u32 idx, const char16* str, i32 n)
{
	if (idx > len)
		return *this;

	if (!isWide)
	{
		if (toWideString () == false)
			return *this;
	}

	u32 stringLength = (u32)((str) ? strlen16 (str) : 0);
	n = n < 0 ? stringLength : Min<u32> (n, stringLength);

	if (n > 0)
	{
		i32 newlen = len + n;
		if (!resize (newlen, true))
			return *this;

		if (buffer && str)
		{
			if (idx < len)
				memmove (buffer16 + idx + n, buffer16 + idx, (len - idx) * sizeof (char16));
			memcpy (buffer16 + idx, str, n * sizeof (char16));
			SMTG_ASSERT (buffer16[newlen] == 0)
		}

		len += n;
	}
	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::replace (u32 idx, i32 n1, const ConstString& str, i32 n2)
{
	if (str.isWideString ())
		return replace (idx, n1, str.text16 (), n2);
	return replace (idx, n1, str.text8 (), n2);
}

// "replace" replaces n1 number of characters at the specified index with
// n2 characters from the specified string.
//-----------------------------------------------------------------------------
Txt& Txt::replace (u32 idx, i32 n1, const char8* str, i32 n2)
{
	if (idx > len || str == nullptr)
		return *this;

	if (isWide)
	{
		Txt tmp (str);
		if (tmp.toWideString () == false)
			return *this;
		if (tmp.length () == 0 || n2 == 0)
			return remove (idx, n1);
		return replace (idx, n1, tmp.buffer16, n2);
	}

	if (n1 < 0 || idx + n1 > len)
		n1 = len - idx;
	if (n1 == 0)
		return *this;

	u32 stringLength = (u32)((str) ? strlen (str) : 0);
	n2 = n2 < 0 ? stringLength : Min<u32> (n2, stringLength);

	u32 newlen = len - n1 + n2;
	if (newlen > len)
		if (!resize (newlen, false))
			return *this;

	if (buffer)
	{
		memmove (buffer8 + idx + n2, buffer8 + idx + n1, (len - (idx + n1)) * sizeof (char8));
		memcpy (buffer8 + idx, str, n2 * sizeof (char8));
		buffer8[newlen] = 0;	// cannot be removed because resize is not called called in all cases (newlen > len)
	}

	len = newlen;

	return *this;
}

//-----------------------------------------------------------------------------
Txt& Txt::replace (u32 idx, i32 n1, const char16* str, i32 n2)
{
	if (idx > len || str == nullptr)
		return *this;

	if (!isWide)
	{
		if (toWideString () == false)
			return *this;
	}

	if (n1 < 0 || idx + n1 > len)
		n1 = len - idx;
	if (n1 == 0)
		return *this;

	u32 stringLength = (u32)((str) ? strlen16 (str) : 0);
	n2 = n2 < 0 ? stringLength : Min<u32> (n2, stringLength);

	u32 newlen = len - n1 + n2;
	if (newlen > len)
		if (!resize (newlen, true))
			return *this;

	if (buffer)
	{
		memmove (buffer16 + idx + n2, buffer16 + idx + n1, (len - (idx + n1)) * sizeof (char16));
		memcpy (buffer16 + idx, str, n2 * sizeof (char16));
		buffer16[newlen] = 0;	// cannot be removed because resize is not called called in all cases (newlen > len)
	}

	len = newlen;

	return *this;
}

//-----------------------------------------------------------------------------
i32 Txt::replace (const char8* toReplace, const char8* toReplaceWith, b8 all, CompareMode m)
{
	if (toReplace == nullptr || toReplaceWith == nullptr)
		return 0;

	i32 result = 0;

	i32 idx = findFirst (toReplace, -1, m);
	if (idx > -1)
	{
		i32 toReplaceLen = static_cast<i32> (strlen (toReplace));
		i32 toReplaceWithLen = static_cast<i32> (strlen (toReplaceWith));
		while (idx > -1)
		{
			replace (idx, toReplaceLen, toReplaceWith, toReplaceWithLen);
			result++;

			if (all)
				idx = findNext (idx + toReplaceWithLen , toReplace, -1, m);
			else
				break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
i32 Txt::replace (const char16* toReplace, const char16* toReplaceWith, b8 all, CompareMode m)
{
	if (toReplace == nullptr || toReplaceWith == nullptr)
		return 0;

	i32 result = 0;

	i32 idx = findFirst (toReplace, -1, m);
	if (idx > -1)
	{
		i32 toReplaceLen = strlen16 (toReplace);
		i32 toReplaceWithLen = strlen16 (toReplaceWith);
		while (idx > -1)
		{
			replace (idx, toReplaceLen, toReplaceWith, toReplaceWithLen);
			result++;

			if (all)
				idx = findNext (idx + toReplaceWithLen, toReplace, -1, m);
			else
				break;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
template <class T>
static b8 performReplace (T* str, const T* toReplace, T toReplaceBy)
{
	b8 anyReplace = false;
	T* p = str;
	while (*p)
	{
		const T* rep = toReplace;
		while (*rep)
		{
			if (*p == *rep)
			{
				*p = toReplaceBy;
				anyReplace = true;
				break;
			}
			rep++;
		}
		p++;
	}
	return anyReplace;
}

//-----------------------------------------------------------------------------
b8 Txt::replaceChars8 (const char8* toReplace, char8 toReplaceBy)
{
	if (isEmpty ())
		return false;

	if (isWide)
	{
		Txt toReplaceW (toReplace);
		if (toReplaceW.toWideString () == false)
			return false;

		char8 src[] = {toReplaceBy, 0};
		char16 dest[2] = {0};
		if (multiByteToWideString (dest, src, 2) > 0)
		{
			return replaceChars16 (toReplaceW.text16 (), dest[0]);
		}
		return false;
	}

	if (toReplaceBy == 0)
		toReplaceBy = ' ';

	return performReplace<char8> (buffer8, toReplace, toReplaceBy);
}

//-----------------------------------------------------------------------------
b8 Txt::replaceChars16 (const char16* toReplace, char16 toReplaceBy)
{
	if (isEmpty ())
		return false;

	if (!isWide)
	{
		Txt toReplaceA (toReplace);
		if (toReplaceA.toMultiByte () == false)
			return false;

		if (toReplaceA.length () > 1)
		{
			SMTG_WARNING("cannot replace non ASCII chars on non Wide Txt")
			return false;
		}

		char16 src[] = {toReplaceBy, 0};
		char8 dest[8] = {0};
		if (wideStringToMultiByte (dest, src, 2) > 0 && dest[1] == 0)
			return replaceChars8 (toReplaceA.text8 (), dest[0]);

		return false;
	}

	if (toReplaceBy == 0)
		toReplaceBy = STR16 (' ');

	return performReplace<char16> (buffer16, toReplace, toReplaceBy);
}

// "remove" removes the specified number of characters from the string
// starting at the specified index.
//-----------------------------------------------------------------------------
Txt& Txt::remove (u32 idx, i32 n)
{
	if (isEmpty () || idx >= len || n == 0)
		return *this;

	if ((idx + n > len) || n < 0)
		n = len - idx;
	else
	{
		i32 toMove = len - idx - n;
		if (buffer)
		{
			if (isWide)
				memmove (buffer16 + idx, buffer16 + idx + n, toMove * sizeof (char16));
			else
				memmove (buffer8 + idx, buffer8 + idx + n, toMove * sizeof (char8));
		}
	}

	resize (len - n, isWide);
	updateLength ();

	return *this;
}

//-----------------------------------------------------------------------------
b8 Txt::removeSubString (const ConstString& subString, b8 allOccurences)
{
	b8 removed = false;
	while (!removed || allOccurences)
	{
		i32 idx = findFirst (subString);
		if (idx < 0)
			break;
		remove (idx, subString.length ());
		removed = true;
	}
	return removed;
}

//-----------------------------------------------------------------------------
template <class T, class F>
static u32 performTrim (T* str, u32 length, F func, b8 funcResult)
{
	u32 toRemoveAtHead = 0;
	u32 toRemoveAtTail = 0;

	T* p = str;

	while ((*p) && ((func (*p) != 0) == funcResult))
		p++;

	toRemoveAtHead = static_cast<u32> (p - str);

	if (toRemoveAtHead < length)
	{
		p = str + length - 1;

		while (((func (*p) != 0) == funcResult) && (p > str))
		{
			p--;
			toRemoveAtTail++;
		}
	}

	u32 newLength = length - (toRemoveAtHead + toRemoveAtTail);
	if (newLength != length)
	{
		if (toRemoveAtHead)
			memmove (str, str + toRemoveAtHead, newLength * sizeof (T));
	}
	return newLength;
}

// "trim" trims the leading and trailing unwanted characters from the string.
//-----------------------------------------------------------------------------
b8 Txt::trim (Txt::CharGroup group)
{
	if (isEmpty ())
		return false;

	u32 newLength;

	switch (group)
	{
		case kSpace:
			if (isWide)
				newLength = performTrim<char16> (buffer16, len, iswspace, true);
			else
				newLength = performTrim<char8> (buffer8, len, isspace, true);
			break;

		case kNotAlphaNum:
			if (isWide)
				newLength = performTrim<char16> (buffer16, len, iswalnum, false);
			else
				newLength = performTrim<char8> (buffer8, len, isalnum, false);
			break;

		case kNotAlpha:
			if (isWide)
				newLength = performTrim<char16> (buffer16, len, iswalpha, false);
			else
				newLength = performTrim<char8> (buffer8, len, isalpha, false);
			break;
            
        default: // Undefined enum value
            return false;
	}

	if (newLength != len)
	{
		resize (newLength, isWide);
		len = newLength;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
template <class T, class F>
static u32 performRemove (T* str, u32 length, F func, b8 funcResult)
{
	T* p = str;

	while (*p)
	{
		if ((func (*p) != 0) == funcResult)
		{
			size_t toMove = length - (p - str);
			memmove (p, p + 1, toMove * sizeof (T));
			length--;
		}
		else
			p++;
	}
	return length;
}
//-----------------------------------------------------------------------------
z0 Txt::removeChars (CharGroup group)
{
	if (isEmpty ())
		return;

	u32 newLength;

	switch (group)
	{
		case kSpace:
			if (isWide)
				newLength = performRemove<char16> (buffer16, len, iswspace, true);
			else
				newLength = performRemove<char8> (buffer8, len, isspace, true);
			break;

		case kNotAlphaNum:
			if (isWide)
				newLength = performRemove<char16> (buffer16, len, iswalnum, false);
			else
				newLength = performRemove<char8> (buffer8, len, isalnum, false);
			break;

		case kNotAlpha:
			if (isWide)
				newLength = performRemove<char16> (buffer16, len, iswalpha, false);
			else
				newLength = performRemove<char8> (buffer8, len, isalpha, false);
			break;
            
        default: // Undefined enum value
            return;
	}

	if (newLength != len)
	{
		resize (newLength, isWide);
		len = newLength;
	}
}

//-----------------------------------------------------------------------------
template <class T>
static u32 performRemoveChars (T* str, u32 length, const T* toRemove)
{
	T* p = str;

	while (*p)
	{
		b8 found = false;
		const T* rem = toRemove;
		while (*rem)
		{
			if (*p == *rem)
			{
				found = true;
				break;
			}
			rem++;
		}

		if (found)
		{
			size_t toMove = length - (p - str);
			memmove (p, p + 1, toMove * sizeof (T));
			length--;
		}
		else
			p++;
	}
	return length;
}

//-----------------------------------------------------------------------------
b8 Txt::removeChars8 (const char8* toRemove)
{
	if (isEmpty () || toRemove == nullptr)
		return true;

	if (isWide)
	{
		Txt wStr (toRemove);
		if (wStr.toWideString () == false)
			return false;
		return removeChars16 (wStr.text16 ());
	}

	u32 newLength = performRemoveChars<char8> (buffer8, len, toRemove);

	if (newLength != len)
	{
		resize (newLength, false);
		len = newLength;
	}
	return true;
}

//-----------------------------------------------------------------------------
b8 Txt::removeChars16 (const char16* toRemove)
{
	if (isEmpty () || toRemove == nullptr)
		return true;

	if (!isWide)
	{
		Txt str8 (toRemove);
		if (str8.toMultiByte () == false)
			return false;
		return removeChars8 (str8.text8 ());
	}

	u32 newLength = performRemoveChars<char16> (buffer16, len, toRemove);

	if (newLength != len)
	{
		resize (newLength, true);
		len = newLength;
	}
	return true;
}

//-----------------------------------------------------------------------------
Txt& Txt::printf (const char8* format, ...)
{
	char8 string[kPrintfBufferSize];

	va_list marker;
	va_start (marker, format);
	
	vsnprintf (string, kPrintfBufferSize-1, format, marker);
	return assign (string);
}


//-----------------------------------------------------------------------------
Txt& Txt::printf (const char16* format, ...)
{
	char16 string[kPrintfBufferSize];

	va_list marker;
	va_start (marker, format);
	
	vsnwprintf (string, kPrintfBufferSize-1, format, marker);
	return assign (string);
}

//-----------------------------------------------------------------------------
Txt& Txt::vprintf (const char8* format, va_list args)
{
	char8 string[kPrintfBufferSize];

	vsnprintf (string, kPrintfBufferSize-1, format, args);
	return assign (string);
}

//-----------------------------------------------------------------------------
Txt& Txt::vprintf (const char16* format, va_list args)
{
	char16 string[kPrintfBufferSize];

	vsnwprintf (string, kPrintfBufferSize-1, format, args);
	return assign (string);
}

//-----------------------------------------------------------------------------
Txt& Txt::printInt64 (z64 value)
{
	if (isWide)
	{
	#if SMTG_CPP11
		return Txt::printf (STR("%") STR(FORMAT_INT64A), value);
	#else
		return Txt::printf (STR("%" FORMAT_INT64A), value);
	#endif
	}
	else
		return Txt::printf ("%" FORMAT_INT64A, value);
}

//-----------------------------------------------------------------------------
Txt& Txt::printFloat (f64 value, u32 maxPrecision)
{
	static constexpr auto kMaxAfterCommaResolution = 16;
	// escape point for integer values, avoid unnecessary complexity later on
	const b8 withinInt64Boundaries = value <= std::numeric_limits<z64>::max () && value >= std::numeric_limits<z64>::lowest ();
	if (withinInt64Boundaries && (maxPrecision == 0 || std::round (value) == value))
		return printInt64 (value);

	const auto absValue = std::abs (value);
	u32k valueExponent = absValue >= 1 ? std::log10 (absValue) : -std::log10 (absValue) + 1;

	maxPrecision = std::min<u32> (kMaxAfterCommaResolution - valueExponent, maxPrecision);

	if (isWide)
		printf (STR ("%s%dlf"), STR ("%."), maxPrecision);
	else
		printf ("%s%dlf", "%.", maxPrecision);

	if (isWide)
		printf (text16 (), value);
	else
		printf (text8 (), value);

	// trim trail zeros
	for (i32 i = length () - 1; i >= 0; i--)
	{
		if (isWide && testChar16 (i, '0') || testChar8 (i, '0'))
			remove (i);
		else if (isWide && testChar16(i,'.') || testChar8(i, '.'))
		{
			remove(i);
			break;
		}
		else
			break;
	}

	return *this;
}

//-----------------------------------------------------------------------------
b8 Txt::incrementTrailingNumber (u32 width, tchar separator, u32 minNumber, b8 applyOnlyFormat)
{
	if (width > 32)
		return false;

	z64 number = 1;
	i32 index = getTrailingNumberIndex ();
	if (index >= 0)
	{
		if (scanInt64 (number, index))
			if (!applyOnlyFormat)
				number++;

		if (separator != 0 && index > 0 && testChar (index - 1, separator) == true)
			index--;

		remove (index);
	}

	if (number < minNumber)
		number = minNumber;

	if (isWide)
	{
		char16 format[64];
		char16 trail[128];
		if (separator && isEmpty () == false)
		{
			sprintf16 (format, STR16 ("%%c%%0%uu"), width);
			sprintf16 (trail, format, separator, (u32) number);
		}
		else
		{
			sprintf16 (format, STR16 ("%%0%uu"), width);
			sprintf16 (trail, format, (u32) number);
		}
		append (trail);
	}
	else
	{
		static constexpr auto kFormatSize = 64u;
		static constexpr auto kTrailSize = 64u;
		t8 format[kFormatSize];
		t8 trail[kTrailSize];
		if (separator && isEmpty () == false)
		{
			snprintf (format, kFormatSize, "%%c%%0%uu", width);
			snprintf (trail, kTrailSize, format, separator, (u32) number);
		}
		else
		{
			snprintf (format, kFormatSize, "%%0%uu", width);
			snprintf (trail, kTrailSize, format, (u32) number);
		}
		append (trail);
	}

	return true;
}

//-----------------------------------------------------------------------------
z0 Txt::toLower (u32 index)
{
	if (buffer && index < len)
	{
		if (isWide)
			buffer16[index] = ConstString::toLower (buffer16[index]);
		else
			buffer8[index] = ConstString::toLower (buffer8[index]);
	}
}

//-----------------------------------------------------------------------------
z0 Txt::toLower ()
{
	i32 i = len;
	if (buffer && i > 0)
	{
		if (isWide)
		{
#if SMTG_OS_MACOS
			CFMutableStringRef cfStr = CFStringCreateMutableWithExternalCharactersNoCopy (kCFAllocator, (UniChar*)buffer16, len, len+1, kCFAllocatorNull);
			CFStringLowercase (cfStr, NULL);
			CFRelease (cfStr);
#else
			char16* c = buffer16;
			while (i--)
			{
				*c = ConstString::toLower (*c);
				c++;
			}
#endif
		}
		else
		{
			char8* c = buffer8;
			while (i--)
			{
				*c = ConstString::toLower (*c);
				c++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
z0 Txt::toUpper (u32 index)
{
	if (buffer && index < len)
	{
		if (isWide)
			buffer16[index] = ConstString::toUpper (buffer16[index]);
		else
			buffer8[index] = ConstString::toUpper (buffer8[index]);
	}
}

//-----------------------------------------------------------------------------
z0 Txt::toUpper ()
{
	i32 i = len;
	if (buffer && i > 0)
	{
		if (isWide)
		{
#if SMTG_OS_MACOS
			CFMutableStringRef cfStr = CFStringCreateMutableWithExternalCharactersNoCopy (kCFAllocator, (UniChar*)buffer16, len, len+1, kCFAllocatorNull);
			CFStringUppercase (cfStr, NULL);
			CFRelease (cfStr);
#else
			char16* c = buffer16;
			while (i--)
			{
				*c = ConstString::toUpper (*c);
				c++;
			}
#endif
		}
		else
		{
			char8* c = buffer8;
			while (i--)
			{
				*c = ConstString::toUpper (*c);
				c++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
b8 Txt::fromVariant (const FVariant& var)
{
	switch (var.getType ())
	{
		case FVariant::kString8:
			assign (var.getString8 ());
			return true;

		case FVariant::kString16:
			assign (var.getString16 ());
			return true;

		case FVariant::kFloat:
			printFloat (var.getFloat ());
			return true;

		case FVariant::kInteger:
			printInt64 (var.getInt ());
			return true;

		case FVariant::kObject:
			if (auto string = ICast<Steinberg::IString> (var.getObject ()))
				if (string->isWideString ())
					assign (string->getText16 ());
				else
					assign (string->getText8 ());
			return true;

		default:
			remove ();
	}
	return false;
}

//-----------------------------------------------------------------------------
z0 Txt::toVariant (FVariant& var) const
{
	if (isWide)
	{
		var.setString16 (text16 ());
	}
	else
	{
		var.setString8 (text8 ());
	}
}

//-----------------------------------------------------------------------------
b8 Txt::fromAttributes (IAttributes* a, IAttrID attrID)
{
	FVariant variant;
	if (a->get (attrID, variant) == kResultTrue)
		return fromVariant (variant);
	return false;
}

//-----------------------------------------------------------------------------
b8 Txt::toAttributes (IAttributes* a, IAttrID attrID)
{
	FVariant variant;
	toVariant (variant);
	if (a->set (attrID, variant) == kResultTrue)
		return true;
	return false;
}

// "swapContent" swaps ownership of the strings pointed to
//-----------------------------------------------------------------------------
z0 Txt::swapContent (Txt& s)
{
	uk tmp = s.buffer;
	u32 tmpLen = s.len;
	b8 tmpWide = s.isWide;
	s.buffer = buffer;
	s.len = len;
	s.isWide = isWide;
	buffer = tmp;
	len = tmpLen;
	isWide = tmpWide;
}

//-----------------------------------------------------------------------------
z0 Txt::take (Txt& other)
{
	resize (0, other.isWide);
	buffer = other.buffer;
	len = other.len;

	other.buffer = nullptr;
	other.len = 0;
}

//-----------------------------------------------------------------------------
z0 Txt::take (uk b, b8 wide)
{
	resize (0, wide);
	buffer = b;
	isWide = wide;
	updateLength ();
}

//-----------------------------------------------------------------------------
uk Txt::pass ()
{
	uk res = buffer;
	len = 0;
	buffer = nullptr;
	return res;
}

//-----------------------------------------------------------------------------
z0 Txt::passToVariant (FVariant& var)
{
	uk passed = pass ();

	if (isWide)
	{
		if (passed)
		{
			var.setString16 ((const char16*)passed);
			var.setOwner (true);
		}
		else
			var.setString16 (kEmptyString16);
	}
	else
	{
		if (passed)
		{
			var.setString8 ((const char8*)passed);
			var.setOwner (true);
		}
		else
			var.setString8 (kEmptyString8);
	}
}


//-----------------------------------------------------------------------------
u8* Txt::toPascalString (u8* buf)
{
	if (buffer)
	{
		if (isWide)
		{
			Txt tmp (*this);
			tmp.toMultiByte ();
			return tmp.toPascalString (buf);
		}

		i32 length = len;
		if (length > 255)
			length = 255;
		buf[0] = (u8)length;
		while (length >= 0)
		{
			buf[length + 1] = buffer8[length];
			length--;
		}
		return buf;
	}
	
	*buf = 0;
	return buf;
}

//-----------------------------------------------------------------------------
const Txt& Txt::fromPascalString (u8k* buf)
{
	resize (0, false);
	isWide = 0;
	i32 length = buf[0];
	resize (length + 1, false);
	buffer8[length] = 0;	// cannot be removed, because we only do the 0-termination for multibyte buffer8
	while (--length >= 0)
		buffer8[length] = buf[length + 1];
	len = buf[0];
	return *this;
}

#if SMTG_OS_MACOS

//-----------------------------------------------------------------------------
b8 Txt::fromCFStringRef (ukk cfStr, u32 encoding)
{
	if (cfStr == 0)
		return false;

	CFStringRef strRef = (CFStringRef)cfStr;
	if (isWide)
	{
		CFRange range = { 0, CFStringGetLength (strRef)};
		CFIndex usedBytes;
		if (resize (static_cast<i32> (range.length + 1), true))
		{
			if (encoding == 0xFFFF)
				encoding = kCFStringEncodingUnicode;
			if (CFStringGetBytes (strRef, range, encoding, ' ', false, (UInt8*)buffer16, range.length * 2, &usedBytes) > 0)
			{
				buffer16[usedBytes/2] = 0;
				this->len = strlen16 (buffer16);
				return true;
			}
		}
	}
	else
	{
		if (cfStr == 0)
			return false;
		if (encoding == 0xFFFF)
			encoding = kCFStringEncodingASCII;
		i32 len = static_cast<i32> (CFStringGetLength (strRef) * 2);
		if (resize (++len, false))
		{
			if (CFStringGetCString (strRef, buffer8, len, encoding))
			{
				this->len = static_cast<i32> (strlen (buffer8));
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
uk ConstString::toCFStringRef (u32 encoding, b8 mutableCFString) const
{
	if (mutableCFString)
	{
		CFMutableStringRef str = CFStringCreateMutable (kCFAllocator, 0);
		if (isWide)
		{
			CFStringAppendCharacters (str, (const UniChar *)buffer16, len);
			return str;
		}
		else
		{
			if (encoding == 0xFFFF)
				encoding = kCFStringEncodingASCII;
			CFStringAppendCString (str, buffer8, encoding);
			return str;
		}
	}
	else
	{
		if (isWide)
		{
			if (encoding == 0xFFFF)
				encoding = kCFStringEncodingUnicode;
			return (uk)CFStringCreateWithBytes (kCFAllocator, (u8k*)buffer16, len * 2, encoding, false);
		}
		else
		{
			if (encoding == 0xFFFF)
				encoding = kCFStringEncodingASCII;
			if (buffer8)
				return (uk)CFStringCreateWithCString (kCFAllocator, buffer8, encoding);
			else
				return (uk)CFStringCreateWithCString (kCFAllocator, "", encoding);
		}
	}
	return nullptr;
}

#endif

//-----------------------------------------------------------------------------
u32 hashString8 (const char8* s, u32 m)
{
	u32 h = 0;
	if (s)
	{
		for (h = 0; *s != '\0'; s++)
			h = (64 * h + *s) % m;
	}
	return h;
}

//-----------------------------------------------------------------------------
u32 hashString16 (const char16* s, u32 m)
{
	u32 h = 0;
	if (s)
	{
		for (h = 0; *s != 0; s++)
			h = (64 * h + *s) % m;
	}
	return h;
}

//------------------------------------------------------------------------
template <class T> i32 tstrnatcmp (const T* s1, const T* s2, b8 caseSensitive = true)
{
	if (s1 == nullptr && s2 == nullptr)
		return 0;
	if (s1 == nullptr)
		return -1;
	if (s2 == nullptr)
		return 1;

	while (*s1 && *s2)
	{
		if (ConstString::isCharDigit (*s1) && ConstString::isCharDigit (*s2))
		{
			i32 s1LeadingZeros = 0;
			while (*s1 == '0')
			{
				s1++; // skip leading zeros
				s1LeadingZeros++;
			}
			i32 s2LeadingZeros = 0;
			while (*s2 == '0')
			{
				s2++; // skip leading zeros
				s2LeadingZeros++;
			}

			i32 countS1Digits = 0;
			while (*(s1 + countS1Digits) && ConstString::isCharDigit (*(s1 + countS1Digits)))
				countS1Digits++;
			i32 countS2Digits = 0;
			while (*(s2 + countS2Digits) && ConstString::isCharDigit (*(s2 + countS2Digits)))
				countS2Digits++;

			if (countS1Digits != countS2Digits)
				return countS1Digits - countS2Digits; // one number is longer than the other

			for (i32 i = 0; i < countS1Digits; i++)
			{
				// countS1Digits == countS2Digits
				if (*s1 != *s2)
					return (i32)(*s1 - *s2); // the digits differ
				s1++;
				s2++;
			}

			if (s1LeadingZeros != s2LeadingZeros)
				return s1LeadingZeros - s2LeadingZeros; // differentiate by the number of leading zeros
		}
		else
		{
			if (caseSensitive == false)
			{
				T srcToUpper = static_cast<T> (toupper (*s1));
				T dstToUpper = static_cast<T> (toupper (*s2));
				if (srcToUpper != dstToUpper)
					return (i32)(srcToUpper - dstToUpper);
			}
			else if (*s1 != *s2)
				return (i32)(*s1 - *s2);

			s1++;
			s2++;
		}
	}

	if (*s1 == 0 && *s2 == 0)
		return 0;
	if (*s1 == 0)
		return -1;
	if (*s2 == 0)
		return 1;
	return (i32)(*s1 - *s2);
}

//------------------------------------------------------------------------
i32 strnatcmp8 (const char8* s1, const char8* s2, b8 caseSensitive /*= true*/)
{
	return tstrnatcmp (s1, s2, caseSensitive);
}

//------------------------------------------------------------------------
i32 strnatcmp16 (const char16* s1, const char16* s2, b8 caseSensitive /*= true*/)
{
	return tstrnatcmp (s1, s2, caseSensitive);
}

//-----------------------------------------------------------------------------
// StringObject Implementation
//-----------------------------------------------------------------------------
z0 PLUGIN_API StringObject::setText (const char8* text)
{
	assign (text);
}

//-----------------------------------------------------------------------------
z0 PLUGIN_API StringObject::setText8 (const char8* text)
{	
	assign (text);
}

//-----------------------------------------------------------------------------
z0 PLUGIN_API StringObject::setText16 (const char16* text)
{
	assign (text);
}

//-----------------------------------------------------------------------------
const char8* PLUGIN_API StringObject::getText8 ()
{
	return text8 ();
}

//-----------------------------------------------------------------------------
const char16* PLUGIN_API StringObject::getText16 ()
{
	return text16 ();
}

//-----------------------------------------------------------------------------
z0 PLUGIN_API StringObject::take (uk s, b8 _isWide)
{
	Txt::take (s, _isWide);
}

//-----------------------------------------------------------------------------
b8 PLUGIN_API StringObject::isWideString () const
{
	return Txt::isWideString ();
}

//------------------------------------------------------------------------
} // namespace Steinberg
