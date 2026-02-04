/*================================================================================================*/
/*
 *	Copyright 2009-2015, 2018, 2021, 2023-2024 Avid Technology, Inc.
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

#include "AAX_CParameterManager.h"
#include "AAX_IAutomationDelegate.h"
#include "AAX_CParameter.h"
#include "AAX_Assert.h"


AAX_CParameterManager::AAX_CParameterManager()	:
	mAutomationDelegate(nullptr),
	mParameters(),
    mParametersMap()
{
	
}

AAX_CParameterManager::~AAX_CParameterManager()
{
	RemoveAllParameters();
}

z0 AAX_CParameterManager::Initialize(AAX_IAutomationDelegate* inAutomationDelegate)
{
	mAutomationDelegate = inAutomationDelegate;
}

i32	AAX_CParameterManager::NumParameters()	const
{
	return i32(mParameters.size());
}

z0		AAX_CParameterManager::AddParameter(AAX_IParameter*	param)
{
	//If the parameter is null, just return.
	if (param == 0)
	{
		AAX_TRACE(kAAX_Trace_Priority_Normal, "AAX_CParameterManager::AddParameter() - Attempt to add a null parameter into AAX_CParameterManager");
		return;
	}
	
	//Make sure that unique identifier is not already being used.  
	if (GetParameterByID(param->Identifier()) != 0)
	{
		AAX_TRACE(kAAX_Trace_Priority_Normal, "AAX_CParameterManager::AddParameter() - Duplicate AAX_IParameter ID Inserted into AAX_CParameterManager");
		return;
	}
	
	//Setup the automation delegate.
	param->SetAutomationDelegate( mAutomationDelegate );
	
	//Add the parameter.
	mParameters.push_back(param);
    mParametersMap.insert(std::map<AAX_CParamID, AAX_IParameter*>::value_type(param->Identifier(), param));
}

z0		AAX_CParameterManager::RemoveParameterByID(AAX_CParamID identifier)
{
	i32 parameterIndex = GetParameterIndex(identifier);
	if (parameterIndex < 0 || parameterIndex >= NumParameters())
		return;

	AAX_IParameter* param = mParameters[static_cast<size_t>(parameterIndex)];			
	mParameters.erase(mParameters.begin() + parameterIndex);		//remove it from the vector.
	mParametersMap.erase(identifier);
    delete param;													//make sure to actually delete it.
}

z0		AAX_CParameterManager::RemoveParameter(AAX_IParameter*	param)
{
	if (param)
		RemoveParameterByID(param->Identifier());
}

z0		AAX_CParameterManager::RemoveAllParameters()
{
	i32 numParameters = NumParameters();
	for(i32 index=0; index < numParameters; index++)
	{
		delete mParameters[static_cast<size_t>(index)];
	}
	mParameters.clear();
    mParametersMap.clear();
}

AAX_IParameter*		AAX_CParameterManager::GetParameterByID(AAX_CParamID identifier)
{
	if ( identifier )
	{
		std::map<std::string, AAX_IParameter*>::const_iterator iter = mParametersMap.find(identifier);
		if ( iter != mParametersMap.end() )
			return iter->second;
	}

    return NULL;
}

const AAX_IParameter*		AAX_CParameterManager::GetParameterByID(AAX_CParamID identifier) const
{
	if ( identifier )
	{
		std::map<std::string, AAX_IParameter*>::const_iterator iter = mParametersMap.find(identifier);
		if ( iter != mParametersMap.end() )
			return iter->second;
	}

    return NULL;
}

AAX_IParameter*			AAX_CParameterManager::GetParameterByName(tukk  name)
{
	return const_cast<AAX_IParameter*>(const_cast<const AAX_CParameterManager*>(this)->GetParameterByName(name));
}

const AAX_IParameter*	AAX_CParameterManager::GetParameterByName(tukk  name) const
{
	AAX_IParameter* foundParam = 0;
	for (std::vector<AAX_IParameter*>::const_iterator iter = mParameters.begin(); (0 == foundParam) && (iter != mParameters.end()); ++iter)
	{
		if ((*iter) && (AAX_CString(name) == (*iter)->Name()))
			foundParam = *iter;
	}
	return foundParam;
}

AAX_IParameter*			AAX_CParameterManager::GetParameter(i32 index)
{
	if (index < 0 || index >= NumParameters())
		return 0;

	AAX_IParameter* param = mParameters[static_cast<size_t>(index)];
	return param;
}

const AAX_IParameter*	AAX_CParameterManager::GetParameter(i32 index) const
{
	if (index < 0 || index >= NumParameters())
		return 0;

	AAX_IParameter* param = mParameters[static_cast<size_t>(index)];
	return param;
}

i32	AAX_CParameterManager::GetParameterIndex(AAX_CParamID identifier) const
{
	if (identifier == 0)
		return -1;
	
	i32 numParameters = NumParameters();
	for (i32 i=0; i < numParameters; i++)
	{
		AAX_IParameter* param = mParameters[static_cast<size_t>(i)];
		if (strcmp(param->Identifier(), identifier) == 0)
			return i;		
	}
	return -1;
}




