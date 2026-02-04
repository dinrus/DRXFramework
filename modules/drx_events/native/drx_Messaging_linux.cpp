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
class InternalMessageQueue
{
public:
    InternalMessageQueue()
    {
        [[maybe_unused]] auto err = ::socketpair (AF_LOCAL, SOCK_STREAM, 0, msgpipe);
        jassert (err == 0);

        LinuxEventLoop::registerFdCallback (getReadHandle(),
                                            [this] (i32 fd)
                                            {
                                                while (auto msg = popNextMessage (fd))
                                                {
                                                    DRX_TRY
                                                    {
                                                        msg->messageCallback();
                                                    }
                                                    DRX_CATCH_EXCEPTION
                                                }
                                            });
    }

    ~InternalMessageQueue()
    {
        LinuxEventLoop::unregisterFdCallback (getReadHandle());

        close (getReadHandle());
        close (getWriteHandle());

        clearSingletonInstance();
    }

    //==============================================================================
    z0 postMessage (MessageManager::MessageBase* const msg) noexcept
    {
        ScopedLock sl (lock);
        queue.add (msg);

        if (bytesInSocket < maxBytesInSocketQueue)
        {
            bytesInSocket++;

            ScopedUnlock ul (lock);
            u8 x = 0xff;
            [[maybe_unused]] auto numBytes = write (getWriteHandle(), &x, 1);
        }
    }

    //==============================================================================
    DRX_DECLARE_SINGLETON_INLINE (InternalMessageQueue, false)

private:
    CriticalSection lock;
    ReferenceCountedArray <MessageManager::MessageBase> queue;

    i32 msgpipe[2];
    i32 bytesInSocket = 0;
    static constexpr i32 maxBytesInSocketQueue = 128;

    i32 getWriteHandle() const noexcept  { return msgpipe[0]; }
    i32 getReadHandle() const noexcept   { return msgpipe[1]; }

    MessageManager::MessageBase::Ptr popNextMessage (i32 fd) noexcept
    {
        const ScopedLock sl (lock);

        if (bytesInSocket > 0)
        {
            --bytesInSocket;

            ScopedUnlock ul (lock);
            u8 x;
            [[maybe_unused]] auto numBytes = read (fd, &x, 1);
        }

        return queue.removeAndReturn (0);
    }
};

//==============================================================================
/*
    Stores callbacks associated with file descriptors (FD).

    The callback for a particular FD should be called whenever that file has data to read.

    For standalone apps, the main thread will call poll to wait for new data on any FD, and then
    call the associated callbacks for any FDs that changed.

    For plugins, the host (generally) provides some kind of run loop mechanism instead.
    - In VST2 plugins, the host should call effEditIdle at regular intervals, and plugins can
      dispatch all pending events inside this callback. The host doesn't know about any of the
      plugin's FDs, so it's possible there will be a bit of latency between an FD becoming ready,
      and its associated callback being called.
    - In VST3 plugins, it's possible to register each FD individually with the host. In this case,
      the facilities in LinuxEventLoopInternal can be used to observe added/removed FD callbacks,
      and the host can be notified whenever the set of FDs changes. The host will call onFDIsSet
      whenever a particular FD has data ready. This call should be forwarded through to
      InternalRunLoop::dispatchEvent.
*/
struct InternalRunLoop
{
public:
    InternalRunLoop() = default;

    z0 registerFdCallback (i32 fd, std::function<z0()>&& cb, short eventMask)
    {
        {
            const ScopedLock sl (lock);

            callbacks.emplace (fd, std::make_shared<std::function<z0()>> (std::move (cb)));

            const auto iter = getPollfd (fd);

            if (iter == pfds.end() || iter->fd != fd)
                pfds.insert (iter, { fd, eventMask, 0 });
            else
                jassertfalse;

            jassert (pfdsAreSorted());
        }

        listeners.call ([] (auto& l) { l.fdCallbacksChanged(); });
    }

    z0 unregisterFdCallback (i32 fd)
    {
        {
            const ScopedLock sl (lock);

            callbacks.erase (fd);

            const auto iter = getPollfd (fd);

            if (iter != pfds.end() && iter->fd == fd)
                pfds.erase (iter);
            else
                jassertfalse;

            jassert (pfdsAreSorted());
        }

        listeners.call ([] (auto& l) { l.fdCallbacksChanged(); });
    }

    b8 dispatchPendingEvents()
    {
        callbackStorage.clear();
        getFunctionsToCallThisTime (callbackStorage);

        // CriticalSection should be available during the callback
        for (auto& fn : callbackStorage)
            (*fn)();

        return ! callbackStorage.empty();
    }

    z0 dispatchEvent (i32 fd) const
    {
        const auto fn = [&]
        {
            const ScopedLock sl (lock);
            const auto iter = callbacks.find (fd);
            return iter != callbacks.end() ? iter->second : nullptr;
        }();

        // CriticalSection should be available during the callback
        if (auto* callback = fn.get())
            (*callback)();
    }

    b8 sleepUntilNextEvent (i32 timeoutMs)
    {
        const ScopedLock sl (lock);
        return poll (pfds.data(), static_cast<nfds_t> (pfds.size()), timeoutMs) != 0;
    }

    std::vector<i32> getRegisteredFds()
    {
        const ScopedLock sl (lock);
        std::vector<i32> result;
        result.reserve (callbacks.size());
        std::transform (callbacks.begin(),
                        callbacks.end(),
                        std::back_inserter (result),
                        [] (const auto& pair) { return pair.first; });
        return result;
    }

    z0 addListener    (LinuxEventLoopInternal::Listener& listener)         { listeners.add    (&listener); }
    z0 removeListener (LinuxEventLoopInternal::Listener& listener)         { listeners.remove (&listener); }

    //==============================================================================
    DRX_DECLARE_SINGLETON_INLINE (InternalRunLoop, false)

private:
    using SharedCallback = std::shared_ptr<std::function<z0()>>;

    /*  Appends any functions that need to be called to the passed-in vector.

        We take a copy of each shared function so that the functions can be called without
        locking or racing in the event that the function attempts to register/deregister a
        new FD callback.
    */
    z0 getFunctionsToCallThisTime (std::vector<SharedCallback>& functions)
    {
        const ScopedLock sl (lock);

        if (! sleepUntilNextEvent (0))
            return;

        for (auto& pfd : pfds)
        {
            if (std::exchange (pfd.revents, 0) != 0)
            {
                const auto iter = callbacks.find (pfd.fd);

                if (iter != callbacks.end())
                    functions.emplace_back (iter->second);
            }
        }
    }

    std::vector<pollfd>::iterator getPollfd (i32 fd)
    {
        return std::lower_bound (pfds.begin(), pfds.end(), fd, [] (auto descriptor, auto toFind)
        {
            return descriptor.fd < toFind;
        });
    }

    b8 pfdsAreSorted() const
    {
        return std::is_sorted (pfds.begin(), pfds.end(), [] (auto a, auto b) { return a.fd < b.fd; });
    }

    CriticalSection lock;

    std::map<i32, SharedCallback> callbacks;
    std::vector<SharedCallback> callbackStorage;
    std::vector<pollfd> pfds;

    ListenerList<LinuxEventLoopInternal::Listener> listeners;
};

//==============================================================================
namespace LinuxErrorHandling
{
    static b8 keyboardBreakOccurred = false;

    static z0 keyboardBreakSignalHandler (i32 sig)
    {
        if (sig == SIGINT)
            keyboardBreakOccurred = true;
    }

    static z0 installKeyboardBreakHandler()
    {
        struct sigaction saction;
        sigset_t maskSet;
        sigemptyset (&maskSet);
        saction.sa_handler = keyboardBreakSignalHandler;
        saction.sa_mask = maskSet;
        saction.sa_flags = 0;
        sigaction (SIGINT, &saction, nullptr);
    }
}

//==============================================================================
z0 MessageManager::doPlatformSpecificInitialisation()
{
    if (DRXApplicationBase::isStandaloneApp())
        LinuxErrorHandling::installKeyboardBreakHandler();

    InternalRunLoop::getInstance();
    InternalMessageQueue::getInstance();
}

z0 MessageManager::doPlatformSpecificShutdown()
{
    InternalMessageQueue::deleteInstance();
    InternalRunLoop::deleteInstance();
}

b8 MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    if (auto* queue = InternalMessageQueue::getInstanceWithoutCreating())
    {
        queue->postMessage (message);
        return true;
    }

    return false;
}

z0 MessageManager::broadcastMessage (const Txt&)
{
    // TODO
}

namespace detail
{
// this function expects that it will NEVER be called simultaneously for two concurrent threads
b8 dispatchNextMessageOnSystemQueue (b8 returnIfNoPendingMessages)
{
    for (;;)
    {
        if (LinuxErrorHandling::keyboardBreakOccurred)
            DRXApplicationBase::quit();

        if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        {
            if (runLoop->dispatchPendingEvents())
                break;

            if (returnIfNoPendingMessages)
                return false;

            runLoop->sleepUntilNextEvent (2000);
        }
    }

    return true;
}
} // namespace detail

//==============================================================================
z0 LinuxEventLoop::registerFdCallback (i32 fd, std::function<z0 (i32)> readCallback, short eventMask)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->registerFdCallback (fd, [cb = std::move (readCallback), fd] { cb (fd); }, eventMask);
}

z0 LinuxEventLoop::unregisterFdCallback (i32 fd)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->unregisterFdCallback (fd);
}

//==============================================================================
z0 LinuxEventLoopInternal::registerLinuxEventLoopListener (LinuxEventLoopInternal::Listener& listener)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->addListener (listener);
}

z0 LinuxEventLoopInternal::deregisterLinuxEventLoopListener (LinuxEventLoopInternal::Listener& listener)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->removeListener (listener);
}

z0 LinuxEventLoopInternal::invokeEventLoopCallbackForFd (i32 fd)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->dispatchEvent (fd);
}

std::vector<i32> LinuxEventLoopInternal::getRegisteredFds()
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        return runLoop->getRegisteredFds();

    return {};
}

} // namespace drx
