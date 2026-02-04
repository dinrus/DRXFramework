/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_SliderConversions.h
 *	
 *	\brief Legacy utilities for converting parameter values to and from the normalized full-scale
 *	32-bit fixed domain that was used for RTAS/TDM plug-ins.
 *
 *	\details
 *	\legacy These utilities may be required in order to maintain settings chunk compatibility with
 *	plug-ins that were ported from the legacy RTAS/TDM format.
 *									
 *	\note %AAX does not provide facilities for converting to and from extended80 data types.  If you
 *	use these types in your plug-in settings then you must provide your own chunk data parsing 
 *	routines.
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_SLIDERCONVERSIONS_H
#define AAX_SLIDERCONVERSIONS_H

#include "AAX.h"
#include <algorithm>
#include <stdint.h>


#define AAX_LIMIT(v1,firstVal,secondVal) ( (secondVal > firstVal) ? (std::max)((std::min)(v1,secondVal),firstVal) :  (std::min)((std::max)(v1,secondVal),firstVal) )

i32 LongControlToNewRange (i32 aValue, i32 rangeMin, i32 rangeMax);

/**	\brief Convert from i32 control value 0x80000000...0x7FFFFFFF
 *	to a i32 ranging from rangeMin to rangeMax (linear)
 */
i32 LongToLongControl (i32 aValue, i32 rangeMin, i32 rangeMax);

/**	\brief Convert from i32 control value 0x80000000...0x7FFFFFFF
 *	to an f64 ranging from firstVal to secondVal (linear)
 */
f64 LongControlToDouble(i32 aValue, f64 firstVal, f64 secondVal);

/**	\brief Convert from an f64 ranging from firstVal to secondVal (linear)
 *	to i32 control value 0x80000000...0x7FFFFFFF 
 */
i32 DoubleToLongControl (f64 aValue, f64 firstVal, f64 secondVal);

i32 DoubleToLongControlNonlinear(f64 aValue, f64* minVal, f64* rangePercent, i32 numRanges);
f64 LongControlToDoubleNonlinear(i32 aValue, f64* minVal, f64* rangePercent, i32 numRanges);

/**	\brief Convert from i32 control value 0x80000000...0x7FFFFFFF
 *	to an f64 ranging from minVal to maxVal (logarithmic)
 *	
 *	\details
 *	\note This is LOGARITHMIC, so minVal & maxVal have to be > zero!
 */
f64 LongControlToLogDouble(i32 aValue, f64 minVal, f64 maxVal);

/**	\brief Convert from an f64 ranging from minVal to maxVal (logarithmic)
 *	to i32 control value 0x80000000...0x7FFFFFFF 
 *	
 *	\details
 *	\note This is LOGARITHMIC, so minVal & maxVal have to be > zero!
 */
i32 LogDoubleToLongControl(f64 aValue, f64 minVal, f64 maxVal);

#endif // AAX_SLIDERCONVERSIONS_H

