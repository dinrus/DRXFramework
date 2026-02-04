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

/** Creates a graphics context object that will render into the given OpenGL target. */
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext&, i32 width, i32 height);

/** Creates a graphics context object that will render into the given OpenGL framebuffer. */
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext&, OpenGLFrameBuffer&);

/** Creates a graphics context object that will render into the given OpenGL framebuffer,
    with the given size.
*/
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext&,
                                                                      u32 frameBufferID,
                                                                      i32 width, i32 height);


//==============================================================================
/**
    Used to create custom shaders for use with an openGL 2D rendering context.

    Given a GL-based rendering context, you can write a fragment shader that applies some
    kind of per-pixel effect.

    @tags{OpenGL}
*/
struct DRX_API  OpenGLGraphicsContextCustomShader
{
    /** Creates a custom shader.

        The shader code will not be compiled until actually needed, so it's OK to call this
        constructor when no GL context is active.

        The code should be a normal fragment shader. As well as the usual GLSL variables, there is
        also an automatically declared varying vec2 called "pixelPos", which indicates the pixel
        position within the graphics context of the pixel being drawn. There is also a varying value
        "pixelAlpha", which indicates the alpha by which the pixel should be multiplied, so that the
        edges of any clip-region masks are anti-aliased correctly.
    */
    OpenGLGraphicsContextCustomShader (const Txt& fragmentShaderCode);

    /** Destructor. */
    ~OpenGLGraphicsContextCustomShader();

    /** Returns the program, if it has been linked and is active.
        This can be called when you're about to use fillRect, to set up any uniforms/textures that
        the program may require.
    */
    OpenGLShaderProgram* getProgram (LowLevelGraphicsContext&) const;

    /** Applies the shader to a rectangle within the graphics context. */
    z0 fillRect (LowLevelGraphicsContext&, Rectangle<i32> area) const;

    /** Attempts to compile the program if necessary, and returns an error message if it fails. */
    Result checkCompilation (LowLevelGraphicsContext&);

    /** Returns the code that was used to create this object. */
    const Txt& getFragmentShaderCode() const noexcept           { return code; }

    /** Optional lambda that will be called when the shader is activated, to allow
        user code to do setup tasks.
    */
    std::function<z0 (OpenGLShaderProgram&)> onShaderActivated;

private:
    Txt code, hashName;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLGraphicsContextCustomShader)
};

} // namespace drx
