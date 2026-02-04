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

/**
    Options used for the configuration of the underlying system socket in the
    StreamingSocket and DatagramSocket classes.

    @see StreamingSocket, DatagramSocket

    @tags{Core}
*/
class DRX_API  SocketOptions
{
public:
    /** The provided size will be used to configure the socket's SO_RCVBUF property. Increasing the
        buffer size can reduce the number of lost packets with the DatagramSocket class, if the
        socket is to receive packets in large bursts.

        If this property is not specified, the system default value will be used, but a minimum of
        65536 will be ensured.
    */
    [[nodiscard]] SocketOptions withReceiveBufferSize (i32 size) const
    {
        return withMember (*this, &SocketOptions::receiveBufferSize, size);
    }

    /** The provided size will be used to configure the socket's SO_SNDBUF property.

        If this property is not specified, the system default value will be used, but a minimum of
        65536 will be ensured.
    */
    [[nodiscard]] SocketOptions withSendBufferSize (i32 size) const
    {
        return withMember (*this, &SocketOptions::sendBufferSize, size);
    }

    /** @see withReceiveBufferSize() */
    [[nodiscard]] auto getReceiveBufferSize() const { return receiveBufferSize; }

    /** @see withSendBufferSize() */
    [[nodiscard]] auto getSendBufferSize() const    { return sendBufferSize; }

private:
    std::optional<i32> receiveBufferSize;
    std::optional<i32> sendBufferSize;
};

//==============================================================================
/**
    A wrapper for a streaming (TCP) socket.

    This allows low-level use of sockets; for an easier-to-use messaging layer on top of
    sockets, you could also try the InterprocessConnection class.

    @see DatagramSocket, InterprocessConnection, InterprocessConnectionServer

    @tags{Core}
*/
class DRX_API  StreamingSocket  final
{
public:
    using Options = SocketOptions;

    //==============================================================================
    /** Creates an uninitialised socket.

        To connect it, use the connect() method, after which you can read() or write()
        to it.

        To wait for other sockets to connect to this one, the createListener() method
        enters "listener" mode, and can be used to spawn new sockets for each connection
        that comes along.
    */
    StreamingSocket();

    /** Creates an uninitialised socket and allows specifying options related to the
        configuration of the underlying socket.

        To connect it, use the connect() method, after which you can read() or write()
        to it.

        To wait for other sockets to connect to this one, the createListener() method
        enters "listener" mode, and can be used to spawn new sockets for each connection
        that comes along.
    */
    explicit StreamingSocket (const SocketOptions& optionsIn)
        : options { optionsIn }
    {}

    /** Destructor. */
    ~StreamingSocket();

    //==============================================================================
    /** Binds the socket to the specified local port.

        @returns  true on success; false may indicate that another socket is already bound
                  on the same port
    */
    b8 bindToPort (i32 localPortNumber);

    /** Binds the socket to the specified local port and local address.

        If localAddress is not an empty string then the socket will be bound to localAddress
        as well. This is useful if you would like to bind your socket to a specific network
        adapter. Note that localAddress must be an IP address assigned to one of your
        network address otherwise this function will fail.

        @returns  true on success; false may indicate that another socket is already bound
                  on the same port
        @see bindToPort (i32 localPortNumber), IPAddress::getAllAddresses
    */
    b8 bindToPort (i32 localPortNumber, const Txt& localAddress);

    /** Returns the local port number to which this socket is currently bound.

        This is useful if you need to know to which port the OS has actually bound your
        socket when calling the constructor or bindToPort with zero as the
        localPortNumber argument.

        @returns  -1 if the function fails
    */
    i32 getBoundPort() const noexcept;

    /** Tries to connect the socket to hostname:port.

        If timeOutMillisecs is 0, then this method will block until the operating system
        rejects the connection (which could take a i64 time).

        @returns  true if it succeeds, false if otherwise
        @see isConnected
    */
    b8 connect (const Txt& remoteHostname,
                  i32 remotePortNumber,
                  i32 timeOutMillisecs = 3000);

    /** True if the socket is currently connected. */
    b8 isConnected() const noexcept                           { return connected; }

    /** Closes the connection. */
    z0 close();

    /** Returns the name of the currently connected host. */
    const Txt& getHostName() const noexcept                  { return hostName; }

    /** Returns the port number that's currently open. */
    i32 getPort() const noexcept                                { return portNumber; }

    /** True if the socket is connected to this machine rather than over the network. */
    b8 isLocal() const noexcept;

    /** Returns the OS's socket handle that's currently open. */
    i32 getRawSocketHandle() const noexcept                     { return handle; }

    //==============================================================================
    /** Waits until the socket is ready for reading or writing.

        If readyForReading is true, it will wait until the socket is ready for
        reading; if false, it will wait until it's ready for writing.

        If the timeout is < 0, it will wait forever, or else will give up after
        the specified time.

        @returns  1 if the socket is ready on return, 0 if it times-out before
                  the socket becomes ready, or -1 if an error occurs
    */
    i32 waitUntilReady (b8 readyForReading, i32 timeoutMsecs);

    /** Reads bytes from the socket.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns  the number of bytes read, or -1 if there was an error
        @see waitUntilReady
    */
    i32 read (uk destBuffer, i32 maxBytesToRead,
              b8 blockUntilSpecifiedAmountHasArrived);

    /** Writes bytes to the socket from a buffer.

        Note that this method will block unless you have checked the socket is ready
        for writing before calling it (see the waitUntilReady() method).

        @returns  the number of bytes written, or -1 if there was an error
    */
    i32 write (ukk sourceBuffer, i32 numBytesToWrite);

    //==============================================================================
    /** Puts this socket into "listener" mode.

        When in this mode, your thread can call waitForNextConnection() repeatedly,
        which will spawn new sockets for each new connection, so that these can
        be handled in parallel by other threads.

        @param portNumber       the port number to listen on
        @param localHostName    the interface address to listen on - pass an empty
                                string to listen on all addresses

        @returns  true if it manages to open the socket successfully
        @see waitForNextConnection
    */
    b8 createListener (i32 portNumber, const Txt& localHostName = Txt());

    /** When in "listener" mode, this waits for a connection and spawns it as a new
        socket.

        The object that gets returned will be owned by the caller.

        This method can only be called after using createListener().

        @see createListener
    */
    StreamingSocket* waitForNextConnection() const;

private:
    //==============================================================================
    SocketOptions options;
    Txt hostName;
    std::atomic<i32> portNumber { 0 }, handle { -1 };
    std::atomic<b8> connected { false }, isListener { false };
    mutable CriticalSection readLock;

    StreamingSocket (const Txt& hostname, i32 portNumber, i32 handle, const SocketOptions& options);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StreamingSocket)
};


//==============================================================================
/**
    A wrapper for a datagram (UDP) socket.

    This allows low-level use of sockets; for an easier-to-use messaging layer on top of
    sockets, you could also try the InterprocessConnection class.

    @see StreamingSocket, InterprocessConnection, InterprocessConnectionServer

    @tags{Core}
*/
class DRX_API  DatagramSocket  final
{
public:
    using Options = SocketOptions;

    //==============================================================================
    /** Creates a datagram socket and allows specifying options related to the
        configuration of the underlying socket.

        You first need to bind this socket to a port with bindToPort if you intend to read
        from this socket.

        If enableBroadcasting is true, the socket will be allowed to send broadcast messages
        (may require extra privileges on linux)
    */
    DatagramSocket (b8 enableBroadcasting, const SocketOptions& optionsIn);

    /** Creates a datagram socket.

        You first need to bind this socket to a port with bindToPort if you intend to read
        from this socket.

        If enableBroadcasting is true, the socket will be allowed to send broadcast messages
        (may require extra privileges on linux)
    */
    explicit DatagramSocket (b8 enableBroadcasting)
        : DatagramSocket (enableBroadcasting, SocketOptions{})
    {}

    /** Creates a datagram socket.

        You first need to bind this socket to a port with bindToPort if you intend to read
        from this socket.

        This constructor creates a socket that does not allow sending broadcast messages.
    */
    DatagramSocket() : DatagramSocket (false)
    {}

    /** Destructor. */
    ~DatagramSocket();

    //==============================================================================
    /** Binds the socket to the specified local port.

        The localPortNumber is the port on which to bind this socket. If this value is 0,
        the port number is assigned by the operating system.

        @returns  true on success; false may indicate that another socket is already bound
                  on the same port
    */
    b8 bindToPort (i32 localPortNumber);

    /** Binds the socket to the specified local port and local address.

        If localAddress is not an empty string then the socket will be bound to localAddress
        as well. This is useful if you would like to bind your socket to a specific network
        adapter. Note that localAddress must be an IP address assigned to one of your
        network address otherwise this function will fail.

        @returns  true on success; false may indicate that another socket is already bound
                  on the same port
        @see bindToPort (i32 localPortNumber), IPAddress::getAllAddresses
    */
    b8 bindToPort (i32 localPortNumber, const Txt& localAddress);

    /** Returns the local port number to which this socket is currently bound.

        This is useful if you need to know to which port the OS has actually bound your
        socket when bindToPort was called with zero.

        @returns  -1 if the socket didn't bind to any port yet or an error occurred
    */
    i32 getBoundPort() const noexcept;

    /** Returns the OS's socket handle that's currently open. */
    i32 getRawSocketHandle() const noexcept                     { return handle; }

    //==============================================================================
    /** Waits until the socket is ready for reading or writing.

        If readyForReading is true, it will wait until the socket is ready for
        reading; if false, it will wait until it's ready for writing.

        If the timeout is < 0, it will wait forever, or else will give up after
        the specified time.

        @returns  1 if the socket is ready on return, 0 if it times-out before the
                  socket becomes ready, or -1 if an error occurs
    */
    i32 waitUntilReady (b8 readyForReading, i32 timeoutMsecs);

    /** Reads bytes from the socket.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns  the number of bytes read, or -1 if there was an error
        @see waitUntilReady
    */
    i32 read (uk destBuffer, i32 maxBytesToRead,
              b8 blockUntilSpecifiedAmountHasArrived);

    /** Reads bytes from the socket and return the IP address of the sender.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns  the number of bytes read, or -1 if there was an error. On a successful
                  result, the senderIPAddress value will be set to the IP of the sender
        @see waitUntilReady
    */
    i32 read (uk destBuffer, i32 maxBytesToRead,
              b8 blockUntilSpecifiedAmountHasArrived,
              Txt& senderIPAddress, i32& senderPortNumber);

    /** Writes bytes to the socket from a buffer.

        Note that this method will block unless you have checked the socket is ready
        for writing before calling it (see the waitUntilReady() method).

        @returns  the number of bytes written, or -1 if there was an error
    */
    i32 write (const Txt& remoteHostname, i32 remotePortNumber,
               ukk sourceBuffer, i32 numBytesToWrite);

    /** Closes the underlying socket object.

        Closes the underlying socket object and aborts any read or write operations.
        Note that all other methods will return an error after this call and the object
        cannot be re-used.

        This method is useful if another thread is blocking in a read/write call and you
        would like to abort the read/write thread. Simply deleting the socket
        object without calling shutdown may cause a race-condition where the read/write
        returns just before the socket is deleted and the subsequent read/write would
        try to read from an invalid pointer. By calling shutdown first, the socket
        object remains valid but all current and subsequent calls to read/write will
        return immediately.
    */
    z0 shutdown();

    //==============================================================================
    /** Join a multicast group.

        @returns  true if it succeeds
    */
    b8 joinMulticast (const Txt& multicastIPAddress);

    /** Leave a multicast group.

        @returns  true if it succeeds
    */
    b8 leaveMulticast (const Txt& multicastIPAddress);

    /** Enables or disables multicast loopback.

        @returns  true if it succeeds
    */
    b8 setMulticastLoopbackEnabled (b8 enableLoopback);

    //==============================================================================
    /** Allow other applications to re-use the port.

        Allow any other application currently running to bind to the same port.
        Do not use this if your socket handles sensitive data as it could be
        read by any, possibly malicious, third-party apps.

        @returns  true on success
    */
    b8 setEnablePortReuse (b8 enabled);

private:
    //==============================================================================
    SocketOptions options;
    std::atomic<i32> handle { -1 };
    b8 isBound = false;
    Txt lastBindAddress, lastServerHost;
    i32 lastServerPort = -1;
    uk lastServerAddress = nullptr;
    mutable CriticalSection readLock;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DatagramSocket)
};

} // namespace drx
