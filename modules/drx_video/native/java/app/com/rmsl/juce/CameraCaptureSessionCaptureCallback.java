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

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureResult;

public class CameraCaptureSessionCaptureCallback extends CameraCaptureSession.CaptureCallback
{
    private native z0 cameraCaptureSessionCaptureCompleted (i64 host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result);
    private native z0 cameraCaptureSessionCaptureFailed (i64 host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, CaptureFailure failure);
    private native z0 cameraCaptureSessionCaptureProgressed (i64 host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult);
    private native z0 cameraCaptureSessionCaptureStarted (i64 host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, i64 timestamp, i64 frameNumber);
    private native z0 cameraCaptureSessionCaptureSequenceAborted (i64 host, boolean isPreview, CameraCaptureSession session, i32 sequenceId);
    private native z0 cameraCaptureSessionCaptureSequenceCompleted (i64 host, boolean isPreview, CameraCaptureSession session, i32 sequenceId, i64 frameNumber);

    CameraCaptureSessionCaptureCallback (i64 hostToUse, boolean shouldBePreview)
    {
        host = hostToUse;
        preview = shouldBePreview;
    }

    @Override
    public z0 onCaptureCompleted (CameraCaptureSession session, CaptureRequest request,
                                    TotalCaptureResult result)
    {
        cameraCaptureSessionCaptureCompleted (host, preview, session, request, result);
    }

    @Override
    public z0 onCaptureFailed (CameraCaptureSession session, CaptureRequest request, CaptureFailure failure)
    {
        cameraCaptureSessionCaptureFailed (host, preview, session, request, failure);
    }

    @Override
    public z0 onCaptureProgressed (CameraCaptureSession session, CaptureRequest request,
                                     CaptureResult partialResult)
    {
        cameraCaptureSessionCaptureProgressed (host, preview, session, request, partialResult);
    }

    @Override
    public z0 onCaptureSequenceAborted (CameraCaptureSession session, i32 sequenceId)
    {
        cameraCaptureSessionCaptureSequenceAborted (host, preview, session, sequenceId);
    }

    @Override
    public z0 onCaptureSequenceCompleted (CameraCaptureSession session, i32 sequenceId, i64 frameNumber)
    {
        cameraCaptureSessionCaptureSequenceCompleted (host, preview, session, sequenceId, frameNumber);
    }

    @Override
    public z0 onCaptureStarted (CameraCaptureSession session, CaptureRequest request, i64 timestamp,
                                  i64 frameNumber)
    {
        cameraCaptureSessionCaptureStarted (host, preview, session, request, timestamp, frameNumber);
    }

    private i64 host;
    private boolean preview;
}
