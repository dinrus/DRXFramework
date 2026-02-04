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

class Version
{
public:
    constexpr Version() = default;

    constexpr explicit Version (i32 majorIn)
        : Version (majorIn, 0) {}

    constexpr Version (i32 majorIn, i32 minorIn)
        : major (majorIn), minor (minorIn) {}

    i32 major = 0, minor = 0;

    constexpr b8 operator== (const Version& other) const noexcept
    {
        return toTuple() == other.toTuple();
    }

    constexpr b8 operator!= (const Version& other) const noexcept
    {
        return toTuple() != other.toTuple();
    }

    constexpr b8 operator< (const Version& other) const noexcept
    {
        return toTuple() < other.toTuple();
    }

    constexpr b8 operator<= (const Version& other) const noexcept
    {
        return toTuple() <= other.toTuple();
    }

    constexpr b8 operator> (const Version& other) const noexcept
    {
        return toTuple() > other.toTuple();
    }

    constexpr b8 operator>= (const Version& other) const noexcept
    {
        return toTuple() >= other.toTuple();
    }

private:
    constexpr std::tuple<i32, i32> toTuple() const noexcept
    {
        return std::make_tuple (major, minor);
    }
};


template <typename Char>
static auto* findNullTerminator (const Char* ptr)
{
    while (*ptr != 0)
        ++ptr;

    return ptr;
}

static Version getOpenGLVersion()
{
    const auto* versionBegin = glGetString (GL_VERSION);

    if (versionBegin == nullptr)
        return {};

    const auto* versionEnd = findNullTerminator (versionBegin);
    const std::string versionString (versionBegin, versionEnd);
    const auto spaceSeparated = StringArray::fromTokens (versionString.c_str(), false);

    for (const auto& token : spaceSeparated)
    {
        const auto pointSeparated = StringArray::fromTokens (token, ".", "");

        const auto major = pointSeparated[0].getIntValue();
        const auto minor = pointSeparated[1].getIntValue();

        if (major != 0)
            return { major, minor };
    }

    return {};
}

z0 OpenGLHelpers::resetErrorState()
{
    while (glGetError() != GL_NO_ERROR) {}
}

uk OpenGLHelpers::getExtensionFunction (tukk functionName)
{
   #if DRX_WINDOWS
    return (uk) wglGetProcAddress (functionName);
   #elif DRX_LINUX || DRX_BSD
    return (uk) glXGetProcAddress ((const GLubyte*) functionName);
   #else
    static uk handle = dlopen (nullptr, RTLD_LAZY);
    return dlsym (handle, functionName);
   #endif
}

b8 OpenGLHelpers::isExtensionSupported (tukk const extensionName)
{
    jassert (extensionName != nullptr); // you must supply a genuine string for this.
    jassert (isContextActive()); // An OpenGL context will need to be active before calling this.

    if (getOpenGLVersion().major >= 3)
    {
        using GetStringi = const GLubyte* (*) (GLenum, GLuint);

        if (auto* thisGlGetStringi = reinterpret_cast<GetStringi> (getExtensionFunction ("glGetStringi")))
        {
            GLint n = 0;
            glGetIntegerv (GL_NUM_EXTENSIONS, &n);

            for (auto i = (decltype (n)) 0; i < n; ++i)
                if (StringRef (extensionName) == StringRef ((tukk) thisGlGetStringi (GL_EXTENSIONS, (GLuint) i)))
                    return true;

            return false;
        }
    }

    tukk extensions = (tukk) glGetString (GL_EXTENSIONS);
    jassert (extensions != nullptr); // Perhaps you didn't activate an OpenGL context before calling this?

    for (;;)
    {
        tukk found = strstr (extensions, extensionName);

        if (found == nullptr)
            break;

        extensions = found + strlen (extensionName);

        if (extensions[0] == ' ' || extensions[0] == 0)
            return true;
    }

    return false;
}

z0 OpenGLHelpers::clear (Color colour)
{
    glClearColor (colour.getFloatRed(), colour.getFloatGreen(),
                  colour.getFloatBlue(), colour.getFloatAlpha());

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

z0 OpenGLHelpers::enableScissorTest (Rectangle<i32> clip)
{
    glEnable (GL_SCISSOR_TEST);
    glScissor (clip.getX(), clip.getY(), clip.getWidth(), clip.getHeight());
}

Txt OpenGLHelpers::getGLSLVersionString()
{
    if (getOpenGLVersion() >= Version (3, 2))
    {
       #if DRX_OPENGL_ES
        return "#version 300 es";
       #else
        return "#version 150";
       #endif
    }

    return "#version 110";
}

Txt OpenGLHelpers::translateVertexShaderToV3 (const Txt& code)
{
    if (getOpenGLVersion() >= Version (3, 2))
    {
        Txt output;

       #if DRX_ANDROID
        {
            i32 numAttributes = 0;

            for (i32 p = code.indexOf (0, "attribute "); p >= 0; p = code.indexOf (p + 1, "attribute "))
                numAttributes++;

            i32 last = 0;

            for (i32 p = code.indexOf (0, "attribute "); p >= 0; p = code.indexOf (p + 1, "attribute "))
            {
                output += code.substring (last, p) + "layout(location=" + Txt (--numAttributes) + ") in ";

                last = p + 10;
            }

            output += code.substring (last);
        }
       #else
        output = code.replace ("attribute", "in");
       #endif

        return getGLSLVersionString() + "\n" + output.replace ("varying", "out");
    }

    return code;
}

Txt OpenGLHelpers::translateFragmentShaderToV3 (const Txt& code)
{
    if (getOpenGLVersion() >= Version (3, 2))
        return getGLSLVersionString() + "\n"
               "out " DRX_MEDIUMP " vec4 fragColor;\n"
                + code.replace ("varying", "in")
                      .replace ("texture2D", "texture")
                      .replace ("gl_FragColor", "fragColor");

    return code;
}

} // namespace drx
