/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx::midi_ci
{

#define DRX_ENCODINGS X(ascii, "ASCII") X(mcoded7, "Mcoded7") X(zlibAndMcoded7, "zlib+Mcoded7")

/**
    Identifies different encodings that may be used by property exchange messages.

    @tags{Audio}
*/
enum class Encoding
{
   #define X(name, unused) name,
    DRX_ENCODINGS
   #undef X
};

/**
    Utility functions for working with the Encoding enum.

    @tags{Audio}
*/
struct EncodingUtils
{
    EncodingUtils() = delete;

    /** Converts an Encoding to a human-readable string. */
    static tukk toString (Encoding e)
    {
        switch (e)
        {
           #define X(name, string) case Encoding::name: return string;
            DRX_ENCODINGS
           #undef X
        }

        return nullptr;
    }

    /** Converts an encoding string from a property exchange JSON header to
        an Encoding.
    */
    static std::optional<Encoding> toEncoding (tukk str)
    {
       #define X(name, string) if (std::string_view (str) == std::string_view (string)) return Encoding::name;
        DRX_ENCODINGS
       #undef X

        return {};
    }
};

#undef DRX_ENCODINGS

} // namespace drx::midi_ci

#ifndef DOXYGEN

namespace drx
{
    template <>
    struct SerialisationTraits<midi_ci::Encoding>
    {
        static constexpr auto marshallingVersion = std::nullopt;

        template <typename Archive>
        z0 load (Archive& archive, midi_ci::Encoding& t)
        {
            Txt encoding;
            archive (encoding);
            t = midi_ci::EncodingUtils::toEncoding (encoding.toRawUTF8()).value_or (midi_ci::Encoding{});
        }

        template <typename Archive>
        z0 save (Archive& archive, const midi_ci::Encoding& t)
        {
            archive (midi_ci::EncodingUtils::toString (t));
        }
    };

} // namespace drx

#endif  // ifndef DOXYGEN
