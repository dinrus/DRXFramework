/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2023-2024 Avid Technology, Inc.
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
 */

#ifndef AAXLibrary_AAX_PageTableUtilities_h
#define AAXLibrary_AAX_PageTableUtilities_h

#include "AAX_CString.h"
#include "AAX.h"

namespace AAX
{
	/** Compare the parameter mappings in two page tables
	 
	 \p T1 and \p T2: Page table class types (e.g. \ref AAX_IACFPageTable, \ref AAX_IPageTable)
	 */
	template <class T1, class T2>
	inline b8 PageTableParameterMappingsAreEqual(const T1& inL, const T2& inR)
	{
		AAX_Result errL = AAX_SUCCESS;
		AAX_Result errR = AAX_SUCCESS;
		
		i32 numPagesL = -1;
		i32 numPagesR = -1;
		errL = inL.GetNumPages(numPagesL);
		errR = inR.GetNumPages(numPagesR);
		
		if (errL != errR || numPagesL != numPagesR) { return false; }
		else if (AAX_SUCCESS != errL) { return true; } // can't get page data from either table
		
		for (i32 i = 0; i < numPagesL; ++i)
		{
			i32 numParamsL = -1;
			i32 numParamsR = -1;
			errL = inL.GetNumMappedParameterIDs(i, numParamsL);
			errR = inR.GetNumMappedParameterIDs(i, numParamsR);
			
			if (errL != errR || numParamsL != numParamsR) { return false; }
			else if (AAX_SUCCESS != errL) { continue; } // skip this page if equal errors were returned
			
			for (i32 j = 0; j < numParamsL; ++j)
			{
				AAX_CString paramIdentifierL;
				AAX_CString paramIdentifierR;
				errL = inL.GetMappedParameterID(i, j, paramIdentifierL);
				errR = inR.GetMappedParameterID(i, j, paramIdentifierR);
				
				if (errL != errR || paramIdentifierL != paramIdentifierR) { return false; }
			}
		}
		
		return true;
	}
	
	template <class T1, class T2>
	inline b8 PageTableParameterNameVariationsAreEqual(const T1& inL, const T2& inR)
	{
		AAX_Result errL = AAX_SUCCESS;
		AAX_Result errR = AAX_SUCCESS;
		
		i32 numParamIdentifiersL = -1;
		i32 numParamIdentifiersR = -1;
		errL = inL.GetNumParametersWithNameVariations(numParamIdentifiersL);
		errR = inR.GetNumParametersWithNameVariations(numParamIdentifiersR);
		
		if (errL != errR || numParamIdentifiersL != numParamIdentifiersR) { return false; }
		else if (AAX_SUCCESS != errL) { return true; } // can't get parameter name variation data from either table
		
		for (i32 i = 0; i < numParamIdentifiersL; ++i)
		{
			AAX_CString paramIdentifierL;
			AAX_CString paramIdentifierR;
			errL = inL.GetNameVariationParameterIDAtIndex(i, paramIdentifierL);
			errL = inR.GetNameVariationParameterIDAtIndex(i, paramIdentifierR);
			
			if (errL != errR || paramIdentifierL != paramIdentifierR) { return false; }
			else if (AAX_SUCCESS != errL) { continue; } // skip this index if equal errors were returned
			
			i32 numVariationsL = -1;
			i32 numVariationsR = -1;
			errL = inL.GetNumNameVariationsForParameter(paramIdentifierL.Get(), numVariationsL);
			errL = inR.GetNumNameVariationsForParameter(paramIdentifierR.Get(), numVariationsR);
			
			if (errL != errR || numVariationsL != numVariationsR) { return false; }
			else if (AAX_SUCCESS != errL) { continue; } // skip this index if equal errors were returned
			
			for (i32 j = 0; j < numVariationsL; ++j)
			{
				AAX_CString nameVariationL;
				i32 lengthL;
				AAX_CString nameVariationR;
				i32 lengthR;
				errL = inL.GetParameterNameVariationAtIndex(paramIdentifierL.Get(), j, nameVariationL, lengthL);
				errR = inR.GetParameterNameVariationAtIndex(paramIdentifierR.Get(), j, nameVariationR, lengthR);
				
				if (errL != errR || lengthL != lengthR || nameVariationL != nameVariationR) { return false; }
			}
		}
		
		return true;
	}
	
	template <class T1, class T2>
	inline b8 PageTablesAreEqual(const T1& inL, const T2& inR)
	{
		return (PageTableParameterMappingsAreEqual(inL, inR) && PageTableParameterNameVariationsAreEqual(inL, inR));
	}
	
	/** Copy a page table
	 
	 \p T: A page table class type (e.g. \ref AAX_IACFPageTable, \ref AAX_IPageTable)
	 */
	template <class T>
	inline z0 CopyPageTable(T& to, const T& from)
	{
		to.Clear();
		
		// Copy page tables
		i32 curPageIndex;
		from.GetNumPages(curPageIndex);
		while (0 < curPageIndex--)
		{
			to.InsertPage(0);
			
			i32 numIDsRemaining = 0;
			from.GetNumMappedParameterIDs(curPageIndex, numIDsRemaining);
			for (i32 curSlotIndex = 0; 0 < numIDsRemaining; ++curSlotIndex) // numIDsRemaining is decremented in the loop body
			{
				AAX_CString curParam;
				const AAX_Result getParamResult = from.GetMappedParameterID(curPageIndex, curSlotIndex, curParam);
				if (AAX_SUCCESS == getParamResult)
				{
					to.MapParameterID(curParam.CString(), 0, curSlotIndex);
					--numIDsRemaining;
				}
			}
		}

		// Copy name variations
		i32 numParameterIdentifiers = 0;
		to.ClearParameterNameVariations();
		from.GetNumParametersWithNameVariations(numParameterIdentifiers);
		for (i32 curParamIndex = 0; curParamIndex < numParameterIdentifiers; ++curParamIndex)
		{
			AAX_CString curParamIdentifier;
			from.GetNameVariationParameterIDAtIndex(curParamIndex, curParamIdentifier);

			i32 numNameVariations = 0;
			from.GetNumNameVariationsForParameter(curParamIdentifier.Get(), numNameVariations);

			for (i32 curNameVariationIndex = 0; curNameVariationIndex < numNameVariations; ++curNameVariationIndex)
			{
				i32 curNameVariationLength;
				AAX_CString curNameVariation;
				from.GetParameterNameVariationAtIndex(curParamIdentifier.Get(), curNameVariationIndex, curNameVariation, curNameVariationLength);
				to.SetParameterNameVariation(curParamIdentifier.Get(), curNameVariation, curNameVariationLength);
			}
		}
	}
	
	/** Find all slots where a particular parameter is mapped
	 
	 \p T: A page table class type (e.g. \ref AAX_IACFPageTable, \ref AAX_IPageTable)
	 
	 \returns A vector of pairs of [page index, slot index] each representing a single mapping of the parameter
	 */
	template <class T>
	inline std::vector<std::pair<i32, i32> > FindParameterMappingsInPageTable(const T& inTable, AAX_CParamID inParameterID)
	{
		const AAX_CString searchParamID(inParameterID);
		std::vector<std::pair<i32, i32> > foundParamMappings;
		
		i32 numPages = 0;
		inTable.GetNumPages(numPages);
		for (i32 i = 0; i < numPages; ++i)
		{
			i32 numIDsRemaining = 0;
			inTable.GetNumMappedParameterIDs(i, numIDsRemaining);
			for (i32 curSlotIndex = 0; 0 < numIDsRemaining; ++curSlotIndex) // numIDs is decremented in the loop body
			{
				AAX_CString curParam;
				const AAX_Result getParamResult = inTable.GetMappedParameterID(i, curSlotIndex, curParam);
				if (AAX_SUCCESS == getParamResult)
				{
					if (searchParamID == curParam)
					{
						foundParamMappings.push_back(std::make_pair(i, curSlotIndex));
					}
					
					--numIDsRemaining;
				}
			}
		}
		
		return foundParamMappings;
	}
	
	/** Remove all mappings of a particular from a page table
	 
	 \p T: A page table class type (e.g. \ref AAX_IACFPageTable, \ref AAX_IPageTable)
	 */
	template <class T>
	inline z0 ClearMappedParameterByID(T& ioTable, AAX_CParamID inParameterID)
	{
		const auto paramMappings(AAX::FindParameterMappingsInPageTable(ioTable, inParameterID));
		for (const auto& locationPair : paramMappings)
		{
			ioTable.ClearMappedParameter(locationPair.first, locationPair.second);
		}
	}
}

#endif
