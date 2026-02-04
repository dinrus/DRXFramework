//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST3 SDK
// Filename    : public.sdk/source/vst/utility/vst2persistence.cpp
// Created by  : Steinberg, 12/2019
// Description : vst2 persistence helper
//
//------------------------------------------------------------------------
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

#include "public.sdk/source/vst/utility/vst2persistence.h"
#include "pluginterfaces/base/fplatform.h"
#include <limits>

//------------------------------------------------------------------------
namespace VST3 {
namespace {
namespace IO {

//------------------------------------------------------------------------
enum class Error
{
	NoError,
	Unknown,
	EndOfFile,
	BufferToBig,
	NotAllowed,
	InvalidArgument,
};

//------------------------------------------------------------------------
enum class SeekMode
{
	Set,
	End,
	Current
};

//------------------------------------------------------------------------
struct Result
{
	Error error {Error::Unknown};
	zu64 bytes {0u};

	Result () noexcept = default;
	Result (Error error, zu64 bytes = 0) noexcept : error (error), bytes (bytes) {}

	operator b8 () const noexcept { return error == Error::NoError; }
};

//------------------------------------------------------------------------
struct ReadBufferDesc
{
	const zu64 bytes;
	uk ptr;
};

//------------------------------------------------------------------------
struct WriteBufferDesc
{
	const zu64 bytes;
	ukk ptr;
};

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
class ByteOrderStream
{
public:
//------------------------------------------------------------------------
	ByteOrderStream (Steinberg::IBStream& stream) noexcept : stream (stream) {}
	ByteOrderStream (ByteOrderStream&&) noexcept = delete;
	ByteOrderStream& operator= (ByteOrderStream&&) noexcept = delete;
	ByteOrderStream (const ByteOrderStream&) noexcept = delete;
	ByteOrderStream& operator= (const ByteOrderStream&) noexcept = delete;

	inline Result operator<< (const std::string& input) noexcept;
	inline Result operator>> (std::string& output) noexcept;

	template <typename T>
	inline Result operator<< (const T& input) noexcept;
	template <typename T>
	inline Result operator>> (T& output) const noexcept;

	inline Result read (const ReadBufferDesc& buffer) const noexcept;
	inline Result write (const WriteBufferDesc& buffer) noexcept;
	inline Result seek (SeekMode mode, z64 bytes) const noexcept;
	inline Result tell () const noexcept;

//------------------------------------------------------------------------
private:
	template <size_t size>
	inline Result swapAndWrite (u8k* buffer) noexcept;
	inline z0 swap (u8* buffer, zu64 size) const noexcept;
	Steinberg::IBStream& stream;
};

//------------------------------------------------------------------------
using LittleEndianStream = ByteOrderStream<kLittleEndian>;
using BigEndianStream = ByteOrderStream<kBigEndian>;
using NativeEndianStream = ByteOrderStream<BYTEORDER>;

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::read (const ReadBufferDesc& buffer) const noexcept
{
	if (buffer.bytes > static_cast<zu64> (std::numeric_limits<i32>::max ()))
		return Result (Error::BufferToBig);
	Steinberg::i32 readBytes = 0;
	auto tres = stream.read (buffer.ptr, static_cast<Steinberg::i32> (buffer.bytes), &readBytes);
	if (tres != Steinberg::kResultTrue)
		return Result (Error::Unknown);
	assert (readBytes >= 0);
	return Result {Error::NoError, static_cast<zu64> (readBytes)};
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::write (const WriteBufferDesc& buffer) noexcept
{
	if (buffer.bytes > static_cast<zu64> (std::numeric_limits<i32>::max ()))
		return Result (Error::BufferToBig);
	Steinberg::i32 writtenBytes = 0;
	auto tres = stream.write (const_cast<uk> (buffer.ptr),
	                          static_cast<Steinberg::i32> (buffer.bytes), &writtenBytes);
	if (tres != Steinberg::kResultTrue)
		return Result (Error::Unknown);
	assert (writtenBytes >= 0);
	return Result {Error::NoError, static_cast<zu64> (writtenBytes)};
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::seek (SeekMode mode, z64 bytes) const noexcept
{
	Steinberg::i32 seekMode = 0;
	switch (mode)
	{
		case SeekMode::Set: seekMode = Steinberg::IBStream::kIBSeekSet; break;
		case SeekMode::Current: seekMode = Steinberg::IBStream::kIBSeekCur; break;
		case SeekMode::End: seekMode = Steinberg::IBStream::kIBSeekEnd; break;
	}
	Steinberg::z64 seekRes = 0;
	auto tres = stream.seek (static_cast<Steinberg::z64> (bytes), seekMode, &seekRes);
	if (tres != Steinberg::kResultTrue || seekRes < 0)
		return Result {Error::Unknown};
	return Result (Error::NoError, static_cast<zu64> (seekRes));
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::tell () const noexcept
{
	Steinberg::z64 tellRes = 0;
	auto tres = stream.tell (&tellRes);
	if (tres != Steinberg::kResultTrue || tellRes < 0)
		return Result {Error::Unknown};
	return Result {Error::NoError, static_cast<zu64> (tellRes)};
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::operator<< (const std::string& input) noexcept
{
	auto res = *this << static_cast<zu64> (input.length ());
	if (!res)
		return res;
	res = stream.write (const_cast<uk> (static_cast<ukk> (input.data ())),
	                    static_cast<Steinberg::i32> (input.length ()));
	res.bytes += sizeof (zu64);
	return res;
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::operator>> (std::string& output) noexcept
{
	zu64 length;
	auto res = *this >> length;
	if (!res)
		return res;
	output.resize (length);
	if (length > 0)
	{
		res = stream.read (&output.front (), static_cast<Steinberg::i32> (length));
		res.bytes += sizeof (zu64);
	}
	return res;
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
template <typename T>
inline Result ByteOrderStream<StreamByteOrder>::operator<< (const T& input) noexcept
{
	static_assert (std::is_standard_layout<T>::value, "Only standard layout types allowed");
	// with C++17: if constexpr (StreamByteOrder == BYTEORDER)
	if (constexpr b8 tmp = (StreamByteOrder == BYTEORDER))
		return write (WriteBufferDesc {sizeof (T), static_cast<ukk> (&input)});

	return swapAndWrite<sizeof (T)> (reinterpret_cast<u8k*> (&input));
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
template <typename T>
inline Result ByteOrderStream<StreamByteOrder>::operator>> (T& output) const noexcept
{
	static_assert (std::is_standard_layout<T>::value, "Only standard layout types allowed");
	auto res = read (ReadBufferDesc {sizeof (T), &output});
	// with C++17: if constexpr (StreamByteOrder == BYTEORDER)
	if (constexpr b8 tmp = (StreamByteOrder == BYTEORDER))
		return res;

	swap (reinterpret_cast<u8*> (&output), res.bytes);
	return res;
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
template <size_t _size>
inline Result ByteOrderStream<StreamByteOrder>::swapAndWrite (u8k* buffer) noexcept
{
	// with C++17: if constexpr (_size > 1)
	if (constexpr b8 tmp2 = (_size > 1))
	{
		i8 tmp[_size];

		constexpr auto halfSize = _size / 2;
		auto size = _size;
		auto low = buffer;
		auto high = buffer + size - 1;

		while (size > halfSize)
		{
			tmp[size - 2] = buffer[(_size - size) + 1];
			tmp[(_size - size) + 1] = buffer[size - 2];
			tmp[_size - size] = *high;
			tmp[size - 1] = *low;
			low += 2;
			high -= 2;
			size -= 2;
		}
		return write (WriteBufferDesc {_size, tmp});
	}
	return write (WriteBufferDesc {1, buffer});
}

//------------------------------------------------------------------------
template <u32 StreamByteOrder>
inline z0 ByteOrderStream<StreamByteOrder>::swap (u8* buffer, zu64 size) const noexcept
{
	if (size < 2)
		return;
	auto low = buffer;
	auto high = buffer + size - 1;
	while (size >= 2)
	{
		auto tmp = *low;
		*low = *high;
		*high = tmp;
		low += 2;
		high -= 2;
		size -= 2;
	}
}

//------------------------------------------------------------------------
} // IO

//------------------------------------------------------------------------
constexpr i32 cMagic = 'CcnK';
constexpr i32 bankMagic = 'FxBk';
constexpr i32 privateChunkID = 'VstW';
constexpr i32 chunkBankMagic = 'FBCh';
constexpr i32 programMagic = 'FxCk';
constexpr i32 chunkProgramMagic = 'FPCh';

//------------------------------------------------------------------------
Optional<VST3::Vst2xProgram> loadProgram (const IO::BigEndianStream& state,
                                          const Optional<i32>& vst2xUniqueID)
{
	Vst2xProgram program;
	i32 id;
	if (!(state >> id))
		return {};
	if (id != cMagic)
		return {};
	i32 bankSize;
	if (!(state >> bankSize))
		return {};
	i32 fxMagic;
	if (!(state >> fxMagic))
		return {};
	if (!(fxMagic == programMagic || fxMagic == chunkProgramMagic))
		return {};
	i32 formatVersion;
	if (!(state >> formatVersion))
		return {};
	i32 fxId;
	if (!(state >> fxId))
		return {};
	if (vst2xUniqueID && fxId != *vst2xUniqueID)
		return {};
	i32 fxVersion;
	if (!(state >> fxVersion))
		return {};
	i32 numParams;
	if (!(state >> numParams))
		return {};
	if (numParams < 0)
		return {};
	t8 name[29];
	if (!state.read ({28, name}))
		return {};
	name[28] = 0;
	program.name = name;
	program.fxUniqueID = fxId;
	program.fxVersion = fxVersion;
	if (fxMagic == chunkProgramMagic)
	{
		u32 chunkSize;
		if (!(state >> chunkSize))
			return {};
		program.chunk.resize (chunkSize);
		if (!state.read ({chunkSize, program.chunk.data ()}))
			return {};
	}
	else
	{
		program.values.resize (numParams);
		f32 paramValue;
		for (i32 i = 0; i < numParams; ++i)
		{
			if (!(state >> paramValue))
				return {};
			program.values[i] = paramValue;
		}
	}
	return {std::move (program)};
}

//------------------------------------------------------------------------
b8 loadPrograms (Steinberg::IBStream& stream, Vst2xState::Programs& programs,
                   const Optional<i32>& vst2xUniqueID)
{
	IO::BigEndianStream state (stream);

	for (auto& program : programs)
	{
		if (auto prg = loadProgram (state, vst2xUniqueID))
			std::swap (program, *prg);
		else
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
template <typename SizeT, typename StreamT, typename Proc>
IO::Error streamSizeWriter (StreamT& stream, Proc proc)
{
	auto startPos = stream.tell ();
	if (startPos.error != IO::Error::NoError)
		return startPos.error;
	auto res = stream << static_cast<SizeT> (0); // placeholder
	if (!res)
		return res.error;
	auto procRes = proc ();
	if (procRes != IO::Error::NoError)
		return procRes;
	auto endPos = stream.tell ();
	if (endPos.error != IO::Error::NoError)
		return endPos.error;
	auto size = (endPos.bytes - startPos.bytes) - 4;
	auto typeSize = static_cast<SizeT> (size);
	if (size != static_cast<zu64> (typeSize))
		return IO::Error::Unknown;
	res = stream.seek (IO::SeekMode::Set, startPos.bytes);
	if (!res)
		return res.error;
	res = (stream << typeSize);
	if (!res)
		return res.error;
	res = stream.seek (IO::SeekMode::Set, endPos.bytes);
	return res.error;
}

//------------------------------------------------------------------------
template <typename StreamT>
IO::Error writePrograms (StreamT& stream, const Vst2xState::Programs& programs)
{
	for (const auto& program : programs)
	{
		auto res = stream << cMagic;
		if (!res)
			return res.error;
		res = streamSizeWriter<i32> (stream, [&] () {
			b8 writeChunk = !program.chunk.empty ();
			if (!(res = stream << (writeChunk ? chunkProgramMagic : programMagic)))
				return res.error;
			i32 version = 1;
			if (!(res = stream << version))
				return res.error;
			if (!(res = stream << program.fxUniqueID))
				return res.error;
			i32 fxVersion = program.fxVersion;
			if (!(res = stream << fxVersion))
				return res.error;
			u32 numParams = static_cast<u32> (program.values.size ());
			if (!(res = stream << numParams))
				return res.error;
			auto programName = program.name;
			programName.resize (28);
			for (auto c : programName)
			{
				if (!(res = stream << c))
					return res.error;
			}
			if (writeChunk)
			{
				if (!(res = stream << static_cast<i32> (program.chunk.size ())))
					return res.error;
				if (!(res = stream.write ({program.chunk.size (), program.chunk.data ()})))
					return res.error;
			}
			else
			{
				for (auto value : program.values)
				{
					if (!(res = stream << value))
						return res.error;
				}
			}
			return IO::Error::NoError;
		});
		if (res.error != IO::Error::NoError)
			return res.error;
	}
	return IO::Error::NoError;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Optional<Vst2xState> tryVst2StateLoad (Steinberg::IBStream& stream,
                                       Optional<i32> vst2xUniqueID) noexcept
{
	Vst2xState result;

	IO::BigEndianStream state (stream);
	i32 version;
	i32 size;
	i32 id;
	if (!(state >> id))
		return {};
	if (id == privateChunkID)
	{
		if (!(state >> size))
			return {};
		if (!(state >> version))
			return {};
		i32 bypass;
		if (!(state >> bypass))
			return {};
		result.isBypassed = bypass ? true : false;
		if (!(state >> id))
			return {};
	}
	if (id != cMagic)
		return {};
	i32 bankSize;
	if (!(state >> bankSize))
		return {};
	i32 fxMagic;
	if (!(state >> fxMagic))
		return {};
	if (!(fxMagic == bankMagic || fxMagic == chunkBankMagic))
		return {};
	i32 bankVersion;
	if (!(state >> bankVersion))
		return {};
	i32 fxId;
	if (!(state >> fxId))
		return {};
	if (vst2xUniqueID && fxId != *vst2xUniqueID)
		return {};
	result.fxUniqueID = fxId;
	i32 fxVersion;
	if (!(state >> fxVersion))
		return {};
	result.fxVersion = fxVersion;

	i32 numPrograms;
	if (!(state >> numPrograms))
		return {};
	if (numPrograms < 1 && fxMagic == bankMagic)
		return {};

	i32 currentProgram = 0;
	if (bankVersion >= 1)
	{
		if (!(state >> currentProgram))
			return {};
		state.seek (IO::SeekMode::Current, 124); // future
	}
	result.currentProgram = currentProgram;
	if (fxMagic == bankMagic)
	{
		result.programs.resize (numPrograms);
		if (!loadPrograms (stream, result.programs, vst2xUniqueID))
			return {};
		assert (static_cast<i32> (result.programs.size ()) > currentProgram);
	}
	else
	{
		u32 chunkSize;
		if (!(state >> chunkSize))
			return {};
		if (chunkSize == 0)
			return {};
		result.chunk.resize (chunkSize);
		if (!state.read ({chunkSize, result.chunk.data ()}))
			return {};
	}
	return {std::move (result)};
}

//------------------------------------------------------------------------
b8 writeVst2State (const Vst2xState& state, Steinberg::IBStream& _stream,
                     b8 writeBypassState) noexcept
{
	IO::BigEndianStream stream (_stream);
	if (writeBypassState)
	{
		if (!(stream << privateChunkID))
			return false;
		if (streamSizeWriter<u32> (stream, [&] () {
			    u32 version = 1;
			    auto res = (stream << version);
			    if (!res)
				    return res.error;
			    i32 bypass = state.isBypassed ? 1 : 0;
			    return (stream << bypass).error;
		    }) != IO::Error::NoError)
		{
			return false;
		}
	}
	if (!(stream << cMagic))
	{
		return false;
	}
	if (streamSizeWriter<i32> (stream, [&] () {
		    b8 writeChunk = !state.chunk.empty ();
		    IO::Result res;
		    if (!(res = (stream << (writeChunk ? chunkBankMagic : bankMagic))))
			    return res.error;
		    i32 bankVersion = 2;
		    if (!(res = (stream << bankVersion)))
			    return res.error;
		    if (!(res = (stream << state.fxUniqueID)))
			    return res.error;
		    if (!(res = (stream << state.fxVersion)))
			    return res.error;
		    i32 numPrograms = writeChunk ? 1 : static_cast<i32> (state.programs.size ());
		    if (!(res = (stream << numPrograms)))
			    return res.error;
		    if (bankVersion > 1)
		    {
			    if (!(res = (stream << state.currentProgram)))
				    return res.error;
			    // write 124 zero bytes
			    u8 byte = 0;
			    for (u32 i = 0; i < 124; ++i)
				    if (!(res = (stream << byte)))
					    return res.error;
		    }
		    if (writeChunk)
		    {
			    auto chunkSize = static_cast<u32> (state.chunk.size ());
			    if (!(res = (stream << chunkSize)))
				    return res.error;
			    stream.write ({state.chunk.size (), state.chunk.data ()});
		    }
		    else
		    {
			    writePrograms (stream, state.programs);
		    }
		    return IO::Error::NoError;
	    }) != IO::Error::NoError)
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------
Optional<Vst2xProgram> tryVst2ProgramLoad (Steinberg::IBStream& stream,
                                           Optional<i32> vst2xUniqueID) noexcept
{
	IO::BigEndianStream state (stream);
	return loadProgram (state, vst2xUniqueID);
}

//------------------------------------------------------------------------
} // VST3
