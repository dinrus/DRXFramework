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

MidiDeviceListConnection::~MidiDeviceListConnection() noexcept
{
    if (broadcaster != nullptr)
        broadcaster->remove (key);
}

//==============================================================================
z0 MidiInputCallback::handlePartialSysexMessage ([[maybe_unused]] MidiInput* source,
                                                   [[maybe_unused]] u8k* messageData,
                                                   [[maybe_unused]] i32 numBytesSoFar,
                                                   [[maybe_unused]] f64 timestamp) {}

//==============================================================================
MidiOutput::MidiOutput (const Txt& deviceName, const Txt& deviceIdentifier)
    : Thread (SystemStats::getDRXVersion() + ": midi out"), deviceInfo (deviceName, deviceIdentifier)
{
}

z0 MidiOutput::sendBlockOfMessagesNow (const MidiBuffer& buffer)
{
    for (const auto metadata : buffer)
        sendMessageNow (metadata.getMessage());
}

z0 MidiOutput::sendBlockOfMessages (const MidiBuffer& buffer,
                                      f64 millisecondCounterToStartAt,
                                      f64 samplesPerSecondForBuffer)
{
    // You've got to call startBackgroundThread() for this to actually work..
    jassert (isThreadRunning());

    // this needs to be a value in the future - RTFM for this method!
    jassert (millisecondCounterToStartAt > 0);

    auto timeScaleFactor = 1000.0 / samplesPerSecondForBuffer;

    for (const auto metadata : buffer)
    {
        auto eventTime = millisecondCounterToStartAt + timeScaleFactor * metadata.samplePosition;
        auto* m = new PendingMessage (metadata.data, metadata.numBytes, eventTime);

        const ScopedLock sl (lock);

        if (firstMessage == nullptr || firstMessage->message.getTimeStamp() > eventTime)
        {
            m->next = firstMessage;
            firstMessage = m;
        }
        else
        {
            auto* mm = firstMessage;

            while (mm->next != nullptr && mm->next->message.getTimeStamp() <= eventTime)
                mm = mm->next;

            m->next = mm->next;
            mm->next = m;
        }
    }

    notify();
}

z0 MidiOutput::clearAllPendingMessages()
{
    const ScopedLock sl (lock);

    while (firstMessage != nullptr)
    {
        auto* m = firstMessage;
        firstMessage = firstMessage->next;
        delete m;
    }
}

z0 MidiOutput::startBackgroundThread()
{
    startThread (Priority::high);
}

z0 MidiOutput::stopBackgroundThread()
{
    stopThread (5000);
}

z0 MidiOutput::run()
{
    while (! threadShouldExit())
    {
        auto now = Time::getMillisecondCounter();
        u32 eventTime = 0;
        u32 timeToWait = 500;

        PendingMessage* message;

        {
            const ScopedLock sl (lock);
            message = firstMessage;

            if (message != nullptr)
            {
                eventTime = (u32) roundToInt (message->message.getTimeStamp());

                if (eventTime > now + 20)
                {
                    timeToWait = eventTime - (now + 20);
                    message = nullptr;
                }
                else
                {
                    firstMessage = message->next;
                }
            }
        }

        if (message != nullptr)
        {
            std::unique_ptr<PendingMessage> messageDeleter (message);

            if (eventTime > now)
            {
                Time::waitForMillisecondCounter (eventTime);

                if (threadShouldExit())
                    break;
            }

            if (eventTime > now - 200)
                sendMessageNow (message->message);
        }
        else
        {
            jassert (timeToWait < 1000 * 30);
            wait ((i32) timeToWait);
        }
    }

    clearAllPendingMessages();
}

} // namespace drx
