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
    Holds a list of ChangeListeners, and sends messages to them when instructed.

    @see ChangeListener

    @tags{Events}
*/
class DRX_API  ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an ChangeBroadcaster. */
    ChangeBroadcaster() noexcept;

    /** Destructor. */
    virtual ~ChangeBroadcaster();

    //==============================================================================
    /** Registers a listener to receive change callbacks from this broadcaster.
        Trying to add a listener that's already on the list will have no effect.
    */
    z0 addChangeListener (ChangeListener* listener);

    /** Unregisters a listener from the list.
        If the listener isn't on the list, this won't have any effect.
    */
    z0 removeChangeListener (ChangeListener* listener);

    /** Removes all listeners from the list. */
    z0 removeAllChangeListeners();

    //==============================================================================
    /** Causes an asynchronous change message to be sent to all the registered listeners.

        The message will be delivered asynchronously by the main message thread, so this
        method will return immediately. To call the listeners synchronously use
        sendSynchronousChangeMessage().
    */
    z0 sendChangeMessage();

    /** Sends a synchronous change message to all the registered listeners.

        This will immediately call all the listeners that are registered. For thread-safety
        reasons, you must only call this method on the main message thread.

        @see dispatchPendingMessages
    */
    z0 sendSynchronousChangeMessage();

    /** If a change message has been sent but not yet dispatched, this will call
        sendSynchronousChangeMessage() to make the callback immediately.

        For thread-safety reasons, you must only call this method on the main message thread.
    */
    z0 dispatchPendingMessages();

private:
    //==============================================================================
    class ChangeBroadcasterCallback  : public AsyncUpdater
    {
    public:
        ChangeBroadcasterCallback();
        ~ChangeBroadcasterCallback() override { cancelPendingUpdate(); }
        z0 handleAsyncUpdate() override;

        ChangeBroadcaster* owner;
    };

    friend class ChangeBroadcasterCallback;
    ChangeBroadcasterCallback broadcastCallback;
    ListenerList <ChangeListener> changeListeners;

    std::atomic<b8> anyListeners { false };

    z0 callListeners();

    DRX_DECLARE_NON_COPYABLE (ChangeBroadcaster)
};

} // namespace drx
