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

struct ThreadPriorities
{
    struct Entry
    {
        Thread::Priority priority;
        i32 native;
    };

   #if DRX_ANDROID
    enum AndroidThreadPriority
    {
        THREAD_PRIORITY_AUDIO          = -16,
        THREAD_PRIORITY_FOREGROUND     = -2,
        THREAD_PRIORITY_MORE_FAVORABLE = -1,
        THREAD_PRIORITY_DEFAULT        = 0,
        THREAD_PRIORITY_LESS_FAVORABLE = 1,
        THREAD_PRIORITY_BACKGROUND     = 10,
        THREAD_PRIORITY_LOWEST         = 19
    };
   #endif

    static inline constexpr Entry table[]
    {
       #if DRX_ANDROID
        { Thread::Priority::highest,    AndroidThreadPriority::THREAD_PRIORITY_AUDIO },
        { Thread::Priority::high,       AndroidThreadPriority::THREAD_PRIORITY_FOREGROUND },
        { Thread::Priority::normal,     AndroidThreadPriority::THREAD_PRIORITY_DEFAULT },
        { Thread::Priority::low,        AndroidThreadPriority::THREAD_PRIORITY_BACKGROUND - 5 },
        { Thread::Priority::background, AndroidThreadPriority::THREAD_PRIORITY_BACKGROUND },
       #endif

       #if DRX_LINUX || DRX_BSD
        { Thread::Priority::highest,    0 },
        { Thread::Priority::high,       0 },
        { Thread::Priority::normal,     0 },
        { Thread::Priority::low,        0 },
        { Thread::Priority::background, 0 },
       #endif

       #if DRX_MAC || DRX_IOS
        { Thread::Priority::highest,    4 },
        { Thread::Priority::high,       3 },
        { Thread::Priority::normal,     2 },
        { Thread::Priority::low,        1 },
        { Thread::Priority::background, 0 },
       #endif

       #if DRX_WINDOWS
        { Thread::Priority::highest,    THREAD_PRIORITY_TIME_CRITICAL },
        { Thread::Priority::high,       THREAD_PRIORITY_HIGHEST },
        { Thread::Priority::normal,     THREAD_PRIORITY_NORMAL },
        { Thread::Priority::low,        THREAD_PRIORITY_LOWEST },
        { Thread::Priority::background, THREAD_PRIORITY_IDLE },
       #endif
    };

    static_assert (std::size (table) == 5,
                   "The platform may be unsupported or there may be a priority entry missing.");

    static Thread::Priority getDrxPriority (i32k value)
    {
        const auto iter = std::min_element (std::begin (table),
                                            std::end   (table),
                                            [value] (const auto& a, const auto& b)
                                            {
                                                return std::abs (a.native - value) < std::abs (b.native - value);
                                            });

        jassert (iter != std::end (table));
        return iter != std::end (table) ? iter->priority : Thread::Priority{};
    }

    static i32 getNativePriority (const Thread::Priority value)
    {
        const auto iter = std::find_if (std::begin (table),
                                        std::end   (table),
                                        [value] (const auto& entry) { return entry.priority == value; });

        jassert (iter != std::end (table));
        return iter != std::end (table) ? iter->native : 0;
    }
};

} // namespace drx
