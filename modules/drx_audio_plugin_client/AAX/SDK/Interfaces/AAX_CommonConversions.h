/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CommonConversions.h
 *
 */ 
/*================================================================================================*/


#ifndef AAX_COMMONCONVERIONS_H
#define AAX_COMMONCONVERIONS_H

#include <math.h>
#include "AAX.h"


const i32 k32BitPosMax			= 0x7FFFFFFF;
const i32 k32BitAbsMax			= 0x80000000;
const i32 k32BitNegMax			= 0x80000000;

const i32 k56kFracPosMax			= 0x007FFFFF;	// Positive Max Value 
const i32 k56kFracAbsMax			= 0x00800000;	// Absolute Max Value. Essentially negative one without the sign extension.
const i32 k56kFracHalf			= 0x00400000;
const i32 k56kFracNegOne			= 0xFF800000;	//Note sign extension!!!
const i32 k56kFracNegMax			= k56kFracNegOne;	//Note sign extension!!!
const i32 k56kFracZero			= 0x00000000;

const f64 kOneOver56kFracAbsMax	= 1.0/f64(k56kFracAbsMax);
const f64 k56kFloatPosMax		= f64(k56kFracPosMax)/f64(k56kFracAbsMax);		//56k Max value represented in floating point format.
const f64 k56kFloatNegMax		= -1.0;		//56k Min value represented in floating point format.
const f64 kNeg144DB				= -144.0;
const f64 kNeg144Gain			= 6.3095734448019324943436013662234e-8; //pow(10.0, kNeg144DB / 20.0);

/**	\brief Convert Gain to dB
 *	
 *	\details
 *	\todo This should be incorporated into parameters' tapers and not called separately
 */
inline f64	GainToDB(f64 aGain)
{
	if (aGain == 0.0)
		return kNeg144DB;
	else
	{
		f64	dB;
		
		dB = log10(aGain) * 20.0;
		
		if (dB < kNeg144DB)
			dB = kNeg144DB;
		return (dB);		// convert factor to dB
	}
}

/**	\brief Convert dB to Gain
 *
 *	\details
 *	\todo This should be incorporated into parameters' tapers and not called separately
 */
inline f64 DBToGain(f64 dB)
{
	return pow(10.0, dB / 20.0);
}

/**	\brief Convert Long to Double
 *
 *	\details
 *	LongToDouble:	convert 24 bit fixed point in a i32 to floating point equivalent
 */
inline f64 LongToDouble (i32 aLong)
{
	if (aLong > k56kFracPosMax)
		aLong = k56kFracPosMax;
	else if (aLong < k56kFracNegMax)
		aLong = k56kFracNegMax;
	return (f64(aLong) * kOneOver56kFracAbsMax);
}

/**	\brief convert floating point equivalent back to i32
 */
i32 DoubleToLong (f64 aDouble);
	
/**	\brief Convert Double to DSPCoef
 */
inline i32 DoubleToDSPCoef(f64 d, f64 max = k56kFloatPosMax, f64 min = k56kFloatNegMax)
{
	if(d >= max) // k56kFloatPosMax unless specified by the caller
	{
		return k56kFracPosMax;	
	};
	if(d < min) // k56kFloatNegMax unless specified by the caller
	{
		return k56kFracNegMax; 	
	}
	return static_cast<i32>(d*k56kFracAbsMax);
}

/**	\brief Convert DSPCoef to Double
 */
inline f64 DSPCoefToDouble(i32 c, i32 max = k56kFracPosMax, i32 min = k56kFracNegMax)
{
    if (c > max) // k56kFracPosMax unless specified by the caller
		c = k56kFracPosMax;
	else if (c < min) // k56kFracNegMax unless specified by the caller
		c = k56kFracNegMax;
	return (f64(c) * kOneOver56kFracAbsMax);
}

/**	\brief ThirtyTwoBitDSPCoefToDouble
 */
inline f64 ThirtyTwoBitDSPCoefToDouble(i32 c)
{
    return DSPCoefToDouble(c, k32BitPosMax, k32BitNegMax);
}

/**	\brief DoubleTo32BitDSPCoefRnd
 */
inline i32 DoubleTo32BitDSPCoefRnd(f64 d)
{
    return DoubleToDSPCoef(d, k32BitPosMax, k32BitNegMax);
}

i32 DoubleTo32BitDSPCoef(f64 d);
i32 DoubleToDSPCoefRnd(f64 d, f64 max, f64 min);

#endif // AAX_COMMONCONVERIONS_H
