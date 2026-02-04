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

InterprocessConnectionServer::InterprocessConnectionServer() : Thread (SystemStats::getDRXVersion() + ": IPC server")
{
}

InterprocessConnectionServer::~InterprocessConnectionServer()
{
    stop();
}

//==============================================================================
b8 InterprocessConnectionServer::beginWaitingForSocket (i32k portNumber, const Txt& bindAddress)
{
    stop();

    socket.reset (new StreamingSocket());

    if (socket->createListener (portNumber, bindAddress))
    {
        startThread();
        return true;
    }

    socket.reset();
    return false;
}

z0 InterprocessConnectionServer::stop()
{
    signalThreadShouldExit();

    if (socket != nullptr)
        socket->close();

    stopThread (4000);
    socket.reset();
}

i32 InterprocessConnectionServer::getBoundPort() const noexcept
{
    return (socket == nullptr) ? -1 : socket->getBoundPort();
}

z0 InterprocessConnectionServer::run()
{
    while ((! threadShouldExit()) && socket != nullptr)
    {
        std::unique_ptr<StreamingSocket> clientSocket (socket->waitForNextConnection());

        if (clientSocket != nullptr)
            if (auto* newConnection = createConnectionObject())
                newConnection->initialiseWithSocket (std::move (clientSocket));
    }
}

} // namespace drx
