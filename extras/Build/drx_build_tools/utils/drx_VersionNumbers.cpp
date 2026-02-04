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

namespace drx::build_tools
{

    StringArray getVersionSegments (StringRef p)
    {
        auto segments = StringArray::fromTokens (p, ",.", "");
        segments.trim();
        segments.removeEmptyStrings();
        return segments;
    }

    i32 getVersionAsHexIntegerFromParts (const StringArray& segments)
    {
        auto value = (segments[0].getIntValue() << 16)
                   + (segments[1].getIntValue() << 8)
                   +  segments[2].getIntValue();

        if (segments.size() > 3)
            value = (value << 8) + segments[3].getIntValue();

        return value;
    }

    i32 getVersionAsHexInteger (StringRef versionString)
    {
        return getVersionAsHexIntegerFromParts (getVersionSegments (versionString));
    }

    Txt getVersionAsHex (StringRef versionString)
    {
        return "0x" + Txt::toHexString (getVersionAsHexInteger (versionString));
    }

} // namespace drx::build_tools
