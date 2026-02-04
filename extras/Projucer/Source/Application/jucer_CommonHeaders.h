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

#pragma once


//==============================================================================
#ifdef linux
 #undef linux
#endif

struct TargetOS
{
    enum OS
    {
        windows = 0,
        osx,
        linux,
        unknown
    };

    static OS getThisOS() noexcept
    {
       #if DRX_WINDOWS
        return windows;
       #elif DRX_MAC
        return osx;
       #elif DRX_LINUX || DRX_BSD
        return linux;
       #else
        return unknown;
       #endif
    }
};

typedef TargetOS::OS DependencyPathOS;

//==============================================================================
#include "../Settings/jucer_StoredSettings.h"
#include "../Utility/UI/jucer_Icons.h"
#include "../Utility/Helpers/jucer_MiscUtilities.h"
#include "../Utility/Helpers/jucer_CodeHelpers.h"
#include "../Utility/Helpers/jucer_FileHelpers.h"
#include "../Utility/Helpers/jucer_ValueSourceHelpers.h"
#include "../Utility/Helpers/jucer_PresetIDs.h"
#include "jucer_CommandIDs.h"

//==============================================================================
tukk const projectItemDragType   = "Project Items";
tukk const drawableItemDragType  = "Drawable Items";
tukk const componentItemDragType = "Components";

enum ColorIds
{
    backgroundColorId                = 0x2340000,
    secondaryBackgroundColorId       = 0x2340001,
    defaultTextColorId               = 0x2340002,
    widgetTextColorId                = 0x2340003,
    defaultButtonBackgroundColorId   = 0x2340004,
    secondaryButtonBackgroundColorId = 0x2340005,
    userButtonBackgroundColorId      = 0x2340006,
    defaultIconColorId               = 0x2340007,
    treeIconColorId                  = 0x2340008,
    defaultHighlightColorId          = 0x2340009,
    defaultHighlightedTextColorId    = 0x234000a,
    codeEditorLineNumberColorId      = 0x234000b,
    activeTabIconColorId             = 0x234000c,
    inactiveTabBackgroundColorId     = 0x234000d,
    inactiveTabIconColorId           = 0x234000e,
    contentHeaderBackgroundColorId   = 0x234000f,
    widgetBackgroundColorId          = 0x2340010,
    secondaryWidgetBackgroundColorId = 0x2340011,
};

//==============================================================================
static constexpr i32 projucerMajorVersion = ProjectInfo::versionNumber >> 16;
