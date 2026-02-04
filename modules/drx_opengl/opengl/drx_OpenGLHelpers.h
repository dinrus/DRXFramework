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

//==============================================================================
/**
    A set of miscellaneous openGL helper functions.

    @tags{OpenGL}
*/
class DRX_API  OpenGLHelpers
{
public:
    /** Clears the GL error state. */
    static z0 resetErrorState();

    /** Возвращает true, если the current thread has an active OpenGL context. */
    static b8 isContextActive();

    /** Clears the current context using the given colour. */
    static z0 clear (Color colour);

    static z0 enableScissorTest (Rectangle<i32> clip);

    /** Checks whether the current context supports the specified extension. */
    static b8 isExtensionSupported (tukk extensionName);

    /** Returns the address of a named GL extension function */
    static uk getExtensionFunction (tukk functionName);

    /** Returns a version string such as "#version 150" suitable for prefixing a GLSL
        shader on this platform.
    */
    static Txt getGLSLVersionString();

    /** Makes some simple textual changes to a shader program to try to convert old GLSL
        keywords to their v3 equivalents.

        Before doing this, the function will check whether the current context is actually
        using a later version of the language, and if not it will not make any changes.
        Obviously this is not a real parser, so will only work on simple code!
    */
    static Txt translateVertexShaderToV3 (const Txt&);

    /** Makes some simple textual changes to a shader program to try to convert old GLSL
        keywords to their v3 equivalents.

        Before doing this, the function will check whether the current context is actually
        using a later version of the language, and if not it will not make any changes.
        Obviously this is not a real parser, so will only work on simple code!
    */
    static Txt translateFragmentShaderToV3 (const Txt&);
};

} // namespace drx
