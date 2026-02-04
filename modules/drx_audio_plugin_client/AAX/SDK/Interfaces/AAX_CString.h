/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2017, 2021, 2023-2024 Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	This file is part of the Avid AAX SDK.
 *	
 *	The AAX SDK is subject to commercial or open-source licensing.
 *	
 *	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
 *	Agreement and Avid Privacy Policy.
 *	
 *	AAX SDK License: https://developer.avid.com/aax
 *	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
 *	
 *	Or: You may also use this code under the terms of the GPL v3 (see
 *	www.gnu.org/licenses).
 *	
 *	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 *	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 *	DISCLAIMED.
 *
 */

/**  
 *	\file  AAX_CString.h
 *
 *	\brief A generic %AAX string class with similar functionality to std::string
 *
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_CSTRING_H
#define AAX_CSTRING_H


#include "AAX_IString.h"
#include "AAX.h"

#include <string>
#include <map>


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

/**
 *	\brief A generic %AAX string class with similar functionality to <tt>std::string</tt>
 */
class AAX_CString : public AAX_IString
{
public:
	static u32k	kInvalidIndex = static_cast<u32>(-1);
	static u32k	kMaxStringLength = static_cast<u32>(-2);

	// AAX_IString Virtual Overrides
	u32		Length() const AAX_OVERRIDE;
	u32		MaxLength() const AAX_OVERRIDE;
	const t8 *	Get () const AAX_OVERRIDE;
	z0			Set ( const t8 * iString ) AAX_OVERRIDE;
	AAX_IString &	operator=(const AAX_IString & iOther) AAX_OVERRIDE;
	AAX_IString &	operator=(const t8 * iString) AAX_OVERRIDE;
	
	/** Constructs an empty string. */
	AAX_CString();
	
	/** Implicit conversion constructor: Constructs a string with a tukk pointer to copy. */
	AAX_CString(tukk str);
	
	/** Copy constructor: Constructs a string from a std::string. Beware of STL variations across various binaries. */
	explicit AAX_CString(const std::string& str);
	
	/** Copy constructor: Constructs a string with another concrete AAX_CString. */
	AAX_CString(const AAX_CString& other);

	/** Copy constructor: Constructs a string from another string that meets the AAX_IString interface. */
	AAX_CString(const AAX_IString& other);

	/** Default move constructor */
	AAX_DEFAULT_MOVE_CTOR(AAX_CString);


	/** Direct access to a std::string. */
	std::string&		StdString();
	
	/** Direct access to a const std::string. */
	const std::string&	StdString() const;

	/** Assignment operator from another AAX_CString */
	AAX_CString&	operator=(const AAX_CString& other);
	
	/** Assignment operator from a std::string. Beware of STL variations across various binaries. */
	AAX_CString &	operator=(const std::string& other);

	/** Move operator */
	AAX_CString &	operator=(AAX_CString&& other);
			
	/** output stream operator for concrete AAX_CString */
	friend std::ostream& operator<< (std::ostream& os, const AAX_CString& str);

	/** input stream operator for concrete AAX_CString */
	friend std::istream& operator>> (std::istream& os, AAX_CString& str);

	
	// Txt Formatting Functions
	z0			Clear();
	b8			Empty() const;
	AAX_CString&	Erase(u32 pos, u32 n);
	AAX_CString&	Append(const AAX_CString& str);
	AAX_CString&	Append(tukk str);
	AAX_CString&	AppendNumber(f64 number, i32 precision);
	AAX_CString&	AppendNumber(i32 number);
	AAX_CString&	AppendHex(i32 number, i32 width);
	AAX_CString&	Insert(u32	pos, const AAX_CString&	str);
	AAX_CString&	Insert(u32	pos, tukk str);
	AAX_CString&	InsertNumber(u32 pos, f64 number, i32 precision);
	AAX_CString&	InsertNumber(u32 pos, i32 number);
	AAX_CString&	InsertHex(u32 pos, i32 number, i32 width);
	AAX_CString&	Replace(u32 pos, u32 n, const AAX_CString& str);
	AAX_CString&	Replace(u32 pos, u32 n, tukk str);
	u32		FindFirst(const AAX_CString& findStr) const;
	u32		FindFirst(tukk findStr) const;
	u32		FindFirst(t8 findChar) const;
	u32		FindLast(const AAX_CString& findStr) const;
	u32		FindLast(tukk findStr) const;
	u32		FindLast(t8 findChar) const;	
	tukk		CString()	const;
	b8			ToDouble(f64* oValue)	const;
	b8			ToInteger(i32* oValue)  const;
	z0			SubString(u32 pos, u32 n, AAX_IString* outputStr) const;
	b8			Equals(const AAX_CString& other) const { return operator==(other); }
	b8			Equals(tukk other) const { return operator==(other); }
	b8			Equals(const std::string& other) const { return operator==(other); } //beware of STL variations between binaries.
	
	// Operator Overrides
	b8			operator==(const AAX_CString& other) const;
	b8			operator==(tukk otherStr) const;
	b8			operator==(const std::string& otherStr) const;      //beware of STL variations between binaries.
	b8			operator!=(const AAX_CString& other) const;
	b8			operator!=(tukk otherStr) const;
	b8			operator!=(const std::string& otherStr) const;      //beware of STL variations between binaries.
	b8			operator<(const AAX_CString& other) const;
	b8			operator>(const AAX_CString& other) const;
	const t8&		operator[](u32 index) const;
	t8&			operator[](u32 index);
	AAX_CString&	operator+=(const AAX_CString& str);
	AAX_CString&	operator+=(const std::string& str);
	AAX_CString&	operator+=(tukk str);

protected:
	std::string		mString;
};

// Non-member operators
inline AAX_CString operator+(AAX_CString lhs, const AAX_CString& rhs)
{
	lhs += rhs;
	return lhs;
}
inline AAX_CString operator+(AAX_CString lhs, tukk rhs)
{
	lhs += rhs;
	return lhs;
}
inline AAX_CString operator+(tukk lhs, const AAX_CString& rhs)
{
	return AAX_CString(lhs) + rhs;
}


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

/**	\brief Helper class to store a collection of name abbreviations
 */
class AAX_CStringAbbreviations
{
public:
	explicit AAX_CStringAbbreviations(const AAX_CString& inPrimary)
	: mPrimary(inPrimary)
	, mAbbreviations()
	{
	}
	
	z0 SetPrimary(const AAX_CString& inPrimary) { mPrimary = inPrimary; }
	const AAX_CString& Primary() const { return mPrimary; }
	
	z0 Add(const AAX_CString& inAbbreviation)
	{
		u32 stringLength = inAbbreviation.Length();
		mAbbreviations[stringLength] = inAbbreviation;	//Does a string copy into the map.
	}
	
	const AAX_CString& Get(i32 inNumCharacters) const
	{
		//More characters than the primary string or no specific shortened names.
		if ((inNumCharacters >= i32(mPrimary.Length())) || (mAbbreviations.empty()) || (0 > inNumCharacters))
			return mPrimary;
		
		std::map<u32, AAX_CString>::const_iterator iter = mAbbreviations.upper_bound(static_cast<u32>(inNumCharacters));
		
		//If the iterator is already pointing to shortest string, return that.
		if (iter == mAbbreviations.begin())
			return iter->second;
		
		//lower_bound() will return the iterator that is larger than the desired value, so decrement the iterator.
		--iter;
		return iter->second;
	}
	
	z0 Clear() { mAbbreviations.clear(); }
	
private:
	AAX_CString									mPrimary;
	std::map<u32, AAX_CString>				mAbbreviations;
};

#endif //AAX_CSTRING_H
