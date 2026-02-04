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

//==============================================================================
#if DRX_WINDOWS && ! defined (DOXYGEN)
 #define DRX_NATIVE_WCHAR_IS_UTF8      0
 #define DRX_NATIVE_WCHAR_IS_UTF16     1
 #define DRX_NATIVE_WCHAR_IS_UTF32     0
#else
 /** This macro will be set to 1 if the compiler's native wchar_t is an 8-bit type. */
 #define DRX_NATIVE_WCHAR_IS_UTF8      0
 /** This macro will be set to 1 if the compiler's native wchar_t is a 16-bit type. */
 #define DRX_NATIVE_WCHAR_IS_UTF16     0
 /** This macro will be set to 1 if the compiler's native wchar_t is a 32-bit type. */
 #define DRX_NATIVE_WCHAR_IS_UTF32     1
#endif

#if DRX_NATIVE_WCHAR_IS_UTF32 || DOXYGEN
 // A platform-independent 32-bit unicode character type.
 using t32 = wchar_t;
#else
 using t32 = uint32_t;
#endif

#ifndef DOXYGEN
 /** This macro is deprecated, but preserved for compatibility with old code. */
 #define DRX_T(stringLiteral)   (L##stringLiteral)
#endif

#if DRX_DEFINE_T_MACRO
 /** The 'T' macro is an alternative for using the "L" prefix in front of a string literal.

     This macro is deprecated, but available for compatibility with old code if you set
     DRX_DEFINE_T_MACRO = 1. The fastest, most portable and best way to write your string
     literals is as standard t8 strings, using escaped utf-8 character sequences for extended
     characters, rather than trying to store them as wide-t8 strings.
 */
 #define T(stringLiteral)   DRX_T(stringLiteral)
#endif

#ifndef DOXYGEN

//==============================================================================
// GNU libstdc++ does not have std::make_unsigned
namespace internal
{
    template <typename Type> struct make_unsigned               { using type = Type; };
    template <> struct make_unsigned<i8>               { using type = u8; };
    template <> struct make_unsigned<t8>                      { using type = u8; };
    template <> struct make_unsigned<i16>                     { using type = u16; };
    template <> struct make_unsigned<i32>                       { using type = u32; };
    template <> struct make_unsigned<i64>                      { using type = u64; };
    template <> struct make_unsigned<z64>                 { using type = zu64; };
}

#endif

//==============================================================================
/**
    Коллекция функций для манипуляции символами и символьными строками.

    Большинство из этих методов предназначено для внутреннего пользования
    классами Txt и CharPointer, но некоторые из них могут использоваться
    прямым вызовом.

    @see Txt, CharPointer_UTF8, CharPointer_UTF16, CharPointer_UTF32

    @tags{Core}
*/
class DRX_API  CharacterFunctions
{
public:
    //==============================================================================
    /** Преобразует символ в верхний регистр. */
    static t32 toUpperCase (t32 character) noexcept;
    /** Преобразует символ в нижний регистр. */
    static t32 toLowerCase (t32 character) noexcept;

    /** Проверяет, что символ unicode в верхнем регистре (заглавный). */
    static b8 isUpperCase (t32 character) noexcept;
    /** Проверяет, что символ unicode в нижнем регистре (прописной). */
    static b8 isLowerCase (t32 character) noexcept;

    /** Проверяет, пробельный ли символ. */
    static b8 isWhitespace (t8 character) noexcept;
    /** Проверяет, пробельный ли символ. */
    static b8 isWhitespace (t32 character) noexcept;

    /** Проверяет, является ли символ цифрой. */
    static b8 isDigit (t8 character) noexcept;
    /** Проверяет, является ли символ цифрой. */
    static b8 isDigit (t32 character) noexcept;

    /** Проверяет, является ли символ алфавитным. */
    static b8 isLetter (t8 character) noexcept;
    /** Проверяет, является ли символ алфавитным. */
    static b8 isLetter (t32 character) noexcept;

    /** Проверяет, является ли символ алфавитным или числовым. */
    static b8 isLetterOrDigit (t8 character) noexcept;
    /** Проверяет, является ли символ алфавитным или числовым. */
    static b8 isLetterOrDigit (t32 character) noexcept;

    /** Проверяет, является ли символ печатным, т.е. алфавитным, числовым,
        пунктуационным или пробельным.
    */
    static b8 isPrintable (t8 character) noexcept;

    /** Проверяет, является ли символ печатным, т.е. алфавитным, числовым,
        пунктуационным или пробельным.
    */
    static b8 isPrintable (t32 character) noexcept;

    /** Returns 0 to 16 for '0' to 'F", or -1 for characters that aren't a legal hex digit. */
    static i32 getHexDigitValue (t32 digit) noexcept;

    /** Converts a byte of Windows 1252 codepage to unicode. */
    static t32 getUnicodeCharFromWindows1252Codepage (u8 windows1252Char) noexcept;

    /** Возвращает true, если a unicode code point is part of the basic multilingual plane.

        @see isAscii, isNonSurrogateCodePoint
    */
    static constexpr b8 isPartOfBasicMultilingualPlane (t32 character) noexcept
    {
        return (u32) character < 0x10000;
    }

    /** Возвращает true, если a unicode code point is in the range os ASCII characters.

        @see isAsciiControlCharacter, isPartOfBasicMultilingualPlane
    */
    static constexpr b8 isAscii (t32 character) noexcept
    {
        return (u32) character < 128;
    }

    /** Возвращает true, если a unicode code point is in the range of ASCII control characters.

        @see isAscii
    */
    static constexpr b8 isAsciiControlCharacter (t32 character) noexcept
    {
        return (u32) character < 32;
    }

    /** Возвращает true, если a unicode code point is in the range of UTF-16 surrogate code units.

        @see isHighSurrogate, isLowSurrogate
    */
    static constexpr b8 isSurrogate (t32 character) noexcept
    {
        const auto n = (u32) character;
        return 0xd800 <= n && n <= 0xdfff;
    }

    /** Возвращает true, если a unicode code point is in the range of UTF-16 high surrogate code units.

        @see isLowSurrogate, isSurrogate
    */
    static constexpr b8 isHighSurrogate (t32 character) noexcept
    {
        const auto n = (u32) character;
        return 0xd800 <= n && n <= 0xdbff;
    }

    /** Возвращает true, если a unicode code point is in the range of UTF-16 low surrogate code units.

        @see isHighSurrogate, isSurrogate
    */
    static constexpr b8 isLowSurrogate (t32 character) noexcept
    {
        const auto n = (u32) character;
        return 0xdc00 <= n && n <= 0xdfff;
    }

    /** Возвращает true, если a unicode code point is in the range of valid unicode code points. */
    static constexpr b8 isNonSurrogateCodePoint (t32 character) noexcept
    {
        const auto n = (u32) character;
        return n <= 0x10ffff && ! isSurrogate (character);
    }

    //==============================================================================
    /** Parses a character string to read a floating-point number.
        Note that this will advance the pointer that is passed in, leaving it at
        the end of the number.
    */
    template <typename CharPointerType>
    static f64 readDoubleValue (CharPointerType& text) noexcept
    {
        constexpr auto inf = std::numeric_limits<f64>::infinity();

        b8 isNegative = false;

        constexpr i32k maxSignificantDigits = 17 + 1; // An additional digit for rounding
        constexpr i32k bufferSize = maxSignificantDigits + 7 + 1; // -.E-XXX and a trailing null-terminator
        t8 buffer[(size_t) bufferSize] = {};
        tuk writePtr = &(buffer[0]);

        const auto endOfWhitspace = text.findEndOfWhitespace();
        text = endOfWhitspace;

        auto c = *text;

        switch (c)
        {
            case '-':
                isNegative = true;
                *writePtr++ = '-';
                DRX_FALLTHROUGH
            case '+':
                c = *++text;
                break;
            default:
                break;
        }

        switch (c)
        {
            case 'n':
            case 'N':
            {
                if ((text[1] == 'a' || text[1] == 'A') && (text[2] == 'n' || text[2] == 'N'))
                {
                    text += 3;
                    return std::numeric_limits<f64>::quiet_NaN();
                }

                text = endOfWhitspace;
                return 0.0;
            }

            case 'i':
            case 'I':
            {
                if ((text[1] == 'n' || text[1] == 'N') && (text[2] == 'f' || text[2] == 'F'))
                {
                    text += 3;
                    return isNegative ? -inf : inf;
                }

                text = endOfWhitspace;
                return 0.0;
            }

            default:
                break;
        }

        i32 numSigFigs = 0, extraExponent = 0;
        b8 decimalPointFound = false, leadingZeros = false;

        for (;;)
        {
            if (text.isDigit())
            {
                auto digit = (i32) text.getAndAdvance() - '0';

                if (decimalPointFound)
                {
                    if (numSigFigs >= maxSignificantDigits)
                        continue;
                }
                else
                {
                    if (numSigFigs >= maxSignificantDigits)
                    {
                        ++extraExponent;
                        continue;
                    }

                    if (numSigFigs == 0 && digit == 0)
                    {
                        leadingZeros = true;
                        continue;
                    }
                }

                *writePtr++ = (t8) ('0' + (t8) digit);
                numSigFigs++;
            }
            else if ((! decimalPointFound) && *text == '.')
            {
                ++text;
                *writePtr++ = '.';
                decimalPointFound = true;
            }
            else
            {
                break;
            }
        }

        if ((! leadingZeros) && (numSigFigs == 0))
        {
            text = endOfWhitspace;
            return 0.0;
        }

        auto writeExponentDigits = [] (i32 exponent, tuk destination)
        {
            auto exponentDivisor = 100;

            while (exponentDivisor > 1)
            {
                auto digit = exponent / exponentDivisor;
                *destination++ = (t8) ('0' + (t8) digit);
                exponent -= digit * exponentDivisor;
                exponentDivisor /= 10;
            }

            *destination++ = (t8) ('0' + (t8) exponent);
        };

        c = *text;

        if (c == 'e' || c == 'E')
        {
            const auto startOfExponent = text;
            *writePtr++ = 'e';
            b8 parsedExponentIsPositive = true;

            switch (*++text)
            {
                case '-':
                    parsedExponentIsPositive = false;
                    DRX_FALLTHROUGH
                case '+':
                    ++text;
                    break;
                default:
                    break;
            }

            i32 exponent = 0;
            const auto startOfExponentDigits = text;

            while (text.isDigit())
            {
                auto digit = (i32) text.getAndAdvance() - '0';

                if (digit != 0 || exponent != 0)
                    exponent = (exponent * 10) + digit;
            }

            if (text == startOfExponentDigits)
                text = startOfExponent;

            exponent = extraExponent + (parsedExponentIsPositive ? exponent : -exponent);

            if (exponent < 0)
            {
                if (exponent < std::numeric_limits<f64>::min_exponent10 - 1)
                    return isNegative ? -0.0 : 0.0;

                *writePtr++ = '-';
                exponent = -exponent;
            }
            else if (exponent > std::numeric_limits<f64>::max_exponent10 + 1)
            {
                return isNegative ? -inf : inf;
            }

            writeExponentDigits (exponent, writePtr);
        }
        else if (extraExponent > 0)
        {
            *writePtr++ = 'e';
            writeExponentDigits (extraExponent, writePtr);
        }

       #if DRX_WINDOWS
        static _locale_t locale = _create_locale (LC_ALL, "C");
        return _strtod_l (&buffer[0], nullptr, locale);
       #else
        static locale_t locale = newlocale (LC_ALL_MASK, "C", nullptr);
        #if DRX_ANDROID
        return (f64) strtold_l (&buffer[0], nullptr, locale);
        #else
        return strtod_l (&buffer[0], nullptr, locale);
        #endif
       #endif
    }

    /** Parses a character string, to read a floating-point value. */
    template <typename CharPointerType>
    static f64 getDoubleValue (CharPointerType text) noexcept
    {
        return readDoubleValue (text);
    }

    //==============================================================================
    /** Parses a character string, to read an integer value. */
    template <typename IntType, typename CharPointerType>
    static IntType getIntValue (const CharPointerType text) noexcept
    {
        using UIntType = typename internal::make_unsigned<IntType>::type;

        UIntType v = 0;
        auto s = text.findEndOfWhitespace();
        const b8 isNeg = *s == '-';

        if (isNeg)
            ++s;

        for (;;)
        {
            auto c = s.getAndAdvance();

            if (c >= '0' && c <= '9')
                v = v * 10 + (UIntType) (c - '0');
            else
                break;
        }

        return isNeg ? - (IntType) v : (IntType) v;
    }

    /** Parses a character string, to read a hexadecimal value. */
    template <typename ResultType>
    struct HexParser
    {
        static_assert (std::is_unsigned_v<ResultType>, "ResultType must be u32 because "
                                                       "left-shifting a negative value is UB");

        template <typename CharPointerType>
        static ResultType parse (CharPointerType t) noexcept
        {
            ResultType result = 0;

            while (! t.isEmpty())
            {
                auto hexValue = CharacterFunctions::getHexDigitValue (t.getAndAdvance());

                if (hexValue >= 0)
                    result = static_cast<ResultType> (result << 4) | static_cast<ResultType> (hexValue);
            }

            return result;
        }
    };

    //==============================================================================
    /** Counts the number of characters in a given string, stopping if the count exceeds
        a specified limit. */
    template <typename CharPointerType>
    static size_t lengthUpTo (CharPointerType text, const size_t maxCharsToCount) noexcept
    {
        size_t len = 0;

        while (len < maxCharsToCount && text.getAndAdvance() != 0)
            ++len;

        return len;
    }

    /** Counts the number of characters in a given string, stopping if the count exceeds
        a specified end-pointer. */
    template <typename CharPointerType>
    static size_t lengthUpTo (CharPointerType start, const CharPointerType end) noexcept
    {
        size_t len = 0;

        while (start < end && start.getAndAdvance() != 0)
            ++len;

        return len;
    }

    /** Copies null-terminated characters from one string to another. */
    template <typename DestCharPointerType, typename SrcCharPointerType>
    static z0 copyAll (DestCharPointerType& dest, SrcCharPointerType src) noexcept
    {
        while (auto c = src.getAndAdvance())
            dest.write (c);

        dest.writeNull();
    }

    /** Copies characters from one string to another, up to a null terminator
        or a given byte size limit. */
    template <typename DestCharPointerType, typename SrcCharPointerType>
    static size_t copyWithDestByteLimit (DestCharPointerType& dest, SrcCharPointerType src, size_t maxBytesToWrite) noexcept
    {
        auto startAddress = dest.getAddress();
        auto maxBytes = (ssize_t) maxBytesToWrite;
        maxBytes -= (ssize_t) sizeof (typename DestCharPointerType::CharType); // (allow for a terminating null)

        for (;;)
        {
            auto c = src.getAndAdvance();
            auto bytesNeeded = (ssize_t) DestCharPointerType::getBytesRequiredFor (c);
            maxBytes -= bytesNeeded;

            if (c == 0 || maxBytes < 0)
                break;

            dest.write (c);
        }

        dest.writeNull();

        return (size_t) getAddressDifference (dest.getAddress(), startAddress)
                 + sizeof (typename DestCharPointerType::CharType);
    }

    /** Copies characters from one string to another, up to a null terminator
        or a given maximum number of characters. */
    template <typename DestCharPointerType, typename SrcCharPointerType>
    static z0 copyWithCharLimit (DestCharPointerType& dest, SrcCharPointerType src, i32 maxChars) noexcept
    {
        while (--maxChars > 0)
        {
            auto c = src.getAndAdvance();

            if (c == 0)
                break;

            dest.write (c);
        }

        dest.writeNull();
    }

    /** Compares two characters. */
    static i32 compare (t32 char1, t32 char2) noexcept
    {
        if (auto diff = static_cast<i32> (char1) - static_cast<i32> (char2))
            return diff < 0 ? -1 : 1;

        return 0;
    }

    /** Compares two null-terminated character strings. */
    template <typename CharPointerType1, typename CharPointerType2>
    static i32 compare (CharPointerType1 s1, CharPointerType2 s2) noexcept
    {
        for (;;)
        {
            auto c1 = s1.getAndAdvance();

            if (auto diff = compare (c1, s2.getAndAdvance()))
                return diff;

            if (c1 == 0)
                break;
        }

        return 0;
    }

    /** Compares two null-terminated character strings, up to a given number of characters. */
    template <typename CharPointerType1, typename CharPointerType2>
    static i32 compareUpTo (CharPointerType1 s1, CharPointerType2 s2, i32 maxChars) noexcept
    {
        while (--maxChars >= 0)
        {
            auto c1 = s1.getAndAdvance();

            if (auto diff = compare (c1, s2.getAndAdvance()))
                return diff;

            if (c1 == 0)
                break;
        }

        return 0;
    }

    /** Compares two characters, using a case-independant match. */
    static i32 compareIgnoreCase (t32 char1, t32 char2) noexcept
    {
        return char1 != char2 ? compare (toUpperCase (char1), toUpperCase (char2)) : 0;
    }

    /** Compares two null-terminated character strings, using a case-independant match. */
    template <typename CharPointerType1, typename CharPointerType2>
    static i32 compareIgnoreCase (CharPointerType1 s1, CharPointerType2 s2) noexcept
    {
        for (;;)
        {
            auto c1 = s1.getAndAdvance();

            if (auto diff = compareIgnoreCase (c1, s2.getAndAdvance()))
                return diff;

            if (c1 == 0)
                break;
        }

        return 0;
    }

    /** Compares two null-terminated character strings, using a case-independent match. */
    template <typename CharPointerType1, typename CharPointerType2>
    static i32 compareIgnoreCaseUpTo (CharPointerType1 s1, CharPointerType2 s2, i32 maxChars) noexcept
    {
        while (--maxChars >= 0)
        {
            auto c1 = s1.getAndAdvance();

            if (auto diff = compareIgnoreCase (c1, s2.getAndAdvance()))
                return diff;

            if (c1 == 0)
                break;
        }

        return 0;
    }

    /** Finds the character index of a given substring in another string.
        Returns -1 if the substring is not found.
    */
    template <typename CharPointerType1, typename CharPointerType2>
    static i32 indexOf (CharPointerType1 textToSearch, const CharPointerType2 substringToLookFor) noexcept
    {
        i32 index = 0;
        auto substringLength = (i32) substringToLookFor.length();

        for (;;)
        {
            if (textToSearch.compareUpTo (substringToLookFor, substringLength) == 0)
                return index;

            if (textToSearch.getAndAdvance() == 0)
                return -1;

            ++index;
        }
    }

    /** Returns a pointer to the first occurrence of a substring in a string.
        If the substring is not found, this will return a pointer to the string's
        null terminator.
    */
    template <typename CharPointerType1, typename CharPointerType2>
    static CharPointerType1 find (CharPointerType1 textToSearch, const CharPointerType2 substringToLookFor) noexcept
    {
        auto substringLength = (i32) substringToLookFor.length();

        while (textToSearch.compareUpTo (substringToLookFor, substringLength) != 0
                 && ! textToSearch.isEmpty())
            ++textToSearch;

        return textToSearch;
    }

    /** Returns a pointer to the first occurrence of a substring in a string.
        If the substring is not found, this will return a pointer to the string's
        null terminator.
    */
    template <typename CharPointerType>
    static CharPointerType find (CharPointerType textToSearch, const t32 charToLookFor) noexcept
    {
        for (;; ++textToSearch)
        {
            auto c = *textToSearch;

            if (c == charToLookFor || c == 0)
                break;
        }

        return textToSearch;
    }

    /** Finds the character index of a given substring in another string, using
        a case-independent match.
        Returns -1 if the substring is not found.
    */
    template <typename CharPointerType1, typename CharPointerType2>
    static i32 indexOfIgnoreCase (CharPointerType1 haystack, const CharPointerType2 needle) noexcept
    {
        i32 index = 0;
        auto needleLength = (i32) needle.length();

        for (;;)
        {
            if (haystack.compareIgnoreCaseUpTo (needle, needleLength) == 0)
                return index;

            if (haystack.getAndAdvance() == 0)
                return -1;

            ++index;
        }
    }

    /** Finds the character index of a given character in another string.
        Returns -1 if the character is not found.
    */
    template <typename Type>
    static i32 indexOfChar (Type text, const t32 charToFind) noexcept
    {
        i32 i = 0;

        while (! text.isEmpty())
        {
            if (text.getAndAdvance() == charToFind)
                return i;

            ++i;
        }

        return -1;
    }

    /** Finds the character index of a given character in another string, using
        a case-independent match.
        Returns -1 if the character is not found.
    */
    template <typename Type>
    static i32 indexOfCharIgnoreCase (Type text, t32 charToFind) noexcept
    {
        charToFind = CharacterFunctions::toLowerCase (charToFind);
        i32 i = 0;

        while (! text.isEmpty())
        {
            if (text.toLowerCase() == charToFind)
                return i;

            ++text;
            ++i;
        }

        return -1;
    }

    /** Increments a pointer until it points to the first non-whitespace character
        in a string.

        If the string contains only whitespace, the pointer will point to the
        string's null terminator.
    */
    template <typename Type>
    static z0 incrementToEndOfWhitespace (Type& text) noexcept
    {
        while (text.isWhitespace())
            ++text;
    }

    /** Returns a pointer to the first non-whitespace character in a string.
        If the string contains only whitespace, this will return a pointer
        to its null terminator.
    */
    template <typename Type>
    static Type findEndOfWhitespace (Type text) noexcept
    {
        incrementToEndOfWhitespace (text);
        return text;
    }

    /** Returns a pointer to the first character in the string which is found in
        the breakCharacters string.
    */
    template <typename Type, typename BreakType>
    static Type findEndOfToken (Type text, BreakType breakCharacters, Type quoteCharacters)
    {
        t32 currentQuoteChar = 0;

        while (! text.isEmpty())
        {
            auto c = text.getAndAdvance();

            if (currentQuoteChar == 0 && breakCharacters.indexOf (c) >= 0)
            {
                --text;
                break;
            }

            if (quoteCharacters.indexOf (c) >= 0)
            {
                if (currentQuoteChar == 0)
                    currentQuoteChar = c;
                else if (currentQuoteChar == c)
                    currentQuoteChar = 0;
            }
        }

        return text;
    }

private:
    static f64 mulexp10 (f64 value, i32 exponent) noexcept;
};

} // namespace drx
