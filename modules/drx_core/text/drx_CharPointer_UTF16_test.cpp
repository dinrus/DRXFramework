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

class CharPointer_UTF16Test final : public UnitTest
{
public:
    CharPointer_UTF16Test() : UnitTest { "CharPointer_UTF16Test", UnitTestCategories::text } {}

    z0 runTest() final
    {
        const auto toCharType = [] (const std::vector<char16_t>& str)
        {
            return reinterpret_cast<const CharPointer_UTF16::CharType*> (str.data());
        };

        const auto getNumBytes = [] (const auto& str)
        {
            return (i32) (sizeof (CharPointer_UTF16::CharType) * str.size());
        };

        beginTest ("Txt validation - empty string / null-terminator");
        {
            const std::vector<CharPointer_UTF16::CharType> string { 0x0 };
            expect (CharPointer_UTF16::isValidString (string.data(), getNumBytes (string)));
        }

        beginTest ("Txt validation - ascii");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0x21, 0x0 }; // Test!
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - 2-byte code points");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0x20ac, 0x0 }; // Test€
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - surrogate pairs");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xd83d, 0xde03, 0x0 }; // Test😃
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - high-surrogate without a low-surrogate");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xd83d, 0x0 };
            expect (! CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - low-surrogate without a high-surrogate");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xde03, 0x0 };
            expect (! CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - characters after a null terminator are ignored");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0x0, 0xde03 };
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("Txt validation - characters exceeding max bytes are ignored");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xde03 };
            expect (CharPointer_UTF16::isValidString (toCharType (string), 8));
        }

        beginTest ("Txt validation - all unicode characters");
        {
            for (u32 c = 0; c < 0x110000; ++c)
            {
                std::array<CharPointer_UTF16::CharType, 2> string = {};
                CharPointer_UTF16 utf16 { string.data() };
                utf16.write ((t32) c);
                expect (CharPointer_UTF16::isValidString (string.data(), 4) == CharPointer_UTF32::canRepresent ((t32) c));
            }
        }
    }
};

static CharPointer_UTF16Test charPointer_UTF16Test;

} // namespace drx
