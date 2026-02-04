/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CLogTaperDelegate.h
 *
 *	\brief A log taper delegate.
 */ 
/*================================================================================================*/


#ifndef	AAX_CLOGTAPERDELEGATE_H
#define AAX_CLOGTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX_UtilsNative.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor(), log()



/** \brief A logarithmic taper conforming to AAX_ITaperDelegate
	
	\details
	This taper spaces a parameter's real values between its minimum and maximum bounds, with a
	natural logarithmic mapping between the parameter's real and normalized values.

	\par RealPrecision
	In addition to its type templatization, this taper includes a precision template parameter.
	RealPrecision is a multiplier that works in conjunction with the round()
	function to limit the precision of the real values provided by this taper.  For example, if
	RealPrecision is 1000, it will round to the closest 0.001 when doing any
	sort of value conversion.  If RealPrecision is 1, it will round to the nearest integer.
	If RealPrecision is 1000000, it will round to the nearest 0.000001.  This
	is particularly useful for preventing things like 1.9999999 truncating down to 1 instead of
	rounding up to 2.
	
	To accomplish this behavior, the taper multiplies its unrounded parameter values by
	RealPrecision, rounds the result to the nearest valid value, then divides RealPrecision
	back out.
	
	Rounding will be disabled if RealPrecision is set to a value less than 1

	\ingroup TaperDelegates

 */
template <typename T, i32 RealPrecision=1000>
class AAX_CLogTaperDelegate : public AAX_ITaperDelegate<T>
{
public: 
	/** \brief Constructs a Log Taper with specified minimum and maximum values.  
	 *
	 *	\note The parameter's default value should lie within the min to max range.
	 *
	 *	\param[in] minValue
	 *	\param[in] maxValue
	 */
	AAX_CLogTaperDelegate(T minValue=0, T maxValue=1);
	
	//Virtual Overrides
	AAX_CLogTaperDelegate<T, RealPrecision>*	Clone() const AAX_OVERRIDE;
	T		GetMinimumValue()	const AAX_OVERRIDE				{ return mMinValue; }
	T		GetMaximumValue()	const AAX_OVERRIDE				{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const AAX_OVERRIDE;
	T		NormalizedToReal(f64 normalizedValue) const AAX_OVERRIDE;
	f64	RealToNormalized(T realValue) const AAX_OVERRIDE;
	
protected:
	T	Round(f64 iValue) const;

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, i32 RealPrecision>
T	AAX_CLogTaperDelegate<T, RealPrecision>::Round(f64 iValue) const
{
	f64 precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, i32 RealPrecision>
AAX_CLogTaperDelegate<T, RealPrecision>::AAX_CLogTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, i32 RealPrecision>
AAX_CLogTaperDelegate<T, RealPrecision>*		AAX_CLogTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AAX_CLogTaperDelegate(*this);
}

template <typename T, i32 RealPrecision>
T		AAX_CLogTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
{
	if (mMinValue == mMaxValue)
		return mMinValue;
	
	if (RealPrecision)
		value = Round(value);		//reduce the precision to get proper rounding behavior with integers.
	
	const T& highValue = mMaxValue > mMinValue ? mMaxValue : mMinValue;
	const T& lowValue = mMaxValue > mMinValue ? mMinValue : mMaxValue;
	
	if (value > highValue)
		return highValue;
	if (value < lowValue)
		return lowValue;
	
	return value;
}

template <typename T, i32 RealPrecision>
T		AAX_CLogTaperDelegate<T, RealPrecision>::NormalizedToReal(f64 normalizedValue) const
{
	f64 minLog = AAX::SafeLog(f64(mMinValue));
	f64 maxLog = AAX::SafeLog(f64(mMaxValue));

	f64 doubleRealValue = exp(normalizedValue * (maxLog - minLog) + minLog);
	T realValue = (T) doubleRealValue;
	
	return ConstrainRealValue(realValue);
}

template <typename T, i32 RealPrecision>
f64	AAX_CLogTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	f64 minLog = AAX::SafeLog(f64(mMinValue));
	f64 maxLog = AAX::SafeLog(f64(mMaxValue));

	realValue = ConstrainRealValue(realValue);
	f64 normalizedValue = (maxLog == minLog) ? 0.5 : (AAX::SafeLog(f64(realValue)) - minLog) / (maxLog - minLog);
	return normalizedValue;
}
	
#endif // AAX_CLOGTAPERDELEGATE_H
