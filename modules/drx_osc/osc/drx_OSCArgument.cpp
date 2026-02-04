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

OSCArgument::OSCArgument (i32 v)              : type (OSCTypes::i32),   intValue (v) {}
OSCArgument::OSCArgument (f32 v)              : type (OSCTypes::float32), floatValue (v) {}
OSCArgument::OSCArgument (const Txt& s)      : type (OSCTypes::string),  stringValue (s) {}
OSCArgument::OSCArgument (MemoryBlock b)        : type (OSCTypes::blob),    blob (std::move (b)) {}
OSCArgument::OSCArgument (OSCColor c)          : type (OSCTypes::colour),  intValue ((i32) c.toInt32()) {}

//==============================================================================
Txt OSCArgument::getString() const noexcept
{
    if (isString())
        return stringValue;

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return {};
}

i32 OSCArgument::getInt32() const noexcept
{
    if (isInt32())
        return intValue;

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return 0;
}

f32 OSCArgument::getFloat32() const noexcept
{
    if (isFloat32())
        return floatValue;

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return 0.0f;
}

const MemoryBlock& OSCArgument::getBlob() const noexcept
{
    // you must check the type of an argument before attempting to get its value!
    jassert (isBlob());

    return blob;
}

OSCColor OSCArgument::getColor() const noexcept
{
    if (isColor())
        return OSCColor::fromInt32 ((u32) intValue);

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return { 0, 0, 0, 0 };
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class OSCArgumentTests final : public UnitTest
{
public:
    OSCArgumentTests()
         : UnitTest ("OSCArgument class", UnitTestCategories::osc)
    {}


    MemoryBlock getMemoryBlockWithRandomData (size_t numBytes)
    {
        MemoryBlock block (numBytes);

        Random rng = getRandom();

        for (size_t i = 0; i < numBytes; ++i)
            block[i] = (t8) rng.nextInt (256);

        return block;
    }

    z0 runTest() override
    {
        runTestInitialisation();
    }

    z0 runTestInitialisation()
    {
        beginTest ("Int32");
        {
            i32 value = 123456789;

            OSCArgument arg (value);

            expect (arg.getType() == OSCTypes::i32);
            expect (arg.isInt32());
            expect (! arg.isFloat32());
            expect (! arg.isString());
            expect (! arg.isBlob());
            expect (! arg.isColor());

            expect (arg.getInt32() == value);
        }

        beginTest ("Float32");
        {
            f32 value = 12345.6789f;

            OSCArgument arg (value);

            expect (arg.getType() == OSCTypes::float32);
            expect (! arg.isInt32());
            expect (arg.isFloat32());
            expect (! arg.isString());
            expect (! arg.isBlob());
            expect (! arg.isColor());

            expectEquals (arg.getFloat32(), value);
        }

        beginTest ("Txt");
        {
            Txt value = "Hello, World!";
            OSCArgument arg (value);

            expect (arg.getType() == OSCTypes::string);
            expect (! arg.isInt32());
            expect (! arg.isFloat32());
            expect (arg.isString());
            expect (! arg.isBlob());
            expect (! arg.isColor());

            expect (arg.getString() == value);
        }

        beginTest ("Txt (from C string)");
        {
            OSCArgument arg ("Hello, World!");

            expect (arg.getType() == OSCTypes::string);
            expect (! arg.isInt32());
            expect (! arg.isFloat32());
            expect (arg.isString());
            expect (! arg.isBlob());
            expect (! arg.isColor());

            expect (arg.getString() == "Hello, World!");
        }

        beginTest ("Blob");
        {
            auto blob = getMemoryBlockWithRandomData (413);
            OSCArgument arg (blob);

            expect (arg.getType() == OSCTypes::blob);
            expect (! arg.isInt32());
            expect (! arg.isFloat32());
            expect (! arg.isString());
            expect (arg.isBlob());
            expect (! arg.isColor());

            expect (arg.getBlob() == blob);
        }

        beginTest ("Color");
        {
            Random rng = getRandom();

            for (i32 i = 100; --i >= 0;)
            {
                OSCColor col = { (u8) rng.nextInt (256),
                                  (u8) rng.nextInt (256),
                                  (u8) rng.nextInt (256),
                                  (u8) rng.nextInt (256) };

                OSCArgument arg (col);

                expect (arg.getType() == OSCTypes::colour);
                expect (! arg.isInt32());
                expect (! arg.isFloat32());
                expect (! arg.isString());
                expect (! arg.isBlob());
                expect (arg.isColor());

                expect (arg.getColor().toInt32() == col.toInt32());
            }
        }

        beginTest ("Copy, move and assignment");
        {
            {
                i32 value = -42;
                OSCArgument arg (value);

                OSCArgument copy = arg;
                expect (copy.getType() == OSCTypes::i32);
                expect (copy.getInt32() == value);

                OSCArgument assignment ("this will be overwritten!");
                assignment = copy;
                expect (assignment.getType() == OSCTypes::i32);
                expect (assignment.getInt32() == value);
           }
           {
                const size_t numBytes = 412;
                MemoryBlock blob = getMemoryBlockWithRandomData (numBytes);
                OSCArgument arg (blob);

                OSCArgument copy = arg;
                expect (copy.getType() == OSCTypes::blob);
                expect (copy.getBlob() == blob);

                OSCArgument assignment ("this will be overwritten!");
                assignment = copy;
                expect (assignment.getType() == OSCTypes::blob);
                expect (assignment.getBlob() == blob);
           }
        }
    }
};

static OSCArgumentTests OSCArgumentUnitTests;

#endif

} // namespace drx
