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
    A class for receiving OSC data.

    An OSCReceiver object allows you to receive OSC bundles and messages.
    It can connect to a network port, receive incoming OSC packets from the
    network via UDP, parse them, and forward the included OSCMessage and OSCBundle
    objects to its listeners.

    @tags{OSC}
*/
class DRX_API  OSCReceiver
{
public:
    //==============================================================================
    /** Creates an OSCReceiver. */
    OSCReceiver();

    /** Creates an OSCReceiver with a specific name for its thread. */
    OSCReceiver (const Txt& threadName);

    /** Destructor. */
    ~OSCReceiver();

    //==============================================================================
    /** Connects to the specified UDP port using a datagram socket,
        and starts listening to OSC packets arriving on this port.

        @returns true if the connection was successful; false otherwise.
    */
    b8 connect (i32 portNumber);

    /** Connects to a UDP datagram socket that is already set up,
        and starts listening to OSC packets arriving on this port.
        Make sure that the object you give it doesn't get deleted while this
        object is still using it!
        @returns true if the connection was successful; false otherwise.
    */
    b8 connectToSocket (DatagramSocket& socketToUse);

    //==============================================================================
    /** Disconnects from the currently used UDP port.
        @returns true if the disconnection was successful; false otherwise.
    */
    b8 disconnect();


    //==============================================================================
    /** Use this struct as the template parameter for Listener and
        ListenerWithOSCAddress to receive incoming OSC data on the message thread.
        This should be used by OSC callbacks that are not realtime-critical, but
        have significant work to do, for example updating Components in your app's
        user interface.

        This is the default type of OSC listener.
     */
    struct DRX_API  MessageLoopCallback {};

    /** Use this struct as the template parameter for Listener and
        ListenerWithOSCAddress to receive incoming OSC data immediately after it
        arrives, called directly on the network thread that listens to incoming
        OSC traffic.
        This type can be used by OSC callbacks that don't do much, but are
        realtime-critical, for example, setting real-time audio parameters.
    */
    struct DRX_API  RealtimeCallback {};

    //==============================================================================
    /** A class for receiving OSC data from an OSCReceiver.

        The template argument CallbackType determines how the callback will be called
        and has to be either MessageLoopCallback or RealtimeCallback. If not specified,
        MessageLoopCallback will be used by default.

        @see OSCReceiver::addListener, OSCReceiver::ListenerWithOSCAddress,
             OSCReceiver::MessageLoopCallback, OSCReceiver::RealtimeCallback

    */
    template <typename CallbackType = MessageLoopCallback>
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the OSCReceiver receives a new OSC message.
            You must implement this function.
        */
        virtual z0 oscMessageReceived (const OSCMessage& message) = 0;

        /** Called when the OSCReceiver receives a new OSC bundle.
            If you are not interested in OSC bundles, just ignore this method.
            The default implementation provided here will simply do nothing.
        */
        virtual z0 oscBundleReceived (const OSCBundle& /*bundle*/) {}
    };

    //==============================================================================
    /** A class for receiving only those OSC messages from an OSCReceiver that match a
        given OSC address.

        Use this class if your app receives OSC messages with different address patterns
        (for example "/drx/fader1", /drx/knob2" etc.) and you want to route those to
        different objects. This class contains pre-build functionality for that OSC
        address routing, including wildcard pattern matching (e.g. "/drx/fader[0-9]").

        This class implements the concept of an "OSC Method" from the OpenSoundControl 1.0
        specification.

        The template argument CallbackType determines how the callback will be called
        and has to be either MessageLoopCallback or RealtimeCallback. If not specified,
        MessageLoopCallback will be used by default.

        Note: This type of listener will ignore OSC bundles.

        @see OSCReceiver::addListener, OSCReceiver::Listener,
             OSCReceiver::MessageLoopCallback, OSCReceiver::RealtimeCallback
    */
    template <typename CallbackType = MessageLoopCallback>
    class DRX_API  ListenerWithOSCAddress
    {
    public:
        /** Destructor. */
        virtual ~ListenerWithOSCAddress() = default;

        /** Called when the OSCReceiver receives an OSC message with an OSC address
            pattern that matches the OSC address with which this listener was added.
        */
        virtual z0 oscMessageReceived (const OSCMessage& message) = 0;
    };

    //==============================================================================
    /** Adds a listener that listens to OSC messages and bundles.
        This listener will be called on the application's message loop.
    */
    z0 addListener (Listener<MessageLoopCallback>* listenerToAdd);

    /** Adds a listener that listens to OSC messages and bundles.
        This listener will be called in real-time directly on the network thread
        that receives OSC data.
    */
    z0 addListener (Listener<RealtimeCallback>* listenerToAdd);

    /** Adds a filtered listener that listens to OSC messages matching the address
        used to register the listener here.
        The listener will be called on the application's message loop.
    */
    z0 addListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToAdd,
                      OSCAddress addressToMatch);

    /** Adds a filtered listener that listens to OSC messages matching the address
        used to register the listener here.
        The listener will be called on the application's message loop.
     */
    z0 addListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToAdd,
                      OSCAddress addressToMatch);

    /** Removes a previously-registered listener. */
    z0 removeListener (Listener<MessageLoopCallback>* listenerToRemove);

    /** Removes a previously-registered listener. */
    z0 removeListener (Listener<RealtimeCallback>* listenerToRemove);

    /** Removes a previously-registered listener. */
    z0 removeListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToRemove);

    /** Removes a previously-registered listener. */
    z0 removeListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToRemove);

    //==============================================================================
    /** An error handler function for OSC format errors that can be called by the
        OSCReceiver.

        The arguments passed are the pointer to and the data of the buffer that
        the OSCReceiver has failed to parse.
    */
    using FormatErrorHandler = std::function<z0 (tukk data, i32 dataSize)>;

    /** Installs a custom error handler which is called in case the receiver
        encounters a stream it cannot parse as an OSC bundle or OSC message.

        By default (i.e. if you never use this method), in case of a parsing error
        nothing happens and the invalid packet is simply discarded.
    */
    z0 registerFormatErrorHandler (FormatErrorHandler handler);

private:
    //==============================================================================
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
    friend struct OSCReceiverCallbackMessage;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCReceiver)
};

} // namespace drx
