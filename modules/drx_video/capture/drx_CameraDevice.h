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

#if DRX_USE_CAMERA || DOXYGEN

//==============================================================================
/**
    Controls any video capture devices that might be available.

    Use getAvailableDevices() to list the devices that are attached to the
    system, then call openDevice() or openDeviceAsync() to open one for use.
    Once you have a CameraDevice object, you can get a viewer component from it,
    and use its methods to stream to a file or capture still-frames.

    @tags{Video}
*/
class DRX_API  CameraDevice
{
public:
    /** Destructor. */
    virtual ~CameraDevice();

    //==============================================================================
    /** Returns a list of the available cameras on this machine.

        You can open one of these devices by calling openDevice() or openDeviceAsync().
    */
    static StringArray getAvailableDevices();

    /** Synchronously opens a camera device. This function should not be used on iOS or
        Android, use openDeviceAsync() instead.

        The index parameter indicates which of the items returned by getAvailableDevices()
        to open.

        The size constraints allow the method to choose between different resolutions if
        the camera supports this. If the resolution can't be specified (e.g. on the Mac)
        then these will be ignored.

        On Mac, if highQuality is false, then the camera will be opened in preview mode
        which will allow the OS to drop frames if the computer cannot keep up in processing
        the frames.
    */
    static CameraDevice* openDevice (i32 deviceIndex,
                                     i32 minWidth = 128, i32 minHeight = 64,
                                     i32 maxWidth = 1024, i32 maxHeight = 768,
                                     b8 highQuality = true);

    using OpenCameraResultCallback = std::function<z0 (CameraDevice*, const Txt& /*error*/)>;

    /** Asynchronously opens a camera device on iOS or Android.
        On other platforms, the function will simply call openDevice(). Upon completion,
        resultCallback will be invoked with valid CameraDevice* and an empty error
        Txt on success, or nullptr CameraDevice and a non-empty error Txt on failure.

        This is the preferred method of opening a camera device, because it works on all
        platforms, whereas synchronous openDevice() does not work on iOS & Android.

        The index parameter indicates which of the items returned by getAvailableDevices()
        to open.

        The size constraints allow the method to choose between different resolutions if
        the camera supports this. If the resolution can't be specified then these will be
        ignored.

        On iOS, if you want to switch a device, it is more efficient to open a new device
        before closing the older one, because this way both devices can share the same
        underlying camera session. Otherwise, the session needs to be close first, and this
        is a lengthy process that can take several seconds.

        The Android implementation currently supports a maximum recording resolution of
        1080p. Choosing a larger size will result in larger pictures taken, but the video
        will be capped at 1080p.
    */
    static z0 openDeviceAsync (i32 deviceIndex,
                                 OpenCameraResultCallback resultCallback,
                                 i32 minWidth = 128, i32 minHeight = 64,
                                 i32 maxWidth = 1024, i32 maxHeight = 768,
                                 b8 highQuality = true);

    //==============================================================================
    /** Returns the name of this device */
    const Txt& getName() const noexcept          { return name; }

    /** Creates a component that can be used to display a preview of the
        video from this camera.

        Note: While you can change the size of the preview component, the actual
        preview display may be smaller than the size requested, because the correct
        aspect ratio is maintained automatically.
    */
    Component* createViewerComponent();

    //==============================================================================
    /** Triggers a still picture capture. Upon completion, pictureTakenCallback will
        be invoked on a message thread.

        On Android, before calling takeStillPicture(), you need to create a preview with
        createViewerComponent() and you need to make it visible on screen.

        Android does not support simultaneous video recording and still picture capture.
     */
    z0 takeStillPicture (std::function<z0 (const Image&)> pictureTakenCallback);

    /** Starts recording video to the specified file.

        You should use getFileExtension() to find out the correct extension to
        use for your filename.

        If the file exists, it will be deleted before the recording starts.

        This method may not start recording instantly, so if you need to know the
        exact time at which the file begins, you can call getTimeOfFirstRecordedFrame()
        after the recording has finished.

        The quality parameter can be 0, 1, or 2, to indicate low, medium, or high. It may
        or may not be used, depending on the driver.

        On Android, before calling startRecordingToFile(), you need to create a preview with
        createViewerComponent() and you need to make it visible on screen.

        The Android camera also requires exclusive access to the audio device, so make sure
        you close any open audio devices with AudioDeviceManager::closeAudioDevice() first.

        Android does not support simultaneous video recording and still picture capture.

        @see AudioDeviceManager::closeAudioDevice, AudioDeviceManager::restartLastAudioDevice
    */
    z0 startRecordingToFile (const File& file, i32 quality = 2);

    /** Stops recording, after a call to startRecordingToFile(). */
    z0 stopRecording();

    /** Returns the file extension that should be used for the files
        that you pass to startRecordingToFile().

        This may be platform-specific, e.g. ".mov" or ".avi".
    */
    static Txt getFileExtension();

    /** After calling stopRecording(), this method can be called to return the timestamp
        of the first frame that was written to the file.
    */
    Time getTimeOfFirstRecordedFrame() const;

    /** Set this callback to be notified whenever an error occurs. You may need to close
        and reopen the device to be able to use it further. */
    std::function<z0 (const Txt& /*error*/)> onErrorOccurred;

    //==============================================================================
    /**
        Receives callbacks with individual frames from a CameraDevice. It is mainly
        useful for processing multiple frames that has to be done as quickly as
        possible. The callbacks can be called from any thread.

        If you just need to take one picture, you should use takeStillPicture() instead.

        @see CameraDevice::addListener
    */
    class DRX_API  Listener
    {
    public:
        Listener() {}
        virtual ~Listener() {}

        /** This method is called when a new image arrives.

            This may be called by any thread, so be careful about thread-safety,
            and make sure that you process the data as quickly as possible to
            avoid glitching!

            Simply add a listener to be continuously notified about new frames becoming
            available and remove the listener when you no longer need new frames.

            If you just need to take one picture, use takeStillPicture() instead.

            @see CameraDevice::takeStillPicture
        */
        virtual z0 imageReceived (const Image& image) = 0;
    };

    /** Adds a listener to receive images from the camera.

        Be very careful not to delete the listener without first removing it by calling
        removeListener().
    */
    z0 addListener (Listener* listenerToAdd);

    /** Removes a listener that was previously added with addListener(). */
    z0 removeListener (Listener* listenerToRemove);

private:
    Txt name;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    struct ViewerComponent;
    friend struct ViewerComponent;

    CameraDevice (const Txt& name, i32 index,
                  i32 minWidth, i32 minHeight, i32 maxWidth, i32 maxHeight, b8 highQuality);

   #if DRX_ANDROID || DRX_IOS
    class CameraFactory;
   #endif

   #if DRX_ANDROID
    friend z0 drx_cameraDeviceStateClosed (z64);
    friend z0 drx_cameraDeviceStateDisconnected (z64);
    friend z0 drx_cameraDeviceStateError (z64, i32);
    friend z0 drx_cameraDeviceStateOpened (z64, uk);

    friend z0 drx_cameraCaptureSessionActive (z64, uk);
    friend z0 drx_cameraCaptureSessionClosed (z64, uk);
    friend z0 drx_cameraCaptureSessionConfigureFailed (z64, uk);
    friend z0 drx_cameraCaptureSessionConfigured (z64, uk);
    friend z0 drx_cameraCaptureSessionReady (z64, uk);

    friend z0 drx_cameraCaptureSessionCaptureCompleted (z64, b8, uk, uk, uk);
    friend z0 drx_cameraCaptureSessionCaptureFailed (z64, b8, uk, uk, uk);
    friend z0 drx_cameraCaptureSessionCaptureProgressed (z64, b8, uk, uk, uk);
    friend z0 drx_cameraCaptureSessionCaptureSequenceAborted (z64, b8, uk, i32);
    friend z0 drx_cameraCaptureSessionCaptureSequenceCompleted (z64, b8, uk, i32, z64);
    friend z0 drx_cameraCaptureSessionCaptureStarted (z64, b8, uk, uk, z64, z64);

    friend z0 drx_deviceOrientationChanged (z64, i32);
   #endif

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDevice)
};

#endif

} // namespace drx
