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
    A container for holding a set of strings which are keyed by another string.

    @see StringArray

    @tags{Core}
*/
class DRX_API  StringPairArray
{
public:
    //==============================================================================
    /** Creates an empty array */
    StringPairArray (b8 ignoreCaseWhenComparingKeys = true);

    /** Creates a copy of another array */
    StringPairArray (const StringPairArray& other);

    /** Destructor. */
    ~StringPairArray() = default;

    /** Copies the contents of another string array into this one */
    StringPairArray& operator= (const StringPairArray& other);

    //==============================================================================
    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    true only if the other array contains exactly the same strings with the same keys
    */
    b8 operator== (const StringPairArray& other) const;

    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    false if the other array contains exactly the same strings with the same keys
    */
    b8 operator!= (const StringPairArray& other) const;

    //==============================================================================
    /** Finds the value corresponding to a key string.

        If no such key is found, this will just return an empty string. To check whether
        a given key actually exists (because it might actually be paired with an empty string), use
        the getAllKeys() method to obtain a list.

        Obviously the reference returned shouldn't be stored for later use, as the
        string it refers to may disappear when the array changes.

        @see getValue
    */
    const Txt& operator[] (StringRef key) const;

    /** Finds the value corresponding to a key string.
        If no such key is found, this will just return the value provided as a default.
        @see operator[]
    */
    Txt getValue (StringRef, const Txt& defaultReturnValue) const;

    /** Возвращает true, если the given key exists. */
    b8 containsKey (StringRef key) const noexcept;

    /** Returns a list of all keys in the array. */
    const StringArray& getAllKeys() const noexcept          { return keys; }

    /** Returns a list of all values in the array. */
    const StringArray& getAllValues() const noexcept        { return values; }

    /** Returns the number of strings in the array */
    inline i32 size() const noexcept                        { return keys.size(); }


    //==============================================================================
    /** Adds or amends a key/value pair.
        If a value already exists with this key, its value will be overwritten,
        otherwise the key/value pair will be added to the array.
    */
    z0 set (const Txt& key, const Txt& value);

    /** Adds the items from another array to this one.
        This is equivalent to using set() to add each of the pairs from the other array.
    */
    z0 addArray (const StringPairArray& other);

    //==============================================================================
    /** Removes all elements from the array. */
    z0 clear();

    /** Removes a string from the array based on its key.
        If the key isn't found, nothing will happen.
    */
    z0 remove (StringRef key);

    /** Removes a string from the array based on its index.
        If the index is out-of-range, no action will be taken.
    */
    z0 remove (i32 index);

    //==============================================================================
    /** Indicates whether to use a case-insensitive search when looking up a key string.
    */
    z0 setIgnoresCase (b8 shouldIgnoreCase);

    /** Indicates whether a case-insensitive search is used when looking up a key string.
    */
    b8 getIgnoresCase() const noexcept;

    //==============================================================================
    /** Returns a descriptive string containing the items.
        This is handy for dumping the contents of an array.
    */
    Txt getDescription() const;

    //==============================================================================
    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    z0 minimiseStorageOverheads();

    //==============================================================================
    /** Adds the contents of a map to this StringPairArray. */
    z0 addMap (const std::map<Txt, Txt>& mapToAdd);

    /** Adds the contents of an unordered map to this StringPairArray. */
    z0 addUnorderedMap (const std::unordered_map<Txt, Txt>& mapToAdd);

private:
    //==============================================================================
    template <typename Map>
    z0 addMapImpl (const Map& mapToAdd);

    StringArray keys, values;
    b8 ignoreCase;

    DRX_LEAK_DETECTOR (StringPairArray)
};

} // namespace drx
