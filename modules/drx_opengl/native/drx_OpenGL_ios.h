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

@interface DrxGLView   : UIView
{
}
+ (Class) layerClass;
@end

@implementation DrxGLView
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}
@end

extern "C" GLvoid glResolveMultisampleFramebufferAPPLE();

namespace drx
{

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& c,
                   const OpenGLPixelFormat& pixFormat,
                   uk contextToShare,
                   b8 multisampling,
                   OpenGLVersion version)
        : component (c), openGLversion (version),
          useDepthBuffer (pixFormat.depthBufferBits > 0),
          useMSAA (multisampling)
    {
        DRX_AUTORELEASEPOOL
        {
            if (auto* peer = component.getPeer())
            {
                auto bounds = peer->getAreaCoveredBy (component);

                view = [[DrxGLView alloc] initWithFrame: convertToCGRect (bounds)];
                view.opaque = YES;
                view.hidden = NO;
                view.backgroundColor = [UIColor blackColor];
                view.userInteractionEnabled = NO;

                glLayer = (CAEAGLLayer*) [view layer];
                glLayer.opaque = true;

                updateWindowPosition (bounds);

                [((UIView*) peer->getNativeHandle()) addSubview: view];

                const auto shouldUseES3 = version != defaultGLVersion
                                       && [[UIDevice currentDevice].systemVersion floatValue] >= 7.0;

                [[maybe_unused]] const auto gotContext = (shouldUseES3 && createContext (kEAGLRenderingAPIOpenGLES3, contextToShare))
                                                         || createContext (kEAGLRenderingAPIOpenGLES2, contextToShare);

                jassert (gotContext);

                if (context != nil)
                {
                    // I'd prefer to put this stuff in the initialiseOnRenderThread() call, but doing
                    // so causes mysterious timing-related failures.
                    [EAGLContext setCurrentContext: context.get()];
                    gl::loadFunctions();
                    createGLBuffers();
                    deactivateCurrentContext();
                }
                else
                {
                    jassertfalse;
                }
            }
            else
            {
                jassertfalse;
            }
        }
    }

    ~NativeContext()
    {
        context.reset();
        [view removeFromSuperview];
        [view release];
    }

    InitResult initialiseOnRenderThread (OpenGLContext&)    { return InitResult::success; }

    z0 shutdownOnRenderThread()
    {
        DRX_CHECK_OPENGL_ERROR
        freeGLBuffers();
        deactivateCurrentContext();
    }

    b8 createdOk() const noexcept             { return getRawContext() != nullptr; }
    uk getRawContext() const noexcept        { return context.get(); }
    GLuint getFrameBufferID() const noexcept    { return useMSAA ? msaaBufferHandle : frameBufferHandle; }

    b8 makeActive() const noexcept
    {
        if (! [EAGLContext setCurrentContext: context.get()])
            return false;

        glBindFramebuffer (GL_FRAMEBUFFER, useMSAA ? msaaBufferHandle
                                                   : frameBufferHandle);
        return true;
    }

    b8 isActive() const noexcept
    {
        return [EAGLContext currentContext] == context.get();
    }

    static z0 deactivateCurrentContext()
    {
        [EAGLContext setCurrentContext: nil];
    }

    z0 swapBuffers()
    {
        if (useMSAA)
        {
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, frameBufferHandle);
            glBindFramebuffer (GL_READ_FRAMEBUFFER, msaaBufferHandle);

            if (openGLversion >= openGL3_2)
            {
                auto w = roundToInt (lastBounds.getWidth()  * glLayer.contentsScale);
                auto h = roundToInt (lastBounds.getHeight() * glLayer.contentsScale);

                glBlitFramebuffer (0, 0, w, h,
                                   0, 0, w, h,
                                   GL_COLOR_BUFFER_BIT,
                                   GL_NEAREST);
            }
            else
            {
                ::glResolveMultisampleFramebufferAPPLE();
            }
        }

        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);
        [context.get() presentRenderbuffer: GL_RENDERBUFFER];

        if (needToRebuildBuffers)
        {
            needToRebuildBuffers = false;

            freeGLBuffers();
            createGLBuffers();
            makeActive();
        }
    }

    z0 updateWindowPosition (Rectangle<i32> bounds)
    {
        view.frame = convertToCGRect (bounds);
        glLayer.contentsScale = (CGFloat) (Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale
                                            / component.getDesktopScaleFactor());

        if (lastBounds != bounds)
        {
            lastBounds = bounds;
            needToRebuildBuffers = true;
        }
    }

    b8 setSwapInterval (i32 numFramesPerSwap) noexcept
    {
        swapFrames = numFramesPerSwap;
        return false;
    }

    i32 getSwapInterval() const noexcept    { return swapFrames; }

    struct Locker
    {
        explicit Locker (NativeContext& ctx) : lock (ctx.mutex) {}
        const ScopedLock lock;
    };

private:
    CriticalSection mutex;
    Component& component;
    DrxGLView* view = nil;
    CAEAGLLayer* glLayer = nil;
    NSUniquePtr<EAGLContext> context;
    const OpenGLVersion openGLversion;
    const b8 useDepthBuffer, useMSAA;

    GLuint frameBufferHandle = 0, colorBufferHandle = 0, depthBufferHandle = 0,
           msaaColorHandle = 0, msaaBufferHandle = 0;

    Rectangle<i32> lastBounds;
    i32 swapFrames = 0;
    b8 needToRebuildBuffers = false;

    b8 createContext (EAGLRenderingAPI type, uk contextToShare)
    {
        jassert (context == nil);
        context.reset ([EAGLContext alloc]);

        if (contextToShare != nullptr)
            [context.get() initWithAPI: type  sharegroup: [(EAGLContext*) contextToShare sharegroup]];
        else
            [context.get() initWithAPI: type];

        return context != nil;
    }

    //==============================================================================
    z0 createGLBuffers()
    {
        glGenFramebuffers (1, &frameBufferHandle);
        glGenRenderbuffers (1, &colorBufferHandle);

        glBindFramebuffer (GL_FRAMEBUFFER, frameBufferHandle);
        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);

        glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferHandle);

        [[maybe_unused]] b8 ok = [context.get() renderbufferStorage: GL_RENDERBUFFER fromDrawable: glLayer];
        jassert (ok);

        GLint width, height;
        glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

        if (useMSAA)
        {
            glGenFramebuffers (1, &msaaBufferHandle);
            glGenRenderbuffers (1, &msaaColorHandle);

            glBindFramebuffer (GL_FRAMEBUFFER, msaaBufferHandle);
            glBindRenderbuffer (GL_RENDERBUFFER, msaaColorHandle);

            glRenderbufferStorageMultisample (GL_RENDERBUFFER, 4, GL_RGBA8, width, height);

            glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorHandle);
        }

        if (useDepthBuffer)
        {
            glGenRenderbuffers (1, &depthBufferHandle);
            glBindRenderbuffer (GL_RENDERBUFFER, depthBufferHandle);

            if (useMSAA)
                glRenderbufferStorageMultisample (GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, width, height);
            else
                glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

            glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferHandle);
        }

        jassert (glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        DRX_CHECK_OPENGL_ERROR
    }

    z0 freeGLBuffers()
    {
        DRX_CHECK_OPENGL_ERROR
        [context.get() renderbufferStorage: GL_RENDERBUFFER fromDrawable: nil];

        deleteFrameBuffer (frameBufferHandle);
        deleteFrameBuffer (msaaBufferHandle);
        deleteRenderBuffer (colorBufferHandle);
        deleteRenderBuffer (depthBufferHandle);
        deleteRenderBuffer (msaaColorHandle);

        DRX_CHECK_OPENGL_ERROR
    }

    static z0 deleteFrameBuffer  (GLuint& i)   { if (i != 0) glDeleteFramebuffers  (1, &i); i = 0; }
    static z0 deleteRenderBuffer (GLuint& i)   { if (i != 0) glDeleteRenderbuffers (1, &i); i = 0; }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
b8 OpenGLHelpers::isContextActive()
{
    return [EAGLContext currentContext] != nil;
}

} // namespace drx
