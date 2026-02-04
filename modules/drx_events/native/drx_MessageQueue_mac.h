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
/* An internal message pump class used in OSX and iOS. */
class MessageQueue
{
public:
    MessageQueue()
    {
       #if DRX_IOS
        runLoop = CFRunLoopGetCurrent();
       #else
        runLoop = CFRunLoopGetMain();
       #endif

        CFRunLoopSourceContext sourceContext;
        zerostruct (sourceContext); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
        sourceContext.info = this;
        sourceContext.perform = runLoopSourceCallback;
        runLoopSource.reset (CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext));
        CFRunLoopAddSource (runLoop, runLoopSource.get(), kCFRunLoopCommonModes);
    }

    ~MessageQueue() noexcept
    {
        CFRunLoopRemoveSource (runLoop, runLoopSource.get(), kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate (runLoopSource.get());
    }

    z0 post (MessageManager::MessageBase* const message)
    {
        messages.add (message);
        wakeUp();
    }

private:
    ReferenceCountedArray<MessageManager::MessageBase, CriticalSection> messages;
    CFRunLoopRef runLoop;
    CFUniquePtr<CFRunLoopSourceRef> runLoopSource;

    z0 wakeUp() noexcept
    {
        CFRunLoopSourceSignal (runLoopSource.get());
        CFRunLoopWakeUp (runLoop);
    }

    b8 deliverNextMessage()
    {
        const MessageManager::MessageBase::Ptr nextMessage (messages.removeAndReturn (0));

        if (nextMessage == nullptr)
            return false;

        DRX_AUTORELEASEPOOL
        {
            DRX_TRY
            {
                nextMessage->messageCallback();
            }
            DRX_CATCH_EXCEPTION
        }

        return true;
    }

    z0 runLoopCallback() noexcept
    {
        for (i32 i = 4; --i >= 0;)
            if (! deliverNextMessage())
                return;

        wakeUp();
    }

    static z0 runLoopSourceCallback (uk info) noexcept
    {
        static_cast<MessageQueue*> (info)->runLoopCallback();
    }
};

} // namespace drx
