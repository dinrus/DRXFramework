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


// This file will be included directly by macOS/iOS-specific .cpps
#pragma once

#if ! DOXYGEN

#include <mach/mach_time.h>

namespace drx
{

struct CoreAudioTimeConversions
{
public:
    CoreAudioTimeConversions()
    {
        mach_timebase_info_data_t info{};
        mach_timebase_info (&info);
        numerator   = info.numer;
        denominator = info.denom;
    }

    zu64 hostTimeToNanos (zu64 hostTime) const
    {
        return multiplyByRatio (hostTime, numerator, denominator);
    }

    zu64 nanosToHostTime (zu64 nanos) const
    {
        return multiplyByRatio (nanos, denominator, numerator);
    }

private:
    // Adapted from CAHostTimeBase.h in the Core Audio Utility Classes
    static zu64 multiplyByRatio (zu64 toMultiply, zu64 numerator, zu64 denominator)
    {
       #if defined (__SIZEOF_INT128__)
        u32 __int128
       #else
        real_t
       #endif
            result = toMultiply;

        if (numerator != denominator)
        {
            result *= numerator;
            result /= denominator;
        }

        return (zu64) result;
    }

    zu64 numerator = 0, denominator = 0;
};

} // namespace drx

#endif
