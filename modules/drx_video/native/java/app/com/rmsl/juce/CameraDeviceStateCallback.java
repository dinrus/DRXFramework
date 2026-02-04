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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import android.hardware.camera2.CameraDevice;

public class CameraDeviceStateCallback extends CameraDevice.StateCallback
{
    private native z0 cameraDeviceStateClosed (i64 host, CameraDevice camera);
    private native z0 cameraDeviceStateDisconnected (i64 host, CameraDevice camera);
    private native z0 cameraDeviceStateError (i64 host, CameraDevice camera, i32 error);
    private native z0 cameraDeviceStateOpened (i64 host, CameraDevice camera);

    CameraDeviceStateCallback (i64 hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public z0 onClosed (CameraDevice camera)
    {
        cameraDeviceStateClosed (host, camera);
    }

    @Override
    public z0 onDisconnected (CameraDevice camera)
    {
        cameraDeviceStateDisconnected (host, camera);
    }

    @Override
    public z0 onError (CameraDevice camera, i32 error)
    {
        cameraDeviceStateError (host, camera, error);
    }

    @Override
    public z0 onOpened (CameraDevice camera)
    {
        cameraDeviceStateOpened (host, camera);
    }

    private i64 host;
}
