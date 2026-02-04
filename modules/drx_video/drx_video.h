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

  ID:                 drx_video
  vendor:             drx
  version:            8.0.7
  name:               DRX video playback and capture classes
  description:        Classes for playing video and capturing camera input.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_gui_extra
  OSXFrameworks:      AVKit AVFoundation CoreMedia
  iOSFrameworks:      AVKit AVFoundation CoreMedia

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_VIDEO_H_INCLUDED

//==============================================================================
#include <drx_gui_extra/drx_gui_extra.h>

//==============================================================================
/** Config: DRX_USE_CAMERA
    Enables camera support using the CameraDevice class (Mac, Windows, iOS, Android).
*/
#ifndef DRX_USE_CAMERA
 #define DRX_USE_CAMERA 0
#endif

#ifndef DRX_CAMERA_LOG_ENABLED
 #define DRX_CAMERA_LOG_ENABLED 0
#endif

#if DRX_CAMERA_LOG_ENABLED
 #define DRX_CAMERA_LOG(x) DBG(x)
#else
 #define DRX_CAMERA_LOG(x) {}
#endif

#if ! (DRX_MAC || DRX_WINDOWS || DRX_IOS || DRX_ANDROID)
 #undef DRX_USE_CAMERA
#endif

//==============================================================================
/** Config: DRX_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
    Enables synchronisation between video playback volume and OS media volume.
    Currently supported on Android only.
 */
#ifndef DRX_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
 #define DRX_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME 1
#endif

#ifndef DRX_VIDEO_LOG_ENABLED
 #define DRX_VIDEO_LOG_ENABLED 1
#endif

#if DRX_VIDEO_LOG_ENABLED
 #define DRX_VIDEO_LOG(x) DBG(x)
#else
 #define DRX_VIDEO_LOG(x) {}
#endif

//==============================================================================
#include "playback/drx_VideoComponent.h"
#include "capture/drx_CameraDevice.h"
