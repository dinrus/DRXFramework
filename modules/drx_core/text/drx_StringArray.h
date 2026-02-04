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
    A special array for holding a list of strings.

    @see Txt, StringPairArray

    @tags{Core}
*/
class DRX_API  StringArray
{
public:
    //==============================================================================
    /** Creates an empty string array */
    StringArray() noexcept;

    /** Creates a copy of another string array */
    StringArray (const StringArray&);

    /** Move constructor */
    StringArray (StringArray&&) noexcept;

    /** Creates an array containing a single string. */
    StringArray (const Txt& firstValue);

    /** Creates an array containing a list of strings. */
    template <typename... OtherElements>
    StringArray (StringRef firstValue, OtherElements&&... otherValues)
        : strings (firstValue, std::forward<OtherElements> (otherValues)...) {}

    /** Creates an array containing a list of strings. */
    StringArray (const std::initializer_list<tukk>& strings);

    /** Creates a StringArray by moving from an Array<Txt> */
    StringArray (Array<Txt>&&) noexcept;

    /** Creates a StringArray from an array of objects which can be implicitly converted to Strings. */
    template <typename Type>
    StringArray (const Array<Type>& stringArray)
    {
        addArray (stringArray.begin(), stringArray.end());
    }

    /** Creates an array from a raw array of strings.
        @param strings          an array of strings to add
        @param numberOfStrings  how many items there are in the array
    */
    StringArray (const Txt* strings, i32 numberOfStrings);

    /** Creates a copy of an array of string literals.
        @param strings          an array of strings to add. Null pointers in the array will be
                                treated as empty strings
        @param numberOfStrings  how many items there are in the array
    */
    StringArray (tukk const* strings, i32 numberOfStrings);

    /** Creates a copy of a null-terminated array of string literals.

        Each item from the array passed-in is added, until it encounters a null pointer,
        at which point it stops.
    */
    explicit StringArray (tukk const* strings);

    /** Creates a copy of a null-terminated array of string literals.
        Each item from the array passed-in is added, until it encounters a null pointer,
        at which point it stops.
    */
    explicit StringArray (const wchar_t* const* strings);

    /** Creates a copy of an array of string literals.
        @param strings          an array of strings to add. Null pointers in the array will be
                                treated as empty strings
        @param numberOfStrings  how many items there are in the array
    */
    StringArray (const wchar_t* const* strings, i32 numberOfStrings);

    /** Destructor. */
    ~StringArray() = default;

    /** Copies the contents of another string array into this one */
    StringArray& operator= (const StringArray&);

    /** Move assignment operator */
    StringArray& operator= (StringArray&&) noexcept;

    /** Copies a StringArray from an array of objects which can be implicitly converted to Strings. */
    template <typename Type>
    StringArray& operator= (const Array<Type>& stringArray)
    {
        addArray (stringArray.begin(), stringArray.end());
        return *this;
    }

    /** Swaps the contents of this and another StringArray. */
    z0 swapWith (StringArray&) noexcept;

    //==============================================================================
    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    true only if the other array contains exactly the same strings in the same order
    */
    b8 operator== (const StringArray&) const noexcept;

    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    false if the other array contains exactly the same strings in the same order
    */
    b8 operator!= (const StringArray&) const noexcept;

    //==============================================================================
    /** Returns the number of strings in the array */
    inline i32 size() const noexcept                                    { return strings.size(); }

    /** Возвращает true, если the array is empty, false otherwise. */
    inline b8 isEmpty() const noexcept                                { return size() == 0; }

    /** Returns one of the strings from the array.

        If the index is out-of-range, an empty string is returned.

        Obviously the reference returned shouldn't be stored for later use, as the
        string it refers to may disappear when the array changes.
    */
    const Txt& operator[] (i32 index) const noexcept;

    /** Returns a reference to one of the strings in the array.
        This lets you modify a string in-place in the array, but you must be sure that
        the index is in-range.
    */
    Txt& getReference (i32 index) noexcept;

    /** Returns a reference to one of the strings in the array.
        This lets you modify a string in-place in the array, but you must be sure that
        the index is in-range.
    */
    const Txt& getReference (i32 index) const noexcept;

    /** Returns a pointer to the first Txt in the array.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline Txt* begin() noexcept                 { return strings.begin(); }

    /** Returns a pointer to the first Txt in the array.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline const Txt* begin() const noexcept     { return strings.begin(); }

    /** Returns a pointer to the Txt which follows the last element in the array.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline Txt* end() noexcept                  { return strings.end(); }

    /** Returns a pointer to the Txt which follows the last element in the array.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline const Txt* end() const noexcept       { return strings.end(); }


    /** Searches for a string in the array.

        The comparison will be case-insensitive if the ignoreCase parameter is true.

        @returns    true if the string is found inside the array
    */
    b8 contains (StringRef stringToLookFor,
                   b8 ignoreCase = false) const;

    /** Searches for a string in the array.

        The comparison will be case-insensitive if the ignoreCase parameter is true.

        @param stringToLookFor  the string to try to find
        @param ignoreCase       whether the comparison should be case-insensitive
        @param startIndex       the first index to start searching from
        @returns                the index of the first occurrence of the string in this array,
                                or -1 if it isn't found.
    */
    i32 indexOf (StringRef stringToLookFor,
                 b8 ignoreCase = false,
                 i32 startIndex = 0) const;

    //==============================================================================
    /** Appends a string at the end of the array. */
    z0 add (Txt stringToAdd);

    /** Inserts a string into the array.

        This will insert a string into the array at the given index, moving
        up the other elements to make room for it.
        If the index is less than zero or greater than the size of the array,
        the new string will be added to the end of the array.
    */
    z0 insert (i32 index, Txt stringToAdd);

    /** Adds a string to the array as i64 as it's not already in there.
        The search can optionally be case-insensitive.

        @return true if the string has been added, false otherwise.
    */
    b8 addIfNotAlreadyThere (const Txt& stringToAdd, b8 ignoreCase = false);

    /** Replaces one of the strings in the array with another one.

        If the index is higher than the array's size, the new string will be
        added to the end of the array; if it's less than zero nothing happens.
    */
    z0 set (i32 index, Txt newString);

    /** Appends some strings from another array to the end of this one.

        @param other                the array to add
        @param startIndex           the first element of the other array to add
        @param numElementsToAdd     the maximum number of elements to add (if this is
                                    less than zero, they are all added)
    */
    z0 addArray (const StringArray& other,
                   i32 startIndex = 0,
                   i32 numElementsToAdd = -1);

    /** Adds items from a range of start/end iterators of some kind of objects which
        can be implicitly converted to Strings.
    */
    template <typename Iterator>
    z0 addArray (Iterator&& start, Iterator&& end)
    {
        ensureStorageAllocated (size() + (i32) static_cast<size_t> (end - start));

        while (start != end)
            strings.add (*start++);
    }

    /** Merges the strings from another array into this one.
        This will not add a string that already exists.

        @param other                the array to add
        @param ignoreCase           ignore case when merging
    */
    z0 mergeArray (const StringArray& other,
                     b8 ignoreCase = false);

    /** Breaks up a string into tokens and adds them to this array.

        This will tokenise the given string using whitespace characters as the
        token delimiters, and will add these tokens to the end of the array.
        @returns    the number of tokens added
        @see fromTokens
    */
    i32 addTokens (StringRef stringToTokenise, b8 preserveQuotedStrings);

    /** Breaks up a string into tokens and adds them to this array.

        This will tokenise the given string (using the string passed in to define the
        token delimiters), and will add these tokens to the end of the array.

        @param stringToTokenise     the string to tokenise
        @param breakCharacters      a string of characters, any of which will be considered
                                    to be a token delimiter.
        @param quoteCharacters      if this string isn't empty, it defines a set of characters
                                    which are treated as quotes. Any text occurring
                                    between quotes is not broken up into tokens.
        @returns    the number of tokens added
        @see fromTokens
    */
    i32 addTokens (StringRef stringToTokenise,
                   StringRef breakCharacters,
                   StringRef quoteCharacters);

    /** Breaks up a string into lines and adds them to this array.

        This breaks a string down into lines separated by \\n or \\r\\n, and adds each line
        to the array. Line-break characters are omitted from the strings that are added to
        the array.
    */
    i32 addLines (StringRef stringToBreakUp);

    /** Returns an array containing the tokens in a given string.

        This will tokenise the given string using whitespace characters as the
        token delimiters, and return the parsed tokens as an array.
        @see addTokens
    */
    static StringArray fromTokens (StringRef stringToTokenise,
                                   b8 preserveQuotedStrings);

    /** Returns an array containing the tokens in a given string.

        This will tokenise the given string using the breakCharacters string to define
        the token delimiters, and will return the parsed tokens as an array.

        @param stringToTokenise     the string to tokenise
        @param breakCharacters      a string of characters, any of which will be considered
                                    to be a token delimiter.
        @param quoteCharacters      if this string isn't empty, it defines a set of characters
                                    which are treated as quotes. Any text occurring
                                    between quotes is not broken up into tokens.
        @see addTokens
    */
    static StringArray fromTokens (StringRef stringToTokenise,
                                   StringRef breakCharacters,
                                   StringRef quoteCharacters);

    /** Returns an array containing the lines in a given string.

        This breaks a string down into lines separated by \\n or \\r\\n, and returns an
        array containing these lines. Line-break characters are omitted from the strings that
        are added to the array.
    */
    static StringArray fromLines (StringRef stringToBreakUp);

    //==============================================================================
    /** Removes all elements from the array. */
    z0 clear();

    /** Removes all elements from the array without freeing the array's allocated storage.
        @see clear
    */
    z0 clearQuick();

    /** Removes a string from the array.
        If the index is out-of-range, no action will be taken.
    */
    z0 remove (i32 index);

    /** Finds a string in the array and removes it.
        This will remove all occurrences of the given string from the array.
        The comparison may be case-insensitive depending on the ignoreCase parameter.
    */
    z0 removeString (StringRef stringToRemove,
                       b8 ignoreCase = false);

    /** Removes a range of elements from the array.

        This will remove a set of elements, starting from the given index,
        and move subsequent elements down to close the gap.

        If the range extends beyond the bounds of the array, it will
        be safely clipped to the size of the array.

        @param startIndex       the index of the first element to remove
        @param numberToRemove   how many elements should be removed
    */
    z0 removeRange (i32 startIndex, i32 numberToRemove);

    /** Removes any duplicated elements from the array.

        If any string appears in the array more than once, only the first occurrence of
        it will be retained.

        @param ignoreCase   whether to use a case-insensitive comparison
    */
    z0 removeDuplicates (b8 ignoreCase);

    /** Removes empty strings from the array.
        @param removeWhitespaceStrings  if true, strings that only contain whitespace
                                        characters will also be removed
    */
    z0 removeEmptyStrings (b8 removeWhitespaceStrings = true);

    /** Moves one of the strings to a different position.

        This will move the string to a specified index, shuffling along
        any intervening elements as required.

        So for example, if you have the array { 0, 1, 2, 3, 4, 5 } then calling
        move (2, 4) would result in { 0, 1, 3, 4, 2, 5 }.

        @param currentIndex     the index of the value to be moved. If this isn't a
                                valid index, then nothing will be done
        @param newIndex         the index at which you'd like this value to end up. If this
                                is less than zero, the value will be moved to the end
                                of the array
    */
    z0 move (i32 currentIndex, i32 newIndex) noexcept;

    /** Deletes any whitespace characters from the starts and ends of all the strings. */
    z0 trim();

    /** Adds numbers to the strings in the array, to make each string unique.

        This will add numbers to the ends of groups of similar strings.
        e.g. if there are two "moose" strings, they will become "moose (1)" and "moose (2)"

        @param ignoreCaseWhenComparing      whether the comparison used is case-insensitive
        @param appendNumberToFirstInstance  whether the first of a group of similar strings
                                            also has a number appended to it.
        @param preNumberString              when adding a number, this string is added before the number.
                                            If you pass nullptr, a default string will be used, which adds
                                            brackets around the number.
        @param postNumberString             this string is appended after any numbers that are added.
                                            If you pass nullptr, a default string will be used, which adds
                                            brackets around the number.
    */
    z0 appendNumbersToDuplicates (b8 ignoreCaseWhenComparing,
                                    b8 appendNumberToFirstInstance,
                                    CharPointer_UTF8 preNumberString = CharPointer_UTF8 (nullptr),
                                    CharPointer_UTF8 postNumberString = CharPointer_UTF8 (nullptr));

    //==============================================================================
    /** Joins the strings in the array together into one string.

        This will join a range of elements from the array into a string, separating
        them with a given string.

        e.g. joinIntoString (",") will turn an array of "a" "b" and "c" into "a,b,c".

        @param separatorString      the string to insert between all the strings
        @param startIndex           the first element to join
        @param numberOfElements     how many elements to join together. If this is less
                                    than zero, all available elements will be used.
    */
    Txt joinIntoString (StringRef separatorString,
                           i32 startIndex = 0,
                           i32 numberOfElements = -1) const;

    //==============================================================================
    /** Sorts the array into alphabetical order.
        @param ignoreCase       if true, the comparisons used will not be case-sensitive.
    */
    z0 sort (b8 ignoreCase);

    /** Sorts the array using extra language-aware rules to do a better job of comparing
        words containing spaces and numbers.
        @see Txt::compareNatural()
    */
    z0 sortNatural();

    //==============================================================================
    /** Increases the array's internal storage to hold a minimum number of elements.

        Calling this before adding a large known number of elements means that
        the array won't have to keep dynamically resizing itself as the elements
        are added, and it'll therefore be more efficient.
    */
    z0 ensureStorageAllocated (i32 minNumElements);

    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    z0 minimiseStorageOverheads();

    /** This is the array holding the actual strings. This is public to allow direct access
        to array methods that may not already be provided by the StringArray class.
    */
    Array<Txt> strings;

private:
    DRX_LEAK_DETECTOR (StringArray)
};

} // namespace drx
