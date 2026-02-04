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

class CachedValueTests final : public UnitTest
{
public:
    CachedValueTests()
        : UnitTest ("CachedValues", UnitTestCategories::values)
    {}

    z0 runTest() override
    {
        beginTest ("default constructor");
        {
            CachedValue<Txt> cv;
            expect (cv.isUsingDefault());
            expect (cv.get() == Txt());
        }

        beginTest ("without default value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            CachedValue<Txt> cv (t, "testkey", nullptr);

            expect (! cv.isUsingDefault());
            expect (cv.get() == "testvalue");

            cv.resetToDefault();

            expect (cv.isUsingDefault());
            expect (cv.get() == Txt());
        }

        beginTest ("with default value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            CachedValue<Txt> cv (t, "testkey", nullptr, "defaultvalue");

            expect (! cv.isUsingDefault());
            expect (cv.get() == "testvalue");

            cv.resetToDefault();

            expect (cv.isUsingDefault());
            expect (cv.get() == "defaultvalue");
        }

        beginTest ("with default value (i32)");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", 23, nullptr);

            CachedValue<i32> cv (t, "testkey", nullptr, 34);

            expect (! cv.isUsingDefault());
            expect (cv == 23);
            expectEquals (cv.get(), 23);

            cv.resetToDefault();

            expect (cv.isUsingDefault());
            expect (cv == 34);
        }

        beginTest ("with z0 value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", var(), nullptr);

            CachedValue<Txt> cv (t, "testkey", nullptr, "defaultvalue");

            expect (! cv.isUsingDefault());
            expect (cv == "");
            expectEquals (cv.get(), Txt());
        }

        beginTest ("with non-existent value");
        {
            ValueTree t ("root");

            CachedValue<Txt> cv (t, "testkey", nullptr, "defaultvalue");

            expect (cv.isUsingDefault());
            expect (cv == "defaultvalue");
            expect (cv.get() == "defaultvalue");
        }

        beginTest ("with value changing");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "oldvalue", nullptr);

            CachedValue<Txt> cv (t, "testkey", nullptr, "defaultvalue");
            expect (cv == "oldvalue");

            t.setProperty ("testkey", "newvalue", nullptr);
            expect (cv != "oldvalue");
            expect (cv == "newvalue");
        }

        beginTest ("set value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", 23, nullptr);

            CachedValue<i32> cv (t, "testkey", nullptr, 45);
            cv = 34;

            expectEquals ((i32) t["testkey"], 34);

            cv.resetToDefault();
            expect (cv == 45);
            expectEquals (cv.get(), 45);

            expect (t["testkey"] == var());
        }
    }
};

static CachedValueTests cachedValueTests;

#endif

} // namespace drx
