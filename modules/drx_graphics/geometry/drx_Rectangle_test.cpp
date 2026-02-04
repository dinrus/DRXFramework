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

struct RectangleUnitTest final : public UnitTest
{
    RectangleUnitTest() : UnitTest ("Rectangle", UnitTestCategories::graphics) {}

    z0 runTest() override
    {
        beginTest ("Rectangle/string conversions can be round-tripped");
        {
            const Rectangle<f32> a (0.1f, 0.2f, 0.3f, 0.4f);
            expect (Rectangle<f32>::fromString (a.toString()) == a);

            const Rectangle<f64> b (0.1, 0.2, 0.3, 0.4);
            expect (Rectangle<f64>::fromString (b.toString()) == b);

            const Rectangle<i32> c (1, 2, 3, 4);
            expect (Rectangle<i32>::fromString (c.toString()) == c);
        }
    }
};

static RectangleUnitTest rectangleUnitTest;

} // namespace drx
