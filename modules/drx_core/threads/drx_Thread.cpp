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
//==============================================================================
Thread::Thread (const Txt& name, size_t stackSize) : threadName (name),
                                                        threadStackSize (stackSize)
{
}

Thread::~Thread()
{
    if (deleteOnThreadEnd)
        return;

    /* If your thread class's destructor has been called without first stopping the thread, that
       means that this partially destructed object is still performing some work - and that's
       probably a Bad Thing!

       To avoid this type of nastiness, always make sure you call stopThread() before or during
       your subclass's destructor.
    */
    jassert (! isThreadRunning());

    stopThread (-1);
}

//==============================================================================
// Use a ref-counted object to hold this shared data, so that it can outlive its static
// shared pointer when threads are still running during static shutdown.
struct CurrentThreadHolder final : public ReferenceCountedObject
{
    CurrentThreadHolder() noexcept {}

    using Ptr = ReferenceCountedObjectPtr<CurrentThreadHolder>;
    ThreadLocalValue<Thread*> value;

    DRX_DECLARE_NON_COPYABLE (CurrentThreadHolder)
};

static t8 currentThreadHolderLock [sizeof (SpinLock)]; // (statically initialised to zeros).

static SpinLock* castToSpinLockWithoutAliasingWarning (uk s)
{
    return static_cast<SpinLock*> (s);
}

static CurrentThreadHolder::Ptr getCurrentThreadHolder()
{
    static CurrentThreadHolder::Ptr currentThreadHolder;
    SpinLock::ScopedLockType lock (*castToSpinLockWithoutAliasingWarning (currentThreadHolderLock));

    if (currentThreadHolder == nullptr)
        currentThreadHolder = new CurrentThreadHolder();

    return currentThreadHolder;
}

z0 Thread::threadEntryPoint()
{
    const CurrentThreadHolder::Ptr currentThreadHolder (getCurrentThreadHolder());
    currentThreadHolder->value = this;

    if (threadName.isNotEmpty())
        setCurrentThreadName (threadName);

    // This 'startSuspensionEvent' protects 'threadId' which is initialised after the platform's native 'CreateThread' method.
    // This ensures it has been initialised correctly before it reaches this point.
    if (startSuspensionEvent.wait (10000))
    {
        jassert (getCurrentThreadId() == threadId);

        if (affinityMask != 0)
            setCurrentThreadAffinityMask (affinityMask);

        try
        {
            run();
        }
        catch (...)
        {
            jassertfalse; // Your run() method mustn't throw any exceptions!
        }
    }

    currentThreadHolder->value.releaseCurrentThreadStorage();

    // Once closeThreadHandle is called this class may be deleted by a different
    // thread, so we need to store deleteOnThreadEnd in a local variable.
    auto shouldDeleteThis = deleteOnThreadEnd;
    closeThreadHandle();

    if (shouldDeleteThis)
        delete this;
}

// used to wrap the incoming call from the platform-specific code
z0 DRX_API drx_threadEntryPoint (uk userData)
{
    static_cast<Thread*> (userData)->threadEntryPoint();
}

//==============================================================================
b8 Thread::startThreadInternal (Priority threadPriority)
{
    shouldExit = false;

    // 'priority' is essentially useless on Linux as only realtime
    // has any options but we need to set this here to satisfy
    // later queries, otherwise we get inconsistent results across
    // platforms.
   #if DRX_ANDROID || DRX_LINUX || DRX_BSD
    priority = threadPriority;
   #endif

    if (createNativeThread (threadPriority))
    {
        startSuspensionEvent.signal();
        return true;
    }

    return false;
}

b8 Thread::startThread()
{
    return startThread (Priority::normal);
}

b8 Thread::startThread (Priority threadPriority)
{
    const ScopedLock sl (startStopLock);

    if (threadHandle == nullptr)
    {
        realtimeOptions.reset();
        return startThreadInternal (threadPriority);
    }

    return false;
}

b8 Thread::startRealtimeThread (const RealtimeOptions& options)
{
    const ScopedLock sl (startStopLock);

    if (threadHandle == nullptr)
    {
        realtimeOptions = std::make_optional (options);

        if (startThreadInternal (Priority::normal))
            return true;

        realtimeOptions.reset();
    }

    return false;
}

b8 Thread::isThreadRunning() const
{
    return threadHandle != nullptr;
}

Thread* DRX_CALLTYPE Thread::getCurrentThread()
{
    return getCurrentThreadHolder()->value.get();
}

Thread::ThreadID Thread::getThreadId() const noexcept
{
    return threadId;
}

//==============================================================================
z0 Thread::signalThreadShouldExit()
{
    shouldExit = true;
    listeners.call ([] (Listener& l) { l.exitSignalSent(); });
}

b8 Thread::threadShouldExit() const
{
    return shouldExit;
}

b8 Thread::currentThreadShouldExit()
{
    if (auto* currentThread = getCurrentThread())
        return currentThread->threadShouldExit();

    return false;
}

b8 Thread::waitForThreadToExit (i32k timeOutMilliseconds) const
{
    // Doh! So how exactly do you expect this thread to wait for itself to stop??
    jassert (getThreadId() != getCurrentThreadId() || getCurrentThreadId() == ThreadID());

    auto timeoutEnd = Time::getMillisecondCounter() + (u32) timeOutMilliseconds;

    while (isThreadRunning())
    {
        if (timeOutMilliseconds >= 0 && Time::getMillisecondCounter() > timeoutEnd)
            return false;

        sleep (2);
    }

    return true;
}

b8 Thread::stopThread (i32k timeOutMilliseconds)
{
    // agh! You can't stop the thread that's calling this method! How on earth
    // would that work??
    jassert (getCurrentThreadId() != getThreadId());

    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        notify();

        if (timeOutMilliseconds != 0)
            waitForThreadToExit (timeOutMilliseconds);

        if (isThreadRunning())
        {
            // very bad karma if this point is reached, as there are bound to be
            // locks and events left in silly states when a thread is killed by force..
            jassertfalse;
            Logger::writeToLog ("!! killing thread by force !!");

            killThread();

            threadHandle = nullptr;
            threadId = {};
            return false;
        }
    }

    return true;
}

z0 Thread::addListener (Listener* listener)
{
    listeners.add (listener);
}

z0 Thread::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

b8 Thread::isRealtime() const
{
    return realtimeOptions.has_value();
}

z0 Thread::setAffinityMask (u32k newAffinityMask)
{
    affinityMask = newAffinityMask;
}

//==============================================================================
b8 Thread::wait (f64 timeOutMilliseconds) const
{
    return defaultEvent.wait (timeOutMilliseconds);
}

z0 Thread::notify() const
{
    defaultEvent.signal();
}

//==============================================================================
struct LambdaThread final : public Thread
{
    LambdaThread (std::function<z0()>&& f) : Thread (SystemStats::getDRXVersion() + ": anonymous"), fn (std::move (f)) {}

    z0 run() override
    {
        fn();
        fn = nullptr; // free any objects that the lambda might contain while the thread is still active
    }

    std::function<z0()> fn;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LambdaThread)
};

b8 Thread::launch (std::function<z0()> functionToRun)
{
    return launch (Priority::normal, std::move (functionToRun));
}

b8 Thread::launch (Priority priority, std::function<z0()> functionToRun)
{
    auto anon = std::make_unique<LambdaThread> (std::move (functionToRun));
    anon->deleteOnThreadEnd = true;

    if (anon->startThread (priority))
    {
        anon.release();
        return true;
    }

    return false;
}

//==============================================================================
z0 SpinLock::enter() const noexcept
{
    if (! tryEnter())
    {
        for (i32 i = 20; --i >= 0;)
            if (tryEnter())
                return;

        while (! tryEnter())
            Thread::yield();
    }
}

//==============================================================================
b8 DRX_CALLTYPE Process::isRunningUnderDebugger() noexcept
{
    return drx_isRunningUnderDebugger();
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class AtomicTests final : public UnitTest
{
public:
    AtomicTests()
        : UnitTest ("Atomics", UnitTestCategories::threads)
    {}

    z0 runTest() override
    {
        beginTest ("Misc");

        t8 a1[7];
        expect (numElementsInArray (a1) == 7);
        i32 a2[3];
        expect (numElementsInArray (a2) == 3);

        expect (ByteOrder::swap ((u16) 0x1122) == 0x2211);
        expect (ByteOrder::swap ((u32) 0x11223344) == 0x44332211);
        expect (ByteOrder::swap ((zu64) 0x1122334455667788ULL) == (zu64) 0x8877665544332211LL);

        beginTest ("Atomic i32");
        AtomicTester <i32>::testInteger (*this);
        beginTest ("Atomic u32");
        AtomicTester <u32>::testInteger (*this);
        beginTest ("Atomic i32");
        AtomicTester <i32>::testInteger (*this);
        beginTest ("Atomic u32");
        AtomicTester <u32>::testInteger (*this);
        beginTest ("Atomic i64");
        AtomicTester <i64>::testInteger (*this);
        beginTest ("Atomic i32*");
        AtomicTester <i32*>::testInteger (*this);
        beginTest ("Atomic f32");
        AtomicTester <f32>::testFloat (*this);
      #if ! DRX_64BIT_ATOMICS_UNAVAILABLE  // 64-bit intrinsics aren't available on some old platforms
        beginTest ("Atomic z64");
        AtomicTester <z64>::testInteger (*this);
        beginTest ("Atomic zu64");
        AtomicTester <zu64>::testInteger (*this);
        beginTest ("Atomic f64");
        AtomicTester <f64>::testFloat (*this);
      #endif
        beginTest ("Atomic pointer increment/decrement");
        Atomic<i32*> a (a2); i32* b (a2);
        expect (++a == ++b);

        {
            beginTest ("Atomic uk");
            Atomic<uk> atomic;
            uk c;

            atomic.set ((uk) 10);
            c = (uk) 10;

            expect (atomic.value == c);
            expect (atomic.get() == c);
        }
    }

    template <typename Type>
    class AtomicTester
    {
    public:
        AtomicTester() = default;

        static z0 testInteger (UnitTest& test)
        {
            Atomic<Type> a, b;
            Type c;

            a.set ((Type) 10);
            c = (Type) 10;

            test.expect (a.value == c);
            test.expect (a.get() == c);

            a += 15;
            c += 15;
            test.expect (a.get() == c);
            a.memoryBarrier();

            a -= 5;
            c -= 5;
            test.expect (a.get() == c);

            test.expect (++a == ++c);
            ++a;
            ++c;
            test.expect (--a == --c);
            test.expect (a.get() == c);
            a.memoryBarrier();

            testFloat (test);
        }



        static z0 testFloat (UnitTest& test)
        {
            Atomic<Type> a, b;
            a = (Type) 101;
            a.memoryBarrier();

            /*  These are some simple test cases to check the atomics - let me know
                if any of these assertions fail on your system!
            */
            test.expect (exactlyEqual (a.get(), (Type) 101));
            test.expect (! a.compareAndSetBool ((Type) 300, (Type) 200));
            test.expect (exactlyEqual (a.get(), (Type) 101));
            test.expect (a.compareAndSetBool ((Type) 200, a.get()));
            test.expect (exactlyEqual (a.get(), (Type) 200));

            test.expect (exactlyEqual (a.exchange ((Type) 300), (Type) 200));
            test.expect (exactlyEqual (a.get(), (Type) 300));

            b = a;
            test.expect (exactlyEqual (b.get(), a.get()));
        }
    };
};

static AtomicTests atomicUnitTests;

//==============================================================================
class ThreadLocalValueUnitTest final : public UnitTest,
                                       private Thread
{
public:
    ThreadLocalValueUnitTest()
        : UnitTest ("ThreadLocalValue", UnitTestCategories::threads),
          Thread (SystemStats::getDRXVersion() + ": ThreadLocalValue Thread")
    {}

    z0 runTest() override
    {
        beginTest ("values are thread local");

        {
            ThreadLocalValue<i32> threadLocal;

            sharedThreadLocal = &threadLocal;

            sharedThreadLocal.get()->get() = 1;

            startThread();
            signalThreadShouldExit();
            waitForThreadToExit (-1);

            mainThreadResult = sharedThreadLocal.get()->get();

            expectEquals (mainThreadResult.get(), 1);
            expectEquals (auxThreadResult.get(), 2);
        }

        beginTest ("values are per-instance");

        {
            ThreadLocalValue<i32> a, b;

            a.get() = 1;
            b.get() = 2;

            expectEquals (a.get(), 1);
            expectEquals (b.get(), 2);
        }
    }

private:
    Atomic<i32> mainThreadResult, auxThreadResult;
    Atomic<ThreadLocalValue<i32>*> sharedThreadLocal;

    z0 run() override
    {
        sharedThreadLocal.get()->get() = 2;
        auxThreadResult = sharedThreadLocal.get()->get();
    }
};

ThreadLocalValueUnitTest threadLocalValueUnitTest;

#endif

} // namespace drx
