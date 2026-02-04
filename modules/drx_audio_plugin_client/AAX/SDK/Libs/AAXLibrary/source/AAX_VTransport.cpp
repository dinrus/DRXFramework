/*================================================================================================*/
/*
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
/*================================================================================================*/

#include "AAX_VTransport.h"
#include "AAX_UIDs.h"

AAX_VTransport::AAX_VTransport( IACFUnknown* pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXTransportV1, (uk *)&mITransport);
		pUnknown->QueryInterface(IID_IAAXTransportV2, (uk *)&mITransportV2);
		pUnknown->QueryInterface(IID_IAAXTransportV3, (uk *)&mITransportV3);
		pUnknown->QueryInterface(IID_IAAXTransportV4, (uk *)&mITransportV4);
		pUnknown->QueryInterface(IID_IAAXTransportV5, (uk *)&mITransportV5);
		pUnknown->QueryInterface(IID_IAAXTransportControlV1, (uk *)&mITransportControl);
	}
}

AAX_VTransport::~AAX_VTransport()
{
}

AAX_Result AAX_VTransport::GetCurrentTempo ( f64* TempoBPM ) const
{
	if ( mITransport )
		return mITransport->GetCurrentTempo( TempoBPM );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetCurrentMeter ( i32* MeterNumerator, i32* MeterDenominator ) const
{
	if ( mITransport )
		return mITransport->GetCurrentMeter ( MeterNumerator, MeterDenominator );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::IsTransportPlaying ( b8* isPlaying ) const
{
	if ( mITransport )
		return mITransport->IsTransportPlaying ( isPlaying );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetCurrentTickPosition ( z64* TickPosition ) const
{
	if ( mITransport )
		return mITransport->GetCurrentTickPosition ( TickPosition );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetCurrentLoopPosition ( b8* bLooping, z64* LoopStartTick, z64* LoopEndTick ) const
{
	if ( mITransport )
		return mITransport->GetCurrentLoopPosition ( bLooping, LoopStartTick, LoopEndTick );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetCurrentNativeSampleLocation ( z64* SampleLocation ) const
{
	if ( mITransport )
		return mITransport->GetCurrentNativeSampleLocation ( SampleLocation );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetCustomTickPosition( z64* oTickPosition, z64 iSampleLocation) const
{
	if ( mITransport )
		return mITransport->GetCustomTickPosition( oTickPosition, iSampleLocation);
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetBarBeatPosition(i32* Bars, i32* Beats, z64* DisplayTicks, z64 SampleLocation) const
{
	if ( mITransport )
		return mITransport->GetBarBeatPosition( Bars, Beats, DisplayTicks, SampleLocation);
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetTicksPerQuarter ( u32* ticks ) const
{
	if ( mITransport )
		return mITransport->GetTicksPerQuarter ( ticks );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetCurrentTicksPerBeat ( u32* ticks ) const
{
	if ( mITransport )
		return mITransport->GetCurrentTicksPerBeat ( ticks );
	
	return AAX_ERROR_NULL_OBJECT;
}

AAX_Result AAX_VTransport::GetTimelineSelectionStartPosition ( z64* oSampleLocation ) const
{
	if ( mITransportV2 )
		return mITransportV2->GetTimelineSelectionStartPosition ( oSampleLocation );

    return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::GetTimeCodeInfo( AAX_EFrameRate* oFrameRate, i32* oOffset ) const
{
	if ( mITransportV2 )
		return mITransportV2->GetTimeCodeInfo( oFrameRate, oOffset );

    return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::GetFeetFramesInfo( AAX_EFeetFramesRate* oFeetFramesRate, z64* oOffset ) const
{
	if ( mITransportV2 )
		return mITransportV2->GetFeetFramesInfo( oFeetFramesRate, oOffset );

    return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::IsMetronomeEnabled ( i32* isEnabled ) const
{
	if ( mITransportV2 )
		return mITransportV2->IsMetronomeEnabled( isEnabled );

    return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::GetHDTimeCodeInfo( AAX_EFrameRate* oHDFrameRate, z64* oHDOffset ) const
{
	if ( mITransportV3 )
		return mITransportV3->GetHDTimeCodeInfo( oHDFrameRate, oHDOffset );
	
	return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::GetTimelineSelectionEndPosition( z64* oSampleLocation ) const
{
	if ( mITransportV4 )
		return mITransportV4->GetTimelineSelectionEndPosition ( oSampleLocation );

    return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::GetKeySignature( z64 iSampleLocation, u32* oKeySignature ) const
{
	if ( mITransportV5 )
		return mITransportV5->GetKeySignature( iSampleLocation, oKeySignature );

	return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::RequestTransportStart()
{
	if ( mITransportControl )
		return mITransportControl->RequestTransportStart();

    return AAX_ERROR_UNIMPLEMENTED;
}

AAX_Result AAX_VTransport::RequestTransportStop()
{
	if ( mITransportControl )
		return mITransportControl->RequestTransportStop();

    return AAX_ERROR_UNIMPLEMENTED;
}
