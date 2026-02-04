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

NewLine newLine;

#if defined (DRX_STRINGS_ARE_UNICODE) && ! DRX_STRINGS_ARE_UNICODE
 #error "DRX_STRINGS_ARE_UNICODE is deprecated! All strings are now unicode by default."
#endif

#if DRX_NATIVE_WCHAR_IS_UTF8
 using CharPointer_wchar_t = CharPointer_UTF8;
#elif DRX_NATIVE_WCHAR_IS_UTF16
 using CharPointer_wchar_t = CharPointer_UTF16;
#else
 using CharPointer_wchar_t = CharPointer_UTF32;
#endif

static CharPointer_wchar_t castToCharPointer_wchar_t (ukk t) noexcept
{
    return CharPointer_wchar_t (static_cast<const CharPointer_wchar_t::CharType*> (t));
}

//==============================================================================
struct StringHolder
{
    using CharPointerType  = Txt::CharPointerType;
    using CharType         = Txt::CharPointerType::CharType;

    std::atomic<i32> refCount { 0 };
    size_t allocatedNumBytes = sizeof (CharType);
    CharType text[1] { 0 };
};

constexpr StringHolder emptyString;

//==============================================================================
class StringHolderUtils
{
public:
    using CharPointerType = StringHolder::CharPointerType;
    using CharType        = StringHolder::CharType;

    static CharPointerType createUninitialisedBytes (size_t numBytes)
    {
        numBytes = (numBytes + 3) & ~(size_t) 3;
        auto* bytes = new t8 [sizeof (StringHolder) - sizeof (CharType) + numBytes];
        auto s = unalignedPointerCast<StringHolder*> (bytes);
        s->refCount = 0;
        s->allocatedNumBytes = numBytes;
        return CharPointerType (unalignedPointerCast<CharType*> (bytes + offsetof (StringHolder, text)));
    }

    template <class CharPointer>
    static CharPointerType createFromCharPointer (const CharPointer text)
    {
        if (text.getAddress() == nullptr || text.isEmpty())
            return CharPointerType (emptyString.text);

        auto bytesNeeded = sizeof (CharType) + CharPointerType::getBytesRequiredFor (text);
        auto dest = createUninitialisedBytes (bytesNeeded);
        CharPointerType (dest).writeAll (text);
        return dest;
    }

    template <class CharPointer>
    static CharPointerType createFromCharPointer (const CharPointer text, size_t maxChars)
    {
        if (text.getAddress() == nullptr || text.isEmpty() || maxChars == 0)
            return CharPointerType (emptyString.text);

        auto end = text;
        size_t numChars = 0;
        size_t bytesNeeded = sizeof (CharType);

        while (numChars < maxChars && ! end.isEmpty())
        {
            bytesNeeded += CharPointerType::getBytesRequiredFor (end.getAndAdvance());
            ++numChars;
        }

        auto dest = createUninitialisedBytes (bytesNeeded);
        CharPointerType (dest).writeWithCharLimit (text, (i32) numChars + 1);
        return dest;
    }

    template <class CharPointer>
    static CharPointerType createFromCharPointer (const CharPointer start, const CharPointer end)
    {
        if (start.getAddress() == nullptr || start.isEmpty())
            return CharPointerType (emptyString.text);

        auto e = start;
        i32 numChars = 0;
        auto bytesNeeded = sizeof (CharType);

        while (e < end && ! e.isEmpty())
        {
            bytesNeeded += CharPointerType::getBytesRequiredFor (e.getAndAdvance());
            ++numChars;
        }

        auto dest = createUninitialisedBytes (bytesNeeded);
        CharPointerType (dest).writeWithCharLimit (start, numChars + 1);
        return dest;
    }

    static CharPointerType createFromCharPointer (const CharPointerType start, const CharPointerType end)
    {
        if (start.getAddress() == nullptr || start.isEmpty())
            return CharPointerType (emptyString.text);

        auto numBytes = (size_t) (reinterpret_cast<tukk> (end.getAddress())
                                   - reinterpret_cast<tukk> (start.getAddress()));
        auto dest = createUninitialisedBytes (numBytes + sizeof (CharType));
        memcpy (dest.getAddress(), start, numBytes);
        dest.getAddress()[numBytes / sizeof (CharType)] = 0;
        return dest;
    }

    static CharPointerType createFromFixedLength (tukk const src, const size_t numChars)
    {
        auto dest = createUninitialisedBytes (numChars * sizeof (CharType) + sizeof (CharType));
        CharPointerType (dest).writeWithCharLimit (CharPointer_UTF8 (src), (i32) (numChars + 1));
        return dest;
    }

    //==============================================================================
    static z0 retain (const CharPointerType text) noexcept
    {
        auto* b = bufferFromText (text);

        if (! isEmptyString (b))
            ++(b->refCount);
    }

    static z0 release (StringHolder* const b) noexcept
    {
        if (! isEmptyString (b))
            if (--(b->refCount) == -1)
                delete[] reinterpret_cast<tuk> (b);
    }

    static z0 release (const CharPointerType text) noexcept
    {
        release (bufferFromText (text));
    }

    static i32 getReferenceCount (const CharPointerType text) noexcept
    {
        return bufferFromText (text)->refCount + 1;
    }

    //==============================================================================
    static CharPointerType makeUniqueWithByteSize (const CharPointerType text, size_t numBytes)
    {
        auto* b = bufferFromText (text);

        if (isEmptyString (b))
        {
            auto newText = createUninitialisedBytes (numBytes);
            newText.writeNull();
            return newText;
        }

        if (b->allocatedNumBytes >= numBytes && b->refCount <= 0)
            return text;

        auto newText = createUninitialisedBytes (jmax (b->allocatedNumBytes, numBytes));
        memcpy (newText.getAddress(), text.getAddress(), b->allocatedNumBytes);
        release (b);

        return newText;
    }

    static size_t getAllocatedNumBytes (const CharPointerType text) noexcept
    {
        return bufferFromText (text)->allocatedNumBytes;
    }

private:
    StringHolderUtils() = delete;
    ~StringHolderUtils() = delete;

    static StringHolder* bufferFromText (const CharPointerType charPtr) noexcept
    {
        return unalignedPointerCast<StringHolder*> (unalignedPointerCast<tuk> (charPtr.getAddress()) - offsetof (StringHolder, text));
    }

    static b8 isEmptyString (StringHolder* other)
    {
        return other == &emptyString;
    }

    z0 compileTimeChecks()
    {
        // Let me know if any of these assertions fail on your system!
       #if DRX_NATIVE_WCHAR_IS_UTF8
        static_assert (sizeof (wchar_t) == 1, "DRX_NATIVE_WCHAR_IS_* macro has incorrect value");
       #elif DRX_NATIVE_WCHAR_IS_UTF16
        static_assert (sizeof (wchar_t) == 2, "DRX_NATIVE_WCHAR_IS_* macro has incorrect value");
       #elif DRX_NATIVE_WCHAR_IS_UTF32
        static_assert (sizeof (wchar_t) == 4, "DRX_NATIVE_WCHAR_IS_* macro has incorrect value");
       #else
        #error "native wchar_t size is unknown"
       #endif
    }
};

//==============================================================================
Txt::Txt() noexcept  : text (emptyString.text)
{
}

Txt::~Txt() noexcept
{
    StringHolderUtils::release (text);
}

Txt::Txt (const Txt& other) noexcept   : text (other.text)
{
    StringHolderUtils::retain (text);
}

z0 Txt::swapWith (Txt& other) noexcept
{
    std::swap (text, other.text);
}

z0 Txt::clear() noexcept
{
    StringHolderUtils::release (text);
    text = emptyString.text;
}

Txt& Txt::operator= (const Txt& other) noexcept
{
    StringHolderUtils::retain (other.text);
    StringHolderUtils::release (text.atomicSwap (other.text));
    return *this;
}

Txt::Txt (Txt&& other) noexcept   : text (other.text)
{
    other.text = emptyString.text;
}

Txt& Txt::operator= (Txt&& other) noexcept
{
    std::swap (text, other.text);
    return *this;
}

inline Txt::PreallocationBytes::PreallocationBytes (const size_t num) noexcept : numBytes (num) {}

Txt::Txt (const PreallocationBytes& preallocationSize)
    : text (StringHolderUtils::createUninitialisedBytes (preallocationSize.numBytes + sizeof (CharPointerType::CharType)))
{
}

z0 Txt::preallocateBytes (const size_t numBytesNeeded)
{
    text = StringHolderUtils::makeUniqueWithByteSize (text, numBytesNeeded + sizeof (CharPointerType::CharType));
}

i32 Txt::getReferenceCount() const noexcept
{
    return StringHolderUtils::getReferenceCount (text);
}

//==============================================================================
Txt::Txt (tukk const t)
    : text (StringHolderUtils::createFromCharPointer (CharPointer_ASCII (t)))
{
    /*  If you get an assertion here, then you're trying to create a string from 8-bit data
        that contains values greater than 127. These can NOT be correctly converted to unicode
        because there's no way for the Txt class to know what encoding was used to
        create them. The source data could be UTF-8, ASCII or one of many local code-pages.

        To get around this problem, you must be more explicit when you pass an ambiguous 8-bit
        string to the Txt class - so for example if your source data is actually UTF-8,
        you'd call Txt (CharPointer_UTF8 ("my utf8 string..")), and it would be able to
        correctly convert the multi-byte characters to unicode. It's *highly* recommended that
        you use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent these strings in a way that isn't dependent on
        the compiler, source code editor and platform.

        Note that the Projucer has a handy string literal generator utility that will convert
        any unicode string to a valid C++ string literal, creating ascii escape sequences that will
        work in any compiler.
    */
    jassert (t == nullptr || CharPointer_ASCII::isValidString (t, std::numeric_limits<i32>::max()));
}

Txt::Txt (tukk const t, const size_t maxChars)
    : text (StringHolderUtils::createFromCharPointer (CharPointer_ASCII (t), maxChars))
{
    /*  If you get an assertion here, then you're trying to create a string from 8-bit data
        that contains values greater than 127. These can NOT be correctly converted to unicode
        because there's no way for the Txt class to know what encoding was used to
        create them. The source data could be UTF-8, ASCII or one of many local code-pages.

        To get around this problem, you must be more explicit when you pass an ambiguous 8-bit
        string to the Txt class - so for example if your source data is actually UTF-8,
        you'd call Txt (CharPointer_UTF8 ("my utf8 string..")), and it would be able to
        correctly convert the multi-byte characters to unicode. It's *highly* recommended that
        you use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent these strings in a way that isn't dependent on
        the compiler, source code editor and platform.

        Note that the Projucer has a handy string literal generator utility that will convert
        any unicode string to a valid C++ string literal, creating ascii escape sequences that will
        work in any compiler.
    */
    jassert (t == nullptr || CharPointer_ASCII::isValidString (t, (i32) maxChars));
}

Txt::Txt (const wchar_t* const t)      : text (StringHolderUtils::createFromCharPointer (castToCharPointer_wchar_t (t))) {}
Txt::Txt (const CharPointer_UTF8  t)   : text (StringHolderUtils::createFromCharPointer (t)) {}
Txt::Txt (const CharPointer_UTF16 t)   : text (StringHolderUtils::createFromCharPointer (t)) {}
Txt::Txt (const CharPointer_UTF32 t)   : text (StringHolderUtils::createFromCharPointer (t)) {}
Txt::Txt (const CharPointer_ASCII t)   : text (StringHolderUtils::createFromCharPointer (t)) {}

Txt::Txt (CharPointer_UTF8  t, size_t maxChars)   : text (StringHolderUtils::createFromCharPointer (t, maxChars)) {}
Txt::Txt (CharPointer_UTF16 t, size_t maxChars)   : text (StringHolderUtils::createFromCharPointer (t, maxChars)) {}
Txt::Txt (CharPointer_UTF32 t, size_t maxChars)   : text (StringHolderUtils::createFromCharPointer (t, maxChars)) {}
Txt::Txt (const wchar_t* t, size_t maxChars)      : text (StringHolderUtils::createFromCharPointer (castToCharPointer_wchar_t (t), maxChars)) {}

#if __cpp_char8_t
Txt::Txt (const char8_t* const t) : Txt (CharPointer_UTF8 (reinterpret_cast<tukk> (t)))
{
    /*  If you get an assertion here, then you're trying to create a string using the standard C++
        type for UTF-8 character representation, but the data consists of invalid UTF-8 characters!
    */
    jassert (t == nullptr || CharPointer_UTF8::isValidString (reinterpret_cast<tukk> (t), std::numeric_limits<i32>::max()));
}

Txt::Txt (const char8_t* t, size_t maxChars) : Txt (CharPointer_UTF8 (reinterpret_cast<tukk> (t)), maxChars)
{
    /*  If you get an assertion here, then you're trying to create a string using the standard C++
        type for UTF-8 character representation, but the data consists of invalid UTF-8 characters!
    */
    jassert (t == nullptr || CharPointer_UTF8::isValidString (reinterpret_cast<tukk> (t), (i32) maxChars));
}
#endif

Txt::Txt (CharPointer_UTF8  start, CharPointer_UTF8  end)  : text (StringHolderUtils::createFromCharPointer (start, end)) {}
Txt::Txt (CharPointer_UTF16 start, CharPointer_UTF16 end)  : text (StringHolderUtils::createFromCharPointer (start, end)) {}
Txt::Txt (CharPointer_UTF32 start, CharPointer_UTF32 end)  : text (StringHolderUtils::createFromCharPointer (start, end)) {}

Txt::Txt (const std::string& s) : text (StringHolderUtils::createFromFixedLength (s.data(), s.size())) {}
Txt::Txt (StringRef s)          : text (StringHolderUtils::createFromCharPointer (s.text)) {}

Txt Txt::charToString (t32 character)
{
    Txt result (PreallocationBytes (CharPointerType::getBytesRequiredFor (character)));
    CharPointerType t (result.text);
    t.write (character);
    t.writeNull();
    return result;
}

//==============================================================================
namespace NumberToStringConverters
{
    enum
    {
        charsNeededForInt = 32,
        charsNeededForDouble = 48
    };

    template <typename Type>
    static tuk printDigits (tuk t, Type v) noexcept
    {
        *--t = 0;

        do
        {
            *--t = static_cast<t8> ('0' + (t8) (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    // pass in a pointer to the END of a buffer..
    static tuk numberToString (tuk t, z64 n) noexcept
    {
        if (n >= 0)
            return printDigits (t, static_cast<zu64> (n));

        // NB: this needs to be careful not to call -std::numeric_limits<z64>::min(),
        // which has undefined behaviour
        t = printDigits (t, static_cast<zu64> (-(n + 1)) + 1);
        *--t = '-';
        return t;
    }

    static tuk numberToString (tuk t, zu64 v) noexcept
    {
        return printDigits (t, v);
    }

    static tuk numberToString (tuk t, i32 n) noexcept
    {
        if (n >= 0)
            return printDigits (t, static_cast<u32> (n));

        // NB: this needs to be careful not to call -std::numeric_limits<i32>::min(),
        // which has undefined behaviour
        t = printDigits (t, static_cast<u32> (-(n + 1)) + 1);
        *--t = '-';
        return t;
    }

    static tuk numberToString (tuk t, u32 v) noexcept
    {
        return printDigits (t, v);
    }

    static tuk numberToString (tuk t, i64 n) noexcept
    {
        if (n >= 0)
            return printDigits (t, static_cast<u64> (n));

        t = printDigits (t, static_cast<u64> (-(n + 1)) + 1);
        *--t = '-';
        return t;
    }

    static tuk numberToString (tuk t, u64 v) noexcept
    {
        return printDigits (t, v);
    }

    struct StackArrayStream final : public std::basic_streambuf<t8, std::char_traits<t8>>
    {
        explicit StackArrayStream (tuk d)
        {
            static const std::locale classicLocale (std::locale::classic());
            imbue (classicLocale);
            setp (d, d + charsNeededForDouble);
        }

        size_t writeDouble (f64 n, i32 numDecPlaces, b8 useScientificNotation)
        {
            {
                std::ostream o (this);

                if (numDecPlaces > 0)
                {
                    o.setf (useScientificNotation ? std::ios_base::scientific : std::ios_base::fixed);
                    o.precision ((std::streamsize) numDecPlaces);
                }

                o << n;
            }

            return (size_t) (pptr() - pbase());
        }
    };

    static tuk doubleToString (tuk buffer, f64 n, i32 numDecPlaces, b8 useScientificNotation, size_t& len) noexcept
    {
        StackArrayStream strm (buffer);
        len = strm.writeDouble (n, numDecPlaces, useScientificNotation);
        jassert (len <= charsNeededForDouble);
        return buffer;
    }

    template <typename IntegerType>
    static Txt::CharPointerType createFromInteger (IntegerType number)
    {
        t8 buffer [charsNeededForInt];
        auto* end = buffer + numElementsInArray (buffer);
        auto* start = numberToString (end, number);
        return StringHolderUtils::createFromFixedLength (start, (size_t) (end - start - 1));
    }

    static Txt::CharPointerType createFromDouble (f64 number, i32 numberOfDecimalPlaces, b8 useScientificNotation)
    {
        t8 buffer [charsNeededForDouble];
        size_t len;
        auto start = doubleToString (buffer, number, numberOfDecimalPlaces, useScientificNotation, len);
        return StringHolderUtils::createFromFixedLength (start, len);
    }
}

//==============================================================================
Txt::Txt (i32 number)            : text (NumberToStringConverters::createFromInteger (number)) {}
Txt::Txt (u32 number)   : text (NumberToStringConverters::createFromInteger (number)) {}
Txt::Txt (short number)          : text (NumberToStringConverters::createFromInteger ((i32) number)) {}
Txt::Txt (u16 number) : text (NumberToStringConverters::createFromInteger ((u32) number)) {}
Txt::Txt (z64  number)         : text (NumberToStringConverters::createFromInteger (number)) {}
Txt::Txt (zu64 number)         : text (NumberToStringConverters::createFromInteger (number)) {}
Txt::Txt (i64 number)           : text (NumberToStringConverters::createFromInteger (number)) {}
Txt::Txt (u64 number)  : text (NumberToStringConverters::createFromInteger (number)) {}

Txt::Txt (f32  number)         : text (NumberToStringConverters::createFromDouble ((f64) number, 0, false)) {}
Txt::Txt (f64 number)         : text (NumberToStringConverters::createFromDouble (         number, 0, false)) {}
Txt::Txt (f32  number, i32 numberOfDecimalPlaces, b8 useScientificNotation)  : text (NumberToStringConverters::createFromDouble ((f64) number, numberOfDecimalPlaces, useScientificNotation)) {}
Txt::Txt (f64 number, i32 numberOfDecimalPlaces, b8 useScientificNotation)  : text (NumberToStringConverters::createFromDouble (         number, numberOfDecimalPlaces, useScientificNotation)) {}

//==============================================================================
i32 Txt::length() const noexcept
{
    return (i32) text.length();
}

static size_t findByteOffsetOfEnd (Txt::CharPointerType text) noexcept
{
    return (size_t) (((tuk) text.findTerminatingNull().getAddress()) - (tuk) text.getAddress());
}

size_t Txt::getByteOffsetOfEnd() const noexcept
{
    return findByteOffsetOfEnd (text);
}

t32 Txt::operator[] (i32 index) const noexcept
{
    jassert (index == 0 || (index > 0 && index <= (i32) text.lengthUpTo ((size_t) index + 1)));
    return text [index];
}

template <typename Type>
struct HashGenerator
{
    template <typename CharPointer>
    static Type calculate (CharPointer t) noexcept
    {
        Type result = {};

        while (! t.isEmpty())
            result = ((Type) multiplier) * result + (Type) t.getAndAdvance();

        return result;
    }

    enum { multiplier = sizeof (Type) > 4 ? 101 : 31 };
};

i32 Txt::hashCode() const noexcept       { return (i32) HashGenerator<u32>    ::calculate (text); }
z64 Txt::hashCode64() const noexcept   { return (z64) HashGenerator<zu64>  ::calculate (text); }
size_t Txt::hash() const noexcept        { return HashGenerator<size_t>          ::calculate (text); }

//==============================================================================
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, const Txt& s2) noexcept            { return s1.compare (s2) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, const Txt& s2) noexcept            { return s1.compare (s2) != 0; }
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, tukk s2) noexcept              { return s1.compare (s2) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, tukk s2) noexcept              { return s1.compare (s2) != 0; }
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, const wchar_t* s2) noexcept           { return s1.compare (s2) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, const wchar_t* s2) noexcept           { return s1.compare (s2) != 0; }
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, StringRef s2) noexcept                { return s1.getCharPointer().compare (s2.text) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, StringRef s2) noexcept                { return s1.getCharPointer().compare (s2.text) != 0; }
DRX_API b8 DRX_CALLTYPE operator<  (const Txt& s1, StringRef s2) noexcept                { return s1.getCharPointer().compare (s2.text) < 0; }
DRX_API b8 DRX_CALLTYPE operator<= (const Txt& s1, StringRef s2) noexcept                { return s1.getCharPointer().compare (s2.text) <= 0; }
DRX_API b8 DRX_CALLTYPE operator>  (const Txt& s1, StringRef s2) noexcept                { return s1.getCharPointer().compare (s2.text) > 0; }
DRX_API b8 DRX_CALLTYPE operator>= (const Txt& s1, StringRef s2) noexcept                { return s1.getCharPointer().compare (s2.text) >= 0; }
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, const CharPointer_UTF8 s2) noexcept   { return s1.getCharPointer().compare (s2) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, const CharPointer_UTF8 s2) noexcept   { return s1.getCharPointer().compare (s2) != 0; }
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, const CharPointer_UTF16 s2) noexcept  { return s1.getCharPointer().compare (s2) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, const CharPointer_UTF16 s2) noexcept  { return s1.getCharPointer().compare (s2) != 0; }
DRX_API b8 DRX_CALLTYPE operator== (const Txt& s1, const CharPointer_UTF32 s2) noexcept  { return s1.getCharPointer().compare (s2) == 0; }
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& s1, const CharPointer_UTF32 s2) noexcept  { return s1.getCharPointer().compare (s2) != 0; }

b8 Txt::equalsIgnoreCase (const wchar_t* const t) const noexcept
{
    return t != nullptr ? text.compareIgnoreCase (castToCharPointer_wchar_t (t)) == 0
                        : isEmpty();
}

b8 Txt::equalsIgnoreCase (tukk const t) const noexcept
{
    return t != nullptr ? text.compareIgnoreCase (CharPointer_UTF8 (t)) == 0
                        : isEmpty();
}

b8 Txt::equalsIgnoreCase (StringRef t) const noexcept
{
    return text.compareIgnoreCase (t.text) == 0;
}

b8 Txt::equalsIgnoreCase (const Txt& other) const noexcept
{
    return text == other.text
            || text.compareIgnoreCase (other.text) == 0;
}

i32 Txt::compare (const Txt& other) const noexcept           { return (text == other.text) ? 0 : text.compare (other.text); }
i32 Txt::compare (tukk const other) const noexcept       { return text.compare (CharPointer_UTF8 (other)); }
i32 Txt::compare (const wchar_t* const other) const noexcept    { return text.compare (castToCharPointer_wchar_t (other)); }
i32 Txt::compareIgnoreCase (const Txt& other) const noexcept { return (text == other.text) ? 0 : text.compareIgnoreCase (other.text); }

static i32 stringCompareRight (Txt::CharPointerType s1, Txt::CharPointerType s2) noexcept
{
    for (i32 bias = 0;;)
    {
        auto c1 = s1.getAndAdvance();
        b8 isDigit1 = CharacterFunctions::isDigit (c1);

        auto c2 = s2.getAndAdvance();
        b8 isDigit2 = CharacterFunctions::isDigit (c2);

        if (! (isDigit1 || isDigit2))   return bias;
        if (! isDigit1)                 return -1;
        if (! isDigit2)                 return 1;

        if (c1 != c2 && bias == 0)
            bias = c1 < c2 ? -1 : 1;

        jassert (c1 != 0 && c2 != 0);
    }
}

static i32 stringCompareLeft (Txt::CharPointerType s1, Txt::CharPointerType s2) noexcept
{
    for (;;)
    {
        auto c1 = s1.getAndAdvance();
        b8 isDigit1 = CharacterFunctions::isDigit (c1);

        auto c2 = s2.getAndAdvance();
        b8 isDigit2 = CharacterFunctions::isDigit (c2);

        if (! (isDigit1 || isDigit2))   return 0;
        if (! isDigit1)                 return -1;
        if (! isDigit2)                 return 1;
        if (c1 < c2)                    return -1;
        if (c1 > c2)                    return 1;
    }
}

static i32 naturalStringCompare (Txt::CharPointerType s1, Txt::CharPointerType s2, b8 isCaseSensitive) noexcept
{
    b8 firstLoop = true;

    for (;;)
    {
        const b8 hasSpace1 = s1.isWhitespace();
        const b8 hasSpace2 = s2.isWhitespace();

        if ((! firstLoop) && (hasSpace1 ^ hasSpace2))
        {
            if (s1.isEmpty())  return -1;
            if (s2.isEmpty())  return 1;

            return hasSpace2 ? 1 : -1;
        }

        firstLoop = false;

        if (hasSpace1)  s1 = s1.findEndOfWhitespace();
        if (hasSpace2)  s2 = s2.findEndOfWhitespace();

        if (s1.isDigit() && s2.isDigit())
        {
            auto result = (*s1 == '0' || *s2 == '0') ? stringCompareLeft  (s1, s2)
                                                     : stringCompareRight (s1, s2);

            if (result != 0)
                return result;
        }

        auto c1 = s1.getAndAdvance();
        auto c2 = s2.getAndAdvance();

        if (c1 != c2 && ! isCaseSensitive)
        {
            c1 = CharacterFunctions::toUpperCase (c1);
            c2 = CharacterFunctions::toUpperCase (c2);
        }

        if (c1 == c2)
        {
            if (c1 == 0)
                return 0;
        }
        else
        {
            const b8 isAlphaNum1 = CharacterFunctions::isLetterOrDigit (c1);
            const b8 isAlphaNum2 = CharacterFunctions::isLetterOrDigit (c2);

            if (isAlphaNum2 && ! isAlphaNum1) return -1;
            if (isAlphaNum1 && ! isAlphaNum2) return 1;

            return c1 < c2 ? -1 : 1;
        }

        jassert (c1 != 0 && c2 != 0);
    }
}

i32 Txt::compareNatural (StringRef other, b8 isCaseSensitive) const noexcept
{
    return naturalStringCompare (getCharPointer(), other.text, isCaseSensitive);
}

//==============================================================================
z0 Txt::append (const Txt& textToAppend, size_t maxCharsToTake)
{
    appendCharPointer (this == &textToAppend ? Txt (textToAppend).text
                                             : textToAppend.text, maxCharsToTake);
}

z0 Txt::appendCharPointer (const CharPointerType textToAppend)
{
    appendCharPointer (textToAppend, textToAppend.findTerminatingNull());
}

z0 Txt::appendCharPointer (const CharPointerType startOfTextToAppend,
                                const CharPointerType endOfTextToAppend)
{
    jassert (startOfTextToAppend.getAddress() != nullptr && endOfTextToAppend.getAddress() != nullptr);

    auto extraBytesNeeded = getAddressDifference (endOfTextToAppend.getAddress(),
                                                  startOfTextToAppend.getAddress());
    jassert (extraBytesNeeded >= 0);

    if (extraBytesNeeded > 0)
    {
        auto byteOffsetOfNull = getByteOffsetOfEnd();
        preallocateBytes ((size_t) extraBytesNeeded + byteOffsetOfNull);

        auto* newStringStart = addBytesToPointer (text.getAddress(), (i32) byteOffsetOfNull);
        memcpy (newStringStart, startOfTextToAppend.getAddress(), (size_t) extraBytesNeeded);
        CharPointerType (addBytesToPointer (newStringStart, extraBytesNeeded)).writeNull();
    }
}

Txt& Txt::operator+= (const wchar_t* t)
{
    appendCharPointer (castToCharPointer_wchar_t (t));
    return *this;
}

Txt& Txt::operator+= (tukk t)
{
    appendCharPointer (CharPointer_UTF8 (t)); // (using UTF8 here triggers a faster code-path than ascii)
    return *this;
}

Txt& Txt::operator+= (const Txt& other)
{
    if (isEmpty())
        return operator= (other);

    if (this == &other)
        return operator+= (Txt (*this));

    appendCharPointer (other.text);
    return *this;
}

Txt& Txt::operator+= (StringRef other)
{
    return operator+= (Txt (other));
}

Txt& Txt::operator+= (t8 ch)
{
    const t8 asString[] = { ch, 0 };
    return operator+= (asString);
}

Txt& Txt::operator+= (wchar_t ch)
{
    const wchar_t asString[] = { ch, 0 };
    return operator+= (asString);
}

#if ! DRX_NATIVE_WCHAR_IS_UTF32
Txt& Txt::operator+= (t32 ch)
{
    const t32 asString[] = { ch, 0 };
    appendCharPointer (CharPointer_UTF32 (asString));
    return *this;
}
#endif

namespace StringHelpers
{
    template <typename T>
    inline Txt& operationAddAssign (Txt& str, const T number)
    {
        t8 buffer [(sizeof (T) * 8) / 2];
        auto* end = buffer + numElementsInArray (buffer);
        auto* start = NumberToStringConverters::numberToString (end, number);

       #if DRX_STRING_UTF_TYPE == 8
        str.appendCharPointer (Txt::CharPointerType (start), Txt::CharPointerType (end));
       #else
        str.appendCharPointer (CharPointer_ASCII (start), CharPointer_ASCII (end));
       #endif

        return str;
    }
}

Txt& Txt::operator+= (i32k number)     { return StringHelpers::operationAddAssign<i32>    (*this, number); }
Txt& Txt::operator+= (const i64 number)    { return StringHelpers::operationAddAssign<i64>   (*this, number); }
Txt& Txt::operator+= (const z64 number)   { return StringHelpers::operationAddAssign<z64>  (*this, number); }
Txt& Txt::operator+= (const zu64 number)  { return StringHelpers::operationAddAssign<zu64> (*this, number); }

//==============================================================================
DRX_API Txt DRX_CALLTYPE operator+ (tukk s1, const Txt& s2)    { Txt s (s1); return s += s2; }
DRX_API Txt DRX_CALLTYPE operator+ (const wchar_t* s1, const Txt& s2) { Txt s (s1); return s += s2; }

DRX_API Txt DRX_CALLTYPE operator+ (t8 s1, const Txt& s2)           { return Txt::charToString ((t32) (u8) s1) + s2; }
DRX_API Txt DRX_CALLTYPE operator+ (wchar_t s1, const Txt& s2)        { return Txt::charToString (s1) + s2; }

DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, const Txt& s2)         { return s1 += s2; }
DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, tukk s2)           { return s1 += s2; }
DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, const wchar_t* s2)        { return s1 += s2; }
DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, const std::string& s2)    { return s1 += s2.c_str(); }

DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, t8 s2)                  { return s1 += s2; }
DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, wchar_t s2)               { return s1 += s2; }

#if ! DRX_NATIVE_WCHAR_IS_UTF32
DRX_API Txt DRX_CALLTYPE operator+ (t32 s1, const Txt& s2)     { return Txt::charToString (s1) + s2; }
DRX_API Txt DRX_CALLTYPE operator+ (Txt s1, t32 s2)            { return s1 += s2; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, t32 s2)         { return s1 += s2; }
#endif

DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, t8 s2)               { return s1 += s2; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, wchar_t s2)            { return s1 += s2; }

DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, tukk s2)        { return s1 += s2; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, const wchar_t* s2)     { return s1 += s2; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, const Txt& s2)      { return s1 += s2; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, StringRef s2)          { return s1 += s2; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, const std::string& s2) { return s1 += s2.c_str(); }

DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, u8  number)         { return s1 += (i32) number; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, short  number)         { return s1 += (i32) number; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, i32    number)         { return s1 += number; }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, i64   number)         { return s1 += Txt (number); }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, u64 number)  { return s1 += Txt (number); }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, z64  number)         { return s1 += Txt (number); }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, zu64 number)         { return s1 += Txt (number); }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, f32  number)         { return s1 += Txt (number); }
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& s1, f64 number)         { return s1 += Txt (number); }

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const Txt& text)
{
    return operator<< (stream, StringRef (text));
}

DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, StringRef text)
{
    auto numBytes = CharPointer_UTF8::getBytesRequiredFor (text.text);

   #if (DRX_STRING_UTF_TYPE == 8)
    stream.write (text.text.getAddress(), numBytes);
   #else
    // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    HeapBlock<t8> temp (numBytes + 1);
    CharPointer_UTF8 (temp).writeAll (text.text);
    stream.write (temp, numBytes);
   #endif

    return stream;
}

//==============================================================================
i32 Txt::indexOfChar (t32 character) const noexcept
{
    return text.indexOf (character);
}

i32 Txt::indexOfChar (i32 startIndex, t32 character) const noexcept
{
    auto t = text;

    for (i32 i = 0; ! t.isEmpty(); ++i)
    {
        if (i >= startIndex)
        {
            if (t.getAndAdvance() == character)
                return i;
        }
        else
        {
            ++t;
        }
    }

    return -1;
}

i32 Txt::lastIndexOfChar (t32 character) const noexcept
{
    auto t = text;
    i32 last = -1;

    for (i32 i = 0; ! t.isEmpty(); ++i)
        if (t.getAndAdvance() == character)
            last = i;

    return last;
}

i32 Txt::indexOfAnyOf (StringRef charactersToLookFor, i32 startIndex, b8 ignoreCase) const noexcept
{
    auto t = text;

    for (i32 i = 0; ! t.isEmpty(); ++i)
    {
        if (i >= startIndex)
        {
            if (charactersToLookFor.text.indexOf (t.getAndAdvance(), ignoreCase) >= 0)
                return i;
        }
        else
        {
            ++t;
        }
    }

    return -1;
}

i32 Txt::indexOf (StringRef other) const noexcept
{
    return other.isEmpty() ? 0 : text.indexOf (other.text);
}

i32 Txt::indexOfIgnoreCase (StringRef other) const noexcept
{
    return other.isEmpty() ? 0 : CharacterFunctions::indexOfIgnoreCase (text, other.text);
}

i32 Txt::indexOf (i32 startIndex, StringRef other) const noexcept
{
    if (other.isEmpty())
        return -1;

    auto t = text;

    for (i32 i = startIndex; --i >= 0;)
    {
        if (t.isEmpty())
            return -1;

        ++t;
    }

    auto found = t.indexOf (other.text);
    return found >= 0 ? found + startIndex : found;
}

i32 Txt::indexOfIgnoreCase (i32k startIndex, StringRef other) const noexcept
{
    if (other.isEmpty())
        return -1;

    auto t = text;

    for (i32 i = startIndex; --i >= 0;)
    {
        if (t.isEmpty())
            return -1;

        ++t;
    }

    auto found = CharacterFunctions::indexOfIgnoreCase (t, other.text);
    return found >= 0 ? found + startIndex : found;
}

i32 Txt::lastIndexOf (StringRef other) const noexcept
{
    if (other.isNotEmpty())
    {
        auto len = other.length();
        i32 i = length() - len;

        if (i >= 0)
        {
            for (auto n = text + i; i >= 0; --i)
            {
                if (n.compareUpTo (other.text, len) == 0)
                    return i;

                --n;
            }
        }
    }

    return -1;
}

i32 Txt::lastIndexOfIgnoreCase (StringRef other) const noexcept
{
    if (other.isNotEmpty())
    {
        auto len = other.length();
        i32 i = length() - len;

        if (i >= 0)
        {
            for (auto n = text + i; i >= 0; --i)
            {
                if (n.compareIgnoreCaseUpTo (other.text, len) == 0)
                    return i;

                --n;
            }
        }
    }

    return -1;
}

i32 Txt::lastIndexOfAnyOf (StringRef charactersToLookFor, const b8 ignoreCase) const noexcept
{
    auto t = text;
    i32 last = -1;

    for (i32 i = 0; ! t.isEmpty(); ++i)
        if (charactersToLookFor.text.indexOf (t.getAndAdvance(), ignoreCase) >= 0)
            last = i;

    return last;
}

b8 Txt::contains (StringRef other) const noexcept
{
    return indexOf (other) >= 0;
}

b8 Txt::containsChar (const t32 character) const noexcept
{
    return text.indexOf (character) >= 0;
}

b8 Txt::containsIgnoreCase (StringRef t) const noexcept
{
    return indexOfIgnoreCase (t) >= 0;
}

i32 Txt::indexOfWholeWord (StringRef word) const noexcept
{
    if (word.isNotEmpty())
    {
        auto t = text;
        auto wordLen = word.length();
        auto end = (i32) t.length() - wordLen;

        for (i32 i = 0; i <= end; ++i)
        {
            if (t.compareUpTo (word.text, wordLen) == 0
                  && (i == 0 || ! (t - 1).isLetterOrDigit())
                  && ! (t + wordLen).isLetterOrDigit())
                return i;

            ++t;
        }
    }

    return -1;
}

i32 Txt::indexOfWholeWordIgnoreCase (StringRef word) const noexcept
{
    if (word.isNotEmpty())
    {
        auto t = text;
        auto wordLen = word.length();
        auto end = (i32) t.length() - wordLen;

        for (i32 i = 0; i <= end; ++i)
        {
            if (t.compareIgnoreCaseUpTo (word.text, wordLen) == 0
                  && (i == 0 || ! (t - 1).isLetterOrDigit())
                  && ! (t + wordLen).isLetterOrDigit())
                return i;

            ++t;
        }
    }

    return -1;
}

b8 Txt::containsWholeWord (StringRef wordToLookFor) const noexcept
{
    return indexOfWholeWord (wordToLookFor) >= 0;
}

b8 Txt::containsWholeWordIgnoreCase (StringRef wordToLookFor) const noexcept
{
    return indexOfWholeWordIgnoreCase (wordToLookFor) >= 0;
}

//==============================================================================
template <typename CharPointer>
struct WildCardMatcher
{
    static b8 matches (CharPointer wildcard, CharPointer test, const b8 ignoreCase) noexcept
    {
        for (;;)
        {
            auto wc = wildcard.getAndAdvance();

            if (wc == '*')
                return wildcard.isEmpty() || matchesAnywhere (wildcard, test, ignoreCase);

            if (! characterMatches (wc, test.getAndAdvance(), ignoreCase))
                return false;

            if (wc == 0)
                return true;
        }
    }

    static b8 characterMatches (const t32 wc, const t32 tc, const b8 ignoreCase) noexcept
    {
        return (wc == tc) || (wc == '?' && tc != 0)
                || (ignoreCase && CharacterFunctions::toLowerCase (wc) == CharacterFunctions::toLowerCase (tc));
    }

    static b8 matchesAnywhere (const CharPointer wildcard, CharPointer test, const b8 ignoreCase) noexcept
    {
        for (; ! test.isEmpty(); ++test)
            if (matches (wildcard, test, ignoreCase))
                return true;

        return false;
    }
};

b8 Txt::matchesWildcard (StringRef wildcard, const b8 ignoreCase) const noexcept
{
    return WildCardMatcher<CharPointerType>::matches (wildcard.text, text, ignoreCase);
}

//==============================================================================
Txt Txt::repeatedString (StringRef stringToRepeat, i32 numberOfTimesToRepeat)
{
    if (numberOfTimesToRepeat <= 0)
        return {};

    Txt result (PreallocationBytes (findByteOffsetOfEnd (stringToRepeat) * (size_t) numberOfTimesToRepeat));
    auto n = result.text;

    while (--numberOfTimesToRepeat >= 0)
        n.writeAll (stringToRepeat.text);

    return result;
}

Txt Txt::paddedLeft (const t32 padCharacter, i32 minimumLength) const
{
    jassert (padCharacter != 0);

    auto extraChars = minimumLength;
    auto end = text;

    while (! end.isEmpty())
    {
        --extraChars;
        ++end;
    }

    if (extraChars <= 0 || padCharacter == 0)
        return *this;

    auto currentByteSize = (size_t) (((tuk) end.getAddress()) - (tuk) text.getAddress());
    Txt result (PreallocationBytes (currentByteSize + (size_t) extraChars * CharPointerType::getBytesRequiredFor (padCharacter)));
    auto n = result.text;

    while (--extraChars >= 0)
        n.write (padCharacter);

    n.writeAll (text);
    return result;
}

Txt Txt::paddedRight (const t32 padCharacter, i32 minimumLength) const
{
    jassert (padCharacter != 0);

    auto extraChars = minimumLength;
    CharPointerType end (text);

    while (! end.isEmpty())
    {
        --extraChars;
        ++end;
    }

    if (extraChars <= 0 || padCharacter == 0)
        return *this;

    auto currentByteSize = (size_t) (((tuk) end.getAddress()) - (tuk) text.getAddress());
    Txt result (PreallocationBytes (currentByteSize + (size_t) extraChars * CharPointerType::getBytesRequiredFor (padCharacter)));
    auto n = result.text;

    n.writeAll (text);

    while (--extraChars >= 0)
        n.write (padCharacter);

    n.writeNull();
    return result;
}

//==============================================================================
Txt Txt::replaceSection (i32 index, i32 numCharsToReplace, StringRef stringToInsert) const
{
    if (index < 0)
    {
        // a negative index to replace from?
        jassertfalse;
        index = 0;
    }

    if (numCharsToReplace < 0)
    {
        // replacing a negative number of characters?
        numCharsToReplace = 0;
        jassertfalse;
    }

    auto insertPoint = text;

    for (i32 i = 0; i < index; ++i)
    {
        if (insertPoint.isEmpty())
        {
            // replacing beyond the end of the string?
            jassertfalse;
            return *this + stringToInsert;
        }

        ++insertPoint;
    }

    auto startOfRemainder = insertPoint;

    for (i32 i = 0; i < numCharsToReplace && ! startOfRemainder.isEmpty(); ++i)
        ++startOfRemainder;

    if (insertPoint == text && startOfRemainder.isEmpty())
        return stringToInsert.text;

    auto initialBytes = (size_t) (((tuk) insertPoint.getAddress()) - (tuk) text.getAddress());
    auto newStringBytes = findByteOffsetOfEnd (stringToInsert);
    auto remainderBytes = (size_t) (((tuk) startOfRemainder.findTerminatingNull().getAddress()) - (tuk) startOfRemainder.getAddress());

    auto newTotalBytes = initialBytes + newStringBytes + remainderBytes;

    if (newTotalBytes <= 0)
        return {};

    Txt result (PreallocationBytes ((size_t) newTotalBytes));

    auto* dest = (tuk) result.text.getAddress();
    memcpy (dest, text.getAddress(), initialBytes);
    dest += initialBytes;
    memcpy (dest, stringToInsert.text.getAddress(), newStringBytes);
    dest += newStringBytes;
    memcpy (dest, startOfRemainder.getAddress(), remainderBytes);
    dest += remainderBytes;
    CharPointerType (unalignedPointerCast<CharPointerType::CharType*> (dest)).writeNull();

    return result;
}

Txt Txt::replace (StringRef stringToReplace, StringRef stringToInsert, const b8 ignoreCase) const
{
    auto stringToReplaceLen = stringToReplace.length();
    auto stringToInsertLen = stringToInsert.length();

    i32 i = 0;
    Txt result (*this);

    while ((i = (ignoreCase ? result.indexOfIgnoreCase (i, stringToReplace)
                            : result.indexOf (i, stringToReplace))) >= 0)
    {
        result = result.replaceSection (i, stringToReplaceLen, stringToInsert);
        i += stringToInsertLen;
    }

    return result;
}

Txt Txt::replaceFirstOccurrenceOf (StringRef stringToReplace, StringRef stringToInsert, const b8 ignoreCase) const
{
    auto stringToReplaceLen = stringToReplace.length();
    auto index = ignoreCase ? indexOfIgnoreCase (stringToReplace)
                            : indexOf (stringToReplace);

    if (index >= 0)
        return replaceSection (index, stringToReplaceLen, stringToInsert);

    return *this;
}

struct StringCreationHelper
{
    StringCreationHelper (size_t initialBytes)  : allocatedBytes (initialBytes)
    {
        result.preallocateBytes (allocatedBytes);
        dest = result.getCharPointer();
    }

    StringCreationHelper (const Txt::CharPointerType s)
        : source (s), allocatedBytes (StringHolderUtils::getAllocatedNumBytes (s))
    {
        result.preallocateBytes (allocatedBytes);
        dest = result.getCharPointer();
    }

    z0 write (t32 c)
    {
        bytesWritten += Txt::CharPointerType::getBytesRequiredFor (c);

        if (bytesWritten > allocatedBytes)
        {
            allocatedBytes += jmax ((size_t) 8, allocatedBytes / 16);
            auto destOffset = (size_t) (((tuk) dest.getAddress()) - (tuk) result.getCharPointer().getAddress());
            result.preallocateBytes (allocatedBytes);
            dest = addBytesToPointer (result.getCharPointer().getAddress(), (i32) destOffset);
        }

        dest.write (c);
    }

    Txt result;
    Txt::CharPointerType source { nullptr }, dest { nullptr };
    size_t allocatedBytes, bytesWritten = 0;
};

Txt Txt::replaceCharacter (const t32 charToReplace, const t32 charToInsert) const
{
    if (! containsChar (charToReplace))
        return *this;

    StringCreationHelper builder (text);

    for (;;)
    {
        auto c = builder.source.getAndAdvance();

        if (c == charToReplace)
            c = charToInsert;

        builder.write (c);

        if (c == 0)
            break;
    }

    return std::move (builder.result);
}

Txt Txt::replaceCharacters (StringRef charactersToReplace, StringRef charactersToInsertInstead) const
{
    // Each character in the first string must have a matching one in the
    // second, so the two strings must be the same length.
    jassert (charactersToReplace.length() == charactersToInsertInstead.length());

    StringCreationHelper builder (text);

    for (;;)
    {
        auto c = builder.source.getAndAdvance();
        auto index = charactersToReplace.text.indexOf (c);

        if (index >= 0)
            c = charactersToInsertInstead [index];

        builder.write (c);

        if (c == 0)
            break;
    }

    return std::move (builder.result);
}

//==============================================================================
b8 Txt::startsWith (StringRef other) const noexcept
{
    return text.compareUpTo (other.text, other.length()) == 0;
}

b8 Txt::startsWithIgnoreCase (StringRef other) const noexcept
{
    return text.compareIgnoreCaseUpTo (other.text, other.length()) == 0;
}

b8 Txt::startsWithChar (const t32 character) const noexcept
{
    jassert (character != 0); // strings can't contain a null character!

    return *text == character;
}

b8 Txt::endsWithChar (const t32 character) const noexcept
{
    jassert (character != 0); // strings can't contain a null character!

    if (text.isEmpty())
        return false;

    auto t = text.findTerminatingNull();
    return *--t == character;
}

b8 Txt::endsWith (StringRef other) const noexcept
{
    auto end = text.findTerminatingNull();
    auto otherEnd = other.text.findTerminatingNull();

    while (end > text && otherEnd > other.text)
    {
        --end;
        --otherEnd;

        if (*end != *otherEnd)
            return false;
    }

    return otherEnd == other.text;
}

b8 Txt::endsWithIgnoreCase (StringRef other) const noexcept
{
    auto end = text.findTerminatingNull();
    auto otherEnd = other.text.findTerminatingNull();

    while (end > text && otherEnd > other.text)
    {
        --end;
        --otherEnd;

        if (end.toLowerCase() != otherEnd.toLowerCase())
            return false;
    }

    return otherEnd == other.text;
}

//==============================================================================
Txt Txt::toUpperCase() const
{
    StringCreationHelper builder (text);

    for (;;)
    {
        auto c = builder.source.toUpperCase();
        builder.write (c);

        if (c == 0)
            break;

        ++(builder.source);
    }

    return std::move (builder.result);
}

Txt Txt::toLowerCase() const
{
    StringCreationHelper builder (text);

    for (;;)
    {
        auto c = builder.source.toLowerCase();
        builder.write (c);

        if (c == 0)
            break;

        ++(builder.source);
    }

    return std::move (builder.result);
}

//==============================================================================
t32 Txt::getLastCharacter() const noexcept
{
    return isEmpty() ? t32() : text [length() - 1];
}

Txt Txt::substring (i32 start, i32k end) const
{
    if (start < 0)
        start = 0;

    if (end <= start)
        return {};

    i32 i = 0;
    auto t1 = text;

    while (i < start)
    {
        if (t1.isEmpty())
            return {};

        ++i;
        ++t1;
    }

    auto t2 = t1;

    while (i < end)
    {
        if (t2.isEmpty())
        {
            if (start == 0)
                return *this;

            break;
        }

        ++i;
        ++t2;
    }

    return Txt (t1, t2);
}

Txt Txt::substring (i32 start) const
{
    if (start <= 0)
        return *this;

    auto t = text;

    while (--start >= 0)
    {
        if (t.isEmpty())
            return {};

        ++t;
    }

    return Txt (t);
}

Txt Txt::dropLastCharacters (i32k numberToDrop) const
{
    return Txt (text, (size_t) jmax (0, length() - numberToDrop));
}

Txt Txt::getLastCharacters (i32k numCharacters) const
{
    return Txt (text + jmax (0, length() - jmax (0, numCharacters)));
}

Txt Txt::fromFirstOccurrenceOf (StringRef sub, b8 includeSubString, b8 ignoreCase) const
{
    auto i = ignoreCase ? indexOfIgnoreCase (sub)
                        : indexOf (sub);
    if (i < 0)
        return {};

    return substring (includeSubString ? i : i + sub.length());
}

Txt Txt::fromLastOccurrenceOf (StringRef sub, b8 includeSubString, b8 ignoreCase) const
{
    auto i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                        : lastIndexOf (sub);
    if (i < 0)
        return *this;

    return substring (includeSubString ? i : i + sub.length());
}

Txt Txt::upToFirstOccurrenceOf (StringRef sub, b8 includeSubString, b8 ignoreCase) const
{
    auto i = ignoreCase ? indexOfIgnoreCase (sub)
                        : indexOf (sub);
    if (i < 0)
        return *this;

    return substring (0, includeSubString ? i + sub.length() : i);
}

Txt Txt::upToLastOccurrenceOf (StringRef sub, b8 includeSubString, b8 ignoreCase) const
{
    auto i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                        : lastIndexOf (sub);
    if (i < 0)
        return *this;

    return substring (0, includeSubString ? i + sub.length() : i);
}

static b8 isQuoteCharacter (t32 c) noexcept
{
    return c == '"' || c == '\'';
}

b8 Txt::isQuotedString() const
{
    return isQuoteCharacter (*text.findEndOfWhitespace());
}

Txt Txt::unquoted() const
{
    if (! isQuoteCharacter (*text))
        return *this;

    auto len = length();
    return substring (1, len - (isQuoteCharacter (text[len - 1]) ? 1 : 0));
}

Txt Txt::quoted (t32 quoteCharacter) const
{
    if (isEmpty())
        return charToString (quoteCharacter) + quoteCharacter;

    Txt t (*this);

    if (! t.startsWithChar (quoteCharacter))
        t = charToString (quoteCharacter) + t;

    if (! t.endsWithChar (quoteCharacter))
        t += quoteCharacter;

    return t;
}

//==============================================================================
static Txt::CharPointerType findTrimmedEnd (const Txt::CharPointerType start,
                                               Txt::CharPointerType end)
{
    while (end > start)
    {
        if (! (--end).isWhitespace())
        {
            ++end;
            break;
        }
    }

    return end;
}

Txt Txt::trim() const
{
    if (isNotEmpty())
    {
        auto start = text.findEndOfWhitespace();
        auto end = start.findTerminatingNull();
        auto trimmedEnd = findTrimmedEnd (start, end);

        if (trimmedEnd <= start)
            return {};

        if (text < start || trimmedEnd < end)
            return Txt (start, trimmedEnd);
    }

    return *this;
}

Txt Txt::trimStart() const
{
    if (isNotEmpty())
    {
        auto t = text.findEndOfWhitespace();

        if (t != text)
            return Txt (t);
    }

    return *this;
}

Txt Txt::trimEnd() const
{
    if (isNotEmpty())
    {
        auto end = text.findTerminatingNull();
        auto trimmedEnd = findTrimmedEnd (text, end);

        if (trimmedEnd < end)
            return Txt (text, trimmedEnd);
    }

    return *this;
}

Txt Txt::trimCharactersAtStart (StringRef charactersToTrim) const
{
    auto t = text;

    while (charactersToTrim.text.indexOf (*t) >= 0)
        ++t;

    return t == text ? *this : Txt (t);
}

Txt Txt::trimCharactersAtEnd (StringRef charactersToTrim) const
{
    if (isNotEmpty())
    {
        auto end = text.findTerminatingNull();
        auto trimmedEnd = end;

        while (trimmedEnd > text)
        {
            if (charactersToTrim.text.indexOf (*--trimmedEnd) < 0)
            {
                ++trimmedEnd;
                break;
            }
        }

        if (trimmedEnd < end)
            return Txt (text, trimmedEnd);
    }

    return *this;
}

//==============================================================================
Txt Txt::retainCharacters (StringRef charactersToRetain) const
{
    if (isEmpty())
        return {};

    StringCreationHelper builder (text);

    for (;;)
    {
        auto c = builder.source.getAndAdvance();

        if (charactersToRetain.text.indexOf (c) >= 0)
            builder.write (c);

        if (c == 0)
            break;
    }

    builder.write (0);
    return std::move (builder.result);
}

Txt Txt::removeCharacters (StringRef charactersToRemove) const
{
    if (isEmpty())
        return {};

    StringCreationHelper builder (text);

    for (;;)
    {
        auto c = builder.source.getAndAdvance();

        if (charactersToRemove.text.indexOf (c) < 0)
            builder.write (c);

        if (c == 0)
            break;
    }

    return std::move (builder.result);
}

Txt Txt::initialSectionContainingOnly (StringRef permittedCharacters) const
{
    for (auto t = text; ! t.isEmpty(); ++t)
        if (permittedCharacters.text.indexOf (*t) < 0)
            return Txt (text, t);

    return *this;
}

Txt Txt::initialSectionNotContaining (StringRef charactersToStopAt) const
{
    for (auto t = text; ! t.isEmpty(); ++t)
        if (charactersToStopAt.text.indexOf (*t) >= 0)
            return Txt (text, t);

    return *this;
}

b8 Txt::containsOnly (StringRef chars) const noexcept
{
    for (auto t = text; ! t.isEmpty();)
        if (chars.text.indexOf (t.getAndAdvance()) < 0)
            return false;

    return true;
}

b8 Txt::containsAnyOf (StringRef chars) const noexcept
{
    for (auto t = text; ! t.isEmpty();)
        if (chars.text.indexOf (t.getAndAdvance()) >= 0)
            return true;

    return false;
}

b8 Txt::containsNonWhitespaceChars() const noexcept
{
    for (auto t = text; ! t.isEmpty(); ++t)
        if (! t.isWhitespace())
            return true;

    return false;
}

Txt Txt::formattedRaw (tukk pf, ...)
{
    size_t bufferSize = 256;

    for (;;)
    {
        va_list args;
        va_start (args, pf);

       #if DRX_WINDOWS
        DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
       #endif

      #if DRX_ANDROID
        HeapBlock<t8> temp (bufferSize);
        i32 num = (i32) vsnprintf (temp.get(), bufferSize - 1, pf, args);
        if (num >= static_cast<i32> (bufferSize))
            num = -1;
      #else
        Txt wideCharVersion (pf);
        HeapBlock<wchar_t> temp (bufferSize);
        i32k num = (i32)
       #if DRX_WINDOWS
            _vsnwprintf
       #else
            vswprintf
       #endif
                (temp.get(), bufferSize - 1, wideCharVersion.toWideCharPointer(), args);
      #endif

       #if DRX_WINDOWS
        DRX_END_IGNORE_DEPRECATION_WARNINGS
       #endif
        va_end (args);

        if (num > 0)
            return Txt (temp.get());

        bufferSize += 256;

        if (num == 0 || bufferSize > 65536) // the upper limit is a sanity check to avoid situations where vprintf repeatedly
            break;                          // returns -1 because of an error rather than because it needs more space.
    }

    return {};
}

//==============================================================================
i32 Txt::getIntValue() const noexcept            { return text.getIntValue32(); }
z64 Txt::getLargeIntValue() const noexcept     { return text.getIntValue64(); }
f32 Txt::getFloatValue() const noexcept        { return (f32) getDoubleValue(); }
f64 Txt::getDoubleValue() const noexcept      { return text.getDoubleValue(); }

i32 Txt::getTrailingIntValue() const noexcept
{
    i32 n = 0;
    i32 mult = 1;
    auto t = text.findTerminatingNull();

    while (--t >= text)
    {
        if (! t.isDigit())
        {
            if (*t == '-')
                n = -n;

            break;
        }

        n += (i32) (((t32) mult) * (*t - '0'));
        mult *= 10;
    }

    return n;
}

static const t8 hexDigits[] = "0123456789abcdef";

template <typename Type>
static Txt hexToString (Type v)
{
    Txt::CharPointerType::CharType buffer[32];
    auto* end = buffer + numElementsInArray (buffer) - 1;
    auto* t = end;
    *t = 0;

    do
    {
        *--t = hexDigits [(i32) (v & 15)];
        v = static_cast<Type> (v >> 4);

    } while (v != 0);

    return Txt (Txt::CharPointerType (t),
                   Txt::CharPointerType (end));
}

Txt Txt::createHex (u8 n)    { return hexToString (n); }
Txt Txt::createHex (u16 n)   { return hexToString (n); }
Txt Txt::createHex (u32 n)   { return hexToString (n); }
Txt Txt::createHex (zu64 n)   { return hexToString (n); }

Txt Txt::toHexString (ukk const d, i32k size, i32k groupSize)
{
    if (size <= 0)
        return {};

    i32 numChars = (size * 2) + 2;
    if (groupSize > 0)
        numChars += size / groupSize;

    Txt s (PreallocationBytes ((size_t) numChars * sizeof (CharPointerType::CharType)));

    auto* data = static_cast<u8k*> (d);
    auto dest = s.text;

    for (i32 i = 0; i < size; ++i)
    {
        u8k nextByte = *data++;
        dest.write ((t32) hexDigits [nextByte >> 4]);
        dest.write ((t32) hexDigits [nextByte & 0xf]);

        if (groupSize > 0 && (i % groupSize) == (groupSize - 1) && i < (size - 1))
            dest.write ((t32) ' ');
    }

    dest.writeNull();
    return s;
}

i32   Txt::getHexValue32() const noexcept    { return (i32) CharacterFunctions::HexParser<u32>::parse (text); }
z64 Txt::getHexValue64() const noexcept    { return (z64) CharacterFunctions::HexParser<zu64>::parse (text); }

//==============================================================================
static Txt getStringFromWindows1252Codepage (tukk data, size_t num)
{
    HeapBlock<t32> unicode (num + 1);

    for (size_t i = 0; i < num; ++i)
        unicode[i] = CharacterFunctions::getUnicodeCharFromWindows1252Codepage ((u8) data[i]);

    unicode[num] = 0;
    return CharPointer_UTF32 (unicode);
}

Txt Txt::createStringFromData (ukk const unknownData, i32 size)
{
    auto* data = static_cast<u8k*> (unknownData);

    if (size <= 0 || data == nullptr)
        return {};

    if (size == 1)
        return charToString ((t32) data[0]);

    if (CharPointer_UTF16::isByteOrderMarkBigEndian (data)
         || CharPointer_UTF16::isByteOrderMarkLittleEndian (data))
    {
        i32k numChars = size / 2 - 1;

        StringCreationHelper builder ((size_t) numChars);

        auto src = unalignedPointerCast<u16k*> (data + 2);

        if (CharPointer_UTF16::isByteOrderMarkBigEndian (data))
        {
            for (i32 i = 0; i < numChars; ++i)
                builder.write ((t32) ByteOrder::swapIfLittleEndian (src[i]));
        }
        else
        {
            for (i32 i = 0; i < numChars; ++i)
                builder.write ((t32) ByteOrder::swapIfBigEndian (src[i]));
        }

        builder.write (0);
        return std::move (builder.result);
    }

    auto* start = (tukk) data;

    if (size >= 3 && CharPointer_UTF8::isByteOrderMark (data))
    {
        start += 3;
        size -= 3;
    }

    if (CharPointer_UTF8::isValidString (start, size))
        return Txt (CharPointer_UTF8 (start),
                       CharPointer_UTF8 (start + size));

    return getStringFromWindows1252Codepage (start, (size_t) size);
}

//==============================================================================
static const t32 emptyChar = 0;

template <class CharPointerType_Src, class CharPointerType_Dest>
struct StringEncodingConverter
{
    static CharPointerType_Dest convert (const Txt& s)
    {
        auto& source = const_cast<Txt&> (s);

        using DestChar = typename CharPointerType_Dest::CharType;

        if (source.isEmpty())
            return CharPointerType_Dest (reinterpret_cast<const DestChar*> (&emptyChar));

        CharPointerType_Src text (source.getCharPointer());
        auto extraBytesNeeded = CharPointerType_Dest::getBytesRequiredFor (text) + sizeof (typename CharPointerType_Dest::CharType);
        auto endOffset = (text.sizeInBytes() + 3) & ~3u; // the new string must be word-aligned or many Windows
                                                         // functions will fail to read it correctly!
        source.preallocateBytes (endOffset + extraBytesNeeded);
        text = source.getCharPointer();

        uk const newSpace = addBytesToPointer (text.getAddress(), (i32) endOffset);
        const CharPointerType_Dest extraSpace (static_cast<DestChar*> (newSpace));

       #if DRX_DEBUG // (This just avoids spurious warnings from valgrind about the uninitialised bytes at the end of the buffer..)
        auto bytesToClear = (size_t) jmin ((i32) extraBytesNeeded, 4);
        zeromem (addBytesToPointer (newSpace, extraBytesNeeded - bytesToClear), bytesToClear);
       #endif

        CharPointerType_Dest (extraSpace).writeAll (text);
        return extraSpace;
    }
};

template <>
struct StringEncodingConverter<CharPointer_UTF8, CharPointer_UTF8>
{
    static CharPointer_UTF8 convert (const Txt& source) noexcept   { return CharPointer_UTF8 (unalignedPointerCast<CharPointer_UTF8::CharType*> (source.getCharPointer().getAddress())); }
};

template <>
struct StringEncodingConverter<CharPointer_UTF16, CharPointer_UTF16>
{
    static CharPointer_UTF16 convert (const Txt& source) noexcept  { return CharPointer_UTF16 (unalignedPointerCast<CharPointer_UTF16::CharType*> (source.getCharPointer().getAddress())); }
};

template <>
struct StringEncodingConverter<CharPointer_UTF32, CharPointer_UTF32>
{
    static CharPointer_UTF32 convert (const Txt& source) noexcept  { return CharPointer_UTF32 (unalignedPointerCast<CharPointer_UTF32::CharType*> (source.getCharPointer().getAddress())); }
};

CharPointer_UTF8  Txt::toUTF8()  const { return StringEncodingConverter<CharPointerType, CharPointer_UTF8 >::convert (*this); }
CharPointer_UTF16 Txt::toUTF16() const { return StringEncodingConverter<CharPointerType, CharPointer_UTF16>::convert (*this); }
CharPointer_UTF32 Txt::toUTF32() const { return StringEncodingConverter<CharPointerType, CharPointer_UTF32>::convert (*this); }

tukk Txt::toRawUTF8() const
{
    return toUTF8().getAddress();
}

const wchar_t* Txt::toWideCharPointer() const
{
    return StringEncodingConverter<CharPointerType, CharPointer_wchar_t>::convert (*this).getAddress();
}

std::string Txt::toStdString() const
{
    return std::string (toRawUTF8());
}

//==============================================================================
template <class CharPointerType_Src, class CharPointerType_Dest>
struct StringCopier
{
    static size_t copyToBuffer (const CharPointerType_Src source, typename CharPointerType_Dest::CharType* const buffer, const size_t maxBufferSizeBytes)
    {
        jassert (((ssize_t) maxBufferSizeBytes) >= 0); // keep this value positive!

        if (buffer == nullptr)
            return CharPointerType_Dest::getBytesRequiredFor (source) + sizeof (typename CharPointerType_Dest::CharType);

        return CharPointerType_Dest (buffer).writeWithDestByteLimit (source, maxBufferSizeBytes);
    }
};

size_t Txt::copyToUTF8 (CharPointer_UTF8::CharType* const buffer, size_t maxBufferSizeBytes) const noexcept
{
    return StringCopier<CharPointerType, CharPointer_UTF8>::copyToBuffer (text, buffer, maxBufferSizeBytes);
}

size_t Txt::copyToUTF16 (CharPointer_UTF16::CharType* const buffer, size_t maxBufferSizeBytes) const noexcept
{
    return StringCopier<CharPointerType, CharPointer_UTF16>::copyToBuffer (text, buffer, maxBufferSizeBytes);
}

size_t Txt::copyToUTF32 (CharPointer_UTF32::CharType* const buffer, size_t maxBufferSizeBytes) const noexcept
{
    return StringCopier<CharPointerType, CharPointer_UTF32>::copyToBuffer (text, buffer, maxBufferSizeBytes);
}

//==============================================================================
size_t Txt::getNumBytesAsUTF8() const noexcept
{
    return CharPointer_UTF8::getBytesRequiredFor (text);
}

Txt Txt::fromUTF8 (tukk const buffer, i32 bufferSizeBytes)
{
    if (buffer == nullptr || bufferSizeBytes == 0)
        return {};

    if (bufferSizeBytes < 0)
    {
        jassert (CharPointer_UTF8::isValidString (buffer, std::numeric_limits<i32>::max()));
        return { CharPointer_UTF8 (buffer) };
    }

    jassert (CharPointer_UTF8::isValidString (buffer, bufferSizeBytes));
    return { CharPointer_UTF8 (buffer), CharPointer_UTF8 (buffer + bufferSizeBytes) };
}

#if __cpp_char8_t
Txt Txt::fromUTF8 (const char8_t* const buffer, i32 bufferSizeBytes)
{
    return { buffer, (size_t) bufferSizeBytes };
}
#endif

DRX_END_IGNORE_WARNINGS_MSVC

//==============================================================================
StringRef::StringRef() noexcept  : text (unalignedPointerCast<const Txt::CharPointerType::CharType*> ("\0\0\0"))
{
}

StringRef::StringRef (tukk stringLiteral) noexcept
   #if DRX_STRING_UTF_TYPE != 8
    : text (nullptr), stringCopy (stringLiteral)
   #else
    : text (stringLiteral)
   #endif
{
   #if DRX_STRING_UTF_TYPE != 8
    text = stringCopy.getCharPointer();
   #endif

    jassert (stringLiteral != nullptr); // This must be a valid string literal, not a null pointer!!

   #if DRX_NATIVE_WCHAR_IS_UTF8
    /*  If you get an assertion here, then you're trying to create a string from 8-bit data
        that contains values greater than 127. These can NOT be correctly converted to unicode
        because there's no way for the Txt class to know what encoding was used to
        create them. The source data could be UTF-8, ASCII or one of many local code-pages.

        To get around this problem, you must be more explicit when you pass an ambiguous 8-bit
        string to the StringRef class - so for example if your source data is actually UTF-8,
        you'd call StringRef (CharPointer_UTF8 ("my utf8 string..")), and it would be able to
        correctly convert the multi-byte characters to unicode. It's *highly* recommended that
        you use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent these strings in a way that isn't dependent on
        the compiler, source code editor and platform.
    */
    jassert (CharPointer_ASCII::isValidString (stringLiteral, std::numeric_limits<i32>::max()));
   #endif
}

StringRef::StringRef (Txt::CharPointerType stringLiteral) noexcept  : text (stringLiteral)
{
    jassert (stringLiteral.getAddress() != nullptr); // This must be a valid string literal, not a null pointer!!
}

StringRef::StringRef (const Txt& string) noexcept   : text (string.getCharPointer()) {}
StringRef::StringRef (const std::string& string)       : StringRef (string.c_str()) {}

//==============================================================================
static Txt reduceLengthOfFloatString (const Txt& input)
{
    const auto start = input.getCharPointer();
    const auto end = start + (i32) input.length();
    auto trimStart = end;
    auto trimEnd = trimStart;
    auto exponentTrimStart = end;
    auto exponentTrimEnd = exponentTrimStart;

    decltype (*start) currentChar = '\0';

    for (auto c = end - 1; c > start; --c)
    {
        currentChar = *c;

        if (currentChar == '0' && c + 1 == trimStart)
        {
            --trimStart;
        }
        else if (currentChar == '.')
        {
            if (trimStart == c + 1 && trimStart != end && *trimStart == '0')
                ++trimStart;

            break;
        }
        else if (currentChar == 'e' || currentChar == 'E')
        {
            auto cNext = c + 1;

            if (cNext != end)
            {
                if (*cNext == '-')
                    ++cNext;

                exponentTrimStart = cNext;

                if (cNext != end && *cNext == '+')
                    ++cNext;

                exponentTrimEnd = cNext;
            }

            while (cNext != end && *cNext++ == '0')
                exponentTrimEnd = cNext;

            if (exponentTrimEnd == end)
                exponentTrimStart = c;

            trimStart = c;
            trimEnd = trimStart;
        }
    }

    if ((trimStart != trimEnd && currentChar == '.') || exponentTrimStart != exponentTrimEnd)
    {
        if (trimStart == trimEnd)
            return Txt (start, exponentTrimStart) + Txt (exponentTrimEnd, end);

        if (exponentTrimStart == exponentTrimEnd)
            return Txt (start, trimStart) + Txt (trimEnd, end);

        if (trimEnd == exponentTrimStart)
            return Txt (start, trimStart) + Txt (exponentTrimEnd, end);

        return Txt (start, trimStart) + Txt (trimEnd, exponentTrimStart) + Txt (exponentTrimEnd, end);
    }

    return input;
}

/*  maxDecimalPlaces <= 0 means "use as many decimal places as necessary"
*/
static Txt serialiseDouble (f64 input, i32 maxDecimalPlaces = 0)
{
    auto absInput = std::abs (input);

    if (absInput >= 1.0e6 || absInput <= 1.0e-5)
        return reduceLengthOfFloatString ({ input, maxDecimalPlaces > 0 ? maxDecimalPlaces : 15, true });

    i32 intInput = (i32) input;

    if (exactlyEqual ((f64) intInput, input))
        return { input, 1 };

    auto numberOfDecimalPlaces = [absInput, maxDecimalPlaces]
    {
        if (maxDecimalPlaces > 0)
            return maxDecimalPlaces;

        if (absInput < 1.0)
        {
            if (absInput >= 1.0e-3)
            {
                if (absInput >= 1.0e-1) return 16;
                if (absInput >= 1.0e-2) return 17;
                return 18;
            }

            if (absInput >= 1.0e-4) return 19;
            return 20;
        }

        if (absInput < 1.0e3)
        {
            if (absInput < 1.0e1) return 15;
            if (absInput < 1.0e2) return 14;
            return 13;
        }

        if (absInput < 1.0e4) return 12;
        if (absInput < 1.0e5) return 11;
        return 10;
    }();

    return reduceLengthOfFloatString (Txt (input, numberOfDecimalPlaces));
}

//==============================================================================
#if DRX_ALLOW_STATIC_NULL_VARIABLES

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

const Txt Txt::empty;

DRX_END_IGNORE_DEPRECATION_WARNINGS

#endif

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

class StringTests final : public UnitTest
{
public:
    StringTests()
        : UnitTest ("Txt class", UnitTestCategories::text)
    {}

    template <class CharPointerType>
    struct TestUTFConversion
    {
        static z0 test (UnitTest& test, Random& r)
        {
            constexpr auto stringLength = 50;
            const Txt s (createRandomWideCharString (r, stringLength));

            using CharType = typename CharPointerType::CharType;
            constexpr auto bytesPerCodeUnit = sizeof (CharType);
            constexpr auto maxCodeUnitsPerCodePoint = 4 / bytesPerCodeUnit;

            std::array<CharType, stringLength * maxCodeUnitsPerCodePoint + 1> codeUnits{};
            const auto codeUnitsSizeInBytes = codeUnits.size() * bytesPerCodeUnit;

            std::memset (codeUnits.data(), 0xff, codeUnitsSizeInBytes);
            CharPointerType (codeUnits.data()).writeAll (s.toUTF32());
            test.expectEquals (Txt (CharPointerType (codeUnits.data())), s);

            std::memset (codeUnits.data(), 0xff, codeUnitsSizeInBytes);
            CharPointerType (codeUnits.data()).writeAll (s.toUTF16());
            test.expectEquals (Txt (CharPointerType (codeUnits.data())), s);

            std::memset (codeUnits.data(), 0xff, codeUnitsSizeInBytes);
            CharPointerType (codeUnits.data()).writeAll (s.toUTF8());
            test.expectEquals (Txt (CharPointerType (codeUnits.data())), s);

            test.expect (CharPointerType::isValidString (codeUnits.data(), codeUnitsSizeInBytes));
        }
    };

    static Txt createRandomWideCharString (Random& r, size_t length)
    {
        std::vector<t32> characters (length, 0);

        for (auto& character : characters)
        {
            if (r.nextBool())
            {
                do
                {
                    character = (t32) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (character));
            }
            else
            {
                character = (t32) (1 + r.nextInt (0xff));
            }
        }

        characters.push_back (0);

        return CharPointer_UTF32 (characters.data());
    }

    z0 runTest() override
    {
        Random r = getRandom();

        beginTest ("Basics");
        {
            expect (Txt().length() == 0);
            expect (Txt() == Txt());
            Txt s1, s2 ("abcd");
            expect (s1.isEmpty() && ! s1.isNotEmpty());
            expect (s2.isNotEmpty() && ! s2.isEmpty());
            expect (s2.length() == 4);
            s1 = "abcd";
            expect (s2 == s1 && s1 == s2);
            expect (s1 == "abcd" && s1 == L"abcd");
            expect (Txt ("abcd") == Txt (L"abcd"));
            expect (Txt ("abcdefg", 4) == L"abcd");
            expect (Txt ("abcdefg", 4) == Txt (L"abcdefg", 4));
            expect (Txt::charToString ('x') == "x");
            expect (Txt::charToString (0) == Txt());
            expect (s2 + "e" == "abcde" && s2 + 'e' == "abcde");
            expect (s2 + L'e' == "abcde" && s2 + L"e" == "abcde");
            expect (s1.equalsIgnoreCase ("abcD") && s1 < "abce" && s1 > "abbb");
            expect (s1.startsWith ("ab") && s1.startsWith ("abcd") && ! s1.startsWith ("abcde"));
            expect (s1.startsWithIgnoreCase ("aB") && s1.endsWithIgnoreCase ("CD"));
            expect (s1.endsWith ("bcd") && ! s1.endsWith ("aabcd"));
            expectEquals (s1.indexOf (Txt()), 0);
            expectEquals (s1.indexOfIgnoreCase (Txt()), 0);
            expect (s1.startsWith (Txt()) && s1.endsWith (Txt()) && s1.contains (Txt()));
            expect (s1.contains ("cd") && s1.contains ("ab") && s1.contains ("abcd"));
            expect (s1.containsChar ('a'));
            expect (! s1.containsChar ('x'));
            expect (! s1.containsChar (0));
            expect (Txt ("abc foo bar").containsWholeWord ("abc") && Txt ("abc foo bar").containsWholeWord ("abc"));
        }

        beginTest ("Operations");
        {
            Txt s ("012345678");
            expect (s.hashCode() != 0);
            expect (s.hashCode64() != 0);
            expect (s.hashCode() != (s + s).hashCode());
            expect (s.hashCode64() != (s + s).hashCode64());
            expect (s.compare (Txt ("012345678")) == 0);
            expect (s.compare (Txt ("012345679")) < 0);
            expect (s.compare (Txt ("012345676")) > 0);
            expect (Txt ("a").compareNatural ("A") == 0);
            expect (Txt ("A").compareNatural ("B") < 0);
            expect (Txt ("a").compareNatural ("B") < 0);
            expect (Txt ("10").compareNatural ("2") > 0);
            expect (Txt ("Abc 10").compareNatural ("aBC 2") > 0);
            expect (Txt ("Abc 1").compareNatural ("aBC 2") < 0);
            expect (s.substring (2, 3) == Txt::charToString (s[2]));
            expect (s.substring (0, 1) == Txt::charToString (s[0]));
            expect (s.getLastCharacter() == s [s.length() - 1]);
            expect (Txt::charToString (s.getLastCharacter()) == s.getLastCharacters (1));
            expect (s.substring (0, 3) == L"012");
            expect (s.substring (0, 100) == s);
            expect (s.substring (-1, 100) == s);
            expect (s.substring (3) == "345678");
            expect (s.indexOf (Txt (L"45")) == 4);
            expect (Txt ("444445").indexOf ("45") == 4);
            expect (Txt ("444445").lastIndexOfChar ('4') == 4);
            expect (Txt ("45454545x").lastIndexOf (Txt (L"45")) == 6);
            expect (Txt ("45454545x").lastIndexOfAnyOf ("456") == 7);
            expect (Txt ("45454545x").lastIndexOfAnyOf (Txt (L"456x")) == 8);
            expect (Txt ("abABaBaBa").lastIndexOfIgnoreCase ("aB") == 6);
            expect (s.indexOfChar (L'4') == 4);
            expect (s + s == "012345678012345678");
            expect (s.startsWith (s));
            expect (s.startsWith (s.substring (0, 4)));
            expect (s.startsWith (s.dropLastCharacters (4)));
            expect (s.endsWith (s.substring (5)));
            expect (s.endsWith (s));
            expect (s.contains (s.substring (3, 6)));
            expect (s.contains (s.substring (3)));
            expect (s.startsWithChar (s[0]));
            expect (s.endsWithChar (s.getLastCharacter()));
            expect (s [s.length()] == 0);
            expect (Txt ("abcdEFGH").toLowerCase() == Txt ("abcdefgh"));
            expect (Txt ("abcdEFGH").toUpperCase() == Txt ("ABCDEFGH"));

            expect (Txt (StringRef ("abc")) == "abc");
            expect (Txt (StringRef ("abc")) == StringRef ("abc"));
            expect (Txt ("abc") + StringRef ("def") == "abcdef");

            expect (Txt ("0x00").getHexValue32() == 0);
            expect (Txt ("0x100").getHexValue32() == 256);

            Txt s2 ("123");
            s2 << ((i32) 4) << ((short) 5) << "678" << L"9" << '0';
            s2 += "xyz";
            expect (s2 == "1234567890xyz");
            s2 += (i32) 123;
            expect (s2 == "1234567890xyz123");
            s2 += (z64) 123;
            expect (s2 == "1234567890xyz123123");
            s2 << StringRef ("def");
            expect (s2 == "1234567890xyz123123def");

            // i16
            {
                Txt numStr (std::numeric_limits<i16>::max());
                expect (numStr == "32767");
            }
            {
                Txt numStr (std::numeric_limits<i16>::min());
                expect (numStr == "-32768");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<i16>::max();
                expect (numStr == "32767");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<i16>::min();
                expect (numStr == "-32768");
            }
            // i32
            {
                Txt numStr (std::numeric_limits<i32>::max());
                expect (numStr == "2147483647");
            }
            {
                Txt numStr (std::numeric_limits<i32>::min());
                expect (numStr == "-2147483648");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<i32>::max();
                expect (numStr == "2147483647");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<i32>::min();
                expect (numStr == "-2147483648");
            }
            // u32
            {
                Txt numStr (std::numeric_limits<u32>::max());
                expect (numStr == "4294967295");
            }
            {
                Txt numStr (std::numeric_limits<u32>::min());
                expect (numStr == "0");
            }
            // z64
            {
                Txt numStr (std::numeric_limits<z64>::max());
                expect (numStr == "9223372036854775807");
            }
            {
                Txt numStr (std::numeric_limits<z64>::min());
                expect (numStr == "-9223372036854775808");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<z64>::max();
                expect (numStr == "9223372036854775807");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<z64>::min();
                expect (numStr == "-9223372036854775808");
            }
            // zu64
            {
                Txt numStr (std::numeric_limits<zu64>::max());
                expect (numStr == "18446744073709551615");
            }
            {
                Txt numStr (std::numeric_limits<zu64>::min());
                expect (numStr == "0");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<zu64>::max();
                expect (numStr == "18446744073709551615");
            }
            {
                Txt numStr;
                numStr << std::numeric_limits<zu64>::min();
                expect (numStr == "0");
            }
            // size_t
            {
                Txt numStr (std::numeric_limits<size_t>::min());
                expect (numStr == "0");
            }

            beginTest ("Numeric conversions");
            expect (Txt().getIntValue() == 0);
            expectEquals (Txt().getDoubleValue(), 0.0);
            expectEquals (Txt().getFloatValue(), 0.0f);
            expect (s.getIntValue() == 12345678);
            expect (s.getLargeIntValue() == (z64) 12345678);
            expectEquals (s.getDoubleValue(), 12345678.0);
            expectEquals (s.getFloatValue(), 12345678.0f);
            expect (Txt (-1234).getIntValue() == -1234);
            expect (Txt ((z64) -1234).getLargeIntValue() == -1234);
            expectEquals (Txt (-1234.56).getDoubleValue(), -1234.56);
            expectEquals (Txt (-1234.56f).getFloatValue(), -1234.56f);
            expect (Txt (std::numeric_limits<i32>::max()).getIntValue() == std::numeric_limits<i32>::max());
            expect (Txt (std::numeric_limits<i32>::min()).getIntValue() == std::numeric_limits<i32>::min());
            expect (Txt (std::numeric_limits<z64>::max()).getLargeIntValue() == std::numeric_limits<z64>::max());
            expect (Txt (std::numeric_limits<z64>::min()).getLargeIntValue() == std::numeric_limits<z64>::min());
            expect (("xyz" + s).getTrailingIntValue() == s.getIntValue());
            expect (Txt ("xyz-5").getTrailingIntValue() == -5);
            expect (Txt ("-12345").getTrailingIntValue() == -12345);
            expect (s.getHexValue32() == 0x12345678);
            expect (s.getHexValue64() == (z64) 0x12345678);
            expect (Txt::toHexString (0x1234abcd).equalsIgnoreCase ("1234abcd"));
            expect (Txt::toHexString ((z64) 0x1234abcd).equalsIgnoreCase ("1234abcd"));
            expect (Txt::toHexString ((short) 0x12ab).equalsIgnoreCase ("12ab"));
            expect (Txt::toHexString ((size_t) 0x12ab).equalsIgnoreCase ("12ab"));
            expect (Txt::toHexString ((i64) 0x12ab).equalsIgnoreCase ("12ab"));
            expect (Txt::toHexString ((i8)  -1).equalsIgnoreCase ("ff"));
            expect (Txt::toHexString ((i16) -1).equalsIgnoreCase ("ffff"));
            expect (Txt::toHexString ((i32) -1).equalsIgnoreCase ("ffffffff"));
            expect (Txt::toHexString ((z64) -1).equalsIgnoreCase ("ffffffffffffffff"));

            u8 data[] = { 1, 2, 3, 4, 0xa, 0xb, 0xc, 0xd };
            expect (Txt::toHexString (data, 8, 0).equalsIgnoreCase ("010203040a0b0c0d"));
            expect (Txt::toHexString (data, 8, 1).equalsIgnoreCase ("01 02 03 04 0a 0b 0c 0d"));
            expect (Txt::toHexString (data, 8, 2).equalsIgnoreCase ("0102 0304 0a0b 0c0d"));

            expectEquals (Txt (12345.67, 4),     Txt ("12345.6700"));
            expectEquals (Txt (12345.67, 6),     Txt ("12345.670000"));
            expectEquals (Txt (2589410.5894, 7), Txt ("2589410.5894000"));
            expectEquals (Txt (12345.67, 8),     Txt ("12345.67000000"));
            expectEquals (Txt (1e19, 4),         Txt ("10000000000000000000.0000"));
            expectEquals (Txt (1e-34, 36),       Txt ("0.000000000000000000000000000000000100"));
            expectEquals (Txt (1.39, 1),         Txt ("1.4"));

            expectEquals (Txt (12345.67, 4,     true), Txt ("1.2346e+04"));
            expectEquals (Txt (12345.67, 6,     true), Txt ("1.234567e+04"));
            expectEquals (Txt (2589410.5894, 7, true), Txt ("2.5894106e+06"));
            expectEquals (Txt (12345.67, 8,     true), Txt ("1.23456700e+04"));
            expectEquals (Txt (1e19, 4,         true), Txt ("1.0000e+19"));
            expectEquals (Txt (1e-34, 5,        true), Txt ("1.00000e-34"));
            expectEquals (Txt (1.39, 1,         true), Txt ("1.4e+00"));

            beginTest ("Subsections");
            Txt s3;
            s3 = "abcdeFGHIJ";
            expect (s3.equalsIgnoreCase ("ABCdeFGhiJ"));
            expect (s3.compareIgnoreCase (L"ABCdeFGhiJ") == 0);
            expect (s3.containsIgnoreCase (s3.substring (3)));
            expect (s3.indexOfAnyOf ("xyzf", 2, true) == 5);
            expect (s3.indexOfAnyOf (Txt (L"xyzf"), 2, false) == -1);
            expect (s3.indexOfAnyOf ("xyzF", 2, false) == 5);
            expect (s3.containsAnyOf (Txt (L"zzzFs")));
            expect (s3.startsWith ("abcd"));
            expect (s3.startsWithIgnoreCase (Txt (L"abCD")));
            expect (s3.startsWith (Txt()));
            expect (s3.startsWithChar ('a'));
            expect (s3.endsWith (Txt ("HIJ")));
            expect (s3.endsWithIgnoreCase (Txt (L"Hij")));
            expect (s3.endsWith (Txt()));
            expect (s3.endsWithChar (L'J'));
            expect (s3.indexOf ("HIJ") == 7);
            expect (s3.indexOf (Txt (L"HIJK")) == -1);
            expect (s3.indexOfIgnoreCase ("hij") == 7);
            expect (s3.indexOfIgnoreCase (Txt (L"hijk")) == -1);
            expect (s3.toStdString() == s3.toRawUTF8());

            Txt s4 (s3);
            s4.append (Txt ("xyz123"), 3);
            expect (s4 == s3 + "xyz");

            expect (Txt (1234) < Txt (1235));
            expect (Txt (1235) > Txt (1234));
            expect (Txt (1234) >= Txt (1234));
            expect (Txt (1234) <= Txt (1234));
            expect (Txt (1235) >= Txt (1234));
            expect (Txt (1234) <= Txt (1235));

            Txt s5 ("word word2 word3");
            expect (s5.containsWholeWord (Txt ("word2")));
            expect (s5.indexOfWholeWord ("word2") == 5);
            expect (s5.containsWholeWord (Txt (L"word")));
            expect (s5.containsWholeWord ("word3"));
            expect (s5.containsWholeWord (s5));
            expect (s5.containsWholeWordIgnoreCase (Txt (L"Word2")));
            expect (s5.indexOfWholeWordIgnoreCase ("Word2") == 5);
            expect (s5.containsWholeWordIgnoreCase (Txt (L"Word")));
            expect (s5.containsWholeWordIgnoreCase ("Word3"));
            expect (! s5.containsWholeWordIgnoreCase (Txt (L"Wordx")));
            expect (! s5.containsWholeWordIgnoreCase ("xWord2"));
            expect (s5.containsNonWhitespaceChars());
            expect (s5.containsOnly ("ordw23 "));
            expect (! Txt (" \n\r\t").containsNonWhitespaceChars());

            expect (s5.matchesWildcard (Txt (L"wor*"), false));
            expect (s5.matchesWildcard ("wOr*", true));
            expect (s5.matchesWildcard (Txt (L"*word3"), true));
            expect (s5.matchesWildcard ("*word?", true));
            expect (s5.matchesWildcard (Txt (L"Word*3"), true));
            expect (! s5.matchesWildcard (Txt (L"*34"), true));
            expect (Txt ("xx**y").matchesWildcard ("*y", true));
            expect (Txt ("xx**y").matchesWildcard ("x*y", true));
            expect (Txt ("xx**y").matchesWildcard ("xx*y", true));
            expect (Txt ("xx**y").matchesWildcard ("xx*", true));
            expect (Txt ("xx?y").matchesWildcard ("x??y", true));
            expect (Txt ("xx?y").matchesWildcard ("xx?y", true));
            expect (! Txt ("xx?y").matchesWildcard ("xx?y?", true));
            expect (Txt ("xx?y").matchesWildcard ("xx??", true));

            expectEquals (s5.fromFirstOccurrenceOf (Txt(), true, false), s5);
            expectEquals (s5.fromFirstOccurrenceOf ("xword2", true, false), s5.substring (100));
            expectEquals (s5.fromFirstOccurrenceOf (Txt (L"word2"), true, false), s5.substring (5));
            expectEquals (s5.fromFirstOccurrenceOf ("Word2", true, true), s5.substring (5));
            expectEquals (s5.fromFirstOccurrenceOf ("word2", false, false), s5.getLastCharacters (6));
            expectEquals (s5.fromFirstOccurrenceOf ("Word2", false, true), s5.getLastCharacters (6));

            expectEquals (s5.fromLastOccurrenceOf (Txt(), true, false), s5);
            expectEquals (s5.fromLastOccurrenceOf ("wordx", true, false), s5);
            expectEquals (s5.fromLastOccurrenceOf ("word", true, false), s5.getLastCharacters (5));
            expectEquals (s5.fromLastOccurrenceOf ("worD", true, true), s5.getLastCharacters (5));
            expectEquals (s5.fromLastOccurrenceOf ("word", false, false), s5.getLastCharacters (1));
            expectEquals (s5.fromLastOccurrenceOf ("worD", false, true), s5.getLastCharacters (1));

            expect (s5.upToFirstOccurrenceOf (Txt(), true, false).isEmpty());
            expectEquals (s5.upToFirstOccurrenceOf ("word4", true, false), s5);
            expectEquals (s5.upToFirstOccurrenceOf ("word2", true, false), s5.substring (0, 10));
            expectEquals (s5.upToFirstOccurrenceOf ("Word2", true, true), s5.substring (0, 10));
            expectEquals (s5.upToFirstOccurrenceOf ("word2", false, false), s5.substring (0, 5));
            expectEquals (s5.upToFirstOccurrenceOf ("Word2", false, true), s5.substring (0, 5));

            expectEquals (s5.upToLastOccurrenceOf (Txt(), true, false), s5);
            expectEquals (s5.upToLastOccurrenceOf ("zword", true, false), s5);
            expectEquals (s5.upToLastOccurrenceOf ("word", true, false), s5.dropLastCharacters (1));
            expectEquals (s5.dropLastCharacters (1).upToLastOccurrenceOf ("word", true, false), s5.dropLastCharacters (1));
            expectEquals (s5.upToLastOccurrenceOf ("Word", true, true), s5.dropLastCharacters (1));
            expectEquals (s5.upToLastOccurrenceOf ("word", false, false), s5.dropLastCharacters (5));
            expectEquals (s5.upToLastOccurrenceOf ("Word", false, true), s5.dropLastCharacters (5));

            expectEquals (s5.replace ("word", "xyz", false), Txt ("xyz xyz2 xyz3"));
            expect (s5.replace ("Word", "xyz", true) == "xyz xyz2 xyz3");
            expect (s5.dropLastCharacters (1).replace ("Word", Txt ("xyz"), true) == L"xyz xyz2 xyz");
            expect (s5.replace ("Word", "", true) == " 2 3");
            expectEquals (s5.replace ("Word2", "xyz", true), Txt ("word xyz word3"));
            expect (s5.replaceCharacter (L'w', 'x') != s5);
            expectEquals (s5.replaceCharacter ('w', L'x').replaceCharacter ('x', 'w'), s5);
            expect (s5.replaceCharacters ("wo", "xy") != s5);
            expectEquals (s5.replaceCharacters ("wo", "xy").replaceCharacters ("xy", "wo"), s5);
            expectEquals (s5.retainCharacters ("1wordxya"), Txt ("wordwordword"));
            expect (s5.retainCharacters (Txt()).isEmpty());
            expect (s5.removeCharacters ("1wordxya") == " 2 3");
            expectEquals (s5.removeCharacters (Txt()), s5);
            expect (s5.initialSectionContainingOnly ("word") == L"word");
            expect (Txt ("word").initialSectionContainingOnly ("word") == L"word");
            expectEquals (s5.initialSectionNotContaining (Txt ("xyz ")), Txt ("word"));
            expectEquals (s5.initialSectionNotContaining (Txt (";[:'/")), s5);
            expect (! s5.isQuotedString());
            expect (s5.quoted().isQuotedString());
            expect (! s5.quoted().unquoted().isQuotedString());
            expect (! Txt ("x'").isQuotedString());
            expect (Txt ("'x").isQuotedString());

            Txt s6 (" \t xyz  \t\r\n");
            expectEquals (s6.trim(), Txt ("xyz"));
            expect (s6.trim().trim() == "xyz");
            expectEquals (s5.trim(), s5);
            expectEquals (s6.trimStart().trimEnd(), s6.trim());
            expectEquals (s6.trimStart().trimEnd(), s6.trimEnd().trimStart());
            expectEquals (s6.trimStart().trimStart().trimEnd().trimEnd(), s6.trimEnd().trimStart());
            expect (s6.trimStart() != s6.trimEnd());
            expectEquals (("\t\r\n " + s6 + "\t\n \r").trim(), s6.trim());
            expect (Txt::repeatedString ("xyz", 3) == L"xyzxyzxyz");
        }

        beginTest ("UTF conversions");
        {
            TestUTFConversion <CharPointer_UTF32>::test (*this, r);
            TestUTFConversion <CharPointer_UTF8>::test (*this, r);
            TestUTFConversion <CharPointer_UTF16>::test (*this, r);
        }

        beginTest ("StringArray");
        {
            StringArray s;
            s.addTokens ("4,3,2,1,0", ";,", "x");
            expectEquals (s.size(), 5);

            expectEquals (s.joinIntoString ("-"), Txt ("4-3-2-1-0"));
            s.remove (2);
            expectEquals (s.joinIntoString ("--"), Txt ("4--3--1--0"));
            expectEquals (s.joinIntoString (StringRef()), Txt ("4310"));
            s.clear();
            expectEquals (s.joinIntoString ("x"), Txt());

            StringArray toks;
            toks.addTokens ("x,,", ";,", "");
            expectEquals (toks.size(), 3);
            expectEquals (toks.joinIntoString ("-"), Txt ("x--"));
            toks.clear();

            toks.addTokens (",x,", ";,", "");
            expectEquals (toks.size(), 3);
            expectEquals (toks.joinIntoString ("-"), Txt ("-x-"));
            toks.clear();

            toks.addTokens ("x,'y,z',", ";,", "'");
            expectEquals (toks.size(), 3);
            expectEquals (toks.joinIntoString ("-"), Txt ("x-'y,z'-"));
        }

        beginTest ("var");
        {
            var v1 = 0;
            var v2 = 0.16;
            var v3 = "0.16";
            var v4 = (z64) 0;
            var v5 = 0.0;
            expect (! v2.equals (v1));
            expect (! v1.equals (v2));
            expect (v2.equals (v3));
            expect (! v3.equals (v1));
            expect (! v1.equals (v3));
            expect (v1.equals (v4));
            expect (v4.equals (v1));
            expect (v5.equals (v4));
            expect (v4.equals (v5));
            expect (! v2.equals (v4));
            expect (! v4.equals (v2));
        }

        beginTest ("Significant figures");
        {
            // Integers

            expectEquals (Txt::toDecimalStringWithSignificantFigures (13, 1), Txt ("10"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (13, 2), Txt ("13"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (13, 3), Txt ("13.0"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (13, 4), Txt ("13.00"));

            expectEquals (Txt::toDecimalStringWithSignificantFigures (19368, 1), Txt ("20000"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (19348, 3), Txt ("19300"));

            expectEquals (Txt::toDecimalStringWithSignificantFigures (-5, 1), Txt ("-5"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (-5, 3), Txt ("-5.00"));

            // Zero

            expectEquals (Txt::toDecimalStringWithSignificantFigures (0, 1), Txt ("0"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (0, 2), Txt ("0.0"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (0, 3), Txt ("0.00"));

            // Floating point

            expectEquals (Txt::toDecimalStringWithSignificantFigures (19.0, 1), Txt ("20"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (19.0, 2), Txt ("19"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (19.0, 3), Txt ("19.0"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (19.0, 4), Txt ("19.00"));

            expectEquals (Txt::toDecimalStringWithSignificantFigures (-5.45, 1), Txt ("-5"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (-5.45, 3), Txt ("-5.45"));

            expectEquals (Txt::toDecimalStringWithSignificantFigures (12345.6789, 9), Txt ("12345.6789"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (12345.6789, 8), Txt ("12345.679"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (12345.6789, 5), Txt ("12346"));

            expectEquals (Txt::toDecimalStringWithSignificantFigures (0.00028647, 6), Txt ("0.000286470"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (0.0028647,  6), Txt ("0.00286470"));
            expectEquals (Txt::toDecimalStringWithSignificantFigures (2.8647,     6), Txt ("2.86470"));

            expectEquals (Txt::toDecimalStringWithSignificantFigures (-0.0000000000019, 1), Txt ("-0.000000000002"));
        }

        beginTest ("Float trimming");
        {
            {
                StringPairArray tests;
                tests.set ("1", "1");
                tests.set ("1.0", "1.0");
                tests.set ("-1", "-1");
                tests.set ("-100", "-100");
                tests.set ("110", "110");
                tests.set ("9090", "9090");
                tests.set ("1000.0", "1000.0");
                tests.set ("1.0", "1.0");
                tests.set ("-1.00", "-1.0");
                tests.set ("1.20", "1.2");
                tests.set ("1.300", "1.3");
                tests.set ("1.301", "1.301");
                tests.set ("1e", "1");
                tests.set ("-1e+", "-1");
                tests.set ("1e-", "1");
                tests.set ("1e0", "1");
                tests.set ("1e+0", "1");
                tests.set ("1e-0", "1");
                tests.set ("1e000", "1");
                tests.set ("1e+000", "1");
                tests.set ("-1e-000", "-1");
                tests.set ("1e100", "1e100");
                tests.set ("100e100", "100e100");
                tests.set ("100.0e0100", "100.0e100");
                tests.set ("-1e1", "-1e1");
                tests.set ("1e10", "1e10");
                tests.set ("-1e+10", "-1e10");
                tests.set ("1e-10", "1e-10");
                tests.set ("1e0010", "1e10");
                tests.set ("1e-0010", "1e-10");
                tests.set ("1e-1", "1e-1");
                tests.set ("-1.0e1", "-1.0e1");
                tests.set ("1.0e-1", "1.0e-1");
                tests.set ("1.00e-1", "1.0e-1");
                tests.set ("1.001e1", "1.001e1");
                tests.set ("1.010e+1", "1.01e1");
                tests.set ("-1.1000e1", "-1.1e1");

                for (auto& input : tests.getAllKeys())
                    expectEquals (reduceLengthOfFloatString (input), tests[input]);
            }

            {
                std::map<f64, Txt> tests;
                tests[1] = "1.0";
                tests[1.1] = "1.1";
                tests[1.01] = "1.01";
                tests[0.76378] = "7.6378e-1";
                tests[-10] = "-1.0e1";
                tests[10.01] = "1.001e1";
                tests[10691.01] = "1.069101e4";
                tests[0.0123] = "1.23e-2";
                tests[-3.7e-27] = "-3.7e-27";
                tests[1e+40] = "1.0e40";

                for (auto& test : tests)
                    expectEquals (reduceLengthOfFloatString (Txt (test.first, 15, true)), test.second);
            }
        }

        beginTest ("Serialisation");
        {
            std::map<f64, Txt> tests;

            tests[364] = "364.0";
            tests[1e7] = "1.0e7";
            tests[12345678901] = "1.2345678901e10";

            tests[1234567890123456.7] = "1.234567890123457e15";
            tests[12345678.901234567] = "1.234567890123457e7";
            tests[1234567.8901234567] = "1.234567890123457e6";
            tests[123456.78901234567] = "123456.7890123457";
            tests[12345.678901234567] = "12345.67890123457";
            tests[1234.5678901234567] = "1234.567890123457";
            tests[123.45678901234567] = "123.4567890123457";
            tests[12.345678901234567] = "12.34567890123457";
            tests[1.2345678901234567] = "1.234567890123457";
            tests[0.12345678901234567] = "0.1234567890123457";
            tests[0.012345678901234567] = "0.01234567890123457";
            tests[0.0012345678901234567] = "0.001234567890123457";
            tests[0.00012345678901234567] = "0.0001234567890123457";
            tests[0.000012345678901234567] = "0.00001234567890123457";
            tests[0.0000012345678901234567] = "1.234567890123457e-6";
            tests[0.00000012345678901234567] = "1.234567890123457e-7";

            for (auto& test : tests)
            {
                expectEquals (serialiseDouble (test.first), test.second);
                expectEquals (serialiseDouble (-test.first), "-" + test.second);
            }

            expectEquals (serialiseDouble (1.0, 0), Txt ("1.0"));
            expectEquals (serialiseDouble (1.0, 1), Txt ("1.0"));
            expectEquals (serialiseDouble (1.0, 2), Txt ("1.0"));
            expectEquals (serialiseDouble (1.0, 3), Txt ("1.0"));
            expectEquals (serialiseDouble (1.0, 10), Txt ("1.0"));

            expectEquals (serialiseDouble (4.567, 0), Txt ("4.567"));
            expectEquals (serialiseDouble (4.567, 1), Txt ("4.6"));
            expectEquals (serialiseDouble (4.567, 2), Txt ("4.57"));
            expectEquals (serialiseDouble (4.567, 3), Txt ("4.567"));
            expectEquals (serialiseDouble (4.567, 10), Txt ("4.567"));
        }

        beginTest ("Loops");
        {
            Txt str (CharPointer_UTF8 ("\xc2\xaf\\_(\xe3\x83\x84)_/\xc2\xaf"));
            std::vector<t32> parts { 175, 92, 95, 40, 12484, 41, 95, 47, 175 };
            size_t index = 0;

            for (auto c : str)
                expectEquals (c, parts[index++]);
        }
    }
};

static StringTests stringUnitTests;

#endif

} // namespace drx
