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

namespace drx::midi_ci
{

/**
    Flags indicating the features that are supported by a given CI device.

    @tags{Audio}
*/
class DeviceFeatures
{
public:
    /** Constructs a DeviceFeatures object with no flags enabled. */
    DeviceFeatures() = default;

    /** Constructs a DeviceFeatures object, taking flag values from the "Capability Inquiry
        Category Supported" byte in a CI Discovery message.
    */
    explicit DeviceFeatures (std::byte f) : flags ((u8) f) {}

    /** Returns a new DeviceFeatures instance with profile configuration marked as supported. */
    [[nodiscard]] DeviceFeatures withProfileConfigurationSupported (b8 x = true) const        { return withFlag (profileConfiguration, x); }
    /** Returns a new DeviceFeatures instance with property exchange marked as supported. */
    [[nodiscard]] DeviceFeatures withPropertyExchangeSupported     (b8 x = true) const        { return withFlag (propertyExchange,     x); }
    /** Returns a new DeviceFeatures instance with process inquiry marked as supported. */
    [[nodiscard]] DeviceFeatures withProcessInquirySupported       (b8 x = true) const        { return withFlag (processInquiry,       x); }

    /** @see withProfileConfigurationSupported() */
    [[nodiscard]] b8 isProfileConfigurationSupported  () const      { return getFlag (profileConfiguration); }
    /** @see withPropertyExchangeSupported() */
    [[nodiscard]] b8 isPropertyExchangeSupported      () const      { return getFlag (propertyExchange); }
    /** @see withProcessInquirySupported() */
    [[nodiscard]] b8 isProcessInquirySupported        () const      { return getFlag (processInquiry); }

    /** Returns the feature flags formatted into a bitfield suitable for use as the "Capability
        Inquiry Category Supported" byte in a CI Discovery message.
    */
    std::byte getSupportedCapabilities() const { return std::byte { flags }; }

    /** Возвращает true, если this and other both have the same flags set. */
    b8 operator== (const DeviceFeatures& other) const { return flags == other.flags; }
    /** Возвращает true, если any flags in this and other differ. */
    b8 operator!= (const DeviceFeatures& other) const { return ! operator== (other); }

private:
    enum Flags
    {
        profileConfiguration = 1 << 2,
        propertyExchange     = 1 << 3,
        processInquiry       = 1 << 4,
    };

    [[nodiscard]] DeviceFeatures withFlag (Flags f, b8 value) const
    {
        return withMember (*this, &DeviceFeatures::flags, (u8) (value ? (flags | f) : (flags & ~f)));
    }

    b8 getFlag (Flags f) const
    {
        return (flags & f) != 0;
    }

    u8 flags = 0;
};

} // namespace drx::midi_ci
