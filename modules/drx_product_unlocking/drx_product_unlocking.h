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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 DRX Module Format.md file.


 BEGIN_DRX_MODULE_DECLARATION

  ID:                 drx_product_unlocking
  vendor:             drx
  version:            8.0.7
  name:               DRX Online marketplace support
  description:        Classes for online product authentication
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_cryptography

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_PRODUCT_UNLOCKING_H_INCLUDED

/**
    The drx_product_unlocking module provides simple user-registration classes
    for allowing you to build apps/plugins with features that are unlocked by a
    user having a suitable account on a webserver.

    Although originally designed for use with products that are sold on the
    Tracktion Marketplace web-store, the module itself is fully open, and can
    be used to connect to your own web-store instead, if you implement your
    own compatible web-server back-end.

    In additional, the module supports in-app purchases both on iOS and Android
    platforms.
*/

//==============================================================================
#include <drx_core/drx_core.h>
#include <drx_cryptography/drx_cryptography.h>
#include <drx_events/drx_events.h>

#if DRX_MODULE_AVAILABLE_drx_data_structures
 #include <drx_data_structures/drx_data_structures.h>
#endif

#if DRX_MODULE_AVAILABLE_drx_gui_extra
 #include <drx_gui_extra/drx_gui_extra.h>
#endif

#if DRX_IN_APP_PURCHASES
 #include "in_app_purchases/drx_InAppPurchases.h"
#endif

#if DRX_MODULE_AVAILABLE_drx_data_structures
 #include "marketplace/drx_OnlineUnlockStatus.h"
 #include "marketplace/drx_TracktionMarketplaceStatus.h"
#endif

#include "marketplace/drx_KeyFileGeneration.h"

#if DRX_MODULE_AVAILABLE_drx_gui_extra
 #include "marketplace/drx_OnlineUnlockForm.h"
#endif
