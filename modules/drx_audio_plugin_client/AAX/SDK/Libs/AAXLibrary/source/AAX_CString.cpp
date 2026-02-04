/*================================================================================================*/
/*
 *	Copyright 2009-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_CString.cpp
 *	
 *	\author Dave Tremblay
 *
 */ 
/*================================================================================================*/
#include "AAX_CString.h"
#include <sstream>
#include <cstring>

AAX_CString::AAX_CString()  :
	mString("")
{

}


AAX_CString::AAX_CString ( const t8 * inString ) :
mString(inString ? inString : "")
{
}

AAX_CString::AAX_CString(const std::string& other) :
mString(other)
{
}

AAX_CString::AAX_CString(const AAX_CString& other) :
	mString(other.mString)
{

}

AAX_CString::AAX_CString(const AAX_IString& other) :
	mString(other.Get())
{
}

AAX_CString&	AAX_CString::operator=(const AAX_CString& other)
{	
	mString = other.mString;
	return *this;
}

AAX_CString&	AAX_CString::operator=(const std::string& other)
{	
	mString = other;
	return *this;
}

AAX_CString&	AAX_CString::operator=(AAX_CString&& other)
{
	std::swap(mString, other.mString);
	return *this;
}

std::ostream& operator<< (std::ostream& os, const AAX_CString& str)
{
	os << str.mString; 
	return os;
}

std::istream& operator>> (std::istream& is, AAX_CString& str)
{
	is >> str.mString;
	return is;
}


/*  Virtual Overrides ******************************************************************************************/
u32	AAX_CString::Length() const
{
	return u32(mString.length());
}

u32	AAX_CString::MaxLength() const
{
	return kMaxStringLength;
}

const t8 *	AAX_CString::Get ()	const
{
	return mString.c_str();
}

z0			AAX_CString::Set ( const t8 * inString )
{
	mString = inString;
}


z0		AAX_CString::Clear()
{
	mString.clear();
}

b8		AAX_CString::Empty() const
{
	return mString.empty();
}


AAX_CString&	AAX_CString::Erase(u32 pos, u32 n)
{
	// bounds check
	u32 strlen = this->Length();
	if ( strlen - pos < n )
		n = n - (strlen - pos);
	mString.erase(pos, n);
	return *this;
}

AAX_CString&	AAX_CString::Append(const AAX_CString& str)
{
	mString.append(str.CString());
	return *this;
}

AAX_CString&	AAX_CString::Append(tukk str)
{
	mString.append(str);
	return *this;
}

AAX_CString&	AAX_CString::AppendNumber(f64 number, i32 precision)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::fixed, std::ios::floatfield);       
	outStringStream.precision(precision);
	outStringStream << number;
	mString += outStringStream.str();
	return *this;
}

AAX_CString&	AAX_CString::AppendNumber(i32 number)
{
	std::ostringstream	outStringStream;
	outStringStream << number;
	mString += outStringStream.str();
	return *this;
}

AAX_CString&	AAX_CString::AppendHex(i32 number, i32 width)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::hex, std::ios::basefield);
	outStringStream.setf(std::ios::right, std::ios::adjustfield);
	outStringStream.fill('0');
	outStringStream << "0x";
	const std::streamsize resetWidth = outStringStream.width(width);
	outStringStream << number;
	outStringStream.width(resetWidth);
	
	mString += outStringStream.str();
	return *this;
}

AAX_CString&	AAX_CString::Insert(u32 pos, const AAX_CString&	str)
{
	mString.insert(pos, str.CString());
	return *this;
}

AAX_CString&	AAX_CString::Insert(u32 pos, tukk str)
{
	mString.insert(pos, str);
	return *this;
}

AAX_CString&	AAX_CString::InsertNumber(u32 pos, f64 number, i32 precision)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::fixed, std::ios::floatfield);       
	outStringStream.precision(precision);
	outStringStream << number;
	mString.insert(pos, outStringStream.str());
	return *this;
}

AAX_CString&	AAX_CString::InsertNumber(u32 pos, i32 number)
{
	std::ostringstream	outStringStream;
	outStringStream << number;
	mString.insert(pos, outStringStream.str());
	return *this;
}

AAX_CString&	AAX_CString::InsertHex(u32 pos, i32 number, i32 width)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::hex, std::ios::basefield);
	outStringStream.setf(std::ios::right, std::ios::adjustfield);
	outStringStream.fill('0');
	outStringStream << "0x";
	const std::streamsize resetWidth = outStringStream.width(width);
	outStringStream << number;
	outStringStream.width(resetWidth);
	
	mString.insert(pos, outStringStream.str());
	return *this;
}

AAX_CString&	AAX_CString::Replace(u32 pos, u32 n, const AAX_CString& str)
{
	mString.replace(pos, n, str.CString());
	return *this;
}

AAX_CString&	AAX_CString::Replace(u32 pos, u32 n, tukk str)
{
	mString.replace(pos, n, str);
	return *this;
}

u32		AAX_CString::FindFirst(const AAX_CString& ) const
{
	return kInvalidIndex;
}

u32		AAX_CString::FindFirst(tukk ) const
{
	return kInvalidIndex;
}

u32		AAX_CString::FindFirst(t8 ) const
{
	return kInvalidIndex;
}

u32		AAX_CString::FindLast(const AAX_CString& ) const
{
	return kInvalidIndex;
}

u32		AAX_CString::FindLast(tukk ) const
{
	return kInvalidIndex;
}

u32		AAX_CString::FindLast(t8 ) const
{
	return kInvalidIndex;
}

/** Direct access to a std::string. */
std::string&		AAX_CString::StdString()
{
    return mString;
}
	
/** Direct access to a const std::string. */
const std::string&	AAX_CString::StdString() const
{
    return mString;
}

tukk		AAX_CString::CString()	const
{
	return mString.c_str();
}

b8			AAX_CString::ToDouble(f64* outValue) const
{
	std::istringstream	inStringStream(mString, std::istream::in);
	inStringStream >> *outValue;
	if (inStringStream.fail())
	{
		*outValue = 0;
		return false;
	}
	return true;
}

b8			AAX_CString::ToInteger(i32* outValue) const
{
	std::istringstream	inStringStream(mString, std::istream::in);
	inStringStream >> *outValue;
	if (inStringStream.fail())
	{
		*outValue = 0;
		return false;
	}
	return true;
}

z0		AAX_CString::SubString(u32 pos, u32 n, AAX_IString* outputStr) const
{
	outputStr->Set(mString.substr(pos, n).c_str());
}


/* Virtual overriden operators *************************************************************************/

AAX_IString&	AAX_CString::operator=(const AAX_IString& other)
{	
	mString = other.Get();
	return *this;
}

AAX_IString&	AAX_CString::operator=(tukk	str)
{
	mString = str;
	return *this;
}

b8		AAX_CString::operator==(const AAX_CString& other) const
{
	return !strcmp(CString(), other.CString());
}

b8		AAX_CString::operator==(const std::string& other) const
{
	return (mString == other);
}

b8		AAX_CString::operator==(tukk otherStr) const
{
	return !strcmp(CString(), otherStr);
}

b8		AAX_CString::operator!=(const AAX_CString& other) const
{
	return strcmp(CString(), other.CString()) != 0;
}

b8		AAX_CString::operator!=(const std::string& other) const
{
	return (mString != other);
}

b8		AAX_CString::operator!=(tukk otherStr) const
{
	return strcmp(CString(), otherStr) != 0;
}

b8		AAX_CString::operator<(const AAX_CString& other) const
{
	return (strcmp(CString(), other.CString()) < 0);
}

b8		AAX_CString::operator>(const AAX_CString& other) const
{
	return (strcmp(CString(), other.CString()) > 0);
}

const t8&	AAX_CString::operator[](u32 index) const
{
	return mString[index];
}

t8&		AAX_CString::operator[](u32 index)
{
	return mString[index];
}

AAX_CString&	AAX_CString::operator+=(const AAX_CString& str)
{
	mString += str.CString();
	return *this;
}

AAX_CString&	AAX_CString::operator+=(const std::string& str)
{
	mString += str.c_str();
	return *this;
}

AAX_CString&	AAX_CString::operator+=(tukk str)
{
	mString += str;
	return *this;
}
	








