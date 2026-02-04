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

struct CURLSymbols
{
    CURL* (*curl_easy_init) (z0);
    CURLcode (*curl_easy_setopt) (CURL *curl, CURLoption option, ...);
    z0 (*curl_easy_cleanup) (CURL *curl);
    CURLcode (*curl_easy_getinfo) (CURL *curl, CURLINFO info, ...);
    CURLMcode (*curl_multi_add_handle) (CURLM *multi_handle, CURL *curl_handle);
    CURLMcode (*curl_multi_cleanup) (CURLM *multi_handle);
    CURLMcode (*curl_multi_fdset) (CURLM *multi_handle, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *exc_fd_set, i32 *max_fd);
    CURLMsg* (*curl_multi_info_read) (CURLM *multi_handle, i32 *msgs_in_queue);
    CURLM* (*curl_multi_init) (z0);
    CURLMcode (*curl_multi_perform) (CURLM *multi_handle, i32 *running_handles);
    CURLMcode (*curl_multi_remove_handle) (CURLM *multi_handle, CURL *curl_handle);
    CURLMcode (*curl_multi_timeout) (CURLM *multi_handle, i64 *milliseconds);
    struct curl_slist* (*curl_slist_append) (struct curl_slist *, const t8 *);
    z0 (*curl_slist_free_all) (struct curl_slist *);
    curl_version_info_data* (*curl_version_info) (CURLversion);

    static std::unique_ptr<CURLSymbols> create()
    {
        std::unique_ptr<CURLSymbols> symbols (new CURLSymbols);

       #if DRX_LOAD_CURL_SYMBOLS_LAZILY
        const ScopedLock sl (getLibcurlLock());
        #define DRX_INIT_CURL_SYMBOL(name)  if (! symbols->loadSymbol (symbols->name, #name)) return nullptr;
       #else
        #define DRX_INIT_CURL_SYMBOL(name)  symbols->name = ::name;
       #endif

        DRX_INIT_CURL_SYMBOL (curl_easy_init)
        DRX_INIT_CURL_SYMBOL (curl_easy_setopt)
        DRX_INIT_CURL_SYMBOL (curl_easy_cleanup)
        DRX_INIT_CURL_SYMBOL (curl_easy_getinfo)
        DRX_INIT_CURL_SYMBOL (curl_multi_add_handle)
        DRX_INIT_CURL_SYMBOL (curl_multi_cleanup)
        DRX_INIT_CURL_SYMBOL (curl_multi_fdset)
        DRX_INIT_CURL_SYMBOL (curl_multi_info_read)
        DRX_INIT_CURL_SYMBOL (curl_multi_init)
        DRX_INIT_CURL_SYMBOL (curl_multi_perform)
        DRX_INIT_CURL_SYMBOL (curl_multi_remove_handle)
        DRX_INIT_CURL_SYMBOL (curl_multi_timeout)
        DRX_INIT_CURL_SYMBOL (curl_slist_append)
        DRX_INIT_CURL_SYMBOL (curl_slist_free_all)
        DRX_INIT_CURL_SYMBOL (curl_version_info)

        return symbols;
    }

    // liburl's curl_multi_init calls curl_global_init which is not thread safe
    // so we need to get a lock during calls to curl_multi_init and curl_multi_cleanup
    static CriticalSection& getLibcurlLock() noexcept
    {
        static CriticalSection cs;
        return cs;
    }

private:
    CURLSymbols() = default;

   #if DRX_LOAD_CURL_SYMBOLS_LAZILY
    static DynamicLibrary& getLibcurl()
    {
        const ScopedLock sl (getLibcurlLock());
        static DynamicLibrary libcurl;

        if (libcurl.getNativeHandle() == nullptr)
            for (auto libName : { "libcurl.so",
                                  "libcurl.so.4", "libcurl.so.3",
                                  "libcurl-gnutls.so.4", "libcurl-gnutls.so.3" })
                if (libcurl.open (libName))
                    break;

        return libcurl;
    }

    template <typename FuncPtr>
    b8 loadSymbol (FuncPtr& dst, tukk name)
    {
        dst = reinterpret_cast<FuncPtr> (getLibcurl().getFunction (name));
        return (dst != nullptr);
    }
   #endif
};


//==============================================================================
class WebInputStream::Pimpl
{
public:
    Pimpl (WebInputStream& ownerStream, const URL& urlToCopy, b8 addParametersToBody)
        : owner (ownerStream),
          url (urlToCopy),
          addParametersToRequestBody (addParametersToBody),
          hasBodyDataToSend (url.hasBodyDataToSend() || addParametersToRequestBody),
          httpRequest (hasBodyDataToSend ? "POST" : "GET")
    {
        jassert (symbols); // Unable to load libcurl!

        {
            const ScopedLock sl (CURLSymbols::getLibcurlLock());
            multi = symbols->curl_multi_init();
        }

        if (multi != nullptr)
        {
            curl = symbols->curl_easy_init();

            if (curl != nullptr)
                if (symbols->curl_multi_add_handle (multi, curl) == CURLM_OK)
                    return;
        }

        cleanup();
    }

    ~Pimpl()
    {
        cleanup();
    }

    //==============================================================================
    // Input Stream overrides
    b8 isError() const                 { return curl == nullptr || lastError != CURLE_OK; }
    b8 isExhausted()                   { return (isError() || finished) && curlBuffer.isEmpty(); }
    z64 getPosition()                  { return streamPos; }
    z64 getTotalLength()               { return contentLength; }

    i32 read (uk buffer, i32 bytesToRead)
    {
        return readOrSkip (buffer, bytesToRead, false);
    }

    b8 setPosition (z64 wantedPos)
    {
        i32k amountToSkip = static_cast<i32> (wantedPos - getPosition());

        if (amountToSkip < 0)
            return false;

        if (amountToSkip == 0)
            return true;

        i32k actuallySkipped = readOrSkip (nullptr, amountToSkip, true);

        return actuallySkipped == amountToSkip;
    }

    //==============================================================================
    // WebInputStream methods
    z0 withExtraHeaders (const Txt& extraHeaders)
    {
        if (! requestHeaders.endsWithChar ('\n') && requestHeaders.isNotEmpty())
            requestHeaders << "\r\n";

        requestHeaders << extraHeaders;

        if (! requestHeaders.endsWithChar ('\n') && requestHeaders.isNotEmpty())
            requestHeaders << "\r\n";
    }

    z0 withCustomRequestCommand (const Txt& customRequestCommand)    { httpRequest = customRequestCommand; }
    z0 withConnectionTimeout (i32 timeoutInMs)                          { timeOutMs = timeoutInMs; }
    z0 withNumRedirectsToFollow (i32 maxRedirectsToFollow)              { maxRedirects = maxRedirectsToFollow; }
    StringPairArray getRequestHeaders() const                             { return WebInputStream::parseHttpHeaders (requestHeaders); }
    StringPairArray getResponseHeaders() const                            { return WebInputStream::parseHttpHeaders (responseHeaders); }
    i32 getStatusCode() const                                             { return statusCode; }

    //==============================================================================
    z0 cleanup()
    {
        const ScopedLock lock (cleanupLock);
        const ScopedLock sl (CURLSymbols::getLibcurlLock());

        if (curl != nullptr)
        {
            symbols->curl_multi_remove_handle (multi, curl);

            if (headerList != nullptr)
            {
                symbols->curl_slist_free_all (headerList);
                headerList = nullptr;
            }

            symbols->curl_easy_cleanup (curl);
            curl = nullptr;
        }

        if (multi != nullptr)
        {
            symbols->curl_multi_cleanup (multi);
            multi = nullptr;
        }
    }

    z0 cancel()
    {
        cleanup();
    }

    //==============================================================================
    b8 setOptions()
    {
        auto address = url.toString (! addParametersToRequestBody);

        curl_version_info_data* data = symbols->curl_version_info (CURLVERSION_NOW);
        jassert (data != nullptr);

        if (! requestHeaders.endsWithChar ('\n'))
            requestHeaders << "\r\n";

        if (hasBodyDataToSend)
            WebInputStream::createHeadersAndPostData (url,
                                                      requestHeaders,
                                                      headersAndPostData,
                                                      addParametersToRequestBody);

        if (! requestHeaders.endsWithChar ('\n'))
            requestHeaders << "\r\n";

        auto userAgent = Txt ("curl/") + data->version;

        if (symbols->curl_easy_setopt (curl, CURLOPT_URL, address.toRawUTF8()) == CURLE_OK
            && symbols->curl_easy_setopt (curl, CURLOPT_WRITEDATA, this) == CURLE_OK
            && symbols->curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, StaticCurlWrite) == CURLE_OK
            && symbols->curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1) == CURLE_OK
            && symbols->curl_easy_setopt (curl, CURLOPT_MAXREDIRS, static_cast<i64> (maxRedirects)) == CURLE_OK
            && symbols->curl_easy_setopt (curl, CURLOPT_USERAGENT, userAgent.toRawUTF8()) == CURLE_OK
            && symbols->curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, (maxRedirects > 0 ? 1 : 0)) == CURLE_OK)
        {
            if (hasBodyDataToSend)
            {
                if (symbols->curl_easy_setopt (curl, CURLOPT_READDATA, this) != CURLE_OK
                    || symbols->curl_easy_setopt (curl, CURLOPT_READFUNCTION, StaticCurlRead) != CURLE_OK)
                    return false;

                if (symbols->curl_easy_setopt (curl, CURLOPT_POST, 1) != CURLE_OK
                    || symbols->curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t> (headersAndPostData.getSize())) != CURLE_OK)
                    return false;
            }

            // handle special http request commands
            const auto hasSpecialRequestCmd = hasBodyDataToSend ? (httpRequest != "POST") : (httpRequest != "GET");

            if (hasSpecialRequestCmd)
                if (symbols->curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, httpRequest.toRawUTF8()) != CURLE_OK)
                    return false;

            if (symbols->curl_easy_setopt (curl, CURLOPT_HEADERDATA, this) != CURLE_OK
                || symbols->curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, StaticCurlHeader) != CURLE_OK)
                return false;

            if (timeOutMs > 0)
            {
                auto timeOutSecs = ((i64) timeOutMs + 999) / 1000;

                if (symbols->curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, timeOutSecs) != CURLE_OK
                    || symbols->curl_easy_setopt (curl, CURLOPT_LOW_SPEED_LIMIT, 100) != CURLE_OK
                    || symbols->curl_easy_setopt (curl, CURLOPT_LOW_SPEED_TIME, timeOutSecs) != CURLE_OK)
                    return false;
            }

            return true;
        }

        return false;
    }

    b8 connect (WebInputStream::Listener* webInputListener)
    {
        {
            const ScopedLock lock (cleanupLock);

            if (curl == nullptr)
                return false;

            if (! setOptions())
            {
                cleanup();
                return false;
            }

            if (requestHeaders.isNotEmpty())
            {
                const StringArray headerLines = StringArray::fromLines (requestHeaders);

                // fromLines will always return at least one line if the string is not empty
                jassert (headerLines.size() > 0);
                headerList = symbols->curl_slist_append (headerList, headerLines [0].toRawUTF8());

                for (i32 i = 1; (i < headerLines.size() && headerList != nullptr); ++i)
                    headerList = symbols->curl_slist_append (headerList, headerLines [i].toRawUTF8());

                if (headerList == nullptr)
                {
                    cleanup();
                    return false;
                }

                if (symbols->curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headerList) != CURLE_OK)
                {
                    cleanup();
                    return false;
                }
            }
        }

        listener = webInputListener;

        if (hasBodyDataToSend)
            postBuffer = &headersAndPostData;

        size_t lastPos = static_cast<size_t> (-1);

        // step until either: 1) there is an error 2) the transaction is complete
        // or 3) data is in the in buffer
        while ((! finished) && curlBuffer.isEmpty())
        {
            {
                const ScopedLock lock (cleanupLock);

                if (curl == nullptr)
                    return false;
            }

            singleStep();

            // call callbacks if this is a post request
            if (hasBodyDataToSend && listener != nullptr && lastPos != postPosition)
            {
                lastPos = postPosition;

                if (! listener->postDataSendProgress (owner, static_cast<i32> (lastPos), static_cast<i32> (headersAndPostData.getSize())))
                {
                    // user has decided to abort the transaction
                    cleanup();
                    return false;
                }
            }
        }

        {
            const ScopedLock lock (cleanupLock);

            if (curl == nullptr)
                return false;

            i64 responseCode;
            if (symbols->curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &responseCode) == CURLE_OK)
                statusCode = static_cast<i32> (responseCode);

           #if LIBCURL_VERSION_MAJOR < 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR < 55)
            f64 curlLength;
            if (symbols->curl_easy_getinfo (curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &curlLength) == CURLE_OK)
            {
           #else
            curl_off_t curlLength;
            if (symbols->curl_easy_getinfo (curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &curlLength) == CURLE_OK)
            {
           #endif
                contentLength = static_cast<z64> (curlLength);
            }
       }

        return true;
    }

    z0 finish()
    {
        const ScopedLock lock (cleanupLock);

        if (curl == nullptr)
            return;

        for (;;)
        {
            i32 cnt = 0;

            if (CURLMsg* msg = symbols->curl_multi_info_read (multi, &cnt))
            {
                if (msg->msg == CURLMSG_DONE && msg->easy_handle == curl)
                {
                    lastError = msg->data.result; // this is the error that stopped our process from continuing
                    break;
                }
            }
            else
            {
                break;
            }
        }

        finished = true;
    }

    //==============================================================================
    z0 singleStep()
    {
        if (lastError != CURLE_OK)
            return;

        fd_set fdread, fdwrite, fdexcep;
        i32 maxfd = -1;
        i64 curl_timeo;

        {
            const ScopedLock lock (cleanupLock);

            if (multi == nullptr)
                return;

            if ((lastError = (i32) symbols->curl_multi_timeout (multi, &curl_timeo)) != CURLM_OK)
                return;
        }

        // why 980? see http://curl.haxx.se/libcurl/c/curl_multi_timeout.html
        if (curl_timeo < 0)
            curl_timeo = 980;

        struct timeval tv;
        tv.tv_sec = curl_timeo / 1000;
        tv.tv_usec = (curl_timeo % 1000) * 1000;

        FD_ZERO (&fdread);
        FD_ZERO (&fdwrite);
        FD_ZERO (&fdexcep);

        {
            const ScopedLock lock (cleanupLock);

            if (multi == nullptr)
                return;

            if ((lastError = (i32) symbols->curl_multi_fdset (multi, &fdread, &fdwrite, &fdexcep, &maxfd)) != CURLM_OK)
                return;
        }

        if (maxfd != -1)
        {
            if (select (maxfd + 1, &fdread, &fdwrite, &fdexcep, &tv) < 0)
            {
                lastError = -1;
                return;
            }
        }
        else
        {
            // if curl does not return any sockets for to wait on, then the doc says to wait 100 ms
            Thread::sleep (100);
        }

        i32 still_running = 0;
        i32 curlRet;

        {
            const ScopedLock lock (cleanupLock);

            while ((curlRet = (i32) symbols->curl_multi_perform (multi, &still_running)) == CURLM_CALL_MULTI_PERFORM)
            {}
        }

        if ((lastError = curlRet) != CURLM_OK)
            return;

        if (still_running <= 0)
            finish();
    }

    i32 readOrSkip (uk buffer, i32 bytesToRead, b8 skip)
    {
        if (bytesToRead <= 0)
            return 0;

        size_t pos = 0;
        size_t len = static_cast<size_t> (bytesToRead);

        while (len > 0)
        {
            size_t bufferBytes = curlBuffer.getSize();
            b8 removeSection = true;

            if (bufferBytes == 0)
            {
                // do not call curl again if we are finished
                {
                    const ScopedLock lock (cleanupLock);

                    if (finished || curl == nullptr)
                        return static_cast<i32> (pos);
                }

                skipBytes = skip ? len : 0;
                singleStep();

                // update the amount that was read/skipped from curl
                bufferBytes = skip ? len - skipBytes : curlBuffer.getSize();
                removeSection = ! skip;
            }

            // can we copy data from the internal buffer?
            if (bufferBytes > 0)
            {
                size_t max = jmin (len, bufferBytes);

                if (! skip)
                    memcpy (addBytesToPointer (buffer, pos), curlBuffer.getData(), max);

                pos += max;
                streamPos += static_cast<z64> (max);
                len -= max;

                if (removeSection)
                    curlBuffer.removeSection (0, max);
            }
        }

        return static_cast<i32> (pos);
    }

    //==============================================================================
    // CURL callbacks
    size_t curlWriteCallback (tuk ptr, size_t size, size_t nmemb)
    {
        if (curl == nullptr || lastError != CURLE_OK)
            return 0;

        const size_t len = size * nmemb;

        // skip bytes if necessary
        size_t max = jmin (skipBytes, len);
        skipBytes -= max;

        if (len > max)
            curlBuffer.append (ptr + max, len - max);

        return len;
    }

    size_t curlReadCallback (tuk ptr, size_t size, size_t nmemb)
    {
        if (curl == nullptr || postBuffer == nullptr || lastError != CURLE_OK)
            return 0;

        const size_t len = size * nmemb;

        size_t max = jmin (postBuffer->getSize() - postPosition, len);
        memcpy (ptr, (tuk)postBuffer->getData() + postPosition, max);
        postPosition += max;

        return max;
    }

    size_t curlHeaderCallback (tuk ptr, size_t size, size_t nmemb)
    {
        if (curl == nullptr || lastError != CURLE_OK)
            return 0;

        size_t len = size * nmemb;

        Txt header (ptr, len);

        if (! header.contains (":") && header.startsWithIgnoreCase ("HTTP/"))
            responseHeaders.clear();
        else
            responseHeaders += header;

        return len;
    }


    //==============================================================================
    // Static method wrappers
    static size_t StaticCurlWrite (tuk ptr, size_t size, size_t nmemb, uk userdata)
    {
        WebInputStream::Pimpl* wi = reinterpret_cast<WebInputStream::Pimpl*> (userdata);
        return wi->curlWriteCallback (ptr, size, nmemb);
    }

    static size_t StaticCurlRead (tuk ptr, size_t size, size_t nmemb, uk userdata)
    {
        WebInputStream::Pimpl* wi = reinterpret_cast<WebInputStream::Pimpl*> (userdata);
        return wi->curlReadCallback (ptr, size, nmemb);
    }

    static size_t StaticCurlHeader (tuk ptr, size_t size, size_t nmemb, uk userdata)
    {
        WebInputStream::Pimpl* wi = reinterpret_cast<WebInputStream::Pimpl*> (userdata);
        return wi->curlHeaderCallback (ptr, size, nmemb);
    }

    //==============================================================================
    WebInputStream& owner;
    const URL url;
    std::unique_ptr<CURLSymbols> symbols { CURLSymbols::create() };

    //==============================================================================
    // curl stuff
    CURLM* multi = nullptr;
    CURL* curl = nullptr;
    struct curl_slist* headerList = nullptr;
    i32 lastError = CURLE_OK;

    //==============================================================================
    // Options
    i32 timeOutMs = 0;
    i32 maxRedirects = 5;
    const b8 addParametersToRequestBody, hasBodyDataToSend;
    Txt httpRequest;

    //==============================================================================
    // internal buffers and buffer positions
    z64 contentLength = -1, streamPos = 0;
    MemoryBlock curlBuffer;
    MemoryBlock headersAndPostData;
    Txt responseHeaders, requestHeaders;
    i32 statusCode = -1;

    //==============================================================================
    b8 finished = false;
    size_t skipBytes = 0;

    //==============================================================================
    // Http POST variables
    const MemoryBlock* postBuffer = nullptr;
    size_t postPosition = 0;

    //==============================================================================
    WebInputStream::Listener* listener = nullptr;

    //==============================================================================
    CriticalSection cleanupLock;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation, const DownloadTaskOptions& options)
{
    return URL::DownloadTask::createFallbackDownloader (*this, targetLocation, options);
}

} // namespace drx
