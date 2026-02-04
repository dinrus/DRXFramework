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

static CommonSmoothedValueTests <SmoothedValue<f32, ValueSmoothingTypes::Linear>> commonLinearSmoothedValueTests;
static CommonSmoothedValueTests <SmoothedValue<f32, ValueSmoothingTypes::Multiplicative>> commonMultiplicativeSmoothedValueTests;

class SmoothedValueTests final : public UnitTest
{
public:
    SmoothedValueTests()
        : UnitTest ("SmoothedValueTests", UnitTestCategories::smoothedValues)
    {}

    z0 runTest() override
    {
        beginTest ("Linear moving target");
        {
            SmoothedValue<f32, ValueSmoothingTypes::Linear> sv;

            sv.reset (12);
            f32 initialValue = 0.0f;
            sv.setCurrentAndTargetValue (initialValue);
            sv.setTargetValue (1.0f);

            auto delta = sv.getNextValue() - initialValue;

            sv.skip (6);

            auto newInitialValue = sv.getCurrentValue();
            sv.setTargetValue (newInitialValue + 2.0f);
            auto doubleDelta = sv.getNextValue() - newInitialValue;

            expectWithinAbsoluteError (doubleDelta, delta * 2.0f, 1.0e-7f);
        }

        beginTest ("Multiplicative curve");
        {
            SmoothedValue<f64, ValueSmoothingTypes::Multiplicative> sv;

            auto numSamples = 12;
            AudioBuffer<f64> values (2, numSamples + 1);

            sv.reset (numSamples);
            sv.setCurrentAndTargetValue (1.0);
            sv.setTargetValue (2.0f);

            values.setSample (0, 0, sv.getCurrentValue());

            for (i32 i = 1; i < values.getNumSamples(); ++i)
                values.setSample (0, i, sv.getNextValue());

            sv.setTargetValue (1.0f);
            values.setSample (1, values.getNumSamples() - 1, sv.getCurrentValue());

            for (i32 i = values.getNumSamples() - 2; i >= 0 ; --i)
                values.setSample (1, i, sv.getNextValue());

            for (i32 i = 0; i < values.getNumSamples(); ++i)
                expectWithinAbsoluteError (values.getSample (0, i), values.getSample (1, i), 1.0e-9);
        }
    }
};

static SmoothedValueTests smoothedValueTests;

#endif

} // namespace drx
