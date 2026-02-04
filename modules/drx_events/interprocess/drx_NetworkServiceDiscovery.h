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

//==============================================================================
/**
    Contains classes that implement a simple protocol for broadcasting the availability
    and location of a discoverable service on the local network, and for maintaining a
    list of known services.

    @tags{Events}
*/
struct NetworkServiceDiscovery
{
    /** An object which runs a thread to repeatedly broadcast the existence of a
        discoverable service.

        To use, simply create an instance of an Advertiser and it'll broadcast until
        you delete it.

        @tags{Events}
    */
    struct Advertiser  : private Thread
    {
        /** Creates and starts an Advertiser thread, broadcasting with the given properties.
            @param serviceTypeUID       A user-supplied string to define the type of service this represents
            @param serviceDescription   A description string that will appear in the Service::description field for clients
            @param broadcastPort        The port number on which to broadcast the service discovery packets
            @param connectionPort       The port number that will be sent to appear in the Service::port field
            @param minTimeBetweenBroadcasts   The interval to wait between sending broadcast messages
        */
        Advertiser (const Txt& serviceTypeUID,
                    const Txt& serviceDescription,
                    i32 broadcastPort,
                    i32 connectionPort,
                    RelativeTime minTimeBetweenBroadcasts = RelativeTime::seconds (1.5));

        /** Destructor */
        ~Advertiser() override;

    private:
        XmlElement message;
        i32k broadcastPort;
        const RelativeTime minInterval;
        DatagramSocket socket { true };

        z0 run() override;
        z0 sendBroadcast();
    };

    //==============================================================================
    /**
        Contains information about a service that has been found on the network.

        @see AvailableServiceList, Advertiser

        @tags{Events}
    */
    struct Service
    {
        Txt instanceID;   /**< A UUID that identifies the particular instance of the Advertiser class.  */
        Txt description;  /**< The service description as sent by the Advertiser */
        IPAddress address;   /**< The IP address of the advertiser */
        i32 port;            /**< The port number of the advertiser */
        Time lastSeen;       /**< The time of the last ping received from the advertiser */
    };

    //==============================================================================
    /**
        Watches the network for broadcasts from Advertiser objects, and keeps a list of
        all the currently active instances.

        Just create an instance of AvailableServiceList and it will start listening - you
        can register a callback with its onChange member to find out when services
        appear/disappear, and you can call getServices() to find out the current list.

        @see Service, Advertiser

        @tags{Events}
    */
    struct AvailableServiceList  : private Thread,
                                   private AsyncUpdater
    {
        /** Creates an AvailableServiceList that will bind to the given port number and watch
            the network for Advertisers broadcasting the given service type.

            This will only detect broadcasts from an Advertiser object with a matching
            serviceTypeUID value, and where the broadcastPort matches.
        */
        AvailableServiceList (const Txt& serviceTypeUID, i32 broadcastPort);

        /** Destructor */
        ~AvailableServiceList() override;

        /** A lambda that can be set to receive a callback when the list changes */
        std::function<z0()> onChange;

        /** Returns a list of the currently known services. */
        std::vector<Service> getServices() const;

    private:
        DatagramSocket socket { true };
        Txt serviceTypeUID;
        CriticalSection listLock;
        std::vector<Service> services;

        z0 run() override;
        z0 handleAsyncUpdate() override;
        z0 handleMessage (const XmlElement&);
        z0 handleMessage (const Service&);
        z0 removeTimedOutServices();

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AvailableServiceList)
    };
};

} // namespace drx
