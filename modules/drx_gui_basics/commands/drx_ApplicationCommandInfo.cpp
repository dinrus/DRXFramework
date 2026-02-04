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

ApplicationCommandInfo::ApplicationCommandInfo (const CommandID cid) noexcept
    : commandID (cid), flags (0)
{
}

z0 ApplicationCommandInfo::setInfo (const Txt& shortName_,
                                      const Txt& description_,
                                      const Txt& categoryName_,
                                      i32k flags_) noexcept
{
    shortName = shortName_;
    description = description_;
    categoryName = categoryName_;
    flags = flags_;
}

z0 ApplicationCommandInfo::setActive (const b8 b) noexcept
{
    if (b)
        flags &= ~isDisabled;
    else
        flags |= isDisabled;
}

z0 ApplicationCommandInfo::setTicked (const b8 b) noexcept
{
    if (b)
        flags |= isTicked;
    else
        flags &= ~isTicked;
}

z0 ApplicationCommandInfo::addDefaultKeypress (i32k keyCode, ModifierKeys modifiers) noexcept
{
    defaultKeypresses.add (KeyPress (keyCode, modifiers, 0));
}

} // namespace drx
