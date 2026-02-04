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
    MessageListener subclasses can post and receive Message objects.

    @see Message, MessageManager, ActionListener, ChangeListener

    @tags{Events}
*/
class DRX_API  MessageListener
{
public:
    //==============================================================================
    MessageListener() noexcept;

    /** Destructor. */
    virtual ~MessageListener();

    //==============================================================================
    /** This is the callback method that receives incoming messages.

        This is called by the MessageManager from its dispatch loop.

        @see postMessage
    */
    virtual z0 handleMessage (const Message& message) = 0;

    //==============================================================================
    /** Sends a message to the message queue, for asynchronous delivery to this listener
        later on.

        This method can be called safely by any thread.

        @param message      the message object to send - this will be deleted
                            automatically by the message queue, so make sure it's
                            allocated on the heap, not the stack!
        @see handleMessage
    */
    z0 postMessage (Message* message) const;

private:
    WeakReference<MessageListener>::Master masterReference;
    friend class WeakReference<MessageListener>;
};

} // namespace drx
