/*================================================================================================*/
/*
 *	Copyright 2010-2015, 2018, 2023-2024 Avid Technology, Inc.
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
 *  \author David Tremblay
 *
 */
/*================================================================================================*/
#include "AAX_CParameter.h"


//----------------------------------------------------------------
// AAX_CParameterValue


template <>
b8		AAX_CParameterValue<b8>::GetValueAsBool(b8* value) const
{
	*value = mValue;
	return true;
}

template<>
b8		AAX_CParameterValue<i32>::GetValueAsInt32(i32* value) const
{
	*value = mValue;
	return true;
}

template<>
b8		AAX_CParameterValue<f32>::GetValueAsFloat(f32* value) const
{
	*value = mValue;
	return true;
}

template<>
b8		AAX_CParameterValue<f64>::GetValueAsDouble(f64* value) const
{
	*value = mValue;
	return true;
}

template<>
b8		AAX_CParameterValue<AAX_CString>::GetValueAsString(AAX_IString* value) const
{
	*value = mValue;
	return true;
}



//----------------------------------------------------------------
// AAX_CParameter

template <>
b8	AAX_CParameter<b8>::GetNormalizedValueFromBool(b8 value, f64 *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
b8	AAX_CParameter<i32>::GetNormalizedValueFromInt32(i32 value, f64 *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
b8	AAX_CParameter<f32>::GetNormalizedValueFromFloat(f32 value, f64 *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
b8	AAX_CParameter<f64>::GetNormalizedValueFromDouble(f64 value, f64 *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
b8		AAX_CParameter<b8>::GetBoolFromNormalizedValue(f64 inNormalizedValue, b8* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
b8		AAX_CParameter<i32>::GetInt32FromNormalizedValue(f64 inNormalizedValue, i32* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
b8		AAX_CParameter<f32>::GetFloatFromNormalizedValue(f64 inNormalizedValue, f32* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
b8		AAX_CParameter<f64>::GetDoubleFromNormalizedValue(f64 inNormalizedValue, f64* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
b8		AAX_CParameter<AAX_CString>::GetValueAsString(AAX_IString *value) const
{
	return mValue.GetValueAsString(value);
}

template<>
b8		AAX_CParameter<b8>::SetValueWithBool(b8 value)
{
	SetValue(value);
	return true;
}

template<>
b8		AAX_CParameter<i32>::SetValueWithInt32(i32 value)
{
	SetValue(value);
	return true;
}

template<>
b8		AAX_CParameter<f32>::SetValueWithFloat(f32 value)
{
	SetValue(value);
	return true;
}

template<>
b8		AAX_CParameter<f64>::SetValueWithDouble(f64 value)
{
	SetValue(value);
	return true;
}

template<>
b8		AAX_CParameter<AAX_CString>::SetValueWithString(const AAX_IString& value)
{
	SetValue(value);
	return true;
}
