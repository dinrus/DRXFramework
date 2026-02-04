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

class PlatformTimer final
{
public:
    explicit PlatformTimer (PlatformTimerListener& ptl)
        : listener { ptl } {}

    z0 startTimer (i32 newIntervalMs)
    {
        jassert (newIntervalMs > 0);

        const auto callback = [] (UINT, UINT, DWORD_PTR context, DWORD_PTR, DWORD_PTR)
        {
            reinterpret_cast<PlatformTimerListener*> (context)->onTimerExpired();
        };

        timerId = timeSetEvent ((UINT) newIntervalMs, 1, callback, (DWORD_PTR) &listener, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
        const auto timerStarted = timerId != 0;

        if (timerStarted)
        {
            intervalMs = newIntervalMs;
            return;
        }

        if (fallbackTimer == nullptr)
        {
            // This assertion indicates that the creation of a high-resolution timer
            // failed, and the timer is falling back to a less accurate implementation.
            // Timer callbacks will still fire but the timing precision of the callbacks
            // will be significantly compromised!
            // The most likely reason for this is that more than the system limit of 16
            // HighResolutionTimers are trying to run simultaneously in the same process.
            // You may be able to reduce the number of HighResolutionTimer instances by
            // only creating one instance that is shared (see SharedResourcePointer).
            //
            // However, if this is a plugin running inside a host, other plugins could
            // be creating timers in the same process. In most cases it's best to find
            // an alternative approach than relying on the precision of any timer!
           #if ! DRX_UNIT_TESTS
            jassertfalse;
           #endif

            fallbackTimer = std::make_unique<GenericPlatformTimer> (listener);
        }

        fallbackTimer->startTimer (newIntervalMs);
        intervalMs = fallbackTimer->getIntervalMs();
    }

    z0 cancelTimer()
    {
        if (timerId != 0)
            timeKillEvent (timerId);
        else if (fallbackTimer != nullptr)
            fallbackTimer->cancelTimer();
        else
            jassertfalse;

        timerId = 0;
        intervalMs = 0;
    }

    i32 getIntervalMs() const { return intervalMs; }

private:
    PlatformTimerListener& listener;
    UINT timerId { 0 };
    i32 intervalMs { 0 };
    std::unique_ptr<GenericPlatformTimer> fallbackTimer;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTimer)
    DRX_DECLARE_NON_MOVEABLE (PlatformTimer)
};

} // namespace drx
