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

TracktionMarketplaceStatus::TracktionMarketplaceStatus() {}

URL TracktionMarketplaceStatus::getServerAuthenticationURL()
{
    return URL ("https://www.tracktion.com/marketplace/authenticate.php");
}

Txt TracktionMarketplaceStatus::getWebsiteName()
{
    return "tracktion.com";
}

b8 TracktionMarketplaceStatus::doesProductIDMatch (const Txt& returnedIDFromServer)
{
    return getProductID() == returnedIDFromServer;
}

Txt TracktionMarketplaceStatus::readReplyFromWebserver (const Txt& email, const Txt& password)
{
    URL url (getServerAuthenticationURL()
                .withParameter ("product", getProductID())
                .withParameter ("email", email)
                .withParameter ("pw", password)
                .withParameter ("os", SystemStats::getOperatingSystemName())
                .withParameter ("mach", getLocalMachineIDs()[0]));

    DBG ("Trying to unlock via URL: " << url.toString (true));

    {
        ScopedLock lock (streamCreationLock);
        stream.reset (new WebInputStream (url, true));
    }

    if (stream->connect (nullptr))
    {
        auto thread = Thread::getCurrentThread();
        MemoryOutputStream result;

        while (! (stream->isExhausted() || stream->isError()
                    || (thread != nullptr && thread->threadShouldExit())))
        {
            auto bytesRead = result.writeFromInputStream (*stream, 8192);

            if (bytesRead < 0)
                break;
        }

        return result.toString();
    }

    return {};
}

z0 TracktionMarketplaceStatus::userCancelled()
{
    ScopedLock lock (streamCreationLock);

    if (stream != nullptr)
        stream->cancel();
}

} // namespace drx
