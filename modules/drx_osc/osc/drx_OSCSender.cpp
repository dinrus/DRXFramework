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

namespace
{
    //==============================================================================
    /** Writes OSC data to an internal memory buffer, which grows as required.

        The data that was written into the stream can then be accessed later as
        a contiguous block of memory.

        This class implements the Open Sound Control 1.0 Specification for
        the format in which the OSC data will be written into the buffer.
    */
    struct OSCOutputStream
    {
        OSCOutputStream() noexcept {}

        /** Returns a pointer to the data that has been written to the stream. */
        ukk getData() const noexcept    { return output.getData(); }

        /** Returns the number of bytes of data that have been written to the stream. */
        size_t getDataSize() const noexcept     { return output.getDataSize(); }

        //==============================================================================
        b8 writeInt32 (i32 value)
        {
            return output.writeIntBigEndian (value);
        }

        b8 writeUint64 (zu64 value)
        {
            return output.writeInt64BigEndian (z64 (value));
        }

        b8 writeFloat32 (f32 value)
        {
            return output.writeFloatBigEndian (value);
        }

        b8 writeString (const Txt& value)
        {
            if (! output.writeString (value))
                return false;

            const size_t numPaddingZeros = ~value.getNumBytesAsUTF8() & 3;

            return output.writeRepeatedByte ('\0', numPaddingZeros);
        }

        b8 writeBlob (const MemoryBlock& blob)
        {
            if (! (output.writeIntBigEndian ((i32) blob.getSize())
                    && output.write (blob.getData(), blob.getSize())))
                return false;

            const size_t numPaddingZeros = ~(blob.getSize() - 1) & 3;

            return output.writeRepeatedByte (0, numPaddingZeros);
        }

        b8 writeColor (OSCColor colour)
        {
            return output.writeIntBigEndian ((i32) colour.toInt32());
        }

        b8 writeTimeTag (OSCTimeTag timeTag)
        {
            return output.writeInt64BigEndian (z64 (timeTag.getRawTimeTag()));
        }

        b8 writeAddress (const OSCAddress& address)
        {
            return writeString (address.toString());
        }

        b8 writeAddressPattern (const OSCAddressPattern& ap)
        {
            return writeString (ap.toString());
        }

        b8 writeTypeTagString (const OSCTypeList& typeList)
        {
            output.writeByte (',');

            if (typeList.size() > 0)
                output.write (typeList.begin(), (size_t) typeList.size());

            output.writeByte ('\0');

            size_t bytesWritten = (size_t) typeList.size() + 1;
            size_t numPaddingZeros = ~bytesWritten & 0x03;

            return output.writeRepeatedByte ('\0', numPaddingZeros);
        }

        b8 writeArgument (const OSCArgument& arg)
        {
            switch (arg.getType())
            {
                case OSCTypes::i32:       return writeInt32 (arg.getInt32());
                case OSCTypes::float32:     return writeFloat32 (arg.getFloat32());
                case OSCTypes::string:      return writeString (arg.getString());
                case OSCTypes::blob:        return writeBlob (arg.getBlob());
                case OSCTypes::colour:      return writeColor (arg.getColor());

                default:
                    // In this very unlikely case you supplied an invalid OSCType!
                    jassertfalse;
                    return false;
            }
        }

        //==============================================================================
        b8 writeMessage (const OSCMessage& msg)
        {
            if (! writeAddressPattern (msg.getAddressPattern()))
                return false;

            OSCTypeList typeList;

            for (auto& arg : msg)
                typeList.add (arg.getType());

            if (! writeTypeTagString (typeList))
                return false;

            for (auto& arg : msg)
                if (! writeArgument (arg))
                    return false;

            return true;
        }

        b8 writeBundle (const OSCBundle& bundle)
        {
            if (! writeString ("#bundle"))
                return false;

            if (! writeTimeTag (bundle.getTimeTag()))
                return false;

            for (auto& element : bundle)
                if (! writeBundleElement (element))
                    return false;

            return true;
        }

        //==============================================================================
        b8 writeBundleElement (const OSCBundle::Element& element)
        {
            const z64 startPos = output.getPosition();

            if (! writeInt32 (0))   // writing dummy value for element size
                return false;

            if (element.isBundle())
            {
                if (! writeBundle (element.getBundle()))
                    return false;
            }
            else
            {
                if (! writeMessage (element.getMessage()))
                    return false;
            }

            const z64 endPos = output.getPosition();
            const z64 elementSize = endPos - (startPos + 4);

            return output.setPosition (startPos)
                     && writeInt32 ((i32) elementSize)
                     && output.setPosition (endPos);
        }

    private:
        MemoryOutputStream output;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCOutputStream)
    };

} // namespace


//==============================================================================
struct OSCSender::Pimpl
{
    Pimpl() noexcept  {}
    ~Pimpl() noexcept { disconnect(); }

    //==============================================================================
    b8 connect (const Txt& newTargetHost, i32 newTargetPort)
    {
        if (! disconnect())
            return false;

        socket.setOwned (new DatagramSocket (true));
        targetHostName = newTargetHost;
        targetPortNumber = newTargetPort;

        if (socket->bindToPort (0)) // 0 = use any local port assigned by the OS.
            return true;

        socket.reset();
        return false;
    }

    b8 connectToSocket (DatagramSocket& newSocket, const Txt& newTargetHost, i32 newTargetPort)
    {
        if (! disconnect())
            return false;

        socket.setNonOwned (&newSocket);
        targetHostName = newTargetHost;
        targetPortNumber = newTargetPort;
        return true;
    }

    b8 disconnect()
    {
        socket.reset();
        return true;
    }

    //==============================================================================
    b8 send (const OSCMessage& message, const Txt& hostName, i32 portNumber)
    {
        OSCOutputStream outStream;

        return outStream.writeMessage (message)
            && sendOutputStream (outStream, hostName, portNumber);
    }

    b8 send (const OSCBundle& bundle, const Txt& hostName, i32 portNumber)
    {
        OSCOutputStream outStream;

        return outStream.writeBundle (bundle)
            && sendOutputStream (outStream, hostName, portNumber);
    }

    b8 send (const OSCMessage& message)   { return send (message, targetHostName, targetPortNumber); }
    b8 send (const OSCBundle& bundle)     { return send (bundle,  targetHostName, targetPortNumber); }

private:
    //==============================================================================
    b8 sendOutputStream (OSCOutputStream& outStream, const Txt& hostName, i32 portNumber)
    {
        if (socket != nullptr)
        {
            i32k streamSize = (i32) outStream.getDataSize();

            i32k bytesWritten = socket->write (hostName, portNumber,
                                                    outStream.getData(), streamSize);
            return bytesWritten == streamSize;
        }

        // if you hit this, you tried to send some OSC data without being
        // connected to a port! You should call OSCSender::connect() first.
        jassertfalse;

        return false;
    }

    //==============================================================================
    OptionalScopedPointer<DatagramSocket> socket;
    Txt targetHostName;
    i32 targetPortNumber = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
OSCSender::OSCSender()   : pimpl (new Pimpl())
{
}

OSCSender::~OSCSender()
{
    pimpl->disconnect();
    pimpl.reset();
}

//==============================================================================
b8 OSCSender::connect (const Txt& targetHostName, i32 targetPortNumber)
{
    return pimpl->connect (targetHostName, targetPortNumber);
}

b8 OSCSender::connectToSocket (DatagramSocket& socket, const Txt& targetHostName, i32 targetPortNumber)
{
    return pimpl->connectToSocket (socket, targetHostName, targetPortNumber);
}

b8 OSCSender::disconnect()
{
    return pimpl->disconnect();
}

//==============================================================================
b8 OSCSender::send (const OSCMessage& message)    { return pimpl->send (message); }
b8 OSCSender::send (const OSCBundle& bundle)      { return pimpl->send (bundle); }

b8 OSCSender::sendToIPAddress (const Txt& host, i32 port, const OSCMessage& message) { return pimpl->send (message, host, port); }
b8 OSCSender::sendToIPAddress (const Txt& host, i32 port, const OSCBundle& bundle)   { return pimpl->send (bundle,  host, port); }


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class OSCBinaryWriterTests final : public UnitTest
{
public:
    OSCBinaryWriterTests()
        : UnitTest ("OSCBinaryWriter class", UnitTestCategories::osc)
    {}

    z0 runTest() override
    {
        beginTest ("writing OSC addresses");
        {
            OSCOutputStream outStream;
            const t8 check[16] = { '/', 't', 'e', 's', 't', '/', 'f', 'a', 'd', 'e', 'r', '7', '\0', '\0', '\0', '\0' };

            OSCAddress address ("/test/fader7");
            expect (outStream.writeAddress (address));

            expect (outStream.getDataSize() == sizeof (check));
            expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
        }

        beginTest ("writing OSC address patterns");
        {
            OSCOutputStream outStream;
            const t8 check[20] = { '/', '*', '/', '*', 'p', 'u', 't', '/', 'f', 'a', 'd', 'e', 'r', '[', '0', '-', '9', ']', '\0', '\0' };

            OSCAddressPattern ap ("/*/*put/fader[0-9]");
            expect (outStream.writeAddressPattern (ap));

            expect (outStream.getDataSize() == sizeof (check));
            expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
        }

        beginTest ("writing OSC time tags");
        {
            OSCOutputStream outStream;
            const t8 check[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
            OSCTimeTag tag;

            expect (outStream.writeTimeTag (tag));
            expect (outStream.getDataSize() == 8);
            expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
        }

        beginTest ("writing OSC type tag strings");
        {
            {
                OSCOutputStream outStream;

                OSCTypeList list;

                const t8 check[4] = { ',', '\0', '\0', '\0' };
                expect (outStream.writeTypeTagString (list));
                expect (outStream.getDataSize() == 4);
                expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
            }

            {
                OSCOutputStream outStream;

                OSCTypeList list;
                list.add (OSCTypes::i32);
                list.add (OSCTypes::float32);

                const t8 check[4] = { ',', 'i', 'f', '\0' };
                expect (outStream.writeTypeTagString (list));
                expect (outStream.getDataSize() == sizeof (check));
                expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
            }

            {
                OSCOutputStream outStream;

                OSCTypeList list;
                list.add (OSCTypes::blob);
                list.add (OSCTypes::blob);
                list.add (OSCTypes::string);

                const t8 check[8] = { ',', 'b', 'b', 's', '\0', '\0', '\0', '\0' };
                expect (outStream.writeTypeTagString (list));
                expect (outStream.getDataSize() == sizeof (check));
                expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
            }
        }

        beginTest ("writing OSC arguments");
        {
            // test data:
            i32 testInt = -2015;
            u8k testIntRepresentation[] =  { 0xFF, 0xFF, 0xF8, 0x21 }; // big endian two's complement

            f32 testFloat = 345.6125f;
            u8k testFloatRepresentation[] = { 0x43, 0xAC, 0xCE, 0x66 }; // big endian IEEE 754

            Txt testString = "Hello, World!";
            const t8 testStringRepresentation[] = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0' }; // padded to size % 4 == 0

            u8k testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
            const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));
            u8k testBlobRepresentation[] = { 0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00 }; // padded to size % 4 == 0

            // write:

            {
                // i32:
                OSCArgument arg (testInt);
                OSCOutputStream outStream;

                expect (outStream.writeArgument (arg));
                expect (outStream.getDataSize() == 4);
                expect (std::memcmp (outStream.getData(), testIntRepresentation, sizeof (testIntRepresentation)) == 0);
            }
            {
                // float32:
                OSCArgument arg (testFloat);
                OSCOutputStream outStream;

                expect (outStream.writeArgument (arg));
                expect (outStream.getDataSize() == 4);
                expect (std::memcmp (outStream.getData(), testFloatRepresentation, sizeof (testFloatRepresentation)) == 0);

            }
            {
                // string:
                expect (testString.length() % 4 != 0); // check whether we actually cover padding
                static_assert (sizeof (testStringRepresentation) % 4 == 0, "Size must be a multiple of 4");

                OSCArgument arg (testString);
                OSCOutputStream outStream;

                expect (outStream.writeArgument (arg));
                expect (outStream.getDataSize() == sizeof (testStringRepresentation));
                expect (std::memcmp (outStream.getData(), testStringRepresentation, sizeof (testStringRepresentation)) == 0);

            }
            {
                // blob:
                expect (testBlob.getSize() % 4 != 0);  // check whether we actually cover padding
                static_assert (sizeof (testBlobRepresentation) % 4 == 0, "Size must be a multiple of 4");

                OSCArgument arg (testBlob);
                OSCOutputStream outStream;

                expect (outStream.writeArgument (arg));
                expect (outStream.getDataSize() == sizeof (testBlobRepresentation));
                expect (std::memcmp (outStream.getData(), testBlobRepresentation, sizeof (testBlobRepresentation)) == 0);

            }
        }

        beginTest ("Writing strings with correct padding");
        {
            // the only OSC-specific thing to check is the correct number of padding zeros

            {
                OSCArgument with15Chars ("123456789012345");
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with15Chars));
                expect (outStream.getDataSize() == 16);
            }
            {
                OSCArgument with16Chars ("1234567890123456");
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with16Chars));
                expect (outStream.getDataSize() == 20);
            }
            {
                OSCArgument with17Chars ("12345678901234567");
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with17Chars));
                expect (outStream.getDataSize() == 20);
            }
            {

                OSCArgument with18Chars ("123456789012345678");
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with18Chars));
                expect (outStream.getDataSize() == 20);
            }
            {

                OSCArgument with19Chars ("1234567890123456789");
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with19Chars));
                expect (outStream.getDataSize() == 20);
            }
            {

                OSCArgument with20Chars ("12345678901234567890");
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with20Chars));
                expect (outStream.getDataSize() == 24);
            }
        }
        beginTest ("Writing blobs with correct padding");
        {
            const t8 buffer[20] = {};
            {
                OSCArgument with15Bytes (MemoryBlock (buffer, 15));
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with15Bytes));
                expect (outStream.getDataSize() == 20);
            }
            {
                OSCArgument with16Bytes (MemoryBlock (buffer, 16));
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with16Bytes));
                expect (outStream.getDataSize() == 20);
            }
            {
                OSCArgument with17Bytes (MemoryBlock (buffer, 17));
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with17Bytes));
                expect (outStream.getDataSize() == 24);
            }
            {
                OSCArgument with18Bytes (MemoryBlock (buffer, 18));
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with18Bytes));
                expect (outStream.getDataSize() == 24);
            }
            {
                OSCArgument with19Bytes (MemoryBlock (buffer, 19));
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with19Bytes));
                expect (outStream.getDataSize() == 24);
            }
            {
                OSCArgument with20Bytes (MemoryBlock (buffer, 20));
                OSCOutputStream outStream;
                expect (outStream.writeArgument (with20Bytes));
                expect (outStream.getDataSize() == 24);
            }
        }

        beginTest ("Writing OSC messages.");
        {
            {
                i32 testInt = -2015;
                f32 testFloat = 345.6125f;
                Txt testString = "Hello, World!";

                u8k testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
                const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));

                u8 check[52] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                                    0xFF, 0xFF, 0xF8, 0x21,
                                    0x43, 0xAC, 0xCE, 0x66,
                                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCOutputStream outStream;

                OSCMessage msg ("/test");

                msg.addInt32 (testInt);
                msg.addFloat32 (testFloat);
                msg.addString (testString);
                msg.addBlob (testBlob);

                expect (outStream.writeMessage (msg));
                expect (outStream.getDataSize() == sizeof (check));
                expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
            }
        }

        beginTest ("Writing OSC bundle.");
        {
            {
                i32 testInt = -2015;
                f32 testFloat = 345.6125f;
                Txt testString = "Hello, World!";
                u8k testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
                const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));

                u8 check[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,

                    0x00, 0x00, 0x00, 0x34,

                    '/', 't', 'e', 's', 't', '/', '1', '\0',
                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x0C,

                    '/', 't', 'e', 's', 't', '/', '2', '\0',
                    ',', '\0', '\0', '\0',

                    0x00, 0x00, 0x00, 0x10,

                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
                };

                OSCOutputStream outStream;

                OSCBundle bundle;

                OSCMessage msg1 ("/test/1");
                msg1.addInt32 (testInt);
                msg1.addFloat32 (testFloat);
                msg1.addString (testString);
                msg1.addBlob (testBlob);
                bundle.addElement (msg1);

                OSCMessage msg2 ("/test/2");
                bundle.addElement (msg2);

                OSCBundle subBundle;
                bundle.addElement (subBundle);

                expect (outStream.writeBundle (bundle));
                expect (outStream.getDataSize() == sizeof (check));
                expect (std::memcmp (outStream.getData(), check, sizeof (check)) == 0);
            }
        }
    }
};

static OSCBinaryWriterTests OSCBinaryWriterUnitTests;

//==============================================================================
class OSCRoundTripTests final : public UnitTest
{
public:
    OSCRoundTripTests()
        : UnitTest ("OSCRoundTripTests class", UnitTestCategories::osc)
    {}

    z0 runTest() override
    {
        beginTest ("Empty OSC message");
        {
            OSCMessage outMessage ("/test/empty");

            OSCOutputStream output;
            output.writeMessage (outMessage);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCMessage inMessage = input.readMessage();

            expectEquals (inMessage.size(), 0);
        }

        beginTest ("OSC message with single argument");
        {
            OSCMessage outMessage ("/test/one_arg", 42);

            OSCOutputStream output;
            output.writeMessage (outMessage);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCMessage inMessage = input.readMessage();

            expectEquals (inMessage.size(), 1);
            expectEquals (inMessage[0].getInt32(), 42);
        }

        beginTest ("OSC message with multiple arguments");
        {
            OSCMessage outMessage ("/test/four_args", 42, 0.5f, Txt ("foo"), Txt ("bar"));

            OSCOutputStream output;
            output.writeMessage (outMessage);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCMessage inMessage = input.readMessage();

            expectEquals (inMessage.size(), 4);
            expectEquals (inMessage[0].getInt32(), 42);
            expectEquals (inMessage[1].getFloat32(), 0.5f);
            expectEquals (inMessage[2].getString(), Txt ("foo"));
            expectEquals (inMessage[3].getString(), Txt ("bar"));
        }

        beginTest ("Empty OSC bundle");
        {
            OSCBundle outBundle;

            OSCOutputStream output;
            output.writeBundle (outBundle);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCBundle inBundle = input.readBundle();

            expectEquals (inBundle.size(), 0);
        }

        beginTest ("OSC bundle with single message");
        {
            OSCMessage outMessage ("/test/one_arg", 42);
            OSCBundle outBundle;
            outBundle.addElement (outMessage);

            OSCOutputStream output;
            output.writeBundle (outBundle);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCBundle inBundle = input.readBundle();

            expectEquals (inBundle.size(), 1);

            OSCMessage inMessage = inBundle[0].getMessage();

            expectEquals (inMessage.getAddressPattern().toString(), Txt ("/test/one_arg"));
            expectEquals (inMessage.size(), 1);
            expectEquals (inMessage[0].getInt32(), 42);
        }

        beginTest ("OSC bundle with multiple messages");
        {
            OSCMessage outMessage1 ("/test/empty");
            OSCMessage outMessage2 ("/test/one_arg", 42);
            OSCMessage outMessage3 ("/test/four_args", 42, 0.5f, Txt ("foo"), Txt ("bar"));

            OSCBundle outBundle;
            outBundle.addElement (outMessage1);
            outBundle.addElement (outMessage2);
            outBundle.addElement (outMessage3);

            OSCOutputStream output;
            output.writeBundle (outBundle);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCBundle inBundle = input.readBundle();

            expectEquals (inBundle.size(), 3);

            {
                OSCMessage inMessage = inBundle[0].getMessage();

                expectEquals (inMessage.getAddressPattern().toString(), Txt ("/test/empty"));
                expectEquals (inMessage.size(), 0);
            }
            {
                OSCMessage inMessage = inBundle[1].getMessage();

                expectEquals (inMessage.getAddressPattern().toString(), Txt ("/test/one_arg"));
                expectEquals (inMessage.size(), 1);
                expectEquals (inMessage[0].getInt32(), 42);
            }
            {
                OSCMessage inMessage = inBundle[2].getMessage();

                expectEquals (inMessage.getAddressPattern().toString(), Txt ("/test/four_args"));
                expectEquals (inMessage.size(), 4);
                expectEquals (inMessage[0].getInt32(), 42);
                expectEquals (inMessage[1].getFloat32(), 0.5f);
                expectEquals (inMessage[2].getString(), Txt ("foo"));
                expectEquals (inMessage[3].getString(), Txt ("bar"));
            }
        }

        beginTest ("OSC bundle containing another bundle");
        {
            OSCBundle outBundleNested;
            outBundleNested.addElement (OSCMessage ("/test/one_arg", 42));

            OSCBundle outBundle;
            outBundle.addElement (outBundleNested);

            OSCOutputStream output;
            output.writeBundle (outBundle);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCBundle inBundle = input.readBundle();

            expectEquals (inBundle.size(), 1);
            expect (inBundle[0].isBundle());
            OSCBundle inBundleNested = inBundle[0].getBundle();
            expectEquals (inBundleNested.size(), 1);
            expect (inBundleNested[0].isMessage());

            OSCMessage msg = inBundleNested[0].getMessage();

            expectEquals (msg.getAddressPattern().toString(), Txt ("/test/one_arg"));
            expectEquals (msg.size(), 1);
            expectEquals (msg[0].getInt32(), 42);
        }

        beginTest ("OSC bundle containing multiple other bundles");
        {
            OSCBundle outBundleNested1;
            outBundleNested1.addElement (OSCMessage ("/test/empty"));
            OSCBundle outBundleNested2;
            outBundleNested2.addElement (OSCMessage ("/test/one_arg", 42));

            OSCBundle outBundle;
            outBundle.addElement (outBundleNested1);
            outBundle.addElement (outBundleNested2);

            OSCOutputStream output;
            output.writeBundle (outBundle);

            OSCInputStream input (output.getData(), output.getDataSize());
            OSCBundle inBundle = input.readBundle();

            expectEquals (inBundle.size(), 2);

            {
                expect (inBundle[0].isBundle());
                OSCBundle inBundleNested = inBundle[0].getBundle();
                expectEquals (inBundleNested.size(), 1);
                expect (inBundleNested[0].isMessage());

                OSCMessage msg = inBundleNested[0].getMessage();

                expectEquals (msg.getAddressPattern().toString(), Txt ("/test/empty"));
                expectEquals (msg.size(), 0);
            }
            {
                expect (inBundle[1].isBundle());
                OSCBundle inBundleNested = inBundle[1].getBundle();
                expectEquals (inBundleNested.size(), 1);
                expect (inBundleNested[0].isMessage());

                OSCMessage msg = inBundleNested[0].getMessage();

                expectEquals (msg.getAddressPattern().toString(), Txt ("/test/one_arg"));
                expectEquals (msg.size(), 1);
                expectEquals (msg[0].getInt32(), 42);
            }
        }
    }
};

static OSCRoundTripTests OSCRoundTripUnitTests;

#endif

} // namespace drx
