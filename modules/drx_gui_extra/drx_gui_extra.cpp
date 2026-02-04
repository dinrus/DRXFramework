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

#ifdef DRX_GUI_EXTRA_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1
#define DRX_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1
#define DRX_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#ifndef DRX_PUSH_NOTIFICATIONS
 #define DRX_PUSH_NOTIFICATIONS 0
#endif

#include "drx_gui_extra.h"

//==============================================================================
#if DRX_MAC
 #if DRX_WEB_BROWSER
  #import <WebKit/WebKit.h>
 #endif
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

 #if DRX_PUSH_NOTIFICATIONS
  #import <Foundation/NSUserNotification.h>

  #include "native/drx_PushNotifications_mac.cpp"
 #endif

//==============================================================================
#elif DRX_IOS
 #if DRX_WEB_BROWSER
  #import <WebKit/WebKit.h>
 #endif

 #if DRX_PUSH_NOTIFICATIONS
  #import <UserNotifications/UserNotifications.h>
  #include "native/drx_PushNotifications_ios.cpp"
 #endif

//==============================================================================
#elif DRX_ANDROID
 #if DRX_PUSH_NOTIFICATIONS
  #include "native/drx_PushNotifications_android.cpp"
 #endif

//==============================================================================
#elif DRX_WINDOWS
 #include <windowsx.h>
 #include <vfw.h>
 #include <commdlg.h>

 #if DRX_WEB_BROWSER
  #include <exdisp.h>
  #include <exdispid.h>

  #if DRX_USE_WIN_WEBVIEW2
   #include <windows.foundation.h>
   #include <windows.foundation.collections.h>
   #include <winuser.h>

   DRX_BEGIN_IGNORE_WARNINGS_MSVC (4265)
   #include <wrl.h>
   #include <wrl/wrappers/corewrappers.h>
   DRX_END_IGNORE_WARNINGS_MSVC

   #include "WebView2.h"

   DRX_BEGIN_IGNORE_WARNINGS_MSVC (4458)
   #include "WebView2EnvironmentOptions.h"
   DRX_END_IGNORE_WARNINGS_MSVC
  #endif

 #endif

//==============================================================================
#elif (DRX_LINUX || DRX_BSD) && DRX_WEB_BROWSER
 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", "-Wparentheses", "-Wdeprecated-declarations")

 // If you're missing this header, you need to install the webkit2gtk-4.1 or webkit2gtk-4.0 package
 #include <gtk/gtk.h>
 #include <gtk/gtkx.h>
 #include <glib-unix.h>
 #include <webkit2/webkit2.h>
 #include <jsc/jsc.h>
 #include <libsoup/soup.h>

 DRX_END_IGNORE_WARNINGS_GCC_LIKE
#endif

//==============================================================================
#include "documents/drx_FileBasedDocument.cpp"
#include "code_editor/drx_CodeDocument.cpp"
#include "code_editor/drx_CodeEditorComponent.cpp"
#include "code_editor/drx_CPlusPlusCodeTokeniser.cpp"
#include "code_editor/drx_XMLCodeTokeniser.cpp"
#include "code_editor/drx_LuaCodeTokeniser.cpp"
#include "misc/drx_BubbleMessageComponent.cpp"
#include "misc/drx_ColorSelector.cpp"
#include "misc/drx_KeyMappingEditorComponent.cpp"
#include "misc/drx_PreferencesPanel.cpp"
#include "misc/drx_PushNotifications.cpp"
#include "misc/drx_RecentlyOpenedFilesList.cpp"
#include "misc/drx_SplashScreen.cpp"

#if DRX_MAC
 #include "native/drx_SystemTrayIcon_mac.cpp"
#elif DRX_WINDOWS
 #include "native/drx_ActiveXComponent_windows.cpp"
 #include "native/drx_SystemTrayIcon_windows.cpp"
#elif DRX_LINUX || DRX_BSD
 #include "native/drx_SystemTrayIcon_linux.cpp"
#endif

#include "misc/drx_SystemTrayIconComponent.cpp"
#include "misc/drx_LiveConstantEditor.cpp"
#include "misc/drx_AnimatedAppComponent.cpp"
#include "misc/drx_WebBrowserComponent.cpp"
#include "misc/drx_WebControlRelays.cpp"

//==============================================================================
#if DRX_MAC || DRX_IOS

 #if DRX_MAC
  #include "native/drx_NSViewFrameWatcher_mac.h"
  #include "native/drx_NSViewComponent_mac.mm"
  #include "native/drx_AppleRemote_mac.mm"
 #endif

 #if DRX_IOS
  #include "native/drx_UIViewComponent_ios.mm"
 #endif

 #if DRX_WEB_BROWSER
  #include "native/drx_WebBrowserComponent_mac.mm"
 #endif

//==============================================================================
#elif DRX_WINDOWS
 #include "native/drx_HWNDComponent_windows.cpp"
 #if DRX_WEB_BROWSER
  #include "native/drx_WebBrowserComponent_windows.cpp"
 #endif

//==============================================================================
#elif DRX_LINUX || DRX_BSD
 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

 #include <drx_gui_basics/native/drx_ScopedWindowAssociation_linux.h>
 #include "native/drx_XEmbedComponent_linux.cpp"

 #if DRX_WEB_BROWSER
  #if DRX_USE_EXTERNAL_TEMPORARY_SUBPROCESS
   #include "drx_LinuxSubprocessHelperBinaryData.h"
  #endif

  #include "native/drx_WebBrowserComponent_linux.cpp"
 #endif

 DRX_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
#elif DRX_ANDROID
 #include "native/drx_AndroidViewComponent.cpp"

 #if DRX_WEB_BROWSER
  #include "native/drx_WebBrowserComponent_android.cpp"
 #endif
#endif
