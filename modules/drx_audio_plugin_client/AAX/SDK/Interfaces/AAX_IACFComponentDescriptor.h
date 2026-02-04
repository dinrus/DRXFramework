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
 *	\file  AAX_IACFComponentDescriptor.h
 *
 *	\brief	Versioned description interface for an %AAX plug-in algorithm callback
 *
 */ 
/*================================================================================================*/


#ifndef _AAX_IACFCOMPONENTDESCRIPTOR_H_
#define _AAX_IACFCOMPONENTDESCRIPTOR_H_

#include "AAX.h"
#include "AAX_Callbacks.h"
#include "AAX_IDma.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/** \brief	Versioned description interface for an %AAX plug-in algorithm callback
 */
class AAX_IACFComponentDescriptor : public IACFUnknown
{
public:
	virtual AAX_Result	Clear () = 0;	///< \copydoc AAX_IComponentDescriptor::Clear()
	virtual AAX_Result	AddReservedField ( AAX_CFieldIndex inFieldIndex, u32 inFieldType ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddReservedField()
    virtual AAX_Result  AddAudioIn ( AAX_CFieldIndex inFieldIndex ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddAudioIn()
    virtual AAX_Result  AddAudioOut ( AAX_CFieldIndex inFieldIndex ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddAudioOut()
    virtual AAX_Result  AddAudioBufferLength ( AAX_CFieldIndex inFieldIndex ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddAudioBufferLength()
    virtual AAX_Result  AddSampleRate ( AAX_CFieldIndex inFieldIndex ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddSampleRate()
    virtual AAX_Result  AddClock ( AAX_CFieldIndex inFieldIndex ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddClock()
    virtual AAX_Result  AddSideChainIn ( AAX_CFieldIndex inFieldIndex ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddSideChainIn()
	virtual AAX_Result	AddDataInPort ( AAX_CFieldIndex inFieldIndex, u32 inPacketSize, AAX_EDataInPortType inPortType) = 0;	///< \copydoc AAX_IComponentDescriptor::AddDataInPort()
	virtual AAX_Result	AddAuxOutputStem ( AAX_CFieldIndex inFieldIndex, i32 inStemFormat, const t8 inNameUTF8[]) = 0;	///< \copydoc AAX_IComponentDescriptor::AddAuxOutputStem()
	virtual AAX_Result	AddPrivateData ( AAX_CFieldIndex inFieldIndex, i32 inDataSize, u32 inOptions = AAX_ePrivateDataOptions_DefaultOptions ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddPrivateData()
	virtual AAX_Result	AddDmaInstance ( AAX_CFieldIndex inFieldIndex, AAX_IDma::EMode inDmaMode ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddDmaInstance()
	virtual AAX_Result	AddMIDINode ( AAX_CFieldIndex inFieldIndex, AAX_EMIDINodeType inNodeType, const t8 inNodeName[], u32 channelMask ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddMIDINode()

	virtual AAX_Result	AddProcessProc_Native (	
		AAX_CProcessProc inProcessProc,
		IACFUnknown * inProperties,
		AAX_CInstanceInitProc inInstanceInitProc,
		AAX_CBackgroundProc inBackgroundProc,
		AAX_CSelector * outProcID) = 0;	///< \copydoc AAX_IComponentDescriptor::AddProcessProc_Native()
	virtual AAX_Result	AddProcessProc_TI ( 
		const t8 inDLLFileNameUTF8 [], 
		const t8 inProcessProcSymbol [],
		IACFUnknown * inProperties,
		const t8 inInstanceInitProcSymbol [],
		const t8 inBackgroundProcSymbol [],
		AAX_CSelector * outProcID) = 0;	///< \copydoc AAX_IComponentDescriptor::AddProcessProc_TI()
	
	virtual AAX_Result	AddMeters ( AAX_CFieldIndex inFieldIndex, const AAX_CTypeID* inMeterIDs, u32k inMeterCount ) = 0;	///< \copydoc AAX_IComponentDescriptor::AddMeters()
};

/** \brief	Versioned description interface for an %AAX plug-in algorithm callback
 */
class AAX_IACFComponentDescriptor_V2 : public AAX_IACFComponentDescriptor
{
public:
    virtual AAX_Result  AddTemporaryData( AAX_CFieldIndex inFieldIndex, u32 inDataElementSize) = 0;	///< \copydoc AAX_IComponentDescriptor::AddTemporaryData()
};

/** \brief	Versioned description interface for an %AAX plug-in algorithm callback
 */
class AAX_IACFComponentDescriptor_V3 : public AAX_IACFComponentDescriptor_V2
{
public:
    virtual AAX_Result AddProcessProc (
		IACFUnknown * inProperties,
		AAX_CSelector* outProcIDs,
		i32 inProcIDsSize) = 0;	///< \copydoc AAX_IComponentDescriptor::AddProcessProc()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // #ifndef _AAX_IACFCOMPONENTDESCRIPTOR_H_
