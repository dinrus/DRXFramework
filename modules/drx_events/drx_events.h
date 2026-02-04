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

  ID:                 drx_events
  vendor:             drx
  version:            8.0.7
  name:               DRX message and event handling classes
  description:        Classes for running an application's main event loop and sending/receiving messages, timers, etc.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_core

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_EVENTS_H_INCLUDED

#include <drx_core/drx_core.h>

//==============================================================================
/** Config: DRX_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
    Will execute your application's suspend method on an iOS background task, giving
    you extra time to save your applications state.
*/
#ifndef DRX_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
 #define DRX_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK 0
#endif

#if DRX_WINDOWS && DRX_EVENTS_INCLUDE_WINRT_WRAPPER
 // If this header file is missing then you are probably attempting to use WinRT
 // functionality without the WinRT libraries installed on your system. Try installing
 // the latest Windows Standalone SDK and maybe also adding the path to the WinRT
 // headers to your build system. This path should have the form
 // "C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\winrt".
 #include <inspectable.h>
 #include <hstring.h>
#endif

#include "messages/drx_MessageManager.h"
#include "messages/drx_Message.h"
#include "messages/drx_MessageListener.h"
#include "messages/drx_CallbackMessage.h"
#include "messages/drx_DeletedAtShutdown.h"
#include "messages/drx_NotificationType.h"
#include "messages/drx_ApplicationBase.h"
#include "messages/drx_Initialisation.h"
#include "messages/drx_MountedVolumeListChangeDetector.h"
#include "broadcasters/drx_ActionBroadcaster.h"
#include "broadcasters/drx_ActionListener.h"
#include "broadcasters/drx_AsyncUpdater.h"
#include "broadcasters/drx_LockingAsyncUpdater.h"
#include "broadcasters/drx_ChangeListener.h"
#include "broadcasters/drx_ChangeBroadcaster.h"
#include "timers/drx_Timer.h"
#include "timers/drx_TimedCallback.h"
#include "timers/drx_MultiTimer.h"
#include "interprocess/drx_ChildProcessManager.h"
#include "interprocess/drx_InterprocessConnection.h"
#include "interprocess/drx_InterprocessConnectionServer.h"
#include "interprocess/drx_ConnectedChildProcess.h"
#include "interprocess/drx_NetworkServiceDiscovery.h"
#include "native/drx_ScopedLowPowerModeDisabler.h"

#if DRX_LINUX || DRX_BSD
 #include "native/drx_EventLoop_linux.h"
#endif

#if DRX_WINDOWS
 #if DRX_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW
  #include "native/drx_HiddenMessageWindow_windows.h"
 #endif
 #if DRX_EVENTS_INCLUDE_WINRT_WRAPPER
  #include "native/drx_WinRTWrapper_windows.h"
 #endif
#endif
