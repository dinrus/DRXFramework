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
/**
    Wraps a pointer to a null-terminated UTF-16 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF8, CharPointer_UTF32

    @tags{Core}
*/
class CharPointer_UTF16  final
{
public:
   #if DRX_NATIVE_WCHAR_IS_UTF16
    using CharType = wchar_t;
   #else
    using CharType = i16;
   #endif

    explicit CharPointer_UTF16 (const CharType* rawPointer) noexcept
        : data (const_cast<CharType*> (rawPointer))
    {
    }

    CharPointer_UTF16 (const CharPointer_UTF16& other) = default;

    CharPointer_UTF16& operator= (const CharPointer_UTF16& other) noexcept = default;

    CharPointer_UTF16& operator= (const CharType* text) noexcept
    {
        data = const_cast<CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    b8 operator== (CharPointer_UTF16 other) const noexcept     { return data == other.data; }
    b8 operator!= (CharPointer_UTF16 other) const noexcept     { return data != other.data; }
    b8 operator<= (CharPointer_UTF16 other) const noexcept     { return data <= other.data; }
    b8 operator<  (CharPointer_UTF16 other) const noexcept     { return data <  other.data; }
    b8 operator>= (CharPointer_UTF16 other) const noexcept     { return data >= other.data; }
    b8 operator>  (CharPointer_UTF16 other) const noexcept     { return data >  other.data; }

    /** Returns the address that this pointer is pointing to. */
    CharType* getAddress() const noexcept        { return data; }

    /** Returns the address that this pointer is pointing to. */
    operator const CharType*() const noexcept    { return data; }

    /** Возвращает true, если this pointer is pointing to a null character. */
    b8 isEmpty() const noexcept                { return *data == 0; }

    /** Возвращает true, если this pointer is not pointing to a null character. */
    b8 isNotEmpty() const noexcept             { return *data != 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    t32 operator*() const noexcept
    {
        auto n = (u32) (u16) *data;

        if (n >= 0xd800 && n <= 0xdfff && ((u32) (u16) data[1]) >= 0xdc00)
            n = 0x10000 + (((n - 0xd800) << 10) | (((u32) (u16) data[1]) - 0xdc00));

        return (t32) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF16& operator++() noexcept
    {
        auto n = (u32) (u16) *data++;

        if (n >= 0xd800 && n <= 0xdfff && ((u32) (u16) *data) >= 0xdc00)
            ++data;

        return *this;
    }

    /** Moves this pointer back to the previous character in the string. */
    CharPointer_UTF16& operator--() noexcept
    {
        auto n = (u32) (u16) (*--data);

        if (n >= 0xdc00 && n <= 0xdfff)
            --data;

        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    t32 getAndAdvance() noexcept
    {
        auto n = (u32) (u16) *data++;

        if (n >= 0xd800 && n <= 0xdfff && ((u32) (u16) *data) >= 0xdc00)
            n = 0x10000 + ((((n - 0xd800) << 10) | (((u32) (u16) *data++) - 0xdc00)));

        return (t32) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF16 operator++ (i32) noexcept
    {
        auto temp (*this);
        ++*this;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    CharPointer_UTF16& operator+= (i32 numToSkip) noexcept
    {
        if (numToSkip < 0)
        {
            while (++numToSkip <= 0)
                --*this;
        }
        else
        {
            while (--numToSkip >= 0)
                ++*this;
        }

        return *this;
    }

    /** Moves this pointer backwards by the specified number of characters. */
    CharPointer_UTF16& operator-= (i32 numToSkip) noexcept
    {
        return operator+= (-numToSkip);
    }

    /** Returns the character at a given character index from the start of the string. */
    t32 operator[] (i32 characterIndex) const noexcept
    {
        auto p (*this);
        p += characterIndex;
        return *p;
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF16 operator+ (i32 numToSkip) const noexcept
    {
        return CharPointer_UTF16 (*this) += numToSkip;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF16 operator- (i32 numToSkip) const noexcept
    {
        return CharPointer_UTF16 (*this) -= numToSkip;
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    z0 write (t32 charToWrite) noexcept
    {
        if (charToWrite >= 0x10000)
        {
            charToWrite -= 0x10000;
            *data++ = (CharType) (0xd800 + (charToWrite >> 10));
            *data++ = (CharType) (0xdc00 + (charToWrite & 0x3ff));
        }
        else
        {
            *data++ = (CharType) charToWrite;
        }
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    z0 writeNull() const noexcept
    {
        *data = 0;
    }

    /** Returns the number of characters in this string. */
    size_t length() const noexcept
    {
        auto* d = data;
        size_t count = 0;

        for (;;)
        {
            auto n = (u32) (u16) *d++;

            if (n >= 0xd800 && n <= 0xdfff)
            {
                if (*d++ == 0)
                    break;
            }
            else if (n == 0)
                break;

            ++count;
        }

        return count;
    }

    /** Returns the number of characters in this string, or the given value, whichever is lower. */
    size_t lengthUpTo (size_t maxCharsToCount) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Returns the number of characters in this string, or up to the given end pointer, whichever is lower. */
    size_t lengthUpTo (CharPointer_UTF16 end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const noexcept
    {
        return sizeof (CharType) * (findNullIndex (data) + 1);
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (t32 charToWrite) noexcept
    {
        return (charToWrite >= 0x10000) ? (sizeof (CharType) * 2) : sizeof (CharType);
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (CharPointer text) noexcept
    {
        size_t count = 0;
        t32 n;

        while ((n = text.getAndAdvance()) != 0)
            count += getBytesRequiredFor (n);

        return count;
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF16 findTerminatingNull() const noexcept
    {
        auto* t = data;

        while (*t != 0)
            ++t;

        return CharPointer_UTF16 (t);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    z0 writeAll (CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    z0 writeAll (CharPointer_UTF16 src) noexcept
    {
        auto* s = src.data;

        while ((*data = *s) != 0)
        {
            ++data;
            ++s;
        }
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxDestBytes parameter specifies the maximum number of bytes that can be written
        to the destination buffer before stopping.
    */
    template <typename CharPointer>
    size_t writeWithDestByteLimit (CharPointer src, size_t maxDestBytes) noexcept
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    z0 writeWithCharLimit (CharPointer src, i32 maxChars) noexcept
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    i32 compare (CharPointer other) const noexcept
    {
        return CharacterFunctions::compare (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    i32 compareUpTo (CharPointer other, i32 maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    i32 compareIgnoreCase (CharPointer other) const noexcept
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    i32 compareIgnoreCaseUpTo (CharPointer other, i32 maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

   #if DRX_MSVC && ! defined (DOXYGEN)
    i32 compareIgnoreCase (CharPointer_UTF16 other) const noexcept
    {
        return _wcsicmp (data, other.data);
    }

    i32 compareIgnoreCaseUpTo (CharPointer_UTF16 other, i32 maxChars) const noexcept
    {
        return _wcsnicmp (data, other.data, (size_t) maxChars);
    }

    i32 indexOf (CharPointer_UTF16 stringToFind) const noexcept
    {
        const CharType* const t = wcsstr (data, stringToFind.getAddress());
        return t == nullptr ? -1 : (i32) (t - data);
    }
   #endif

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    i32 indexOf (CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    i32 indexOf (t32 charToFind) const noexcept
    {
        return CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    i32 indexOf (t32 charToFind, b8 ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Возвращает true, если the first character of this string is whitespace. */
    b8 isWhitespace() const noexcept          { return CharacterFunctions::isWhitespace (operator*()) != 0; }
    /** Возвращает true, если the first character of this string is a digit. */
    b8 isDigit() const noexcept               { return CharacterFunctions::isDigit (operator*()) != 0; }
    /** Возвращает true, если the first character of this string is a letter. */
    b8 isLetter() const noexcept              { return CharacterFunctions::isLetter (operator*()) != 0; }
    /** Возвращает true, если the first character of this string is a letter or digit. */
    b8 isLetterOrDigit() const noexcept       { return CharacterFunctions::isLetterOrDigit (operator*()) != 0; }
    /** Возвращает true, если the first character of this string is upper-case. */
    b8 isUpperCase() const noexcept           { return CharacterFunctions::isUpperCase (operator*()) != 0; }
    /** Возвращает true, если the first character of this string is lower-case. */
    b8 isLowerCase() const noexcept           { return CharacterFunctions::isLowerCase (operator*()) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    t32 toUpperCase() const noexcept     { return CharacterFunctions::toUpperCase (operator*()); }
    /** Returns a lower-case version of the first character of this string. */
    t32 toLowerCase() const noexcept     { return CharacterFunctions::toLowerCase (operator*()); }

    /** Parses this string as a 32-bit integer. */
    i32 getIntValue32() const noexcept
    {
       #if DRX_MSVC
        return _wtoi (data);
       #else
        return CharacterFunctions::getIntValue<i32, CharPointer_UTF16> (*this);
       #endif
    }

    /** Parses this string as a 64-bit integer. */
    z64 getIntValue64() const noexcept
    {
       #if DRX_MSVC
        return _wtoi64 (data);
       #else
        return CharacterFunctions::getIntValue<z64, CharPointer_UTF16> (*this);
       #endif
    }

    /** Parses this string as a floating point f64. */
    f64 getDoubleValue() const noexcept                      { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF16 findEndOfWhitespace() const noexcept      { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Move this pointer to the first non-whitespace character in the string. */
    z0 incrementToEndOfWhitespace() noexcept                  { CharacterFunctions::incrementToEndOfWhitespace (*this); }

    /** Возвращает true, если the given unicode character can be represented in this encoding. */
    static b8 canRepresent (t32 character) noexcept
    {
        return CharacterFunctions::isNonSurrogateCodePoint (character);
    }

    /** Возвращает true, если this data contains a valid string in this encoding. */
    static b8 isValidString (const CharType* codeUnits, i32 maxBytesToRead)
    {
        const auto maxCodeUnitsToRead = (size_t) maxBytesToRead / sizeof (CharType);

        for (size_t codeUnitIndex = 0; codeUnitIndex < maxCodeUnitsToRead; ++codeUnitIndex)
        {
            const auto c = toCodePoint (codeUnits[codeUnitIndex]);

            if (c == 0)
                return true;

            if (canRepresent (c))
                continue;

            if (! CharacterFunctions::isHighSurrogate (c))
                return false;

            if (++codeUnitIndex >= maxCodeUnitsToRead)
                return false;

            if (! CharacterFunctions::isLowSurrogate (toCodePoint (codeUnits[codeUnitIndex])))
                return false;
        }

        return true;
    }

    /** Atomically swaps this pointer for a new value, returning the previous value. */
    CharPointer_UTF16 atomicSwap (CharPointer_UTF16 newValue)
    {
        return CharPointer_UTF16 (reinterpret_cast<Atomic<CharType*>&> (data).exchange (newValue.data));
    }

    /** These values are the byte-order-mark (BOM) values for a UTF-16 stream. */
    enum
    {
        byteOrderMarkBE1 = 0xfe,
        byteOrderMarkBE2 = 0xff,
        byteOrderMarkLE1 = 0xff,
        byteOrderMarkLE2 = 0xfe
    };

    /** Возвращает true, если the first pair of bytes in this pointer are the UTF16 byte-order mark (big endian).
        The pointer must not be null, and must contain at least two valid bytes.
    */
    static b8 isByteOrderMarkBigEndian (ukk possibleByteOrder) noexcept
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (28182)
        jassert (possibleByteOrder != nullptr);
        auto c = static_cast<u8k*> (possibleByteOrder);

        return c[0] == (u8) byteOrderMarkBE1
            && c[1] == (u8) byteOrderMarkBE2;
        DRX_END_IGNORE_WARNINGS_MSVC
    }

    /** Возвращает true, если the first pair of bytes in this pointer are the UTF16 byte-order mark (little endian).
        The pointer must not be null, and must contain at least two valid bytes.
    */
    static b8 isByteOrderMarkLittleEndian (ukk possibleByteOrder) noexcept
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (28182)
        jassert (possibleByteOrder != nullptr);
        auto c = static_cast<u8k*> (possibleByteOrder);

        return c[0] == (u8) byteOrderMarkLE1
            && c[1] == (u8) byteOrderMarkLE2;
        DRX_END_IGNORE_WARNINGS_MSVC
    }

private:
    CharType* data;

    static u32 findNullIndex (const CharType* t) noexcept
    {
        u32 n = 0;

        while (t[n] != 0)
            ++n;

        return n;
    }

    static inline u32 toUint32 (CharType c) noexcept
    {
        return (u32) (u16) c;
    }

    static inline t32 toCodePoint (CharType c) noexcept
    {
        return (t32) toUint32 (c);
    }
};

} // namespace drx
