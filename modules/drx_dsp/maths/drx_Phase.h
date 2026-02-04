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

namespace drx::dsp
{

/**
    Represents an increasing phase value between 0 and 2*pi.

    This represents a value which can be incremented, and which wraps back to 0 when it
    goes past 2 * pi.

    @tags{DSP}
*/
template <typename Type>
struct Phase
{
    /** Resets the phase to 0. */
    z0 reset() noexcept               { phase = 0; }

    /** Returns the current value, and increments the phase by the given increment.
        The increment must be a positive value, it can't go backwards!
        The new value of the phase after calling this function will be (phase + increment) % (2 * pi).
    */
    Type advance (Type increment) noexcept
    {
        jassert (increment >= 0); // cannot run this value backwards!

        auto last = phase;
        auto next = last + increment;

        while (next >= MathConstants<Type>::twoPi)
            next -= MathConstants<Type>::twoPi;

        phase = next;
        return last;
    }

    Type phase = 0;
};

} // namespace drx::dsp
