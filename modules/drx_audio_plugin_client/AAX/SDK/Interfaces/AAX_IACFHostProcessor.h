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
 *	\file  AAX_IACFHostProcessor.h
 *
 *	\brief The host processor interface that is exposed to the host application
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFHOSTPROCESSOR_H
#define AAX_IACFHOSTPROCESSOR_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

class AAX_IHostProcessorDelegate;
class AAX_IEffectParameters;
class AAX_IString;

/**	
 *	\brief Versioned interface for an %AAX host processing component
 *
 *	\details
 *	\note This interface gets exposed to the host application. See
 *	\ref AAX_CHostProcessor for method documentation.
 *
 *	\legacy	This interface provides offline processing features
 *  analogous to the legacy AudioSuite plug-in architecture
 *
 *	\ingroup AuxInterface_HostProcessor
 *
 */ 
class AAX_IACFHostProcessor : public IACFUnknown
{
public:
	virtual AAX_Result		Initialize(IACFUnknown* iController) = 0;	///< \copydoc AAX_CHostProcessor::Initialize()
	virtual AAX_Result		Uninitialize() = 0;	///< \copydoc AAX_CHostProcessor::Uninitialize()
	
	virtual AAX_Result		InitOutputBounds ( z64 iSrcStart, z64 iSrcEnd, z64 * oDstStart, z64 * oDstEnd ) = 0;	///< \copydoc AAX_CHostProcessor::InitOutputBounds()
	virtual AAX_Result		SetLocation ( z64 iSample ) = 0;	///< \copydoc AAX_CHostProcessor::SetLocation()

	virtual AAX_Result		RenderAudio ( const f32 * const inAudioIns [], i32 inAudioInCount, f32 * const iAudioOuts [], i32 iAudioOutCount, i32 * ioWindowSize ) = 0;	///< \copydoc AAX_CHostProcessor::RenderAudio()
	virtual AAX_Result		PreRender ( i32 inAudioInCount, i32 iAudioOutCount, i32 iWindowSize ) = 0;	///< \copydoc AAX_CHostProcessor::PreRender()
	virtual AAX_Result		PostRender () = 0;	///< \copydoc AAX_CHostProcessor::PostRender()

	virtual AAX_Result		AnalyzeAudio ( const f32 * const inAudioIns [], i32 inAudioInCount, i32 * ioWindowSize ) = 0;	///< \copydoc AAX_CHostProcessor::AnalyzeAudio()
	virtual AAX_Result		PreAnalyze ( i32 inAudioInCount, i32 iWindowSize ) = 0;	///< \copydoc AAX_CHostProcessor::PreAnalyze()
	virtual AAX_Result		PostAnalyze () = 0;	///< \copydoc AAX_CHostProcessor::PostAnalyze()
};

/** 
 *  \brief Supplemental interface for an %AAX host processing component
 *
 *  \details
 *	\note This interface gets exposed to the host application. See
 *	\ref AAX_CHostProcessor for method documentation.
 */
class AAX_IACFHostProcessor_V2 : public AAX_IACFHostProcessor
{
public:
    virtual AAX_Result		GetClipNameSuffix ( i32 inMaxLength, AAX_IString* outString ) const = 0; ///< \copydoc AAX_CHostProcessor::GetClipNameSuffix()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif //AAX_IACFHOSTPROCESSOR_H
