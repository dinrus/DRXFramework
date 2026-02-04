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

class OpenGLTexture;

//==============================================================================
/**
    Creates an OpenGL context, which can be attached to a component.

    To render some OpenGL, you should create an instance of an OpenGLContext,
    and call attachTo() to make it use a component as its render target.

    To provide threaded rendering, you can supply an OpenGLRenderer object that
    will be used to render each frame.

    Before your target component or OpenGLRenderer is deleted, you MUST call
    detach() or delete the OpenGLContext to allow the background thread to
    stop and the native resources to be freed safely.

    @see OpenGLRenderer

    @tags{OpenGL}
*/
class DRX_API  OpenGLContext
{
public:
    OpenGLContext();

    /** Destructor. */
    ~OpenGLContext();

    //==============================================================================
    /** Gives the context an OpenGLRenderer to use to do the drawing.
        The object that you give it will not be owned by the context, so it's the caller's
        responsibility to manage its lifetime and make sure that it doesn't get deleted
        while the context may be using it. To stop the context using a renderer, just call
        this method with a null pointer.
        Note: This must be called BEFORE attaching your context to a target component!
    */
    z0 setRenderer (OpenGLRenderer*) noexcept;

    /** Attaches the context to a target component.

        If the component is not fully visible, this call will wait until the component
        is shown before actually creating a native context for it.

        When a native context is created, a thread is started, and will be used to call
        the OpenGLRenderer methods. The context will be floated above the target component,
        and when the target moves, it will track it. If the component is hidden/shown, the
        context may be deleted and re-created.
    */
    z0 attachTo (Component&);

    /** Detaches the context from its target component and deletes any native resources.
        If the context has not been attached, this will do nothing. Otherwise, it will block
        until the context and its thread have been cleaned up.
    */
    z0 detach();

    /** Возвращает true, если the context is attached to a component and is on-screen.
        Note that if you call attachTo() for a non-visible component, this method will
        return false until the component is made visible.
    */
    b8 isAttached() const noexcept;

    /** Returns the component to which this context is currently attached, or nullptr. */
    Component* getTargetComponent() const noexcept;

    /** If the given component has an OpenGLContext attached, then this will return it. */
    static OpenGLContext* getContextAttachedTo (Component& component) noexcept;

    //==============================================================================
    /** Sets the pixel format which you'd like to use for the target GL surface.
        Note: This must be called BEFORE attaching your context to a target component!
    */
    z0 setPixelFormat (const OpenGLPixelFormat& preferredPixelFormat) noexcept;

    /** Texture magnification filters, used by setTextureMagnificationFilter(). */
    enum TextureMagnificationFilter
    {
        nearest,
        linear
    };

    /** Sets the texture magnification filter. By default the texture magnification
        filter is linear. However, for faster rendering you may want to use the
        'nearest' magnification filter. This option will not affect any textures
        created before this function was called. */
    z0 setTextureMagnificationFilter (TextureMagnificationFilter magFilterMode) noexcept;

     //==============================================================================
    /** Provides a context with which you'd like this context's resources to be shared.
        The object passed-in here is a platform-dependent native context object, and
        must not be deleted while this context may still be using it! To turn off sharing,
        you can call this method with a null pointer.
        Note: This must be called BEFORE attaching your context to a target component!
    */
    z0 setNativeSharedContext (uk nativeContextToShareWith) noexcept;

    /** Enables multisampling on platforms where this is implemented.
        If enabling this, you must call this method before attachTo().
    */
    z0 setMultisamplingEnabled (b8) noexcept;

    /** Возвращает true, если shaders can be used in this context. */
    b8 areShadersAvailable() const;

    /** Возвращает true, если non-power-of-two textures are supported in this context. */
    b8 isTextureNpotSupported() const;

    /** OpenGL versions, used by setOpenGLVersionRequired().

        The Core profile doesn't include some legacy functionality, including the
        fixed-function pipeline.

        The Compatibility profile is backwards-compatible, and includes functionality
        deprecated in the Core profile. However, not all implementations provide
        compatibility profiles targeting later versions of OpenGL. To run on the
        broadest range of hardware, using the 3.2 Core profile is recommended.
    */
    enum OpenGLVersion
    {
        defaultGLVersion = 0, ///< Whatever the device decides to give us, normally a compatibility profile
        openGL3_2,            ///< 3.2 Core profile
        openGL4_1,            ///< 4.1 Core profile, the latest supported by macOS at time of writing
        openGL4_3             ///< 4.3 Core profile, will enable improved debugging support when building in Debug
    };

    /** Sets a preference for the version of GL that this context should use, if possible.
        Some platforms may ignore this value.
    */
    z0 setOpenGLVersionRequired (OpenGLVersion) noexcept;

    /** Enables or disables the use of the GL context to perform 2D rendering
        of the component to which it is attached.
        If this is false, then only your OpenGLRenderer will be used to perform
        any rendering. If true, then each time your target's paint() method needs
        to be called, an OpenGLGraphicsContext will be used to render it, (after
        calling your OpenGLRenderer if there is one).

        By default this is set to true. If you're not using any paint() method functionality
        and are doing all your rendering in an OpenGLRenderer, you should disable it
        to improve performance.

        Note: This must be called BEFORE attaching your context to a target component!
    */
    z0 setComponentPaintingEnabled (b8 shouldPaintComponent) noexcept;

    /** Enables or disables continuous repainting.
        If set to true, the context will run a loop, re-rendering itself without waiting
        for triggerRepaint() to be called, at a frequency determined by the swap interval
        (see setSwapInterval). If false, then after each render callback, it will wait for
        another call to triggerRepaint() before rendering again.
        This is disabled by default.
        @see setSwapInterval
    */
    z0 setContinuousRepainting (b8 shouldContinuouslyRepaint) noexcept;

    /** Asynchronously causes a repaint to be made. */
    z0 triggerRepaint();

    //==============================================================================
    /** This retrieves an object that was previously stored with setAssociatedObject().
        If no object is found with the given name, this will return nullptr.
        This method must only be called from within the GL rendering methods.
        @see setAssociatedObject
    */
    ReferenceCountedObject* getAssociatedObject (tukk name) const;

    /** Attaches a named object to the context, which will be deleted when the context is
        destroyed.

        This allows you to store an object which will be released before the context is
        deleted. The main purpose is for caching GL objects such as shader programs, which
        will become invalid when the context is deleted.

        This method must only be called from within the GL rendering methods.
    */
    z0 setAssociatedObject (tukk name, ReferenceCountedObject* newObject);

    //==============================================================================
    /** Makes this context the currently active one.
        You should never need to call this in normal use - the context will already be
        active when OpenGLRenderer::renderOpenGL() is invoked.
    */
    b8 makeActive() const noexcept;

    /** Возвращает true, если this context is currently active for the calling thread. */
    b8 isActive() const noexcept;

    /** If any context is active on the current thread, this deactivates it.
        Note that on some platforms, like Android, this isn't possible.
    */
    static z0 deactivateCurrentContext();

    /** Returns the context that's currently in active use by the calling thread, or
        nullptr if no context is active.
    */
    static OpenGLContext* getCurrentContext();

    //==============================================================================
    /** Swaps the buffers (if the context can do this).
        There's normally no need to call this directly - the buffers will be swapped
        automatically after your OpenGLRenderer::renderOpenGL() method has been called.
    */
    z0 swapBuffers();

    /** Sets whether the context checks the vertical sync before swapping.

        The value is the number of frames to allow between buffer-swapping. This is
        fairly system-dependent, but 0 turns off syncing, 1 makes it swap on frame-boundaries,
        and greater numbers indicate that it should swap less often.

        By default, this will be set to 1.

        Возвращает true, если it sets the value successfully - some platforms won't support
        this setting.

        @see setContinuousRepainting
    */
    b8 setSwapInterval (i32 numFramesPerSwap);

    /** Returns the current swap-sync interval.
        See setSwapInterval() for info about the value returned.
    */
    i32 getSwapInterval() const;

    //==============================================================================
    /** Execute a lambda, function or functor on the OpenGL thread with an active
        context.

        This method will attempt to execute functor on the OpenGL thread. If
        blockUntilFinished is true then the method will block until the functor
        has finished executing.

        This function can only be called if the context is attached to a component.
        Otherwise, this function will assert.

        This function is useful when you need to execute house-keeping tasks such
        as allocating, deallocating textures or framebuffers. As such, the functor
        will execute without locking the message thread. Therefore, it is not
        intended for any drawing commands or GUI code. Any GUI code should be
        executed in the OpenGLRenderer::renderOpenGL callback instead.
    */
    template <typename T>
    z0 executeOnGLThread (T&& functor, b8 blockUntilFinished);

    //==============================================================================
    /** Returns a scale factor that relates the context component's size to the number
        of physical pixels it covers on the screen.

        In special cases it will be the same as Displays::Display::scale, but it also
        includes AffineTransforms that affect the rendered area, and will be correctly
        reported not just in standalone applications but plugins as well.

        Note that this should only be called during an OpenGLRenderer::renderOpenGL()
        callback - at other times the value it returns is undefined.
    */
    f64 getRenderingScale() const noexcept   { return currentRenderScale; }

    //==============================================================================
    /** If this context is backed by a frame buffer, this returns its ID number,
        or 0 if the context does not use a framebuffer.
    */
    u32 getFrameBufferID() const noexcept;

    /** Returns an OS-dependent handle to some kind of underlying OS-provided GL context.

        The exact type of the value returned will depend on the OS and may change
        if the implementation changes. If you want to use this, digging around in the
        native code is probably the best way to find out what it is.
    */
    uk getRawContext() const noexcept;

    /** Возвращает true, если this context is using the core profile.

        @see OpenGLVersion
    */
    b8 isCoreProfile() const;

    /** This structure holds a set of dynamically loaded GL functions for use on this context. */
    OpenGLExtensionFunctions extensions;

    //==============================================================================
    /** Draws the currently selected texture into this context at its original size.

        @param targetClipArea  the target area to draw into (in top-left origin coords)
        @param anchorPosAndTextureSize  the position of this rectangle is the texture's top-left
                                        anchor position in the target space, and the size must be
                                        the total size of the texture.
        @param contextWidth     the width of the context or framebuffer that is being drawn into,
                                used for scaling of the coordinates.
        @param contextHeight    the height of the context or framebuffer that is being drawn into,
                                used for vertical flipping of the y coordinates.
        @param textureOriginIsBottomLeft    if true, the texture's origin is treated as being at
                                (0, 0). If false, it is assumed to be (0, 1)
    */
    z0 copyTexture (const Rectangle<i32>& targetClipArea,
                      const Rectangle<i32>& anchorPosAndTextureSize,
                      i32 contextWidth, i32 contextHeight,
                      b8 textureOriginIsBottomLeft,
                      b8 blend = true);

    /** Changes the amount of GPU memory that the internal cache for Images is allowed to use. */
    z0 setImageCacheSize (size_t cacheSizeBytes) noexcept;

    /** Returns the amount of GPU memory that the internal cache for Images is allowed to use. */
    size_t getImageCacheSize() const noexcept;

    //==============================================================================
   #ifndef DOXYGEN
    class NativeContext;
   #endif

private:
    enum class InitResult
    {
        fatal,
        retry,
        success
    };

    friend class OpenGLTexture;

    class CachedImage;
    class Attachment;
    NativeContext* nativeContext = nullptr;
    OpenGLRenderer* renderer = nullptr;
    f64 currentRenderScale = 1.0;
    std::unique_ptr<Attachment> attachment;
    OpenGLPixelFormat openGLPixelFormat;
    uk contextToShareWith = nullptr;
    OpenGLVersion versionRequired = defaultGLVersion;
    size_t imageCacheMaxSize = 8 * 1024 * 1024;
    b8 renderComponents = true, useMultisampling = false, overrideCanAttach = false;
    std::atomic<b8> continuousRepaint { false };
    TextureMagnificationFilter texMagFilter = linear;

    //==============================================================================
    struct AsyncWorker  : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<AsyncWorker>;
        virtual z0 operator() (OpenGLContext&) = 0;
        ~AsyncWorker() override = default;
    };

    template <typename FunctionType>
    struct AsyncWorkerFunctor  : public AsyncWorker
    {
        AsyncWorkerFunctor (FunctionType functorToUse) : functor (functorToUse) {}
        z0 operator() (OpenGLContext& callerContext) override     { functor (callerContext); }
        FunctionType functor;

        DRX_DECLARE_NON_COPYABLE (AsyncWorkerFunctor)
    };

    //==============================================================================
    CachedImage* getCachedImage() const noexcept;
    z0 execute (AsyncWorker::Ptr, b8);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLContext)
};

//==============================================================================
#ifndef DOXYGEN
template <typename FunctionType>
z0 OpenGLContext::executeOnGLThread (FunctionType&& f, b8 shouldBlock) { execute (new AsyncWorkerFunctor<FunctionType> (f), shouldBlock); }
#endif

} // namespace drx
