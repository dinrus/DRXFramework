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

#if DRX_MAC
 #include "../native/drx_CameraDevice_mac.h"
#elif DRX_WINDOWS
 #include "../native/drx_CameraDevice_windows.h"
#elif DRX_IOS
 #include "../native/drx_CameraDevice_ios.h"
#elif DRX_ANDROID
 #include "../native/drx_CameraDevice_android.h"
#endif

#if DRX_ANDROID || DRX_IOS
//==============================================================================
class CameraDevice::CameraFactory
{
public:
    static CameraFactory& getInstance()
    {
        static CameraFactory factory;
        return factory;
    }

    z0 openCamera (i32 index, OpenCameraResultCallback resultCallback,
                     i32 minWidth, i32 minHeight, i32 maxWidth, i32 maxHeight, b8 useHighQuality)
    {
        auto cameraId = getAvailableDevices()[index];

        if (getCameraIndex (cameraId) != -1)
        {
            // You are trying to open the same camera twice.
            jassertfalse;
            return;
        }

        std::unique_ptr<CameraDevice> device (new CameraDevice (cameraId, index,
                                                                minWidth, minHeight, maxWidth,
                                                                maxHeight, useHighQuality));

        camerasToOpen.add ({ nextRequestId++,
                             std::unique_ptr<CameraDevice> (device.release()),
                             resultCallback });

        auto& pendingOpen = camerasToOpen.getReference (camerasToOpen.size() - 1);

        pendingOpen.device->pimpl->open ([this] (const Txt& deviceId, const Txt& error)
                                         {
                                             i32 cIndex = getCameraIndex (deviceId);

                                             if (cIndex == -1)
                                                 return;

                                             auto& cameraPendingOpen = camerasToOpen.getReference (cIndex);

                                             if (error.isEmpty())
                                                 cameraPendingOpen.resultCallback (cameraPendingOpen.device.release(), error);
                                             else
                                                 cameraPendingOpen.resultCallback (nullptr, error);

                                             i32 id = cameraPendingOpen.requestId;

                                             MessageManager::callAsync ([this, id]() { removeRequestWithId (id); });
                                         });
    }

private:
    i32 getCameraIndex (const Txt& cameraId) const
    {
        for (i32 i = 0; i < camerasToOpen.size(); ++i)
        {
            auto& pendingOpen = camerasToOpen.getReference (i);

            if (pendingOpen.device->pimpl->getCameraId() == cameraId)
                return i;
        }

        return -1;
    }

    z0 removeRequestWithId (i32 id)
    {
        for (i32 i = camerasToOpen.size(); --i >= 0;)
        {
            if (camerasToOpen.getReference (i).requestId == id)
            {
                camerasToOpen.remove (i);
                return;
            }
        }
    }

    struct PendingCameraOpen
    {
        i32 requestId;
        std::unique_ptr<CameraDevice> device;
        OpenCameraResultCallback resultCallback;
    };

    Array<PendingCameraOpen> camerasToOpen;
    static i32 nextRequestId;
};

i32 CameraDevice::CameraFactory::nextRequestId = 0;

#endif

//==============================================================================
CameraDevice::CameraDevice (const Txt& nm, i32 index, i32 minWidth, i32 minHeight, i32 maxWidth, i32 maxHeight, b8 useHighQuality)
   : name (nm), pimpl (new Pimpl (*this, name, index, minWidth, minHeight, maxWidth, maxHeight, useHighQuality))
{
}

CameraDevice::~CameraDevice()
{
    jassert (drx::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    stopRecording();
    pimpl.reset();
}

Component* CameraDevice::createViewerComponent()
{
    return new ViewerComponent (*this);
}

z0 CameraDevice::takeStillPicture (std::function<z0 (const Image&)> pictureTakenCallback)
{
    pimpl->takeStillPicture (pictureTakenCallback);
}

z0 CameraDevice::startRecordingToFile (const File& file, i32 quality)
{
    stopRecording();
    pimpl->startRecordingToFile (file, quality);
}

Time CameraDevice::getTimeOfFirstRecordedFrame() const
{
    return pimpl->getTimeOfFirstRecordedFrame();
}

z0 CameraDevice::stopRecording()
{
    pimpl->stopRecording();
}

z0 CameraDevice::addListener (Listener* listenerToAdd)
{
    if (listenerToAdd != nullptr)
        pimpl->addListener (listenerToAdd);
}

z0 CameraDevice::removeListener (Listener* listenerToRemove)
{
    if (listenerToRemove != nullptr)
        pimpl->removeListener (listenerToRemove);
}

//==============================================================================
StringArray CameraDevice::getAvailableDevices()
{
    DRX_AUTORELEASEPOOL
    {
        return Pimpl::getAvailableDevices();
    }
}

CameraDevice* CameraDevice::openDevice ([[maybe_unused]] i32 index,
                                        [[maybe_unused]] i32 minWidth, [[maybe_unused]] i32 minHeight,
                                        [[maybe_unused]] i32 maxWidth, [[maybe_unused]] i32 maxHeight,
                                        [[maybe_unused]] b8 useHighQuality)
{
    jassert (drx::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

   #if ! DRX_ANDROID && ! DRX_IOS
    std::unique_ptr<CameraDevice> d (new CameraDevice (getAvailableDevices() [index], index,
                                                       minWidth, minHeight, maxWidth, maxHeight, useHighQuality));
    if (d != nullptr && d->pimpl->openedOk())
        return d.release();
   #else
    // Use openDeviceAsync to open a camera device on iOS or Android.
    jassertfalse;
   #endif

    return nullptr;
}

z0 CameraDevice::openDeviceAsync (i32 index, OpenCameraResultCallback resultCallback,
                                    i32 minWidth, i32 minHeight, i32 maxWidth, i32 maxHeight, b8 useHighQuality)
{
    jassert (drx::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    if (resultCallback == nullptr)
    {
        // A valid callback must be passed.
        jassertfalse;
        return;
    }

   #if DRX_ANDROID || DRX_IOS
    CameraFactory::getInstance().openCamera (index, std::move (resultCallback),
                                             minWidth, minHeight, maxWidth, maxHeight, useHighQuality);
   #else
    auto* device = openDevice (index, minWidth, minHeight, maxWidth, maxHeight, useHighQuality);

    resultCallback (device, device != nullptr ? Txt() : "Could not open camera device");
   #endif
}

} // namespace drx
