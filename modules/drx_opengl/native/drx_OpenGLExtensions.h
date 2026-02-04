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

namespace drx
{

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define DRX_GL_BASE_FUNCTIONS \
    X (glActiveTexture) \
    X (glBindBuffer) \
    X (glDeleteBuffers) \
    X (glGenBuffers) \
    X (glBufferData) \
    X (glBufferSubData) \
    X (glCreateProgram) \
    X (glDeleteProgram) \
    X (glCreateShader) \
    X (glDeleteShader) \
    X (glShaderSource) \
    X (glCompileShader) \
    X (glAttachShader) \
    X (glLinkProgram) \
    X (glUseProgram) \
    X (glGetShaderiv) \
    X (glGetShaderInfoLog) \
    X (glGetProgramInfoLog) \
    X (glGetProgramiv) \
    X (glGetUniformLocation) \
    X (glGetAttribLocation) \
    X (glVertexAttribPointer) \
    X (glEnableVertexAttribArray) \
    X (glDisableVertexAttribArray) \
    X (glUniform1f) \
    X (glUniform1i) \
    X (glUniform2f) \
    X (glUniform3f) \
    X (glUniform4f) \
    X (glUniform4i) \
    X (glUniform1fv) \
    X (glUniformMatrix2fv) \
    X (glUniformMatrix3fv) \
    X (glUniformMatrix4fv) \
    X (glBindAttribLocation)

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define DRX_GL_EXTENSION_FUNCTIONS \
    X (glIsRenderbuffer) \
    X (glBindRenderbuffer) \
    X (glDeleteRenderbuffers) \
    X (glGenRenderbuffers) \
    X (glRenderbufferStorage) \
    X (glGetRenderbufferParameteriv) \
    X (glIsFramebuffer) \
    X (glBindFramebuffer) \
    X (glDeleteFramebuffers) \
    X (glGenFramebuffers) \
    X (glCheckFramebufferStatus) \
    X (glFramebufferTexture2D) \
    X (glFramebufferRenderbuffer) \
    X (glGetFramebufferAttachmentParameteriv)

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define DRX_GL_VERTEXBUFFER_FUNCTIONS \
    X (glGenVertexArrays) \
    X (glDeleteVertexArrays) \
    X (glBindVertexArray)

/** This class contains a generated list of OpenGL extension functions, which are either dynamically loaded
    for a specific GL context, or simply call-through to the appropriate OS function where available.

    This class is provided for backwards compatibility. In new code, you should prefer to use
    functions from the drx::gl namespace. By importing all these symbols with
    `using namespace ::drx::gl;`, all GL enumerations and functions will be made available at
    global scope. This may be helpful if you need to write code with C source compatibility, or
    which is compatible with a different extension-loading library.
    All the normal guidance about `using namespace` should still apply - don't do this in a header,
    or at all if you can possibly avoid it!

    @tags{OpenGL}
*/
struct OpenGLExtensionFunctions
{
    //==============================================================================
   #ifndef DOXYGEN
    [[deprecated ("A more complete set of GL commands can be found in the drx::gl namespace. "
                  "You should use drx::gl::loadFunctions() to load GL functions.")]]
    static z0 initialise();
   #endif

   #if DRX_WINDOWS && ! defined (DOXYGEN)
    typedef t8 GLchar;
    typedef pointer_sized_int GLsizeiptr;
    typedef pointer_sized_int GLintptr;
   #endif

   #define X(name) static decltype (::drx::gl::name)& name;
    DRX_GL_BASE_FUNCTIONS
    DRX_GL_EXTENSION_FUNCTIONS
    DRX_GL_VERTEXBUFFER_FUNCTIONS
   #undef X
};

enum MissingOpenGLDefinitions
{
   #if DRX_ANDROID
    DRX_RGBA_FORMAT                = ::drx::gl::GL_RGBA,
   #else
    DRX_RGBA_FORMAT                = ::drx::gl::GL_BGRA_EXT,
   #endif
};

} // namespace drx
