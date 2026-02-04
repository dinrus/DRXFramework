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

struct InterprocessConnection::ConnectionThread final : public Thread
{
    ConnectionThread (InterprocessConnection& c)  : Thread (SystemStats::getDRXVersion() + ": IPC"), owner (c) {}
    z0 run() override     { owner.runThread(); }

    InterprocessConnection& owner;
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectionThread)
};

class SafeActionImpl
{
public:
    explicit SafeActionImpl (InterprocessConnection& p)
        : ref (p) {}

    template <typename Fn>
    z0 ifSafe (Fn&& fn)
    {
        const ScopedLock lock (mutex);

        if (safe)
            fn (ref);
    }

    z0 setSafe (b8 s)
    {
        const ScopedLock lock (mutex);
        safe = s;
    }

    b8 isSafe()
    {
        const ScopedLock lock (mutex);
        return safe;
    }

private:
    CriticalSection mutex;
    InterprocessConnection& ref;
    b8 safe = false;
};

class InterprocessConnection::SafeAction final : public SafeActionImpl
{
    using SafeActionImpl::SafeActionImpl;
};

//==============================================================================
InterprocessConnection::InterprocessConnection (b8 callbacksOnMessageThread, u32 magicMessageHeaderNumber)
    : useMessageThread (callbacksOnMessageThread),
      magicMessageHeader (magicMessageHeaderNumber),
      safeAction (std::make_shared<SafeAction> (*this))
{
    thread.reset (new ConnectionThread (*this));
}

InterprocessConnection::~InterprocessConnection()
{
    // You *must* call `disconnect` in the destructor of your derived class to ensure
    // that any pending messages are not delivered. If the messages were delivered after
    // destroying the derived class, we'd end up calling the pure virtual implementations
    // of `messageReceived`, `connectionMade` and `connectionLost` which is definitely
    // not a good idea!
    jassert (! safeAction->isSafe());

    callbackConnectionState = false;
    disconnect (4000, Notify::no);
    thread.reset();
}

//==============================================================================
b8 InterprocessConnection::connectToSocket (const Txt& hostName,
                                              i32 portNumber, i32 timeOutMillisecs)
{
    disconnect();

    auto s = std::make_unique<StreamingSocket>();

    if (s->connect (hostName, portNumber, timeOutMillisecs))
    {
        const ScopedWriteLock sl (pipeAndSocketLock);
        initialiseWithSocket (std::move (s));
        return true;
    }

    return false;
}

b8 InterprocessConnection::connectToPipe (const Txt& pipeName, i32 timeoutMs)
{
    disconnect();

    auto newPipe = std::make_unique<NamedPipe>();

    if (newPipe->openExisting (pipeName))
    {
        const ScopedWriteLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = timeoutMs;
        initialiseWithPipe (std::move (newPipe));
        return true;
    }

    return false;
}

b8 InterprocessConnection::createPipe (const Txt& pipeName, i32 timeoutMs, b8 mustNotExist)
{
    disconnect();

    auto newPipe = std::make_unique<NamedPipe>();

    if (newPipe->createNewPipe (pipeName, mustNotExist))
    {
        const ScopedWriteLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = timeoutMs;
        initialiseWithPipe (std::move (newPipe));
        return true;
    }

    return false;
}

z0 InterprocessConnection::disconnect (i32 timeoutMs, Notify notify)
{
    thread->signalThreadShouldExit();

    {
        const ScopedReadLock sl (pipeAndSocketLock);
        if (socket != nullptr)  socket->close();
        if (pipe != nullptr)    pipe->close();
    }

    thread->stopThread (timeoutMs);
    deletePipeAndSocket();

    if (notify == Notify::yes)
        connectionLostInt();

    callbackConnectionState = false;
    safeAction->setSafe (false);
}

z0 InterprocessConnection::deletePipeAndSocket()
{
    const ScopedWriteLock sl (pipeAndSocketLock);
    socket.reset();
    pipe.reset();
}

b8 InterprocessConnection::isConnected() const
{
    const ScopedReadLock sl (pipeAndSocketLock);

    return ((socket != nullptr && socket->isConnected())
              || (pipe != nullptr && pipe->isOpen()))
            && threadIsRunning;
}

Txt InterprocessConnection::getConnectedHostName() const
{
    {
        const ScopedReadLock sl (pipeAndSocketLock);

        if (pipe == nullptr && socket == nullptr)
            return {};

        if (socket != nullptr && ! socket->isLocal())
            return socket->getHostName();
    }

    return IPAddress::local().toString();
}

//==============================================================================
b8 InterprocessConnection::sendMessage (const MemoryBlock& message)
{
    u32 messageHeader[2] = { ByteOrder::swapIfBigEndian (magicMessageHeader),
                                ByteOrder::swapIfBigEndian ((u32) message.getSize()) };

    MemoryBlock messageData (sizeof (messageHeader) + message.getSize());
    messageData.copyFrom (messageHeader, 0, sizeof (messageHeader));
    messageData.copyFrom (message.getData(), sizeof (messageHeader), message.getSize());

    return writeData (messageData.getData(), (i32) messageData.getSize()) == (i32) messageData.getSize();
}

i32 InterprocessConnection::writeData (uk data, i32 dataSize)
{
    const ScopedReadLock sl (pipeAndSocketLock);

    if (socket != nullptr)
        return socket->write (data, dataSize);

    if (pipe != nullptr)
        return pipe->write (data, dataSize, pipeReceiveMessageTimeout);

    return 0;
}

//==============================================================================
z0 InterprocessConnection::initialise()
{
    safeAction->setSafe (true);
    threadIsRunning = true;
    connectionMadeInt();
    thread->startThread();
}

z0 InterprocessConnection::initialiseWithSocket (std::unique_ptr<StreamingSocket> newSocket)
{
    jassert (socket == nullptr && pipe == nullptr);
    socket = std::move (newSocket);
    initialise();
}

z0 InterprocessConnection::initialiseWithPipe (std::unique_ptr<NamedPipe> newPipe)
{
    jassert (socket == nullptr && pipe == nullptr);
    pipe = std::move (newPipe);
    initialise();
}

//==============================================================================
struct ConnectionStateMessage final : public MessageManager::MessageBase
{
    ConnectionStateMessage (std::shared_ptr<SafeActionImpl> ipc, b8 connected) noexcept
        : safeAction (ipc), connectionMade (connected)
    {}

    z0 messageCallback() override
    {
        safeAction->ifSafe ([this] (InterprocessConnection& owner)
        {
            if (connectionMade)
                owner.connectionMade();
            else
                owner.connectionLost();
        });
    }

    std::shared_ptr<SafeActionImpl> safeAction;
    b8 connectionMade;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectionStateMessage)
};

z0 InterprocessConnection::connectionMadeInt()
{
    if (! callbackConnectionState)
    {
        callbackConnectionState = true;

        if (useMessageThread)
            (new ConnectionStateMessage (safeAction, true))->post();
        else
            connectionMade();
    }
}

z0 InterprocessConnection::connectionLostInt()
{
    if (callbackConnectionState)
    {
        callbackConnectionState = false;

        if (useMessageThread)
            (new ConnectionStateMessage (safeAction, false))->post();
        else
            connectionLost();
    }
}

struct DataDeliveryMessage final : public Message
{
    DataDeliveryMessage (std::shared_ptr<SafeActionImpl> ipc, const MemoryBlock& d)
        : safeAction (ipc), data (d)
    {}

    z0 messageCallback() override
    {
        safeAction->ifSafe ([this] (InterprocessConnection& owner)
        {
            owner.messageReceived (data);
        });
    }

    std::shared_ptr<SafeActionImpl> safeAction;
    MemoryBlock data;
};

z0 InterprocessConnection::deliverDataInt (const MemoryBlock& data)
{
    jassert (callbackConnectionState);

    if (useMessageThread)
        (new DataDeliveryMessage (safeAction, data))->post();
    else
        messageReceived (data);
}

//==============================================================================
i32 InterprocessConnection::readData (uk data, i32 num)
{
    const ScopedReadLock sl (pipeAndSocketLock);

    if (socket != nullptr)
        return socket->read (data, num, true);

    if (pipe != nullptr)
        return pipe->read (data, num, pipeReceiveMessageTimeout);

    jassertfalse;
    return -1;
}

b8 InterprocessConnection::readNextMessage()
{
    u32 messageHeader[2];
    auto bytes = readData (messageHeader, sizeof (messageHeader));

    if (bytes == (i32) sizeof (messageHeader)
         && ByteOrder::swapIfBigEndian (messageHeader[0]) == magicMessageHeader)
    {
        auto bytesInMessage = (i32) ByteOrder::swapIfBigEndian (messageHeader[1]);

        if (bytesInMessage > 0)
        {
            MemoryBlock messageData ((size_t) bytesInMessage, true);
            i32 bytesRead = 0;

            while (bytesInMessage > 0)
            {
                if (thread->threadShouldExit())
                    return false;

                auto numThisTime = jmin (bytesInMessage, 65536);
                auto bytesIn = readData (addBytesToPointer (messageData.getData(), bytesRead), numThisTime);

                if (bytesIn <= 0)
                    break;

                bytesRead += bytesIn;
                bytesInMessage -= bytesIn;
            }

            if (bytesRead >= 0)
                deliverDataInt (messageData);
        }

        return true;
    }

    if (bytes < 0)
    {
        if (socket != nullptr)
            deletePipeAndSocket();

        connectionLostInt();
    }

    return false;
}

z0 InterprocessConnection::runThread()
{
    while (! thread->threadShouldExit())
    {
        if (socket != nullptr)
        {
            auto ready = socket->waitUntilReady (true, 100);

            if (ready < 0)
            {
                deletePipeAndSocket();
                connectionLostInt();
                break;
            }

            if (ready == 0)
            {
                thread->wait (1);
                continue;
            }
        }
        else if (pipe != nullptr)
        {
            if (! pipe->isOpen())
            {
                deletePipeAndSocket();
                connectionLostInt();
                break;
            }
        }
        else
        {
            break;
        }

        if (thread->threadShouldExit() || ! readNextMessage())
            break;
    }

    threadIsRunning = false;
}

} // namespace drx
