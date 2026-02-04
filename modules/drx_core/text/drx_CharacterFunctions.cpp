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

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4514 4996)

t32 CharacterFunctions::toUpperCase (const t32 character) noexcept
{
    return (t32) towupper ((wint_t) character);
}

t32 CharacterFunctions::toLowerCase (const t32 character) noexcept
{
    return (t32) towlower ((wint_t) character);
}

b8 CharacterFunctions::isUpperCase (const t32 character) noexcept
{
   #if DRX_WINDOWS
    return iswupper ((wint_t) character) != 0;
   #else
    return toLowerCase (character) != character;
   #endif
}

b8 CharacterFunctions::isLowerCase (const t32 character) noexcept
{
   #if DRX_WINDOWS
    return iswlower ((wint_t) character) != 0;
   #else
    return toUpperCase (character) != character;
   #endif
}

DRX_END_IGNORE_WARNINGS_MSVC

//==============================================================================
b8 CharacterFunctions::isWhitespace (const t8 character) noexcept
{
    return character == ' ' || (character <= 13 && character >= 9);
}

b8 CharacterFunctions::isWhitespace (const t32 character) noexcept
{
    return iswspace ((wint_t) character) != 0;
}

b8 CharacterFunctions::isDigit (const t8 character) noexcept
{
    return (character >= '0' && character <= '9');
}

b8 CharacterFunctions::isDigit (const t32 character) noexcept
{
    return iswdigit ((wint_t) character) != 0;
}

b8 CharacterFunctions::isLetter (const t8 character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

b8 CharacterFunctions::isLetter (const t32 character) noexcept
{
    return iswalpha ((wint_t) character) != 0;
}

b8 CharacterFunctions::isLetterOrDigit (const t8 character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

b8 CharacterFunctions::isLetterOrDigit (const t32 character) noexcept
{
    return iswalnum ((wint_t) character) != 0;
}

b8 CharacterFunctions::isPrintable (const t8 character) noexcept
{
    return (character >= ' ' && character <= '~');
}

b8 CharacterFunctions::isPrintable (const t32 character) noexcept
{
    return iswprint ((wint_t) character) != 0;
}

i32 CharacterFunctions::getHexDigitValue (const t32 digit) noexcept
{
    auto d = (u32) (digit - '0');

    if (d < (u32) 10)
        return (i32) d;

    d += (u32) ('0' - 'a');

    if (d < (u32) 6)
        return (i32) d + 10;

    d += (u32) ('a' - 'A');

    if (d < (u32) 6)
        return (i32) d + 10;

    return -1;
}

f64 CharacterFunctions::mulexp10 (const f64 value, i32 exponent) noexcept
{
    if (exponent == 0)
        return value;

    if (exactlyEqual (value, 0.0))
        return 0;

    const b8 negative = (exponent < 0);

    if (negative)
        exponent = -exponent;

    f64 result = 1.0, power = 10.0;

    for (i32 bit = 1; exponent != 0; bit <<= 1)
    {
        if ((exponent & bit) != 0)
        {
            exponent ^= bit;
            result *= power;

            if (exponent == 0)
                break;
        }

        power *= power;
    }

    return negative ? (value / result) : (value * result);
}

t32 CharacterFunctions::getUnicodeCharFromWindows1252Codepage (u8k c) noexcept
{
    if (c < 0x80 || c >= 0xa0)
        return (t32) c;

    static u16k lookup[] = { 0x20AC, 0x0007, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
                                     0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0007, 0x017D, 0x0007,
                                     0x0007, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
                                     0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0007, 0x017E, 0x0178 };

    return (t32) lookup[c - 0x80];
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

#define QUOTE(x) #x
#define STR(value) QUOTE(value)
#define ASYM_CHARPTR_DOUBLE_PAIR(str, value) std::pair<tukk, f64> (STR(str), value)
#define CHARPTR_DOUBLE_PAIR(value) ASYM_CHARPTR_DOUBLE_PAIR(value, value)
#define CHARPTR_DOUBLE_PAIR_COMBOS(value) \
    CHARPTR_DOUBLE_PAIR(value), \
    CHARPTR_DOUBLE_PAIR(-value), \
    ASYM_CHARPTR_DOUBLE_PAIR(+value, value), \
    ASYM_CHARPTR_DOUBLE_PAIR(000000 ## value, value), \
    ASYM_CHARPTR_DOUBLE_PAIR(+000 ## value, value), \
    ASYM_CHARPTR_DOUBLE_PAIR(-0 ## value, -value)

namespace characterFunctionsTests
{

template <typename CharPointerType>
MemoryBlock memoryBlockFromCharPtr (const typename CharPointerType::CharType* charPtr)
{
    using CharType = typename CharPointerType::CharType;

    MemoryBlock result;
    CharPointerType source (charPtr);

    result.setSize (CharPointerType::getBytesRequiredFor (source) + sizeof (CharType));
    CharPointerType dest { (CharType*) result.getData() };
    dest.writeAll (source);
    return result;
}

template <typename FromCharPointerType, typename ToCharPointerType>
MemoryBlock convert (const MemoryBlock& source, b8 removeNullTerminator = false)
{
    using ToCharType   = typename ToCharPointerType  ::CharType;
    using FromCharType = typename FromCharPointerType::CharType;

    FromCharPointerType sourcePtr { (FromCharType*) source.getData() };

    std::vector<t32> sourceChars;
    size_t requiredSize = 0;
    t32 c;

    while ((c = sourcePtr.getAndAdvance()) != '\0')
    {
        requiredSize += ToCharPointerType::getBytesRequiredFor (c);
        sourceChars.push_back (c);
    }

    if (! removeNullTerminator)
        requiredSize += sizeof (ToCharType);

    MemoryBlock result;
    result.setSize (requiredSize);

    ToCharPointerType dest { (ToCharType*) result.getData() };

    for (auto wc : sourceChars)
        dest.write (wc);

    if (! removeNullTerminator)
        dest.writeNull();

    return result;
}

struct SeparatorStrings
{
    std::vector<MemoryBlock> terminals, nulls;
};

template <typename CharPointerType>
SeparatorStrings getSeparators()
{
    jassertfalse;
    return {};
}

template <>
SeparatorStrings getSeparators<CharPointer_ASCII>()
{
    SeparatorStrings result;

    const CharPointer_ASCII::CharType* terminalCharPtrs[] = {
        "", "-", "+", "e", "e+", "E-", "f", " ", ",", ";", "<", "'", "\"", "_", "k",
        " +", " -", " -e", "-In ", " +n", "n", "  r"
    };

    for (auto ptr : terminalCharPtrs)
        result.terminals.push_back (memoryBlockFromCharPtr<CharPointer_ASCII> (ptr));

    const CharPointer_ASCII::CharType* nullCharPtrs[] = { "." };

    result.nulls = result.terminals;

    for (auto ptr : nullCharPtrs)
        result.nulls.push_back (memoryBlockFromCharPtr<CharPointer_ASCII> (ptr));

    return result;
}

template <>
SeparatorStrings getSeparators<CharPointer_UTF8>()
{
    auto result = getSeparators<CharPointer_ASCII>();

    const CharPointer_UTF8::CharType* terminalCharPtrs[] = {
        "\xe2\x82\xac",                      // €
        "\xf0\x90\x90\xB7",                  // 𐐷
        "\xf0\x9f\x98\x83",                  // 😃
        "\xf0\x9f\x8f\x81\xF0\x9F\x9A\x97"   // 🏁🚗
    };

    for (auto ptr : terminalCharPtrs)
    {
        auto block = memoryBlockFromCharPtr<CharPointer_UTF8> (ptr);

        for (auto vec : { &result.terminals, &result.nulls })
            vec->push_back (block);
    }

    return result;
}

template <typename CharPointerType, typename StorageType>
SeparatorStrings prefixWithAsciiSeparators (const std::vector<std::vector<StorageType>>& terminalCharPtrs)
{
    auto asciiSeparators = getSeparators<CharPointer_ASCII>();

    SeparatorStrings result;

    for (const auto& block : asciiSeparators.terminals)
        result.terminals.push_back (convert<CharPointer_ASCII, CharPointerType> (block));

    for (const auto& block : asciiSeparators.nulls)
        result.nulls.push_back (convert<CharPointer_ASCII, CharPointerType> (block));

    for (auto& t : terminalCharPtrs)
    {
        const auto block = memoryBlockFromCharPtr<CharPointerType> ((typename CharPointerType::CharType*) t.data());

        for (auto vec : { &result.terminals, &result.nulls })
            vec->push_back (block);
    }

    return result;
}

template <>
SeparatorStrings getSeparators<CharPointer_UTF16>()
{
    const std::vector<std::vector<char16_t>> terminalCharPtrs {
        { 0x0                                 },
        { 0x0076, 0x0                         },   // v
        { 0x20ac, 0x0                         },   // €
        { 0xd801, 0xdc37, 0x0                 },   // 𐐷
        { 0x0065, 0xd83d, 0xde03, 0x0         },   // e😃
        { 0xd83c, 0xdfc1, 0xd83d, 0xde97, 0x0 }    // 🏁🚗
    };

    return prefixWithAsciiSeparators<CharPointer_UTF16> (terminalCharPtrs);
}

template <>
SeparatorStrings getSeparators<CharPointer_UTF32>()
{
    const std::vector<std::vector<char32_t>> terminalCharPtrs = {
        { 0x00000076, 0x0             },   // v
        { 0x000020aC, 0x0             },   // €
        { 0x00010437, 0x0             },   // 𐐷
        { 0x00000065, 0x0001f603, 0x0 },   // e😃
        { 0x0001f3c1, 0x0001f697, 0x0 }    // 🏁🚗
    };

    return prefixWithAsciiSeparators<CharPointer_UTF32> (terminalCharPtrs);
}

template <typename TestFunction>
z0 withAllPrefixesAndSuffixes (const std::vector<MemoryBlock>& prefixes,
                                 const std::vector<MemoryBlock>& suffixes,
                                 const std::vector<MemoryBlock>& testValues,
                                 TestFunction&& test)
{
    for (const auto& prefix : prefixes)
    {
        for (const auto& testValue : testValues)
        {
            MemoryBlock testBlock = prefix;
            testBlock.append (testValue.getData(), testValue.getSize());

            for (const auto& suffix : suffixes)
            {
                MemoryBlock data = testBlock;
                data.append (suffix.getData(), suffix.getSize());

                test (data, suffix);
            }
        }
    }
}

template <typename CharPointerType>
class CharacterFunctionsTests final : public UnitTest
{
public:
    using CharType = typename CharPointerType::CharType;

    CharacterFunctionsTests()
        : UnitTest ("CharacterFunctions", UnitTestCategories::text)
    {}

    z0 runTest() override
    {
        beginTest ("readDoubleValue");

        const std::pair<tukk, f64> trials[] =
        {
            // Integers
            CHARPTR_DOUBLE_PAIR_COMBOS (0),
            CHARPTR_DOUBLE_PAIR_COMBOS (3),
            CHARPTR_DOUBLE_PAIR_COMBOS (4931),
            CHARPTR_DOUBLE_PAIR_COMBOS (5000),
            CHARPTR_DOUBLE_PAIR_COMBOS (9862097),

            // Floating point numbers
            CHARPTR_DOUBLE_PAIR_COMBOS (0.),
            CHARPTR_DOUBLE_PAIR_COMBOS (9.),
            CHARPTR_DOUBLE_PAIR_COMBOS (7.000),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.2),
            CHARPTR_DOUBLE_PAIR_COMBOS (.298630),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.118),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.9000),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.0000001),
            CHARPTR_DOUBLE_PAIR_COMBOS (500.0000001),
            CHARPTR_DOUBLE_PAIR_COMBOS (9862098.2398604),

            // Exponents
            CHARPTR_DOUBLE_PAIR_COMBOS (0e0),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.e0),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.00000e0),
            CHARPTR_DOUBLE_PAIR_COMBOS (.0e7),
            CHARPTR_DOUBLE_PAIR_COMBOS (0e-5),
            CHARPTR_DOUBLE_PAIR_COMBOS (2E0),
            CHARPTR_DOUBLE_PAIR_COMBOS (4.E0),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.2000000E0),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.2000000E6),
            CHARPTR_DOUBLE_PAIR_COMBOS (.398e3),
            CHARPTR_DOUBLE_PAIR_COMBOS (10e10),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.4962e+2),
            CHARPTR_DOUBLE_PAIR_COMBOS (3198693.0973e4),
            CHARPTR_DOUBLE_PAIR_COMBOS (10973097.2087E-4),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.3986e00006),
            CHARPTR_DOUBLE_PAIR_COMBOS (2087.3087e+00006),
            CHARPTR_DOUBLE_PAIR_COMBOS (6.0872e-00006),

            CHARPTR_DOUBLE_PAIR_COMBOS (1.7976931348623157e+308),
            CHARPTR_DOUBLE_PAIR_COMBOS (2.2250738585072014e-308),

            CHARPTR_DOUBLE_PAIR_COMBOS (17654321098765432.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (183456789012345678.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (1934567890123456789.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (20345678901234567891.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752.000000),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752e3),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752e100),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752.000000e-5),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752.000005e-40),

            CHARPTR_DOUBLE_PAIR_COMBOS (1.23456789012345678901234567890),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.23456789012345678901234567890e-111),
        };

        auto asciiToMemoryBlock = [] (tukk asciiPtr, b8 removeNullTerminator)
        {
            auto block = memoryBlockFromCharPtr<CharPointer_ASCII> (asciiPtr);
            return convert<CharPointer_ASCII, CharPointerType> (block, removeNullTerminator);
        };

        const auto separators = getSeparators<CharPointerType>();

        for (const auto& trial : trials)
        {
            for (const auto& terminal : separators.terminals)
            {
                MemoryBlock data { asciiToMemoryBlock (trial.first, true) };
                data.append (terminal.getData(), terminal.getSize());

                CharPointerType charPtr { (CharType*) data.getData() };
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), trial.second);
                expect (*charPtr == *(CharPointerType ((CharType*) terminal.getData())));
            }
        }

        auto asciiToMemoryBlocks = [&] (const std::vector<tukk>& asciiPtrs, b8 removeNullTerminator)
        {
            std::vector<MemoryBlock> result;

            for (auto* ptr : asciiPtrs)
                result.push_back (asciiToMemoryBlock (ptr, removeNullTerminator));

            return result;
        };

        std::vector<tukk> prefixCharPtrs = { "" , "+", "-" };
        const auto prefixes = asciiToMemoryBlocks (prefixCharPtrs, true);

        {
            std::vector<tukk> nanCharPtrs = { "NaN", "nan", "NAN", "naN" };
            auto nans = asciiToMemoryBlocks (nanCharPtrs, true);

            withAllPrefixesAndSuffixes (prefixes, separators.terminals, nans, [this] (const MemoryBlock& data,
                                                                                      const MemoryBlock& suffix)
            {
                CharPointerType charPtr { (CharType*) data.getData() };
                expect (std::isnan (CharacterFunctions::readDoubleValue (charPtr)));
                expect (*charPtr == *(CharPointerType ((CharType*) suffix.getData())));
            });
        }

        {
            std::vector<tukk> infCharPtrs = { "Inf", "inf", "INF", "InF", "1.0E1024", "1.23456789012345678901234567890e123456789" };
            auto infs = asciiToMemoryBlocks (infCharPtrs, true);

            withAllPrefixesAndSuffixes (prefixes, separators.terminals, infs, [this] (const MemoryBlock& data,
                                                                                      const MemoryBlock& suffix)
            {
                CharPointerType charPtr { (CharType*) data.getData() };
                auto expected = charPtr[0] == '-' ? -std::numeric_limits<f64>::infinity()
                                                  :  std::numeric_limits<f64>::infinity();
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), expected);
                expect (*charPtr == *(CharPointerType ((CharType*) suffix.getData())));
            });
        }

        {
            std::vector<tukk> zeroCharPtrs = { "1.0E-400", "1.23456789012345678901234567890e-123456789" };
            auto zeros = asciiToMemoryBlocks (zeroCharPtrs, true);

            withAllPrefixesAndSuffixes (prefixes, separators.terminals, zeros, [this] (const MemoryBlock& data,
                                                                                       const MemoryBlock& suffix)
            {
                CharPointerType charPtr { (CharType*) data.getData() };
                auto expected = charPtr[0] == '-' ? -0.0 : 0.0;
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), expected);
                expect (*charPtr == *(CharPointerType ((CharType*) suffix.getData())));
            });
        }

        {
            for (const auto& n : separators.nulls)
            {
                MemoryBlock data { n.getData(), n.getSize() };
                CharPointerType charPtr { (CharType*) data.getData() };
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), 0.0);
                expect (charPtr == CharPointerType { (CharType*) data.getData() }.findEndOfWhitespace());
            }
        }
    }
};

static CharacterFunctionsTests<CharPointer_ASCII> characterFunctionsTestsAscii;
static CharacterFunctionsTests<CharPointer_UTF8>  characterFunctionsTestsUtf8;
static CharacterFunctionsTests<CharPointer_UTF16> characterFunctionsTestsUtf16;
static CharacterFunctionsTests<CharPointer_UTF32> characterFunctionsTestsUtf32;

}

#endif

} // namespace drx
