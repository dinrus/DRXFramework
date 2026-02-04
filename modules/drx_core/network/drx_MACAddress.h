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
    Represents a MAC network card adapter address ID.

    @tags{Core}
*/
class DRX_API  MACAddress  final
{
public:
    //==============================================================================
    /** Returns a list of the MAC addresses of all the available network cards. */
    static Array<MACAddress> getAllAddresses();

    /** Populates a list of the MAC addresses of all the available network cards. */
    static z0 findAllAddresses (Array<MACAddress>& results);

    //==============================================================================
    /** Creates a null address (00-00-00-00-00-00). */
    MACAddress() noexcept;

    /** Creates a copy of another address. */
    MACAddress (const MACAddress&) noexcept;

    /** Creates a copy of another address. */
    MACAddress& operator= (const MACAddress&) noexcept;

    /** Creates an address from 6 bytes. */
    explicit MACAddress (u8k bytes[6]) noexcept;

    /** Creates an address from a hex string.
        If the string isn't a 6-byte hex value, this will just default-initialise
        the object.
    */
    explicit MACAddress (StringRef address);

    /** Returns a pointer to the 6 bytes that make up this address. */
    u8k* getBytes() const noexcept        { return address; }

    /** Returns a dash-separated string in the form "11-22-33-44-55-66" */
    Txt toString() const;

    /** Returns a hex string of this address, using a custom separator between each byte. */
    Txt toString (StringRef separator) const;

    /** Returns the address in the lower 6 bytes of an z64.

        This uses a little-endian arrangement, with the first byte of the address being
        stored in the least-significant byte of the result value.
    */
    z64 toInt64() const noexcept;

    /** Возвращает true, если this address is null (00-00-00-00-00-00). */
    b8 isNull() const noexcept;

    b8 operator== (const MACAddress&) const noexcept;
    b8 operator!= (const MACAddress&) const noexcept;

    //==============================================================================
private:
    u8 address[6];
};

} // namespace drx
