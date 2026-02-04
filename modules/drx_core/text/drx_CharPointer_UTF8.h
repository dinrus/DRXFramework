
namespace drx
{

//==============================================================================
/**
    Wraps a pointer to a null-terminated UTF-8 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF16, CharPointer_UTF32

    @tags{Core}
*/
class CharPointer_UTF8  final
{
public:
    using CharType = t8;

    explicit CharPointer_UTF8 (const CharType* rawPointer) noexcept
        : data (const_cast<CharType*> (rawPointer))
    {
    }

    CharPointer_UTF8 (const CharPointer_UTF8& other) = default;

    CharPointer_UTF8& operator= (const CharPointer_UTF8& other) noexcept = default;

    CharPointer_UTF8& operator= (const CharType* text) noexcept
    {
        data = const_cast<CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    b8 operator== (CharPointer_UTF8 other) const noexcept      { return data == other.data; }
    b8 operator!= (CharPointer_UTF8 other) const noexcept      { return data != other.data; }
    b8 operator<= (CharPointer_UTF8 other) const noexcept      { return data <= other.data; }
    b8 operator<  (CharPointer_UTF8 other) const noexcept      { return data <  other.data; }
    b8 operator>= (CharPointer_UTF8 other) const noexcept      { return data >= other.data; }
    b8 operator>  (CharPointer_UTF8 other) const noexcept      { return data >  other.data; }

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
        auto byte_ = (i8) *data;

        if (byte_ >= 0)
            return (t32) (u8) byte_;

        u32 n = (u32) (u8) byte_;
        u32 mask = 0x7f;
        u32 bit = 0x40;
        i32 numExtraValues = 0;

        while ((n & bit) != 0 && bit > 0x8)
        {
            mask >>= 1;
            ++numExtraValues;
            bit >>= 1;
        }

        n &= mask;

        for (i32 i = 1; i <= numExtraValues; ++i)
        {
            auto nextByte = (u32) (u8) data[i];

            if ((nextByte & 0xc0) != 0x80)
                break;

            n <<= 6;
            n |= (nextByte & 0x3f);
        }

        return (t32) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF8& operator++() noexcept
    {
        jassert (*data != 0); // trying to advance past the end of the string?
        auto n = (i8) *data++;

        if (n < 0)
        {
            u8 bit = 0x40;

            while ((static_cast<u8> (n) & bit) != 0 && bit > 0x8)
            {
                ++data;
                bit = static_cast<u8> (bit >> 1);
            }
        }

        return *this;
    }

    /** Moves this pointer back to the previous character in the string. */
    CharPointer_UTF8& operator--() noexcept
    {
        i32 count = 0;

        while ((*--data & 0xc0) == 0x80 && ++count < 4)
        {}

        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    t32 getAndAdvance() noexcept
    {
        auto byte_ = (i8) *data++;

        if (byte_ >= 0)
            return (t32) (u8) byte_;

        u32 n = (u32) (u8) byte_;
        u32 mask = 0x7f;
        u32 bit = 0x40;
        i32 numExtraValues = 0;

        while ((n & bit) != 0 && bit > 0x8)
        {
            mask >>= 1;
            ++numExtraValues;
            bit >>= 1;
        }

        n &= mask;

        while (--numExtraValues >= 0)
        {
            auto nextByte = (u32) (u8) *data;

            if ((nextByte & 0xc0) != 0x80)
                break;

            ++data;
            n <<= 6;
            n |= (nextByte & 0x3f);
        }

        return (t32) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF8 operator++ (i32) noexcept
    {
        CharPointer_UTF8 temp (*this);
        ++*this;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    CharPointer_UTF8& operator+= (i32 numToSkip) noexcept
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
    CharPointer_UTF8& operator-= (i32 numToSkip) noexcept
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
    CharPointer_UTF8 operator+ (i32 numToSkip) const noexcept
    {
        return CharPointer_UTF8 (*this) += numToSkip;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF8 operator- (i32 numToSkip) const noexcept
    {
        return CharPointer_UTF8 (*this) -= numToSkip;
    }

    /** Returns the number of characters in this string. */
    size_t length() const noexcept
    {
        auto* d = data;
        size_t count = 0;

        for (;;)
        {
            auto n = (u32) (u8) *d++;

            if ((n & 0x80) != 0)
            {
                while ((*d & 0xc0) == 0x80)
                    ++d;
            }
            else if (n == 0)
                break;

            ++count;
        }

        return count;
    }

    /** Returns the number of characters in this string, or the given value, whichever is lower. */
    size_t lengthUpTo (const size_t maxCharsToCount) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Returns the number of characters in this string, or up to the given end pointer, whichever is lower. */
    size_t lengthUpTo (const CharPointer_UTF8 end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const noexcept
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6387)
        jassert (data != nullptr);
        return strlen (data) + 1;
        DRX_END_IGNORE_WARNINGS_MSVC
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (const t32 charToWrite) noexcept
    {
        size_t num = 1;
        auto c = (u32) charToWrite;

        if (c >= 0x80)
        {
            ++num;
            if (c >= 0x800)
            {
                ++num;
                if (c >= 0x10000)
                    ++num;
            }
        }

        return num;
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (CharPointer text) noexcept
    {
        size_t count = 0;

        while (auto n = text.getAndAdvance())
            count += getBytesRequiredFor (n);

        return count;
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF8 findTerminatingNull() const noexcept
    {
        return CharPointer_UTF8 (data + strlen (data));
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    z0 write (const t32 charToWrite) noexcept
    {
        auto c = (u32) charToWrite;

        if (c >= 0x80)
        {
            i32 numExtraBytes = 1;
            if (c >= 0x800)
            {
                ++numExtraBytes;
                if (c >= 0x10000)
                    ++numExtraBytes;
            }

            *data++ = (CharType) ((u32) (0xff << (7 - numExtraBytes)) | (c >> (numExtraBytes * 6)));

            while (--numExtraBytes >= 0)
                *data++ = (CharType) (0x80 | (0x3f & (c >> (numExtraBytes * 6))));
        }
        else
        {
            *data++ = (CharType) c;
        }
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    z0 writeNull() const noexcept
    {
        *data = 0;
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    z0 writeAll (const CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    z0 writeAll (const CharPointer_UTF8 src) noexcept
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
    size_t writeWithDestByteLimit (const CharPointer src, const size_t maxDestBytes) noexcept
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    z0 writeWithCharLimit (const CharPointer src, i32k maxChars) noexcept
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    i32 compare (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compare (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    i32 compareUpTo (const CharPointer other, i32k maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    i32 compareIgnoreCase (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one. */
    i32 compareIgnoreCase (const CharPointer_UTF8 other) const noexcept
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    i32 compareIgnoreCaseUpTo (const CharPointer other, i32k maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    i32 indexOf (const CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    i32 indexOf (const t32 charToFind) const noexcept
    {
        return CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    i32 indexOf (const t32 charToFind, const b8 ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Возвращает true, если the first character of this string is whitespace. */
    b8 isWhitespace() const noexcept          { return CharacterFunctions::isWhitespace ((t32) *(*this)); }
    /** Возвращает true, если the first character of this string is a digit. */
    b8 isDigit() const noexcept               { const CharType c = *data; return c >= '0' && c <= '9'; }
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
    i32 getIntValue32() const noexcept          { return atoi (data); }

    /** Parses this string as a 64-bit integer. */
    z64 getIntValue64() const noexcept
    {
       #if DRX_WINDOWS
        return _atoi64 (data);
       #else
        return atoll (data);
       #endif
    }

    /** Parses this string as a floating point f64. */
    f64 getDoubleValue() const noexcept                      { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF8 findEndOfWhitespace() const noexcept       { return CharacterFunctions::findEndOfWhitespace (*this); }

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
            const auto firstByte = (u8) codeUnits[codeUnitIndex];

            if (firstByte == 0)
                return true;

            if (CharacterFunctions::isAscii ((t32) firstByte))
                continue;

            auto numExtraBytes = [&]
            {
                if (firstByte < 0xc0)
                    return 0;

                if (firstByte < 0xe0)
                    return 1;

                if (firstByte <  0xf0)
                    return 2;

                if (firstByte <= 0xf4)
                    return 3;

                return 0;
            }();

            if (numExtraBytes == 0)
                return false;

            auto bytes = (u32) firstByte;

            while (numExtraBytes--)
            {
                if (++codeUnitIndex >= maxCodeUnitsToRead)
                    return false;

                bytes <<= 8;
                bytes |= (u32) (u8) codeUnits[codeUnitIndex];
            }

            if (constexpr u32 firstTwoByteCodePoint = 0xc280; bytes < firstTwoByteCodePoint)
                return false;

            if (constexpr u32 lastTwoByteCodePoint = 0xdfbf; bytes <= lastTwoByteCodePoint)
                continue;

            if (constexpr u32 firstThreeByteCodePoint = 0xe0a080; bytes < firstThreeByteCodePoint)
                return false;

            if (constexpr u32 firstSurrogateCodePoint = 0xeda080; bytes < firstSurrogateCodePoint)
                continue;

            if (constexpr u32 lastSurrogateCodePoint = 0xedbfbf; bytes <= lastSurrogateCodePoint)
                return false;

            if (constexpr u32 lastThreeByteCodePoint = 0xefbfbf; bytes <= lastThreeByteCodePoint)
                continue;

            if (constexpr u32 firstFourByteCodePoint = 0xf0908080; bytes < firstFourByteCodePoint)
                return false;

            if (constexpr u32 lastFourByteCodePoint = 0xf48fbfbf; bytes <= lastFourByteCodePoint)
                continue;

            return false;
        }

        return true;
    }

    /** Atomically swaps this pointer for a new value, returning the previous value. */
    CharPointer_UTF8 atomicSwap (const CharPointer_UTF8 newValue)
    {
        return CharPointer_UTF8 (reinterpret_cast<Atomic<CharType*>&> (data).exchange (newValue.data));
    }

    /** These values are the byte_-order mark (BOM) values for a UTF-8 stream. */
    enum
    {
        byteOrderMark1 = 0xef,
        byteOrderMark2 = 0xbb,
        byteOrderMark3 = 0xbf
    };

    /** Возвращает true, если the first three bytes in this pointer are the UTF8 byte_-order mark (BOM).
        The pointer must not be null, and must point to at least 3 valid bytes.
    */
    static b8 isByteOrderMark (ukk possibleByteOrder) noexcept
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (28182)
        jassert (possibleByteOrder != nullptr);
        auto c = static_cast<u8k*> (possibleByteOrder);

        return c[0] == (u8) byteOrderMark1
            && c[1] == (u8) byteOrderMark2
            && c[2] == (u8) byteOrderMark3;
        DRX_END_IGNORE_WARNINGS_MSVC
    }

private:
    CharType* data;
};

} // namespace drx
