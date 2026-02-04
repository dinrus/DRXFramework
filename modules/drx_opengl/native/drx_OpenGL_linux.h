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

struct XFreeDeleter
{
    z0 operator() (uk ptr) const
    {
        if (ptr != nullptr)
            X11Symbols::getInstance()->xFree (ptr);
    }
};

template <typename Data>
std::unique_ptr<Data, XFreeDeleter> makeXFreePtr (Data* raw) { return std::unique_ptr<Data, XFreeDeleter> (raw); }

//==============================================================================
// Defined in drx_Windowing_linux.cpp
z0 drx_LinuxAddRepaintListener (ComponentPeer*, Component* dummy);
z0 drx_LinuxRemoveRepaintListener (ComponentPeer*, Component* dummy);

class PeerListener : private ComponentMovementWatcher
{
public:
    PeerListener (Component& comp, Window embeddedWindow)
        : ComponentMovementWatcher (&comp),
          window (embeddedWindow),
          association (comp.getPeer(), window) {}

private:
    using ComponentMovementWatcher::componentMovedOrResized,
          ComponentMovementWatcher::componentVisibilityChanged;

    z0 componentMovedOrResized (b8, b8) override {}
    z0 componentVisibilityChanged() override {}

    z0 componentPeerChanged() override
    {
        // This should not be rewritten as a ternary expression or similar.
        // The old association must be destroyed before the new one is created.
        association = {};

        if (auto* comp = getComponent())
            association = ScopedWindowAssociation (comp->getPeer(), window);
    }

    Window window{};
    ScopedWindowAssociation association;
};

//==============================================================================
class OpenGLContext::NativeContext
{
private:
    struct DummyComponent  : public Component
    {
        DummyComponent (OpenGLContext::NativeContext& nativeParentContext)
            : native (nativeParentContext)
        {
        }

        z0 handleCommandMessage (i32 commandId) override
        {
            if (commandId == 0)
                native.triggerRepaint();
        }

        OpenGLContext::NativeContext& native;
    };

    template <typename Traits>
    class ScopedGLXObject
    {
    public:
        using Type = typename Traits::Type;

        ScopedGLXObject() = default;

        explicit ScopedGLXObject (Type obj, ::Display* d)
            : object (obj), display (d) {}

        ScopedGLXObject (ScopedGLXObject&& other) noexcept
            : object (std::exchange (other.object, Type{})),
              display (std::exchange (other.display, nullptr)) {}

        ScopedGLXObject& operator= (ScopedGLXObject&& other) noexcept
        {
            ScopedGLXObject { std::move (other) }.swap (*this);
            return *this;
        }

        ~ScopedGLXObject() noexcept
        {
            if (object != Type{})
                Traits::destroy (display, object);
        }

        Type get() const { return object; }

        z0 reset() noexcept
        {
            *this = ScopedGLXObject();
        }

        z0 swap (ScopedGLXObject& other) noexcept
        {
            std::swap (other.object, object);
            std::swap (other.display, display);
        }

        b8 operator== (const ScopedGLXObject& other) const
        {
            const auto tie = [] (const auto& x) { return std::tie (x.object, x.display); };
            return tie (*this) == tie (other);
        }

        b8 operator!= (const ScopedGLXObject& other) const
        {
            return ! operator== (other);
        }

    private:
        Type object{};
        ::Display* display{};
    };

    struct TraitsGLXContext
    {
        using Type = GLXContext;

        static z0 destroy (::Display* display, Type t)
        {
            glXDestroyContext (display, t);
        }
    };

    struct TraitsGLXWindow
    {
        using Type = GLXWindow;

        static z0 destroy (::Display* display, Type t)
        {
            glXDestroyWindow (display, t);
        }
    };

    using PtrGLXContext = ScopedGLXObject<TraitsGLXContext>;
    using PtrGLXWindow = ScopedGLXObject<TraitsGLXWindow>;

public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& cPixelFormat,
                   uk shareContext,
                   b8 useMultisamplingIn,
                   OpenGLVersion)
        : component (comp), contextToShareWith (shareContext), dummy (*this)
    {
        display = XWindowSystem::getInstance()->getDisplay();

        XWindowSystemUtilities::ScopedXLock xLock;

        X11Symbols::getInstance()->xSync (display, False);

        const std::vector<GLint> optionalAttribs
        {
            GLX_SAMPLE_BUFFERS, useMultisamplingIn ? 1 : 0,
            GLX_SAMPLES,        cPixelFormat.multisamplingLevel
        };

        if (! tryChooseVisual (cPixelFormat, optionalAttribs) && ! tryChooseVisual (cPixelFormat, {}))
            return;

        auto* peer = component.getPeer();
        jassert (peer != nullptr);

        auto windowH = (Window) peer->getNativeHandle();
        auto visual = glXGetVisualFromFBConfig (display, *bestConfig);
        auto colourMap = X11Symbols::getInstance()->xCreateColormap (display, windowH, visual->visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap = colourMap;
        swa.border_pixel = 0;
        swa.event_mask = embeddedWindowEventMask;

        auto glBounds = component.getTopLevelComponent()->getLocalArea (&component, component.getLocalBounds());

        glBounds = Desktop::getInstance().getDisplays().logicalToPhysical (glBounds);

        embeddedWindow = X11Symbols::getInstance()->xCreateWindow (display, windowH,
                                                                   glBounds.getX(), glBounds.getY(),
                                                                   (u32) jmax (1, glBounds.getWidth()),
                                                                   (u32) jmax (1, glBounds.getHeight()),
                                                                   0, visual->depth,
                                                                   InputOutput,
                                                                   visual->visual,
                                                                   CWBorderPixel | CWColormap | CWEventMask,
                                                                   &swa);

        peerListener.emplace (component, embeddedWindow);

        X11Symbols::getInstance()->xMapWindow (display, embeddedWindow);
        X11Symbols::getInstance()->xFreeColormap (display, colourMap);

        X11Symbols::getInstance()->xSync (display, False);

        drx_LinuxAddRepaintListener (peer, &dummy);
    }

    ~NativeContext()
    {
        if (auto* peer = component.getPeer())
        {
            drx_LinuxRemoveRepaintListener (peer, &dummy);

            if (embeddedWindow != 0)
            {
                XWindowSystemUtilities::ScopedXLock xLock;

                X11Symbols::getInstance()->xUnmapWindow (display, embeddedWindow);
                X11Symbols::getInstance()->xDestroyWindow (display, embeddedWindow);
                X11Symbols::getInstance()->xSync (display, False);

                XEvent event;
                while (X11Symbols::getInstance()->xCheckWindowEvent (display,
                                                                     embeddedWindow,
                                                                     embeddedWindowEventMask,
                                                                     &event) == True)
                {
                }
            }
        }
    }

    InitResult initialiseOnRenderThread (OpenGLContext& c)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        const auto components = [&]() -> Optional<Version>
        {
            switch (c.versionRequired)
            {
                case OpenGLVersion::openGL3_2: return Version { 3, 2 };
                case OpenGLVersion::openGL4_1: return Version { 4, 1 };
                case OpenGLVersion::openGL4_3: return Version { 4, 3 };

                case OpenGLVersion::defaultGLVersion: break;
            }

            return {};
        }();

        if (components.hasValue())
        {
            using GLXCreateContextAttribsARB = GLXContext (*) (Display*, GLXFBConfig, GLXContext, Bool, i32k*);

            if (const auto glXCreateContextAttribsARB = (GLXCreateContextAttribsARB) OpenGLHelpers::getExtensionFunction ("glXCreateContextAttribsARB"))
            {
               #if DRX_DEBUG
                constexpr auto contextFlags = GLX_CONTEXT_DEBUG_BIT_ARB;
               #else
                constexpr auto contextFlags = 0;
               #endif

                i32k attribs[]
                {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, components->major,
                    GLX_CONTEXT_MINOR_VERSION_ARB, components->minor,
                    GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                    GLX_CONTEXT_FLAGS_ARB,         contextFlags,
                    None
                };

                renderContext = PtrGLXContext { glXCreateContextAttribsARB (display, *bestConfig, (GLXContext) contextToShareWith, GL_TRUE, attribs),
                                                display };
            }
        }

        if (renderContext == PtrGLXContext{})
            renderContext = PtrGLXContext { glXCreateNewContext (display, *bestConfig, GLX_RGBA_TYPE, (GLXContext) contextToShareWith, GL_TRUE),
                                            display };

        if (renderContext == PtrGLXContext{})
            return InitResult::fatal;

        glxWindow = PtrGLXWindow { glXCreateWindow (display, *bestConfig, embeddedWindow, nullptr),
                                   display };
        c.makeActive();
        context = &c;
        return InitResult::success;
    }

    z0 shutdownOnRenderThread()
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        context = nullptr;
        deactivateCurrentContext();
        renderContext.reset();
        glxWindow.reset();
    }

    b8 makeActive() const noexcept
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        return renderContext != PtrGLXContext{}
                 && glXMakeContextCurrent (display, glxWindow.get(), glxWindow.get(), renderContext.get());
    }

    b8 isActive() const noexcept
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        return glXGetCurrentContext() == renderContext.get() && renderContext != PtrGLXContext{};
    }

    static z0 deactivateCurrentContext()
    {
        if (auto* display = XWindowSystem::getInstance()->getDisplay())
        {
            XWindowSystemUtilities::ScopedXLock xLock;
            glXMakeCurrent (display, None, nullptr);
        }
    }

    z0 swapBuffers()
    {
        glXSwapBuffers (display, glxWindow.get());
    }

    z0 updateWindowPosition (Rectangle<i32> newBounds)
    {
        bounds = newBounds;
        auto physicalBounds = Desktop::getInstance().getDisplays().logicalToPhysical (bounds);

        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xMoveResizeWindow (display, embeddedWindow,
                                                      physicalBounds.getX(), physicalBounds.getY(),
                                                      (u32) jmax (1, physicalBounds.getWidth()),
                                                      (u32) jmax (1, physicalBounds.getHeight()));
    }

    b8 setSwapInterval (i32 numFramesPerSwap)
    {
        if (numFramesPerSwap == swapFrames)
            return true;

        if (auto GLXSwapIntervalEXT
              = (PFNGLXSWAPINTERVALEXTPROC) OpenGLHelpers::getExtensionFunction ("glXSwapIntervalEXT"))
        {
            XWindowSystemUtilities::ScopedXLock xLock;
            swapFrames = numFramesPerSwap;
            GLXSwapIntervalEXT (display, glxWindow.get(), numFramesPerSwap);
            return true;
        }

        return false;
    }

    i32 getSwapInterval() const                 { return swapFrames; }
    b8 createdOk() const noexcept             { return true; }
    uk getRawContext() const noexcept        { return renderContext.get(); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

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

private:
    b8 tryChooseVisual (const OpenGLPixelFormat& format, const std::vector<GLint>& optionalAttribs)
    {
        std::vector<GLint> allAttribs
        {
            GLX_RENDER_TYPE,      GLX_RGBA_BIT,
            GLX_DOUBLEBUFFER,     True,
            GLX_RED_SIZE,         format.redBits,
            GLX_GREEN_SIZE,       format.greenBits,
            GLX_BLUE_SIZE,        format.blueBits,
            GLX_ALPHA_SIZE,       format.alphaBits,
            GLX_DEPTH_SIZE,       format.depthBufferBits,
            GLX_STENCIL_SIZE,     format.stencilBufferBits,
            GLX_ACCUM_RED_SIZE,   format.accumulationBufferRedBits,
            GLX_ACCUM_GREEN_SIZE, format.accumulationBufferGreenBits,
            GLX_ACCUM_BLUE_SIZE,  format.accumulationBufferBlueBits,
            GLX_ACCUM_ALPHA_SIZE, format.accumulationBufferAlphaBits
        };

        allAttribs.insert (allAttribs.end(), optionalAttribs.begin(), optionalAttribs.end());

        allAttribs.push_back (None);

        i32 nElements = 0;
        bestConfig = makeXFreePtr (glXChooseFBConfig (display, X11Symbols::getInstance()->xDefaultScreen (display), allAttribs.data(), &nElements));

        return nElements != 0 && bestConfig != nullptr;
    }

    static constexpr i32 embeddedWindowEventMask = ExposureMask | StructureNotifyMask;

    CriticalSection mutex;
    Component& component;
    PtrGLXContext renderContext;
    PtrGLXWindow glxWindow;
    Window embeddedWindow = {};

    std::optional<PeerListener> peerListener;

    i32 swapFrames = 0;
    Rectangle<i32> bounds;
    std::unique_ptr<GLXFBConfig, XFreeDeleter> bestConfig;
    uk contextToShareWith;

    OpenGLContext* context = nullptr;
    DummyComponent dummy;

    ::Display* display = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
b8 OpenGLHelpers::isContextActive()
{
    XWindowSystemUtilities::ScopedXLock xLock;
    return glXGetCurrentContext() != nullptr;
}

} // namespace drx
