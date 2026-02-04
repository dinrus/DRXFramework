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
    /** Allows a block of data to be accessed as a stream of OSC data.

        The memory is shared and will be neither copied nor owned by the OSCInputStream.

        This class is implementing the Open Sound Control 1.0 Specification for
        interpreting the data.

        Note: Some older implementations of OSC may omit the OSC Type Tag string
        in OSC messages. This class will treat such OSC messages as format errors.
    */
    class OSCInputStream
    {
    public:
        /** Creates an OSCInputStream.

            @param sourceData               the block of data to use as the stream's source
            @param sourceDataSize           the number of bytes in the source data block
        */
        OSCInputStream (ukk sourceData, size_t sourceDataSize)
            : input (sourceData, sourceDataSize, false)
        {}

        //==============================================================================
        /** Returns a pointer to the source data block from which this stream is reading. */
        ukk getData() const noexcept        { return input.getData(); }

        /** Returns the number of bytes of source data in the block from which this stream is reading. */
        size_t getDataSize() const noexcept         { return input.getDataSize(); }

        /** Returns the current position of the stream. */
        zu64 getPosition()                        { return (zu64) input.getPosition(); }

        /** Attempts to set the current position of the stream. Возвращает true, если this was successful. */
        b8 setPosition (z64 pos)                { return input.setPosition (pos); }

        /** Returns the total amount of data in bytes accessible by this stream. */
        z64 getTotalLength()                      { return input.getTotalLength(); }

        /** Возвращает true, если the stream has no more data to read. */
        b8 isExhausted()                          { return input.isExhausted(); }

        //==============================================================================
        i32 readInt32()
        {
            checkBytesAvailable (4, "OSC input stream exhausted while reading i32");
            return input.readIntBigEndian();
        }

        zu64 readUint64()
        {
            checkBytesAvailable (8, "OSC input stream exhausted while reading zu64");
            return (zu64) input.readInt64BigEndian();
        }

        f32 readFloat32()
        {
            checkBytesAvailable (4, "OSC input stream exhausted while reading f32");
            return input.readFloatBigEndian();
        }

        Txt readString()
        {
            checkBytesAvailable (4, "OSC input stream exhausted while reading string");

            auto posBegin = (size_t) getPosition();
            auto s = input.readString();
            auto posEnd = (size_t) getPosition();

            if (static_cast<tukk> (getData()) [posEnd - 1] != '\0')
                throw OSCFormatError ("OSC input stream exhausted before finding null terminator of string");

            size_t bytesRead = posEnd - posBegin;
            readPaddingZeros (bytesRead);

            return s;
        }

        MemoryBlock readBlob()
        {
            checkBytesAvailable (4, "OSC input stream exhausted while reading blob");

            auto blobDataSize = input.readIntBigEndian();
            checkBytesAvailable ((blobDataSize + 3) % 4, "OSC input stream exhausted before reaching end of blob");

            MemoryBlock blob;
            auto bytesRead = input.readIntoMemoryBlock (blob, (ssize_t) blobDataSize);
            readPaddingZeros (bytesRead);

            return blob;
        }

        OSCColor readColor()
        {
            checkBytesAvailable (4, "OSC input stream exhausted while reading colour");
            return OSCColor::fromInt32 ((u32) input.readIntBigEndian());
        }

        OSCTimeTag readTimeTag()
        {
            checkBytesAvailable (8, "OSC input stream exhausted while reading time tag");
            return OSCTimeTag (zu64 (input.readInt64BigEndian()));
        }

        OSCAddress readAddress()
        {
            return OSCAddress (readString());
        }

        OSCAddressPattern readAddressPattern()
        {
            return OSCAddressPattern (readString());
        }

        //==============================================================================
        OSCTypeList readTypeTagString()
        {
            OSCTypeList typeList;

            checkBytesAvailable (4, "OSC input stream exhausted while reading type tag string");

            if (input.readByte() != ',')
                throw OSCFormatError ("OSC input stream format error: expected type tag string");

            for (;;)
            {
                if (isExhausted())
                    throw OSCFormatError ("OSC input stream exhausted while reading type tag string");

                const OSCType type = input.readByte();

                if (type == 0)
                    break;  // encountered null terminator. list is complete.

                if (! OSCTypes::isSupportedType (type))
                    throw OSCFormatError ("OSC input stream format error: encountered unsupported type tag");

                typeList.add (type);
            }

            auto bytesRead = (size_t) typeList.size() + 2;
            readPaddingZeros (bytesRead);

            return typeList;
        }

        //==============================================================================
        OSCArgument readArgument (OSCType type)
        {
            switch (type)
            {
                case OSCTypes::i32:       return OSCArgument (readInt32());
                case OSCTypes::float32:     return OSCArgument (readFloat32());
                case OSCTypes::string:      return OSCArgument (readString());
                case OSCTypes::blob:        return OSCArgument (readBlob());
                case OSCTypes::colour:      return OSCArgument (readColor());

                default:
                    // You supplied an invalid OSCType when calling readArgument! This should never happen.
                    jassertfalse;
                    throw OSCInternalError ("OSC input stream: internal error while reading message argument");
            }
        }

        //==============================================================================
        OSCMessage readMessage()
        {
            auto ap = readAddressPattern();
            auto types = readTypeTagString();

            OSCMessage msg (ap);

            for (auto& type : types)
                msg.addArgument (readArgument (type));

            return msg;
        }

        //==============================================================================
        OSCBundle readBundle (size_t maxBytesToRead = std::numeric_limits<size_t>::max())
        {
            // maxBytesToRead is only passed in here in case this bundle is a nested
            // bundle, so we know when to consider the next element *not* part of this
            // bundle anymore (but part of the outer bundle) and return.

            checkBytesAvailable (16, "OSC input stream exhausted while reading bundle");

            if (readString() != "#bundle")
                throw OSCFormatError ("OSC input stream format error: bundle does not start with string '#bundle'");

            OSCBundle bundle (readTimeTag());

            size_t bytesRead = 16; // already read "#bundle" and timeTag
            auto pos = getPosition();

            while (! isExhausted() && bytesRead < maxBytesToRead)
            {
                bundle.addElement (readElement());

                auto newPos = getPosition();
                bytesRead += (size_t) (newPos - pos);
                pos = newPos;
            }

            return bundle;
        }

        //==============================================================================
        OSCBundle::Element readElement()
        {
            checkBytesAvailable (4, "OSC input stream exhausted while reading bundle element size");

            auto elementSize = (size_t) readInt32();

            if (elementSize < 4)
                throw OSCFormatError ("OSC input stream format error: invalid bundle element size");

            return readElementWithKnownSize (elementSize);
        }

        //==============================================================================
        OSCBundle::Element readElementWithKnownSize (size_t elementSize)
        {
            checkBytesAvailable ((z64) elementSize, "OSC input stream exhausted while reading bundle element content");

            auto firstContentChar = static_cast<tukk> (getData()) [getPosition()];

            if (firstContentChar == '/')  return OSCBundle::Element (readMessageWithCheckedSize (elementSize));
            if (firstContentChar == '#')  return OSCBundle::Element (readBundleWithCheckedSize (elementSize));

            throw OSCFormatError ("OSC input stream: invalid bundle element content");
        }

    private:
        MemoryInputStream input;

        //==============================================================================
        z0 readPaddingZeros (size_t bytesRead)
        {
            size_t numZeros = ~(bytesRead - 1) & 0x03;

            while (numZeros > 0)
            {
                if (isExhausted() || input.readByte() != 0)
                    throw OSCFormatError ("OSC input stream format error: missing padding zeros");

                --numZeros;
            }
        }

        OSCBundle readBundleWithCheckedSize (size_t size)
        {
            auto begin = (size_t) getPosition();
            auto maxBytesToRead = size - 4; // we've already read 4 bytes (the bundle size)

            OSCBundle bundle (readBundle (maxBytesToRead));

            if (getPosition() - begin != size)
                throw OSCFormatError ("OSC input stream format error: wrong element content size encountered while reading");

            return bundle;
        }

        OSCMessage readMessageWithCheckedSize (size_t size)
        {
            auto begin = (size_t) getPosition();
            auto message = readMessage();

            if (getPosition() - begin != size)
                throw OSCFormatError ("OSC input stream format error: wrong element content size encountered while reading");

            return message;
        }

        z0 checkBytesAvailable (z64 requiredBytes, tukk message)
        {
            if (input.getNumBytesRemaining() < requiredBytes)
                throw OSCFormatError (message);
        }
    };

} // namespace


//==============================================================================
struct OSCReceiver::Pimpl   : private Thread,
                              private MessageListener
{
    Pimpl (const Txt& oscThreadName)  : Thread (oscThreadName)
    {
    }

    ~Pimpl() override
    {
        disconnect();
    }

    //==============================================================================
    b8 connectToPort (i32 portNumber)
    {
        if (! disconnect())
            return false;

        socket.setOwned (new DatagramSocket (false));

        if (! socket->bindToPort (portNumber))
            return false;

        startThread();
        return true;
    }

    b8 connectToSocket (DatagramSocket& newSocket)
    {
        if (! disconnect())
            return false;

        socket.setNonOwned (&newSocket);
        startThread();
        return true;
    }

    b8 disconnect()
    {
        if (socket != nullptr)
        {
            signalThreadShouldExit();

            if (socket.willDeleteObject())
                socket->shutdown();

            waitForThreadToExit (10000);
            socket.reset();
        }

        return true;
    }

    //==============================================================================
    z0 addListener (OSCReceiver::Listener<MessageLoopCallback>* listenerToAdd)
    {
        listeners.add (listenerToAdd);
    }

    z0 addListener (OSCReceiver::Listener<RealtimeCallback>* listenerToAdd)
    {
        realtimeListeners.add (listenerToAdd);
    }

    z0 addListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToAdd,
                      OSCAddress addressToMatch)
    {
        addListenerWithAddress (listenerToAdd, addressToMatch, listenersWithAddress);
    }

    z0 addListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToAdd, OSCAddress addressToMatch)
    {
        addListenerWithAddress (listenerToAdd, addressToMatch, realtimeListenersWithAddress);
    }

    z0 removeListener (OSCReceiver::Listener<MessageLoopCallback>* listenerToRemove)
    {
        listeners.remove (listenerToRemove);
    }

    z0 removeListener (OSCReceiver::Listener<RealtimeCallback>* listenerToRemove)
    {
        realtimeListeners.remove (listenerToRemove);
    }

    z0 removeListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToRemove)
    {
        removeListenerWithAddress (listenerToRemove, listenersWithAddress);
    }

    z0 removeListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToRemove)
    {
        removeListenerWithAddress (listenerToRemove, realtimeListenersWithAddress);
    }

    //==============================================================================
    struct CallbackMessage final : public Message
    {
        CallbackMessage (OSCBundle::Element oscElement)  : content (oscElement) {}

        // the payload of the message. Can be either an OSCMessage or an OSCBundle.
        OSCBundle::Element content;
    };

    //==============================================================================
    z0 handleBuffer (tukk data, size_t dataSize)
    {
        OSCInputStream inStream (data, dataSize);

        try
        {
            auto content = inStream.readElementWithKnownSize (dataSize);

            // realtime listeners should receive the OSC content first - and immediately
            // on this thread:
            callRealtimeListeners (content);

            if (content.isMessage())
                callRealtimeListenersWithAddress (content.getMessage());

            // now post the message that will trigger the handleMessage callback
            // dealing with the non-realtime listeners.
            if (listeners.size() > 0 || listenersWithAddress.size() > 0)
                postMessage (new CallbackMessage (content));
        }
        catch (const OSCFormatError&)
        {
            NullCheckedInvocation::invoke (formatErrorHandler, data, (i32) dataSize);
        }
    }

    //==============================================================================
    z0 registerFormatErrorHandler (OSCReceiver::FormatErrorHandler handler)
    {
        formatErrorHandler = handler;
    }

private:
    //==============================================================================
    z0 run() override
    {
        i32 bufferSize = 65535;
        HeapBlock<t8> oscBuffer (bufferSize);

        while (! threadShouldExit())
        {
            jassert (socket != nullptr);
            auto ready = socket->waitUntilReady (true, 100);

            if (ready < 0 || threadShouldExit())
                return;

            if (ready == 0)
                continue;

            auto bytesRead = (size_t) socket->read (oscBuffer.getData(), bufferSize, false);

            if (bytesRead >= 4)
                handleBuffer (oscBuffer.getData(), bytesRead);
        }
    }

    //==============================================================================
    template <typename ListenerType>
    z0 addListenerWithAddress (ListenerType* listenerToAdd,
                                 OSCAddress address,
                                 Array<std::pair<OSCAddress, ListenerType*>>& array)
    {
        for (auto& i : array)
            if (address == i.first && listenerToAdd == i.second)
                return;

        array.add (std::make_pair (address, listenerToAdd));
    }

    //==============================================================================
    template <typename ListenerType>
    z0 removeListenerWithAddress (ListenerType* listenerToRemove,
                                    Array<std::pair<OSCAddress, ListenerType*>>& array)
    {
        for (i32 i = 0; i < array.size(); ++i)
        {
            if (listenerToRemove == array.getReference (i).second)
            {
                // aarrgh... can't simply call array.remove (i) because this
                // requires a default c'tor to be present for OSCAddress...
                // luckily, we don't care about methods preserving element order:
                array.swap (i, array.size() - 1);
                array.removeLast();
                break;
            }
        }
    }

    //==============================================================================
    z0 handleMessage (const Message& msg) override
    {
        if (auto* callbackMessage = dynamic_cast<const CallbackMessage*> (&msg))
        {
            auto& content = callbackMessage->content;

            callListeners (content);

            if (content.isMessage())
                callListenersWithAddress (content.getMessage());
        }
    }

    //==============================================================================
    z0 callListeners (const OSCBundle::Element& content)
    {
        using OSCListener = OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>;

        if (content.isMessage())
        {
            auto&& message = content.getMessage();
            listeners.call ([&] (OSCListener& l) { l.oscMessageReceived (message); });
        }
        else if (content.isBundle())
        {
            auto&& bundle = content.getBundle();
            listeners.call ([&] (OSCListener& l) { l.oscBundleReceived (bundle); });
        }
    }

    z0 callRealtimeListeners (const OSCBundle::Element& content)
    {
        using OSCListener = OSCReceiver::Listener<OSCReceiver::RealtimeCallback>;

        if (content.isMessage())
        {
            auto&& message = content.getMessage();
            realtimeListeners.call ([&] (OSCListener& l) { l.oscMessageReceived (message); });
        }
        else if (content.isBundle())
        {
            auto&& bundle = content.getBundle();
            realtimeListeners.call ([&] (OSCListener& l) { l.oscBundleReceived (bundle); });
        }
    }

    //==============================================================================
    z0 callListenersWithAddress (const OSCMessage& message)
    {
        for (auto& entry : listenersWithAddress)
            if (auto* listener = entry.second)
                if (message.getAddressPattern().matches (entry.first))
                    listener->oscMessageReceived (message);
    }

    z0 callRealtimeListenersWithAddress (const OSCMessage& message)
    {
        for (auto& entry : realtimeListenersWithAddress)
            if (auto* listener = entry.second)
                if (message.getAddressPattern().matches (entry.first))
                    listener->oscMessageReceived (message);
    }

    //==============================================================================
    ListenerList<OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>> listeners;
    LightweightListenerList<OSCReceiver::Listener<OSCReceiver::RealtimeCallback>> realtimeListeners;

    Array<std::pair<OSCAddress, OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>*>> listenersWithAddress;
    Array<std::pair<OSCAddress, OSCReceiver::ListenerWithOSCAddress<OSCReceiver::RealtimeCallback>*>>    realtimeListenersWithAddress;

    OptionalScopedPointer<DatagramSocket> socket;
    OSCReceiver::FormatErrorHandler formatErrorHandler { nullptr };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
OSCReceiver::OSCReceiver (const Txt& threadName)   : pimpl (new Pimpl (threadName))
{
}

OSCReceiver::OSCReceiver()  : OSCReceiver ("DRX OSC server")
{
}

OSCReceiver::~OSCReceiver()
{
    pimpl.reset();
}

b8 OSCReceiver::connect (i32 portNumber)
{
    return pimpl->connectToPort (portNumber);
}

b8 OSCReceiver::connectToSocket (DatagramSocket& socket)
{
    return pimpl->connectToSocket (socket);
}

b8 OSCReceiver::disconnect()
{
    return pimpl->disconnect();
}

z0 OSCReceiver::addListener (OSCReceiver::Listener<MessageLoopCallback>* listenerToAdd)
{
    pimpl->addListener (listenerToAdd);
}

z0 OSCReceiver::addListener (Listener<RealtimeCallback>* listenerToAdd)
{
    pimpl->addListener (listenerToAdd);
}

z0 OSCReceiver::addListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToAdd, OSCAddress addressToMatch)
{
    pimpl->addListener (listenerToAdd, addressToMatch);
}

z0 OSCReceiver::addListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToAdd, OSCAddress addressToMatch)
{
    pimpl->addListener (listenerToAdd, addressToMatch);
}

z0 OSCReceiver::removeListener (Listener<MessageLoopCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

z0 OSCReceiver::removeListener (Listener<RealtimeCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

z0 OSCReceiver::removeListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

z0 OSCReceiver::removeListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

z0 OSCReceiver::registerFormatErrorHandler (FormatErrorHandler handler)
{
    pimpl->registerFormatErrorHandler (handler);
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class OSCInputStreamTests final : public UnitTest
{
public:
    OSCInputStreamTests()
        : UnitTest ("OSCInputStream class", UnitTestCategories::osc)
    {}

    z0 runTest() override
    {
        beginTest ("reading OSC addresses");
        {
            const t8 buffer[16] = {
                '/', 't', 'e', 's', 't', '/', 'f', 'a',
                'd', 'e', 'r', '7', '\0', '\0', '\0', '\0' };

            // reading a valid osc address:
            {
                OSCInputStream inStream (buffer, sizeof (buffer));
                OSCAddress address = inStream.readAddress();

                expect (inStream.getPosition() == sizeof (buffer));
                expectEquals (address.toString(), Txt ("/test/fader7"));
            }

            // check various possible failures:
            {
                // zero padding is present, but size is not modulo 4:
                OSCInputStream inStream (buffer, 15);
                expectThrowsType (inStream.readAddress(), OSCFormatError)
            }
            {
                // zero padding is missing:
                OSCInputStream inStream (buffer, 12);
                expectThrowsType (inStream.readAddress(), OSCFormatError)
            }
            {
                // pattern does not start with a forward slash:
                OSCInputStream inStream (buffer + 4, 12);
                expectThrowsType (inStream.readAddress(), OSCFormatError)
            }
        }

        beginTest ("reading OSC address patterns");
        {
            const t8 buffer[20] = {
                '/', '*', '/', '*', 'p', 'u', 't', '/',
                'f', 'a', 'd', 'e', 'r', '[', '0', '-',
                '9', ']', '\0', '\0' };

            // reading a valid osc address pattern:
            {
                OSCInputStream inStream (buffer, sizeof (buffer));
                expectDoesNotThrow (inStream.readAddressPattern());
            }
            {
                OSCInputStream inStream (buffer, sizeof (buffer));
                OSCAddressPattern ap = inStream.readAddressPattern();

                expect (inStream.getPosition() == sizeof (buffer));
                expectEquals (ap.toString(), Txt ("/*/*put/fader[0-9]"));
                expect (ap.containsWildcards());
            }

            // check various possible failures:
            {
                // zero padding is present, but size is not modulo 4:
                OSCInputStream inStream (buffer, 19);
                expectThrowsType (inStream.readAddressPattern(), OSCFormatError)
            }
            {
                // zero padding is missing:
                OSCInputStream inStream (buffer, 16);
                expectThrowsType (inStream.readAddressPattern(), OSCFormatError)
            }
            {
                // pattern does not start with a forward slash:
                OSCInputStream inStream (buffer + 4, 16);
                expectThrowsType (inStream.readAddressPattern(), OSCFormatError)
            }
        }

        beginTest ("reading OSC time tags");

        {
            t8 buffer[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
            OSCInputStream inStream (buffer, sizeof (buffer));

            OSCTimeTag tag = inStream.readTimeTag();
            expect (tag.isImmediately());
        }
        {
            t8 buffer[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            OSCInputStream inStream (buffer, sizeof (buffer));

            OSCTimeTag tag = inStream.readTimeTag();
            expect (! tag.isImmediately());
        }

        beginTest ("reading OSC arguments");

        {
            // test data:
            i32 testInt = -2015;
            u8k testIntRepresentation[] =  { 0xFF, 0xFF, 0xF8, 0x21 }; // big endian two's complement

            f32 testFloat = 345.6125f;
            u8k testFloatRepresentation[] = { 0x43, 0xAC, 0xCE, 0x66 }; // big endian IEEE 754

            Txt testString = "Hello, World!";
            const t8 testStringRepresentation[] = {
                'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                'o', 'r', 'l', 'd', '!', '\0', '\0', '\0' }; // padded to size % 4 == 0

            u8k testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
            const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));
            u8k testBlobRepresentation[] = {
                0x00, 0x00, 0x00, 0x05,
                0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00 }; // size prefixed + padded to size % 4 == 0

            // read:
            {
                {
                    // i32:
                    OSCInputStream inStream (testIntRepresentation, sizeof (testIntRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::i32);

                    expect (inStream.getPosition() == 4);
                    expect (arg.isInt32());
                    expectEquals (arg.getInt32(), testInt);
                }
                {
                    // float32:
                    OSCInputStream inStream (testFloatRepresentation, sizeof (testFloatRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::float32);

                    expect (inStream.getPosition() == 4);
                    expect (arg.isFloat32());
                    expectEquals (arg.getFloat32(), testFloat);
                }
                {
                    // string:
                    OSCInputStream inStream (testStringRepresentation, sizeof (testStringRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::string);

                    expect (inStream.getPosition() == sizeof (testStringRepresentation));
                    expect (arg.isString());
                    expectEquals (arg.getString(), testString);
                }
                {
                    // blob:
                    OSCInputStream inStream (testBlobRepresentation, sizeof (testBlobRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::blob);

                    expect (inStream.getPosition() == sizeof (testBlobRepresentation));
                    expect (arg.isBlob());
                    expect (arg.getBlob() == testBlob);
                }
            }

            // read invalid representations:

            {
                // not enough bytes
                {
                    u8k rawData[] = { 0xF8, 0x21 };

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::i32), OSCFormatError);
                    expectThrowsType (inStream.readArgument (OSCTypes::float32), OSCFormatError);
                }

                // test string not being padded to multiple of 4 bytes:
                {
                    const t8 rawData[] = {
                        'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                        'o', 'r', 'l', 'd', '!', '\0' }; // padding missing

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::string), OSCFormatError);
                }
                {
                    const t8 rawData[] = {
                        'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                        'o', 'r', 'l', 'd', '!', '\0', 'x', 'x' }; // padding with non-zero chars

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::string), OSCFormatError);
                }

                // test blob not being padded to multiple of 4 bytes:
                {
                    u8k rawData[] = { 0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }; // padding missing

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::blob), OSCFormatError);
                }

                // test blob having wrong size
                {
                    u8k rawData[] = { 0x00, 0x00, 0x00, 0x12, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::blob), OSCFormatError);
                }
            }
        }

        beginTest ("reading OSC messages (type tag string)");
        {
            {
                // valid empty message
                const t8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));

                auto msg = inStream.readMessage();
                expect (msg.getAddressPattern().toString() == "/test");
                expect (msg.size() == 0);
            }

            {
                // invalid message: no type tag string
                const t8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                    'o', 'r', 'l', 'd', '!', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                // invalid message: no type tag string and also empty
                const t8 data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            // invalid message: wrong padding
            {
                const t8 data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0', ',', '\0', '\0', '\0' };
                OSCInputStream inStream (data, sizeof (data) - 1);

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            // invalid message: says it contains an arg, but doesn't
            {
                const t8 data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0', ',', 'i', '\0', '\0' };
                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            // invalid message: binary size does not match size deducted from type tag string
            {
                const t8 data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0', ',', 'i', 'f', '\0' };
                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }
        }

        beginTest ("reading OSC messages (contents)");
        {
            // valid non-empty message.

            {
                i32 testInt = -2015;
                f32 testFloat = 345.6125f;
                Txt testString = "Hello, World!";

                u8k testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
                const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));

                u8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));

                auto msg = inStream.readMessage();

                expectEquals (msg.getAddressPattern().toString(), Txt ("/test"));
                expectEquals (msg.size(), 4);

                expectEquals (msg[0].getType(), OSCTypes::i32);
                expectEquals (msg[1].getType(), OSCTypes::float32);
                expectEquals (msg[2].getType(), OSCTypes::string);
                expectEquals (msg[3].getType(), OSCTypes::blob);

                expectEquals (msg[0].getInt32(), testInt);
                expectEquals (msg[1].getFloat32(), testFloat);
                expectEquals (msg[2].getString(), testString);
                expect (msg[3].getBlob() == testBlob);
            }
        }
        beginTest ("reading OSC messages (handling of corrupted messages)");
        {
            // invalid messages

            {
                OSCInputStream inStream (nullptr, 0);
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                u8k data[] = { 0x00 };
                OSCInputStream inStream (data, 0);
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', 'i', 'f', 's', 'b',   // type tag string not padded
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0'  // rest of message cut off
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }
        }

        beginTest ("reading OSC messages (handling messages without type tag strings)");
        {

            {
                u8 data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }
        }

        beginTest ("reading OSC bundles");
        {
            // valid bundle (empty)
            {
                u8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
                };

                OSCInputStream inStream (data, sizeof (data));
                OSCBundle bundle = inStream.readBundle();

                expect (bundle.getTimeTag().isImmediately());
                expect (bundle.size() == 0);
            }

            // valid bundle (containing both messages and other bundles)

            {
                i32 testInt = -2015;
                f32 testFloat = 345.6125f;
                Txt testString = "Hello, World!";
                u8k testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
                const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));

                u8 data[] = {
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
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
                };

                OSCInputStream inStream (data, sizeof (data));
                OSCBundle bundle = inStream.readBundle();

                expect (bundle.getTimeTag().isImmediately());
                expect (bundle.size() == 3);

                OSCBundle::Element* elements = bundle.begin();

                expect (elements[0].isMessage());
                expect (elements[0].getMessage().getAddressPattern().toString() == "/test/1");
                expect (elements[0].getMessage().size() == 4);
                expect (elements[0].getMessage()[0].isInt32());
                expect (elements[0].getMessage()[1].isFloat32());
                expect (elements[0].getMessage()[2].isString());
                expect (elements[0].getMessage()[3].isBlob());
                expectEquals (elements[0].getMessage()[0].getInt32(), testInt);
                expectEquals (elements[0].getMessage()[1].getFloat32(), testFloat);
                expectEquals (elements[0].getMessage()[2].getString(), testString);
                expect (elements[0].getMessage()[3].getBlob() == testBlob);

                expect (elements[1].isMessage());
                expect (elements[1].getMessage().getAddressPattern().toString() == "/test/2");
                expect (elements[1].getMessage().size() == 0);

                expect (elements[2].isBundle());
                expect (! elements[2].getBundle().getTimeTag().isImmediately());
            }

            // invalid bundles.

            {
                u8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,

                    0x00, 0x00, 0x00, 0x34,  // wrong bundle element size (too large)

                    '/', 't', 'e', 's', 't', '/', '1', '\0',
                    ',', 's', '\0', '\0',
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0'
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,

                    0x00, 0x00, 0x00, 0x08,  // wrong bundle element size (too small)

                    '/', 't', 'e', 's', 't', '/', '1', '\0',
                    ',', 's', '\0', '\0',
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0'
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00  // incomplete time tag
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '#', 'b', 'u', 'n', 'x', 'l', 'e', '\0', // wrong initial string
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                u8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', // padding missing from string
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }
        }
    }
};

static OSCInputStreamTests OSCInputStreamUnitTests;

#endif

} // namespace drx
