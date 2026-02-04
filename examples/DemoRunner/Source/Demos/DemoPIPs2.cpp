/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#include <DrxHeader.h>
#include "../../../Assets/DemoUtilities.h"
#include "DRXDemos.h"

#include "../../../Assets/AudioLiveScrollingDisplay.h"

//==============================================================================
#if DRX_MAC || DRX_WINDOWS || DRX_IOS || DRX_ANDROID
 #include "../../../GUI/AccessibilityDemo.h"
#endif
#include "../../../GUI/AnimationAppDemo.h"
#include "../../../GUI/AnimationEasingDemo.h"
#include "../../../GUI/AnimatorsDemo.h"
#include "../../../GUI/BouncingBallWavetableDemo.h"
#if DRX_USE_CAMERA && ! (DRX_LINUX || DRX_BSD)
 #include "../../../GUI/CameraDemo.h"
#endif
#if ! DRX_ANDROID
 #include "../../../GUI/CodeEditorDemo.h"
#endif
#include "../../../GUI/ComponentDemo.h"
#include "../../../GUI/ComponentTransformsDemo.h"
#include "../../../GUI/DialogsDemo.h"
#include "../../../GUI/FlexBoxDemo.h"
#include "../../../GUI/FontsDemo.h"
#include "../../../GUI/GraphicsDemo.h"
#include "../../../GUI/GridDemo.h"
#include "../../../GUI/ImagesDemo.h"
#include "../../../GUI/KeyMappingsDemo.h"
#include "../../../GUI/LookAndFeelDemo.h"
#include "../../../GUI/MDIDemo.h"
#include "../../../GUI/MenusDemo.h"
#include "../../../GUI/MultiTouchDemo.h"
#if DRX_OPENGL
 #include "../../../GUI/OpenGLAppDemo.h"
 #include "../../../GUI/OpenGLDemo.h"
 #include "../../../GUI/OpenGLDemo2D.h"
#endif
#include "../../../GUI/PropertiesDemo.h"
#if ! (DRX_LINUX || DRX_BSD)
 #include "../../../GUI/VideoDemo.h"
#endif
#include "../../../GUI/WebBrowserDemo.h"
#include "../../../GUI/WidgetsDemo.h"
#include "../../../GUI/WindowsDemo.h"

z0 registerDemos_Two() noexcept
{
   #if DRX_MAC || DRX_WINDOWS || DRX_IOS || DRX_ANDROID
    REGISTER_DEMO (AccessibilityDemo,         GUI, false)
   #endif
    REGISTER_DEMO (AnimationAppDemo,          GUI, false)
    REGISTER_DEMO (AnimationEasingDemo,       GUI, false)
    REGISTER_DEMO (AnimatorsDemo,             GUI, false)
    REGISTER_DEMO (BouncingBallWavetableDemo, GUI, false)
   #if DRX_USE_CAMERA && ! (DRX_LINUX || DRX_BSD)
    REGISTER_DEMO (CameraDemo,                GUI, true)
   #endif
   #if ! DRX_ANDROID
    REGISTER_DEMO (CodeEditorDemo,            GUI, false)
   #endif
    REGISTER_DEMO (ComponentDemo,             GUI, false)
    REGISTER_DEMO (ComponentTransformsDemo,   GUI, false)
    REGISTER_DEMO (DialogsDemo,               GUI, false)
    REGISTER_DEMO (FlexBoxDemo,               GUI, false)
    REGISTER_DEMO (FontsDemo,                 GUI, false)
    REGISTER_DEMO (GraphicsDemo,              GUI, false)
    REGISTER_DEMO (GridDemo,                  GUI, false)
    REGISTER_DEMO (ImagesDemo,                GUI, false)
    REGISTER_DEMO (KeyMappingsDemo,           GUI, false)
    REGISTER_DEMO (LookAndFeelDemo,           GUI, false)
    REGISTER_DEMO (MDIDemo,                   GUI, false)
    REGISTER_DEMO (MenusDemo,                 GUI, false)
    REGISTER_DEMO (MultiTouchDemo,            GUI, false)
   #if DRX_OPENGL || DRX_LINUX
    REGISTER_DEMO (OpenGLAppDemo,             GUI, true)
    REGISTER_DEMO (OpenGLDemo2D,              GUI, true)
    REGISTER_DEMO (OpenGLDemo,                GUI, true)
   #endif
    REGISTER_DEMO (PropertiesDemo,            GUI, false)
   #if ! (DRX_LINUX || DRX_BSD)
    REGISTER_DEMO (VideoDemo,                 GUI, true)
   #endif
   #if DRX_WEB_BROWSER
    REGISTER_DEMO (WebBrowserDemo,            GUI, true)
   #endif
    REGISTER_DEMO (WidgetsDemo,               GUI, false)
    REGISTER_DEMO (WindowsDemo,               GUI, false)
}

CodeEditorComponent::ColorScheme getDarkColorScheme()
{
    return getDarkCodeEditorColorScheme();
}

CodeEditorComponent::ColorScheme getLightColorScheme()
{
    return getLightCodeEditorColorScheme();
}
