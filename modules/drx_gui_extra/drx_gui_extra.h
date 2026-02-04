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

  ID:                     drx_gui_extra
  vendor:                 drx
  version:                8.0.7
  name:                   DRX extended GUI classes
  description:            Miscellaneous GUI classes for specialised tasks.
  website:                http://www.drx.com/drx
  license:                AGPLv3/Commercial
  minimumCppStandard:     17

  dependencies:           drx_gui_basics
  OSXFrameworks:          WebKit
  iOSFrameworks:          WebKit
  WeakiOSFrameworks:      UserNotifications
  WeakMacOSFrameworks:    UserNotifications

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_GUI_EXTRA_H_INCLUDED

#include <drx_gui_basics/drx_gui_basics.h>

//==============================================================================
/** Config: DRX_WEB_BROWSER
    This lets you disable the WebBrowserComponent class.
    If you're not using any embedded web-pages, turning this off may reduce your code size.
*/
#ifndef DRX_WEB_BROWSER
 #define DRX_WEB_BROWSER 0
#endif

/** Config: DRX_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING
    Enables the use of the Microsoft Edge (Chromium) WebView2 browser on Windows.

    If using the Projucer, the Microsoft.Web.WebView2 package will be added to the
    project solution if this flag is enabled. If you are building using CMake you
    will need to manually add the package via the NuGet package manager.

    Using this flag requires statically linking against WebView2LoaderStatic.lib,
    which at this time is only available through the NuGet package, but is missing
    in VCPKG.

    In addition to enabling this macro, you will need to use the
    WebBrowserComponent::Options::Backend::webview2 option when instantiating the
    WebBrowserComponent.
*/
#ifndef DRX_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING
 #define DRX_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING 0
#else
 #define DRX_USE_WIN_WEBVIEW2 1
#endif

/** Config: DRX_USE_WIN_WEBVIEW2
    Enables the use of the Microsoft Edge (Chromium) WebView2 browser on Windows.

    If using the Projucer, the Microsoft.Web.WebView2 package will be added to the
    project solution if this flag is enabled. If you are building using CMake you
    will need to manually add the package via the Visual Studio package manager.

    If the WITH_STATIC_LINKING variant of this flag is also set, you must statically
    link against WebView2LoaderStatic.lib, otherwise dynamic loading will be used.

    See more about dynamic linking in the documentation of
    WebBrowserComponent::Options::WinWebView2::withDLLLocation().

    In addition to enabling this macro, you will need to use the
    WebBrowserComponent::Options::Backend::webview2 option when instantiating the
    WebBrowserComponent.
*/
#ifndef DRX_USE_WIN_WEBVIEW2
 #define DRX_USE_WIN_WEBVIEW2 0
#endif

/** Config: DRX_ENABLE_LIVE_CONSTANT_EDITOR
    This lets you turn on the DRX_ENABLE_LIVE_CONSTANT_EDITOR support (desktop only). By default
    this will be enabled for debug builds and disabled for release builds. See the documentation
    for that macro for more details.
*/
#ifndef DRX_ENABLE_LIVE_CONSTANT_EDITOR
 #if DRX_DEBUG && ! (DRX_IOS || DRX_ANDROID)
  #define DRX_ENABLE_LIVE_CONSTANT_EDITOR 1
 #endif
#endif

//==============================================================================
#include "documents/drx_FileBasedDocument.h"
#include "code_editor/drx_CodeDocument.h"
#include "code_editor/drx_CodeEditorComponent.h"
#include "code_editor/drx_CodeTokeniser.h"
#include "code_editor/drx_CPlusPlusCodeTokeniser.h"
#include "code_editor/drx_CPlusPlusCodeTokeniserFunctions.h"
#include "code_editor/drx_XMLCodeTokeniser.h"
#include "code_editor/drx_LuaCodeTokeniser.h"
#include "embedding/drx_ActiveXControlComponent.h"
#include "embedding/drx_AndroidViewComponent.h"
#include "embedding/drx_NSViewComponent.h"
#include "embedding/drx_UIViewComponent.h"
#include "embedding/drx_XEmbedComponent.h"
#include "embedding/drx_HWNDComponent.h"
#include "misc/drx_AppleRemote.h"
#include "misc/drx_BubbleMessageComponent.h"
#include "misc/drx_ColorSelector.h"
#include "misc/drx_KeyMappingEditorComponent.h"
#include "misc/drx_PreferencesPanel.h"
#include "misc/drx_PushNotifications.h"
#include "misc/drx_RecentlyOpenedFilesList.h"
#include "misc/drx_SplashScreen.h"
#include "misc/drx_SystemTrayIconComponent.h"
#include "misc/drx_WebBrowserComponent.h"
#include "misc/drx_LiveConstantEditor.h"
#include "misc/drx_AnimatedAppComponent.h"
#include "detail/drx_WebControlRelayEvents.h"
#include "misc/drx_WebControlRelays.h"
#include "misc/drx_WebControlParameterIndexReceiver.h"
