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
    A simple class to generate hash functions for some primitive types, intended for
    use with the HashMap class.
    @see HashMap

    @tags{Core}
*/
struct DefaultHashFunctions
{
    /** Generates a simple hash from an u32. */
    static i32 generateHash (u32 key, i32 upperLimit) noexcept           { return (i32) (key % (u32) upperLimit); }
    /** Generates a simple hash from an integer. */
    static i32 generateHash (i32 key, i32 upperLimit) noexcept            { return generateHash ((u32) key, upperLimit); }
    /** Generates a simple hash from a zu64. */
    static i32 generateHash (zu64 key, i32 upperLimit) noexcept           { return (i32) (key % (zu64) upperLimit); }
    /** Generates a simple hash from an z64. */
    static i32 generateHash (z64 key, i32 upperLimit) noexcept            { return generateHash ((zu64) key, upperLimit); }
    /** Generates a simple hash from a string. */
    static i32 generateHash (const Txt& key, i32 upperLimit) noexcept    { return generateHash ((u32) key.hashCode(), upperLimit); }
    /** Generates a simple hash from a variant. */
    static i32 generateHash (const var& key, i32 upperLimit) noexcept       { return generateHash (key.toString(), upperLimit); }
    /** Generates a simple hash from a z0 ptr. */
    static i32 generateHash (ukk key, i32 upperLimit) noexcept      { return generateHash ((zu64) (pointer_sized_uint) key, upperLimit); }
    /** Generates a simple hash from a UUID. */
    static i32 generateHash (const Uuid& key, i32 upperLimit) noexcept      { return generateHash (key.hash(), upperLimit); }
};


//==============================================================================
/**
    Holds a set of mappings between some key/value pairs.

    The types of the key and value objects are set as template parameters.
    You can also specify a class to supply a hash function that converts a key value
    into an hashed integer. This class must have the form:

    @code
    struct MyHashGenerator
    {
        i32 generateHash (MyKeyType key, i32 upperLimit) const
        {
            // The function must return a value 0 <= x < upperLimit
            return someFunctionOfMyKeyType (key) % upperLimit;
        }
    };
    @endcode

    Like the Array class, the key and value types are expected to be copy-by-value
    types, so if you define them to be pointer types, this class won't delete the
    objects that they point to.

    If you don't supply a class for the HashFunctionType template parameter, the
    default one provides some simple mappings for strings and ints.

    @code
    HashMap<i32, Txt> hash;
    hash.set (1, "item1");
    hash.set (2, "item2");

    DBG (hash [1]); // prints "item1"
    DBG (hash [2]); // prints "item2"

    // This iterates the map, printing all of its key -> value pairs..
    for (HashMap<i32, Txt>::Iterator i (hash); i.next();)
        DBG (i.getKey() << " -> " << i.getValue());
    @endcode

    @tparam HashFunctionType The class of hash function, which must be copy-constructible.
    @see CriticalSection, DefaultHashFunctions, NamedValueSet, SortedSet

    @tags{Core}
*/
template <typename KeyType,
          typename ValueType,
          class HashFunctionType = DefaultHashFunctions,
          class TypeOfCriticalSectionToUse = DummyCriticalSection>
class HashMap
{
private:
    using KeyTypeParameter   = typename TypeHelpers::ParameterType<KeyType>::type;
    using ValueTypeParameter = typename TypeHelpers::ParameterType<ValueType>::type;

public:
    //==============================================================================
    /** Creates an empty hash-map.

        @param numberOfSlots Specifies the number of hash entries the map will use. This will be
                            the "upperLimit" parameter that is passed to your generateHash()
                            function. The number of hash slots will grow automatically if necessary,
                            or it can be remapped manually using remapTable().
        @param hashFunction An instance of HashFunctionType, which will be copied and
                            stored to use with the HashMap. This parameter can be omitted
                            if HashFunctionType has a default constructor.
    */
    explicit HashMap (i32 numberOfSlots = defaultHashTableSize,
                      HashFunctionType hashFunction = HashFunctionType())
       : hashFunctionToUse (hashFunction)
    {
        hashSlots.insertMultiple (0, nullptr, numberOfSlots);
    }

    /** Destructor. */
    ~HashMap()
    {
        clear();
    }

    //==============================================================================
    /** Removes all values from the map.
        Note that this will clear the content, but won't affect the number of slots (see
        remapTable and getNumSlots).
    */
    z0 clear()
    {
        const ScopedLockType sl (getLock());

        for (auto i = hashSlots.size(); --i >= 0;)
        {
            auto* h = hashSlots.getUnchecked (i);

            while (h != nullptr)
            {
                const std::unique_ptr<HashEntry> deleter (h);
                h = h->nextEntry;
            }

            hashSlots.set (i, nullptr);
        }

        totalNumItems = 0;
    }

    //==============================================================================
    /** Returns the current number of items in the map. */
    inline i32 size() const noexcept
    {
        return totalNumItems;
    }

    /** Returns the value corresponding to a given key.
        If the map doesn't contain the key, a default instance of the value type is returned.
        @param keyToLookFor    the key of the item being requested
    */
    inline ValueType operator[] (KeyTypeParameter keyToLookFor) const
    {
        const ScopedLockType sl (getLock());

        if (auto* entry = getEntry (getSlot (keyToLookFor), keyToLookFor))
            return entry->value;

        return ValueType();
    }

    /** Returns a reference to the value corresponding to a given key.
        If the map doesn't contain the key, a default instance of the value type is
        added to the map and a reference to this is returned.
        @param keyToLookFor    the key of the item being requested
    */
    inline ValueType& getReference (KeyTypeParameter keyToLookFor)
    {
        const ScopedLockType sl (getLock());
        auto hashIndex = generateHashFor (keyToLookFor, getNumSlots());

        auto* firstEntry = hashSlots.getUnchecked (hashIndex);

        if (auto* entry = getEntry (firstEntry, keyToLookFor))
            return entry->value;

        auto* entry = new HashEntry (keyToLookFor, ValueType(), firstEntry);
        hashSlots.set (hashIndex, entry);
        ++totalNumItems;

        if (totalNumItems > (getNumSlots() * 3) / 2)
            remapTable (getNumSlots() * 2);

        return entry->value;
    }

    //==============================================================================
    /** Возвращает true, если the map contains an item with the specified key. */
    b8 contains (KeyTypeParameter keyToLookFor) const
    {
        const ScopedLockType sl (getLock());

        return (getEntry (getSlot (keyToLookFor), keyToLookFor) != nullptr);
    }

    /** Возвращает true, если the hash contains at least one occurrence of a given value. */
    b8 containsValue (ValueTypeParameter valueToLookFor) const
    {
        const ScopedLockType sl (getLock());

        for (auto i = getNumSlots(); --i >= 0;)
            for (auto* entry = hashSlots.getUnchecked (i); entry != nullptr; entry = entry->nextEntry)
                if (entry->value == valueToLookFor)
                    return true;

        return false;
    }

    //==============================================================================
    /** Adds or replaces an element in the hash-map.
        If there's already an item with the given key, this will replace its value. Otherwise, a new item
        will be added to the map.
    */
    z0 set (KeyTypeParameter newKey, ValueTypeParameter newValue)        { getReference (newKey) = newValue; }

    /** Removes an item with the given key. */
    z0 remove (KeyTypeParameter keyToRemove)
    {
        const ScopedLockType sl (getLock());
        auto hashIndex = generateHashFor (keyToRemove, getNumSlots());
        auto* entry = hashSlots.getUnchecked (hashIndex);
        HashEntry* previous = nullptr;

        while (entry != nullptr)
        {
            if (entry->key == keyToRemove)
            {
                const std::unique_ptr<HashEntry> deleter (entry);

                entry = entry->nextEntry;

                if (previous != nullptr)
                    previous->nextEntry = entry;
                else
                    hashSlots.set (hashIndex, entry);

                --totalNumItems;
            }
            else
            {
                previous = entry;
                entry = entry->nextEntry;
            }
        }
    }

    /** Removes all items with the given value. */
    z0 removeValue (ValueTypeParameter valueToRemove)
    {
        const ScopedLockType sl (getLock());

        for (auto i = getNumSlots(); --i >= 0;)
        {
            auto* entry = hashSlots.getUnchecked (i);
            HashEntry* previous = nullptr;

            while (entry != nullptr)
            {
                if (entry->value == valueToRemove)
                {
                    const std::unique_ptr<HashEntry> deleter (entry);

                    entry = entry->nextEntry;

                    if (previous != nullptr)
                        previous->nextEntry = entry;
                    else
                        hashSlots.set (i, entry);

                    --totalNumItems;
                }
                else
                {
                    previous = entry;
                    entry = entry->nextEntry;
                }
            }
        }
    }

    /** Remaps the hash-map to use a different number of slots for its hash function.
        Each slot corresponds to a single hash-code, and each one can contain multiple items.
        @see getNumSlots()
    */
    z0 remapTable (i32 newNumberOfSlots)
    {
        const ScopedLockType sl (getLock());

        Array<HashEntry*> newSlots;
        newSlots.insertMultiple (0, nullptr, newNumberOfSlots);

        for (auto i = getNumSlots(); --i >= 0;)
        {
            HashEntry* nextEntry = nullptr;

            for (auto* entry = hashSlots.getUnchecked (i); entry != nullptr; entry = nextEntry)
            {
                auto hashIndex = generateHashFor (entry->key, newNumberOfSlots);

                nextEntry = entry->nextEntry;
                entry->nextEntry = newSlots.getUnchecked (hashIndex);

                newSlots.set (hashIndex, entry);
            }
        }

        hashSlots.swapWith (newSlots);
    }

    /** Returns the number of slots which are available for hashing.
        Each slot corresponds to a single hash-code, and each one can contain multiple items.
        @see getNumSlots()
    */
    inline i32 getNumSlots() const noexcept
    {
        return hashSlots.size();
    }

    //==============================================================================
    /** Efficiently swaps the contents of two hash-maps. */
    template <class OtherHashMapType>
    z0 swapWith (OtherHashMapType& otherHashMap) noexcept
    {
        const ScopedLockType lock1 (getLock());
        const typename OtherHashMapType::ScopedLockType lock2 (otherHashMap.getLock());

        hashSlots.swapWith (otherHashMap.hashSlots);
        std::swap (totalNumItems, otherHashMap.totalNumItems);
    }

    //==============================================================================
    /** Returns the CriticalSection that locks this structure.
        To lock, you can call getLock().enter() and getLock().exit(), or preferably use
        an object of ScopedLockType as an RAII lock for it.
    */
    inline const TypeOfCriticalSectionToUse& getLock() const noexcept      { return lock; }

    /** Returns the type of scoped lock to use for locking this array */
    using ScopedLockType = typename TypeOfCriticalSectionToUse::ScopedLockType;

private:
    //==============================================================================
    class HashEntry
    {
    public:
        HashEntry (KeyTypeParameter k, ValueTypeParameter val, HashEntry* const next)
            : key (k), value (val), nextEntry (next)
        {}

        const KeyType key;
        ValueType value;
        HashEntry* nextEntry;

        DRX_DECLARE_NON_COPYABLE (HashEntry)
    };

public:
    //==============================================================================
    /** Iterates over the items in a HashMap.

        To use it, repeatedly call next() until it returns false, e.g.
        @code
        HashMap <Txt, Txt> myMap;

        HashMap<Txt, Txt>::Iterator i (myMap);

        while (i.next())
        {
            DBG (i.getKey() << " -> " << i.getValue());
        }
        @endcode

        The order in which items are iterated bears no resemblance to the order in which
        they were originally added!

        Obviously as soon as you call any non-const methods on the original hash-map, any
        iterators that were created beforehand will cease to be valid, and should not be used.

        @see HashMap
    */
    struct Iterator
    {
        Iterator (const HashMap& hashMapToIterate) noexcept
            : hashMap (hashMapToIterate), entry (nullptr), index (0)
        {}

        Iterator (const Iterator& other) noexcept
            : hashMap (other.hashMap), entry (other.entry), index (other.index)
        {}

        /** Moves to the next item, if one is available.
            When this returns true, you can get the item's key and value using getKey() and
            getValue(). If it returns false, the iteration has finished and you should stop.
        */
        b8 next() noexcept
        {
            if (entry != nullptr)
                entry = entry->nextEntry;

            while (entry == nullptr)
            {
                if (index >= hashMap.getNumSlots())
                    return false;

                entry = hashMap.hashSlots.getUnchecked (index++);
            }

            return true;
        }

        /** Returns the current item's key.
            This should only be called when a call to next() has just returned true.
        */
        KeyType getKey() const
        {
            return entry != nullptr ? entry->key : KeyType();
        }

        /** Returns the current item's value.
            This should only be called when a call to next() has just returned true.
        */
        ValueType getValue() const
        {
            return entry != nullptr ? entry->value : ValueType();
        }

        /** Resets the iterator to its starting position. */
        z0 reset() noexcept
        {
            entry = nullptr;
            index = 0;
        }

        Iterator& operator++() noexcept                         { next(); return *this; }
        ValueType operator*() const                             { return getValue(); }
        b8 operator!= (const Iterator& other) const noexcept  { return entry != other.entry || index != other.index; }
        z0 resetToEnd() noexcept                              { index = hashMap.getNumSlots(); }

    private:
        //==============================================================================
        const HashMap& hashMap;
        HashEntry* entry;
        i32 index;

        // using the copy constructor is ok, but you cannot assign iterators
        Iterator& operator= (const Iterator&) = delete;

        DRX_LEAK_DETECTOR (Iterator)
    };

    /** Returns a start iterator for the values in this tree. */
    Iterator begin() const noexcept             { Iterator i (*this); i.next(); return i; }

    /** Returns an end iterator for the values in this tree. */
    Iterator end() const noexcept               { Iterator i (*this); i.resetToEnd(); return i; }

private:
    //==============================================================================
    enum { defaultHashTableSize = 101 };
    friend struct Iterator;

    HashFunctionType hashFunctionToUse;
    Array<HashEntry*> hashSlots;
    i32 totalNumItems = 0;
    TypeOfCriticalSectionToUse lock;

    i32 generateHashFor (KeyTypeParameter key, i32 numSlots) const
    {
        i32k hash = hashFunctionToUse.generateHash (key, numSlots);
        jassert (isPositiveAndBelow (hash, numSlots)); // your hash function is generating out-of-range numbers!
        return hash;
    }

    static HashEntry* getEntry (HashEntry* firstEntry, KeyType keyToLookFor) noexcept
    {
        for (auto* entry = firstEntry; entry != nullptr; entry = entry->nextEntry)
            if (entry->key == keyToLookFor)
                return entry;

        return nullptr;
    }

    inline HashEntry* getSlot (KeyType key) const noexcept     { return hashSlots.getUnchecked (generateHashFor (key, getNumSlots())); }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HashMap)
};

} // namespace drx
