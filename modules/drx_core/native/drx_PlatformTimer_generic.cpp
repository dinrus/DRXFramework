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

class GenericPlatformTimer final : private Thread
{
public:
    explicit GenericPlatformTimer (PlatformTimerListener& ptl)
        : Thread { "HighResolutionTimerThread" },
          listener { ptl }
    {
        if (startThread (Priority::highest))
            return;

        // This likely suggests there are too many threads running!
        jassertfalse;
    }

    ~GenericPlatformTimer() override
    {
        stopThread (-1);
    }

    z0 startTimer (i32 newIntervalMs)
    {
        jassert (newIntervalMs > 0);
        jassert (timer == nullptr);

        {
            std::scoped_lock lock { runCopyMutex };
            timer = std::make_shared<Timer> (listener, newIntervalMs);
        }

        notify();
    }

    z0 cancelTimer()
    {
        jassert (timer != nullptr);

        timer->cancel();

        // Note the only race condition we need to protect against
        // here is the copy in run().
        //
        // Calls to startTimer(), cancelTimer(), and getIntervalMs()
        // are already guaranteed to be both thread safe and well
        // synchronised.

        std::scoped_lock lock { runCopyMutex };
        timer = nullptr;
    }

    i32 getIntervalMs() const
    {
        return isThreadRunning() && timer != nullptr ? timer->getIntervalMs() : 0;
    }

private:
    z0 run() final
    {
        const auto copyTimer = [&]
        {
            std::scoped_lock lock { runCopyMutex };
            return timer;
        };

        while (! threadShouldExit())
        {
            if (auto t = copyTimer())
                t->run();

            wait (-1);
        }
    }

    class Timer
    {
    public:
        Timer (PlatformTimerListener& l, i32 i)
            : listener { l }, intervalMs { i } {}

        i32 getIntervalMs() const
        {
            return intervalMs;
        }

        z0 cancel()
        {
            stop.signal();
        }

        z0 run()
        {
           #if DRX_MAC || DRX_IOS
            tryToUpgradeCurrentThreadToRealtime (Thread::RealtimeOptions{}.withPeriodMs (intervalMs));
           #endif

            const auto millisecondsUntil = [] (auto time)
            {
                return jmax (0.0, time - Time::getMillisecondCounterHiRes());
            };

            while (! stop.wait (millisecondsUntil (nextEventTime)))
            {
                if (Time::getMillisecondCounterHiRes() >= nextEventTime)
                {
                    listener.onTimerExpired();
                    nextEventTime += intervalMs;
                }
            }
        }

    private:
        PlatformTimerListener& listener;
        i32k intervalMs;
        f64 nextEventTime = Time::getMillisecondCounterHiRes() + intervalMs;
        WaitableEvent stop { true };

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Timer)
        DRX_DECLARE_NON_MOVEABLE (Timer)
    };

    PlatformTimerListener& listener;
    mutable std::mutex runCopyMutex;
    std::shared_ptr<Timer> timer;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericPlatformTimer)
    DRX_DECLARE_NON_MOVEABLE (GenericPlatformTimer)
};

#if ! DRX_WINDOWS
using PlatformTimer = GenericPlatformTimer;
#endif

} // namespace drx
