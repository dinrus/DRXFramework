/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFTransport.h
 *
 *	\brief Interface for accessing the host's transport state
 */ 
/*================================================================================================*/

#ifndef AAX_IACFTRANSPORT_H
#define AAX_IACFTRANSPORT_H

#pragma once

#include "AAX.h"
#include "AAX_Enums.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport : public IACFUnknown
{
public:

	virtual AAX_Result	GetCurrentTempo ( f64* TempoBPM ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentTempo()
	virtual AAX_Result	GetCurrentMeter ( i32* MeterNumerator, i32* MeterDenominator ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentMeter()
	virtual AAX_Result	IsTransportPlaying ( b8* isPlaying ) const = 0;	///< \copydoc AAX_ITransport::IsTransportPlaying()
	virtual AAX_Result	GetCurrentTickPosition ( z64* TickPosition ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentTickPosition()
	virtual AAX_Result	GetCurrentLoopPosition ( b8* bLooping, z64* LoopStartTick, z64* LoopEndTick ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentLoopPosition()
	virtual AAX_Result	GetCurrentNativeSampleLocation ( z64* SampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentNativeSampleLocation()
	virtual AAX_Result	GetCustomTickPosition ( z64* oTickPosition, z64 iSampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetCustomTickPosition()
	virtual AAX_Result	GetBarBeatPosition ( i32* Bars, i32* Beats, z64* DisplayTicks, z64 SampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetBarBeatPosition()
	virtual AAX_Result	GetTicksPerQuarter ( u32* ticks ) const = 0;	///< \copydoc AAX_ITransport::GetTicksPerQuarter()
	virtual AAX_Result	GetCurrentTicksPerBeat ( u32* ticks ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentTicksPerBeat()

};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V2 : public AAX_IACFTransport
{
public:

	virtual AAX_Result	GetTimelineSelectionStartPosition( z64* oSampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetTimelineSelectionStartPosition()
	virtual AAX_Result	GetTimeCodeInfo( AAX_EFrameRate* oFrameRate, i32* oOffset ) const = 0;	///< \copydoc AAX_ITransport::GetTimeCodeInfo() 
	virtual AAX_Result	GetFeetFramesInfo( AAX_EFeetFramesRate* oFeetFramesRate, z64* oOffset ) const =  0;	///< \copydoc AAX_ITransport::GetFeetFramesInfo()
	virtual AAX_Result	IsMetronomeEnabled ( i32* isEnabled ) const = 0;	///< \copydoc AAX_ITransport::IsMetronomeEnabled()
};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V3 : public AAX_IACFTransport_V2
{
public:
	
	virtual AAX_Result	GetHDTimeCodeInfo( AAX_EFrameRate* oHDFrameRate, z64* oHDOffset ) const = 0;	///< \copydoc AAX_ITransport::GetHDTimeCodeInfo()
};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V4 : public AAX_IACFTransport_V3
{
public:
	
	virtual AAX_Result	GetTimelineSelectionEndPosition( z64* oSampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetTimelineSelectionEndPosition()
};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V5 : public AAX_IACFTransport_V4
{
public:
	
	virtual AAX_Result	GetKeySignature( z64 iSampleLocation, u32* oKeySignature ) const = 0;	///< \copydoc AAX_ITransport::GetKeySignature()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFTRANSPORT_H

