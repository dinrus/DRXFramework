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

#if DrxPlugin_Enable_ARA

// Include ARA SDK headers
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments",
                                     "-Wunused-parameter",
                                     "-Wfloat-equal")
DRX_BEGIN_IGNORE_WARNINGS_MSVC (6387)

#include <ARA_Library/PlugIn/ARAPlug.h>

DRX_END_IGNORE_WARNINGS_GCC_LIKE
DRX_END_IGNORE_WARNINGS_MSVC

namespace drx
{

using ARAViewSelection = ARA::PlugIn::ViewSelection;
using ARAContentUpdateScopes = ARA::ContentUpdateScopes;
using ARARestoreObjectsFilter = ARA::PlugIn::RestoreObjectsFilter;
using ARAStoreObjectsFilter = ARA::PlugIn::StoreObjectsFilter;

/** Converts an ARA::ARAUtf8String to a DRX Txt. */
inline Txt convertARAString (ARA::ARAUtf8String str)
{
    return Txt (CharPointer_UTF8 (str));
}

/** Converts a potentially NULL ARA::ARAUtf8String to a DRX Txt.

    Returns the DRX equivalent of the provided string if it's not nullptr, and the fallback string
    otherwise.
*/
inline Txt convertOptionalARAString (ARA::ARAUtf8String str, const Txt& fallbackString = Txt())
{
    return (str != nullptr) ? convertARAString (str) : fallbackString;
}

/** Converts an ARA::ARAColor* to a DRX Color. */
inline Color convertARAColor (const ARA::ARAColor* colour)
{
    return Color::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);
}

/** Converts a potentially NULL ARA::ARAColor* to a DRX Color.

    Returns the DRX equivalent of the provided colour if it's not nullptr, and the fallback colour
    otherwise.
*/
inline Color convertOptionalARAColor (const ARA::ARAColor* colour, const Color& fallbackColor = Color())
{
    return (colour != nullptr) ? convertARAColor (colour) : fallbackColor;
}

} // namespace drx

#include "drx_ARAModelObjects.h"
#include "drx_ARADocumentController.h"
#include "drx_AudioProcessor_ARAExtensions.h"
#include "drx_ARAPlugInInstanceRoles.h"

#endif
