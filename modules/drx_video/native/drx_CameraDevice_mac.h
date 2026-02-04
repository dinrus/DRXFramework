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

struct CameraDevice::Pimpl
{
    Pimpl (CameraDevice& ownerToUse, const Txt& deviceNameToUse, i32 /*index*/,
           i32 /*minWidth*/, i32 /*minHeight*/,
           i32 /*maxWidth*/, i32 /*maxHeight*/,
           b8 useHighQuality)
        : owner (ownerToUse),
          deviceName (deviceNameToUse)
    {
        imageOutput = []() -> std::unique_ptr<ImageOutputBase>
        {
            if (@available (macOS 10.15, *))
                return std::make_unique<PostCatalinaPhotoOutput>();

           return std::make_unique<PreCatalinaStillImageOutput>();
        }();

        session = [[AVCaptureSession alloc] init];

        session.sessionPreset = useHighQuality ? AVCaptureSessionPresetHigh
                                               : AVCaptureSessionPresetMedium;

        refreshConnections();

        static DelegateClass cls;
        callbackDelegate = (id<AVCaptureFileOutputRecordingDelegate>) [cls.createInstance() init];
        DelegateClass::setOwner (callbackDelegate, this);

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter] addObserver: callbackDelegate
                                                 selector: @selector (captureSessionRuntimeError:)
                                                     name: AVCaptureSessionRuntimeErrorNotification
                                                   object: session];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    ~Pimpl()
    {
        [[NSNotificationCenter defaultCenter] removeObserver: callbackDelegate];

        [session stopRunning];
        removeInput();
        removeImageCapture();
        removeMovieCapture();
        [session release];
        [callbackDelegate release];
    }

    //==============================================================================
    b8 openedOk() const noexcept       { return openingError.isEmpty(); }

    z0 startSession()
    {
        if (! [session isRunning])
            [session startRunning];
    }

    z0 takeStillPicture (std::function<z0 (const Image&)> pictureTakenCallbackToUse)
    {
        if (pictureTakenCallbackToUse == nullptr)
        {
            jassertfalse;
            return;
        }

        pictureTakenCallback = std::move (pictureTakenCallbackToUse);

        triggerImageCapture();
    }

    z0 startRecordingToFile (const File& file, i32 /*quality*/)
    {
        stopRecording();
        refreshIfNeeded();
        firstPresentationTime = Time::getCurrentTime();
        file.deleteFile();

        startSession();
        isRecording = true;
        [fileOutput startRecordingToOutputFileURL: createNSURLFromFile (file)
                                recordingDelegate: callbackDelegate];
    }

    z0 stopRecording()
    {
        if (isRecording)
        {
            [fileOutput stopRecording];
            isRecording = false;
        }
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return firstPresentationTime;
    }

    z0 addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.add (listenerToAdd);

        if (listeners.size() == 1)
            triggerImageCapture();
    }

    z0 removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);
    }

    static NSArray* getCaptureDevices()
    {
        if (@available (macOS 10.15, *))
        {
            DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
            const auto deviceType = AVCaptureDeviceTypeExternalUnknown;
            DRX_END_IGNORE_DEPRECATION_WARNINGS

            auto* discovery = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes: @[AVCaptureDeviceTypeBuiltInWideAngleCamera, deviceType]
                                                                                     mediaType: AVMediaTypeVideo
                                                                                      position: AVCaptureDevicePositionUnspecified];

            return [discovery devices];
        }

        DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
        return [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
        DRX_END_IGNORE_DEPRECATION_WARNINGS
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;

        for (AVCaptureDevice* device : getCaptureDevices())
            results.add (nsStringToDrx ([device localizedName]));

        return results;
    }

    AVCaptureSession* getCaptureSession()
    {
        return session;
    }

    NSView* createVideoCapturePreview()
    {
        // The video preview must be created before the capture session is
        // started. Make sure you haven't called `addListener`,
        // `startRecordingToFile`, or `takeStillPicture` before calling this
        // function.
        jassert (! [session isRunning]);
        startSession();

        DRX_AUTORELEASEPOOL
        {
            NSView* view = [[NSView alloc] init];
            [view setLayer: [AVCaptureVideoPreviewLayer layerWithSession: getCaptureSession()]];
            return view;
        }
    }

private:
    //==============================================================================
    struct DelegateClass  : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("DRXCameraDelegate_")
        {
            addIvar<Pimpl*> ("owner");
            addProtocol (@protocol (AVCaptureFileOutputRecordingDelegate));

            addMethod (@selector (captureOutput:didStartRecordingToOutputFileAtURL:  fromConnections:),       didStartRecordingToOutputFileAtURL);
            addMethod (@selector (captureOutput:didPauseRecordingToOutputFileAtURL:  fromConnections:),       didPauseRecordingToOutputFileAtURL);
            addMethod (@selector (captureOutput:didResumeRecordingToOutputFileAtURL: fromConnections:),       didResumeRecordingToOutputFileAtURL);
            addMethod (@selector (captureOutput:willFinishRecordingToOutputFileAtURL:fromConnections:error:), willFinishRecordingToOutputFileAtURL);

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (captureSessionRuntimeError:), sessionRuntimeError);
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }

        static z0 setOwner (id self, Pimpl* owner)   { object_setInstanceVariable (self, "owner", owner); }
        static Pimpl& getOwner (id self)               { return *getIvar<Pimpl*> (self, "owner"); }

    private:
        static z0 didStartRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*) {}
        static z0 didPauseRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*) {}
        static z0 didResumeRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*) {}
        static z0 willFinishRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*, NSError*) {}

        static z0 sessionRuntimeError (id self, SEL, NSNotification* notification)
        {
            DRX_CAMERA_LOG (nsStringToDrx ([notification description]));

            NSError* error = [notification.userInfo objectForKey: AVCaptureSessionErrorKey];
            auto errorString = error != nil ? nsStringToDrx (error.localizedDescription) : Txt();
            getOwner (self).cameraSessionRuntimeError (errorString);
        }
    };

    //==============================================================================
    struct ImageOutputBase
    {
        virtual ~ImageOutputBase() = default;

        virtual z0 addImageCapture (AVCaptureSession*) = 0;
        virtual z0 removeImageCapture (AVCaptureSession*) = 0;
        virtual NSArray<AVCaptureConnection*>* getConnections() const = 0;
        virtual z0 triggerImageCapture (Pimpl& p) = 0;
    };

    class API_AVAILABLE (macos (10.15)) PostCatalinaPhotoOutput  : public ImageOutputBase
    {
    public:
        PostCatalinaPhotoOutput()
        {
            static PhotoOutputDelegateClass cls;
            delegate.reset ([cls.createInstance() init]);
        }

        z0 addImageCapture (AVCaptureSession* s) override
        {
            if (imageOutput != nil)
                return;

            imageOutput = [[AVCapturePhotoOutput alloc] init];
            [s addOutput: imageOutput];
        }

        z0 removeImageCapture (AVCaptureSession* s) override
        {
            if (imageOutput == nil)
                return;

            [s removeOutput: imageOutput];
            [imageOutput release];
            imageOutput = nil;
        }

        NSArray<AVCaptureConnection*>* getConnections() const override
        {
            if (imageOutput != nil)
                return imageOutput.connections;

            return nil;
        }

        z0 triggerImageCapture (Pimpl& p) override
        {
            if (imageOutput == nil)
                return;

            PhotoOutputDelegateClass::setOwner (delegate.get(), &p);

            [imageOutput capturePhotoWithSettings: [AVCapturePhotoSettings photoSettings]
                                         delegate: delegate.get()];
        }

    private:
        class PhotoOutputDelegateClass : public ObjCClass<NSObject<AVCapturePhotoCaptureDelegate>>
        {
        public:
            PhotoOutputDelegateClass()
                : ObjCClass ("PhotoOutputDelegateClass_")
            {
                addMethod (@selector (captureOutput:didFinishProcessingPhoto:error:), [] (id self, SEL, AVCapturePhotoOutput*, AVCapturePhoto* photo, NSError* error)
                {
                    if (error != nil)
                    {
                        [[maybe_unused]] Txt errorString = error != nil ? nsStringToDrx (error.localizedDescription) : Txt();

                        DRX_CAMERA_LOG ("Still picture capture failed, error: " + errorString);
                        jassertfalse;

                        return;
                    }

                    auto* imageData = [photo fileDataRepresentation];
                    auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                    getOwner (self).imageCaptureFinished (image);
                });

                addIvar<Pimpl*> ("owner");
                registerClass();
            }

            static Pimpl& getOwner (id self) { return *getIvar<Pimpl*> (self, "owner"); }
            static z0 setOwner (id self, Pimpl* t) { object_setInstanceVariable (self, "owner", t); }
        };

        AVCapturePhotoOutput* imageOutput = nil;
        NSUniquePtr<NSObject<AVCapturePhotoCaptureDelegate>> delegate;
    };

    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    class PreCatalinaStillImageOutput  : public ImageOutputBase
    {
    public:
        z0 addImageCapture (AVCaptureSession* s) override
        {
            if (imageOutput != nil)
                return;

            const auto codecType = []
            {
                if (@available (macOS 10.13, *))
                   return AVVideoCodecTypeJPEG;

                return AVVideoCodecJPEG;
            }();

            imageOutput = [[AVCaptureStillImageOutput alloc] init];
            auto imageSettings = [[NSDictionary alloc] initWithObjectsAndKeys: codecType, AVVideoCodecKey, nil];
            [imageOutput setOutputSettings: imageSettings];
            [imageSettings release];
            [s addOutput: imageOutput];
        }

        z0 removeImageCapture (AVCaptureSession* s) override
        {
            if (imageOutput == nil)
                return;

            [s removeOutput: imageOutput];
            [imageOutput release];
            imageOutput = nil;
        }

        NSArray<AVCaptureConnection*>* getConnections() const override
        {
            if (imageOutput != nil)
                return imageOutput.connections;

            return nil;
        }

        z0 triggerImageCapture (Pimpl& p) override
        {
            if (auto* videoConnection = p.getVideoConnection())
            {
                [imageOutput captureStillImageAsynchronouslyFromConnection: videoConnection
                                                         completionHandler: ^(CMSampleBufferRef sampleBuffer, NSError* error)
                {
                    if (error != nil)
                    {
                        DRX_CAMERA_LOG ("Still picture capture failed, error: " + nsStringToDrx (error.localizedDescription));
                        jassertfalse;
                        return;
                    }

                    auto* imageData = [AVCaptureStillImageOutput jpegStillImageNSDataRepresentation: sampleBuffer];
                    auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);
                    p.imageCaptureFinished (image);
                }];
            }
        }

    private:
        AVCaptureStillImageOutput* imageOutput = nil;
    };
    DRX_END_IGNORE_DEPRECATION_WARNINGS

    //==============================================================================
    z0 addImageCapture()
    {
        imageOutput->addImageCapture (session);
    }

    z0 addMovieCapture()
    {
        if (fileOutput == nil)
        {
            fileOutput = [[AVCaptureMovieFileOutput alloc] init];
            [session addOutput: fileOutput];
        }
    }

    z0 removeImageCapture()
    {
        imageOutput->removeImageCapture (session);
    }

    z0 removeMovieCapture()
    {
        if (fileOutput != nil)
        {
            [session removeOutput: fileOutput];
            [fileOutput release];
            fileOutput = nil;
        }
    }

    z0 removeCurrentSessionVideoInputs()
    {
        if (session != nil)
        {
            NSArray<AVCaptureDeviceInput*>* inputs = session.inputs;

            for (AVCaptureDeviceInput* input : inputs)
                if ([input.device hasMediaType: AVMediaTypeVideo])
                    [session removeInput:input];
        }
    }

    z0 addInput()
    {
        if (currentInput == nil)
        {
            for (AVCaptureDevice* device : getCaptureDevices())
            {
                if (deviceName == nsStringToDrx ([device localizedName]))
                {
                    removeCurrentSessionVideoInputs();

                    NSError* err = nil;
                    AVCaptureDeviceInput* inputDevice = [[AVCaptureDeviceInput alloc] initWithDevice: device
                                                                                               error: &err];

                    jassert (err == nil);

                    if ([session canAddInput: inputDevice])
                    {
                        [session addInput: inputDevice];
                        currentInput = inputDevice;
                    }
                    else
                    {
                        jassertfalse;
                        [inputDevice release];
                    }

                    return;
                }
            }
        }
    }

    z0 removeInput()
    {
        if (currentInput != nil)
        {
            [session removeInput: currentInput];
            [currentInput release];
            currentInput = nil;
        }
    }

    z0 refreshConnections()
    {
        [session beginConfiguration];
        removeInput();
        removeImageCapture();
        removeMovieCapture();
        addInput();
        addImageCapture();
        addMovieCapture();
        [session commitConfiguration];
    }

    z0 refreshIfNeeded()
    {
        if (getVideoConnection() == nullptr)
            refreshConnections();
    }

    AVCaptureConnection* getVideoConnection() const
    {
        auto* connections = imageOutput->getConnections();

        if (connections != nil)
            for (AVCaptureConnection* connection in connections)
                if ([connection isActive] && [connection isEnabled])
                    for (AVCaptureInputPort* port in [connection inputPorts])
                        if ([[port mediaType] isEqual: AVMediaTypeVideo])
                            return connection;

        return nil;
    }

    z0 imageCaptureFinished (const Image& image)
    {
        handleImageCapture (image);

        MessageManager::callAsync ([weakRef = WeakReference<Pimpl> { this }, image]() mutable
        {
            if (weakRef != nullptr)
                NullCheckedInvocation::invoke (weakRef->pictureTakenCallback, image);
        });
    }

    z0 handleImageCapture (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });

        if (! listeners.isEmpty())
            triggerImageCapture();
    }

    z0 triggerImageCapture()
    {
        refreshIfNeeded();

        startSession();

        if (getVideoConnection() != nullptr)
            imageOutput->triggerImageCapture (*this);
    }

    z0 cameraSessionRuntimeError (const Txt& error)
    {
        DRX_CAMERA_LOG ("cameraSessionRuntimeError(), error = " + error);

        NullCheckedInvocation::invoke (owner.onErrorOccurred, error);
    }

    //==============================================================================
    CameraDevice& owner;
    Txt deviceName;

    AVCaptureSession* session = nil;
    AVCaptureMovieFileOutput* fileOutput = nil;
    std::unique_ptr<ImageOutputBase> imageOutput;
    AVCaptureDeviceInput* currentInput = nil;

    id<AVCaptureFileOutputRecordingDelegate> callbackDelegate = nil;
    Txt openingError;
    Time firstPresentationTime;
    b8 isRecording = false;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    std::function<z0 (const Image&)> pictureTakenCallback = nullptr;

    //==============================================================================
    DRX_DECLARE_WEAK_REFERENCEABLE (Pimpl)
    DRX_DECLARE_NON_COPYABLE       (Pimpl)
};

//==============================================================================
struct CameraDevice::ViewerComponent  : public NSViewComponent
{
    ViewerComponent (CameraDevice& device)
    {
        setView (device.pimpl->createVideoCapturePreview());
    }

    ~ViewerComponent()
    {
        setView (nil);
    }

    DRX_DECLARE_NON_COPYABLE (ViewerComponent)
};

Txt CameraDevice::getFileExtension()
{
    return ".mov";
}
