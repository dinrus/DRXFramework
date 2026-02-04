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

namespace
{
    struct InterfaceInfo
    {
        IPAddress interfaceAddress, broadcastAddress;
    };

    inline b8 operator== (const InterfaceInfo& lhs, const InterfaceInfo& rhs)
    {
        return lhs.interfaceAddress == rhs.interfaceAddress
            && lhs.broadcastAddress == rhs.broadcastAddress;
    }

   #if ! DRX_WASM
    static IPAddress makeAddress (const sockaddr_in6* addr_in)
    {
        if (addr_in == nullptr)
            return {};

        auto addr = addr_in->sin6_addr;

        IPAddressByteUnion temp;
        u16 arr[8];

        for (i32 i = 0; i < 8; ++i) // Swap bytes from network to host order
        {
            temp.split[0] = addr.s6_addr[i * 2 + 1];
            temp.split[1] = addr.s6_addr[i * 2];

            arr[i] = temp.combined;
        }

        return IPAddress (arr);
    }

    static IPAddress makeAddress (const sockaddr_in* addr_in)
    {
        if (addr_in->sin_addr.s_addr == INADDR_NONE)
            return {};

        return IPAddress (ntohl (addr_in->sin_addr.s_addr));
    }

    b8 populateInterfaceInfo (struct ifaddrs* ifa, InterfaceInfo& interfaceInfo)
    {
        if (ifa->ifa_addr != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                auto interfaceAddressInfo = unalignedPointerCast<sockaddr_in*> (ifa->ifa_addr);
                auto broadcastAddressInfo = unalignedPointerCast<sockaddr_in*> (ifa->ifa_dstaddr);

                if (interfaceAddressInfo->sin_addr.s_addr != INADDR_NONE)
                {
                    interfaceInfo.interfaceAddress = makeAddress (interfaceAddressInfo);
                    interfaceInfo.broadcastAddress = makeAddress (broadcastAddressInfo);
                    return true;
                }
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            {
                interfaceInfo.interfaceAddress = makeAddress (unalignedPointerCast<sockaddr_in6*> (ifa->ifa_addr));
                interfaceInfo.broadcastAddress = makeAddress (unalignedPointerCast<sockaddr_in6*> (ifa->ifa_dstaddr));
                return true;
            }
        }

        return false;
    }
   #endif

    Array<InterfaceInfo> getAllInterfaceInfo()
    {
        Array<InterfaceInfo> interfaces;

       #if DRX_WASM
        // TODO
       #else
        struct ifaddrs* ifaddr = nullptr;

        if (getifaddrs (&ifaddr) != -1)
        {
            for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
            {
                InterfaceInfo i;

                if (populateInterfaceInfo (ifa, i))
                    interfaces.addIfNotAlreadyThere (i);
            }

            freeifaddrs (ifaddr);
        }
       #endif

        return interfaces;
    }
}

z0 IPAddress::findAllAddresses (Array<IPAddress>& result, b8 includeIPv6)
{
    for (auto& i : getAllInterfaceInfo())
        if (includeIPv6 || ! i.interfaceAddress.isIPv6)
            result.addIfNotAlreadyThere (i.interfaceAddress);
}

IPAddress IPAddress::getInterfaceBroadcastAddress (const IPAddress& interfaceAddress)
{
    for (auto& i : getAllInterfaceInfo())
        if (i.interfaceAddress == interfaceAddress)
            return i.broadcastAddress;

    return {};
}

} // namespace drx
