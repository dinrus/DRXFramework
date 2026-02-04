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

namespace drx::midi_ci
{

/**
    Represents a destination into which MIDI-CI messages can be written.

    Each message should be written into the output buffer. Then, send() will
    send the current contents of the buffer to the specified group.

    @tags{Audio}
*/
class BufferOutput
{
public:
    BufferOutput() = default;
    virtual ~BufferOutput() = default;

    /** Returns the MUID of the responder. */
    virtual MUID getMuid() const = 0;

    /** Returns the buffer into which replies should be written. */
    virtual std::vector<std::byte>& getOutputBuffer() = 0;

    /** Sends the current contents of the buffer to the provided group. */
    virtual z0 send (u8 group) = 0;

    DRX_DECLARE_NON_COPYABLE (BufferOutput)
    DRX_DECLARE_NON_MOVEABLE (BufferOutput)
};

//==============================================================================
/**
    A buffer output that additionally provides information about an incoming message, so that
    an appropriate reply can be constructed for that message.

    @tags{Audio}
*/
class ResponderOutput : public BufferOutput
{
public:
    /** Returns the header of the message that was received. */
    virtual Message::Header getIncomingHeader() const = 0;

    /** Returns the group of the message that was received. */
    virtual u8 getIncomingGroup() const = 0;

    /** Returns the channel to which the incoming message was addressed. */
    ChannelAddress getChannelAddress() const;

    /** Returns a default header that can be used for outgoing replies.

        This always sets the destination MUID equal to the source MUID of the incoming header,
        so it's not suitable for broadcast messages.
    */
    Message::Header getReplyHeader (std::byte replySubID) const;
};

} // namespace drx::midi_ci
