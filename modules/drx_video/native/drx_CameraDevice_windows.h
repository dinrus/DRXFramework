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

// The following definitions would normally be found in qedit.h, which is not part of the
// Windows SDK, and which is incompatible with newer versions of DirectX.
DRX_COMCLASS (ISampleGrabberCB, "0579154A-2B53-4994-B0D0-E773148EFF85") : public IUnknown
{
    DRX_COMCALL SampleCB (f64, IMediaSample*) = 0;
    DRX_COMCALL BufferCB (f64, BYTE*, i64) = 0;
};

DRX_COMCLASS (ISampleGrabber, "6B652FFF-11FE-4fce-92AD-0266B5D7C78F") : public IUnknown
{
    DRX_COMCALL SetOneShot (BOOL) = 0;
    DRX_COMCALL SetMediaType (const AM_MEDIA_TYPE*) = 0;
    DRX_COMCALL GetConnectedMediaType (AM_MEDIA_TYPE*) = 0;
    DRX_COMCALL SetBufferSamples (BOOL) = 0;
    DRX_COMCALL GetCurrentBuffer (i64*, i64*) = 0;
    DRX_COMCALL GetCurrentSample (IMediaSample**) = 0;
    DRX_COMCALL SetCallback (ISampleGrabberCB*, i64) = 0;
};

constexpr CLSID CLSID_NullRenderer  = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
constexpr CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

//==============================================================================
struct CameraDevice::Pimpl  : public ChangeBroadcaster
{
    Pimpl (CameraDevice& ownerToUse, const Txt&, i32 index,
           i32 minWidth, i32 minHeight, i32 maxWidth, i32 maxHeight,
           b8 /*highQuality*/)
       : owner (ownerToUse)
    {
        HRESULT hr = captureGraphBuilder.CoCreateInstance (CLSID_CaptureGraphBuilder2);
        if (FAILED (hr))
            return;

        filter = enumerateCameras (nullptr, index);
        if (filter == nullptr)
            return;

        hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph);
        if (FAILED (hr))
            return;

        hr = captureGraphBuilder->SetFiltergraph (graphBuilder);
        if (FAILED (hr))
            return;

        mediaControl = graphBuilder.getInterface<IMediaControl>();
        if (mediaControl == nullptr)
            return;

        {
            ComSmartPtr<IAMStreamConfig> streamConfig;

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            hr = captureGraphBuilder->FindInterface (&PIN_CATEGORY_CAPTURE, nullptr, filter,
                                                     __uuidof (IAMStreamConfig), (uk*) streamConfig.resetAndGetPointerAddress());
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            if (streamConfig != nullptr)
            {
                getVideoSizes (streamConfig);

                if (! selectVideoSize (streamConfig, minWidth, minHeight, maxWidth, maxHeight))
                    return;
            }
        }

        hr = graphBuilder->AddFilter (filter, _T ("Video Capture"));
        if (FAILED (hr))
            return;

        hr = smartTee.CoCreateInstance (CLSID_SmartTee);
        if (FAILED (hr))
            return;

        hr = graphBuilder->AddFilter (smartTee, _T ("Smart Tee"));
        if (FAILED (hr))
            return;

        if (! connectFilters (filter, smartTee))
            return;

        ComSmartPtr<IBaseFilter> sampleGrabberBase;
        hr = sampleGrabberBase.CoCreateInstance (CLSID_SampleGrabber);
        if (FAILED (hr))
            return;

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        hr = sampleGrabberBase.QueryInterface (__uuidof (ISampleGrabber), sampleGrabber);
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        if (FAILED (hr))
            return;

        {
            AM_MEDIA_TYPE mt = {};
            mt.majortype = MEDIATYPE_Video;
            mt.subtype = MEDIASUBTYPE_RGB24;
            mt.formattype = FORMAT_VideoInfo;
            sampleGrabber->SetMediaType (&mt);
        }

        callback = becomeComSmartPtrOwner (new GrabberCallback (*this));
        hr = sampleGrabber->SetCallback (callback, 1);

        hr = graphBuilder->AddFilter (sampleGrabberBase, _T ("Sample Grabber"));
        if (FAILED (hr))
            return;

        ComSmartPtr<IPin> grabberInputPin;
        if (! (getPin (smartTee, PINDIR_OUTPUT, smartTeeCaptureOutputPin, "capture")
                && getPin (smartTee, PINDIR_OUTPUT, smartTeePreviewOutputPin, "preview")
                && getPin (sampleGrabberBase, PINDIR_INPUT, grabberInputPin)))
            return;

        hr = graphBuilder->Connect (smartTeePreviewOutputPin, grabberInputPin);
        if (FAILED (hr))
            return;

        AM_MEDIA_TYPE mt = {};
        hr = sampleGrabber->GetConnectedMediaType (&mt);

        if (auto* pVih = unalignedPointerCast<VIDEOINFOHEADER*> (mt.pbFormat))
        {
            width = pVih->bmiHeader.biWidth;
            height = pVih->bmiHeader.biHeight;
        }

        ComSmartPtr<IBaseFilter> nullFilter;
        hr = nullFilter.CoCreateInstance (CLSID_NullRenderer);
        hr = graphBuilder->AddFilter (nullFilter, _T ("Null Renderer"));

        if (connectFilters (sampleGrabberBase, nullFilter)
              && addGraphToRot())
        {
            openedSuccessfully = true;
        }
    }

    ~Pimpl()
    {
        if (mediaControl != nullptr)
            mediaControl->Stop();

        removeGraphFromRot();
        disconnectAnyViewers();

        if (sampleGrabber != nullptr)
        {
            sampleGrabber->SetCallback (nullptr, 0);
            sampleGrabber = nullptr;
        }

        callback = nullptr;
        graphBuilder = nullptr;
        mediaControl = nullptr;
        filter = nullptr;
        captureGraphBuilder = nullptr;
        smartTee = nullptr;
        smartTeePreviewOutputPin = nullptr;
        smartTeeCaptureOutputPin = nullptr;
        asfWriter = nullptr;
    }

    b8 openedOk() const noexcept       { return openedSuccessfully; }

    z0 takeStillPicture (std::function<z0 (const Image&)> pictureTakenCallbackToUse)
    {
        {
            const ScopedLock sl (pictureTakenCallbackLock);

            jassert (pictureTakenCallbackToUse != nullptr);

            if (pictureTakenCallbackToUse == nullptr)
                return;

            pictureTakenCallback = std::move (pictureTakenCallbackToUse);
        }

        addUser();
    }

    z0 startRecordingToFile (const File& file, i32 quality)
    {
        addUser();
        isRecording = createFileCaptureFilter (file, quality);
    }

    z0 stopRecording()
    {
        if (isRecording)
        {
            removeFileCaptureFilter();
            removeUser();
            isRecording = false;
        }
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return firstRecordedTime;
    }

    z0 addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);

        if (listeners.size() == 0)
            addUser();

        listeners.add (listenerToAdd);
    }

    z0 removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);

        if (listeners.size() == 0)
            removeUser();
    }

    z0 callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });
    }

    z0 notifyPictureTakenIfNeeded (const Image& image)
    {
        {
            const ScopedLock sl (pictureTakenCallbackLock);

            if (pictureTakenCallback == nullptr)
                return;
        }

        MessageManager::callAsync ([weakRef = WeakReference<Pimpl> { this }, image]() mutable
                                   {
                                       if (weakRef == nullptr)
                                           return;

                                       NullCheckedInvocation::invoke (weakRef->pictureTakenCallback, image);
                                       weakRef->pictureTakenCallback = nullptr;
                                   });
    }

    z0 addUser()
    {
        if (openedSuccessfully && activeUsers++ == 0)
            mediaControl->Run();
    }

    z0 removeUser()
    {
        if (openedSuccessfully && --activeUsers == 0)
            mediaControl->Stop();
    }

    z0 handleFrame (f64 /*time*/, BYTE* buffer, i64 /*bufferSize*/)
    {
        if (recordNextFrameTime)
        {
            const f64 defaultCameraLatency = 0.1;

            firstRecordedTime = Time::getCurrentTime() - RelativeTime (defaultCameraLatency);
            recordNextFrameTime = false;

            ComSmartPtr<IPin> pin;
            if (getPin (filter, PINDIR_OUTPUT, pin))
            {
                if (auto pushSource = pin.getInterface<IAMPushSource>())
                {
                    REFERENCE_TIME latency = 0;
                    pushSource->GetLatency (&latency);

                    firstRecordedTime = firstRecordedTime - RelativeTime ((f64) latency);
                }
            }
        }

        Image loadingImage (Image::RGB, width, height, true);
        i32k lineStride = width * 3;

        {
            const Image::BitmapData destData (loadingImage, 0, 0, width, height, Image::BitmapData::writeOnly);

            for (i32 i = 0; i < height; ++i)
                memcpy (destData.getLinePointer ((height - 1) - i),
                        buffer + lineStride * i,
                        (size_t) lineStride);
        }

        if (! listeners.isEmpty())
            callListeners (loadingImage);

        notifyPictureTakenIfNeeded (loadingImage);

        sendChangeMessage();

        const ScopedLock sl (imageSwapLock);
        activeImage = loadingImage;
    }

    z0 drawCurrentImage (Graphics& g, Rectangle<i32> area)
    {
        const auto imageToDraw = [this]
        {
            const ScopedLock sl (imageSwapLock);
            return activeImage;
        }();

        Rectangle<i32> centred (RectanglePlacement (RectanglePlacement::centred)
                                    .appliedTo (Rectangle<i32> (width, height), area));

        RectangleList<i32> borders (area);
        borders.subtract (centred);
        g.setColor (Colors::black);
        g.fillRectList (borders);

        g.drawImage (imageToDraw, centred.getX(), centred.getY(),
                     centred.getWidth(), centred.getHeight(), 0, 0, width, height);
    }

    b8 createFileCaptureFilter (const File& file, i32 quality)
    {
        removeFileCaptureFilter();
        file.deleteFile();
        mediaControl->Stop();
        firstRecordedTime = Time();
        recordNextFrameTime = true;
        previewMaxFPS = 60;

        HRESULT hr = asfWriter.CoCreateInstance (CLSID_WMAsfWriter);

        if (SUCCEEDED (hr))
        {
            if (auto fileSink = asfWriter.getInterface<IFileSinkFilter>())
            {
                hr = fileSink->SetFileName (file.getFullPathName().toWideCharPointer(), nullptr);

                if (SUCCEEDED (hr))
                {
                    hr = graphBuilder->AddFilter (asfWriter, _T ("AsfWriter"));

                    if (SUCCEEDED (hr))
                    {
                        if (auto asfConfig = asfWriter.getInterface<IConfigAsfWriter>())
                        {
                            asfConfig->SetIndexMode (true);
                            ComSmartPtr<IWMProfileManager> profileManager;

                            hr = WMCreateProfileManager (profileManager.resetAndGetPointerAddress());

                            // This gibberish is the DirectShow profile for a video-only wmv file.
                            Txt prof ("<profile version=\"589824\" storageformat=\"1\" name=\"Quality\" description=\"Quality type for output.\">"
                                           "<streamconfig majortype=\"{73646976-0000-0010-8000-00AA00389B71}\" streamnumber=\"1\" "
                                                         "streamname=\"Video Stream\" inputname=\"Video409\" bitrate=\"894960\" "
                                                         "bufferwindow=\"0\" reliabletransport=\"1\" decodercomplexity=\"AU\" rfc1766langid=\"en-us\">"
                                             "<videomediaprops maxkeyframespacing=\"50000000\" quality=\"90\"/>"
                                             "<wmmediatype subtype=\"{33564D57-0000-0010-8000-00AA00389B71}\" bfixedsizesamples=\"0\" "
                                                          "btemporalcompression=\"1\" lsamplesize=\"0\">"
                                             "<videoinfoheader dwbitrate=\"894960\" dwbiterrorrate=\"0\" avgtimeperframe=\"$AVGTIMEPERFRAME\">"
                                                 "<rcsource left=\"0\" top=\"0\" right=\"$WIDTH\" bottom=\"$HEIGHT\"/>"
                                                 "<rctarget left=\"0\" top=\"0\" right=\"$WIDTH\" bottom=\"$HEIGHT\"/>"
                                                 "<bitmapinfoheader biwidth=\"$WIDTH\" biheight=\"$HEIGHT\" biplanes=\"1\" bibitcount=\"24\" "
                                                                   "bicompression=\"WMV3\" bisizeimage=\"0\" bixpelspermeter=\"0\" biypelspermeter=\"0\" "
                                                                   "biclrused=\"0\" biclrimportant=\"0\"/>"
                                               "</videoinfoheader>"
                                             "</wmmediatype>"
                                           "</streamconfig>"
                                         "</profile>");

                            i32k fps[] = { 10, 15, 30 };
                            i32 maxFramesPerSecond = fps[jlimit (0, numElementsInArray (fps) - 1, quality & 0xff)];

                            if (((u32) quality & 0xff000000) != 0) // (internal hacky way to pass explicit frame rates for testing)
                                maxFramesPerSecond = (quality >> 24) & 0xff;

                            prof = prof.replace ("$WIDTH", Txt (width))
                                       .replace ("$HEIGHT", Txt (height))
                                       .replace ("$AVGTIMEPERFRAME", Txt (10000000 / maxFramesPerSecond));

                            ComSmartPtr<IWMProfile> currentProfile;
                            hr = profileManager->LoadProfileByData (prof.toWideCharPointer(), currentProfile.resetAndGetPointerAddress());
                            hr = asfConfig->ConfigureFilterUsingProfile (currentProfile);

                            if (SUCCEEDED (hr))
                            {
                                ComSmartPtr<IPin> asfWriterInputPin;

                                if (getPin (asfWriter, PINDIR_INPUT, asfWriterInputPin, "Video Input 01"))
                                {
                                    hr = graphBuilder->Connect (smartTeeCaptureOutputPin, asfWriterInputPin);

                                    if (SUCCEEDED (hr) && openedSuccessfully && activeUsers > 0
                                        && SUCCEEDED (mediaControl->Run()))
                                    {
                                        previewMaxFPS = (quality < 2) ? 15 : 25; // throttle back the preview comps to try to leave the cpu free for encoding

                                        if ((quality & 0x00ff0000) != 0)  // (internal hacky way to pass explicit frame rates for testing)
                                            previewMaxFPS = (quality >> 16) & 0xff;

                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        removeFileCaptureFilter();

        if (openedSuccessfully && activeUsers > 0)
            mediaControl->Run();

        return false;
    }

    z0 removeFileCaptureFilter()
    {
        mediaControl->Stop();

        if (asfWriter != nullptr)
        {
            graphBuilder->RemoveFilter (asfWriter);
            asfWriter = nullptr;
        }

        if (openedSuccessfully && activeUsers > 0)
            mediaControl->Run();

        previewMaxFPS = 60;
    }

    static ComSmartPtr<IBaseFilter> enumerateCameras (StringArray* names, i32k deviceIndexToOpen)
    {
        i32 index = 0;
        ComSmartPtr<ICreateDevEnum> pDevEnum;

        struct Deleter
        {
            z0 operator() (IUnknown* ptr) const noexcept { ptr->Release(); }
        };

        using ContextPtr = std::unique_ptr<IBindCtx, Deleter>;

        if (SUCCEEDED (pDevEnum.CoCreateInstance (CLSID_SystemDeviceEnum)))
        {
            ComSmartPtr<IEnumMoniker> enumerator;
            HRESULT hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, enumerator.resetAndGetPointerAddress(), 0);

            if (SUCCEEDED (hr) && enumerator != nullptr)
            {
                ComSmartPtr<IMoniker> moniker;
                ULONG fetched;

                while (enumerator->Next (1, moniker.resetAndGetPointerAddress(), &fetched) == S_OK)
                {
                    auto context = []
                    {
                        IBindCtx* ptr = nullptr;
                        [[maybe_unused]] const auto result = CreateBindCtx (0, &ptr);
                        return ContextPtr (ptr);
                    }();

                    ComSmartPtr<IBaseFilter> captureFilter;
                    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
                    hr = moniker->BindToObject (context.get(), nullptr, __uuidof (IBaseFilter), (uk*) captureFilter.resetAndGetPointerAddress());
                    DRX_END_IGNORE_WARNINGS_GCC_LIKE

                    if (SUCCEEDED (hr))
                    {
                        ComSmartPtr<IPropertyBag> propertyBag;
                        hr = moniker->BindToStorage (context.get(), nullptr, IID_IPropertyBag, (uk*) propertyBag.resetAndGetPointerAddress());

                        if (SUCCEEDED (hr))
                        {
                            VARIANT var;
                            var.vt = VT_BSTR;

                            hr = propertyBag->Read (_T ("FriendlyName"), &var, nullptr);
                            propertyBag = nullptr;

                            if (SUCCEEDED (hr))
                            {
                                if (names != nullptr)
                                    names->add (var.bstrVal);

                                if (index == deviceIndexToOpen)
                                    return captureFilter;

                                ++index;
                            }
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    static StringArray getAvailableDevices()
    {
        StringArray devs;
        enumerateCameras (&devs, -1);
        return devs;
    }

    struct GrabberCallback   : public ComBaseClassHelperBase<ISampleGrabberCB>
    {
        explicit GrabberCallback (Pimpl& p)
            : owner (p) {}

        DRX_COMRESULT QueryInterface (REFIID refId, uk* result) override
        {
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            if (refId == __uuidof (ISampleGrabberCB))
                return castToType<ISampleGrabberCB> (result);
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            return ComBaseClassHelperBase<ISampleGrabberCB>::QueryInterface (refId, result);
        }

        DRX_COMRESULT SampleCB (f64, IMediaSample*)  override { return E_FAIL; }

        DRX_COMRESULT BufferCB (f64 time, BYTE* buffer, i64 bufferSize) override
        {
            owner.handleFrame (time, buffer, bufferSize);
            return S_OK;
        }

        Pimpl& owner;

        DRX_DECLARE_NON_COPYABLE (GrabberCallback)
    };

    CameraDevice& owner;

    ComSmartPtr<GrabberCallback> callback;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    CriticalSection pictureTakenCallbackLock;
    std::function<z0 (const Image&)> pictureTakenCallback;

    b8 isRecording = false, openedSuccessfully = false;
    i32 width = 0, height = 0;
    Time firstRecordedTime;

    Array<ViewerComponent*> viewerComps;

    ComSmartPtr<ICaptureGraphBuilder2> captureGraphBuilder;
    ComSmartPtr<IBaseFilter> filter, smartTee, asfWriter;
    ComSmartPtr<IGraphBuilder> graphBuilder;
    ComSmartPtr<ISampleGrabber> sampleGrabber;
    ComSmartPtr<IMediaControl> mediaControl;
    ComSmartPtr<IPin> smartTeePreviewOutputPin, smartTeeCaptureOutputPin;
    i32 activeUsers = 0;
    Array<i32> widths, heights;
    DWORD graphRegistrationID;

    b8 recordNextFrameTime = false;
    i32 previewMaxFPS = 60;

    DRX_DECLARE_WEAK_REFERENCEABLE (Pimpl)

private:
    CriticalSection imageSwapLock;
    Image activeImage;

    z0 getVideoSizes (IAMStreamConfig* const streamConfig)
    {
        widths.clear();
        heights.clear();

        i32 count = 0, size = 0;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == (i32) sizeof (VIDEO_STREAM_CONFIG_CAPS))
        {
            for (i32 i = 0; i < count; ++i)
            {
                VIDEO_STREAM_CONFIG_CAPS scc;
                AM_MEDIA_TYPE* config;

                HRESULT hr = streamConfig->GetStreamCaps (i, &config, (BYTE*) &scc);

                if (SUCCEEDED (hr))
                {
                    i32k w = scc.InputSize.cx;
                    i32k h = scc.InputSize.cy;

                    b8 duplicate = false;

                    for (i32 j = widths.size(); --j >= 0;)
                    {
                        if (w == widths.getUnchecked (j) && h == heights.getUnchecked (j))
                        {
                            duplicate = true;
                            break;
                        }
                    }

                    if (! duplicate)
                    {
                        widths.add (w);
                        heights.add (h);
                    }

                    deleteMediaType (config);
                }
            }
        }
    }

    b8 selectVideoSize (IAMStreamConfig* const streamConfig,
                          i32k minWidth, i32k minHeight,
                          i32k maxWidth, i32k maxHeight)
    {
        i32 count = 0, size = 0, bestArea = 0, bestIndex = -1;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == (i32) sizeof (VIDEO_STREAM_CONFIG_CAPS))
        {
            AM_MEDIA_TYPE* config;
            VIDEO_STREAM_CONFIG_CAPS scc;

            for (i32 i = 0; i < count; ++i)
            {
                HRESULT hr = streamConfig->GetStreamCaps (i, &config, (BYTE*) &scc);

                if (SUCCEEDED (hr))
                {
                    if (scc.InputSize.cx >= minWidth
                         && scc.InputSize.cy >= minHeight
                         && scc.InputSize.cx <= maxWidth
                         && scc.InputSize.cy <= maxHeight)
                    {
                        i32 area = scc.InputSize.cx * scc.InputSize.cy;
                        if (area > bestArea)
                        {
                            bestIndex = i;
                            bestArea = area;
                        }
                    }

                    deleteMediaType (config);
                }
            }

            if (bestIndex >= 0)
            {
                HRESULT hr = streamConfig->GetStreamCaps (bestIndex, &config, (BYTE*) &scc);

                hr = streamConfig->SetFormat (config);
                deleteMediaType (config);
                return SUCCEEDED (hr);
            }
        }

        return false;
    }

    static b8 getPin (IBaseFilter* filter, const PIN_DIRECTION wantedDirection,
                        ComSmartPtr<IPin>& result, tukk pinName = nullptr)
    {
        ComSmartPtr<IEnumPins> enumerator;
        ComSmartPtr<IPin> pin;

        filter->EnumPins (enumerator.resetAndGetPointerAddress());

        while (enumerator->Next (1, pin.resetAndGetPointerAddress(), nullptr) == S_OK)
        {
            PIN_DIRECTION dir;
            pin->QueryDirection (&dir);

            if (wantedDirection == dir)
            {
                PIN_INFO info = {};
                pin->QueryPinInfo (&info);

                if (pinName == nullptr || Txt (pinName).equalsIgnoreCase (Txt (info.achName)))
                {
                    result = pin;
                    return true;
                }
            }
        }

        return false;
    }

    b8 connectFilters (IBaseFilter* const first, IBaseFilter* const second) const
    {
        ComSmartPtr<IPin> in, out;

        return getPin (first, PINDIR_OUTPUT, out)
                && getPin (second, PINDIR_INPUT, in)
                && SUCCEEDED (graphBuilder->Connect (out, in));
    }

    b8 addGraphToRot()
    {
        ComSmartPtr<IRunningObjectTable> rot;
        if (FAILED (GetRunningObjectTable (0, rot.resetAndGetPointerAddress())))
            return false;

        ComSmartPtr<IMoniker> moniker;
        WCHAR buffer[128]{};
        HRESULT hr = CreateItemMoniker (_T ("!"), buffer, moniker.resetAndGetPointerAddress());
        if (FAILED (hr))
            return false;

        graphRegistrationID = 0;
        return SUCCEEDED (rot->Register (0, graphBuilder, moniker, &graphRegistrationID));
    }

    z0 removeGraphFromRot()
    {
        ComSmartPtr<IRunningObjectTable> rot;

        if (SUCCEEDED (GetRunningObjectTable (0, rot.resetAndGetPointerAddress())))
            rot->Revoke (graphRegistrationID);
    }

    z0 disconnectAnyViewers();

    static z0 deleteMediaType (AM_MEDIA_TYPE* const pmt)
    {
        if (pmt->cbFormat != 0)
            CoTaskMemFree ((PVOID) pmt->pbFormat);

        if (pmt->pUnk != nullptr)
            pmt->pUnk->Release();

        CoTaskMemFree (pmt);
    }

    DRX_DECLARE_NON_COPYABLE (Pimpl)
};

//==============================================================================
struct CameraDevice::ViewerComponent  : public Component,
                                        public ChangeListener
{
    ViewerComponent (CameraDevice& d)
       : owner (d.pimpl.get()), maxFPS (15), lastRepaintTime (0)
    {
        setOpaque (true);
        owner->addChangeListener (this);
        owner->addUser();
        owner->viewerComps.add (this);
        setSize (owner->width, owner->height);
    }

    ~ViewerComponent() override
    {
        if (owner != nullptr)
        {
            owner->viewerComps.removeFirstMatchingValue (this);
            owner->removeUser();
            owner->removeChangeListener (this);
        }
    }

    z0 ownerDeleted()
    {
        owner = nullptr;
    }

    z0 paint (Graphics& g) override
    {
        g.setColor (Colors::black);
        g.setImageResamplingQuality (Graphics::lowResamplingQuality);

        if (owner != nullptr)
            owner->drawCurrentImage (g, getLocalBounds());
        else
            g.fillAll();
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        const z64 now = Time::currentTimeMillis();

        if (now >= lastRepaintTime + (1000 / maxFPS))
        {
            lastRepaintTime = now;
            repaint();

            if (owner != nullptr)
                maxFPS = owner->previewMaxFPS;
        }
    }

private:
    Pimpl* owner;
    i32 maxFPS;
    z64 lastRepaintTime;
};

z0 CameraDevice::Pimpl::disconnectAnyViewers()
{
    for (i32 i = viewerComps.size(); --i >= 0;)
        viewerComps.getUnchecked (i)->ownerDeleted();
}

Txt CameraDevice::getFileExtension()
{
    return ".wmv";
}
