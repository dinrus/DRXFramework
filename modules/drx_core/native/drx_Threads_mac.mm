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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in drx_posix_SharedCode.h!
*/

#if DRX_IOS
 b8 isIOSAppActive = true;
#endif

static auto getNativeQOS (Thread::Priority priority)
{
    switch (priority)
    {
        case Thread::Priority::highest:    return QOS_CLASS_USER_INTERACTIVE;
        case Thread::Priority::high:       return QOS_CLASS_USER_INITIATED;
        case Thread::Priority::low:        return QOS_CLASS_UTILITY;
        case Thread::Priority::background: return QOS_CLASS_BACKGROUND;
        case Thread::Priority::normal:     break;
    }

    return QOS_CLASS_DEFAULT;
}

static auto getDrxPriority (qos_class_t qos)
{
    switch (qos)
    {
        case QOS_CLASS_USER_INTERACTIVE:    return Thread::Priority::highest;
        case QOS_CLASS_USER_INITIATED:      return Thread::Priority::high;
        case QOS_CLASS_UTILITY:             return Thread::Priority::low;
        case QOS_CLASS_BACKGROUND:          return Thread::Priority::background;

        case QOS_CLASS_UNSPECIFIED:
        case QOS_CLASS_DEFAULT:             break;
    }

    return Thread::Priority::normal;
}

template<typename Type>
static std::optional<Type> firstOptionalWithValue (const std::initializer_list<std::optional<Type>>& optionals)
{
    for (const auto& optional : optionals)
        if (optional.has_value())
            return optional;

    return {};
}

static b8 tryToUpgradeCurrentThreadToRealtime (const Thread::RealtimeOptions& options)
{
    const auto periodMs = options.getPeriodMs().value_or (0.0);

    const auto processingTimeMs = firstOptionalWithValue (
    {
        options.getProcessingTimeMs(),
        options.getMaximumProcessingTimeMs(),
        options.getPeriodMs()
    }).value_or (10.0);

    const auto maxProcessingTimeMs = options.getMaximumProcessingTimeMs()
                                            .value_or (processingTimeMs);

    // The processing time can not exceed the maximum processing time!
    jassert (maxProcessingTimeMs >= processingTimeMs);

    thread_time_constraint_policy_data_t policy;
    policy.period = (u32) Time::secondsToHighResolutionTicks (periodMs / 1'000.0);
    policy.computation = (u32) Time::secondsToHighResolutionTicks (processingTimeMs / 1'000.0);
    policy.constraint = (u32) Time::secondsToHighResolutionTicks (maxProcessingTimeMs / 1'000.0);
    policy.preemptible = true;

    const auto result = thread_policy_set (pthread_mach_thread_np (pthread_self()),
                                           THREAD_TIME_CONSTRAINT_POLICY,
                                           (thread_policy_t) &policy,
                                           THREAD_TIME_CONSTRAINT_POLICY_COUNT);

    if (result == KERN_SUCCESS)
        return true;

    // testing has shown that passing a computation value > 50ms can
    // lead to thread_policy_set returning an error indicating that an
    // invalid argument was passed. If that happens this code tries to
    // limit that value in the hope of resolving the issue.

    if (result == KERN_INVALID_ARGUMENT && options.getProcessingTimeMs() > 50.0)
        return tryToUpgradeCurrentThreadToRealtime (options.withProcessingTimeMs (50.0));

    return false;
}

b8 Thread::createNativeThread (Priority priority)
{
    PosixThreadAttribute attribute { threadStackSize };

    pthread_attr_set_qos_class_np (attribute.get(), getNativeQOS (priority), 0);

    struct ThreadData
    {
        Thread& thread;
        std::promise<b8> started{};
    };

    ThreadData threadData { *this, {} };

    threadId = threadHandle = makeThreadHandle (attribute, &threadData, [] (uk userData) -> uk
    {
        auto& data { *static_cast<ThreadData*> (userData) };
        auto& thread = data.thread;

        if (thread.isRealtime()
            && ! tryToUpgradeCurrentThreadToRealtime (*thread.realtimeOptions))
        {
            data.started.set_value (false);
            return nullptr;
        }

        data.started.set_value (true);

        DRX_AUTORELEASEPOOL
        {
            drx_threadEntryPoint (&thread);
        }

        return nullptr;
    });

    return threadId != nullptr
        && threadData.started.get_future().get();
}

z0 Thread::killThread()
{
    if (threadHandle != nullptr)
        pthread_cancel ((pthread_t) threadHandle.load());
}

Thread::Priority Thread::getPriority() const
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    if (! isRealtime())
        return getDrxPriority (qos_class_self());

    return {};
}

b8 Thread::setPriority (Priority priority)
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    return pthread_set_qos_class_self_np (getNativeQOS (priority), 0) == 0;
}

//==============================================================================
DRX_API b8 DRX_CALLTYPE Process::isForegroundProcess()
{
   if (SystemStats::isRunningInAppExtensionSandbox())
       return true;

   #if DRX_MAC
    return [NSApp isActive];
   #else
    return isIOSAppActive;
   #endif
}

DRX_API z0 DRX_CALLTYPE Process::makeForegroundProcess()
{
   #if DRX_MAC
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [NSApp activateIgnoringOtherApps: YES];
   #endif
}

DRX_API z0 DRX_CALLTYPE Process::hide()
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
    {
       #if DRX_MAC
        [NSApp hide: nil];
       #elif DRX_IOS
        [[UIApplication sharedApplication] performSelector: @selector (suspend)];
       #endif
    }
}

DRX_API z0 DRX_CALLTYPE Process::raisePrivilege() {}
DRX_API z0 DRX_CALLTYPE Process::lowerPrivilege() {}

DRX_API z0 DRX_CALLTYPE Process::setPriority (ProcessPriority) {}

//==============================================================================
DRX_API b8 DRX_CALLTYPE drx_isRunningUnderDebugger() noexcept
{
    struct kinfo_proc info;
    i32 m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    size_t sz = sizeof (info);
    sysctl (m, 4, &info, &sz, nullptr, 0);
    return (info.kp_proc.p_flag & P_TRACED) != 0;
}

} // namespace drx
