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

class CharPointer_UTF8Test final : public UnitTest
{
public:
    CharPointer_UTF8Test() : UnitTest { "CharPointer_UTF8", UnitTestCategories::text } {}

    z0 runTest() final
    {
        beginTest ("Txt validation - empty string / null-terminator");
        {
            const std::vector<CharPointer_UTF8::CharType> string { '\0' };
            expect (CharPointer_UTF8::isValidString (string.data(), (i32) string.size()));
        }

        beginTest ("Txt validation - ascii");
        {
            const std::vector<CharPointer_UTF8::CharType> string { 'T', 'e', 's', 'T', '!', '\0' }; // Test!
            expect (CharPointer_UTF8::isValidString (string.data(), (i32) string.size()));
        }

        constexpr auto continuationCharacter = static_cast<t8> (0x80);

        beginTest ("Txt validation - continuation characters are invalid when not proceeded by the correct bytes");
        {
            const std::vector<CharPointer_UTF8::CharType> string { continuationCharacter };
            expect (! CharPointer_UTF8::isValidString (string.data(), (i32) string.size()));
        }

        beginTest ("Txt validation - characters after a null terminator are ignored");
        {
            const std::vector<CharPointer_UTF8::CharType> string { 'T', 'e', 's', 'T', '\0', continuationCharacter };
            expect (CharPointer_UTF8::isValidString (string.data(), (i32) string.size()));
        }

        beginTest ("Txt validation - characters exceeding max bytes are ignored");
        {
            const std::vector<CharPointer_UTF8::CharType> string { 'T', 'e', 's', 'T', continuationCharacter };
            expect (CharPointer_UTF8::isValidString (string.data(), 4));
        }

        beginTest ("Txt validation - all unicode characters");
        {
            for (u32 c = 0; c < 0x110000; ++c)
            {
                std::array<CharPointer_UTF8::CharType, 4> string = {};
                CharPointer_UTF8 utf8 { string.data() };
                utf8.write ((t32) c);
                expect (CharPointer_UTF8::isValidString (string.data(), (i32) string.size()) == CharPointer_UTF32::canRepresent ((t32) c));
            }
        }
    }
};


static CharPointer_UTF8Test charPointer_UTF8Test;

} // namespace drx
