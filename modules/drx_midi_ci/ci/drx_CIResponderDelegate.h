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
    An interface for types that implement responses for certain message types.

    @tags{Audio}
*/
class ResponderDelegate
{
public:
    ResponderDelegate() = default;
    virtual ~ResponderDelegate() = default;

    /** If the message is processed successfully, and a response sent, then
        this returns true. Otherwise, returns false, allowing other ResponderDelegates
        to attempt to handle the message if necessary.
    */
    virtual b8 tryRespond (ResponderOutput& output, const Message::Parsed& message) = 0;

    DRX_DECLARE_NON_COPYABLE (ResponderDelegate)
    DRX_DECLARE_NON_MOVEABLE (ResponderDelegate)
};

} // namespace drx::midi_ci
