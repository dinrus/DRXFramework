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

namespace drx::midi_ci::detail
{

/*
    Parses individual messages, and additionally gives ResponderDelegates a chance to formulate
    a response to any message that would normally necessitate a reply.
*/
struct Responder
{
    Responder() = delete;

    /*  Parses the message, then calls tryParse on each ResponderDelegate in
        turn until one returns true, indicating that the message has been
        handled. Most 'inquiry' messages should emit one or more reply messages.
        These replies will be written to the provided BufferOutput.
        If none of the provided delegates are able to handle the message, then
        a generic NAK will be written to the BufferOutput.
    */
    static Parser::Status processCompleteMessage (BufferOutput& output,
                                                  ump::BytesOnGroup message,
                                                  Span<ResponderDelegate* const> delegates);
};

} // namespace drx::midi_ci
