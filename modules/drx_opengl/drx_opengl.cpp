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

#ifdef DRX_OPENGL_H_INCLUDED
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
#define DRX_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1
#define DRX_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#include "drx_opengl.h"

#define DRX_STATIC_LINK_GL_VERSION_1_0 1
#define DRX_STATIC_LINK_GL_VERSION_1_1 1

#if DRX_MAC
 #define DRX_STATIC_LINK_GL_VERSION_1_2 1
 #define DRX_STATIC_LINK_GL_VERSION_1_3 1
 #define DRX_STATIC_LINK_GL_VERSION_1_4 1
 #define DRX_STATIC_LINK_GL_VERSION_1_5 1
 #define DRX_STATIC_LINK_GL_VERSION_2_0 1
 #define DRX_STATIC_LINK_GL_VERSION_2_1 1
 #define DRX_STATIC_LINK_GL_VERSION_3_0 1
 #define DRX_STATIC_LINK_GL_VERSION_3_1 1
 #define DRX_STATIC_LINK_GL_VERSION_3_2 1
#endif

#define DRX_STATIC_LINK_GL_ES_VERSION_2_0 1
#if !DRX_ANDROID || DRX_ANDROID_GL_ES_VERSION_3_0
#define DRX_STATIC_LINK_GL_ES_VERSION_3_0 1
#endif

#if DRX_OPENGL_ES
 #include "opengl/drx_gles2.cpp"
#else
 #include "opengl/drx_gl.cpp"
#endif

//==============================================================================
#if DRX_IOS
 #import <QuartzCore/QuartzCore.h>

//==============================================================================
#elif DRX_WINDOWS
 #include <windowsx.h>

 #if ! DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "OpenGL32.Lib")
 #endif

//==============================================================================
#elif DRX_LINUX || DRX_BSD
 /* Got an include error here?

    If you want to install OpenGL support, the packages to get are "mesa-common-dev"
    and "freeglut3-dev".
 */
 #include <GL/glx.h>

//==============================================================================
#elif DRX_MAC
 #include <OpenGL/CGLCurrent.h> // These are both just needed with the 10.5 SDK
 #include <OpenGL/OpenGL.h>

//==============================================================================
#elif DRX_ANDROID
 #include <android/native_window.h>
 #include <android/native_window_jni.h>
 #include <EGL/egl.h>
#endif

//==============================================================================
namespace drx
{

using namespace ::drx::gl;

z0 OpenGLExtensionFunctions::initialise()
{
    gl::loadFunctions();
}

#define X(name) decltype (::drx::gl::name)& OpenGLExtensionFunctions::name = ::drx::gl::name;
DRX_GL_BASE_FUNCTIONS
DRX_GL_EXTENSION_FUNCTIONS
DRX_GL_VERTEXBUFFER_FUNCTIONS
#undef X

#if DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS && ! defined (DRX_CHECK_OPENGL_ERROR)
static tukk getGLErrorMessage (const GLenum e) noexcept
{
    switch (e)
    {
        case GL_INVALID_ENUM:                   return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                  return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:              return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:                  return "GL_OUT_OF_MEMORY";
       #ifdef GL_STACK_OVERFLOW
        case GL_STACK_OVERFLOW:                 return "GL_STACK_OVERFLOW";
       #endif
       #ifdef GL_STACK_UNDERFLOW
        case GL_STACK_UNDERFLOW:                return "GL_STACK_UNDERFLOW";
       #endif
       #ifdef GL_INVALID_FRAMEBUFFER_OPERATION
        case GL_INVALID_FRAMEBUFFER_OPERATION:  return "GL_INVALID_FRAMEBUFFER_OPERATION";
       #endif
        default: break;
    }

    return "Unknown error";
}

#if DRX_MAC || DRX_IOS

 #ifndef DRX_IOS_MAC_VIEW
  #if DRX_IOS
   #define DRX_IOS_MAC_VIEW    UIView
   #define DRX_IOS_MAC_WINDOW  UIWindow
  #else
   #define DRX_IOS_MAC_VIEW    NSView
   #define DRX_IOS_MAC_WINDOW  NSWindow
  #endif
 #endif

#endif

static b8 checkPeerIsValid (OpenGLContext* context)
{
    jassert (context != nullptr);

    if (context != nullptr)
    {
        if (auto* comp = context->getTargetComponent())
        {
            if (auto* peer [[maybe_unused]] = comp->getPeer())
            {
               #if DRX_MAC || DRX_IOS
                if (auto* nsView = (DRX_IOS_MAC_VIEW*) peer->getNativeHandle())
                {
                    if ([[maybe_unused]] auto nsWindow = [nsView window])
                    {
                       #if DRX_MAC
                        return ([nsWindow isVisible]
                                  && (! [nsWindow hidesOnDeactivate] || [NSApp isActive]));
                       #else
                        return true;
                       #endif
                    }
                }
               #else
                return true;
               #endif
            }
        }
    }

    return false;
}

static z0 checkGLError ([[maybe_unused]] tukk file, [[maybe_unused]] i32k line)
{
    for (;;)
    {
        const GLenum e = glGetError();

        if (e == GL_NO_ERROR)
            break;

        // if the peer is not valid then ignore errors
        if (! checkPeerIsValid (OpenGLContext::getCurrentContext()))
            continue;

        DBG ("***** " << getGLErrorMessage (e) << "  at " << file << " : " << line);
        jassertfalse;
    }
}

 #define DRX_CHECK_OPENGL_ERROR checkGLError (__FILE__, __LINE__);
#else
 #define DRX_CHECK_OPENGL_ERROR ;
#endif

static z0 clearGLError() noexcept
{
   #if DRX_DEBUG
    while (glGetError() != GL_NO_ERROR) {}
   #endif
}

struct OpenGLTargetSaver
{
    OpenGLTargetSaver (const OpenGLContext& c) noexcept
        : context (c), oldFramebuffer (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
    {
        glGetIntegerv (GL_VIEWPORT, oldViewport);
    }

    ~OpenGLTargetSaver() noexcept
    {
        context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, oldFramebuffer);
        glViewport (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    }

private:
    const OpenGLContext& context;
    GLuint oldFramebuffer;
    GLint oldViewport[4];

    OpenGLTargetSaver& operator= (const OpenGLTargetSaver&);
};

} // namespace drx

//==============================================================================
#include "opengl/drx_OpenGLFrameBuffer.cpp"
#include "opengl/drx_OpenGLGraphicsContext.cpp"
#include "opengl/drx_OpenGLHelpers.cpp"
#include "opengl/drx_OpenGLImage.cpp"
#include "opengl/drx_OpenGLPixelFormat.cpp"
#include "opengl/drx_OpenGLShaderProgram.cpp"
#include "opengl/drx_OpenGLTexture.cpp"

//==============================================================================
#if DRX_MAC || DRX_IOS

 #if DRX_MAC
  #include "native/drx_OpenGL_mac.h"
 #else
  #include "native/drx_OpenGL_ios.h"
 #endif

#elif DRX_WINDOWS
 #include "opengl/drx_wgl.h"
 #include "native/drx_OpenGL_windows.h"

#define DRX_IMPL_WGL_EXTENSION_FUNCTION(name) \
    decltype (drx::OpenGLContext::NativeContext::name) drx::OpenGLContext::NativeContext::name = nullptr;

DRX_IMPL_WGL_EXTENSION_FUNCTION (wglChoosePixelFormatARB)
DRX_IMPL_WGL_EXTENSION_FUNCTION (wglSwapIntervalEXT)
DRX_IMPL_WGL_EXTENSION_FUNCTION (wglGetSwapIntervalEXT)
DRX_IMPL_WGL_EXTENSION_FUNCTION (wglCreateContextAttribsARB)

#undef DRX_IMPL_WGL_EXTENSION_FUNCTION

#elif DRX_LINUX || DRX_BSD
 #include <drx_gui_basics/native/drx_ScopedWindowAssociation_linux.h>
 #include "native/drx_OpenGL_linux.h"

#elif DRX_ANDROID
 #include "native/drx_OpenGL_android.h"

#endif

#include "opengl/drx_OpenGLContext.cpp"
#include "utils/drx_OpenGLAppComponent.cpp"
