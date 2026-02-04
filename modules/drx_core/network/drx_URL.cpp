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

struct FallbackDownloadTask final : public URL::DownloadTask,
                                    public Thread
{
    FallbackDownloadTask (std::unique_ptr<FileOutputStream> outputStreamToUse,
                          size_t bufferSizeToUse,
                          std::unique_ptr<WebInputStream> streamToUse,
                          URL::DownloadTask::Listener* listenerToUse)
        : Thread (SystemStats::getDRXVersion() + ": DownloadTask thread"),
          fileStream (std::move (outputStreamToUse)),
          stream (std::move (streamToUse)),
          bufferSize (bufferSizeToUse),
          buffer (bufferSize),
          listener (listenerToUse)
    {
        jassert (fileStream != nullptr);
        jassert (stream != nullptr);

        targetLocation = fileStream->getFile();
        contentLength  = stream->getTotalLength();
        httpCode       = stream->getStatusCode();

        startThread();
    }

    ~FallbackDownloadTask() override
    {
        signalThreadShouldExit();
        stream->cancel();
        waitForThreadToExit (-1);
    }

    //==============================================================================
    z0 run() override
    {
        while (! (stream->isExhausted() || stream->isError() || threadShouldExit()))
        {
            if (listener != nullptr)
                listener->progress (this, downloaded, contentLength);

            auto max = (i32) jmin ((z64) bufferSize, contentLength < 0 ? std::numeric_limits<z64>::max()
                                                                         : static_cast<z64> (contentLength - downloaded));

            auto actual = stream->read (buffer.get(), max);

            if (actual < 0 || threadShouldExit() || stream->isError())
                break;

            if (! fileStream->write (buffer.get(), static_cast<size_t> (actual)))
            {
                error = true;
                break;
            }

            downloaded += actual;

            if (downloaded == contentLength)
                break;
        }

        fileStream.reset();

        if (threadShouldExit() || stream->isError())
            error = true;

        if (contentLength > 0 && downloaded < contentLength)
            error = true;

        finished = true;

        if (listener != nullptr && ! threadShouldExit())
            listener->finished (this, ! error);
    }

    //==============================================================================
    std::unique_ptr<FileOutputStream> fileStream;
    const std::unique_ptr<WebInputStream> stream;
    const size_t bufferSize;
    HeapBlock<t8> buffer;
    URL::DownloadTask::Listener* const listener;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FallbackDownloadTask)
};

z0 URL::DownloadTaskListener::progress (DownloadTask*, z64, z64) {}

//==============================================================================
std::unique_ptr<URL::DownloadTask> URL::DownloadTask::createFallbackDownloader (const URL& urlToUse,
                                                                                const File& targetFileToUse,
                                                                                const DownloadTaskOptions& options)
{
    const size_t bufferSize = 0x8000;
    targetFileToUse.deleteFile();

    if (auto outputStream = targetFileToUse.createOutputStream (bufferSize))
    {
        auto stream = std::make_unique<WebInputStream> (urlToUse, options.usePost);
        stream->withExtraHeaders (options.extraHeaders);

        if (stream->connect (nullptr))
            return std::make_unique<FallbackDownloadTask> (std::move (outputStream),
                                                           bufferSize,
                                                           std::move (stream),
                                                           options.listener);
    }

    return nullptr;
}

URL::DownloadTask::DownloadTask() {}
URL::DownloadTask::~DownloadTask() {}

//==============================================================================
URL::URL() {}

URL::URL (const Txt& u)  : url (u)
{
    init();
}

URL::URL (File localFile)
{
    if (localFile == File())
        return;

   #if DRX_WINDOWS
    b8 isUncPath = localFile.getFullPathName().startsWith ("\\\\");
   #endif

    while (! localFile.isRoot())
    {
        url = "/" + addEscapeChars (localFile.getFileName(), false) + url;
        localFile = localFile.getParentDirectory();
    }

    url = addEscapeChars (localFile.getFileName(), false) + url;

   #if DRX_WINDOWS
    if (isUncPath)
    {
        url = url.fromFirstOccurrenceOf ("/", false, false);
    }
    else
   #endif
    {
        if (! url.startsWithChar (L'/'))
            url = "/" + url;
    }

    url = "file://" + url;

    jassert (isWellFormed());
}

z0 URL::init()
{
    auto i = url.indexOfChar ('#');

    if (i >= 0)
    {
        anchor = removeEscapeChars (url.substring (i + 1));
        url = url.upToFirstOccurrenceOf ("#", false, false);
    }

    i = url.indexOfChar ('?');

    if (i >= 0)
    {
        do
        {
            auto nextAmp   = url.indexOfChar (i + 1, '&');
            auto equalsPos = url.indexOfChar (i + 1, '=');

            if (nextAmp < 0)
            {
                addParameter (removeEscapeChars (equalsPos < 0 ? url.substring (i + 1) : url.substring (i + 1, equalsPos)),
                              equalsPos < 0 ? Txt() : removeEscapeChars (url.substring (equalsPos + 1)));
            }
            else if (nextAmp > 0 && equalsPos < nextAmp)
            {
                addParameter (removeEscapeChars (equalsPos < 0 ? url.substring (i + 1, nextAmp) : url.substring (i + 1, equalsPos)),
                              equalsPos < 0 ? Txt() : removeEscapeChars (url.substring (equalsPos + 1, nextAmp)));
            }

            i = nextAmp;
        }
        while (i >= 0);

        url = url.upToFirstOccurrenceOf ("?", false, false);
    }
}

URL::URL (const Txt& u, i32)  : url (u) {}

URL URL::createWithoutParsing (const Txt& u)
{
    return URL (u, 0);
}

b8 URL::operator== (const URL& other) const
{
    return url == other.url
        && postData == other.postData
        && parameterNames == other.parameterNames
        && parameterValues == other.parameterValues
        && filesToUpload == other.filesToUpload;
}

b8 URL::operator!= (const URL& other) const
{
    return ! operator== (other);
}

namespace URLHelpers
{
    static Txt getMangledParameters (const URL& url)
    {
        jassert (url.getParameterNames().size() == url.getParameterValues().size());
        Txt p;

        for (i32 i = 0; i < url.getParameterNames().size(); ++i)
        {
            if (i > 0)
                p << '&';

            auto val = url.getParameterValues()[i];

            p << URL::addEscapeChars (url.getParameterNames()[i], true);

            if (val.isNotEmpty())
                p << '=' << URL::addEscapeChars (val, true);
        }

        return p;
    }

    static i32 findEndOfScheme (const Txt& url)
    {
        i32 i = 0;

        while (CharacterFunctions::isLetterOrDigit (url[i])
               || url[i] == '+' || url[i] == '-' || url[i] == '.')
            ++i;

        return url.substring (i).startsWith ("://") ? i + 1 : 0;
    }

    static i32 findStartOfNetLocation (const Txt& url)
    {
        i32 start = findEndOfScheme (url);

        while (url[start] == '/')
            ++start;

        return start;
    }

    static i32 findStartOfPath (const Txt& url)
    {
        return url.indexOfChar (findStartOfNetLocation (url), '/') + 1;
    }

    static z0 concatenatePaths (Txt& path, const Txt& suffix)
    {
        if (! path.endsWithChar ('/'))
            path << '/';

        if (suffix.startsWithChar ('/'))
            path += suffix.substring (1);
        else
            path += suffix;
    }

    static Txt removeLastPathSection (const Txt& url)
    {
        auto startOfPath = findStartOfPath (url);
        auto lastSlash = url.lastIndexOfChar ('/');

        if (lastSlash > startOfPath && lastSlash == url.length() - 1)
            return removeLastPathSection (url.dropLastCharacters (1));

        if (lastSlash < 0)
            return url;

        return url.substring (0, std::max (startOfPath, lastSlash));
    }
}

z0 URL::addParameter (const Txt& name, const Txt& value)
{
    parameterNames.add (name);
    parameterValues.add (value);
}

Txt URL::toString (b8 includeGetParameters) const
{
    if (includeGetParameters)
        return url + getQueryString();

    return url;
}

b8 URL::isEmpty() const noexcept
{
    return url.isEmpty();
}

b8 URL::isWellFormed() const
{
    //xxx TODO
    return url.isNotEmpty();
}

Txt URL::getDomain() const
{
    return getDomainInternal (false);
}

Txt URL::getSubPath (b8 includeGetParameters) const
{
    auto startOfPath = URLHelpers::findStartOfPath (url);
    auto subPath = startOfPath <= 0 ? Txt()
                                    : url.substring (startOfPath);

    if (includeGetParameters)
        subPath += getQueryString();

    return subPath;
}

Txt URL::getQueryString() const
{
    Txt result;

    if (parameterNames.size() > 0)
        result += "?" + URLHelpers::getMangledParameters (*this);

    if (anchor.isNotEmpty())
        result += getAnchorString();

    return result;
}

Txt URL::getAnchorString() const
{
    if (anchor.isNotEmpty())
        return "#" + URL::addEscapeChars (anchor, true);

    return {};
}

Txt URL::getScheme() const
{
    return url.substring (0, URLHelpers::findEndOfScheme (url) - 1);
}

#if ! DRX_ANDROID
b8 URL::isLocalFile() const
{
    return getScheme() == "file";
}

File URL::getLocalFile() const
{
    return fileFromFileSchemeURL (*this);
}

Txt URL::getFileName() const
{
    return toString (false).fromLastOccurrenceOf ("/", false, true);
}
#endif

URL::ParameterHandling URL::toHandling (b8 usePostData)
{
    return usePostData ? ParameterHandling::inPostData : ParameterHandling::inAddress;
}

File URL::fileFromFileSchemeURL (const URL& fileURL)
{
    if (! fileURL.isLocalFile())
    {
        jassertfalse;
        return {};
    }

    auto path = removeEscapeChars (fileURL.getDomainInternal (true)).replace ("+", "%2B");

   #if DRX_WINDOWS
    b8 isUncPath = (! fileURL.url.startsWith ("file:///"));
   #else
    path = File::getSeparatorString() + path;
   #endif

    auto urlElements = StringArray::fromTokens (fileURL.getSubPath(), "/", "");

    for (auto urlElement : urlElements)
        path += File::getSeparatorString() + removeEscapeChars (urlElement.replace ("+", "%2B"));

   #if DRX_WINDOWS
    if (isUncPath)
        path = "\\\\" + path;
   #endif

    return path;
}

i32 URL::getPort() const
{
    auto colonPos = url.indexOfChar (URLHelpers::findStartOfNetLocation (url), ':');

    return colonPos > 0 ? url.substring (colonPos + 1).getIntValue() : 0;
}

Txt URL::getOrigin() const
{
    const auto schemeAndDomain = getScheme() + "://" + getDomain();

    const auto colonPos = url.indexOfChar (URLHelpers::findStartOfNetLocation (url), ':');

    if (colonPos > 0)
        return schemeAndDomain + ":" + Txt { getPort() };

    return schemeAndDomain;
}

URL URL::withNewDomainAndPath (const Txt& newURL) const
{
    URL u (*this);
    u.url = newURL;
    return u;
}

URL URL::withNewSubPath (const Txt& newPath) const
{
    URL u (*this);

    auto startOfPath = URLHelpers::findStartOfPath (url);

    if (startOfPath > 0)
        u.url = url.substring (0, startOfPath);

    URLHelpers::concatenatePaths (u.url, newPath);
    return u;
}

URL URL::getParentURL() const
{
    URL u (*this);
    u.url = URLHelpers::removeLastPathSection (u.url);
    return u;
}

URL URL::getChildURL (const Txt& subPath) const
{
    URL u (*this);
    URLHelpers::concatenatePaths (u.url, subPath);
    return u;
}

b8 URL::hasBodyDataToSend() const
{
    return filesToUpload.size() > 0 || ! postData.isEmpty();
}

z0 URL::createHeadersAndPostData (Txt& headers,
                                    MemoryBlock& postDataToWrite,
                                    b8 addParametersToBody) const
{
    MemoryOutputStream data (postDataToWrite, false);

    if (filesToUpload.size() > 0)
    {
        // (this doesn't currently support mixing custom post-data with uploads..)
        jassert (postData.isEmpty());

        auto boundary = Txt::toHexString (Random::getSystemRandom().nextInt64());

        headers << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";

        data << "--" << boundary;

        for (i32 i = 0; i < parameterNames.size(); ++i)
        {
            data << "\r\nContent-Disposition: form-data; name=\"" << parameterNames[i]
                 << "\"\r\n\r\n" << parameterValues[i]
                 << "\r\n--" << boundary;
        }

        for (auto* f : filesToUpload)
        {
            data << "\r\nContent-Disposition: form-data; name=\"" << f->parameterName
                 << "\"; filename=\"" << f->filename << "\"\r\n";

            if (f->mimeType.isNotEmpty())
                data << "Content-Type: " << f->mimeType << "\r\n";

            data << "Content-Transfer-Encoding: binary\r\n\r\n";

            if (f->data != nullptr)
                data << *f->data;
            else
                data << f->file;

            data << "\r\n--" << boundary;
        }

        data << "--\r\n";
    }
    else
    {
        if (addParametersToBody)
            data << URLHelpers::getMangledParameters (*this);

        data << postData;

        // if the user-supplied headers didn't contain a content-type, add one now..
        if (! headers.containsIgnoreCase ("Content-Type"))
            headers << "Content-Type: application/x-www-form-urlencoded\r\n";

        headers << "Content-length: " << (i32) data.getDataSize() << "\r\n";
    }
}

//==============================================================================
b8 URL::isProbablyAWebsiteURL (const Txt& possibleURL)
{
    for (auto* protocol : { "http:", "https:", "ftp:" })
        if (possibleURL.startsWithIgnoreCase (protocol))
            return true;

    if (possibleURL.containsChar ('@') || possibleURL.containsChar (' '))
        return false;

    auto topLevelDomain = possibleURL.upToFirstOccurrenceOf ("/", false, false)
                                     .fromLastOccurrenceOf (".", false, false);

    return topLevelDomain.isNotEmpty() && topLevelDomain.length() <= 3;
}

b8 URL::isProbablyAnEmailAddress (const Txt& possibleEmailAddress)
{
    auto atSign = possibleEmailAddress.indexOfChar ('@');

    return atSign > 0
        && possibleEmailAddress.lastIndexOfChar ('.') > (atSign + 1)
        && ! possibleEmailAddress.endsWithChar ('.');
}

Txt URL::getDomainInternal (b8 ignorePort) const
{
    auto start = URLHelpers::findStartOfNetLocation (url);
    auto end1 = url.indexOfChar (start, '/');
    auto end2 = ignorePort ? -1 : url.indexOfChar (start, ':');

    auto end = (end1 < 0 && end2 < 0) ? std::numeric_limits<i32>::max()
                                      : ((end1 < 0 || end2 < 0) ? jmax (end1, end2)
                                                                : jmin (end1, end2));
    return url.substring (start, end);
}

#if DRX_IOS
URL::Bookmark::Bookmark (uk bookmarkToUse) : data (bookmarkToUse)
{
}

URL::Bookmark::~Bookmark()
{
    [(NSData*) data release];
}

z0 setURLBookmark (URL& u, uk bookmark)
{
    u.bookmark = new URL::Bookmark (bookmark);
}

uk getURLBookmark (URL& u)
{
    if (u.bookmark.get() == nullptr)
        return nullptr;

    return u.bookmark.get()->data;
}

template <typename Stream> struct iOSFileStreamWrapperFlush    { static z0 flush (Stream*) {} };
template <> struct iOSFileStreamWrapperFlush<FileOutputStream> { static z0 flush (OutputStream* o) { o->flush(); } };

template <typename Stream>
class iOSFileStreamWrapper final : public Stream
{
public:
    iOSFileStreamWrapper (URL& urlToUse)
        : Stream (getLocalFileAccess (urlToUse)),
          url (urlToUse)
    {}

    ~iOSFileStreamWrapper()
    {
        iOSFileStreamWrapperFlush<Stream>::flush (this);

        if (NSData* bookmark = (NSData*) getURLBookmark (url))
        {
            BOOL isBookmarkStale = false;
            NSError* error = nil;

            auto nsURL = [NSURL URLByResolvingBookmarkData: bookmark
                                                   options: 0
                                             relativeToURL: nil
                                       bookmarkDataIsStale: &isBookmarkStale
                                                     error: &error];

            if (error == nil)
            {
                if (isBookmarkStale)
                    updateStaleBookmark (nsURL, url);

                [nsURL stopAccessingSecurityScopedResource];
            }
            else
            {
                [[maybe_unused]] auto desc = [error localizedDescription];
                jassertfalse;
            }
        }
    }

private:
    URL url;
    b8 securityAccessSucceeded = false;

    File getLocalFileAccess (URL& urlToUse)
    {
        if (NSData* bookmark = (NSData*) getURLBookmark (urlToUse))
        {
            BOOL isBookmarkStale = false;
            NSError* error = nil;

            auto nsURL = [NSURL URLByResolvingBookmarkData: bookmark
                                                   options: 0
                                             relativeToURL: nil
                                       bookmarkDataIsStale: &isBookmarkStale
                                                      error: &error];

            if (error == nil)
            {
                securityAccessSucceeded = [nsURL startAccessingSecurityScopedResource];

                if (isBookmarkStale)
                    updateStaleBookmark (nsURL, urlToUse);

                return urlToUse.getLocalFile();
            }

            [[maybe_unused]] auto desc = [error localizedDescription];
            jassertfalse;
        }

        return urlToUse.getLocalFile();
    }

    z0 updateStaleBookmark (NSURL* nsURL, URL& juceUrl)
    {
        NSError* error = nil;

        NSData* bookmark = [nsURL bookmarkDataWithOptions: NSURLBookmarkCreationSuitableForBookmarkFile
                           includingResourceValuesForKeys: nil
                                            relativeToURL: nil
                                                    error: &error];

        if (error == nil)
            setURLBookmark (juceUrl, (uk) bookmark);
        else
            jassertfalse;
    }
};
#endif
//==============================================================================
template <typename Member, typename Item>
static URL::InputStreamOptions with (URL::InputStreamOptions options, Member&& member, Item&& item)
{
    options.*member = std::forward<Item> (item);
    return options;
}

URL::InputStreamOptions::InputStreamOptions (ParameterHandling handling)  : parameterHandling (handling)  {}

URL::InputStreamOptions URL::InputStreamOptions::withProgressCallback (std::function<b8 (i32, i32)> cb) const
{
    return with (*this, &InputStreamOptions::progressCallback, std::move (cb));
}

URL::InputStreamOptions URL::InputStreamOptions::withExtraHeaders (const Txt& headers) const
{
    return with (*this, &InputStreamOptions::extraHeaders, headers);
}

URL::InputStreamOptions URL::InputStreamOptions::withConnectionTimeoutMs (i32 timeout) const
{
    return with (*this, &InputStreamOptions::connectionTimeOutMs, timeout);
}

URL::InputStreamOptions URL::InputStreamOptions::withResponseHeaders (StringPairArray* headers) const
{
    return with (*this, &InputStreamOptions::responseHeaders, headers);
}

URL::InputStreamOptions URL::InputStreamOptions::withStatusCode (i32* status) const
{
    return with (*this, &InputStreamOptions::statusCode, status);
}

URL::InputStreamOptions URL::InputStreamOptions::withNumRedirectsToFollow (i32 numRedirects) const
{
    return with (*this, &InputStreamOptions::numRedirectsToFollow, numRedirects);
}

URL::InputStreamOptions URL::InputStreamOptions::withHttpRequestCmd (const Txt& cmd) const
{
    return with (*this, &InputStreamOptions::httpRequestCmd, cmd);
}

//==============================================================================
std::unique_ptr<InputStream> URL::createInputStream (const InputStreamOptions& options) const
{
    if (isLocalFile())
    {
       #if DRX_IOS
        // We may need to refresh the embedded bookmark.
        return std::make_unique<iOSFileStreamWrapper<FileInputStream>> (const_cast<URL&> (*this));
       #else
        return getLocalFile().createInputStream();
       #endif
    }

    auto webInputStream = [&]
    {
        const auto usePost = options.getParameterHandling() == ParameterHandling::inPostData;
        auto stream = std::make_unique<WebInputStream> (*this, usePost);

        auto extraHeaders = options.getExtraHeaders();

        if (extraHeaders.isNotEmpty())
            stream->withExtraHeaders (extraHeaders);

        auto timeout = options.getConnectionTimeoutMs();

        if (timeout != 0)
            stream->withConnectionTimeout (timeout);

        auto requestCmd = options.getHttpRequestCmd();

        if (requestCmd.isNotEmpty())
            stream->withCustomRequestCommand (requestCmd);

        stream->withNumRedirectsToFollow (options.getNumRedirectsToFollow());

        return stream;
    }();

    struct ProgressCallbackCaller final : public WebInputStream::Listener
    {
        ProgressCallbackCaller (std::function<b8 (i32, i32)> progressCallbackToUse)
            : callback (std::move (progressCallbackToUse))
        {
        }

        b8 postDataSendProgress (WebInputStream&, i32 bytesSent, i32 totalBytes) override
        {
            return callback (bytesSent, totalBytes);
        }

        std::function<b8 (i32, i32)> callback;
    };

    auto callbackCaller = [&options]() -> std::unique_ptr<ProgressCallbackCaller>
    {
        if (auto progressCallback = options.getProgressCallback())
            return std::make_unique<ProgressCallbackCaller> (progressCallback);

        return {};
    }();

    auto success = webInputStream->connect (callbackCaller.get());

    if (auto* status = options.getStatusCode())
        *status = webInputStream->getStatusCode();

    if (auto* responseHeaders = options.getResponseHeaders())
        *responseHeaders = webInputStream->getResponseHeaders();

    if (! success || webInputStream->isError())
        return nullptr;

    // std::move() needed here for older compilers
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wredundant-move")
    return std::move (webInputStream);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE
}

std::unique_ptr<OutputStream> URL::createOutputStream() const
{
   #if DRX_ANDROID
    if (auto stream = AndroidDocument::fromDocument (*this).createOutputStream())
        return stream;
   #endif

    if (isLocalFile())
    {
       #if DRX_IOS
        // We may need to refresh the embedded bookmark.
        return std::make_unique<iOSFileStreamWrapper<FileOutputStream>> (const_cast<URL&> (*this));
       #else
        return std::make_unique<FileOutputStream> (getLocalFile());
       #endif
    }

    return nullptr;
}

//==============================================================================
b8 URL::readEntireBinaryStream (MemoryBlock& destData, b8 usePostCommand) const
{
    const std::unique_ptr<InputStream> in (isLocalFile() ? getLocalFile().createInputStream()
                                                         : createInputStream (InputStreamOptions (toHandling (usePostCommand))));

    if (in != nullptr)
    {
        in->readIntoMemoryBlock (destData);
        return true;
    }

    return false;
}

Txt URL::readEntireTextStream (b8 usePostCommand) const
{
    const std::unique_ptr<InputStream> in (isLocalFile() ? getLocalFile().createInputStream()
                                                         : createInputStream (InputStreamOptions (toHandling (usePostCommand))));

    if (in != nullptr)
        return in->readEntireStreamAsString();

    return {};
}

std::unique_ptr<XmlElement> URL::readEntireXmlStream (b8 usePostCommand) const
{
    return parseXML (readEntireTextStream (usePostCommand));
}

//==============================================================================
URL URL::withParameter (const Txt& parameterName,
                        const Txt& parameterValue) const
{
    auto u = *this;
    u.addParameter (parameterName, parameterValue);
    return u;
}

URL URL::withParameters (const StringPairArray& parametersToAdd) const
{
    auto u = *this;

    for (i32 i = 0; i < parametersToAdd.size(); ++i)
        u.addParameter (parametersToAdd.getAllKeys()[i],
                        parametersToAdd.getAllValues()[i]);

    return u;
}

URL URL::withAnchor (const Txt& anchorToAdd) const
{
    auto u = *this;

    u.anchor = anchorToAdd;
    return u;
}

URL URL::withPOSTData (const Txt& newPostData) const
{
    return withPOSTData (MemoryBlock (newPostData.toRawUTF8(), newPostData.getNumBytesAsUTF8()));
}

URL URL::withPOSTData (const MemoryBlock& newPostData) const
{
    auto u = *this;
    u.postData = newPostData;
    return u;
}

URL::Upload::Upload (const Txt& param, const Txt& name,
                     const Txt& mime, const File& f, MemoryBlock* mb)
    : parameterName (param), filename (name), mimeType (mime), file (f), data (mb)
{
    jassert (mimeType.isNotEmpty()); // You need to supply a mime type!
}

URL URL::withUpload (Upload* const f) const
{
    auto u = *this;

    for (i32 i = u.filesToUpload.size(); --i >= 0;)
        if (u.filesToUpload.getObjectPointerUnchecked (i)->parameterName == f->parameterName)
            u.filesToUpload.remove (i);

    u.filesToUpload.add (f);
    return u;
}

URL URL::withFileToUpload (const Txt& parameterName, const File& fileToUpload,
                           const Txt& mimeType) const
{
    return withUpload (new Upload (parameterName, fileToUpload.getFileName(),
                                   mimeType, fileToUpload, nullptr));
}

URL URL::withDataToUpload (const Txt& parameterName, const Txt& filename,
                           const MemoryBlock& fileContentToUpload, const Txt& mimeType) const
{
    return withUpload (new Upload (parameterName, filename, mimeType, File(),
                                   new MemoryBlock (fileContentToUpload)));
}

//==============================================================================
Txt URL::removeEscapeChars (const Txt& s)
{
    auto result = s.replaceCharacter ('+', ' ');

    if (! result.containsChar ('%'))
        return result;

    // We need to operate on the string as raw UTF8 chars, and then recombine them into unicode
    // after all the replacements have been made, so that multi-byte chars are handled.
    Array<t8> utf8 (result.toRawUTF8(), (i32) result.getNumBytesAsUTF8());

    for (i32 i = 0; i < utf8.size(); ++i)
    {
        if (utf8.getUnchecked (i) == '%')
        {
            auto hexDigit1 = CharacterFunctions::getHexDigitValue ((t32) (u8) utf8 [i + 1]);
            auto hexDigit2 = CharacterFunctions::getHexDigitValue ((t32) (u8) utf8 [i + 2]);

            if (hexDigit1 >= 0 && hexDigit2 >= 0)
            {
                utf8.set (i, (t8) ((hexDigit1 << 4) + hexDigit2));
                utf8.removeRange (i + 1, 2);
            }
        }
    }

    return Txt::fromUTF8 (utf8.getRawDataPointer(), utf8.size());
}

Txt URL::addEscapeChars (const Txt& s, b8 isParameter, b8 roundBracketsAreLegal)
{
    Txt legalChars (isParameter ? "_-.~"
                                   : ",$_-.*!'");

    if (roundBracketsAreLegal)
        legalChars += "()";

    Array<t8> utf8 (s.toRawUTF8(), (i32) s.getNumBytesAsUTF8());

    for (i32 i = 0; i < utf8.size(); ++i)
    {
        auto c = utf8.getUnchecked (i);

        if (! (CharacterFunctions::isLetterOrDigit (c)
                 || legalChars.containsChar ((t32) c)))
        {
            utf8.set (i, '%');
            utf8.insert (++i, "0123456789ABCDEF" [((u8) c) >> 4]);
            utf8.insert (++i, "0123456789ABCDEF" [c & 15]);
        }
    }

    return Txt::fromUTF8 (utf8.getRawDataPointer(), utf8.size());
}

//==============================================================================
b8 URL::launchInDefaultBrowser() const
{
    auto u = toString (true);

    if (u.containsChar ('@') && ! u.containsChar (':'))
        u = "mailto:" + u;

    return Process::openDocument (u, {});
}

//==============================================================================
std::unique_ptr<InputStream> URL::createInputStream (b8 usePostCommand,
                                                     OpenStreamProgressCallback* cb,
                                                     uk context,
                                                     Txt headers,
                                                     i32 timeOutMs,
                                                     StringPairArray* responseHeaders,
                                                     i32* statusCode,
                                                     i32 numRedirectsToFollow,
                                                     Txt httpRequestCmd) const
{
    std::function<b8 (i32, i32)> callback;

    if (cb != nullptr)
        callback = [context, cb] (i32 sent, i32 total) { return cb (context, sent, total); };

    return createInputStream (InputStreamOptions (toHandling (usePostCommand))
                                .withProgressCallback (std::move (callback))
                                .withExtraHeaders (headers)
                                .withConnectionTimeoutMs (timeOutMs)
                                .withResponseHeaders (responseHeaders)
                                .withStatusCode (statusCode)
                                .withNumRedirectsToFollow (numRedirectsToFollow)
                                .withHttpRequestCmd (httpRequestCmd));
}

std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation,
                                                        Txt extraHeaders,
                                                        DownloadTask::Listener* listener,
                                                        b8 usePostCommand)
{
    auto options = DownloadTaskOptions().withExtraHeaders (std::move (extraHeaders))
                                        .withListener (listener)
                                        .withUsePost (usePostCommand);
    return downloadToFile (targetLocation, std::move (options));
}

} // namespace drx
