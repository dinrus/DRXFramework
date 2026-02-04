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

z0 MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    while (quitMessagePosted.get() == 0)
    {
        DRX_AUTORELEASEPOOL
        {
            [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                     beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.001]];
        }
    }
}

z0 MessageManager::stopDispatchLoop()
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
       [[[UIApplication sharedApplication] delegate] applicationWillTerminate: [UIApplication sharedApplication]];

    exit (0); // iOS apps get no mercy..
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 MessageManager::runDispatchLoopUntil (i32 millisecondsToRunFor)
{
    DRX_AUTORELEASEPOOL
    {
        jassert (isThisTheMessageThread()); // must only be called by the message thread

        u32 startTime = Time::getMillisecondCounter();
        NSDate* endDate = [NSDate dateWithTimeIntervalSinceNow: millisecondsToRunFor * 0.001];

        while (quitMessagePosted.get() == 0)
        {
            DRX_AUTORELEASEPOOL
            {
                [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                         beforeDate: endDate];

                if (millisecondsToRunFor >= 0
                     && Time::getMillisecondCounter() >= startTime + (u32) millisecondsToRunFor)
                    break;
            }
        }

        return quitMessagePosted.get() == 0;
    }
}
#endif

//==============================================================================
static std::unique_ptr<MessageQueue> messageQueue;

z0 MessageManager::doPlatformSpecificInitialisation()
{
    if (messageQueue == nullptr)
        messageQueue.reset (new MessageQueue());
}

z0 MessageManager::doPlatformSpecificShutdown()
{
    messageQueue = nullptr;
}

b8 MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    if (messageQueue != nullptr)
        messageQueue->post (message);

    return true;
}

z0 MessageManager::broadcastMessage (const Txt&)
{
    // N/A on current iOS
}

} // namespace drx
