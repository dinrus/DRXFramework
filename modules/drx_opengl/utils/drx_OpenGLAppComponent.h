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
    A base class for writing simple one-page graphical apps.

    A subclass can inherit from this and implement just a few methods such as
    paint() and mouse-handling. The base class provides some simple abstractions
    to take care of continuously repainting itself.

    @tags{OpenGL}
*/
class DRX_API  OpenGLAppComponent   : public Component,
                                       private OpenGLRenderer
{
public:
    OpenGLAppComponent();

    /** Destructor. */
    ~OpenGLAppComponent() override;

    /** Returns the number of times that the render method has been called since
        the component started running.
    */
    i32 getFrameCounter() const noexcept        { return frameCounter; }

    /** This must be called from your subclass's destructor, to shut down
        the GL system and stop it calling render() before your class is destroyed.
    */
    z0 shutdownOpenGL();

    /** Implement this method to set up any GL objects that you need for rendering.
        The GL context will be active when this method is called.

        Note that because the GL context could be destroyed and re-created ad-hoc by
        the underlying platform, the shutdown() and initialise() calls could be called
        multiple times while your app is running. So don't make your code assume that
        this will only be called once!
    */
    virtual z0 initialise() = 0;

    /** Implement this method to free any GL objects that you created during rendering.
        The GL context will still be active when this method is called.

        Note that because the GL context could be destroyed and re-created ad-hoc by
        the underlying platform, the shutdown() and initialise() calls could be called
        multiple times while your app is running. So don't make your code assume that
        this will only be called once!
    */
    virtual z0 shutdown() = 0;

    /** Called to render your openGL.
        @see OpenGLRenderer::render()
    */
    virtual z0 render() = 0;

    /** The GL context */
    OpenGLContext openGLContext;

private:
    //==============================================================================
    i32 frameCounter = 0;

    z0 newOpenGLContextCreated() override;
    z0 renderOpenGL() override;
    z0 openGLContextClosing() override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLAppComponent)
};

} // namespace drx
