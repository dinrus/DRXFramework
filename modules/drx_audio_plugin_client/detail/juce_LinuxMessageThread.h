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

#if DRX_LINUX || DRX_BSD

namespace drx::detail
{

// Implemented in drx_Messaging_linux.cpp
b8 dispatchNextMessageOnSystemQueue (b8 returnIfNoPendingMessages);

class MessageThread : public Thread
{
public:
    MessageThread() : Thread (SystemStats::getDRXVersion() + ": Plugin Message Thread")
    {
        start();
    }

    ~MessageThread() override
    {
        MessageManager::getInstance()->stopDispatchLoop();
        stop();
    }

    z0 start()
    {
        startThread (Priority::high);

        // Wait for setCurrentThreadAsMessageThread() and getInstance to be executed
        // before leaving this method
        threadInitialised.wait (10000);
    }

    z0 stop()
    {
        signalThreadShouldExit();
        stopThread (-1);
    }

    b8 isRunning() const noexcept  { return isThreadRunning(); }

    z0 run() override
    {
        MessageManager::getInstance()->setCurrentThreadAsMessageThread();
        XWindowSystem::getInstance();

        threadInitialised.signal();

        while (! threadShouldExit())
        {
            if (! dispatchNextMessageOnSystemQueue (true))
                Thread::sleep (1);
        }
    }

private:
    WaitableEvent threadInitialised;
    DRX_DECLARE_NON_MOVEABLE (MessageThread)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageThread)
};

//==============================================================================
class HostDrivenEventLoop
{
public:
    HostDrivenEventLoop()
    {
        messageThread->stop();
        MessageManager::getInstance()->setCurrentThreadAsMessageThread();
    }

    z0 processPendingEvents()
    {
        MessageManager::getInstance()->setCurrentThreadAsMessageThread();

        for (;;)
            if (! dispatchNextMessageOnSystemQueue (true))
                return;
    }

    ~HostDrivenEventLoop()
    {
        messageThread->start();
    }

private:
    SharedResourcePointer<MessageThread> messageThread;
};

} // namespace drx::detail

#endif
