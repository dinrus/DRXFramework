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

  ID:                 drx_opengl
  vendor:             drx
  version:            8.0.7
  name:               DRX OpenGL classes
  description:        Classes for rendering OpenGL in a DRX window.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_gui_extra
  OSXFrameworks:      OpenGL
  iOSFrameworks:      OpenGLES
  linuxPackages:      gl

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_OPENGL_H_INCLUDED

#include <drx_core/system/drx_TargetPlatform.h>

#undef DRX_OPENGL
#define DRX_OPENGL 1

#if DRX_IOS || DRX_ANDROID
 #define DRX_OPENGL_ES 1
 #include "opengl/drx_gles2.h"
#else
 #include "opengl/drx_gl.h"
#endif

#include <drx_gui_extra/drx_gui_extra.h>

//==============================================================================
#if DRX_OPENGL_ES || DOXYGEN
 /** This macro is a helper for use in GLSL shader code which needs to compile on both GLES and desktop GL.
     Since it's mandatory in GLES to mark a variable with a precision, but the keywords don't exist in normal GLSL,
     these macros define the various precision keywords only on GLES.
 */
 #define DRX_MEDIUMP  "mediump"

 /** This macro is a helper for use in GLSL shader code which needs to compile on both GLES and desktop GL.
     Since it's mandatory in GLES to mark a variable with a precision, but the keywords don't exist in normal GLSL,
     these macros define the various precision keywords only on GLES.
 */
 #define DRX_HIGHP    "highp"

 /** This macro is a helper for use in GLSL shader code which needs to compile on both GLES and desktop GL.
     Since it's mandatory in GLES to mark a variable with a precision, but the keywords don't exist in normal GLSL,
     these macros define the various precision keywords only on GLES.
 */
 #define DRX_LOWP     "lowp"
#else
 #define DRX_MEDIUMP
 #define DRX_HIGHP
 #define DRX_LOWP
#endif

//==============================================================================
namespace drx
{
    class OpenGLTexture;
    class OpenGLFrameBuffer;
    class OpenGLShaderProgram;
}

#include "geometry/drx_Vector3D.h"
#include "geometry/drx_Matrix3D.h"
#include "geometry/drx_Quaternion.h"
#include "geometry/drx_Draggable3DOrientation.h"
#include "opengl/drx_OpenGLHelpers.h"
#include "opengl/drx_OpenGLPixelFormat.h"
#include "native/drx_OpenGLExtensions.h"
#include "opengl/drx_OpenGLRenderer.h"
#include "opengl/drx_OpenGLContext.h"
#include "opengl/drx_OpenGLFrameBuffer.h"
#include "opengl/drx_OpenGLGraphicsContext.h"
#include "opengl/drx_OpenGLImage.h"
#include "opengl/drx_OpenGLShaderProgram.h"
#include "opengl/drx_OpenGLTexture.h"
#include "utils/drx_OpenGLAppComponent.h"
