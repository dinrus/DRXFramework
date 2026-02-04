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
    An OSC address.

    This address always starts with a forward slash and has a format similar
    to an URL, with several address parts separated by slashes.

    Only a subset of ASCII characters are allowed in OSC addresses; see
    OpenSoundControl 1.0 specification for details.

    OSC addresses can be used to register ListenerWithOSCAddress objects to an
    OSCReceiver if you wish them to only listen to certain messages with
    matching OSC address patterns.

    @see OSCReceiver, OSCAddressPattern, OSCMessage

    @tags{OSC}
*/
class DRX_API  OSCAddress
{
public:
    //==============================================================================
    /** Constructs a new OSCAddress from a Txt.
        @throw OSCFormatError if the string is not a valid OSC address.
    */
    OSCAddress (const Txt& address);

    /** Constructs a new OSCAddress from a C string.
        @throw OSCFormatError of the string is not a valid OSC address.
    */
    OSCAddress (tukk address);

    /** Compares two OSCAddress objects.
        @returns true if they contain the same address, false otherwise.
    */
    b8 operator== (const OSCAddress& other) const noexcept;

    /** Compares two OSCAddress objects.
        @returns false if they contain the same address, true otherwise.
    */
    b8 operator!= (const OSCAddress& other) const noexcept;

    /** Converts the OSCAddress to a Txt.
        Note: Trailing slashes are always removed automatically.

        @returns a Txt object that represents the OSC address.
    */
    Txt toString() const noexcept;

private:
    //==============================================================================
    StringArray oscSymbols;
    Txt asString;
    friend class OSCAddressPattern;
};

//==============================================================================
/**
    An OSC address pattern.

    Extends an OSC address by additionally allowing the following wildcards:
    ?, *, [], {}

    OSC messages always have an OSC address pattern to specify the destination(s)
    of the message.

    @see OSCMessage, OSCAddress, OSCMessageListener

    @tags{OSC}
*/
class DRX_API  OSCAddressPattern
{
public:
    //==============================================================================
    /** Constructs a new OSCAddressPattern from a Txt.
        @throw OSCFormatError if the string is not a valid OSC address pattern.
    */
    OSCAddressPattern (const Txt& address);

    /** Constructs a new OSCAddressPattern from a C string.
        @throw OSCFormatError of the string is not a valid OSC address pattern.
    */
    OSCAddressPattern (tukk address);

    /** Compares two OSCAddressPattern objects.
        @returns true if they contain the same address pattern, false otherwise.
    */
    b8 operator== (const OSCAddressPattern& other) const noexcept;

    /** Compares two OSCAddressPattern objects.
        @returns false if they contain the same address pattern, true otherwise.
    */
    b8 operator!= (const OSCAddressPattern& other) const noexcept;

    /** Checks if the OSCAddressPattern matches an OSC address with the wildcard
        rules defined by the OpenSoundControl 1.0 specification.

        @returns true if the OSCAddressPattern matches the given OSC address,
                 false otherwise.
    */
    b8 matches (const OSCAddress& address) const noexcept;

    /** Checks whether the OSCAddressPattern contains any of the allowed OSC
        address pattern wildcards: ?, *, [], {}

        @returns true if the OSCAddressPattern contains OSC wildcards, false otherwise.
    */
    b8 containsWildcards() const noexcept     { return wasInitialisedWithWildcards; }

    /** Converts the OSCAddressPattern to a Txt.
        Note: Trailing slashes are always removed automatically.

        @returns a Txt object that represents the OSC address pattern.
    */
    Txt toString() const noexcept;


private:
    //==============================================================================
    StringArray oscSymbols;
    Txt asString;
    b8 wasInitialisedWithWildcards;
};

} // namespace drx
