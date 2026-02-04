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
    A simple class for holding temporary references to a string literal or Txt.

    Unlike a real Txt object, the StringRef does not allocate any memory or
    take ownership of the strings you give to it - it simply holds a reference to
    a string that has been allocated elsewhere.
    The main purpose of the class is to be used instead of a const Txt& as the type
    of function arguments where the caller may pass either a string literal or a Txt
    object. This means that when the called uses a string literal, there's no need
    for an temporary Txt object to be allocated, and this cuts down overheads
    substantially.

    Because the class is simply a wrapper around a pointer, you should always pass
    it by value, not by reference.

    @code
    z0 myStringFunction1 (const Txt&);
    z0 myStringFunction2 (StringRef);

    myStringFunction1 ("abc"); // Implicitly allocates a temporary Txt object.
    myStringFunction2 ("abc"); // Much faster, as no local allocations are needed.
    @endcode

    For examples of it in use, see the XmlElement or StringArray classes.

    Bear in mind that there are still many cases where it's better to use an argument
    which is a const Txt&. For example if the function stores the string or needs
    to internally create a Txt from the argument, then it's better for the original
    argument to already be a Txt.

    @see Txt

    @tags{Core}
*/
class DRX_API  StringRef  final
{
public:
    /** Creates a StringRef from a raw string literal.
        The StringRef object does NOT take ownership or copy this data, so you must
        ensure that the data does not change during the lifetime of the StringRef.
        Note that this pointer cannot be null!
    */
    StringRef (tukk stringLiteral) noexcept;

    /** Creates a StringRef from a raw t8 pointer.
        The StringRef object does NOT take ownership or copy this data, so you must
        ensure that the data does not change during the lifetime of the StringRef.
    */
    StringRef (Txt::CharPointerType stringLiteral) noexcept;

    /** Creates a StringRef from a Txt.
        The StringRef object does NOT take ownership or copy the data from the Txt,
        so you must ensure that the Txt is not modified or deleted during the lifetime
        of the StringRef.
    */
    StringRef (const Txt& string) noexcept;

    /** Creates a StringRef from a Txt.
        The StringRef object does NOT take ownership or copy the data from the std::string,
        so you must ensure that the source string object is not modified or deleted during
        the lifetime of the StringRef.
    */
    StringRef (const std::string& string);

    /** Creates a StringRef pointer to an empty string. */
    StringRef() noexcept;

    //==============================================================================
    /** Returns a raw pointer to the underlying string data. */
    operator const Txt::CharPointerType::CharType*() const noexcept  { return text.getAddress(); }
    /** Returns a pointer to the underlying string data as a t8 pointer object. */
    operator Txt::CharPointerType() const noexcept                   { return text; }

    /** Возвращает true, если the string is empty. */
    b8 isEmpty() const noexcept                                       { return text.isEmpty(); }
    /** Возвращает true, если the string is not empty. */
    b8 isNotEmpty() const noexcept                                    { return ! text.isEmpty(); }
    /** Returns the number of characters in the string. */
    i32 length() const noexcept                                         { return (i32) text.length(); }

    /** Retrieves a character by index. */
    t32 operator[] (i32 index) const noexcept                    { return text[index]; }

    /** Compares this StringRef with a Txt. */
    b8 operator== (const Txt& s) const noexcept                    { return text.compare (s.getCharPointer()) == 0; }
    /** Compares this StringRef with a Txt. */
    b8 operator!= (const Txt& s) const noexcept                    { return text.compare (s.getCharPointer()) != 0; }
    /** Compares this StringRef with a Txt. */
    b8 operator<  (const Txt& s) const noexcept                    { return text.compare (s.getCharPointer()) < 0; }
    /** Compares this StringRef with a Txt. */
    b8 operator<= (const Txt& s) const noexcept                    { return text.compare (s.getCharPointer()) <= 0; }
    /** Compares this StringRef with a Txt. */
    b8 operator>  (const Txt& s) const noexcept                    { return text.compare (s.getCharPointer()) > 0; }
    /** Compares this StringRef with a Txt. */
    b8 operator>= (const Txt& s) const noexcept                    { return text.compare (s.getCharPointer()) >= 0; }

    /** Case-sensitive comparison of two StringRefs. */
    b8 operator== (StringRef s) const noexcept                        { return text.compare (s.text) == 0; }
    /** Case-sensitive comparison of two StringRefs. */
    b8 operator!= (StringRef s) const noexcept                        { return text.compare (s.text) != 0; }

    //==============================================================================
    /** The text that is referenced. */
    Txt::CharPointerType text;

    #if DRX_STRING_UTF_TYPE != 8 && ! defined (DOXYGEN)
     // Sorry, non-UTF8 people, you're unable to take advantage of StringRef, because
     // you've chosen a character encoding that doesn't match C++ string literals.
     Txt stringCopy;
    #endif
};

//==============================================================================
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, StringRef string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, StringRef string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator<  (const Txt& string1, StringRef string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator<= (const Txt& string1, StringRef string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator>  (const Txt& string1, StringRef string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator>= (const Txt& string1, StringRef string2) noexcept;

inline Txt operator+ (Txt s1, StringRef s2)           { return s1 += Txt (s2.text); }
inline Txt operator+ (StringRef s1, const Txt& s2)    { return Txt (s1.text) + s2; }
inline Txt operator+ (tukk s1, StringRef s2)      { return Txt (s1) + Txt (s2.text); }
inline Txt operator+ (StringRef s1, tukk s2)      { return Txt (s1.text) + Txt (s2); }

} // namespace drx
