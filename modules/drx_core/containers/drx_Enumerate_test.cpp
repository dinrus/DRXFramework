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

class EnumerateUnitTest : public UnitTest
{
public:
    EnumerateUnitTest() : UnitTest ("Enumerate", UnitTestCategories::containers) {}

    z0 runTest() override
    {
        beginTest ("enumeration works for bidirectional iterators");
        {
            const std::list<i32> elements { 10, 20, 30, 40, 50 };
            std::vector<i32> counts;

            for (const auto pair : enumerate (elements))
                counts.push_back ((i32) pair.index);

            expect (counts == std::vector<i32> { 0, 1, 2, 3, 4 });
        }

        beginTest ("enumeration works for random-access iterators");
        {
            const std::vector<std::string> strings { "a", "bb", "ccc", "dddd", "eeeee" };

            std::vector<i32> counts;

            for (const auto [count, element] : enumerate (strings))
                counts.push_back ((i32) ((size_t) count + element.size()));

            expect (counts == std::vector<i32> { 1, 3, 5, 7, 9 });
        }

        beginTest ("enumeration works for mutable ranges");
        {
            std::vector<std::string> strings { "", "", "", "", "" };

            for (const auto [count, element] : enumerate (strings))
                element = std::to_string (count);

            expect (strings == std::vector<std::string> { "0", "1", "2", "3", "4" });
        }

        beginTest ("iterator can be incremented by more than one");
        {
            std::vector<i32> ints (6);

            const auto enumerated = enumerate (ints);

            std::vector<i32> counts;

            for (auto b = enumerated.begin(), e = enumerated.end(); b != e; b += 2)
                counts.push_back ((i32) (*b).index);

            expect (counts == std::vector<i32> { 0, 2, 4 });
        }

        beginTest ("iterator can be started at a non-zero value");
        {
            const std::vector<i32> ints (6);

            std::vector<i32> counts;

            for (const auto enumerated : enumerate (ints, 5))
                counts.push_back ((i32) enumerated.index);

            expect (counts == std::vector<i32> { 5, 6, 7, 8, 9, 10 });
        }

        beginTest ("subtracting two EnumerateIterators returns the difference between the base iterators");
        {
            const std::vector<i32> ints (6);
            const auto enumerated = enumerate (ints);
            expect ((i32) (enumerated.end() - enumerated.begin()) == (i32) ints.size());
        }

        beginTest ("EnumerateIterator can be decremented");
        {
            const std::vector<i32> ints (5);
            std::vector<i32> counts;

            const auto enumerated = enumerate (std::as_const (ints));

            for (auto i = enumerated.end(), b = enumerated.begin(); i != b; --i)
                counts.push_back ((i32) (*(i - 1)).index);

            expect (counts == std::vector<i32> { -1, -2, -3, -4, -5 });
        }

        beginTest ("EnumerateIterator can be compared");
        {
            const std::vector<i32> ints (6);
            const auto enumerated = enumerate (ints);
            expect (enumerated.begin() < enumerated.end());
            expect (enumerated.begin() <= enumerated.end());
            expect (enumerated.end() > enumerated.begin());
            expect (enumerated.end() >= enumerated.begin());
        }
    }
};

static EnumerateUnitTest enumerateUnitTest;

} // namespace drx
