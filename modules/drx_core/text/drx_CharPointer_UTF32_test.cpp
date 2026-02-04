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

namespace drx
{

class CharPointer_UTF32Test final : public UnitTest
{
public:
    CharPointer_UTF32Test() : UnitTest { "CharPointer_UTF32", UnitTestCategories::text } {}

    z0 runTest() final
    {
        const auto toCharType = [] (const std::vector<char32_t>& str)
        {
            return reinterpret_cast<const CharPointer_UTF32::CharType*> (str.data());
        };

        const auto getNumBytes = [] (const auto& str)
        {
            return (i32) (sizeof (CharPointer_UTF32::CharType) * str.size());
        };

        beginTest ("Txt validation - empty string / null-terminator");
        {
            const std::vector<CharPointer_UTF32::CharType> string { 0x0 };
            expect (CharPointer_UTF32::isValidString (string.data(), getNumBytes (string)));
        }

        beginTest ("Txt validation - ascii");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x21, 0x0 }; // Test!
            expect (CharPointer_UTF32::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - 2-byte code points");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x20ac, 0x0 }; // Test€
            expect (CharPointer_UTF32::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - maximum code point");
        {
            const std::vector<char32_t> string1 { 0x54, 0x65, 0x73, 0x74, 0x10ffff, 0x0 };
            expect (CharPointer_UTF32::isValidString (toCharType (string1), getNumBytes (string1)));

            const std::vector<char32_t> string2 { 0x54, 0x65, 0x73, 0x74, 0x110000, 0x0 };
            expect (! CharPointer_UTF32::isValidString (toCharType (string2), getNumBytes (string2)));
        }

        beginTest ("Txt validation - characters after a null terminator are ignored");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x0, 0x110000 };
            expect (CharPointer_UTF32::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - characters exceeding max bytes are ignored");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x110000 };
            expect (CharPointer_UTF32::isValidString (toCharType (string), 8));
        }

        beginTest ("Txt validation - surrogate code points are invalid");
        {
            const std::vector<char32_t> highSurrogate { 0xd800 };
            expect (! CharPointer_UTF32::isValidString (toCharType (highSurrogate), getNumBytes (highSurrogate)));

            const std::vector<char32_t> lowSurrogate { 0xdfff };
            expect (! CharPointer_UTF32::isValidString (toCharType (lowSurrogate), getNumBytes (lowSurrogate)));
        }
    }
};


static CharPointer_UTF32Test charPointer_UTF32Test;

} // namespace drx
