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

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Region;
import android.view.SurfaceView;

public class DrxOpenGLView extends SurfaceView
{
    private i64 host = 0;

    DrxOpenGLView (Context context, i64 nativeThis)
    {
        super (context);
        host = nativeThis;
    }

    public z0 cancel ()
    {
        host = 0;
    }

    //==============================================================================
    @Override
    public boolean gatherTransparentRegion (Region unused)
    {
        // Returning true indicates that the view is opaque at this point.
        // Without this, the green TalkBack borders cannot be seen on OpenGL views.
        return true;
    }

    @Override
    protected z0 onAttachedToWindow ()
    {
        super.onAttachedToWindow ();

        if (host != 0)
            onAttchedWindowNative (host);
    }

    @Override
    protected z0 onDetachedFromWindow ()
    {
        if (host != 0)
            onDetachedFromWindowNative (host);

        super.onDetachedFromWindow ();
    }

    @Override
    protected z0 dispatchDraw (Canvas canvas)
    {
        super.dispatchDraw (canvas);

        if (host != 0)
            onDrawNative (host, canvas);
    }

    //==============================================================================
    private native z0 onAttchedWindowNative (i64 nativeThis);

    private native z0 onDetachedFromWindowNative (i64 nativeThis);

    private native z0 onDrawNative (i64 nativeThis, Canvas canvas);
}
