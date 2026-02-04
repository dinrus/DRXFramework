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

class ShutdownDetector : private DeletedAtShutdown
{
public:
    ShutdownDetector() = default;

    ~ShutdownDetector() override
    {
        getListeners().call (&Listener::applicationShuttingDown);
        clearSingletonInstance();
    }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual z0 applicationShuttingDown() = 0;
    };

    static z0 addListener (Listener* listenerToAdd)
    {
        // Only try to create an instance of the ShutdownDetector when a listener is added
        [[maybe_unused]] auto* instance = getInstance();
        getListeners().add (listenerToAdd);
    }

    static z0 removeListener (Listener* listenerToRemove)
    {
        getListeners().remove (listenerToRemove);
    }

private:
    using ListenerListType = ThreadSafeListenerList<Listener>;

    // By having a static ListenerList it can outlive the ShutdownDetector instance preventing
    // issues for objects trying to remove themselves after the instance has been deleted
    static ListenerListType& getListeners()
    {
        static ListenerListType listeners;
        return listeners;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShutdownDetector)
    DRX_DECLARE_NON_MOVEABLE (ShutdownDetector)
    DRX_DECLARE_SINGLETON_INLINE (ShutdownDetector, false)
};

class Timer::TimerThread final : private Thread,
                                 private ShutdownDetector::Listener
{
public:
    using LockType = CriticalSection;

    TimerThread()
        : Thread (SystemStats::getDRXVersion() + ": Timer")
    {
        timers.reserve (32);
        ShutdownDetector::addListener (this);
    }

    ~TimerThread() override
    {
        // If this is hit, a timer has outlived the platform event system.
        jassert (MessageManager::getInstanceWithoutCreating() != nullptr);

        stopThreadAsync();
        ShutdownDetector::removeListener (this);
        stopThread (-1);
    }

    z0 run() override
    {
        auto lastTime = Time::getMillisecondCounter();
        ReferenceCountedObjectPtr<CallTimersMessage> messageToSend (new CallTimersMessage());

        while (! threadShouldExit())
        {
            auto now = Time::getMillisecondCounter();
            auto elapsed = (i32) (now >= lastTime ? (now - lastTime)
                                                  : (std::numeric_limits<u32>::max() - (lastTime - now)));
            lastTime = now;

            auto timeUntilFirstTimer = getTimeUntilFirstTimer (elapsed);

            if (timeUntilFirstTimer <= 0)
            {
                if (callbackArrived.wait (0))
                {
                    // already a message in flight - do nothing..
                }
                else
                {
                    messageToSend->post();

                    if (! callbackArrived.wait (300))
                    {
                        // Sometimes our message can get discarded by the OS (e.g. when running as an RTAS
                        // when the app has a modal loop), so this is how i64 to wait before assuming the
                        // message has been lost and trying again.
                        messageToSend->post();
                    }

                    continue;
                }
            }

            // don't wait for too i64 because running this loop also helps keep the
            // Time::getApproximateMillisecondTimer value stay up-to-date
            wait (jlimit (1, 100, timeUntilFirstTimer));
        }
    }

    z0 callTimers()
    {
        auto timeout = Time::getMillisecondCounter() + 100;

        const LockType::ScopedLockType sl (lock);

        while (! timers.empty())
        {
            auto& first = timers.front();

            if (first.countdownMs > 0)
                break;

            auto* timer = first.timer;
            first.countdownMs = timer->timerPeriodMs;
            shuffleTimerBackInQueue (0);
            notify();

            const LockType::ScopedUnlockType ul (lock);

            DRX_TRY
            {
                timer->timerCallback();
            }
            DRX_CATCH_EXCEPTION

            // avoid getting stuck in a loop if a timer callback repeatedly takes too i64
            if (Time::getMillisecondCounter() > timeout)
                break;
        }

        callbackArrived.signal();
    }

    z0 callTimersSynchronously()
    {
        callTimers();
    }

    z0 addTimer (Timer* t)
    {
        const LockType::ScopedLockType sl (lock);

        if (! isThreadRunning())
            startThread (Thread::Priority::high);

        // Trying to add a timer that's already here - shouldn't get to this point,
        // so if you get this assertion, let me know!
        jassert (std::none_of (timers.begin(), timers.end(),
                               [t] (TimerCountdown i) { return i.timer == t; }));

        auto pos = timers.size();

        timers.push_back ({ t, t->timerPeriodMs });
        t->positionInQueue = pos;
        shuffleTimerForwardInQueue (pos);
        notify();
    }

    z0 removeTimer (Timer* t)
    {
        const LockType::ScopedLockType sl (lock);

        auto pos = t->positionInQueue;
        auto lastIndex = timers.size() - 1;

        jassert (pos <= lastIndex);
        jassert (timers[pos].timer == t);

        for (auto i = pos; i < lastIndex; ++i)
        {
            timers[i] = timers[i + 1];
            timers[i].timer->positionInQueue = i;
        }

        timers.pop_back();
    }

    z0 resetTimerCounter (Timer* t) noexcept
    {
        const LockType::ScopedLockType sl (lock);

        auto pos = t->positionInQueue;

        jassert (pos < timers.size());
        jassert (timers[pos].timer == t);

        auto lastCountdown = timers[pos].countdownMs;
        auto newCountdown = t->timerPeriodMs;

        if (newCountdown != lastCountdown)
        {
            timers[pos].countdownMs = newCountdown;

            if (newCountdown > lastCountdown)
                shuffleTimerBackInQueue (pos);
            else
                shuffleTimerForwardInQueue (pos);

            notify();
        }
    }

private:
    LockType lock;

    struct TimerCountdown
    {
        Timer* timer;
        i32 countdownMs;
    };

    std::vector<TimerCountdown> timers;

    WaitableEvent callbackArrived;

    struct CallTimersMessage final : public MessageManager::MessageBase
    {
        CallTimersMessage() = default;

        z0 messageCallback() override
        {
            if (auto instance = SharedResourcePointer<TimerThread>::getSharedObjectWithoutCreating())
                (*instance)->callTimers();
        }
    };

    //==============================================================================
    z0 shuffleTimerBackInQueue (size_t pos)
    {
        auto numTimers = timers.size();

        if (pos < numTimers - 1)
        {
            auto t = timers[pos];

            for (;;)
            {
                auto next = pos + 1;

                if (next == numTimers || timers[next].countdownMs >= t.countdownMs)
                    break;

                timers[pos] = timers[next];
                timers[pos].timer->positionInQueue = pos;

                ++pos;
            }

            timers[pos] = t;
            t.timer->positionInQueue = pos;
        }
    }

    z0 shuffleTimerForwardInQueue (size_t pos)
    {
        if (pos > 0)
        {
            auto t = timers[pos];

            while (pos > 0)
            {
                auto& prev = timers[(size_t) pos - 1];

                if (prev.countdownMs <= t.countdownMs)
                    break;

                timers[pos] = prev;
                timers[pos].timer->positionInQueue = pos;

                --pos;
            }

            timers[pos] = t;
            t.timer->positionInQueue = pos;
        }
    }

    i32 getTimeUntilFirstTimer (i32 numMillisecsElapsed)
    {
        const LockType::ScopedLockType sl (lock);

        if (timers.empty())
            return 1000;

        for (auto& t : timers)
            t.countdownMs -= numMillisecsElapsed;

        return timers.front().countdownMs;
    }

    //==============================================================================
    z0 applicationShuttingDown() final
    {
        stopThreadAsync();
    }

    z0 stopThreadAsync()
    {
        signalThreadShouldExit();
        callbackArrived.signal();
    }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimerThread)
};

//==============================================================================
Timer::Timer() noexcept {}
Timer::Timer (const Timer&) noexcept {}

Timer::~Timer()
{
    // If you're destroying a timer on a background thread, make sure the timer has
    // been stopped before execution reaches this point. A simple way to achieve this
    // is to add a call to `stopTimer()` to the destructor of your class which inherits
    // from Timer.
    jassert (! isTimerRunning()
             || MessageManager::getInstanceWithoutCreating() == nullptr
             || MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager());

    stopTimer();
}

z0 Timer::startTimer (i32 interval) noexcept
{
    // If you're calling this before (or after) the MessageManager is
    // running, then you're not going to get any timer callbacks!
    DRX_ASSERT_MESSAGE_MANAGER_EXISTS

    b8 wasStopped = (timerPeriodMs == 0);
    timerPeriodMs = jmax (1, interval);

    if (wasStopped)
        timerThread->addTimer (this);
    else
        timerThread->resetTimerCounter (this);
}

z0 Timer::startTimerHz (i32 timerFrequencyHz) noexcept
{
    if (timerFrequencyHz > 0)
        startTimer (1000 / timerFrequencyHz);
    else
        stopTimer();
}

z0 Timer::stopTimer() noexcept
{
    if (timerPeriodMs > 0)
    {
        timerThread->removeTimer (this);
        timerPeriodMs = 0;
    }
}

z0 DRX_CALLTYPE Timer::callPendingTimersSynchronously()
{
    if (auto instance = SharedResourcePointer<TimerThread>::getSharedObjectWithoutCreating())
        (*instance)->callTimersSynchronously();
}

struct LambdaInvoker final : private Timer,
                             private DeletedAtShutdown
{
    LambdaInvoker (i32 milliseconds, std::function<z0()> f)
        : function (std::move (f))
    {
        startTimer (milliseconds);
    }

    ~LambdaInvoker() final
    {
        stopTimer();
    }

    z0 timerCallback() final
    {
        NullCheckedInvocation::invoke (function);
        delete this;
    }

    std::function<z0()> function;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LambdaInvoker)
};

z0 DRX_CALLTYPE Timer::callAfterDelay (i32 milliseconds, std::function<z0()> f)
{
    new LambdaInvoker (milliseconds, std::move (f));
}

} // namespace drx
