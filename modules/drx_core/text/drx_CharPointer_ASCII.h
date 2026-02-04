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
    Оборачивает указатель на символьную строку ASCII, с окончанием на ноль,
    и предоставляет различные методы для оперирования над данными.

    Считается, что полноценная строка ASCII не содержит символов, выше 127.

    @see CharPointer_UTF8, CharPointer_UTF16, CharPointer_UTF32

    @tags{Core}
*/
class CharPointer_ASCII  final
{
public:
    using CharType = t8;

    explicit CharPointer_ASCII (const CharType* rawPointer) noexcept
        : data (const_cast<CharType*> (rawPointer))
    {
    }

    CharPointer_ASCII (const CharPointer_ASCII& other) = default;

    CharPointer_ASCII& operator= (const CharPointer_ASCII& other) noexcept = default;

    CharPointer_ASCII& operator= (const CharType* text) noexcept
    {
        data = const_cast<CharType*> (text);
        return *this;
    }

    /** Это сравнение указателей, действительный текст не сравнивается. */
    b8 operator== (CharPointer_ASCII other) const noexcept     { return data == other.data; }
    b8 operator!= (CharPointer_ASCII other) const noexcept     { return data != other.data; }
    b8 operator<= (CharPointer_ASCII other) const noexcept     { return data <= other.data; }
    b8 operator<  (CharPointer_ASCII other) const noexcept     { return data <  other.data; }
    b8 operator>= (CharPointer_ASCII other) const noexcept     { return data >= other.data; }
    b8 operator>  (CharPointer_ASCII other) const noexcept     { return data >  other.data; }

    /** Возвращает адрес, на который указывает этот указатель. */
    CharType* getAddress() const noexcept        { return data; }

    /** Возвращает адрес, на который указывает этот указатель. */
    operator const CharType*() const noexcept    { return data; }

    /** Возвращает true, если этот указатель указывает на нулевой символ. */
    b8 isEmpty() const noexcept                { return *data == 0; }

    /** Возвращает true, если если этот указатель не указывает на нулевой символ. */
    b8 isNotEmpty() const noexcept             { return *data != 0; }

    /** Возвращает символ unicode, на который указывает этот указатель. */
    t32 operator*() const noexcept        { return (t32) (u8) *data; }

    /** Перемещает этот указатель к следующему символу в строке. */
    CharPointer_ASCII& operator++() noexcept
    {
        ++data;
        return *this;
    }

    /** Перемещает этот указатель к предыдущему символу в строке. */
    CharPointer_ASCII& operator--() noexcept
    {
        --data;
        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    t32 getAndAdvance() noexcept  { return (t32) (u8) *data++; }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_ASCII operator++ (i32) noexcept
    {
        auto temp (*this);
        ++data;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    CharPointer_ASCII& operator+= (i32k numToSkip) noexcept
    {
        data += numToSkip;
        return *this;
    }

    CharPointer_ASCII& operator-= (i32k numToSkip) noexcept
    {
        data -= numToSkip;
        return *this;
    }

    /** Returns the character at a given character index from the start of the string. */
    t32 operator[] (i32k characterIndex) const noexcept
    {
        return (t32) (u8) data [characterIndex];
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_ASCII operator+ (i32k numToSkip) const noexcept
    {
        return CharPointer_ASCII (*this) += numToSkip;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_ASCII operator- (i32k numToSkip) const noexcept
    {
        return CharPointer_ASCII (*this) -= numToSkip;
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    z0 write (const t32 charToWrite) noexcept
    {
        *data++ = (t8) charToWrite;
    }

    z0 replaceChar (const t32 newChar) noexcept
    {
        *data = (t8) newChar;
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    z0 writeNull() const noexcept
    {
        *data = 0;
    }

    /** Возвращает число символов в этой строке. */
    size_t length() const noexcept
    {
        return (size_t) strlen (data);
    }

    /** Возвращает число символов в этой строке, либо заданное значение, смотря что меньше. */
    size_t lengthUpTo (const size_t maxCharsToCount) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Возвращает число символов в этой строке, либо до заданного указателя на конец,
     *  смотря что меньше. */
    size_t lengthUpTo (const CharPointer_ASCII end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Возвращает число байтов, используемых для представления этой строки.
        Сюда входит терминирующий символ нуля.
    */
    size_t sizeInBytes() const noexcept
    {
        return length() + 1;
    }

    /** Возвращает число байтов, которое потребуется для представления данного
        символа unicode в формате этой кодировки.
    */
    static size_t getBytesRequiredFor (const t32) noexcept
    {
        return 1;
    }

    /** Возвращает число байтов, которое потребуется для представления данного
        символа unicode в формате этой кодировки.
        В возвращаемое значение НЕ включается терминирующий нулевой символ.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (const CharPointer text) noexcept
    {
        return text.length();
    }

    /** Возвращает указатель на нулевой символ, терминирующий данную строку. */
    CharPointer_ASCII findTerminatingNull() const noexcept
    {
        return CharPointer_ASCII (data + length());
    }

    /** Копирует текст из источника в этот указатель, перемещая его по мере прохождения. */
    template <typename CharPointer>
    z0 writeAll (const CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Копирует текст из источника в этот указатель, перемещая его по мере прохождения.
        The maxDestBytes parameter specifies the maximum number of bytes that can be written
        to the destination buffer before stopping.
    */
    template <typename CharPointer>
    size_t writeWithDestByteLimit (const CharPointer src, const size_t maxDestBytes) noexcept
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** Копирует текст из источника в этот указатель, перемещая его по мере прохождения.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    z0 writeWithCharLimit (const CharPointer src, i32k maxChars) noexcept
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Сравнивает эту строку с другой. */
    template <typename CharPointer>
    i32 compare (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compare (*this, other);
    }

    /** Сравнивает эту строку с другой. */
    i32 compare (const CharPointer_ASCII other) const noexcept
    {
        return strcmp (data, other.data);
    }

    /** Сравнивает эту строку с другой, до указанного числа символов. */
    template <typename CharPointer>
    i32 compareUpTo (const CharPointer other, i32k maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Сравнивает эту строку с другой, до указанного числа символов. */
    i32 compareUpTo (const CharPointer_ASCII other, i32k maxChars) const noexcept
    {
        return strncmp (data, other.data, (size_t) maxChars);
    }

    /** Сравнивает эту строку с другой. */
    template <typename CharPointer>
    i32 compareIgnoreCase (const CharPointer other) const
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    i32 compareIgnoreCase (const CharPointer_ASCII other) const
    {
       #if DRX_WINDOWS && DRX_CLANG
        return CharacterFunctions::compareIgnoreCase (*this, other);
       #elif DRX_WINDOWS
        return stricmp (data, other.data);
       #else
        return strcasecmp (data, other.data);
       #endif
    }

    /** Сравнивает эту строку с другой, до указанного числа символов. */
    template <typename CharPointer>
    i32 compareIgnoreCaseUpTo (const CharPointer other, i32k maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

    /** Возвращает индекс символа подстроки, либо -1, если он не найден. */
    template <typename CharPointer>
    i32 indexOf (const CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Возвращает индекс символа unicode, либо -1, если он не найден. */
    i32 indexOf (const t32 charToFind) const noexcept
    {
        i32 i = 0;

        while (data[i] != 0)
        {
            if (data[i] == (t8) charToFind)
                return i;

            ++i;
        }

        return -1;
    }

    /** Возвращает индекс символа unicode, либо -1, если он не найден. */
    i32 indexOf (const t32 charToFind, const b8 ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Возвращает true, если первый символ этой строки пробельный. */
    b8 isWhitespace() const                   { return CharacterFunctions::isWhitespace (*data) != 0; }
    /** Возвращает true, если первый символ этой строки цифровой. */
    b8 isDigit() const                        { return CharacterFunctions::isDigit (*data) != 0; }
    /** Возвращает true, если первый символ этой строки буквенный. */
    b8 isLetter() const                       { return CharacterFunctions::isLetter (*data) != 0; }
    /** Возвращает true, если первый символ этой строки буква или цифра. */
    b8 isLetterOrDigit() const                { return CharacterFunctions::isLetterOrDigit (*data) != 0; }
    /** Возвращает true, если первый символ этой строки заглавный. */
    b8 isUpperCase() const                    { return CharacterFunctions::isUpperCase ((t32) (u8) *data) != 0; }
    /** Возвращает true, если первый символ этой строки прописной. */
    b8 isLowerCase() const                    { return CharacterFunctions::isLowerCase ((t32) (u8) *data) != 0; }

    /** Возвращает заглавную версию первого символа этой строки. */
    t32 toUpperCase() const noexcept     { return CharacterFunctions::toUpperCase ((t32) (u8) *data); }
    /** Возвращает прописную версию первого символа этой строки. */
    t32 toLowerCase() const noexcept     { return CharacterFunctions::toLowerCase ((t32) (u8) *data); }

    /** Разбирает эту строку как 32-битное ЦЧ. */
    i32 getIntValue32() const noexcept          { return atoi (data); }

    /** Разбирает эту строку как 64-битное ЦЧ. */
    z64 getIntValue64() const noexcept
    {
       #if DRX_LINUX || DRX_BSD || DRX_ANDROID
        return atoll (data);
       #elif DRX_WINDOWS
        return _atoi64 (data);
       #else
        return CharacterFunctions::getIntValue <z64, CharPointer_ASCII> (*this);
       #endif
    }

    /** Разбирает эту строку как f64 с ПЗ. */
    f64 getDoubleValue() const noexcept                      { return CharacterFunctions::getDoubleValue (*this); }

    /** Возвращает первый непробельный символ в этой строке. */
    CharPointer_ASCII findEndOfWhitespace() const noexcept      { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Перемещает указатель к первому непробельному символу в этой строке. */
    z0 incrementToEndOfWhitespace() noexcept                  { CharacterFunctions::incrementToEndOfWhitespace (*this); }

    /** Возвращает true, если заданный символ unicode можно представить в этой кодировке. */
    static b8 canRepresent (t32 character) noexcept
    {
        return CharacterFunctions::isAscii (character);
    }

    /** Возвращает true, если данные содержат полноценную строку в  данной кодировке. */
    static b8 isValidString (const CharType* dataToTest, i32 maxBytesToRead)
    {
        while (--maxBytesToRead >= 0)
        {
            if (((i8) *dataToTest) <= 0)
                return *dataToTest == 0;

            ++dataToTest;
        }

        return true;
    }

private:
    CharType* data;
};

} // namespace drx
