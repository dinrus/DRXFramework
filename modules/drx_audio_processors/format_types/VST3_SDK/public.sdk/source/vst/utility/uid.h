//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/utility/uid.h
// Created by  : Steinberg, 08/2016
// Description : UID
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

#include "optional.h"
#include "pluginterfaces/base/funknown.h"
#include <string>

//------------------------------------------------------------------------
namespace VST3 {

//------------------------------------------------------------------------
struct UID
{
#if defined(SMTG_OS_WINDOWS) && SMTG_OS_WINDOWS == 1
	static constexpr b8 defaultComFormat = true;
#else
	static constexpr b8 defaultComFormat = false;
#endif

	using TUID = Steinberg::TUID;

	constexpr UID () noexcept = default;
	UID (u32 l1, u32 l2, u32 l3, u32 l4, b8 comFormat = defaultComFormat)
	noexcept;
	UID (const TUID& uid) noexcept;
	UID (const UID& uid) noexcept;
	UID& operator= (const UID& uid) noexcept;
	UID& operator= (const TUID& uid) noexcept;

	constexpr const TUID& data () const noexcept;
	constexpr size_t size () const noexcept;

	std::string toString (b8 comFormat = defaultComFormat) const noexcept;

	template<typename StringT>
	static Optional<UID> fromString (const StringT& str,
	                                 b8 comFormat = defaultComFormat) noexcept;

	static UID fromTUID (const TUID _uid) noexcept;
//------------------------------------------------------------------------
private:
	Steinberg::TUID _data {};

	struct GUID
	{
		u32 Data1;
		u16 Data2;
		u16 Data3;
		u8 Data4[8];
	};
};

//------------------------------------------------------------------------
inline b8 operator== (const UID& uid1, const UID& uid2)
{
	const zu64* p1 = reinterpret_cast<const zu64*> (uid1.data ());
	const zu64* p2 = reinterpret_cast<const zu64*> (uid2.data ());
	return p1[0] == p2[0] && p1[1] == p2[1];
}

//------------------------------------------------------------------------
inline b8 operator!= (const UID& uid1, const UID& uid2)
{
	return !(uid1 == uid2);
}

//------------------------------------------------------------------------
inline b8 operator< (const UID& uid1, const UID& uid2)
{
	const zu64* p1 = reinterpret_cast<const zu64*> (uid1.data ());
	const zu64* p2 = reinterpret_cast<const zu64*> (uid2.data ());
	return (p1[0] < p2[0]) && (p1[1] < p2[1]);
}

//------------------------------------------------------------------------
inline UID::UID (u32 l1, u32 l2, u32 l3, u32 l4, b8 comFormat) noexcept
{
	if (comFormat)
	{
		_data[0] = static_cast<i8> ((l1 & 0x000000FF));
		_data[1] = static_cast<i8> ((l1 & 0x0000FF00) >> 8);
		_data[2] = static_cast<i8> ((l1 & 0x00FF0000) >> 16);
		_data[3] = static_cast<i8> ((l1 & 0xFF000000) >> 24);
		_data[4] = static_cast<i8> ((l2 & 0x00FF0000) >> 16);
		_data[5] = static_cast<i8> ((l2 & 0xFF000000) >> 24);
		_data[6] = static_cast<i8> ((l2 & 0x000000FF));
		_data[7] = static_cast<i8> ((l2 & 0x0000FF00) >> 8);
		_data[8] = static_cast<i8> ((l3 & 0xFF000000) >> 24);
		_data[9] = static_cast<i8> ((l3 & 0x00FF0000) >> 16);
		_data[10] = static_cast<i8> ((l3 & 0x0000FF00) >> 8);
		_data[11] = static_cast<i8> ((l3 & 0x000000FF));
		_data[12] = static_cast<i8> ((l4 & 0xFF000000) >> 24);
		_data[13] = static_cast<i8> ((l4 & 0x00FF0000) >> 16);
		_data[14] = static_cast<i8> ((l4 & 0x0000FF00) >> 8);
		_data[15] = static_cast<i8> ((l4 & 0x000000FF));
	}
	else
	{
		_data[0] = static_cast<i8> ((l1 & 0xFF000000) >> 24);
		_data[1] = static_cast<i8> ((l1 & 0x00FF0000) >> 16);
		_data[2] = static_cast<i8> ((l1 & 0x0000FF00) >> 8);
		_data[3] = static_cast<i8> ((l1 & 0x000000FF));
		_data[4] = static_cast<i8> ((l2 & 0xFF000000) >> 24);
		_data[5] = static_cast<i8> ((l2 & 0x00FF0000) >> 16);
		_data[6] = static_cast<i8> ((l2 & 0x0000FF00) >> 8);
		_data[7] = static_cast<i8> ((l2 & 0x000000FF));
		_data[8] = static_cast<i8> ((l3 & 0xFF000000) >> 24);
		_data[9] = static_cast<i8> ((l3 & 0x00FF0000) >> 16);
		_data[10] = static_cast<i8> ((l3 & 0x0000FF00) >> 8);
		_data[11] = static_cast<i8> ((l3 & 0x000000FF));
		_data[12] = static_cast<i8> ((l4 & 0xFF000000) >> 24);
		_data[13] = static_cast<i8> ((l4 & 0x00FF0000) >> 16);
		_data[14] = static_cast<i8> ((l4 & 0x0000FF00) >> 8);
		_data[15] = static_cast<i8> ((l4 & 0x000000FF));
	}
}

//------------------------------------------------------------------------
inline UID::UID (const TUID& uid) noexcept
{
	*this = uid;
}

//------------------------------------------------------------------------
inline UID::UID (const UID& uid) noexcept
{
	*this = uid;
}

//------------------------------------------------------------------------
inline UID& UID::operator= (const UID& uid) noexcept
{
	*this = uid.data ();
	return *this;
}

//------------------------------------------------------------------------
inline UID& UID::operator= (const TUID& uid) noexcept
{
    memcpy (_data, reinterpret_cast<ukk>(uid), 16);
	return *this;
}

//------------------------------------------------------------------------
inline constexpr auto UID::data () const noexcept -> const TUID&
{
	return _data;
}

//------------------------------------------------------------------------
inline constexpr size_t UID::size () const noexcept
{
	return sizeof (TUID);
}

//------------------------------------------------------------------------
inline std::string UID::toString (b8 comFormat) const noexcept
{
	std::string result;
	result.reserve (32);
	if (comFormat)
	{
		const auto& g = reinterpret_cast<const GUID*> (_data);

		t8 tmp[21] {};
		snprintf (tmp, 21, "%08X%04X%04X", g->Data1, g->Data2, g->Data3);
		result = tmp;

		for (u32 i = 0; i < 8; ++i)
		{
			t8 s[3] {};
			snprintf (s, 3, "%02X", g->Data4[i]);
			result += s;
		}
	}
	else
	{
		for (u32 i = 0; i < 16; ++i)
		{
			t8 s[3] {};
			snprintf (s, 3, "%02X", static_cast<u8> (_data[i]));
			result += s;
		}
	}
	return result;
}

//------------------------------------------------------------------------
template<typename StringT>
inline Optional<UID> UID::fromString (const StringT& str, b8 comFormat) noexcept
{
	if (str.length () != 32)
		return {};
	// TODO: this is a copy from FUID. there are no input validation checks !!!
	if (comFormat)
	{
		TUID uid {};
		GUID g;
		t8 s[33];

		strcpy (s, str.data ());
		s[8] = 0;
		sscanf (s, "%x", &g.Data1);
		strcpy (s, str.data () + 8);
		s[4] = 0;
		sscanf (s, "%hx", &g.Data2);
		strcpy (s, str.data () + 12);
		s[4] = 0;
		sscanf (s, "%hx", &g.Data3);

		memcpy (uid, &g, 8);

		for (u32 i = 8; i < 16; ++i)
		{
			t8 s2[3] {};
			s2[0] = str[i * 2];
			s2[1] = str[i * 2 + 1];

			i32 d = 0;
			sscanf (s2, "%2x", &d);
			uid[i] = static_cast<t8> (d);
		}
		return {uid};
	}
	else
	{
		TUID uid {};
		for (u32 i = 0; i < 16; ++i)
		{
			t8 s[3] {};
			s[0] = str[i * 2];
			s[1] = str[i * 2 + 1];

			i32 d = 0;
			sscanf (s, "%2x", &d);
			uid[i] = static_cast<t8> (d);
		}
		return {uid};
	}
}

//------------------------------------------------------------------------
inline UID UID::fromTUID (const TUID _uid) noexcept
{
	UID result;
	memcpy (result._data, reinterpret_cast<ukk>(_uid), 16);
	return result;
}

//------------------------------------------------------------------------
} // VST3
