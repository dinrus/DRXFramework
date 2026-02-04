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

#ifndef DOXYGEN

namespace drx
{

template <typename Key, typename Value, size_t maxEntries = 128>
class LruCache
{
public:
    template <typename Fn>
    const Value& get (Key key, Fn&& fn)
    {
        if (const auto iter = map.find (key); iter != map.end())
        {
            list.erase (iter->second.listIterator);
            iter->second.listIterator = list.insert (list.end(), iter);
            return iter->second.value;
        }

        while (list.size() >= maxEntries)
        {
            const auto toRemove = list.begin();
            map.erase (*toRemove);
            list.erase (toRemove);
        }

        auto value = fn (key);
        const auto mapIteratorPair = map.emplace (std::move (key), Pair { std::move (value), {} });

        jassert (mapIteratorPair.second);

        mapIteratorPair.first->second.listIterator = list.insert (list.end(), mapIteratorPair.first);
        return mapIteratorPair.first->second.value;
    }

private:
    struct Pair
    {
        using Map = std::map<Key, Pair>;
        using MapIterator = typename Map::const_iterator;
        using List = std::list<MapIterator>;
        using ListIterator = typename List::const_iterator;

        Value value;
        ListIterator listIterator;
    };

    typename Pair::Map map;
    typename Pair::List list;
};

} // namespace drx

#endif
