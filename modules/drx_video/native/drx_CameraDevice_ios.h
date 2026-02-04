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
    static z0 applyDeviceOrientation (AVCaptureDevice*, AVCaptureVideoPreviewLayer*, AVCaptureConnection* outputConnection)
    {
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
        const auto statusBarOrientation = [UIApplication sharedApplication].statusBarOrientation;
        const auto videoOrientation = statusBarOrientation != UIInterfaceOrientationUnknown
                                    ? (AVCaptureVideoOrientation) statusBarOrientation
                                    : AVCaptureVideoOrientationPortrait;
        outputConnection.videoOrientation = videoOrientation;
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    struct PreviewLayerAngleTrait
    {
       #if DRX_IOS_API_VERSION_CAN_BE_BUILT (17, 0)
        API_AVAILABLE (ios (17))
        static z0 newFn (AVCaptureDevice* device, AVCaptureVideoPreviewLayer* previewLayer, AVCaptureConnection* outputConnection)
        {
            const NSUniquePtr<AVCaptureDeviceRotationCoordinator> coordinator
            {
                [[AVCaptureDeviceRotationCoordinator alloc] initWithDevice: device
                                                              previewLayer: previewLayer]
            };

            outputConnection.videoRotationAngle = coordinator.get().videoRotationAngleForHorizonLevelPreview;
        }
       #endif

        static constexpr auto oldFn = applyDeviceOrientation;
    };

    struct CaptureLayerAngleTrait
    {
       #if DRX_IOS_API_VERSION_CAN_BE_BUILT (17, 0)
        API_AVAILABLE (ios (17))
        static z0 newFn (AVCaptureDevice* device, AVCaptureVideoPreviewLayer* previewLayer, AVCaptureConnection* outputConnection)
        {
            const NSUniquePtr<AVCaptureDeviceRotationCoordinator> coordinator
            {
                [[AVCaptureDeviceRotationCoordinator alloc] initWithDevice: device
                                                              previewLayer: previewLayer]
            };

            outputConnection.videoRotationAngle = coordinator.get().videoRotationAngleForHorizonLevelCapture;
        }
       #endif

        static constexpr auto oldFn = applyDeviceOrientation;
    };

    using InternalOpenCameraResultCallback = std::function<z0 (const Txt& /*cameraId*/, const Txt& /*error*/)>;

    Pimpl (CameraDevice& ownerToUse, const Txt& cameraIdToUse, i32 /*index*/,
           i32 /*minWidth*/, i32 /*minHeight*/, i32 /*maxWidth*/, i32 /*maxHeight*/,
           b8 useHighQuality)
        : owner (ownerToUse),
          cameraId (cameraIdToUse),
          captureSession (*this, useHighQuality)
    {
    }

    Txt getCameraId() const noexcept { return cameraId; }

    z0 open (InternalOpenCameraResultCallback cameraOpenCallbackToUse)
    {
        cameraOpenCallback = std::move (cameraOpenCallbackToUse);

        if (cameraOpenCallback == nullptr)
        {
            // A valid camera open callback must be passed.
            jassertfalse;
            return;
        }

        [AVCaptureDevice requestAccessForMediaType: AVMediaTypeVideo
                                 completionHandler: ^([[maybe_unused]] BOOL granted)
         {
             // Access to video is required for camera to work,
             // black images will be produced otherwise!
             jassert (granted);
         }];

        [AVCaptureDevice requestAccessForMediaType: AVMediaTypeAudio
                                 completionHandler: ^([[maybe_unused]] BOOL granted)
         {
             // Access to audio is required for camera to work,
             // silence will be produced otherwise!
             jassert (granted);
         }];

        captureSession.startSessionForDeviceWithId (cameraId);
    }

    b8 openedOk() const noexcept { return captureSession.openedOk(); }

    z0 takeStillPicture (std::function<z0 (const Image&)> pictureTakenCallbackToUse)
    {
        if (pictureTakenCallbackToUse == nullptr)
        {
            jassertfalse;
            return;
        }

        pictureTakenCallback = std::move (pictureTakenCallbackToUse);

        triggerStillPictureCapture();
    }

    z0 startRecordingToFile (const File& file, i32 /*quality*/)
    {
        file.deleteFile();

        captureSession.startRecording (file);
    }

    z0 stopRecording()
    {
        captureSession.stopRecording();
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return captureSession.getTimeOfFirstRecordedFrame();
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;

        DRX_CAMERA_LOG ("Available camera devices: ");

        for (AVCaptureDevice* device in getDevices())
        {
            DRX_CAMERA_LOG ("Device start----------------------------------");
            printDebugCameraInfo (device);
            DRX_CAMERA_LOG ("Device end----------------------------------");

            results.add (nsStringToDrx (device.uniqueID));
        }

        return results;
    }

    z0 addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.add (listenerToAdd);

        if (listeners.size() == 1)
            triggerStillPictureCapture();
    }

    z0 removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);
    }

private:
    static NSArray<AVCaptureDevice*>* getDevices()
    {
        std::unique_ptr<NSMutableArray<AVCaptureDeviceType>, NSObjectDeleter> deviceTypes ([[NSMutableArray alloc] initWithCapacity: 2]);

        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInWideAngleCamera];
        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInTelephotoCamera];

        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInDualCamera];

        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInTrueDepthCamera];

        auto discoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes: deviceTypes.get()
                                                                                       mediaType: AVMediaTypeVideo
                                                                                        position: AVCaptureDevicePositionUnspecified];

        return [discoverySession devices];
    }

    //==============================================================================
    static z0 printDebugCameraInfo (AVCaptureDevice* device)
    {
        auto position = device.position;

        Txt positionString = position == AVCaptureDevicePositionBack
                              ? "Back"
                              : position == AVCaptureDevicePositionFront
                                         ? "Front"
                                         : "Unspecified";

        DRX_CAMERA_LOG ("Position: " + positionString);
        DRX_CAMERA_LOG ("Model ID: " + nsStringToDrx (device.modelID));
        DRX_CAMERA_LOG ("Localized name: " + nsStringToDrx (device.localizedName));
        DRX_CAMERA_LOG ("Unique ID: " + nsStringToDrx (device.uniqueID));
        DRX_CAMERA_LOG ("Lens aperture: " + Txt (device.lensAperture));

        DRX_CAMERA_LOG ("Has flash: " + Txt ((i32)device.hasFlash));
        DRX_CAMERA_LOG ("Supports flash always on: " + Txt ((i32)[device isFlashModeSupported: AVCaptureFlashModeOn]));
        DRX_CAMERA_LOG ("Supports auto flash: " + Txt ((i32)[device isFlashModeSupported: AVCaptureFlashModeAuto]));

        DRX_CAMERA_LOG ("Has torch: " + Txt ((i32)device.hasTorch));
        DRX_CAMERA_LOG ("Supports torch always on: " + Txt ((i32)[device isTorchModeSupported: AVCaptureTorchModeOn]));
        DRX_CAMERA_LOG ("Supports auto torch: " + Txt ((i32)[device isTorchModeSupported: AVCaptureTorchModeAuto]));

        DRX_CAMERA_LOG ("Low light boost supported: " + Txt ((i32)device.lowLightBoostEnabled));

        DRX_CAMERA_LOG ("Supports auto white balance: " + Txt ((i32)[device isWhiteBalanceModeSupported: AVCaptureWhiteBalanceModeAutoWhiteBalance]));
        DRX_CAMERA_LOG ("Supports continuous auto white balance: " + Txt ((i32)[device isWhiteBalanceModeSupported: AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance]));

        DRX_CAMERA_LOG ("Supports auto focus: " + Txt ((i32)[device isFocusModeSupported: AVCaptureFocusModeAutoFocus]));
        DRX_CAMERA_LOG ("Supports continuous auto focus: " + Txt ((i32)[device isFocusModeSupported: AVCaptureFocusModeContinuousAutoFocus]));
        DRX_CAMERA_LOG ("Supports point of interest focus: " + Txt ((i32)device.focusPointOfInterestSupported));
        DRX_CAMERA_LOG ("Smooth auto focus supported: " + Txt ((i32)device.smoothAutoFocusSupported));
        DRX_CAMERA_LOG ("Auto focus range restriction supported: " + Txt ((i32)device.autoFocusRangeRestrictionSupported));

        DRX_CAMERA_LOG ("Supports auto exposure: " + Txt ((i32)[device isExposureModeSupported: AVCaptureExposureModeAutoExpose]));
        DRX_CAMERA_LOG ("Supports continuous auto exposure: " + Txt ((i32)[device isExposureModeSupported: AVCaptureExposureModeContinuousAutoExposure]));
        DRX_CAMERA_LOG ("Supports custom exposure: " + Txt ((i32)[device isExposureModeSupported: AVCaptureExposureModeCustom]));
        DRX_CAMERA_LOG ("Supports point of interest exposure: " + Txt ((i32)device.exposurePointOfInterestSupported));

        DRX_CAMERA_LOG ("Device type: " + nsStringToDrx (device.deviceType));
        DRX_CAMERA_LOG ("Locking focus with custom lens position supported: " + Txt ((i32)device.lockingFocusWithCustomLensPositionSupported));

        DRX_CAMERA_LOG ("Min available video zoom factor: " + Txt (device.minAvailableVideoZoomFactor));
        DRX_CAMERA_LOG ("Max available video zoom factor: " + Txt (device.maxAvailableVideoZoomFactor));
        DRX_CAMERA_LOG ("Dual camera switch over video zoom factor: " + Txt (device.dualCameraSwitchOverVideoZoomFactor));

        DRX_CAMERA_LOG ("Capture formats start-------------------");
        for (AVCaptureDeviceFormat* format in device.formats)
        {
            DRX_CAMERA_LOG ("Capture format start------");
            printDebugCameraFormatInfo (format);
            DRX_CAMERA_LOG ("Capture format end------");
        }
        DRX_CAMERA_LOG ("Capture formats end-------------------");
    }

    static z0 printDebugCameraFormatInfo (AVCaptureDeviceFormat* format)
    {
        DRX_CAMERA_LOG ("Media type: " + nsStringToDrx (format.mediaType));

        Txt colourSpaces;

        for (NSNumber* number in format.supportedColorSpaces)
        {
            switch ([number intValue])
            {
                case AVCaptureColorSpace_sRGB:   colourSpaces << "sRGB ";  break;
                case AVCaptureColorSpace_P3_D65: colourSpaces << "P3_D65 "; break;
                default: break;
            }
        }

        DRX_CAMERA_LOG ("Supported colour spaces: " + colourSpaces);

        DRX_CAMERA_LOG ("Video field of view: " + Txt (format.videoFieldOfView));
        DRX_CAMERA_LOG ("Video max zoom factor: " + Txt (format.videoMaxZoomFactor));
        DRX_CAMERA_LOG ("Video zoom factor upscale threshold: " + Txt (format.videoZoomFactorUpscaleThreshold));

        Txt videoFrameRateRangesString = "Video supported frame rate ranges: ";

        for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges)
            videoFrameRateRangesString << frameRateRangeToString (range);
        DRX_CAMERA_LOG (videoFrameRateRangesString);

        DRX_CAMERA_LOG ("Video binned: " + Txt (i32 (format.videoBinned)));

        DRX_CAMERA_LOG ("Video HDR supported: " + Txt (i32 (format.videoHDRSupported)));
        DRX_CAMERA_LOG ("High resolution still image dimensions: " + getHighResStillImgDimensionsString (format.highResolutionStillImageDimensions));
        DRX_CAMERA_LOG ("Min ISO: " + Txt (format.minISO));
        DRX_CAMERA_LOG ("Max ISO: " + Txt (format.maxISO));
        DRX_CAMERA_LOG ("Min exposure duration: " + cmTimeToString (format.minExposureDuration));

        Txt autoFocusSystemString;
        switch (format.autoFocusSystem)
        {
            case AVCaptureAutoFocusSystemPhaseDetection:    autoFocusSystemString = "PhaseDetection";    break;
            case AVCaptureAutoFocusSystemContrastDetection: autoFocusSystemString = "ContrastDetection"; break;
            case AVCaptureAutoFocusSystemNone:
            default:                                        autoFocusSystemString = "None";
        }
        DRX_CAMERA_LOG ("Auto focus system: " + autoFocusSystemString);

        DRX_CAMERA_LOG ("Standard video stabilization supported: " + Txt ((i32) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeStandard]));
        DRX_CAMERA_LOG ("Cinematic video stabilization supported: " + Txt ((i32) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeCinematic]));
        DRX_CAMERA_LOG ("Auto video stabilization supported: " + Txt ((i32) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeAuto]));

        DRX_CAMERA_LOG ("Min zoom factor for depth data delivery: " + Txt (format.videoMinZoomFactorForDepthDataDelivery));
        DRX_CAMERA_LOG ("Max zoom factor for depth data delivery: " + Txt (format.videoMaxZoomFactorForDepthDataDelivery));
    }

    static Txt getHighResStillImgDimensionsString (CMVideoDimensions d)
    {
        return "[" + Txt (d.width) + " " + Txt (d.height) + "]";
    }

    static Txt cmTimeToString (CMTime time)
    {
        CFUniquePtr<CFStringRef> timeDesc (CMTimeCopyDescription (nullptr, time));
        return Txt::fromCFString (timeDesc.get());
    }

    static Txt frameRateRangeToString (AVFrameRateRange* range)
    {
        Txt result;
        result << "[minFrameDuration: " + cmTimeToString (range.minFrameDuration);
        result << " maxFrameDuration: " + cmTimeToString (range.maxFrameDuration);
        result << " minFrameRate: " + Txt (range.minFrameRate);
        result << " maxFrameRate: " + Txt (range.maxFrameRate) << "] ";

        return result;
    }

    //==============================================================================
    class CaptureSession
    {
    public:
        CaptureSession (Pimpl& ownerToUse, b8 useHighQuality)
            : owner (ownerToUse),
              captureSessionQueue (dispatch_queue_create ("DrxCameraDeviceBackgroundDispatchQueue", DISPATCH_QUEUE_SERIAL)),
              captureSession ([[AVCaptureSession alloc] init]),
              delegate (nullptr),
              stillPictureTaker (*this),
              videoRecorder (*this)
        {
            static SessionDelegateClass cls;
            delegate.reset ([cls.createInstance() init]);
            SessionDelegateClass::setOwner (delegate.get(), this);

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionDidStartRunning:)
                                                         name: AVCaptureSessionDidStartRunningNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionDidStopRunning:)
                                                         name: AVCaptureSessionDidStopRunningNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (runtimeError:)
                                                         name: AVCaptureSessionRuntimeErrorNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionWasInterrupted:)
                                                         name: AVCaptureSessionWasInterruptedNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionInterruptionEnded:)
                                                         name: AVCaptureSessionInterruptionEndedNotification
                                                       object: captureSession.get()];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            dispatch_async (captureSessionQueue,^
                            {
                                [captureSession.get() setSessionPreset: useHighQuality ? AVCaptureSessionPresetHigh
                                                                                       : AVCaptureSessionPresetMedium];
                            });

            ++numCaptureSessions;
        }

        ~CaptureSession()
        {
            [[NSNotificationCenter defaultCenter] removeObserver: delegate.get()];

            stopRecording();

            if (--numCaptureSessions == 0)
            {
                dispatch_async (captureSessionQueue, ^
                                {
                                    if (captureSession.get().running)
                                        [captureSession.get() stopRunning];

                                    sessionClosedEvent.signal();
                                });

                sessionClosedEvent.wait (-1);
            }
        }

        b8 openedOk() const noexcept { return sessionStarted; }

        z0 startSessionForDeviceWithId (const Txt& cameraIdToUse)
        {
            dispatch_async (captureSessionQueue,^
                            {
                                cameraDevice = [AVCaptureDevice deviceWithUniqueID: juceStringToNS (cameraIdToUse)];
                                auto audioDevice = [AVCaptureDevice defaultDeviceWithMediaType: AVMediaTypeAudio];

                                [captureSession.get() beginConfiguration];

                                // This will add just video...
                                auto error = addInputToDevice (cameraDevice);

                                if (error.isNotEmpty())
                                {
                                    MessageManager::callAsync ([weakRef = WeakReference<CaptureSession> { this }, error]() mutable
                                    {
                                        if (weakRef != nullptr)
                                            weakRef->owner.cameraOpenCallback ({}, error);
                                    });

                                    return;
                                }

                                // ... so add audio explicitly here
                                error = addInputToDevice (audioDevice);

                                if (error.isNotEmpty())
                                {
                                    MessageManager::callAsync ([weakRef = WeakReference<CaptureSession> { this }, error]() mutable
                                    {
                                        if (weakRef != nullptr)
                                            weakRef->owner.cameraOpenCallback ({}, error);
                                    });

                                    return;
                                }

                                [captureSession.get() commitConfiguration];

                                if (! captureSession.get().running)
                                    [captureSession.get() startRunning];
                            });
        }

        AVCaptureVideoPreviewLayer* createPreviewLayer()
        {
            if (! openedOk())
            {
                // A session must be started first!
                jassertfalse;
                return nullptr;
            }

            previewLayer = [AVCaptureVideoPreviewLayer layerWithSession: captureSession.get()];

            updatePreviewOrientation();

            return previewLayer;
        }

        z0 takeStillPicture()
        {
            if (! openedOk())
            {
                // A session must be started first!
                jassert (openedOk());
                return;
            }

            stillPictureTaker.takePicture();
        }

        z0 startRecording (const File& file)
        {
            if (! openedOk())
            {
                // A session must be started first!
                jassertfalse;
                return;
            }

            if (file.existsAsFile())
            {
                // File overwriting не поддерживается by iOS video recorder, the target
                // file must not exist.
                jassertfalse;
                return;
            }

            videoRecorder.startRecording (file);
        }

        z0 stopRecording()
        {
            videoRecorder.stopRecording();
        }

        Time getTimeOfFirstRecordedFrame() const
        {
            return videoRecorder.getTimeOfFirstRecordedFrame();
        }

        AVCaptureDevice* getDevice() const
        {
            return cameraDevice;
        }

        AVCaptureVideoPreviewLayer* getPreviewLayer() const
        {
            return previewLayer;
        }

        z0 updatePreviewOrientation()
        {
            ifelse_17_0<PreviewLayerAngleTrait> (cameraDevice, previewLayer, previewLayer.connection);
        }

        DRX_DECLARE_WEAK_REFERENCEABLE (CaptureSession)

    private:
        Txt addInputToDevice (AVCaptureDevice* device)
        {
            NSError* error = nil;

            auto input = [AVCaptureDeviceInput deviceInputWithDevice: device
                                                               error: &error];

            if (error != nil)
                return nsStringToDrx (error.localizedDescription);

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
            if (! [captureSession.get() canAddInput: input])
                return "Could not add input to camera session.";

            [captureSession.get() addInput: input];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
            return {};
        }

        //==============================================================================
        struct SessionDelegateClass    : public ObjCClass<NSObject>
        {
            SessionDelegateClass()  : ObjCClass<NSObject> ("SessionDelegateClass_")
            {
                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
                addMethod (@selector (sessionDidStartRunning:),
                           [] (id self, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               DRX_CAMERA_LOG (nsStringToDrx ([notification description]));

                               dispatch_async (dispatch_get_main_queue(),
                                               ^{
                                                   getOwner (self).cameraSessionStarted();
                                               });
                           });

                addMethod (@selector (sessionDidStopRunning:),
                           [] (id, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               DRX_CAMERA_LOG (nsStringToDrx ([notification description]));
                           });

                addMethod (@selector (runtimeError:),
                           [] (id self, SEL, NSNotification* notification)
                           {
                               DRX_CAMERA_LOG (nsStringToDrx ([notification description]));

                               dispatch_async (dispatch_get_main_queue(),
                                               ^{
                                                   NSError* error = notification.userInfo[AVCaptureSessionErrorKey];
                                                   auto errorString = error != nil ? nsStringToDrx (error.localizedDescription) : Txt();
                                                   getOwner (self).cameraSessionRuntimeError (errorString);
                                               });
                           });

                addMethod (@selector (sessionWasInterrupted:),
                           [] (id, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               DRX_CAMERA_LOG (nsStringToDrx ([notification description]));
                           });

                addMethod (@selector (sessionInterruptionEnded:),
                           [] (id, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               DRX_CAMERA_LOG (nsStringToDrx ([notification description]));
                           });

                DRX_END_IGNORE_WARNINGS_GCC_LIKE

                addIvar<CaptureSession*> ("owner");

                registerClass();
            }

            //==============================================================================
            static CaptureSession& getOwner (id self)         { return *getIvar<CaptureSession*> (self, "owner"); }
            static z0 setOwner (id self, CaptureSession* s) { object_setInstanceVariable (self, "owner", s); }
        };

        //==============================================================================
        class StillPictureTaker
        {
        public:
            explicit StillPictureTaker (CaptureSession& cs)
                : captureSession (cs),
                  captureOutput (createCaptureOutput()),
                  photoOutputDelegate (nullptr)
            {
                static PhotoOutputDelegateClass cls;
                photoOutputDelegate.reset ([cls.createInstance() init]);
                PhotoOutputDelegateClass::setOwner (photoOutputDelegate.get(), this);

                captureSession.addOutputIfPossible (captureOutput);
            }

            z0 takePicture()
            {
                if (takingPicture)
                {
                    // Picture taking already in progress!
                    jassertfalse;
                    return;
                }

                takingPicture = true;

                printImageOutputDebugInfo (captureOutput);

                if (findVideoConnection (captureOutput) != nullptr)
                {
                    auto* photoOutput = (AVCapturePhotoOutput*) captureOutput;
                    auto outputConnection = [photoOutput connectionWithMediaType: AVMediaTypeVideo];

                    ifelse_17_0<CaptureLayerAngleTrait> (captureSession.cameraDevice, captureSession.previewLayer, outputConnection);

                    [photoOutput capturePhotoWithSettings: [AVCapturePhotoSettings photoSettings]
                                                 delegate: id<AVCapturePhotoCaptureDelegate> (photoOutputDelegate.get())];
                }
                else
                {
                    // Could not find a connection of video type
                    jassertfalse;
                }
            }

        private:
            static AVCaptureOutput* createCaptureOutput()
            {
                return [AVCapturePhotoOutput new];
            }

            static z0 printImageOutputDebugInfo (AVCaptureOutput* captureOutput)
            {
                auto* photoOutput = (AVCapturePhotoOutput*) captureOutput;

                Txt typesString;

                for (id type in photoOutput.availablePhotoCodecTypes)
                    typesString << nsStringToDrx (type) << " ";

                DRX_CAMERA_LOG ("Available image codec types: " + typesString);

                DRX_CAMERA_LOG ("Still image stabilization supported: " + Txt ((i32) photoOutput.stillImageStabilizationSupported));
                DRX_CAMERA_LOG ("Dual camera fusion supported: " + Txt ((i32) photoOutput.dualCameraFusionSupported));
                DRX_CAMERA_LOG ("Supports flash: "      + Txt ((i32) [photoOutput.supportedFlashModes containsObject: @(AVCaptureFlashModeOn)]));
                DRX_CAMERA_LOG ("Supports auto flash: " + Txt ((i32) [photoOutput.supportedFlashModes containsObject: @(AVCaptureFlashModeAuto)]));
                DRX_CAMERA_LOG ("Max bracketed photo count: " + Txt (photoOutput.maxBracketedCapturePhotoCount));
                DRX_CAMERA_LOG ("Lens stabilization during bracketed capture supported: " + Txt ((i32) photoOutput.lensStabilizationDuringBracketedCaptureSupported));
                DRX_CAMERA_LOG ("Live photo capture supported: " + Txt ((i32) photoOutput.livePhotoCaptureSupported));

                typesString.clear();

                for (AVFileType type in photoOutput.availablePhotoFileTypes)
                    typesString << nsStringToDrx (type) << " ";

                DRX_CAMERA_LOG ("Available photo file types: " + typesString);

                typesString.clear();

                for (AVFileType type in photoOutput.availableRawPhotoFileTypes)
                    typesString << nsStringToDrx (type) << " ";

                DRX_CAMERA_LOG ("Available RAW photo file types: " + typesString);

                typesString.clear();

                for (AVFileType type in photoOutput.availableLivePhotoVideoCodecTypes)
                    typesString << nsStringToDrx (type) << " ";

                DRX_CAMERA_LOG ("Available live photo video codec types: " + typesString);

                DRX_CAMERA_LOG ("Dual camera dual photo delivery supported: " + Txt ((i32) photoOutput.dualCameraDualPhotoDeliverySupported));
                DRX_CAMERA_LOG ("Camera calibration data delivery supported: " + Txt ((i32) photoOutput.cameraCalibrationDataDeliverySupported));
                DRX_CAMERA_LOG ("Depth data delivery supported: " + Txt ((i32) photoOutput.depthDataDeliverySupported));
            }

            //==============================================================================
            static AVCaptureConnection* findVideoConnection (AVCaptureOutput* output)
            {
                for (AVCaptureConnection* connection in output.connections)
                    for (AVCaptureInputPort* port in connection.inputPorts)
                        if ([port.mediaType isEqual: AVMediaTypeVideo])
                            return connection;

                return nullptr;
            }

            //==============================================================================
            class PhotoOutputDelegateClass : public ObjCClass<NSObject>
            {
            public:
                PhotoOutputDelegateClass() : ObjCClass<NSObject> ("PhotoOutputDelegateClass_")
                {
                    addMethod (@selector (captureOutput:willBeginCaptureForResolvedSettings:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                               {
                                   DRX_CAMERA_LOG ("willBeginCaptureForSettings()");
                               });

                    addMethod (@selector (captureOutput:willCapturePhotoForResolvedSettings:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                               {
                                   DRX_CAMERA_LOG ("willCaptureForSettings()");
                               });

                    addMethod (@selector (captureOutput:didCapturePhotoForResolvedSettings:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                               {
                                   DRX_CAMERA_LOG ("didCaptureForSettings()");
                               });

                    addMethod (@selector (captureOutput:didFinishCaptureForResolvedSettings:error:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*, NSError* error)
                               {
                                   [[maybe_unused]] Txt errorString = error != nil ? nsStringToDrx (error.localizedDescription) : Txt();

                                   DRX_CAMERA_LOG ("didFinishCaptureForSettings(), error = " + errorString);
                               });

                    addMethod (@selector (captureOutput:didFinishProcessingPhoto:error:),
                               [] (id self, SEL, AVCapturePhotoOutput*, AVCapturePhoto* capturePhoto, NSError* error)
                               {
                                   getOwner (self).takingPicture = false;

                                   [[maybe_unused]] Txt errorString = error != nil ? nsStringToDrx (error.localizedDescription) : Txt();

                                   DRX_CAMERA_LOG ("didFinishProcessingPhoto(), error = " + errorString);

                                   if (error != nil)
                                   {
                                       DRX_CAMERA_LOG ("Still picture capture failed, error: " + nsStringToDrx (error.localizedDescription));
                                       jassertfalse;
                                       return;
                                   }

                                   auto* imageOrientation = (NSNumber *) capturePhoto.metadata[(NSString*) kCGImagePropertyOrientation];

                                   auto* uiImage = getImageWithCorrectOrientation ((CGImagePropertyOrientation) imageOrientation.unsignedIntValue,
                                   [capturePhoto CGImageRepresentation]);

                                   auto* imageData = UIImageJPEGRepresentation (uiImage, 0.f);

                                   auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                                   getOwner (self).callListeners (image);

                                   MessageManager::callAsync ([self, image]() { getOwner (self).notifyPictureTaken (image); });
                               });

                    addIvar<StillPictureTaker*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static StillPictureTaker& getOwner (id self) { return *getIvar<StillPictureTaker*> (self, "owner"); }
                static z0 setOwner (id self, StillPictureTaker* t) { object_setInstanceVariable (self, "owner", t); }

            private:
                static UIImage* getImageWithCorrectOrientation (CGImagePropertyOrientation imageOrientation,
                                                                CGImageRef imageData)
                {
                    auto origWidth  = CGImageGetWidth (imageData);
                    auto origHeight = CGImageGetHeight (imageData);

                    auto targetSize = getTargetImageDimensionFor (imageOrientation, imageData);

                    UIGraphicsBeginImageContext (targetSize);
                    CGContextRef context = UIGraphicsGetCurrentContext();

                    switch (imageOrientation)
                    {
                        case kCGImagePropertyOrientationUp:
                            CGContextScaleCTM (context, 1.0, -1.0);
                            CGContextTranslateCTM (context, 0.0, -targetSize.height);
                            break;
                        case kCGImagePropertyOrientationRight:
                            CGContextRotateCTM (context, 90 * MathConstants<CGFloat>::pi / 180);
                            CGContextScaleCTM (context, targetSize.height / (CGFloat) origHeight, -targetSize.width / (CGFloat) origWidth);
                            break;
                        case kCGImagePropertyOrientationDown:
                            CGContextTranslateCTM (context, targetSize.width, 0.0);
                            CGContextScaleCTM (context, -1.0, 1.0);
                            break;
                        case kCGImagePropertyOrientationLeft:
                            CGContextRotateCTM (context, -90 * MathConstants<CGFloat>::pi / 180);
                            CGContextScaleCTM (context, targetSize.height / (CGFloat) origHeight, -targetSize.width / (CGFloat) origWidth);
                            CGContextTranslateCTM (context, -targetSize.width, -targetSize.height);
                            break;
                        case kCGImagePropertyOrientationUpMirrored:
                        case kCGImagePropertyOrientationDownMirrored:
                        case kCGImagePropertyOrientationLeftMirrored:
                        case kCGImagePropertyOrientationRightMirrored:
                        default:
                            // Not implemented.
                            jassertfalse;
                            break;
                    }

                    CGContextDrawImage (context, CGRectMake (0, 0, targetSize.width, targetSize.height), imageData);

                    UIImage* correctedImage = UIGraphicsGetImageFromCurrentImageContext();
                    UIGraphicsEndImageContext();

                    return correctedImage;
                }

                static CGSize getTargetImageDimensionFor (CGImagePropertyOrientation imageOrientation,
                                                          CGImageRef imageData)
                {
                    auto width = CGImageGetWidth (imageData);
                    auto height = CGImageGetHeight (imageData);

                    switch (imageOrientation)
                    {
                        case kCGImagePropertyOrientationUp:
                        case kCGImagePropertyOrientationUpMirrored:
                        case kCGImagePropertyOrientationDown:
                        case kCGImagePropertyOrientationDownMirrored:
                            return CGSizeMake ((CGFloat) width, (CGFloat) height);
                        case kCGImagePropertyOrientationRight:
                        case kCGImagePropertyOrientationRightMirrored:
                        case kCGImagePropertyOrientationLeft:
                        case kCGImagePropertyOrientationLeftMirrored:
                            return CGSizeMake ((CGFloat) height, (CGFloat) width);
                    }

                    jassertfalse;
                    return CGSizeMake ((CGFloat) width, (CGFloat) height);
                }

                static CGImagePropertyOrientation uiImageOrientationToCGImageOrientation (UIImageOrientation orientation)
                {
                    switch (orientation)
                    {
                        case UIImageOrientationUp:            return kCGImagePropertyOrientationUp;
                        case UIImageOrientationDown:          return kCGImagePropertyOrientationDown;
                        case UIImageOrientationLeft:          return kCGImagePropertyOrientationLeft;
                        case UIImageOrientationRight:         return kCGImagePropertyOrientationRight;
                        case UIImageOrientationUpMirrored:    return kCGImagePropertyOrientationUpMirrored;
                        case UIImageOrientationDownMirrored:  return kCGImagePropertyOrientationDownMirrored;
                        case UIImageOrientationLeftMirrored:  return kCGImagePropertyOrientationLeftMirrored;
                        case UIImageOrientationRightMirrored: return kCGImagePropertyOrientationRightMirrored;
                    }
                }
            };

            //==============================================================================
            z0 callListeners (const Image& image)
            {
                captureSession.callListeners (image);
            }

            z0 notifyPictureTaken (const Image& image)
            {
                captureSession.notifyPictureTaken (image);
            }

            CaptureSession& captureSession;
            AVCaptureOutput* captureOutput;

            std::unique_ptr<NSObject, NSObjectDeleter> photoOutputDelegate;

            b8 takingPicture = false;
        };

        //==============================================================================
        // NB: FileOutputRecordingDelegateClass callbacks can be called from any thread (incl.
        // the message thread), so waiting for an event when stopping recording is not an
        // option and VideoRecorder must be alive at all times in order to get stopped
        // recording callback.
        class VideoRecorder
        {
        public:
            explicit VideoRecorder (CaptureSession& session)
                : captureSession (session),
                  movieFileOutput ([AVCaptureMovieFileOutput new]),
                  delegate (nullptr)
            {
                static FileOutputRecordingDelegateClass cls;
                delegate.reset ([cls.createInstance() init]);
                FileOutputRecordingDelegateClass::setOwner (delegate.get(), this);

                session.addOutputIfPossible (movieFileOutput);
            }

            ~VideoRecorder()
            {
                stopRecording();

                // Shutting down a device while recording will stop the recording
                // abruptly and the recording will be lost.
                jassert (! recordingInProgress);
            }

            z0 startRecording (const File& file)
            {
                printVideoOutputDebugInfo (movieFileOutput);

                auto url = [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())
                                      isDirectory: NO];

                auto outputConnection = [movieFileOutput connectionWithMediaType: AVMediaTypeVideo];
                ifelse_17_0<CaptureLayerAngleTrait> (captureSession.cameraDevice, captureSession.previewLayer, outputConnection);

                [movieFileOutput startRecordingToOutputFileURL: url recordingDelegate: delegate.get()];
            }

            z0 stopRecording()
            {
                [movieFileOutput stopRecording];
            }

            Time getTimeOfFirstRecordedFrame() const
            {
                return Time (firstRecordedFrameTimeMs.get());
            }

        private:
            static z0 printVideoOutputDebugInfo ([[maybe_unused]] AVCaptureMovieFileOutput* output)
            {
                DRX_CAMERA_LOG ("Available video codec types:");

               #if DRX_CAMERA_LOG_ENABLED
                for (id type in output.availableVideoCodecTypes)
                    DRX_CAMERA_LOG (nsStringToDrx (type));
               #endif

                DRX_CAMERA_LOG ("Output settings per video connection:");

               #if DRX_CAMERA_LOG_ENABLED
                for (AVCaptureConnection* connection in output.connections)
                    DRX_CAMERA_LOG (nsStringToDrx ([[output outputSettingsForConnection: connection] description]));
               #endif
            }

            //==============================================================================
            struct FileOutputRecordingDelegateClass    : public ObjCClass<NSObject<AVCaptureFileOutputRecordingDelegate>>
            {
                FileOutputRecordingDelegateClass()  : ObjCClass<NSObject<AVCaptureFileOutputRecordingDelegate>> ("FileOutputRecordingDelegateClass_")
                {
                    addMethod (@selector (captureOutput:didStartRecordingToOutputFileAtURL:fromConnections:),
                               [] (id self, SEL, AVCaptureFileOutput*, NSURL*, NSArray<AVCaptureConnection*>*)
                               {
                                   DRX_CAMERA_LOG ("Started recording");

                                   getOwner (self).firstRecordedFrameTimeMs.set (Time::getCurrentTime().toMilliseconds());
                                   getOwner (self).recordingInProgress = true;
                               });

                    addMethod (@selector (captureOutput:didFinishRecordingToOutputFileAtURL:fromConnections:error:),
                               [] (id self, SEL, AVCaptureFileOutput*, NSURL*, NSArray<AVCaptureConnection*>*, NSError* error)
                               {
                                   Txt errorString;
                                   b8 recordingPlayable = true;

                                   // There might have been an error in the recording, yet there may be a playable file...
                                   if ([error code] != noErr)
                                   {
                                       id value = [[error userInfo] objectForKey: AVErrorRecordingSuccessfullyFinishedKey];

                                       if (value != nil && ! [value boolValue])
                                       recordingPlayable = false;

                                       errorString = nsStringToDrx (error.localizedDescription) + ", playable: " + Txt ((i32) recordingPlayable);
                                   }

                                   DRX_CAMERA_LOG ("Stopped recording, error = " + errorString);

                                   getOwner (self).recordingInProgress = false;
                               });

                    addIvar<VideoRecorder*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static VideoRecorder& getOwner (id self)         { return *getIvar<VideoRecorder*> (self, "owner"); }
                static z0 setOwner (id self, VideoRecorder* r) { object_setInstanceVariable (self, "owner", r); }
            };

            CaptureSession& captureSession;
            AVCaptureMovieFileOutput* movieFileOutput;
            std::unique_ptr<NSObject<AVCaptureFileOutputRecordingDelegate>, NSObjectDeleter> delegate;
            b8 recordingInProgress = false;
            Atomic<z64> firstRecordedFrameTimeMs { 0 };
        };

        //==============================================================================
        z0 addOutputIfPossible (AVCaptureOutput* output)
        {
            dispatch_async (captureSessionQueue,^
                            {
                                if ([captureSession.get() canAddOutput: output])
                                {
                                    [captureSession.get() beginConfiguration];
                                    [captureSession.get() addOutput: output];
                                    [captureSession.get() commitConfiguration];

                                    return;
                                }

                                // Can't add output to camera session!
                                jassertfalse;
                            });
        }

        //==============================================================================
        z0 cameraSessionStarted()
        {
            sessionStarted = true;

            owner.cameraSessionStarted();
        }

        z0 cameraSessionRuntimeError (const Txt& error)
        {
            owner.cameraSessionRuntimeError (error);
        }

        z0 callListeners (const Image& image)
        {
            owner.callListeners (image);
        }

        z0 notifyPictureTaken (const Image& image)
        {
            owner.notifyPictureTaken (image);
        }

        Pimpl& owner;

        dispatch_queue_t captureSessionQueue;
        std::unique_ptr<AVCaptureSession, NSObjectDeleter> captureSession;
        std::unique_ptr<NSObject, NSObjectDeleter> delegate;

        StillPictureTaker stillPictureTaker;
        VideoRecorder videoRecorder;

        AVCaptureDevice* cameraDevice = nil;
        AVCaptureVideoPreviewLayer* previewLayer = nil;

        b8 sessionStarted = false;

        WaitableEvent sessionClosedEvent;

        static i32 numCaptureSessions;
    };

    //==============================================================================
    z0 cameraSessionStarted()
    {
        DRX_CAMERA_LOG ("cameraSessionStarted()");

        cameraOpenCallback (cameraId, {});
    }

    z0 cameraSessionRuntimeError (const Txt& error)
    {
        DRX_CAMERA_LOG ("cameraSessionRuntimeError(), error = " + error);

        if (! notifiedOfCameraOpening)
            cameraOpenCallback ({}, error);
        else
            NullCheckedInvocation::invoke (owner.onErrorOccurred, error);
    }

    z0 callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });

        if (listeners.size() == 1)
            triggerStillPictureCapture();
    }

    z0 notifyPictureTaken (const Image& image)
    {
        DRX_CAMERA_LOG ("notifyPictureTaken()");

        NullCheckedInvocation::invoke (pictureTakenCallback, image);
    }

    //==============================================================================
    z0 triggerStillPictureCapture()
    {
        captureSession.takeStillPicture();
    }

    //==============================================================================
    CameraDevice& owner;
    Txt cameraId;
    InternalOpenCameraResultCallback cameraOpenCallback;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    std::function<z0 (const Image&)> pictureTakenCallback;

    CaptureSession captureSession;

    b8 notifiedOfCameraOpening = false;

    friend struct CameraDevice::ViewerComponent;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE (Pimpl)
};

i32 CameraDevice::Pimpl::CaptureSession::numCaptureSessions = 0;

//==============================================================================
struct CameraDevice::ViewerComponent  : public UIViewComponent
{
    //==============================================================================
    struct DrxCameraDeviceViewerClass    : public ObjCClass<UIView>
    {
        DrxCameraDeviceViewerClass()  : ObjCClass ("DrxCameraDeviceViewerClass_")
        {
            addMethod (@selector (layoutSubviews),
                       [] (id self, SEL)
                       {
                           sendSuperclassMessage<z0> (self, @selector (layoutSubviews));

                           UIView* asUIView = (UIView*) self;

                           updateOrientation (self);

                           if (auto* previewLayer = getPreviewLayer (self))
                               previewLayer.frame = asUIView.bounds;
                       });

            addMethod (@selector (observeValueForKeyPath:ofObject:change:context:),
                       [] (id self, SEL, NSString* keyPath, id, NSDictionary*, uk context)
                       {
                           if ([keyPath isEqualToString: @"videoRotationAngleForHorizonLevelPreview"])
                           {
                               if (getPreviewLayer (self) != nullptr)
                               {
                                   auto* viewer = static_cast<ViewerComponent*> (context);
                                   auto& session = viewer->cameraDevice.pimpl->captureSession;
                                   session.updatePreviewOrientation();
                               }
                           }
                       });

            registerClass();
        }

    private:
        static AVCaptureVideoPreviewLayer* getPreviewLayer (id self)
        {
            UIView* asUIView = (UIView*) self;

            if (asUIView.layer.sublayers != nil && [asUIView.layer.sublayers count] > 0)
                if ([asUIView.layer.sublayers[0] isKindOfClass: [AVCaptureVideoPreviewLayer class]])
                     return (AVCaptureVideoPreviewLayer*) asUIView.layer.sublayers[0];

            return nil;
        }

        static z0 updateOrientation (id self)
        {
            if (@available (ios 17, *))
                return;

            if (auto* previewLayer = getPreviewLayer (self))
            {
                UIDeviceOrientation o = [UIDevice currentDevice].orientation;

                if (UIDeviceOrientationIsPortrait (o) || UIDeviceOrientationIsLandscape (o))
                {
                    if (previewLayer.connection != nil)
                    {
                        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
                        previewLayer.connection.videoOrientation = (AVCaptureVideoOrientation) o;
                        DRX_END_IGNORE_WARNINGS_GCC_LIKE
                    }
                }
            }
        }
    };

    struct AddObserverTrait
    {
       #if DRX_IOS_API_VERSION_CAN_BE_BUILT (17, 0)
        API_AVAILABLE (ios (17))
        static z0 newFn (ViewerComponent* self)
        {
            auto& session = self->cameraDevice.pimpl->captureSession;
            self->coordinator.reset ([[AVCaptureDeviceRotationCoordinator alloc] initWithDevice: session.getDevice()
                                                                                   previewLayer: session.getPreviewLayer()]);
            [self->coordinator.get() addObserver: static_cast<UIView*> (self->getView())
                                      forKeyPath: @"videoRotationAngleForHorizonLevelPreview"
                                         options: NSKeyValueObservingOptionNew
                                         context: self];
        }
       #endif

        static z0 oldFn (ViewerComponent*) {}
    };

    struct RemoveObserverTrait
    {
        API_AVAILABLE (ios (17))
        static z0 newFn (ViewerComponent* self)
        {
            if (self->coordinator != nullptr)
            {
                [self->coordinator.get() removeObserver: static_cast<UIView*> (self->getView())
                                             forKeyPath: @"videoRotationAngleForHorizonLevelPreview"];
            }
        }

        static z0 oldFn (ViewerComponent*) {}
    };

    explicit ViewerComponent (CameraDevice& device)
        : cameraDevice (device)
    {
        static DrxCameraDeviceViewerClass cls;

        // Initial size that can be overridden later.
        setSize (640, 480);

        auto view = [cls.createInstance() init];
        setView (view);

        auto* previewLayer = device.pimpl->captureSession.createPreviewLayer();
        previewLayer.frame = view.bounds;

        [view.layer addSublayer: previewLayer];

        ifelse_17_0<AddObserverTrait> (this);
    }

    ~ViewerComponent() override
    {
        ifelse_17_0<RemoveObserverTrait> (this);
    }

    CameraDevice& cameraDevice;

    NSUniquePtr<NSObject> coordinator;
};

//==============================================================================
Txt CameraDevice::getFileExtension()
{
    return ".mov";
}
