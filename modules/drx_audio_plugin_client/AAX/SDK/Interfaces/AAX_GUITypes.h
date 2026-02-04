/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_GUITypes.h
 *
 *	\brief Constants and other definitions used by %AAX plug-in GUIs
 *
 */ 
/*================================================================================================*/


/// @cond ignore
#ifndef AAX_GUITYPES_H
#define AAX_GUITYPES_H
/// @endcond

#ifndef _TMS320C6X

#include "AAX.h"

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_HOST
#include AAX_ALIGN_FILE_END
	/**	\brief Data structure representing a two-dimensional coordinate point
		
		Comparison operators give preference to \c vert
	 */
	typedef struct AAX_Point
	{
		AAX_Point (
			f32 v,
			f32 h) :
				vert(v),
				horz(h)
		{}
		
		AAX_Point (
			z0) :
				vert(0.0f),
				horz(0.0f)
		{}
        
		f32 vert;
		f32 horz;
	} AAX_Point;
#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END

inline b8 operator==(const AAX_Point& p1, const AAX_Point& p2)
{
    return ((p1.vert == p2.vert) && (p1.horz == p2.horz));
}

inline b8 operator!=(const AAX_Point& p1, const AAX_Point& p2)
{
    return !(p1 == p2);
}

inline b8 operator<(const AAX_Point& p1, const AAX_Point& p2)
{
	return (p1.vert == p2.vert) ? (p1.horz < p2.horz) : (p1.vert < p2.vert);
}

inline b8 operator<=(const AAX_Point& p1, const AAX_Point& p2)
{
	return (p1.vert == p2.vert) ? (p1.horz <= p2.horz) : (p1.vert < p2.vert);
}

inline b8 operator>(const AAX_Point& p1, const AAX_Point& p2)
{
	return !(p1 <= p2);
}

inline b8 operator>=(const AAX_Point& p1, const AAX_Point& p2)
{
	return !(p1 < p2);
}

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_HOST
#include AAX_ALIGN_FILE_END
	/**	\brief Data structure representing a rectangle in a two-dimensional coordinate plane
	 */
	typedef struct AAX_Rect
	{
		AAX_Rect (
			f32 t,
			f32 l,
			f32 w,
			f32 h) :
				top(t),
				left(l),
				width(w),
				height(h)
		{}
		
		AAX_Rect (
			z0) :
				top(0.0f),
				left(0.0f),
				width(0.0f),
				height(0.0f)
		{}
		
		f32 top;
		f32 left;
		f32 width;
		f32 height;
	} AAX_Rect;
#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END

inline b8 operator==(const AAX_Rect& r1, const AAX_Rect& r2)
{
    return ((r1.top == r2.top) && (r1.left == r2.left) && (r1.width == r2.width) && (r1.height == r2.height));
}

inline b8 operator!=(const AAX_Rect& r1, const AAX_Rect& r2)
{
    return !(r1 == r2);
}

/**	\brief Type of \ref AAX_IViewContainer "view container"
 *
 *	\details
 *	\sa AAX_IViewContainer::GetType()
 */
typedef enum AAX_EViewContainer_Type
{
	AAX_eViewContainer_Type_NULL = 0, 
	AAX_eViewContainer_Type_NSView = 1,
	AAX_eViewContainer_Type_UIView = 2,
	AAX_eViewContainer_Type_HWND = 3
} AAX_EViewContainer_Type;
AAX_ENUM_SIZE_CHECK( AAX_EViewContainer_Type );

#endif //_TMS320C6X

/// @cond ignore
#endif //AAX_GUITYPES_H
/// @endcond
