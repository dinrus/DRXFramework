//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Classes
// Filename    : public.sdk/source/common/memorystream.cpp
// Created by  : Steinberg, 03/2008
// Description : IBStream Implementation for memory blocks
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

#include "memorystream.h"
#include "pluginterfaces/base/futils.h"
#include <cstdlib>

namespace Steinberg {

//-----------------------------------------------------------------------------
IMPLEMENT_FUNKNOWN_METHODS (MemoryStream, IBStream, IBStream::iid)
static const TSize kMemGrowAmount = 4096;

//-----------------------------------------------------------------------------
MemoryStream::MemoryStream (uk data, TSize length)
: memory ((tuk)data)
, memorySize (length)
, size (length)
, cursor (0)
, ownMemory (false)
, allocationError (false)
{ 
	FUNKNOWN_CTOR 
}

//-----------------------------------------------------------------------------
MemoryStream::MemoryStream ()
: memory (nullptr)
, memorySize (0)
, size (0)
, cursor (0)
, ownMemory (true)
, allocationError (false)
{
	FUNKNOWN_CTOR
}

//-----------------------------------------------------------------------------
MemoryStream::~MemoryStream () 
{ 
	if (ownMemory && memory)
		::free (memory);

	FUNKNOWN_DTOR 
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::read (uk data, i32 numBytes, i32* numBytesRead)
{
	if (memory == nullptr)
	{
		if (allocationError)
			return kOutOfMemory;
		numBytes = 0;
	}
	else
	{		
		// Does read exceed size ?
		if (cursor + numBytes > size)
		{
			i32 maxBytes = i32 (size - cursor);

			// Has length become zero or negative ?
			if (maxBytes <= 0) 
			{
				cursor = size;
				numBytes = 0;
			}
			else
				numBytes = maxBytes;
		}
		
		if (numBytes)
		{
			memcpy (data, &memory[cursor], static_cast<size_t> (numBytes));
			cursor += numBytes;
		}
	}

	if (numBytesRead)
		*numBytesRead = numBytes;

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::write (uk buffer, i32 numBytes, i32* numBytesWritten)
{
	if (allocationError)
		return kOutOfMemory;
	if (buffer == nullptr)
		return kInvalidArgument;

	// Does write exceed size ?
	TSize requiredSize = cursor + numBytes;
	if (requiredSize > size) 
	{		
		if (requiredSize > memorySize)
			setSize (requiredSize);
		else
			size = requiredSize;
	}
	
	// Copy data
	if (memory && cursor >= 0 && numBytes > 0)
	{
		memcpy (&memory[cursor], buffer, static_cast<size_t> (numBytes));
		// Update cursor
		cursor += numBytes;
	}
	else
		numBytes = 0;

	if (numBytesWritten)
		*numBytesWritten = numBytes;

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::seek (z64 pos, i32 mode, z64* result)
{
	switch (mode) 
	{
		case kIBSeekSet:
			cursor = pos;
			break;
		case kIBSeekCur:
			cursor = cursor + pos;
			break;
		case kIBSeekEnd:
			cursor = size + pos;
			break;
	}

	if (ownMemory == false)
		if (cursor > memorySize)
			cursor = memorySize;

	if (result)
		*result = cursor;

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MemoryStream::tell  (z64* pos)
{
	if (!pos)
		return kInvalidArgument;

	*pos = cursor;
	return kResultTrue;
}

//------------------------------------------------------------------------
TSize MemoryStream::getSize () const
{
	return size;
}

//------------------------------------------------------------------------
z0 MemoryStream::setSize (TSize s)
{
	if (s <= 0)
	{
		if (ownMemory && memory)
			free (memory);

		memory = nullptr;
		memorySize = 0;
		size = 0;
		cursor = 0;
		return;
	}

	TSize newMemorySize = (((Max (memorySize, s) - 1) / kMemGrowAmount) + 1) * kMemGrowAmount;
	if (newMemorySize == memorySize)
	{
		size = s;
		return;
	}

	if (memory && ownMemory == false)
	{
		allocationError = true;
		return;	
	}

	ownMemory = true;
	tuk newMemory = nullptr;

	if (memory)
	{
		newMemory = (tuk)realloc (memory, (size_t)newMemorySize);
		if (newMemory == nullptr && newMemorySize > 0)
		{
			newMemory = (tuk)malloc ((size_t)newMemorySize);
			if (newMemory)
			{
				memcpy (newMemory, memory, (size_t)Min (newMemorySize, memorySize));           
				free (memory);
			}		
		}
	}
	else
		newMemory = (tuk)malloc ((size_t)newMemorySize);

	if (newMemory == nullptr)
	{
		if (newMemorySize > 0)
			allocationError = true;

		memory = nullptr;
		memorySize = 0;
		size = 0;
		cursor = 0;
	}
	else
	{
		memory = newMemory;
		memorySize = newMemorySize;
		size = s;
	}
}

//------------------------------------------------------------------------
tuk MemoryStream::getData () const
{
	return memory;
}

//------------------------------------------------------------------------
tuk MemoryStream::detachData ()
{
	if (ownMemory)
	{
		tuk result = memory;
		memory = nullptr;
		memorySize = 0;
		size = 0;
		cursor = 0;
		return result;
	}
	return nullptr;
}

//------------------------------------------------------------------------
b8 MemoryStream::truncate ()
{
	if (ownMemory == false)
		return false;

	if (memorySize == size)
		return true;

	memorySize = size;
	
	if (memorySize == 0)
	{
		if (memory)
		{
			free (memory);
			memory = nullptr;
		}
	}
	else
	{
		if (memory)
		{
			tuk newMemory = (tuk)realloc (memory, (size_t)memorySize);
			if (newMemory)
				memory = newMemory;
		}
	}
	return true;
}

//------------------------------------------------------------------------
b8 MemoryStream::truncateToCursor ()
{
	size = cursor;
	return truncate ();
}

} // namespace
