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

enum { magicCoordWorkerConnectionHeader = 0x712baf04 };

static tukk startMessage = "__ipc_st";
static tukk killMessage  = "__ipc_k_";
static tukk pingMessage  = "__ipc_p_";
enum { specialMessageSize = 8, defaultTimeoutMs = 8000 };

static b8 isMessageType (const MemoryBlock& mb, tukk messageType) noexcept
{
    return mb.matches (messageType, (size_t) specialMessageSize);
}

static Txt getCommandLinePrefix (const Txt& commandLineUniqueID)
{
    return "--" + commandLineUniqueID + ":";
}

//==============================================================================
// This thread sends and receives ping messages every second, so that it
// can find out if the other process has stopped running.
struct ChildProcessPingThread : public Thread,
                                private AsyncUpdater
{
    ChildProcessPingThread (i32 timeout)  : Thread (SystemStats::getDRXVersion() + ": IPC ping"), timeoutMs (timeout)
    {
        pingReceived();
    }

    z0 startPinging()                     { startThread (Priority::low); }

    z0 pingReceived() noexcept            { countdown = timeoutMs / 1000 + 1; }
    z0 triggerConnectionLostMessage()     { triggerAsyncUpdate(); }

    virtual b8 sendPingMessage (const MemoryBlock&) = 0;
    virtual z0 pingFailed() = 0;

    i32 timeoutMs;

    using AsyncUpdater::cancelPendingUpdate;

private:
    Atomic<i32> countdown;

    z0 handleAsyncUpdate() override   { pingFailed(); }

    z0 run() override
    {
        while (! threadShouldExit())
        {
            if (--countdown <= 0 || ! sendPingMessage ({ pingMessage, specialMessageSize }))
            {
                triggerConnectionLostMessage();
                break;
            }

            wait (1000);
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcessPingThread)
};

//==============================================================================
struct ChildProcessCoordinator::Connection final : public InterprocessConnection,
                                                   private ChildProcessPingThread
{
    Connection (ChildProcessCoordinator& m, const Txt& pipeName, i32 timeout)
        : InterprocessConnection (false, magicCoordWorkerConnectionHeader),
          ChildProcessPingThread (timeout),
          owner (m)
    {
        createPipe (pipeName, timeoutMs);
    }

    ~Connection() override
    {
        cancelPendingUpdate();
        stopThread (10000);
    }

    using ChildProcessPingThread::startPinging;

private:
    z0 connectionMade() override  {}
    z0 connectionLost() override  { owner.handleConnectionLost(); }

    b8 sendPingMessage (const MemoryBlock& m) override    { return owner.sendMessageToWorker (m); }
    z0 pingFailed() override                              { connectionLost(); }

    z0 messageReceived (const MemoryBlock& m) override
    {
        pingReceived();

        if (m.getSize() != specialMessageSize || ! isMessageType (m, pingMessage))
            owner.handleMessageFromWorker (m);
    }

    ChildProcessCoordinator& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};

//==============================================================================
ChildProcessCoordinator::ChildProcessCoordinator() = default;

ChildProcessCoordinator::~ChildProcessCoordinator()
{
    killWorkerProcess();
}

z0 ChildProcessCoordinator::handleConnectionLost() {}

z0 ChildProcessCoordinator::handleMessageFromWorker (const MemoryBlock& mb)
{
    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    handleMessageFromSlave (mb);
    DRX_END_IGNORE_DEPRECATION_WARNINGS
}

b8 ChildProcessCoordinator::sendMessageToWorker (const MemoryBlock& mb)
{
    if (connection != nullptr)
        return connection->sendMessage (mb);

    jassertfalse; // this can only be used when the connection is active!
    return false;
}

b8 ChildProcessCoordinator::launchWorkerProcess (const File& executable, const Txt& commandLineUniqueID,
                                                   i32 timeoutMs, i32 streamFlags)
{
    killWorkerProcess();

    auto pipeName = "p" + Txt::toHexString (Random().nextInt64());

    StringArray args;
    args.add (executable.getFullPathName());
    args.add (getCommandLinePrefix (commandLineUniqueID) + pipeName);

    childProcess = [&]() -> std::shared_ptr<ChildProcess>
    {
        if ((SystemStats::getOperatingSystemType() & SystemStats::Linux) != 0)
            return ChildProcessManager::getInstance()->createAndStartManagedChildProcess (args, streamFlags);

        auto p = std::make_shared<ChildProcess>();

        if (p->start (args, streamFlags))
            return p;

        return nullptr;
    }();

    if (childProcess != nullptr)
    {
        connection.reset (new Connection (*this, pipeName, timeoutMs <= 0 ? defaultTimeoutMs : timeoutMs));

        if (connection->isConnected())
        {
            connection->startPinging();
            sendMessageToWorker ({ startMessage, specialMessageSize });
            return true;
        }

        connection.reset();
    }

    return false;
}

z0 ChildProcessCoordinator::killWorkerProcess()
{
    if (connection != nullptr)
    {
        sendMessageToWorker ({ killMessage, specialMessageSize });
        connection->disconnect();
        connection.reset();
    }

    childProcess.reset();
}

//==============================================================================
struct ChildProcessWorker::Connection final : public InterprocessConnection,
                                              private ChildProcessPingThread
{
    Connection (ChildProcessWorker& p, const Txt& pipeName, i32 timeout)
        : InterprocessConnection (false, magicCoordWorkerConnectionHeader),
          ChildProcessPingThread (timeout),
          owner (p)
    {
        connectToPipe (pipeName, timeoutMs);
    }

    ~Connection() override
    {
        cancelPendingUpdate();
        stopThread (10000);
        disconnect();
    }

    using ChildProcessPingThread::startPinging;

private:
    ChildProcessWorker& owner;

    z0 connectionMade() override  {}
    z0 connectionLost() override  { owner.handleConnectionLost(); }

    b8 sendPingMessage (const MemoryBlock& m) override    { return owner.sendMessageToCoordinator (m); }
    z0 pingFailed() override                              { connectionLost(); }

    z0 messageReceived (const MemoryBlock& m) override
    {
        pingReceived();

        if (isMessageType (m, pingMessage))
            return;

        if (isMessageType (m, killMessage))
            return triggerConnectionLostMessage();

        if (isMessageType (m, startMessage))
            return owner.handleConnectionMade();

        owner.handleMessageFromCoordinator (m);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};

//==============================================================================
ChildProcessWorker::ChildProcessWorker() = default;
ChildProcessWorker::~ChildProcessWorker() = default;

z0 ChildProcessWorker::handleConnectionMade() {}
z0 ChildProcessWorker::handleConnectionLost() {}

z0 ChildProcessWorker::handleMessageFromCoordinator (const MemoryBlock& mb)
{
    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    handleMessageFromMaster (mb);
    DRX_END_IGNORE_DEPRECATION_WARNINGS
}

b8 ChildProcessWorker::sendMessageToCoordinator (const MemoryBlock& mb)
{
    if (connection != nullptr)
        return connection->sendMessage (mb);

    jassertfalse; // this can only be used when the connection is active!
    return false;
}

b8 ChildProcessWorker::initialiseFromCommandLine (const Txt& commandLine,
                                                    const Txt& commandLineUniqueID,
                                                    i32 timeoutMs)
{
    auto prefix = getCommandLinePrefix (commandLineUniqueID);

    if (commandLine.trim().startsWith (prefix))
    {
        auto pipeName = commandLine.fromFirstOccurrenceOf (prefix, false, false)
                                   .upToFirstOccurrenceOf (" ", false, false).trim();

        if (pipeName.isNotEmpty())
        {
            connection.reset (new Connection (*this, pipeName, timeoutMs <= 0 ? defaultTimeoutMs : timeoutMs));

            if (connection->isConnected())
                connection->startPinging();
            else
                connection.reset();
        }
    }

    return connection != nullptr;
}

} // namespace drx
