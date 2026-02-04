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

namespace VideoRenderers
{
    //==============================================================================
    struct Base
    {
        virtual ~Base() = default;

        virtual HRESULT create (ComSmartPtr<IGraphBuilder>&, ComSmartPtr<IBaseFilter>&, HWND) = 0;
        virtual z0 setVideoWindow (HWND) = 0;
        virtual z0 setVideoPosition (HWND) = 0;
        virtual z0 repaintVideo (HWND, HDC) = 0;
        virtual z0 displayModeChanged() = 0;
        virtual HRESULT getVideoSize (i64& videoWidth, i64& videoHeight) = 0;
    };

    //==============================================================================
    struct VMR7  : public Base
    {
        VMR7() {}

        HRESULT create (ComSmartPtr<IGraphBuilder>& graphBuilder,
                        ComSmartPtr<IBaseFilter>& baseFilter, HWND hwnd) override
        {
            ComSmartPtr<IVMRFilterConfig> filterConfig;

            HRESULT hr = baseFilter.CoCreateInstance (CLSID_VideoMixingRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"VMR-7");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (filterConfig);
            if (SUCCEEDED (hr))   hr = filterConfig->SetRenderingMode (VMRMode_Windowless);
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (windowlessControl);
            if (SUCCEEDED (hr))   hr = windowlessControl->SetVideoClippingWindow (hwnd);
            if (SUCCEEDED (hr))   hr = windowlessControl->SetAspectRatioMode (VMR_ARMODE_LETTER_BOX);

            return hr;
        }

        z0 setVideoWindow (HWND hwnd) override
        {
            windowlessControl->SetVideoClippingWindow (hwnd);
        }

        z0 setVideoPosition (HWND hwnd) override
        {
            i64 videoWidth = 0, videoHeight = 0;
            windowlessControl->GetNativeVideoSize (&videoWidth, &videoHeight, nullptr, nullptr);

            RECT src, dest;
            SetRect (&src, 0, 0, videoWidth, videoHeight);
            GetClientRect (hwnd, &dest);

            windowlessControl->SetVideoPosition (&src, &dest);
        }

        z0 repaintVideo (HWND hwnd, HDC hdc) override
        {
            windowlessControl->RepaintVideo (hwnd, hdc);
        }

        z0 displayModeChanged() override
        {
            windowlessControl->DisplayModeChanged();
        }

        HRESULT getVideoSize (i64& videoWidth, i64& videoHeight) override
        {
            return windowlessControl->GetNativeVideoSize (&videoWidth, &videoHeight, nullptr, nullptr);
        }

        ComSmartPtr<IVMRWindowlessControl> windowlessControl;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VMR7)
    };


    //==============================================================================
    struct EVR  : public Base
    {
        EVR() = default;

        HRESULT create (ComSmartPtr<IGraphBuilder>& graphBuilder,
                        ComSmartPtr<IBaseFilter>& baseFilter, HWND hwnd) override
        {
            ComSmartPtr<IMFGetService> getService;

            HRESULT hr = baseFilter.CoCreateInstance (CLSID_EnhancedVideoRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"EVR");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (getService);
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            if (SUCCEEDED (hr))   hr = getService->GetService (MR_VIDEO_RENDER_SERVICE, __uuidof (IMFVideoDisplayControl),
                                                               (uk*) videoDisplayControl.resetAndGetPointerAddress());
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetVideoWindow (hwnd);
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetAspectRatioMode (MFVideoARMode_PreservePicture);

            return hr;
        }

        z0 setVideoWindow (HWND hwnd) override
        {
            videoDisplayControl->SetVideoWindow (hwnd);
        }

        z0 setVideoPosition (HWND hwnd) override
        {
            const MFVideoNormalizedRect src { 0.0f, 0.0f, 1.0f, 1.0f };

            RECT dest;
            GetClientRect (hwnd, &dest);

            videoDisplayControl->SetVideoPosition (&src, &dest);
        }

        z0 repaintVideo (HWND, HDC) override
        {
            videoDisplayControl->RepaintVideo();
        }

        z0 displayModeChanged() override {}

        HRESULT getVideoSize (i64& videoWidth, i64& videoHeight) override
        {
            SIZE sz = { 0, 0 };
            HRESULT hr = videoDisplayControl->GetNativeVideoSize (&sz, nullptr);
            videoWidth  = sz.cx;
            videoHeight = sz.cy;
            return hr;
        }

        ComSmartPtr<IMFVideoDisplayControl> videoDisplayControl;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EVR)
    };
}

//==============================================================================
struct VideoComponent::Pimpl  : public Component,
                                private ComponentPeer::ScaleFactorListener
{
    Pimpl (VideoComponent& ownerToUse, b8)
        : owner (ownerToUse)
    {
        setOpaque (true);
        context.reset (new DirectShowContext (*this));
        componentWatcher.reset (new ComponentWatcher (*this));
    }

    ~Pimpl() override
    {
        close();
        context = nullptr;
        componentWatcher = nullptr;

        if (currentPeer != nullptr)
            currentPeer->removeScaleFactorListener (this);
    }

    Result loadFromString (const Txt& fileOrURLPath)
    {
        close();
        auto r = context->loadFile (fileOrURLPath);

        if (r.wasOk())
        {
            videoLoaded = true;
            context->updateVideoPosition();
        }

        return r;
    }

    Result load (const File& file)
    {
        auto r = loadFromString (file.getFullPathName());

        if (r.wasOk())
            currentFile = file;

        return r;
    }

    Result load (const URL& url)
    {
        auto r = loadFromString (URL::removeEscapeChars (url.toString (true)));

        if (r.wasOk())
            currentURL = url;

        return r;
    }

    z0 close()
    {
        stop();
        context->release();

        videoLoaded = false;
        currentFile = File();
        currentURL = {};
    }

    b8 isOpen() const
    {
        return videoLoaded;
    }

    b8 isPlaying() const
    {
        return context->state == DirectShowContext::runningState;
    }

    z0 play()
    {
        if (videoLoaded)
            context->play();
    }

    z0 stop()
    {
        if (videoLoaded)
            context->pause();
    }

    z0 setPosition (f64 newPosition)
    {
        if (videoLoaded)
            context->setPosition (newPosition);
    }

    f64 getPosition() const
    {
        return videoLoaded ? context->getPosition() : 0.0;
    }

    z0 setSpeed (f64 newSpeed)
    {
        if (videoLoaded)
            context->setSpeed (newSpeed);
    }

    f64 getSpeed() const
    {
        return videoLoaded ? context->getSpeed() : 0.0;
    }

    Rectangle<i32> getNativeSize() const
    {
        return videoLoaded ? context->getVideoSize()
                           : Rectangle<i32>();
    }

    f64 getDuration() const
    {
        return videoLoaded ? context->getDuration() : 0.0;
    }

    z0 setVolume (f32 newVolume)
    {
        if (videoLoaded)
            context->setVolume (newVolume);
    }

    f32 getVolume() const
    {
        return videoLoaded ? context->getVolume() : 0.0f;
    }

    z0 paint (Graphics& g) override
    {
        if (videoLoaded)
            context->handleUpdateNowIfNeeded();
        else
            g.fillAll (Colors::grey);
    }

    z0 updateContextPosition()
    {
        context->updateContextPosition();

        if (getWidth() > 0 && getHeight() > 0)
            if (auto* peer = getTopLevelComponent()->getPeer())
                context->updateWindowPosition ((peer->getAreaCoveredBy (*this).toDouble()
                                                * peer->getPlatformScaleFactor()).toNearestInt());
    }

    z0 updateContextVisibility()
    {
        context->showWindow (isShowing());
    }

    z0 recreateNativeWindowAsync()
    {
        context->recreateNativeWindowAsync();
        repaint();
    }

    z0 playbackStarted()
    {
        NullCheckedInvocation::invoke (owner.onPlaybackStarted);
    }

    z0 playbackStopped()
    {
        NullCheckedInvocation::invoke (owner.onPlaybackStopped);
    }

    z0 errorOccurred (const Txt& errorMessage)
    {
        NullCheckedInvocation::invoke (owner.onErrorOccurred, errorMessage);
    }

    File currentFile;
    URL currentURL;

private:
    VideoComponent& owner;
    ComponentPeer* currentPeer = nullptr;
    b8 videoLoaded = false;

    //==============================================================================
    z0 nativeScaleFactorChanged (f64 /*newScaleFactor*/) override
    {
        if (videoLoaded)
            updateContextPosition();
    }

    //==============================================================================
    struct ComponentWatcher   : public ComponentMovementWatcher
    {
        ComponentWatcher (Pimpl& c)  : ComponentMovementWatcher (&c), owner (c)
        {
        }

        using ComponentMovementWatcher::componentMovedOrResized;
        z0 componentMovedOrResized (b8, b8) override
        {
            if (owner.videoLoaded)
                owner.updateContextPosition();
        }

        z0 componentPeerChanged() override
        {
            if (owner.currentPeer != nullptr)
                owner.currentPeer->removeScaleFactorListener (&owner);

            if (owner.videoLoaded)
                owner.recreateNativeWindowAsync();
        }

        using ComponentMovementWatcher::componentVisibilityChanged;
        z0 componentVisibilityChanged() override
        {
            if (owner.videoLoaded)
                owner.updateContextVisibility();
        }

        Pimpl& owner;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentWatcher)
    };

    std::unique_ptr<ComponentWatcher> componentWatcher;

    //==============================================================================
    struct DirectShowContext    : public AsyncUpdater
    {
        DirectShowContext (Pimpl& c)  : component (c)
        {
            [[maybe_unused]] const auto result = CoInitialize (nullptr);
        }

        ~DirectShowContext() override
        {
            release();
            CoUninitialize();
        }

        //==============================================================================
        z0 updateWindowPosition (const Rectangle<i32>& newBounds)
        {
            nativeWindow->setWindowPosition (newBounds);
        }

        z0 showWindow (b8 shouldBeVisible)
        {
            nativeWindow->showWindow (shouldBeVisible);
        }

        //==============================================================================
        z0 repaint()
        {
            if (hasVideo)
                videoRenderer->repaintVideo (nativeWindow->hwnd, nativeWindow->hdc);
        }

        z0 updateVideoPosition()
        {
            if (hasVideo)
                videoRenderer->setVideoPosition (nativeWindow->hwnd);
        }

        z0 displayResolutionChanged()
        {
            if (hasVideo)
                videoRenderer->displayModeChanged();
        }

        //==============================================================================
        z0 peerChanged()
        {
            deleteNativeWindow();

            mediaEvent->SetNotifyWindow (0, 0, 0);

            if (videoRenderer != nullptr)
                videoRenderer->setVideoWindow (nullptr);

            createNativeWindow();

            mediaEvent->CancelDefaultHandling (EC_STATE_CHANGE);
            mediaEvent->SetNotifyWindow ((OAHWND) hwnd, graphEventID, 0);

            if (videoRenderer != nullptr)
                videoRenderer->setVideoWindow (hwnd);
        }

        z0 handleAsyncUpdate() override
        {
            if (hwnd != nullptr)
            {
                if (needToRecreateNativeWindow)
                {
                    peerChanged();
                    needToRecreateNativeWindow = false;
                }

                if (needToUpdateViewport)
                {
                    updateVideoPosition();
                    needToUpdateViewport = false;
                }

                repaint();
            }
            else
            {
                triggerAsyncUpdate();
            }
        }

        z0 recreateNativeWindowAsync()
        {
            needToRecreateNativeWindow = true;
            triggerAsyncUpdate();
        }

        z0 updateContextPosition()
        {
            needToUpdateViewport = true;
            triggerAsyncUpdate();
        }

        //==============================================================================
        Result loadFile (const Txt& fileOrURLPath)
        {
            jassert (state == uninitializedState);

            if (! createNativeWindow())
                return Result::fail ("Can't create window");

            HRESULT hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph);

            // basic playback interfaces
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaControl);
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaPosition);
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaEvent);
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (basicAudio);

            // video renderer interface
            if (SUCCEEDED (hr))
            {
                if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
                {
                    videoRenderer.reset (new VideoRenderers::EVR());
                    hr = videoRenderer->create (graphBuilder, baseFilter, hwnd);

                    if (FAILED (hr))
                        videoRenderer = nullptr;
                }

                if (videoRenderer == nullptr)
                {
                    videoRenderer.reset (new VideoRenderers::VMR7());
                    hr = videoRenderer->create (graphBuilder, baseFilter, hwnd);
                }
            }

            // build filter graph
            if (SUCCEEDED (hr))
            {
                hr = graphBuilder->RenderFile (fileOrURLPath.toWideCharPointer(), nullptr);

                if (FAILED (hr))
                {
                   #if DRX_MODAL_LOOPS_PERMITTED
                    // Annoyingly, if we don't run the msg loop between failing and deleting the window, the
                    // whole OS message-dispatch system gets itself into a state, and refuses to deliver any
                    // more messages for the whole app. (That's what happens in Win7, anyway)
                    MessageManager::getInstance()->runDispatchLoopUntil (200);
                   #endif
                }
            }

            // remove video renderer if not connected (no video)
            if (SUCCEEDED (hr))
            {
                if (isRendererConnected())
                {
                    hasVideo = true;
                }
                else
                {
                    hasVideo = false;
                    graphBuilder->RemoveFilter (baseFilter);
                    videoRenderer = nullptr;
                    baseFilter = nullptr;
                }
            }

            // set window to receive events
            if (SUCCEEDED (hr))
            {
                mediaEvent->CancelDefaultHandling (EC_STATE_CHANGE);
                hr = mediaEvent->SetNotifyWindow ((OAHWND) hwnd, graphEventID, 0);
            }

            if (SUCCEEDED (hr))
            {
                state = stoppedState;
                pause();
                return Result::ok();
            }

            // Note that if you're trying to open a file and this method fails, you may
            // just need to install a suitable codec. It seems that by default DirectShow
            // doesn't support a very good range of formats.
            release();
            return getErrorMessageFromResult (hr);
        }

        static Result getErrorMessageFromResult (HRESULT hr)
        {
            switch (hr)
            {
                case VFW_E_INVALID_FILE_FORMAT:         return Result::fail ("Invalid file format");
                case VFW_E_NOT_FOUND:                   return Result::fail ("File not found");
                case VFW_E_UNKNOWN_FILE_TYPE:           return Result::fail ("Unknown file type");
                case VFW_E_UNSUPPORTED_STREAM:          return Result::fail ("Unsupported stream");
                case VFW_E_CANNOT_CONNECT:              return Result::fail ("Cannot connect");
                case VFW_E_CANNOT_LOAD_SOURCE_FILTER:   return Result::fail ("Cannot load source filter");
            }

            TCHAR messageBuffer[512] = { 0 };

            FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr, (DWORD) hr, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                           messageBuffer, (DWORD) numElementsInArray (messageBuffer) - 1, nullptr);

            return Result::fail (Txt (messageBuffer));
        }

        z0 release()
        {
            if (mediaControl != nullptr)
                mediaControl->Stop();

            if (mediaEvent != nullptr)
                mediaEvent->SetNotifyWindow (0, 0, 0);

            if (videoRenderer != nullptr)
                videoRenderer->setVideoWindow (nullptr);

            hasVideo = false;
            videoRenderer = nullptr;
            baseFilter = nullptr;
            basicAudio = nullptr;
            mediaEvent = nullptr;
            mediaPosition = nullptr;
            mediaControl = nullptr;
            graphBuilder = nullptr;

            state = uninitializedState;

            if (nativeWindow != nullptr)
                deleteNativeWindow();
        }

        z0 graphEventProc()
        {
            LONG ec = 0;
            LONG_PTR p1 = {}, p2 = {};

            jassert (mediaEvent != nullptr);

            while (SUCCEEDED (mediaEvent->GetEvent (&ec, &p1, &p2, 0)))
            {
                mediaEvent->FreeEventParams (ec, p1, p2);

                switch (ec)
                {
                    case EC_REPAINT:
                        component.repaint();
                        break;

                    case EC_COMPLETE:
                        component.stop();
                        component.setPosition (0.0);
                        break;

                    case EC_ERRORABORT:
                    case EC_ERRORABORTEX:
                        component.errorOccurred (getErrorMessageFromResult ((HRESULT) p1).getErrorMessage());
                        // intentional fallthrough
                    case EC_USERABORT:
                        component.close();
                        break;

                    case EC_STATE_CHANGE:
                        switch (p1)
                        {
                            case State_Paused:  component.playbackStopped(); break;
                            case State_Running: component.playbackStarted(); break;
                            default: break;
                        }

                    default:
                        break;
                }
            }
        }

        //==============================================================================
        z0 play()
        {
            mediaControl->Run();
            state = runningState;
        }

        z0 stop()
        {
            mediaControl->Stop();
            state = stoppedState;
        }

        z0 pause()
        {
            mediaControl->Pause();
            state = pausedState;
        }

        //==============================================================================
        Rectangle<i32> getVideoSize() const noexcept
        {
            i64 width = 0, height = 0;

            if (hasVideo)
                videoRenderer->getVideoSize (width, height);

            return { (i32) width, (i32) height };
        }

        //==============================================================================
        f64 getDuration() const
        {
            REFTIME duration;
            mediaPosition->get_Duration (&duration);
            return duration;
        }

        f64 getSpeed() const
        {
            f64 speed;
            mediaPosition->get_Rate (&speed);
            return speed;
        }

        f64 getPosition() const
        {
            REFTIME seconds;
            mediaPosition->get_CurrentPosition (&seconds);
            return seconds;
        }

        z0 setSpeed (f64 newSpeed)     { mediaPosition->put_Rate (newSpeed); }
        z0 setPosition (f64 seconds)   { mediaPosition->put_CurrentPosition (seconds); }
        z0 setVolume (f32 newVolume)    { basicAudio->put_Volume (convertToDShowVolume (newVolume)); }

        // in DirectShow, full volume is 0, silence is -10000
        static i64 convertToDShowVolume (f32 vol) noexcept
        {
            if (vol >= 1.0f) return 0;
            if (vol <= 0.0f) return -10000;

            return roundToInt ((vol * 10000.0f) - 10000.0f);
        }

        f32 getVolume() const
        {
            i64 volume;
            basicAudio->get_Volume (&volume);
            return (f32) (volume + 10000) / 10000.0f;
        }

        enum State { uninitializedState, runningState, pausedState, stoppedState };
        State state = uninitializedState;

    private:
        //==============================================================================
        enum { graphEventID = WM_APP + 0x43f0 };

        Pimpl& component;
        HWND hwnd = {};
        HDC hdc = {};

        ComSmartPtr<IGraphBuilder> graphBuilder;
        ComSmartPtr<IMediaControl> mediaControl;
        ComSmartPtr<IMediaPosition> mediaPosition;
        ComSmartPtr<IMediaEventEx> mediaEvent;
        ComSmartPtr<IBasicAudio> basicAudio;
        ComSmartPtr<IBaseFilter> baseFilter;

        std::unique_ptr<VideoRenderers::Base> videoRenderer;

        b8 hasVideo = false, needToUpdateViewport = true, needToRecreateNativeWindow = false;

        //==============================================================================
        b8 createNativeWindow()
        {
            jassert (nativeWindow == nullptr);

            if (auto* topLevelPeer = component.getTopLevelComponent()->getPeer())
            {
                nativeWindow.reset (new NativeWindow ((HWND) topLevelPeer->getNativeHandle(), this));

                hwnd = nativeWindow->hwnd;
                component.currentPeer = topLevelPeer;
                component.currentPeer->addScaleFactorListener (&component);

                if (hwnd != nullptr)
                {
                    hdc = GetDC (hwnd);
                    component.updateContextPosition();
                    component.updateContextVisibility();
                    return true;
                }

                nativeWindow = nullptr;
            }
            else
            {
                jassertfalse;
            }

            return false;
        }

        z0 deleteNativeWindow()
        {
            jassert (nativeWindow != nullptr);
            ReleaseDC (hwnd, hdc);
            hwnd = {};
            hdc = {};
            nativeWindow = nullptr;
        }

        b8 isRendererConnected()
        {
            ComSmartPtr<IEnumPins> enumPins;

            HRESULT hr = baseFilter->EnumPins (enumPins.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                hr = enumPins->Reset();

            ComSmartPtr<IPin> pin;

            while (SUCCEEDED (hr)
                    && enumPins->Next (1, pin.resetAndGetPointerAddress(), nullptr) == S_OK)
            {
                ComSmartPtr<IPin> otherPin;

                hr = pin->ConnectedTo (otherPin.resetAndGetPointerAddress());

                if (SUCCEEDED (hr))
                {
                    PIN_DIRECTION direction;
                    hr = pin->QueryDirection (&direction);

                    if (SUCCEEDED (hr) && direction == PINDIR_INPUT)
                        return true;
                }
                else if (hr == VFW_E_NOT_CONNECTED)
                {
                    hr = S_OK;
                }
            }

            return false;
        }

        //==============================================================================
        struct NativeWindowClass   : private DeletedAtShutdown
        {
            b8 isRegistered() const noexcept              { return atom != 0; }
            LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) (pointer_sized_uint) MAKELONG (atom, 0); }

            DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (NativeWindowClass)

        private:
            NativeWindowClass()
            {
                Txt windowClassName ("DRX_DIRECTSHOW_");
                windowClassName << (i32) (Time::currentTimeMillis() & 0x7fffffff);

                HINSTANCE moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

                TCHAR moduleFile [1024] = {};
                GetModuleFileName (moduleHandle, moduleFile, 1024);

                WNDCLASSEX wcex = {};
                wcex.cbSize         = sizeof (wcex);
                wcex.style          = CS_OWNDC;
                wcex.lpfnWndProc    = (WNDPROC) wndProc;
                wcex.lpszClassName  = windowClassName.toWideCharPointer();
                wcex.hInstance      = moduleHandle;

                atom = RegisterClassEx (&wcex);
                jassert (atom != 0);
            }

            ~NativeWindowClass()
            {
                if (atom != 0)
                    UnregisterClass (getWindowClassName(), (HINSTANCE) Process::getCurrentModuleInstanceHandle());

                clearSingletonInstance();
            }

            static LRESULT CALLBACK wndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                if (auto* c = (DirectShowContext*) GetWindowLongPtr (hwnd, GWLP_USERDATA))
                {
                    switch (msg)
                    {
                        case WM_NCHITTEST:          return HTTRANSPARENT;
                        case WM_ERASEBKGND:         return 1;
                        case WM_DISPLAYCHANGE:      c->displayResolutionChanged(); break;
                        case graphEventID:          c->graphEventProc(); return 0;
                        default:                    break;
                    }
                }

                return DefWindowProc (hwnd, msg, wParam, lParam);
            }

            ATOM atom = {};

            DRX_DECLARE_NON_COPYABLE (NativeWindowClass)
        };

        //==============================================================================
        struct NativeWindow
        {
            NativeWindow (HWND parentToAddTo, uk userData)
            {
                auto* wc = NativeWindowClass::getInstance();

                if (wc->isRegistered())
                {
                    DWORD exstyle = 0;
                    DWORD type = WS_CHILD;

                    hwnd = CreateWindowEx (exstyle, wc->getWindowClassName(),
                                           L"", type, 0, 0, 0, 0, parentToAddTo, nullptr,
                                           (HINSTANCE) Process::getCurrentModuleInstanceHandle(), nullptr);

                    if (hwnd != nullptr)
                    {
                        hdc = GetDC (hwnd);
                        SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) userData);
                    }
                }

                jassert (hwnd != nullptr);
            }

            ~NativeWindow()
            {
                if (hwnd != nullptr)
                {
                    SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) 0);
                    DestroyWindow (hwnd);
                }
            }

            z0 setWindowPosition (Rectangle<i32> newBounds)
            {
                SetWindowPos (hwnd, nullptr, newBounds.getX(), newBounds.getY(),
                              newBounds.getWidth(), newBounds.getHeight(),
                              SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            }

            z0 showWindow (b8 shouldBeVisible)
            {
                ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
            }

            HWND hwnd = {};
            HDC hdc = {};

            DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeWindow)
        };

        std::unique_ptr<NativeWindow> nativeWindow;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowContext)
    };

    std::unique_ptr<DirectShowContext> context;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};
