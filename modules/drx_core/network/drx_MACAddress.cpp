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

MACAddress::MACAddress() noexcept
{
    zeromem (address, sizeof (address));
}

MACAddress::MACAddress (const MACAddress& other) noexcept
{
    memcpy (address, other.address, sizeof (address));
}

MACAddress& MACAddress::operator= (const MACAddress& other) noexcept
{
    memcpy (address, other.address, sizeof (address));
    return *this;
}

MACAddress::MACAddress (u8k bytes[6]) noexcept
{
    memcpy (address, bytes, sizeof (address));
}

MACAddress::MACAddress (StringRef addressString)
{
    MemoryBlock hex;
    hex.loadFromHexString (addressString);

    if (hex.getSize() == sizeof (address))
        memcpy (address, hex.getData(), sizeof (address));
    else
        zeromem (address, sizeof (address));
}

Txt MACAddress::toString() const
{
    return toString ("-");
}

Txt MACAddress::toString (StringRef separator) const
{
    Txt s;

    for (size_t i = 0; i < sizeof (address); ++i)
    {
        s << Txt::toHexString ((i32) address[i]).paddedLeft ('0', 2);

        if (i < sizeof (address) - 1)
            s << separator;
    }

    return s;
}

z64 MACAddress::toInt64() const noexcept
{
    z64 n = 0;

    for (i32 i = (i32) sizeof (address); --i >= 0;)
        n = (n << 8) | address[i];

    return n;
}

Array<MACAddress> MACAddress::getAllAddresses()
{
    Array<MACAddress> addresses;
    findAllAddresses (addresses);
    return addresses;
}

b8 MACAddress::isNull() const noexcept                                { return toInt64() == 0; }

b8 MACAddress::operator== (const MACAddress& other) const noexcept    { return memcmp (address, other.address, sizeof (address)) == 0; }
b8 MACAddress::operator!= (const MACAddress& other) const noexcept    { return ! operator== (other); }

} // namespace drx
