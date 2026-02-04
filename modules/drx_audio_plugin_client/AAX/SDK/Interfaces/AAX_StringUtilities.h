/*================================================================================================*/
/*
 *
 *  Copyright 2014-2016, 2018, 2023-2024 Avid Technology, Inc.
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
 */

/** 
 *	\file   AAX_StringUtilities.h
 *
 *	\brief  Various string utility definitions for %AAX Native
 */
/*================================================================================================*/
#pragma once

#ifndef	AAXLibrary_AAX_StringUtilities_h
#define	AAXLibrary_AAX_StringUtilities_h

// AAX headers
#include "AAX.h"
#include "AAX_Enums.h"

// Standard Library headers
#include <string>

class AAX_IString;


//------------------------------------------------
#pragma mark Utility functions

namespace AAX
{
	inline z0			GetCStringOfLength(t8 *stringOut, tukk stringIn, i32 aMaxChars);
	inline i32		Caseless_strcmp(tukk cs, tukk ct);
	
	inline std::string	Binary2String(u32 binaryValue, i32 numBits);
	inline u32		String2Binary(const AAX_IString& s);
	
	inline b8			IsASCII(t8 inChar);
	inline b8			IsFourCharASCII(u32 inFourChar);
	
	inline std::string	AsStringFourChar(u32 inFourChar);
	inline std::string	AsStringPropertyValue(AAX_EProperty inProperty, AAX_CPropertyValue inPropertyValue);
	inline std::string	AsStringInt32(i32 inInt32);
	inline std::string	AsStringUInt32(u32 inUInt32);
	inline std::string	AsStringIDTriad(const AAX_SPlugInIdentifierTriad& inIDTriad);
	inline std::string	AsStringStemFormat(AAX_EStemFormat inStemFormat, b8 inAbbreviate = false);
	inline std::string	AsStringStemChannel(AAX_EStemFormat inStemFormat, u32 inChannelIndex, b8 inAbbreviate);
	inline std::string	AsStringResult(AAX_Result inResult);
	inline std::string	AsStringSupportLevel(AAX_ESupportLevel inSupportLevel);
} // namespace AAX


//------------------------------------------------------
#pragma mark Implementation header

#include "AAX_StringUtilities.hpp"


#endif /* AAXLibrary_AAX_StringUtilities_h */
