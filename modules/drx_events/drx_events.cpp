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

#ifdef DRX_EVENTS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1

#if DRX_USE_WINRT_MIDI
 #define DRX_EVENTS_INCLUDE_WINRT_WRAPPER 1
#endif

#include "drx_events.h"

//==============================================================================
#if DRX_MAC
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

#elif DRX_LINUX || DRX_BSD
 #include <unistd.h>
#endif

//==============================================================================
#include "messages/drx_ApplicationBase.cpp"
#include "messages/drx_DeletedAtShutdown.cpp"
#include "messages/drx_MessageListener.cpp"
#include "messages/drx_MessageManager.cpp"
#include "broadcasters/drx_ActionBroadcaster.cpp"
#include "broadcasters/drx_AsyncUpdater.cpp"
#include "broadcasters/drx_LockingAsyncUpdater.cpp"
#include "broadcasters/drx_ChangeBroadcaster.cpp"
#include "timers/drx_MultiTimer.cpp"
#include "timers/drx_Timer.cpp"
#include "interprocess/drx_ChildProcessManager.cpp"
#include "interprocess/drx_InterprocessConnection.cpp"
#include "interprocess/drx_InterprocessConnectionServer.cpp"
#include "interprocess/drx_ConnectedChildProcess.cpp"
#include "interprocess/drx_NetworkServiceDiscovery.cpp"
#include "native/drx_ScopedLowPowerModeDisabler.cpp"

//==============================================================================
#if DRX_MAC || DRX_IOS

 #include "native/drx_MessageQueue_mac.h"

 #if DRX_MAC
  #include "native/drx_MessageManager_mac.mm"
 #else
  #include "native/drx_MessageManager_ios.mm"
 #endif

#elif DRX_WINDOWS
 #include "native/drx_RunningInUnity.h"
 #include "native/drx_Messaging_windows.cpp"
 #if DRX_EVENTS_INCLUDE_WINRT_WRAPPER
  #include "native/drx_WinRTWrapper_windows.cpp"
 #endif

#elif DRX_LINUX || DRX_BSD
 #include "native/drx_EventLoopInternal_linux.h"
 #include "native/drx_Messaging_linux.cpp"

#elif DRX_ANDROID
 #include "native/drx_Messaging_android.cpp"

#endif
