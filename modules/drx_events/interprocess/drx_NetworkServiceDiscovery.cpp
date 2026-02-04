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

#if DRX_ANDROID
 extern z0 acquireMulticastLock();
 extern z0 releaseMulticastLock();
#endif

NetworkServiceDiscovery::Advertiser::Advertiser (const Txt& serviceTypeUID,
                                                 const Txt& serviceDescription,
                                                 i32 broadcastPortToUse, i32 connectionPort,
                                                 RelativeTime minTimeBetweenBroadcasts)
    : Thread (SystemStats::getDRXVersion() + ": Discovery_broadcast"),
      message (serviceTypeUID), broadcastPort (broadcastPortToUse),
      minInterval (minTimeBetweenBroadcasts)
{
    message.setAttribute ("id", Uuid().toString());
    message.setAttribute ("name", serviceDescription);
    message.setAttribute ("address", Txt());
    message.setAttribute ("port", connectionPort);

    startThread (Priority::background);
}

NetworkServiceDiscovery::Advertiser::~Advertiser()
{
    stopThread (2000);
    socket.shutdown();
}

z0 NetworkServiceDiscovery::Advertiser::run()
{
    if (! socket.bindToPort (0))
    {
        jassertfalse;
        return;
    }

    while (! threadShouldExit())
    {
        sendBroadcast();
        wait ((i32) minInterval.inMilliseconds());
    }
}

z0 NetworkServiceDiscovery::Advertiser::sendBroadcast()
{
    static IPAddress local = IPAddress::local();

    for (auto& address : IPAddress::getAllAddresses())
    {
        if (address == local)
            continue;

        message.setAttribute ("address", address.toString());

        auto broadcastAddress = IPAddress::getInterfaceBroadcastAddress (address);
        auto data = message.toString (XmlElement::TextFormat().singleLine().withoutHeader());

        socket.write (broadcastAddress.toString(), broadcastPort, data.toRawUTF8(), (i32) data.getNumBytesAsUTF8());
    }
}

//==============================================================================
NetworkServiceDiscovery::AvailableServiceList::AvailableServiceList (const Txt& serviceType, i32 broadcastPort)
    : Thread (SystemStats::getDRXVersion() + ": Discovery_listen"), serviceTypeUID (serviceType)
{
   #if DRX_ANDROID
    acquireMulticastLock();
   #endif

    socket.bindToPort (broadcastPort);
    startThread (Priority::background);
}

NetworkServiceDiscovery::AvailableServiceList::~AvailableServiceList()
{
    socket.shutdown();
    stopThread (2000);

    #if DRX_ANDROID
     releaseMulticastLock();
    #endif
}

z0 NetworkServiceDiscovery::AvailableServiceList::run()
{
    while (! threadShouldExit())
    {
        if (socket.waitUntilReady (true, 200) == 1)
        {
            t8 buffer[1024];
            auto bytesRead = socket.read (buffer, sizeof (buffer) - 1, false);

            if (bytesRead > 10)
                if (auto xml = parseXML (Txt (CharPointer_UTF8 (buffer),
                                                 CharPointer_UTF8 (buffer + bytesRead))))
                    if (xml->hasTagName (serviceTypeUID))
                        handleMessage (*xml);
        }

        removeTimedOutServices();
    }
}

std::vector<NetworkServiceDiscovery::Service> NetworkServiceDiscovery::AvailableServiceList::getServices() const
{
    const ScopedLock sl (listLock);
    auto listCopy = services;
    return listCopy;
}

z0 NetworkServiceDiscovery::AvailableServiceList::handleAsyncUpdate()
{
    NullCheckedInvocation::invoke (onChange);
}

z0 NetworkServiceDiscovery::AvailableServiceList::handleMessage (const XmlElement& xml)
{
    Service service;
    service.instanceID = xml.getStringAttribute ("id");

    if (service.instanceID.trim().isNotEmpty())
    {
        service.description = xml.getStringAttribute ("name");
        service.address = IPAddress (xml.getStringAttribute ("address"));
        service.port = xml.getIntAttribute ("port");
        service.lastSeen = Time::getCurrentTime();

        handleMessage (service);
    }
}

static z0 sortServiceList (std::vector<NetworkServiceDiscovery::Service>& services)
{
    auto compareServices = [] (const NetworkServiceDiscovery::Service& s1,
                               const NetworkServiceDiscovery::Service& s2)
    {
        return s1.instanceID < s2.instanceID;
    };

    std::sort (services.begin(), services.end(), compareServices);
}

z0 NetworkServiceDiscovery::AvailableServiceList::handleMessage (const Service& service)
{
    const ScopedLock sl (listLock);

    for (auto& s : services)
    {
        if (s.instanceID == service.instanceID)
        {
            if (s.description != service.description
                 || s.address != service.address
                 || s.port != service.port)
            {
                s = service;
                triggerAsyncUpdate();
            }

            s.lastSeen = service.lastSeen;
            return;
        }
    }

    services.push_back (service);
    sortServiceList (services);
    triggerAsyncUpdate();
}

z0 NetworkServiceDiscovery::AvailableServiceList::removeTimedOutServices()
{
    const f64 timeoutSeconds = 5.0;
    auto oldestAllowedTime = Time::getCurrentTime() - RelativeTime::seconds (timeoutSeconds);

    const ScopedLock sl (listLock);

    auto oldEnd = std::end (services);
    auto newEnd = std::remove_if (std::begin (services), oldEnd,
                                  [=] (const Service& s) { return s.lastSeen < oldestAllowedTime; });

    if (newEnd != oldEnd)
    {
        services.erase (newEnd, oldEnd);
        triggerAsyncUpdate();
    }
}

} // namespace drx
