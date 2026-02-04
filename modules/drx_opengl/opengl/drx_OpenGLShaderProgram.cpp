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

OpenGLShaderProgram::OpenGLShaderProgram (const OpenGLContext& c) noexcept  : context (c)
{
}

OpenGLShaderProgram::~OpenGLShaderProgram() noexcept
{
    release();
}

GLuint OpenGLShaderProgram::getProgramID() const noexcept
{
    if (programID == 0)
    {
        // This method should only be called when the current thread has an active OpenGL context.
        jassert (OpenGLHelpers::isContextActive());

        programID = context.extensions.glCreateProgram();
    }

    return programID;
}

z0 OpenGLShaderProgram::release() noexcept
{
    if (programID != 0)
    {
        context.extensions.glDeleteProgram (programID);
        programID = 0;
    }
}

f64 OpenGLShaderProgram::getLanguageVersion()
{
    return Txt::fromUTF8 ((tukk) glGetString (GL_SHADING_LANGUAGE_VERSION))
            .retainCharacters ("1234567890.").getDoubleValue();
}

b8 OpenGLShaderProgram::addShader (const Txt& code, GLenum type)
{
    GLuint shaderID = context.extensions.glCreateShader (type);

    const GLchar* c = code.toRawUTF8();
    context.extensions.glShaderSource (shaderID, 1, &c, nullptr);

    context.extensions.glCompileShader (shaderID);

    GLint status = GL_FALSE;
    context.extensions.glGetShaderiv (shaderID, GL_COMPILE_STATUS, &status);

    if (status == (GLint) GL_FALSE)
    {
        std::vector<GLchar> infoLog (16384);
        GLsizei infoLogLength = 0;
        context.extensions.glGetShaderInfoLog (shaderID, (GLsizei) infoLog.size(), &infoLogLength, infoLog.data());
        errorLog = Txt (infoLog.data(), (size_t) infoLogLength);

       #if DRX_DEBUG && ! DRX_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
        // Your GLSL code contained compile errors!
        // Hopefully this compile log should help to explain what went wrong.
        DBG (errorLog);
        jassertfalse;
       #endif

        return false;
    }

    context.extensions.glAttachShader (getProgramID(), shaderID);
    context.extensions.glDeleteShader (shaderID);
    DRX_CHECK_OPENGL_ERROR
    return true;
}

b8 OpenGLShaderProgram::addVertexShader (const Txt& code)    { return addShader (code, GL_VERTEX_SHADER); }
b8 OpenGLShaderProgram::addFragmentShader (const Txt& code)  { return addShader (code, GL_FRAGMENT_SHADER); }

b8 OpenGLShaderProgram::link() noexcept
{
    // This method can only be used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());

    GLuint progID = getProgramID();

    context.extensions.glLinkProgram (progID);

    GLint status = GL_FALSE;
    context.extensions.glGetProgramiv (progID, GL_LINK_STATUS, &status);

    if (status == (GLint) GL_FALSE)
    {
        std::vector<GLchar> infoLog (16384);
        GLsizei infoLogLength = 0;
        context.extensions.glGetProgramInfoLog (progID, (GLsizei) infoLog.size(), &infoLogLength, infoLog.data());
        errorLog = Txt (infoLog.data(), (size_t) infoLogLength);

       #if DRX_DEBUG && ! DRX_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
        // Your GLSL code contained link errors!
        // Hopefully this compile log should help to explain what went wrong.
        DBG (errorLog);
        jassertfalse;
       #endif
    }

    DRX_CHECK_OPENGL_ERROR
    return status != (GLint) GL_FALSE;
}

z0 OpenGLShaderProgram::use() const noexcept
{
    // The shader program must have been successfully linked when this method is called!
    jassert (programID != 0);

    context.extensions.glUseProgram (programID);
}

GLint OpenGLShaderProgram::getUniformIDFromName (tukk uniformName) const noexcept
{
    // The shader program must be active when this method is called!
    jassert (programID != 0);

    return (GLint) context.extensions.glGetUniformLocation (programID, uniformName);
}

z0 OpenGLShaderProgram::setUniform (tukk name, GLfloat n1) noexcept                                       { context.extensions.glUniform1f  (getUniformIDFromName (name), n1); }
z0 OpenGLShaderProgram::setUniform (tukk name, GLint n1) noexcept                                         { context.extensions.glUniform1i  (getUniformIDFromName (name), n1); }
z0 OpenGLShaderProgram::setUniform (tukk name, GLfloat n1, GLfloat n2) noexcept                           { context.extensions.glUniform2f  (getUniformIDFromName (name), n1, n2); }
z0 OpenGLShaderProgram::setUniform (tukk name, GLfloat n1, GLfloat n2, GLfloat n3) noexcept               { context.extensions.glUniform3f  (getUniformIDFromName (name), n1, n2, n3); }
z0 OpenGLShaderProgram::setUniform (tukk name, GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4) noexcept   { context.extensions.glUniform4f  (getUniformIDFromName (name), n1, n2, n3, n4); }
z0 OpenGLShaderProgram::setUniform (tukk name, GLint n1, GLint n2, GLint n3, GLint n4) noexcept           { context.extensions.glUniform4i  (getUniformIDFromName (name), n1, n2, n3, n4); }
z0 OpenGLShaderProgram::setUniform (tukk name, const GLfloat* values, GLsizei numValues) noexcept         { context.extensions.glUniform1fv (getUniformIDFromName (name), numValues, values); }
z0 OpenGLShaderProgram::setUniformMat2 (tukk name, const GLfloat* v, GLint num, GLboolean trns) noexcept  { context.extensions.glUniformMatrix2fv (getUniformIDFromName (name), num, trns, v); }
z0 OpenGLShaderProgram::setUniformMat3 (tukk name, const GLfloat* v, GLint num, GLboolean trns) noexcept  { context.extensions.glUniformMatrix3fv (getUniformIDFromName (name), num, trns, v); }
z0 OpenGLShaderProgram::setUniformMat4 (tukk name, const GLfloat* v, GLint num, GLboolean trns) noexcept  { context.extensions.glUniformMatrix4fv (getUniformIDFromName (name), num, trns, v); }

//==============================================================================
OpenGLShaderProgram::Attribute::Attribute (const OpenGLShaderProgram& program, tukk name)
    : attributeID ((GLuint) program.context.extensions.glGetAttribLocation (program.getProgramID(), name))
{
   #if DRX_DEBUG && ! DRX_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    jassert ((GLint) attributeID >= 0);
   #endif
}

//==============================================================================
OpenGLShaderProgram::Uniform::Uniform (const OpenGLShaderProgram& program, tukk const name)
    : uniformID (program.context.extensions.glGetUniformLocation (program.getProgramID(), name)), context (program.context)
{
   #if DRX_DEBUG && ! DRX_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    jassert (uniformID >= 0);
   #endif
}

z0 OpenGLShaderProgram::Uniform::set (GLfloat n1) const noexcept                                    { context.extensions.glUniform1f (uniformID, n1); }
z0 OpenGLShaderProgram::Uniform::set (GLint n1) const noexcept                                      { context.extensions.glUniform1i (uniformID, n1); }
z0 OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2) const noexcept                        { context.extensions.glUniform2f (uniformID, n1, n2); }
z0 OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3) const noexcept            { context.extensions.glUniform3f (uniformID, n1, n2, n3); }
z0 OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4) const noexcept  { context.extensions.glUniform4f (uniformID, n1, n2, n3, n4); }
z0 OpenGLShaderProgram::Uniform::set (GLint n1, GLint n2, GLint n3, GLint n4) const noexcept        { context.extensions.glUniform4i (uniformID, n1, n2, n3, n4); }
z0 OpenGLShaderProgram::Uniform::set (const GLfloat* values, GLsizei numValues) const noexcept      { context.extensions.glUniform1fv (uniformID, numValues, values); }

z0 OpenGLShaderProgram::Uniform::setMatrix2 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix2fv (uniformID, num, trns, v); }
z0 OpenGLShaderProgram::Uniform::setMatrix3 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix3fv (uniformID, num, trns, v); }
z0 OpenGLShaderProgram::Uniform::setMatrix4 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix4fv (uniformID, num, trns, v); }

} // namespace drx
