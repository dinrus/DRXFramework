//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fstring.h
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

#pragma once

#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/fstrdefs.h"
#include "pluginterfaces/base/istringresult.h"
#include "pluginterfaces/base/ipersistent.h"

#include "base/source/fobject.h"

#include <cstdarg>
#include <cstdlib>

namespace Steinberg {

class FVariant;
class Txt;

#ifdef UNICODE
static const b8 kWideStringDefault = true;
#else
static const b8 kWideStringDefault = false;
#endif

static u16k kBomUtf16 = 0xFEFF;					///< UTF16 Byte Order Mark
static const char8* const kBomUtf8 = "\xEF\xBB\xBF";	///< UTF8 Byte Order Mark
static const i32 kBomUtf8Length = 3;


enum MBCodePage
{
	kCP_ANSI = 0,			///< Default ANSI codepage.
	kCP_MAC_ROMAN = 2,		///< Default Mac codepage.

	kCP_ANSI_WEL = 1252,	///< West European Latin Encoding.
	kCP_MAC_CEE = 10029,	///< Mac Central European Encoding.
	kCP_Utf8 = 65001,		///< UTF8 Encoding.
	kCP_ShiftJIS = 932,		///< Shifted Japan Industrial Standard Encoding.
	kCP_US_ASCII = 20127,	///< US-ASCII (7-bit).

	kCP_Default = kCP_ANSI	///< Default ANSI codepage.
};

enum UnicodeNormalization
{
	kUnicodeNormC,	///< Unicode normalization Form C, canonical composition. 
	kUnicodeNormD,	///< Unicode normalization Form D, canonical decomposition. 
	kUnicodeNormKC,	///< Unicode normalization form KC, compatibility composition. 
	kUnicodeNormKD 	///< Unicode normalization form KD, compatibility decomposition. 
};

//------------------------------------------------------------------------
// Helper functions to create hash codes from string data.
//------------------------------------------------------------------------
extern u32 hashString8 (const char8* s, u32 m);
extern u32 hashString16 (const char16* s, u32 m);
inline u32 hashString (const tchar* s, u32 m)
{
#ifdef UNICODE
	return hashString16 (s, m);
#else
	return hashString8 (s, m);
#endif
}


//-----------------------------------------------------------------------------
/** Invariant Txt. 
@ingroup adt

A base class which provides methods to work with its
member string. Neither of the operations allows modifying the member string and 
that is why all operation are declared as const. 

There are operations for access, comparison, find, numbers and conversion.

Almost all operations exist in three versions for char8, char16 and the 
polymorphic type tchar. The type tchar can either be char8 or char16 depending
on whether UNICODE is activated or not.*/
//-----------------------------------------------------------------------------
class ConstString
{
public:
//-----------------------------------------------------------------------------
	ConstString (const char8* str, i32 length = -1);	///< Assign from string of type char8 (length=-1: all)
	ConstString (const char16* str, i32 length = -1);	///< Assign from string of type char16 (length=-1: all)
	ConstString (const ConstString& str, i32 offset = 0, i32 length = -1); ///< Copy constructor (length=-1: all).
	ConstString (const FVariant& var);	///< Assign a string from FVariant
	ConstString ();
	virtual ~ConstString () {}			///< Destructor.

	// access -----------------------------------------------------------------
	virtual i32 length () const {return static_cast<i32> (len);}	///< Return length of string
	inline b8 isEmpty () const {return buffer == nullptr || len == 0;}		///< Return true if string is empty

	operator const char8* () const {return text8 ();} 							///< Returns pointer to string of type char8 (no modification allowed)
	operator const char16* () const {return text16 ();}							///< Returns pointer to string of type char16(no modification allowed)
	inline tchar operator[] (short idx) const {return getChar (static_cast<u32> (idx));} ///< Returns character at 'idx'
	inline tchar operator[] (i64 idx) const {return getChar (static_cast<u32> (idx));}
	inline tchar operator[] (i32 idx) const {return getChar (static_cast<u32> (idx));}
	inline tchar operator[] (u16 idx) const {return getChar (idx);}
	inline tchar operator[] (u64 idx) const {return getChar (static_cast<u32> (idx));}
	inline tchar operator[] (u32 idx) const {return getChar (idx);}

	inline virtual const char8* text8 () const;		///< Returns pointer to string of type char8
	inline virtual const char16* text16 () const;	///< Returns pointer to string of type char16
	inline virtual const tchar* text () const;		///< Returns pointer to string of type tchar
	inline virtual ukk ptr () const {return buffer;}	///< Returns pointer to string of type z0

	inline virtual char8 getChar8 (u32 index) const;		///< Returns character of type char16 at 'index'
	inline virtual char16 getChar16 (u32 index) const;	///< Returns character of type char8 at 'index'
	inline tchar getChar (u32 index) const;				///< Returns character of type tchar at 'index'
	inline tchar getCharAt (u32 index) const;			///< Returns character of type tchar at 'index', no conversion!

	b8 testChar8 (u32 index, char8 c) const;			///< Возвращает true, если character is equal at position 'index'
	b8 testChar16 (u32 index, char16 c) const;
	inline b8 testChar (u32 index, char8 c) const {return testChar8 (index, c);}
	inline b8 testChar (u32 index, char16 c) const {return testChar16 (index, c);}

	b8 extract (Txt& result, u32 idx, i32 n = -1) const;		///< Get n characters i64 substring starting at index (n=-1: until end)
	i32 copyTo8 (char8* str, u32 idx = 0, i32 n = -1) const;
	i32 copyTo16 (char16* str, u32 idx = 0, i32 n = -1) const;
	i32 copyTo (tchar* str, u32 idx = 0, i32 n = -1) const;
	z0 copyTo (IStringResult* result) const;							///< Copies whole member string
	z0 copyTo (IString& string) const;							    ///< Copies whole member string

	inline u32 hash (u32 tsize) const 
	{
		return isWide ? hashString16 (buffer16, tsize) : hashString8 (buffer8, tsize) ;  
	}
	//-------------------------------------------------------------------------

	// compare ----------------------------------------------------------------
	enum CompareMode
	{
		kCaseSensitive,		///< Comparison is done with regard to character's case
		kCaseInsensitive	///< Comparison is done without regard to character's case
	};

	i32 compareAt (u32 index, const ConstString& str, i32 n = -1, CompareMode m = kCaseSensitive) const; 				///< Compare n characters of str with n characters of this starting at index (return: see above)
	i32 compare (const ConstString& str, i32 n, CompareMode m = kCaseSensitive) const;										///< Compare n characters of str with n characters of this (return: see above)
	i32 compare (const ConstString& str, CompareMode m = kCaseSensitive) const;												///< Compare all characters of str with this (return: see above)

	i32 naturalCompare (const ConstString& str, CompareMode mode = kCaseSensitive) const;

	b8 startsWith (const ConstString& str, CompareMode m = kCaseSensitive) const;												///< Check if this starts with str
	b8 endsWith (const ConstString& str, CompareMode m = kCaseSensitive) const;												///< Check if this ends with str
	b8 contains (const ConstString& str, CompareMode m = kCaseSensitive) const;												///< Check if this contains str											

	// static methods
	static b8 isCharSpace (char8 character);	   ///< Возвращает true, если character is a space
	static b8 isCharSpace (char16 character);    ///< @copydoc isCharSpace(const char8)
	static b8 isCharAlpha (char8 character);	   ///< Возвращает true, если character is an alphabetic character
	static b8 isCharAlpha (char16 character);    ///< @copydoc isCharAlpha(const char8)
	static b8 isCharAlphaNum (char8 character);  ///< Возвращает true, если character is an alphanumeric character
	static b8 isCharAlphaNum (char16 character); ///< @copydoc isCharAlphaNum(const char8)
	static b8 isCharDigit (char8 character);	   ///< Возвращает true, если character is a number
	static b8 isCharDigit (char16 character);    ///< @copydoc isCharDigit(const char8)
	static b8 isCharAscii (char8 character);     ///< Возвращает true, если character is in ASCII range
	static b8 isCharAscii (char16 character);    ///< Возвращает true, если character is in ASCII range
	static b8 isCharUpper (char8 character);
	static b8 isCharUpper (char16 character);
	static b8 isCharLower (char8 character);
	static b8 isCharLower (char16 character);
	//-------------------------------------------------------------------------

	/** @name Find first occurrence of n characters of str in this (n=-1: all) ending at endIndex (endIndex = -1: all)*/
	///@{
	inline i32 findFirst (const ConstString& str, i32 n = -1, CompareMode m = kCaseSensitive, i32 endIndex = -1) const {return findNext (0, str, n, m, endIndex);}
	inline i32 findFirst (char8 c, CompareMode m = kCaseSensitive, i32 endIndex = -1) const {return findNext (0, c, m, endIndex);}
	inline i32 findFirst (char16 c, CompareMode m = kCaseSensitive, i32 endIndex = -1) const {return findNext (0, c, m, endIndex);}
	///@}
	/** @name Find next occurrence of n characters of str starting at startIndex in this (n=-1: all) ending at endIndex (endIndex = -1: all)*/
	///@{
	i32 findNext (i32 startIndex, const ConstString& str, i32 n = -1, CompareMode = kCaseSensitive, i32 endIndex = -1) const;
	i32 findNext (i32 startIndex, char8 c, CompareMode = kCaseSensitive, i32 endIndex = -1) const;
	i32 findNext (i32 startIndex, char16 c, CompareMode = kCaseSensitive, i32 endIndex = -1) const;
	///@}
	/** @name Find previous occurrence of n characters of str starting at startIndex in this (n=-1: all) */
	///@{
	i32 findPrev (i32 startIndex, const ConstString& str, i32 n = -1, CompareMode = kCaseSensitive) const;
	i32 findPrev (i32 startIndex, char8 c, CompareMode = kCaseSensitive) const;
	i32 findPrev (i32 startIndex, char16 c, CompareMode = kCaseSensitive) const;
	///@}
	
	inline i32 findLast (const ConstString& str, i32 n = -1, CompareMode m = kCaseSensitive) const {return findPrev (-1, str, n, m);}	///< Find last occurrence of n characters of str in this (n=-1: all)
	inline i32 findLast (char8 c, CompareMode m = kCaseSensitive) const {return findPrev (-1, c, m);}
	inline i32 findLast (char16 c, CompareMode m = kCaseSensitive) const {return findPrev (-1, c, m);}

	i32 countOccurences (char8 c, u32 startIndex, CompareMode = kCaseSensitive) const; ///< Counts occurences of c within this starting at index
	i32 countOccurences (char16 c, u32 startIndex, CompareMode = kCaseSensitive) const;
	i32 getFirstDifferent (const ConstString& str, CompareMode = kCaseSensitive) const;	///< Returns position of first different character
	//-------------------------------------------------------------------------

	// numbers ----------------------------------------------------------------
	b8 isDigit (u32 index) const;	///< Возвращает true, если character at position is a digit
	b8 scanFloat (f64& value, u32 offset = 0, b8 scanToEnd = true) const;		///< Converts string to f64 value starting at offset
	b8 scanInt64 (z64& value, u32 offset = 0, b8 scanToEnd = true) const;		///< Converts string to z64 value starting at offset
	b8 scanUInt64 (zu64& value, u32 offset = 0, b8 scanToEnd = true) const;	///< Converts string to zu64 value starting at offset
	b8 scanInt32 (i32& value, u32 offset = 0, b8 scanToEnd = true) const;		///< Converts string to i32 value starting at offset
	b8 scanUInt32 (u32& value, u32 offset = 0, b8 scanToEnd = true) const;	///< Converts string to u32 value starting at offset
	b8 scanHex (u8& value, u32 offset = 0, b8 scanToEnd = true) const;		///< Converts string to hex/u8 value starting at offset

	i32 getTrailingNumberIndex (u32 width = 0) const;	///< Returns start index of trailing number
	z64 getTrailingNumber (z64 fallback = 0) const;		///< Returns result of scanInt64 or the fallback
	z64 getNumber () const;								///< Returns result of scanInt64

	// static methods
	static b8 scanInt64_8 (const char8* text, z64& value, b8 scanToEnd = true);	///< Converts string of type char8 to z64 value
	static b8 scanInt64_16 (const char16* text, z64& value, b8 scanToEnd = true);	///< Converts string of type char16 to z64 value
	static b8 scanInt64 (const tchar* text, z64& value, b8 scanToEnd = true);		///< Converts string of type tchar to z64 value

	static b8 scanUInt64_8 (const char8* text, zu64& value, b8 scanToEnd = true);		///< Converts string of type char8 to zu64 value
	static b8 scanUInt64_16 (const char16* text, zu64& value, b8 scanToEnd = true);	///< Converts string of type char16 to zu64 value
	static b8 scanUInt64 (const tchar* text, zu64& value, b8 scanToEnd = true);		///< Converts string of type tchar to zu64 value

	static b8 scanInt32_8 (const char8* text, i32& value, b8 scanToEnd = true);		///< Converts string of type char8 to i32 value
	static b8 scanInt32_16 (const char16* text, i32& value, b8 scanToEnd = true);		///< Converts string of type char16 to i32 value
	static b8 scanInt32 (const tchar* text, i32& value, b8 scanToEnd = true);			///< Converts string of type tchar to i32 value

	static b8 scanUInt32_8 (const char8* text, u32& value, b8 scanToEnd = true);		///< Converts string of type char8 to i32 value
	static b8 scanUInt32_16 (const char16* text, u32& value, b8 scanToEnd = true);		///< Converts string of type char16 to i32 value
	static b8 scanUInt32 (const tchar* text, u32& value, b8 scanToEnd = true);			///< Converts string of type tchar to i32 value

	static b8 scanHex_8 (const char8* text, u8& value, b8 scanToEnd = true);		///< Converts string of type char8 to hex/unit8 value
	static b8 scanHex_16 (const char16* text, u8& value, b8 scanToEnd = true);	///< Converts string of type char16 to hex/unit8 value
	static b8 scanHex (const tchar* text, u8& value, b8 scanToEnd = true);		///< Converts string of type tchar to hex/unit8 value
	//-------------------------------------------------------------------------

	// conversion -------------------------------------------------------------
	z0 toVariant (FVariant& var) const;

	static char8 toLower (char8 c);		///< Converts to lower case
	static char8 toUpper (char8 c);		///< Converts to upper case
	static char16 toLower (char16 c);
	static char16 toUpper (char16 c);

	static i32 multiByteToWideString (char16* dest, const char8* source, i32 wcharCount, u32 sourceCodePage = kCP_Default);	///< If dest is zero, this returns the maximum number of bytes needed to convert source
	static i32 wideStringToMultiByte (char8* dest, const char16* source, i32 char8Count, u32 destCodePage = kCP_Default);	///< If dest is zero, this returns the maximum number of bytes needed to convert source

	b8 isWideString () const {return isWide != 0;}	///< Возвращает true, если string is wide
	b8 isAsciiString () const; 						///< Checks if all characters in string are in ascii range

	b8 isNormalized (UnicodeNormalization = kUnicodeNormC); ///< On PC only kUnicodeNormC is working

#if SMTG_OS_WINDOWS
	ConstString (const wchar_t* str, i32 length = -1) : ConstString (wscast (str), length) {}
	operator const wchar_t* () const { return wscast (text16 ());}
#endif

#if SMTG_OS_MACOS
	virtual uk toCFStringRef (u32 encoding = 0xFFFF, b8 mutableCFString = false) const;	///< CFString conversion
#endif
//-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
protected:

	union 
	{
		uk buffer;
		char8* buffer8;
		char16* buffer16;
	};
	u32 len : 30;
	u32 isWide : 1;
};

//-----------------------------------------------------------------------------
/** Txt.
@ingroup adt

Extends class ConstString by operations which allow modifications. 

If allocated externally and passed in via take() or extracted via pass(), memory
is expected to be allocated with C's malloc() (rather than new) and deallocated with free().
Use the NEWSTR8/16 and DELETESTR8/16 macros below.

\see ConstString */
//-----------------------------------------------------------------------------
class Txt : public ConstString
{
public:
	
//-----------------------------------------------------------------------------
	Txt ();
	Txt (const char8* str, MBCodePage codepage, i32 n = -1, b8 isTerminated = true);							///< assign n characters of str and convert to wide string by using the specified codepage
	Txt (const char8* str, i32 n = -1, b8 isTerminated = true);	///< assign n characters of str (-1: all)
	Txt (const char16* str, i32 n = -1, b8 isTerminated = true);	///< assign n characters of str (-1: all)
	Txt (const Txt& str, i32 n = -1);							///< assign n characters of str (-1: all)
	Txt (const ConstString& str, i32 n = -1);		///< assign n characters of str (-1: all)
	Txt (const FVariant& var);						///< assign from FVariant
	Txt (IString* str);						///< assign from IString
	~Txt () SMTG_OVERRIDE;

#if SMTG_CPP11_STDLIBSUPPORT
	Txt (Txt&& str);
	Txt& operator= (Txt&& str);
#endif

	// access------------------------------------------------------------------
	z0 updateLength (); ///< Call this when the string is truncated outside (not recommended though)
	const char8* text8 () const SMTG_OVERRIDE;
	const char16* text16 () const SMTG_OVERRIDE;
	char8 getChar8 (u32 index) const SMTG_OVERRIDE;
	char16 getChar16 (u32 index) const SMTG_OVERRIDE;

	b8 setChar8 (u32 index, char8 c);
	b8 setChar16 (u32 index, char16 c);
	inline b8 setChar (u32 index, char8 c) {return setChar8 (index, c);}
	inline b8 setChar (u32 index, char16 c) {return setChar16 (index, c);}
	//-------------------------------------------------------------------------

	// assignment--------------------------------------------------------------
	Txt& operator= (const char8* str) {return assign (str);}	///< Assign from a string of type char8
	Txt& operator= (const char16* str) {return assign (str);}
	Txt& operator= (const ConstString& str) {return assign (str);}
	Txt& operator= (const Txt& str) {return assign (str);}
	Txt& operator= (char8 c) {return assign (c);}
	Txt& operator= (char16 c) {return assign (c);}

	Txt& assign (const ConstString& str, i32 n = -1);			///< Assign n characters of str (-1: all)
	Txt& assign (const char8* str, i32 n = -1, b8 isTerminated = true);			///< Assign n characters of str (-1: all)
	Txt& assign (const char16* str, i32 n = -1, b8 isTerminated = true);			///< Assign n characters of str (-1: all)
	Txt& assign (char8 c, i32 n = 1);
	Txt& assign (char16 c, i32 n = 1);
	//-------------------------------------------------------------------------

	// concat------------------------------------------------------------------
	Txt& append (const ConstString& str, i32 n = -1);		///< Append n characters of str to this (n=-1: all)
	Txt& append (const char8* str, i32 n = -1);			///< Append n characters of str to this (n=-1: all)
	Txt& append (const char16* str, i32 n = -1);			///< Append n characters of str to this (n=-1: all)
	Txt& append (const char8 c, i32 n = 1);                ///< Append t8 c n times
	Txt& append (const char16 c, i32 n = 1);               ///< Append t8 c n times

	Txt& insertAt (u32 idx, const ConstString& str, i32 n = -1);	///< Insert n characters of str at position idx (n=-1: all)
	Txt& insertAt (u32 idx, const char8* str, i32 n = -1);	///< Insert n characters of str at position idx (n=-1: all)
	Txt& insertAt (u32 idx, const char16* str, i32 n = -1);	///< Insert n characters of str at position idx (n=-1: all)
	Txt& insertAt (u32 idx, char8 c) {char8 str[] = {c, 0}; return insertAt (idx, str, 1);} 
	Txt& insertAt (u32 idx, char16 c) {char16 str[] = {c, 0}; return insertAt (idx, str, 1);}

	Txt& operator+= (const Txt& str) {return append (str);}
	Txt& operator+= (const ConstString& str) {return append (str);}
	Txt& operator+= (const char8* str) {return append (str);}
	Txt& operator+= (const char16* str) {return append (str);}
	Txt& operator+= (const char8 c) {return append (c);}
	Txt& operator+= (const char16 c) {return append (c);}
	//-------------------------------------------------------------------------

	// replace-----------------------------------------------------------------
	Txt& replace (u32 idx, i32 n1, const ConstString& str, i32 n2 = -1);		///< Replace n1 characters of this (starting at idx) with n2 characters of str (n1,n2=-1: until end)
	Txt& replace (u32 idx, i32 n1, const char8* str, i32 n2 = -1);			///< Replace n1 characters of this (starting at idx) with n2 characters of str (n1,n2=-1: until end)
	Txt& replace (u32 idx, i32 n1, const char16* str, i32 n2 = -1);			///< Replace n1 characters of this (starting at idx) with n2 characters of str (n1,n2=-1: until end)

	i32 replace (const char8* toReplace, const char8* toReplaceWith, b8 all = false, CompareMode m = kCaseSensitive);			///< Replace find string with replace string - returns number of replacements
	i32 replace (const char16* toReplace, const char16* toReplaceWith, b8 all = false, CompareMode m = kCaseSensitive);		///< Replace find string with replace string - returns number of replacements

	b8 replaceChars8 (const char8* toReplace, char8 toReplaceBy); ///< Returns true when any replacement was done
	b8 replaceChars16 (const char16* toReplace, char16 toReplaceBy);
	inline b8 replaceChars8 (char8 toReplace, char8 toReplaceBy)  {char8 str[] = {toReplace, 0}; return replaceChars8 (str, toReplaceBy);}
	inline b8 replaceChars16 (char16 toReplace, char16 toReplaceBy)  {char16 str[] = {toReplace, 0}; return replaceChars16 (str, toReplaceBy);}
	inline b8 replaceChars (char8 toReplace, char8 toReplaceBy) {return replaceChars8 (toReplace, toReplaceBy);}
	inline b8 replaceChars (char16 toReplace, char16 toReplaceBy) {return replaceChars16 (toReplace, toReplaceBy);}
	inline b8 replaceChars (const char8* toReplace, char8 toReplaceBy) {return replaceChars8 (toReplace, toReplaceBy);}
	inline b8 replaceChars (const char16* toReplace, char16 toReplaceBy) {return replaceChars16 (toReplace, toReplaceBy);}
	//-------------------------------------------------------------------------

	// remove------------------------------------------------------------------
	Txt& remove (u32 index = 0, i32 n = -1);		///< Remove n characters from string starting at index (n=-1: until end)
	enum CharGroup {kSpace, kNotAlphaNum, kNotAlpha};
	b8 trim (CharGroup mode = kSpace);					///< Trim lead/trail.
	z0 removeChars (CharGroup mode = kSpace);				///< Removes all of group.
	b8 removeChars8 (const char8* which);					///< Remove all occurrences of each t8 in 'which'
	b8 removeChars16 (const char16* which);				///< Remove all occurrences of each t8 in 'which'
	inline b8 removeChars8 (const char8 which) {char8 str[] = {which, 0}; return removeChars8 (str); }     
	inline b8 removeChars16 (const char16 which) {char16 str[] = {which, 0}; return removeChars16 (str); }                
	inline b8 removeChars (const char8* which) {return removeChars8 (which);}
	inline b8 removeChars (const char16* which) {return removeChars16 (which);}
	inline b8 removeChars (const char8 which) {return removeChars8 (which);}
	inline b8 removeChars (const char16 which) {return removeChars16 (which);}
	b8 removeSubString (const ConstString& subString, b8 allOccurences = true);
	//-------------------------------------------------------------------------

	// print-------------------------------------------------------------------
	Txt& printf (const char8* format, ...);					///< Print formatted data into string
	Txt& printf (const char16* format, ...);					///< Print formatted data into string
	Txt& vprintf (const char8* format, va_list args);
	Txt& vprintf (const char16* format, va_list args);
	//-------------------------------------------------------------------------

	// numbers-----------------------------------------------------------------
	Txt& printInt64 (z64 value);

	/**
	* @brief				print a f32 into a string, trailing zeros will be trimmed
	* @param value			the floating value to be printed
	* @param maxPrecision	(optional) the max precision allowed for this, num of significant digits after the comma
	*						For instance printFloat (1.234, 2) => 1.23
	*  @return				the resulting string.
	*/
	Txt& printFloat (f64 value, u32 maxPrecision = 6);
	/** Increment the trailing number if present else start with minNumber, width specifies the string width format (width 2 for number 3 is 03),
		applyOnlyFormat set to true will only format the string to the given width without incrementing the founded trailing number */
	b8 incrementTrailingNumber (u32 width = 2, tchar separator = STR (' '), u32 minNumber = 1, b8 applyOnlyFormat = false);
	//-------------------------------------------------------------------------

	// conversion--------------------------------------------------------------
	b8 fromVariant (const FVariant& var);		///< Assigns string from FVariant
	z0 toVariant (FVariant& var) const;
	b8 fromAttributes (IAttributes* a, IAttrID attrID);	///< Assigns string from FAttributes
	b8 toAttributes (IAttributes* a, IAttrID attrID);

	z0 swapContent (Txt& s); 								///< Swaps ownership of the strings pointed to
	z0 take (Txt& str);      								///< Take ownership of the string of 'str'
	z0 take (uk _buffer, b8 wide);      					///< Take ownership of buffer
	uk pass ();
	z0 passToVariant (FVariant& var);							///< Pass ownership of buffer to Variant - sets Variant ownership

	z0 toLower (u32 index);								///< Lower case the character.
	z0 toLower ();											///< Lower case the string.
	z0 toUpper (u32 index);								///< Upper case the character.
	z0 toUpper ();											///< Upper case the string.

	u8* toPascalString (u8* buf);			///< Pascal string conversion
	const Txt& fromPascalString (u8k* buf);	///< Pascal string conversion

	b8 toWideString (u32 sourceCodePage = kCP_Default);	///< Converts to wide string according to sourceCodePage
	b8 toMultiByte (u32 destCodePage = kCP_Default);

	z0 fromUTF8 (const char8* utf8String);				///< Assigns from UTF8 string
	b8 normalize (UnicodeNormalization = kUnicodeNormC);	///< On PC only kUnicodeNormC is working

#if SMTG_OS_WINDOWS
	Txt (const wchar_t* str, i32 length = -1, b8 isTerminated = true) : Txt (wscast (str), length, isTerminated) {}
	Txt& operator= (const wchar_t* str) {return Txt::operator= (wscast (str)); }
#endif

#if SMTG_OS_MACOS
	virtual b8 fromCFStringRef (ukk, u32 encoding = 0xFFFF);	///< CFString conversion
#endif
	//-------------------------------------------------------------------------

	//-----------------------------------------------------------------------------
protected:
	b8 resize (u32 newSize, b8 wide, b8 fill = false);

private:
	b8 _toWideString (const char8* src, i32 length, u32 sourceCodePage = kCP_Default);
	z0 tryFreeBuffer ();
	b8 checkToMultiByte (u32 destCodePage = kCP_Default) const; // to remove debug code from inline - const_cast inside!!!
};

// Txt memory allocation macros
#define NEWSTR8(len)   ((char8*)::malloc(len))      // len includes trailing zero
#define NEWSTR16(len)  ((char16*)::malloc(2*(len)))
#define DELETESTR8(p)  (::free((tuk)(p)))         // cast to strip const
#define DELETESTR16(p) (::free((char16*)(p)))

// Txt concatenation functions.
inline Txt operator+ (const ConstString& s1, const ConstString& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const ConstString& s1, const char8* s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const ConstString& s1, const char16* s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const char8* s1, const ConstString& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const char16* s1, const ConstString& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const ConstString& s1, const Txt& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const Txt& s1, const ConstString& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const Txt& s1, const Txt& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const Txt& s1, const char8* s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const Txt& s1, const char16* s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const char8* s1, const Txt& s2) {return Txt (s1).append (s2);}
inline Txt operator+ (const char16* s1, const Txt& s2) {return Txt (s1).append (s2);}

//-----------------------------------------------------------------------------
// ConstString
//-----------------------------------------------------------------------------
inline const tchar* ConstString::text () const
{
#ifdef UNICODE
	return text16 ();
#else
	return text8 ();
#endif
}

//-----------------------------------------------------------------------------
inline const char8* ConstString::text8 () const
{
	return (!isWide && buffer8) ? buffer8: kEmptyString8;
}

//-----------------------------------------------------------------------------
inline const char16* ConstString::text16 () const
{
	return (isWide && buffer16) ? buffer16 : kEmptyString16;
}

//-----------------------------------------------------------------------------
inline char8 ConstString::getChar8 (u32 index) const
{
	if (index < len && buffer8 && !isWide)
		return buffer8[index];
	return 0;
}

//-----------------------------------------------------------------------------
inline char16 ConstString::getChar16 (u32 index) const
{
	if (index < len && buffer16 && isWide)
		return buffer16[index];
	return 0;
}

//-----------------------------------------------------------------------------
inline tchar ConstString::getChar (u32 index) const
{
#ifdef UNICODE
	return getChar16 (index);
#else
	return getChar8 (index);
#endif
}

//-----------------------------------------------------------------------------
inline tchar ConstString::getCharAt (u32 index) const
{
#ifdef UNICODE
	if (isWide)
		return getChar16 (index);
#endif

	return static_cast<tchar> (getChar8 (index));
}

//-----------------------------------------------------------------------------
inline z64 ConstString::getNumber () const
{
	z64 tmp = 0;
	scanInt64 (tmp);
	return tmp;
}

//-----------------------------------------------------------------------------
inline b8 ConstString::scanInt32_8 (const char8* text, i32& value, b8 scanToEnd)
{
	z64 tmp;
	if (scanInt64_8 (text, tmp, scanToEnd))
	{
		value = (i32)tmp;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
inline b8 ConstString::scanInt32_16 (const char16* text, i32& value, b8 scanToEnd)
{
	z64 tmp;
	if (scanInt64_16 (text, tmp, scanToEnd))
	{
		value = (i32)tmp;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
inline b8 ConstString::scanInt32 (const tchar* text, i32& value, b8 scanToEnd)
{
	z64 tmp;
	if (scanInt64 (text, tmp, scanToEnd))
	{
		value = (i32)tmp;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
inline b8 ConstString::scanUInt32_8 (const char8* text, u32& value, b8 scanToEnd)
{
	zu64 tmp;
	if (scanUInt64_8 (text, tmp, scanToEnd))
	{
		value = (u32)tmp;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
inline b8 ConstString::scanUInt32_16 (const char16* text, u32& value, b8 scanToEnd)
{
	zu64 tmp;
	if (scanUInt64_16 (text, tmp, scanToEnd))
	{
		value = (u32)tmp;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
inline b8 ConstString::scanUInt32 (const tchar* text, u32& value, b8 scanToEnd)
{
	zu64 tmp;
	if (scanUInt64 (text, tmp, scanToEnd))
	{
		value = (u32)tmp;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
inline const char8* Txt::text8 () const
{
	if (isWide && !isEmpty ())
		checkToMultiByte (); // this should be avoided, since it can lead to information loss

	return ConstString::text8 ();
}

//-----------------------------------------------------------------------------
inline const char16* Txt::text16 () const
{
	if (!isWide && !isEmpty ())
	{
		const_cast<Txt&> (*this).toWideString ();
	}
	return ConstString::text16 ();
}

//-----------------------------------------------------------------------------
inline char8 Txt::getChar8 (u32 index) const
{
	if (isWide && !isEmpty ())
		checkToMultiByte (); // this should be avoided, since it can lead to information loss

	return ConstString::getChar8 (index);
}

//-----------------------------------------------------------------------------
inline char16 Txt::getChar16 (u32 index) const
{
	if (!isWide && !isEmpty ())
	{
		const_cast<Txt&> (*this).toWideString ();
	}
	return ConstString::getChar16 (index);
}

//-----------------------------------------------------------------------------


inline b8 operator<  (const ConstString& s1, const ConstString& s2) {return (s1.compare (s2) < 0) ? true : false;}
inline b8 operator<= (const ConstString& s1, const ConstString& s2) {return (s1.compare (s2) <= 0) ? true : false;}
inline b8 operator>  (const ConstString& s1, const ConstString& s2) {return (s1.compare (s2) > 0) ? true : false;}
inline b8 operator>= (const ConstString& s1, const ConstString& s2) {return (s1.compare (s2) >= 0) ? true : false;}
inline b8 operator== (const ConstString& s1, const ConstString& s2) {return (s1.compare (s2) == 0) ? true : false;}
inline b8 operator!= (const ConstString& s1, const ConstString& s2) {return (s1.compare (s2) != 0) ? true : false;}

inline b8 operator<  (const ConstString& s1, const char8* s2) {return (s1.compare (s2) < 0) ? true : false;}
inline b8 operator<= (const ConstString& s1, const char8* s2) {return (s1.compare (s2) <= 0) ? true : false;}
inline b8 operator>  (const ConstString& s1, const char8* s2) {return (s1.compare (s2) > 0) ? true : false;}
inline b8 operator>= (const ConstString& s1, const char8* s2) {return (s1.compare (s2) >= 0) ? true : false;}
inline b8 operator== (const ConstString& s1, const char8* s2) {return (s1.compare (s2) == 0) ? true : false;}
inline b8 operator!= (const ConstString& s1, const char8* s2) {return (s1.compare (s2) != 0) ? true : false;}
inline b8 operator<  (const char8* s1, const ConstString& s2) {return (s2.compare (s1) > 0) ? true : false;}
inline b8 operator<= (const char8* s1, const ConstString& s2) {return (s2.compare (s1) >= 0) ? true : false;}
inline b8 operator>  (const char8* s1, const ConstString& s2) {return (s2.compare (s1) < 0) ? true : false;}
inline b8 operator>= (const char8* s1, const ConstString& s2) {return (s2.compare (s1) <= 0) ? true : false;}
inline b8 operator== (const char8* s1, const ConstString& s2) {return (s2.compare (s1) == 0) ? true : false;}
inline b8 operator!= (const char8* s1, const ConstString& s2) {return (s2.compare (s1) != 0) ? true : false;}

inline b8 operator<  (const ConstString& s1, const char16* s2) {return (s1.compare (s2) < 0) ? true : false;}
inline b8 operator<= (const ConstString& s1, const char16* s2) {return (s1.compare (s2) <= 0) ? true : false;}
inline b8 operator>  (const ConstString& s1, const char16* s2) {return (s1.compare (s2) > 0) ? true : false;}
inline b8 operator>= (const ConstString& s1, const char16* s2) {return (s1.compare (s2) >= 0) ? true : false;}
inline b8 operator== (const ConstString& s1, const char16* s2) {return (s1.compare (s2) == 0) ? true : false;}
inline b8 operator!= (const ConstString& s1, const char16* s2) {return (s1.compare (s2) != 0) ? true : false;}
inline b8 operator<  (const char16* s1, const ConstString& s2) {return (s2.compare (s1) > 0) ? true : false;}
inline b8 operator<= (const char16* s1, const ConstString& s2) {return (s2.compare (s1) >= 0) ? true : false;}
inline b8 operator>  (const char16* s1, const ConstString& s2) {return (s2.compare (s1) < 0) ? true : false;}
inline b8 operator>= (const char16* s1, const ConstString& s2) {return (s2.compare (s1) <= 0) ? true : false;}
inline b8 operator== (const char16* s1, const ConstString& s2) {return (s2.compare (s1) == 0) ? true : false;}
inline b8 operator!= (const char16* s1, const ConstString& s2) {return (s2.compare (s1) != 0) ? true : false;}

// The following functions will only work with European Numbers!
// (e.g. Arabic, Tibetan, and Khmer digits are not supported)
extern i32 strnatcmp8 (const char8* s1, const char8* s2, b8 caseSensitive = true);
extern i32 strnatcmp16 (const char16* s1, const char16* s2, b8 caseSensitive = true);
inline i32 strnatcmp (const tchar* s1, const tchar* s2, b8 caseSensitive = true)
{
#ifdef UNICODE
	return strnatcmp16 (s1, s2, caseSensitive);
#else
	return strnatcmp8 (s1, s2, caseSensitive);
#endif
}

//-----------------------------------------------------------------------------
/** StringObject implements IStringResult and IString methods.
    It can therefore be exchanged with other Steinberg objects using one or both of these
interfaces.

\see Txt, ConstString
*/
//-----------------------------------------------------------------------------
class StringObject : public FObject, public Txt, public IStringResult, public IString
{
public:
//-----------------------------------------------------------------------------
	StringObject () {}										
	StringObject (const char16* str, i32 n = -1, b8 isTerminated = true) : Txt (str, n, isTerminated) {}
	StringObject (const char8* str, i32 n = -1, b8 isTerminated = true) : Txt (str, n, isTerminated) {}
	StringObject (const StringObject& str, i32 n = -1) : Txt (str, n) {}		
	StringObject (const Txt& str, i32 n = -1) : Txt (str, n) {}		
	StringObject (const FVariant& var) : Txt (var) {}		

	using Txt::operator=;

	// IStringResult ----------------------------------------------------------
	z0 PLUGIN_API setText (const char8* text) SMTG_OVERRIDE;
	//-------------------------------------------------------------------------

	// IString-----------------------------------------------------------------
	z0 PLUGIN_API setText8 (const char8* text) SMTG_OVERRIDE;
	z0 PLUGIN_API setText16 (const char16* text) SMTG_OVERRIDE;

	const char8* PLUGIN_API getText8 () SMTG_OVERRIDE;
	const char16* PLUGIN_API getText16 () SMTG_OVERRIDE;

	z0 PLUGIN_API take (uk s, b8 _isWide) SMTG_OVERRIDE;
	b8 PLUGIN_API isWideString () const SMTG_OVERRIDE;
	//-------------------------------------------------------------------------

	OBJ_METHODS (StringObject, FObject)
	FUNKNOWN_METHODS2 (IStringResult, IString, FObject)
};

//------------------------------------------------------------------------
} // namespace Steinberg
