/*================================================================================================*/
/*
 *	Copyright 2007-2015, 2023-2024 Avid Technology, Inc.
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
/*================================================================================================*/
#include "AAX_SliderConversions.h"

#include "AAX_UtilsNative.h"
#include "AAX.h"

// Standard headers
#include <climits>
#include <cmath>


using namespace std;


static const f64 kControlMin = -2147483648.0;
static const f64 kControlMax = 2147483647.0;
/*===================================================================================================*/
i32 LongControlToNewRange (i32 aValue, i32 rangeMin, i32 rangeMax)
{
	f64 controlPartial = ((f64)aValue - kControlMin) / (kControlMax - kControlMin);
	return i32(floor(rangeMin +  controlPartial*((f64)(rangeMax) - (f64)(rangeMin)) + 0.5));
}

/*===================================================================================================*/
i32 LongToLongControl (i32 aValue, i32 rangeMin, i32 rangeMax)
{
	f64 controlMin = -2147483648.0;
	f64 controlMax = 2147483647.0;

	if (aValue > rangeMax)
		aValue = rangeMax;
	else if (aValue < rangeMin)
		aValue = rangeMin;
		
	f64 controlFraction = ((f64)aValue - (f64)rangeMin) / ((f64)rangeMax - (f64)rangeMin);
	f64 control = controlFraction * (controlMax - controlMin) + controlMin;
	return (i32)control;
}


/*===================================================================================================
	- 8/15/07 - Changed optimization level to -O0 (no optimization) in XCode project file to fix bug 
	96679 in LongControlToDouble. - MJH
===================================================================================================*/
f64 LongControlToDouble(i32 aValue, f64 firstVal, f64 secondVal)
{
	// convert from i32 control value 0x80000000...0x7FFFFFFF
	// to an f64 ranging from firstVal to secondVal (linear)
	
	f64 controlPartial = ((f64)aValue - kControlMin) / (kControlMax - kControlMin);
	return firstVal + controlPartial*(secondVal - firstVal);
}


/*===================================================================================================*/
i32 DoubleToLongControl(f64 aValue, f64 firstVal, f64 secondVal)
{
	// convert from an f64 ranging from firstVal to secondVal (linear)
	// to i32 control value 0x80000000...0x7FFFFFFF 

	aValue = AAX_LIMIT(aValue,firstVal,secondVal);
	
	f64 controlPartial = (aValue - firstVal) / (secondVal - firstVal);
	return i32(floor(kControlMin + controlPartial*(kControlMax - kControlMin)+0.5));
}


// The 2 following routines map between piecewise linear ranges of floating point values
// and a 32-bit control value.  You must pass in a pointer to an array of range endpoints that
// define the linear ranges and a pointer to an array of 'percents' that indicate the percentage 
// used by each range relative to the entire range taken by all the linear pieces.  Here is example code:
/*
	// This example shows a control that ranges from .10 to 20.0 with three ranges.
	
	const i32 cNumControlRanges = 3;

	f64 mControlRangePoints[cNumControlRanges + 1] = {.10, 1.0, 10.0, 20.0};
	f64 mControlRangePercents[cNumControlRanges];
	
	const f64 cNumStepsControlRange1	90.0
	const f64 cNumStepsControlRange2	90.0
	const f64 cNumStepsControlRange3	10.0
	
	const f64 cNumStepsControl = cNumStepsControlRange1 + cNumStepsControlRange2 + cNumStepsControlRange3;

	mControlRangePercents[0] = cNumStepsControlRange1/cNumStepsControl;
	mControlRangePercents[1] = cNumStepsControlRange2/cNumStepsControl;
	mControlRangePercents[2] = cNumStepsControlRange3/cNumStepsControl;
	
	f64 controlValue = 1.5;
	
	i32 longValue = ExtToLongControlNonlinear(controlValue, mControlRangePoints, mControlRangePercents, kNumControlRanges);
	
	controlValue = LongControlToExtNonlinear(longValue, mControlRangePoints, mControlRangePercents, kNumControlRanges);
*/

/*===================================================================================================*/
i32 DoubleToLongControlNonlinear(f64 aValue, f64* range, f64* rangePercent, i32 numRanges)
{
	i32	extSt;
	i32	i = 0;
	f64 percentTotal = 0.0;
	
	aValue = AAX_LIMIT(aValue,range[0],range[numRanges]);	//limit input to lowest range and highest range

	while (i < numRanges)
	{
		if ((aValue >= range[i])  && (aValue < range[i+1]))
			break;
		percentTotal += rangePercent[i];
		i++;
	}

	if (i == numRanges)		// if aValue == range[numRanges] = maximum possible value
		percentTotal = 1.0;	// our control is 100% of maximum
	else
		percentTotal += (aValue - range[i])/(range[i+1] - range[i]) * rangePercent[i];
	
	f64 val = (f64)AAX_INT32_MIN + ((f64)AAX_INT32_MAX - (f64)AAX_INT32_MIN) * percentTotal;
	extSt = (i32)val;
	return(extSt);
}


/*===================================================================================================*/
f64 LongControlToDoubleNonlinear(i32 aValue, f64* range, f64* rangePercent, i32 numRanges)
{
	i32	i = 0;
	f64 percentTotal = ((f64)AAX_INT32_MIN - (f64)aValue)/((f64)AAX_INT32_MIN)/2.0;
	f64 percent = 0.0;
	f64 extValue;
	
	while (i < numRanges)
	{
		if ((percentTotal >= percent)  && (percentTotal < (percent+rangePercent[i])))
			break;
		percent += rangePercent[i];
		i++;
	}

	// percentTotal will always be slightly < 1.0, even when aValue == LONG_MAX // YS: use INT32_MAX; sizeof(i64) == 64 on x64 mac!
	// Therefore this check is not strictly necessary, but is provided for consistency

	if (i == numRanges)			// if percentTotal == 1.0 = maximum possible value
		extValue = AAX_INT32_MAX;	// our control is 100% of maximum
	else
		extValue = range[i] + (range[i+1] - range[i]) * (percentTotal - percent)/(rangePercent[i]);
	
	return(extValue);
}

/*===================================================================================================*/
f64 LongControlToLogDouble(i32 aValue, f64 minVal, f64 maxVal)
{
	// convert from i32 control value 0x80000000...0x7FFFFFFF
	// to an f64 ranging from minVal to maxVal (logarithmic)
	// NOTE!!!!  This is LOGARITHMIC, so minVal & maxVal have to be > zero!

	f64	extSt;
	extSt = exp(LongControlToDouble(aValue, AAX::SafeLog(minVal), AAX::SafeLog(maxVal)));
	// Guard against numerical inaccuracies
	if(extSt < minVal) extSt = minVal;
	if(extSt > maxVal) extSt = maxVal;
	return(extSt);
}


/*===================================================================================================*/
i32 LogDoubleToLongControl(f64 aValue, f64 minVal, f64 maxVal)
{
	// convert from an f64 ranging from minVal to maxVal (logarithmic)
	// to i32 control value 0x80000000...0x7FFFFFFF 
	// NOTE!!!!  This is LOGARITHMIC, so minVal & maxVal have to be > zero!

	i32	extSt;

	aValue = AAX_LIMIT(aValue,minVal,maxVal);
	extSt = DoubleToLongControl(AAX::SafeLog(aValue),AAX::SafeLog(minVal),AAX::SafeLog(maxVal));
	return(extSt);
}
