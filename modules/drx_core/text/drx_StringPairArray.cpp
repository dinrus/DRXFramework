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

StringPairArray::StringPairArray (b8 shouldIgnoreCase)  : ignoreCase (shouldIgnoreCase)
{
}

StringPairArray::StringPairArray (const StringPairArray& other)
    : keys (other.keys),
      values (other.values),
      ignoreCase (other.ignoreCase)
{
}

StringPairArray& StringPairArray::operator= (const StringPairArray& other)
{
    keys = other.keys;
    values = other.values;
    return *this;
}

b8 StringPairArray::operator== (const StringPairArray& other) const
{
    auto num = size();

    if (num != other.size())
        return false;

    for (i32 i = 0; i < num; ++i)
    {
        if (keys[i] == other.keys[i]) // optimise for the case where the keys are in the same order
        {
            if (values[i] != other.values[i])
                return false;
        }
        else
        {
            // if we encounter keys that are in a different order, search remaining items by brute force..
            for (i32 j = i; j < num; ++j)
            {
                auto otherIndex = other.keys.indexOf (keys[j], other.ignoreCase);

                if (otherIndex < 0 || values[j] != other.values[otherIndex])
                    return false;
            }

            return true;
        }
    }

    return true;
}

b8 StringPairArray::operator!= (const StringPairArray& other) const
{
    return ! operator== (other);
}

const Txt& StringPairArray::operator[] (StringRef key) const
{
    return values[keys.indexOf (key, ignoreCase)];
}

Txt StringPairArray::getValue (StringRef key, const Txt& defaultReturnValue) const
{
    auto i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
        return values[i];

    return defaultReturnValue;
}

b8 StringPairArray::containsKey (StringRef key) const noexcept
{
    return keys.contains (key, ignoreCase);
}

z0 StringPairArray::set (const Txt& key, const Txt& value)
{
    auto i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
    {
        values.set (i, value);
    }
    else
    {
        keys.add (key);
        values.add (value);
    }
}

z0 StringPairArray::addArray (const StringPairArray& other)
{
    for (i32 i = 0; i < other.size(); ++i)
        set (other.keys[i], other.values[i]);
}

z0 StringPairArray::clear()
{
    keys.clear();
    values.clear();
}

z0 StringPairArray::remove (StringRef key)
{
    remove (keys.indexOf (key, ignoreCase));
}

z0 StringPairArray::remove (i32 index)
{
    keys.remove (index);
    values.remove (index);
}

z0 StringPairArray::setIgnoresCase (b8 shouldIgnoreCase)
{
    ignoreCase = shouldIgnoreCase;
}

b8 StringPairArray::getIgnoresCase() const noexcept
{
    return ignoreCase;
}

Txt StringPairArray::getDescription() const
{
    Txt s;

    for (i32 i = 0; i < keys.size(); ++i)
    {
        s << keys[i] << " = " << values[i];

        if (i < keys.size())
            s << ", ";
    }

    return s;
}

z0 StringPairArray::minimiseStorageOverheads()
{
    keys.minimiseStorageOverheads();
    values.minimiseStorageOverheads();
}

template <typename Map>
z0 StringPairArray::addMapImpl (const Map& toAdd)
{
    // If we just called `set` for each item in `toAdd`, that would
    // perform badly when adding to large StringPairArrays, as `set`
    // has to loop through the whole container looking for matching keys.
    // Instead, we use a temporary map to give us better lookup performance.
    std::map<Txt, i32> contents;

    const auto normaliseKey = [this] (const Txt& key)
    {
        return ignoreCase ? key.toLowerCase() : key;
    };

    for (auto i = 0; i != size(); ++i)
        contents.emplace (normaliseKey (getAllKeys().getReference (i)), i);

    for (const auto& pair : toAdd)
    {
        const auto key = normaliseKey (pair.first);
        const auto it = contents.find (key);

        if (it != contents.cend())
        {
            values.getReference (it->second) = pair.second;
        }
        else
        {
            contents.emplace (key, static_cast<i32> (contents.size()));
            keys.add (pair.first);
            values.add (pair.second);
        }
    }
}

z0 StringPairArray::addUnorderedMap (const std::unordered_map<Txt, Txt>& toAdd) { addMapImpl (toAdd); }
z0 StringPairArray::addMap (const std::map<Txt, Txt>& toAdd)                    { addMapImpl (toAdd); }

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

static Txt operator""_S (tukk chars, size_t)
{
    return Txt { chars };
}

class StringPairArrayTests final : public UnitTest
{
public:
    StringPairArrayTests()
        : UnitTest ("StringPairArray", UnitTestCategories::text)
    {}

    z0 runTest() override
    {
        beginTest ("addMap respects case sensitivity of StringPairArray");
        {
            StringPairArray insensitive { true };
            insensitive.addMap ({ { "duplicate", "a" },
                                  { "Duplicate", "b" } });

            expect (insensitive.size() == 1);
            expectEquals (insensitive["DUPLICATE"], "a"_S);

            StringPairArray sensitive { false };
            sensitive.addMap ({ { "duplicate", "a"_S },
                                { "Duplicate", "b"_S } });

            expect (sensitive.size() == 2);
            expectEquals (sensitive["duplicate"], "a"_S);
            expectEquals (sensitive["Duplicate"], "b"_S);
            expectEquals (sensitive["DUPLICATE"], ""_S);
        }

        beginTest ("addMap overwrites existing pairs");
        {
            StringPairArray insensitive { true };
            insensitive.set ("key", "value");
            insensitive.addMap ({ { "KEY", "VALUE" } });

            expect (insensitive.size() == 1);
            expectEquals (insensitive.getAllKeys()[0], "key"_S);
            expectEquals (insensitive.getAllValues()[0], "VALUE"_S);

            StringPairArray sensitive { false };
            sensitive.set ("key", "value");
            sensitive.addMap ({ { "KEY", "VALUE" },
                                { "key", "another value" } });

            expect (sensitive.size() == 2);
            expect (sensitive.getAllKeys() == StringArray { "key", "KEY" });
            expect (sensitive.getAllValues() == StringArray { "another value", "VALUE" });
        }

        beginTest ("addMap doesn't change the order of existing keys");
        {
            StringPairArray array;
            array.set ("a", "a");
            array.set ("z", "z");
            array.set ("b", "b");
            array.set ("y", "y");
            array.set ("c", "c");

            array.addMap ({ { "B", "B" },
                            { "0", "0" },
                            { "Z", "Z" } });

            expect (array.getAllKeys() == StringArray { "a", "z", "b", "y", "c", "0" });
            expect (array.getAllValues() == StringArray { "a", "Z", "B", "y", "c", "0" });
        }

        beginTest ("addMap has equivalent behaviour to addArray");
        {
            StringPairArray initial;
            initial.set ("aaa", "aaa");
            initial.set ("zzz", "zzz");
            initial.set ("bbb", "bbb");

            auto withAddMap = initial;
            withAddMap.addMap ({ { "ZZZ", "ZZZ" },
                                 { "ddd", "ddd" } });

            auto withAddArray = initial;
            withAddArray.addArray ([]
            {
                StringPairArray toAdd;
                toAdd.set ("ZZZ", "ZZZ");
                toAdd.set ("ddd", "ddd");
                return toAdd;
            }());

            expect (withAddMap == withAddArray);
        }
    }
};

static StringPairArrayTests stringPairArrayTests;

#endif

} // namespace drx
