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

MessageManager::MessageManager() noexcept
  : messageThreadId (Thread::getCurrentThreadId())
{
    DRX_VERSION_ID

    if (DRXApplicationBase::isStandaloneApp())
        Thread::setCurrentThreadName (SystemStats::getDRXVersion() + ": Message Thread");
}

MessageManager::~MessageManager() noexcept
{
    broadcaster.reset();

    doPlatformSpecificShutdown();

    jassert (instance == this);
    instance = nullptr;  // do this last in case this instance is still needed by doPlatformSpecificShutdown()
}

MessageManager* MessageManager::instance = nullptr;

MessageManager* MessageManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();
    }

    return instance;
}

MessageManager* MessageManager::getInstanceWithoutCreating() noexcept
{
    return instance;
}

z0 MessageManager::deleteInstance()
{
    deleteAndZero (instance);
}

//==============================================================================
b8 MessageManager::MessageBase::post()
{
    auto* mm = MessageManager::instance;

    if (mm == nullptr || mm->quitMessagePosted.get() != 0 || ! postMessageToSystemQueue (this))
    {
        Ptr deleter (this); // (this will delete messages that were just created with a 0 ref count)
        return false;
    }

    return true;
}

//==============================================================================
#if ! (DRX_MAC || DRX_IOS || DRX_ANDROID)
// implemented in platform-specific code (drx_Messaging_linux.cpp and drx_Messaging_windows.cpp)
namespace detail
{
b8 dispatchNextMessageOnSystemQueue (b8 returnIfNoPendingMessages);
} // namespace detail

class MessageManager::QuitMessage final : public MessageManager::MessageBase
{
public:
    QuitMessage() {}

    z0 messageCallback() override
    {
        if (auto* mm = MessageManager::instance)
            mm->quitMessageReceived = true;
    }

    DRX_DECLARE_NON_COPYABLE (QuitMessage)
};

z0 MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    while (quitMessageReceived.get() == 0)
    {
        DRX_TRY
        {
            if (! detail::dispatchNextMessageOnSystemQueue (false))
                Thread::sleep (1);
        }
        DRX_CATCH_EXCEPTION
    }
}

z0 MessageManager::stopDispatchLoop()
{
    (new QuitMessage())->post();
    quitMessagePosted = true;
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 MessageManager::runDispatchLoopUntil (i32 millisecondsToRunFor)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    auto endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while (quitMessageReceived.get() == 0)
    {
        DRX_TRY
        {
            if (! detail::dispatchNextMessageOnSystemQueue (millisecondsToRunFor >= 0))
                Thread::sleep (1);
        }
        DRX_CATCH_EXCEPTION

        if (millisecondsToRunFor >= 0 && Time::currentTimeMillis() >= endTime)
            break;
    }

    return quitMessageReceived.get() == 0;
}
#endif

#endif

//==============================================================================
uk MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* func, uk parameter)
{
    return callSync ([func, parameter] { return func (parameter); }).value_or (nullptr);
}

//==============================================================================
z0 MessageManager::deliverBroadcastMessage (const Txt& value)
{
    if (broadcaster != nullptr)
        broadcaster->sendActionMessage (value);
}

z0 MessageManager::registerBroadcastListener (ActionListener* const listener)
{
    if (broadcaster == nullptr)
        broadcaster.reset (new ActionBroadcaster());

    broadcaster->addActionListener (listener);
}

z0 MessageManager::deregisterBroadcastListener (ActionListener* const listener)
{
    if (broadcaster != nullptr)
        broadcaster->removeActionListener (listener);
}

//==============================================================================
b8 MessageManager::isThisTheMessageThread() const noexcept
{
    const std::lock_guard<std::mutex> lock { messageThreadIdMutex };

    return Thread::getCurrentThreadId() == messageThreadId;
}

z0 MessageManager::setCurrentThreadAsMessageThread()
{
    auto thisThread = Thread::getCurrentThreadId();

    const std::lock_guard<std::mutex> lock { messageThreadIdMutex };

    if (std::exchange (messageThreadId, thisThread) != thisThread)
    {
       #if DRX_WINDOWS
        // This is needed on windows to make sure the message window is created by this thread
        doPlatformSpecificShutdown();
        doPlatformSpecificInitialisation();
       #endif
    }
}

b8 MessageManager::currentThreadHasLockedMessageManager() const noexcept
{
    auto thisThread = Thread::getCurrentThreadId();
    return thisThread == messageThreadId || thisThread == threadWithLock.get();
}

b8 MessageManager::existsAndIsLockedByCurrentThread() noexcept
{
    if (auto i = getInstanceWithoutCreating())
        return i->currentThreadHasLockedMessageManager();

    return false;
}

b8 MessageManager::existsAndIsCurrentThread() noexcept
{
    if (auto i = getInstanceWithoutCreating())
        return i->isThisTheMessageThread();

    return false;
}

//==============================================================================
//==============================================================================
/*  The only safe way to lock the message thread while another thread does
    some work is by posting a special message, whose purpose is to tie up the event
    loop until the other thread has finished its business.

    Any other approach can get horribly deadlocked if the OS uses its own hidden locks which
    get locked before making an event callback, because if the same OS lock gets indirectly
    accessed from another thread inside a MM lock, you're screwed. (this is exactly what happens
    in Cocoa).
*/
struct MessageManager::Lock::BlockingMessage final : public MessageManager::MessageBase
{
    explicit BlockingMessage (const MessageManager::Lock* parent) noexcept
        : owner (parent) {}

    z0 messageCallback() override
    {
        std::unique_lock lock { mutex };

        if (owner != nullptr)
            owner->setAcquired (true);

        condvar.wait (lock, [&] { return owner == nullptr; });
    }

    z0 stopWaiting()
    {
        const ScopeGuard scope { [&] { condvar.notify_one(); } };
        const std::scoped_lock lock { mutex };
        owner = nullptr;
    }

private:
    std::mutex mutex;
    std::condition_variable condvar;

    const MessageManager::Lock* owner = nullptr;

    DRX_DECLARE_NON_COPYABLE (BlockingMessage)
};

//==============================================================================
MessageManager::Lock::Lock()                            {}
MessageManager::Lock::~Lock()                           { exit(); }
z0 MessageManager::Lock::enter()    const noexcept    {        exclusiveTryAcquire (true); }
b8 MessageManager::Lock::tryEnter() const noexcept    { return exclusiveTryAcquire (false); }

b8 MessageManager::Lock::exclusiveTryAcquire (b8 lockIsMandatory) const noexcept
{
    if (lockIsMandatory)
        entryMutex.enter();
    else if (! entryMutex.tryEnter())
        return false;

    const auto result = tryAcquire (lockIsMandatory);

    if (! result)
        entryMutex.exit();

    return result;
}

b8 MessageManager::Lock::tryAcquire (b8 lockIsMandatory) const noexcept
{
    auto* mm = MessageManager::instance;

    if (mm == nullptr)
    {
        jassertfalse;
        return false;
    }

    if (! lockIsMandatory && [&]
                             {
                                 const std::scoped_lock lock { mutex };
                                 return std::exchange (abortWait, false);
                             }())
    {
        return false;
    }

    if (mm->currentThreadHasLockedMessageManager())
        return true;

    try
    {
        blockingMessage = *new BlockingMessage (this);
    }
    catch (...)
    {
        jassert (! lockIsMandatory);
        return false;
    }

    if (! blockingMessage->post())
    {
        // post of message failed while trying to get the lock
        jassert (! lockIsMandatory);
        blockingMessage = nullptr;
        return false;
    }

    for (;;)
    {
        {
            std::unique_lock lock { mutex };
            condvar.wait (lock, [&] { return std::exchange (abortWait, false); });
        }

        if (acquired)
        {
            mm->threadWithLock = Thread::getCurrentThreadId();
            return true;
        }

        if (! lockIsMandatory)
            break;
    }

    // we didn't get the lock

    blockingMessage->stopWaiting();
    blockingMessage = nullptr;
    return false;
}

z0 MessageManager::Lock::exit() const noexcept
{
    const auto wasAcquired = [&]
    {
        const std::scoped_lock lock { mutex };
        return acquired;
    }();

    if (! wasAcquired)
        return;

    const ScopeGuard unlocker { [&] { entryMutex.exit(); } };

    if (blockingMessage == nullptr)
        return;

    if (auto* mm = MessageManager::instance)
    {
        jassert (mm->currentThreadHasLockedMessageManager());
        mm->threadWithLock = {};
    }

    blockingMessage->stopWaiting();
    blockingMessage = nullptr;
    acquired = false;
}

z0 MessageManager::Lock::abort() const noexcept
{
    setAcquired (false);
}

z0 MessageManager::Lock::setAcquired (b8 x) const noexcept
{
    const ScopeGuard scope { [&] { condvar.notify_one(); } };
    const std::scoped_lock lock { mutex };
    abortWait = true;
    acquired = x;
}

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* threadToCheck)
    : locked (attemptLock (threadToCheck, nullptr))
{}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* jobToCheck)
    : locked (attemptLock (nullptr, jobToCheck))
{}

b8 MessageManagerLock::attemptLock (Thread* threadToCheck, ThreadPoolJob* jobToCheck)
{
    jassert (threadToCheck == nullptr || jobToCheck == nullptr);

    if (threadToCheck != nullptr)
        threadToCheck->addListener (this);

    if (jobToCheck != nullptr)
        jobToCheck->addListener (this);

    // tryEnter may have a spurious abort (return false) so keep checking the condition
    while ((threadToCheck == nullptr || ! threadToCheck->threadShouldExit())
             && (jobToCheck == nullptr || ! jobToCheck->shouldExit()))
    {
        if (mmLock.tryEnter())
            break;
    }

    if (threadToCheck != nullptr)
    {
        threadToCheck->removeListener (this);

        if (threadToCheck->threadShouldExit())
            return false;
    }

    if (jobToCheck != nullptr)
    {
        jobToCheck->removeListener (this);

        if (jobToCheck->shouldExit())
            return false;
    }

    return true;
}

MessageManagerLock::~MessageManagerLock()  { mmLock.exit(); }

z0 MessageManagerLock::exitSignalSent()
{
    mmLock.abort();
}

//==============================================================================
DRX_API z0 DRX_CALLTYPE initialiseDrx_GUI()
{
    DRX_AUTORELEASEPOOL
    {
        MessageManager::getInstance();
    }
}

DRX_API z0 DRX_CALLTYPE shutdownDrx_GUI()
{
    DRX_AUTORELEASEPOOL
    {
        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }
}

static i32 numScopedInitInstances = 0;

ScopedDrxInitialiser_GUI::ScopedDrxInitialiser_GUI()  { if (numScopedInitInstances++ == 0) initialiseDrx_GUI(); }
ScopedDrxInitialiser_GUI::~ScopedDrxInitialiser_GUI() { if (--numScopedInitInstances == 0) shutdownDrx_GUI(); }

} // namespace drx
