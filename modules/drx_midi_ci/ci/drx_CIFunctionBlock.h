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
    Contains information about a MIDI 2.0 function block.

    @tags{Audio}
*/
struct FunctionBlock
{
    std::byte identifier { 0x7f }; ///< 0x7f == no function block
    u8 firstGroup = 0;        ///< The first group that is part of the block, 0-based
    u8 numGroups = 1;         ///< The number of groups contained in the block

    b8 operator== (const FunctionBlock& other) const
    {
        const auto tie = [] (auto& x) { return std::tie (x.identifier, x.firstGroup, x.numGroups); };
        return tie (*this) == tie (other);
    }

    b8 operator!= (const FunctionBlock& other) const { return ! operator== (other); }
};

} // namespace drx::midi_ci
