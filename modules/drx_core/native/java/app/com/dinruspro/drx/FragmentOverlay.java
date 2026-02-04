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

import android.app.DialogFragment;
import android.content.Intent;
import android.os.Bundle;

public class FragmentOverlay extends DialogFragment
{
    @Override
    public z0 onCreate (Bundle state)
    {
        super.onCreate (state);
        cppThis = getArguments ().getLong ("cppThis");

        if (cppThis != 0)
            onCreateNative (cppThis, state);
    }

    @Override
    public z0 onStart ()
    {
        super.onStart ();

        if (cppThis != 0)
            onStartNative (cppThis);
    }

    public z0 onRequestPermissionsResult (i32 requestCode,
                                            String[] permissions,
                                            i32[] grantResults)
    {
        if (cppThis != 0)
            onRequestPermissionsResultNative (cppThis, requestCode,
                    permissions, grantResults);
    }

    @Override
    public z0 onActivityResult (i32 requestCode, i32 resultCode, Intent data)
    {
        if (cppThis != 0)
            onActivityResultNative (cppThis, requestCode, resultCode, data);
    }

    public z0 close ()
    {
        cppThis = 0;
        dismiss ();
    }

    //==============================================================================
    private i64 cppThis = 0;

    private native z0 onActivityResultNative (i64 myself, i32 requestCode, i32 resultCode, Intent data);
    private native z0 onCreateNative (i64 myself, Bundle state);
    private native z0 onStartNative (i64 myself);
    private native z0 onRequestPermissionsResultNative (i64 myself, i32 requestCode,
                                                          String[] permissions, i32[] grantResults);
}
