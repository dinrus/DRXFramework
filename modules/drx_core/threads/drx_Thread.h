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
/**
    Encapsulates a thread.

    Subclasses derive from Thread and implement the run() method, in which they
    do their business. The thread can then be started with the startThread() or
    startRealtimeThread() methods and controlled with various other methods.

    This class also contains some thread-related static methods, such
    as sleep(), yield(), getCurrentThreadId() etc.

    @see CriticalSection, WaitableEvent, Process, ThreadWithProgressWindow,
         MessageManagerLock

    @tags{Core}
*/
class DRX_API  Thread
{
public:
    //==============================================================================
    static constexpr size_t osDefaultStackSize { 0 };

    //==============================================================================
    /** The different runtime priorities of non-realtime threads.

        @see startThread
    */
    enum class Priority
    {
        /** The highest possible priority that isn't a dedicated realtime thread. */
        highest     = 2,

        /** Makes use of performance cores and higher clocks. */
        high        = 1,

        /** The OS default. It will balance out across all cores. */
        normal      = 0,

        /** Uses efficiency cores when possible. */
        low         = -1,

        /** Restricted to efficiency cores on platforms that have them. */
        background  = -2
    };

    //==============================================================================
    /** A selection of options available when creating realtime threads.

        @see startRealtimeThread
    */
    struct RealtimeOptions
    {
        /** A value with a range of 0-10, where 10 is the highest priority.

            Currently only used by Posix platforms.

            @see getPriority
        */
        [[nodiscard]] RealtimeOptions withPriority (i32 newPriority) const
        {
            jassert (isPositiveAndNotGreaterThan (newPriority, 10));
            return withMember (*this, &RealtimeOptions::priority, drx::jlimit (0, 10, newPriority));
        }

        /** Specify the expected amount of processing time required each time the thread wakes up.

            Only used by macOS/iOS.

            @see getProcessingTimeMs, withMaximumProcessingTimeMs, withPeriodMs, withPeriodHz
        */
        [[nodiscard]] RealtimeOptions withProcessingTimeMs (f64 newProcessingTimeMs) const
        {
            jassert (newProcessingTimeMs > 0.0);
            return withMember (*this, &RealtimeOptions::processingTimeMs, newProcessingTimeMs);
        }

        /** Specify the maximum amount of processing time required each time the thread wakes up.

            Only used by macOS/iOS.

            @see getMaximumProcessingTimeMs, withProcessingTimeMs, withPeriodMs, withPeriodHz
        */
        [[nodiscard]] RealtimeOptions withMaximumProcessingTimeMs (f64 newMaximumProcessingTimeMs) const
        {
            jassert (newMaximumProcessingTimeMs > 0.0);
            return withMember (*this, &RealtimeOptions::maximumProcessingTimeMs, newMaximumProcessingTimeMs);
        }

        /** Specify the maximum amount of processing time required each time the thread wakes up.

            This is identical to 'withMaximumProcessingTimeMs' except it calculates the processing time
            from a sample rate and block size. This is useful if you want to run this thread in parallel
            to an audio device thread.

            Only used by macOS/iOS.

            @see withMaximumProcessingTimeMs, AudioWorkgroup, ScopedWorkgroupToken
        */
        [[nodiscard]] RealtimeOptions withApproximateAudioProcessingTime (i32 samplesPerFrame, f64 sampleRate) const
        {
            jassert (samplesPerFrame > 0);
            jassert (sampleRate > 0.0);

            const auto approxFrameTimeMs = (samplesPerFrame / sampleRate) * 1000.0;
            return withMaximumProcessingTimeMs (approxFrameTimeMs);
        }

        /** Specify the approximate amount of time between each thread wake up.

            Alternatively call withPeriodHz().

            Only used by macOS/iOS.

            @see getPeriodMs, withPeriodHz, withProcessingTimeMs, withMaximumProcessingTimeMs,
        */
        [[nodiscard]] RealtimeOptions withPeriodMs (f64 newPeriodMs) const
        {
            jassert (newPeriodMs > 0.0);
            return withMember (*this, &RealtimeOptions::periodMs, newPeriodMs);
        }

        /** Specify the approximate frequency at which the thread will be woken up.

            Alternatively call withPeriodMs().

            Only used by macOS/iOS.

            @see getPeriodHz, withPeriodMs, withProcessingTimeMs, withMaximumProcessingTimeMs,
        */
        [[nodiscard]] RealtimeOptions withPeriodHz (f64 newPeriodHz) const
        {
            jassert (newPeriodHz > 0.0);
            return withPeriodMs (1'000.0 / newPeriodHz);
        }

        /** Returns a value with a range of 0-10, where 10 is the highest priority.

            @see withPriority
        */
        [[nodiscard]] i32 getPriority() const
        {
            return priority;
        }

        /** Returns the expected amount of processing time required each time the thread
            wakes up.

            @see withProcessingTimeMs, getMaximumProcessingTimeMs, getPeriodMs
        */
        [[nodiscard]] std::optional<f64> getProcessingTimeMs() const
        {
            return processingTimeMs;
        }

        /** Returns the maximum amount of processing time required each time the thread
            wakes up.

            @see withMaximumProcessingTimeMs, getProcessingTimeMs, getPeriodMs
        */
        [[nodiscard]] std::optional<f64> getMaximumProcessingTimeMs() const
        {
            return maximumProcessingTimeMs;
        }

        /** Returns the approximate amount of time between each thread wake up, or
            nullopt if there is no inherent periodicity.

            @see withPeriodMs, withPeriodHz, getProcessingTimeMs, getMaximumProcessingTimeMs
        */
        [[nodiscard]] std::optional<f64> getPeriodMs() const
        {
            return periodMs;
        }

    private:
        i32 priority { 5 };
        std::optional<f64> processingTimeMs;
        std::optional<f64> maximumProcessingTimeMs;
        std::optional<f64> periodMs{};
    };

    //==============================================================================
    /**
        Creates a thread.

        When first created, the thread is not running. Use the startThread()
        method to start it.

        @param threadName       The name of the thread which typically appears in
                                debug logs and profiles.
        @param threadStackSize  The size of the stack of the thread. If this value
                                is zero then the default stack size of the OS will
                                be used.
    */
    explicit Thread (const Txt& threadName, size_t threadStackSize = osDefaultStackSize);

    /** Destructor.

        You must never attempt to delete a Thread object while it's still running -
        always call stopThread() and make sure your thread has stopped before deleting
        the object. Failing to do so will throw an assertion, and put you firmly into
        undefined behaviour territory.
    */
    virtual ~Thread();

    //==============================================================================
    /** Must be implemented to perform the thread's actual code.

        Remember that the thread must regularly check the threadShouldExit()
        method whilst running, and if this returns true it should return from
        the run() method as soon as possible to avoid being forcibly killed.

        @see threadShouldExit, startThread
    */
    virtual z0 run() = 0;

    //==============================================================================
    /** Attempts to start a new thread with default ('Priority::normal') priority.

        This will cause the thread's run() method to be called by a new thread.
        If this thread is already running, startThread() won't do anything.

        If a thread cannot be created with the requested priority, this will return false
        and Thread::run() will not be called. An exception to this is the Android platform,
        which always starts a thread and attempts to upgrade the thread after creation.

        @returns    true if the thread started successfully. false if it was unsuccessful.

        @see stopThread
    */
    b8 startThread();

    /** Attempts to start a new thread with a given priority.

        This will cause the thread's run() method to be called by a new thread.
        If this thread is already running, startThread() won't do anything.

        If a thread cannot be created with the requested priority, this will return false
        and Thread::run() will not be called. An exception to this is the Android platform,
        which always starts a thread and attempts to upgrade the thread after creation.

        @param newPriority    Priority the thread should be assigned. This parameter is ignored
                              on Linux.

        @returns    true if the thread started successfully, false if it was unsuccesful.

        @see startThread, setPriority, startRealtimeThread
    */
    b8 startThread (Priority newPriority);

    /** Starts the thread with realtime performance characteristics on platforms
        that support it.

        You cannot change the options of a running realtime thread, nor switch
        a non-realtime thread to a realtime thread. To make these changes you must
        first stop the thread and then restart with different options.

        @param options    Realtime options the thread should be created with.

        @see startThread, RealtimeOptions
    */
    b8 startRealtimeThread (const RealtimeOptions& options);

    /** Attempts to stop the thread running.

        This method will cause the threadShouldExit() method to return true
        and call notify() in case the thread is currently waiting.

        Hopefully the thread will then respond to this by exiting cleanly, and
        the stopThread method will wait for a given time-period for this to
        happen.

        If the thread is stuck and fails to respond after the timeout, it gets
        forcibly killed, which is a very bad thing to happen, as it could still
        be holding locks, etc. which are needed by other parts of your program.

        @param timeOutMilliseconds  The number of milliseconds to wait for the
                                    thread to finish before killing it by force. A negative
                                    value in here will wait forever.
        @returns    true if the thread was cleanly stopped before the timeout, or false
                    if it had to be killed by force.
        @see signalThreadShouldExit, threadShouldExit, waitForThreadToExit, isThreadRunning
    */
    b8 stopThread (i32 timeOutMilliseconds);

    //==============================================================================
    /** Invokes a lambda or function on its own thread with the default priority.

        This will spin up a Thread object which calls the function and then exits.
        Bear in mind that starting and stopping a thread can be a fairly heavyweight
        operation, so you might prefer to use a ThreadPool if you're kicking off a lot
        of short background tasks.

        Also note that using an anonymous thread makes it very difficult to interrupt
        the function when you need to stop it, e.g. when your app quits. So it's up to
        you to deal with situations where the function may fail to stop in time.

        @param functionToRun  The lambda to be called from the new Thread.

        @returns    true if the thread started successfully, or false if it failed.

        @see launch.
    */
    static b8 launch (std::function<z0()> functionToRun);

    //==============================================================================
    /** Invokes a lambda or function on its own thread with a custom priority.

        This will spin up a Thread object which calls the function and then exits.
        Bear in mind that starting and stopping a thread can be a fairly heavyweight
        operation, so you might prefer to use a ThreadPool if you're kicking off a lot
        of short background tasks.

        Also note that using an anonymous thread makes it very difficult to interrupt
        the function when you need to stop it, e.g. when your app quits. So it's up to
        you to deal with situations where the function may fail to stop in time.

        @param priority       The priority the thread is started with.
        @param functionToRun  The lambda to be called from the new Thread.

        @returns    true if the thread started successfully, or false if it failed.
    */
    static b8 launch (Priority priority, std::function<z0()> functionToRun);

    //==============================================================================
    /** Возвращает true, если the thread is currently active */
    b8 isThreadRunning() const;

    /** Sets a flag to tell the thread it should stop.

        Calling this means that the threadShouldExit() method will then return true.
        The thread should be regularly checking this to see whether it should exit.

        If your thread makes use of wait(), you might want to call notify() after calling
        this method, to interrupt any waits that might be in progress, and allow it
        to reach a point where it can exit.

        @see threadShouldExit, waitForThreadToExit
    */
    z0 signalThreadShouldExit();

    /** Checks whether the thread has been told to stop running.

        Threads need to check this regularly, and if it returns true, they should
        return from their run() method at the first possible opportunity.

        @see signalThreadShouldExit, currentThreadShouldExit
    */
    b8 threadShouldExit() const;

    /** Checks whether the current thread has been told to stop running.
        On the message thread, this will always return false, otherwise
        it will return threadShouldExit() called on the current thread.

        @see threadShouldExit
    */
    static b8 currentThreadShouldExit();

    /** Waits for the thread to stop.
        This will wait until isThreadRunning() is false or until a timeout expires.

        @param timeOutMilliseconds  the time to wait, in milliseconds. If this value
                                    is less than zero, it will wait forever.
        @returns    true if the thread exits, or false if the timeout expires first.
    */
    b8 waitForThreadToExit (i32 timeOutMilliseconds) const;

    //==============================================================================
    /** Used to receive callbacks for thread exit calls */
    class DRX_API Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called if Thread::signalThreadShouldExit was called.
            @see Thread::threadShouldExit, Thread::addListener, Thread::removeListener
        */
        virtual z0 exitSignalSent() = 0;
    };

    /** Add a listener to this thread which will receive a callback when
        signalThreadShouldExit was called on this thread.

        @see signalThreadShouldExit, removeListener
    */
    z0 addListener (Listener*);

    /** Removes a listener added with addListener. */
    z0 removeListener (Listener*);

    /** Возвращает true, если this Thread represents a realtime thread. */
    b8 isRealtime() const;

    //==============================================================================
    /** Sets the affinity mask for the thread.

        This will only have an effect next time the thread is started - i.e. if the
        thread is already running when called, it'll have no effect.

        @see setCurrentThreadAffinityMask
    */
    z0 setAffinityMask (u32 affinityMask);

    /** Changes the affinity mask for the caller thread.

        This will change the affinity mask for the thread that calls this static method.

        @see setAffinityMask
    */
    static z0 DRX_CALLTYPE setCurrentThreadAffinityMask (u32 affinityMask);

    //==============================================================================
    /** Suspends the execution of the current thread until the specified timeout period
        has elapsed (note that this may not be exact).

        The timeout period must not be negative and whilst sleeping the thread cannot
        be woken up so it should only be used for short periods of time and when other
        methods such as using a WaitableEvent or CriticalSection are not possible.
    */
    static z0 DRX_CALLTYPE sleep (i32 milliseconds);

    /** Yields the current thread's CPU time-slot and allows a new thread to run.

        If there are no other threads of equal or higher priority currently running then
        this will return immediately and the current thread will continue to run.
    */
    static z0 DRX_CALLTYPE yield();

    //==============================================================================
    /** Suspends the execution of this thread until either the specified timeout period
        has elapsed, or another thread calls the notify() method to wake it up.

        A negative timeout value means that the method will wait indefinitely.

        @returns    true if the event has been signalled, false if the timeout expires.
    */
    b8 wait (f64 timeOutMilliseconds) const;

    /** Wakes up the thread.

        If the thread has called the wait() method, this will wake it up.

        @see wait
    */
    z0 notify() const;

    //==============================================================================
    /** A value type used for thread IDs.

        @see getCurrentThreadId(), getThreadId()
    */
    using ThreadID = uk;

    /** Returns an id that identifies the caller thread.

        To find the ID of a particular thread object, use getThreadId().

        @returns    a unique identifier that identifies the calling thread.
        @see getThreadId
    */
    static ThreadID DRX_CALLTYPE getCurrentThreadId();

    /** Finds the thread object that is currently running.

        Note that the main UI thread (or other non-DRX threads) don't have a Thread
        object associated with them, so this will return nullptr.
    */
    static Thread* DRX_CALLTYPE getCurrentThread();

    /** Returns the ID of this thread.

        That means the ID of this thread object - not of the thread that's calling the method.
        This can change when the thread is started and stopped, and will be invalid if the
        thread's not actually running.

        @see getCurrentThreadId
    */
    ThreadID getThreadId() const noexcept;

    /** Returns the name of the thread. This is the name that gets set in the constructor. */
    const Txt& getThreadName() const noexcept                    { return threadName; }

    /** Changes the name of the caller thread.

        Different OSes may place different length or content limits on this name.
    */
    static z0 DRX_CALLTYPE setCurrentThreadName (const Txt& newThreadName);

   #if DRX_ANDROID || DOXYGEN
    //==============================================================================
    /** Initialises the DRX subsystem for projects not created by the Projucer

        On Android, DRX needs to be initialised once before it is used. The Projucer
        will automatically generate the necessary java code to do this. However, if
        you are using DRX without the Projucer or are creating a library made with
        DRX intended for use in non-DRX apks, then you must call this method
        manually once on apk startup.

        You can call this method from C++ or directly from java by calling the
        following java method:

        @code
        com.rmsl.drx.Java.initialiseDRX (myContext);
        @endcode

        Note that the above java method is only available in Android Studio projects
        created by the Projucer. If you need to call this from another type of project
        then you need to add the following java file to
        your project:

        @code
        package com.rmsl.drx;

        public class Java
        {
            static { System.loadLibrary ("drx_jni"); }
            public native static z0 initialiseDRX (Context context);
        }
        @endcode

        @param jniEnv   this is a pointer to JNI's JNIEnv variable. Any callback
                        from Java into C++ will have this passed in as it's first
                        parameter.
        @param jContext this is a jobject referring to your app/service/receiver/
                        provider's Context. DRX needs this for many of it's internal
                        functions.
    */
    static z0 initialiseDRX (uk jniEnv, uk jContext);
   #endif

protected:
    //==============================================================================
    /** Returns the current priority of this thread.

        This can only be called from the target thread. Doing so from another thread
        will cause an assert.

        @see setPriority
    */
    Priority getPriority() const;

    /** Attempts to set the priority for this thread. Возвращает true, если the new priority
        was set successfully, false if not.

        This can only be called from the target thread. Doing so from another thread
        will cause an assert.

        @param newPriority The new priority to be applied to the thread. Note: This
                           has no effect on Linux platforms, subsequent calls to
                           'getPriority' will return this value.

        @see Priority
    */
    b8 setPriority (Priority newPriority);

private:
    //==============================================================================
    const Txt threadName;
    std::atomic<uk> threadHandle { nullptr };
    std::atomic<ThreadID> threadId { nullptr };
    std::optional<RealtimeOptions> realtimeOptions = {};
    CriticalSection startStopLock;
    WaitableEvent startSuspensionEvent, defaultEvent;
    size_t threadStackSize;
    u32 affinityMask = 0;
    b8 deleteOnThreadEnd = false;
    std::atomic<b8> shouldExit { false };
    ThreadSafeListenerList<Listener> listeners;

   #if DRX_ANDROID || DRX_LINUX || DRX_BSD
    std::atomic<Priority> priority;
   #endif

   #ifndef DOXYGEN
    friend z0 DRX_API drx_threadEntryPoint (uk);
   #endif

    b8 startThreadInternal (Priority);
    b8 createNativeThread (Priority);
    z0 closeThreadHandle();
    z0 killThread();
    z0 threadEntryPoint();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Thread)
};

} // namespace drx
