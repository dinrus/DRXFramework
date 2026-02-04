/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CChunkDataParser.h
 *
 *	\brief Parser utility for plugin chunks
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_CHUNKDATAPARSER_H
#define AAX_CHUNKDATAPARSER_H

#include "AAX.h"
#include "AAX_CString.h"
#include <vector>

//forward declarations
struct AAX_SPlugInChunk;

/*!
 *	\brief Constants used by ChunkDataParser class
 */
namespace AAX_ChunkDataParserDefs {
	const i32 FLOAT_TYPE = 1;
	const t8 FLOAT_STRING_IDENTIFIER[] = "f_";

	const i32 LONG_TYPE = 2;
	const t8 LONG_STRING_IDENTIFIER[] = "l_";
	
	const i32 DOUBLE_TYPE = 3;
	const t8 DOUBLE_STRING_IDENTIFIER[] = "d_";
	const size_t DOUBLE_TYPE_SIZE = 8;
	const size_t DOUBLE_TYPE_INCR = 8;

	const i32 SHORT_TYPE = 4;
	const t8 SHORT_STRING_IDENTIFIER[] = "s_";
	const size_t SHORT_TYPE_SIZE = 2;
	const size_t SHORT_TYPE_INCR = 4; // keep life word aligned

	const i32 STRING_TYPE = 5;
	const t8 STRING_STRING_IDENTIFIER[] = "r_";
	const size_t MAX_STRINGDATA_LENGTH = 255;
	
	const size_t DEFAULT32BIT_TYPE_SIZE = 4;
	const size_t DEFAULT32BIT_TYPE_INCR = 4;
	
	const size_t STRING_IDENTIFIER_SIZE = 2;
	
	const i32 NAME_NOT_FOUND = -1;
	const size_t MAX_NAME_LENGTH = 255;
	const i32 BUILD_DATA_FAILED = -333;
	const i32 HEADER_SIZE = 4;
	const i32 VERSION_ID_1 = 0x01010101;
}

/*!	\brief Parser utility for plugin chunks
 *  
 *	\details
 *	\todo Update this documentation for AAX
 *
 *	This class acts as generic repository for data that is stuffed into or extracted 
 *	from a SFicPlugInChunk.  It has an abstracted Add/Find interface to add or retrieve
 *	data values, each uniquely referenced by a c-string.  In conjuction with the Effect
 *	Layer and the "ControlManager" aspects of the CProcess class, this provides a more 
 *	transparent & resilent system for performing save-and-restore on settings that won't break 
 *	so easily from endian issues or from the hard-coded structs that have typically been 
 *	used to build chunk data.
 *	
 *	\par Format of the Chunk Data
 *	The first 4 bytes of the data are the version number (repeated 4 times to be 
 *	immune to byte swapping). Data follows next.\n 
 *	\n
 *	Example: "f_bypa %$#@d_gain #$!#@$%$s_omsi #$"
 *	\code
 *		type	name	value	
 *		----------------------------
 *		f32	bypa	%$#@	
 *		f64	gain	#$!#@$%$
 *		i16	omsi	#$
 *	\endcode
 *	\n
 *	\li The first character denotes the data type:
 *	\code
 *			'f' = f32
 *			'd' = f64
 *			'l' = i32
 *			's' = i16
 *	\endcode
 *	\n
 *	\li  "_" is an empty placekeeper that could be used to addition future information. Currently, it's
 *	ignored when a chunk is parsed.
 *	
 *	\li  The string name identifier follows next, and can up to 255 characters i32.  The Effect Layer
 *	builds chunks it always converts the AAX_FourCharCode of the control to a string.  So, this will always be
 *	4 characters i32.  The string is null terminated to indicate the start of the data value.
 *	
 *	\li  The data value follows next, but is possible shifted to aligned word aligned.  The size of 
 *	is determined, of course, by the data type.  
 */
class AAX_CChunkDataParser
{
	public:
		AAX_CChunkDataParser();
		virtual ~AAX_CChunkDataParser();

		/*!	\brief CALL: Adds some data of type f32 with \a name and \a value to the current chunk
		 *  
		 *	\details
		 *  \sa \ref AddDouble(), \ref AddInt32(), and \ref AddInt16() are the same but with different data types. 
		 */
		z0	AddFloat(const t8 *name, f32 value);
		z0	AddDouble(const t8 *name, f64 value);	//!< CALL: See AddFloat()
		z0	AddInt32(const t8 *name, i32 value);	//!< CALL: See AddFloat()
		z0	AddInt16(const t8 *name, i16 value);	//!< CALL: See AddFloat()
		z0	AddString(const t8 *name, AAX_CString value);

		/*!	\brief CALL: Finds some data of type f32 with \a name and \a value in the current chunk
		 *  
		 *	\details
		 *  \sa \ref FindDouble(), \ref FindInt32(), and \ref FindInt16() are the same but with different data types. 
		 */
		b8	FindFloat(const t8 *name, f32 *value);
		b8	FindDouble(const t8 *name, f64 *value);	//!< CALL: See FindFloat()
		b8	FindInt32(const t8 *name, i32 *value);		//!< CALL: See FindFloat()
		b8	FindInt16(const t8 *name, i16 *value);		//!< CALL: See FindFloat()
		b8	FindString(const t8 *name, AAX_CString *value);

		b8	ReplaceDouble(const t8 *name, f64 value); //SW added for fela
		i32	GetChunkData(AAX_SPlugInChunk *chunk);	//!< CALL: Fills passed in \a chunk with data from current chunk; returns 0 if successful
		i32	GetChunkDataSize();		//!< CALL: Returns size of current chunk
		i32	GetChunkVersion() {return mChunkVersion;}		//!< CALL: Lists fVersion in chunk header for convenience.  
		b8	IsEmpty();				//!< CALL: Возвращает true, если no data is in the chunk
		z0	Clear();				//!< Resets chunk
		//@{
		/*!	\name Internal Methods
		 *  An Effect Layer plugin can ignore these methods. They are handled by or used internally by the Effect Layer.
		 */
		z0	LoadChunk(const AAX_SPlugInChunk *chunk);	//!< Sets current chunk to data in \a chunk parameter

	protected:	
		z0	WordAlign(u32 &index);			//!< sets \a index to 4-byte boundary
		z0	WordAlign(i32 &index);			//!< sets \a index to 4-byte boundary
		i32	FindName(const AAX_CString &Name);	//!< used by public Find methods
		//@}	END Internal Methods group
		/*!	\brief	The last index found in the chunk
		 *	
		 *	\details
		 *	Since control values in chunks should tend to stay in order and in sync with
		 *	the way they're checked with controls within the plug-in, we'll keep track of
		 *	the value index to speed up searching.
		 */
		i32	mLastFoundIndex;

		t8	*mChunkData;
		
		i32 mChunkVersion;						//!< Equal to fVersion from the chunk header.  Equal to -1 if no chunk is loaded.
public:
		struct DataValue
		{
			i32		mDataType;
			AAX_CString	mDataName;		//!< name of the stored data
			z64		mIntValue;		//!< used if this DataValue is not a string
			AAX_CString	mStringValue;	//!< used if this DataValue is a string
			
			DataValue():
				mDataType(0),
				mDataName(AAX_CString()),
				mIntValue(0),
				mStringValue(AAX_CString())
			{};
		};
	
		std::vector<DataValue>	mDataValues;
};

#endif  //AAX_CHUNKDATAPARSER_H
