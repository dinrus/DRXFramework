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

#if ! DRX_WASM

NamedPipe::NamedPipe() {}

NamedPipe::~NamedPipe()
{
    close();
}

b8 NamedPipe::openExisting (const Txt& pipeName)
{
    close();

    ScopedWriteLock sl (lock);
    currentPipeName = pipeName;
    return openInternal (pipeName, false, false);
}

b8 NamedPipe::isOpen() const
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr;
}

b8 NamedPipe::createNewPipe (const Txt& pipeName, b8 mustNotExist)
{
    close();

    ScopedWriteLock sl (lock);
    currentPipeName = pipeName;
    return openInternal (pipeName, true, mustNotExist);
}

Txt NamedPipe::getName() const
{
    ScopedReadLock sl (lock);
    return currentPipeName;
}

// other methods for this class are implemented in the platform-specific files


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class NamedPipeTests final : public UnitTest
{
public:
    //==============================================================================
    NamedPipeTests()
        : UnitTest ("NamedPipe", UnitTestCategories::networking)
    {}

    z0 runTest() override
    {
        const auto pipeName = "TestPipe" + Txt ((intptr_t) Thread::getCurrentThreadId());

        beginTest ("Pre test cleanup");
        {
            NamedPipe pipe;
            expect (pipe.createNewPipe (pipeName, false));
        }

        beginTest ("Create pipe");
        {
            NamedPipe pipe;
            expect (! pipe.isOpen());

            expect (pipe.createNewPipe (pipeName, true));
            expect (pipe.isOpen());

            expect (pipe.createNewPipe (pipeName, false));
            expect (pipe.isOpen());

            NamedPipe otherPipe;
            expect (! otherPipe.createNewPipe (pipeName, true));
            expect (! otherPipe.isOpen());
        }

        beginTest ("Existing pipe");
        {
            NamedPipe pipe;

            expect (! pipe.openExisting (pipeName));
            expect (! pipe.isOpen());

            expect (pipe.createNewPipe (pipeName, true));

            NamedPipe otherPipe;
            expect (otherPipe.openExisting (pipeName));
            expect (otherPipe.isOpen());
        }

        i32 sendData = 4684682;

        beginTest ("Receive message created pipe");
        {
            NamedPipe pipe;
            expect (pipe.createNewPipe (pipeName, true));

            WaitableEvent senderFinished;
            SenderThread sender (pipeName, false, senderFinished, sendData);

            sender.startThread();

            i32 recvData = -1;
            auto bytesRead = pipe.read (&recvData, sizeof (recvData), 2000);

            expect (senderFinished.wait (4000));

            expectEquals (bytesRead, (i32) sizeof (recvData));
            expectEquals (sender.result, (i32) sizeof (sendData));
            expectEquals (recvData, sendData);
        }

        beginTest ("Receive message existing pipe");
        {
            WaitableEvent senderFinished;
            SenderThread sender (pipeName, true, senderFinished, sendData);

            NamedPipe pipe;
            expect (pipe.openExisting (pipeName));

            sender.startThread();

            i32 recvData = -1;
            auto bytesRead = pipe.read (&recvData, sizeof (recvData), 2000);

            expect (senderFinished.wait (4000));

            expectEquals (bytesRead, (i32) sizeof (recvData));
            expectEquals (sender.result, (i32) sizeof (sendData));
            expectEquals (recvData, sendData);
        }

        beginTest ("Send message created pipe");
        {
            NamedPipe pipe;
            expect (pipe.createNewPipe (pipeName, true));

            WaitableEvent receiverFinished;
            ReceiverThread receiver (pipeName, false, receiverFinished);

            receiver.startThread();

            auto bytesWritten = pipe.write (&sendData, sizeof (sendData), 2000);

            expect (receiverFinished.wait (4000));

            expectEquals (bytesWritten, (i32) sizeof (sendData));
            expectEquals (receiver.result, (i32) sizeof (receiver.recvData));
            expectEquals (receiver.recvData, sendData);
        }

        beginTest ("Send message existing pipe");
        {
            WaitableEvent receiverFinished;
            ReceiverThread receiver (pipeName, true, receiverFinished);

            NamedPipe pipe;
            expect (pipe.openExisting (pipeName));

            receiver.startThread();

            auto bytesWritten = pipe.write (&sendData, sizeof (sendData), 2000);

            expect (receiverFinished.wait (4000));

            expectEquals (bytesWritten, (i32) sizeof (sendData));
            expectEquals (receiver.result, (i32) sizeof (receiver.recvData));
            expectEquals (receiver.recvData, sendData);
        }
    }

private:
    //==============================================================================
    struct NamedPipeThread : public Thread
    {
        NamedPipeThread (const Txt& tName, const Txt& pName,
                         b8 shouldCreatePipe, WaitableEvent& completed)
            : Thread (tName), pipeName (pName), workCompleted (completed)
        {
            if (shouldCreatePipe)
                pipe.createNewPipe (pipeName);
            else
                pipe.openExisting (pipeName);
        }

        NamedPipe pipe;
        const Txt& pipeName;
        WaitableEvent& workCompleted;

        i32 result = -2;
    };

    //==============================================================================
    struct SenderThread final : public NamedPipeThread
    {
        SenderThread (const Txt& pName, b8 shouldCreatePipe,
                      WaitableEvent& completed, i32 sData)
            : NamedPipeThread ("NamePipeSender", pName, shouldCreatePipe, completed),
              sendData (sData)
        {}

        ~SenderThread() override
        {
            stopThread (100);
        }

        z0 run() override
        {
            result = pipe.write (&sendData, sizeof (sendData), 2000);
            workCompleted.signal();
        }

        i32k sendData;
    };

    //==============================================================================
    struct ReceiverThread final : public NamedPipeThread
    {
        ReceiverThread (const Txt& pName, b8 shouldCreatePipe,
                        WaitableEvent& completed)
            : NamedPipeThread ("NamePipeReceiver", pName, shouldCreatePipe, completed)
        {}

        ~ReceiverThread() override
        {
            stopThread (100);
        }

        z0 run() override
        {
            result = pipe.read (&recvData, sizeof (recvData), 2000);
            workCompleted.signal();
        }

        i32 recvData = -2;
    };
};

static NamedPipeTests namedPipeTests;

#endif
#endif

} // namespace drx
