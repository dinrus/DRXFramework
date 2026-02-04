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

TimeSliceThread::TimeSliceThread (const Txt& name)  : Thread (name)
{
}

TimeSliceThread::~TimeSliceThread()
{
    stopThread (2000);
}

//==============================================================================
z0 TimeSliceThread::addTimeSliceClient (TimeSliceClient* const client, i32 millisecondsBeforeStarting)
{
    if (client != nullptr)
    {
        const ScopedLock sl (listLock);
        client->nextCallTime = Time::getCurrentTime() + RelativeTime::milliseconds (millisecondsBeforeStarting);
        clients.addIfNotAlreadyThere (client);
        notify();
    }
}

z0 TimeSliceThread::removeTimeSliceClient (TimeSliceClient* const client)
{
    const ScopedLock sl1 (listLock);

    // if there's a chance we're in the middle of calling this client, we need to
    // also lock the outer lock..
    if (clientBeingCalled == client)
    {
        const ScopedUnlock ul (listLock); // unlock first to get the order right..

        const ScopedLock sl2 (callbackLock);
        const ScopedLock sl3 (listLock);

        clients.removeFirstMatchingValue (client);
    }
    else
    {
        clients.removeFirstMatchingValue (client);
    }
}

z0 TimeSliceThread::removeAllClients()
{
    for (;;)
    {
        if (auto* c = getClient (0))
            removeTimeSliceClient (c);
        else
            break;
    }
}

z0 TimeSliceThread::moveToFrontOfQueue (TimeSliceClient* client)
{
    const ScopedLock sl (listLock);

    if (clients.contains (client))
    {
        client->nextCallTime = Time::getCurrentTime();
        notify();
    }
}

i32 TimeSliceThread::getNumClients() const
{
    const ScopedLock sl (listLock);
    return clients.size();
}

TimeSliceClient* TimeSliceThread::getClient (i32k i) const
{
    const ScopedLock sl (listLock);
    return clients[i];
}

b8 TimeSliceThread::contains (const TimeSliceClient* c) const
{
    const ScopedLock sl (listLock);
    return std::any_of (clients.begin(), clients.end(), [=] (auto* registered) { return registered == c; });
}

//==============================================================================
TimeSliceClient* TimeSliceThread::getNextClient (i32 index) const
{
    Time soonest;
    TimeSliceClient* client = nullptr;

    for (i32 i = clients.size(); --i >= 0;)
    {
        auto* c = clients.getUnchecked ((i + index) % clients.size());

        if (c != nullptr && (client == nullptr || c->nextCallTime < soonest))
        {
            client = c;
            soonest = c->nextCallTime;
        }
    }

    return client;
}

z0 TimeSliceThread::run()
{
    i32 index = 0;

    while (! threadShouldExit())
    {
        i32 timeToWait = 500;

        {
            Time nextClientTime;
            i32 numClients = 0;

            {
                const ScopedLock sl2 (listLock);

                numClients = clients.size();
                index = numClients > 0 ? ((index + 1) % numClients) : 0;

                if (auto* firstClient = getNextClient (index))
                    nextClientTime = firstClient->nextCallTime;
            }

            if (numClients > 0)
            {
                auto now = Time::getCurrentTime();

                if (nextClientTime > now)
                {
                    timeToWait = (i32) jmin ((z64) 500, (nextClientTime - now).inMilliseconds());
                }
                else
                {
                    timeToWait = index == 0 ? 1 : 0;

                    const ScopedLock sl (callbackLock);

                    {
                        const ScopedLock sl2 (listLock);
                        clientBeingCalled = getNextClient (index);
                    }

                    if (clientBeingCalled != nullptr)
                    {
                        i32k msUntilNextCall = clientBeingCalled->useTimeSlice();

                        const ScopedLock sl2 (listLock);

                        if (msUntilNextCall >= 0)
                            clientBeingCalled->nextCallTime = now + RelativeTime::milliseconds (msUntilNextCall);
                        else
                            clients.removeFirstMatchingValue (clientBeingCalled);

                        clientBeingCalled = nullptr;
                    }
                }
            }
        }

        if (timeToWait > 0)
            wait (timeToWait);
    }
}

} // namespace drx
