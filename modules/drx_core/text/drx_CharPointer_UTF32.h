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
    Wraps a pointer to a null-terminated UTF-32 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF8, CharPointer_UTF16

    @tags{Core}
*/
class CharPointer_UTF32  final
{
public:
    using CharType = t32;

    explicit CharPointer_UTF32 (const CharType* rawPointer) noexcept
        : data (const_cast<CharType*> (rawPointer))
    {
    }

    CharPointer_UTF32 (const CharPointer_UTF32& other) = default;

    CharPointer_UTF32& operator= (const CharPointer_UTF32& other) noexcept = default;

    CharPointer_UTF32& operator= (const CharType* text) noexcept
    {
        data = const_cast<CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    b8 operator== (CharPointer_UTF32 other) const noexcept     { return data == other.data; }
    b8 operator!= (CharPointer_UTF32 other) const noexcept     { return data != other.data; }
    b8 operator<= (CharPointer_UTF32 other) const noexcept     { return data <= other.data; }
    b8 operator<  (CharPointer_UTF32 other) const noexcept     { return data <  other.data; }
    b8 operator>= (CharPointer_UTF32 other) const noexcept     { return data >= other.data; }
    b8 operator>  (CharPointer_UTF32 other) const noexcept     { return data >  other.data; }

    /** Returns the address that this pointer is pointing to. */
    CharType* getAddress() const noexcept        { return data; }

    /** Returns the address that this pointer is pointing to. */
    operator const CharType*() const noexcept    { return data; }

    /** Возвращает true, если this pointer is pointing to a null character. */
    b8 isEmpty() const noexcept                { return *data == 0; }

    /** Возвращает true, если this pointer is not pointing to a null character. */
    b8 isNotEmpty() const noexcept             { return *data != 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    t32 operator*() const noexcept        { return *data; }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF32& operator++() noexcept
    {
        ++data;
        return *this;
    }

    /** Moves this pointer to the previous character in the string. */
    CharPointer_UTF32& operator--() noexcept
    {
        --data;
        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    t32 getAndAdvance() noexcept  { return *data++; }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF32 operator++ (i32) noexcept
    {
        auto temp (*this);
        ++data;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    CharPointer_UTF32& operator+= (i32 numToSkip) noexcept
    {
        data += numToSkip;
        return *this;
    }

    CharPointer_UTF32& operator-= (i32 numToSkip) noexcept
    {
        data -= numToSkip;
        return *this;
    }

    /** Returns the character at a given character index from the start of the string. */
    t32& operator[] (i32 characterIndex) const noexcept
    {
        return data [characterIndex];
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF32 operator+ (i32 numToSkip) const noexcept
    {
        return CharPointer_UTF32 (*this) += numToSkip;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF32 operator- (i32 numToSkip) const noexcept
    {
        return CharPointer_UTF32 (*this) -= numToSkip;
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    z0 write (t32 charToWrite) noexcept
    {
        *data++ = charToWrite;
    }

    z0 replaceChar (t32 newChar) noexcept
    {
        *data = newChar;
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    z0 writeNull() const noexcept
    {
        *data = 0;
    }

    /** Возвращает число символов в этой строке. */
    size_t length() const noexcept
    {
       #if DRX_NATIVE_WCHAR_IS_UTF32 && ! DRX_ANDROID
        return wcslen (data);
       #else
        size_t n = 0;
        while (data[n] != 0)
            ++n;
        return n;
       #endif
    }

    /** Возвращает число символов в этой строке, or the given value, whichever is lower. */
    size_t lengthUpTo (size_t maxCharsToCount) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Возвращает число символов в этой строке, or up to the given end pointer, whichever is lower. */
    size_t lengthUpTo (CharPointer_UTF32 end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const noexcept
    {
        return sizeof (CharType) * (length() + 1);
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (t32) noexcept
    {
        return sizeof (CharType);
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (CharPointer text) noexcept
    {
        return sizeof (CharType) * text.length();
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF32 findTerminatingNull() const noexcept
    {
        return CharPointer_UTF32 (data + length());
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    z0 writeAll (CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    z0 writeAll (CharPointer_UTF32 src) noexcept
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

   #if DRX_NATIVE_WCHAR_IS_UTF32 && ! DRX_ANDROID
    /** Compares this string with another one. */
    i32 compare (CharPointer_UTF32 other) const noexcept
    {
        return wcscmp (data, other.data);
    }
   #endif

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    i32 compareUpTo (CharPointer other, i32 maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    i32 compareIgnoreCase (CharPointer other) const
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    i32 compareIgnoreCaseUpTo (CharPointer other, i32 maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    i32 indexOf (CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    i32 indexOf (t32 charToFind) const noexcept
    {
        i32 i = 0;

        while (data[i] != 0)
        {
            if (data[i] == charToFind)
                return i;

            ++i;
        }

        return -1;
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    i32 indexOf (t32 charToFind, b8 ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Возвращает true, если the first character of this string is whitespace. */
    b8 isWhitespace() const                   { return CharacterFunctions::isWhitespace (*data) != 0; }
    /** Возвращает true, если the first character of this string is a digit. */
    b8 isDigit() const                        { return CharacterFunctions::isDigit (*data) != 0; }
    /** Возвращает true, если the first character of this string is a letter. */
    b8 isLetter() const                       { return CharacterFunctions::isLetter (*data) != 0; }
    /** Возвращает true, если the first character of this string is a letter or digit. */
    b8 isLetterOrDigit() const                { return CharacterFunctions::isLetterOrDigit (*data) != 0; }
    /** Возвращает true, если the first character of this string is upper-case. */
    b8 isUpperCase() const                    { return CharacterFunctions::isUpperCase (*data) != 0; }
    /** Возвращает true, если the first character of this string is lower-case. */
    b8 isLowerCase() const                    { return CharacterFunctions::isLowerCase (*data) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    t32 toUpperCase() const noexcept     { return CharacterFunctions::toUpperCase (*data); }
    /** Returns a lower-case version of the first character of this string. */
    t32 toLowerCase() const noexcept     { return CharacterFunctions::toLowerCase (*data); }

    /** Parses this string as a 32-bit integer. */
    i32 getIntValue32() const noexcept          { return CharacterFunctions::getIntValue <i32, CharPointer_UTF32> (*this); }
    /** Parses this string as a 64-bit integer. */
    z64 getIntValue64() const noexcept        { return CharacterFunctions::getIntValue <z64, CharPointer_UTF32> (*this); }

    /** Parses this string as a floating point f64. */
    f64 getDoubleValue() const noexcept      { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF32 findEndOfWhitespace() const noexcept   { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Move this pointer to the first non-whitespace character in the string. */
    z0 incrementToEndOfWhitespace() noexcept               { CharacterFunctions::incrementToEndOfWhitespace (*this); }

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
            const auto c = codeUnits[codeUnitIndex];

            if (c == 0)
                return true;

            if (! canRepresent (c))
                return false;
        }

        return true;
    }

    /** Atomically swaps this pointer for a new value, returning the previous value. */
    CharPointer_UTF32 atomicSwap (CharPointer_UTF32 newValue)
    {
        return CharPointer_UTF32 (reinterpret_cast<Atomic<CharType*>&> (data).exchange (newValue.data));
    }

private:
    CharType* data;
};

} // namespace drx
