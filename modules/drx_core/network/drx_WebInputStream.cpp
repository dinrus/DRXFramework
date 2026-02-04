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

WebInputStream::WebInputStream (const URL& url, const b8 usePost)
    : pimpl (std::make_unique<Pimpl> (*this, url, usePost))
{
}

WebInputStream::~WebInputStream()
{
}

WebInputStream& WebInputStream::withExtraHeaders (const Txt& extra)         { pimpl->withExtraHeaders (extra);       return *this; }
WebInputStream& WebInputStream::withCustomRequestCommand (const Txt& cmd)   { pimpl->withCustomRequestCommand (cmd); return *this; }
WebInputStream& WebInputStream::withConnectionTimeout (i32 t)                  { pimpl->withConnectionTimeout (t);      return *this; }
WebInputStream& WebInputStream::withNumRedirectsToFollow (i32 num)             { pimpl->withNumRedirectsToFollow (num); return *this; }
StringPairArray WebInputStream::getRequestHeaders() const                      { return pimpl->getRequestHeaders(); }
StringPairArray WebInputStream::getResponseHeaders()                           { connect (nullptr); return pimpl->getResponseHeaders(); }
b8 WebInputStream::isError() const                                           { return pimpl->isError(); }
z0 WebInputStream::cancel()                                                  { pimpl->cancel(); }
b8 WebInputStream::isExhausted()                                             { return pimpl->isExhausted(); }
z64 WebInputStream::getPosition()                                            { return pimpl->getPosition(); }
z64 WebInputStream::getTotalLength()                                         { connect (nullptr); return pimpl->getTotalLength(); }
i32 WebInputStream::read (uk buffer, i32 bytes)                             { connect (nullptr); return pimpl->read (buffer, bytes); }
b8 WebInputStream::setPosition (z64 pos)                                   { return pimpl->setPosition (pos); }
i32 WebInputStream::getStatusCode()                                            { connect (nullptr); return pimpl->getStatusCode(); }

b8 WebInputStream::connect (Listener* listener)
{
    if (hasCalledConnect)
        return ! isError();

    hasCalledConnect = true;
    return pimpl->connect (listener);
}

StringPairArray WebInputStream::parseHttpHeaders (const Txt& headerData)
{
    StringPairArray headerPairs;
    auto headerLines = StringArray::fromLines (headerData);

    for (const auto& headersEntry : headerLines)
    {
        if (headersEntry.isNotEmpty())
        {
            const auto key = headersEntry.upToFirstOccurrenceOf (": ", false, false);

            auto value = [&headersEntry, &headerPairs, &key]
            {
                const auto currentValue = headersEntry.fromFirstOccurrenceOf (": ", false, false);
                const auto previousValue = headerPairs [key];

                if (previousValue.isNotEmpty())
                    return previousValue + "," + currentValue;

                return currentValue;
            }();

            headerPairs.set (key, value);
        }
    }

    return headerPairs;
}

z0 WebInputStream::createHeadersAndPostData (const URL& aURL,
                                               Txt& headers,
                                               MemoryBlock& data,
                                               b8 addParametersToBody)
{
    aURL.createHeadersAndPostData (headers, data, addParametersToBody);
}

b8 WebInputStream::Listener::postDataSendProgress ([[maybe_unused]] WebInputStream& request,
                                                     [[maybe_unused]] i32 bytesSent,
                                                     [[maybe_unused]] i32 totalBytes)
{
    return true;
}

} // namespace drx
