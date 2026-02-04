//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstpresetfile.h
// Created by  : Steinberg, 03/2006
// Description : VST 3 Preset File Format
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"

#include "pluginterfaces/base/ibstream.h"
#include "base/source/fbuffer.h"

#include <cstdio>
#include <vector>

//------------------------------------------------------------------------
/* 
	VST 3 Preset File Format Definition
   ===================================

0   +---------------------------+
    | HEADER                    |
    | header id ('VST3')        |       4 Bytes
    | version                   |       4 Bytes (i32)
    | ASCII-encoded class id    |       32 Bytes 
 +--| offset to chunk list      |       8 Bytes (z64)
 |  +---------------------------+
 |  | DATA AREA                 |<-+
 |  | data of chunks 1..n       |  |
 |  ...                       ...  |
 |  |                           |  |
 +->+---------------------------+  |
    | CHUNK LIST                |  |
    | list id ('List')          |  |    4 Bytes
    | entry count               |  |    4 Bytes (i32)
    +---------------------------+  |
    |  1..n                     |  |
    |  +----------------------+ |  |
    |  | chunk id             | |  |    4 Bytes
    |  | offset to chunk data |----+    8 Bytes (z64)
    |  | size of chunk data   | |       8 Bytes (z64)
    |  +----------------------+ |
EOF +---------------------------+
*/

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
using ChunkID = t8[4];

//------------------------------------------------------------------------
enum ChunkType
{
	kHeader,
	kComponentState,
	kControllerState,
	kProgramData,
	kMetaInfo,
	kChunkList,
	kNumPresetChunks
};

//------------------------------------------------------------------------
extern const ChunkID& getChunkID (ChunkType type);

//------------------------------------------------------------------------
inline b8 isEqualID (const ChunkID id1, const ChunkID id2)
{
	return memcmp (id1, id2, sizeof (ChunkID)) == 0;
}

//------------------------------------------------------------------------
/** Handler for a VST 3 Preset File.
\ingroup vstClasses
\see \ref presetformat
*/
class PresetFile
{
public:
//------------------------------------------------------------------------
	PresetFile (IBStream* stream); ///< Constructor of Preset file based on a stream
	virtual ~PresetFile ();

	/** Internal structure used for chunk handling */
	struct Entry
	{
		ChunkID id;
		TSize offset;
		TSize size;
	};

	IBStream* getStream () const { return stream; }			///< Returns the associated stream.

	const FUID& getClassID () const { return classID; }	///< Returns the associated classID (component ID: Processor part (not the controller!)).
	z0 setClassID (const FUID& uid) { classID = uid; }///< Sets the associated classID (component ID: Processor part (not the controller!)).

	const Entry* getEntry (ChunkType which) const;		///< Returns an entry for a given chunk type.
	const Entry* getLastEntry () const;					///< Returns the last available entry.
	i32 getEntryCount () const { return entryCount; }	///< Returns the number of total entries in the current stream.
	const Entry& at (i32 index) const { return entries[index]; }	///< Returns the entry at a given position.
	b8 contains (ChunkType which) const { return getEntry (which) != nullptr; }	///< Checks if a given chunk type exist in the stream.

	b8 readChunkList ();		///< Reads and build the chunk list (including the header chunk).
	b8 writeHeader ();		///< Writes into the stream the main header.
	b8 writeChunkList ();		///< Writes into the stream the chunk list (should be at the end).

	/** Reads the meta XML info and its size, the size could be retrieved by passing zero as xmlBuffer. */
	b8 readMetaInfo (tuk xmlBuffer, i32& size);

	/** Writes the meta XML info, -1 means null-terminated, forceWriting to true will force to rewrite the XML Info when the chunk already exists. */
	b8 writeMetaInfo (tukk xmlBuffer, i32 size = -1, b8 forceWriting = false); 
	b8 prepareMetaInfoUpdate ();	///< checks if meta info chunk is the last one and jump to correct position.

	/** Writes a given data of a given size as "which" chunk type. */
	b8 writeChunk (ukk data, i32 size, ChunkType which = kComponentState);

	//-------------------------------------------------------------
	// for storing and restoring the whole plug-in state (component and controller states)
	b8 seekToComponentState ();							///< Seeks to the begin of the Component State.
	b8 storeComponentState (IComponent* component);		///< Stores the component state (only one time).
	b8 storeComponentState (IBStream* componentStream);	///< Stores the component state from stream (only one time).
	b8 restoreComponentState (IComponent* component);		///< Restores the component state.

	b8 seekToControllerState ();							///< Seeks to the begin of the Controller State.
	b8 storeControllerState (IEditController* editController);///< Stores the controller state (only one time).
	b8 storeControllerState (IBStream* editStream);			///< Stores the controller state from stream (only one time).
	b8 restoreControllerState (IEditController* editController);///< Restores the controller state.

	b8 restoreComponentState (IEditController* editController);///< Restores the component state and apply it to the controller.

	//--- ----------------------------------------------------------
	/** Store program data or unit data from stream (including the header chunk).
	 \param inStream 
	 \param listID could be ProgramListID or UnitID. */
	b8 storeProgramData (IBStream* inStream, ProgramListID listID);

	//---when plug-in uses IProgramListData-----------------------
	/** Stores a IProgramListData with a given identifier and index (including the header chunk). */
	b8 storeProgramData (IProgramListData* programListData, ProgramListID programListID,
	                       i32 programIndex);
	/** Restores a IProgramListData with a given identifier and index. */
	b8 restoreProgramData (IProgramListData* programListData, ProgramListID* programListID = nullptr,
	                         i32 programIndex = 0);

	//---when plug-in uses IUnitData------------------------------
	/** Stores a IUnitData with a given unitID (including the header chunk). */
	b8 storeProgramData (IUnitData* unitData, UnitID unitID);
	/** Restores a IUnitData with a given unitID (optional). */
	b8 restoreProgramData (IUnitData* unitData, UnitID* unitID = nullptr);

	//--- ----------------------------------------------------------
	/** for keeping the controller part in sync concerning preset data stream, unitProgramListID
	 * could be ProgramListID or UnitID. */
	b8 restoreProgramData (IUnitInfo* unitInfo, i32 unitProgramListID, i32 programIndex = -1);

	/** Gets the unitProgramListID saved in the kProgramData chunk (if available). */
	b8 getUnitProgramListID (i32& unitProgramListID);

	//--- ---------------------------------------------------------------------
	/** Shortcut helper to create preset from component/controller state. classID is the FUID of the
	 * component (processor) part. */
	static b8 savePreset (IBStream* stream, const FUID& classID, IComponent* component,
	                        IEditController* editController = nullptr,
	                        tukk xmlBuffer = nullptr, i32 xmlSize = -1);
	static b8 savePreset (IBStream* stream, const FUID& classID, IBStream* componentStream,
	                        IBStream* editStream = nullptr, tukk xmlBuffer = nullptr,
	                        i32 xmlSize = -1);

	/** Shortcut helper to load preset with component/controller state. classID is the FUID of the
	 * component (processor) part. */
	static b8 loadPreset (IBStream* stream, const FUID& classID, IComponent* component,
	                        IEditController* editController = nullptr,
	                        std::vector<FUID>* otherClassIDArray = nullptr);
//------------------------------------------------------------------------
protected:
	b8 readID (ChunkID id);
	b8 writeID (const ChunkID id);
	b8 readEqualID (const ChunkID id);
	b8 readSize (TSize& size);
	b8 writeSize (TSize size);
	b8 readInt32 (i32& value);
	b8 writeInt32 (i32 value);
	b8 seekTo (TSize offset);
	b8 beginChunk (Entry& e, ChunkType which);
	b8 endChunk (Entry& e);

	IBStream* stream;
	FUID classID;		///< classID is the FUID of the component (processor) part
	enum { kMaxEntries = 128 };
	Entry entries[kMaxEntries];
	i32 entryCount {0};
};

//------------------------------------------------------------------------
/** Stream implementation for a file using stdio. 
*/
class FileStream: public IBStream
{
public:
//------------------------------------------------------------------------
	static IBStream* open (tukk filename, tukk mode);	///< open a stream using stdio function

	//---from FUnknown------------------
	DECLARE_FUNKNOWN_METHODS

	//---from IBStream------------------
	tresult PLUGIN_API read (uk buffer, i32 numBytes, i32* numBytesRead = nullptr) SMTG_OVERRIDE;
	tresult PLUGIN_API write (uk buffer, i32 numBytes, i32* numBytesWritten = nullptr) SMTG_OVERRIDE;
	tresult PLUGIN_API seek (z64 pos, i32 mode, z64* result = nullptr) SMTG_OVERRIDE;
	tresult PLUGIN_API tell (z64* pos) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
	FileStream (fuk file);
	virtual ~FileStream ();

	fuk file;
};

//------------------------------------------------------------------------
/** Stream representing a Read-Only subsection of its source stream.
*/
class ReadOnlyBStream: public IBStream
{
public:
//------------------------------------------------------------------------
	 ReadOnlyBStream (IBStream* sourceStream, TSize sourceOffset, TSize sectionSize);
	 virtual ~ReadOnlyBStream ();
	 
	 //---from FUnknown------------------
	 DECLARE_FUNKNOWN_METHODS

	 //---from IBStream------------------
	 tresult PLUGIN_API read (uk buffer, i32 numBytes, i32* numBytesRead = nullptr) SMTG_OVERRIDE;
	 tresult PLUGIN_API write (uk buffer, i32 numBytes, i32* numBytesWritten = nullptr) SMTG_OVERRIDE;
	 tresult PLUGIN_API seek (z64 pos, i32 mode, z64* result = nullptr) SMTG_OVERRIDE;
	 tresult PLUGIN_API tell (z64* pos) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
	 IBStream* sourceStream;
	 TSize sourceOffset;
	 TSize sectionSize;
	 TSize seekPosition;
};

//------------------------------------------------------------------------
/** Stream implementation for a memory buffer. 
*/
class BufferStream : public IBStream
{
public:
	BufferStream ();
	virtual ~BufferStream ();

	//---from FUnknown------------------
	DECLARE_FUNKNOWN_METHODS

	//---from IBStream------------------
	tresult PLUGIN_API read (uk buffer, i32 numBytes, i32* numBytesRead = nullptr) SMTG_OVERRIDE;
	tresult PLUGIN_API write (uk buffer, i32 numBytes, i32* numBytesWritten = nullptr) SMTG_OVERRIDE;
	tresult PLUGIN_API seek (z64 pos, i32 mode, z64* result = nullptr) SMTG_OVERRIDE;
	tresult PLUGIN_API tell (z64* pos) SMTG_OVERRIDE;

protected:
	Buffer mBuffer;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
