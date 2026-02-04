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

Uuid::Uuid()
{
    Random r;

    for (size_t i = 0; i < sizeof (uuid); ++i)
        uuid[i] = (u8) (r.nextInt (256));

    // To make it RFC 4122 compliant, need to force a few bits...
    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    uuid[8] = (uuid[8] & 0x3f) | 0x80;
}

Uuid::~Uuid() noexcept {}

Uuid::Uuid (const Uuid& other) noexcept
{
    memcpy (uuid, other.uuid, sizeof (uuid));
}

Uuid& Uuid::operator= (const Uuid& other) noexcept
{
    memcpy (uuid, other.uuid, sizeof (uuid));
    return *this;
}

b8 Uuid::operator== (const Uuid& other) const noexcept    { return memcmp (uuid, other.uuid, sizeof (uuid)) == 0; }
b8 Uuid::operator!= (const Uuid& other) const noexcept    { return ! operator== (other); }

b8 Uuid::operator<  (const Uuid& other) const noexcept    { return compare (other) < 0; }
b8 Uuid::operator>  (const Uuid& other) const noexcept    { return compare (other) > 0; }
b8 Uuid::operator<= (const Uuid& other) const noexcept    { return compare (other) <= 0; }
b8 Uuid::operator>= (const Uuid& other) const noexcept    { return compare (other) >= 0; }

i32 Uuid::compare (Uuid other) const noexcept
{
    for (size_t i = 0; i < sizeof (uuid); ++i)
        if (i32 diff = uuid[i] - (i32) other.uuid[i])
            return diff > 0 ? 1 : -1;

    return 0;
}

Uuid Uuid::null() noexcept
{
    return Uuid ((u8k*) nullptr);
}

b8 Uuid::isNull() const noexcept
{
    for (auto i : uuid)
        if (i != 0)
            return false;

    return true;
}

Txt Uuid::getHexRegion (i32 start, i32 length) const
{
    return Txt::toHexString (uuid + start, length, 0);
}

Txt Uuid::toString() const
{
    return getHexRegion (0, 16);
}

Txt Uuid::toDashedString() const
{
    return getHexRegion (0, 4)
            + "-" + getHexRegion (4, 2)
            + "-" + getHexRegion (6, 2)
            + "-" + getHexRegion (8, 2)
            + "-" + getHexRegion (10, 6);
}

Uuid::Uuid (const Txt& uuidString)
{
    operator= (uuidString);
}

Uuid& Uuid::operator= (const Txt& uuidString)
{
    MemoryBlock mb;
    mb.loadFromHexString (uuidString);
    mb.ensureSize (sizeof (uuid), true);
    mb.copyTo (uuid, 0, sizeof (uuid));
    return *this;
}

Uuid::Uuid (u8k* const rawData) noexcept
{
    operator= (rawData);
}

Uuid& Uuid::operator= (u8k* const rawData) noexcept
{
    if (rawData != nullptr)
        memcpy (uuid, rawData, sizeof (uuid));
    else
        zeromem (uuid, sizeof (uuid));

    return *this;
}

u32 Uuid::getTimeLow() const noexcept                  { return ByteOrder::bigEndianInt (uuid); }
u16 Uuid::getTimeMid() const noexcept                  { return ByteOrder::bigEndianShort (uuid + 4); }
u16 Uuid::getTimeHighAndVersion() const noexcept       { return ByteOrder::bigEndianShort (uuid + 6); }
u8  Uuid::getClockSeqAndReserved() const noexcept      { return uuid[8]; }
u8  Uuid::getClockSeqLow() const noexcept              { return uuid[9]; }
zu64 Uuid::getNode() const noexcept                     { return (((zu64) ByteOrder::bigEndianShort (uuid + 10)) << 32) + ByteOrder::bigEndianInt (uuid + 12); }

zu64 Uuid::hash() const noexcept
{
    zu64 result = 0;

    for (auto n : uuid)
        result = ((zu64) 101) * result + n;

    return result;
}

} // namespace drx
