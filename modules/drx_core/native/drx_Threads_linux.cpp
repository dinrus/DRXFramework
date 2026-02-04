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

b8 Thread::createNativeThread (Priority)
{
    PosixThreadAttribute attr { threadStackSize };
    PosixSchedulerPriority::getNativeSchedulerAndPriority (realtimeOptions, {}).apply (attr);

    threadId = threadHandle = makeThreadHandle (attr, this, [] (uk userData) -> uk
    {
        auto* myself = static_cast<Thread*> (userData);

        drx_threadEntryPoint (myself);

        return nullptr;
    });

    return threadId != nullptr;
}

z0 Thread::killThread()
{
    if (threadHandle != nullptr)
        pthread_cancel ((pthread_t) threadHandle.load());
}

// Until we implement Nice awareness, these don't do anything on Linux.
Thread::Priority Thread::getPriority() const
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    return priority;
}

b8 Thread::setPriority (Priority newPriority)
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    // Return true to make it compatible with other platforms.
    priority = newPriority;
    return true;
}

//==============================================================================
DRX_API z0 DRX_CALLTYPE Process::setPriority (ProcessPriority) {}

static b8 swapUserAndEffectiveUser()
{
    auto result1 = setreuid (geteuid(), getuid());
    auto result2 = setregid (getegid(), getgid());
    return result1 == 0 && result2 == 0;
}

DRX_API z0 DRX_CALLTYPE Process::raisePrivilege()  { if (geteuid() != 0 && getuid() == 0) swapUserAndEffectiveUser(); }
DRX_API z0 DRX_CALLTYPE Process::lowerPrivilege()  { if (geteuid() == 0 && getuid() != 0) swapUserAndEffectiveUser(); }

} // namespace drx
