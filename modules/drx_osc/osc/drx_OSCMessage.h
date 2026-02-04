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
    An OSC Message.

    An OSCMessage consists of an OSCAddressPattern and zero or more OSCArguments.

    OSC messages are the elementary objects that are used to exchange any data
    via OSC. An OSCSender can send OSCMessage objects to an OSCReceiver.

    @tags{OSC}
*/
class DRX_API  OSCMessage
{
public:

    //==============================================================================
    /** Constructs an OSCMessage object with the given address pattern and no
        arguments.

        @param ap    the address pattern of the message. This must be a valid OSC
                     address (starting with a forward slash) and may contain
                     OSC wildcard expressions. You can pass in a string literal
                     or a drx Txt (they will be converted to an OSCAddressPattern
                     automatically).
    */
    OSCMessage (const OSCAddressPattern& ap) noexcept;


    /** Constructs an OSCMessage object with the given address pattern and list
        of arguments.

        @param ap    the address pattern of the message. This must be a valid OSC
                     address (starting with a forward slash) and may contain
                     OSC wildcard expressions. You can pass in a string literal
                     or a drx Txt (they will be converted to an OSCAddressPattern
                     automatically).

        @param arg1  the first argument of the message.
        @param args  an optional list of further arguments to add to the message.
    */
    template <typename Arg1, typename... Args>
    OSCMessage (const OSCAddressPattern& ap, Arg1&& arg1, Args&&... args);

    /** Sets the address pattern of the OSCMessage.

        @param ap    the address pattern of the message. This must be a valid OSC
                     address (starting with a forward slash) and may contain
                     OSC wildcard expressions. You can pass in a string literal
                     or a drx Txt (they will be converted to an OSCAddressPattern
                     automatically).
    */
    z0 setAddressPattern (const OSCAddressPattern& ap) noexcept;

    /** Returns the address pattern of the OSCMessage. */
    OSCAddressPattern getAddressPattern() const noexcept;

    /** Returns the number of OSCArgument objects that belong to this OSCMessage. */
    i32 size() const noexcept;

    /** Возвращает true, если the OSCMessage contains no OSCArgument objects; false otherwise. */
    b8 isEmpty() const noexcept;

    /** Returns a reference to the OSCArgument at index i in the OSCMessage object.
        This method does not check the range and results in undefined behaviour
        in case i < 0 or i >= size().
    */
    OSCArgument& operator[] (i32 i) noexcept;
    const OSCArgument& operator[] (i32 i) const noexcept;

    /** Returns a pointer to the first OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    OSCArgument* begin() noexcept;

    /** Returns a pointer to the first OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    const OSCArgument* begin() const noexcept;

    /** Returns a pointer to the last OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    OSCArgument* end() noexcept;

    /** Returns a pointer to the last OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    const OSCArgument* end() const noexcept;

    /** Removes all arguments from the OSCMessage. */
    z0 clear();

    //==============================================================================
    /** Creates a new OSCArgument of type i32 with the given value,
        and adds it to the OSCMessage object.
    */
    z0 addInt32 (i32 value);

    /** Creates a new OSCArgument of type float32 with the given value,
        and adds it to the OSCMessage object.
    */
    z0 addFloat32 (f32 value);

    /** Creates a new OSCArgument of type string with the given value,
        and adds it to the OSCMessage object.
    */
    z0 addString (const Txt& value);

    /** Creates a new OSCArgument of type blob with binary data content copied from
        the given MemoryBlock.

        Note: If the argument passed is an lvalue, this may copy the binary data.
    */
    z0 addBlob (MemoryBlock blob);

    /** Creates a new OSCArgument of type colour with the given value,
        and adds it to the OSCMessage object.
    */
    z0 addColor (OSCColor colour);

    /** Adds the OSCArgument argument to the OSCMessage object.

        Note: This method will result in a copy of the OSCArgument object if it is passed
        as an lvalue. If the OSCArgument is of type blob, this will also copy the underlying
        binary data. In general, you should use addInt32, addFloat32, etc. instead.
    */
    z0 addArgument (OSCArgument argument);

private:

    //==============================================================================
    template <typename Arg1, typename... Args>
    z0 addArguments (Arg1&& arg1, Args&&... args)
    {
        addArgument (arg1);
        addArguments (std::forward<Args> (args)...);
    }

    z0 addArguments() {}

    //==============================================================================
    OSCAddressPattern addressPattern;
    Array<OSCArgument> arguments;
};


//==============================================================================
template <typename Arg1, typename... Args>
OSCMessage::OSCMessage (const OSCAddressPattern& ap, Arg1&& arg1, Args&&... args)
    : addressPattern (ap)
{
    addArguments (std::forward<Arg1> (arg1), std::forward<Args> (args)...);
}

} // namespace drx
