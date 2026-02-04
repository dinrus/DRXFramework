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

class ParallelogramTest : public UnitTest
{
public:
    ParallelogramTest() : UnitTest ("Parallelogram", UnitTestCategories::graphics) {}

    z0 runTest() override
    {
        beginTest ("isEmpty");
        {
            expect (! Parallelogram (Rectangle<i32> (10, 10, 20, 20)).isEmpty());
            expect (Parallelogram (Rectangle<i32> (10, 10, 0, 20)).isEmpty());
            expect (Parallelogram (Rectangle<i32> (10, 10, 20, 0)).isEmpty());

            expect (! Parallelogram (Point<i32> (0, 0), Point<i32> (10, 10), Point<i32> (20, 0)).isEmpty());
            expect (Parallelogram (Point<i32> (0, 0), Point<i32> (0, 0), Point<i32> (20, 0)).isEmpty());
            expect (Parallelogram (Point<i32> (0, 0), Point<i32> (10, 10), Point<i32> (10, 10)).isEmpty());
            expect (Parallelogram (Point<i32> (20, 0), Point<i32> (10, 10), Point<i32> (20, 0)).isEmpty());
        }

        beginTest ("operators");
        {
            Parallelogram p (Rectangle<i32> (10, 10, 20, 20));
            p += Point<i32> (5, 10);
            expect (p.topLeft == Point<i32> (15, 20));
            expect (p.topRight == Point<i32> (35, 20));
            expect (p.bottomLeft == Point<i32> (15, 40));

            p -= Point<i32> (10, 5);
            expect (p.topLeft == Point<i32> (5, 15));
            expect (p.topRight == Point<i32> (25, 15));
            expect (p.bottomLeft == Point<i32> (5, 35));
        }
    }
};

static ParallelogramTest parallelogramTest;

} // namespace drx
