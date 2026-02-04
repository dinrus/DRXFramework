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

#ifdef DRX_PRODUCT_UNLOCKING_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_JNI_HELPERS    1
#define DRX_CORE_INCLUDE_OBJC_HELPERS   1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1

// Set this flag to 1 to use test servers on iOS
#ifndef DRX_IN_APP_PURCHASES_USE_SANDBOX_ENVIRONMENT
    #define DRX_IN_APP_PURCHASES_USE_SANDBOX_ENVIRONMENT 0
#endif

#include "drx_product_unlocking.h"

#if DRX_IOS || DRX_MAC
 #import <StoreKit/StoreKit.h>
#endif

#if DRX_IN_APP_PURCHASES
 #if DRX_ANDROID
  #include "native/drx_InAppPurchases_android.cpp"
 #elif DRX_IOS || DRX_MAC
  #include "native/drx_InAppPurchases_ios.cpp"
 #endif

 #include "in_app_purchases/drx_InAppPurchases.cpp"
#endif

#include "marketplace/drx_OnlineUnlockStatus.cpp"

#if DRX_MODULE_AVAILABLE_drx_data_structures
 #include "marketplace/drx_TracktionMarketplaceStatus.cpp"
#endif

#if DRX_MODULE_AVAILABLE_drx_gui_extra
 #include "marketplace/drx_OnlineUnlockForm.cpp"
#endif
