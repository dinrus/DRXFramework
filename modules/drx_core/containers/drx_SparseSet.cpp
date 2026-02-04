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

#if DRX_UNIT_TESTS

class SparseSetTests final : public UnitTest
{
public:
    SparseSetTests()
        : UnitTest ("SparseSet class", UnitTestCategories::containers)
    {}

    z0 runTest() override
    {
        beginTest ("basic operations");
        {
            SparseSet<i32> set;

            expect (set.isEmpty());
            expectEquals (set.size(), 0);
            expectEquals (set.getNumRanges(), 0);
            expect (set.getTotalRange().isEmpty());

            set.addRange ({0, 10});
            expect (! set.isEmpty());
            expectEquals (set.size(), 10);
            expectEquals (set.getNumRanges(), 1);
            expect (! set.getTotalRange().isEmpty());
            expect (set.getRange (0) == Range<i32> (0, 10));

            expectEquals (set[0], 0);
            expectEquals (set[5], 5);
            expectEquals (set[9], 9);
            // Index out of range yields a default value for a type
            expectEquals (set[10], 0);
            expect (set.contains (0));
            expect (set.contains (9));
            expect (! set.contains (10));
        }

        beginTest ("adding ranges");
        {
            SparseSet<i32> set;

            // Adding same range twice should yield just a single range
            set.addRange ({0, 10});
            set.addRange ({0, 10});
            expectEquals (set.getNumRanges(), 1);
            expect (set.getRange (0) == Range<i32> (0, 10));

            // Adding already included range does not increase num ranges
            set.addRange ({0, 2});
            expectEquals (set.getNumRanges(), 1);
            set.addRange ({8, 10});
            expectEquals (set.getNumRanges(), 1);
            set.addRange ({2, 5});
            expectEquals (set.getNumRanges(), 1);

            // Adding non adjacent range includes total number of ranges
            set.addRange ({-10, -5});
            expectEquals (set.getNumRanges(), 2);
            expect (set.getRange (0) == Range<i32> (-10, -5));
            expect (set.getRange (1) == Range<i32> (0, 10));
            expect (set.getTotalRange() == Range<i32> (-10, 10));

            set.addRange ({15, 20});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (0) == Range<i32> (-10, -5));
            expect (set.getRange (1) == Range<i32> (0, 10));
            expect (set.getRange (2) == Range<i32> (15, 20));
            expect (set.getTotalRange() == Range<i32> (-10, 20));

            // Adding adjacent ranges merges them.
            set.addRange ({-5, -3});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (0) == Range<i32> (-10, -3));
            expect (set.getRange (1) == Range<i32> (0, 10));
            expect (set.getRange (2) == Range<i32> (15, 20));
            expect (set.getTotalRange() == Range<i32> (-10, 20));

            set.addRange ({20, 25});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (0) == Range<i32> (-10, -3));
            expect (set.getRange (1) == Range<i32> (0, 10));
            expect (set.getRange (2) == Range<i32> (15, 25));
            expect (set.getTotalRange() == Range<i32> (-10, 25));

            // Adding range containing other ranges merges them
            set.addRange ({-50, 50});
            expectEquals (set.getNumRanges(), 1);
            expect (set.getRange (0) == Range<i32> (-50, 50));
            expect (set.getTotalRange() == Range<i32> (-50, 50));
        }

        beginTest ("removing ranges");
        {
            SparseSet<i32> set;

            set.addRange ({-20, -10});
            set.addRange ({0, 10});
            set.addRange ({20, 30});
            expectEquals (set.getNumRanges(), 3);

            // Removing ranges not included in the set has no effect
            set.removeRange ({-5, 5});
            expectEquals (set.getNumRanges(), 3);

            // Removing partially overlapping range
            set.removeRange ({-15, 5});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (0) == Range<i32> (-20, -15));
            expect (set.getRange (1) == Range<i32> (5, 10));
            expect (set.getRange (2) == Range<i32> (20, 30));

            // Removing subrange of existing range
            set.removeRange ({20, 22});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (2) == Range<i32> (22, 30));

            set.removeRange ({28, 30});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (2) == Range<i32> (22, 28));

            set.removeRange ({24, 26});
            expectEquals (set.getNumRanges(), 4);
            expect (set.getRange (0) == Range<i32> (-20, -15));
            expect (set.getRange (1) == Range<i32> (5, 10));
            expect (set.getRange (2) == Range<i32> (22, 24));
            expect (set.getRange (3) == Range<i32> (26, 28));
        }

        beginTest ("XORing ranges");
        {
            SparseSet<i32> set;
            set.addRange ({0, 10});

            set.invertRange ({0, 10});
            expectEquals (set.getNumRanges(), 0);
            set.invertRange ({0, 10});
            expectEquals (set.getNumRanges(), 1);

            set.invertRange ({4, 6});
            expectEquals (set.getNumRanges(), 2);
            expect (set.getRange (0) == Range<i32> (0, 4));
            expect (set.getRange (1) == Range<i32> (6, 10));

            set.invertRange ({-2, 2});
            expectEquals (set.getNumRanges(), 3);
            expect (set.getRange (0) == Range<i32> (-2, 0));
            expect (set.getRange (1) == Range<i32> (2, 4));
            expect (set.getRange (2) == Range<i32> (6, 10));
        }

        beginTest ("range contains & overlaps checks");
        {
            SparseSet<i32> set;
            set.addRange ({0, 10});

            expect (set.containsRange (Range<i32> (0, 2)));
            expect (set.containsRange (Range<i32> (8, 10)));
            expect (set.containsRange (Range<i32> (0, 10)));

            expect (! set.containsRange (Range<i32> (-2, 0)));
            expect (! set.containsRange (Range<i32> (-2, 10)));
            expect (! set.containsRange (Range<i32> (10, 12)));
            expect (! set.containsRange (Range<i32> (0, 12)));

            expect (set.overlapsRange (Range<i32> (0, 2)));
            expect (set.overlapsRange (Range<i32> (8, 10)));
            expect (set.overlapsRange (Range<i32> (0, 10)));

            expect (! set.overlapsRange (Range<i32> (-2, 0)));
            expect (  set.overlapsRange (Range<i32> (-2, 10)));
            expect (! set.overlapsRange (Range<i32> (10, 12)));
            expect (  set.overlapsRange (Range<i32> (0, 12)));
        }
    }
};

static SparseSetTests sparseSetTests;

#endif

} // namespace drx
