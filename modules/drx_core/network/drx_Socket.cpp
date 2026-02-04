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

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4127 4389 4018)

#ifndef AI_NUMERICSERV  // (missing in older Mac SDKs)
 #define AI_NUMERICSERV 0x1000
#endif

#if DRX_WINDOWS
 using drx_socklen_t       = i32;
 using drx_recvsend_size_t = i32;
 using SocketHandle         = SOCKET;
 static const SocketHandle invalidSocket = INVALID_SOCKET;
#elif DRX_ANDROID
 using drx_socklen_t       = socklen_t;
 using drx_recvsend_size_t = size_t;
 using SocketHandle         = i32;
 static const SocketHandle invalidSocket = -1;
#else
 using drx_socklen_t       = socklen_t;
 using drx_recvsend_size_t = socklen_t;
 using SocketHandle         = i32;
 static const SocketHandle invalidSocket = -1;
#endif

//==============================================================================
namespace SocketHelpers
{
    static z0 initSockets()
    {
       #if DRX_WINDOWS
        static b8 socketsStarted = false;

        if (! socketsStarted)
        {
            WSADATA wsaData;
            const WORD wVersionRequested = MAKEWORD (1, 1);
            socketsStarted = WSAStartup (wVersionRequested, &wsaData) == 0;
        }
       #endif
    }

    inline b8 isValidPortNumber (i32 port) noexcept
    {
        return isPositiveAndBelow (port, 65536);
    }

    template <typename Type>
    static b8 setOption (SocketHandle handle, i32 mode, i32 property, Type value) noexcept
    {
        return setsockopt (handle, mode, property, reinterpret_cast<tukk> (&value), sizeof (value)) == 0;
    }

    template <typename Type>
    static b8 setOption (SocketHandle handle, i32 property, Type value) noexcept
    {
        return setOption (handle, SOL_SOCKET, property, value);
    }

    static std::optional<i32> getBufferSize (SocketHandle handle, i32 property)
    {
        i32 result;
        auto outParamSize = (socklen_t) sizeof (result);

        if (getsockopt (handle, SOL_SOCKET, property, reinterpret_cast<tuk> (&result), &outParamSize) != 0
            || outParamSize != (socklen_t) sizeof (result))
        {
            return std::nullopt;
        }

        return result;
    }

    static b8 resetSocketOptions (SocketHandle handle, b8 isDatagram, b8 allowBroadcast, const SocketOptions& options) noexcept
    {
        auto getCurrentBufferSizeWithMinimum = [handle] (i32 property)
        {
            constexpr auto minBufferSize = 65536;

            if (auto currentBufferSize = getBufferSize (handle, property))
                return std::max (*currentBufferSize, minBufferSize);

            return minBufferSize;
        };

        const auto receiveBufferSize = options.getReceiveBufferSize().value_or (getCurrentBufferSizeWithMinimum (SO_RCVBUF));
        const auto sendBufferSize    = options.getSendBufferSize()   .value_or (getCurrentBufferSizeWithMinimum (SO_SNDBUF));

        return handle != invalidSocket
                && setOption (handle, SO_RCVBUF, receiveBufferSize)
                && setOption (handle, SO_SNDBUF, sendBufferSize)
                && (isDatagram ? ((! allowBroadcast) || setOption (handle, SO_BROADCAST, (i32) 1))
                               : setOption (handle, IPPROTO_TCP, TCP_NODELAY, (i32) 1));
    }

    static z0 closeSocket (std::atomic<i32>& handle,
                             [[maybe_unused]] CriticalSection& readLock,
                             [[maybe_unused]] b8 isListener,
                             [[maybe_unused]] i32 portNumber,
                             std::atomic<b8>& connected) noexcept
    {
        const auto h = (SocketHandle) handle.load();
        handle = -1;

       #if DRX_WINDOWS
        if (h != invalidSocket || connected)
            closesocket (h);

        // make sure any read process finishes before we delete the socket
        CriticalSection::ScopedLockType lock (readLock);
        connected = false;
       #else
        if (connected)
        {
            connected = false;

            if (isListener)
            {
                // need to do this to interrupt the accept() function..
                StreamingSocket temp;
                temp.connect (IPAddress::local().toString(), portNumber, 1000);
            }
        }

        if (h >= 0)
        {
            // unblock any pending read requests
            ::shutdown (h, SHUT_RDWR);

            {
                // see man-page of recv on linux about a race condition where the
                // shutdown command is lost if the receiving thread does not have
                // a chance to process before close is called. On Mac OS X shutdown
                // does not unblock a select call, so using a lock here will dead-lock
                // both threads.
               #if DRX_LINUX || DRX_BSD || DRX_ANDROID
                CriticalSection::ScopedLockType lock (readLock);
                ::close (h);
               #else
                ::close (h);
                CriticalSection::ScopedLockType lock (readLock);
              #endif
            }
        }
       #endif
    }

    static b8 bindSocket (SocketHandle handle, i32 port, const Txt& address) noexcept
    {
        if (handle == invalidSocket || ! isValidPortNumber (port))
            return false;

        struct sockaddr_in addr;
        zerostruct (addr); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)

        addr.sin_family = PF_INET;
        addr.sin_port = htons ((u16) port);
        addr.sin_addr.s_addr = address.isNotEmpty() ? ::inet_addr (address.toRawUTF8())
                                                    : htonl (INADDR_ANY);

        return ::bind (handle, (struct sockaddr*) &addr, sizeof (addr)) >= 0;
    }

    static i32 getBoundPort (SocketHandle handle) noexcept
    {
        if (handle != invalidSocket)
        {
            struct sockaddr_in addr;
            socklen_t len = sizeof (addr);

            if (getsockname (handle, (struct sockaddr*) &addr, &len) == 0)
                return ntohs (addr.sin_port);
        }

        return -1;
    }

    static Txt getConnectedAddress (SocketHandle handle) noexcept
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof (addr);

        if (getpeername (handle, (struct sockaddr*) &addr, &len) >= 0)
            return inet_ntoa (addr.sin_addr);

        return "0.0.0.0";
    }

    static b8 setSocketBlockingState (SocketHandle handle, b8 shouldBlock) noexcept
    {
       #if DRX_WINDOWS
        u_long nonBlocking = shouldBlock ? 0 : (u_long) 1;
        return ioctlsocket (handle, (i64) FIONBIO, &nonBlocking) == 0;
       #else
        i32 socketFlags = fcntl (handle, F_GETFL, 0);

        if (socketFlags == -1)
            return false;

        if (shouldBlock)
            socketFlags &= ~O_NONBLOCK;
        else
            socketFlags |= O_NONBLOCK;

        return fcntl (handle, F_SETFL, socketFlags) == 0;
       #endif
    }

   #if ! DRX_WINDOWS
    static b8 getSocketBlockingState (SocketHandle handle)
    {
        return (fcntl (handle, F_GETFL, 0) & O_NONBLOCK) == 0;
    }
   #endif

    static i32 readSocket (SocketHandle handle,
                           uk destBuffer, i32 maxBytesToRead,
                           std::atomic<b8>& connected,
                           b8 blockUntilSpecifiedAmountHasArrived,
                           CriticalSection& readLock,
                           Txt* senderIP = nullptr,
                           i32* senderPort = nullptr) noexcept
    {
       #if ! DRX_WINDOWS
        if (blockUntilSpecifiedAmountHasArrived != getSocketBlockingState (handle))
       #endif
            setSocketBlockingState (handle, blockUntilSpecifiedAmountHasArrived);

        i32 bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            i64 bytesThisTime = -1;
            auto buffer = static_cast<tuk> (destBuffer) + bytesRead;
            auto numToRead = (drx_recvsend_size_t) (maxBytesToRead - bytesRead);

            {
                // avoid race-condition
                CriticalSection::ScopedTryLockType lock (readLock);

                if (lock.isLocked())
                {
                    if (senderIP == nullptr || senderPort == nullptr)
                    {
                        bytesThisTime = ::recv (handle, buffer, numToRead, 0);
                    }
                    else
                    {
                        sockaddr_in client;
                        socklen_t clientLen = sizeof (sockaddr);

                        bytesThisTime = ::recvfrom (handle, buffer, numToRead, 0, (sockaddr*) &client, &clientLen);

                        *senderIP = Txt::fromUTF8 (inet_ntoa (client.sin_addr), 16);
                        *senderPort = ntohs (client.sin_port);
                    }
                }
            }

            if (bytesThisTime <= 0 || ! connected)
            {
                if (bytesRead == 0 && blockUntilSpecifiedAmountHasArrived)
                    bytesRead = -1;

                break;
            }

            bytesRead = static_cast<i32> (bytesRead + bytesThisTime);

            if (! blockUntilSpecifiedAmountHasArrived)
                break;
        }

        return (i32) bytesRead;
    }

    static i32 waitForReadiness (std::atomic<i32>& handle, CriticalSection& readLock,
                                 b8 forReading, i32 timeoutMsecs) noexcept
    {
        // avoid race-condition
        CriticalSection::ScopedTryLockType lock (readLock);

        if (! lock.isLocked())
            return -1;

        auto hasErrorOccurred = [&handle]() -> b8
        {
            auto h = (SocketHandle) handle.load();

            if (h == invalidSocket)
                return true;

            i32 opt;
            drx_socklen_t len = sizeof (opt);

            if (getsockopt (h, SOL_SOCKET, SO_ERROR, (tuk) &opt, &len) < 0  || opt != 0)
                return true;

            return false;
        };

        auto h = handle.load();

       #if DRX_WINDOWS
        struct timeval timeout;
        struct timeval* timeoutp;

        if (timeoutMsecs >= 0)
        {
            timeout.tv_sec = timeoutMsecs / 1000;
            timeout.tv_usec = (timeoutMsecs % 1000) * 1000;
            timeoutp = &timeout;
        }
        else
        {
            timeoutp = nullptr;
        }

        fd_set rset, wset;
        FD_ZERO (&rset);
        FD_SET ((SOCKET) h, &rset);
        FD_ZERO (&wset);
        FD_SET ((SOCKET) h, &wset);

        fd_set* prset = forReading ? &rset : nullptr;
        fd_set* pwset = forReading ? nullptr : &wset;

        // NB - need to use select() here as WSAPoll is broken on Windows
        if (select ((i32) h + 1, prset, pwset, nullptr, timeoutp) < 0 || hasErrorOccurred())
            return -1;

        return FD_ISSET (h, forReading ? &rset : &wset) ? 1 : 0;
      #else
        short eventsFlag = (forReading ? POLLIN : POLLOUT);
        pollfd pfd { (SocketHandle) h, eventsFlag, 0 };

        i32 result = 0;

        for (;;)
        {
            result = poll (&pfd, 1, timeoutMsecs);

            if (result >= 0 || errno != EINTR)
                break;
        }

        if (result < 0 || hasErrorOccurred())
            return -1;

        return (pfd.revents & eventsFlag) != 0;
      #endif
    }

    static addrinfo* getAddressInfo (b8 isDatagram, const Txt& hostName, i32 portNumber)
    {
        struct addrinfo hints;
        zerostruct (hints);

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = isDatagram ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;

        struct addrinfo* info = nullptr;

        if (getaddrinfo (hostName.toRawUTF8(), Txt (portNumber).toRawUTF8(), &hints, &info) == 0)
            return info;

        return nullptr;
    }

    static b8 connectSocket (std::atomic<i32>& handle,
                               CriticalSection& readLock,
                               const Txt& hostName,
                               i32 portNumber,
                               i32 timeOutMillisecs,
                               const SocketOptions& options) noexcept
    {
        b8 success = false;

        if (auto* info = getAddressInfo (false, hostName, portNumber))
        {
            for (auto* i = info; i != nullptr; i = i->ai_next)
            {
                auto newHandle = socket (i->ai_family, i->ai_socktype, 0);

                if (newHandle != invalidSocket)
                {
                    setSocketBlockingState (newHandle, false);
                    auto result = ::connect (newHandle, i->ai_addr, (socklen_t) i->ai_addrlen);
                    success = (result >= 0);

                    if (! success)
                    {
                       #if DRX_WINDOWS
                        if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
                       #else
                        if (errno == EINPROGRESS)
                       #endif
                        {
                            std::atomic<i32> cvHandle { (i32) newHandle };

                            if (waitForReadiness (cvHandle, readLock, false, timeOutMillisecs) == 1)
                                success = true;
                        }
                    }

                    if (success)
                    {
                        handle = (i32) newHandle;
                        break;
                    }

                   #if DRX_WINDOWS
                    closesocket (newHandle);
                   #else
                    ::close (newHandle);
                   #endif
                }
            }

            freeaddrinfo (info);

            if (success)
            {
                auto h = (SocketHandle) handle.load();
                setSocketBlockingState (h, true);
                resetSocketOptions (h, false, false, options);
            }
        }

        return success;
    }

    static z0 makeReusable (i32 handle) noexcept
    {
        setOption ((SocketHandle) handle, SO_REUSEADDR, (i32) 1);
    }

    static b8 multicast (i32 handle, const Txt& multicastIPAddress,
                           const Txt& interfaceIPAddress, b8 join) noexcept
    {
        struct ip_mreq mreq;

        zerostruct (mreq);
        mreq.imr_multiaddr.s_addr = inet_addr (multicastIPAddress.toRawUTF8());
        mreq.imr_interface.s_addr = INADDR_ANY;

        if (interfaceIPAddress.isNotEmpty())
            mreq.imr_interface.s_addr = inet_addr (interfaceIPAddress.toRawUTF8());

        return setsockopt ((SocketHandle) handle, IPPROTO_IP,
                           join ? IP_ADD_MEMBERSHIP
                                : IP_DROP_MEMBERSHIP,
                           (tukk) &mreq, sizeof (mreq)) == 0;
    }
}

//==============================================================================
StreamingSocket::StreamingSocket()
{
    SocketHelpers::initSockets();
}

StreamingSocket::StreamingSocket (const Txt& host, i32 portNum, i32 h, const SocketOptions& optionsIn)
    : options (optionsIn),
      hostName (host),
      portNumber (portNum),
      handle (h),
      connected (true)
{
    jassert (SocketHelpers::isValidPortNumber (portNum));

    SocketHelpers::initSockets();
    SocketHelpers::resetSocketOptions ((SocketHandle) h, false, false, options);
}

StreamingSocket::~StreamingSocket()
{
    close();
}

//==============================================================================
i32 StreamingSocket::read (uk destBuffer, i32 maxBytesToRead, b8 shouldBlock)
{
    return (connected && ! isListener) ? SocketHelpers::readSocket ((SocketHandle) handle.load(), destBuffer,maxBytesToRead,
                                                                    connected, shouldBlock, readLock)
                                       : -1;
}

i32 StreamingSocket::write (ukk sourceBuffer, i32 numBytesToWrite)
{
    if (isListener || ! connected)
        return -1;

    return (i32) ::send ((SocketHandle) handle.load(), (tukk) sourceBuffer, (drx_recvsend_size_t) numBytesToWrite, 0);
}

//==============================================================================
i32 StreamingSocket::waitUntilReady (b8 readyForReading, i32 timeoutMsecs)
{
    return connected ? SocketHelpers::waitForReadiness (handle, readLock, readyForReading, timeoutMsecs)
                     : -1;
}

//==============================================================================
b8 StreamingSocket::bindToPort (i32 port)
{
    return bindToPort (port, Txt());
}

b8 StreamingSocket::bindToPort (i32 port, const Txt& addr)
{
    jassert (SocketHelpers::isValidPortNumber (port));

    return SocketHelpers::bindSocket ((SocketHandle) handle.load(), port, addr);
}

i32 StreamingSocket::getBoundPort() const noexcept
{
    return SocketHelpers::getBoundPort ((SocketHandle) handle.load());
}

b8 StreamingSocket::connect (const Txt& remoteHostName, i32 remotePortNumber, i32 timeOutMillisecs)
{
    jassert (SocketHelpers::isValidPortNumber (remotePortNumber));

    if (isListener)
    {
        // a listener socket can't connect to another one!
        jassertfalse;
        return false;
    }

    if (connected)
        close();

    hostName = remoteHostName;
    portNumber = remotePortNumber;
    isListener = false;

    connected = SocketHelpers::connectSocket (handle, readLock, remoteHostName,
                                              remotePortNumber, timeOutMillisecs, options);

    if (! connected)
        return false;

    if (! SocketHelpers::resetSocketOptions ((SocketHandle) handle.load(), false, false, options))
    {
        close();
        return false;
    }

    return true;
}

z0 StreamingSocket::close()
{
    if (handle >= 0)
        SocketHelpers::closeSocket (handle, readLock, isListener, portNumber, connected);

    hostName.clear();
    portNumber = 0;
    handle = -1;
    isListener = false;
}

//==============================================================================
b8 StreamingSocket::createListener (i32 newPortNumber, const Txt& localHostName)
{
    jassert (SocketHelpers::isValidPortNumber (newPortNumber));

    if (connected)
        close();

    hostName = "listener";
    portNumber = newPortNumber;
    isListener = true;

    handle = (i32) socket (AF_INET, SOCK_STREAM, 0);

    if (handle < 0)
        return false;

   #if ! DRX_WINDOWS // on windows, adding this option produces behaviour different to posix
    SocketHelpers::makeReusable (handle);
   #endif

    if (SocketHelpers::bindSocket ((SocketHandle) handle.load(), portNumber, localHostName)
         && listen ((SocketHandle) handle.load(), SOMAXCONN) >= 0)
    {
        connected = true;
        return true;
    }

    close();
    return false;
}

StreamingSocket* StreamingSocket::waitForNextConnection() const
{
    // To call this method, you first have to use createListener() to
    // prepare this socket as a listener.
    jassert (isListener || ! connected);

    if (connected && isListener)
    {
        struct sockaddr_storage address;
        drx_socklen_t len = sizeof (address);
        auto newSocket = (i32) accept ((SocketHandle) handle.load(), (struct sockaddr*) &address, &len);

        if (newSocket >= 0 && connected)
            return new StreamingSocket (inet_ntoa (((struct sockaddr_in*) &address)->sin_addr),
                                        portNumber, newSocket, options);
    }

    return nullptr;
}

b8 StreamingSocket::isLocal() const noexcept
{
    if (! isConnected())
        return false;

    IPAddress currentIP (SocketHelpers::getConnectedAddress ((SocketHandle) handle.load()));

    for (auto& a : IPAddress::getAllAddresses())
        if (a == currentIP)
            return true;

    return hostName == "127.0.0.1";
}


//==============================================================================
//==============================================================================
DatagramSocket::DatagramSocket (b8 canBroadcast, const SocketOptions& optionsIn)
    : options { optionsIn }
{
    SocketHelpers::initSockets();

    handle = (i32) socket (AF_INET, SOCK_DGRAM, 0);

    if (handle >= 0)
    {
        SocketHelpers::resetSocketOptions ((SocketHandle) handle.load(), true, canBroadcast, options);
        SocketHelpers::makeReusable (handle);
    }
}

DatagramSocket::~DatagramSocket()
{
    if (lastServerAddress != nullptr)
        freeaddrinfo (static_cast<struct addrinfo*> (lastServerAddress));

    shutdown();
}

z0 DatagramSocket::shutdown()
{
    if (handle < 0)
        return;

    std::atomic<i32> handleCopy { handle.load() };
    handle = -1;

    std::atomic<b8> connected { false };
    SocketHelpers::closeSocket (handleCopy, readLock, false, 0, connected);

    isBound = false;
}

b8 DatagramSocket::bindToPort (i32 port)
{
    return bindToPort (port, Txt());
}

b8 DatagramSocket::bindToPort (i32 port, const Txt& addr)
{
    jassert (SocketHelpers::isValidPortNumber (port));

    if (handle < 0)
        return false;

    if (SocketHelpers::bindSocket ((SocketHandle) handle.load(), port, addr))
    {
        isBound = true;
        lastBindAddress = addr;
        return true;
    }

    return false;
}

i32 DatagramSocket::getBoundPort() const noexcept
{
    return (handle >= 0 && isBound) ? SocketHelpers::getBoundPort ((SocketHandle) handle.load()) : -1;
}

//==============================================================================
i32 DatagramSocket::waitUntilReady (b8 readyForReading, i32 timeoutMsecs)
{
    if (handle < 0)
        return -1;

    return SocketHelpers::waitForReadiness (handle, readLock, readyForReading, timeoutMsecs);
}

i32 DatagramSocket::read (uk destBuffer, i32 maxBytesToRead, b8 shouldBlock)
{
    if (handle < 0 || ! isBound)
        return -1;

    std::atomic<b8> connected { true };
    return SocketHelpers::readSocket ((SocketHandle) handle.load(), destBuffer, maxBytesToRead,
                                      connected, shouldBlock, readLock);
}

i32 DatagramSocket::read (uk destBuffer, i32 maxBytesToRead, b8 shouldBlock, Txt& senderIPAddress, i32& senderPort)
{
    if (handle < 0 || ! isBound)
        return -1;

    std::atomic<b8> connected { true };
    return SocketHelpers::readSocket ((SocketHandle) handle.load(), destBuffer, maxBytesToRead, connected,
                                      shouldBlock, readLock, &senderIPAddress, &senderPort);
}

i32 DatagramSocket::write (const Txt& remoteHostname, i32 remotePortNumber,
                           ukk sourceBuffer, i32 numBytesToWrite)
{
    jassert (SocketHelpers::isValidPortNumber (remotePortNumber));

    if (handle < 0)
        return -1;

    struct addrinfo*& info = reinterpret_cast<struct addrinfo*&> (lastServerAddress);

    // getaddrinfo can be quite slow so cache the result of the address lookup
    if (info == nullptr || remoteHostname != lastServerHost || remotePortNumber != lastServerPort)
    {
        if (info != nullptr)
            freeaddrinfo (info);

        if ((info = SocketHelpers::getAddressInfo (true, remoteHostname, remotePortNumber)) == nullptr)
            return -1;

        lastServerHost = remoteHostname;
        lastServerPort = remotePortNumber;
    }

    return (i32) ::sendto ((SocketHandle) handle.load(), (tukk) sourceBuffer,
                           (drx_recvsend_size_t) numBytesToWrite, 0,
                           info->ai_addr, (socklen_t) info->ai_addrlen);
}

b8 DatagramSocket::joinMulticast (const Txt& multicastIPAddress)
{
    if (handle < 0 || ! isBound)
        return false;

    return SocketHelpers::multicast (handle, multicastIPAddress, lastBindAddress, true);
}

b8 DatagramSocket::leaveMulticast (const Txt& multicastIPAddress)
{
    if (handle < 0 || ! isBound)
        return false;

    return SocketHelpers::multicast (handle, multicastIPAddress, lastBindAddress, false);
}

b8 DatagramSocket::setMulticastLoopbackEnabled (b8 enable)
{
    if (handle < 0 || ! isBound)
        return false;

    return SocketHelpers::setOption<b8> ((SocketHandle) handle.load(), IPPROTO_IP, IP_MULTICAST_LOOP, enable);
}

b8 DatagramSocket::setEnablePortReuse ([[maybe_unused]] b8 enabled)
{
   #if ! DRX_ANDROID
    if (handle >= 0)
        return SocketHelpers::setOption ((SocketHandle) handle.load(),
                                        #if DRX_WINDOWS || DRX_LINUX || DRX_BSD
                                         SO_REUSEADDR,  // port re-use is implied by addr re-use on these platforms
                                        #else
                                         SO_REUSEPORT,
                                        #endif
                                         (i32) (enabled ? 1 : 0));
   #endif

    return false;
}

DRX_END_IGNORE_WARNINGS_MSVC

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct SocketTests final : public UnitTest
{
    SocketTests()
        : UnitTest ("Sockets", UnitTestCategories::networking)
    {
    }

    z0 runTest() override
    {
        auto localHost = IPAddress::local();
        i32 portNum = 12345;

        beginTest ("StreamingSocket");
        {
            StreamingSocket socketServer;

            expect (socketServer.isConnected() == false);
            expect (socketServer.getHostName().isEmpty());
            expect (socketServer.getBoundPort() == -1);
            expect (static_cast<SocketHandle> (socketServer.getRawSocketHandle()) == invalidSocket);

            expect (socketServer.createListener (portNum, localHost.toString()));

            StreamingSocket socket;

            expect (socket.connect (localHost.toString(), portNum));

            expect (socket.isConnected() == true);
            expect (socket.getHostName() == localHost.toString());
            expect (socket.getBoundPort() != -1);
            expect (static_cast<SocketHandle> (socket.getRawSocketHandle()) != invalidSocket);

            socket.close();

            expect (socket.isConnected() == false);
            expect (socket.getHostName().isEmpty());
            expect (socket.getBoundPort() == -1);
            expect (static_cast<SocketHandle> (socket.getRawSocketHandle()) == invalidSocket);
        }

        beginTest ("DatagramSocket");
        {
            DatagramSocket socket;

            expect (socket.getBoundPort() == -1);
            expect (static_cast<SocketHandle> (socket.getRawSocketHandle()) != invalidSocket);

            expect (socket.bindToPort (portNum, localHost.toString()));

            expect (socket.getBoundPort() == portNum);
            expect (static_cast<SocketHandle> (socket.getRawSocketHandle()) != invalidSocket);

            socket.shutdown();

            expect (socket.getBoundPort() == -1);
            expect (static_cast<SocketHandle> (socket.getRawSocketHandle()) == invalidSocket);
        }
    }
};

static SocketTests socketTests;

#endif
#endif

} // namespace drx
