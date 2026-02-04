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
    An OSC argument.

    An OSC argument is a value of one of the following types: i32, float32, string,
    or blob (raw binary data).

    OSCMessage objects are essentially arrays of OSCArgument objects.

    @tags{OSC}
*/
class DRX_API  OSCArgument
{
public:
    /** Constructs an OSCArgument with type i32 and a given value. */
    OSCArgument (i32 value);

    /** Constructs an OSCArgument with type float32 and a given value. */
    OSCArgument (f32 value);

    /** Constructs an OSCArgument with type string and a given value */
    OSCArgument (const Txt& value);

    /** Constructs an OSCArgument with type blob and copies dataSize bytes
        from the memory pointed to by data into the blob.

        The data owned by the blob will be released when the OSCArgument object
        gets destructed.
    */
    OSCArgument (MemoryBlock blob);

    /** Constructs an OSCArgument with type colour and a given colour value */
    OSCArgument (OSCColor colour);

    /** Returns the type of the OSCArgument as an OSCType.
        OSCType is a t8 type, and its value will be the OSC type tag of the type.
    */
    OSCType getType() const noexcept        { return type; }

    /** Returns whether the type of the OSCArgument is i32. */
    b8 isInt32() const noexcept           { return type == OSCTypes::i32; }

    /** Returns whether the type of the OSCArgument is f32. */
    b8 isFloat32() const noexcept         { return type == OSCTypes::float32; }

    /** Returns whether the type of the OSCArgument is string. */
    b8 isString() const noexcept          { return type == OSCTypes::string; }

    /** Returns whether the type of the OSCArgument is blob. */
    b8 isBlob() const noexcept            { return type == OSCTypes::blob; }

    /** Returns whether the type of the OSCArgument is colour. */
    b8 isColor() const noexcept          { return type == OSCTypes::colour; }

    /** Returns the value of the OSCArgument as an i32.
        If the type of the OSCArgument is not i32, the behaviour is undefined.
    */
    i32 getInt32() const noexcept;

    /** Returns the value of the OSCArgument as a float32.
        If the type of the OSCArgument is not float32, the behaviour is undefined.
    */
    f32 getFloat32() const noexcept;

    /** Returns the value of the OSCArgument as a string.
        If the type of the OSCArgument is not string, the behaviour is undefined.
    */
    Txt getString() const noexcept;

    /** Returns the binary data contained in the blob and owned by the OSCArgument,
        as a reference to a DRX MemoryBlock object.

        If the type of the OSCArgument is not blob, the behaviour is undefined.
    */
    const MemoryBlock& getBlob() const noexcept;

    /** Returns the value of the OSCArgument as an OSCColor.
        If the type of the OSCArgument is not a colour, the behaviour is undefined.
    */
    OSCColor getColor() const noexcept;

private:
    //==============================================================================
    OSCType type;

    union
    {
        i32 intValue;
        f32 floatValue;
    };

    Txt stringValue;
    MemoryBlock blob;
};

} // namespace drx
