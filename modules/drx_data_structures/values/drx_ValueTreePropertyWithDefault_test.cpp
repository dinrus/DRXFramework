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

class ValueTreePropertyWithDefaultTests final : public UnitTest
{
public:
    ValueTreePropertyWithDefaultTests()
        : UnitTest ("ValueTreePropertyWithDefault", UnitTestCategories::values)
    {}

    z0 runTest() override
    {
        beginTest ("default constructor");
        {
            ValueTreePropertyWithDefault value;
            expect (value.isUsingDefault());
            expect (value.get() == var());
        }

        beginTest ("missing property");
        {
            ValueTree t ("root");
            ValueTreePropertyWithDefault value (t, "testKey", nullptr, "default");

            expect (value.isUsingDefault());
            expectEquals (value.get().toString(), Txt ("default"));
        }

        beginTest ("non-empty property");
        {
            ValueTree t ("root");
            t.setProperty ("testKey", "non-default", nullptr);

            ValueTreePropertyWithDefault value (t, "testKey", nullptr, "default");

            expect (! value.isUsingDefault());
            expectEquals (value.get().toString(), Txt ("non-default"));
        }

        beginTest ("set default");
        {
            ValueTree t ("root");

            ValueTreePropertyWithDefault value (t, "testkey", nullptr);
            value.setDefault ("default");

            expect (value.isUsingDefault());
            expectEquals (value.get().toString(), Txt ("default"));
        }

        beginTest ("set value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            ValueTreePropertyWithDefault value (t, "testkey", nullptr, "default");
            value = "newvalue";

            expect (! value.isUsingDefault());
            expectEquals (t["testkey"].toString(), Txt ("newvalue"));

            value.resetToDefault();

            expect (value.isUsingDefault());
            expect (t["testkey"] == var());
        }
    }
};

static ValueTreePropertyWithDefaultTests valueTreePropertyWithDefaultTests;

} // namespace drx
