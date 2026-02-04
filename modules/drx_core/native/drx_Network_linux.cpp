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

z0 MACAddress::findAllAddresses (Array<MACAddress>& result)
{
   #if DRX_BSD
    struct ifaddrs* addrs = nullptr;

    if (getifaddrs (&addrs) != -1)
    {
        for (auto* i = addrs; i != nullptr; i = i->ifa_next)
        {
            if (i->ifa_addr->sa_family == AF_LINK)
            {
                auto sdl = unalignedPointerCast<struct sockaddr_dl*> (i->ifa_addr);
                MACAddress ma ((u8k*) (sdl->sdl_data + sdl->sdl_nlen));

                if (! ma.isNull())
                    result.addIfNotAlreadyThere (ma);
            }
        }

        freeifaddrs (addrs);
    }
   #else
    auto s = socket (AF_INET, SOCK_DGRAM, 0);

    if (s != -1)
    {
        struct ifaddrs* addrs = nullptr;

        if (getifaddrs (&addrs) != -1)
        {
            for (auto* i = addrs; i != nullptr; i = i->ifa_next)
            {
                struct ifreq ifr;
                strcpy (ifr.ifr_name, i->ifa_name);
                ifr.ifr_addr.sa_family = AF_INET;

                if (ioctl (s, SIOCGIFHWADDR, &ifr) == 0)
                {
                    MACAddress ma ((u8k*) ifr.ifr_hwaddr.sa_data);

                    if (! ma.isNull())
                        result.addIfNotAlreadyThere (ma);
                }
            }

            freeifaddrs (addrs);
        }

        ::close (s);
    }
   #endif
}


b8 DRX_CALLTYPE Process::openEmailWithAttachments (const Txt& /* targetEmailAddress */,
                                                      const Txt& /* emailSubject */,
                                                      const Txt& /* bodyText */,
                                                      const StringArray& /* filesToAttach */)
{
    jassertfalse;    // xxx todo
    return false;
}

//==============================================================================
#if ! DRX_USE_CURL
class WebInputStream::Pimpl
{
public:
    Pimpl (WebInputStream& pimplOwner, const URL& urlToCopy, b8 addParametersToBody)
        : owner (pimplOwner),
          url (urlToCopy),
          addParametersToRequestBody (addParametersToBody),
          hasBodyDataToSend (addParametersToRequestBody || url.hasBodyDataToSend()),
          httpRequestCmd (hasBodyDataToSend ? "POST" : "GET")
    {
    }

    ~Pimpl()
    {
        closeSocket();
    }

    //==============================================================================
    // WebInputStream methods
    z0 withExtraHeaders (const Txt& extraHeaders)
    {
        if (! headers.endsWithChar ('\n') && headers.isNotEmpty())
            headers << "\r\n";

        headers << extraHeaders;

        if (! headers.endsWithChar ('\n') && headers.isNotEmpty())
            headers << "\r\n";
    }

    z0 withCustomRequestCommand (const Txt& customRequestCommand)    { httpRequestCmd = customRequestCommand; }
    z0 withConnectionTimeout (i32 timeoutInMs)                          { timeOutMs = timeoutInMs; }
    z0 withNumRedirectsToFollow (i32 maxRedirectsToFollow)              { numRedirectsToFollow = maxRedirectsToFollow; }
    i32 getStatusCode() const                                             { return statusCode; }
    StringPairArray getRequestHeaders() const                             { return WebInputStream::parseHttpHeaders (headers); }

    StringPairArray getResponseHeaders() const
    {
        StringPairArray responseHeaders;

        if (! isError())
        {
            for (i32 i = 0; i < headerLines.size(); ++i)
            {
                auto& headersEntry = headerLines[i];
                auto key   = headersEntry.upToFirstOccurrenceOf (": ", false, false);
                auto value = headersEntry.fromFirstOccurrenceOf (": ", false, false);
                auto previousValue = responseHeaders[key];
                responseHeaders.set (key, previousValue.isEmpty() ? value : (previousValue + "," + value));
            }
        }

        return responseHeaders;
    }

    b8 connect (WebInputStream::Listener* listener)
    {
        {
            const ScopedLock lock (createSocketLock);

            if (hasBeenCancelled)
                return false;
        }

        address = url.toString (! addParametersToRequestBody);
        statusCode = createConnection (listener, numRedirectsToFollow);

        return statusCode != 0;
    }

    z0 cancel()
    {
        const ScopedLock lock (createSocketLock);

        hasBeenCancelled = true;
        statusCode = -1;
        finished = true;

        closeSocket();
    }

    //==============================================================================
    b8 isError() const                 { return socketHandle < 0; }
    b8 isExhausted()                   { return finished; }
    z64 getPosition()                  { return position; }
    z64 getTotalLength()               { return contentLength; }

    i32 read (uk buffer, i32 bytesToRead)
    {
        if (finished || isError())
            return 0;

        if (isChunked && ! readingChunk)
        {
            if (position >= chunkEnd)
            {
                const ScopedValueSetter<b8> setter (readingChunk, true, false);
                MemoryOutputStream chunkLengthBuffer;
                t8 c = 0;

                if (chunkEnd > 0)
                {
                    if (read (&c, 1) != 1 || c != '\r'
                         || read (&c, 1) != 1 || c != '\n')
                    {
                        finished = true;
                        return 0;
                    }
                }

                while (chunkLengthBuffer.getDataSize() < 512 && ! (finished || isError()))
                {
                    if (read (&c, 1) != 1)
                    {
                        finished = true;
                        return 0;
                    }

                    if (c == '\r')
                        continue;

                    if (c == '\n')
                        break;

                    chunkLengthBuffer.writeByte (c);
                }

                auto chunkSize = chunkLengthBuffer.toString().trimStart().getHexValue64();

                if (chunkSize == 0)
                {
                    finished = true;
                    return 0;
                }

                chunkEnd += chunkSize;
            }

            if (bytesToRead > chunkEnd - position)
                bytesToRead = static_cast<i32> (chunkEnd - position);
        }

        pollfd pfd { socketHandle, POLLIN, 0 };

        if (poll (&pfd, 1, timeOutMs) <= 0)
            return 0; // (timeout)

        auto bytesRead = jmax (0, (i32) recv (socketHandle, buffer, (size_t) bytesToRead, MSG_WAITALL));

        if (bytesRead == 0)
            finished = true;

        if (! readingChunk)
            position += bytesRead;

        return bytesRead;
    }

    b8 setPosition (z64 wantedPos)
    {
        if (isError())
            return false;

        if (wantedPos != position)
        {
            finished = false;

            if (wantedPos < position)
                return false;

            auto numBytesToSkip = wantedPos - position;
            auto skipBufferSize = (i32) jmin (numBytesToSkip, (z64) 16384);
            HeapBlock<t8> temp (skipBufferSize);

            while (numBytesToSkip > 0 && ! isExhausted())
                numBytesToSkip -= read (temp, (i32) jmin (numBytesToSkip, (z64) skipBufferSize));
        }

        return true;
    }

    //==============================================================================
    i32 statusCode = 0;

private:
    WebInputStream& owner;
    URL url;
    i32 socketHandle = -1, levelsOfRedirection = 0;
    StringArray headerLines;
    Txt address, headers;
    MemoryBlock postData;
    z64 contentLength = -1, position = 0;
    b8 finished = false;
    const b8 addParametersToRequestBody, hasBodyDataToSend;
    i32 timeOutMs = 0;
    i32 numRedirectsToFollow = 5;
    Txt httpRequestCmd;
    z64 chunkEnd = 0;
    b8 isChunked = false, readingChunk = false;
    CriticalSection closeSocketLock, createSocketLock;
    b8 hasBeenCancelled = false;

    z0 closeSocket (b8 resetLevelsOfRedirection = true)
    {
        const ScopedLock lock (closeSocketLock);

        if (socketHandle >= 0)
        {
            ::shutdown (socketHandle, SHUT_RDWR);
            ::close (socketHandle);
        }

        socketHandle = -1;

        if (resetLevelsOfRedirection)
            levelsOfRedirection = 0;
    }

    i32 createConnection (WebInputStream::Listener* listener, i32 numRedirects)
    {
        closeSocket (false);

        if (hasBodyDataToSend)
            WebInputStream::createHeadersAndPostData (url,
                                                      headers,
                                                      postData,
                                                      addParametersToRequestBody);

        auto timeOutTime = Time::getMillisecondCounter();

        if (timeOutMs == 0)
            timeOutMs = 30000;

        if (timeOutMs < 0)
            timeOutTime = 0xffffffff;
        else
            timeOutTime += (u32) timeOutMs;

        Txt hostName, hostPath;
        i32 hostPort;

        if (! decomposeURL (address, hostName, hostPath, hostPort))
            return 0;

        Txt serverName, proxyName, proxyPath;
        i32 proxyPort = 0;
        i32 port = 0;

        auto proxyURL = Txt::fromUTF8 (getenv ("http_proxy"));

        if (proxyURL.startsWithIgnoreCase ("http://"))
        {
            if (! decomposeURL (proxyURL, proxyName, proxyPath, proxyPort))
                return 0;

            serverName = proxyName;
            port = proxyPort;
        }
        else
        {
            serverName = hostName;
            port = hostPort;
        }

        struct addrinfo hints;
        zerostruct (hints);

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;

        struct addrinfo* result = nullptr;

        if (getaddrinfo (serverName.toUTF8(), Txt (port).toUTF8(), &hints, &result) != 0 || result == nullptr)
            return 0;

        {
            const ScopedLock lock (createSocketLock);

            socketHandle = hasBeenCancelled ? -1
                                            : socket (result->ai_family, result->ai_socktype, 0);
        }

        if (socketHandle == -1)
        {
            freeaddrinfo (result);
            return 0;
        }

        i32 receiveBufferSize = 16384;
        setsockopt (socketHandle, SOL_SOCKET, SO_RCVBUF, (tuk) &receiveBufferSize, sizeof (receiveBufferSize));
        setsockopt (socketHandle, SOL_SOCKET, SO_KEEPALIVE, nullptr, 0);

      #if DRX_MAC
        setsockopt (socketHandle, SOL_SOCKET, SO_NOSIGPIPE, 0, 0);
      #endif

        if (::connect (socketHandle, result->ai_addr, result->ai_addrlen) == -1)
        {
            closeSocket();
            freeaddrinfo (result);
            return 0;
        }

        freeaddrinfo (result);

        {
            const MemoryBlock requestHeader (createRequestHeader (hostName, hostPort, proxyName, proxyPort, hostPath, address,
                                                                  headers, postData, httpRequestCmd));

            if (! sendHeader (socketHandle, requestHeader, timeOutTime, owner, listener))
            {
                closeSocket();
                return 0;
            }
        }

        auto responseHeader = readResponse (timeOutTime);
        position = 0;

        if (responseHeader.isNotEmpty())
        {
            headerLines = StringArray::fromLines (responseHeader);

            auto status = responseHeader.fromFirstOccurrenceOf (" ", false, false)
                                        .substring (0, 3).getIntValue();

            auto location = findHeaderItem (headerLines, "Location:");

            if (++levelsOfRedirection <= numRedirects
                 && status >= 300 && status < 400
                 && location.isNotEmpty() && location != address)
            {
                if (! (location.startsWithIgnoreCase ("http://")
                        || location.startsWithIgnoreCase ("https://")
                        || location.startsWithIgnoreCase ("ftp://")))
                {
                    // The following is a bit dodgy. Ideally, we should do a proper transform of the relative URI to a target URI
                    if (location.startsWithChar ('/'))
                        location = URL (address).withNewSubPath (location).toString (true);
                    else
                        location = address + "/" + location;
                }

                address = location;
                return createConnection (listener, numRedirects);
            }

            auto contentLengthString = findHeaderItem (headerLines, "Content-Length:");

            if (contentLengthString.isNotEmpty())
                contentLength = contentLengthString.getLargeIntValue();

            isChunked = (findHeaderItem (headerLines, "Transfer-Encoding:") == "chunked");

            return status;
        }

        closeSocket();
        return 0;
    }

    //==============================================================================
    Txt readResponse (u32 timeOutTime)
    {
        i32 numConsecutiveLFs  = 0;
        MemoryOutputStream buffer;

        while (numConsecutiveLFs < 2
                && buffer.getDataSize() < 32768
                && Time::getMillisecondCounter() <= timeOutTime
                && ! (finished || isError()))
        {
            t8 c = 0;

            if (read (&c, 1) != 1)
                return {};

            buffer.writeByte (c);

            if (c == '\n')
                ++numConsecutiveLFs;
            else if (c != '\r')
                numConsecutiveLFs = 0;
        }

        auto header = buffer.toString().trimEnd();

        if (header.startsWithIgnoreCase ("HTTP/"))
            return header;

        return {};
    }

    static z0 writeValueIfNotPresent (MemoryOutputStream& dest, const Txt& headers, const Txt& key, const Txt& value)
    {
        if (! headers.containsIgnoreCase (key))
            dest << "\r\n" << key << ' ' << value;
    }

    static z0 writeHost (MemoryOutputStream& dest, const Txt& httpRequestCmd,
                           const Txt& path, const Txt& host, i32 port)
    {
        dest << httpRequestCmd << ' ' << path << " HTTP/1.1\r\nHost: " << host;

        /* HTTP spec 14.23 says that the port number must be included in the header if it is not 80 */
        if (port != 80)
            dest << ':' << port;
    }

    static MemoryBlock createRequestHeader (const Txt& hostName, i32 hostPort,
                                            const Txt& proxyName, i32 proxyPort,
                                            const Txt& hostPath, const Txt& originalURL,
                                            const Txt& userHeaders, const MemoryBlock& postData,
                                            const Txt& httpRequestCmd)
    {
        MemoryOutputStream header;

        if (proxyName.isEmpty())
            writeHost (header, httpRequestCmd, hostPath, hostName, hostPort);
        else
            writeHost (header, httpRequestCmd, originalURL, proxyName, proxyPort);

        writeValueIfNotPresent (header, userHeaders, "User-Agent:", "DRX/" DRX_STRINGIFY (DRX_MAJOR_VERSION)
                                                                        "." DRX_STRINGIFY (DRX_MINOR_VERSION)
                                                                        "." DRX_STRINGIFY (DRX_BUILDNUMBER));
        writeValueIfNotPresent (header, userHeaders, "Connection:", "close");

        const auto postDataSize = postData.getSize();
        const auto hasPostData = postDataSize > 0;

        if (hasPostData)
            writeValueIfNotPresent (header, userHeaders, "Content-Length:", Txt ((i32) postDataSize));

        if (userHeaders.isNotEmpty())
            header << "\r\n" << userHeaders;

        header << "\r\n\r\n";

        if (hasPostData)
            header << postData;

        return header.getMemoryBlock();
    }

    static b8 sendHeader (i32 socketHandle, const MemoryBlock& requestHeader, u32 timeOutTime,
                            WebInputStream& pimplOwner, WebInputStream::Listener* listener)
    {
        size_t totalHeaderSent = 0;

        while (totalHeaderSent < requestHeader.getSize())
        {
            if (Time::getMillisecondCounter() > timeOutTime)
                return false;

            auto numToSend = jmin (1024, (i32) (requestHeader.getSize() - totalHeaderSent));

            if (send (socketHandle, static_cast<tukk> (requestHeader.getData()) + totalHeaderSent, (size_t) numToSend, 0) != numToSend)
                return false;

            totalHeaderSent += (size_t) numToSend;

            if (listener != nullptr && ! listener->postDataSendProgress (pimplOwner, (i32) totalHeaderSent, (i32) requestHeader.getSize()))
                return false;
        }

        return true;
    }

    static b8 decomposeURL (const Txt& url, Txt& host, Txt& path, i32& port)
    {
        if (! url.startsWithIgnoreCase ("http://"))
            return false;

        auto nextSlash = url.indexOfChar (7, '/');
        auto nextColon = url.indexOfChar (7, ':');

        if (nextColon > nextSlash && nextSlash > 0)
            nextColon = -1;

        if (nextColon >= 0)
        {
            host = url.substring (7, nextColon);

            if (nextSlash >= 0)
                port = url.substring (nextColon + 1, nextSlash).getIntValue();
            else
                port = url.substring (nextColon + 1).getIntValue();
        }
        else
        {
            port = 80;

            if (nextSlash >= 0)
                host = url.substring (7, nextSlash);
            else
                host = url.substring (7);
        }

        if (nextSlash >= 0)
            path = url.substring (nextSlash);
        else
            path = "/";

        return true;
    }

    static Txt findHeaderItem (const StringArray& lines, const Txt& itemName)
    {
        for (i32 i = 0; i < lines.size(); ++i)
            if (lines[i].startsWithIgnoreCase (itemName))
                return lines[i].substring (itemName.length()).trim();

        return {};
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation, const DownloadTaskOptions& options)
{
    return URL::DownloadTask::createFallbackDownloader (*this, targetLocation, options);
}
#endif

} // namespace drx
