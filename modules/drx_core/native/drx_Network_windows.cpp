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

#ifndef INTERNET_FLAG_NEED_FILE
 #define INTERNET_FLAG_NEED_FILE 0x00000010
#endif

#ifndef INTERNET_OPTION_DISABLE_AUTODIAL
 #define INTERNET_OPTION_DISABLE_AUTODIAL 70
#endif

//==============================================================================
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
        closeConnection();
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
    StringPairArray getRequestHeaders() const                             { return WebInputStream::parseHttpHeaders (headers); }
    StringPairArray getResponseHeaders() const                            { return responseHeaders; }
    i32 getStatusCode() const                                             { return statusCode; }

    //==============================================================================
    b8 connect (WebInputStream::Listener* listener)
    {
        {
            const ScopedLock lock (createConnectionLock);

            if (hasBeenCancelled)
                return false;
        }

        auto address = url.toString (! addParametersToRequestBody);

        while (numRedirectsToFollow-- >= 0)
        {
            createConnection (address, listener);

            if (! isError())
            {
                DWORD bufferSizeBytes = 4096;
                StringPairArray dataHeaders;

                for (;;)
                {
                    HeapBlock<t8> buffer (bufferSizeBytes);

                    if (HttpQueryInfo (request, HTTP_QUERY_RAW_HEADERS_CRLF, buffer.getData(), &bufferSizeBytes, nullptr))
                    {
                        StringArray headersArray;
                        headersArray.addLines (Txt (reinterpret_cast<const WCHAR*> (buffer.getData())));

                        for (i32 i = 0; i < headersArray.size(); ++i)
                        {
                            const Txt& header = headersArray[i];
                            const Txt key   (header.upToFirstOccurrenceOf (": ", false, false));
                            const Txt value (header.fromFirstOccurrenceOf (": ", false, false));
                            const Txt previousValue (dataHeaders[key]);
                            dataHeaders.set (key, previousValue.isEmpty() ? value : (previousValue + "," + value));
                        }

                        break;
                    }

                    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                        return false;

                    bufferSizeBytes += 4096;
                }

                DWORD status = 0;
                DWORD statusSize = sizeof (status);

                if (HttpQueryInfo (request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &statusSize, nullptr))
                {
                    statusCode = (i32) status;

                    if (numRedirectsToFollow >= 0
                         && (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307))
                    {
                        Txt newLocation (dataHeaders["Location"]);

                        // Check whether location is a relative URI - this is an incomplete test for relative path,
                        // but we'll use it for now (valid protocols for this implementation are http, https & ftp)
                        if (! (newLocation.startsWithIgnoreCase ("http://")
                                || newLocation.startsWithIgnoreCase ("https://")
                                || newLocation.startsWithIgnoreCase ("ftp://")))
                        {
                            if (newLocation.startsWithChar ('/'))
                                newLocation = URL (address).withNewSubPath (newLocation).toString (true);
                            else
                                newLocation = address + "/" + newLocation;
                        }

                        if (newLocation.isNotEmpty() && newLocation != address)
                        {
                            address = newLocation;
                            continue;
                        }
                    }
                }

                responseHeaders.addArray (dataHeaders);
            }

            break;
        }

        return (request != nullptr);
    }

    b8 isError() const        { return request == nullptr; }
    b8 isExhausted()          { return finished; }
    z64 getPosition()         { return position; }

    z64 getTotalLength()
    {
        if (! isError())
        {
            DWORD index = 0, result = 0, size = sizeof (result);

            if (HttpQueryInfo (request, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &result, &size, &index))
                return (z64) result;
        }

        return -1;
    }

    i32 read (uk buffer, i32 bytesToRead)
    {
        jassert (bytesToRead >= 0);

        if (buffer == nullptr)
        {
            jassertfalse;
            return 0;
        }

        DWORD bytesRead = 0;

        if (! (finished || isError()))
        {
            InternetReadFile (request, buffer, (DWORD) bytesToRead, &bytesRead);
            position += bytesRead;

            if (bytesRead == 0)
                finished = true;
        }

        return (i32) bytesRead;
    }

    z0 cancel()
    {
        {
            const ScopedLock lock (createConnectionLock);

            hasBeenCancelled = true;

            closeConnection();
        }
    }

    b8 setPosition (z64 wantedPos)
    {
        if (isError())
            return false;

        if (wantedPos != position)
        {
            finished = false;
            position = (z64) InternetSetFilePointer (request, (LONG) wantedPos, nullptr, FILE_BEGIN, 0);

            if (position == wantedPos)
                return true;

            if (wantedPos < position)
                return false;

            z64 numBytesToSkip = wantedPos - position;
            auto skipBufferSize = (i32) jmin (numBytesToSkip, (z64) 16384);
            HeapBlock<t8> temp (skipBufferSize);

            while (numBytesToSkip > 0 && ! isExhausted())
                numBytesToSkip -= read (temp, (i32) jmin (numBytesToSkip, (z64) skipBufferSize));
        }

        return true;
    }

    i32 statusCode = 0;

private:
    //==============================================================================
    WebInputStream& owner;
    const URL url;
    HINTERNET connection = nullptr, request = nullptr;
    Txt headers;
    MemoryBlock postData;
    z64 position = 0;
    b8 finished = false;
    const b8 addParametersToRequestBody, hasBodyDataToSend;
    i32 timeOutMs = 0;
    Txt httpRequestCmd;
    i32 numRedirectsToFollow = 5;
    StringPairArray responseHeaders;
    CriticalSection createConnectionLock;
    b8 hasBeenCancelled = false;

    z0 closeConnection()
    {
        HINTERNET requestCopy = request;

        request = nullptr;

        if (requestCopy != nullptr)
            InternetCloseHandle (requestCopy);

        if (connection != nullptr)
        {
            InternetCloseHandle (connection);
            connection = nullptr;
        }
    }

    z0 createConnection (const Txt& address, WebInputStream::Listener* listener)
    {
        static HINTERNET sessionHandle = InternetOpen (_T ("drx"), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);

        closeConnection();

        if (sessionHandle != nullptr)
        {
            // break up the url..
            i32k fileNumChars = 65536;
            i32k serverNumChars = 2048;
            i32k usernameNumChars = 1024;
            i32k passwordNumChars = 1024;
            HeapBlock<TCHAR> file (fileNumChars), server (serverNumChars),
                             username (usernameNumChars), password (passwordNumChars);

            URL_COMPONENTS uc = {};
            uc.dwStructSize = sizeof (uc);
            uc.lpszUrlPath = file;
            uc.dwUrlPathLength = fileNumChars;
            uc.lpszHostName = server;
            uc.dwHostNameLength = serverNumChars;
            uc.lpszUserName = username;
            uc.dwUserNameLength = usernameNumChars;
            uc.lpszPassword = password;
            uc.dwPasswordLength = passwordNumChars;

            if (hasBodyDataToSend)
                WebInputStream::createHeadersAndPostData (url,
                                                          headers,
                                                          postData,
                                                          addParametersToRequestBody);

            if (InternetCrackUrl (address.toWideCharPointer(), 0, 0, &uc))
                openConnection (uc, sessionHandle, address, listener);
        }
    }

    z0 openConnection (URL_COMPONENTS& uc, HINTERNET sessionHandle,
                         const Txt& address,
                         WebInputStream::Listener* listener)
    {
        i32 disable = 1;
        InternetSetOption (sessionHandle, INTERNET_OPTION_DISABLE_AUTODIAL, &disable, sizeof (disable));

        if (timeOutMs == 0)
            timeOutMs = 30000;
        else if (timeOutMs < 0)
            timeOutMs = -1;

        applyTimeout (sessionHandle, INTERNET_OPTION_CONNECT_TIMEOUT);
        applyTimeout (sessionHandle, INTERNET_OPTION_RECEIVE_TIMEOUT);
        applyTimeout (sessionHandle, INTERNET_OPTION_SEND_TIMEOUT);
        applyTimeout (sessionHandle, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT);
        applyTimeout (sessionHandle, INTERNET_OPTION_DATA_SEND_TIMEOUT);

        const b8 isFtp = address.startsWithIgnoreCase ("ftp:");

        {
            const ScopedLock lock (createConnectionLock);

            connection = hasBeenCancelled ? nullptr
                                          : InternetConnect (sessionHandle,
                                                             uc.lpszHostName, uc.nPort,
                                                             uc.lpszUserName, uc.lpszPassword,
                                                             isFtp ? (DWORD) INTERNET_SERVICE_FTP
                                                                   : (DWORD) INTERNET_SERVICE_HTTP,
                                                             0, 0);
        }

        if (connection != nullptr)
        {
            if (isFtp)
                request = FtpOpenFile (connection, uc.lpszUrlPath, GENERIC_READ,
                                       FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_NEED_FILE, 0);
            else
                openHTTPConnection (uc, address, listener);
        }
    }

    z0 applyTimeout (HINTERNET sessionHandle, const DWORD option)
    {
        InternetSetOption (sessionHandle, option, &timeOutMs, sizeof (timeOutMs));
    }

    z0 sendHTTPRequest (INTERNET_BUFFERS& buffers, WebInputStream::Listener* listener)
    {
        if (! HttpSendRequestEx (request, &buffers, nullptr, HSR_INITIATE, 0))
            return;

        i32 totalBytesSent = 0;

        while (totalBytesSent < (i32) postData.getSize())
        {
            auto bytesToSend = jmin (1024, (i32) postData.getSize() - totalBytesSent);
            DWORD bytesSent = 0;

            if (bytesToSend == 0
                || ! InternetWriteFile (request, static_cast<tukk> (postData.getData()) + totalBytesSent,
                                        (DWORD) bytesToSend, &bytesSent))
            {
                return;
            }

            totalBytesSent += (i32) bytesSent;

            if (listener != nullptr
                && ! listener->postDataSendProgress (owner, totalBytesSent, (i32) postData.getSize()))
            {
                return;
            }
        }
    }

    z0 openHTTPConnection (URL_COMPONENTS& uc, const Txt& address, WebInputStream::Listener* listener)
    {
        const TCHAR* mimeTypes[] = { _T ("*/*"), nullptr };

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES
                        | INTERNET_FLAG_NO_AUTO_REDIRECT;

        if (address.startsWithIgnoreCase ("https:"))
            flags |= INTERNET_FLAG_SECURE;  // (this flag only seems necessary if the OS is running IE6 -
                                            //  IE7 seems to automatically work out when it's https)

        {
            const ScopedLock lock (createConnectionLock);

            request = hasBeenCancelled ? nullptr
                                       : HttpOpenRequest (connection, httpRequestCmd.toWideCharPointer(),
                                                          uc.lpszUrlPath, nullptr, nullptr, mimeTypes, flags, 0);
        }

        if (request != nullptr)
        {
            INTERNET_BUFFERS buffers = {};
            buffers.dwStructSize    = sizeof (INTERNET_BUFFERS);
            buffers.lpcszHeader     = headers.toWideCharPointer();
            buffers.dwHeadersLength = (DWORD) headers.length();
            buffers.dwBufferTotal   = (DWORD) postData.getSize();

            auto sendRequestAndTryEnd = [this, &buffers, &listener]() -> b8
            {
                sendHTTPRequest (buffers, listener);

                if (HttpEndRequest (request, nullptr, 0, 0))
                    return true;

                return false;
            };

            auto closed = sendRequestAndTryEnd();

            // N.B. this is needed for some authenticated HTTP connections
            if (! closed && GetLastError() == ERROR_INTERNET_FORCE_RETRY)
                closed = sendRequestAndTryEnd();

            if (closed)
                return;
        }

        closeConnection();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
struct GetAdaptersAddressesHelper
{
    b8 callGetAdaptersAddresses()
    {
        DynamicLibrary dll ("iphlpapi.dll");
        DRX_LOAD_WINAPI_FUNCTION (dll, GetAdaptersAddresses, getAdaptersAddresses, DWORD, (ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG))

        if (getAdaptersAddresses == nullptr)
            return false;

        adaptersAddresses.malloc (1);
        ULONG len = sizeof (IP_ADAPTER_ADDRESSES);

        if (getAdaptersAddresses (AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adaptersAddresses, &len) == ERROR_BUFFER_OVERFLOW)
            adaptersAddresses.malloc (len, 1);

        return getAdaptersAddresses (AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adaptersAddresses, &len) == NO_ERROR;
    }

    HeapBlock<IP_ADAPTER_ADDRESSES> adaptersAddresses;
};

namespace MACAddressHelpers
{
    static z0 addAddress (Array<MACAddress>& result, const MACAddress& ma)
    {
        if (! ma.isNull())
            result.addIfNotAlreadyThere (ma);
    }

    static z0 getViaGetAdaptersAddresses (Array<MACAddress>& result)
    {
        GetAdaptersAddressesHelper addressesHelper;

        if (addressesHelper.callGetAdaptersAddresses())
        {
            for (PIP_ADAPTER_ADDRESSES adapter = addressesHelper.adaptersAddresses; adapter != nullptr; adapter = adapter->Next)
            {
                if (adapter->PhysicalAddressLength >= 6)
                    addAddress (result, MACAddress (adapter->PhysicalAddress));
            }
        }
    }

    static z0 getViaNetBios (Array<MACAddress>& result)
    {
        DynamicLibrary dll ("netapi32.dll");
        DRX_LOAD_WINAPI_FUNCTION (dll, Netbios, NetbiosCall, UCHAR, (PNCB))

        if (NetbiosCall != nullptr)
        {
            LANA_ENUM enums = {};

            {
                NCB ncb = {};
                ncb.ncb_command = NCBENUM;
                ncb.ncb_buffer = (u8*) &enums;
                ncb.ncb_length = sizeof (LANA_ENUM);
                NetbiosCall (&ncb);
            }

            for (i32 i = 0; i < enums.length; ++i)
            {
                NCB ncb2 = {};
                ncb2.ncb_command = NCBRESET;
                ncb2.ncb_lana_num = enums.lana[i];

                if (NetbiosCall (&ncb2) == 0)
                {
                    NCB ncb = {};
                    memcpy (ncb.ncb_callname, "*                   ", NCBNAMSZ);
                    ncb.ncb_command = NCBASTAT;
                    ncb.ncb_lana_num = enums.lana[i];

                    struct ASTAT
                    {
                        ADAPTER_STATUS adapt;
                        NAME_BUFFER    NameBuff [30];
                    };

                    ASTAT astat;
                    zerostruct (astat);
                    ncb.ncb_buffer = (u8*) &astat;
                    ncb.ncb_length = sizeof (ASTAT);

                    if (NetbiosCall (&ncb) == 0 && astat.adapt.adapter_type == 0xfe)
                        addAddress (result, MACAddress (astat.adapt.adapter_address));
                }
            }
        }
    }

    static z0 split (const sockaddr_in6* sa_in6, i32 off, u8* split)
    {
        split[0] = sa_in6->sin6_addr.u.Byte[off + 1];
        split[1] = sa_in6->sin6_addr.u.Byte[off];
    }

    static IPAddress createAddress (const sockaddr_in6* sa_in6)
    {
        IPAddressByteUnion temp;
        u16 arr[8];

        for (i32 i = 0; i < 8; ++i)
        {
            split (sa_in6, i * 2, temp.split);
            arr[i] = temp.combined;
        }

        return IPAddress (arr);
    }

    static IPAddress createAddress (const sockaddr_in* sa_in)
    {
        return IPAddress ((u8*) &sa_in->sin_addr.s_addr, false);
    }

    template <typename Type>
    static z0 findAddresses (Array<IPAddress>& result, b8 includeIPv6, Type start)
    {
        for (auto addr = start; addr != nullptr; addr = addr->Next)
        {
            if (addr->Address.lpSockaddr->sa_family == AF_INET)
                result.addIfNotAlreadyThere (createAddress (unalignedPointerCast<sockaddr_in*> (addr->Address.lpSockaddr)));
            else if (addr->Address.lpSockaddr->sa_family == AF_INET6 && includeIPv6)
                result.addIfNotAlreadyThere (createAddress (unalignedPointerCast<sockaddr_in6*> (addr->Address.lpSockaddr)));
        }
    }
}

z0 MACAddress::findAllAddresses (Array<MACAddress>& result)
{
    MACAddressHelpers::getViaGetAdaptersAddresses (result);
    MACAddressHelpers::getViaNetBios (result);
}

z0 IPAddress::findAllAddresses (Array<IPAddress>& result, b8 includeIPv6)
{
    result.addIfNotAlreadyThere (IPAddress::local());

    if (includeIPv6)
        result.addIfNotAlreadyThere (IPAddress::local (true));

    GetAdaptersAddressesHelper addressesHelper;

    if (addressesHelper.callGetAdaptersAddresses())
    {
        for (PIP_ADAPTER_ADDRESSES adapter = addressesHelper.adaptersAddresses; adapter != nullptr; adapter = adapter->Next)
        {
            MACAddressHelpers::findAddresses (result, includeIPv6, adapter->FirstUnicastAddress);
            MACAddressHelpers::findAddresses (result, includeIPv6, adapter->FirstAnycastAddress);
            MACAddressHelpers::findAddresses (result, includeIPv6, adapter->FirstMulticastAddress);
        }
    }
}

IPAddress IPAddress::getInterfaceBroadcastAddress (const IPAddress&)
{
    // TODO
    return {};
}


//==============================================================================
b8 DRX_CALLTYPE Process::openEmailWithAttachments (const Txt& targetEmailAddress,
                                                      const Txt& emailSubject,
                                                      const Txt& bodyText,
                                                      const StringArray& filesToAttach)
{
    DynamicLibrary dll ("MAPI32.dll");
    DRX_LOAD_WINAPI_FUNCTION (dll, MAPISendMail, mapiSendMail,
                               ULONG, (LHANDLE, ULONG, lpMapiMessage, ::FLAGS, ULONG))

    if (mapiSendMail == nullptr)
        return false;

    MapiMessage message = {};
    message.lpszSubject = (LPSTR) emailSubject.toRawUTF8();
    message.lpszNoteText = (LPSTR) bodyText.toRawUTF8();

    MapiRecipDesc recip = {};
    recip.ulRecipClass = MAPI_TO;
    Txt targetEmailAddress_ (targetEmailAddress);
    if (targetEmailAddress_.isEmpty())
        targetEmailAddress_ = " "; // (Windows Mail can't deal with a blank address)
    recip.lpszName = (LPSTR) targetEmailAddress_.toRawUTF8();
    message.nRecipCount = 1;
    message.lpRecips = &recip;

    HeapBlock<MapiFileDesc> files;
    files.calloc (filesToAttach.size());

    message.nFileCount = (ULONG) filesToAttach.size();
    message.lpFiles = files;

    for (i32 i = 0; i < filesToAttach.size(); ++i)
    {
        files[i].nPosition = (ULONG) -1;
        files[i].lpszPathName = (LPSTR) filesToAttach[i].toRawUTF8();
    }

    return mapiSendMail (0, 0, &message, MAPI_DIALOG | MAPI_LOGON_UI, 0) == SUCCESS_SUCCESS;
}

std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation, const DownloadTaskOptions& options)
{
    return URL::DownloadTask::createFallbackDownloader (*this, targetLocation, options);
}

} // namespace drx
