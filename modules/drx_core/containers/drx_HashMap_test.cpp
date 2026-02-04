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

struct HashMapTest final : public UnitTest
{
    HashMapTest()
        : UnitTest ("HashMap", UnitTestCategories::containers)
    {}

    z0 runTest() override
    {
        doTest<AddElementsTest> ("AddElementsTest");
        doTest<AccessTest> ("AccessTest");
        doTest<RemoveTest> ("RemoveTest");
        doTest<PersistantMemoryLocationOfValues> ("PersistantMemoryLocationOfValues");
    }

    //==============================================================================
    struct AddElementsTest
    {
        template <typename KeyType>
        static z0 run (UnitTest& u)
        {
            AssociativeMap<KeyType, i32> groundTruth;
            HashMap<KeyType, i32> hashMap;

            RandomKeys<KeyType> keyOracle (300, 3827829);
            Random valueOracle (48735);

            i32 totalValues = 0;
            for (i32 i = 0; i < 10000; ++i)
            {
                auto key = keyOracle.next();
                auto value = valueOracle.nextInt();

                b8 contains = (groundTruth.find (key) != nullptr);
                u.expectEquals ((i32) contains, (i32) hashMap.contains (key));

                groundTruth.add (key, value);
                hashMap.set (key, value);

                if (! contains) totalValues++;

                u.expectEquals (hashMap.size(), totalValues);
            }
        }
    };

    struct AccessTest
    {
        template <typename KeyType>
        static z0 run (UnitTest& u)
        {
            AssociativeMap<KeyType, i32> groundTruth;
            HashMap<KeyType, i32> hashMap;

            fillWithRandomValues (hashMap, groundTruth);

            for (auto pair : groundTruth.pairs)
                u.expectEquals (hashMap[pair.key], pair.value);
        }
    };

    struct RemoveTest
    {
        template <typename KeyType>
        static z0 run (UnitTest& u)
        {
            AssociativeMap<KeyType, i32> groundTruth;
            HashMap<KeyType, i32> hashMap;

            fillWithRandomValues (hashMap, groundTruth);
            auto n = groundTruth.size();

            Random r (3827387);

            for (i32 i = 0; i < 100; ++i)
            {
                auto idx = r.nextInt (n-- - 1);
                auto key = groundTruth.pairs.getReference (idx).key;

                groundTruth.pairs.remove (idx);
                hashMap.remove (key);

                u.expect (! hashMap.contains (key));

                for (auto pair : groundTruth.pairs)
                    u.expectEquals (hashMap[pair.key], pair.value);
            }
        }
    };

    // ensure that the addresses of object references don't change
    struct PersistantMemoryLocationOfValues
    {
        struct AddressAndValue { i32 value; i32k* valueAddress; };

        template <typename KeyType>
        static z0 run (UnitTest& u)
        {
            AssociativeMap<KeyType, AddressAndValue> groundTruth;
            HashMap<KeyType, i32> hashMap;

            RandomKeys<KeyType> keyOracle (300, 3827829);
            Random valueOracle (48735);

            for (i32 i = 0; i < 1000; ++i)
            {
                auto key = keyOracle.next();
                auto value = valueOracle.nextInt();

                hashMap.set (key, value);

                if (auto* existing = groundTruth.find (key))
                {
                    // don't change the address: only the value
                    existing->value = value;
                }
                else
                {
                    groundTruth.add (key, { value, &hashMap.getReference (key) });
                }

                for (auto pair : groundTruth.pairs)
                {
                    const auto& hashMapValue = hashMap.getReference (pair.key);

                    u.expectEquals (hashMapValue, pair.value.value);
                    u.expect (&hashMapValue == pair.value.valueAddress);
                }
            }

            auto n = groundTruth.size();
            Random r (3827387);

            for (i32 i = 0; i < 100; ++i)
            {
                auto idx = r.nextInt (n-- - 1);
                auto key = groundTruth.pairs.getReference (idx).key;

                groundTruth.pairs.remove (idx);
                hashMap.remove (key);

                for (auto pair : groundTruth.pairs)
                {
                    const auto& hashMapValue = hashMap.getReference (pair.key);

                    u.expectEquals (hashMapValue, pair.value.value);
                    u.expect (&hashMapValue == pair.value.valueAddress);
                }
            }
        }
    };

    //==============================================================================
    template <class Test>
    z0 doTest (const Txt& testName)
    {
        beginTest (testName);

        Test::template run<i32> (*this);
        Test::template run<uk> (*this);
        Test::template run<Txt> (*this);
    }

    //==============================================================================
    template <typename KeyType, typename ValueType>
    struct AssociativeMap
    {
        struct KeyValuePair { KeyType key; ValueType value; };

        ValueType* find (KeyType key)
        {
            auto n = pairs.size();

            for (i32 i = 0; i < n; ++i)
            {
                auto& pair = pairs.getReference (i);

                if (pair.key == key)
                    return &pair.value;
            }

            return nullptr;
        }

        z0 add (KeyType key, ValueType value)
        {
            if (ValueType* v = find (key))
                *v = value;
            else
                pairs.add ({key, value});
        }

        i32 size() const { return pairs.size(); }

        Array<KeyValuePair> pairs;
    };

    template <typename KeyType, typename ValueType>
    static z0 fillWithRandomValues (HashMap<KeyType, i32>& hashMap, AssociativeMap<KeyType, ValueType>& groundTruth)
    {
        RandomKeys<KeyType> keyOracle (300, 3827829);
        Random valueOracle (48735);

        for (i32 i = 0; i < 10000; ++i)
        {
            auto key = keyOracle.next();
            auto value = valueOracle.nextInt();

            groundTruth.add (key, value);
            hashMap.set (key, value);
        }
    }

    //==============================================================================
    template <typename KeyType>
    class RandomKeys
    {
    public:
        RandomKeys (i32 maxUniqueKeys, i32 seed) : r (seed)
        {
            for (i32 i = 0; i < maxUniqueKeys; ++i)
                keys.add (generateRandomKey (r));
        }

        const KeyType& next()
        {
            i32 i = r.nextInt (keys.size() - 1);
            return keys.getReference (i);
        }
    private:
        static KeyType generateRandomKey (Random&);

        Random r;
        Array<KeyType> keys;
    };
};

template <> i32   HashMapTest::RandomKeys<i32>  ::generateRandomKey (Random& rnd) { return rnd.nextInt(); }
template <> uk HashMapTest::RandomKeys<uk>::generateRandomKey (Random& rnd) { return reinterpret_cast<uk> (rnd.nextInt64()); }

template <> Txt HashMapTest::RandomKeys<Txt>::generateRandomKey (Random& rnd)
{
    Txt str;

    i32 len = rnd.nextInt (8)+1;
    for (i32 i = 0; i < len; ++i)
        str += static_cast<t8> (rnd.nextInt (95) + 32);

    return str;
}

static HashMapTest hashMapTest;

} // namespace drx
