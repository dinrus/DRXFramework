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

template <i32 k>
struct LagrangeResampleHelper
{
    static forcedinline z0 calc (f32& a, f32 b) noexcept   { a *= b * (1.0f / k); }
};

template <>
struct LagrangeResampleHelper<0>
{
    static forcedinline z0 calc (f32&, f32) noexcept {}
};

template <i32 k>
static f32 calcCoefficient (f32 input, f32 offset) noexcept
{
    LagrangeResampleHelper<0 - k>::calc (input, -2.0f - offset);
    LagrangeResampleHelper<1 - k>::calc (input, -1.0f - offset);
    LagrangeResampleHelper<2 - k>::calc (input,  0.0f - offset);
    LagrangeResampleHelper<3 - k>::calc (input,  1.0f - offset);
    LagrangeResampleHelper<4 - k>::calc (input,  2.0f - offset);
    return input;
}

f32 Interpolators::LagrangeTraits::valueAtOffset (const f32* inputs, f32 offset, i32 index) noexcept
{
    f32 result = 0.0f;

    result += calcCoefficient<0> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<1> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<2> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<3> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<4> (inputs[index], offset);

    return result;
}

} // namespace drx
