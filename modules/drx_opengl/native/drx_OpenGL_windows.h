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

extern ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component&, Component* parent);

//==============================================================================
class OpenGLContext::NativeContext  : private ComponentPeer::ScaleFactorListener,
                                      private AsyncUpdater
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixelFormat,
                   uk contextToShareWithIn,
                   b8 /*useMultisampling*/,
                   OpenGLVersion version)
        : sharedContext (contextToShareWithIn)
    {
        placeholderComponent.reset (new PlaceholderComponent (*this));
        createNativeWindow (component);

        PIXELFORMATDESCRIPTOR pfd;
        initialisePixelFormatDescriptor (pfd, pixelFormat);

        auto pixFormat = ChoosePixelFormat (dc.get(), &pfd);

        if (pixFormat != 0)
            SetPixelFormat (dc.get(), pixFormat, &pfd);

        initialiseWGLExtensions (dc.get());
        renderContext.reset (createRenderContext (version, dc.get()));

        if (renderContext != nullptr)
        {
            makeActive();

            auto wglFormat = wglChoosePixelFormatExtension (pixelFormat);
            deactivateCurrentContext();

            if (wglFormat != pixFormat && wglFormat != 0)
            {
                // can't change the pixel format of a window, so need to delete the
                // old one and create a new one.
                dc.reset();
                nativeWindow = nullptr;
                createNativeWindow (component);

                if (SetPixelFormat (dc.get(), wglFormat, &pfd))
                {
                    renderContext.reset();
                    renderContext.reset (createRenderContext (version, dc.get()));
                }
            }

            component.getTopLevelComponent()->repaint();
            component.repaint();
        }
    }

    ~NativeContext() override
    {
        cancelPendingUpdate();
        renderContext.reset();
        dc.reset();

        if (safeComponent != nullptr)
            if (auto* peer = safeComponent->getTopLevelComponent()->getPeer())
                peer->removeScaleFactorListener (this);
    }

    InitResult initialiseOnRenderThread (OpenGLContext& c)
    {
        threadAwarenessSetter = std::make_unique<ScopedThreadDPIAwarenessSetter> (nativeWindow->getNativeHandle());
        context = &c;

        if (sharedContext != nullptr)
        {
            if (! wglShareLists ((HGLRC) sharedContext, renderContext.get()))
            {
                TCHAR messageBuffer[256] = {};

                FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               nullptr,
                               GetLastError(),
                               MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                               messageBuffer,
                               (DWORD) numElementsInArray (messageBuffer) - 1,
                               nullptr);

                DBG (messageBuffer);
                jassertfalse;
            }
        }

        return InitResult::success;
    }

    z0 shutdownOnRenderThread()
    {
        deactivateCurrentContext();
        context = nullptr;
        threadAwarenessSetter = nullptr;
    }

    static z0 deactivateCurrentContext()  { wglMakeCurrent (nullptr, nullptr); }
    b8 makeActive() const noexcept        { return isActive() || wglMakeCurrent (dc.get(), renderContext.get()) != FALSE; }
    b8 isActive() const noexcept          { return wglGetCurrentContext() == renderContext.get(); }

    z0 swapBuffers() noexcept
    {
        SwapBuffers (dc.get());

        if (! std::exchange (haveBuffersBeenSwapped, true))
            triggerAsyncUpdate();
    }

    b8 setSwapInterval (i32 numFramesPerSwap)
    {
        jassert (isActive()); // this can only be called when the context is active..
        return wglSwapIntervalEXT != nullptr && wglSwapIntervalEXT (numFramesPerSwap) != FALSE;
    }

    i32 getSwapInterval() const
    {
        jassert (isActive()); // this can only be called when the context is active..
        return wglGetSwapIntervalEXT != nullptr ? wglGetSwapIntervalEXT() : 0;
    }

    z0 updateWindowPosition (Rectangle<i32> bounds)
    {
        if (nativeWindow != nullptr)
        {
            if (! approximatelyEqual (nativeScaleFactor, 1.0))
                bounds = (bounds.toDouble() * nativeScaleFactor).toNearestInt();

            SetWindowPos ((HWND) nativeWindow->getNativeHandle(), nullptr,
                          bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                          SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }

    b8 createdOk() const noexcept                 { return getRawContext() != nullptr; }
    uk getRawContext() const noexcept            { return renderContext.get(); }
    u32 getFrameBufferID() const noexcept  { return 0; }

    z0 triggerRepaint()
    {
        if (context != nullptr)
            context->triggerRepaint();
    }

    struct Locker
    {
        explicit Locker (NativeContext& ctx) : lock (ctx.mutex) {}
        const ScopedLock lock;
    };

    HWND getNativeHandle()
    {
        if (nativeWindow != nullptr)
            return (HWND) nativeWindow->getNativeHandle();

        return nullptr;
    }

private:
    //==============================================================================
    z0 handleAsyncUpdate() override
    {
        nativeWindow->setVisible (true);
    }

    static z0 initialiseWGLExtensions (HDC dcIn)
    {
        static b8 initialised = false;

        if (initialised)
            return;

        initialised = true;

        const auto dummyContext = wglCreateContext (dcIn);
        wglMakeCurrent (dcIn, dummyContext);

        #define DRX_INIT_WGL_FUNCTION(name)    name = (type_ ## name) OpenGLHelpers::getExtensionFunction (#name);
        DRX_INIT_WGL_FUNCTION (wglChoosePixelFormatARB)
        DRX_INIT_WGL_FUNCTION (wglSwapIntervalEXT)
        DRX_INIT_WGL_FUNCTION (wglGetSwapIntervalEXT)
        DRX_INIT_WGL_FUNCTION (wglCreateContextAttribsARB)
        #undef DRX_INIT_WGL_FUNCTION

        wglMakeCurrent (nullptr, nullptr);
        wglDeleteContext (dummyContext);
    }

    static z0 initialisePixelFormatDescriptor (PIXELFORMATDESCRIPTOR& pfd, const OpenGLPixelFormat& pixelFormat)
    {
        zerostruct (pfd);
        pfd.nSize           = sizeof (pfd);
        pfd.nVersion        = 1;
        pfd.dwFlags         = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType      = PFD_TYPE_RGBA;
        pfd.iLayerType      = PFD_MAIN_PLANE;
        pfd.cColorBits      = (BYTE) (pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits);
        pfd.cRedBits        = (BYTE) pixelFormat.redBits;
        pfd.cGreenBits      = (BYTE) pixelFormat.greenBits;
        pfd.cBlueBits       = (BYTE) pixelFormat.blueBits;
        pfd.cAlphaBits      = (BYTE) pixelFormat.alphaBits;
        pfd.cDepthBits      = (BYTE) pixelFormat.depthBufferBits;
        pfd.cStencilBits    = (BYTE) pixelFormat.stencilBufferBits;
        pfd.cAccumBits      = (BYTE) (pixelFormat.accumulationBufferRedBits + pixelFormat.accumulationBufferGreenBits
                                        + pixelFormat.accumulationBufferBlueBits + pixelFormat.accumulationBufferAlphaBits);
        pfd.cAccumRedBits   = (BYTE) pixelFormat.accumulationBufferRedBits;
        pfd.cAccumGreenBits = (BYTE) pixelFormat.accumulationBufferGreenBits;
        pfd.cAccumBlueBits  = (BYTE) pixelFormat.accumulationBufferBlueBits;
        pfd.cAccumAlphaBits = (BYTE) pixelFormat.accumulationBufferAlphaBits;
    }

    static HGLRC createRenderContext (OpenGLVersion version, HDC dcIn)
    {
        const auto components = [&]() -> Optional<Version>
        {
            switch (version)
            {
                case OpenGLVersion::openGL3_2: return Version { 3, 2 };
                case OpenGLVersion::openGL4_1: return Version { 4, 1 };
                case OpenGLVersion::openGL4_3: return Version { 4, 3 };

                case OpenGLVersion::defaultGLVersion: break;
            }

            return {};
        }();

        if (components.hasValue() && wglCreateContextAttribsARB != nullptr)
        {
           #if DRX_DEBUG
            constexpr auto contextFlags = WGL_CONTEXT_DEBUG_BIT_ARB;
            constexpr auto noErrorChecking = GL_FALSE;
           #else
            constexpr auto contextFlags = 0;
            constexpr auto noErrorChecking = GL_TRUE;
           #endif

            i32k attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB,   components->major,
                WGL_CONTEXT_MINOR_VERSION_ARB,   components->minor,
                WGL_CONTEXT_PROFILE_MASK_ARB,    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB,           contextFlags,
                WGL_CONTEXT_OPENGL_NO_ERROR_ARB, noErrorChecking,
                0
            };

            const auto c = wglCreateContextAttribsARB (dcIn, nullptr, attribs);

            if (c != nullptr)
                return c;
        }

        return wglCreateContext (dcIn);
    }

    //==============================================================================
    struct PlaceholderComponent  : public Component
    {
        explicit PlaceholderComponent (NativeContext& c)
            : context (c)
        {
            setOpaque (true);
        }

        // The windowing code will call this when a paint callback happens
        z0 handleCommandMessage (i32) override   { context.triggerRepaint(); }

        NativeContext& context;
    };

    //==============================================================================
    z0 nativeScaleFactorChanged (f64 newScaleFactor) override
    {
        if (approximatelyEqual (newScaleFactor, nativeScaleFactor)
            || safeComponent == nullptr)
            return;

        if (auto* peer = safeComponent->getTopLevelComponent()->getPeer())
        {
            nativeScaleFactor = newScaleFactor;
            updateWindowPosition (peer->getAreaCoveredBy (*safeComponent));
        }
    }

    z0 createNativeWindow (Component& component)
    {
        auto* topComp = component.getTopLevelComponent();

        {
            auto* parentHWND = topComp->getWindowHandle();

            ScopedThreadDPIAwarenessSetter setter { parentHWND };
            nativeWindow.reset (createNonRepaintingEmbeddedWindowsPeer (*placeholderComponent, topComp));
        }

        if (auto* peer = topComp->getPeer())
        {
            safeComponent = Component::SafePointer<Component> (&component);

            nativeScaleFactor = peer->getPlatformScaleFactor();
            updateWindowPosition (peer->getAreaCoveredBy (component));
            peer->addScaleFactorListener (this);
        }

        dc = std::unique_ptr<std::remove_pointer_t<HDC>, DeviceContextDeleter> { GetDC ((HWND) nativeWindow->getNativeHandle()),
                                                                                 DeviceContextDeleter { (HWND) nativeWindow->getNativeHandle() } };
    }

    i32 wglChoosePixelFormatExtension (const OpenGLPixelFormat& pixelFormat) const
    {
        i32 format = 0;

        if (wglChoosePixelFormatARB != nullptr)
        {
            i32 atts[64];
            i32 n = 0;

            atts[n++] = WGL_DRAW_TO_WINDOW_ARB;   atts[n++] = GL_TRUE;
            atts[n++] = WGL_SUPPORT_OPENGL_ARB;   atts[n++] = GL_TRUE;
            atts[n++] = WGL_DOUBLE_BUFFER_ARB;    atts[n++] = GL_TRUE;
            atts[n++] = WGL_PIXEL_TYPE_ARB;       atts[n++] = WGL_TYPE_RGBA_ARB;
            atts[n++] = WGL_ACCELERATION_ARB;
            atts[n++] = WGL_FULL_ACCELERATION_ARB;

            atts[n++] = WGL_COLOR_BITS_ARB;  atts[n++] = pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits;
            atts[n++] = WGL_RED_BITS_ARB;    atts[n++] = pixelFormat.redBits;
            atts[n++] = WGL_GREEN_BITS_ARB;  atts[n++] = pixelFormat.greenBits;
            atts[n++] = WGL_BLUE_BITS_ARB;   atts[n++] = pixelFormat.blueBits;
            atts[n++] = WGL_ALPHA_BITS_ARB;  atts[n++] = pixelFormat.alphaBits;
            atts[n++] = WGL_DEPTH_BITS_ARB;  atts[n++] = pixelFormat.depthBufferBits;

            atts[n++] = WGL_STENCIL_BITS_ARB;       atts[n++] = pixelFormat.stencilBufferBits;
            atts[n++] = WGL_ACCUM_RED_BITS_ARB;     atts[n++] = pixelFormat.accumulationBufferRedBits;
            atts[n++] = WGL_ACCUM_GREEN_BITS_ARB;   atts[n++] = pixelFormat.accumulationBufferGreenBits;
            atts[n++] = WGL_ACCUM_BLUE_BITS_ARB;    atts[n++] = pixelFormat.accumulationBufferBlueBits;
            atts[n++] = WGL_ACCUM_ALPHA_BITS_ARB;   atts[n++] = pixelFormat.accumulationBufferAlphaBits;

            if (pixelFormat.multisamplingLevel > 0
                  && OpenGLHelpers::isExtensionSupported ("GL_ARB_multisample"))
            {
                atts[n++] = WGL_SAMPLE_BUFFERS_ARB;
                atts[n++] = 1;
                atts[n++] = WGL_SAMPLES_ARB;
                atts[n++] = pixelFormat.multisamplingLevel;
            }

            atts[n++] = 0;
            jassert (n <= numElementsInArray (atts));

            UINT formatsCount = 0;
            wglChoosePixelFormatARB (dc.get(), atts, nullptr, 1, &format, &formatsCount);
        }

        return format;
    }

    //==============================================================================
    #define DRX_DECLARE_WGL_EXTENSION_FUNCTION(name, returnType, params) \
        typedef returnType (__stdcall *type_ ## name) params; static type_ ## name name;

    DRX_DECLARE_WGL_EXTENSION_FUNCTION (wglChoosePixelFormatARB,    BOOL,  (HDC, i32k*, const FLOAT*, UINT, i32*, UINT*))
    DRX_DECLARE_WGL_EXTENSION_FUNCTION (wglSwapIntervalEXT,         BOOL,  (i32))
    DRX_DECLARE_WGL_EXTENSION_FUNCTION (wglGetSwapIntervalEXT,      i32,   ())
    DRX_DECLARE_WGL_EXTENSION_FUNCTION (wglCreateContextAttribsARB, HGLRC, (HDC, HGLRC, i32k*))
    #undef DRX_DECLARE_WGL_EXTENSION_FUNCTION

    //==============================================================================
    struct RenderContextDeleter
    {
        z0 operator() (HGLRC ptr) const { wglDeleteContext (ptr); }
    };

    struct DeviceContextDeleter
    {
        z0 operator() (HDC ptr) const { ReleaseDC (hwnd, ptr); }
        HWND hwnd;
    };

    CriticalSection mutex;
    std::unique_ptr<PlaceholderComponent> placeholderComponent;
    std::unique_ptr<ComponentPeer> nativeWindow;
    std::unique_ptr<ScopedThreadDPIAwarenessSetter> threadAwarenessSetter;
    Component::SafePointer<Component> safeComponent;
    std::unique_ptr<std::remove_pointer_t<HGLRC>, RenderContextDeleter> renderContext;
    std::unique_ptr<std::remove_pointer_t<HDC>, DeviceContextDeleter> dc;
    OpenGLContext* context = nullptr;
    uk sharedContext = nullptr;
    f64 nativeScaleFactor = 1.0;
    b8 haveBuffersBeenSwapped = false;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};


//==============================================================================
b8 OpenGLHelpers::isContextActive()
{
    return wglGetCurrentContext() != nullptr;
}

} // namespace drx
