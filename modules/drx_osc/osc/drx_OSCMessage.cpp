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

OSCMessage::OSCMessage (const OSCAddressPattern& ap) noexcept  : addressPattern (ap)
{
}

//==============================================================================
z0 OSCMessage::setAddressPattern (const OSCAddressPattern& ap) noexcept
{
    addressPattern = ap;
}

OSCAddressPattern OSCMessage::getAddressPattern() const noexcept
{
    return addressPattern;
}

//==============================================================================
i32 OSCMessage::size() const noexcept
{
    return arguments.size();
}

b8 OSCMessage::isEmpty() const noexcept
{
    return arguments.isEmpty();
}

OSCArgument& OSCMessage::operator[] (i32k i) noexcept
{
    return arguments.getReference (i);
}

const OSCArgument& OSCMessage::operator[] (i32k i) const noexcept
{
    return arguments.getReference (i);
}

OSCArgument* OSCMessage::begin() noexcept
{
    return arguments.begin();
}

const OSCArgument* OSCMessage::begin() const noexcept
{
    return arguments.begin();
}

OSCArgument* OSCMessage::end() noexcept
{
    return arguments.end();
}

const OSCArgument* OSCMessage::end() const noexcept
{
    return arguments.end();
}

z0 OSCMessage::clear()
{
    arguments.clear();
}

//==============================================================================
z0 OSCMessage::addInt32 (i32 value)             { arguments.add (OSCArgument (value)); }
z0 OSCMessage::addFloat32 (f32 value)           { arguments.add (OSCArgument (value)); }
z0 OSCMessage::addString (const Txt& value)    { arguments.add (OSCArgument (value)); }
z0 OSCMessage::addBlob (MemoryBlock blob)         { arguments.add (OSCArgument (std::move (blob))); }
z0 OSCMessage::addColor (OSCColor colour)       { arguments.add (OSCArgument (colour)); }
z0 OSCMessage::addArgument (OSCArgument arg)      { arguments.add (arg); }


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class OSCMessageTests final : public UnitTest
{
public:
    OSCMessageTests()
        : UnitTest ("OSCMessage class", UnitTestCategories::osc)
    {}

    z0 runTest() override
    {
        beginTest ("Basic usage");
        {
            OSCMessage msg ("/test/param0");
            expectEquals (msg.size(), 0);
            expect (msg.getAddressPattern().toString() == "/test/param0");

            i32k numTestArgs = 5;

            i32k testInt = 42;
            const f32 testFloat = 3.14159f;
            const Txt testString = "Hello, World!";
            const OSCColor testColor = { 10, 20, 150, 200 };

            u8k testBlobData[5] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
            const MemoryBlock testBlob (testBlobData,  sizeof (testBlobData));

            msg.addInt32 (testInt);
            msg.addFloat32 (testFloat);
            msg.addString (testString);
            msg.addBlob (testBlob);
            msg.addColor (testColor);

            expectEquals (msg.size(), numTestArgs);

            expectEquals (msg[0].getType(), OSCTypes::i32);
            expectEquals (msg[1].getType(), OSCTypes::float32);
            expectEquals (msg[2].getType(), OSCTypes::string);
            expectEquals (msg[3].getType(), OSCTypes::blob);
            expectEquals (msg[4].getType(), OSCTypes::colour);

            expect (msg[0].isInt32());
            expect (msg[1].isFloat32());
            expect (msg[2].isString());
            expect (msg[3].isBlob());
            expect (msg[4].isColor());

            expectEquals (msg[0].getInt32(), testInt);
            expectEquals (msg[1].getFloat32(), testFloat);
            expectEquals (msg[2].getString(), testString);
            expect (msg[3].getBlob() == testBlob);
            expect (msg[4].getColor().toInt32() == testColor.toInt32());

            expect (msg.begin() + numTestArgs == msg.end());

            auto arg = msg.begin();
            expect (arg->isInt32());
            expectEquals (arg->getInt32(), testInt);
            ++arg;
            expect (arg->isFloat32());
            expectEquals (arg->getFloat32(), testFloat);
            ++arg;
            expect (arg->isString());
            expectEquals (arg->getString(), testString);
            ++arg;
            expect (arg->isBlob());
            expect (arg->getBlob() == testBlob);
            ++arg;
            expect (arg->isColor());
            expect (arg->getColor().toInt32() == testColor.toInt32());
            ++arg;
            expect (arg == msg.end());
        }


        beginTest ("Initialisation with argument list (C++11 only)");
        {
            i32 testInt = 42;
            f32 testFloat = 5.5;
            Txt testString = "Hello, World!";

            {
                OSCMessage msg ("/test", testInt);
                expect (msg.getAddressPattern().toString() == Txt ("/test"));
                expectEquals (msg.size(), 1);
                expect (msg[0].isInt32());
                expectEquals (msg[0].getInt32(), testInt);
            }
            {
                OSCMessage msg ("/test", testFloat);
                expect (msg.getAddressPattern().toString() == Txt ("/test"));
                expectEquals (msg.size(), 1);
                expect (msg[0].isFloat32());
                expectEquals (msg[0].getFloat32(), testFloat);
            }
            {
                OSCMessage msg ("/test", testString);
                expect (msg.getAddressPattern().toString() == Txt ("/test"));
                expectEquals (msg.size(), 1);
                expect (msg[0].isString());
                expectEquals (msg[0].getString(), testString);
            }
            {
                OSCMessage msg ("/test", testInt, testFloat, testString, testFloat, testInt);
                expect (msg.getAddressPattern().toString() == Txt ("/test"));
                expectEquals (msg.size(), 5);
                expect (msg[0].isInt32());
                expect (msg[1].isFloat32());
                expect (msg[2].isString());
                expect (msg[3].isFloat32());
                expect (msg[4].isInt32());

                expectEquals (msg[0].getInt32(), testInt);
                expectEquals (msg[1].getFloat32(), testFloat);
                expectEquals (msg[2].getString(), testString);
                expectEquals (msg[3].getFloat32(), testFloat);
                expectEquals (msg[4].getInt32(), testInt);
            }
        }
    }
};

static OSCMessageTests OSCMessageUnitTests;

#endif

} // namespace drx
