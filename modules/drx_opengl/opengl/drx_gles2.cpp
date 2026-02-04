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

// This file was generated automatically using data from the opengl-registry
// https://github.com/KhronosGroup/OpenGL-Registry


// This file's corresponding header contains a reference to a function pointer
// for each command in the GL spec. The functions from earlier GL versions are
// (pretty much) guaranteed to be available in the platform GL library. For
// these functions, the references will be initialised to point directly at the
// library implementations. This behaviour is somewhat configurable:
// preprocessor defs of the form `DRX_STATIC_LINK_<some gl version>` will
// ensure that the functions from a particular GL version are linked
// statically. Of course, this may fail to link if the platform doesn't
// implement the requested GL version. Any GL versions that are not explicitly
// requested for static linking, along with all known GL extensions, are loaded
// at runtime using gl::loadFunctions(). Again, these functions can be accessed
// via the references in the header.

// You should be aware that *any* of the functions declared in the header may
// be nullptr if the implementation does not supply that function. If you
// depend on specific GL features/extensions, it's probably a good idea to
// check each function pointer to ensure that the function was loaded
// successfully.


#define DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0 \
    X (z0        , glActiveTexture, (GLenum texture)) \
    X (z0        , glAttachShader, (GLuint program, GLuint shader)) \
    X (z0        , glBindAttribLocation, (GLuint program, GLuint index, const GLchar *name)) \
    X (z0        , glBindBuffer, (GLenum target, GLuint buffer)) \
    X (z0        , glBindFramebuffer, (GLenum target, GLuint framebuffer)) \
    X (z0        , glBindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
    X (z0        , glBindTexture, (GLenum target, GLuint texture)) \
    X (z0        , glBlendColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (z0        , glBlendEquation, (GLenum mode)) \
    X (z0        , glBlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glBlendFunc, (GLenum sfactor, GLenum dfactor)) \
    X (z0        , glBlendFuncSeparate, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)) \
    X (z0        , glBufferData, (GLenum target, GLsizeiptr size, ukk data, GLenum usage)) \
    X (z0        , glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, ukk data)) \
    X (GLenum      , glCheckFramebufferStatus, (GLenum target)) \
    X (z0        , glClear, (GLbitfield mask)) \
    X (z0        , glClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (z0        , glClearDepthf, (GLfloat d)) \
    X (z0        , glClearStencil, (GLint s)) \
    X (z0        , glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) \
    X (z0        , glCompileShader, (GLuint shader)) \
    X (z0        , glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (z0        , glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (GLuint      , glCreateProgram, ()) \
    X (GLuint      , glCreateShader, (GLenum type)) \
    X (z0        , glCullFace, (GLenum mode)) \
    X (z0        , glDeleteBuffers, (GLsizei n, const GLuint *buffers)) \
    X (z0        , glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers)) \
    X (z0        , glDeleteProgram, (GLuint program)) \
    X (z0        , glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers)) \
    X (z0        , glDeleteShader, (GLuint shader)) \
    X (z0        , glDeleteTextures, (GLsizei n, const GLuint *textures)) \
    X (z0        , glDepthFunc, (GLenum func)) \
    X (z0        , glDepthMask, (GLboolean flag)) \
    X (z0        , glDepthRangef, (GLfloat n, GLfloat f)) \
    X (z0        , glDetachShader, (GLuint program, GLuint shader)) \
    X (z0        , glDisable, (GLenum cap)) \
    X (z0        , glDisableVertexAttribArray, (GLuint index)) \
    X (z0        , glDrawArrays, (GLenum mode, GLint first, GLsizei count)) \
    X (z0        , glDrawElements, (GLenum mode, GLsizei count, GLenum type, ukk indices)) \
    X (z0        , glEnable, (GLenum cap)) \
    X (z0        , glEnableVertexAttribArray, (GLuint index)) \
    X (z0        , glFinish, ()) \
    X (z0        , glFlush, ()) \
    X (z0        , glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (z0        , glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glFrontFace, (GLenum mode)) \
    X (z0        , glGenBuffers, (GLsizei n, GLuint *buffers)) \
    X (z0        , glGenerateMipmap, (GLenum target)) \
    X (z0        , glGenFramebuffers, (GLsizei n, GLuint *framebuffers)) \
    X (z0        , glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
    X (z0        , glGenTextures, (GLsizei n, GLuint *textures)) \
    X (z0        , glGetActiveAttrib, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (z0        , glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (z0        , glGetAttachedShaders, (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)) \
    X (GLint       , glGetAttribLocation, (GLuint program, const GLchar *name)) \
    X (z0        , glGetBooleanv, (GLenum pname, GLboolean *data)) \
    X (z0        , glGetBufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (GLenum      , glGetError, ()) \
    X (z0        , glGetFloatv, (GLenum pname, GLfloat *data)) \
    X (z0        , glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint *params)) \
    X (z0        , glGetIntegerv, (GLenum pname, GLint *data)) \
    X (z0        , glGetProgramiv, (GLuint program, GLenum pname, GLint *params)) \
    X (z0        , glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetShaderiv, (GLuint shader, GLenum pname, GLint *params)) \
    X (z0        , glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glGetShaderPrecisionFormat, (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)) \
    X (z0        , glGetShaderSource, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)) \
    X (const GLubyte *, glGetString, (GLenum name)) \
    X (z0        , glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetUniformfv, (GLuint program, GLint location, GLfloat *params)) \
    X (z0        , glGetUniformiv, (GLuint program, GLint location, GLint *params)) \
    X (GLint       , glGetUniformLocation, (GLuint program, const GLchar *name)) \
    X (z0        , glGetVertexAttribfv, (GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVertexAttribiv, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribPointerv, (GLuint index, GLenum pname, uk *pointer)) \
    X (z0        , glHint, (GLenum target, GLenum mode)) \
    X (GLboolean   , glIsBuffer, (GLuint buffer)) \
    X (GLboolean   , glIsEnabled, (GLenum cap)) \
    X (GLboolean   , glIsFramebuffer, (GLuint framebuffer)) \
    X (GLboolean   , glIsProgram, (GLuint program)) \
    X (GLboolean   , glIsRenderbuffer, (GLuint renderbuffer)) \
    X (GLboolean   , glIsShader, (GLuint shader)) \
    X (GLboolean   , glIsTexture, (GLuint texture)) \
    X (z0        , glLineWidth, (GLfloat width)) \
    X (z0        , glLinkProgram, (GLuint program)) \
    X (z0        , glPixelStorei, (GLenum pname, GLint param)) \
    X (z0        , glPolygonOffset, (GLfloat factor, GLfloat units)) \
    X (z0        , glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, uk pixels)) \
    X (z0        , glReleaseShaderCompiler, ()) \
    X (z0        , glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glSampleCoverage, (GLfloat value, GLboolean invert)) \
    X (z0        , glScissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glShaderBinary, (GLsizei count, const GLuint *shaders, GLenum binaryFormat, ukk binary, GLsizei length)) \
    X (z0        , glShaderSource, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)) \
    X (z0        , glStencilFunc, (GLenum func, GLint ref, GLuint mask)) \
    X (z0        , glStencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask)) \
    X (z0        , glStencilMask, (GLuint mask)) \
    X (z0        , glStencilMaskSeparate, (GLenum face, GLuint mask)) \
    X (z0        , glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass)) \
    X (z0        , glStencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
    X (z0        , glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexParameterf, (GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glTexParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glTexParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glUniform1f, (GLint location, GLfloat v0)) \
    X (z0        , glUniform1fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform1i, (GLint location, GLint v0)) \
    X (z0        , glUniform1iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform2f, (GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glUniform2fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform2i, (GLint location, GLint v0, GLint v1)) \
    X (z0        , glUniform2iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glUniform3fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glUniform3iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glUniform4fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform4i, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glUniform4iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUseProgram, (GLuint program)) \
    X (z0        , glValidateProgram, (GLuint program)) \
    X (z0        , glVertexAttrib1f, (GLuint index, GLfloat x)) \
    X (z0        , glVertexAttrib1fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib2f, (GLuint index, GLfloat x, GLfloat y)) \
    X (z0        , glVertexAttrib2fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib3f, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glVertexAttrib3fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib4f, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glVertexAttrib4fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, ukk pointer)) \
    X (z0        , glViewport, (GLint x, GLint y, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0 \
    X (z0        , glReadBuffer, (GLenum src)) \
    X (z0        , glDrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices)) \
    X (z0        , glTexImage3D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glCompressedTexImage3D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glGenQueries, (GLsizei n, GLuint *ids)) \
    X (z0        , glDeleteQueries, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQuery, (GLuint id)) \
    X (z0        , glBeginQuery, (GLenum target, GLuint id)) \
    X (z0        , glEndQuery, (GLenum target)) \
    X (z0        , glGetQueryiv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint *params)) \
    X (GLboolean   , glUnmapBuffer, (GLenum target)) \
    X (z0        , glGetBufferPointerv, (GLenum target, GLenum pname, uk *params)) \
    X (z0        , glDrawBuffers, (GLsizei n, const GLenum *bufs)) \
    X (z0        , glUniformMatrix2x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix2x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (z0        , glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (uk       , glMapBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (z0        , glFlushMappedBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length)) \
    X (z0        , glBindVertexArray, (GLuint array)) \
    X (z0        , glDeleteVertexArrays, (GLsizei n, const GLuint *arrays)) \
    X (z0        , glGenVertexArrays, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArray, (GLuint array)) \
    X (z0        , glGetIntegeri_v, (GLenum target, GLuint index, GLint *data)) \
    X (z0        , glBeginTransformFeedback, (GLenum primitiveMode)) \
    X (z0        , glEndTransformFeedback, ()) \
    X (z0        , glBindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glBindBufferBase, (GLenum target, GLuint index, GLuint buffer)) \
    X (z0        , glTransformFeedbackVaryings, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)) \
    X (z0        , glGetTransformFeedbackVarying, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)) \
    X (z0        , glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glGetVertexAttribIiv, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribIuiv, (GLuint index, GLenum pname, GLuint *params)) \
    X (z0        , glVertexAttribI4i, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glVertexAttribI4ui, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (z0        , glVertexAttribI4iv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI4uiv, (GLuint index, const GLuint *v)) \
    X (z0        , glGetUniformuiv, (GLuint program, GLint location, GLuint *params)) \
    X (GLint       , glGetFragDataLocation, (GLuint program, const GLchar *name)) \
    X (z0        , glUniform1ui, (GLint location, GLuint v0)) \
    X (z0        , glUniform2ui, (GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glUniform3ui, (GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glUniform4ui, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glUniform1uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform2uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform3uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform4uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glClearBufferiv, (GLenum buffer, GLint drawbuffer, const GLint *value)) \
    X (z0        , glClearBufferuiv, (GLenum buffer, GLint drawbuffer, const GLuint *value)) \
    X (z0        , glClearBufferfv, (GLenum buffer, GLint drawbuffer, const GLfloat *value)) \
    X (z0        , glClearBufferfi, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
    X (const GLubyte *, glGetStringi, (GLenum name, GLuint index)) \
    X (z0        , glCopyBufferSubData, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (z0        , glGetUniformIndices, (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)) \
    X (z0        , glGetActiveUniformsiv, (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)) \
    X (GLuint      , glGetUniformBlockIndex, (GLuint program, const GLchar *uniformBlockName)) \
    X (z0        , glGetActiveUniformBlockiv, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)) \
    X (z0        , glGetActiveUniformBlockName, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)) \
    X (z0        , glUniformBlockBinding, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)) \
    X (z0        , glDrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X (z0        , glDrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount)) \
    X (GLsync      , glFenceSync, (GLenum condition, GLbitfield flags)) \
    X (GLboolean   , glIsSync, (GLsync sync)) \
    X (z0        , glDeleteSync, (GLsync sync)) \
    X (GLenum      , glClientWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (z0        , glWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (z0        , glGetInteger64v, (GLenum pname, GLint64 *data)) \
    X (z0        , glGetSynciv, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values)) \
    X (z0        , glGetInteger64i_v, (GLenum target, GLuint index, GLint64 *data)) \
    X (z0        , glGetBufferParameteri64v, (GLenum target, GLenum pname, GLint64 *params)) \
    X (z0        , glGenSamplers, (GLsizei count, GLuint *samplers)) \
    X (z0        , glDeleteSamplers, (GLsizei count, const GLuint *samplers)) \
    X (GLboolean   , glIsSampler, (GLuint sampler)) \
    X (z0        , glBindSampler, (GLuint unit, GLuint sampler)) \
    X (z0        , glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param)) \
    X (z0        , glSamplerParameteriv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (z0        , glSamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param)) \
    X (z0        , glSamplerParameterfv, (GLuint sampler, GLenum pname, const GLfloat *param)) \
    X (z0        , glGetSamplerParameteriv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (z0        , glGetSamplerParameterfv, (GLuint sampler, GLenum pname, GLfloat *params)) \
    X (z0        , glVertexAttribDivisor, (GLuint index, GLuint divisor)) \
    X (z0        , glBindTransformFeedback, (GLenum target, GLuint id)) \
    X (z0        , glDeleteTransformFeedbacks, (GLsizei n, const GLuint *ids)) \
    X (z0        , glGenTransformFeedbacks, (GLsizei n, GLuint *ids)) \
    X (GLboolean   , glIsTransformFeedback, (GLuint id)) \
    X (z0        , glPauseTransformFeedback, ()) \
    X (z0        , glResumeTransformFeedback, ()) \
    X (z0        , glGetProgramBinary, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, uk binary)) \
    X (z0        , glProgramBinary, (GLuint program, GLenum binaryFormat, ukk binary, GLsizei length)) \
    X (z0        , glProgramParameteri, (GLuint program, GLenum pname, GLint value)) \
    X (z0        , glInvalidateFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments)) \
    X (z0        , glInvalidateSubFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glTexStorage2D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTexStorage3D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glGetInternalformativ, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params))

#define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1 \
    X (z0        , glDispatchCompute, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)) \
    X (z0        , glDispatchComputeIndirect, (GLintptr indirect)) \
    X (z0        , glDrawArraysIndirect, (GLenum mode, ukk indirect)) \
    X (z0        , glDrawElementsIndirect, (GLenum mode, GLenum type, ukk indirect)) \
    X (z0        , glFramebufferParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glGetFramebufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetProgramInterfaceiv, (GLuint program, GLenum programInterface, GLenum pname, GLint *params)) \
    X (GLuint      , glGetProgramResourceIndex, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (z0        , glGetProgramResourceName, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (z0        , glGetProgramResourceiv, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params)) \
    X (GLint       , glGetProgramResourceLocation, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (z0        , glUseProgramStages, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (z0        , glActiveShaderProgram, (GLuint pipeline, GLuint program)) \
    X (GLuint      , glCreateShaderProgramv, (GLenum type, GLsizei count, const GLchar *const*strings)) \
    X (z0        , glBindProgramPipeline, (GLuint pipeline)) \
    X (z0        , glDeleteProgramPipelines, (GLsizei n, const GLuint *pipelines)) \
    X (z0        , glGenProgramPipelines, (GLsizei n, GLuint *pipelines)) \
    X (GLboolean   , glIsProgramPipeline, (GLuint pipeline)) \
    X (z0        , glGetProgramPipelineiv, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (z0        , glProgramUniform1i, (GLuint program, GLint location, GLint v0)) \
    X (z0        , glProgramUniform2i, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (z0        , glProgramUniform3i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glProgramUniform4i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glProgramUniform1ui, (GLuint program, GLint location, GLuint v0)) \
    X (z0        , glProgramUniform2ui, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glProgramUniform3ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glProgramUniform4ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glProgramUniform1f, (GLuint program, GLint location, GLfloat v0)) \
    X (z0        , glProgramUniform2f, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glProgramUniform3f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glProgramUniform4f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glProgramUniform1iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform2iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform3iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform4iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform1uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform2uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform3uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform4uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform1fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform2fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform3fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform4fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glValidateProgramPipeline, (GLuint pipeline)) \
    X (z0        , glGetProgramPipelineInfoLog, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glBindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)) \
    X (z0        , glGetBooleani_v, (GLenum target, GLuint index, GLboolean *data)) \
    X (z0        , glMemoryBarrier, (GLbitfield barriers)) \
    X (z0        , glMemoryBarrierByRegion, (GLbitfield barriers)) \
    X (z0        , glTexStorage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (z0        , glGetMultisamplefv, (GLenum pname, GLuint index, GLfloat *val)) \
    X (z0        , glSampleMaski, (GLuint maskNumber, GLbitfield mask)) \
    X (z0        , glGetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (z0        , glGetTexLevelParameterfv, (GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (z0        , glBindVertexBuffer, (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (z0        , glVertexAttribFormat, (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (z0        , glVertexAttribIFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexAttribBinding, (GLuint attribindex, GLuint bindingindex)) \
    X (z0        , glVertexBindingDivisor, (GLuint bindingindex, GLuint divisor))

#define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2 \
    X (z0        , glBlendBarrier, ()) \
    X (z0        , glCopyImageSubData, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)) \
    X (z0        , glDebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (z0        , glDebugMessageInsert, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (z0        , glDebugMessageCallback, (GLDEBUGPROC callback, ukk userParam)) \
    X (GLuint      , glGetDebugMessageLog, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)) \
    X (z0        , glPushDebugGroup, (GLenum source, GLuint id, GLsizei length, const GLchar *message)) \
    X (z0        , glPopDebugGroup, ()) \
    X (z0        , glObjectLabel, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectLabel, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (z0        , glObjectPtrLabel, (ukk ptr, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectPtrLabel, (ukk ptr, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (z0        , glGetPointerv, (GLenum pname, uk *params)) \
    X (z0        , glEnablei, (GLenum target, GLuint index)) \
    X (z0        , glDisablei, (GLenum target, GLuint index)) \
    X (z0        , glBlendEquationi, (GLuint buf, GLenum mode)) \
    X (z0        , glBlendEquationSeparatei, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glBlendFunci, (GLuint buf, GLenum src, GLenum dst)) \
    X (z0        , glBlendFuncSeparatei, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (z0        , glColorMaski, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (GLboolean   , glIsEnabledi, (GLenum target, GLuint index)) \
    X (z0        , glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLint basevertex)) \
    X (z0        , glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (z0        , glPrimitiveBoundingBox, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)) \
    X (GLenum      , glGetGraphicsResetStatus, ()) \
    X (z0        , glReadnPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, uk data)) \
    X (z0        , glGetnUniformfv, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (z0        , glGetnUniformiv, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (z0        , glGetnUniformuiv, (GLuint program, GLint location, GLsizei bufSize, GLuint *params)) \
    X (z0        , glMinSampleShading, (GLfloat value)) \
    X (z0        , glPatchParameteri, (GLenum pname, GLint value)) \
    X (z0        , glTexParameterIiv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexParameterIuiv, (GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetTexParameterIiv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexParameterIuiv, (GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glSamplerParameterIiv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (z0        , glSamplerParameterIuiv, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (z0        , glGetSamplerParameterIiv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (z0        , glGetSamplerParameterIuiv, (GLuint sampler, GLenum pname, GLuint *params)) \
    X (z0        , glTexBuffer, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (z0        , glTexBufferRange, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glTexStorage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))

#define DRX_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    X (z0        , glRenderbufferStorageMultisampleAdvancedAMD, (GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glNamedRenderbufferStorageMultisampleAdvancedAMD, (GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_AMD_performance_monitor \
    X (z0        , glGetPerfMonitorGroupsAMD, (GLint *numGroups, GLsizei groupsSize, GLuint *groups)) \
    X (z0        , glGetPerfMonitorCountersAMD, (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters)) \
    X (z0        , glGetPerfMonitorGroupStringAMD, (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString)) \
    X (z0        , glGetPerfMonitorCounterStringAMD, (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString)) \
    X (z0        , glGetPerfMonitorCounterInfoAMD, (GLuint group, GLuint counter, GLenum pname, uk data)) \
    X (z0        , glGenPerfMonitorsAMD, (GLsizei n, GLuint *monitors)) \
    X (z0        , glDeletePerfMonitorsAMD, (GLsizei n, GLuint *monitors)) \
    X (z0        , glSelectPerfMonitorCountersAMD, (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *counterList)) \
    X (z0        , glBeginPerfMonitorAMD, (GLuint monitor)) \
    X (z0        , glEndPerfMonitorAMD, (GLuint monitor)) \
    X (z0        , glGetPerfMonitorCounterDataAMD, (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten))

#define DRX_GL_FUNCTIONS_GL_ANGLE_framebuffer_blit \
    X (z0        , glBlitFramebufferANGLE, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

#define DRX_GL_FUNCTIONS_GL_ANGLE_framebuffer_multisample \
    X (z0        , glRenderbufferStorageMultisampleANGLE, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_ANGLE_instanced_arrays \
    X (z0        , glDrawArraysInstancedANGLE, (GLenum mode, GLint first, GLsizei count, GLsizei primcount)) \
    X (z0        , glDrawElementsInstancedANGLE, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei primcount)) \
    X (z0        , glVertexAttribDivisorANGLE, (GLuint index, GLuint divisor))

#define DRX_GL_FUNCTIONS_GL_ANGLE_translated_shader_source \
    X (z0        , glGetTranslatedShaderSourceANGLE, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source))

#define DRX_GL_FUNCTIONS_GL_APPLE_copy_texture_levels \
    X (z0        , glCopyTextureLevelsAPPLE, (GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount))

#define DRX_GL_FUNCTIONS_GL_APPLE_framebuffer_multisample \
    X (z0        , glRenderbufferStorageMultisampleAPPLE, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glResolveMultisampleFramebufferAPPLE, ())

#define DRX_GL_FUNCTIONS_GL_APPLE_sync \
    X (GLsync      , glFenceSyncAPPLE, (GLenum condition, GLbitfield flags)) \
    X (GLboolean   , glIsSyncAPPLE, (GLsync sync)) \
    X (z0        , glDeleteSyncAPPLE, (GLsync sync)) \
    X (GLenum      , glClientWaitSyncAPPLE, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (z0        , glWaitSyncAPPLE, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (z0        , glGetInteger64vAPPLE, (GLenum pname, GLint64 *params)) \
    X (z0        , glGetSyncivAPPLE, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values))

#define DRX_GL_FUNCTIONS_GL_ARM_shader_core_properties \
    X (z0        , glMaxActiveShaderCoresARM, (GLuint count))

#define DRX_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    X (z0        , glEGLImageTargetTexStorageEXT, (GLenum target, GLeglImageOES image, const GLint* attrib_list)) \
    X (z0        , glEGLImageTargetTextureStorageEXT, (GLuint texture, GLeglImageOES image, const GLint* attrib_list))

#define DRX_GL_FUNCTIONS_GL_EXT_base_instance \
    X (z0        , glDrawArraysInstancedBaseInstanceEXT, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)) \
    X (z0        , glDrawElementsInstancedBaseInstanceEXT, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLuint baseinstance)) \
    X (z0        , glDrawElementsInstancedBaseVertexBaseInstanceEXT, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance))

#define DRX_GL_FUNCTIONS_GL_EXT_blend_func_extended \
    X (z0        , glBindFragDataLocationIndexedEXT, (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)) \
    X (z0        , glBindFragDataLocationEXT, (GLuint program, GLuint color, const GLchar *name)) \
    X (GLint       , glGetProgramResourceLocationIndexEXT, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (GLint       , glGetFragDataIndexEXT, (GLuint program, const GLchar *name))

#define DRX_GL_FUNCTIONS_GL_EXT_blend_minmax \
    X (z0        , glBlendEquationEXT, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_EXT_buffer_storage \
    X (z0        , glBufferStorageEXT, (GLenum target, GLsizeiptr size, ukk data, GLbitfield flags))

#define DRX_GL_FUNCTIONS_GL_EXT_clear_texture \
    X (z0        , glClearTexImageEXT, (GLuint texture, GLint level, GLenum format, GLenum type, ukk data)) \
    X (z0        , glClearTexSubImageEXT, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk data))

#define DRX_GL_FUNCTIONS_GL_EXT_clip_control \
    X (z0        , glClipControlEXT, (GLenum origin, GLenum depth))

#define DRX_GL_FUNCTIONS_GL_EXT_copy_image \
    X (z0        , glCopyImageSubDataEXT, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth))

#define DRX_GL_FUNCTIONS_GL_EXT_debug_label \
    X (z0        , glLabelObjectEXT, (GLenum type, GLuint object, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectLabelEXT, (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label))

#define DRX_GL_FUNCTIONS_GL_EXT_debug_marker \
    X (z0        , glInsertEventMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (z0        , glPushGroupMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (z0        , glPopGroupMarkerEXT, ())

#define DRX_GL_FUNCTIONS_GL_EXT_discard_framebuffer \
    X (z0        , glDiscardFramebufferEXT, (GLenum target, GLsizei numAttachments, const GLenum *attachments))

#define DRX_GL_FUNCTIONS_GL_EXT_disjoint_timer_query \
    X (z0        , glGenQueriesEXT, (GLsizei n, GLuint *ids)) \
    X (z0        , glDeleteQueriesEXT, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQueryEXT, (GLuint id)) \
    X (z0        , glBeginQueryEXT, (GLenum target, GLuint id)) \
    X (z0        , glEndQueryEXT, (GLenum target)) \
    X (z0        , glQueryCounterEXT, (GLuint id, GLenum target)) \
    X (z0        , glGetQueryivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectivEXT, (GLuint id, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectuivEXT, (GLuint id, GLenum pname, GLuint *params)) \
    X (z0        , glGetQueryObjecti64vEXT, (GLuint id, GLenum pname, GLint64 *params)) \
    X (z0        , glGetQueryObjectui64vEXT, (GLuint id, GLenum pname, GLuint64 *params)) \
    X (z0        , glGetInteger64vEXT, (GLenum pname, GLint64 *data))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_buffers \
    X (z0        , glDrawBuffersEXT, (GLsizei n, const GLenum *bufs))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_buffers_indexed \
    X (z0        , glEnableiEXT, (GLenum target, GLuint index)) \
    X (z0        , glDisableiEXT, (GLenum target, GLuint index)) \
    X (z0        , glBlendEquationiEXT, (GLuint buf, GLenum mode)) \
    X (z0        , glBlendEquationSeparateiEXT, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glBlendFunciEXT, (GLuint buf, GLenum src, GLenum dst)) \
    X (z0        , glBlendFuncSeparateiEXT, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (z0        , glColorMaskiEXT, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (GLboolean   , glIsEnablediEXT, (GLenum target, GLuint index))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_elements_base_vertex \
    X (z0        , glDrawElementsBaseVertexEXT, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawRangeElementsBaseVertexEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawElementsInstancedBaseVertexEXT, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLint basevertex)) \
    X (z0        , glMultiDrawElementsBaseVertexEXT, (GLenum mode, const GLsizei *count, GLenum type, ukk const*indices, GLsizei drawcount, const GLint *basevertex))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_instanced \
    X (z0        , glDrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount)) \
    X (z0        , glDrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei primcount))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_transform_feedback \
    X (z0        , glDrawTransformFeedbackEXT, (GLenum mode, GLuint id)) \
    X (z0        , glDrawTransformFeedbackInstancedEXT, (GLenum mode, GLuint id, GLsizei instancecount))

#define DRX_GL_FUNCTIONS_GL_EXT_external_buffer \
    X (z0        , glBufferStorageExternalEXT, (GLenum target, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags)) \
    X (z0        , glNamedBufferStorageExternalEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags))

#define DRX_GL_FUNCTIONS_GL_EXT_fragment_shading_rate \
    X (z0        , glGetFragmentShadingRatesEXT, (GLsizei samples, GLsizei maxCount, GLsizei *count, GLenum *shadingRates)) \
    X (z0        , glShadingRateEXT, (GLenum rate)) \
    X (z0        , glShadingRateCombinerOpsEXT, (GLenum combinerOp0, GLenum combinerOp1)) \
    X (z0        , glFramebufferShadingRateEXT, (GLenum target, GLenum attachment, GLuint texture, GLint baseLayer, GLsizei numLayers, GLsizei texelWidth, GLsizei texelHeight))

#define DRX_GL_FUNCTIONS_GL_EXT_framebuffer_blit_layers \
    X (z0        , glBlitFramebufferLayersEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (z0        , glBlitFramebufferLayerEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint srcLayer, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLint dstLayer, GLbitfield mask, GLenum filter))

#define DRX_GL_FUNCTIONS_GL_EXT_geometry_shader \
    X (z0        , glFramebufferTextureEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level))

#define DRX_GL_FUNCTIONS_GL_EXT_instanced_arrays \
    X (z0        , glVertexAttribDivisorEXT, (GLuint index, GLuint divisor))

#define DRX_GL_FUNCTIONS_GL_EXT_map_buffer_range \
    X (uk       , glMapBufferRangeEXT, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (z0        , glFlushMappedBufferRangeEXT, (GLenum target, GLintptr offset, GLsizeiptr length))

#define DRX_GL_FUNCTIONS_GL_EXT_memory_object \
    X (z0        , glGetUnsignedBytevEXT, (GLenum pname, GLubyte *data)) \
    X (z0        , glGetUnsignedBytei_vEXT, (GLenum target, GLuint index, GLubyte *data)) \
    X (z0        , glDeleteMemoryObjectsEXT, (GLsizei n, const GLuint *memoryObjects)) \
    X (GLboolean   , glIsMemoryObjectEXT, (GLuint memoryObject)) \
    X (z0        , glCreateMemoryObjectsEXT, (GLsizei n, GLuint *memoryObjects)) \
    X (z0        , glMemoryObjectParameterivEXT, (GLuint memoryObject, GLenum pname, const GLint *params)) \
    X (z0        , glGetMemoryObjectParameterivEXT, (GLuint memoryObject, GLenum pname, GLint *params)) \
    X (z0        , glTexStorageMem2DEXT, (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset)) \
    X (z0        , glTexStorageMem2DMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (z0        , glTexStorageMem3DEXT, (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset)) \
    X (z0        , glTexStorageMem3DMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (z0        , glBufferStorageMemEXT, (GLenum target, GLsizeiptr size, GLuint memory, GLuint64 offset)) \
    X (z0        , glTextureStorageMem2DEXT, (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset)) \
    X (z0        , glTextureStorageMem2DMultisampleEXT, (GLuint texture, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (z0        , glTextureStorageMem3DEXT, (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset)) \
    X (z0        , glTextureStorageMem3DMultisampleEXT, (GLuint texture, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (z0        , glNamedBufferStorageMemEXT, (GLuint buffer, GLsizeiptr size, GLuint memory, GLuint64 offset)) \
    X (z0        , glTexStorageMem1DEXT, (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLuint memory, GLuint64 offset)) \
    X (z0        , glTextureStorageMem1DEXT, (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLuint memory, GLuint64 offset))

#define DRX_GL_FUNCTIONS_GL_EXT_memory_object_fd \
    X (z0        , glImportMemoryFdEXT, (GLuint memory, GLuint64 size, GLenum handleType, GLint fd))

#define DRX_GL_FUNCTIONS_GL_EXT_memory_object_win32 \
    X (z0        , glImportMemoryWin32HandleEXT, (GLuint memory, GLuint64 size, GLenum handleType, uk handle)) \
    X (z0        , glImportMemoryWin32NameEXT, (GLuint memory, GLuint64 size, GLenum handleType, ukk name))

#define DRX_GL_FUNCTIONS_GL_EXT_multi_draw_arrays \
    X (z0        , glMultiDrawArraysEXT, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)) \
    X (z0        , glMultiDrawElementsEXT, (GLenum mode, const GLsizei *count, GLenum type, ukk const*indices, GLsizei primcount))

#define DRX_GL_FUNCTIONS_GL_EXT_multi_draw_indirect \
    X (z0        , glMultiDrawArraysIndirectEXT, (GLenum mode, ukk indirect, GLsizei drawcount, GLsizei stride)) \
    X (z0        , glMultiDrawElementsIndirectEXT, (GLenum mode, GLenum type, ukk indirect, GLsizei drawcount, GLsizei stride))

#define DRX_GL_FUNCTIONS_GL_EXT_multisampled_render_to_texture \
    X (z0        , glRenderbufferStorageMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glFramebufferTexture2DMultisampleEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples))

#define DRX_GL_FUNCTIONS_GL_EXT_multiview_draw_buffers \
    X (z0        , glReadBufferIndexedEXT, (GLenum src, GLint index)) \
    X (z0        , glDrawBuffersIndexedEXT, (GLint n, const GLenum *location, const GLint *indices)) \
    X (z0        , glGetIntegeri_vEXT, (GLenum target, GLuint index, GLint *data))

#define DRX_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    X (z0        , glPolygonOffsetClampEXT, (GLfloat factor, GLfloat units, GLfloat clamp))

#define DRX_GL_FUNCTIONS_GL_EXT_primitive_bounding_box \
    X (z0        , glPrimitiveBoundingBoxEXT, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW))

#define DRX_GL_FUNCTIONS_GL_EXT_raster_multisample \
    X (z0        , glRasterSamplesEXT, (GLuint samples, GLboolean fixedsamplelocations))

#define DRX_GL_FUNCTIONS_GL_EXT_robustness \
    X (GLenum      , glGetGraphicsResetStatusEXT, ()) \
    X (z0        , glReadnPixelsEXT, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, uk data)) \
    X (z0        , glGetnUniformfvEXT, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (z0        , glGetnUniformivEXT, (GLuint program, GLint location, GLsizei bufSize, GLint *params))

#define DRX_GL_FUNCTIONS_GL_EXT_semaphore \
    X (z0        , glGenSemaphoresEXT, (GLsizei n, GLuint *semaphores)) \
    X (z0        , glDeleteSemaphoresEXT, (GLsizei n, const GLuint *semaphores)) \
    X (GLboolean   , glIsSemaphoreEXT, (GLuint semaphore)) \
    X (z0        , glSemaphoreParameterui64vEXT, (GLuint semaphore, GLenum pname, const GLuint64 *params)) \
    X (z0        , glGetSemaphoreParameterui64vEXT, (GLuint semaphore, GLenum pname, GLuint64 *params)) \
    X (z0        , glWaitSemaphoreEXT, (GLuint semaphore, GLuint numBufferBarriers, const GLuint *buffers, GLuint numTextureBarriers, const GLuint *textures, const GLenum *srcLayouts)) \
    X (z0        , glSignalSemaphoreEXT, (GLuint semaphore, GLuint numBufferBarriers, const GLuint *buffers, GLuint numTextureBarriers, const GLuint *textures, const GLenum *dstLayouts))

#define DRX_GL_FUNCTIONS_GL_EXT_semaphore_fd \
    X (z0        , glImportSemaphoreFdEXT, (GLuint semaphore, GLenum handleType, GLint fd))

#define DRX_GL_FUNCTIONS_GL_EXT_semaphore_win32 \
    X (z0        , glImportSemaphoreWin32HandleEXT, (GLuint semaphore, GLenum handleType, uk handle)) \
    X (z0        , glImportSemaphoreWin32NameEXT, (GLuint semaphore, GLenum handleType, ukk name))

#define DRX_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    X (z0        , glUseShaderProgramEXT, (GLenum type, GLuint program)) \
    X (z0        , glActiveProgramEXT, (GLuint program)) \
    X (GLuint      , glCreateShaderProgramEXT, (GLenum type, const GLchar *string)) \
    X (z0        , glActiveShaderProgramEXT, (GLuint pipeline, GLuint program)) \
    X (z0        , glBindProgramPipelineEXT, (GLuint pipeline)) \
    X (GLuint      , glCreateShaderProgramvEXT, (GLenum type, GLsizei count, const GLchar *const*strings)) \
    X (z0        , glDeleteProgramPipelinesEXT, (GLsizei n, const GLuint *pipelines)) \
    X (z0        , glGenProgramPipelinesEXT, (GLsizei n, GLuint *pipelines)) \
    X (z0        , glGetProgramPipelineInfoLogEXT, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glGetProgramPipelineivEXT, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsProgramPipelineEXT, (GLuint pipeline)) \
    X (z0        , glProgramParameteriEXT, (GLuint program, GLenum pname, GLint value)) \
    X (z0        , glProgramUniform1fEXT, (GLuint program, GLint location, GLfloat v0)) \
    X (z0        , glProgramUniform1fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform1iEXT, (GLuint program, GLint location, GLint v0)) \
    X (z0        , glProgramUniform1ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform2fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glProgramUniform2fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform2iEXT, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (z0        , glProgramUniform2ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform3fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glProgramUniform3fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform3iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glProgramUniform3ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform4fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glProgramUniform4fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform4iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glProgramUniform4ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniformMatrix2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUseProgramStagesEXT, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (z0        , glValidateProgramPipelineEXT, (GLuint pipeline)) \
    X (z0        , glProgramUniform1uiEXT, (GLuint program, GLint location, GLuint v0)) \
    X (z0        , glProgramUniform2uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glProgramUniform3uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glProgramUniform4uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glProgramUniform1uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform2uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform3uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform4uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniformMatrix2x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

#define DRX_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    X (z0        , glFramebufferFetchBarrierEXT, ())

#define DRX_GL_FUNCTIONS_GL_EXT_shader_pixel_local_storage2 \
    X (z0        , glFramebufferPixelLocalStorageSizeEXT, (GLuint target, GLsizei size)) \
    X (GLsizei     , glGetFramebufferPixelLocalStorageSizeEXT, (GLuint target)) \
    X (z0        , glClearPixelLocalStorageuiEXT, (GLsizei offset, GLsizei n, const GLuint *values))

#define DRX_GL_FUNCTIONS_GL_EXT_sparse_texture \
    X (z0        , glTexPageCommitmentEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit))

#define DRX_GL_FUNCTIONS_GL_EXT_tessellation_shader \
    X (z0        , glPatchParameteriEXT, (GLenum pname, GLint value))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_border_clamp \
    X (z0        , glTexParameterIivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexParameterIuivEXT, (GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetTexParameterIivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexParameterIuivEXT, (GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glSamplerParameterIivEXT, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (z0        , glSamplerParameterIuivEXT, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (z0        , glGetSamplerParameterIivEXT, (GLuint sampler, GLenum pname, GLint *params)) \
    X (z0        , glGetSamplerParameterIuivEXT, (GLuint sampler, GLenum pname, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_buffer \
    X (z0        , glTexBufferEXT, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (z0        , glTexBufferRangeEXT, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_storage \
    X (z0        , glTexStorage1DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (z0        , glTexStorage2DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTexStorage3DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glTextureStorage1DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (z0        , glTextureStorage2DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTextureStorage3DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_storage_compression \
    X (z0        , glTexStorageAttribs2DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, const GLint* attrib_list)) \
    X (z0        , glTexStorageAttribs3DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, const GLint* attrib_list))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_view \
    X (z0        , glTextureViewEXT, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers))

#define DRX_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    X (z0        , glCreateSemaphoresNV, (GLsizei n, GLuint *semaphores)) \
    X (z0        , glSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, const GLint *params)) \
    X (z0        , glGetSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    X (GLboolean   , glAcquireKeyedMutexWin32EXT, (GLuint memory, GLuint64 key, GLuint timeout)) \
    X (GLboolean   , glReleaseKeyedMutexWin32EXT, (GLuint memory, GLuint64 key))

#define DRX_GL_FUNCTIONS_GL_EXT_window_rectangles \
    X (z0        , glWindowRectanglesEXT, (GLenum mode, GLsizei count, const GLint *box))

#define DRX_GL_FUNCTIONS_GL_IMG_bindless_texture \
    X (GLuint64    , glGetTextureHandleIMG, (GLuint texture)) \
    X (GLuint64    , glGetTextureSamplerHandleIMG, (GLuint texture, GLuint sampler)) \
    X (z0        , glUniformHandleui64IMG, (GLint location, GLuint64 value)) \
    X (z0        , glUniformHandleui64vIMG, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glProgramUniformHandleui64IMG, (GLuint program, GLint location, GLuint64 value)) \
    X (z0        , glProgramUniformHandleui64vIMG, (GLuint program, GLint location, GLsizei count, const GLuint64 *values))

#define DRX_GL_FUNCTIONS_GL_IMG_framebuffer_downsample \
    X (z0        , glFramebufferTexture2DDownsampleIMG, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint xscale, GLint yscale)) \
    X (z0        , glFramebufferTextureLayerDownsampleIMG, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer, GLint xscale, GLint yscale))

#define DRX_GL_FUNCTIONS_GL_IMG_multisampled_render_to_texture \
    X (z0        , glRenderbufferStorageMultisampleIMG, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glFramebufferTexture2DMultisampleIMG, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples))

#define DRX_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    X (z0        , glApplyFramebufferAttachmentCMAAINTEL, ())

#define DRX_GL_FUNCTIONS_GL_INTEL_performance_query \
    X (z0        , glBeginPerfQueryINTEL, (GLuint queryHandle)) \
    X (z0        , glCreatePerfQueryINTEL, (GLuint queryId, GLuint *queryHandle)) \
    X (z0        , glDeletePerfQueryINTEL, (GLuint queryHandle)) \
    X (z0        , glEndPerfQueryINTEL, (GLuint queryHandle)) \
    X (z0        , glGetFirstPerfQueryIdINTEL, (GLuint *queryId)) \
    X (z0        , glGetNextPerfQueryIdINTEL, (GLuint queryId, GLuint *nextQueryId)) \
    X (z0        , glGetPerfCounterInfoINTEL, (GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar *counterName, GLuint counterDescLength, GLchar *counterDesc, GLuint *counterOffset, GLuint *counterDataSize, GLuint *counterTypeEnum, GLuint *counterDataTypeEnum, GLuint64 *rawCounterMaxValue)) \
    X (z0        , glGetPerfQueryDataINTEL, (GLuint queryHandle, GLuint flags, GLsizei dataSize, uk data, GLuint *bytesWritten)) \
    X (z0        , glGetPerfQueryIdByNameINTEL, (GLchar *queryName, GLuint *queryId)) \
    X (z0        , glGetPerfQueryInfoINTEL, (GLuint queryId, GLuint queryNameLength, GLchar *queryName, GLuint *dataSize, GLuint *noCounters, GLuint *noInstances, GLuint *capsMask))

#define DRX_GL_FUNCTIONS_GL_KHR_blend_equation_advanced \
    X (z0        , glBlendBarrierKHR, ())

#define DRX_GL_FUNCTIONS_GL_KHR_debug \
    X (z0        , glDebugMessageControlKHR, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (z0        , glDebugMessageInsertKHR, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (z0        , glDebugMessageCallbackKHR, (GLDEBUGPROCKHR callback, ukk userParam)) \
    X (GLuint      , glGetDebugMessageLogKHR, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)) \
    X (z0        , glPushDebugGroupKHR, (GLenum source, GLuint id, GLsizei length, const GLchar *message)) \
    X (z0        , glPopDebugGroupKHR, ()) \
    X (z0        , glObjectLabelKHR, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectLabelKHR, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (z0        , glObjectPtrLabelKHR, (ukk ptr, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectPtrLabelKHR, (ukk ptr, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (z0        , glGetPointervKHR, (GLenum pname, uk *params))

#define DRX_GL_FUNCTIONS_GL_KHR_robustness \
    X (GLenum      , glGetGraphicsResetStatusKHR, ()) \
    X (z0        , glReadnPixelsKHR, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, uk data)) \
    X (z0        , glGetnUniformfvKHR, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (z0        , glGetnUniformivKHR, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (z0        , glGetnUniformuivKHR, (GLuint program, GLint location, GLsizei bufSize, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_KHR_parallel_shader_compile \
    X (z0        , glMaxShaderCompilerThreadsKHR, (GLuint count))

#define DRX_GL_FUNCTIONS_GL_MESA_framebuffer_flip_y \
    X (z0        , glFramebufferParameteriMESA, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glGetFramebufferParameterivMESA, (GLenum target, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_NV_bindless_texture \
    X (GLuint64    , glGetTextureHandleNV, (GLuint texture)) \
    X (GLuint64    , glGetTextureSamplerHandleNV, (GLuint texture, GLuint sampler)) \
    X (z0        , glMakeTextureHandleResidentNV, (GLuint64 handle)) \
    X (z0        , glMakeTextureHandleNonResidentNV, (GLuint64 handle)) \
    X (GLuint64    , glGetImageHandleNV, (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)) \
    X (z0        , glMakeImageHandleResidentNV, (GLuint64 handle, GLenum access)) \
    X (z0        , glMakeImageHandleNonResidentNV, (GLuint64 handle)) \
    X (z0        , glUniformHandleui64NV, (GLint location, GLuint64 value)) \
    X (z0        , glUniformHandleui64vNV, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glProgramUniformHandleui64NV, (GLuint program, GLint location, GLuint64 value)) \
    X (z0        , glProgramUniformHandleui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64 *values)) \
    X (GLboolean   , glIsTextureHandleResidentNV, (GLuint64 handle)) \
    X (GLboolean   , glIsImageHandleResidentNV, (GLuint64 handle))

#define DRX_GL_FUNCTIONS_GL_NV_blend_equation_advanced \
    X (z0        , glBlendParameteriNV, (GLenum pname, GLint value)) \
    X (z0        , glBlendBarrierNV, ())

#define DRX_GL_FUNCTIONS_GL_NV_clip_space_w_scaling \
    X (z0        , glViewportPositionWScaleNV, (GLuint index, GLfloat xcoeff, GLfloat ycoeff))

#define DRX_GL_FUNCTIONS_GL_NV_conditional_render \
    X (z0        , glBeginConditionalRenderNV, (GLuint id, GLenum mode)) \
    X (z0        , glEndConditionalRenderNV, ())

#define DRX_GL_FUNCTIONS_GL_NV_conservative_raster \
    X (z0        , glSubpixelPrecisionBiasNV, (GLuint xbits, GLuint ybits))

#define DRX_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    X (z0        , glConservativeRasterParameteriNV, (GLenum pname, GLint param))

#define DRX_GL_FUNCTIONS_GL_NV_copy_buffer \
    X (z0        , glCopyBufferSubDataNV, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size))

#define DRX_GL_FUNCTIONS_GL_NV_coverage_sample \
    X (z0        , glCoverageMaskNV, (GLboolean mask)) \
    X (z0        , glCoverageOperationNV, (GLenum operation))

#define DRX_GL_FUNCTIONS_GL_NV_draw_buffers \
    X (z0        , glDrawBuffersNV, (GLsizei n, const GLenum *bufs))

#define DRX_GL_FUNCTIONS_GL_NV_draw_instanced \
    X (z0        , glDrawArraysInstancedNV, (GLenum mode, GLint first, GLsizei count, GLsizei primcount)) \
    X (z0        , glDrawElementsInstancedNV, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei primcount))

#define DRX_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    X (z0        , glDrawVkImageNV, (GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1)) \
    X (GLVULKANPROCNV, glGetVkProcAddrNV, (const GLchar *name)) \
    X (z0        , glWaitVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (z0        , glSignalVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (z0        , glSignalVkFenceNV, (GLuint64 vkFence))

#define DRX_GL_FUNCTIONS_GL_NV_fence \
    X (z0        , glDeleteFencesNV, (GLsizei n, const GLuint *fences)) \
    X (z0        , glGenFencesNV, (GLsizei n, GLuint *fences)) \
    X (GLboolean   , glIsFenceNV, (GLuint fence)) \
    X (GLboolean   , glTestFenceNV, (GLuint fence)) \
    X (z0        , glGetFenceivNV, (GLuint fence, GLenum pname, GLint *params)) \
    X (z0        , glFinishFenceNV, (GLuint fence)) \
    X (z0        , glSetFenceNV, (GLuint fence, GLenum condition))

#define DRX_GL_FUNCTIONS_GL_NV_fragment_coverage_to_color \
    X (z0        , glFragmentCoverageColorNV, (GLuint color))

#define DRX_GL_FUNCTIONS_GL_NV_framebuffer_blit \
    X (z0        , glBlitFramebufferNV, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

#define DRX_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    X (z0        , glCoverageModulationTableNV, (GLsizei n, const GLfloat *v)) \
    X (z0        , glGetCoverageModulationTableNV, (GLsizei bufSize, GLfloat *v)) \
    X (z0        , glCoverageModulationNV, (GLenum components))

#define DRX_GL_FUNCTIONS_GL_NV_framebuffer_multisample \
    X (z0        , glRenderbufferStorageMultisampleNV, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_NV_gpu_shader5 \
    X (z0        , glUniform1i64NV, (GLint location, GLint64EXT x)) \
    X (z0        , glUniform2i64NV, (GLint location, GLint64EXT x, GLint64EXT y)) \
    X (z0        , glUniform3i64NV, (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)) \
    X (z0        , glUniform4i64NV, (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)) \
    X (z0        , glUniform1i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glUniform2i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glUniform3i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glUniform4i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glUniform1ui64NV, (GLint location, GLuint64EXT x)) \
    X (z0        , glUniform2ui64NV, (GLint location, GLuint64EXT x, GLuint64EXT y)) \
    X (z0        , glUniform3ui64NV, (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)) \
    X (z0        , glUniform4ui64NV, (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)) \
    X (z0        , glUniform1ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glUniform2ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glUniform3ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glUniform4ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glGetUniformi64vNV, (GLuint program, GLint location, GLint64EXT *params)) \
    X (z0        , glProgramUniform1i64NV, (GLuint program, GLint location, GLint64EXT x)) \
    X (z0        , glProgramUniform2i64NV, (GLuint program, GLint location, GLint64EXT x, GLint64EXT y)) \
    X (z0        , glProgramUniform3i64NV, (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)) \
    X (z0        , glProgramUniform4i64NV, (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)) \
    X (z0        , glProgramUniform1i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glProgramUniform2i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glProgramUniform3i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glProgramUniform4i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (z0        , glProgramUniform1ui64NV, (GLuint program, GLint location, GLuint64EXT x)) \
    X (z0        , glProgramUniform2ui64NV, (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y)) \
    X (z0        , glProgramUniform3ui64NV, (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)) \
    X (z0        , glProgramUniform4ui64NV, (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)) \
    X (z0        , glProgramUniform1ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glProgramUniform2ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glProgramUniform3ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glProgramUniform4ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value))

#define DRX_GL_FUNCTIONS_GL_NV_instanced_arrays \
    X (z0        , glVertexAttribDivisorNV, (GLuint index, GLuint divisor))

#define DRX_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    X (z0        , glGetInternalformatSampleivNV, (GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint *params))

#define DRX_GL_FUNCTIONS_GL_NV_memory_attachment \
    X (z0        , glGetMemoryObjectDetachedResourcesuivNV, (GLuint memory, GLenum pname, GLint first, GLsizei count, GLuint *params)) \
    X (z0        , glResetMemoryObjectParameterNV, (GLuint memory, GLenum pname)) \
    X (z0        , glTexAttachMemoryNV, (GLenum target, GLuint memory, GLuint64 offset)) \
    X (z0        , glBufferAttachMemoryNV, (GLenum target, GLuint memory, GLuint64 offset)) \
    X (z0        , glTextureAttachMemoryNV, (GLuint texture, GLuint memory, GLuint64 offset)) \
    X (z0        , glNamedBufferAttachMemoryNV, (GLuint buffer, GLuint memory, GLuint64 offset))

#define DRX_GL_FUNCTIONS_GL_NV_memory_object_sparse \
    X (z0        , glBufferPageCommitmentMemNV, (GLenum target, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit)) \
    X (z0        , glTexPageCommitmentMemNV, (GLenum target, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit)) \
    X (z0        , glNamedBufferPageCommitmentMemNV, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit)) \
    X (z0        , glTexturePageCommitmentMemNV, (GLuint texture, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit))

#define DRX_GL_FUNCTIONS_GL_NV_mesh_shader \
    X (z0        , glDrawMeshTasksNV, (GLuint first, GLuint count)) \
    X (z0        , glDrawMeshTasksIndirectNV, (GLintptr indirect)) \
    X (z0        , glMultiDrawMeshTasksIndirectNV, (GLintptr indirect, GLsizei drawcount, GLsizei stride)) \
    X (z0        , glMultiDrawMeshTasksIndirectCountNV, (GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride))

#define DRX_GL_FUNCTIONS_GL_NV_non_square_matrices \
    X (z0        , glUniformMatrix2x3fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3x2fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix2x4fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4x2fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3x4fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4x3fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

#define DRX_GL_FUNCTIONS_GL_NV_path_rendering \
    X (GLuint      , glGenPathsNV, (GLsizei range)) \
    X (z0        , glDeletePathsNV, (GLuint path, GLsizei range)) \
    X (GLboolean   , glIsPathNV, (GLuint path)) \
    X (z0        , glPathCommandsNV, (GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, ukk coords)) \
    X (z0        , glPathCoordsNV, (GLuint path, GLsizei numCoords, GLenum coordType, ukk coords)) \
    X (z0        , glPathSubCommandsNV, (GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, ukk coords)) \
    X (z0        , glPathSubCoordsNV, (GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, ukk coords)) \
    X (z0        , glPathStringNV, (GLuint path, GLenum format, GLsizei length, ukk pathString)) \
    X (z0        , glPathGlyphsNV, (GLuint firstPathName, GLenum fontTarget, ukk fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, ukk charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (z0        , glPathGlyphRangeNV, (GLuint firstPathName, GLenum fontTarget, ukk fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (z0        , glWeightPathsNV, (GLuint resultPath, GLsizei numPaths, const GLuint *paths, const GLfloat *weights)) \
    X (z0        , glCopyPathNV, (GLuint resultPath, GLuint srcPath)) \
    X (z0        , glInterpolatePathsNV, (GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight)) \
    X (z0        , glTransformPathNV, (GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat *transformValues)) \
    X (z0        , glPathParameterivNV, (GLuint path, GLenum pname, const GLint *value)) \
    X (z0        , glPathParameteriNV, (GLuint path, GLenum pname, GLint value)) \
    X (z0        , glPathParameterfvNV, (GLuint path, GLenum pname, const GLfloat *value)) \
    X (z0        , glPathParameterfNV, (GLuint path, GLenum pname, GLfloat value)) \
    X (z0        , glPathDashArrayNV, (GLuint path, GLsizei dashCount, const GLfloat *dashArray)) \
    X (z0        , glPathStencilFuncNV, (GLenum func, GLint ref, GLuint mask)) \
    X (z0        , glPathStencilDepthOffsetNV, (GLfloat factor, GLfloat units)) \
    X (z0        , glStencilFillPathNV, (GLuint path, GLenum fillMode, GLuint mask)) \
    X (z0        , glStencilStrokePathNV, (GLuint path, GLint reference, GLuint mask)) \
    X (z0        , glStencilFillPathInstancedNV, (GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues)) \
    X (z0        , glStencilStrokePathInstancedNV, (GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat *transformValues)) \
    X (z0        , glPathCoverDepthFuncNV, (GLenum func)) \
    X (z0        , glCoverFillPathNV, (GLuint path, GLenum coverMode)) \
    X (z0        , glCoverStrokePathNV, (GLuint path, GLenum coverMode)) \
    X (z0        , glCoverFillPathInstancedNV, (GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (z0        , glCoverStrokePathInstancedNV, (GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (z0        , glGetPathParameterivNV, (GLuint path, GLenum pname, GLint *value)) \
    X (z0        , glGetPathParameterfvNV, (GLuint path, GLenum pname, GLfloat *value)) \
    X (z0        , glGetPathCommandsNV, (GLuint path, GLubyte *commands)) \
    X (z0        , glGetPathCoordsNV, (GLuint path, GLfloat *coords)) \
    X (z0        , glGetPathDashArrayNV, (GLuint path, GLfloat *dashArray)) \
    X (z0        , glGetPathMetricsNV, (GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLsizei stride, GLfloat *metrics)) \
    X (z0        , glGetPathMetricRangeNV, (GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat *metrics)) \
    X (z0        , glGetPathSpacingNV, (GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat *returnedSpacing)) \
    X (GLboolean   , glIsPointInFillPathNV, (GLuint path, GLuint mask, GLfloat x, GLfloat y)) \
    X (GLboolean   , glIsPointInStrokePathNV, (GLuint path, GLfloat x, GLfloat y)) \
    X (GLfloat     , glGetPathLengthNV, (GLuint path, GLsizei startSegment, GLsizei numSegments)) \
    X (GLboolean   , glPointAlongPathNV, (GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat *x, GLfloat *y, GLfloat *tangentX, GLfloat *tangentY)) \
    X (z0        , glMatrixLoad3x2fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (z0        , glMatrixLoad3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (z0        , glMatrixLoadTranspose3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (z0        , glMatrixMult3x2fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (z0        , glMatrixMult3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (z0        , glMatrixMultTranspose3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (z0        , glStencilThenCoverFillPathNV, (GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode)) \
    X (z0        , glStencilThenCoverStrokePathNV, (GLuint path, GLint reference, GLuint mask, GLenum coverMode)) \
    X (z0        , glStencilThenCoverFillPathInstancedNV, (GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (z0        , glStencilThenCoverStrokePathInstancedNV, (GLsizei numPaths, GLenum pathNameType, ukk paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (GLenum      , glPathGlyphIndexRangeNV, (GLenum fontTarget, ukk fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint *baseAndCount)) \
    X (GLenum      , glPathGlyphIndexArrayNV, (GLuint firstPathName, GLenum fontTarget, ukk fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (GLenum      , glPathMemoryGlyphIndexArrayNV, (GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, ukk fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (z0        , glProgramPathFragmentInputGenNV, (GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat *coeffs)) \
    X (z0        , glGetProgramResourcefvNV, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLfloat *params)) \
    X (z0        , glPathColorGenNV, (GLenum color, GLenum genMode, GLenum colorFormat, const GLfloat *coeffs)) \
    X (z0        , glPathTexGenNV, (GLenum texCoordSet, GLenum genMode, GLint components, const GLfloat *coeffs)) \
    X (z0        , glPathFogGenNV, (GLenum genMode)) \
    X (z0        , glGetPathColorGenivNV, (GLenum color, GLenum pname, GLint *value)) \
    X (z0        , glGetPathColorGenfvNV, (GLenum color, GLenum pname, GLfloat *value)) \
    X (z0        , glGetPathTexGenivNV, (GLenum texCoordSet, GLenum pname, GLint *value)) \
    X (z0        , glGetPathTexGenfvNV, (GLenum texCoordSet, GLenum pname, GLfloat *value)) \
    X (z0        , glMatrixFrustumEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (z0        , glMatrixLoadIdentityEXT, (GLenum mode)) \
    X (z0        , glMatrixLoadTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixLoadTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixLoadfEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixLoaddEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixMultTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixMultTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixMultfEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixMultdEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixOrthoEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (z0        , glMatrixPopEXT, (GLenum mode)) \
    X (z0        , glMatrixPushEXT, (GLenum mode)) \
    X (z0        , glMatrixRotatefEXT, (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glMatrixRotatedEXT, (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glMatrixScalefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glMatrixScaledEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glMatrixTranslatefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glMatrixTranslatedEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z))

#define DRX_GL_FUNCTIONS_GL_NV_polygon_mode \
    X (z0        , glPolygonModeNV, (GLenum face, GLenum mode))

#define DRX_GL_FUNCTIONS_GL_NV_read_buffer \
    X (z0        , glReadBufferNV, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_NV_sample_locations \
    X (z0        , glFramebufferSampleLocationsfvNV, (GLenum target, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glNamedFramebufferSampleLocationsfvNV, (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glResolveDepthValuesNV, ())

#define DRX_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    X (z0        , glScissorExclusiveNV, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glScissorExclusiveArrayvNV, (GLuint first, GLsizei count, const GLint *v))

#define DRX_GL_FUNCTIONS_GL_NV_shading_rate_image \
    X (z0        , glBindShadingRateImageNV, (GLuint texture)) \
    X (z0        , glGetShadingRateImagePaletteNV, (GLuint viewport, GLuint entry, GLenum *rate)) \
    X (z0        , glGetShadingRateSampleLocationivNV, (GLenum rate, GLuint samples, GLuint index, GLint *location)) \
    X (z0        , glShadingRateImageBarrierNV, (GLboolean synchronize)) \
    X (z0        , glShadingRateImagePaletteNV, (GLuint viewport, GLuint first, GLsizei count, const GLenum *rates)) \
    X (z0        , glShadingRateSampleOrderNV, (GLenum order)) \
    X (z0        , glShadingRateSampleOrderCustomNV, (GLenum rate, GLuint samples, const GLint *locations))

#define DRX_GL_FUNCTIONS_GL_NV_viewport_array \
    X (z0        , glViewportArrayvNV, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (z0        , glViewportIndexedfNV, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)) \
    X (z0        , glViewportIndexedfvNV, (GLuint index, const GLfloat *v)) \
    X (z0        , glScissorArrayvNV, (GLuint first, GLsizei count, const GLint *v)) \
    X (z0        , glScissorIndexedNV, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)) \
    X (z0        , glScissorIndexedvNV, (GLuint index, const GLint *v)) \
    X (z0        , glDepthRangeArrayfvNV, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (z0        , glDepthRangeIndexedfNV, (GLuint index, GLfloat n, GLfloat f)) \
    X (z0        , glGetFloati_vNV, (GLenum target, GLuint index, GLfloat *data)) \
    X (z0        , glEnableiNV, (GLenum target, GLuint index)) \
    X (z0        , glDisableiNV, (GLenum target, GLuint index)) \
    X (GLboolean   , glIsEnablediNV, (GLenum target, GLuint index))

#define DRX_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    X (z0        , glViewportSwizzleNV, (GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew))

#define DRX_GL_FUNCTIONS_GL_OES_EGL_image \
    X (z0        , glEGLImageTargetTexture2DOES, (GLenum target, GLeglImageOES image)) \
    X (z0        , glEGLImageTargetRenderbufferStorageOES, (GLenum target, GLeglImageOES image))

#define DRX_GL_FUNCTIONS_GL_OES_copy_image \
    X (z0        , glCopyImageSubDataOES, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth))

#define DRX_GL_FUNCTIONS_GL_OES_draw_buffers_indexed \
    X (z0        , glEnableiOES, (GLenum target, GLuint index)) \
    X (z0        , glDisableiOES, (GLenum target, GLuint index)) \
    X (z0        , glBlendEquationiOES, (GLuint buf, GLenum mode)) \
    X (z0        , glBlendEquationSeparateiOES, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glBlendFunciOES, (GLuint buf, GLenum src, GLenum dst)) \
    X (z0        , glBlendFuncSeparateiOES, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (z0        , glColorMaskiOES, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (GLboolean   , glIsEnablediOES, (GLenum target, GLuint index))

#define DRX_GL_FUNCTIONS_GL_OES_draw_elements_base_vertex \
    X (z0        , glDrawElementsBaseVertexOES, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawRangeElementsBaseVertexOES, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawElementsInstancedBaseVertexOES, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLint basevertex))

#define DRX_GL_FUNCTIONS_GL_OES_geometry_shader \
    X (z0        , glFramebufferTextureOES, (GLenum target, GLenum attachment, GLuint texture, GLint level))

#define DRX_GL_FUNCTIONS_GL_OES_get_program_binary \
    X (z0        , glGetProgramBinaryOES, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, uk binary)) \
    X (z0        , glProgramBinaryOES, (GLuint program, GLenum binaryFormat, ukk binary, GLint length))

#define DRX_GL_FUNCTIONS_GL_OES_mapbuffer \
    X (uk       , glMapBufferOES, (GLenum target, GLenum access)) \
    X (GLboolean   , glUnmapBufferOES, (GLenum target)) \
    X (z0        , glGetBufferPointervOES, (GLenum target, GLenum pname, uk *params))

#define DRX_GL_FUNCTIONS_GL_OES_primitive_bounding_box \
    X (z0        , glPrimitiveBoundingBoxOES, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW))

#define DRX_GL_FUNCTIONS_GL_OES_sample_shading \
    X (z0        , glMinSampleShadingOES, (GLfloat value))

#define DRX_GL_FUNCTIONS_GL_OES_tessellation_shader \
    X (z0        , glPatchParameteriOES, (GLenum pname, GLint value))

#define DRX_GL_FUNCTIONS_GL_OES_texture_3D \
    X (z0        , glTexImage3DOES, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glCompressedTexImage3DOES, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glFramebufferTexture3DOES, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))

#define DRX_GL_FUNCTIONS_GL_OES_texture_border_clamp \
    X (z0        , glTexParameterIivOES, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexParameterIuivOES, (GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetTexParameterIivOES, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexParameterIuivOES, (GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glSamplerParameterIivOES, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (z0        , glSamplerParameterIuivOES, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (z0        , glGetSamplerParameterIivOES, (GLuint sampler, GLenum pname, GLint *params)) \
    X (z0        , glGetSamplerParameterIuivOES, (GLuint sampler, GLenum pname, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_OES_texture_buffer \
    X (z0        , glTexBufferOES, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (z0        , glTexBufferRangeOES, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size))

#define DRX_GL_FUNCTIONS_GL_OES_texture_storage_multisample_2d_array \
    X (z0        , glTexStorage3DMultisampleOES, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))

#define DRX_GL_FUNCTIONS_GL_OES_texture_view \
    X (z0        , glTextureViewOES, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers))

#define DRX_GL_FUNCTIONS_GL_OES_vertex_array_object \
    X (z0        , glBindVertexArrayOES, (GLuint array)) \
    X (z0        , glDeleteVertexArraysOES, (GLsizei n, const GLuint *arrays)) \
    X (z0        , glGenVertexArraysOES, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArrayOES, (GLuint array))

#define DRX_GL_FUNCTIONS_GL_OES_viewport_array \
    X (z0        , glViewportArrayvOES, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (z0        , glViewportIndexedfOES, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)) \
    X (z0        , glViewportIndexedfvOES, (GLuint index, const GLfloat *v)) \
    X (z0        , glScissorArrayvOES, (GLuint first, GLsizei count, const GLint *v)) \
    X (z0        , glScissorIndexedOES, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)) \
    X (z0        , glScissorIndexedvOES, (GLuint index, const GLint *v)) \
    X (z0        , glDepthRangeArrayfvOES, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (z0        , glDepthRangeIndexedfOES, (GLuint index, GLfloat n, GLfloat f)) \
    X (z0        , glGetFloati_vOES, (GLenum target, GLuint index, GLfloat *data))

#define DRX_GL_FUNCTIONS_GL_OVR_multiview \
    X (z0        , glFramebufferTextureMultiviewOVR, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews))

#define DRX_GL_FUNCTIONS_GL_OVR_multiview_multisampled_render_to_texture \
    X (z0        , glFramebufferTextureMultisampleMultiviewOVR, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei samples, GLint baseViewIndex, GLsizei numViews))

#define DRX_GL_FUNCTIONS_GL_QCOM_alpha_test \
    X (z0        , glAlphaFuncQCOM, (GLenum func, GLclampf ref))

#define DRX_GL_FUNCTIONS_GL_QCOM_driver_control \
    X (z0        , glGetDriverControlsQCOM, (GLint *num, GLsizei size, GLuint *driverControls)) \
    X (z0        , glGetDriverControlStringQCOM, (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString)) \
    X (z0        , glEnableDriverControlQCOM, (GLuint driverControl)) \
    X (z0        , glDisableDriverControlQCOM, (GLuint driverControl))

#define DRX_GL_FUNCTIONS_GL_QCOM_extended_get \
    X (z0        , glExtGetTexturesQCOM, (GLuint *textures, GLint maxTextures, GLint *numTextures)) \
    X (z0        , glExtGetBuffersQCOM, (GLuint *buffers, GLint maxBuffers, GLint *numBuffers)) \
    X (z0        , glExtGetRenderbuffersQCOM, (GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers)) \
    X (z0        , glExtGetFramebuffersQCOM, (GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers)) \
    X (z0        , glExtGetTexLevelParameterivQCOM, (GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params)) \
    X (z0        , glExtTexObjectStateOverrideiQCOM, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glExtGetTexSubImageQCOM, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, uk texels)) \
    X (z0        , glExtGetBufferPointervQCOM, (GLenum target, uk *params))

#define DRX_GL_FUNCTIONS_GL_QCOM_extended_get2 \
    X (z0        , glExtGetShadersQCOM, (GLuint *shaders, GLint maxShaders, GLint *numShaders)) \
    X (z0        , glExtGetProgramsQCOM, (GLuint *programs, GLint maxPrograms, GLint *numPrograms)) \
    X (GLboolean   , glExtIsProgramBinaryQCOM, (GLuint program)) \
    X (z0        , glExtGetProgramBinarySourceQCOM, (GLuint program, GLenum shadertype, GLchar *source, GLint *length))

#define DRX_GL_FUNCTIONS_GL_QCOM_framebuffer_foveated \
    X (z0        , glFramebufferFoveationConfigQCOM, (GLuint framebuffer, GLuint numLayers, GLuint focalPointsPerLayer, GLuint requestedFeatures, GLuint *providedFeatures)) \
    X (z0        , glFramebufferFoveationParametersQCOM, (GLuint framebuffer, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea))

#define DRX_GL_FUNCTIONS_GL_QCOM_motion_estimation \
    X (z0        , glTexEstimateMotionQCOM, (GLuint ref, GLuint target, GLuint output)) \
    X (z0        , glTexEstimateMotionRegionsQCOM, (GLuint ref, GLuint target, GLuint output, GLuint mask))

#define DRX_GL_FUNCTIONS_GL_QCOM_frame_extrapolation \
    X (z0        , glExtrapolateTex2DQCOM, (GLuint src1, GLuint src2, GLuint output, GLfloat scaleFactor))

#define DRX_GL_FUNCTIONS_GL_QCOM_texture_foveated \
    X (z0        , glTextureFoveationParametersQCOM, (GLuint texture, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea))

#define DRX_GL_FUNCTIONS_GL_QCOM_shader_framebuffer_fetch_noncoherent \
    X (z0        , glFramebufferFetchBarrierQCOM, ())

#define DRX_GL_FUNCTIONS_GL_QCOM_shading_rate \
    X (z0        , glShadingRateQCOM, (GLenum rate))

#define DRX_GL_FUNCTIONS_GL_QCOM_tiled_rendering \
    X (z0        , glStartTilingQCOM, (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask)) \
    X (z0        , glEndTilingQCOM, (GLbitfield preserveMask))


#if DRX_STATIC_LINK_GL_ES_VERSION_2_0
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0_STATIC DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0_STATIC
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0_DYNAMIC DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0
#endif

#if DRX_STATIC_LINK_GL_ES_VERSION_3_0
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0_STATIC DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0_STATIC
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0_DYNAMIC DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0
#endif

#if DRX_STATIC_LINK_GL_ES_VERSION_3_1
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1_STATIC DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1_STATIC
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1_DYNAMIC DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1
#endif

#if DRX_STATIC_LINK_GL_ES_VERSION_3_2
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2_STATIC DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2_STATIC
 #define DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2_DYNAMIC DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2
#endif


#define DRX_STATIC_GL_FUNCTIONS \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0_STATIC \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0_STATIC \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1_STATIC \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2_STATIC

#define DRX_DYNAMIC_GL_FUNCTIONS \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_2_0_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_3_0_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_3_1_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_ES_VERSION_3_2_DYNAMIC

#define DRX_EXTENSION_GL_FUNCTIONS \
    DRX_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    DRX_GL_FUNCTIONS_GL_AMD_performance_monitor \
    DRX_GL_FUNCTIONS_GL_ANGLE_framebuffer_blit \
    DRX_GL_FUNCTIONS_GL_ANGLE_framebuffer_multisample \
    DRX_GL_FUNCTIONS_GL_ANGLE_instanced_arrays \
    DRX_GL_FUNCTIONS_GL_ANGLE_translated_shader_source \
    DRX_GL_FUNCTIONS_GL_APPLE_copy_texture_levels \
    DRX_GL_FUNCTIONS_GL_APPLE_framebuffer_multisample \
    DRX_GL_FUNCTIONS_GL_APPLE_sync \
    DRX_GL_FUNCTIONS_GL_ARM_shader_core_properties \
    DRX_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    DRX_GL_FUNCTIONS_GL_EXT_base_instance \
    DRX_GL_FUNCTIONS_GL_EXT_blend_func_extended \
    DRX_GL_FUNCTIONS_GL_EXT_blend_minmax \
    DRX_GL_FUNCTIONS_GL_EXT_buffer_storage \
    DRX_GL_FUNCTIONS_GL_EXT_clear_texture \
    DRX_GL_FUNCTIONS_GL_EXT_clip_control \
    DRX_GL_FUNCTIONS_GL_EXT_copy_image \
    DRX_GL_FUNCTIONS_GL_EXT_debug_label \
    DRX_GL_FUNCTIONS_GL_EXT_debug_marker \
    DRX_GL_FUNCTIONS_GL_EXT_discard_framebuffer \
    DRX_GL_FUNCTIONS_GL_EXT_disjoint_timer_query \
    DRX_GL_FUNCTIONS_GL_EXT_draw_buffers \
    DRX_GL_FUNCTIONS_GL_EXT_draw_buffers_indexed \
    DRX_GL_FUNCTIONS_GL_EXT_draw_elements_base_vertex \
    DRX_GL_FUNCTIONS_GL_EXT_draw_instanced \
    DRX_GL_FUNCTIONS_GL_EXT_draw_transform_feedback \
    DRX_GL_FUNCTIONS_GL_EXT_external_buffer \
    DRX_GL_FUNCTIONS_GL_EXT_fragment_shading_rate \
    DRX_GL_FUNCTIONS_GL_EXT_framebuffer_blit_layers \
    DRX_GL_FUNCTIONS_GL_EXT_geometry_shader \
    DRX_GL_FUNCTIONS_GL_EXT_instanced_arrays \
    DRX_GL_FUNCTIONS_GL_EXT_map_buffer_range \
    DRX_GL_FUNCTIONS_GL_EXT_memory_object \
    DRX_GL_FUNCTIONS_GL_EXT_memory_object_fd \
    DRX_GL_FUNCTIONS_GL_EXT_memory_object_win32 \
    DRX_GL_FUNCTIONS_GL_EXT_multi_draw_arrays \
    DRX_GL_FUNCTIONS_GL_EXT_multi_draw_indirect \
    DRX_GL_FUNCTIONS_GL_EXT_multisampled_render_to_texture \
    DRX_GL_FUNCTIONS_GL_EXT_multiview_draw_buffers \
    DRX_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    DRX_GL_FUNCTIONS_GL_EXT_primitive_bounding_box \
    DRX_GL_FUNCTIONS_GL_EXT_raster_multisample \
    DRX_GL_FUNCTIONS_GL_EXT_robustness \
    DRX_GL_FUNCTIONS_GL_EXT_semaphore \
    DRX_GL_FUNCTIONS_GL_EXT_semaphore_fd \
    DRX_GL_FUNCTIONS_GL_EXT_semaphore_win32 \
    DRX_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    DRX_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    DRX_GL_FUNCTIONS_GL_EXT_shader_pixel_local_storage2 \
    DRX_GL_FUNCTIONS_GL_EXT_sparse_texture \
    DRX_GL_FUNCTIONS_GL_EXT_tessellation_shader \
    DRX_GL_FUNCTIONS_GL_EXT_texture_border_clamp \
    DRX_GL_FUNCTIONS_GL_EXT_texture_buffer \
    DRX_GL_FUNCTIONS_GL_EXT_texture_storage \
    DRX_GL_FUNCTIONS_GL_EXT_texture_storage_compression \
    DRX_GL_FUNCTIONS_GL_EXT_texture_view \
    DRX_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    DRX_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    DRX_GL_FUNCTIONS_GL_EXT_window_rectangles \
    DRX_GL_FUNCTIONS_GL_IMG_bindless_texture \
    DRX_GL_FUNCTIONS_GL_IMG_framebuffer_downsample \
    DRX_GL_FUNCTIONS_GL_IMG_multisampled_render_to_texture \
    DRX_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    DRX_GL_FUNCTIONS_GL_INTEL_performance_query \
    DRX_GL_FUNCTIONS_GL_KHR_blend_equation_advanced \
    DRX_GL_FUNCTIONS_GL_KHR_debug \
    DRX_GL_FUNCTIONS_GL_KHR_robustness \
    DRX_GL_FUNCTIONS_GL_KHR_parallel_shader_compile \
    DRX_GL_FUNCTIONS_GL_MESA_framebuffer_flip_y \
    DRX_GL_FUNCTIONS_GL_NV_bindless_texture \
    DRX_GL_FUNCTIONS_GL_NV_blend_equation_advanced \
    DRX_GL_FUNCTIONS_GL_NV_clip_space_w_scaling \
    DRX_GL_FUNCTIONS_GL_NV_conditional_render \
    DRX_GL_FUNCTIONS_GL_NV_conservative_raster \
    DRX_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    DRX_GL_FUNCTIONS_GL_NV_copy_buffer \
    DRX_GL_FUNCTIONS_GL_NV_coverage_sample \
    DRX_GL_FUNCTIONS_GL_NV_draw_buffers \
    DRX_GL_FUNCTIONS_GL_NV_draw_instanced \
    DRX_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    DRX_GL_FUNCTIONS_GL_NV_fence \
    DRX_GL_FUNCTIONS_GL_NV_fragment_coverage_to_color \
    DRX_GL_FUNCTIONS_GL_NV_framebuffer_blit \
    DRX_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    DRX_GL_FUNCTIONS_GL_NV_framebuffer_multisample \
    DRX_GL_FUNCTIONS_GL_NV_gpu_shader5 \
    DRX_GL_FUNCTIONS_GL_NV_instanced_arrays \
    DRX_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    DRX_GL_FUNCTIONS_GL_NV_memory_attachment \
    DRX_GL_FUNCTIONS_GL_NV_memory_object_sparse \
    DRX_GL_FUNCTIONS_GL_NV_mesh_shader \
    DRX_GL_FUNCTIONS_GL_NV_non_square_matrices \
    DRX_GL_FUNCTIONS_GL_NV_path_rendering \
    DRX_GL_FUNCTIONS_GL_NV_polygon_mode \
    DRX_GL_FUNCTIONS_GL_NV_read_buffer \
    DRX_GL_FUNCTIONS_GL_NV_sample_locations \
    DRX_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    DRX_GL_FUNCTIONS_GL_NV_shading_rate_image \
    DRX_GL_FUNCTIONS_GL_NV_viewport_array \
    DRX_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    DRX_GL_FUNCTIONS_GL_OES_EGL_image \
    DRX_GL_FUNCTIONS_GL_OES_copy_image \
    DRX_GL_FUNCTIONS_GL_OES_draw_buffers_indexed \
    DRX_GL_FUNCTIONS_GL_OES_draw_elements_base_vertex \
    DRX_GL_FUNCTIONS_GL_OES_geometry_shader \
    DRX_GL_FUNCTIONS_GL_OES_get_program_binary \
    DRX_GL_FUNCTIONS_GL_OES_mapbuffer \
    DRX_GL_FUNCTIONS_GL_OES_primitive_bounding_box \
    DRX_GL_FUNCTIONS_GL_OES_sample_shading \
    DRX_GL_FUNCTIONS_GL_OES_tessellation_shader \
    DRX_GL_FUNCTIONS_GL_OES_texture_3D \
    DRX_GL_FUNCTIONS_GL_OES_texture_border_clamp \
    DRX_GL_FUNCTIONS_GL_OES_texture_buffer \
    DRX_GL_FUNCTIONS_GL_OES_texture_storage_multisample_2d_array \
    DRX_GL_FUNCTIONS_GL_OES_texture_view \
    DRX_GL_FUNCTIONS_GL_OES_vertex_array_object \
    DRX_GL_FUNCTIONS_GL_OES_viewport_array \
    DRX_GL_FUNCTIONS_GL_OVR_multiview \
    DRX_GL_FUNCTIONS_GL_OVR_multiview_multisampled_render_to_texture \
    DRX_GL_FUNCTIONS_GL_QCOM_alpha_test \
    DRX_GL_FUNCTIONS_GL_QCOM_driver_control \
    DRX_GL_FUNCTIONS_GL_QCOM_extended_get \
    DRX_GL_FUNCTIONS_GL_QCOM_extended_get2 \
    DRX_GL_FUNCTIONS_GL_QCOM_framebuffer_foveated \
    DRX_GL_FUNCTIONS_GL_QCOM_motion_estimation \
    DRX_GL_FUNCTIONS_GL_QCOM_frame_extrapolation \
    DRX_GL_FUNCTIONS_GL_QCOM_texture_foveated \
    DRX_GL_FUNCTIONS_GL_QCOM_shader_framebuffer_fetch_noncoherent \
    DRX_GL_FUNCTIONS_GL_QCOM_shading_rate \
    DRX_GL_FUNCTIONS_GL_QCOM_tiled_rendering

#define X(returns, name, params) \
    extern "C" KHRONOS_APICALL returns KHRONOS_APIENTRY name params; \
    returns (KHRONOS_APIENTRY* const& ::drx::gl::name) params = ::name;
DRX_STATIC_GL_FUNCTIONS
#undef X

#define X(returns, name, params) \
    static returns (KHRONOS_APIENTRY* drx_ ## name) params = nullptr; \
    returns (KHRONOS_APIENTRY* const& ::drx::gl::name) params = drx_ ## name;
DRX_DYNAMIC_GL_FUNCTIONS
DRX_EXTENSION_GL_FUNCTIONS
#undef X

z0 drx::gl::loadFunctions()
{
   #define X(returns, name, params) \
       drx_ ## name = reinterpret_cast<returns (KHRONOS_APIENTRY*) params> (::drx::OpenGLHelpers::getExtensionFunction (#name));
    DRX_DYNAMIC_GL_FUNCTIONS
   #undef X
}

z0 drx::gl::loadExtensions()
{
   #define X(returns, name, params) \
       drx_ ## name = reinterpret_cast<returns (KHRONOS_APIENTRY*) params> (::drx::OpenGLHelpers::getExtensionFunction (#name));
    DRX_EXTENSION_GL_FUNCTIONS
   #undef X
}

#undef DRX_STATIC_GL_FUNCTIONS
#undef DRX_DYNAMIC_GL_FUNCTIONS
#undef DRX_EXTENSION_GL_FUNCTIONS
