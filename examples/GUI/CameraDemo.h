/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             CameraDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases camera features.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_core, drx_cryptography,
                   drx_data_structures, drx_events, drx_graphics, drx_gui_basics,
                   drx_gui_extra, drx_video
 exporters:        xcode_mac, vs2022, androidstudio, xcode_iphone

 moduleFlags:      DRX_USE_CAMERA=1, DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CameraDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class CameraDemo final : public Component
{
public:
    CameraDemo()
    {
        setOpaque (true);

       #if DRX_ANDROID
        // Android requires exclusive access to the audio device when recording videos.
        audioDeviceManager.closeAudioDevice();
       #endif

        addAndMakeVisible (cameraSelectorComboBox);
        updateCameraList();
        cameraSelectorComboBox.setSelectedId (1);
        cameraSelectorComboBox.onChange = [this] { cameraChanged(); };

        addAndMakeVisible (snapshotButton);
        snapshotButton.onClick = [this] { takeSnapshot(); };
        snapshotButton.setEnabled (false);

        addAndMakeVisible (recordMovieButton);
        recordMovieButton.onClick = [this] { startRecording(); };
        recordMovieButton.setEnabled (false);

        addAndMakeVisible (lastSnapshot);

        cameraSelectorComboBox.setSelectedId (2);

        setSize (500, 500);

       #if DRX_IOS || DRX_ANDROID
        setPortraitOrientationEnabled (true);
       #endif
    }

    ~CameraDemo() override
    {
       #if DRX_IOS || DRX_ANDROID
        setPortraitOrientationEnabled (false);
       #endif

       #if DRX_ANDROID
        audioDeviceManager.restartLastAudioDevice();
       #endif
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (5);

        auto top = r.removeFromTop (25);
        cameraSelectorComboBox.setBounds (top.removeFromLeft (250));

        r.removeFromTop (4);
        top = r.removeFromTop (25);

        snapshotButton.changeWidthToFitText (24);
        snapshotButton.setBounds (top.removeFromLeft (snapshotButton.getWidth()));
        top.removeFromLeft (4);
        recordMovieButton.changeWidthToFitText (24);
        recordMovieButton.setBounds (top.removeFromLeft (recordMovieButton.getWidth()));

        r.removeFromTop (4);
        auto previewArea = shouldUseLandscapeLayout() ? r.removeFromLeft (r.getWidth() / 2)
                                                      : r.removeFromTop (r.getHeight() / 2);

        if (cameraPreviewComp != nullptr)
            cameraPreviewComp->setBounds (previewArea);

        if (shouldUseLandscapeLayout())
            r.removeFromLeft (4);
        else
            r.removeFromTop (4);

        lastSnapshot.setBounds (r);
    }


private:
    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    std::unique_ptr<CameraDevice> cameraDevice;
    std::unique_ptr<Component> cameraPreviewComp;
    ImageComponent lastSnapshot;

    ComboBox cameraSelectorComboBox  { "Camera" };
    TextButton snapshotButton        { "Take a snapshot" };
   #if ! DRX_ANDROID && ! DRX_IOS
    TextButton recordMovieButton     { "Record a movie (to your desktop)..." };
   #else
    TextButton recordMovieButton     { "Record a movie" };
   #endif
    b8 recordingMovie = false;
    File recordingFile;
    b8 contentSharingPending = false;

    z0 setPortraitOrientationEnabled (b8 shouldBeEnabled)
    {
        auto allowedOrientations = Desktop::getInstance().getOrientationsEnabled();

        if (shouldBeEnabled)
            allowedOrientations |= Desktop::upright;
        else
            allowedOrientations &= ~Desktop::upright;

        Desktop::getInstance().setOrientationsEnabled (allowedOrientations);
    }

    b8 shouldUseLandscapeLayout() const noexcept
    {
       #if DRX_ANDROID || DRX_IOS
        auto orientation = Desktop::getInstance().getCurrentOrientation();
        return orientation == Desktop::rotatedClockwise || orientation == Desktop::rotatedAntiClockwise;
       #else
        return false;
       #endif
    }

    z0 updateCameraList()
    {
        cameraSelectorComboBox.clear();
        cameraSelectorComboBox.addItem ("No camera", 1);
        cameraSelectorComboBox.addSeparator();

        auto cameras = CameraDevice::getAvailableDevices();

        for (i32 i = 0; i < cameras.size(); ++i)
            cameraSelectorComboBox.addItem (cameras[i], i + 2);
    }

    z0 cameraChanged()
    {
        // This is called when the user chooses a camera from the drop-down list.
       #if DRX_IOS
        // On iOS, when switching camera, open the new camera first, so that it can
        // share the underlying camera session with the old camera. Otherwise, the
        // session would have to be closed first, which can take several seconds.
        if (cameraSelectorComboBox.getSelectedId() == 1)
            cameraDevice.reset();
       #else
        cameraDevice.reset();
       #endif
        cameraPreviewComp.reset();
        recordingMovie = false;

        if (cameraSelectorComboBox.getSelectedId() > 1)
        {
           #if DRX_ANDROID || DRX_IOS
            openCameraAsync();
           #else
            cameraDeviceOpenResult (CameraDevice::openDevice (cameraSelectorComboBox.getSelectedId() - 2), {});
           #endif
        }
        else
        {
            snapshotButton   .setEnabled (cameraDevice != nullptr && ! contentSharingPending);
            recordMovieButton.setEnabled (cameraDevice != nullptr && ! contentSharingPending);
            resized();
        }
    }

    z0 openCameraAsync()
    {
        SafePointer<CameraDemo> safeThis (this);

        CameraDevice::openDeviceAsync (cameraSelectorComboBox.getSelectedId() - 2,
                                       [safeThis] (CameraDevice* device, const Txt& error) mutable
                                       {
                                           if (safeThis)
                                               safeThis->cameraDeviceOpenResult (device, error);
                                       });
    }

    z0 cameraDeviceOpenResult (CameraDevice* device, const Txt& error)
    {
        // If camera opening worked, create a preview component for it..
        cameraDevice.reset (device);

        if (cameraDevice.get() != nullptr)
        {
           #if DRX_ANDROID
            SafePointer<CameraDemo> safeThis (this);
            cameraDevice->onErrorOccurred = [safeThis] (const Txt& cameraError) mutable { if (safeThis) safeThis->errorOccurred (cameraError); };
           #endif
            cameraPreviewComp.reset (cameraDevice->createViewerComponent());
            addAndMakeVisible (cameraPreviewComp.get());
        }
        else
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "Camera open failed",
                                                             "Camera open failed, reason: " + error);
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }

        snapshotButton   .setEnabled (cameraDevice.get() != nullptr && ! contentSharingPending);
        recordMovieButton.setEnabled (cameraDevice.get() != nullptr && ! contentSharingPending);
        resized();
    }

    z0 startRecording()
    {
        if (cameraDevice.get() != nullptr)
        {
            // The user has clicked the record movie button..
            if (! recordingMovie)
            {
                // Start recording to a file on the user's desktop..
                recordingMovie = true;

               #if DRX_ANDROID || DRX_IOS
                recordingFile = File::getSpecialLocation (File::tempDirectory)
               #else
                recordingFile = File::getSpecialLocation (File::userDesktopDirectory)
               #endif
                                 .getNonexistentChildFile ("DrxCameraVideoDemo", CameraDevice::getFileExtension());

               #if DRX_ANDROID
                // Android does not support taking pictures while recording video.
                snapshotButton.setEnabled (false);
               #endif

                cameraSelectorComboBox.setEnabled (false);
                cameraDevice->startRecordingToFile (recordingFile);
                recordMovieButton.setButtonText ("Stop Recording");
            }
            else
            {
                // Already recording, so stop...
                recordingMovie = false;
                cameraDevice->stopRecording();
               #if ! DRX_ANDROID && ! DRX_IOS
                recordMovieButton.setButtonText ("Start recording (to a file on your desktop)");
               #else
                recordMovieButton.setButtonText ("Record a movie");
               #endif
                cameraSelectorComboBox.setEnabled (true);

               #if DRX_ANDROID
                snapshotButton.setEnabled (true);
               #endif

               #if DRX_CONTENT_SHARING
                URL url (recordingFile);

                snapshotButton   .setEnabled (false);
                recordMovieButton.setEnabled (false);
                contentSharingPending = true;

                SafePointer<CameraDemo> safeThis (this);

                messageBox = ContentSharer::shareFilesScoped ({ url }, [safeThis] (b8 success, const Txt&)
                {
                    if (safeThis)
                        safeThis->sharingFinished (success, false);
                });
               #endif
            }
        }
    }

    z0 takeSnapshot()
    {
        SafePointer<CameraDemo> safeThis (this);
        cameraDevice->takeStillPicture ([safeThis] (const Image& image) mutable { safeThis->imageReceived (image); });
    }

    // This is called by the camera device when a new image arrives
    z0 imageReceived (const Image& image)
    {
        if (! image.isValid())
            return;

        lastSnapshot.setImage (image);

       #if DRX_CONTENT_SHARING
        auto imageFile = File::getSpecialLocation (File::tempDirectory).getNonexistentChildFile ("DrxCameraPhotoDemo", ".jpg");

        FileOutputStream stream (imageFile);

        if (stream.openedOk()
             && JPEGImageFormat().writeImageToStream (image, stream))
        {
            URL url (imageFile);

            snapshotButton   .setEnabled (false);
            recordMovieButton.setEnabled (false);
            contentSharingPending = true;

            SafePointer<CameraDemo> safeThis (this);

            messageBox = ContentSharer::shareFilesScoped ({ url }, [safeThis] (b8 success, const Txt&)
            {
                if (safeThis)
                    safeThis->sharingFinished (success, true);
            });
        }
       #endif
    }

    z0 errorOccurred (const Txt& error)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                         "Camera Device Error",
                                                         "An error has occurred: " + error + " Camera will be closed.");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);

        cameraDevice.reset();

        cameraSelectorComboBox.setSelectedId (1);
        snapshotButton   .setEnabled (false);
        recordMovieButton.setEnabled (false);
    }

    z0 sharingFinished (b8 success, b8 isCapture)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                         isCapture ? "Image sharing result" : "Video sharing result",
                                                         success ? "Success!" : "Failed!");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);

        contentSharingPending = false;
        snapshotButton   .setEnabled (true);
        recordMovieButton.setEnabled (true);
    }

    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDemo)
};
