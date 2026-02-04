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

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixFormat,
                   uk contextToShare,
                   b8 shouldUseMultisampling,
                   OpenGLVersion version)
        : owner (component)
    {
        const auto attribs = createAttribs (version, pixFormat, shouldUseMultisampling);

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs.data()];

        static MouseForwardingNSOpenGLViewClass cls;
        view = [cls.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                       pixelFormat: format];

        if ([view respondsToSelector: @selector (setWantsBestResolutionOpenGLSurface:)])
            [view setWantsBestResolutionOpenGLSurface: YES];

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter] addObserver: view
                                                 selector: @selector (_surfaceNeedsUpdate:)
                                                     name: NSViewGlobalFrameDidChangeNotification
                                                   object: view];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        renderContext = [[[NSOpenGLContext alloc] initWithFormat: format
                                                    shareContext: (NSOpenGLContext*) contextToShare] autorelease];

        [view setOpenGLContext: renderContext];
        [format release];

        viewAttachment = NSViewComponent::attachViewToComponent (component, view);
    }

    ~NativeContext()
    {
        [[NSNotificationCenter defaultCenter] removeObserver: view];
        [renderContext clearDrawable];
        [renderContext setView: nil];
        [view setOpenGLContext: nil];
        [view release];
    }

    static std::vector<NSOpenGLPixelFormatAttribute> createAttribs (OpenGLVersion version,
                                                                    const OpenGLPixelFormat& pixFormat,
                                                                    b8 shouldUseMultisampling)
    {
        std::vector<NSOpenGLPixelFormatAttribute> attribs
        {
            NSOpenGLPFAOpenGLProfile, [version]
            {
                if (version == openGL3_2)
                    return NSOpenGLProfileVersion3_2Core;

                if (version != defaultGLVersion)
                    return NSOpenGLProfileVersion4_1Core;

                return NSOpenGLProfileVersionLegacy;
            }(),
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAClosestPolicy,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAColorSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.redBits + pixFormat.greenBits + pixFormat.blueBits),
            NSOpenGLPFAAlphaSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.alphaBits),
            NSOpenGLPFADepthSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.depthBufferBits),
            NSOpenGLPFAStencilSize, static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.stencilBufferBits),
            NSOpenGLPFAAccumSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.accumulationBufferRedBits  + pixFormat.accumulationBufferGreenBits
                                                                             + pixFormat.accumulationBufferBlueBits + pixFormat.accumulationBufferAlphaBits)
        };

        if (shouldUseMultisampling)
        {
            attribs.insert (attribs.cend(),
            {
                NSOpenGLPFAMultisample,
                NSOpenGLPFASampleBuffers,   static_cast<NSOpenGLPixelFormatAttribute> (1),
                NSOpenGLPFASamples,         static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.multisamplingLevel)
            });
        }

        attribs.push_back (0);

        return attribs;
    }

    InitResult initialiseOnRenderThread (OpenGLContext&)  { return InitResult::success; }
    z0 shutdownOnRenderThread()                         { deactivateCurrentContext(); }

    b8 createdOk() const noexcept                   { return getRawContext() != nullptr; }
    NSOpenGLView* getNSView() const noexcept          { return view; }
    NSOpenGLContext* getRawContext() const noexcept   { return renderContext; }
    GLuint getFrameBufferID() const noexcept          { return 0; }

    b8 makeActive() const noexcept
    {
        jassert (renderContext != nil);

        if ([renderContext view] != view)
            [renderContext setView: view];

        if (NSOpenGLContext* context = [view openGLContext])
        {
            [context makeCurrentContext];
            return true;
        }

        return false;
    }

    b8 isActive() const noexcept
    {
        return [NSOpenGLContext currentContext] == renderContext;
    }

    static z0 deactivateCurrentContext()
    {
        [NSOpenGLContext clearCurrentContext];
    }

    struct Locker
    {
        Locker (NativeContext& nc) : cglContext ((CGLContextObj) [nc.renderContext CGLContextObj])
        {
            CGLLockContext (cglContext);
        }

        ~Locker()
        {
            CGLUnlockContext (cglContext);
        }

    private:
        CGLContextObj cglContext;
    };

    z0 swapBuffers()
    {
        auto now = Time::getMillisecondCounterHiRes();
        [renderContext flushBuffer];

        if (const auto minSwapTime = minSwapTimeMs.get(); minSwapTime > 0)
        {
            // When our window is entirely occluded by other windows, flushBuffer
            // fails to wait for the swap interval, so the render loop spins at full
            // speed, burning CPU. This hack detects when things are going too fast
            // and sleeps if necessary.

            auto swapTime = Time::getMillisecondCounterHiRes() - now;
            auto frameTime = (i32) std::min ((zu64) std::numeric_limits<i32>::max(),
                                             (zu64) now - (zu64) lastSwapTime);

            if (swapTime < 0.5 && frameTime < minSwapTime - 3)
            {
                if (underrunCounter > 3)
                {
                    Thread::sleep (2 * (minSwapTime - frameTime));
                    now = Time::getMillisecondCounterHiRes();
                }
                else
                {
                    ++underrunCounter;
                }
            }
            else
            {
                if (underrunCounter > 0)
                    --underrunCounter;
            }
        }

        lastSwapTime = now;
    }

    z0 updateWindowPosition (Rectangle<i32>)
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            const auto newArea = peer->getAreaCoveredBy (owner);

            if (convertToRectInt ([view frame]) != newArea)
                [view setFrame: makeCGRect (newArea)];
        }
    }

    b8 setSwapInterval (i32 numFramesPerSwapIn)
    {
        // The macOS OpenGL programming guide says that numFramesPerSwap
        // can only be 0 or 1.
        jassert (isPositiveAndBelow (numFramesPerSwapIn, 2));

        [renderContext setValues: (const GLint*) &numFramesPerSwapIn
                    forParameter: getSwapIntervalParameter()];

        minSwapTimeMs.setFramesPerSwap (numFramesPerSwapIn);

        return true;
    }

    i32 getSwapInterval() const
    {
        GLint numFrames = 0;
        [renderContext getValues: &numFrames
                    forParameter: getSwapIntervalParameter()];

        return numFrames;
    }

    z0 setNominalVideoRefreshPeriodS (f64 periodS)
    {
        jassert (periodS > 0.0);
        minSwapTimeMs.setVideoRefreshPeriodS (periodS);
    }

    static NSOpenGLContextParameter getSwapIntervalParameter()
    {
        if (@available (macOS 10.12, *))
            return NSOpenGLContextParameterSwapInterval;

        return NSOpenGLCPSwapInterval;
    }

    class MinSwapTimeMs
    {
    public:
        i32 get() const
        {
            return minSwapTimeMs;
        }

        z0 setFramesPerSwap (i32 n)
        {
            const std::scoped_lock lock { mutex };
            numFramesPerSwap = n;
            updateMinSwapTime();
        }

        z0 setVideoRefreshPeriodS (f64 n)
        {
            const std::scoped_lock lock { mutex };
            videoRefreshPeriodS = n;
            updateMinSwapTime();
        }

    private:
        z0 updateMinSwapTime()
        {
            minSwapTimeMs = static_cast<i32> (numFramesPerSwap * 1000 * videoRefreshPeriodS);
        }

        std::mutex mutex;
        std::atomic<i32> minSwapTimeMs { 0 };
        i32 numFramesPerSwap = 0;
        f64 videoRefreshPeriodS = 1.0 / 60.0;
    };

    Component& owner;
    NSOpenGLContext* renderContext = nil;
    NSOpenGLView* view = nil;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;
    f64 lastSwapTime = 0;
    i32 underrunCounter = 0;
    MinSwapTimeMs minSwapTimeMs;

    //==============================================================================
    struct MouseForwardingNSOpenGLViewClass  : public ObjCClass<NSOpenGLView>
    {
        MouseForwardingNSOpenGLViewClass()  : ObjCClass ("DRXGLView_")
        {
            addMethod (@selector (rightMouseDown:), [] (id self, SEL, NSEvent* ev)
            {
                [[(NSOpenGLView*) self superview] rightMouseDown: ev];
            });

            addMethod (@selector (rightMouseUp:), [] (id self, SEL, NSEvent* ev)
            {
                [[(NSOpenGLView*) self superview] rightMouseUp:   ev];
            });

            addMethod (@selector (acceptsFirstMouse:), [] (id, SEL, NSEvent*) -> BOOL
            {
                return YES;
            });

            addMethod (@selector (accessibilityHitTest:), [] (id self, SEL, NSPoint p) -> id
            {
                return [[(NSOpenGLView*) self superview] accessibilityHitTest: p];
            });

            addMethod (@selector (hitTest:), [] (id, SEL, NSPoint) -> NSView*
            {
                return nil;
            });

            registerClass();
        }
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
b8 OpenGLHelpers::isContextActive()
{
    return CGLGetCurrentContext() != CGLContextObj();
}

DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx
