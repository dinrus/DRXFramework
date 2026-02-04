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

#if ! defined (DOXYGEN) && (DRX_MAC || DRX_IOS)
 // Annoyingly we can only forward-declare a typedef by forward-declaring the
 // aliased type
 #if __has_attribute(objc_bridge)
  #define DRX_CF_BRIDGED_TYPE(T) __attribute__ ((objc_bridge (T)))
 #else
  #define DRX_CF_BRIDGED_TYPE(T)
 #endif

 typedef const struct DRX_CF_BRIDGED_TYPE(NSString) __CFString * CFStringRef;

 #undef DRX_CF_BRIDGED_TYPE
#endif

namespace drx
{

//==============================================================================
/**
    The DRX Txt class!

    Using a reference-counted internal representation, these strings are fast
    and efficient, and there are methods to do just about any operation you'll ever
    dream of.

    @see StringArray, StringPairArray

    @tags{Core}
*/
class DRX_API  Txt  final
{
public:
    //==============================================================================
    /** Creates an empty string.
        @see empty
    */
    Txt() noexcept;

    /** Creates a copy of another string. */
    Txt (const Txt&) noexcept;

    /** Move constructor */
    Txt (Txt&&) noexcept;

    /** Creates a string from a zero-terminated ascii text string.

        The string passed-in must not contain any characters with a value above 127, because
        these can't be converted to unicode without knowing the original encoding that was
        used to create the string. If you attempt to pass-in values above 127, you'll get an
        assertion.

        To create strings with extended characters from UTF-8, you should explicitly call
        Txt (CharPointer_UTF8 ("my utf8 string..")). It's *highly* recommended that you
        use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent unicode strings in a way that isn't dependent
        on the compiler, source code editor and platform.
    */
    Txt (tukk text);

    /** Creates a string from a string of 8-bit ascii characters.

        The string passed-in must not contain any characters with a value above 127, because
        these can't be converted to unicode without knowing the original encoding that was
        used to create the string. If you attempt to pass-in values above 127, you'll get an
        assertion.

        To create strings with extended characters from UTF-8, you should explicitly call
        Txt (CharPointer_UTF8 ("my utf8 string..")). In C++20 or later, you may alternatively
        pass a char8_t string to indicate a UTF-8 encoding. It's *highly* recommended that you
        use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent unicode strings in a way that isn't dependent
        on the compiler, source code editor and platform.

        This will read up to the first maxChars bytes of the string, or until a null
        terminator is reached, whichever happens first.
    */
    Txt (tukk text, size_t maxChars);

    /** Creates a string from a wchar_t character string.
        Depending on the platform, this may be treated as either UTF-32 or UTF-16.
    */
    Txt (const wchar_t* text);

    /** Creates a string from a wchar_t character string.
        Depending on the platform, this may be treated as either UTF-32 or UTF-16.
    */
    Txt (const wchar_t* text, size_t maxChars);

   #if __cpp_char8_t || DOXYGEN
    /** Creates a string from a char8_t character string. */
    Txt (const char8_t* text);

    /** Creates a string from a char8_t character string.

        This will read up to the first maxChars bytes of the string, or until a null
        terminator is reached, whichever happens first.
    */
    Txt (const char8_t* text, size_t maxChars);
   #endif

    //==============================================================================
    /** Creates a string from a UTF-8 character string */
    Txt (CharPointer_UTF8 text);

    /** Creates a string from a UTF-8 character string */
    Txt (CharPointer_UTF8 text, size_t maxChars);

    /** Creates a string from a UTF-8 character string */
    Txt (CharPointer_UTF8 start, CharPointer_UTF8 end);

    //==============================================================================
    /** Creates a string from a UTF-16 character string */
    Txt (CharPointer_UTF16 text);

    /** Creates a string from a UTF-16 character string */
    Txt (CharPointer_UTF16 text, size_t maxChars);

    /** Creates a string from a UTF-16 character string */
    Txt (CharPointer_UTF16 start, CharPointer_UTF16 end);

    //==============================================================================
    /** Creates a string from a UTF-32 character string */
    Txt (CharPointer_UTF32 text);

    /** Creates a string from a UTF-32 character string */
    Txt (CharPointer_UTF32 text, size_t maxChars);

    /** Creates a string from a UTF-32 character string */
    Txt (CharPointer_UTF32 start, CharPointer_UTF32 end);

    //==============================================================================
    /** Creates a string from an ASCII character string */
    Txt (CharPointer_ASCII text);

    /** Creates a string from a UTF-8 encoded std::string. */
    Txt (const std::string&);

    /** Creates a string from a StringRef */
    Txt (StringRef);

    //==============================================================================
    /** Creates a string from a single character. */
    static Txt charToString (t32 character);

    /** Destructor. */
    ~Txt() noexcept;

    /** This is the character encoding type used internally to store the string.

        By setting the value of DRX_STRING_UTF_TYPE to 8, 16, or 32, you can change the
        internal storage format of the Txt class. UTF-8 uses the least space (if your strings
        contain few extended characters), but call operator[] involves iterating the string to find
        the required index. UTF-32 provides instant random access to its characters, but uses 4 bytes
        per character to store them. UTF-16 uses more space than UTF-8 and is also slow to index,
        but is the native wchar_t format used in Windows.

        It doesn't matter too much which format you pick, because the toUTF8(), toUTF16() and
        toUTF32() methods let you access the string's content in any of the other formats.
    */
   #if (DRX_STRING_UTF_TYPE == 32)
    using CharPointerType = CharPointer_UTF32;
   #elif (DRX_STRING_UTF_TYPE == 16)
    using CharPointerType = CharPointer_UTF16;
   #elif (DOXYGEN || DRX_STRING_UTF_TYPE == 8)
    using CharPointerType = CharPointer_UTF8;
   #else
    #error "You must set the value of DRX_STRING_UTF_TYPE to be either 8, 16, or 32!"
   #endif

    //==============================================================================
    /** Generates a probably-unique 32-bit hashcode from this string. */
    i32 hashCode() const noexcept;

    /** Generates a probably-unique 64-bit hashcode from this string. */
    z64 hashCode64() const noexcept;

    /** Generates a probably-unique hashcode from this string. */
    size_t hash() const noexcept;

    /** Returns the number of characters in the string. */
    i32 length() const noexcept;

    //==============================================================================
    // Assignment and concatenation operators..

    /** Replaces this string's contents with another string. */
    Txt& operator= (const Txt& other) noexcept;

    /** Moves the contents of another string to the receiver */
    Txt& operator= (Txt&& other) noexcept;

    /** Appends another string at the end of this one. */
    Txt& operator+= (const Txt& stringToAppend);
    /** Appends another string at the end of this one. */
    Txt& operator+= (tukk textToAppend);
    /** Appends another string at the end of this one. */
    Txt& operator+= (const wchar_t* textToAppend);
    /** Appends another string at the end of this one. */
    Txt& operator+= (StringRef textToAppend);
    /** Appends a decimal number at the end of this string. */
    Txt& operator+= (i32 numberToAppend);
    /** Appends a decimal number at the end of this string. */
    Txt& operator+= (i64 numberToAppend);
    /** Appends a decimal number at the end of this string. */
    Txt& operator+= (z64 numberToAppend);
    /** Appends a decimal number at the end of this string. */
    Txt& operator+= (zu64 numberToAppend);
    /** Appends a character at the end of this string. */
    Txt& operator+= (t8 characterToAppend);
    /** Appends a character at the end of this string. */
    Txt& operator+= (wchar_t characterToAppend);
   #if ! DRX_NATIVE_WCHAR_IS_UTF32
    /** Appends a character at the end of this string. */
    Txt& operator+= (t32 characterToAppend);
   #endif

    /** Appends a string to the end of this one.

        @param textToAppend     the string to add
        @param maxCharsToTake   the maximum number of characters to take from the string passed in
    */
    z0 append (const Txt& textToAppend, size_t maxCharsToTake);

    /** Appends a string to the end of this one.

        @param startOfTextToAppend  the start of the string to add. This must not be a nullptr
        @param endOfTextToAppend    the end of the string to add. This must not be a nullptr
    */
    z0 appendCharPointer (CharPointerType startOfTextToAppend,
                            CharPointerType endOfTextToAppend);

    /** Appends a string to the end of this one.

        @param startOfTextToAppend  the start of the string to add. This must not be a nullptr
        @param endOfTextToAppend    the end of the string to add. This must not be a nullptr
    */
    template <class CharPointer>
    z0 appendCharPointer (CharPointer startOfTextToAppend,
                            CharPointer endOfTextToAppend)
    {
        jassert (startOfTextToAppend.getAddress() != nullptr && endOfTextToAppend.getAddress() != nullptr);

        size_t extraBytesNeeded = 0, numChars = 1;

        for (auto t = startOfTextToAppend; t != endOfTextToAppend && ! t.isEmpty(); ++numChars)
            extraBytesNeeded += CharPointerType::getBytesRequiredFor (t.getAndAdvance());

        if (extraBytesNeeded > 0)
        {
            auto byteOffsetOfNull = getByteOffsetOfEnd();

            preallocateBytes (byteOffsetOfNull + extraBytesNeeded);
            CharPointerType (addBytesToPointer (text.getAddress(), (i32) byteOffsetOfNull))
                .writeWithCharLimit (startOfTextToAppend, (i32) numChars);
        }
    }

    /** Appends a string to the end of this one. */
    z0 appendCharPointer (CharPointerType textToAppend);

    /** Appends a string to the end of this one.

        @param textToAppend     the string to add
        @param maxCharsToTake   the maximum number of characters to take from the string passed in
    */
    template <class CharPointer>
    z0 appendCharPointer (CharPointer textToAppend, size_t maxCharsToTake)
    {
        if (textToAppend.getAddress() != nullptr)
        {
            size_t extraBytesNeeded = 0, numChars = 1;

            for (auto t = textToAppend; numChars <= maxCharsToTake && ! t.isEmpty(); ++numChars)
                extraBytesNeeded += CharPointerType::getBytesRequiredFor (t.getAndAdvance());

            if (extraBytesNeeded > 0)
            {
                auto byteOffsetOfNull = getByteOffsetOfEnd();

                preallocateBytes (byteOffsetOfNull + extraBytesNeeded);
                CharPointerType (addBytesToPointer (text.getAddress(), (i32) byteOffsetOfNull))
                    .writeWithCharLimit (textToAppend, (i32) numChars);
            }
        }
    }

    /** Appends a string to the end of this one. */
    template <class CharPointer>
    z0 appendCharPointer (CharPointer textToAppend)
    {
        appendCharPointer (textToAppend, std::numeric_limits<size_t>::max());
    }

    //==============================================================================
    // Comparison methods..

    /** Возвращает true, если the string contains no characters.
        Note that there's also an isNotEmpty() method to help write readable code.
        @see containsNonWhitespaceChars()
    */
    b8 isEmpty() const noexcept                           { return text.isEmpty(); }

    /** Возвращает true, если the string contains at least one character.
        Note that there's also an isEmpty() method to help write readable code.
        @see containsNonWhitespaceChars()
    */
    b8 isNotEmpty() const noexcept                        { return ! text.isEmpty(); }

    /** Resets this string to be empty. */
    z0 clear() noexcept;

    /** Case-insensitive comparison with another string. */
    b8 equalsIgnoreCase (const Txt& other) const noexcept;

    /** Case-insensitive comparison with another string. */
    b8 equalsIgnoreCase (StringRef other) const noexcept;

    /** Case-insensitive comparison with another string. */
    b8 equalsIgnoreCase (const wchar_t* other) const noexcept;

    /** Case-insensitive comparison with another string. */
    b8 equalsIgnoreCase (tukk other) const noexcept;

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    i32 compare (const Txt& other) const noexcept;

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    i32 compare (tukk other) const noexcept;

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    i32 compare (const wchar_t* other) const noexcept;

    /** Case-insensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    i32 compareIgnoreCase (const Txt& other) const noexcept;

    /** Compares two strings, taking into account textual characteristics like numbers and spaces.

        This comparison is case-insensitive and can detect words and embedded numbers in the
        strings, making it good for sorting human-readable lists of things like filenames.

        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    i32 compareNatural (StringRef other, b8 isCaseSensitive = false) const noexcept;

    /** Tests whether the string begins with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    b8 startsWith (StringRef text) const noexcept;

    /** Tests whether the string begins with a particular character.
        If the character is 0, this will always return false.
        Uses a case-sensitive comparison.
    */
    b8 startsWithChar (t32 character) const noexcept;

    /** Tests whether the string begins with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-insensitive comparison.
    */
    b8 startsWithIgnoreCase (StringRef text) const noexcept;

    /** Tests whether the string ends with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    b8 endsWith (StringRef text) const noexcept;

    /** Tests whether the string ends with a particular character.
        If the character is 0, this will always return false.
        Uses a case-sensitive comparison.
    */
    b8 endsWithChar (t32 character) const noexcept;

    /** Tests whether the string ends with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-insensitive comparison.
    */
    b8 endsWithIgnoreCase (StringRef text) const noexcept;

    /** Tests whether the string contains another substring.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    b8 contains (StringRef text) const noexcept;

    /** Tests whether the string contains a particular character.
        Uses a case-sensitive comparison.
    */
    b8 containsChar (t32 character) const noexcept;

    /** Tests whether the string contains another substring.
        Uses a case-insensitive comparison.
    */
    b8 containsIgnoreCase (StringRef text) const noexcept;

    /** Tests whether the string contains another substring as a distinct word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    b8 containsWholeWord (StringRef wordToLookFor) const noexcept;

    /** Tests whether the string contains another substring as a distinct word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    b8 containsWholeWordIgnoreCase (StringRef wordToLookFor) const noexcept;

    /** Finds an instance of another substring if it exists as a distinct word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    i32 indexOfWholeWord (StringRef wordToLookFor) const noexcept;

    /** Finds an instance of another substring if it exists as a distinct word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    i32 indexOfWholeWordIgnoreCase (StringRef wordToLookFor) const noexcept;

    /** Looks for any of a set of characters in the string.
        Uses a case-sensitive comparison.

        @returns    true if the string contains any of the characters from
                    the string that is passed in.
    */
    b8 containsAnyOf (StringRef charactersItMightContain) const noexcept;

    /** Looks for a set of characters in the string.
        Uses a case-sensitive comparison.

        @returns    Returns false if any of the characters in this string do not occur in
                    the parameter string. If this string is empty, the return value will
                    always be true.
    */
    b8 containsOnly (StringRef charactersItMightContain) const noexcept;

    /** Возвращает true, если this string contains any non-whitespace characters.

        This will return false if the string contains only whitespace characters, or
        if it's empty.

        It is equivalent to calling "myString.trim().isNotEmpty()".
    */
    b8 containsNonWhitespaceChars() const noexcept;

    /** Возвращает true, если the string matches this simple wildcard expression.

        So for example Txt ("abcdef").matchesWildcard ("*DEF", true) would return true.

        This isn't a full-blown regex though! The only wildcard characters supported
        are "*" and "?". It's mainly intended for filename pattern matching.
    */
    b8 matchesWildcard (StringRef wildcard, b8 ignoreCase) const noexcept;

    //==============================================================================
    // Substring location methods..

    /** Searches for a character inside this string.
        Uses a case-sensitive comparison.
        @returns    the index of the first occurrence of the character in this
                    string, or -1 if it's not found.
    */
    i32 indexOfChar (t32 characterToLookFor) const noexcept;

    /** Searches for a character inside this string.
        Uses a case-sensitive comparison.
        @param startIndex           the index from which the search should proceed
        @param characterToLookFor   the character to look for
        @returns            the index of the first occurrence of the character in this
                            string, or -1 if it's not found.
    */
    i32 indexOfChar (i32 startIndex, t32 characterToLookFor) const noexcept;

    /** Returns the index of the first character that matches one of the characters
        passed-in to this method.

        This scans the string, beginning from the startIndex supplied, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see indexOfChar, lastIndexOfAnyOf
    */
    i32 indexOfAnyOf (StringRef charactersToLookFor,
                      i32 startIndex = 0,
                      b8 ignoreCase = false) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-sensitive comparison.
        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
                    If textToLookFor is an empty string, this will always return 0.
    */
    i32 indexOf (StringRef textToLookFor) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-sensitive comparison.
        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
                                If textToLookFor is an empty string, this will always return -1.
    */
    i32 indexOf (i32 startIndex, StringRef textToLookFor) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-insensitive comparison.
        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
                    If textToLookFor is an empty string, this will always return 0.
    */
    i32 indexOfIgnoreCase (StringRef textToLookFor) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-insensitive comparison.
        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
                                If textToLookFor is an empty string, this will always return -1.
    */
    i32 indexOfIgnoreCase (i32 startIndex, StringRef textToLookFor) const noexcept;

    /** Searches for a character inside this string (working backwards from the end of the string).
        Uses a case-sensitive comparison.
        @returns    the index of the last occurrence of the character in this string, or -1 if it's not found.
    */
    i32 lastIndexOfChar (t32 character) const noexcept;

    /** Searches for a substring inside this string (working backwards from the end of the string).
        Uses a case-sensitive comparison.
        @returns    the index of the start of the last occurrence of the substring within this string,
                    or -1 if it's not found. If textToLookFor is an empty string, this will always return -1.
    */
    i32 lastIndexOf (StringRef textToLookFor) const noexcept;

    /** Searches for a substring inside this string (working backwards from the end of the string).
        Uses a case-insensitive comparison.
        @returns    the index of the start of the last occurrence of the substring within this string, or -1
                    if it's not found. If textToLookFor is an empty string, this will always return -1.
    */
    i32 lastIndexOfIgnoreCase (StringRef textToLookFor) const noexcept;

    /** Returns the index of the last character in this string that matches one of the
        characters passed-in to this method.

        This scans the string backwards, starting from its end, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see lastIndexOf, indexOfAnyOf
    */
    i32 lastIndexOfAnyOf (StringRef charactersToLookFor,
                          b8 ignoreCase = false) const noexcept;


    //==============================================================================
    // Substring extraction and manipulation methods..

    /** Returns the character at this index in the string.
        In a release build, no checks are made to see if the index is within a valid range, so be
        careful! In a debug build, the index is checked and an assertion fires if it's out-of-range.

        Also beware that depending on the encoding format that the string is using internally, this
        method may execute in either O(1) or O(n) time, so be careful when using it in your algorithms.
        If you're scanning through a string to inspect its characters, you should never use this operator
        for random access, it's far more efficient to call getCharPointer() to return a pointer, and
        then to use that to iterate the string.
        @see getCharPointer
    */
    t32 operator[] (i32 index) const noexcept;

    /** Returns the final character of the string.
        If the string is empty this will return 0.
    */
    t32 getLastCharacter() const noexcept;

    //==============================================================================
    /** Returns a subsection of the string.

        If the range specified is beyond the limits of the string, as much as
        possible is returned.

        @param startIndex   the index of the start of the substring needed
        @param endIndex     all characters from startIndex up to (but not including)
                            this index are returned
        @see fromFirstOccurrenceOf, dropLastCharacters, getLastCharacters, upToFirstOccurrenceOf
    */
    Txt substring (i32 startIndex, i32 endIndex) const;

    /** Returns a section of the string, starting from a given position.

        @param startIndex   the first character to include. If this is beyond the end
                            of the string, an empty string is returned. If it is zero or
                            less, the whole string is returned.
        @returns            the substring from startIndex up to the end of the string
        @see dropLastCharacters, getLastCharacters, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    Txt substring (i32 startIndex) const;

    /** Returns a version of this string with a number of characters removed
        from the end.

        @param numberToDrop     the number of characters to drop from the end of the
                                string. If this is greater than the length of the string,
                                an empty string will be returned. If zero or less, the
                                original string will be returned.
        @see substring, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf, getLastCharacter
    */
    Txt dropLastCharacters (i32 numberToDrop) const;

    /** Returns a number of characters from the end of the string.

        This returns the last numCharacters characters from the end of the string. If the
        string is shorter than numCharacters, the whole string is returned.

        @see substring, dropLastCharacters, getLastCharacter
    */
    Txt getLastCharacters (i32 numCharacters) const;

    //==============================================================================
    /** Returns a section of the string starting from a given substring.

        This will search for the first occurrence of the given substring, and
        return the section of the string starting from the point where this is
        found (optionally not including the substring itself).

        e.g. for the string "123456", fromFirstOccurrenceOf ("34", true) would return "3456", and
                                      fromFirstOccurrenceOf ("34", false) would return "56".

        If the substring isn't found, the method will return an empty string.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    Txt fromFirstOccurrenceOf (StringRef substringToStartFrom,
                                  b8 includeSubStringInResult,
                                  b8 ignoreCase) const;

    /** Returns a section of the string starting from the last occurrence of a given substring.

        Similar to fromFirstOccurrenceOf(), but using the last occurrence of the substring, and
        unlike fromFirstOccurrenceOf(), if the substring isn't found, this method will
        return the whole of the original string.

        @see fromFirstOccurrenceOf, upToLastOccurrenceOf
    */
    Txt fromLastOccurrenceOf (StringRef substringToFind,
                                 b8 includeSubStringInResult,
                                 b8 ignoreCase) const;

    /** Returns the start of this string, up to the first occurrence of a substring.

        This will search for the first occurrence of a given substring, and then
        return a copy of the string, up to the position of this substring,
        optionally including or excluding the substring itself in the result.

        e.g. for the string "123456", upTo ("34", false) would return "12", and
                                      upTo ("34", true) would return "1234".

        If the substring isn't found, this will return the whole of the original string.

        @see upToLastOccurrenceOf, fromFirstOccurrenceOf
    */
    Txt upToFirstOccurrenceOf (StringRef substringToEndWith,
                                  b8 includeSubStringInResult,
                                  b8 ignoreCase) const;

    /** Returns the start of this string, up to the last occurrence of a substring.

        Similar to upToFirstOccurrenceOf(), but this finds the last occurrence rather than the first.
        If the substring isn't found, this will return the whole of the original string.

        @see upToFirstOccurrenceOf, fromFirstOccurrenceOf
    */
    Txt upToLastOccurrenceOf (StringRef substringToFind,
                                 b8 includeSubStringInResult,
                                 b8 ignoreCase) const;

    //==============================================================================
    /** Returns a copy of this string with any whitespace characters removed from the start and end. */
    Txt trim() const;

    /** Returns a copy of this string with any whitespace characters removed from the start. */
    Txt trimStart() const;

    /** Returns a copy of this string with any whitespace characters removed from the end. */
    Txt trimEnd() const;

    /** Returns a copy of this string, having removed a specified set of characters from its start.
        Characters are removed from the start of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove.
        @see trim, trimStart, trimCharactersAtEnd
    */
    Txt trimCharactersAtStart (StringRef charactersToTrim) const;

    /** Returns a copy of this string, having removed a specified set of characters from its end.
        Characters are removed from the end of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove.
        @see trim, trimEnd, trimCharactersAtStart
    */
    Txt trimCharactersAtEnd (StringRef charactersToTrim) const;

    //==============================================================================
    /** Returns an upper-case version of this string. */
    Txt toUpperCase() const;

    /** Returns an lower-case version of this string. */
    Txt toLowerCase() const;

    //==============================================================================
    /** Replaces a sub-section of the string with another string.

        This will return a copy of this string, with a set of characters
        from startIndex to startIndex + numCharsToReplace removed, and with
        a new string inserted in their place.

        Note that this is a const method, and won't alter the string itself.

        @param startIndex               the first character to remove. If this is beyond the bounds of the string,
                                        it will be constrained to a valid range.
        @param numCharactersToReplace   the number of characters to remove. If zero or less, no
                                        characters will be taken out.
        @param stringToInsert           the new string to insert at startIndex after the characters have been
                                        removed.
    */
    Txt replaceSection (i32 startIndex,
                           i32 numCharactersToReplace,
                           StringRef stringToInsert) const;

    /** Replaces all occurrences of a substring with another string.

        Returns a copy of this string, with any occurrences of stringToReplace
        swapped for stringToInsertInstead.

        Note that this is a const method, and won't alter the string itself.
    */
    Txt replace (StringRef stringToReplace,
                    StringRef stringToInsertInstead,
                    b8 ignoreCase = false) const;

    /** Replaces the first occurrence of a substring with another string.

        Returns a copy of this string, with the first occurrence of stringToReplace
        swapped for stringToInsertInstead.

        Note that this is a const method, and won't alter the string itself.
    */
    Txt replaceFirstOccurrenceOf (StringRef stringToReplace,
                                     StringRef stringToInsertInstead,
                                     b8 ignoreCase = false) const;

    /** Returns a string with all occurrences of a character replaced with a different one. */
    Txt replaceCharacter (t32 characterToReplace,
                             t32 characterToInsertInstead) const;

    /** Replaces a set of characters with another set.

        Returns a string in which each character from charactersToReplace has been replaced
        by the character at the equivalent position in newCharacters (so the two strings
        passed in must be the same length).

        e.g. replaceCharacters ("abc", "def") replaces 'a' with 'd', 'b' with 'e', etc.

        Note that this is a const method, and won't affect the string itself.
    */
    Txt replaceCharacters (StringRef charactersToReplace,
                              StringRef charactersToInsertInstead) const;

    /** Returns a version of this string that only retains a fixed set of characters.

        This will return a copy of this string, omitting any characters which are not
        found in the string passed-in.

        e.g. for "1122334455", retainCharacters ("432") would return "223344"

        Note that this is a const method, and won't alter the string itself.
    */
    Txt retainCharacters (StringRef charactersToRetain) const;

    /** Returns a version of this string with a set of characters removed.

        This will return a copy of this string, omitting any characters which are
        found in the string passed-in.

        e.g. for "1122334455", removeCharacters ("432") would return "1155"

        Note that this is a const method, and won't alter the string itself.
    */
    Txt removeCharacters (StringRef charactersToRemove) const;

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that doesn't appear in the string passed in.
    */
    Txt initialSectionContainingOnly (StringRef permittedCharacters) const;

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that occurs in the string passed in. (If none of the specified
        characters are found in the string, the return value will just be the original string).
    */
    Txt initialSectionNotContaining (StringRef charactersToStopAt) const;

    //==============================================================================
    /** Checks whether the string might be in quotation marks.

        @returns    true if the string begins with a quote character (either a f64 or single quote).
                    It is also true if there is whitespace before the quote, but it doesn't check the end of the string.
        @see unquoted, quoted
    */
    b8 isQuotedString() const;

    /** Removes quotation marks from around the string, (if there are any).

        Returns a copy of this string with any quotes removed from its ends. Quotes that aren't
        at the ends of the string are not affected. If there aren't any quotes, the original string
        is returned.

        Note that this is a const method, and won't alter the string itself.

        @see isQuotedString, quoted
    */
    Txt unquoted() const;

    /** Adds quotation marks around a string.

        This will return a copy of the string with a quote at the start and end, (but won't
        add the quote if there's already one there, so it's safe to call this on strings that
        may already have quotes around them).

        Note that this is a const method, and won't alter the string itself.

        @param quoteCharacter   the character to add at the start and end
        @see isQuotedString, unquoted
    */
    Txt quoted (t32 quoteCharacter = '"') const;


    //==============================================================================
    /** Creates a string which is a version of a string repeated and joined together.

        @param stringToRepeat         the string to repeat
        @param numberOfTimesToRepeat  how many times to repeat it
    */
    static Txt repeatedString (StringRef stringToRepeat,
                                  i32 numberOfTimesToRepeat);

    /** Returns a copy of this string with the specified character repeatedly added to its
        beginning until the total length is at least the minimum length specified.
    */
    Txt paddedLeft (t32 padCharacter, i32 minimumLength) const;

    /** Returns a copy of this string with the specified character repeatedly added to its
        end until the total length is at least the minimum length specified.
    */
    Txt paddedRight (t32 padCharacter, i32 minimumLength) const;

    /** Creates a string from data in an unknown format.

        This looks at some binary data and tries to guess whether it's Unicode
        or 8-bit characters, then returns a string that represents it correctly.

        Should be able to handle Unicode endianness correctly, by looking at
        the first two bytes.
    */
    static Txt createStringFromData (ukk data, i32 size);

    /** Creates a Txt from a printf-style parameter list.

        I don't like this method. I don't use it myself, and I recommend avoiding it and
        using the operator<< methods or pretty much anything else instead. It's only provided
        here because of the popular unrest that was stirred-up when I tried to remove it...

        If you're really determined to use it, at least make sure that you never, ever,
        pass any Txt objects to it as parameters. And bear in mind that internally, depending
        on the platform, it may be using wchar_t or t8 character types, so that even string
        literals can't be safely used as parameters if you're writing portable code.
    */
    template <typename... Args>
    static Txt formatted (const Txt& formatStr, Args... args)      { return formattedRaw (formatStr.toRawUTF8(), args...); }

    /** Returns an iterator pointing at the beginning of the string. */
    CharPointerType begin() const                                        { return getCharPointer(); }

    /** Returns an iterator pointing at the terminating null of the string.

        Note that this has to find the terminating null before returning it, so prefer to
        call this once before looping and then reuse the result, rather than calling 'end()'
        each time through the loop.

        @code
        Txt str = ...;

        // BEST
        for (auto c : str)
            DBG (c);

        // GOOD
        for (auto ptr = str.begin(), end = str.end(); ptr != end; ++ptr)
            DBG (*ptr);

        std::for_each (str.begin(), str.end(), [] (t32 c) { DBG (c); });

        // BAD
        for (auto ptr = str.begin(); ptr != str.end(); ++ptr)
            DBG (*ptr);
        @endcode
    */
    CharPointerType end() const                                          { return begin().findTerminatingNull(); }

    //==============================================================================
    // Numeric conversions..

    /** Creates a string containing this signed 32-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (i32 decimalInteger);

    /** Creates a string containing this u32 32-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (u32 decimalInteger);

    /** Creates a string containing this signed 16-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (short decimalInteger);

    /** Creates a string containing this u32 16-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (u16 decimalInteger);

    /** Creates a string containing this signed 64-bit integer as a decimal number.
        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (z64 largeIntegerValue);

    /** Creates a string containing this u32 64-bit integer as a decimal number.
        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (zu64 largeIntegerValue);

    /** Creates a string containing this i64 integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (i64 decimalInteger);

    /** Creates a string containing this u64 integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit Txt (u64 decimalInteger);

    /** Creates a string representing this floating-point number.
        @param floatValue               the value to convert to a string
        @see getDoubleValue, getIntValue
    */
    explicit Txt (f32 floatValue);

    /** Creates a string representing this floating-point number.
        @param doubleValue              the value to convert to a string
        @see getFloatValue, getIntValue
    */
    explicit Txt (f64 doubleValue);

    /** Creates a string representing this floating-point number.
        @param floatValue               the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0 the number will be formatted using that many
                                        decimal places, adding trailing zeros as required. If 0 or
                                        less the number will be formatted using the C++ standard
                                        library default format, which uses scientific notation for
                                        large and small numbers.
        @param useScientificNotation    if the number should be formatted using scientific notation
        @see getDoubleValue, getIntValue
    */
    Txt (f32 floatValue, i32 numberOfDecimalPlaces, b8 useScientificNotation = false);

    /** Creates a string representing this floating-point number.
        @param doubleValue              the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, adding trailing zeros as required, and
                                        will not use exponent notation. If 0 or less, it will use
                                        exponent notation if necessary.
        @param useScientificNotation    if the number should be formatted using scientific notation
        @see getFloatValue, getIntValue
    */
    Txt (f64 doubleValue, i32 numberOfDecimalPlaces, b8 useScientificNotation = false);

   #ifndef DOXYGEN
    // Automatically creating a Txt from a b8 opens up lots of nasty type conversion edge cases.
    // If you want a Txt representation of a b8 you can cast the b8 to an i32 first.
    explicit Txt (b8) = delete;
   #endif

    /** Reads the value of the string as a decimal number (up to 32 bits in size).

        @returns the value of the string as a 32 bit signed base-10 integer.
        @see getTrailingIntValue, getHexValue32, getHexValue64
    */
    i32 getIntValue() const noexcept;

    /** Reads the value of the string as a decimal number (up to 64 bits in size).
        @returns the value of the string as a 64 bit signed base-10 integer.
    */
    z64 getLargeIntValue() const noexcept;

    /** Parses a decimal number from the end of the string.

        This will look for a value at the end of the string.
        e.g. for "321 xyz654" it will return 654; for "2 3 4" it'll return 4.

        If the string ends with a hyphen followed by numeric characters, the
        return value will be negative.

        @see getIntValue
    */
    i32 getTrailingIntValue() const noexcept;

    /** Parses this string as a floating point number.

        @returns    the value of the string as a 32-bit floating point value.
        @see getDoubleValue
    */
    f32 getFloatValue() const noexcept;

    /** Parses this string as a floating point number.

        @returns    the value of the string as a 64-bit floating point value.
        @see getFloatValue
    */
    f64 getDoubleValue() const noexcept;

    /** Parses the string as a hexadecimal number.

        Non-hexadecimal characters in the string are ignored.

        If the string contains too many characters, then the lowest significant
        digits are returned, e.g. "ffff12345678" would produce 0x12345678.

        @returns    a 32-bit number which is the value of the string in hex.
    */
    i32 getHexValue32() const noexcept;

    /** Parses the string as a hexadecimal number.

        Non-hexadecimal characters in the string are ignored.

        If the string contains too many characters, then the lowest significant
        digits are returned, e.g. "ffff1234567812345678" would produce 0x1234567812345678.

        @returns    a 64-bit number which is the value of the string in hex.
    */
    z64 getHexValue64() const noexcept;

    /** Returns a string representing this numeric value in hexadecimal. */
    template <typename IntegerType>
    static Txt toHexString (IntegerType number)      { return createHex (number); }

    /** Returns a string containing a hex dump of a block of binary data.

        @param data         the binary data to use as input
        @param size         how many bytes of data to use
        @param groupSize    how many bytes are grouped together before inserting a
                            space into the output. e.g. group size 0 has no spaces,
                            group size 1 looks like: "be a1 c2 ff", group size 2 looks
                            like "bea1 c2ff".
    */
    static Txt toHexString (ukk data, i32 size, i32 groupSize = 1);

    /** Returns a string containing a decimal with a set number of significant figures.

        @param number                         the input number
        @param numberOfSignificantFigures     the number of significant figures to use
    */
    template <typename DecimalType>
    static Txt toDecimalStringWithSignificantFigures (DecimalType number, i32 numberOfSignificantFigures)
    {
        jassert (numberOfSignificantFigures > 0);

        if (exactlyEqual (number, DecimalType()))
        {
            if (numberOfSignificantFigures > 1)
            {
                Txt result ("0.0");

                for (i32 i = 2; i < numberOfSignificantFigures; ++i)
                    result += "0";

                return result;
            }

            return "0";
        }

        auto numDigitsBeforePoint = (i32) std::ceil (std::log10 (number < 0 ? -number : number));

        auto shift = numberOfSignificantFigures - numDigitsBeforePoint;
        auto factor = std::pow (10.0, shift);
        auto rounded = std::round (number * factor) / factor;

        std::stringstream ss;
        ss << std::fixed << std::setprecision (std::max (shift, 0)) << rounded;
        return ss.str();
    }

    //==============================================================================
    /** Returns the character pointer currently being used to store this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    CharPointerType getCharPointer() const noexcept             { return text; }

    /** Returns a pointer to a UTF-8 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        To find out how many bytes you need to store this string as UTF-8, you can call
        CharPointer_UTF8::getBytesRequiredFor (myString.getCharPointer())

        @see toRawUTF8, getCharPointer, toUTF16, toUTF32
    */
    CharPointer_UTF8 toUTF8() const;

    /** Returns a pointer to a UTF-8 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        To find out how many bytes you need to store this string as UTF-8, you can call
        CharPointer_UTF8::getBytesRequiredFor (myString.getCharPointer())

        @see getCharPointer, toUTF8, toUTF16, toUTF32
    */
    tukk toRawUTF8() const;

    /** Returns a pointer to a UTF-16 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        To find out how many bytes you need to store this string as UTF-16, you can call
        CharPointer_UTF16::getBytesRequiredFor (myString.getCharPointer())

        @see getCharPointer, toUTF8, toUTF32
    */
    CharPointer_UTF16 toUTF16() const;

    /** Returns a pointer to a UTF-32 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        @see getCharPointer, toUTF8, toUTF16
    */
    CharPointer_UTF32 toUTF32() const;

    /** Returns a pointer to a wchar_t version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        Bear in mind that the wchar_t type is different on different platforms, so on
        Windows, this will be equivalent to calling toUTF16(), on unix it'll be the same
        as calling toUTF32(), etc.

        @see getCharPointer, toUTF8, toUTF16, toUTF32
    */
    const wchar_t* toWideCharPointer() const;

    /** */
    std::string toStdString() const;

    //==============================================================================
    /** Creates a Txt from a UTF-8 encoded buffer.
        If the size is < 0, it'll keep reading until it hits a zero.
    */
    static Txt fromUTF8 (tukk utf8buffer, i32 bufferSizeBytes = -1);

   #if __cpp_char8_t || DOXYGEN

    /** Creates a Txt from a UTF-8 encoded buffer.
        If the size is < 0, it'll keep reading until it hits a zero.
    */
    static Txt fromUTF8 (const char8_t* utf8buffer, i32 bufferSizeBytes = -1);

   #endif

    /** Returns the number of bytes required to represent this string as UTF8.
        The number returned does NOT include the trailing zero.
        @see toUTF8, copyToUTF8
    */
    size_t getNumBytesAsUTF8() const noexcept;

    //==============================================================================
    /** Copies the string to a buffer as UTF-8 characters.

        Returns the number of bytes copied to the buffer, including the terminating null
        character.

        To find out how many bytes you need to store this string as UTF-8, you can call
        CharPointer_UTF8::getBytesRequiredFor (myString.getCharPointer())

        @param destBuffer       the place to copy it to; if this is a null pointer, the method just
                                returns the number of bytes required (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the string won't fit, it'll
                                put in as many as it can while still allowing for a terminating null t8 at the
                                end, and will return the number of bytes that were actually used.
        @see CharPointer_UTF8::writeWithDestByteLimit
    */
    size_t copyToUTF8 (CharPointer_UTF8::CharType* destBuffer, size_t maxBufferSizeBytes) const noexcept;

    /** Copies the string to a buffer as UTF-16 characters.

        Returns the number of bytes copied to the buffer, including the terminating null
        character.

        To find out how many bytes you need to store this string as UTF-16, you can call
        CharPointer_UTF16::getBytesRequiredFor (myString.getCharPointer())

        @param destBuffer       the place to copy it to; if this is a null pointer, the method just
                                returns the number of bytes required (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the string won't fit, it'll
                                put in as many as it can while still allowing for a terminating null t8 at the
                                end, and will return the number of bytes that were actually used.
        @see CharPointer_UTF16::writeWithDestByteLimit
    */
    size_t copyToUTF16 (CharPointer_UTF16::CharType* destBuffer, size_t maxBufferSizeBytes) const noexcept;

    /** Copies the string to a buffer as UTF-32 characters.

        Returns the number of bytes copied to the buffer, including the terminating null
        character.

        To find out how many bytes you need to store this string as UTF-32, you can call
        CharPointer_UTF32::getBytesRequiredFor (myString.getCharPointer())

        @param destBuffer       the place to copy it to; if this is a null pointer, the method just
                                returns the number of bytes required (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the string won't fit, it'll
                                put in as many as it can while still allowing for a terminating null t8 at the
                                end, and will return the number of bytes that were actually used.
        @see CharPointer_UTF32::writeWithDestByteLimit
    */
    size_t copyToUTF32 (CharPointer_UTF32::CharType* destBuffer, size_t maxBufferSizeBytes) const noexcept;

    //==============================================================================
    /** Increases the string's internally allocated storage.

        Although the string's contents won't be affected by this call, it will
        increase the amount of memory allocated internally for the string to grow into.

        If you're about to make a large number of calls to methods such
        as += or <<, it's more efficient to preallocate enough extra space
        beforehand, so that these methods won't have to keep resizing the string
        to append the extra characters.

        @param numBytesNeeded   the number of bytes to allocate storage for. If this
                                value is less than the currently allocated size, it will
                                have no effect.
    */
    z0 preallocateBytes (size_t numBytesNeeded);

    /** Swaps the contents of this string with another one.
        This is a very fast operation, as no allocation or copying needs to be done.
    */
    z0 swapWith (Txt& other) noexcept;

    //==============================================================================
   #if DRX_MAC || DRX_IOS || DOXYGEN
    /** OSX ONLY - Creates a Txt from an OSX CFString. */
    static Txt fromCFString (CFStringRef cfString);

    /** OSX ONLY - Converts this string to a CFString.
        Remember that you must use CFRelease() to free the returned string when you're
        finished with it.
    */
    CFStringRef toCFString() const;

    /** OSX ONLY - Returns a copy of this string in which any decomposed unicode characters have
        been converted to their precomposed equivalents. */
    Txt convertToPrecomposedUnicode() const;
   #endif

    /** Returns the number of Txt objects which are currently sharing the same internal
        data as this one.
    */
    i32 getReferenceCount() const noexcept;

    //==============================================================================
   #if DRX_ALLOW_STATIC_NULL_VARIABLES && ! defined (DOXYGEN)
    [[deprecated ("This was a static empty string object, but is now deprecated as it's too easy to accidentally "
                 "use it indirectly during a static constructor, leading to hard-to-find order-of-initialisation "
                 "problems. If you need an empty Txt object, just use Txt() or {}. For returning an empty "
                 "Txt from a function by reference, use a function-local static Txt object and return that.")]]
    static const Txt empty;
   #endif

private:
    //==============================================================================
    CharPointerType text;

    //==============================================================================
    struct PreallocationBytes
    {
        explicit PreallocationBytes (size_t) noexcept;
        size_t numBytes;
    };

    explicit Txt (const PreallocationBytes&); // This constructor preallocates a certain amount of memory
    size_t getByteOffsetOfEnd() const noexcept;

    // This private cast operator should prevent strings being accidentally cast
    // to bools (this is possible because the compiler can add an implicit cast
    // via a tukk)
    operator b8() const noexcept  { return false; }

    //==============================================================================
    static Txt formattedRaw (tukk, ...);

    static Txt createHex (u8);
    static Txt createHex (u16);
    static Txt createHex (u32);
    static Txt createHex (zu64);

    template <typename Type>
    static Txt createHex (Type n)  { return createHex (static_cast<typename TypeHelpers::UnsignedTypeWithSize<(i32) sizeof (n)>::type> (n)); }
};

//==============================================================================
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (tukk string1,     const Txt& string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (const wchar_t* string1,  const Txt& string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (t8 string1,            const Txt& string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (wchar_t string1,         const Txt& string2);
#if ! DRX_NATIVE_WCHAR_IS_UTF32
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (t32 string1,      const Txt& string2);
#endif

/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, const Txt& string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, tukk string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, const wchar_t* string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, const std::string& string2);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, t8 characterToAppend);
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, wchar_t characterToAppend);
#if ! DRX_NATIVE_WCHAR_IS_UTF32
/** Concatenates two strings. */
DRX_API Txt DRX_CALLTYPE operator+ (Txt string1, t32 characterToAppend);
#endif

//==============================================================================
/** Appends a character at the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, t8 characterToAppend);
/** Appends a character at the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, wchar_t characterToAppend);
#if ! DRX_NATIVE_WCHAR_IS_UTF32
/** Appends a character at the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, t32 characterToAppend);
#endif

/** Appends a string to the end of the first one. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, tukk string2);
/** Appends a string to the end of the first one. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, const wchar_t* string2);
/** Appends a string to the end of the first one. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, const Txt& string2);
/** Appends a string to the end of the first one. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, StringRef string2);
/** Appends a string to the end of the first one. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, const std::string& string2);

/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, u8 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, short number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, i32 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, i64 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, u64 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, z64 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, zu64 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, f32 number);
/** Appends a decimal number to the end of a string. */
DRX_API Txt& DRX_CALLTYPE operator<< (Txt& string1, f64 number);

#ifndef DOXYGEN
// Automatically creating a Txt from a b8 opens up lots of nasty type conversion edge cases.
// If you want a Txt representation of a b8 you can cast the b8 to an i32 first.
Txt& DRX_CALLTYPE operator<< (Txt&, b8) = delete;
#endif

//==============================================================================
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, const Txt& string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, tukk string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, const wchar_t* string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, CharPointer_UTF8 string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, CharPointer_UTF16 string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator== (const Txt& string1, CharPointer_UTF32 string2) noexcept;

/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, const Txt& string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, tukk string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, const wchar_t* string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, CharPointer_UTF8 string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, CharPointer_UTF16 string2) noexcept;
/** Case-sensitive comparison of two strings. */
DRX_API b8 DRX_CALLTYPE operator!= (const Txt& string1, CharPointer_UTF32 string2) noexcept;

//==============================================================================
/** This operator allows you to write a drx Txt directly to std output streams.
    This is handy for writing strings to std::cout, std::cerr, etc.
*/
template <class traits>
std::basic_ostream <t8, traits>& DRX_CALLTYPE operator<< (std::basic_ostream <t8, traits>& stream, const Txt& stringToWrite)
{
    return stream << stringToWrite.toRawUTF8();
}

/** This operator allows you to write a drx Txt directly to std output streams.
    This is handy for writing strings to std::wcout, std::wcerr, etc.
*/
template <class traits>
std::basic_ostream <wchar_t, traits>& DRX_CALLTYPE operator<< (std::basic_ostream <wchar_t, traits>& stream, const Txt& stringToWrite)
{
    return stream << stringToWrite.toWideCharPointer();
}

/** Writes a string to an OutputStream as UTF8. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const Txt& stringToWrite);

/** Writes a string to an OutputStream as UTF8. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, StringRef stringToWrite);

} // namespace drx

#ifndef DOXYGEN
namespace std
{
    template <> struct hash<drx::Txt>
    {
        size_t operator() (const drx::Txt& s) const noexcept    { return s.hash(); }
    };
}
#endif
