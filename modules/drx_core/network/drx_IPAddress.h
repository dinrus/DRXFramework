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
    Represents an IP address.

    @tags{Core}
*/
class DRX_API  IPAddress  final
{
public:
    //==============================================================================
    /** Returns an IP address meaning "any", equivalent to 0.0.0.0 (IPv4) or ::, (IPv6)  */
    static IPAddress any() noexcept;

    /** Returns an IPv4 address meaning "broadcast" (255.255.255.255) */
    static IPAddress broadcast() noexcept;

    /** Returns an IPv4 or IPv6 address meaning "localhost", equivalent to 127.0.0.1 (IPv4) or ::1 (IPv6) */
    static IPAddress local (b8 IPv6 = false) noexcept;

    //==============================================================================
    /** Populates a list of all the IP addresses that this machine is using. */
    static z0 findAllAddresses (Array<IPAddress>& results, b8 includeIPv6 = false);

    /** Populates a list of all the IP addresses that this machine is using. */
    static Array<IPAddress> getAllAddresses (b8 includeIPv6 = false);

    /** Returns the first 'real' address for the local machine.
        Unlike local(), this will attempt to find the machine's actual assigned
        address rather than "127.0.0.1". If there are multiple network cards, this
        may return any of their addresses. If it doesn't find any, then it'll return
        local() as a fallback.
    */
    static IPAddress getLocalAddress (b8 includeIPv6 = false);

    //==============================================================================
    /** Creates a null address - 0.0.0.0 (IPv4) or ::, (IPv6) */
    IPAddress() noexcept;

    /** Creates an IPv4 or IPv6 address by reading 4 or 16 bytes from an array.
        @param bytes The array containing the bytes to read.
        @param IPv6 if true indicates that 16 bytes should be read instead of 4.
    */
    explicit IPAddress (u8k* bytes, b8 IPv6 = false) noexcept;

    /** Creates an IPv6 address from an array of 8 16-bit integers
        @param bytes The array containing the bytes to read.
    */
    explicit IPAddress (u16k* bytes) noexcept;

    /** Creates an IPv4 address from 4 bytes. */
    IPAddress (u8 address1, u8 address2, u8 address3, u8 address4) noexcept;

    /** Creates an IPv6 address from 8 16-bit integers */
    IPAddress (u16 address1, u16 address2, u16 address3, u16 address4,
               u16 address5, u16 address6, u16 address7, u16 address8) noexcept;

    /** Creates an IPv4 address from a packed 32-bit integer, where the
        MSB is the first number in the address, and the LSB is the last.
    */
    explicit IPAddress (u32 asNativeEndian32Bit) noexcept;

    /** Parses a string IP address of the form "1.2.3.4" (IPv4) or "1:2:3:4:5:6:7:8" (IPv6). */
    explicit IPAddress (const Txt& address);

    /** Returns whether the address contains the null address (e.g. 0.0.0.0). */
    b8 isNull() const;

    //==============================================================================
    /** Returns a dot- or colon-separated string in the form "1.2.3.4" (IPv4) or "1:2:3:4:5:6:7:8" (IPv6). */
    Txt toString() const;

    /** Compares this IPAddress with another.

        @returns 0 if the two addresses are identical, negative if this address is smaller than
                 the other one, or positive if is greater.
    */
    i32 compare (const IPAddress&) const noexcept;

    b8 operator== (const IPAddress&) const noexcept;
    b8 operator!= (const IPAddress&) const noexcept;
    b8 operator<  (const IPAddress&) const noexcept;
    b8 operator>  (const IPAddress&) const noexcept;
    b8 operator<= (const IPAddress&) const noexcept;
    b8 operator>= (const IPAddress&) const noexcept;

    //==============================================================================
    /** The elements of the IP address. */
    u8 address[16];

    b8 isIPv6 = false;

    //==============================================================================
    /** Returns a formatted version of the provided IPv6 address conforming to RFC 5952 with leading zeros suppressed,
        lower case characters, and f64-colon notation used to represent contiguous 16-bit fields of zeros.

        @param unformattedAddress    the IPv6 address to be formatted
    */
    static Txt getFormattedAddress (const Txt& unformattedAddress);

    /** Возвращает true, если the given IP address is an IPv4-mapped IPv6 address. */
    static b8 isIPv4MappedAddress (const IPAddress& mappedAddress);

    /** Converts an IPv4-mapped IPv6 address to an IPv4 address.
        If the address is not IPv4-mapped, this will return a null address.
    */
    static IPAddress convertIPv4MappedAddressToIPv4 (const IPAddress& mappedAddress);

    /** Converts an IPv4 address to an IPv4-mapped IPv6 address. */
    static IPAddress convertIPv4AddressToIPv4Mapped (const IPAddress& addressToMap);

    /** If the IPAdress is the address of an interface on the machine, returns the associated broadcast address.
        If the address is not an interface, it will return a null address.
    */
    static IPAddress getInterfaceBroadcastAddress (const IPAddress& interfaceAddress);
};

} // namespace drx
