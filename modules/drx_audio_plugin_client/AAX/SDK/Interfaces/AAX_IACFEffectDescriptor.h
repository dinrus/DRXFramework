/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFEffectDescriptor.h
 *
 *	\brief Versioned interface for an AAX_IEffectDescriptor
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFEFFECTDESCRIPTOR_H
#define AAX_IACFEFFECTDESCRIPTOR_H

#include "AAX.h"
#include "AAX_Callbacks.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/**	\brief Versioned interface for an AAX_IEffectDescriptor
 */ 
class AAX_IACFEffectDescriptor : public IACFUnknown
{
public:
	
	virtual AAX_Result							AddComponent ( IACFUnknown* inComponentDescriptor ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddComponent()
	virtual AAX_Result							AddName ( const t8 *inPlugInName ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddName()
	virtual AAX_Result							AddCategory ( u32 inCategory ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddCategory()
	virtual AAX_Result							AddCategoryBypassParameter ( u32 inCategory, AAX_CParamID inParamID ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddCategoryBypassParameter()
	virtual AAX_Result							AddProcPtr ( uk  inProcPtr, AAX_CProcPtrID inProcID ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddProcPtr()
	virtual AAX_Result							SetProperties ( IACFUnknown * inProperties ) = 0;	///< \copydoc AAX_IEffectDescriptor::SetProperties()
	virtual AAX_Result							AddResourceInfo ( AAX_EResourceType inResourceType, const t8 * inInfo ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddResourceInfo()
	virtual AAX_Result							AddMeterDescription( AAX_CTypeID inMeterID, const t8 * inMeterName, IACFUnknown * inProperties ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddMeterDescription()
};

/**
 *	\brief Versioned interface for an AAX_IEffectDescriptor
 *
 */
class AAX_IACFEffectDescriptor_V2 : public AAX_IACFEffectDescriptor
{
public:
	virtual AAX_Result							AddControlMIDINode ( AAX_CTypeID inNodeID, AAX_EMIDINodeType inNodeType, const t8 inNodeName[], u32 inChannelMask ) = 0;	///< \copydoc AAX_IEffectDescriptor::AddControlMIDINode()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFEFFECTDESCRIPTOR_H
