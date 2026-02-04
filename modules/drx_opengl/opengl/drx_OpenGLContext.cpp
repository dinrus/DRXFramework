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

#if DRX_MAC
 #include <drx_gui_basics/native/drx_PerScreenDisplayLinks_mac.h>
#endif

namespace drx
{

#if DRX_IOS
struct AppInactivityCallback // NB: this is a duplicate of an internal declaration in drx_core
{
    virtual ~AppInactivityCallback() {}
    virtual z0 appBecomingInactive() = 0;
};

extern Array<AppInactivityCallback*> appBecomingInactiveCallbacks;

// On iOS, all GL calls will crash when the app is running in the background, so
// this prevents them from happening (which some messy locking behaviour)
struct iOSBackgroundProcessCheck final : public AppInactivityCallback
{
    iOSBackgroundProcessCheck()              { isBackgroundProcess(); appBecomingInactiveCallbacks.add (this); }
    ~iOSBackgroundProcessCheck() override    { appBecomingInactiveCallbacks.removeAllInstancesOf (this); }

    b8 isBackgroundProcess()
    {
        const b8 b = Process::isForegroundProcess();
        isForeground.set (b ? 1 : 0);
        return ! b;
    }

    z0 appBecomingInactive() override
    {
        i32 counter = 2000;

        while (--counter > 0 && isForeground.get() != 0)
            Thread::sleep (1);
    }

private:
    Atomic<i32> isForeground;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSBackgroundProcessCheck)
};

#endif

#if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
 extern DRX_API f64 getScaleFactorForWindow (HWND);
#endif

static b8 contextHasTextureNpotFeature()
{
    if (getOpenGLVersion() >= Version (2))
        return true;

    // If the version is < 2, we can't use the newer extension-checking API
    // so we have to use glGetString
    const auto* extensionsBegin = glGetString (GL_EXTENSIONS);

    if (extensionsBegin == nullptr)
        return false;

    const auto* extensionsEnd = findNullTerminator (extensionsBegin);
    const std::string extensionsString (extensionsBegin, extensionsEnd);
    const auto stringTokens = StringArray::fromTokens (extensionsString.c_str(), false);
    return stringTokens.contains ("GL_ARB_texture_non_power_of_two");
}

//==============================================================================
class OpenGLContext::CachedImage final : public CachedComponentImage
{
    template <typename T, typename U>
    static constexpr b8 isFlagSet (const T& t, const U& u) { return (t & u) != 0; }

    struct AreaAndScale
    {
        Rectangle<i32> area;
        f64 scale;

        auto tie() const { return std::tie (area, scale); }

        auto operator== (const AreaAndScale& other) const { return tie() == other.tie(); }
        auto operator!= (const AreaAndScale& other) const { return tie() != other.tie(); }
    };

    class LockedAreaAndScale
    {
    public:
        auto get() const
        {
            const ScopedLock lock (mutex);
            return data;
        }

        template <typename Fn>
        z0 set (const AreaAndScale& d, Fn&& ifDifferent)
        {
            const auto old = [&]
            {
                const ScopedLock lock (mutex);
                return std::exchange (data, d);
            }();

            if (old != d)
                ifDifferent();
        }

    private:
        CriticalSection mutex;
        AreaAndScale data { {}, 1.0 };
    };

public:
    CachedImage (OpenGLContext& c, Component& comp,
                 const OpenGLPixelFormat& pixFormat, uk contextToShare)
        : context (c),
          component (comp)
    {
        nativeContext.reset (new NativeContext (component, pixFormat, contextToShare,
                                                c.useMultisampling, c.versionRequired));

        if (nativeContext->createdOk())
            context.nativeContext = nativeContext.get();
        else
            nativeContext.reset();

        refreshDisplayLinkConnection();
    }

    ~CachedImage() override
    {
        stop();
    }

    //==============================================================================
    z0 start()
    {
        if (nativeContext != nullptr)
            resume();
    }

    z0 stop()
    {
        // make sure everything has finished executing
        state |= StateFlags::pendingDestruction;

        if (workQueue.size() > 0)
        {
            if (! renderThread->contains (this))
                resume();

            while (workQueue.size() != 0)
                Thread::sleep (20);
        }

        pause();
    }

    //==============================================================================
    z0 pause()
    {
        renderThread->remove (this);

        if ((state.fetch_and (~StateFlags::initialised) & StateFlags::initialised) == 0)
            return;

        ScopedContextActivator activator;
        activator.activate (context);

        if (context.renderer != nullptr)
            context.renderer->openGLContextClosing();

        associatedObjectNames.clear();
        associatedObjects.clear();
        cachedImageFrameBuffer.release();
        nativeContext->shutdownOnRenderThread();
    }

    z0 resume()
    {
        renderThread->add (this);
    }

    //==============================================================================
    z0 paint (Graphics&) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            updateViewportSize();
        }
        else
        {
            // If you hit this assertion, it's because paint has been called from a thread other
            // than the message thread. This commonly happens when nesting OpenGL contexts, because
            // the 'outer' OpenGL renderer will attempt to call paint on the 'inner' context's
            // component from the OpenGL thread.
            // Nesting OpenGL contexts is not directly supported, however there is a workaround:
            // https://forum.drx.com/t/opengl-how-do-3d-with-custom-shaders-and-2d-with-drx-paint-methods-work-together/28026/7
            jassertfalse;
        }
    }

    b8 invalidateAll() override
    {
        validArea.clear();
        triggerRepaint();
        return false;
    }

    b8 invalidate (const Rectangle<i32>& area) override
    {
        validArea.subtract (area.toFloat().transformedBy (transform).getSmallestIntegerContainer());
        triggerRepaint();
        return false;
    }

    z0 releaseResources() override
    {
        stop();
    }

    z0 triggerRepaint()
    {
        state |= (StateFlags::pendingRender | StateFlags::paintComponents);
        renderThread->triggerRepaint();
    }

    //==============================================================================
    b8 ensureFrameBufferSize (Rectangle<i32> viewportArea)
    {
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        auto fbW = cachedImageFrameBuffer.getWidth();
        auto fbH = cachedImageFrameBuffer.getHeight();

        if (fbW != viewportArea.getWidth() || fbH != viewportArea.getHeight() || ! cachedImageFrameBuffer.isValid())
        {
            if (! cachedImageFrameBuffer.initialise (context, viewportArea.getWidth(), viewportArea.getHeight()))
                return false;

            validArea.clear();
            DRX_CHECK_OPENGL_ERROR
        }

        return true;
    }

    z0 clearRegionInFrameBuffer (const RectangleList<i32>& list)
    {
        glClearColor (0, 0, 0, 0);
        glEnable (GL_SCISSOR_TEST);

        auto previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();
        cachedImageFrameBuffer.makeCurrentRenderingTarget();
        auto imageH = cachedImageFrameBuffer.getHeight();

        for (auto& r : list)
        {
            glScissor (r.getX(), imageH - r.getBottom(), r.getWidth(), r.getHeight());
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        glDisable (GL_SCISSOR_TEST);
        context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
        DRX_CHECK_OPENGL_ERROR
    }

    struct ScopedContextActivator
    {
        b8 activate (OpenGLContext& ctx)
        {
            if (! active)
                active = ctx.makeActive();

            return active;
        }

        ~ScopedContextActivator()
        {
            if (active)
                OpenGLContext::deactivateCurrentContext();
        }

    private:
        b8 active = false;
    };

    enum class RenderStatus
    {
        nominal,
        messageThreadAborted,
        noWork,
    };

    RenderStatus renderFrame (MessageManager::Lock& mmLock)
    {
        ScopedContextActivator contextActivator;

        if (! isFlagSet (state, StateFlags::initialised))
        {
            switch (initialiseOnThread (contextActivator))
            {
                case InitResult::fatal:
                case InitResult::retry: return RenderStatus::noWork;
                case InitResult::success: break;
            }
        }

        state |= StateFlags::initialised;

       #if DRX_IOS
        if (backgroundProcessCheck.isBackgroundProcess())
            return RenderStatus::noWork;
       #endif

        std::optional<MessageManager::Lock::ScopedTryLockType> scopedLock;

        const auto stateToUse = state.fetch_and (StateFlags::persistent);

       #if DRX_MAC
        // On macOS, we use a display link callback to trigger repaints, rather than
        // letting them run at full throttle
        const auto noAutomaticRepaint = true;
       #else
        const auto noAutomaticRepaint = ! context.continuousRepaint;
       #endif

        if (! isFlagSet (stateToUse, StateFlags::pendingRender) && noAutomaticRepaint)
            return RenderStatus::noWork;

        const auto isUpdating = isFlagSet (stateToUse, StateFlags::paintComponents);

        if (context.renderComponents && isUpdating)
        {
            b8 abortScope = false;
            // If we early-exit here, we need to restore these flags so that the render is
            // attempted again in the next time slice.
            const ScopeGuard scope { [&] { if (! abortScope) state |= stateToUse; } };

            // This avoids hogging the message thread when doing intensive rendering.
            std::this_thread::sleep_until (lastMMLockReleaseTime + std::chrono::milliseconds { 2 });

            if (renderThread->isListChanging())
                return RenderStatus::messageThreadAborted;

            doWorkWhileWaitingForLock (contextActivator);

            scopedLock.emplace (mmLock);

            // If we can't get the lock here, it's probably because a context has been removed
            // on the main thread.
            // We return, just in case this renderer needs to be removed from the rendering thread.
            // If another renderer is being removed instead, then we should be able to get the lock
            // next time round.
            if (! scopedLock->isLocked())
                return RenderStatus::messageThreadAborted;

            abortScope = true;
        }

        {
            NativeContext::Locker locker (*nativeContext);

            if (! contextActivator.activate (context))
                return RenderStatus::noWork;

            DRX_CHECK_OPENGL_ERROR

            doWorkWhileWaitingForLock (contextActivator);

            const auto currentAreaAndScale = areaAndScale.get();
            const auto viewportArea = currentAreaAndScale.area;

            if (context.renderer != nullptr)
            {
                OpenGLRendering::SavedBinding<OpenGLRendering::TraitsVAO> vaoBinding;

                glViewport (0, 0, viewportArea.getWidth(), viewportArea.getHeight());
                context.currentRenderScale = currentAreaAndScale.scale;
                context.renderer->renderOpenGL();
                clearGLError();
            }

            if (context.renderComponents)
            {
                if (isUpdating)
                {
                    paintComponent (currentAreaAndScale);

                    if (! isFlagSet (state, StateFlags::initialised))
                        return RenderStatus::noWork;

                    scopedLock.reset();
                    lastMMLockReleaseTime = std::chrono::steady_clock::now();
                }

                glViewport (0, 0, viewportArea.getWidth(), viewportArea.getHeight());
                drawComponentBuffer();
            }
        }

        bufferSwapper.swap();
        return RenderStatus::nominal;
    }

    z0 updateViewportSize()
    {
        DRX_ASSERT_MESSAGE_THREAD

        if (auto* peer = component.getPeer())
        {
           #if DRX_MAC
            updateScreen();

            const auto displayScale = Desktop::getInstance().getGlobalScaleFactor() * [this]
            {
                if (auto* view = getCurrentView())
                {
                    if ([view respondsToSelector: @selector (backingScaleFactor)])
                        return [(id) view backingScaleFactor];

                    if (auto* window = [view window])
                        return [window backingScaleFactor];
                }

                return areaAndScale.get().scale;
            }();
           #else
            const auto displayScale = Desktop::getInstance().getDisplays()
                                                            .getDisplayForRect (component.getTopLevelComponent()
                                                                                        ->getScreenBounds())
                                                           ->scale;
           #endif

            const auto localBounds = component.getLocalBounds();
            const auto newArea = peer->getComponent().getLocalArea (&component, localBounds).withZeroOrigin() * displayScale;

            // On Windows some hosts (Pro Tools 2022.7) do not take the current DPI into account
            // when sizing plugin editor windows.
            //
            // Also in plugins on Windows, the plugin HWND's DPI settings generally don't reflect
            // the desktop scaling setting and Displays::Display::scale will return an incorrect 1.0
            // value. Our plugin wrappers will use a combination of querying the plugin HWND's
            // parent HWND (the host HWND), and utilising the scale factor reported by the host
            // through the plugin API. This scale is then added as a transformation to the
            // AudioProcessorEditor.
            //
            // Hence, instead of querying the OS for the DPI of the editor window,
            // we approximate based on the physical size of the window that was actually provided
            // for the context to draw into. This may break if the OpenGL context's component is
            // scaled differently in its width and height - but in this case, a single scale factor
            // isn't that helpful anyway.
            const auto newScale = (f32) newArea.getWidth() / (f32) localBounds.getWidth();

            areaAndScale.set ({ newArea, newScale }, [&]
            {
                // Transform is only accessed when the message manager is locked
                transform = AffineTransform::scale ((f32) newArea.getWidth()  / (f32) localBounds.getWidth(),
                                                    (f32) newArea.getHeight() / (f32) localBounds.getHeight());

                nativeContext->updateWindowPosition (peer->getAreaCoveredBy (component));
                invalidateAll();
            });
        }
    }

    z0 checkViewportBounds()
    {
        auto screenBounds = component.getTopLevelComponent()->getScreenBounds();

        if (lastScreenBounds != screenBounds)
        {
            updateViewportSize();
            lastScreenBounds = screenBounds;
        }
    }

    z0 paintComponent (const AreaAndScale& currentAreaAndScale)
    {
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        // you mustn't set your own cached image object when attaching a GL context!
        jassert (get (component) == this);

        if (! ensureFrameBufferSize (currentAreaAndScale.area))
            return;

        RectangleList<i32> invalid (currentAreaAndScale.area);
        invalid.subtract (validArea);
        validArea = currentAreaAndScale.area;

        if (! invalid.isEmpty())
        {
            clearRegionInFrameBuffer (invalid);

            {
                std::unique_ptr<LowLevelGraphicsContext> g (createOpenGLGraphicsContext (context, cachedImageFrameBuffer));
                g->clipToRectangleList (invalid);
                g->addTransform (transform);

                paintOwner (*g);
                DRX_CHECK_OPENGL_ERROR
            }
        }

        DRX_CHECK_OPENGL_ERROR
    }

    z0 drawComponentBuffer()
    {
        if (! OpenGLRendering::TraitsVAO::isCoreProfile())
            glEnable (GL_TEXTURE_2D);

       #if DRX_WINDOWS
        // some stupidly old drivers are missing this function, so try to at least avoid a crash here,
        // but if you hit this assertion you may want to have your own version check before using the
        // component rendering stuff on such old drivers.
        jassert (context.extensions.glActiveTexture != nullptr);
        if (context.extensions.glActiveTexture != nullptr)
       #endif
        {
            context.extensions.glActiveTexture (GL_TEXTURE0);
        }

        glBindTexture (GL_TEXTURE_2D, cachedImageFrameBuffer.getTextureID());

        const Rectangle<i32> cacheBounds (cachedImageFrameBuffer.getWidth(), cachedImageFrameBuffer.getHeight());
        context.copyTexture (cacheBounds, cacheBounds, cacheBounds.getWidth(), cacheBounds.getHeight(), false);
        glBindTexture (GL_TEXTURE_2D, 0);
        DRX_CHECK_OPENGL_ERROR
    }

    z0 paintOwner (LowLevelGraphicsContext& llgc)
    {
        Graphics g (llgc);

      #if DRX_ENABLE_REPAINT_DEBUGGING
       #ifdef DRX_IS_REPAINT_DEBUGGING_ACTIVE
        if (DRX_IS_REPAINT_DEBUGGING_ACTIVE)
       #endif
        {
            g.saveState();
        }
       #endif

        DRX_TRY
        {
            component.paintEntireComponent (g, false);
        }
        DRX_CATCH_EXCEPTION

      #if DRX_ENABLE_REPAINT_DEBUGGING
       #ifdef DRX_IS_REPAINT_DEBUGGING_ACTIVE
        if (DRX_IS_REPAINT_DEBUGGING_ACTIVE)
       #endif
        {
            // enabling this code will fill all areas that get repainted with a colour overlay, to show
            // clearly when things are being repainted.
            g.restoreState();

            static Random rng;
            g.fillAll (Color ((u8) rng.nextInt (255),
                               (u8) rng.nextInt (255),
                               (u8) rng.nextInt (255),
                               (u8) 0x50));
        }
       #endif
    }

    z0 handleResize()
    {
        updateViewportSize();

       #if DRX_MAC
        if (isFlagSet (state, StateFlags::initialised))
        {
            [nativeContext->view update];

            // We're already on the message thread, no need to lock it again.
            MessageManager::Lock mml;
            renderFrame (mml);
        }
       #endif
    }

    //==============================================================================
    InitResult initialiseOnThread (ScopedContextActivator& activator)
    {
        // On android, this can get called twice, so drop any previous state.
        associatedObjectNames.clear();
        associatedObjects.clear();
        cachedImageFrameBuffer.release();

        activator.activate (context);

        if (const auto nativeResult = nativeContext->initialiseOnRenderThread (context); nativeResult != InitResult::success)
            return nativeResult;

       #if DRX_ANDROID
        // On android the context may be created in initialiseOnRenderThread
        // and we therefore need to call makeActive again
        context.makeActive();
       #endif

        gl::loadFunctions();

       #if DRX_DEBUG && ! DRX_DISABLE_ASSERTIONS
        if (getOpenGLVersion() >= Version { 4, 3 } && glDebugMessageCallback != nullptr)
        {
            glEnable (GL_DEBUG_OUTPUT);
            glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback ([] (GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, ukk)
            {
                // This may reiterate issues that are also flagged by DRX_CHECK_OPENGL_ERROR.
                // The advantage of this callback is that it will catch *all* errors, even if we
                // forget to check manually.
                DBG ("OpenGL DBG message: " << message);
                jassert (type != GL_DEBUG_TYPE_ERROR && severity != GL_DEBUG_SEVERITY_HIGH);
            }, nullptr);
        }
       #endif

        const auto currentViewportArea = areaAndScale.get().area;
        glViewport (0, 0, currentViewportArea.getWidth(), currentViewportArea.getHeight());

        nativeContext->setSwapInterval (1);

       #if ! DRX_OPENGL_ES
        DRX_CHECK_OPENGL_ERROR
        shadersAvailable = OpenGLShaderProgram::getLanguageVersion() > 0;
        clearGLError();
       #endif

        textureNpotSupported = contextHasTextureNpotFeature();

        if (context.renderer != nullptr)
            context.renderer->newOpenGLContextCreated();

        return InitResult::success;
    }

    //==============================================================================
    struct BlockingWorker final : public OpenGLContext::AsyncWorker
    {
        BlockingWorker (OpenGLContext::AsyncWorker::Ptr && workerToUse)
            : originalWorker (std::move (workerToUse))
        {}

        z0 operator() (OpenGLContext& calleeContext) override
        {
            if (originalWorker != nullptr)
                (*originalWorker) (calleeContext);

            finishedSignal.signal();
        }

        z0 block() noexcept  { finishedSignal.wait(); }

        OpenGLContext::AsyncWorker::Ptr originalWorker;
        WaitableEvent finishedSignal;
    };

    z0 doWorkWhileWaitingForLock (ScopedContextActivator& contextActivator)
    {
        while (const auto work = workQueue.removeAndReturn (0))
        {
            if (renderThread->isListChanging() || ! contextActivator.activate (context))
                break;

            NativeContext::Locker locker (*nativeContext);

            (*work) (context);
            clearGLError();
        }
    }

    z0 execute (OpenGLContext::AsyncWorker::Ptr workerToUse, b8 shouldBlock)
    {
        if (! isFlagSet (state, StateFlags::pendingDestruction))
        {
            if (shouldBlock)
            {
                auto blocker = new BlockingWorker (std::move (workerToUse));
                OpenGLContext::AsyncWorker::Ptr worker (*blocker);
                workQueue.add (worker);

                renderThread->abortLock();
                context.triggerRepaint();

                blocker->block();
            }
            else
            {
                workQueue.add (std::move (workerToUse));

                renderThread->abortLock();
                context.triggerRepaint();
            }
        }
        else
        {
            jassertfalse; // you called execute AFTER you detached your OpenGLContext
        }
    }

    //==============================================================================
    static CachedImage* get (Component& c) noexcept
    {
        return dynamic_cast<CachedImage*> (c.getCachedComponentImage());
    }

    class RenderThread
    {
    public:
        RenderThread() = default;

        ~RenderThread()
        {
            flags.setDestructing();
            thread.join();
        }

        z0 add (CachedImage* x)
        {
            const std::scoped_lock lock { listMutex };
            images.push_back (x);
        }

        z0 remove (CachedImage* x)
        {
            DRX_ASSERT_MESSAGE_THREAD;

            flags.setSafe (false);
            abortLock();

            {
                const std::scoped_lock lock { callbackMutex, listMutex };
                images.remove (x);
            }

            flags.setSafe (true);
        }

        b8 contains (CachedImage* x)
        {
            const std::scoped_lock lock { listMutex };
            return std::find (images.cbegin(), images.cend(), x) != images.cend();
        }

        z0 triggerRepaint()   { flags.setRenderRequested(); }

        z0 abortLock()        { messageManagerLock.abort(); }

        b8 isListChanging()   { return ! flags.isSafe(); }

    private:
        RenderStatus renderAll()
        {
            auto result = RenderStatus::noWork;

            const std::scoped_lock lock { callbackMutex, listMutex };

            for (auto* x : images)
            {
                listMutex.unlock();
                const ScopeGuard scope { [&] { listMutex.lock(); } };

                const auto status = x->renderFrame (messageManagerLock);

                switch (status)
                {
                    case RenderStatus::noWork: break;
                    case RenderStatus::nominal: result = RenderStatus::nominal; break;
                    case RenderStatus::messageThreadAborted: return RenderStatus::messageThreadAborted;
                }
            }

            return result;
        }

        /*  Allows the main thread to communicate changes to the render thread.

            When the render thread needs to change in some way (asked to resume rendering,
            a renderer is added/removed, or the thread needs to stop prior to destruction),
            the main thread can set the appropriate flag on this structure. The render thread
            will call waitForWork() repeatedly, pausing when the render thread has no work to do,
            and resuming when requested by the main thread.
        */
        class Flags
        {
        public:
            z0 setDestructing()       { update ([] (auto& f) { f |= destructorCalled; }); }
            z0 setRenderRequested()   { update ([] (auto& f) { f |= renderRequested;  }); }

            z0 setSafe (const b8 safe)
            {
                update ([safe] (auto& f)
                {
                    if (safe)
                        f |= listSafe;
                    else
                        f &= ~listSafe;
                });
            }

            b8 isSafe()
            {
                const std::scoped_lock lock { mutex };
                return (flags & listSafe) != 0;
            }

            /*  Blocks until the 'safe' flag is set, and at least one other flag is set.
                After returning, the renderRequested flag will be unset.
                Возвращает true, если rendering should continue.
            */
            b8 waitForWork (b8 requestRender)
            {
                std::unique_lock lock { mutex };
                flags |= (requestRender ? renderRequested : 0);
                condvar.wait (lock, [this] { return flags > listSafe; });
                flags &= ~renderRequested;
                return ((flags & destructorCalled) == 0);
            }

        private:
            template <typename Fn>
            z0 update (Fn fn)
            {
                {
                    const std::scoped_lock lock { mutex };
                    fn (flags);
                }

                condvar.notify_one();
            }

            enum
            {
                renderRequested  = 1 << 0,
                destructorCalled = 1 << 1,
                listSafe         = 1 << 2
            };

            std::mutex mutex;
            std::condition_variable condvar;
            i32 flags = listSafe;
        };

        MessageManager::Lock messageManagerLock;
        std::mutex listMutex, callbackMutex;
        std::list<CachedImage*> images;
        Flags flags;

        std::thread thread { [this]
        {
            Thread::setCurrentThreadName ("OpenGL Renderer");
            while (flags.waitForWork (renderAll() != RenderStatus::noWork)) {}
        } };
    };

    z0 refreshDisplayLinkConnection()
    {
       #if DRX_MAC
        if (context.continuousRepaint)
        {
            connection.emplace (sharedDisplayLinks->registerFactory ([this] (CGDirectDisplayID display)
            {
                return [this, display] (f64)
                {
                    if (display == lastDisplay)
                        triggerRepaint();
                };
            }));
        }
        else
        {
            connection.reset();
        }
       #endif
    }

    //==============================================================================
    class BufferSwapper final : private AsyncUpdater
    {
    public:
        explicit BufferSwapper (CachedImage& img)
            : image (img) {}

        ~BufferSwapper() override
        {
            cancelPendingUpdate();
        }

        z0 swap()
        {
            static const auto swapBuffersOnMainThread = []
            {
                const auto os = SystemStats::getOperatingSystemType();

                if ((os & SystemStats::MacOSX) != 0)
                    return (os != SystemStats::MacOSX && os < SystemStats::MacOSX_10_14);

                return false;
            }();

            if (swapBuffersOnMainThread && ! MessageManager::getInstance()->isThisTheMessageThread())
                triggerAsyncUpdate();
            else
                image.nativeContext->swapBuffers();
        }

    private:
        z0 handleAsyncUpdate() override
        {
            ScopedContextActivator activator;
            activator.activate (image.context);

            NativeContext::Locker locker (*image.nativeContext);
            image.nativeContext->swapBuffers();
        }

        CachedImage& image;
    };

    //==============================================================================
    friend class NativeContext;
    std::unique_ptr<NativeContext> nativeContext;

    OpenGLContext& context;
    Component& component;

    SharedResourcePointer<RenderThread> renderThread;

    OpenGLFrameBuffer cachedImageFrameBuffer;
    RectangleList<i32> validArea;
    Rectangle<i32> lastScreenBounds;
    AffineTransform transform;
    LockedAreaAndScale areaAndScale;

    StringArray associatedObjectNames;
    ReferenceCountedArray<ReferenceCountedObject> associatedObjects;

    WaitableEvent canPaintNowFlag, finishedPaintingFlag;
   #if DRX_OPENGL_ES
    b8 shadersAvailable = true;
   #else
    b8 shadersAvailable = false;
   #endif
    b8 textureNpotSupported = false;
    std::chrono::steady_clock::time_point lastMMLockReleaseTime{};
    BufferSwapper bufferSwapper { *this };

   #if DRX_MAC
    NSView* getCurrentView() const
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (auto* peer = component.getPeer())
            return static_cast<NSView*> (peer->getNativeHandle());

        return nullptr;
    }

    NSWindow* getCurrentWindow() const
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (auto* view = getCurrentView())
            return [view window];

        return nullptr;
    }

    NSScreen* getCurrentScreen() const
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (auto* window = getCurrentWindow())
            return [window screen];

        return nullptr;
    }

    z0 updateScreen()
    {
        const auto screen = getCurrentScreen();
        const auto display = ScopedDisplayLink::getDisplayIdForScreen (screen);

        if (lastDisplay.exchange (display) == display)
            return;

        const auto newRefreshPeriod = sharedDisplayLinks->getNominalVideoRefreshPeriodSForScreen (display);

        if (newRefreshPeriod != 0.0 && ! approximatelyEqual (std::exchange (refreshPeriod, newRefreshPeriod), newRefreshPeriod))
            nativeContext->setNominalVideoRefreshPeriodS (newRefreshPeriod);

        updateColorSpace();
    }

    z0 updateColorSpace()
    {
        if (auto* view = nativeContext->getNSView())
            if (auto* window = [view window])
                [window setColorSpace: [NSColorSpace sRGBColorSpace]];
    }

    std::atomic<CGDirectDisplayID> lastDisplay { 0 };
    f64 refreshPeriod = 0.0;

    FunctionNotificationCenterObserver observer { NSWindowDidChangeScreenNotification,
                                                  getCurrentWindow(),
                                                  [this] { updateScreen(); } };

    // Note: the NSViewComponentPeer also has a SharedResourcePointer<PerScreenDisplayLinks> to
    // avoid unnecessarily duplicating display-link threads.
    SharedResourcePointer<PerScreenDisplayLinks> sharedDisplayLinks;

    // On macOS, rather than letting swapBuffers block as appropriate, we use a display link
    // callback to mark the view as needing to repaint.
    std::optional<PerScreenDisplayLinks::Connection> connection;
   #endif

    enum StateFlags
    {
        pendingRender           = 1 << 0,
        paintComponents         = 1 << 1,
        pendingDestruction      = 1 << 2,
        initialised             = 1 << 3,

        // Flags that should retain their state after each frame
        persistent              = initialised | pendingDestruction
    };

    std::atomic<i32> state { 0 };
    ReferenceCountedArray<OpenGLContext::AsyncWorker, CriticalSection> workQueue;

   #if DRX_IOS
    iOSBackgroundProcessCheck backgroundProcessCheck;
   #endif

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImage)
};

//==============================================================================
class OpenGLContext::Attachment final : public ComponentMovementWatcher,
                                        private Timer
{
public:
    Attachment (OpenGLContext& c, Component& comp)
       : ComponentMovementWatcher (&comp), context (c)
    {
        if (canBeAttached (comp))
            attach();
    }

    ~Attachment() override
    {
        detach();
    }

    z0 detach()
    {
        auto& comp = *getComponent();
        stop();
        comp.setCachedComponentImage (nullptr);
        context.nativeContext = nullptr;
    }

    z0 componentMovedOrResized (b8 /*wasMoved*/, b8 /*wasResized*/) override
    {
        auto& comp = *getComponent();

        if (isAttached (comp) != canBeAttached (comp))
            componentVisibilityChanged();

        if (comp.getWidth() > 0 && comp.getHeight() > 0
             && context.nativeContext != nullptr)
        {
            if (auto* c = CachedImage::get (comp))
                c->handleResize();

            if (auto* peer = comp.getTopLevelComponent()->getPeer())
                context.nativeContext->updateWindowPosition (peer->getAreaCoveredBy (comp));
        }
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    z0 componentPeerChanged() override
    {
        detach();
        componentVisibilityChanged();
    }

    z0 componentVisibilityChanged() override
    {
        auto& comp = *getComponent();

        if (canBeAttached (comp))
        {
            if (isAttached (comp))
                comp.repaint(); // (needed when windows are un-minimised)
            else
                attach();
        }
        else
        {
            detach();
        }
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

   #if DRX_DEBUG || DRX_LOG_ASSERTIONS
    z0 componentBeingDeleted (Component& c) override
    {
        /* You must call detach() or delete your OpenGLContext to remove it
           from a component BEFORE deleting the component that it is using!
        */
        jassertfalse;

        ComponentMovementWatcher::componentBeingDeleted (c);
    }
   #endif

private:
    OpenGLContext& context;

    b8 canBeAttached (const Component& comp) noexcept
    {
        return (! context.overrideCanAttach) && comp.getWidth() > 0 && comp.getHeight() > 0 && isShowingOrMinimised (comp);
    }

    static b8 isShowingOrMinimised (const Component& c)
    {
        if (! c.isVisible())
            return false;

        if (auto* p = c.getParentComponent())
            return isShowingOrMinimised (*p);

        return c.getPeer() != nullptr;
    }

    static b8 isAttached (const Component& comp) noexcept
    {
        return comp.getCachedComponentImage() != nullptr;
    }

    z0 attach()
    {
        auto& comp = *getComponent();
        auto* newCachedImage = new CachedImage (context, comp,
                                                context.openGLPixelFormat,
                                                context.contextToShareWith);
        comp.setCachedComponentImage (newCachedImage);

        start();
    }

    z0 stop()
    {
        stopTimer();

        auto& comp = *getComponent();

       #if DRX_MAC
        #if ! DRX_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST (15, 0)
        // According to a warning triggered on macOS 15 and above this doesn't do anything!
        [[(NSView*) comp.getWindowHandle() window] disableScreenUpdatesUntilFlush];
        #endif
       #endif

        if (auto* oldCachedImage = CachedImage::get (comp))
            oldCachedImage->stop(); // (must stop this before detaching it from the component)
    }

    z0 start()
    {
        auto& comp = *getComponent();

        if (auto* cachedImage = CachedImage::get (comp))
        {
            cachedImage->start(); // (must wait until this is attached before starting its thread)
            cachedImage->updateViewportSize();

            startTimer (400);
        }
    }

    z0 timerCallback() override
    {
        if (auto* cachedImage = CachedImage::get (*getComponent()))
            cachedImage->checkViewportBounds();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Attachment)
};

//==============================================================================
OpenGLContext::OpenGLContext()
{
}

OpenGLContext::~OpenGLContext()
{
    detach();
}

z0 OpenGLContext::setRenderer (OpenGLRenderer* rendererToUse) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    renderer = rendererToUse;
}

z0 OpenGLContext::setComponentPaintingEnabled (b8 shouldPaintComponent) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    renderComponents = shouldPaintComponent;
}

z0 OpenGLContext::setContinuousRepainting (b8 shouldContinuouslyRepaint) noexcept
{
    continuousRepaint = shouldContinuouslyRepaint;

   #if DRX_MAC
    if (auto* component = getTargetComponent())
    {
        detach();
        attachment.reset (new Attachment (*this, *component));
    }

    if (auto* cachedImage = getCachedImage())
        cachedImage->refreshDisplayLinkConnection();
   #endif

    triggerRepaint();
}

z0 OpenGLContext::setPixelFormat (const OpenGLPixelFormat& preferredPixelFormat) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    openGLPixelFormat = preferredPixelFormat;
}

z0 OpenGLContext::setTextureMagnificationFilter (OpenGLContext::TextureMagnificationFilter magFilterMode) noexcept
{
    texMagFilter = magFilterMode;
}

z0 OpenGLContext::setNativeSharedContext (uk nativeContextToShareWith) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    contextToShareWith = nativeContextToShareWith;
}

z0 OpenGLContext::setMultisamplingEnabled (b8 b) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    useMultisampling = b;
}

z0 OpenGLContext::setOpenGLVersionRequired (OpenGLVersion v) noexcept
{
    versionRequired = v;
}

z0 OpenGLContext::attachTo (Component& component)
{
    component.repaint();

    if (getTargetComponent() != &component)
    {
        detach();
        attachment.reset (new Attachment (*this, component));
    }
}

z0 OpenGLContext::detach()
{
    if (auto* a = attachment.get())
    {
        a->detach(); // must detach before nulling our pointer
        attachment.reset();
    }

    nativeContext = nullptr;
}

b8 OpenGLContext::isAttached() const noexcept
{
    return nativeContext != nullptr;
}

Component* OpenGLContext::getTargetComponent() const noexcept
{
    return attachment != nullptr ? attachment->getComponent() : nullptr;
}

OpenGLContext* OpenGLContext::getContextAttachedTo (Component& c) noexcept
{
    if (auto* ci = CachedImage::get (c))
        return &(ci->context);

    return nullptr;
}

thread_local OpenGLContext* currentThreadActiveContext = nullptr;

OpenGLContext* OpenGLContext::getCurrentContext()
{
    return currentThreadActiveContext;
}

b8 OpenGLContext::makeActive() const noexcept
{
    auto& current = currentThreadActiveContext;

    if (nativeContext != nullptr && nativeContext->makeActive())
    {
        current = const_cast<OpenGLContext*> (this);
        return true;
    }

    current = nullptr;
    return false;
}

b8 OpenGLContext::isActive() const noexcept
{
    return nativeContext != nullptr && nativeContext->isActive();
}

z0 OpenGLContext::deactivateCurrentContext()
{
    NativeContext::deactivateCurrentContext();
    currentThreadActiveContext = nullptr;
}

z0 OpenGLContext::triggerRepaint()
{
    if (auto* cachedImage = getCachedImage())
        cachedImage->triggerRepaint();
}

z0 OpenGLContext::swapBuffers()
{
    if (nativeContext != nullptr)
        nativeContext->swapBuffers();
}

u32 OpenGLContext::getFrameBufferID() const noexcept
{
    return nativeContext != nullptr ? nativeContext->getFrameBufferID() : 0;
}

b8 OpenGLContext::setSwapInterval (i32 numFramesPerSwap)
{
    return nativeContext != nullptr && nativeContext->setSwapInterval (numFramesPerSwap);
}

i32 OpenGLContext::getSwapInterval() const
{
    return nativeContext != nullptr ? nativeContext->getSwapInterval() : 0;
}

uk OpenGLContext::getRawContext() const noexcept
{
    return nativeContext != nullptr ? nativeContext->getRawContext() : nullptr;
}

b8 OpenGLContext::isCoreProfile() const
{
    auto* c = getCachedImage();
    return c != nullptr && OpenGLRendering::TraitsVAO::isCoreProfile();
}

OpenGLContext::CachedImage* OpenGLContext::getCachedImage() const noexcept
{
    if (auto* comp = getTargetComponent())
        return CachedImage::get (*comp);

    return nullptr;
}

b8 OpenGLContext::areShadersAvailable() const
{
    auto* c = getCachedImage();
    return c != nullptr && c->shadersAvailable;
}

b8 OpenGLContext::isTextureNpotSupported() const
{
    auto* c = getCachedImage();
    return c != nullptr && c->textureNpotSupported;
}

ReferenceCountedObject* OpenGLContext::getAssociatedObject (tukk name) const
{
    jassert (name != nullptr);

    auto* c = getCachedImage();

    // This method must only be called from an openGL rendering callback.
    jassert (c != nullptr && nativeContext != nullptr);
    jassert (getCurrentContext() != nullptr);

    auto index = c->associatedObjectNames.indexOf (name);
    return index >= 0 ? c->associatedObjects.getUnchecked (index).get() : nullptr;
}

z0 OpenGLContext::setAssociatedObject (tukk name, ReferenceCountedObject* newObject)
{
    jassert (name != nullptr);

    if (auto* c = getCachedImage())
    {
        // This method must only be called from an openGL rendering callback.
        jassert (nativeContext != nullptr);
        jassert (getCurrentContext() != nullptr);

        i32k index = c->associatedObjectNames.indexOf (name);

        if (index >= 0)
        {
            if (newObject != nullptr)
            {
                c->associatedObjects.set (index, newObject);
            }
            else
            {
                c->associatedObjectNames.remove (index);
                c->associatedObjects.remove (index);
            }
        }
        else if (newObject != nullptr)
        {
            c->associatedObjectNames.add (name);
            c->associatedObjects.add (newObject);
        }
    }
}

z0 OpenGLContext::setImageCacheSize (size_t newSize) noexcept     { imageCacheMaxSize = newSize; }
size_t OpenGLContext::getImageCacheSize() const noexcept            { return imageCacheMaxSize; }

z0 OpenGLContext::execute (OpenGLContext::AsyncWorker::Ptr workerToUse, b8 shouldBlock)
{
    if (auto* c = getCachedImage())
        c->execute (std::move (workerToUse), shouldBlock);
    else
        jassertfalse; // You must have attached the context to a component
}

//==============================================================================
struct DepthTestDisabler
{
    DepthTestDisabler() noexcept
    {
        glGetBooleanv (GL_DEPTH_TEST, &wasEnabled);

        if (wasEnabled)
            glDisable (GL_DEPTH_TEST);
    }

    ~DepthTestDisabler() noexcept
    {
        if (wasEnabled)
            glEnable (GL_DEPTH_TEST);
    }

    GLboolean wasEnabled;
};

//==============================================================================
z0 OpenGLContext::copyTexture (const Rectangle<i32>& targetClipArea,
                                 const Rectangle<i32>& anchorPosAndTextureSize,
                                 i32k contextWidth, i32k contextHeight,
                                 b8 flippedVertically,
                                 b8 blend)
{
    if (contextWidth <= 0 || contextHeight <= 0)
        return;

    DRX_CHECK_OPENGL_ERROR
    if (blend)
    {
        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_BLEND);
    }
    else
    {
        glDisable (GL_BLEND);
    }

    DepthTestDisabler depthDisabler;

    if (areShadersAvailable())
    {
        OpenGLRendering::SavedBinding<OpenGLRendering::TraitsVAO> vaoBinding;

        struct OverlayShaderProgram final : public ReferenceCountedObject
        {
            explicit OverlayShaderProgram (OpenGLContext& context)
                : program (context), params (program)
            {}

            static const OverlayShaderProgram& select (OpenGLContext& context)
            {
                static const t8 programValueID[] = "juceGLComponentOverlayShader";
                OverlayShaderProgram* program = static_cast<OverlayShaderProgram*> (context.getAssociatedObject (programValueID));

                if (program == nullptr)
                {
                    program = new OverlayShaderProgram (context);
                    context.setAssociatedObject (programValueID, program);
                }

                program->program.use();
                return *program;
            }

            struct BuiltProgram final : public OpenGLShaderProgram
            {
                explicit BuiltProgram (OpenGLContext& ctx)
                    : OpenGLShaderProgram (ctx)
                {
                    addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (
                        "attribute " DRX_HIGHP " vec2 position;"
                        "uniform " DRX_HIGHP " vec2 screenSize;"
                        "uniform " DRX_HIGHP " f32 textureBounds[4];"
                        "uniform " DRX_HIGHP " vec2 vOffsetAndScale;"
                        "varying " DRX_HIGHP " vec2 texturePos;"
                        "z0 main()"
                        "{"
                          DRX_HIGHP " vec2 scaled = position / (0.5 * screenSize.xy);"
                          "gl_Position = vec4 (scaled.x - 1.0, 1.0 - scaled.y, 0, 1.0);"
                          "texturePos = (position - vec2 (textureBounds[0], textureBounds[1])) / vec2 (textureBounds[2], textureBounds[3]);"
                          "texturePos = vec2 (texturePos.x, vOffsetAndScale.x + vOffsetAndScale.y * texturePos.y);"
                        "}"));

                    addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (
                        "uniform sampler2D imageTexture;"
                        "varying " DRX_HIGHP " vec2 texturePos;"
                        "z0 main()"
                        "{"
                          "gl_FragColor = texture2D (imageTexture, texturePos);"
                        "}"));

                    link();
                }
            };

            struct Params
            {
                explicit Params (OpenGLShaderProgram& prog)
                    : positionAttribute (prog, "position"),
                      screenSize (prog, "screenSize"),
                      imageTexture (prog, "imageTexture"),
                      textureBounds (prog, "textureBounds"),
                      vOffsetAndScale (prog, "vOffsetAndScale")
                {}

                z0 set (const f32 targetWidth, const f32 targetHeight, const Rectangle<f32>& bounds, b8 flipVertically) const
                {
                    const GLfloat m[] = { bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() };
                    textureBounds.set (m, 4);
                    imageTexture.set (0);
                    screenSize.set (targetWidth, targetHeight);

                    vOffsetAndScale.set (flipVertically ? 0.0f : 1.0f,
                                         flipVertically ? 1.0f : -1.0f);
                }

                OpenGLShaderProgram::Attribute positionAttribute;
                OpenGLShaderProgram::Uniform screenSize, imageTexture, textureBounds, vOffsetAndScale;
            };

            BuiltProgram program;
            Params params;
        };

        auto left   = (GLshort) targetClipArea.getX();
        auto top    = (GLshort) targetClipArea.getY();
        auto right  = (GLshort) targetClipArea.getRight();
        auto bottom = (GLshort) targetClipArea.getBottom();
        const GLshort vertices[] = { left, bottom, right, bottom, left, top, right, top };

        GLint oldProgram{};
        glGetIntegerv (GL_CURRENT_PROGRAM, &oldProgram);

        const ScopeGuard bindPreviousProgram { [&] { extensions.glUseProgram ((GLuint) oldProgram); } };

        auto& program = OverlayShaderProgram::select (*this);
        program.params.set ((f32) contextWidth, (f32) contextHeight, anchorPosAndTextureSize.toFloat(), flippedVertically);

        OpenGLRendering::SavedBinding<OpenGLRendering::TraitsArrayBuffer> savedArrayBuffer;
        extensions.glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

        auto index = (GLuint) program.params.positionAttribute.attributeID;
        extensions.glVertexAttribPointer (index, 2, GL_SHORT, GL_FALSE, 4, nullptr);
        extensions.glEnableVertexAttribArray (index);
        DRX_CHECK_OPENGL_ERROR

        if (extensions.glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        {
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
            extensions.glDisableVertexAttribArray (index);
        }
        else
        {
            clearGLError();
        }
    }
    else
    {
        jassert (attachment == nullptr); // Running on an old graphics card!
    }

    DRX_CHECK_OPENGL_ERROR
}

#if DRX_ANDROID

z0 OpenGLContext::NativeContext::surfaceCreated (LocalRef<jobject>)
{
    {
        const std::lock_guard lock { nativeHandleMutex };

        jassert (hasInitialised);

        // has the context already attached?
        jassert (surface.get() == EGL_NO_SURFACE && context.get() == EGL_NO_CONTEXT);

        const auto window = getNativeWindow();

        if (window == nullptr)
        {
            // failed to get a pointer to the native window so bail out
            jassertfalse;
            return;
        }

        // create the surface
        surface.reset (eglCreateWindowSurface (display, config, window.get(), nullptr));
        jassert (surface.get() != EGL_NO_SURFACE);

        // create the OpenGL context
        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        context.reset (eglCreateContext (display, config, EGL_NO_CONTEXT, contextAttribs));
        jassert (context.get() != EGL_NO_CONTEXT);
    }

    if (auto* cached = CachedImage::get (component))
    {
        cached->resume();
        cached->triggerRepaint();
    }
}

z0 OpenGLContext::NativeContext::surfaceDestroyed (LocalRef<jobject>)
{
    if (auto* cached = CachedImage::get (component))
        cached->pause();

    {
        const std::lock_guard lock { nativeHandleMutex };

        context.reset (EGL_NO_CONTEXT);
        surface.reset (EGL_NO_SURFACE);
    }
}

#endif

} // namespace drx
