/*================================================================================================*/
/*
 *	Copyright 2010-2017, 2023-2024 Avid Technology, Inc.
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

#include "AAX_CHostProcessor.h"
#include "AAX_IEffectParameters.h"
#include "AAX_VHostProcessorDelegate.h"
#include "AAX_VController.h"
#include "AAX_IEffectParameters.h"
#include "AAX_UIDs.h"

// ***************************************************************************
// METHOD:	AAX_CHostProcessor
// ***************************************************************************
AAX_CHostProcessor::AAX_CHostProcessor (z0) :
	mController( NULL ),
	mHostProcessingDelegate ( NULL ),
	mEffectParameters( NULL )
{		
	mSrcStart			= 0;
	mSrcEnd				= 0;
	mDstStart			= 0;
	mDstEnd				= 0;	
	mLocation			= 0;
}

// ***************************************************************************
// METHOD:	~AAX_CHostProcessor
// ***************************************************************************
AAX_CHostProcessor::~AAX_CHostProcessor ()
{
	Uninitialize();	//<DMT> Just try to f64 check that this was all torn down.
}

// ***************************************************************************
// METHOD:	Initialize
// ***************************************************************************
AAX_Result		AAX_CHostProcessor::Initialize(IACFUnknown* iController)
{
	mController = new AAX_VController(iController);
	if (iController == 0 || mController == 0)
		return AAX_ERROR_NOT_INITIALIZED;

	mHostProcessingDelegate = new AAX_VHostProcessorDelegate(iController);
	if (iController == 0 || mHostProcessingDelegate == 0)
		return AAX_ERROR_NOT_INITIALIZED;
	
	if ( iController )
	{
		iController->QueryInterface(IID_IAAXEffectParametersV1, (uk *)&mEffectParameters);		
	}	
	
	return AAX_SUCCESS;
}
		
// *******************************************************************************
// METHOD:	Uninitialize
// *******************************************************************************
AAX_Result		AAX_CHostProcessor::Uninitialize ( )
{
	if (mEffectParameters)
	{
		mEffectParameters->Release();
		mEffectParameters = 0;
	}
	
	if ( mHostProcessingDelegate )
	{
		delete ( mHostProcessingDelegate );
		mHostProcessingDelegate = 0;
	}
	
	if ( mController )
	{
		delete ( mController );
		mController = 0;
	}	
	return AAX_SUCCESS;
}
		

// ***************************************************************************
// METHOD:	InitOutputBounds
// ***************************************************************************
AAX_Result AAX_CHostProcessor::InitOutputBounds ( z64 iSrcStart, z64 iSrcEnd, z64 * oDstStart, z64 * oDstEnd )
{
	AAX_Result result = AAX_SUCCESS;
	if (oDstStart && oDstEnd)
	{
		mSrcStart = iSrcStart;
		mSrcEnd = iSrcEnd;

		this->TranslateOutputBounds( mSrcStart, mSrcEnd, *oDstStart, *oDstEnd );

		mDstStart = *oDstStart;
		mDstEnd = *oDstEnd;
	}
	else
	{
		result = AAX_ERROR_PLUGIN_NULL_PARAMETER;
	}
		
	return result;
}

// ***************************************************************************
// METHOD:	TranslateOutputBounds
// ***************************************************************************
AAX_Result AAX_CHostProcessor::TranslateOutputBounds ( z64 iSrcStart, z64 iSrcEnd, z64& oDstStart, z64& oDstEnd )
{
	oDstStart = iSrcStart;
	oDstEnd = iSrcEnd;
	return AAX_SUCCESS;
}

// ***************************************************************************
// METHOD:	SetLocation
// ***************************************************************************
AAX_Result AAX_CHostProcessor::SetLocation ( z64 iSample )
{
	mLocation = iSample;
	return AAX_SUCCESS;
}

// ***************************************************************************
// METHOD:	RenderAudio
// ***************************************************************************
AAX_Result AAX_CHostProcessor::RenderAudio ( const f32 * const /*inAudioIns*/ [], i32 /*inAudioInCount*/, f32 * const /*iAudioOuts*/ [], i32 /*iAudioOutCount*/, i32 * /*ioWindowSize*/ )
{
	return AAX_ERROR_UNIMPLEMENTED;
}

// ***************************************************************************
// METHOD:	PreRender
// ***************************************************************************
AAX_Result AAX_CHostProcessor::PreRender ( i32 /*inAudioInCount*/, i32 /*iAudioOutCount*/, i32 /*iWindowSize*/ )
{
	return AAX_SUCCESS;
}

// ***************************************************************************
// METHOD:	PostRender
// ***************************************************************************
AAX_Result AAX_CHostProcessor::PostRender ()
{
	return AAX_SUCCESS;
}

// ***************************************************************************
// METHOD:	AnalyzeAudio
// ***************************************************************************
AAX_Result AAX_CHostProcessor::AnalyzeAudio ( const f32 * const /*inAudioIns*/ [], i32 /*inAudioInCount*/, i32 * /*ioWindowSize*/ )
{
	return AAX_ERROR_UNIMPLEMENTED;
}

// ***************************************************************************
// METHOD:	PreAnalyze
// ***************************************************************************
AAX_Result AAX_CHostProcessor::PreAnalyze ( i32 /*inAudioInCount*/, i32 /*iWindowSize*/ )
{
	return AAX_SUCCESS;
}

// ***************************************************************************
// METHOD:	PostAnalyze
// ***************************************************************************
AAX_Result AAX_CHostProcessor::PostAnalyze ()
{
	return AAX_SUCCESS;
}

// ***************************************************************************
// METHOD:	GetClipNameSuffix
// ***************************************************************************
AAX_Result AAX_CHostProcessor::GetClipNameSuffix ( i32 /*inMaxLength*/, AAX_IString* /*outString*/ ) const
{
    return AAX_ERROR_UNIMPLEMENTED;
}

// ***************************************************************************
// METHOD:	GetAudio
// ***************************************************************************
AAX_Result AAX_CHostProcessor::GetAudio ( const f32 * const inAudioIns [], i32 inAudioInCount, z64 inLocation, i32 * ioNumSamples )
{
	return mHostProcessingDelegate->GetAudio( inAudioIns, inAudioInCount, inLocation, ioNumSamples );
}

// ***************************************************************************
// METHOD:	GetSideChainInputNum
// ***************************************************************************
i32 AAX_CHostProcessor::GetSideChainInputNum ()
{
	return mHostProcessingDelegate->GetSideChainInputNum();
}
