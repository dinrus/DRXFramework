/*================================================================================================*/
/*
 *	Copyright 2013-2015, 2018, 2023-2024 Avid Technology, Inc.
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

#include "AAX_VHostServices.h"
#include "AAX_UIDs.h"
#include "acfbaseapi.h"
#include "ACFPtr.h"

// ******************************************************************************************
// METHOD:	AAX_VHostServices
// ******************************************************************************************
AAX_VHostServices::AAX_VHostServices( IACFUnknown * pUnkHost ) :
	mIACFHostServices( NULL ),
	mIACFHostServices2( NULL )
{
	if ( pUnkHost )
	{
		// Get the component factory service from the host so we can create the 
		// built-in plug-in definition.
		ACFPtr<IACFComponentFactory> pFactory;
		if ( pUnkHost->QueryInterface(IID_IACFComponentFactory, (uk *)&pFactory) == ACF_OK )
		{
			// Create the object and get the base interface for it.
			pFactory->CreateComponent(AAXCompID_HostServices, 0, IID_IAAXHostServicesV1, (uk *)&mIACFHostServices);
			pFactory->CreateComponent(AAXCompID_HostServices, 0, IID_IAAXHostServicesV2, (uk *)&mIACFHostServices2);
			pFactory->CreateComponent(AAXCompID_HostServices, 0, IID_IAAXHostServicesV3, (uk *)&mIACFHostServices3);
		}
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VHostServices
// ******************************************************************************************
AAX_VHostServices::~AAX_VHostServices( )
{
}

// ******************************************************************************************
// METHOD:	HandleAssertFailure
// ******************************************************************************************
AAX_Result AAX_VHostServices::HandleAssertFailure ( const t8 * iFile, i32 iLine, const t8 * iNote, i32 iFlags ) const
{
	if (mIACFHostServices3) {
		return mIACFHostServices3->HandleAssertFailure( iFile, iLine, iNote, iFlags );
	}
// See comment at AAX_IACFHostServices::Assert()
#if defined _DEBUG
	else if (mIACFHostServices) {
		return const_cast<AAX_VHostServices*>(this)->mIACFHostServices->Assert( iFile, iLine, iNote );
	}
#endif
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	Trace
// ******************************************************************************************
AAX_Result AAX_VHostServices::Trace ( i32 iPriority, const t8 * iMessage ) const
{
	if ( mIACFHostServices )
		return const_cast<AAX_VHostServices*>(this)->mIACFHostServices->Trace( iPriority, iMessage );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	StackTrace
// ******************************************************************************************
AAX_Result AAX_VHostServices::StackTrace ( i32 iTracePriority, i32 iStackTracePriority, const t8 * iMessage ) const
{
	if ( mIACFHostServices2 )
		return const_cast<AAX_VHostServices*>(this)->mIACFHostServices2->StackTrace( iTracePriority, iStackTracePriority, iMessage );
	
	return AAX_ERROR_UNIMPLEMENTED;
}
