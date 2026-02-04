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


#define DRX_GL_FUNCTIONS_GL_VERSION_1_0 \
    X (z0        , glCullFace, (GLenum mode)) \
    X (z0        , glFrontFace, (GLenum mode)) \
    X (z0        , glHint, (GLenum target, GLenum mode)) \
    X (z0        , glLineWidth, (GLfloat width)) \
    X (z0        , glPointSize, (GLfloat size)) \
    X (z0        , glPolygonMode, (GLenum face, GLenum mode)) \
    X (z0        , glScissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glTexParameterf, (GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glTexParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glTexParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexImage1D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glDrawBuffer, (GLenum buf)) \
    X (z0        , glClear, (GLbitfield mask)) \
    X (z0        , glClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (z0        , glClearStencil, (GLint s)) \
    X (z0        , glClearDepth, (GLdouble depth)) \
    X (z0        , glStencilMask, (GLuint mask)) \
    X (z0        , glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) \
    X (z0        , glDepthMask, (GLboolean flag)) \
    X (z0        , glDisable, (GLenum cap)) \
    X (z0        , glEnable, (GLenum cap)) \
    X (z0        , glFinish, ()) \
    X (z0        , glFlush, ()) \
    X (z0        , glBlendFunc, (GLenum sfactor, GLenum dfactor)) \
    X (z0        , glLogicOp, (GLenum opcode)) \
    X (z0        , glStencilFunc, (GLenum func, GLint ref, GLuint mask)) \
    X (z0        , glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass)) \
    X (z0        , glDepthFunc, (GLenum func)) \
    X (z0        , glPixelStoref, (GLenum pname, GLfloat param)) \
    X (z0        , glPixelStorei, (GLenum pname, GLint param)) \
    X (z0        , glReadBuffer, (GLenum src)) \
    X (z0        , glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, uk pixels)) \
    X (z0        , glGetBooleanv, (GLenum pname, GLboolean *data)) \
    X (z0        , glGetDoublev, (GLenum pname, GLdouble *data)) \
    X (GLenum      , glGetError, ()) \
    X (z0        , glGetFloatv, (GLenum pname, GLfloat *data)) \
    X (z0        , glGetIntegerv, (GLenum pname, GLint *data)) \
    X (const GLubyte *, glGetString, (GLenum name)) \
    X (z0        , glGetTexImage, (GLenum target, GLint level, GLenum format, GLenum type, uk pixels)) \
    X (z0        , glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexLevelParameterfv, (GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsEnabled, (GLenum cap)) \
    X (z0        , glDepthRange, (GLdouble n, GLdouble f)) \
    X (z0        , glViewport, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glNewList, (GLuint list, GLenum mode)) \
    X (z0        , glEndList, ()) \
    X (z0        , glCallList, (GLuint list)) \
    X (z0        , glCallLists, (GLsizei n, GLenum type, ukk lists)) \
    X (z0        , glDeleteLists, (GLuint list, GLsizei range)) \
    X (GLuint      , glGenLists, (GLsizei range)) \
    X (z0        , glListBase, (GLuint base)) \
    X (z0        , glBegin, (GLenum mode)) \
    X (z0        , glBitmap, (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)) \
    X (z0        , glColor3b, (GLbyte red, GLbyte green, GLbyte blue)) \
    X (z0        , glColor3bv, (const GLbyte *v)) \
    X (z0        , glColor3d, (GLdouble red, GLdouble green, GLdouble blue)) \
    X (z0        , glColor3dv, (const GLdouble *v)) \
    X (z0        , glColor3f, (GLfloat red, GLfloat green, GLfloat blue)) \
    X (z0        , glColor3fv, (const GLfloat *v)) \
    X (z0        , glColor3i, (GLint red, GLint green, GLint blue)) \
    X (z0        , glColor3iv, (const GLint *v)) \
    X (z0        , glColor3s, (GLshort red, GLshort green, GLshort blue)) \
    X (z0        , glColor3sv, (const GLshort *v)) \
    X (z0        , glColor3ub, (GLubyte red, GLubyte green, GLubyte blue)) \
    X (z0        , glColor3ubv, (const GLubyte *v)) \
    X (z0        , glColor3ui, (GLuint red, GLuint green, GLuint blue)) \
    X (z0        , glColor3uiv, (const GLuint *v)) \
    X (z0        , glColor3us, (GLushort red, GLushort green, GLushort blue)) \
    X (z0        , glColor3usv, (const GLushort *v)) \
    X (z0        , glColor4b, (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)) \
    X (z0        , glColor4bv, (const GLbyte *v)) \
    X (z0        , glColor4d, (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)) \
    X (z0        , glColor4dv, (const GLdouble *v)) \
    X (z0        , glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (z0        , glColor4fv, (const GLfloat *v)) \
    X (z0        , glColor4i, (GLint red, GLint green, GLint blue, GLint alpha)) \
    X (z0        , glColor4iv, (const GLint *v)) \
    X (z0        , glColor4s, (GLshort red, GLshort green, GLshort blue, GLshort alpha)) \
    X (z0        , glColor4sv, (const GLshort *v)) \
    X (z0        , glColor4ub, (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)) \
    X (z0        , glColor4ubv, (const GLubyte *v)) \
    X (z0        , glColor4ui, (GLuint red, GLuint green, GLuint blue, GLuint alpha)) \
    X (z0        , glColor4uiv, (const GLuint *v)) \
    X (z0        , glColor4us, (GLushort red, GLushort green, GLushort blue, GLushort alpha)) \
    X (z0        , glColor4usv, (const GLushort *v)) \
    X (z0        , glEdgeFlag, (GLboolean flag)) \
    X (z0        , glEdgeFlagv, (const GLboolean *flag)) \
    X (z0        , glEnd, ()) \
    X (z0        , glIndexd, (GLdouble c)) \
    X (z0        , glIndexdv, (const GLdouble *c)) \
    X (z0        , glIndexf, (GLfloat c)) \
    X (z0        , glIndexfv, (const GLfloat *c)) \
    X (z0        , glIndexi, (GLint c)) \
    X (z0        , glIndexiv, (const GLint *c)) \
    X (z0        , glIndexs, (GLshort c)) \
    X (z0        , glIndexsv, (const GLshort *c)) \
    X (z0        , glNormal3b, (GLbyte nx, GLbyte ny, GLbyte nz)) \
    X (z0        , glNormal3bv, (const GLbyte *v)) \
    X (z0        , glNormal3d, (GLdouble nx, GLdouble ny, GLdouble nz)) \
    X (z0        , glNormal3dv, (const GLdouble *v)) \
    X (z0        , glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz)) \
    X (z0        , glNormal3fv, (const GLfloat *v)) \
    X (z0        , glNormal3i, (GLint nx, GLint ny, GLint nz)) \
    X (z0        , glNormal3iv, (const GLint *v)) \
    X (z0        , glNormal3s, (GLshort nx, GLshort ny, GLshort nz)) \
    X (z0        , glNormal3sv, (const GLshort *v)) \
    X (z0        , glRasterPos2d, (GLdouble x, GLdouble y)) \
    X (z0        , glRasterPos2dv, (const GLdouble *v)) \
    X (z0        , glRasterPos2f, (GLfloat x, GLfloat y)) \
    X (z0        , glRasterPos2fv, (const GLfloat *v)) \
    X (z0        , glRasterPos2i, (GLint x, GLint y)) \
    X (z0        , glRasterPos2iv, (const GLint *v)) \
    X (z0        , glRasterPos2s, (GLshort x, GLshort y)) \
    X (z0        , glRasterPos2sv, (const GLshort *v)) \
    X (z0        , glRasterPos3d, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glRasterPos3dv, (const GLdouble *v)) \
    X (z0        , glRasterPos3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glRasterPos3fv, (const GLfloat *v)) \
    X (z0        , glRasterPos3i, (GLint x, GLint y, GLint z)) \
    X (z0        , glRasterPos3iv, (const GLint *v)) \
    X (z0        , glRasterPos3s, (GLshort x, GLshort y, GLshort z)) \
    X (z0        , glRasterPos3sv, (const GLshort *v)) \
    X (z0        , glRasterPos4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glRasterPos4dv, (const GLdouble *v)) \
    X (z0        , glRasterPos4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glRasterPos4fv, (const GLfloat *v)) \
    X (z0        , glRasterPos4i, (GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glRasterPos4iv, (const GLint *v)) \
    X (z0        , glRasterPos4s, (GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glRasterPos4sv, (const GLshort *v)) \
    X (z0        , glRectd, (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)) \
    X (z0        , glRectdv, (const GLdouble *v1, const GLdouble *v2)) \
    X (z0        , glRectf, (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)) \
    X (z0        , glRectfv, (const GLfloat *v1, const GLfloat *v2)) \
    X (z0        , glRecti, (GLint x1, GLint y1, GLint x2, GLint y2)) \
    X (z0        , glRectiv, (const GLint *v1, const GLint *v2)) \
    X (z0        , glRects, (GLshort x1, GLshort y1, GLshort x2, GLshort y2)) \
    X (z0        , glRectsv, (const GLshort *v1, const GLshort *v2)) \
    X (z0        , glTexCoord1d, (GLdouble s)) \
    X (z0        , glTexCoord1dv, (const GLdouble *v)) \
    X (z0        , glTexCoord1f, (GLfloat s)) \
    X (z0        , glTexCoord1fv, (const GLfloat *v)) \
    X (z0        , glTexCoord1i, (GLint s)) \
    X (z0        , glTexCoord1iv, (const GLint *v)) \
    X (z0        , glTexCoord1s, (GLshort s)) \
    X (z0        , glTexCoord1sv, (const GLshort *v)) \
    X (z0        , glTexCoord2d, (GLdouble s, GLdouble t)) \
    X (z0        , glTexCoord2dv, (const GLdouble *v)) \
    X (z0        , glTexCoord2f, (GLfloat s, GLfloat t)) \
    X (z0        , glTexCoord2fv, (const GLfloat *v)) \
    X (z0        , glTexCoord2i, (GLint s, GLint t)) \
    X (z0        , glTexCoord2iv, (const GLint *v)) \
    X (z0        , glTexCoord2s, (GLshort s, GLshort t)) \
    X (z0        , glTexCoord2sv, (const GLshort *v)) \
    X (z0        , glTexCoord3d, (GLdouble s, GLdouble t, GLdouble r)) \
    X (z0        , glTexCoord3dv, (const GLdouble *v)) \
    X (z0        , glTexCoord3f, (GLfloat s, GLfloat t, GLfloat r)) \
    X (z0        , glTexCoord3fv, (const GLfloat *v)) \
    X (z0        , glTexCoord3i, (GLint s, GLint t, GLint r)) \
    X (z0        , glTexCoord3iv, (const GLint *v)) \
    X (z0        , glTexCoord3s, (GLshort s, GLshort t, GLshort r)) \
    X (z0        , glTexCoord3sv, (const GLshort *v)) \
    X (z0        , glTexCoord4d, (GLdouble s, GLdouble t, GLdouble r, GLdouble q)) \
    X (z0        , glTexCoord4dv, (const GLdouble *v)) \
    X (z0        , glTexCoord4f, (GLfloat s, GLfloat t, GLfloat r, GLfloat q)) \
    X (z0        , glTexCoord4fv, (const GLfloat *v)) \
    X (z0        , glTexCoord4i, (GLint s, GLint t, GLint r, GLint q)) \
    X (z0        , glTexCoord4iv, (const GLint *v)) \
    X (z0        , glTexCoord4s, (GLshort s, GLshort t, GLshort r, GLshort q)) \
    X (z0        , glTexCoord4sv, (const GLshort *v)) \
    X (z0        , glVertex2d, (GLdouble x, GLdouble y)) \
    X (z0        , glVertex2dv, (const GLdouble *v)) \
    X (z0        , glVertex2f, (GLfloat x, GLfloat y)) \
    X (z0        , glVertex2fv, (const GLfloat *v)) \
    X (z0        , glVertex2i, (GLint x, GLint y)) \
    X (z0        , glVertex2iv, (const GLint *v)) \
    X (z0        , glVertex2s, (GLshort x, GLshort y)) \
    X (z0        , glVertex2sv, (const GLshort *v)) \
    X (z0        , glVertex3d, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertex3dv, (const GLdouble *v)) \
    X (z0        , glVertex3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glVertex3fv, (const GLfloat *v)) \
    X (z0        , glVertex3i, (GLint x, GLint y, GLint z)) \
    X (z0        , glVertex3iv, (const GLint *v)) \
    X (z0        , glVertex3s, (GLshort x, GLshort y, GLshort z)) \
    X (z0        , glVertex3sv, (const GLshort *v)) \
    X (z0        , glVertex4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertex4dv, (const GLdouble *v)) \
    X (z0        , glVertex4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glVertex4fv, (const GLfloat *v)) \
    X (z0        , glVertex4i, (GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glVertex4iv, (const GLint *v)) \
    X (z0        , glVertex4s, (GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glVertex4sv, (const GLshort *v)) \
    X (z0        , glClipPlane, (GLenum plane, const GLdouble *equation)) \
    X (z0        , glColorMaterial, (GLenum face, GLenum mode)) \
    X (z0        , glFogf, (GLenum pname, GLfloat param)) \
    X (z0        , glFogfv, (GLenum pname, const GLfloat *params)) \
    X (z0        , glFogi, (GLenum pname, GLint param)) \
    X (z0        , glFogiv, (GLenum pname, const GLint *params)) \
    X (z0        , glLightf, (GLenum light, GLenum pname, GLfloat param)) \
    X (z0        , glLightfv, (GLenum light, GLenum pname, const GLfloat *params)) \
    X (z0        , glLighti, (GLenum light, GLenum pname, GLint param)) \
    X (z0        , glLightiv, (GLenum light, GLenum pname, const GLint *params)) \
    X (z0        , glLightModelf, (GLenum pname, GLfloat param)) \
    X (z0        , glLightModelfv, (GLenum pname, const GLfloat *params)) \
    X (z0        , glLightModeli, (GLenum pname, GLint param)) \
    X (z0        , glLightModeliv, (GLenum pname, const GLint *params)) \
    X (z0        , glLineStipple, (GLint factor, GLushort pattern)) \
    X (z0        , glMaterialf, (GLenum face, GLenum pname, GLfloat param)) \
    X (z0        , glMaterialfv, (GLenum face, GLenum pname, const GLfloat *params)) \
    X (z0        , glMateriali, (GLenum face, GLenum pname, GLint param)) \
    X (z0        , glMaterialiv, (GLenum face, GLenum pname, const GLint *params)) \
    X (z0        , glPolygonStipple, (const GLubyte *mask)) \
    X (z0        , glShadeModel, (GLenum mode)) \
    X (z0        , glTexEnvf, (GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glTexEnvfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glTexEnvi, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glTexEnviv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexGend, (GLenum coord, GLenum pname, GLdouble param)) \
    X (z0        , glTexGendv, (GLenum coord, GLenum pname, const GLdouble *params)) \
    X (z0        , glTexGenf, (GLenum coord, GLenum pname, GLfloat param)) \
    X (z0        , glTexGenfv, (GLenum coord, GLenum pname, const GLfloat *params)) \
    X (z0        , glTexGeni, (GLenum coord, GLenum pname, GLint param)) \
    X (z0        , glTexGeniv, (GLenum coord, GLenum pname, const GLint *params)) \
    X (z0        , glFeedbackBuffer, (GLsizei size, GLenum type, GLfloat *buffer)) \
    X (z0        , glSelectBuffer, (GLsizei size, GLuint *buffer)) \
    X (GLint       , glRenderMode, (GLenum mode)) \
    X (z0        , glInitNames, ()) \
    X (z0        , glLoadName, (GLuint name)) \
    X (z0        , glPassThrough, (GLfloat token)) \
    X (z0        , glPopName, ()) \
    X (z0        , glPushName, (GLuint name)) \
    X (z0        , glClearAccum, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (z0        , glClearIndex, (GLfloat c)) \
    X (z0        , glIndexMask, (GLuint mask)) \
    X (z0        , glAccum, (GLenum op, GLfloat value)) \
    X (z0        , glPopAttrib, ()) \
    X (z0        , glPushAttrib, (GLbitfield mask)) \
    X (z0        , glMap1d, (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)) \
    X (z0        , glMap1f, (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)) \
    X (z0        , glMap2d, (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)) \
    X (z0        , glMap2f, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)) \
    X (z0        , glMapGrid1d, (GLint un, GLdouble u1, GLdouble u2)) \
    X (z0        , glMapGrid1f, (GLint un, GLfloat u1, GLfloat u2)) \
    X (z0        , glMapGrid2d, (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)) \
    X (z0        , glMapGrid2f, (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)) \
    X (z0        , glEvalCoord1d, (GLdouble u)) \
    X (z0        , glEvalCoord1dv, (const GLdouble *u)) \
    X (z0        , glEvalCoord1f, (GLfloat u)) \
    X (z0        , glEvalCoord1fv, (const GLfloat *u)) \
    X (z0        , glEvalCoord2d, (GLdouble u, GLdouble v)) \
    X (z0        , glEvalCoord2dv, (const GLdouble *u)) \
    X (z0        , glEvalCoord2f, (GLfloat u, GLfloat v)) \
    X (z0        , glEvalCoord2fv, (const GLfloat *u)) \
    X (z0        , glEvalMesh1, (GLenum mode, GLint i1, GLint i2)) \
    X (z0        , glEvalPoint1, (GLint i)) \
    X (z0        , glEvalMesh2, (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)) \
    X (z0        , glEvalPoint2, (GLint i, GLint j)) \
    X (z0        , glAlphaFunc, (GLenum func, GLfloat ref)) \
    X (z0        , glPixelZoom, (GLfloat xfactor, GLfloat yfactor)) \
    X (z0        , glPixelTransferf, (GLenum pname, GLfloat param)) \
    X (z0        , glPixelTransferi, (GLenum pname, GLint param)) \
    X (z0        , glPixelMapfv, (GLenum map, GLsizei mapsize, const GLfloat *values)) \
    X (z0        , glPixelMapuiv, (GLenum map, GLsizei mapsize, const GLuint *values)) \
    X (z0        , glPixelMapusv, (GLenum map, GLsizei mapsize, const GLushort *values)) \
    X (z0        , glCopyPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)) \
    X (z0        , glDrawPixels, (GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glGetClipPlane, (GLenum plane, GLdouble *equation)) \
    X (z0        , glGetLightfv, (GLenum light, GLenum pname, GLfloat *params)) \
    X (z0        , glGetLightiv, (GLenum light, GLenum pname, GLint *params)) \
    X (z0        , glGetMapdv, (GLenum target, GLenum query, GLdouble *v)) \
    X (z0        , glGetMapfv, (GLenum target, GLenum query, GLfloat *v)) \
    X (z0        , glGetMapiv, (GLenum target, GLenum query, GLint *v)) \
    X (z0        , glGetMaterialfv, (GLenum face, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMaterialiv, (GLenum face, GLenum pname, GLint *params)) \
    X (z0        , glGetPixelMapfv, (GLenum map, GLfloat *values)) \
    X (z0        , glGetPixelMapuiv, (GLenum map, GLuint *values)) \
    X (z0        , glGetPixelMapusv, (GLenum map, GLushort *values)) \
    X (z0        , glGetPolygonStipple, (GLubyte *mask)) \
    X (z0        , glGetTexEnvfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTexEnviv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexGendv, (GLenum coord, GLenum pname, GLdouble *params)) \
    X (z0        , glGetTexGenfv, (GLenum coord, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTexGeniv, (GLenum coord, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsList, (GLuint list)) \
    X (z0        , glFrustum, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (z0        , glLoadIdentity, ()) \
    X (z0        , glLoadMatrixf, (const GLfloat *m)) \
    X (z0        , glLoadMatrixd, (const GLdouble *m)) \
    X (z0        , glMatrixMode, (GLenum mode)) \
    X (z0        , glMultMatrixf, (const GLfloat *m)) \
    X (z0        , glMultMatrixd, (const GLdouble *m)) \
    X (z0        , glOrtho, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (z0        , glPopMatrix, ()) \
    X (z0        , glPushMatrix, ()) \
    X (z0        , glRotated, (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glRotatef, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glScaled, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glScalef, (GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glTranslated, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glTranslatef, (GLfloat x, GLfloat y, GLfloat z))

#define DRX_GL_FUNCTIONS_GL_VERSION_1_1 \
    X (z0        , glDrawArrays, (GLenum mode, GLint first, GLsizei count)) \
    X (z0        , glDrawElements, (GLenum mode, GLsizei count, GLenum type, ukk indices)) \
    X (z0        , glGetPointerv, (GLenum pname, uk *params)) \
    X (z0        , glPolygonOffset, (GLfloat factor, GLfloat units)) \
    X (z0        , glCopyTexImage1D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (z0        , glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (z0        , glCopyTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glBindTexture, (GLenum target, GLuint texture)) \
    X (z0        , glDeleteTextures, (GLsizei n, const GLuint *textures)) \
    X (z0        , glGenTextures, (GLsizei n, GLuint *textures)) \
    X (GLboolean   , glIsTexture, (GLuint texture)) \
    X (z0        , glArrayElement, (GLint i)) \
    X (z0        , glColorPointer, (GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glDisableClientState, (GLenum array)) \
    X (z0        , glEdgeFlagPointer, (GLsizei stride, ukk pointer)) \
    X (z0        , glEnableClientState, (GLenum array)) \
    X (z0        , glIndexPointer, (GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glInterleavedArrays, (GLenum format, GLsizei stride, ukk pointer)) \
    X (z0        , glNormalPointer, (GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glVertexPointer, (GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (GLboolean   , glAreTexturesResident, (GLsizei n, const GLuint *textures, GLboolean *residences)) \
    X (z0        , glPrioritizeTextures, (GLsizei n, const GLuint *textures, const GLfloat *priorities)) \
    X (z0        , glIndexub, (GLubyte c)) \
    X (z0        , glIndexubv, (const GLubyte *c)) \
    X (z0        , glPopClientAttrib, ()) \
    X (z0        , glPushClientAttrib, (GLbitfield mask))

#define DRX_GL_FUNCTIONS_GL_VERSION_1_2 \
    X (z0        , glDrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices)) \
    X (z0        , glTexImage3D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_VERSION_1_3 \
    X (z0        , glActiveTexture, (GLenum texture)) \
    X (z0        , glSampleCoverage, (GLfloat value, GLboolean invert)) \
    X (z0        , glCompressedTexImage3D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexImage1D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glGetCompressedTexImage, (GLenum target, GLint level, uk img)) \
    X (z0        , glClientActiveTexture, (GLenum texture)) \
    X (z0        , glMultiTexCoord1d, (GLenum target, GLdouble s)) \
    X (z0        , glMultiTexCoord1dv, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord1f, (GLenum target, GLfloat s)) \
    X (z0        , glMultiTexCoord1fv, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord1i, (GLenum target, GLint s)) \
    X (z0        , glMultiTexCoord1iv, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord1s, (GLenum target, GLshort s)) \
    X (z0        , glMultiTexCoord1sv, (GLenum target, const GLshort *v)) \
    X (z0        , glMultiTexCoord2d, (GLenum target, GLdouble s, GLdouble t)) \
    X (z0        , glMultiTexCoord2dv, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord2f, (GLenum target, GLfloat s, GLfloat t)) \
    X (z0        , glMultiTexCoord2fv, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord2i, (GLenum target, GLint s, GLint t)) \
    X (z0        , glMultiTexCoord2iv, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord2s, (GLenum target, GLshort s, GLshort t)) \
    X (z0        , glMultiTexCoord2sv, (GLenum target, const GLshort *v)) \
    X (z0        , glMultiTexCoord3d, (GLenum target, GLdouble s, GLdouble t, GLdouble r)) \
    X (z0        , glMultiTexCoord3dv, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord3f, (GLenum target, GLfloat s, GLfloat t, GLfloat r)) \
    X (z0        , glMultiTexCoord3fv, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord3i, (GLenum target, GLint s, GLint t, GLint r)) \
    X (z0        , glMultiTexCoord3iv, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord3s, (GLenum target, GLshort s, GLshort t, GLshort r)) \
    X (z0        , glMultiTexCoord3sv, (GLenum target, const GLshort *v)) \
    X (z0        , glMultiTexCoord4d, (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)) \
    X (z0        , glMultiTexCoord4dv, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord4f, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)) \
    X (z0        , glMultiTexCoord4fv, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord4i, (GLenum target, GLint s, GLint t, GLint r, GLint q)) \
    X (z0        , glMultiTexCoord4iv, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord4s, (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)) \
    X (z0        , glMultiTexCoord4sv, (GLenum target, const GLshort *v)) \
    X (z0        , glLoadTransposeMatrixf, (const GLfloat *m)) \
    X (z0        , glLoadTransposeMatrixd, (const GLdouble *m)) \
    X (z0        , glMultTransposeMatrixf, (const GLfloat *m)) \
    X (z0        , glMultTransposeMatrixd, (const GLdouble *m))

#define DRX_GL_FUNCTIONS_GL_VERSION_1_4 \
    X (z0        , glBlendFuncSeparate, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)) \
    X (z0        , glMultiDrawArrays, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)) \
    X (z0        , glMultiDrawElements, (GLenum mode, const GLsizei *count, GLenum type, ukk const*indices, GLsizei drawcount)) \
    X (z0        , glPointParameterf, (GLenum pname, GLfloat param)) \
    X (z0        , glPointParameterfv, (GLenum pname, const GLfloat *params)) \
    X (z0        , glPointParameteri, (GLenum pname, GLint param)) \
    X (z0        , glPointParameteriv, (GLenum pname, const GLint *params)) \
    X (z0        , glFogCoordf, (GLfloat coord)) \
    X (z0        , glFogCoordfv, (const GLfloat *coord)) \
    X (z0        , glFogCoordd, (GLdouble coord)) \
    X (z0        , glFogCoorddv, (const GLdouble *coord)) \
    X (z0        , glFogCoordPointer, (GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glSecondaryColor3b, (GLbyte red, GLbyte green, GLbyte blue)) \
    X (z0        , glSecondaryColor3bv, (const GLbyte *v)) \
    X (z0        , glSecondaryColor3d, (GLdouble red, GLdouble green, GLdouble blue)) \
    X (z0        , glSecondaryColor3dv, (const GLdouble *v)) \
    X (z0        , glSecondaryColor3f, (GLfloat red, GLfloat green, GLfloat blue)) \
    X (z0        , glSecondaryColor3fv, (const GLfloat *v)) \
    X (z0        , glSecondaryColor3i, (GLint red, GLint green, GLint blue)) \
    X (z0        , glSecondaryColor3iv, (const GLint *v)) \
    X (z0        , glSecondaryColor3s, (GLshort red, GLshort green, GLshort blue)) \
    X (z0        , glSecondaryColor3sv, (const GLshort *v)) \
    X (z0        , glSecondaryColor3ub, (GLubyte red, GLubyte green, GLubyte blue)) \
    X (z0        , glSecondaryColor3ubv, (const GLubyte *v)) \
    X (z0        , glSecondaryColor3ui, (GLuint red, GLuint green, GLuint blue)) \
    X (z0        , glSecondaryColor3uiv, (const GLuint *v)) \
    X (z0        , glSecondaryColor3us, (GLushort red, GLushort green, GLushort blue)) \
    X (z0        , glSecondaryColor3usv, (const GLushort *v)) \
    X (z0        , glSecondaryColorPointer, (GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glWindowPos2d, (GLdouble x, GLdouble y)) \
    X (z0        , glWindowPos2dv, (const GLdouble *v)) \
    X (z0        , glWindowPos2f, (GLfloat x, GLfloat y)) \
    X (z0        , glWindowPos2fv, (const GLfloat *v)) \
    X (z0        , glWindowPos2i, (GLint x, GLint y)) \
    X (z0        , glWindowPos2iv, (const GLint *v)) \
    X (z0        , glWindowPos2s, (GLshort x, GLshort y)) \
    X (z0        , glWindowPos2sv, (const GLshort *v)) \
    X (z0        , glWindowPos3d, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glWindowPos3dv, (const GLdouble *v)) \
    X (z0        , glWindowPos3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glWindowPos3fv, (const GLfloat *v)) \
    X (z0        , glWindowPos3i, (GLint x, GLint y, GLint z)) \
    X (z0        , glWindowPos3iv, (const GLint *v)) \
    X (z0        , glWindowPos3s, (GLshort x, GLshort y, GLshort z)) \
    X (z0        , glWindowPos3sv, (const GLshort *v)) \
    X (z0        , glBlendColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (z0        , glBlendEquation, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_VERSION_1_5 \
    X (z0        , glGenQueries, (GLsizei n, GLuint *ids)) \
    X (z0        , glDeleteQueries, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQuery, (GLuint id)) \
    X (z0        , glBeginQuery, (GLenum target, GLuint id)) \
    X (z0        , glEndQuery, (GLenum target)) \
    X (z0        , glGetQueryiv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectiv, (GLuint id, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint *params)) \
    X (z0        , glBindBuffer, (GLenum target, GLuint buffer)) \
    X (z0        , glDeleteBuffers, (GLsizei n, const GLuint *buffers)) \
    X (z0        , glGenBuffers, (GLsizei n, GLuint *buffers)) \
    X (GLboolean   , glIsBuffer, (GLuint buffer)) \
    X (z0        , glBufferData, (GLenum target, GLsizeiptr size, ukk data, GLenum usage)) \
    X (z0        , glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, ukk data)) \
    X (z0        , glGetBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, uk data)) \
    X (uk       , glMapBuffer, (GLenum target, GLenum access)) \
    X (GLboolean   , glUnmapBuffer, (GLenum target)) \
    X (z0        , glGetBufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetBufferPointerv, (GLenum target, GLenum pname, uk *params))

#define DRX_GL_FUNCTIONS_GL_VERSION_2_0 \
    X (z0        , glBlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glDrawBuffers, (GLsizei n, const GLenum *bufs)) \
    X (z0        , glStencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
    X (z0        , glStencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask)) \
    X (z0        , glStencilMaskSeparate, (GLenum face, GLuint mask)) \
    X (z0        , glAttachShader, (GLuint program, GLuint shader)) \
    X (z0        , glBindAttribLocation, (GLuint program, GLuint index, const GLchar *name)) \
    X (z0        , glCompileShader, (GLuint shader)) \
    X (GLuint      , glCreateProgram, ()) \
    X (GLuint      , glCreateShader, (GLenum type)) \
    X (z0        , glDeleteProgram, (GLuint program)) \
    X (z0        , glDeleteShader, (GLuint shader)) \
    X (z0        , glDetachShader, (GLuint program, GLuint shader)) \
    X (z0        , glDisableVertexAttribArray, (GLuint index)) \
    X (z0        , glEnableVertexAttribArray, (GLuint index)) \
    X (z0        , glGetActiveAttrib, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (z0        , glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (z0        , glGetAttachedShaders, (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)) \
    X (GLint       , glGetAttribLocation, (GLuint program, const GLchar *name)) \
    X (z0        , glGetProgramiv, (GLuint program, GLenum pname, GLint *params)) \
    X (z0        , glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glGetShaderiv, (GLuint shader, GLenum pname, GLint *params)) \
    X (z0        , glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glGetShaderSource, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)) \
    X (GLint       , glGetUniformLocation, (GLuint program, const GLchar *name)) \
    X (z0        , glGetUniformfv, (GLuint program, GLint location, GLfloat *params)) \
    X (z0        , glGetUniformiv, (GLuint program, GLint location, GLint *params)) \
    X (z0        , glGetVertexAttribdv, (GLuint index, GLenum pname, GLdouble *params)) \
    X (z0        , glGetVertexAttribfv, (GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVertexAttribiv, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribPointerv, (GLuint index, GLenum pname, uk *pointer)) \
    X (GLboolean   , glIsProgram, (GLuint program)) \
    X (GLboolean   , glIsShader, (GLuint shader)) \
    X (z0        , glLinkProgram, (GLuint program)) \
    X (z0        , glShaderSource, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)) \
    X (z0        , glUseProgram, (GLuint program)) \
    X (z0        , glUniform1f, (GLint location, GLfloat v0)) \
    X (z0        , glUniform2f, (GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glUniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glUniform1i, (GLint location, GLint v0)) \
    X (z0        , glUniform2i, (GLint location, GLint v0, GLint v1)) \
    X (z0        , glUniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glUniform4i, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glUniform1fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform2fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform3fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform4fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform1iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform2iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform3iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform4iv, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glValidateProgram, (GLuint program)) \
    X (z0        , glVertexAttrib1d, (GLuint index, GLdouble x)) \
    X (z0        , glVertexAttrib1dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib1f, (GLuint index, GLfloat x)) \
    X (z0        , glVertexAttrib1fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib1s, (GLuint index, GLshort x)) \
    X (z0        , glVertexAttrib1sv, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib2d, (GLuint index, GLdouble x, GLdouble y)) \
    X (z0        , glVertexAttrib2dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib2f, (GLuint index, GLfloat x, GLfloat y)) \
    X (z0        , glVertexAttrib2fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib2s, (GLuint index, GLshort x, GLshort y)) \
    X (z0        , glVertexAttrib2sv, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib3d, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertexAttrib3dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib3f, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glVertexAttrib3fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib3s, (GLuint index, GLshort x, GLshort y, GLshort z)) \
    X (z0        , glVertexAttrib3sv, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4Nbv, (GLuint index, const GLbyte *v)) \
    X (z0        , glVertexAttrib4Niv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttrib4Nsv, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4Nub, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)) \
    X (z0        , glVertexAttrib4Nubv, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttrib4Nuiv, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttrib4Nusv, (GLuint index, const GLushort *v)) \
    X (z0        , glVertexAttrib4bv, (GLuint index, const GLbyte *v)) \
    X (z0        , glVertexAttrib4d, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertexAttrib4dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib4f, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glVertexAttrib4fv, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib4iv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttrib4s, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glVertexAttrib4sv, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4ubv, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttrib4uiv, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttrib4usv, (GLuint index, const GLushort *v)) \
    X (z0        , glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_VERSION_2_1 \
    X (z0        , glUniformMatrix2x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix2x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

#define DRX_GL_FUNCTIONS_GL_VERSION_3_0 \
    X (z0        , glColorMaski, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (z0        , glGetBooleani_v, (GLenum target, GLuint index, GLboolean *data)) \
    X (z0        , glGetIntegeri_v, (GLenum target, GLuint index, GLint *data)) \
    X (z0        , glEnablei, (GLenum target, GLuint index)) \
    X (z0        , glDisablei, (GLenum target, GLuint index)) \
    X (GLboolean   , glIsEnabledi, (GLenum target, GLuint index)) \
    X (z0        , glBeginTransformFeedback, (GLenum primitiveMode)) \
    X (z0        , glEndTransformFeedback, ()) \
    X (z0        , glBindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glBindBufferBase, (GLenum target, GLuint index, GLuint buffer)) \
    X (z0        , glTransformFeedbackVaryings, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)) \
    X (z0        , glGetTransformFeedbackVarying, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)) \
    X (z0        , glClampColor, (GLenum target, GLenum clamp)) \
    X (z0        , glBeginConditionalRender, (GLuint id, GLenum mode)) \
    X (z0        , glEndConditionalRender, ()) \
    X (z0        , glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glGetVertexAttribIiv, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribIuiv, (GLuint index, GLenum pname, GLuint *params)) \
    X (z0        , glVertexAttribI1i, (GLuint index, GLint x)) \
    X (z0        , glVertexAttribI2i, (GLuint index, GLint x, GLint y)) \
    X (z0        , glVertexAttribI3i, (GLuint index, GLint x, GLint y, GLint z)) \
    X (z0        , glVertexAttribI4i, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glVertexAttribI1ui, (GLuint index, GLuint x)) \
    X (z0        , glVertexAttribI2ui, (GLuint index, GLuint x, GLuint y)) \
    X (z0        , glVertexAttribI3ui, (GLuint index, GLuint x, GLuint y, GLuint z)) \
    X (z0        , glVertexAttribI4ui, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (z0        , glVertexAttribI1iv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI2iv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI3iv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI4iv, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI1uiv, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI2uiv, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI3uiv, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI4uiv, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI4bv, (GLuint index, const GLbyte *v)) \
    X (z0        , glVertexAttribI4sv, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttribI4ubv, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttribI4usv, (GLuint index, const GLushort *v)) \
    X (z0        , glGetUniformuiv, (GLuint program, GLint location, GLuint *params)) \
    X (z0        , glBindFragDataLocation, (GLuint program, GLuint color, const GLchar *name)) \
    X (GLint       , glGetFragDataLocation, (GLuint program, const GLchar *name)) \
    X (z0        , glUniform1ui, (GLint location, GLuint v0)) \
    X (z0        , glUniform2ui, (GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glUniform3ui, (GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glUniform4ui, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glUniform1uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform2uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform3uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform4uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glTexParameterIiv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexParameterIuiv, (GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetTexParameterIiv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexParameterIuiv, (GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glClearBufferiv, (GLenum buffer, GLint drawbuffer, const GLint *value)) \
    X (z0        , glClearBufferuiv, (GLenum buffer, GLint drawbuffer, const GLuint *value)) \
    X (z0        , glClearBufferfv, (GLenum buffer, GLint drawbuffer, const GLfloat *value)) \
    X (z0        , glClearBufferfi, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
    X (const GLubyte *, glGetStringi, (GLenum name, GLuint index)) \
    X (GLboolean   , glIsRenderbuffer, (GLuint renderbuffer)) \
    X (z0        , glBindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
    X (z0        , glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers)) \
    X (z0        , glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
    X (z0        , glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsFramebuffer, (GLuint framebuffer)) \
    X (z0        , glBindFramebuffer, (GLenum target, GLuint framebuffer)) \
    X (z0        , glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers)) \
    X (z0        , glGenFramebuffers, (GLsizei n, GLuint *framebuffers)) \
    X (GLenum      , glCheckFramebufferStatus, (GLenum target)) \
    X (z0        , glFramebufferTexture1D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glFramebufferTexture3D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X (z0        , glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (z0        , glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint *params)) \
    X (z0        , glGenerateMipmap, (GLenum target)) \
    X (z0        , glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (z0        , glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (uk       , glMapBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (z0        , glFlushMappedBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length)) \
    X (z0        , glBindVertexArray, (GLuint array)) \
    X (z0        , glDeleteVertexArrays, (GLsizei n, const GLuint *arrays)) \
    X (z0        , glGenVertexArrays, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArray, (GLuint array))

#define DRX_GL_FUNCTIONS_GL_VERSION_3_1 \
    X (z0        , glDrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X (z0        , glDrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount)) \
    X (z0        , glTexBuffer, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (z0        , glPrimitiveRestartIndex, (GLuint index)) \
    X (z0        , glCopyBufferSubData, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (z0        , glGetUniformIndices, (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)) \
    X (z0        , glGetActiveUniformsiv, (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)) \
    X (z0        , glGetActiveUniformName, (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)) \
    X (GLuint      , glGetUniformBlockIndex, (GLuint program, const GLchar *uniformBlockName)) \
    X (z0        , glGetActiveUniformBlockiv, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)) \
    X (z0        , glGetActiveUniformBlockName, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)) \
    X (z0        , glUniformBlockBinding, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding))

#define DRX_GL_FUNCTIONS_GL_VERSION_3_2 \
    X (z0        , glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices, GLint basevertex)) \
    X (z0        , glDrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLint basevertex)) \
    X (z0        , glMultiDrawElementsBaseVertex, (GLenum mode, const GLsizei *count, GLenum type, ukk const*indices, GLsizei drawcount, const GLint *basevertex)) \
    X (z0        , glProvokingVertex, (GLenum mode)) \
    X (GLsync      , glFenceSync, (GLenum condition, GLbitfield flags)) \
    X (GLboolean   , glIsSync, (GLsync sync)) \
    X (z0        , glDeleteSync, (GLsync sync)) \
    X (GLenum      , glClientWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (z0        , glWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (z0        , glGetInteger64v, (GLenum pname, GLint64 *data)) \
    X (z0        , glGetSynciv, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values)) \
    X (z0        , glGetInteger64i_v, (GLenum target, GLuint index, GLint64 *data)) \
    X (z0        , glGetBufferParameteri64v, (GLenum target, GLenum pname, GLint64 *params)) \
    X (z0        , glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (z0        , glTexImage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (z0        , glTexImage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (z0        , glGetMultisamplefv, (GLenum pname, GLuint index, GLfloat *val)) \
    X (z0        , glSampleMaski, (GLuint maskNumber, GLbitfield mask))

#define DRX_GL_FUNCTIONS_GL_VERSION_3_3 \
    X (z0        , glBindFragDataLocationIndexed, (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)) \
    X (GLint       , glGetFragDataIndex, (GLuint program, const GLchar *name)) \
    X (z0        , glGenSamplers, (GLsizei count, GLuint *samplers)) \
    X (z0        , glDeleteSamplers, (GLsizei count, const GLuint *samplers)) \
    X (GLboolean   , glIsSampler, (GLuint sampler)) \
    X (z0        , glBindSampler, (GLuint unit, GLuint sampler)) \
    X (z0        , glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param)) \
    X (z0        , glSamplerParameteriv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (z0        , glSamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param)) \
    X (z0        , glSamplerParameterfv, (GLuint sampler, GLenum pname, const GLfloat *param)) \
    X (z0        , glSamplerParameterIiv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (z0        , glSamplerParameterIuiv, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (z0        , glGetSamplerParameteriv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (z0        , glGetSamplerParameterIiv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (z0        , glGetSamplerParameterfv, (GLuint sampler, GLenum pname, GLfloat *params)) \
    X (z0        , glGetSamplerParameterIuiv, (GLuint sampler, GLenum pname, GLuint *params)) \
    X (z0        , glQueryCounter, (GLuint id, GLenum target)) \
    X (z0        , glGetQueryObjecti64v, (GLuint id, GLenum pname, GLint64 *params)) \
    X (z0        , glGetQueryObjectui64v, (GLuint id, GLenum pname, GLuint64 *params)) \
    X (z0        , glVertexAttribDivisor, (GLuint index, GLuint divisor)) \
    X (z0        , glVertexAttribP1ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (z0        , glVertexAttribP1uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (z0        , glVertexAttribP2ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (z0        , glVertexAttribP2uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (z0        , glVertexAttribP3ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (z0        , glVertexAttribP3uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (z0        , glVertexAttribP4ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (z0        , glVertexAttribP4uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (z0        , glVertexP2ui, (GLenum type, GLuint value)) \
    X (z0        , glVertexP2uiv, (GLenum type, const GLuint *value)) \
    X (z0        , glVertexP3ui, (GLenum type, GLuint value)) \
    X (z0        , glVertexP3uiv, (GLenum type, const GLuint *value)) \
    X (z0        , glVertexP4ui, (GLenum type, GLuint value)) \
    X (z0        , glVertexP4uiv, (GLenum type, const GLuint *value)) \
    X (z0        , glTexCoordP1ui, (GLenum type, GLuint coords)) \
    X (z0        , glTexCoordP1uiv, (GLenum type, const GLuint *coords)) \
    X (z0        , glTexCoordP2ui, (GLenum type, GLuint coords)) \
    X (z0        , glTexCoordP2uiv, (GLenum type, const GLuint *coords)) \
    X (z0        , glTexCoordP3ui, (GLenum type, GLuint coords)) \
    X (z0        , glTexCoordP3uiv, (GLenum type, const GLuint *coords)) \
    X (z0        , glTexCoordP4ui, (GLenum type, GLuint coords)) \
    X (z0        , glTexCoordP4uiv, (GLenum type, const GLuint *coords)) \
    X (z0        , glMultiTexCoordP1ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (z0        , glMultiTexCoordP1uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (z0        , glMultiTexCoordP2ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (z0        , glMultiTexCoordP2uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (z0        , glMultiTexCoordP3ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (z0        , glMultiTexCoordP3uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (z0        , glMultiTexCoordP4ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (z0        , glMultiTexCoordP4uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (z0        , glNormalP3ui, (GLenum type, GLuint coords)) \
    X (z0        , glNormalP3uiv, (GLenum type, const GLuint *coords)) \
    X (z0        , glColorP3ui, (GLenum type, GLuint color)) \
    X (z0        , glColorP3uiv, (GLenum type, const GLuint *color)) \
    X (z0        , glColorP4ui, (GLenum type, GLuint color)) \
    X (z0        , glColorP4uiv, (GLenum type, const GLuint *color)) \
    X (z0        , glSecondaryColorP3ui, (GLenum type, GLuint color)) \
    X (z0        , glSecondaryColorP3uiv, (GLenum type, const GLuint *color))

#define DRX_GL_FUNCTIONS_GL_VERSION_4_0 \
    X (z0        , glMinSampleShading, (GLfloat value)) \
    X (z0        , glBlendEquationi, (GLuint buf, GLenum mode)) \
    X (z0        , glBlendEquationSeparatei, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glBlendFunci, (GLuint buf, GLenum src, GLenum dst)) \
    X (z0        , glBlendFuncSeparatei, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (z0        , glDrawArraysIndirect, (GLenum mode, ukk indirect)) \
    X (z0        , glDrawElementsIndirect, (GLenum mode, GLenum type, ukk indirect)) \
    X (z0        , glUniform1d, (GLint location, GLdouble x)) \
    X (z0        , glUniform2d, (GLint location, GLdouble x, GLdouble y)) \
    X (z0        , glUniform3d, (GLint location, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glUniform4d, (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glUniform1dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glUniform2dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glUniform3dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glUniform4dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glUniformMatrix2dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix3dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix4dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix2x3dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix2x4dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix3x2dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix3x4dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix4x2dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glUniformMatrix4x3dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glGetUniformdv, (GLuint program, GLint location, GLdouble *params)) \
    X (GLint       , glGetSubroutineUniformLocation, (GLuint program, GLenum shadertype, const GLchar *name)) \
    X (GLuint      , glGetSubroutineIndex, (GLuint program, GLenum shadertype, const GLchar *name)) \
    X (z0        , glGetActiveSubroutineUniformiv, (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)) \
    X (z0        , glGetActiveSubroutineUniformName, (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (z0        , glGetActiveSubroutineName, (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (z0        , glUniformSubroutinesuiv, (GLenum shadertype, GLsizei count, const GLuint *indices)) \
    X (z0        , glGetUniformSubroutineuiv, (GLenum shadertype, GLint location, GLuint *params)) \
    X (z0        , glGetProgramStageiv, (GLuint program, GLenum shadertype, GLenum pname, GLint *values)) \
    X (z0        , glPatchParameteri, (GLenum pname, GLint value)) \
    X (z0        , glPatchParameterfv, (GLenum pname, const GLfloat *values)) \
    X (z0        , glBindTransformFeedback, (GLenum target, GLuint id)) \
    X (z0        , glDeleteTransformFeedbacks, (GLsizei n, const GLuint *ids)) \
    X (z0        , glGenTransformFeedbacks, (GLsizei n, GLuint *ids)) \
    X (GLboolean   , glIsTransformFeedback, (GLuint id)) \
    X (z0        , glPauseTransformFeedback, ()) \
    X (z0        , glResumeTransformFeedback, ()) \
    X (z0        , glDrawTransformFeedback, (GLenum mode, GLuint id)) \
    X (z0        , glDrawTransformFeedbackStream, (GLenum mode, GLuint id, GLuint stream)) \
    X (z0        , glBeginQueryIndexed, (GLenum target, GLuint index, GLuint id)) \
    X (z0        , glEndQueryIndexed, (GLenum target, GLuint index)) \
    X (z0        , glGetQueryIndexediv, (GLenum target, GLuint index, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_VERSION_4_1 \
    X (z0        , glReleaseShaderCompiler, ()) \
    X (z0        , glShaderBinary, (GLsizei count, const GLuint *shaders, GLenum binaryFormat, ukk binary, GLsizei length)) \
    X (z0        , glGetShaderPrecisionFormat, (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)) \
    X (z0        , glDepthRangef, (GLfloat n, GLfloat f)) \
    X (z0        , glClearDepthf, (GLfloat d)) \
    X (z0        , glGetProgramBinary, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, uk binary)) \
    X (z0        , glProgramBinary, (GLuint program, GLenum binaryFormat, ukk binary, GLsizei length)) \
    X (z0        , glProgramParameteri, (GLuint program, GLenum pname, GLint value)) \
    X (z0        , glUseProgramStages, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (z0        , glActiveShaderProgram, (GLuint pipeline, GLuint program)) \
    X (GLuint      , glCreateShaderProgramv, (GLenum type, GLsizei count, const GLchar *const*strings)) \
    X (z0        , glBindProgramPipeline, (GLuint pipeline)) \
    X (z0        , glDeleteProgramPipelines, (GLsizei n, const GLuint *pipelines)) \
    X (z0        , glGenProgramPipelines, (GLsizei n, GLuint *pipelines)) \
    X (GLboolean   , glIsProgramPipeline, (GLuint pipeline)) \
    X (z0        , glGetProgramPipelineiv, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (z0        , glProgramUniform1i, (GLuint program, GLint location, GLint v0)) \
    X (z0        , glProgramUniform1iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform1f, (GLuint program, GLint location, GLfloat v0)) \
    X (z0        , glProgramUniform1fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform1d, (GLuint program, GLint location, GLdouble v0)) \
    X (z0        , glProgramUniform1dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform1ui, (GLuint program, GLint location, GLuint v0)) \
    X (z0        , glProgramUniform1uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform2i, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (z0        , glProgramUniform2iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform2f, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glProgramUniform2fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform2d, (GLuint program, GLint location, GLdouble v0, GLdouble v1)) \
    X (z0        , glProgramUniform2dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform2ui, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glProgramUniform2uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform3i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glProgramUniform3iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform3f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glProgramUniform3fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform3d, (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)) \
    X (z0        , glProgramUniform3dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform3ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glProgramUniform3uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform4i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glProgramUniform4iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform4f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glProgramUniform4fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform4d, (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)) \
    X (z0        , glProgramUniform4dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform4ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glProgramUniform4uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniformMatrix2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix3dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix4dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix2x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x3dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix3x2dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix2x4dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix4x2dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix3x4dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix4x3dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glValidateProgramPipeline, (GLuint pipeline)) \
    X (z0        , glGetProgramPipelineInfoLog, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (z0        , glVertexAttribL1d, (GLuint index, GLdouble x)) \
    X (z0        , glVertexAttribL2d, (GLuint index, GLdouble x, GLdouble y)) \
    X (z0        , glVertexAttribL3d, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertexAttribL4d, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertexAttribL1dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribL2dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribL3dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribL4dv, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribLPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glGetVertexAttribLdv, (GLuint index, GLenum pname, GLdouble *params)) \
    X (z0        , glViewportArrayv, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (z0        , glViewportIndexedf, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)) \
    X (z0        , glViewportIndexedfv, (GLuint index, const GLfloat *v)) \
    X (z0        , glScissorArrayv, (GLuint first, GLsizei count, const GLint *v)) \
    X (z0        , glScissorIndexed, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)) \
    X (z0        , glScissorIndexedv, (GLuint index, const GLint *v)) \
    X (z0        , glDepthRangeArrayv, (GLuint first, GLsizei count, const GLdouble *v)) \
    X (z0        , glDepthRangeIndexed, (GLuint index, GLdouble n, GLdouble f)) \
    X (z0        , glGetFloati_v, (GLenum target, GLuint index, GLfloat *data)) \
    X (z0        , glGetDoublei_v, (GLenum target, GLuint index, GLdouble *data))

#define DRX_GL_FUNCTIONS_GL_VERSION_4_2 \
    X (z0        , glDrawArraysInstancedBaseInstance, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)) \
    X (z0        , glDrawElementsInstancedBaseInstance, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLuint baseinstance)) \
    X (z0        , glDrawElementsInstancedBaseVertexBaseInstance, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)) \
    X (z0        , glGetInternalformativ, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params)) \
    X (z0        , glGetActiveAtomicCounterBufferiv, (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params)) \
    X (z0        , glBindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)) \
    X (z0        , glMemoryBarrier, (GLbitfield barriers)) \
    X (z0        , glTexStorage1D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (z0        , glTexStorage2D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTexStorage3D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glDrawTransformFeedbackInstanced, (GLenum mode, GLuint id, GLsizei instancecount)) \
    X (z0        , glDrawTransformFeedbackStreamInstanced, (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount))

#define DRX_GL_FUNCTIONS_GL_VERSION_4_3 \
    X (z0        , glClearBufferData, (GLenum target, GLenum internalformat, GLenum format, GLenum type, ukk data)) \
    X (z0        , glClearBufferSubData, (GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, ukk data)) \
    X (z0        , glDispatchCompute, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)) \
    X (z0        , glDispatchComputeIndirect, (GLintptr indirect)) \
    X (z0        , glCopyImageSubData, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)) \
    X (z0        , glFramebufferParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glGetFramebufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetInternalformati64v, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params)) \
    X (z0        , glInvalidateTexSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glInvalidateTexImage, (GLuint texture, GLint level)) \
    X (z0        , glInvalidateBufferSubData, (GLuint buffer, GLintptr offset, GLsizeiptr length)) \
    X (z0        , glInvalidateBufferData, (GLuint buffer)) \
    X (z0        , glInvalidateFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments)) \
    X (z0        , glInvalidateSubFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glMultiDrawArraysIndirect, (GLenum mode, ukk indirect, GLsizei drawcount, GLsizei stride)) \
    X (z0        , glMultiDrawElementsIndirect, (GLenum mode, GLenum type, ukk indirect, GLsizei drawcount, GLsizei stride)) \
    X (z0        , glGetProgramInterfaceiv, (GLuint program, GLenum programInterface, GLenum pname, GLint *params)) \
    X (GLuint      , glGetProgramResourceIndex, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (z0        , glGetProgramResourceName, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (z0        , glGetProgramResourceiv, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params)) \
    X (GLint       , glGetProgramResourceLocation, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (GLint       , glGetProgramResourceLocationIndex, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (z0        , glShaderStorageBlockBinding, (GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)) \
    X (z0        , glTexBufferRange, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glTexStorage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (z0        , glTexStorage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (z0        , glTextureView, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)) \
    X (z0        , glBindVertexBuffer, (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (z0        , glVertexAttribFormat, (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (z0        , glVertexAttribIFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexAttribLFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexAttribBinding, (GLuint attribindex, GLuint bindingindex)) \
    X (z0        , glVertexBindingDivisor, (GLuint bindingindex, GLuint divisor)) \
    X (z0        , glDebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (z0        , glDebugMessageInsert, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (z0        , glDebugMessageCallback, (GLDEBUGPROC callback, ukk userParam)) \
    X (GLuint      , glGetDebugMessageLog, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)) \
    X (z0        , glPushDebugGroup, (GLenum source, GLuint id, GLsizei length, const GLchar *message)) \
    X (z0        , glPopDebugGroup, ()) \
    X (z0        , glObjectLabel, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectLabel, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (z0        , glObjectPtrLabel, (ukk ptr, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectPtrLabel, (ukk ptr, GLsizei bufSize, GLsizei *length, GLchar *label))

#define DRX_GL_FUNCTIONS_GL_VERSION_4_4 \
    X (z0        , glBufferStorage, (GLenum target, GLsizeiptr size, ukk data, GLbitfield flags)) \
    X (z0        , glClearTexImage, (GLuint texture, GLint level, GLenum format, GLenum type, ukk data)) \
    X (z0        , glClearTexSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk data)) \
    X (z0        , glBindBuffersBase, (GLenum target, GLuint first, GLsizei count, const GLuint *buffers)) \
    X (z0        , glBindBuffersRange, (GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes)) \
    X (z0        , glBindTextures, (GLuint first, GLsizei count, const GLuint *textures)) \
    X (z0        , glBindSamplers, (GLuint first, GLsizei count, const GLuint *samplers)) \
    X (z0        , glBindImageTextures, (GLuint first, GLsizei count, const GLuint *textures)) \
    X (z0        , glBindVertexBuffers, (GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides))

#define DRX_GL_FUNCTIONS_GL_VERSION_4_5 \
    X (z0        , glClipControl, (GLenum origin, GLenum depth)) \
    X (z0        , glCreateTransformFeedbacks, (GLsizei n, GLuint *ids)) \
    X (z0        , glTransformFeedbackBufferBase, (GLuint xfb, GLuint index, GLuint buffer)) \
    X (z0        , glTransformFeedbackBufferRange, (GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glGetTransformFeedbackiv, (GLuint xfb, GLenum pname, GLint *param)) \
    X (z0        , glGetTransformFeedbacki_v, (GLuint xfb, GLenum pname, GLuint index, GLint *param)) \
    X (z0        , glGetTransformFeedbacki64_v, (GLuint xfb, GLenum pname, GLuint index, GLint64 *param)) \
    X (z0        , glCreateBuffers, (GLsizei n, GLuint *buffers)) \
    X (z0        , glNamedBufferStorage, (GLuint buffer, GLsizeiptr size, ukk data, GLbitfield flags)) \
    X (z0        , glNamedBufferData, (GLuint buffer, GLsizeiptr size, ukk data, GLenum usage)) \
    X (z0        , glNamedBufferSubData, (GLuint buffer, GLintptr offset, GLsizeiptr size, ukk data)) \
    X (z0        , glCopyNamedBufferSubData, (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (z0        , glClearNamedBufferData, (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, ukk data)) \
    X (z0        , glClearNamedBufferSubData, (GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, ukk data)) \
    X (uk       , glMapNamedBuffer, (GLuint buffer, GLenum access)) \
    X (uk       , glMapNamedBufferRange, (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (GLboolean   , glUnmapNamedBuffer, (GLuint buffer)) \
    X (z0        , glFlushMappedNamedBufferRange, (GLuint buffer, GLintptr offset, GLsizeiptr length)) \
    X (z0        , glGetNamedBufferParameteriv, (GLuint buffer, GLenum pname, GLint *params)) \
    X (z0        , glGetNamedBufferParameteri64v, (GLuint buffer, GLenum pname, GLint64 *params)) \
    X (z0        , glGetNamedBufferPointerv, (GLuint buffer, GLenum pname, uk *params)) \
    X (z0        , glGetNamedBufferSubData, (GLuint buffer, GLintptr offset, GLsizeiptr size, uk data)) \
    X (z0        , glCreateFramebuffers, (GLsizei n, GLuint *framebuffers)) \
    X (z0        , glNamedFramebufferRenderbuffer, (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (z0        , glNamedFramebufferParameteri, (GLuint framebuffer, GLenum pname, GLint param)) \
    X (z0        , glNamedFramebufferTexture, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)) \
    X (z0        , glNamedFramebufferTextureLayer, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (z0        , glNamedFramebufferDrawBuffer, (GLuint framebuffer, GLenum buf)) \
    X (z0        , glNamedFramebufferDrawBuffers, (GLuint framebuffer, GLsizei n, const GLenum *bufs)) \
    X (z0        , glNamedFramebufferReadBuffer, (GLuint framebuffer, GLenum src)) \
    X (z0        , glInvalidateNamedFramebufferData, (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments)) \
    X (z0        , glInvalidateNamedFramebufferSubData, (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glClearNamedFramebufferiv, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value)) \
    X (z0        , glClearNamedFramebufferuiv, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value)) \
    X (z0        , glClearNamedFramebufferfv, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value)) \
    X (z0        , glClearNamedFramebufferfi, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
    X (z0        , glBlitNamedFramebuffer, (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (GLenum      , glCheckNamedFramebufferStatus, (GLuint framebuffer, GLenum target)) \
    X (z0        , glGetNamedFramebufferParameteriv, (GLuint framebuffer, GLenum pname, GLint *param)) \
    X (z0        , glGetNamedFramebufferAttachmentParameteriv, (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)) \
    X (z0        , glCreateRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
    X (z0        , glNamedRenderbufferStorage, (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glNamedRenderbufferStorageMultisample, (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glGetNamedRenderbufferParameteriv, (GLuint renderbuffer, GLenum pname, GLint *params)) \
    X (z0        , glCreateTextures, (GLenum target, GLsizei n, GLuint *textures)) \
    X (z0        , glTextureBuffer, (GLuint texture, GLenum internalformat, GLuint buffer)) \
    X (z0        , glTextureBufferRange, (GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glTextureStorage1D, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (z0        , glTextureStorage2D, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTextureStorage3D, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glTextureStorage2DMultisample, (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (z0        , glTextureStorage3DMultisample, (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (z0        , glTextureSubImage1D, (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTextureSubImage2D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTextureSubImage3D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCompressedTextureSubImage1D, (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTextureSubImage2D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTextureSubImage3D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCopyTextureSubImage1D, (GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyTextureSubImage2D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glCopyTextureSubImage3D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glTextureParameterf, (GLuint texture, GLenum pname, GLfloat param)) \
    X (z0        , glTextureParameterfv, (GLuint texture, GLenum pname, const GLfloat *param)) \
    X (z0        , glTextureParameteri, (GLuint texture, GLenum pname, GLint param)) \
    X (z0        , glTextureParameterIiv, (GLuint texture, GLenum pname, const GLint *params)) \
    X (z0        , glTextureParameterIuiv, (GLuint texture, GLenum pname, const GLuint *params)) \
    X (z0        , glTextureParameteriv, (GLuint texture, GLenum pname, const GLint *param)) \
    X (z0        , glGenerateTextureMipmap, (GLuint texture)) \
    X (z0        , glBindTextureUnit, (GLuint unit, GLuint texture)) \
    X (z0        , glGetTextureImage, (GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, uk pixels)) \
    X (z0        , glGetCompressedTextureImage, (GLuint texture, GLint level, GLsizei bufSize, uk pixels)) \
    X (z0        , glGetTextureLevelParameterfv, (GLuint texture, GLint level, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTextureLevelParameteriv, (GLuint texture, GLint level, GLenum pname, GLint *params)) \
    X (z0        , glGetTextureParameterfv, (GLuint texture, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTextureParameterIiv, (GLuint texture, GLenum pname, GLint *params)) \
    X (z0        , glGetTextureParameterIuiv, (GLuint texture, GLenum pname, GLuint *params)) \
    X (z0        , glGetTextureParameteriv, (GLuint texture, GLenum pname, GLint *params)) \
    X (z0        , glCreateVertexArrays, (GLsizei n, GLuint *arrays)) \
    X (z0        , glDisableVertexArrayAttrib, (GLuint vaobj, GLuint index)) \
    X (z0        , glEnableVertexArrayAttrib, (GLuint vaobj, GLuint index)) \
    X (z0        , glVertexArrayElementBuffer, (GLuint vaobj, GLuint buffer)) \
    X (z0        , glVertexArrayVertexBuffer, (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (z0        , glVertexArrayVertexBuffers, (GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)) \
    X (z0        , glVertexArrayAttribBinding, (GLuint vaobj, GLuint attribindex, GLuint bindingindex)) \
    X (z0        , glVertexArrayAttribFormat, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (z0        , glVertexArrayAttribIFormat, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexArrayAttribLFormat, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexArrayBindingDivisor, (GLuint vaobj, GLuint bindingindex, GLuint divisor)) \
    X (z0        , glGetVertexArrayiv, (GLuint vaobj, GLenum pname, GLint *param)) \
    X (z0        , glGetVertexArrayIndexediv, (GLuint vaobj, GLuint index, GLenum pname, GLint *param)) \
    X (z0        , glGetVertexArrayIndexed64iv, (GLuint vaobj, GLuint index, GLenum pname, GLint64 *param)) \
    X (z0        , glCreateSamplers, (GLsizei n, GLuint *samplers)) \
    X (z0        , glCreateProgramPipelines, (GLsizei n, GLuint *pipelines)) \
    X (z0        , glCreateQueries, (GLenum target, GLsizei n, GLuint *ids)) \
    X (z0        , glGetQueryBufferObjecti64v, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (z0        , glGetQueryBufferObjectiv, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (z0        , glGetQueryBufferObjectui64v, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (z0        , glGetQueryBufferObjectuiv, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (z0        , glMemoryBarrierByRegion, (GLbitfield barriers)) \
    X (z0        , glGetTextureSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, uk pixels)) \
    X (z0        , glGetCompressedTextureSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, uk pixels)) \
    X (GLenum      , glGetGraphicsResetStatus, ()) \
    X (z0        , glGetnCompressedTexImage, (GLenum target, GLint lod, GLsizei bufSize, uk pixels)) \
    X (z0        , glGetnTexImage, (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, uk pixels)) \
    X (z0        , glGetnUniformdv, (GLuint program, GLint location, GLsizei bufSize, GLdouble *params)) \
    X (z0        , glGetnUniformfv, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (z0        , glGetnUniformiv, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (z0        , glGetnUniformuiv, (GLuint program, GLint location, GLsizei bufSize, GLuint *params)) \
    X (z0        , glReadnPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, uk data)) \
    X (z0        , glGetnMapdv, (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v)) \
    X (z0        , glGetnMapfv, (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v)) \
    X (z0        , glGetnMapiv, (GLenum target, GLenum query, GLsizei bufSize, GLint *v)) \
    X (z0        , glGetnPixelMapfv, (GLenum map, GLsizei bufSize, GLfloat *values)) \
    X (z0        , glGetnPixelMapuiv, (GLenum map, GLsizei bufSize, GLuint *values)) \
    X (z0        , glGetnPixelMapusv, (GLenum map, GLsizei bufSize, GLushort *values)) \
    X (z0        , glGetnPolygonStipple, (GLsizei bufSize, GLubyte *pattern)) \
    X (z0        , glGetnColorTable, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, uk table)) \
    X (z0        , glGetnConvolutionFilter, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, uk image)) \
    X (z0        , glGetnSeparableFilter, (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, uk row, GLsizei columnBufSize, uk column, uk span)) \
    X (z0        , glGetnHistogram, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, uk values)) \
    X (z0        , glGetnMinmax, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, uk values)) \
    X (z0        , glTextureBarrier, ())

#define DRX_GL_FUNCTIONS_GL_VERSION_4_6 \
    X (z0        , glSpecializeShader, (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue)) \
    X (z0        , glMultiDrawArraysIndirectCount, (GLenum mode, ukk indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)) \
    X (z0        , glMultiDrawElementsIndirectCount, (GLenum mode, GLenum type, ukk indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)) \
    X (z0        , glPolygonOffsetClamp, (GLfloat factor, GLfloat units, GLfloat clamp))

#define DRX_GL_FUNCTIONS_GL_3DFX_tbuffer \
    X (z0        , glTbufferMask3DFX, (GLuint mask))

#define DRX_GL_FUNCTIONS_GL_AMD_debug_output \
    X (z0        , glDebugMessageEnableAMD, (GLenum category, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (z0        , glDebugMessageInsertAMD, (GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar *buf)) \
    X (z0        , glDebugMessageCallbackAMD, (GLDEBUGPROCAMD callback, uk userParam)) \
    X (GLuint      , glGetDebugMessageLogAMD, (GLuint count, GLsizei bufSize, GLenum *categories, GLenum *severities, GLuint *ids, GLsizei *lengths, GLchar *message))

#define DRX_GL_FUNCTIONS_GL_AMD_draw_buffers_blend \
    X (z0        , glBlendFuncIndexedAMD, (GLuint buf, GLenum src, GLenum dst)) \
    X (z0        , glBlendFuncSeparateIndexedAMD, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (z0        , glBlendEquationIndexedAMD, (GLuint buf, GLenum mode)) \
    X (z0        , glBlendEquationSeparateIndexedAMD, (GLuint buf, GLenum modeRGB, GLenum modeAlpha))

#define DRX_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    X (z0        , glRenderbufferStorageMultisampleAdvancedAMD, (GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glNamedRenderbufferStorageMultisampleAdvancedAMD, (GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_AMD_framebuffer_sample_positions \
    X (z0        , glFramebufferSamplePositionsfvAMD, (GLenum target, GLuint numsamples, GLuint pixelindex, const GLfloat *values)) \
    X (z0        , glNamedFramebufferSamplePositionsfvAMD, (GLuint framebuffer, GLuint numsamples, GLuint pixelindex, const GLfloat *values)) \
    X (z0        , glGetFramebufferParameterfvAMD, (GLenum target, GLenum pname, GLuint numsamples, GLuint pixelindex, GLsizei size, GLfloat *values)) \
    X (z0        , glGetNamedFramebufferParameterfvAMD, (GLuint framebuffer, GLenum pname, GLuint numsamples, GLuint pixelindex, GLsizei size, GLfloat *values))

#define DRX_GL_FUNCTIONS_GL_AMD_gpu_shader_int64 \
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
    X (z0        , glGetUniformui64vNV, (GLuint program, GLint location, GLuint64EXT *params)) \
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

#define DRX_GL_FUNCTIONS_GL_AMD_interleaved_elements \
    X (z0        , glVertexAttribParameteriAMD, (GLuint index, GLenum pname, GLint param))

#define DRX_GL_FUNCTIONS_GL_AMD_multi_draw_indirect \
    X (z0        , glMultiDrawArraysIndirectAMD, (GLenum mode, ukk indirect, GLsizei primcount, GLsizei stride)) \
    X (z0        , glMultiDrawElementsIndirectAMD, (GLenum mode, GLenum type, ukk indirect, GLsizei primcount, GLsizei stride))

#define DRX_GL_FUNCTIONS_GL_AMD_name_gen_delete \
    X (z0        , glGenNamesAMD, (GLenum identifier, GLuint num, GLuint *names)) \
    X (z0        , glDeleteNamesAMD, (GLenum identifier, GLuint num, const GLuint *names)) \
    X (GLboolean   , glIsNameAMD, (GLenum identifier, GLuint name))

#define DRX_GL_FUNCTIONS_GL_AMD_occlusion_query_event \
    X (z0        , glQueryObjectParameteruiAMD, (GLenum target, GLuint id, GLenum pname, GLuint param))

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

#define DRX_GL_FUNCTIONS_GL_AMD_sample_positions \
    X (z0        , glSetMultisamplefvAMD, (GLenum pname, GLuint index, const GLfloat *val))

#define DRX_GL_FUNCTIONS_GL_AMD_sparse_texture \
    X (z0        , glTexStorageSparseAMD, (GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags)) \
    X (z0        , glTextureStorageSparseAMD, (GLuint texture, GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags))

#define DRX_GL_FUNCTIONS_GL_AMD_stencil_operation_extended \
    X (z0        , glStencilOpValueAMD, (GLenum face, GLuint value))

#define DRX_GL_FUNCTIONS_GL_AMD_vertex_shader_tessellator \
    X (z0        , glTessellationFactorAMD, (GLfloat factor)) \
    X (z0        , glTessellationModeAMD, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_APPLE_element_array \
    X (z0        , glElementPointerAPPLE, (GLenum type, ukk pointer)) \
    X (z0        , glDrawElementArrayAPPLE, (GLenum mode, GLint first, GLsizei count)) \
    X (z0        , glDrawRangeElementArrayAPPLE, (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count)) \
    X (z0        , glMultiDrawElementArrayAPPLE, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)) \
    X (z0        , glMultiDrawRangeElementArrayAPPLE, (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount))

#define DRX_GL_FUNCTIONS_GL_APPLE_fence \
    X (z0        , glGenFencesAPPLE, (GLsizei n, GLuint *fences)) \
    X (z0        , glDeleteFencesAPPLE, (GLsizei n, const GLuint *fences)) \
    X (z0        , glSetFenceAPPLE, (GLuint fence)) \
    X (GLboolean   , glIsFenceAPPLE, (GLuint fence)) \
    X (GLboolean   , glTestFenceAPPLE, (GLuint fence)) \
    X (z0        , glFinishFenceAPPLE, (GLuint fence)) \
    X (GLboolean   , glTestObjectAPPLE, (GLenum object, GLuint name)) \
    X (z0        , glFinishObjectAPPLE, (GLenum object, GLint name))

#define DRX_GL_FUNCTIONS_GL_APPLE_flush_buffer_range \
    X (z0        , glBufferParameteriAPPLE, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glFlushMappedBufferRangeAPPLE, (GLenum target, GLintptr offset, GLsizeiptr size))

#define DRX_GL_FUNCTIONS_GL_APPLE_object_purgeable \
    X (GLenum      , glObjectPurgeableAPPLE, (GLenum objectType, GLuint name, GLenum option)) \
    X (GLenum      , glObjectUnpurgeableAPPLE, (GLenum objectType, GLuint name, GLenum option)) \
    X (z0        , glGetObjectParameterivAPPLE, (GLenum objectType, GLuint name, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_APPLE_texture_range \
    X (z0        , glTextureRangeAPPLE, (GLenum target, GLsizei length, ukk pointer)) \
    X (z0        , glGetTexParameterPointervAPPLE, (GLenum target, GLenum pname, uk *params))

#define DRX_GL_FUNCTIONS_GL_APPLE_vertex_array_object \
    X (z0        , glBindVertexArrayAPPLE, (GLuint array)) \
    X (z0        , glDeleteVertexArraysAPPLE, (GLsizei n, const GLuint *arrays)) \
    X (z0        , glGenVertexArraysAPPLE, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArrayAPPLE, (GLuint array))

#define DRX_GL_FUNCTIONS_GL_APPLE_vertex_array_range \
    X (z0        , glVertexArrayRangeAPPLE, (GLsizei length, uk pointer)) \
    X (z0        , glFlushVertexArrayRangeAPPLE, (GLsizei length, uk pointer)) \
    X (z0        , glVertexArrayParameteriAPPLE, (GLenum pname, GLint param))

#define DRX_GL_FUNCTIONS_GL_APPLE_vertex_program_evaluators \
    X (z0        , glEnableVertexAttribAPPLE, (GLuint index, GLenum pname)) \
    X (z0        , glDisableVertexAttribAPPLE, (GLuint index, GLenum pname)) \
    X (GLboolean   , glIsVertexAttribEnabledAPPLE, (GLuint index, GLenum pname)) \
    X (z0        , glMapVertexAttrib1dAPPLE, (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)) \
    X (z0        , glMapVertexAttrib1fAPPLE, (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)) \
    X (z0        , glMapVertexAttrib2dAPPLE, (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)) \
    X (z0        , glMapVertexAttrib2fAPPLE, (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points))

#define DRX_GL_FUNCTIONS_GL_ARB_ES3_2_compatibility \
    X (z0        , glPrimitiveBoundingBoxARB, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW))

#define DRX_GL_FUNCTIONS_GL_ARB_bindless_texture \
    X (GLuint64    , glGetTextureHandleARB, (GLuint texture)) \
    X (GLuint64    , glGetTextureSamplerHandleARB, (GLuint texture, GLuint sampler)) \
    X (z0        , glMakeTextureHandleResidentARB, (GLuint64 handle)) \
    X (z0        , glMakeTextureHandleNonResidentARB, (GLuint64 handle)) \
    X (GLuint64    , glGetImageHandleARB, (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)) \
    X (z0        , glMakeImageHandleResidentARB, (GLuint64 handle, GLenum access)) \
    X (z0        , glMakeImageHandleNonResidentARB, (GLuint64 handle)) \
    X (z0        , glUniformHandleui64ARB, (GLint location, GLuint64 value)) \
    X (z0        , glUniformHandleui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glProgramUniformHandleui64ARB, (GLuint program, GLint location, GLuint64 value)) \
    X (z0        , glProgramUniformHandleui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *values)) \
    X (GLboolean   , glIsTextureHandleResidentARB, (GLuint64 handle)) \
    X (GLboolean   , glIsImageHandleResidentARB, (GLuint64 handle)) \
    X (z0        , glVertexAttribL1ui64ARB, (GLuint index, GLuint64EXT x)) \
    X (z0        , glVertexAttribL1ui64vARB, (GLuint index, const GLuint64EXT *v)) \
    X (z0        , glGetVertexAttribLui64vARB, (GLuint index, GLenum pname, GLuint64EXT *params))

#define DRX_GL_FUNCTIONS_GL_ARB_cl_event \
    X (GLsync      , glCreateSyncFromCLeventARB, (struct _cl_context *context, struct _cl_event *event, GLbitfield flags))

#define DRX_GL_FUNCTIONS_GL_ARB_color_buffer_float \
    X (z0        , glClampColorARB, (GLenum target, GLenum clamp))

#define DRX_GL_FUNCTIONS_GL_ARB_compute_variable_group_size \
    X (z0        , glDispatchComputeGroupSizeARB, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z))

#define DRX_GL_FUNCTIONS_GL_ARB_debug_output \
    X (z0        , glDebugMessageControlARB, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (z0        , glDebugMessageInsertARB, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (z0        , glDebugMessageCallbackARB, (GLDEBUGPROCARB callback, ukk userParam)) \
    X (GLuint      , glGetDebugMessageLogARB, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog))

#define DRX_GL_FUNCTIONS_GL_ARB_draw_buffers \
    X (z0        , glDrawBuffersARB, (GLsizei n, const GLenum *bufs))

#define DRX_GL_FUNCTIONS_GL_ARB_draw_buffers_blend \
    X (z0        , glBlendEquationiARB, (GLuint buf, GLenum mode)) \
    X (z0        , glBlendEquationSeparateiARB, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (z0        , glBlendFunciARB, (GLuint buf, GLenum src, GLenum dst)) \
    X (z0        , glBlendFuncSeparateiARB, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha))

#define DRX_GL_FUNCTIONS_GL_ARB_draw_instanced \
    X (z0        , glDrawArraysInstancedARB, (GLenum mode, GLint first, GLsizei count, GLsizei primcount)) \
    X (z0        , glDrawElementsInstancedARB, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei primcount))

#define DRX_GL_FUNCTIONS_GL_ARB_fragment_program \
    X (z0        , glProgramStringARB, (GLenum target, GLenum format, GLsizei len, ukk string)) \
    X (z0        , glBindProgramARB, (GLenum target, GLuint program)) \
    X (z0        , glDeleteProgramsARB, (GLsizei n, const GLuint *programs)) \
    X (z0        , glGenProgramsARB, (GLsizei n, GLuint *programs)) \
    X (z0        , glProgramEnvParameter4dARB, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glProgramEnvParameter4dvARB, (GLenum target, GLuint index, const GLdouble *params)) \
    X (z0        , glProgramEnvParameter4fARB, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glProgramEnvParameter4fvARB, (GLenum target, GLuint index, const GLfloat *params)) \
    X (z0        , glProgramLocalParameter4dARB, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glProgramLocalParameter4dvARB, (GLenum target, GLuint index, const GLdouble *params)) \
    X (z0        , glProgramLocalParameter4fARB, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glProgramLocalParameter4fvARB, (GLenum target, GLuint index, const GLfloat *params)) \
    X (z0        , glGetProgramEnvParameterdvARB, (GLenum target, GLuint index, GLdouble *params)) \
    X (z0        , glGetProgramEnvParameterfvARB, (GLenum target, GLuint index, GLfloat *params)) \
    X (z0        , glGetProgramLocalParameterdvARB, (GLenum target, GLuint index, GLdouble *params)) \
    X (z0        , glGetProgramLocalParameterfvARB, (GLenum target, GLuint index, GLfloat *params)) \
    X (z0        , glGetProgramivARB, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetProgramStringARB, (GLenum target, GLenum pname, uk string)) \
    X (GLboolean   , glIsProgramARB, (GLuint program))

#define DRX_GL_FUNCTIONS_GL_ARB_geometry_shader4 \
    X (z0        , glProgramParameteriARB, (GLuint program, GLenum pname, GLint value)) \
    X (z0        , glFramebufferTextureARB, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (z0        , glFramebufferTextureLayerARB, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (z0        , glFramebufferTextureFaceARB, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face))

#define DRX_GL_FUNCTIONS_GL_ARB_gl_spirv \
    X (z0        , glSpecializeShaderARB, (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue))

#define DRX_GL_FUNCTIONS_GL_ARB_gpu_shader_int64 \
    X (z0        , glUniform1i64ARB, (GLint location, GLint64 x)) \
    X (z0        , glUniform2i64ARB, (GLint location, GLint64 x, GLint64 y)) \
    X (z0        , glUniform3i64ARB, (GLint location, GLint64 x, GLint64 y, GLint64 z)) \
    X (z0        , glUniform4i64ARB, (GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w)) \
    X (z0        , glUniform1i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glUniform2i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glUniform3i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glUniform4i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glUniform1ui64ARB, (GLint location, GLuint64 x)) \
    X (z0        , glUniform2ui64ARB, (GLint location, GLuint64 x, GLuint64 y)) \
    X (z0        , glUniform3ui64ARB, (GLint location, GLuint64 x, GLuint64 y, GLuint64 z)) \
    X (z0        , glUniform4ui64ARB, (GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w)) \
    X (z0        , glUniform1ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glUniform2ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glUniform3ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glUniform4ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glGetUniformi64vARB, (GLuint program, GLint location, GLint64 *params)) \
    X (z0        , glGetUniformui64vARB, (GLuint program, GLint location, GLuint64 *params)) \
    X (z0        , glGetnUniformi64vARB, (GLuint program, GLint location, GLsizei bufSize, GLint64 *params)) \
    X (z0        , glGetnUniformui64vARB, (GLuint program, GLint location, GLsizei bufSize, GLuint64 *params)) \
    X (z0        , glProgramUniform1i64ARB, (GLuint program, GLint location, GLint64 x)) \
    X (z0        , glProgramUniform2i64ARB, (GLuint program, GLint location, GLint64 x, GLint64 y)) \
    X (z0        , glProgramUniform3i64ARB, (GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z)) \
    X (z0        , glProgramUniform4i64ARB, (GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w)) \
    X (z0        , glProgramUniform1i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glProgramUniform2i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glProgramUniform3i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glProgramUniform4i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (z0        , glProgramUniform1ui64ARB, (GLuint program, GLint location, GLuint64 x)) \
    X (z0        , glProgramUniform2ui64ARB, (GLuint program, GLint location, GLuint64 x, GLuint64 y)) \
    X (z0        , glProgramUniform3ui64ARB, (GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z)) \
    X (z0        , glProgramUniform4ui64ARB, (GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w)) \
    X (z0        , glProgramUniform1ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glProgramUniform2ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glProgramUniform3ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value)) \
    X (z0        , glProgramUniform4ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value))

#define DRX_GL_FUNCTIONS_GL_ARB_imaging \
    X (z0        , glColorTable, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, ukk table)) \
    X (z0        , glColorTableParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glColorTableParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glCopyColorTable, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (z0        , glGetColorTable, (GLenum target, GLenum format, GLenum type, uk table)) \
    X (z0        , glGetColorTableParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetColorTableParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glColorSubTable, (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, ukk data)) \
    X (z0        , glCopyColorSubTable, (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)) \
    X (z0        , glConvolutionFilter1D, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, ukk image)) \
    X (z0        , glConvolutionFilter2D, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk image)) \
    X (z0        , glConvolutionParameterf, (GLenum target, GLenum pname, GLfloat params)) \
    X (z0        , glConvolutionParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glConvolutionParameteri, (GLenum target, GLenum pname, GLint params)) \
    X (z0        , glConvolutionParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glCopyConvolutionFilter1D, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyConvolutionFilter2D, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glGetConvolutionFilter, (GLenum target, GLenum format, GLenum type, uk image)) \
    X (z0        , glGetConvolutionParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetConvolutionParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetSeparableFilter, (GLenum target, GLenum format, GLenum type, uk row, uk column, uk span)) \
    X (z0        , glSeparableFilter2D, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk row, ukk column)) \
    X (z0        , glGetHistogram, (GLenum target, GLboolean reset, GLenum format, GLenum type, uk values)) \
    X (z0        , glGetHistogramParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetHistogramParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetMinmax, (GLenum target, GLboolean reset, GLenum format, GLenum type, uk values)) \
    X (z0        , glGetMinmaxParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMinmaxParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glHistogram, (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)) \
    X (z0        , glMinmax, (GLenum target, GLenum internalformat, GLboolean sink)) \
    X (z0        , glResetHistogram, (GLenum target)) \
    X (z0        , glResetMinmax, (GLenum target))

#define DRX_GL_FUNCTIONS_GL_ARB_indirect_parameters \
    X (z0        , glMultiDrawArraysIndirectCountARB, (GLenum mode, ukk indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)) \
    X (z0        , glMultiDrawElementsIndirectCountARB, (GLenum mode, GLenum type, ukk indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride))

#define DRX_GL_FUNCTIONS_GL_ARB_instanced_arrays \
    X (z0        , glVertexAttribDivisorARB, (GLuint index, GLuint divisor))

#define DRX_GL_FUNCTIONS_GL_ARB_matrix_palette \
    X (z0        , glCurrentPaletteMatrixARB, (GLint index)) \
    X (z0        , glMatrixIndexubvARB, (GLint size, const GLubyte *indices)) \
    X (z0        , glMatrixIndexusvARB, (GLint size, const GLushort *indices)) \
    X (z0        , glMatrixIndexuivARB, (GLint size, const GLuint *indices)) \
    X (z0        , glMatrixIndexPointerARB, (GLint size, GLenum type, GLsizei stride, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_ARB_multisample \
    X (z0        , glSampleCoverageARB, (GLfloat value, GLboolean invert))

#define DRX_GL_FUNCTIONS_GL_ARB_multitexture \
    X (z0        , glActiveTextureARB, (GLenum texture)) \
    X (z0        , glClientActiveTextureARB, (GLenum texture)) \
    X (z0        , glMultiTexCoord1dARB, (GLenum target, GLdouble s)) \
    X (z0        , glMultiTexCoord1dvARB, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord1fARB, (GLenum target, GLfloat s)) \
    X (z0        , glMultiTexCoord1fvARB, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord1iARB, (GLenum target, GLint s)) \
    X (z0        , glMultiTexCoord1ivARB, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord1sARB, (GLenum target, GLshort s)) \
    X (z0        , glMultiTexCoord1svARB, (GLenum target, const GLshort *v)) \
    X (z0        , glMultiTexCoord2dARB, (GLenum target, GLdouble s, GLdouble t)) \
    X (z0        , glMultiTexCoord2dvARB, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord2fARB, (GLenum target, GLfloat s, GLfloat t)) \
    X (z0        , glMultiTexCoord2fvARB, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord2iARB, (GLenum target, GLint s, GLint t)) \
    X (z0        , glMultiTexCoord2ivARB, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord2sARB, (GLenum target, GLshort s, GLshort t)) \
    X (z0        , glMultiTexCoord2svARB, (GLenum target, const GLshort *v)) \
    X (z0        , glMultiTexCoord3dARB, (GLenum target, GLdouble s, GLdouble t, GLdouble r)) \
    X (z0        , glMultiTexCoord3dvARB, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord3fARB, (GLenum target, GLfloat s, GLfloat t, GLfloat r)) \
    X (z0        , glMultiTexCoord3fvARB, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord3iARB, (GLenum target, GLint s, GLint t, GLint r)) \
    X (z0        , glMultiTexCoord3ivARB, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord3sARB, (GLenum target, GLshort s, GLshort t, GLshort r)) \
    X (z0        , glMultiTexCoord3svARB, (GLenum target, const GLshort *v)) \
    X (z0        , glMultiTexCoord4dARB, (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)) \
    X (z0        , glMultiTexCoord4dvARB, (GLenum target, const GLdouble *v)) \
    X (z0        , glMultiTexCoord4fARB, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)) \
    X (z0        , glMultiTexCoord4fvARB, (GLenum target, const GLfloat *v)) \
    X (z0        , glMultiTexCoord4iARB, (GLenum target, GLint s, GLint t, GLint r, GLint q)) \
    X (z0        , glMultiTexCoord4ivARB, (GLenum target, const GLint *v)) \
    X (z0        , glMultiTexCoord4sARB, (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)) \
    X (z0        , glMultiTexCoord4svARB, (GLenum target, const GLshort *v))

#define DRX_GL_FUNCTIONS_GL_ARB_occlusion_query \
    X (z0        , glGenQueriesARB, (GLsizei n, GLuint *ids)) \
    X (z0        , glDeleteQueriesARB, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQueryARB, (GLuint id)) \
    X (z0        , glBeginQueryARB, (GLenum target, GLuint id)) \
    X (z0        , glEndQueryARB, (GLenum target)) \
    X (z0        , glGetQueryivARB, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectivARB, (GLuint id, GLenum pname, GLint *params)) \
    X (z0        , glGetQueryObjectuivARB, (GLuint id, GLenum pname, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_ARB_parallel_shader_compile \
    X (z0        , glMaxShaderCompilerThreadsARB, (GLuint count))

#define DRX_GL_FUNCTIONS_GL_ARB_point_parameters \
    X (z0        , glPointParameterfARB, (GLenum pname, GLfloat param)) \
    X (z0        , glPointParameterfvARB, (GLenum pname, const GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_ARB_robustness \
    X (GLenum      , glGetGraphicsResetStatusARB, ()) \
    X (z0        , glGetnTexImageARB, (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, uk img)) \
    X (z0        , glReadnPixelsARB, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, uk data)) \
    X (z0        , glGetnCompressedTexImageARB, (GLenum target, GLint lod, GLsizei bufSize, uk img)) \
    X (z0        , glGetnUniformfvARB, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (z0        , glGetnUniformivARB, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (z0        , glGetnUniformuivARB, (GLuint program, GLint location, GLsizei bufSize, GLuint *params)) \
    X (z0        , glGetnUniformdvARB, (GLuint program, GLint location, GLsizei bufSize, GLdouble *params)) \
    X (z0        , glGetnMapdvARB, (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v)) \
    X (z0        , glGetnMapfvARB, (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v)) \
    X (z0        , glGetnMapivARB, (GLenum target, GLenum query, GLsizei bufSize, GLint *v)) \
    X (z0        , glGetnPixelMapfvARB, (GLenum map, GLsizei bufSize, GLfloat *values)) \
    X (z0        , glGetnPixelMapuivARB, (GLenum map, GLsizei bufSize, GLuint *values)) \
    X (z0        , glGetnPixelMapusvARB, (GLenum map, GLsizei bufSize, GLushort *values)) \
    X (z0        , glGetnPolygonStippleARB, (GLsizei bufSize, GLubyte *pattern)) \
    X (z0        , glGetnColorTableARB, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, uk table)) \
    X (z0        , glGetnConvolutionFilterARB, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, uk image)) \
    X (z0        , glGetnSeparableFilterARB, (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, uk row, GLsizei columnBufSize, uk column, uk span)) \
    X (z0        , glGetnHistogramARB, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, uk values)) \
    X (z0        , glGetnMinmaxARB, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, uk values))

#define DRX_GL_FUNCTIONS_GL_ARB_sample_locations \
    X (z0        , glFramebufferSampleLocationsfvARB, (GLenum target, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glNamedFramebufferSampleLocationsfvARB, (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glEvaluateDepthValuesARB, ())

#define DRX_GL_FUNCTIONS_GL_ARB_sample_shading \
    X (z0        , glMinSampleShadingARB, (GLfloat value))

#define DRX_GL_FUNCTIONS_GL_ARB_shader_objects \
    X (z0        , glDeleteObjectARB, (GLhandleARB obj)) \
    X (GLhandleARB , glGetHandleARB, (GLenum pname)) \
    X (z0        , glDetachObjectARB, (GLhandleARB containerObj, GLhandleARB attachedObj)) \
    X (GLhandleARB , glCreateShaderObjectARB, (GLenum shaderType)) \
    X (z0        , glShaderSourceARB, (GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length)) \
    X (z0        , glCompileShaderARB, (GLhandleARB shaderObj)) \
    X (GLhandleARB , glCreateProgramObjectARB, ()) \
    X (z0        , glAttachObjectARB, (GLhandleARB containerObj, GLhandleARB obj)) \
    X (z0        , glLinkProgramARB, (GLhandleARB programObj)) \
    X (z0        , glUseProgramObjectARB, (GLhandleARB programObj)) \
    X (z0        , glValidateProgramARB, (GLhandleARB programObj)) \
    X (z0        , glUniform1fARB, (GLint location, GLfloat v0)) \
    X (z0        , glUniform2fARB, (GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glUniform3fARB, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glUniform4fARB, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glUniform1iARB, (GLint location, GLint v0)) \
    X (z0        , glUniform2iARB, (GLint location, GLint v0, GLint v1)) \
    X (z0        , glUniform3iARB, (GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glUniform4iARB, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glUniform1fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform2fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform3fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform4fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glUniform1ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform2ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform3ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniform4ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glUniformMatrix2fvARB, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix3fvARB, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glUniformMatrix4fvARB, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glGetObjectParameterfvARB, (GLhandleARB obj, GLenum pname, GLfloat *params)) \
    X (z0        , glGetObjectParameterivARB, (GLhandleARB obj, GLenum pname, GLint *params)) \
    X (z0        , glGetInfoLogARB, (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)) \
    X (z0        , glGetAttachedObjectsARB, (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj)) \
    X (GLint       , glGetUniformLocationARB, (GLhandleARB programObj, const GLcharARB *name)) \
    X (z0        , glGetActiveUniformARB, (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name)) \
    X (z0        , glGetUniformfvARB, (GLhandleARB programObj, GLint location, GLfloat *params)) \
    X (z0        , glGetUniformivARB, (GLhandleARB programObj, GLint location, GLint *params)) \
    X (z0        , glGetShaderSourceARB, (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source))

#define DRX_GL_FUNCTIONS_GL_ARB_shading_language_include \
    X (z0        , glNamedStringARB, (GLenum type, GLint namelen, const GLchar *name, GLint stringlen, const GLchar *string)) \
    X (z0        , glDeleteNamedStringARB, (GLint namelen, const GLchar *name)) \
    X (z0        , glCompileShaderIncludeARB, (GLuint shader, GLsizei count, const GLchar *const*path, const GLint *length)) \
    X (GLboolean   , glIsNamedStringARB, (GLint namelen, const GLchar *name)) \
    X (z0        , glGetNamedStringARB, (GLint namelen, const GLchar *name, GLsizei bufSize, GLint *stringlen, GLchar *string)) \
    X (z0        , glGetNamedStringivARB, (GLint namelen, const GLchar *name, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_ARB_sparse_buffer \
    X (z0        , glBufferPageCommitmentARB, (GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit)) \
    X (z0        , glNamedBufferPageCommitmentEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit)) \
    X (z0        , glNamedBufferPageCommitmentARB, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit))

#define DRX_GL_FUNCTIONS_GL_ARB_sparse_texture \
    X (z0        , glTexPageCommitmentARB, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit))

#define DRX_GL_FUNCTIONS_GL_ARB_texture_buffer_object \
    X (z0        , glTexBufferARB, (GLenum target, GLenum internalformat, GLuint buffer))

#define DRX_GL_FUNCTIONS_GL_ARB_texture_compression \
    X (z0        , glCompressedTexImage3DARB, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexImage2DARB, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexImage1DARB, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage3DARB, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage2DARB, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glCompressedTexSubImage1DARB, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, ukk data)) \
    X (z0        , glGetCompressedTexImageARB, (GLenum target, GLint level, uk img))

#define DRX_GL_FUNCTIONS_GL_ARB_transpose_matrix \
    X (z0        , glLoadTransposeMatrixfARB, (const GLfloat *m)) \
    X (z0        , glLoadTransposeMatrixdARB, (const GLdouble *m)) \
    X (z0        , glMultTransposeMatrixfARB, (const GLfloat *m)) \
    X (z0        , glMultTransposeMatrixdARB, (const GLdouble *m))

#define DRX_GL_FUNCTIONS_GL_ARB_vertex_blend \
    X (z0        , glWeightbvARB, (GLint size, const GLbyte *weights)) \
    X (z0        , glWeightsvARB, (GLint size, const GLshort *weights)) \
    X (z0        , glWeightivARB, (GLint size, const GLint *weights)) \
    X (z0        , glWeightfvARB, (GLint size, const GLfloat *weights)) \
    X (z0        , glWeightdvARB, (GLint size, const GLdouble *weights)) \
    X (z0        , glWeightubvARB, (GLint size, const GLubyte *weights)) \
    X (z0        , glWeightusvARB, (GLint size, const GLushort *weights)) \
    X (z0        , glWeightuivARB, (GLint size, const GLuint *weights)) \
    X (z0        , glWeightPointerARB, (GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glVertexBlendARB, (GLint count))

#define DRX_GL_FUNCTIONS_GL_ARB_vertex_buffer_object \
    X (z0        , glBindBufferARB, (GLenum target, GLuint buffer)) \
    X (z0        , glDeleteBuffersARB, (GLsizei n, const GLuint *buffers)) \
    X (z0        , glGenBuffersARB, (GLsizei n, GLuint *buffers)) \
    X (GLboolean   , glIsBufferARB, (GLuint buffer)) \
    X (z0        , glBufferDataARB, (GLenum target, GLsizeiptrARB size, ukk data, GLenum usage)) \
    X (z0        , glBufferSubDataARB, (GLenum target, GLintptrARB offset, GLsizeiptrARB size, ukk data)) \
    X (z0        , glGetBufferSubDataARB, (GLenum target, GLintptrARB offset, GLsizeiptrARB size, uk data)) \
    X (uk       , glMapBufferARB, (GLenum target, GLenum access)) \
    X (GLboolean   , glUnmapBufferARB, (GLenum target)) \
    X (z0        , glGetBufferParameterivARB, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetBufferPointervARB, (GLenum target, GLenum pname, uk *params))

#define DRX_GL_FUNCTIONS_GL_ARB_vertex_program \
    X (z0        , glVertexAttrib1dARB, (GLuint index, GLdouble x)) \
    X (z0        , glVertexAttrib1dvARB, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib1fARB, (GLuint index, GLfloat x)) \
    X (z0        , glVertexAttrib1fvARB, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib1sARB, (GLuint index, GLshort x)) \
    X (z0        , glVertexAttrib1svARB, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib2dARB, (GLuint index, GLdouble x, GLdouble y)) \
    X (z0        , glVertexAttrib2dvARB, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib2fARB, (GLuint index, GLfloat x, GLfloat y)) \
    X (z0        , glVertexAttrib2fvARB, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib2sARB, (GLuint index, GLshort x, GLshort y)) \
    X (z0        , glVertexAttrib2svARB, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib3dARB, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertexAttrib3dvARB, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib3fARB, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glVertexAttrib3fvARB, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib3sARB, (GLuint index, GLshort x, GLshort y, GLshort z)) \
    X (z0        , glVertexAttrib3svARB, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4NbvARB, (GLuint index, const GLbyte *v)) \
    X (z0        , glVertexAttrib4NivARB, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttrib4NsvARB, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4NubARB, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)) \
    X (z0        , glVertexAttrib4NubvARB, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttrib4NuivARB, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttrib4NusvARB, (GLuint index, const GLushort *v)) \
    X (z0        , glVertexAttrib4bvARB, (GLuint index, const GLbyte *v)) \
    X (z0        , glVertexAttrib4dARB, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertexAttrib4dvARB, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib4fARB, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glVertexAttrib4fvARB, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib4ivARB, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttrib4sARB, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glVertexAttrib4svARB, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4ubvARB, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttrib4uivARB, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttrib4usvARB, (GLuint index, const GLushort *v)) \
    X (z0        , glVertexAttribPointerARB, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, ukk pointer)) \
    X (z0        , glEnableVertexAttribArrayARB, (GLuint index)) \
    X (z0        , glDisableVertexAttribArrayARB, (GLuint index)) \
    X (z0        , glGetVertexAttribdvARB, (GLuint index, GLenum pname, GLdouble *params)) \
    X (z0        , glGetVertexAttribfvARB, (GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVertexAttribivARB, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribPointervARB, (GLuint index, GLenum pname, uk *pointer))

#define DRX_GL_FUNCTIONS_GL_ARB_vertex_shader \
    X (z0        , glBindAttribLocationARB, (GLhandleARB programObj, GLuint index, const GLcharARB *name)) \
    X (z0        , glGetActiveAttribARB, (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name)) \
    X (GLint       , glGetAttribLocationARB, (GLhandleARB programObj, const GLcharARB *name))

#define DRX_GL_FUNCTIONS_GL_ARB_viewport_array \
    X (z0        , glDepthRangeArraydvNV, (GLuint first, GLsizei count, const GLdouble *v)) \
    X (z0        , glDepthRangeIndexeddNV, (GLuint index, GLdouble n, GLdouble f))

#define DRX_GL_FUNCTIONS_GL_ARB_window_pos \
    X (z0        , glWindowPos2dARB, (GLdouble x, GLdouble y)) \
    X (z0        , glWindowPos2dvARB, (const GLdouble *v)) \
    X (z0        , glWindowPos2fARB, (GLfloat x, GLfloat y)) \
    X (z0        , glWindowPos2fvARB, (const GLfloat *v)) \
    X (z0        , glWindowPos2iARB, (GLint x, GLint y)) \
    X (z0        , glWindowPos2ivARB, (const GLint *v)) \
    X (z0        , glWindowPos2sARB, (GLshort x, GLshort y)) \
    X (z0        , glWindowPos2svARB, (const GLshort *v)) \
    X (z0        , glWindowPos3dARB, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glWindowPos3dvARB, (const GLdouble *v)) \
    X (z0        , glWindowPos3fARB, (GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glWindowPos3fvARB, (const GLfloat *v)) \
    X (z0        , glWindowPos3iARB, (GLint x, GLint y, GLint z)) \
    X (z0        , glWindowPos3ivARB, (const GLint *v)) \
    X (z0        , glWindowPos3sARB, (GLshort x, GLshort y, GLshort z)) \
    X (z0        , glWindowPos3svARB, (const GLshort *v))

#define DRX_GL_FUNCTIONS_GL_ATI_draw_buffers \
    X (z0        , glDrawBuffersATI, (GLsizei n, const GLenum *bufs))

#define DRX_GL_FUNCTIONS_GL_ATI_element_array \
    X (z0        , glElementPointerATI, (GLenum type, ukk pointer)) \
    X (z0        , glDrawElementArrayATI, (GLenum mode, GLsizei count)) \
    X (z0        , glDrawRangeElementArrayATI, (GLenum mode, GLuint start, GLuint end, GLsizei count))

#define DRX_GL_FUNCTIONS_GL_ATI_envmap_bumpmap \
    X (z0        , glTexBumpParameterivATI, (GLenum pname, const GLint *param)) \
    X (z0        , glTexBumpParameterfvATI, (GLenum pname, const GLfloat *param)) \
    X (z0        , glGetTexBumpParameterivATI, (GLenum pname, GLint *param)) \
    X (z0        , glGetTexBumpParameterfvATI, (GLenum pname, GLfloat *param))

#define DRX_GL_FUNCTIONS_GL_ATI_fragment_shader \
    X (GLuint      , glGenFragmentShadersATI, (GLuint range)) \
    X (z0        , glBindFragmentShaderATI, (GLuint id)) \
    X (z0        , glDeleteFragmentShaderATI, (GLuint id)) \
    X (z0        , glBeginFragmentShaderATI, ()) \
    X (z0        , glEndFragmentShaderATI, ()) \
    X (z0        , glPassTexCoordATI, (GLuint dst, GLuint coord, GLenum swizzle)) \
    X (z0        , glSampleMapATI, (GLuint dst, GLuint interp, GLenum swizzle)) \
    X (z0        , glColorFragmentOp1ATI, (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)) \
    X (z0        , glColorFragmentOp2ATI, (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)) \
    X (z0        , glColorFragmentOp3ATI, (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)) \
    X (z0        , glAlphaFragmentOp1ATI, (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)) \
    X (z0        , glAlphaFragmentOp2ATI, (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)) \
    X (z0        , glAlphaFragmentOp3ATI, (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)) \
    X (z0        , glSetFragmentShaderConstantATI, (GLuint dst, const GLfloat *value))

#define DRX_GL_FUNCTIONS_GL_ATI_map_object_buffer \
    X (uk       , glMapObjectBufferATI, (GLuint buffer)) \
    X (z0        , glUnmapObjectBufferATI, (GLuint buffer))

#define DRX_GL_FUNCTIONS_GL_ATI_pn_triangles \
    X (z0        , glPNTrianglesiATI, (GLenum pname, GLint param)) \
    X (z0        , glPNTrianglesfATI, (GLenum pname, GLfloat param))

#define DRX_GL_FUNCTIONS_GL_ATI_separate_stencil \
    X (z0        , glStencilOpSeparateATI, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
    X (z0        , glStencilFuncSeparateATI, (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask))

#define DRX_GL_FUNCTIONS_GL_ATI_vertex_array_object \
    X (GLuint      , glNewObjectBufferATI, (GLsizei size, ukk pointer, GLenum usage)) \
    X (GLboolean   , glIsObjectBufferATI, (GLuint buffer)) \
    X (z0        , glUpdateObjectBufferATI, (GLuint buffer, GLuint offset, GLsizei size, ukk pointer, GLenum preserve)) \
    X (z0        , glGetObjectBufferfvATI, (GLuint buffer, GLenum pname, GLfloat *params)) \
    X (z0        , glGetObjectBufferivATI, (GLuint buffer, GLenum pname, GLint *params)) \
    X (z0        , glFreeObjectBufferATI, (GLuint buffer)) \
    X (z0        , glArrayObjectATI, (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)) \
    X (z0        , glGetArrayObjectfvATI, (GLenum array, GLenum pname, GLfloat *params)) \
    X (z0        , glGetArrayObjectivATI, (GLenum array, GLenum pname, GLint *params)) \
    X (z0        , glVariantArrayObjectATI, (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)) \
    X (z0        , glGetVariantArrayObjectfvATI, (GLuint id, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVariantArrayObjectivATI, (GLuint id, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_ATI_vertex_attrib_array_object \
    X (z0        , glVertexAttribArrayObjectATI, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)) \
    X (z0        , glGetVertexAttribArrayObjectfvATI, (GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVertexAttribArrayObjectivATI, (GLuint index, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_ATI_vertex_streams \
    X (z0        , glVertexStream1sATI, (GLenum stream, GLshort x)) \
    X (z0        , glVertexStream1svATI, (GLenum stream, const GLshort *coords)) \
    X (z0        , glVertexStream1iATI, (GLenum stream, GLint x)) \
    X (z0        , glVertexStream1ivATI, (GLenum stream, const GLint *coords)) \
    X (z0        , glVertexStream1fATI, (GLenum stream, GLfloat x)) \
    X (z0        , glVertexStream1fvATI, (GLenum stream, const GLfloat *coords)) \
    X (z0        , glVertexStream1dATI, (GLenum stream, GLdouble x)) \
    X (z0        , glVertexStream1dvATI, (GLenum stream, const GLdouble *coords)) \
    X (z0        , glVertexStream2sATI, (GLenum stream, GLshort x, GLshort y)) \
    X (z0        , glVertexStream2svATI, (GLenum stream, const GLshort *coords)) \
    X (z0        , glVertexStream2iATI, (GLenum stream, GLint x, GLint y)) \
    X (z0        , glVertexStream2ivATI, (GLenum stream, const GLint *coords)) \
    X (z0        , glVertexStream2fATI, (GLenum stream, GLfloat x, GLfloat y)) \
    X (z0        , glVertexStream2fvATI, (GLenum stream, const GLfloat *coords)) \
    X (z0        , glVertexStream2dATI, (GLenum stream, GLdouble x, GLdouble y)) \
    X (z0        , glVertexStream2dvATI, (GLenum stream, const GLdouble *coords)) \
    X (z0        , glVertexStream3sATI, (GLenum stream, GLshort x, GLshort y, GLshort z)) \
    X (z0        , glVertexStream3svATI, (GLenum stream, const GLshort *coords)) \
    X (z0        , glVertexStream3iATI, (GLenum stream, GLint x, GLint y, GLint z)) \
    X (z0        , glVertexStream3ivATI, (GLenum stream, const GLint *coords)) \
    X (z0        , glVertexStream3fATI, (GLenum stream, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glVertexStream3fvATI, (GLenum stream, const GLfloat *coords)) \
    X (z0        , glVertexStream3dATI, (GLenum stream, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertexStream3dvATI, (GLenum stream, const GLdouble *coords)) \
    X (z0        , glVertexStream4sATI, (GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glVertexStream4svATI, (GLenum stream, const GLshort *coords)) \
    X (z0        , glVertexStream4iATI, (GLenum stream, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glVertexStream4ivATI, (GLenum stream, const GLint *coords)) \
    X (z0        , glVertexStream4fATI, (GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glVertexStream4fvATI, (GLenum stream, const GLfloat *coords)) \
    X (z0        , glVertexStream4dATI, (GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertexStream4dvATI, (GLenum stream, const GLdouble *coords)) \
    X (z0        , glNormalStream3bATI, (GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz)) \
    X (z0        , glNormalStream3bvATI, (GLenum stream, const GLbyte *coords)) \
    X (z0        , glNormalStream3sATI, (GLenum stream, GLshort nx, GLshort ny, GLshort nz)) \
    X (z0        , glNormalStream3svATI, (GLenum stream, const GLshort *coords)) \
    X (z0        , glNormalStream3iATI, (GLenum stream, GLint nx, GLint ny, GLint nz)) \
    X (z0        , glNormalStream3ivATI, (GLenum stream, const GLint *coords)) \
    X (z0        , glNormalStream3fATI, (GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz)) \
    X (z0        , glNormalStream3fvATI, (GLenum stream, const GLfloat *coords)) \
    X (z0        , glNormalStream3dATI, (GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz)) \
    X (z0        , glNormalStream3dvATI, (GLenum stream, const GLdouble *coords)) \
    X (z0        , glClientActiveVertexStreamATI, (GLenum stream)) \
    X (z0        , glVertexBlendEnviATI, (GLenum pname, GLint param)) \
    X (z0        , glVertexBlendEnvfATI, (GLenum pname, GLfloat param))

#define DRX_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    X (z0        , glEGLImageTargetTexStorageEXT, (GLenum target, GLeglImageOES image, const GLint* attrib_list)) \
    X (z0        , glEGLImageTargetTextureStorageEXT, (GLuint texture, GLeglImageOES image, const GLint* attrib_list))

#define DRX_GL_FUNCTIONS_GL_EXT_bindable_uniform \
    X (z0        , glUniformBufferEXT, (GLuint program, GLint location, GLuint buffer)) \
    X (GLint       , glGetUniformBufferSizeEXT, (GLuint program, GLint location)) \
    X (GLintptr    , glGetUniformOffsetEXT, (GLuint program, GLint location))

#define DRX_GL_FUNCTIONS_GL_EXT_blend_color \
    X (z0        , glBlendColorEXT, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))

#define DRX_GL_FUNCTIONS_GL_EXT_blend_equation_separate \
    X (z0        , glBlendEquationSeparateEXT, (GLenum modeRGB, GLenum modeAlpha))

#define DRX_GL_FUNCTIONS_GL_EXT_blend_func_separate \
    X (z0        , glBlendFuncSeparateEXT, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))

#define DRX_GL_FUNCTIONS_GL_EXT_blend_minmax \
    X (z0        , glBlendEquationEXT, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_EXT_color_subtable \
    X (z0        , glColorSubTableEXT, (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, ukk data)) \
    X (z0        , glCopyColorSubTableEXT, (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width))

#define DRX_GL_FUNCTIONS_GL_EXT_compiled_vertex_array \
    X (z0        , glLockArraysEXT, (GLint first, GLsizei count)) \
    X (z0        , glUnlockArraysEXT, ())

#define DRX_GL_FUNCTIONS_GL_EXT_convolution \
    X (z0        , glConvolutionFilter1DEXT, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, ukk image)) \
    X (z0        , glConvolutionFilter2DEXT, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk image)) \
    X (z0        , glConvolutionParameterfEXT, (GLenum target, GLenum pname, GLfloat params)) \
    X (z0        , glConvolutionParameterfvEXT, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glConvolutionParameteriEXT, (GLenum target, GLenum pname, GLint params)) \
    X (z0        , glConvolutionParameterivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glCopyConvolutionFilter1DEXT, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyConvolutionFilter2DEXT, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glGetConvolutionFilterEXT, (GLenum target, GLenum format, GLenum type, uk image)) \
    X (z0        , glGetConvolutionParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetConvolutionParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetSeparableFilterEXT, (GLenum target, GLenum format, GLenum type, uk row, uk column, uk span)) \
    X (z0        , glSeparableFilter2DEXT, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk row, ukk column))

#define DRX_GL_FUNCTIONS_GL_EXT_coordinate_frame \
    X (z0        , glTangent3bEXT, (GLbyte tx, GLbyte ty, GLbyte tz)) \
    X (z0        , glTangent3bvEXT, (const GLbyte *v)) \
    X (z0        , glTangent3dEXT, (GLdouble tx, GLdouble ty, GLdouble tz)) \
    X (z0        , glTangent3dvEXT, (const GLdouble *v)) \
    X (z0        , glTangent3fEXT, (GLfloat tx, GLfloat ty, GLfloat tz)) \
    X (z0        , glTangent3fvEXT, (const GLfloat *v)) \
    X (z0        , glTangent3iEXT, (GLint tx, GLint ty, GLint tz)) \
    X (z0        , glTangent3ivEXT, (const GLint *v)) \
    X (z0        , glTangent3sEXT, (GLshort tx, GLshort ty, GLshort tz)) \
    X (z0        , glTangent3svEXT, (const GLshort *v)) \
    X (z0        , glBinormal3bEXT, (GLbyte bx, GLbyte by, GLbyte bz)) \
    X (z0        , glBinormal3bvEXT, (const GLbyte *v)) \
    X (z0        , glBinormal3dEXT, (GLdouble bx, GLdouble by, GLdouble bz)) \
    X (z0        , glBinormal3dvEXT, (const GLdouble *v)) \
    X (z0        , glBinormal3fEXT, (GLfloat bx, GLfloat by, GLfloat bz)) \
    X (z0        , glBinormal3fvEXT, (const GLfloat *v)) \
    X (z0        , glBinormal3iEXT, (GLint bx, GLint by, GLint bz)) \
    X (z0        , glBinormal3ivEXT, (const GLint *v)) \
    X (z0        , glBinormal3sEXT, (GLshort bx, GLshort by, GLshort bz)) \
    X (z0        , glBinormal3svEXT, (const GLshort *v)) \
    X (z0        , glTangentPointerEXT, (GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glBinormalPointerEXT, (GLenum type, GLsizei stride, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_EXT_copy_texture \
    X (z0        , glCopyTexImage1DEXT, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (z0        , glCopyTexImage2DEXT, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (z0        , glCopyTexSubImage1DEXT, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyTexSubImage2DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glCopyTexSubImage3DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_EXT_cull_vertex \
    X (z0        , glCullParameterdvEXT, (GLenum pname, GLdouble *params)) \
    X (z0        , glCullParameterfvEXT, (GLenum pname, GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_EXT_debug_label \
    X (z0        , glLabelObjectEXT, (GLenum type, GLuint object, GLsizei length, const GLchar *label)) \
    X (z0        , glGetObjectLabelEXT, (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label))

#define DRX_GL_FUNCTIONS_GL_EXT_debug_marker \
    X (z0        , glInsertEventMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (z0        , glPushGroupMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (z0        , glPopGroupMarkerEXT, ())

#define DRX_GL_FUNCTIONS_GL_EXT_depth_bounds_test \
    X (z0        , glDepthBoundsEXT, (GLclampd zmin, GLclampd zmax))

#define DRX_GL_FUNCTIONS_GL_EXT_direct_state_access \
    X (z0        , glMatrixLoadfEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixLoaddEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixMultfEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixMultdEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixLoadIdentityEXT, (GLenum mode)) \
    X (z0        , glMatrixRotatefEXT, (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glMatrixRotatedEXT, (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glMatrixScalefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glMatrixScaledEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glMatrixTranslatefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glMatrixTranslatedEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glMatrixFrustumEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (z0        , glMatrixOrthoEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (z0        , glMatrixPopEXT, (GLenum mode)) \
    X (z0        , glMatrixPushEXT, (GLenum mode)) \
    X (z0        , glClientAttribDefaultEXT, (GLbitfield mask)) \
    X (z0        , glPushClientAttribDefaultEXT, (GLbitfield mask)) \
    X (z0        , glTextureParameterfEXT, (GLuint texture, GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glTextureParameterfvEXT, (GLuint texture, GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glTextureParameteriEXT, (GLuint texture, GLenum target, GLenum pname, GLint param)) \
    X (z0        , glTextureParameterivEXT, (GLuint texture, GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTextureImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTextureImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTextureSubImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTextureSubImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyTextureImage1DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (z0        , glCopyTextureImage2DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (z0        , glCopyTextureSubImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyTextureSubImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glGetTextureImageEXT, (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, uk pixels)) \
    X (z0        , glGetTextureParameterfvEXT, (GLuint texture, GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTextureParameterivEXT, (GLuint texture, GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTextureLevelParameterfvEXT, (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (z0        , glGetTextureLevelParameterivEXT, (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (z0        , glTextureImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTextureSubImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyTextureSubImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glBindMultiTextureEXT, (GLenum texunit, GLenum target, GLuint texture)) \
    X (z0        , glMultiTexCoordPointerEXT, (GLenum texunit, GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glMultiTexEnvfEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glMultiTexEnvfvEXT, (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glMultiTexEnviEXT, (GLenum texunit, GLenum target, GLenum pname, GLint param)) \
    X (z0        , glMultiTexEnvivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glMultiTexGendEXT, (GLenum texunit, GLenum coord, GLenum pname, GLdouble param)) \
    X (z0        , glMultiTexGendvEXT, (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params)) \
    X (z0        , glMultiTexGenfEXT, (GLenum texunit, GLenum coord, GLenum pname, GLfloat param)) \
    X (z0        , glMultiTexGenfvEXT, (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params)) \
    X (z0        , glMultiTexGeniEXT, (GLenum texunit, GLenum coord, GLenum pname, GLint param)) \
    X (z0        , glMultiTexGenivEXT, (GLenum texunit, GLenum coord, GLenum pname, const GLint *params)) \
    X (z0        , glGetMultiTexEnvfvEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMultiTexEnvivEXT, (GLenum texunit, GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetMultiTexGendvEXT, (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params)) \
    X (z0        , glGetMultiTexGenfvEXT, (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMultiTexGenivEXT, (GLenum texunit, GLenum coord, GLenum pname, GLint *params)) \
    X (z0        , glMultiTexParameteriEXT, (GLenum texunit, GLenum target, GLenum pname, GLint param)) \
    X (z0        , glMultiTexParameterivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glMultiTexParameterfEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glMultiTexParameterfvEXT, (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glMultiTexImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glMultiTexImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glMultiTexSubImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glMultiTexSubImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyMultiTexImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (z0        , glCopyMultiTexImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (z0        , glCopyMultiTexSubImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (z0        , glCopyMultiTexSubImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glGetMultiTexImageEXT, (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, uk pixels)) \
    X (z0        , glGetMultiTexParameterfvEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMultiTexParameterivEXT, (GLenum texunit, GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetMultiTexLevelParameterfvEXT, (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMultiTexLevelParameterivEXT, (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (z0        , glMultiTexImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glMultiTexSubImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glCopyMultiTexSubImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glEnableClientStateIndexedEXT, (GLenum array, GLuint index)) \
    X (z0        , glDisableClientStateIndexedEXT, (GLenum array, GLuint index)) \
    X (z0        , glGetFloatIndexedvEXT, (GLenum target, GLuint index, GLfloat *data)) \
    X (z0        , glGetDoubleIndexedvEXT, (GLenum target, GLuint index, GLdouble *data)) \
    X (z0        , glGetPointerIndexedvEXT, (GLenum target, GLuint index, uk *data)) \
    X (z0        , glEnableIndexedEXT, (GLenum target, GLuint index)) \
    X (z0        , glDisableIndexedEXT, (GLenum target, GLuint index)) \
    X (GLboolean   , glIsEnabledIndexedEXT, (GLenum target, GLuint index)) \
    X (z0        , glGetIntegerIndexedvEXT, (GLenum target, GLuint index, GLint *data)) \
    X (z0        , glGetBooleanIndexedvEXT, (GLenum target, GLuint index, GLboolean *data)) \
    X (z0        , glCompressedTextureImage3DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedTextureImage2DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedTextureImage1DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedTextureSubImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedTextureSubImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedTextureSubImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, ukk bits)) \
    X (z0        , glGetCompressedTextureImageEXT, (GLuint texture, GLenum target, GLint lod, uk img)) \
    X (z0        , glCompressedMultiTexImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedMultiTexImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedMultiTexImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedMultiTexSubImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedMultiTexSubImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, ukk bits)) \
    X (z0        , glCompressedMultiTexSubImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, ukk bits)) \
    X (z0        , glGetCompressedMultiTexImageEXT, (GLenum texunit, GLenum target, GLint lod, uk img)) \
    X (z0        , glMatrixLoadTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixLoadTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glMatrixMultTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (z0        , glMatrixMultTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (z0        , glNamedBufferDataEXT, (GLuint buffer, GLsizeiptr size, ukk data, GLenum usage)) \
    X (z0        , glNamedBufferSubDataEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, ukk data)) \
    X (uk       , glMapNamedBufferEXT, (GLuint buffer, GLenum access)) \
    X (GLboolean   , glUnmapNamedBufferEXT, (GLuint buffer)) \
    X (z0        , glGetNamedBufferParameterivEXT, (GLuint buffer, GLenum pname, GLint *params)) \
    X (z0        , glGetNamedBufferPointervEXT, (GLuint buffer, GLenum pname, uk *params)) \
    X (z0        , glGetNamedBufferSubDataEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, uk data)) \
    X (z0        , glProgramUniform1fEXT, (GLuint program, GLint location, GLfloat v0)) \
    X (z0        , glProgramUniform2fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (z0        , glProgramUniform3fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (z0        , glProgramUniform4fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (z0        , glProgramUniform1iEXT, (GLuint program, GLint location, GLint v0)) \
    X (z0        , glProgramUniform2iEXT, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (z0        , glProgramUniform3iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (z0        , glProgramUniform4iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (z0        , glProgramUniform1fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform2fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform3fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform4fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (z0        , glProgramUniform1ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform2ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform3ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniform4ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (z0        , glProgramUniformMatrix2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix2x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix3x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glProgramUniformMatrix4x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (z0        , glTextureBufferEXT, (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer)) \
    X (z0        , glMultiTexBufferEXT, (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer)) \
    X (z0        , glTextureParameterIivEXT, (GLuint texture, GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTextureParameterIuivEXT, (GLuint texture, GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetTextureParameterIivEXT, (GLuint texture, GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTextureParameterIuivEXT, (GLuint texture, GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glMultiTexParameterIivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glMultiTexParameterIuivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetMultiTexParameterIivEXT, (GLenum texunit, GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetMultiTexParameterIuivEXT, (GLenum texunit, GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glProgramUniform1uiEXT, (GLuint program, GLint location, GLuint v0)) \
    X (z0        , glProgramUniform2uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glProgramUniform3uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glProgramUniform4uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glProgramUniform1uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform2uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform3uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glProgramUniform4uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glNamedProgramLocalParameters4fvEXT, (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params)) \
    X (z0        , glNamedProgramLocalParameterI4iEXT, (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glNamedProgramLocalParameterI4ivEXT, (GLuint program, GLenum target, GLuint index, const GLint *params)) \
    X (z0        , glNamedProgramLocalParametersI4ivEXT, (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params)) \
    X (z0        , glNamedProgramLocalParameterI4uiEXT, (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (z0        , glNamedProgramLocalParameterI4uivEXT, (GLuint program, GLenum target, GLuint index, const GLuint *params)) \
    X (z0        , glNamedProgramLocalParametersI4uivEXT, (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params)) \
    X (z0        , glGetNamedProgramLocalParameterIivEXT, (GLuint program, GLenum target, GLuint index, GLint *params)) \
    X (z0        , glGetNamedProgramLocalParameterIuivEXT, (GLuint program, GLenum target, GLuint index, GLuint *params)) \
    X (z0        , glEnableClientStateiEXT, (GLenum array, GLuint index)) \
    X (z0        , glDisableClientStateiEXT, (GLenum array, GLuint index)) \
    X (z0        , glGetFloati_vEXT, (GLenum pname, GLuint index, GLfloat *params)) \
    X (z0        , glGetDoublei_vEXT, (GLenum pname, GLuint index, GLdouble *params)) \
    X (z0        , glGetPointeri_vEXT, (GLenum pname, GLuint index, uk *params)) \
    X (z0        , glNamedProgramStringEXT, (GLuint program, GLenum target, GLenum format, GLsizei len, ukk string)) \
    X (z0        , glNamedProgramLocalParameter4dEXT, (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glNamedProgramLocalParameter4dvEXT, (GLuint program, GLenum target, GLuint index, const GLdouble *params)) \
    X (z0        , glNamedProgramLocalParameter4fEXT, (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glNamedProgramLocalParameter4fvEXT, (GLuint program, GLenum target, GLuint index, const GLfloat *params)) \
    X (z0        , glGetNamedProgramLocalParameterdvEXT, (GLuint program, GLenum target, GLuint index, GLdouble *params)) \
    X (z0        , glGetNamedProgramLocalParameterfvEXT, (GLuint program, GLenum target, GLuint index, GLfloat *params)) \
    X (z0        , glGetNamedProgramivEXT, (GLuint program, GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetNamedProgramStringEXT, (GLuint program, GLenum target, GLenum pname, uk string)) \
    X (z0        , glNamedRenderbufferStorageEXT, (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glGetNamedRenderbufferParameterivEXT, (GLuint renderbuffer, GLenum pname, GLint *params)) \
    X (z0        , glNamedRenderbufferStorageMultisampleEXT, (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glNamedRenderbufferStorageMultisampleCoverageEXT, (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (GLenum      , glCheckNamedFramebufferStatusEXT, (GLuint framebuffer, GLenum target)) \
    X (z0        , glNamedFramebufferTexture1DEXT, (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glNamedFramebufferTexture2DEXT, (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glNamedFramebufferTexture3DEXT, (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X (z0        , glNamedFramebufferRenderbufferEXT, (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (z0        , glGetNamedFramebufferAttachmentParameterivEXT, (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)) \
    X (z0        , glGenerateTextureMipmapEXT, (GLuint texture, GLenum target)) \
    X (z0        , glGenerateMultiTexMipmapEXT, (GLenum texunit, GLenum target)) \
    X (z0        , glFramebufferDrawBufferEXT, (GLuint framebuffer, GLenum mode)) \
    X (z0        , glFramebufferDrawBuffersEXT, (GLuint framebuffer, GLsizei n, const GLenum *bufs)) \
    X (z0        , glFramebufferReadBufferEXT, (GLuint framebuffer, GLenum mode)) \
    X (z0        , glGetFramebufferParameterivEXT, (GLuint framebuffer, GLenum pname, GLint *params)) \
    X (z0        , glNamedCopyBufferSubDataEXT, (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (z0        , glNamedFramebufferTextureEXT, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)) \
    X (z0        , glNamedFramebufferTextureLayerEXT, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (z0        , glNamedFramebufferTextureFaceEXT, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face)) \
    X (z0        , glTextureRenderbufferEXT, (GLuint texture, GLenum target, GLuint renderbuffer)) \
    X (z0        , glMultiTexRenderbufferEXT, (GLenum texunit, GLenum target, GLuint renderbuffer)) \
    X (z0        , glVertexArrayVertexOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayColorOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayEdgeFlagOffsetEXT, (GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayIndexOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayNormalOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayTexCoordOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayMultiTexCoordOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayFogCoordOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArraySecondaryColorOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayVertexAttribOffsetEXT, (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset)) \
    X (z0        , glVertexArrayVertexAttribIOffsetEXT, (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glEnableVertexArrayEXT, (GLuint vaobj, GLenum array)) \
    X (z0        , glDisableVertexArrayEXT, (GLuint vaobj, GLenum array)) \
    X (z0        , glEnableVertexArrayAttribEXT, (GLuint vaobj, GLuint index)) \
    X (z0        , glDisableVertexArrayAttribEXT, (GLuint vaobj, GLuint index)) \
    X (z0        , glGetVertexArrayIntegervEXT, (GLuint vaobj, GLenum pname, GLint *param)) \
    X (z0        , glGetVertexArrayPointervEXT, (GLuint vaobj, GLenum pname, uk *param)) \
    X (z0        , glGetVertexArrayIntegeri_vEXT, (GLuint vaobj, GLuint index, GLenum pname, GLint *param)) \
    X (z0        , glGetVertexArrayPointeri_vEXT, (GLuint vaobj, GLuint index, GLenum pname, uk *param)) \
    X (uk       , glMapNamedBufferRangeEXT, (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (z0        , glFlushMappedNamedBufferRangeEXT, (GLuint buffer, GLintptr offset, GLsizeiptr length)) \
    X (z0        , glNamedBufferStorageEXT, (GLuint buffer, GLsizeiptr size, ukk data, GLbitfield flags)) \
    X (z0        , glClearNamedBufferDataEXT, (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, ukk data)) \
    X (z0        , glClearNamedBufferSubDataEXT, (GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, ukk data)) \
    X (z0        , glNamedFramebufferParameteriEXT, (GLuint framebuffer, GLenum pname, GLint param)) \
    X (z0        , glGetNamedFramebufferParameterivEXT, (GLuint framebuffer, GLenum pname, GLint *params)) \
    X (z0        , glProgramUniform1dEXT, (GLuint program, GLint location, GLdouble x)) \
    X (z0        , glProgramUniform2dEXT, (GLuint program, GLint location, GLdouble x, GLdouble y)) \
    X (z0        , glProgramUniform3dEXT, (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glProgramUniform4dEXT, (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glProgramUniform1dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform2dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform3dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniform4dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix2dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix3dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix4dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix2x3dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix2x4dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix3x2dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix3x4dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix4x2dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glProgramUniformMatrix4x3dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (z0        , glTextureBufferRangeEXT, (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glTextureStorage1DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (z0        , glTextureStorage2DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTextureStorage3DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glTextureStorage2DMultisampleEXT, (GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (z0        , glTextureStorage3DMultisampleEXT, (GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (z0        , glVertexArrayBindVertexBufferEXT, (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (z0        , glVertexArrayVertexAttribFormatEXT, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (z0        , glVertexArrayVertexAttribIFormatEXT, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexArrayVertexAttribLFormatEXT, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (z0        , glVertexArrayVertexAttribBindingEXT, (GLuint vaobj, GLuint attribindex, GLuint bindingindex)) \
    X (z0        , glVertexArrayVertexBindingDivisorEXT, (GLuint vaobj, GLuint bindingindex, GLuint divisor)) \
    X (z0        , glVertexArrayVertexAttribLOffsetEXT, (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (z0        , glTexturePageCommitmentEXT, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit)) \
    X (z0        , glVertexArrayVertexAttribDivisorEXT, (GLuint vaobj, GLuint index, GLuint divisor))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_buffers2 \
    X (z0        , glColorMaskIndexedEXT, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_instanced \
    X (z0        , glDrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount)) \
    X (z0        , glDrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, ukk indices, GLsizei primcount))

#define DRX_GL_FUNCTIONS_GL_EXT_draw_range_elements \
    X (z0        , glDrawRangeElementsEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, ukk indices))

#define DRX_GL_FUNCTIONS_GL_EXT_external_buffer \
    X (z0        , glBufferStorageExternalEXT, (GLenum target, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags)) \
    X (z0        , glNamedBufferStorageExternalEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags))

#define DRX_GL_FUNCTIONS_GL_EXT_fog_coord \
    X (z0        , glFogCoordfEXT, (GLfloat coord)) \
    X (z0        , glFogCoordfvEXT, (const GLfloat *coord)) \
    X (z0        , glFogCoorddEXT, (GLdouble coord)) \
    X (z0        , glFogCoorddvEXT, (const GLdouble *coord)) \
    X (z0        , glFogCoordPointerEXT, (GLenum type, GLsizei stride, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_EXT_framebuffer_blit \
    X (z0        , glBlitFramebufferEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

#define DRX_GL_FUNCTIONS_GL_EXT_framebuffer_blit_layers \
    X (z0        , glBlitFramebufferLayersEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (z0        , glBlitFramebufferLayerEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint srcLayer, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLint dstLayer, GLbitfield mask, GLenum filter))

#define DRX_GL_FUNCTIONS_GL_EXT_framebuffer_multisample \
    X (z0        , glRenderbufferStorageMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_EXT_framebuffer_object \
    X (GLboolean   , glIsRenderbufferEXT, (GLuint renderbuffer)) \
    X (z0        , glBindRenderbufferEXT, (GLenum target, GLuint renderbuffer)) \
    X (z0        , glDeleteRenderbuffersEXT, (GLsizei n, const GLuint *renderbuffers)) \
    X (z0        , glGenRenderbuffersEXT, (GLsizei n, GLuint *renderbuffers)) \
    X (z0        , glRenderbufferStorageEXT, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glGetRenderbufferParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsFramebufferEXT, (GLuint framebuffer)) \
    X (z0        , glBindFramebufferEXT, (GLenum target, GLuint framebuffer)) \
    X (z0        , glDeleteFramebuffersEXT, (GLsizei n, const GLuint *framebuffers)) \
    X (z0        , glGenFramebuffersEXT, (GLsizei n, GLuint *framebuffers)) \
    X (GLenum      , glCheckFramebufferStatusEXT, (GLenum target)) \
    X (z0        , glFramebufferTexture1DEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glFramebufferTexture2DEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (z0        , glFramebufferTexture3DEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X (z0        , glFramebufferRenderbufferEXT, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (z0        , glGetFramebufferAttachmentParameterivEXT, (GLenum target, GLenum attachment, GLenum pname, GLint *params)) \
    X (z0        , glGenerateMipmapEXT, (GLenum target))

#define DRX_GL_FUNCTIONS_GL_EXT_geometry_shader4 \
    X (z0        , glProgramParameteriEXT, (GLuint program, GLenum pname, GLint value))

#define DRX_GL_FUNCTIONS_GL_EXT_gpu_program_parameters \
    X (z0        , glProgramEnvParameters4fvEXT, (GLenum target, GLuint index, GLsizei count, const GLfloat *params)) \
    X (z0        , glProgramLocalParameters4fvEXT, (GLenum target, GLuint index, GLsizei count, const GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_EXT_gpu_shader4 \
    X (z0        , glGetUniformuivEXT, (GLuint program, GLint location, GLuint *params)) \
    X (z0        , glBindFragDataLocationEXT, (GLuint program, GLuint color, const GLchar *name)) \
    X (GLint       , glGetFragDataLocationEXT, (GLuint program, const GLchar *name)) \
    X (z0        , glUniform1uiEXT, (GLint location, GLuint v0)) \
    X (z0        , glUniform2uiEXT, (GLint location, GLuint v0, GLuint v1)) \
    X (z0        , glUniform3uiEXT, (GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (z0        , glUniform4uiEXT, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (z0        , glUniform1uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform2uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform3uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glUniform4uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (z0        , glVertexAttribI1iEXT, (GLuint index, GLint x)) \
    X (z0        , glVertexAttribI2iEXT, (GLuint index, GLint x, GLint y)) \
    X (z0        , glVertexAttribI3iEXT, (GLuint index, GLint x, GLint y, GLint z)) \
    X (z0        , glVertexAttribI4iEXT, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glVertexAttribI1uiEXT, (GLuint index, GLuint x)) \
    X (z0        , glVertexAttribI2uiEXT, (GLuint index, GLuint x, GLuint y)) \
    X (z0        , glVertexAttribI3uiEXT, (GLuint index, GLuint x, GLuint y, GLuint z)) \
    X (z0        , glVertexAttribI4uiEXT, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (z0        , glVertexAttribI1ivEXT, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI2ivEXT, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI3ivEXT, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI4ivEXT, (GLuint index, const GLint *v)) \
    X (z0        , glVertexAttribI1uivEXT, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI2uivEXT, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI3uivEXT, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI4uivEXT, (GLuint index, const GLuint *v)) \
    X (z0        , glVertexAttribI4bvEXT, (GLuint index, const GLbyte *v)) \
    X (z0        , glVertexAttribI4svEXT, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttribI4ubvEXT, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttribI4usvEXT, (GLuint index, const GLushort *v)) \
    X (z0        , glVertexAttribIPointerEXT, (GLuint index, GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glGetVertexAttribIivEXT, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribIuivEXT, (GLuint index, GLenum pname, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_EXT_histogram \
    X (z0        , glGetHistogramEXT, (GLenum target, GLboolean reset, GLenum format, GLenum type, uk values)) \
    X (z0        , glGetHistogramParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetHistogramParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetMinmaxEXT, (GLenum target, GLboolean reset, GLenum format, GLenum type, uk values)) \
    X (z0        , glGetMinmaxParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMinmaxParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glHistogramEXT, (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)) \
    X (z0        , glMinmaxEXT, (GLenum target, GLenum internalformat, GLboolean sink)) \
    X (z0        , glResetHistogramEXT, (GLenum target)) \
    X (z0        , glResetMinmaxEXT, (GLenum target))

#define DRX_GL_FUNCTIONS_GL_EXT_index_func \
    X (z0        , glIndexFuncEXT, (GLenum func, GLclampf ref))

#define DRX_GL_FUNCTIONS_GL_EXT_index_material \
    X (z0        , glIndexMaterialEXT, (GLenum face, GLenum mode))

#define DRX_GL_FUNCTIONS_GL_EXT_light_texture \
    X (z0        , glApplyTextureEXT, (GLenum mode)) \
    X (z0        , glTextureLightEXT, (GLenum pname)) \
    X (z0        , glTextureMaterialEXT, (GLenum face, GLenum mode))

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

#define DRX_GL_FUNCTIONS_GL_EXT_multisample \
    X (z0        , glSampleMaskEXT, (GLclampf value, GLboolean invert)) \
    X (z0        , glSamplePatternEXT, (GLenum pattern))

#define DRX_GL_FUNCTIONS_GL_EXT_paletted_texture \
    X (z0        , glColorTableEXT, (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, ukk table)) \
    X (z0        , glGetColorTableEXT, (GLenum target, GLenum format, GLenum type, uk data)) \
    X (z0        , glGetColorTableParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetColorTableParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_EXT_pixel_transform \
    X (z0        , glPixelTransformParameteriEXT, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glPixelTransformParameterfEXT, (GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glPixelTransformParameterivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glPixelTransformParameterfvEXT, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glGetPixelTransformParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetPixelTransformParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_EXT_point_parameters \
    X (z0        , glPointParameterfEXT, (GLenum pname, GLfloat param)) \
    X (z0        , glPointParameterfvEXT, (GLenum pname, const GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_EXT_polygon_offset \
    X (z0        , glPolygonOffsetEXT, (GLfloat factor, GLfloat bias))

#define DRX_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    X (z0        , glPolygonOffsetClampEXT, (GLfloat factor, GLfloat units, GLfloat clamp))

#define DRX_GL_FUNCTIONS_GL_EXT_provoking_vertex \
    X (z0        , glProvokingVertexEXT, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_EXT_raster_multisample \
    X (z0        , glRasterSamplesEXT, (GLuint samples, GLboolean fixedsamplelocations))

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

#define DRX_GL_FUNCTIONS_GL_EXT_secondary_color \
    X (z0        , glSecondaryColor3bEXT, (GLbyte red, GLbyte green, GLbyte blue)) \
    X (z0        , glSecondaryColor3bvEXT, (const GLbyte *v)) \
    X (z0        , glSecondaryColor3dEXT, (GLdouble red, GLdouble green, GLdouble blue)) \
    X (z0        , glSecondaryColor3dvEXT, (const GLdouble *v)) \
    X (z0        , glSecondaryColor3fEXT, (GLfloat red, GLfloat green, GLfloat blue)) \
    X (z0        , glSecondaryColor3fvEXT, (const GLfloat *v)) \
    X (z0        , glSecondaryColor3iEXT, (GLint red, GLint green, GLint blue)) \
    X (z0        , glSecondaryColor3ivEXT, (const GLint *v)) \
    X (z0        , glSecondaryColor3sEXT, (GLshort red, GLshort green, GLshort blue)) \
    X (z0        , glSecondaryColor3svEXT, (const GLshort *v)) \
    X (z0        , glSecondaryColor3ubEXT, (GLubyte red, GLubyte green, GLubyte blue)) \
    X (z0        , glSecondaryColor3ubvEXT, (const GLubyte *v)) \
    X (z0        , glSecondaryColor3uiEXT, (GLuint red, GLuint green, GLuint blue)) \
    X (z0        , glSecondaryColor3uivEXT, (const GLuint *v)) \
    X (z0        , glSecondaryColor3usEXT, (GLushort red, GLushort green, GLushort blue)) \
    X (z0        , glSecondaryColor3usvEXT, (const GLushort *v)) \
    X (z0        , glSecondaryColorPointerEXT, (GLint size, GLenum type, GLsizei stride, ukk pointer))

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
    X (z0        , glUseProgramStagesEXT, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (z0        , glValidateProgramPipelineEXT, (GLuint pipeline))

#define DRX_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    X (z0        , glFramebufferFetchBarrierEXT, ())

#define DRX_GL_FUNCTIONS_GL_EXT_shader_image_load_store \
    X (z0        , glBindImageTextureEXT, (GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format)) \
    X (z0        , glMemoryBarrierEXT, (GLbitfield barriers))

#define DRX_GL_FUNCTIONS_GL_EXT_stencil_clear_tag \
    X (z0        , glStencilClearTagEXT, (GLsizei stencilTagBits, GLuint stencilClearTag))

#define DRX_GL_FUNCTIONS_GL_EXT_stencil_two_side \
    X (z0        , glActiveStencilFaceEXT, (GLenum face))

#define DRX_GL_FUNCTIONS_GL_EXT_subtexture \
    X (z0        , glTexSubImage1DEXT, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage2DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, ukk pixels))

#define DRX_GL_FUNCTIONS_GL_EXT_texture3D \
    X (z0        , glTexImage3DEXT, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage3DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, ukk pixels))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_array \
    X (z0        , glFramebufferTextureLayerEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_buffer_object \
    X (z0        , glTexBufferEXT, (GLenum target, GLenum internalformat, GLuint buffer))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_integer \
    X (z0        , glTexParameterIivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glTexParameterIuivEXT, (GLenum target, GLenum pname, const GLuint *params)) \
    X (z0        , glGetTexParameterIivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetTexParameterIuivEXT, (GLenum target, GLenum pname, GLuint *params)) \
    X (z0        , glClearColorIiEXT, (GLint red, GLint green, GLint blue, GLint alpha)) \
    X (z0        , glClearColorIuiEXT, (GLuint red, GLuint green, GLuint blue, GLuint alpha))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_object \
    X (GLboolean   , glAreTexturesResidentEXT, (GLsizei n, const GLuint *textures, GLboolean *residences)) \
    X (z0        , glBindTextureEXT, (GLenum target, GLuint texture)) \
    X (z0        , glDeleteTexturesEXT, (GLsizei n, const GLuint *textures)) \
    X (z0        , glGenTexturesEXT, (GLsizei n, GLuint *textures)) \
    X (GLboolean   , glIsTextureEXT, (GLuint texture)) \
    X (z0        , glPrioritizeTexturesEXT, (GLsizei n, const GLuint *textures, const GLclampf *priorities))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_perturb_normal \
    X (z0        , glTextureNormalEXT, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_EXT_texture_storage \
    X (z0        , glTexStorage1DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (z0        , glTexStorage2DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (z0        , glTexStorage3DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))

#define DRX_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    X (z0        , glCreateSemaphoresNV, (GLsizei n, GLuint *semaphores)) \
    X (z0        , glSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, const GLint *params)) \
    X (z0        , glGetSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_EXT_timer_query \
    X (z0        , glGetQueryObjecti64vEXT, (GLuint id, GLenum pname, GLint64 *params)) \
    X (z0        , glGetQueryObjectui64vEXT, (GLuint id, GLenum pname, GLuint64 *params))

#define DRX_GL_FUNCTIONS_GL_EXT_transform_feedback \
    X (z0        , glBeginTransformFeedbackEXT, (GLenum primitiveMode)) \
    X (z0        , glEndTransformFeedbackEXT, ()) \
    X (z0        , glBindBufferRangeEXT, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glBindBufferOffsetEXT, (GLenum target, GLuint index, GLuint buffer, GLintptr offset)) \
    X (z0        , glBindBufferBaseEXT, (GLenum target, GLuint index, GLuint buffer)) \
    X (z0        , glTransformFeedbackVaryingsEXT, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)) \
    X (z0        , glGetTransformFeedbackVaryingEXT, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name))

#define DRX_GL_FUNCTIONS_GL_EXT_vertex_array \
    X (z0        , glArrayElementEXT, (GLint i)) \
    X (z0        , glColorPointerEXT, (GLint size, GLenum type, GLsizei stride, GLsizei count, ukk pointer)) \
    X (z0        , glDrawArraysEXT, (GLenum mode, GLint first, GLsizei count)) \
    X (z0        , glEdgeFlagPointerEXT, (GLsizei stride, GLsizei count, const GLboolean *pointer)) \
    X (z0        , glGetPointervEXT, (GLenum pname, uk *params)) \
    X (z0        , glIndexPointerEXT, (GLenum type, GLsizei stride, GLsizei count, ukk pointer)) \
    X (z0        , glNormalPointerEXT, (GLenum type, GLsizei stride, GLsizei count, ukk pointer)) \
    X (z0        , glTexCoordPointerEXT, (GLint size, GLenum type, GLsizei stride, GLsizei count, ukk pointer)) \
    X (z0        , glVertexPointerEXT, (GLint size, GLenum type, GLsizei stride, GLsizei count, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_EXT_vertex_attrib_64bit \
    X (z0        , glVertexAttribL1dEXT, (GLuint index, GLdouble x)) \
    X (z0        , glVertexAttribL2dEXT, (GLuint index, GLdouble x, GLdouble y)) \
    X (z0        , glVertexAttribL3dEXT, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertexAttribL4dEXT, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertexAttribL1dvEXT, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribL2dvEXT, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribL3dvEXT, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribL4dvEXT, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttribLPointerEXT, (GLuint index, GLint size, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glGetVertexAttribLdvEXT, (GLuint index, GLenum pname, GLdouble *params))

#define DRX_GL_FUNCTIONS_GL_EXT_vertex_shader \
    X (z0        , glBeginVertexShaderEXT, ()) \
    X (z0        , glEndVertexShaderEXT, ()) \
    X (z0        , glBindVertexShaderEXT, (GLuint id)) \
    X (GLuint      , glGenVertexShadersEXT, (GLuint range)) \
    X (z0        , glDeleteVertexShaderEXT, (GLuint id)) \
    X (z0        , glShaderOp1EXT, (GLenum op, GLuint res, GLuint arg1)) \
    X (z0        , glShaderOp2EXT, (GLenum op, GLuint res, GLuint arg1, GLuint arg2)) \
    X (z0        , glShaderOp3EXT, (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3)) \
    X (z0        , glSwizzleEXT, (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)) \
    X (z0        , glWriteMaskEXT, (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)) \
    X (z0        , glInsertComponentEXT, (GLuint res, GLuint src, GLuint num)) \
    X (z0        , glExtractComponentEXT, (GLuint res, GLuint src, GLuint num)) \
    X (GLuint      , glGenSymbolsEXT, (GLenum datatype, GLenum storagetype, GLenum range, GLuint components)) \
    X (z0        , glSetInvariantEXT, (GLuint id, GLenum type, ukk addr)) \
    X (z0        , glSetLocalConstantEXT, (GLuint id, GLenum type, ukk addr)) \
    X (z0        , glVariantbvEXT, (GLuint id, const GLbyte *addr)) \
    X (z0        , glVariantsvEXT, (GLuint id, const GLshort *addr)) \
    X (z0        , glVariantivEXT, (GLuint id, const GLint *addr)) \
    X (z0        , glVariantfvEXT, (GLuint id, const GLfloat *addr)) \
    X (z0        , glVariantdvEXT, (GLuint id, const GLdouble *addr)) \
    X (z0        , glVariantubvEXT, (GLuint id, const GLubyte *addr)) \
    X (z0        , glVariantusvEXT, (GLuint id, const GLushort *addr)) \
    X (z0        , glVariantuivEXT, (GLuint id, const GLuint *addr)) \
    X (z0        , glVariantPointerEXT, (GLuint id, GLenum type, GLuint stride, ukk addr)) \
    X (z0        , glEnableVariantClientStateEXT, (GLuint id)) \
    X (z0        , glDisableVariantClientStateEXT, (GLuint id)) \
    X (GLuint      , glBindLightParameterEXT, (GLenum light, GLenum value)) \
    X (GLuint      , glBindMaterialParameterEXT, (GLenum face, GLenum value)) \
    X (GLuint      , glBindTexGenParameterEXT, (GLenum unit, GLenum coord, GLenum value)) \
    X (GLuint      , glBindTextureUnitParameterEXT, (GLenum unit, GLenum value)) \
    X (GLuint      , glBindParameterEXT, (GLenum value)) \
    X (GLboolean   , glIsVariantEnabledEXT, (GLuint id, GLenum cap)) \
    X (z0        , glGetVariantBooleanvEXT, (GLuint id, GLenum value, GLboolean *data)) \
    X (z0        , glGetVariantIntegervEXT, (GLuint id, GLenum value, GLint *data)) \
    X (z0        , glGetVariantFloatvEXT, (GLuint id, GLenum value, GLfloat *data)) \
    X (z0        , glGetVariantPointervEXT, (GLuint id, GLenum value, uk *data)) \
    X (z0        , glGetInvariantBooleanvEXT, (GLuint id, GLenum value, GLboolean *data)) \
    X (z0        , glGetInvariantIntegervEXT, (GLuint id, GLenum value, GLint *data)) \
    X (z0        , glGetInvariantFloatvEXT, (GLuint id, GLenum value, GLfloat *data)) \
    X (z0        , glGetLocalConstantBooleanvEXT, (GLuint id, GLenum value, GLboolean *data)) \
    X (z0        , glGetLocalConstantIntegervEXT, (GLuint id, GLenum value, GLint *data)) \
    X (z0        , glGetLocalConstantFloatvEXT, (GLuint id, GLenum value, GLfloat *data))

#define DRX_GL_FUNCTIONS_GL_EXT_vertex_weighting \
    X (z0        , glVertexWeightfEXT, (GLfloat weight)) \
    X (z0        , glVertexWeightfvEXT, (const GLfloat *weight)) \
    X (z0        , glVertexWeightPointerEXT, (GLint size, GLenum type, GLsizei stride, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    X (GLboolean   , glAcquireKeyedMutexWin32EXT, (GLuint memory, GLuint64 key, GLuint timeout)) \
    X (GLboolean   , glReleaseKeyedMutexWin32EXT, (GLuint memory, GLuint64 key))

#define DRX_GL_FUNCTIONS_GL_EXT_window_rectangles \
    X (z0        , glWindowRectanglesEXT, (GLenum mode, GLsizei count, const GLint *box))

#define DRX_GL_FUNCTIONS_GL_EXT_x11_sync_object \
    X (GLsync      , glImportSyncEXT, (GLenum external_sync_type, GLintptr external_sync, GLbitfield flags))

#define DRX_GL_FUNCTIONS_GL_GREMEDY_frame_terminator \
    X (z0        , glFrameTerminatorGREMEDY, ())

#define DRX_GL_FUNCTIONS_GL_GREMEDY_string_marker \
    X (z0        , glStringMarkerGREMEDY, (GLsizei len, ukk string))

#define DRX_GL_FUNCTIONS_GL_HP_image_transform \
    X (z0        , glImageTransformParameteriHP, (GLenum target, GLenum pname, GLint param)) \
    X (z0        , glImageTransformParameterfHP, (GLenum target, GLenum pname, GLfloat param)) \
    X (z0        , glImageTransformParameterivHP, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glImageTransformParameterfvHP, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glGetImageTransformParameterivHP, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetImageTransformParameterfvHP, (GLenum target, GLenum pname, GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_IBM_multimode_draw_arrays \
    X (z0        , glMultiModeDrawArraysIBM, (const GLenum *mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride)) \
    X (z0        , glMultiModeDrawElementsIBM, (const GLenum *mode, const GLsizei *count, GLenum type, ukk const*indices, GLsizei primcount, GLint modestride))

#define DRX_GL_FUNCTIONS_GL_IBM_static_data \
    X (z0        , glFlushStaticDataIBM, (GLenum target))

#define DRX_GL_FUNCTIONS_GL_IBM_vertex_array_lists \
    X (z0        , glColorPointerListIBM, (GLint size, GLenum type, GLint stride, ukk *pointer, GLint ptrstride)) \
    X (z0        , glSecondaryColorPointerListIBM, (GLint size, GLenum type, GLint stride, ukk *pointer, GLint ptrstride)) \
    X (z0        , glEdgeFlagPointerListIBM, (GLint stride, const GLboolean **pointer, GLint ptrstride)) \
    X (z0        , glFogCoordPointerListIBM, (GLenum type, GLint stride, ukk *pointer, GLint ptrstride)) \
    X (z0        , glIndexPointerListIBM, (GLenum type, GLint stride, ukk *pointer, GLint ptrstride)) \
    X (z0        , glNormalPointerListIBM, (GLenum type, GLint stride, ukk *pointer, GLint ptrstride)) \
    X (z0        , glTexCoordPointerListIBM, (GLint size, GLenum type, GLint stride, ukk *pointer, GLint ptrstride)) \
    X (z0        , glVertexPointerListIBM, (GLint size, GLenum type, GLint stride, ukk *pointer, GLint ptrstride))

#define DRX_GL_FUNCTIONS_GL_INGR_blend_func_separate \
    X (z0        , glBlendFuncSeparateINGR, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))

#define DRX_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    X (z0        , glApplyFramebufferAttachmentCMAAINTEL, ())

#define DRX_GL_FUNCTIONS_GL_INTEL_map_texture \
    X (z0        , glSyncTextureINTEL, (GLuint texture)) \
    X (z0        , glUnmapTexture2DINTEL, (GLuint texture, GLint level)) \
    X (uk       , glMapTexture2DINTEL, (GLuint texture, GLint level, GLbitfield access, GLint *stride, GLenum *layout))

#define DRX_GL_FUNCTIONS_GL_INTEL_parallel_arrays \
    X (z0        , glVertexPointervINTEL, (GLint size, GLenum type, ukk *pointer)) \
    X (z0        , glNormalPointervINTEL, (GLenum type, ukk *pointer)) \
    X (z0        , glColorPointervINTEL, (GLint size, GLenum type, ukk *pointer)) \
    X (z0        , glTexCoordPointervINTEL, (GLint size, GLenum type, ukk *pointer))

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

#define DRX_GL_FUNCTIONS_GL_MESA_resize_buffers \
    X (z0        , glResizeBuffersMESA, ())

#define DRX_GL_FUNCTIONS_GL_MESA_window_pos \
    X (z0        , glWindowPos2dMESA, (GLdouble x, GLdouble y)) \
    X (z0        , glWindowPos2dvMESA, (const GLdouble *v)) \
    X (z0        , glWindowPos2fMESA, (GLfloat x, GLfloat y)) \
    X (z0        , glWindowPos2fvMESA, (const GLfloat *v)) \
    X (z0        , glWindowPos2iMESA, (GLint x, GLint y)) \
    X (z0        , glWindowPos2ivMESA, (const GLint *v)) \
    X (z0        , glWindowPos2sMESA, (GLshort x, GLshort y)) \
    X (z0        , glWindowPos2svMESA, (const GLshort *v)) \
    X (z0        , glWindowPos3dMESA, (GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glWindowPos3dvMESA, (const GLdouble *v)) \
    X (z0        , glWindowPos3fMESA, (GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glWindowPos3fvMESA, (const GLfloat *v)) \
    X (z0        , glWindowPos3iMESA, (GLint x, GLint y, GLint z)) \
    X (z0        , glWindowPos3ivMESA, (const GLint *v)) \
    X (z0        , glWindowPos3sMESA, (GLshort x, GLshort y, GLshort z)) \
    X (z0        , glWindowPos3svMESA, (const GLshort *v)) \
    X (z0        , glWindowPos4dMESA, (GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glWindowPos4dvMESA, (const GLdouble *v)) \
    X (z0        , glWindowPos4fMESA, (GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glWindowPos4fvMESA, (const GLfloat *v)) \
    X (z0        , glWindowPos4iMESA, (GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glWindowPos4ivMESA, (const GLint *v)) \
    X (z0        , glWindowPos4sMESA, (GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glWindowPos4svMESA, (const GLshort *v))

#define DRX_GL_FUNCTIONS_GL_NVX_conditional_render \
    X (z0        , glBeginConditionalRenderNVX, (GLuint id)) \
    X (z0        , glEndConditionalRenderNVX, ())

#define DRX_GL_FUNCTIONS_GL_NVX_linked_gpu_multicast \
    X (z0        , glLGPUNamedBufferSubDataNVX, (GLbitfield gpuMask, GLuint buffer, GLintptr offset, GLsizeiptr size, ukk data)) \
    X (z0        , glLGPUCopyImageSubDataNVX, (GLuint sourceGpu, GLbitfield destinationGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srxY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth)) \
    X (z0        , glLGPUInterlockNVX, ())

#define DRX_GL_FUNCTIONS_GL_NV_alpha_to_coverage_dither_control \
    X (z0        , glAlphaToCoverageDitherControlNV, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect \
    X (z0        , glMultiDrawArraysIndirectBindlessNV, (GLenum mode, ukk indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount)) \
    X (z0        , glMultiDrawElementsIndirectBindlessNV, (GLenum mode, GLenum type, ukk indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount))

#define DRX_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect_count \
    X (z0        , glMultiDrawArraysIndirectBindlessCountNV, (GLenum mode, ukk indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount)) \
    X (z0        , glMultiDrawElementsIndirectBindlessCountNV, (GLenum mode, GLenum type, ukk indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount))

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

#define DRX_GL_FUNCTIONS_GL_NV_command_list \
    X (z0        , glCreateStatesNV, (GLsizei n, GLuint *states)) \
    X (z0        , glDeleteStatesNV, (GLsizei n, const GLuint *states)) \
    X (GLboolean   , glIsStateNV, (GLuint state)) \
    X (z0        , glStateCaptureNV, (GLuint state, GLenum mode)) \
    X (GLuint      , glGetCommandHeaderNV, (GLenum tokenID, GLuint size)) \
    X (GLushort    , glGetStageIndexNV, (GLenum shadertype)) \
    X (z0        , glDrawCommandsNV, (GLenum primitiveMode, GLuint buffer, const GLintptr *indirects, const GLsizei *sizes, GLuint count)) \
    X (z0        , glDrawCommandsAddressNV, (GLenum primitiveMode, const GLuint64 *indirects, const GLsizei *sizes, GLuint count)) \
    X (z0        , glDrawCommandsStatesNV, (GLuint buffer, const GLintptr *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count)) \
    X (z0        , glDrawCommandsStatesAddressNV, (const GLuint64 *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count)) \
    X (z0        , glCreateCommandListsNV, (GLsizei n, GLuint *lists)) \
    X (z0        , glDeleteCommandListsNV, (GLsizei n, const GLuint *lists)) \
    X (GLboolean   , glIsCommandListNV, (GLuint list)) \
    X (z0        , glListDrawCommandsStatesClientNV, (GLuint list, GLuint segment, ukk *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count)) \
    X (z0        , glCommandListSegmentsNV, (GLuint list, GLuint segments)) \
    X (z0        , glCompileCommandListNV, (GLuint list)) \
    X (z0        , glCallCommandListNV, (GLuint list))

#define DRX_GL_FUNCTIONS_GL_NV_conditional_render \
    X (z0        , glBeginConditionalRenderNV, (GLuint id, GLenum mode)) \
    X (z0        , glEndConditionalRenderNV, ())

#define DRX_GL_FUNCTIONS_GL_NV_conservative_raster \
    X (z0        , glSubpixelPrecisionBiasNV, (GLuint xbits, GLuint ybits))

#define DRX_GL_FUNCTIONS_GL_NV_conservative_raster_dilate \
    X (z0        , glConservativeRasterParameterfNV, (GLenum pname, GLfloat value))

#define DRX_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    X (z0        , glConservativeRasterParameteriNV, (GLenum pname, GLint param))

#define DRX_GL_FUNCTIONS_GL_NV_copy_image \
    X (z0        , glCopyImageSubDataNV, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth))

#define DRX_GL_FUNCTIONS_GL_NV_depth_buffer_float \
    X (z0        , glDepthRangedNV, (GLdouble zNear, GLdouble zFar)) \
    X (z0        , glClearDepthdNV, (GLdouble depth)) \
    X (z0        , glDepthBoundsdNV, (GLdouble zmin, GLdouble zmax))

#define DRX_GL_FUNCTIONS_GL_NV_draw_texture \
    X (z0        , glDrawTextureNV, (GLuint texture, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1))

#define DRX_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    X (z0        , glDrawVkImageNV, (GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1)) \
    X (GLVULKANPROCNV, glGetVkProcAddrNV, (const GLchar *name)) \
    X (z0        , glWaitVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (z0        , glSignalVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (z0        , glSignalVkFenceNV, (GLuint64 vkFence))

#define DRX_GL_FUNCTIONS_GL_NV_evaluators \
    X (z0        , glMapControlPointsNV, (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, ukk points)) \
    X (z0        , glMapParameterivNV, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glMapParameterfvNV, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glGetMapControlPointsNV, (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, uk points)) \
    X (z0        , glGetMapParameterivNV, (GLenum target, GLenum pname, GLint *params)) \
    X (z0        , glGetMapParameterfvNV, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetMapAttribParameterivNV, (GLenum target, GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetMapAttribParameterfvNV, (GLenum target, GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glEvalMapsNV, (GLenum target, GLenum mode))

#define DRX_GL_FUNCTIONS_GL_NV_explicit_multisample \
    X (z0        , glGetMultisamplefvNV, (GLenum pname, GLuint index, GLfloat *val)) \
    X (z0        , glSampleMaskIndexedNV, (GLuint index, GLbitfield mask)) \
    X (z0        , glTexRenderbufferNV, (GLenum target, GLuint renderbuffer))

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

#define DRX_GL_FUNCTIONS_GL_NV_fragment_program \
    X (z0        , glProgramNamedParameter4fNV, (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glProgramNamedParameter4fvNV, (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v)) \
    X (z0        , glProgramNamedParameter4dNV, (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glProgramNamedParameter4dvNV, (GLuint id, GLsizei len, const GLubyte *name, const GLdouble *v)) \
    X (z0        , glGetProgramNamedParameterfvNV, (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params)) \
    X (z0        , glGetProgramNamedParameterdvNV, (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params))

#define DRX_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    X (z0        , glCoverageModulationTableNV, (GLsizei n, const GLfloat *v)) \
    X (z0        , glGetCoverageModulationTableNV, (GLsizei bufSize, GLfloat *v)) \
    X (z0        , glCoverageModulationNV, (GLenum components))

#define DRX_GL_FUNCTIONS_GL_NV_framebuffer_multisample_coverage \
    X (z0        , glRenderbufferStorageMultisampleCoverageNV, (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height))

#define DRX_GL_FUNCTIONS_GL_NV_geometry_program4 \
    X (z0        , glProgramVertexLimitNV, (GLenum target, GLint limit)) \
    X (z0        , glFramebufferTextureEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (z0        , glFramebufferTextureFaceEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face))

#define DRX_GL_FUNCTIONS_GL_NV_gpu_program4 \
    X (z0        , glProgramLocalParameterI4iNV, (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glProgramLocalParameterI4ivNV, (GLenum target, GLuint index, const GLint *params)) \
    X (z0        , glProgramLocalParametersI4ivNV, (GLenum target, GLuint index, GLsizei count, const GLint *params)) \
    X (z0        , glProgramLocalParameterI4uiNV, (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (z0        , glProgramLocalParameterI4uivNV, (GLenum target, GLuint index, const GLuint *params)) \
    X (z0        , glProgramLocalParametersI4uivNV, (GLenum target, GLuint index, GLsizei count, const GLuint *params)) \
    X (z0        , glProgramEnvParameterI4iNV, (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (z0        , glProgramEnvParameterI4ivNV, (GLenum target, GLuint index, const GLint *params)) \
    X (z0        , glProgramEnvParametersI4ivNV, (GLenum target, GLuint index, GLsizei count, const GLint *params)) \
    X (z0        , glProgramEnvParameterI4uiNV, (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (z0        , glProgramEnvParameterI4uivNV, (GLenum target, GLuint index, const GLuint *params)) \
    X (z0        , glProgramEnvParametersI4uivNV, (GLenum target, GLuint index, GLsizei count, const GLuint *params)) \
    X (z0        , glGetProgramLocalParameterIivNV, (GLenum target, GLuint index, GLint *params)) \
    X (z0        , glGetProgramLocalParameterIuivNV, (GLenum target, GLuint index, GLuint *params)) \
    X (z0        , glGetProgramEnvParameterIivNV, (GLenum target, GLuint index, GLint *params)) \
    X (z0        , glGetProgramEnvParameterIuivNV, (GLenum target, GLuint index, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_NV_gpu_program5 \
    X (z0        , glProgramSubroutineParametersuivNV, (GLenum target, GLsizei count, const GLuint *params)) \
    X (z0        , glGetProgramSubroutineParameteruivNV, (GLenum target, GLuint index, GLuint *param))

#define DRX_GL_FUNCTIONS_GL_NV_half_float \
    X (z0        , glVertex2hNV, (GLhalfNV x, GLhalfNV y)) \
    X (z0        , glVertex2hvNV, (const GLhalfNV *v)) \
    X (z0        , glVertex3hNV, (GLhalfNV x, GLhalfNV y, GLhalfNV z)) \
    X (z0        , glVertex3hvNV, (const GLhalfNV *v)) \
    X (z0        , glVertex4hNV, (GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)) \
    X (z0        , glVertex4hvNV, (const GLhalfNV *v)) \
    X (z0        , glNormal3hNV, (GLhalfNV nx, GLhalfNV ny, GLhalfNV nz)) \
    X (z0        , glNormal3hvNV, (const GLhalfNV *v)) \
    X (z0        , glColor3hNV, (GLhalfNV red, GLhalfNV green, GLhalfNV blue)) \
    X (z0        , glColor3hvNV, (const GLhalfNV *v)) \
    X (z0        , glColor4hNV, (GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha)) \
    X (z0        , glColor4hvNV, (const GLhalfNV *v)) \
    X (z0        , glTexCoord1hNV, (GLhalfNV s)) \
    X (z0        , glTexCoord1hvNV, (const GLhalfNV *v)) \
    X (z0        , glTexCoord2hNV, (GLhalfNV s, GLhalfNV t)) \
    X (z0        , glTexCoord2hvNV, (const GLhalfNV *v)) \
    X (z0        , glTexCoord3hNV, (GLhalfNV s, GLhalfNV t, GLhalfNV r)) \
    X (z0        , glTexCoord3hvNV, (const GLhalfNV *v)) \
    X (z0        , glTexCoord4hNV, (GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)) \
    X (z0        , glTexCoord4hvNV, (const GLhalfNV *v)) \
    X (z0        , glMultiTexCoord1hNV, (GLenum target, GLhalfNV s)) \
    X (z0        , glMultiTexCoord1hvNV, (GLenum target, const GLhalfNV *v)) \
    X (z0        , glMultiTexCoord2hNV, (GLenum target, GLhalfNV s, GLhalfNV t)) \
    X (z0        , glMultiTexCoord2hvNV, (GLenum target, const GLhalfNV *v)) \
    X (z0        , glMultiTexCoord3hNV, (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r)) \
    X (z0        , glMultiTexCoord3hvNV, (GLenum target, const GLhalfNV *v)) \
    X (z0        , glMultiTexCoord4hNV, (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)) \
    X (z0        , glMultiTexCoord4hvNV, (GLenum target, const GLhalfNV *v)) \
    X (z0        , glVertexAttrib1hNV, (GLuint index, GLhalfNV x)) \
    X (z0        , glVertexAttrib1hvNV, (GLuint index, const GLhalfNV *v)) \
    X (z0        , glVertexAttrib2hNV, (GLuint index, GLhalfNV x, GLhalfNV y)) \
    X (z0        , glVertexAttrib2hvNV, (GLuint index, const GLhalfNV *v)) \
    X (z0        , glVertexAttrib3hNV, (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z)) \
    X (z0        , glVertexAttrib3hvNV, (GLuint index, const GLhalfNV *v)) \
    X (z0        , glVertexAttrib4hNV, (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)) \
    X (z0        , glVertexAttrib4hvNV, (GLuint index, const GLhalfNV *v)) \
    X (z0        , glVertexAttribs1hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (z0        , glVertexAttribs2hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (z0        , glVertexAttribs3hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (z0        , glVertexAttribs4hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (z0        , glFogCoordhNV, (GLhalfNV fog)) \
    X (z0        , glFogCoordhvNV, (const GLhalfNV *fog)) \
    X (z0        , glSecondaryColor3hNV, (GLhalfNV red, GLhalfNV green, GLhalfNV blue)) \
    X (z0        , glSecondaryColor3hvNV, (const GLhalfNV *v)) \
    X (z0        , glVertexWeighthNV, (GLhalfNV weight)) \
    X (z0        , glVertexWeighthvNV, (const GLhalfNV *weight))

#define DRX_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    X (z0        , glGetInternalformatSampleivNV, (GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint *params))

#define DRX_GL_FUNCTIONS_GL_NV_gpu_multicast \
    X (z0        , glRenderGpuMaskNV, (GLbitfield mask)) \
    X (z0        , glMulticastBufferSubDataNV, (GLbitfield gpuMask, GLuint buffer, GLintptr offset, GLsizeiptr size, ukk data)) \
    X (z0        , glMulticastCopyBufferSubDataNV, (GLuint readGpu, GLbitfield writeGpuMask, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (z0        , glMulticastCopyImageSubDataNV, (GLuint srcGpu, GLbitfield dstGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)) \
    X (z0        , glMulticastBlitFramebufferNV, (GLuint srcGpu, GLuint dstGpu, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (z0        , glMulticastFramebufferSampleLocationsfvNV, (GLuint gpu, GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glMulticastBarrierNV, ()) \
    X (z0        , glMulticastWaitSyncNV, (GLuint signalGpu, GLbitfield waitGpuMask)) \
    X (z0        , glMulticastGetQueryObjectivNV, (GLuint gpu, GLuint id, GLenum pname, GLint *params)) \
    X (z0        , glMulticastGetQueryObjectuivNV, (GLuint gpu, GLuint id, GLenum pname, GLuint *params)) \
    X (z0        , glMulticastGetQueryObjecti64vNV, (GLuint gpu, GLuint id, GLenum pname, GLint64 *params)) \
    X (z0        , glMulticastGetQueryObjectui64vNV, (GLuint gpu, GLuint id, GLenum pname, GLuint64 *params))

#define DRX_GL_FUNCTIONS_GL_NVX_gpu_multicast2 \
    X (z0        , glUploadGpuMaskNVX, (GLbitfield mask)) \
    X (z0        , glMulticastViewportArrayvNVX, (GLuint gpu, GLuint first, GLsizei count, const GLfloat *v)) \
    X (z0        , glMulticastViewportPositionWScaleNVX, (GLuint gpu, GLuint index, GLfloat xcoeff, GLfloat ycoeff)) \
    X (z0        , glMulticastScissorArrayvNVX, (GLuint gpu, GLuint first, GLsizei count, const GLint *v)) \
    X (GLuint      , glAsyncCopyBufferSubDataNVX, (GLsizei waitSemaphoreCount, const GLuint *waitSemaphoreArray, const GLuint64 *fenceValueArray, GLuint readGpu, GLbitfield writeGpuMask, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size, GLsizei signalSemaphoreCount, const GLuint *signalSemaphoreArray, const GLuint64 *signalValueArray)) \
    X (GLuint      , glAsyncCopyImageSubDataNVX, (GLsizei waitSemaphoreCount, const GLuint *waitSemaphoreArray, const GLuint64 *waitValueArray, GLuint srcGpu, GLbitfield dstGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth, GLsizei signalSemaphoreCount, const GLuint *signalSemaphoreArray, const GLuint64 *signalValueArray))

#define DRX_GL_FUNCTIONS_GL_NVX_progress_fence \
    X (GLuint      , glCreateProgressFenceNVX, ()) \
    X (z0        , glSignalSemaphoreui64NVX, (GLuint signalGpu, GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray)) \
    X (z0        , glWaitSemaphoreui64NVX, (GLuint waitGpu, GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray)) \
    X (z0        , glClientWaitSemaphoreui64NVX, (GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray))

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

#define DRX_GL_FUNCTIONS_GL_NV_occlusion_query \
    X (z0        , glGenOcclusionQueriesNV, (GLsizei n, GLuint *ids)) \
    X (z0        , glDeleteOcclusionQueriesNV, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsOcclusionQueryNV, (GLuint id)) \
    X (z0        , glBeginOcclusionQueryNV, (GLuint id)) \
    X (z0        , glEndOcclusionQueryNV, ()) \
    X (z0        , glGetOcclusionQueryivNV, (GLuint id, GLenum pname, GLint *params)) \
    X (z0        , glGetOcclusionQueryuivNV, (GLuint id, GLenum pname, GLuint *params))

#define DRX_GL_FUNCTIONS_GL_NV_parameter_buffer_object \
    X (z0        , glProgramBufferParametersfvNV, (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLfloat *params)) \
    X (z0        , glProgramBufferParametersIivNV, (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLint *params)) \
    X (z0        , glProgramBufferParametersIuivNV, (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLuint *params))

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
    X (z0        , glGetPathTexGenfvNV, (GLenum texCoordSet, GLenum pname, GLfloat *value))

#define DRX_GL_FUNCTIONS_GL_NV_pixel_data_range \
    X (z0        , glPixelDataRangeNV, (GLenum target, GLsizei length, ukk pointer)) \
    X (z0        , glFlushPixelDataRangeNV, (GLenum target))

#define DRX_GL_FUNCTIONS_GL_NV_point_sprite \
    X (z0        , glPointParameteriNV, (GLenum pname, GLint param)) \
    X (z0        , glPointParameterivNV, (GLenum pname, const GLint *params))

#define DRX_GL_FUNCTIONS_GL_NV_present_video \
    X (z0        , glPresentFrameKeyedNV, (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1)) \
    X (z0        , glPresentFrameDualFillNV, (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3)) \
    X (z0        , glGetVideoivNV, (GLuint video_slot, GLenum pname, GLint *params)) \
    X (z0        , glGetVideouivNV, (GLuint video_slot, GLenum pname, GLuint *params)) \
    X (z0        , glGetVideoi64vNV, (GLuint video_slot, GLenum pname, GLint64EXT *params)) \
    X (z0        , glGetVideoui64vNV, (GLuint video_slot, GLenum pname, GLuint64EXT *params))

#define DRX_GL_FUNCTIONS_GL_NV_primitive_restart \
    X (z0        , glPrimitiveRestartNV, ()) \
    X (z0        , glPrimitiveRestartIndexNV, (GLuint index))

#define DRX_GL_FUNCTIONS_GL_NV_query_resource \
    X (GLint       , glQueryResourceNV, (GLenum queryType, GLint tagId, GLuint count, GLint *buffer))

#define DRX_GL_FUNCTIONS_GL_NV_query_resource_tag \
    X (z0        , glGenQueryResourceTagNV, (GLsizei n, GLint *tagIds)) \
    X (z0        , glDeleteQueryResourceTagNV, (GLsizei n, const GLint *tagIds)) \
    X (z0        , glQueryResourceTagNV, (GLint tagId, const GLchar *tagString))

#define DRX_GL_FUNCTIONS_GL_NV_register_combiners \
    X (z0        , glCombinerParameterfvNV, (GLenum pname, const GLfloat *params)) \
    X (z0        , glCombinerParameterfNV, (GLenum pname, GLfloat param)) \
    X (z0        , glCombinerParameterivNV, (GLenum pname, const GLint *params)) \
    X (z0        , glCombinerParameteriNV, (GLenum pname, GLint param)) \
    X (z0        , glCombinerInputNV, (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)) \
    X (z0        , glCombinerOutputNV, (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum)) \
    X (z0        , glFinalCombinerInputNV, (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)) \
    X (z0        , glGetCombinerInputParameterfvNV, (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params)) \
    X (z0        , glGetCombinerInputParameterivNV, (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params)) \
    X (z0        , glGetCombinerOutputParameterfvNV, (GLenum stage, GLenum portion, GLenum pname, GLfloat *params)) \
    X (z0        , glGetCombinerOutputParameterivNV, (GLenum stage, GLenum portion, GLenum pname, GLint *params)) \
    X (z0        , glGetFinalCombinerInputParameterfvNV, (GLenum variable, GLenum pname, GLfloat *params)) \
    X (z0        , glGetFinalCombinerInputParameterivNV, (GLenum variable, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_NV_register_combiners2 \
    X (z0        , glCombinerStageParameterfvNV, (GLenum stage, GLenum pname, const GLfloat *params)) \
    X (z0        , glGetCombinerStageParameterfvNV, (GLenum stage, GLenum pname, GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_NV_sample_locations \
    X (z0        , glFramebufferSampleLocationsfvNV, (GLenum target, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glNamedFramebufferSampleLocationsfvNV, (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (z0        , glResolveDepthValuesNV, ())

#define DRX_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    X (z0        , glScissorExclusiveNV, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (z0        , glScissorExclusiveArrayvNV, (GLuint first, GLsizei count, const GLint *v))

#define DRX_GL_FUNCTIONS_GL_NV_shader_buffer_load \
    X (z0        , glMakeBufferResidentNV, (GLenum target, GLenum access)) \
    X (z0        , glMakeBufferNonResidentNV, (GLenum target)) \
    X (GLboolean   , glIsBufferResidentNV, (GLenum target)) \
    X (z0        , glMakeNamedBufferResidentNV, (GLuint buffer, GLenum access)) \
    X (z0        , glMakeNamedBufferNonResidentNV, (GLuint buffer)) \
    X (GLboolean   , glIsNamedBufferResidentNV, (GLuint buffer)) \
    X (z0        , glGetBufferParameterui64vNV, (GLenum target, GLenum pname, GLuint64EXT *params)) \
    X (z0        , glGetNamedBufferParameterui64vNV, (GLuint buffer, GLenum pname, GLuint64EXT *params)) \
    X (z0        , glGetIntegerui64vNV, (GLenum value, GLuint64EXT *result)) \
    X (z0        , glUniformui64NV, (GLint location, GLuint64EXT value)) \
    X (z0        , glUniformui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (z0        , glProgramUniformui64NV, (GLuint program, GLint location, GLuint64EXT value)) \
    X (z0        , glProgramUniformui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value))

#define DRX_GL_FUNCTIONS_GL_NV_shading_rate_image \
    X (z0        , glBindShadingRateImageNV, (GLuint texture)) \
    X (z0        , glGetShadingRateImagePaletteNV, (GLuint viewport, GLuint entry, GLenum *rate)) \
    X (z0        , glGetShadingRateSampleLocationivNV, (GLenum rate, GLuint samples, GLuint index, GLint *location)) \
    X (z0        , glShadingRateImageBarrierNV, (GLboolean synchronize)) \
    X (z0        , glShadingRateImagePaletteNV, (GLuint viewport, GLuint first, GLsizei count, const GLenum *rates)) \
    X (z0        , glShadingRateSampleOrderNV, (GLenum order)) \
    X (z0        , glShadingRateSampleOrderCustomNV, (GLenum rate, GLuint samples, const GLint *locations))

#define DRX_GL_FUNCTIONS_GL_NV_texture_barrier \
    X (z0        , glTextureBarrierNV, ())

#define DRX_GL_FUNCTIONS_GL_NV_texture_multisample \
    X (z0        , glTexImage2DMultisampleCoverageNV, (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)) \
    X (z0        , glTexImage3DMultisampleCoverageNV, (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)) \
    X (z0        , glTextureImage2DMultisampleNV, (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)) \
    X (z0        , glTextureImage3DMultisampleNV, (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)) \
    X (z0        , glTextureImage2DMultisampleCoverageNV, (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)) \
    X (z0        , glTextureImage3DMultisampleCoverageNV, (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations))

#define DRX_GL_FUNCTIONS_GL_NV_transform_feedback \
    X (z0        , glBeginTransformFeedbackNV, (GLenum primitiveMode)) \
    X (z0        , glEndTransformFeedbackNV, ()) \
    X (z0        , glTransformFeedbackAttribsNV, (GLsizei count, const GLint *attribs, GLenum bufferMode)) \
    X (z0        , glBindBufferRangeNV, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (z0        , glBindBufferOffsetNV, (GLenum target, GLuint index, GLuint buffer, GLintptr offset)) \
    X (z0        , glBindBufferBaseNV, (GLenum target, GLuint index, GLuint buffer)) \
    X (z0        , glTransformFeedbackVaryingsNV, (GLuint program, GLsizei count, const GLint *locations, GLenum bufferMode)) \
    X (z0        , glActiveVaryingNV, (GLuint program, const GLchar *name)) \
    X (GLint       , glGetVaryingLocationNV, (GLuint program, const GLchar *name)) \
    X (z0        , glGetActiveVaryingNV, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)) \
    X (z0        , glGetTransformFeedbackVaryingNV, (GLuint program, GLuint index, GLint *location)) \
    X (z0        , glTransformFeedbackStreamAttribsNV, (GLsizei count, const GLint *attribs, GLsizei nbuffers, const GLint *bufstreams, GLenum bufferMode))

#define DRX_GL_FUNCTIONS_GL_NV_transform_feedback2 \
    X (z0        , glBindTransformFeedbackNV, (GLenum target, GLuint id)) \
    X (z0        , glDeleteTransformFeedbacksNV, (GLsizei n, const GLuint *ids)) \
    X (z0        , glGenTransformFeedbacksNV, (GLsizei n, GLuint *ids)) \
    X (GLboolean   , glIsTransformFeedbackNV, (GLuint id)) \
    X (z0        , glPauseTransformFeedbackNV, ()) \
    X (z0        , glResumeTransformFeedbackNV, ()) \
    X (z0        , glDrawTransformFeedbackNV, (GLenum mode, GLuint id))

#define DRX_GL_FUNCTIONS_GL_NV_vdpau_interop \
    X (z0        , glVDPAUInitNV, (ukk vdpDevice, ukk getProcAddress)) \
    X (z0        , glVDPAUFiniNV, ()) \
    X (GLvdpauSurfaceNV, glVDPAURegisterVideoSurfaceNV, (ukk vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames)) \
    X (GLvdpauSurfaceNV, glVDPAURegisterOutputSurfaceNV, (ukk vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames)) \
    X (GLboolean   , glVDPAUIsSurfaceNV, (GLvdpauSurfaceNV surface)) \
    X (z0        , glVDPAUUnregisterSurfaceNV, (GLvdpauSurfaceNV surface)) \
    X (z0        , glVDPAUGetSurfaceivNV, (GLvdpauSurfaceNV surface, GLenum pname, GLsizei count, GLsizei *length, GLint *values)) \
    X (z0        , glVDPAUSurfaceAccessNV, (GLvdpauSurfaceNV surface, GLenum access)) \
    X (z0        , glVDPAUMapSurfacesNV, (GLsizei numSurfaces, const GLvdpauSurfaceNV *surfaces)) \
    X (z0        , glVDPAUUnmapSurfacesNV, (GLsizei numSurface, const GLvdpauSurfaceNV *surfaces))

#define DRX_GL_FUNCTIONS_GL_NV_vdpau_interop2 \
    X (GLvdpauSurfaceNV, glVDPAURegisterVideoSurfaceWithPictureStructureNV, (ukk vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames, GLboolean isFrameStructure))

#define DRX_GL_FUNCTIONS_GL_NV_vertex_array_range \
    X (z0        , glFlushVertexArrayRangeNV, ()) \
    X (z0        , glVertexArrayRangeNV, (GLsizei length, ukk pointer))

#define DRX_GL_FUNCTIONS_GL_NV_vertex_attrib_integer_64bit \
    X (z0        , glVertexAttribL1i64NV, (GLuint index, GLint64EXT x)) \
    X (z0        , glVertexAttribL2i64NV, (GLuint index, GLint64EXT x, GLint64EXT y)) \
    X (z0        , glVertexAttribL3i64NV, (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z)) \
    X (z0        , glVertexAttribL4i64NV, (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)) \
    X (z0        , glVertexAttribL1i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (z0        , glVertexAttribL2i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (z0        , glVertexAttribL3i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (z0        , glVertexAttribL4i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (z0        , glVertexAttribL1ui64NV, (GLuint index, GLuint64EXT x)) \
    X (z0        , glVertexAttribL2ui64NV, (GLuint index, GLuint64EXT x, GLuint64EXT y)) \
    X (z0        , glVertexAttribL3ui64NV, (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)) \
    X (z0        , glVertexAttribL4ui64NV, (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)) \
    X (z0        , glVertexAttribL1ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (z0        , glVertexAttribL2ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (z0        , glVertexAttribL3ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (z0        , glVertexAttribL4ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (z0        , glGetVertexAttribLi64vNV, (GLuint index, GLenum pname, GLint64EXT *params)) \
    X (z0        , glGetVertexAttribLui64vNV, (GLuint index, GLenum pname, GLuint64EXT *params)) \
    X (z0        , glVertexAttribLFormatNV, (GLuint index, GLint size, GLenum type, GLsizei stride))

#define DRX_GL_FUNCTIONS_GL_NV_vertex_buffer_unified_memory \
    X (z0        , glBufferAddressRangeNV, (GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length)) \
    X (z0        , glVertexFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (z0        , glNormalFormatNV, (GLenum type, GLsizei stride)) \
    X (z0        , glColorFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (z0        , glIndexFormatNV, (GLenum type, GLsizei stride)) \
    X (z0        , glTexCoordFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (z0        , glEdgeFlagFormatNV, (GLsizei stride)) \
    X (z0        , glSecondaryColorFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (z0        , glFogCoordFormatNV, (GLenum type, GLsizei stride)) \
    X (z0        , glVertexAttribFormatNV, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride)) \
    X (z0        , glVertexAttribIFormatNV, (GLuint index, GLint size, GLenum type, GLsizei stride)) \
    X (z0        , glGetIntegerui64i_vNV, (GLenum value, GLuint index, GLuint64EXT *result))

#define DRX_GL_FUNCTIONS_GL_NV_vertex_program \
    X (GLboolean   , glAreProgramsResidentNV, (GLsizei n, const GLuint *programs, GLboolean *residences)) \
    X (z0        , glBindProgramNV, (GLenum target, GLuint id)) \
    X (z0        , glDeleteProgramsNV, (GLsizei n, const GLuint *programs)) \
    X (z0        , glExecuteProgramNV, (GLenum target, GLuint id, const GLfloat *params)) \
    X (z0        , glGenProgramsNV, (GLsizei n, GLuint *programs)) \
    X (z0        , glGetProgramParameterdvNV, (GLenum target, GLuint index, GLenum pname, GLdouble *params)) \
    X (z0        , glGetProgramParameterfvNV, (GLenum target, GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glGetProgramivNV, (GLuint id, GLenum pname, GLint *params)) \
    X (z0        , glGetProgramStringNV, (GLuint id, GLenum pname, GLubyte *program)) \
    X (z0        , glGetTrackMatrixivNV, (GLenum target, GLuint address, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribdvNV, (GLuint index, GLenum pname, GLdouble *params)) \
    X (z0        , glGetVertexAttribfvNV, (GLuint index, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVertexAttribivNV, (GLuint index, GLenum pname, GLint *params)) \
    X (z0        , glGetVertexAttribPointervNV, (GLuint index, GLenum pname, uk *pointer)) \
    X (GLboolean   , glIsProgramNV, (GLuint id)) \
    X (z0        , glLoadProgramNV, (GLenum target, GLuint id, GLsizei len, const GLubyte *program)) \
    X (z0        , glProgramParameter4dNV, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glProgramParameter4dvNV, (GLenum target, GLuint index, const GLdouble *v)) \
    X (z0        , glProgramParameter4fNV, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glProgramParameter4fvNV, (GLenum target, GLuint index, const GLfloat *v)) \
    X (z0        , glProgramParameters4dvNV, (GLenum target, GLuint index, GLsizei count, const GLdouble *v)) \
    X (z0        , glProgramParameters4fvNV, (GLenum target, GLuint index, GLsizei count, const GLfloat *v)) \
    X (z0        , glRequestResidentProgramsNV, (GLsizei n, const GLuint *programs)) \
    X (z0        , glTrackMatrixNV, (GLenum target, GLuint address, GLenum matrix, GLenum transform)) \
    X (z0        , glVertexAttribPointerNV, (GLuint index, GLint fsize, GLenum type, GLsizei stride, ukk pointer)) \
    X (z0        , glVertexAttrib1dNV, (GLuint index, GLdouble x)) \
    X (z0        , glVertexAttrib1dvNV, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib1fNV, (GLuint index, GLfloat x)) \
    X (z0        , glVertexAttrib1fvNV, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib1sNV, (GLuint index, GLshort x)) \
    X (z0        , glVertexAttrib1svNV, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib2dNV, (GLuint index, GLdouble x, GLdouble y)) \
    X (z0        , glVertexAttrib2dvNV, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib2fNV, (GLuint index, GLfloat x, GLfloat y)) \
    X (z0        , glVertexAttrib2fvNV, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib2sNV, (GLuint index, GLshort x, GLshort y)) \
    X (z0        , glVertexAttrib2svNV, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib3dNV, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (z0        , glVertexAttrib3dvNV, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib3fNV, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glVertexAttrib3fvNV, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib3sNV, (GLuint index, GLshort x, GLshort y, GLshort z)) \
    X (z0        , glVertexAttrib3svNV, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4dNV, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (z0        , glVertexAttrib4dvNV, (GLuint index, const GLdouble *v)) \
    X (z0        , glVertexAttrib4fNV, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glVertexAttrib4fvNV, (GLuint index, const GLfloat *v)) \
    X (z0        , glVertexAttrib4sNV, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (z0        , glVertexAttrib4svNV, (GLuint index, const GLshort *v)) \
    X (z0        , glVertexAttrib4ubNV, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)) \
    X (z0        , glVertexAttrib4ubvNV, (GLuint index, const GLubyte *v)) \
    X (z0        , glVertexAttribs1dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (z0        , glVertexAttribs1fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (z0        , glVertexAttribs1svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (z0        , glVertexAttribs2dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (z0        , glVertexAttribs2fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (z0        , glVertexAttribs2svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (z0        , glVertexAttribs3dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (z0        , glVertexAttribs3fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (z0        , glVertexAttribs3svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (z0        , glVertexAttribs4dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (z0        , glVertexAttribs4fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (z0        , glVertexAttribs4svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (z0        , glVertexAttribs4ubvNV, (GLuint index, GLsizei count, const GLubyte *v))

#define DRX_GL_FUNCTIONS_GL_NV_video_capture \
    X (z0        , glBeginVideoCaptureNV, (GLuint video_capture_slot)) \
    X (z0        , glBindVideoCaptureStreamBufferNV, (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset)) \
    X (z0        , glBindVideoCaptureStreamTextureNV, (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture)) \
    X (z0        , glEndVideoCaptureNV, (GLuint video_capture_slot)) \
    X (z0        , glGetVideoCaptureivNV, (GLuint video_capture_slot, GLenum pname, GLint *params)) \
    X (z0        , glGetVideoCaptureStreamivNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, GLint *params)) \
    X (z0        , glGetVideoCaptureStreamfvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat *params)) \
    X (z0        , glGetVideoCaptureStreamdvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble *params)) \
    X (GLenum      , glVideoCaptureNV, (GLuint video_capture_slot, GLuint *sequence_num, GLuint64EXT *capture_time)) \
    X (z0        , glVideoCaptureStreamParameterivNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint *params)) \
    X (z0        , glVideoCaptureStreamParameterfvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat *params)) \
    X (z0        , glVideoCaptureStreamParameterdvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble *params))

#define DRX_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    X (z0        , glViewportSwizzleNV, (GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew))

#define DRX_GL_FUNCTIONS_GL_OES_byte_coordinates \
    X (z0        , glMultiTexCoord1bOES, (GLenum texture, GLbyte s)) \
    X (z0        , glMultiTexCoord1bvOES, (GLenum texture, const GLbyte *coords)) \
    X (z0        , glMultiTexCoord2bOES, (GLenum texture, GLbyte s, GLbyte t)) \
    X (z0        , glMultiTexCoord2bvOES, (GLenum texture, const GLbyte *coords)) \
    X (z0        , glMultiTexCoord3bOES, (GLenum texture, GLbyte s, GLbyte t, GLbyte r)) \
    X (z0        , glMultiTexCoord3bvOES, (GLenum texture, const GLbyte *coords)) \
    X (z0        , glMultiTexCoord4bOES, (GLenum texture, GLbyte s, GLbyte t, GLbyte r, GLbyte q)) \
    X (z0        , glMultiTexCoord4bvOES, (GLenum texture, const GLbyte *coords)) \
    X (z0        , glTexCoord1bOES, (GLbyte s)) \
    X (z0        , glTexCoord1bvOES, (const GLbyte *coords)) \
    X (z0        , glTexCoord2bOES, (GLbyte s, GLbyte t)) \
    X (z0        , glTexCoord2bvOES, (const GLbyte *coords)) \
    X (z0        , glTexCoord3bOES, (GLbyte s, GLbyte t, GLbyte r)) \
    X (z0        , glTexCoord3bvOES, (const GLbyte *coords)) \
    X (z0        , glTexCoord4bOES, (GLbyte s, GLbyte t, GLbyte r, GLbyte q)) \
    X (z0        , glTexCoord4bvOES, (const GLbyte *coords)) \
    X (z0        , glVertex2bOES, (GLbyte x, GLbyte y)) \
    X (z0        , glVertex2bvOES, (const GLbyte *coords)) \
    X (z0        , glVertex3bOES, (GLbyte x, GLbyte y, GLbyte z)) \
    X (z0        , glVertex3bvOES, (const GLbyte *coords)) \
    X (z0        , glVertex4bOES, (GLbyte x, GLbyte y, GLbyte z, GLbyte w)) \
    X (z0        , glVertex4bvOES, (const GLbyte *coords))

#define DRX_GL_FUNCTIONS_GL_OES_fixed_point \
    X (z0        , glAlphaFuncxOES, (GLenum func, GLfixed ref)) \
    X (z0        , glClearColorxOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (z0        , glClearDepthxOES, (GLfixed depth)) \
    X (z0        , glClipPlanexOES, (GLenum plane, const GLfixed *equation)) \
    X (z0        , glColor4xOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (z0        , glDepthRangexOES, (GLfixed n, GLfixed f)) \
    X (z0        , glFogxOES, (GLenum pname, GLfixed param)) \
    X (z0        , glFogxvOES, (GLenum pname, const GLfixed *param)) \
    X (z0        , glFrustumxOES, (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)) \
    X (z0        , glGetClipPlanexOES, (GLenum plane, GLfixed *equation)) \
    X (z0        , glGetFixedvOES, (GLenum pname, GLfixed *params)) \
    X (z0        , glGetTexEnvxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (z0        , glGetTexParameterxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (z0        , glLightModelxOES, (GLenum pname, GLfixed param)) \
    X (z0        , glLightModelxvOES, (GLenum pname, const GLfixed *param)) \
    X (z0        , glLightxOES, (GLenum light, GLenum pname, GLfixed param)) \
    X (z0        , glLightxvOES, (GLenum light, GLenum pname, const GLfixed *params)) \
    X (z0        , glLineWidthxOES, (GLfixed width)) \
    X (z0        , glLoadMatrixxOES, (const GLfixed *m)) \
    X (z0        , glMaterialxOES, (GLenum face, GLenum pname, GLfixed param)) \
    X (z0        , glMaterialxvOES, (GLenum face, GLenum pname, const GLfixed *param)) \
    X (z0        , glMultMatrixxOES, (const GLfixed *m)) \
    X (z0        , glMultiTexCoord4xOES, (GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q)) \
    X (z0        , glNormal3xOES, (GLfixed nx, GLfixed ny, GLfixed nz)) \
    X (z0        , glOrthoxOES, (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)) \
    X (z0        , glPointParameterxvOES, (GLenum pname, const GLfixed *params)) \
    X (z0        , glPointSizexOES, (GLfixed size)) \
    X (z0        , glPolygonOffsetxOES, (GLfixed factor, GLfixed units)) \
    X (z0        , glRotatexOES, (GLfixed angle, GLfixed x, GLfixed y, GLfixed z)) \
    X (z0        , glScalexOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (z0        , glTexEnvxOES, (GLenum target, GLenum pname, GLfixed param)) \
    X (z0        , glTexEnvxvOES, (GLenum target, GLenum pname, const GLfixed *params)) \
    X (z0        , glTexParameterxOES, (GLenum target, GLenum pname, GLfixed param)) \
    X (z0        , glTexParameterxvOES, (GLenum target, GLenum pname, const GLfixed *params)) \
    X (z0        , glTranslatexOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (z0        , glGetLightxvOES, (GLenum light, GLenum pname, GLfixed *params)) \
    X (z0        , glGetMaterialxvOES, (GLenum face, GLenum pname, GLfixed *params)) \
    X (z0        , glPointParameterxOES, (GLenum pname, GLfixed param)) \
    X (z0        , glSampleCoveragexOES, (GLclampx value, GLboolean invert)) \
    X (z0        , glAccumxOES, (GLenum op, GLfixed value)) \
    X (z0        , glBitmapxOES, (GLsizei width, GLsizei height, GLfixed xorig, GLfixed yorig, GLfixed xmove, GLfixed ymove, const GLubyte *bitmap)) \
    X (z0        , glBlendColorxOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (z0        , glClearAccumxOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (z0        , glColor3xOES, (GLfixed red, GLfixed green, GLfixed blue)) \
    X (z0        , glColor3xvOES, (const GLfixed *components)) \
    X (z0        , glColor4xvOES, (const GLfixed *components)) \
    X (z0        , glConvolutionParameterxOES, (GLenum target, GLenum pname, GLfixed param)) \
    X (z0        , glConvolutionParameterxvOES, (GLenum target, GLenum pname, const GLfixed *params)) \
    X (z0        , glEvalCoord1xOES, (GLfixed u)) \
    X (z0        , glEvalCoord1xvOES, (const GLfixed *coords)) \
    X (z0        , glEvalCoord2xOES, (GLfixed u, GLfixed v)) \
    X (z0        , glEvalCoord2xvOES, (const GLfixed *coords)) \
    X (z0        , glFeedbackBufferxOES, (GLsizei n, GLenum type, const GLfixed *buffer)) \
    X (z0        , glGetConvolutionParameterxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (z0        , glGetHistogramParameterxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (z0        , glGetLightxOES, (GLenum light, GLenum pname, GLfixed *params)) \
    X (z0        , glGetMapxvOES, (GLenum target, GLenum query, GLfixed *v)) \
    X (z0        , glGetMaterialxOES, (GLenum face, GLenum pname, GLfixed param)) \
    X (z0        , glGetPixelMapxv, (GLenum map, GLint size, GLfixed *values)) \
    X (z0        , glGetTexGenxvOES, (GLenum coord, GLenum pname, GLfixed *params)) \
    X (z0        , glGetTexLevelParameterxvOES, (GLenum target, GLint level, GLenum pname, GLfixed *params)) \
    X (z0        , glIndexxOES, (GLfixed component)) \
    X (z0        , glIndexxvOES, (const GLfixed *component)) \
    X (z0        , glLoadTransposeMatrixxOES, (const GLfixed *m)) \
    X (z0        , glMap1xOES, (GLenum target, GLfixed u1, GLfixed u2, GLint stride, GLint order, GLfixed points)) \
    X (z0        , glMap2xOES, (GLenum target, GLfixed u1, GLfixed u2, GLint ustride, GLint uorder, GLfixed v1, GLfixed v2, GLint vstride, GLint vorder, GLfixed points)) \
    X (z0        , glMapGrid1xOES, (GLint n, GLfixed u1, GLfixed u2)) \
    X (z0        , glMapGrid2xOES, (GLint n, GLfixed u1, GLfixed u2, GLfixed v1, GLfixed v2)) \
    X (z0        , glMultTransposeMatrixxOES, (const GLfixed *m)) \
    X (z0        , glMultiTexCoord1xOES, (GLenum texture, GLfixed s)) \
    X (z0        , glMultiTexCoord1xvOES, (GLenum texture, const GLfixed *coords)) \
    X (z0        , glMultiTexCoord2xOES, (GLenum texture, GLfixed s, GLfixed t)) \
    X (z0        , glMultiTexCoord2xvOES, (GLenum texture, const GLfixed *coords)) \
    X (z0        , glMultiTexCoord3xOES, (GLenum texture, GLfixed s, GLfixed t, GLfixed r)) \
    X (z0        , glMultiTexCoord3xvOES, (GLenum texture, const GLfixed *coords)) \
    X (z0        , glMultiTexCoord4xvOES, (GLenum texture, const GLfixed *coords)) \
    X (z0        , glNormal3xvOES, (const GLfixed *coords)) \
    X (z0        , glPassThroughxOES, (GLfixed token)) \
    X (z0        , glPixelMapx, (GLenum map, GLint size, const GLfixed *values)) \
    X (z0        , glPixelStorex, (GLenum pname, GLfixed param)) \
    X (z0        , glPixelTransferxOES, (GLenum pname, GLfixed param)) \
    X (z0        , glPixelZoomxOES, (GLfixed xfactor, GLfixed yfactor)) \
    X (z0        , glPrioritizeTexturesxOES, (GLsizei n, const GLuint *textures, const GLfixed *priorities)) \
    X (z0        , glRasterPos2xOES, (GLfixed x, GLfixed y)) \
    X (z0        , glRasterPos2xvOES, (const GLfixed *coords)) \
    X (z0        , glRasterPos3xOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (z0        , glRasterPos3xvOES, (const GLfixed *coords)) \
    X (z0        , glRasterPos4xOES, (GLfixed x, GLfixed y, GLfixed z, GLfixed w)) \
    X (z0        , glRasterPos4xvOES, (const GLfixed *coords)) \
    X (z0        , glRectxOES, (GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2)) \
    X (z0        , glRectxvOES, (const GLfixed *v1, const GLfixed *v2)) \
    X (z0        , glTexCoord1xOES, (GLfixed s)) \
    X (z0        , glTexCoord1xvOES, (const GLfixed *coords)) \
    X (z0        , glTexCoord2xOES, (GLfixed s, GLfixed t)) \
    X (z0        , glTexCoord2xvOES, (const GLfixed *coords)) \
    X (z0        , glTexCoord3xOES, (GLfixed s, GLfixed t, GLfixed r)) \
    X (z0        , glTexCoord3xvOES, (const GLfixed *coords)) \
    X (z0        , glTexCoord4xOES, (GLfixed s, GLfixed t, GLfixed r, GLfixed q)) \
    X (z0        , glTexCoord4xvOES, (const GLfixed *coords)) \
    X (z0        , glTexGenxOES, (GLenum coord, GLenum pname, GLfixed param)) \
    X (z0        , glTexGenxvOES, (GLenum coord, GLenum pname, const GLfixed *params)) \
    X (z0        , glVertex2xOES, (GLfixed x)) \
    X (z0        , glVertex2xvOES, (const GLfixed *coords)) \
    X (z0        , glVertex3xOES, (GLfixed x, GLfixed y)) \
    X (z0        , glVertex3xvOES, (const GLfixed *coords)) \
    X (z0        , glVertex4xOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (z0        , glVertex4xvOES, (const GLfixed *coords))

#define DRX_GL_FUNCTIONS_GL_OES_query_matrix \
    X (GLbitfield  , glQueryMatrixxOES, (GLfixed *mantissa, GLint *exponent))

#define DRX_GL_FUNCTIONS_GL_OES_single_precision \
    X (z0        , glClearDepthfOES, (GLclampf depth)) \
    X (z0        , glClipPlanefOES, (GLenum plane, const GLfloat *equation)) \
    X (z0        , glDepthRangefOES, (GLclampf n, GLclampf f)) \
    X (z0        , glFrustumfOES, (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)) \
    X (z0        , glGetClipPlanefOES, (GLenum plane, GLfloat *equation)) \
    X (z0        , glOrthofOES, (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f))

#define DRX_GL_FUNCTIONS_GL_OVR_multiview \
    X (z0        , glFramebufferTextureMultiviewOVR, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews))

#define DRX_GL_FUNCTIONS_GL_PGI_misc_hints \
    X (z0        , glHintPGI, (GLenum target, GLint mode))

#define DRX_GL_FUNCTIONS_GL_SGIS_detail_texture \
    X (z0        , glDetailTexFuncSGIS, (GLenum target, GLsizei n, const GLfloat *points)) \
    X (z0        , glGetDetailTexFuncSGIS, (GLenum target, GLfloat *points))

#define DRX_GL_FUNCTIONS_GL_SGIS_fog_function \
    X (z0        , glFogFuncSGIS, (GLsizei n, const GLfloat *points)) \
    X (z0        , glGetFogFuncSGIS, (GLfloat *points))

#define DRX_GL_FUNCTIONS_GL_SGIS_multisample \
    X (z0        , glSampleMaskSGIS, (GLclampf value, GLboolean invert)) \
    X (z0        , glSamplePatternSGIS, (GLenum pattern))

#define DRX_GL_FUNCTIONS_GL_SGIS_pixel_texture \
    X (z0        , glPixelTexGenParameteriSGIS, (GLenum pname, GLint param)) \
    X (z0        , glPixelTexGenParameterivSGIS, (GLenum pname, const GLint *params)) \
    X (z0        , glPixelTexGenParameterfSGIS, (GLenum pname, GLfloat param)) \
    X (z0        , glPixelTexGenParameterfvSGIS, (GLenum pname, const GLfloat *params)) \
    X (z0        , glGetPixelTexGenParameterivSGIS, (GLenum pname, GLint *params)) \
    X (z0        , glGetPixelTexGenParameterfvSGIS, (GLenum pname, GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_SGIS_point_parameters \
    X (z0        , glPointParameterfSGIS, (GLenum pname, GLfloat param)) \
    X (z0        , glPointParameterfvSGIS, (GLenum pname, const GLfloat *params))

#define DRX_GL_FUNCTIONS_GL_SGIS_sharpen_texture \
    X (z0        , glSharpenTexFuncSGIS, (GLenum target, GLsizei n, const GLfloat *points)) \
    X (z0        , glGetSharpenTexFuncSGIS, (GLenum target, GLfloat *points))

#define DRX_GL_FUNCTIONS_GL_SGIS_texture4D \
    X (z0        , glTexImage4DSGIS, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, ukk pixels)) \
    X (z0        , glTexSubImage4DSGIS, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, ukk pixels))

#define DRX_GL_FUNCTIONS_GL_SGIS_texture_color_mask \
    X (z0        , glTextureColorMaskSGIS, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))

#define DRX_GL_FUNCTIONS_GL_SGIS_texture_filter4 \
    X (z0        , glGetTexFilterFuncSGIS, (GLenum target, GLenum filter, GLfloat *weights)) \
    X (z0        , glTexFilterFuncSGIS, (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights))

#define DRX_GL_FUNCTIONS_GL_SGIX_async \
    X (z0        , glAsyncMarkerSGIX, (GLuint marker)) \
    X (GLint       , glFinishAsyncSGIX, (GLuint *markerp)) \
    X (GLint       , glPollAsyncSGIX, (GLuint *markerp)) \
    X (GLuint      , glGenAsyncMarkersSGIX, (GLsizei range)) \
    X (z0        , glDeleteAsyncMarkersSGIX, (GLuint marker, GLsizei range)) \
    X (GLboolean   , glIsAsyncMarkerSGIX, (GLuint marker))

#define DRX_GL_FUNCTIONS_GL_SGIX_flush_raster \
    X (z0        , glFlushRasterSGIX, ())

#define DRX_GL_FUNCTIONS_GL_SGIX_fragment_lighting \
    X (z0        , glFragmentColorMaterialSGIX, (GLenum face, GLenum mode)) \
    X (z0        , glFragmentLightfSGIX, (GLenum light, GLenum pname, GLfloat param)) \
    X (z0        , glFragmentLightfvSGIX, (GLenum light, GLenum pname, const GLfloat *params)) \
    X (z0        , glFragmentLightiSGIX, (GLenum light, GLenum pname, GLint param)) \
    X (z0        , glFragmentLightivSGIX, (GLenum light, GLenum pname, const GLint *params)) \
    X (z0        , glFragmentLightModelfSGIX, (GLenum pname, GLfloat param)) \
    X (z0        , glFragmentLightModelfvSGIX, (GLenum pname, const GLfloat *params)) \
    X (z0        , glFragmentLightModeliSGIX, (GLenum pname, GLint param)) \
    X (z0        , glFragmentLightModelivSGIX, (GLenum pname, const GLint *params)) \
    X (z0        , glFragmentMaterialfSGIX, (GLenum face, GLenum pname, GLfloat param)) \
    X (z0        , glFragmentMaterialfvSGIX, (GLenum face, GLenum pname, const GLfloat *params)) \
    X (z0        , glFragmentMaterialiSGIX, (GLenum face, GLenum pname, GLint param)) \
    X (z0        , glFragmentMaterialivSGIX, (GLenum face, GLenum pname, const GLint *params)) \
    X (z0        , glGetFragmentLightfvSGIX, (GLenum light, GLenum pname, GLfloat *params)) \
    X (z0        , glGetFragmentLightivSGIX, (GLenum light, GLenum pname, GLint *params)) \
    X (z0        , glGetFragmentMaterialfvSGIX, (GLenum face, GLenum pname, GLfloat *params)) \
    X (z0        , glGetFragmentMaterialivSGIX, (GLenum face, GLenum pname, GLint *params)) \
    X (z0        , glLightEnviSGIX, (GLenum pname, GLint param))

#define DRX_GL_FUNCTIONS_GL_SGIX_framezoom \
    X (z0        , glFrameZoomSGIX, (GLint factor))

#define DRX_GL_FUNCTIONS_GL_SGIX_igloo_interface \
    X (z0        , glIglooInterfaceSGIX, (GLenum pname, ukk params))

#define DRX_GL_FUNCTIONS_GL_SGIX_instruments \
    X (GLint       , glGetInstrumentsSGIX, ()) \
    X (z0        , glInstrumentsBufferSGIX, (GLsizei size, GLint *buffer)) \
    X (GLint       , glPollInstrumentsSGIX, (GLint *marker_p)) \
    X (z0        , glReadInstrumentsSGIX, (GLint marker)) \
    X (z0        , glStartInstrumentsSGIX, ()) \
    X (z0        , glStopInstrumentsSGIX, (GLint marker))

#define DRX_GL_FUNCTIONS_GL_SGIX_list_priority \
    X (z0        , glGetListParameterfvSGIX, (GLuint list, GLenum pname, GLfloat *params)) \
    X (z0        , glGetListParameterivSGIX, (GLuint list, GLenum pname, GLint *params)) \
    X (z0        , glListParameterfSGIX, (GLuint list, GLenum pname, GLfloat param)) \
    X (z0        , glListParameterfvSGIX, (GLuint list, GLenum pname, const GLfloat *params)) \
    X (z0        , glListParameteriSGIX, (GLuint list, GLenum pname, GLint param)) \
    X (z0        , glListParameterivSGIX, (GLuint list, GLenum pname, const GLint *params))

#define DRX_GL_FUNCTIONS_GL_SGIX_pixel_texture \
    X (z0        , glPixelTexGenSGIX, (GLenum mode))

#define DRX_GL_FUNCTIONS_GL_SGIX_polynomial_ffd \
    X (z0        , glDeformationMap3dSGIX, (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble *points)) \
    X (z0        , glDeformationMap3fSGIX, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat *points)) \
    X (z0        , glDeformSGIX, (GLbitfield mask)) \
    X (z0        , glLoadIdentityDeformationMapSGIX, (GLbitfield mask))

#define DRX_GL_FUNCTIONS_GL_SGIX_reference_plane \
    X (z0        , glReferencePlaneSGIX, (const GLdouble *equation))

#define DRX_GL_FUNCTIONS_GL_SGIX_sprite \
    X (z0        , glSpriteParameterfSGIX, (GLenum pname, GLfloat param)) \
    X (z0        , glSpriteParameterfvSGIX, (GLenum pname, const GLfloat *params)) \
    X (z0        , glSpriteParameteriSGIX, (GLenum pname, GLint param)) \
    X (z0        , glSpriteParameterivSGIX, (GLenum pname, const GLint *params))

#define DRX_GL_FUNCTIONS_GL_SGIX_tag_sample_buffer \
    X (z0        , glTagSampleBufferSGIX, ())

#define DRX_GL_FUNCTIONS_GL_SGI_color_table \
    X (z0        , glColorTableSGI, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, ukk table)) \
    X (z0        , glColorTableParameterfvSGI, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (z0        , glColorTableParameterivSGI, (GLenum target, GLenum pname, const GLint *params)) \
    X (z0        , glCopyColorTableSGI, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (z0        , glGetColorTableSGI, (GLenum target, GLenum format, GLenum type, uk table)) \
    X (z0        , glGetColorTableParameterfvSGI, (GLenum target, GLenum pname, GLfloat *params)) \
    X (z0        , glGetColorTableParameterivSGI, (GLenum target, GLenum pname, GLint *params))

#define DRX_GL_FUNCTIONS_GL_SUNX_constant_data \
    X (z0        , glFinishTextureSUNX, ())

#define DRX_GL_FUNCTIONS_GL_SUN_global_alpha \
    X (z0        , glGlobalAlphaFactorbSUN, (GLbyte factor)) \
    X (z0        , glGlobalAlphaFactorsSUN, (GLshort factor)) \
    X (z0        , glGlobalAlphaFactoriSUN, (GLint factor)) \
    X (z0        , glGlobalAlphaFactorfSUN, (GLfloat factor)) \
    X (z0        , glGlobalAlphaFactordSUN, (GLdouble factor)) \
    X (z0        , glGlobalAlphaFactorubSUN, (GLubyte factor)) \
    X (z0        , glGlobalAlphaFactorusSUN, (GLushort factor)) \
    X (z0        , glGlobalAlphaFactoruiSUN, (GLuint factor))

#define DRX_GL_FUNCTIONS_GL_SUN_mesh_array \
    X (z0        , glDrawMeshArraysSUN, (GLenum mode, GLint first, GLsizei count, GLsizei width))

#define DRX_GL_FUNCTIONS_GL_SUN_triangle_list \
    X (z0        , glReplacementCodeuiSUN, (GLuint code)) \
    X (z0        , glReplacementCodeusSUN, (GLushort code)) \
    X (z0        , glReplacementCodeubSUN, (GLubyte code)) \
    X (z0        , glReplacementCodeuivSUN, (const GLuint *code)) \
    X (z0        , glReplacementCodeusvSUN, (const GLushort *code)) \
    X (z0        , glReplacementCodeubvSUN, (const GLubyte *code)) \
    X (z0        , glReplacementCodePointerSUN, (GLenum type, GLsizei stride, ukk *pointer))

#define DRX_GL_FUNCTIONS_GL_SUN_vertex \
    X (z0        , glColor4ubVertex2fSUN, (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y)) \
    X (z0        , glColor4ubVertex2fvSUN, (const GLubyte *c, const GLfloat *v)) \
    X (z0        , glColor4ubVertex3fSUN, (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glColor4ubVertex3fvSUN, (const GLubyte *c, const GLfloat *v)) \
    X (z0        , glColor3fVertex3fSUN, (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glColor3fVertex3fvSUN, (const GLfloat *c, const GLfloat *v)) \
    X (z0        , glNormal3fVertex3fSUN, (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glNormal3fVertex3fvSUN, (const GLfloat *n, const GLfloat *v)) \
    X (z0        , glColor4fNormal3fVertex3fSUN, (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glColor4fNormal3fVertex3fvSUN, (const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glTexCoord2fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glTexCoord2fVertex3fvSUN, (const GLfloat *tc, const GLfloat *v)) \
    X (z0        , glTexCoord4fVertex4fSUN, (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glTexCoord4fVertex4fvSUN, (const GLfloat *tc, const GLfloat *v)) \
    X (z0        , glTexCoord2fColor4ubVertex3fSUN, (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glTexCoord2fColor4ubVertex3fvSUN, (const GLfloat *tc, const GLubyte *c, const GLfloat *v)) \
    X (z0        , glTexCoord2fColor3fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glTexCoord2fColor3fVertex3fvSUN, (const GLfloat *tc, const GLfloat *c, const GLfloat *v)) \
    X (z0        , glTexCoord2fNormal3fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glTexCoord2fNormal3fVertex3fvSUN, (const GLfloat *tc, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glTexCoord2fColor4fNormal3fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glTexCoord2fColor4fNormal3fVertex3fvSUN, (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glTexCoord4fColor4fNormal3fVertex4fSUN, (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (z0        , glTexCoord4fColor4fNormal3fVertex4fvSUN, (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiVertex3fSUN, (GLuint rc, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiVertex3fvSUN, (const GLuint *rc, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiColor4ubVertex3fSUN, (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiColor4ubVertex3fvSUN, (const GLuint *rc, const GLubyte *c, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiColor3fVertex3fSUN, (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiColor3fVertex3fvSUN, (const GLuint *rc, const GLfloat *c, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiNormal3fVertex3fSUN, (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiColor4fNormal3fVertex3fSUN, (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiColor4fNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiTexCoord2fVertex3fSUN, (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiTexCoord2fVertex3fvSUN, (const GLuint *rc, const GLfloat *tc, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN, (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v)) \
    X (z0        , glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN, (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (z0        , glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v))


#if DRX_STATIC_LINK_GL_VERSION_1_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_0_STATIC DRX_GL_FUNCTIONS_GL_VERSION_1_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_0_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_0_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_0_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_1_0
#endif

#if DRX_STATIC_LINK_GL_VERSION_1_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_1_STATIC DRX_GL_FUNCTIONS_GL_VERSION_1_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_1_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_1_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_1_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_1_1
#endif

#if DRX_STATIC_LINK_GL_VERSION_1_2
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_2_STATIC DRX_GL_FUNCTIONS_GL_VERSION_1_2
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_2_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_2_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_2_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_1_2
#endif

#if DRX_STATIC_LINK_GL_VERSION_1_3
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_3_STATIC DRX_GL_FUNCTIONS_GL_VERSION_1_3
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_3_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_3_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_3_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_1_3
#endif

#if DRX_STATIC_LINK_GL_VERSION_1_4
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_4_STATIC DRX_GL_FUNCTIONS_GL_VERSION_1_4
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_4_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_4_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_4_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_1_4
#endif

#if DRX_STATIC_LINK_GL_VERSION_1_5
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_5_STATIC DRX_GL_FUNCTIONS_GL_VERSION_1_5
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_5_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_5_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_1_5_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_1_5
#endif

#if DRX_STATIC_LINK_GL_VERSION_2_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_0_STATIC DRX_GL_FUNCTIONS_GL_VERSION_2_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_0_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_0_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_0_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_2_0
#endif

#if DRX_STATIC_LINK_GL_VERSION_2_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_1_STATIC DRX_GL_FUNCTIONS_GL_VERSION_2_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_1_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_1_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_2_1_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_2_1
#endif

#if DRX_STATIC_LINK_GL_VERSION_3_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_0_STATIC DRX_GL_FUNCTIONS_GL_VERSION_3_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_0_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_0_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_0_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_3_0
#endif

#if DRX_STATIC_LINK_GL_VERSION_3_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_1_STATIC DRX_GL_FUNCTIONS_GL_VERSION_3_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_1_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_1_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_1_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_3_1
#endif

#if DRX_STATIC_LINK_GL_VERSION_3_2
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_2_STATIC DRX_GL_FUNCTIONS_GL_VERSION_3_2
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_2_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_2_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_2_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_3_2
#endif

#if DRX_STATIC_LINK_GL_VERSION_3_3
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_3_STATIC DRX_GL_FUNCTIONS_GL_VERSION_3_3
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_3_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_3_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_3_3_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_3_3
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_0_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_0
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_0_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_0_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_0_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_0
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_1_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_1
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_1_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_1_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_1_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_1
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_2
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_2_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_2
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_2_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_2_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_2_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_2
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_3
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_3_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_3
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_3_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_3_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_3_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_3
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_4
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_4_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_4
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_4_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_4_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_4_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_4
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_5
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_5_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_5
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_5_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_5_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_5_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_5
#endif

#if DRX_STATIC_LINK_GL_VERSION_4_6
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_6_STATIC DRX_GL_FUNCTIONS_GL_VERSION_4_6
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_6_DYNAMIC
#else
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_6_STATIC
 #define DRX_GL_FUNCTIONS_GL_VERSION_4_6_DYNAMIC DRX_GL_FUNCTIONS_GL_VERSION_4_6
#endif


#define DRX_STATIC_GL_FUNCTIONS \
    DRX_GL_FUNCTIONS_GL_VERSION_1_0_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_1_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_2_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_3_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_4_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_5_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_2_0_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_2_1_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_0_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_1_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_2_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_3_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_0_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_1_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_2_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_3_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_4_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_5_STATIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_6_STATIC

#define DRX_DYNAMIC_GL_FUNCTIONS \
    DRX_GL_FUNCTIONS_GL_VERSION_1_0_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_1_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_2_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_3_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_4_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_1_5_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_2_0_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_2_1_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_0_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_1_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_2_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_3_3_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_0_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_1_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_2_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_3_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_4_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_5_DYNAMIC \
    DRX_GL_FUNCTIONS_GL_VERSION_4_6_DYNAMIC

#define DRX_EXTENSION_GL_FUNCTIONS \
    DRX_GL_FUNCTIONS_GL_3DFX_tbuffer \
    DRX_GL_FUNCTIONS_GL_AMD_debug_output \
    DRX_GL_FUNCTIONS_GL_AMD_draw_buffers_blend \
    DRX_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    DRX_GL_FUNCTIONS_GL_AMD_framebuffer_sample_positions \
    DRX_GL_FUNCTIONS_GL_AMD_gpu_shader_int64 \
    DRX_GL_FUNCTIONS_GL_AMD_interleaved_elements \
    DRX_GL_FUNCTIONS_GL_AMD_multi_draw_indirect \
    DRX_GL_FUNCTIONS_GL_AMD_name_gen_delete \
    DRX_GL_FUNCTIONS_GL_AMD_occlusion_query_event \
    DRX_GL_FUNCTIONS_GL_AMD_performance_monitor \
    DRX_GL_FUNCTIONS_GL_AMD_sample_positions \
    DRX_GL_FUNCTIONS_GL_AMD_sparse_texture \
    DRX_GL_FUNCTIONS_GL_AMD_stencil_operation_extended \
    DRX_GL_FUNCTIONS_GL_AMD_vertex_shader_tessellator \
    DRX_GL_FUNCTIONS_GL_APPLE_element_array \
    DRX_GL_FUNCTIONS_GL_APPLE_fence \
    DRX_GL_FUNCTIONS_GL_APPLE_flush_buffer_range \
    DRX_GL_FUNCTIONS_GL_APPLE_object_purgeable \
    DRX_GL_FUNCTIONS_GL_APPLE_texture_range \
    DRX_GL_FUNCTIONS_GL_APPLE_vertex_array_object \
    DRX_GL_FUNCTIONS_GL_APPLE_vertex_array_range \
    DRX_GL_FUNCTIONS_GL_APPLE_vertex_program_evaluators \
    DRX_GL_FUNCTIONS_GL_ARB_ES3_2_compatibility \
    DRX_GL_FUNCTIONS_GL_ARB_bindless_texture \
    DRX_GL_FUNCTIONS_GL_ARB_cl_event \
    DRX_GL_FUNCTIONS_GL_ARB_color_buffer_float \
    DRX_GL_FUNCTIONS_GL_ARB_compute_variable_group_size \
    DRX_GL_FUNCTIONS_GL_ARB_debug_output \
    DRX_GL_FUNCTIONS_GL_ARB_draw_buffers \
    DRX_GL_FUNCTIONS_GL_ARB_draw_buffers_blend \
    DRX_GL_FUNCTIONS_GL_ARB_draw_instanced \
    DRX_GL_FUNCTIONS_GL_ARB_fragment_program \
    DRX_GL_FUNCTIONS_GL_ARB_geometry_shader4 \
    DRX_GL_FUNCTIONS_GL_ARB_gl_spirv \
    DRX_GL_FUNCTIONS_GL_ARB_gpu_shader_int64 \
    DRX_GL_FUNCTIONS_GL_ARB_imaging \
    DRX_GL_FUNCTIONS_GL_ARB_indirect_parameters \
    DRX_GL_FUNCTIONS_GL_ARB_instanced_arrays \
    DRX_GL_FUNCTIONS_GL_ARB_matrix_palette \
    DRX_GL_FUNCTIONS_GL_ARB_multisample \
    DRX_GL_FUNCTIONS_GL_ARB_multitexture \
    DRX_GL_FUNCTIONS_GL_ARB_occlusion_query \
    DRX_GL_FUNCTIONS_GL_ARB_parallel_shader_compile \
    DRX_GL_FUNCTIONS_GL_ARB_point_parameters \
    DRX_GL_FUNCTIONS_GL_ARB_robustness \
    DRX_GL_FUNCTIONS_GL_ARB_sample_locations \
    DRX_GL_FUNCTIONS_GL_ARB_sample_shading \
    DRX_GL_FUNCTIONS_GL_ARB_shader_objects \
    DRX_GL_FUNCTIONS_GL_ARB_shading_language_include \
    DRX_GL_FUNCTIONS_GL_ARB_sparse_buffer \
    DRX_GL_FUNCTIONS_GL_ARB_sparse_texture \
    DRX_GL_FUNCTIONS_GL_ARB_texture_buffer_object \
    DRX_GL_FUNCTIONS_GL_ARB_texture_compression \
    DRX_GL_FUNCTIONS_GL_ARB_transpose_matrix \
    DRX_GL_FUNCTIONS_GL_ARB_vertex_blend \
    DRX_GL_FUNCTIONS_GL_ARB_vertex_buffer_object \
    DRX_GL_FUNCTIONS_GL_ARB_vertex_program \
    DRX_GL_FUNCTIONS_GL_ARB_vertex_shader \
    DRX_GL_FUNCTIONS_GL_ARB_viewport_array \
    DRX_GL_FUNCTIONS_GL_ARB_window_pos \
    DRX_GL_FUNCTIONS_GL_ATI_draw_buffers \
    DRX_GL_FUNCTIONS_GL_ATI_element_array \
    DRX_GL_FUNCTIONS_GL_ATI_envmap_bumpmap \
    DRX_GL_FUNCTIONS_GL_ATI_fragment_shader \
    DRX_GL_FUNCTIONS_GL_ATI_map_object_buffer \
    DRX_GL_FUNCTIONS_GL_ATI_pn_triangles \
    DRX_GL_FUNCTIONS_GL_ATI_separate_stencil \
    DRX_GL_FUNCTIONS_GL_ATI_vertex_array_object \
    DRX_GL_FUNCTIONS_GL_ATI_vertex_attrib_array_object \
    DRX_GL_FUNCTIONS_GL_ATI_vertex_streams \
    DRX_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    DRX_GL_FUNCTIONS_GL_EXT_bindable_uniform \
    DRX_GL_FUNCTIONS_GL_EXT_blend_color \
    DRX_GL_FUNCTIONS_GL_EXT_blend_equation_separate \
    DRX_GL_FUNCTIONS_GL_EXT_blend_func_separate \
    DRX_GL_FUNCTIONS_GL_EXT_blend_minmax \
    DRX_GL_FUNCTIONS_GL_EXT_color_subtable \
    DRX_GL_FUNCTIONS_GL_EXT_compiled_vertex_array \
    DRX_GL_FUNCTIONS_GL_EXT_convolution \
    DRX_GL_FUNCTIONS_GL_EXT_coordinate_frame \
    DRX_GL_FUNCTIONS_GL_EXT_copy_texture \
    DRX_GL_FUNCTIONS_GL_EXT_cull_vertex \
    DRX_GL_FUNCTIONS_GL_EXT_debug_label \
    DRX_GL_FUNCTIONS_GL_EXT_debug_marker \
    DRX_GL_FUNCTIONS_GL_EXT_depth_bounds_test \
    DRX_GL_FUNCTIONS_GL_EXT_direct_state_access \
    DRX_GL_FUNCTIONS_GL_EXT_draw_buffers2 \
    DRX_GL_FUNCTIONS_GL_EXT_draw_instanced \
    DRX_GL_FUNCTIONS_GL_EXT_draw_range_elements \
    DRX_GL_FUNCTIONS_GL_EXT_external_buffer \
    DRX_GL_FUNCTIONS_GL_EXT_fog_coord \
    DRX_GL_FUNCTIONS_GL_EXT_framebuffer_blit \
    DRX_GL_FUNCTIONS_GL_EXT_framebuffer_blit_layers \
    DRX_GL_FUNCTIONS_GL_EXT_framebuffer_multisample \
    DRX_GL_FUNCTIONS_GL_EXT_framebuffer_object \
    DRX_GL_FUNCTIONS_GL_EXT_geometry_shader4 \
    DRX_GL_FUNCTIONS_GL_EXT_gpu_program_parameters \
    DRX_GL_FUNCTIONS_GL_EXT_gpu_shader4 \
    DRX_GL_FUNCTIONS_GL_EXT_histogram \
    DRX_GL_FUNCTIONS_GL_EXT_index_func \
    DRX_GL_FUNCTIONS_GL_EXT_index_material \
    DRX_GL_FUNCTIONS_GL_EXT_light_texture \
    DRX_GL_FUNCTIONS_GL_EXT_memory_object \
    DRX_GL_FUNCTIONS_GL_EXT_memory_object_fd \
    DRX_GL_FUNCTIONS_GL_EXT_memory_object_win32 \
    DRX_GL_FUNCTIONS_GL_EXT_multi_draw_arrays \
    DRX_GL_FUNCTIONS_GL_EXT_multisample \
    DRX_GL_FUNCTIONS_GL_EXT_paletted_texture \
    DRX_GL_FUNCTIONS_GL_EXT_pixel_transform \
    DRX_GL_FUNCTIONS_GL_EXT_point_parameters \
    DRX_GL_FUNCTIONS_GL_EXT_polygon_offset \
    DRX_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    DRX_GL_FUNCTIONS_GL_EXT_provoking_vertex \
    DRX_GL_FUNCTIONS_GL_EXT_raster_multisample \
    DRX_GL_FUNCTIONS_GL_EXT_semaphore \
    DRX_GL_FUNCTIONS_GL_EXT_semaphore_fd \
    DRX_GL_FUNCTIONS_GL_EXT_semaphore_win32 \
    DRX_GL_FUNCTIONS_GL_EXT_secondary_color \
    DRX_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    DRX_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    DRX_GL_FUNCTIONS_GL_EXT_shader_image_load_store \
    DRX_GL_FUNCTIONS_GL_EXT_stencil_clear_tag \
    DRX_GL_FUNCTIONS_GL_EXT_stencil_two_side \
    DRX_GL_FUNCTIONS_GL_EXT_subtexture \
    DRX_GL_FUNCTIONS_GL_EXT_texture3D \
    DRX_GL_FUNCTIONS_GL_EXT_texture_array \
    DRX_GL_FUNCTIONS_GL_EXT_texture_buffer_object \
    DRX_GL_FUNCTIONS_GL_EXT_texture_integer \
    DRX_GL_FUNCTIONS_GL_EXT_texture_object \
    DRX_GL_FUNCTIONS_GL_EXT_texture_perturb_normal \
    DRX_GL_FUNCTIONS_GL_EXT_texture_storage \
    DRX_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    DRX_GL_FUNCTIONS_GL_EXT_timer_query \
    DRX_GL_FUNCTIONS_GL_EXT_transform_feedback \
    DRX_GL_FUNCTIONS_GL_EXT_vertex_array \
    DRX_GL_FUNCTIONS_GL_EXT_vertex_attrib_64bit \
    DRX_GL_FUNCTIONS_GL_EXT_vertex_shader \
    DRX_GL_FUNCTIONS_GL_EXT_vertex_weighting \
    DRX_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    DRX_GL_FUNCTIONS_GL_EXT_window_rectangles \
    DRX_GL_FUNCTIONS_GL_EXT_x11_sync_object \
    DRX_GL_FUNCTIONS_GL_GREMEDY_frame_terminator \
    DRX_GL_FUNCTIONS_GL_GREMEDY_string_marker \
    DRX_GL_FUNCTIONS_GL_HP_image_transform \
    DRX_GL_FUNCTIONS_GL_IBM_multimode_draw_arrays \
    DRX_GL_FUNCTIONS_GL_IBM_static_data \
    DRX_GL_FUNCTIONS_GL_IBM_vertex_array_lists \
    DRX_GL_FUNCTIONS_GL_INGR_blend_func_separate \
    DRX_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    DRX_GL_FUNCTIONS_GL_INTEL_map_texture \
    DRX_GL_FUNCTIONS_GL_INTEL_parallel_arrays \
    DRX_GL_FUNCTIONS_GL_INTEL_performance_query \
    DRX_GL_FUNCTIONS_GL_KHR_blend_equation_advanced \
    DRX_GL_FUNCTIONS_GL_KHR_debug \
    DRX_GL_FUNCTIONS_GL_KHR_robustness \
    DRX_GL_FUNCTIONS_GL_KHR_parallel_shader_compile \
    DRX_GL_FUNCTIONS_GL_MESA_framebuffer_flip_y \
    DRX_GL_FUNCTIONS_GL_MESA_resize_buffers \
    DRX_GL_FUNCTIONS_GL_MESA_window_pos \
    DRX_GL_FUNCTIONS_GL_NVX_conditional_render \
    DRX_GL_FUNCTIONS_GL_NVX_linked_gpu_multicast \
    DRX_GL_FUNCTIONS_GL_NV_alpha_to_coverage_dither_control \
    DRX_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect \
    DRX_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect_count \
    DRX_GL_FUNCTIONS_GL_NV_bindless_texture \
    DRX_GL_FUNCTIONS_GL_NV_blend_equation_advanced \
    DRX_GL_FUNCTIONS_GL_NV_clip_space_w_scaling \
    DRX_GL_FUNCTIONS_GL_NV_command_list \
    DRX_GL_FUNCTIONS_GL_NV_conditional_render \
    DRX_GL_FUNCTIONS_GL_NV_conservative_raster \
    DRX_GL_FUNCTIONS_GL_NV_conservative_raster_dilate \
    DRX_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    DRX_GL_FUNCTIONS_GL_NV_copy_image \
    DRX_GL_FUNCTIONS_GL_NV_depth_buffer_float \
    DRX_GL_FUNCTIONS_GL_NV_draw_texture \
    DRX_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    DRX_GL_FUNCTIONS_GL_NV_evaluators \
    DRX_GL_FUNCTIONS_GL_NV_explicit_multisample \
    DRX_GL_FUNCTIONS_GL_NV_fence \
    DRX_GL_FUNCTIONS_GL_NV_fragment_coverage_to_color \
    DRX_GL_FUNCTIONS_GL_NV_fragment_program \
    DRX_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    DRX_GL_FUNCTIONS_GL_NV_framebuffer_multisample_coverage \
    DRX_GL_FUNCTIONS_GL_NV_geometry_program4 \
    DRX_GL_FUNCTIONS_GL_NV_gpu_program4 \
    DRX_GL_FUNCTIONS_GL_NV_gpu_program5 \
    DRX_GL_FUNCTIONS_GL_NV_half_float \
    DRX_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    DRX_GL_FUNCTIONS_GL_NV_gpu_multicast \
    DRX_GL_FUNCTIONS_GL_NVX_gpu_multicast2 \
    DRX_GL_FUNCTIONS_GL_NVX_progress_fence \
    DRX_GL_FUNCTIONS_GL_NV_memory_attachment \
    DRX_GL_FUNCTIONS_GL_NV_memory_object_sparse \
    DRX_GL_FUNCTIONS_GL_NV_mesh_shader \
    DRX_GL_FUNCTIONS_GL_NV_occlusion_query \
    DRX_GL_FUNCTIONS_GL_NV_parameter_buffer_object \
    DRX_GL_FUNCTIONS_GL_NV_path_rendering \
    DRX_GL_FUNCTIONS_GL_NV_pixel_data_range \
    DRX_GL_FUNCTIONS_GL_NV_point_sprite \
    DRX_GL_FUNCTIONS_GL_NV_present_video \
    DRX_GL_FUNCTIONS_GL_NV_primitive_restart \
    DRX_GL_FUNCTIONS_GL_NV_query_resource \
    DRX_GL_FUNCTIONS_GL_NV_query_resource_tag \
    DRX_GL_FUNCTIONS_GL_NV_register_combiners \
    DRX_GL_FUNCTIONS_GL_NV_register_combiners2 \
    DRX_GL_FUNCTIONS_GL_NV_sample_locations \
    DRX_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    DRX_GL_FUNCTIONS_GL_NV_shader_buffer_load \
    DRX_GL_FUNCTIONS_GL_NV_shading_rate_image \
    DRX_GL_FUNCTIONS_GL_NV_texture_barrier \
    DRX_GL_FUNCTIONS_GL_NV_texture_multisample \
    DRX_GL_FUNCTIONS_GL_NV_transform_feedback \
    DRX_GL_FUNCTIONS_GL_NV_transform_feedback2 \
    DRX_GL_FUNCTIONS_GL_NV_vdpau_interop \
    DRX_GL_FUNCTIONS_GL_NV_vdpau_interop2 \
    DRX_GL_FUNCTIONS_GL_NV_vertex_array_range \
    DRX_GL_FUNCTIONS_GL_NV_vertex_attrib_integer_64bit \
    DRX_GL_FUNCTIONS_GL_NV_vertex_buffer_unified_memory \
    DRX_GL_FUNCTIONS_GL_NV_vertex_program \
    DRX_GL_FUNCTIONS_GL_NV_video_capture \
    DRX_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    DRX_GL_FUNCTIONS_GL_OES_byte_coordinates \
    DRX_GL_FUNCTIONS_GL_OES_fixed_point \
    DRX_GL_FUNCTIONS_GL_OES_query_matrix \
    DRX_GL_FUNCTIONS_GL_OES_single_precision \
    DRX_GL_FUNCTIONS_GL_OVR_multiview \
    DRX_GL_FUNCTIONS_GL_PGI_misc_hints \
    DRX_GL_FUNCTIONS_GL_SGIS_detail_texture \
    DRX_GL_FUNCTIONS_GL_SGIS_fog_function \
    DRX_GL_FUNCTIONS_GL_SGIS_multisample \
    DRX_GL_FUNCTIONS_GL_SGIS_pixel_texture \
    DRX_GL_FUNCTIONS_GL_SGIS_point_parameters \
    DRX_GL_FUNCTIONS_GL_SGIS_sharpen_texture \
    DRX_GL_FUNCTIONS_GL_SGIS_texture4D \
    DRX_GL_FUNCTIONS_GL_SGIS_texture_color_mask \
    DRX_GL_FUNCTIONS_GL_SGIS_texture_filter4 \
    DRX_GL_FUNCTIONS_GL_SGIX_async \
    DRX_GL_FUNCTIONS_GL_SGIX_flush_raster \
    DRX_GL_FUNCTIONS_GL_SGIX_fragment_lighting \
    DRX_GL_FUNCTIONS_GL_SGIX_framezoom \
    DRX_GL_FUNCTIONS_GL_SGIX_igloo_interface \
    DRX_GL_FUNCTIONS_GL_SGIX_instruments \
    DRX_GL_FUNCTIONS_GL_SGIX_list_priority \
    DRX_GL_FUNCTIONS_GL_SGIX_pixel_texture \
    DRX_GL_FUNCTIONS_GL_SGIX_polynomial_ffd \
    DRX_GL_FUNCTIONS_GL_SGIX_reference_plane \
    DRX_GL_FUNCTIONS_GL_SGIX_sprite \
    DRX_GL_FUNCTIONS_GL_SGIX_tag_sample_buffer \
    DRX_GL_FUNCTIONS_GL_SGI_color_table \
    DRX_GL_FUNCTIONS_GL_SUNX_constant_data \
    DRX_GL_FUNCTIONS_GL_SUN_global_alpha \
    DRX_GL_FUNCTIONS_GL_SUN_mesh_array \
    DRX_GL_FUNCTIONS_GL_SUN_triangle_list \
    DRX_GL_FUNCTIONS_GL_SUN_vertex

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
